#include "e.h"

E_API int E_EVENT_INIT_DONE = 0;

/* local variables */
static int done = 0;
static int undone = 0;
static Evas_Object *_e_init_object = NULL;
static Eina_List *splash_objs = NULL;
static Ecore_Timer *_e_init_timeout_timer = NULL;
static Ecore_Job *_e_init_update_job = NULL;
static Ecore_Event_Handler *_e_init_event_zone_add = NULL;
static Ecore_Event_Handler *_e_init_event_zone_del = NULL;
static Ecore_Event_Handler *_e_init_event_zone_move_resize = NULL;
static Eina_Bool _pre_called = EINA_FALSE;

static Eina_Bool
_e_init_cb_timeout(void *data EINA_UNUSED)
{
   _e_init_timeout_timer = NULL;
   e_init_hide();
   return ECORE_CALLBACK_CANCEL;
}

static void
_e_init_cb_signal_done_ok(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   e_init_hide();
}

static void
_e_init_render_pre(void *data EINA_UNUSED, Evas *e, void *info EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *o;

   _pre_called = EINA_TRUE;
   evas_event_callback_del(e, EVAS_CALLBACK_RENDER_PRE, _e_init_render_pre);
   EINA_LIST_FOREACH(splash_objs, l, o)
     {
        evas_object_show(o);
     }
}

static E_Zone *
_get_zone_num(int num)
{
   E_Zone *zone;
   Eina_List *l;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if ((int)zone->num == num) return zone;
     }
   return NULL;
}

static Evas_Object *
_get_splash_num(int num)
{
   Evas_Object *o;
   Eina_List *l;

   EINA_LIST_FOREACH(splash_objs, l, o)
     {
        int n = (intptr_t)evas_object_data_get(o, "num");
        if (n > 0)
          {
             n--;
             if (num == n) return o;
          }
     }
   return NULL;
}

static void
_e_init_splash_obj_zone_update(Evas_Object *o, E_Zone *zone)
{
   evas_object_clip_set(o, zone->bg_clip_object);
   evas_object_move(o, zone->x, zone->y);
   evas_object_resize(o, zone->w, zone->h);
   evas_object_layer_set(o, E_LAYER_MAX - 1000);
}

static Evas_Object *
_e_init_splash_obj_new(E_Zone *zone)
{
   Evas_Object *o;

   o = edje_object_add(e_comp->evas);
   evas_object_data_set(o, "num", (void *)(intptr_t)(zone->num + 1));
   if (!zone->num)
     {
        e_theme_edje_object_set(o, NULL, "e/init/splash");
        edje_object_part_text_set(o, "e.text.disable_text", "");
        edje_object_signal_callback_add(o, "e,state,done_ok", "e",
                                        _e_init_cb_signal_done_ok, NULL);
     }
   else
     e_theme_edje_object_set(o, NULL, "e/init/extra_screen");
   _e_init_splash_obj_zone_update(o, zone);
   splash_objs = eina_list_append(splash_objs, o);
   return o;
}

static void
_e_init_zone_change_job(void *data EINA_UNUSED)
{
   Evas_Object *o;
   E_Zone *zone;
   Eina_List *l, *ll;

   _e_init_update_job = NULL;
   if (done > 0) return;
   // pass 1 - delete splash objects for zones that have gone OR
   // update the zone obj to have the right clip and geometry
   EINA_LIST_FOREACH_SAFE(splash_objs, l, ll, o)
     {
        int num = (intptr_t)evas_object_data_get(o, "num");
        if (num > 0)
          {
             num--;
             zone = _get_zone_num(num);
             if (!zone)
               {
                  if (o == _e_init_object)
                    {
                       if (_e_init_timeout_timer)
                         {
                            ecore_timer_del(_e_init_timeout_timer);
                            _e_init_timeout_timer =
                              ecore_timer_add(2.0, _e_init_cb_timeout, NULL);
                         }
                       _e_init_object = NULL;
                    }
                  splash_objs = eina_list_remove_list(splash_objs, l);
                  evas_object_del(o);
               }
             else _e_init_splash_obj_zone_update(o, zone);
          }
        // something went wrong - so delete it
        else
          {
             if (o == _e_init_object) _e_init_object = NULL;
             splash_objs = eina_list_remove_list(splash_objs, l);
             evas_object_del(o);
          }
     }
   // pass 2 - add splash objects for new zones
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        o = _get_splash_num(zone->num);
        if (!o) // no splash obj for this zone, add one
          {
             o = _e_init_splash_obj_new(zone);
             if (_pre_called) evas_object_show(o);
          }
     }
}

static Eina_Bool
_e_init_zone_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   if (_e_init_update_job) ecore_job_del(_e_init_update_job);
   _e_init_update_job = ecore_job_add(_e_init_zone_change_job, NULL);
   return ECORE_CALLBACK_PASS_ON;
}

/* public functions */
EINTERN int
e_init_init(void)
{
   E_EVENT_INIT_DONE = ecore_event_type_new();

   done = 0;
   return 1;
}

EINTERN int
e_init_shutdown(void)
{
   /* if not killed, kill init */
   ecore_event_handler_del(_e_init_event_zone_add);
   ecore_event_handler_del(_e_init_event_zone_del);
   if (_e_init_event_zone_move_resize)
     {
        ecore_event_handler_del(_e_init_event_zone_move_resize);
        _e_init_event_zone_move_resize = NULL;
     }
   _e_init_event_zone_add = NULL;
   _e_init_event_zone_del = NULL;
   e_init_hide();
   return 1;
}

E_API void
e_init_show(void)
{
   Evas_Object *o;
   E_Zone *zone;
   Eina_List *l;
   /* exec init */

   _e_init_event_zone_add =
     ecore_event_handler_add(E_EVENT_ZONE_ADD, _e_init_zone_change, NULL);
   _e_init_event_zone_del =
     ecore_event_handler_add(E_EVENT_ZONE_DEL, _e_init_zone_change, NULL);
   _e_init_event_zone_move_resize =
     ecore_event_handler_add(E_EVENT_ZONE_MOVE_RESIZE, _e_init_zone_change, NULL);
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        o = _e_init_splash_obj_new(zone);
        if (!zone->num) _e_init_object = o;
     }
   evas_event_callback_add
     (e_comp->evas, EVAS_CALLBACK_RENDER_PRE, _e_init_render_pre, NULL);
   _e_init_timeout_timer = ecore_timer_loop_add(60.0, _e_init_cb_timeout, NULL);
   e_init_title_set(_("Enlightenment"));
   e_init_version_set(VERSION);
}

E_API void
e_init_hide(void)
{
   if (_e_init_event_zone_move_resize)
     {
        ecore_event_handler_del(_e_init_event_zone_move_resize);
        _e_init_event_zone_move_resize = NULL;
     }
   E_FREE_LIST(splash_objs, evas_object_del);
   e_comp_shape_queue();
   _e_init_object = NULL;
   E_FREE_FUNC(_e_init_timeout_timer, ecore_timer_del);
   E_FREE_FUNC(_e_init_update_job, ecore_job_del);
}

E_API void
e_init_title_set(const char *str)
{
   if (!_e_init_object) return;
   edje_object_part_text_set(_e_init_object, "e.text.title", str);
}

E_API void
e_init_version_set(const char *str)
{
   if (!_e_init_object) return;
   edje_object_part_text_set(_e_init_object, "e.text.version", str);
}

E_API void
e_init_status_set(const char *str)
{
   if (!_e_init_object) return;
   edje_object_part_text_set(_e_init_object, "e.text.status", str);
}

E_API void
e_init_done(void)
{
   Eina_List *l;
   Evas_Object *o;

   undone--;
   if (undone > 0) return;
   if (!done)
     {
        done = 1;
        ecore_event_add(E_EVENT_INIT_DONE, NULL, NULL, NULL);
        EINA_LIST_FOREACH(splash_objs, l, o)
          {
             edje_object_signal_emit(o, "e,state,done", "e");
          }
     }
}

E_API void
e_init_undone(void)
{
   undone++;
}

E_API int
e_init_count_get(void)
{
   return undone;
}
