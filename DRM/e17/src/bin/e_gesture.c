#include <e.h>

#ifdef HAVE_ELPUT

#include <Eina.h>
#include <grp.h>
#include <sys/types.h>
#include <pwd.h>
#include <Elput.h>

static Eina_Hash *active_gestures;
static Elput_Manager *manager;

typedef struct {
   Eina_Vector2 pos;
   unsigned int fingers;
   struct {
     Evas_Object *visuals, *win;
   } visuals;
} Swipe_Stats;

static int gesture_capable_devices = 0;
static E_Bindings_Swipe_Live_Update live_update;
static void* live_update_data;

static Swipe_Stats*
_find_swipe_gesture_recognizition(Elput_Device *dev)
{
   Swipe_Stats *stats = eina_hash_find(active_gestures, dev);

   return stats;
}

static Swipe_Stats*
_start_swipe_gesture_recognizition(Elput_Device *dev)
{
   Swipe_Stats *stats = _find_swipe_gesture_recognizition(dev);

   if (stats)
     eina_hash_del_by_key(active_gestures, dev);

   stats = calloc(1, sizeof(Swipe_Stats));
   EINA_SAFETY_ON_NULL_RETURN_VAL(stats, NULL);

   if (e_bindings_swipe_available())
     {
        E_Zone *zone = e_zone_current_get();

        stats->visuals.win =  elm_notify_add(zone->base);
        elm_notify_align_set(stats->visuals.win, 0.5, 0.5);
        elm_object_tree_focus_allow_set(stats->visuals.win, EINA_FALSE);
        evas_object_layer_set(stats->visuals.win, E_LAYER_CLIENT_PRIO);
        evas_object_show(stats->visuals.win);

        stats->visuals.visuals = elm_progressbar_add(stats->visuals.win);
        elm_object_text_set(stats->visuals.visuals, "Progress of visuals");
        evas_object_size_hint_min_set(stats->visuals.visuals, 300, 50);
        evas_object_show(stats->visuals.visuals);
        elm_object_content_set(stats->visuals.win, stats->visuals.visuals);
     }


   eina_hash_add(active_gestures, dev, stats);

   return stats;
}

static void
_end_swipe_gesture_recognizition(Elput_Device *dev)
{
   eina_hash_del_by_key(active_gestures, dev);
}

static double
_config_angle(Eina_Vector2 pos)
{
   double res = atan(pos.y/pos.x);

   if (res < 0) res += M_PI;
   if (pos.y < 0) res += M_PI;
   return res;
}

static void
_stats_free(void *ptr)
{
   Swipe_Stats *stats = ptr;

   evas_object_del(stats->visuals.win);
   free(stats);
}

static void
_apply_visual_changes(Swipe_Stats *stats)
{
   if (live_update)
     {
        live_update(live_update_data, EINA_FALSE, _config_angle(stats->pos), eina_vector2_length_get(&stats->pos), 0.8, stats->fingers);
     }
   else if (stats->visuals.win)
     {
        Eina_Inarray *res = e_bindings_swipe_find_candidates(E_BINDING_CONTEXT_NONE, _config_angle (stats->pos), eina_vector2_length_get(&stats->pos), stats->fingers);
        E_Binding_Swipe_Candidate *itr;
        double total = 0.0f;
        unsigned int len = 0;

        EINA_INARRAY_FOREACH(res, itr)
          {
             total += itr->acceptance;
             len ++;
          }

        if (len > 0)
          {
             char text_buffer[1000];

             snprintf(text_buffer, sizeof(text_buffer), "%d gestures possible", len);
             elm_progressbar_value_set(stats->visuals.visuals, total/len);
             elm_object_text_set(stats->visuals.visuals, text_buffer);
          }
        else
          {
             elm_progressbar_value_set(stats->visuals.visuals, 0.0f);
             elm_object_text_set(stats->visuals.visuals, "No gesture found");
          }

        eina_inarray_free(res);
     }
}

static Eina_Bool
_swipe_cb(void *data EINA_UNUSED, int type, void *event)
{
   Elput_Swipe_Gesture *gesture = event;
   Elput_Device *dev = elput_swipe_device_get(gesture);

   if (type == ELPUT_EVENT_SWIPE_BEGIN)
     {
        Swipe_Stats *stats = _start_swipe_gesture_recognizition(dev);
        stats->fingers = elput_swipe_finger_count_get(gesture);
        stats->pos.x = stats->pos.y = 0;
     }
   else if (type == ELPUT_EVENT_SWIPE_UPDATE)
     {
        Swipe_Stats *stats = _find_swipe_gesture_recognizition(dev);

        stats->pos.x += elput_swipe_dx_get(gesture);
        stats->pos.y += elput_swipe_dy_get(gesture);

        _apply_visual_changes(stats);
     }
   else if (type == ELPUT_EVENT_SWIPE_END)
     {
        Swipe_Stats *stats = _find_swipe_gesture_recognizition(dev);

        if (live_update)
          live_update(live_update_data, EINA_TRUE, _config_angle(stats->pos), eina_vector2_length_get(&stats->pos), 0.8, stats->fingers);
        else
          e_bindings_swipe_handle(E_BINDING_CONTEXT_NONE, NULL, _config_angle(stats->pos), eina_vector2_length_get(&stats->pos), stats->fingers);

        _end_swipe_gesture_recognizition(dev);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_debug(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Elput_Event_Seat_Caps *caps = event;
   const Eina_List *node, *devices = elput_seat_devices_get(caps->seat);
   Elput_Device *device;
   int number_of_gesture_devices = 0;

   EINA_LIST_FOREACH(devices, node, device)
     {
        if (elput_device_caps_get(device) & ELPUT_DEVICE_CAPS_GESTURE)
          {
             number_of_gesture_devices++;
          }
     }
   gesture_capable_devices= number_of_gesture_devices;
   return ECORE_CALLBACK_PASS_ON;
}

static void
_init_for_x11(void)
{
   const char *device = NULL;

   if (!elput_init())
     {
        ERR("Failed to init elput");
        return;
     }
   device = getenv("XDG_SEAT");
   if (!device) device = "seat0";
   manager = elput_manager_connect_gestures(device, 0);

   EINA_SAFETY_ON_NULL_RETURN(manager);
   elput_input_init(manager);
}

static void
_shutdown_for_x11(void)
{
   elput_manager_disconnect(manager);
   manager = NULL;
   elput_shutdown();
}


E_API int
e_gesture_init(void)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        _init_for_x11();
     }

   active_gestures = eina_hash_pointer_new(_stats_free);

   ecore_event_handler_add(ELPUT_EVENT_SWIPE_BEGIN, _swipe_cb, NULL);
   ecore_event_handler_add(ELPUT_EVENT_SWIPE_UPDATE, _swipe_cb, NULL);
   ecore_event_handler_add(ELPUT_EVENT_SWIPE_END, _swipe_cb, NULL);
   ecore_event_handler_add(ELPUT_EVENT_SEAT_CAPS, _debug, NULL);

   return 1;
}

E_API int
e_gesture_shutdown(void)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        _shutdown_for_x11();
     }

   return 1;
}

E_API void
e_bindings_swipe_live_update_hook_set(E_Bindings_Swipe_Live_Update update, void *data)
{
   live_update = update;
   live_update_data = data;
}

E_API int
e_bindings_gesture_capable_devices_get(void)
{
   return gesture_capable_devices;
}

#endif
