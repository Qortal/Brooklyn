#include <e.h>
#include <Eina.h>
#include "emix.h"
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "backend.h"

#define VOLUME_STEP 5

#define BARRIER_CHECK(old_val, new_val) \
   (old_val > EMIX_VOLUME_BARRIER - 20) && \
   (old_val <= EMIX_VOLUME_BARRIER) && \
   (new_val > EMIX_VOLUME_BARRIER) && \
   (new_val < EMIX_VOLUME_BARRIER + 20)

typedef struct _Client_Mixer Client_Mixer;
struct _Client_Mixer
{
   Evas_Object *win;
   Evas_Object *volume;
   Evas_Object *mute;
   E_Client *ec;
   Evas_Object *bx;
   Eina_List *sinks;
};


static void _notify_cb(void *data, unsigned int id);
static void _notify(const int val);
static void _volume_increase_cb(E_Object *obj, const char *params);
static void _volume_decrease_cb(E_Object *obj, const char *params);
static void _volume_mute_cb(E_Object *obj, const char *params);
static void _volume_increase_app_cb(E_Object *obj, const char *params);
static void _volume_decrease_app_cb(E_Object *obj, const char *params);
static void _volume_mute_app_cb(E_Object *obj, const char *params);
static void _actions_register(void);
static void _actions_unregister(void);
static void _sink_event(int type, void *info);
static void _disconnected(void);
static void _ready(void);
static void _sink_input_get(int *volume, Eina_Bool *muted, void *data);
static void _sink_input_set(int volume, Eina_Bool muted, void *data);
static int _sink_input_min_get(void *data);
static int _sink_input_max_get(void *data);
static const char *_sink_input_name_get(void *data);
static pid_t _get_ppid(pid_t pid);
static void _sink_input_event(int type, Emix_Sink_Input *input);
static void _events_cb(void *data, enum Emix_Event type, void *event_info);
static Eina_Bool _desklock_cb(void *data, int type, void *info);
static void _emix_backend_changed(const char *backend, void *data);
static void _bd_hook_volume_changed(void *data, Evas_Object *obj, void *event_info);
static void _bd_hook_volume_drag_stop(void *data, Evas_Object *obj, void *event_info);
static void _bd_hook_mute_changed(void *data, Evas_Object *obj, void *event_info);
static void _bd_hook_sink_volume_changed(void *data, Evas_Object *obj, void *event_info);
static void _bd_hook_sink_volume_drag_stop(void *data, Evas_Object *obj, void *event_info);
static void _bd_hook_sink_mute_changed(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _e_client_volume_changed(void *data, int type, void *event);
static Eina_Bool _e_client_mute_changed(void *data, int type, void *event);
static void _e_client_mixer_sink_append(E_Client_Volume_Sink *sink, Client_Mixer *cm);
static Eina_Bool _e_client_volume_sink_add(void *data, int type, void *event);
static Eina_Bool _e_client_volume_sink_del(void *data, int type, void *event);
static Eina_Bool _e_client_volume_sink_changed(void *data, int type, void *event);
static void _client_mixer_del(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool _e_client_add(void *data, int type, void *event);
static Eina_Bool _e_client_remove(void *data, int type, void *event);
static void _bd_hook_cb(void *data, E_Menu *m, E_Menu_Item *it);
static void _bd_hook(void *data, E_Client *ec);
static Eina_Bool _emixer_del_cb(void *data, int type, void *info);
static void _backend_changed(void);

static int _backend_log_domain = 0;
static int _volume_init = 0;
static Eina_Bool _backend_init_flag = EINA_FALSE;
static Eina_Bool _restored = EINA_FALSE;

EINTERN int E_EVENT_MIXER_BACKEND_CHANGED = 0;
EINTERN int E_EVENT_MIXER_SINKS_CHANGED = 0;

static const Emix_Sink *_sink_default = NULL;
static Eina_List *_client_sinks = NULL;
static Eina_List *_client_mixers = NULL;
static Eina_List *_client_handlers = NULL;
static E_Client_Menu_Hook *_border_hook = NULL;
static int _notification_id = 0;
static Ecore_Exe *_emixer_exe = NULL;
static Ecore_Event_Handler *_emix_exe_event_del_handler = NULL;

static E_Action *_action_incr = NULL;
static E_Action *_action_decr = NULL;
static E_Action *_action_mute = NULL;
static E_Action *_action_incr_app = NULL;
static E_Action *_action_decr_app = NULL;
static E_Action *_action_mute_app = NULL;

static void
_notify_cb(void *data EINA_UNUSED, unsigned int id)
{
   _notification_id = id;
}

static void
_notify(const int val)
{
   E_Notification_Notify n;
   char *icon, buf[56];
   int ret;

   if (!emix_config_notify_get()) return;

   memset(&n, 0, sizeof(E_Notification_Notify));
   if (val < 0) return;

   ret = snprintf(buf, (sizeof(buf) - 1), "%s: %d%%", _("New volume"), val);
   if ((ret < 0) || ((unsigned int)ret > sizeof(buf)))
     return;
   //Names are taken from FDO icon naming scheme
   if (val == 0)
     icon = "audio-volume-muted";
   else if ((val > 33) && (val < 66))
     icon = "audio-volume-medium";
   else if (val <= 33)
     icon = "audio-volume-low";
   else
     icon = "audio-volume-high";

   n.app_name = _("Mixer");
   n.replaces_id = _notification_id;
   n.icon.icon = icon;
   n.summary = _("Volume changed");
   n.body = buf;
   n.timeout = 2000;
   e_notification_client_send(&n, _notify_cb, NULL);
}

static void
_volume_increase_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   unsigned int i;
   Emix_Volume volume;

   EINA_SAFETY_ON_NULL_RETURN(_sink_default);
   Emix_Sink *s = (Emix_Sink *)_sink_default;

   if (!s->volume.channel_count) return;

   if (BARRIER_CHECK(s->volume.volumes[0], s->volume.volumes[0] + VOLUME_STEP))
     return;

   volume.channel_count = s->volume.channel_count;
   volume.volumes = calloc(s->volume.channel_count, sizeof(int));
   for (i = 0; i < volume.channel_count; i++)
     {
        if (s->volume.volumes[i] < (emix_max_volume_get()) - VOLUME_STEP)
          volume.volumes[i] = s->volume.volumes[i] + VOLUME_STEP;
        else if (s->volume.volumes[i] < emix_max_volume_get())
          volume.volumes[i] = emix_max_volume_get();
        else
          volume.volumes[i] = s->volume.volumes[i];
     }

   emix_sink_volume_set(s, &volume);
   emix_config_save_state_get();
   if (emix_config_save_get()) e_config_save_queue();
   free(volume.volumes);
}

static void
_volume_decrease_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   unsigned int i;
   Emix_Volume volume;

   EINA_SAFETY_ON_NULL_RETURN(_sink_default);
   Emix_Sink *s = (Emix_Sink *)_sink_default;
   volume.channel_count = s->volume.channel_count;
   volume.volumes = calloc(s->volume.channel_count, sizeof(int));
   for (i = 0; i < volume.channel_count; i++)
     {
        if (s->volume.volumes[i] > VOLUME_STEP)
          volume.volumes[i] = s->volume.volumes[i] - VOLUME_STEP;
        else if (s->volume.volumes[i] < VOLUME_STEP)
          volume.volumes[i] = 0;
        else
          volume.volumes[i] = s->volume.volumes[i];
     }

   emix_sink_volume_set((Emix_Sink *)_sink_default, &volume);
   emix_config_save_state_get();
   if (emix_config_save_get()) e_config_save_queue();
   free(volume.volumes);
}

static void
_volume_mute_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   EINA_SAFETY_ON_NULL_RETURN(_sink_default);
   backend_mute_set(!_sink_default->mute);
}

static void
_volume_increase_app_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Client *ec;

   ec = e_client_focused_get();
   if (ec && ec->volume_control_enabled)
     {
        e_client_volume_set(ec, ec->volume + VOLUME_STEP);
     }
}

static void
_volume_decrease_app_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Client *ec;

   ec = e_client_focused_get();
   if (ec && ec->volume_control_enabled)
     {
        e_client_volume_set(ec, ec->volume - VOLUME_STEP);
     }
}

static void
_volume_mute_app_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Client *ec;

   ec = e_client_focused_get();
   if (ec && ec->volume_control_enabled)
     {
        e_client_volume_mute_set(ec, !ec->mute);
     }
}

static void
_actions_register(void)
{
   _action_incr = e_action_add("volume_increase");
   if (_action_incr)
     {
        _action_incr->func.go = _volume_increase_cb;
        e_action_predef_name_set("Mixer", _("Increase Volume"),
                                 "volume_increase", NULL, NULL, 0);
     }

   _action_decr = e_action_add("volume_decrease");
   if (_action_decr)
     {
        _action_decr->func.go = _volume_decrease_cb;
        e_action_predef_name_set("Mixer", _("Decrease Volume"),
                                 "volume_decrease", NULL, NULL, 0);
     }

   _action_mute = e_action_add("volume_mute");
   if (_action_mute)
     {
        _action_mute->func.go = _volume_mute_cb;
        e_action_predef_name_set("Mixer", _("Mute volume"), "volume_mute",
                                 NULL, NULL, 0);
     }
   _action_incr_app = e_action_add("volume_increase_app");
   if (_action_incr_app)
     {
        _action_incr_app->func.go = _volume_increase_app_cb;
        e_action_predef_name_set("Mixer",
                                 _("Increase Volume of Focused Application"),
                                 "volume_increase_app", NULL, NULL, 0);
     }
   _action_decr_app = e_action_add("volume_decrease_app");
   if (_action_decr_app)
     {
        _action_decr_app->func.go = _volume_decrease_app_cb;
        e_action_predef_name_set("Mixer",
                                 _("Decrease Volume of Focused Application"),
                                 "volume_decrease_app", NULL, NULL, 0);
     }
   _action_mute_app = e_action_add("volume_mute_app");
   if (_action_mute_app)
     {
        _action_mute_app->func.go = _volume_mute_app_cb;
        e_action_predef_name_set("Mixer",
                                 _("Mute Volume of Focused Application"),
                                 "volume_mute_app", NULL, NULL, 0);
     }

   e_comp_canvas_keys_ungrab();
   e_comp_canvas_keys_grab();
}

static void
_actions_unregister(void)
{
   if (_action_incr)
     {
        e_action_predef_name_del("Mixer", _("Increase Volume"));
        e_action_del("volume_increase");
        _action_incr = NULL;
     }

   if (_action_decr)
     {
        e_action_predef_name_del("Mixer", _("Decrease Volume"));
        e_action_del("volume_decrease");
        _action_decr = NULL;
     }

   if (_action_mute)
     {
        e_action_predef_name_del("Mixer", _("Mute Volume"));
        e_action_del("volume_mute");
        _action_mute = NULL;
     }

   if (_action_incr_app)
     {
        e_action_predef_name_del("Mixer",
                                 _("Increase Volume of Focused Application"));
        e_action_del("volume_increase_app");
        _action_incr_app = NULL;
     }

   if (_action_decr_app)
     {
        e_action_predef_name_del("Mixer",
                                 _("Decrease Volume of Focused Application"));
        e_action_del("volume_decrease_app");
        _action_decr_app = NULL;
     }

   if (_action_mute_app)
     {
        e_action_predef_name_del("Mixer",
                                 _("Mute Volume of Focused Application"));
        e_action_del("volume_mute_app");
        _action_mute_app = NULL;
     }

   e_comp_canvas_keys_ungrab();
   e_comp_canvas_keys_grab();
}

static void
_sink_event(int type, void *info)
{
   Emix_Sink *sink = info;
   const Eina_List *l;

   if (type == EMIX_SINK_REMOVED_EVENT)
     {
        if (sink == _sink_default)
          {
             l = emix_sinks_get();
             if (l) _sink_default = l->data;
             else _sink_default = NULL;
             if (emix_config_save_get()) e_config_save_queue();
             _backend_changed();
          }
     }
   else if (type == EMIX_SINK_CHANGED_EVENT)
     {
        /* If pulseaudio changed the default sink, swap the UI to display it
           instead of previously selected sink */
        if (sink->default_sink)
          _sink_default = sink;
        if (_sink_default == sink)
          {
             static int prev_vol = -1;
             int vol;

             _backend_changed();
             if (sink->mute || !sink->volume.channel_count) vol = 0;
             else vol = sink->volume.volumes[0];
             if (vol != prev_vol)
               {
                  _notify(vol);
                  prev_vol = vol;
               }
          }
     }
   else
     {
        DBG("Sink added");
     }
   /*
     Only safe the state if we are not in init mode,
     If we are in init mode, this is a result of the restore call.
     Restore iterates over a list of sinks which would get deleted in the
     save_state_get call.
    */
   if (!_backend_init_flag)
     {
        emix_config_save_state_get();
        if (emix_config_save_get()) e_config_save_queue();
        ecore_event_add(E_EVENT_MIXER_SINKS_CHANGED, NULL, NULL, NULL);
     }
}

static void
_disconnected(void)
{
   E_Client_Volume_Sink *sink;

   EINA_LIST_FREE(_client_sinks, sink)
     {
        e_client_volume_sink_del(sink);
     }
   _sink_default = NULL;
   _restored = EINA_FALSE;
   _backend_changed();
}


static void
_ready(void)
{
   _backend_init_flag = EINA_TRUE;
   if (emix_sink_default_support())
     _sink_default = emix_sink_default_get();
   else
     {
        if (emix_sinks_get())
          _sink_default = emix_sinks_get()->data;
     }

   if (emix_config_save_get() && !_restored)
     {
        Emix_Sink *s;
        const char *sinkname;

        sinkname = emix_config_save_sink_get();
        if (sinkname)
          {
             Eina_List *sinks = (Eina_List *)emix_sinks_get();
             Eina_List *l;

             EINA_LIST_FOREACH(sinks, l, s)
               {
                  if ((s->name) && (!strcmp(s->name, sinkname)))
                    {
                       _sink_default = s;
                       break;
                    }
               }
          }
        emix_config_save_state_restore();
        _restored = EINA_TRUE;
     }

   _backend_changed();
   _backend_init_flag = EINA_FALSE;
}

static void
_sink_input_get(int *volume, Eina_Bool *muted, void *data)
{
   Emix_Sink_Input *input;

   input = data;

   if (input->volume.channel_count > 0)
     {
        if (volume) *volume = input->volume.volumes[0];
     }
   if (muted) *muted = input->mute;
}

static void
_sink_input_set(int volume, Eina_Bool muted, void *data)
{
   Emix_Sink_Input *input;

   input = data;

   VOLSET(volume, input->volume, input, emix_sink_input_volume_set);
   emix_sink_input_mute_set(input, muted);
}

static int
_sink_input_min_get(void *data EINA_UNUSED)
{
   return 0;
}

static int
_sink_input_max_get(void *data EINA_UNUSED)
{
   return emix_max_volume_get();
}

static const char *
_sink_input_name_get(void *data)
{
   Emix_Sink_Input *input;

   input = data;
   return input->name;
}

static pid_t
_get_ppid(pid_t pid)
{
   int fd;
   char buf[128];
   char *s;
   pid_t ppid;

   /* Open the status info process file provided by kernel to get the parent
    * process id. 'man 5 proc' and go to /proc/[pid]/stat to get information
    * about the content of this file.
    */
   snprintf(buf, sizeof(buf), "/proc/%d/stat", pid);
   fd = open(buf, O_RDONLY);
   if (fd == -1)
     {
        ERR("Can't open %s, maybee the process exited.", buf);
        return -1;
     }
   if ((read(fd, buf, sizeof(buf))) < 4)
     {
        close(fd);
        return -1;
     }
   buf[sizeof(buf) - 1] = 0;
   s = strrchr(buf, ')');
   s += 3;
   ppid = atoi(s);
   close(fd);
   return ppid;
}

static void
_sink_input_event(int type, Emix_Sink_Input *input)
{
   Eina_List *clients, *l, *ll;
   E_Client *ec;
   E_Client_Volume_Sink *sink;
   pid_t pid;
   Eina_Bool found = EINA_FALSE;

   switch (type)
     {
      case EMIX_SINK_INPUT_ADDED_EVENT:
         pid = input->pid;
         for (;;)
           {
              if ((pid <= 1) || (pid == getpid())) return;
              clients = e_client_focus_stack_get();
              EINA_LIST_FOREACH(clients, l, ec)
                {
                   if ((ec->netwm.pid == pid) && (!ec->parent))
                     {
                        DBG("Sink found the client %s",
                            e_client_util_name_get(ec));
                        sink = e_client_volume_sink_new(_sink_input_get,
                                                        _sink_input_set,
                                                        _sink_input_min_get,
                                                        _sink_input_max_get,
                                                        _sink_input_name_get,
                                                        input);
                        e_client_volume_sink_append(ec, sink);
                        _client_sinks = eina_list_append(_client_sinks, sink);
                        found = EINA_TRUE;
                     }
                }
              if (found) break;
              pid = _get_ppid(pid);
           }
         break;
      case EMIX_SINK_INPUT_REMOVED_EVENT:
         EINA_LIST_FOREACH_SAFE(_client_sinks, l, ll, sink)
           {
              if (sink->data == input)
                {
                   e_client_volume_sink_del(sink);
                   _client_sinks = eina_list_remove_list(_client_sinks, l);
                }
           }
         break;
      case EMIX_SINK_INPUT_CHANGED_EVENT:
         EINA_LIST_FOREACH(_client_sinks, l, sink)
           {
              if (sink->data == input)
                {
                   e_client_volume_sink_update(sink);
                }
           }
         break;
     }
}

static void
_events_cb(void *data EINA_UNUSED, enum Emix_Event type, void *event_info)
{
   switch (type)
     {
      case EMIX_SINK_ADDED_EVENT:
      case EMIX_SINK_CHANGED_EVENT:
      case EMIX_SINK_REMOVED_EVENT:
         _sink_event(type, event_info);
         break;
      case EMIX_DISCONNECTED_EVENT:
         _disconnected();
         break;
      case EMIX_READY_EVENT:
         _ready();
         break;
      case EMIX_SINK_INPUT_ADDED_EVENT:
      case EMIX_SINK_INPUT_REMOVED_EVENT:
      case EMIX_SINK_INPUT_CHANGED_EVENT:
         _sink_input_event(type, event_info);
         break;

      default:
         break;
     }
}

static Eina_Bool
_desklock_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *info)
{
   E_Event_Desklock *ev = info;
   static Eina_Bool _was_mute = EINA_FALSE;

   if (emix_config_desklock_mute_get() == EINA_FALSE)
     return ECORE_CALLBACK_PASS_ON;

   if (ev->on)
     {
        if (_sink_default)
          {
             _was_mute = _sink_default->mute;
             if (!_was_mute)
               emix_sink_mute_set((Emix_Sink *)_sink_default, EINA_TRUE);
          }
     }
   else
     {
        if (_sink_default)
          {
             if (!_was_mute)
               emix_sink_mute_set((Emix_Sink *)_sink_default, EINA_FALSE);
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_emix_backend_changed(const char *backend, void *data EINA_UNUSED)
{
   _disconnected();

   if (emix_backend_set(backend) == EINA_FALSE)
     ERR("Could not load backend: %s", backend);
}

static void
_bd_hook_volume_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Client *ec;

   ec = data;

   e_client_volume_set(ec, elm_slider_value_get(obj));
}

static void
_bd_hook_volume_drag_stop(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *ec;

   ec = data;

   elm_slider_value_set(obj, ec->volume);
}


static void
_bd_hook_mute_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Client *ec;

   ec = data;

   e_client_volume_mute_set(ec, elm_check_state_get(obj));
}

static void
_bd_hook_sink_volume_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Client_Volume_Sink *sink;
   Evas_Object *check;

   sink = data;

   check = evas_object_data_get(obj, "e_sink_check");

   e_client_volume_sink_set(sink,
                            elm_slider_value_get(obj),
                            elm_check_state_get(check));
}

static void
_bd_hook_sink_volume_drag_stop(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Client_Volume_Sink *sink;
   Evas_Object *check;
   Eina_Bool mute;
   int vol;

   sink = data;

   check = evas_object_data_get(obj, "e_sink_check");

   e_client_volume_sink_get(sink, &vol, &mute);
   elm_slider_value_set(obj, vol);
   elm_check_state_set(check, mute);
}


static void
_bd_hook_sink_mute_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Client_Volume_Sink *sink;
   Evas_Object *slider;

   sink = data;
   slider = evas_object_data_get(obj, "e_sink_volume");

   e_client_volume_sink_set(sink,
                            elm_slider_value_get(slider),
                            elm_check_state_get(obj));
}

static Eina_Bool
_e_client_volume_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   Client_Mixer *cm;
   E_Client_Volume_Sink *sink;
   Evas_Object *o;
   Eina_List *l;

   ev = event;

   EINA_LIST_FOREACH(_client_mixers, l, cm)
     {
        if (cm->ec == ev->ec)
          {
             elm_slider_value_set(cm->volume, cm->ec->volume);
             EINA_LIST_FOREACH(cm->sinks, l, o)
               {
                  int volume;
                  sink = evas_object_data_get(o, "e_sink");
                  e_client_volume_sink_get(sink, &volume, NULL);
                  elm_slider_value_set(o, volume);
               }
             break;
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_client_mute_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   Client_Mixer *cm;
   E_Client_Volume_Sink *sink;
   Evas_Object *o, *check;
   Eina_List *l;
   Eina_Bool mute;

   ev = event;

   EINA_LIST_FOREACH(_client_mixers, l, cm)
     {
        if (cm->ec == ev->ec)
          {
             elm_check_state_set(cm->mute, !!cm->ec->mute);
             elm_object_disabled_set(cm->volume, !!cm->ec->mute);
             EINA_LIST_FOREACH(cm->sinks, l, o)
               {
                  sink = evas_object_data_get(o, "e_sink");
                  check = evas_object_data_get(o, "e_sink_check");
                  e_client_volume_sink_get(sink, NULL, &mute);
                  elm_check_state_set(check, mute);
                  elm_object_disabled_set(o, mute);
               }
             break;
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_e_client_mixer_sink_append(E_Client_Volume_Sink *sink, Client_Mixer *cm)
{
   Evas_Object *lbl, *slider, *check, *sep;
   int volume;
   int min, max;
   Eina_Bool mute;

   min = e_client_volume_sink_min_get(sink);
   max = e_client_volume_sink_max_get(sink);
   e_client_volume_sink_get(sink, &volume, &mute);

   sep = elm_separator_add(cm->bx);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_size_hint_weight_set(sep, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sep, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(cm->bx, sep);
   evas_object_show(sep);

   lbl = elm_label_add(cm->bx);
   elm_object_text_set(lbl, e_client_volume_sink_name_get(sink));
   evas_object_size_hint_align_set(lbl, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(cm->bx, lbl);
   evas_object_show(lbl);

   slider = elm_slider_add(cm->bx);
   elm_slider_horizontal_set(slider, EINA_TRUE);
   elm_slider_min_max_set(slider, min, max);
   elm_slider_span_size_set(slider, max * elm_config_scale_get());
   elm_slider_unit_format_set(slider, "%.0f");
   elm_slider_indicator_format_set(slider, "%.0f");
   evas_object_size_hint_weight_set(slider, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_slider_value_set(slider, volume);
   evas_object_smart_callback_add(slider, "changed",
                                  _bd_hook_sink_volume_changed, sink);
   evas_object_smart_callback_add(slider, "slider,drag,stop",
                                  _bd_hook_sink_volume_drag_stop, sink);
   elm_box_pack_end(cm->bx, slider);
   evas_object_show(slider);

   check = elm_check_add(cm->bx);
   elm_object_text_set(check, _("Mute"));
   evas_object_size_hint_align_set(check, 0.0, EVAS_HINT_FILL);
   elm_check_state_set(check, !!mute);
   elm_object_disabled_set(slider, !!mute);
   evas_object_smart_callback_add(check, "changed",
                                  _bd_hook_sink_mute_changed, sink);

   elm_box_pack_end(cm->bx, check);
   evas_object_show(check);

   evas_object_data_set(slider, "e_sink", sink);
   evas_object_data_set(slider, "e_sink_check", check);
   evas_object_data_set(slider, "e_sink_label", lbl);
   evas_object_data_set(slider, "e_sink_separator", sep);
   evas_object_data_set(check, "e_sink_volume", slider);
   cm->sinks = eina_list_append(cm->sinks, slider);
}

static Eina_Bool
_e_client_volume_sink_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client_Volume_Sink *ev;
   Client_Mixer *cm;
   Eina_List *l;

   ev = event;

   EINA_LIST_FOREACH(_client_mixers, l, cm)
     {
        if (cm->ec == ev->ec)
          {
             _e_client_mixer_sink_append(ev->sink, cm);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_client_volume_sink_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client_Volume_Sink *ev;
   E_Client_Volume_Sink *sink;
   Client_Mixer *cm;
   Evas_Object *o, *lbl, *check, *sep;
   Eina_List *l;

   ev = event;

   EINA_LIST_FOREACH(_client_mixers, l, cm)
     {
        if (cm->ec == ev->ec)
          {
             EINA_LIST_FOREACH(cm->sinks, l, o)
               {
                  sink = evas_object_data_get(o, "e_sink");
                  if (sink == ev->sink)
                    {
                       lbl = evas_object_data_get(o, "e_sink_label");
                       check = evas_object_data_get(o, "e_sink_check");
                       sep = evas_object_data_get(o, "e_sink_separator");
                       evas_object_del(sep);
                       evas_object_del(lbl);
                       evas_object_del(o);
                       evas_object_del(check);
                       cm->sinks = eina_list_remove_list(cm->sinks, l);
                    }
               }
             break;
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_client_volume_sink_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client_Volume_Sink *ev;
   Client_Mixer *cm;
   E_Client_Volume_Sink *sink;
   Evas_Object *o, *check;
   Eina_List *l;
   int volume;
   Eina_Bool mute;

   ev = event;

   EINA_LIST_FOREACH(_client_mixers, l, cm)
     {
        if (cm->ec == ev->ec)
          {
             EINA_LIST_FOREACH(cm->sinks, l, o)
               {
                  sink = evas_object_data_get(o, "e_sink");
                  if (sink != ev->sink) continue;
                  check = evas_object_data_get(o, "e_sink_check");
                  e_client_volume_sink_get(sink, &volume, &mute);
                  elm_slider_value_set(o, volume);
                  elm_object_disabled_set(o, mute);
                  elm_check_state_set(check, mute);
               }
             break;
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_client_mixer_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Client_Mixer *cm;

   cm = data;

   _client_mixers = eina_list_remove(_client_mixers, cm);
   free(cm);
}

static Eina_Bool
_e_client_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   Eina_List *l;
   Emix_Sink_Input *input;
   pid_t pid;
   E_Client_Volume_Sink *sink;

   ev = event;

   if (ev->ec->parent) return ECORE_CALLBACK_PASS_ON;
   EINA_LIST_FOREACH((Eina_List *)emix_sink_inputs_get(), l, input)
     {
        pid = input->pid;
        while (42)
          {
             if (pid <= 1 || pid == getpid()) return ECORE_CALLBACK_PASS_ON;
             if (ev->ec->netwm.pid == pid)
               {
                  DBG("Client(%s) found a sink input",
                      e_client_util_name_get(ev->ec));
                  sink = e_client_volume_sink_new(_sink_input_get,
                                                  _sink_input_set,
                                                  _sink_input_min_get,
                                                  _sink_input_max_get,
                                                  _sink_input_name_get,
                                                  input);
                  e_client_volume_sink_append(ev->ec, sink);
                  _client_sinks = eina_list_append(_client_sinks, sink);
                  return ECORE_CALLBACK_PASS_ON;
               }
             pid = _get_ppid(pid);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_client_remove(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   Client_Mixer *cm;
   Eina_List *l;

   ev = event;

   EINA_LIST_FOREACH(_client_mixers, l, cm)
     {
        if (cm->ec == ev->ec)
          {
             evas_object_event_callback_del_full(cm->win, EVAS_CALLBACK_DEL,
                                                 _client_mixer_del, cm);
             evas_object_del(cm->win);
             _client_mixers = eina_list_remove_list(_client_mixers, l);
             free(cm);
             break;
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_bd_hook_cb(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *it EINA_UNUSED)
{
   E_Client *ec;
   Client_Mixer *cm;
   E_Client_Volume_Sink *sink;
   Evas_Object *win, *popup, *bx, *o;
   Eina_List *l;
   int w, h;

   ec = data;

   EINA_LIST_FOREACH(_client_mixers, l, cm)
     {
        if (cm->ec == ec)
          return;
     }
   cm = E_NEW(Client_Mixer, 1);
   cm->ec = ec;

   win = elm_popup_add(e_comp->elm);
   elm_popup_allow_events_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_FALSE);
   elm_object_content_set(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   cm->bx = bx;

   o = elm_label_add(bx);
   elm_object_text_set(o, _("Main"));
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_slider_add(bx);
   elm_slider_horizontal_set(o, EINA_TRUE);
   elm_slider_min_max_set(o, ec->volume_min, ec->volume_max);
   elm_slider_span_size_set(o, ec->volume_max * elm_config_scale_get());
   elm_slider_unit_format_set(o, "%.0f");
   elm_slider_indicator_format_set(o, "%.0f");
   evas_object_size_hint_weight_set(o, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_slider_value_set(o, ec->volume);
   evas_object_smart_callback_add(o, "changed", _bd_hook_volume_changed, ec);
   evas_object_smart_callback_add(o, "slider,drag,stop",
                                  _bd_hook_volume_drag_stop, ec);
   elm_box_pack_end(bx, o);
   evas_object_show(o);
   cm->volume = o;

   o = elm_check_add(bx);
   elm_object_text_set(o, _("Mute"));
   evas_object_size_hint_align_set(o, 0.0, EVAS_HINT_FILL);
   elm_check_state_set(o, !!ec->mute);
   elm_object_disabled_set(o, !!ec->mute);
   evas_object_smart_callback_add(o, "changed", _bd_hook_mute_changed, ec);
   elm_box_pack_end(bx, o);
   evas_object_show(o);
   cm->mute = o;

   EINA_LIST_FOREACH(ec->sinks, l, sink)
     {
        _e_client_mixer_sink_append(sink, cm);
     }

   evas_object_show(bx);
   evas_object_size_hint_min_get(bx, &w, &h);
   evas_object_resize(win, w, h);
   evas_object_show(win);
   popup = e_comp_object_util_add(win, E_COMP_OBJECT_TYPE_NONE);
   evas_object_layer_set(popup, E_LAYER_POPUP);
   e_comp_object_util_center_on_zone(popup, ec->zone);
   evas_object_show(popup);
   e_comp_object_util_autoclose(popup, NULL, NULL, NULL);
   _client_mixers = eina_list_append(_client_mixers, cm);
   evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _client_mixer_del, cm);
   cm->win = popup;
}

static void
_bd_hook(void *data EINA_UNUSED, E_Client *ec)
{
   E_Menu_Item *it;
   E_Menu *m;
   Eina_List *l;

   if (!ec->volume_control_enabled) return;
   m = ec->border_menu;

   /* Find the first separator and add us below him */
   EINA_LIST_FOREACH(m->items, l, it)
     {
        if (it->separator) break;
     }
   if ((!it) || (!it->separator)) return;

   it = e_menu_item_new_relative(m, it);
   e_menu_item_label_set(it, _("Volume"));
   e_util_menu_item_theme_icon_set(it, "preferences-desktop-mixer");
   e_menu_item_callback_set(it, _bd_hook_cb, ec);

   it = e_menu_item_new_relative(m, it);
   e_menu_item_separator_set(it, EINA_TRUE);
}

static Eina_Bool
_emixer_del_cb(void *data EINA_UNUSED, int type EINA_UNUSED,  void *info EINA_UNUSED)
{
   _emixer_exe = NULL;
   if (_emix_exe_event_del_handler)
     {
        ecore_event_handler_del(_emix_exe_event_del_handler);
        _emix_exe_event_del_handler = NULL;
     }
   return EINA_TRUE;
}

static void
_backend_changed(void)
{
   ecore_event_add(E_EVENT_MIXER_BACKEND_CHANGED, NULL, NULL, NULL);
}


EINTERN int
backend_init(void)
{
   Eina_List *l;
   const char *backend;
   Eina_Bool backend_loaded = EINA_FALSE;

   if (_volume_init++) return _volume_init;
   _backend_log_domain = eina_log_domain_register("mixer_backend", EINA_COLOR_RED);

   DBG("Init mixer backend");
   EINA_SAFETY_ON_FALSE_RETURN_VAL(emix_init(), --_volume_init);
   DBG("Init mixer backend config");
   emix_config_init(_emix_backend_changed, NULL);
   DBG("Init mixer backend callback");
   emix_event_callback_add(_events_cb, NULL);

   backend = emix_config_backend_get();
   if (backend && emix_backend_set(backend))
      backend_loaded = EINA_TRUE;
   else
     {
        if (backend)
           WRN("Could not load %s, trying another one ...", backend);
        EINA_LIST_FOREACH((Eina_List *)emix_backends_available(), l,
                          backend)
          {
             if (emix_backend_set(backend) == EINA_TRUE)
               {
                  DBG("Loaded backend: %s!", backend);
                  backend_loaded = EINA_TRUE;
                  emix_config_backend_set(backend);
                  break;
               }
          }
     }

   if (!backend_loaded) goto err;

   if (emix_sink_default_support())
     _sink_default = emix_sink_default_get();

   _actions_register();

   _border_hook = e_int_client_menu_hook_add(_bd_hook, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_DESKLOCK,
                         _desklock_cb, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_VOLUME,
                         _e_client_volume_changed, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_MUTE,
                         _e_client_mute_changed, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_UNMUTE,
                         _e_client_mute_changed, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_ADD,
                         _e_client_add, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_REMOVE,
                         _e_client_remove, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_VOLUME_SINK_ADD,
                         _e_client_volume_sink_add, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_VOLUME_SINK_DEL,
                         _e_client_volume_sink_del, NULL);
   E_LIST_HANDLER_APPEND(_client_handlers, E_EVENT_CLIENT_VOLUME_SINK_CHANGED,
                         _e_client_volume_sink_changed, NULL);

   E_EVENT_MIXER_BACKEND_CHANGED = ecore_event_type_new();
   E_EVENT_MIXER_SINKS_CHANGED = ecore_event_type_new();

   return _volume_init;
err:
   emix_config_shutdown();
   emix_shutdown();
   return --_volume_init;
}

EINTERN int
backend_shutdown(void)
{
   E_Client_Volume_Sink *sink;
   Client_Mixer *cm;

   if (!--_volume_init) return _volume_init;

   E_FREE_LIST(_client_handlers, ecore_event_handler_del);
   EINA_LIST_FREE(_client_mixers, cm)
     {
        evas_object_event_callback_del_full(cm->win, EVAS_CALLBACK_DEL,
                                            _client_mixer_del, cm);
        evas_object_del(cm->win);
        free(cm);
     }

   e_int_client_menu_hook_del(_border_hook);
   _actions_unregister();

   EINA_LIST_FREE(_client_sinks, sink)
      e_client_volume_sink_del(sink);

   if (_emix_exe_event_del_handler)
     {
        ecore_event_handler_del(_emix_exe_event_del_handler);
        _emix_exe_event_del_handler = NULL;
     }


   emix_event_callback_del(_events_cb, NULL);
   emix_shutdown();
   emix_config_shutdown();

   eina_log_domain_unregister(_backend_log_domain);
   return _volume_init;
}

EINTERN void
backend_emixer_exec(void)
{
   if (_emixer_exe) return;

   _emixer_exe = e_util_exe_safe_run("emixer", NULL);
   if (_emix_exe_event_del_handler)
      ecore_event_handler_del(_emix_exe_event_del_handler);
   _emix_exe_event_del_handler =
      ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _emixer_del_cb, NULL);
}

EINTERN void
backend_volume_set(unsigned int volume)
{
   EINA_SAFETY_ON_NULL_RETURN(_sink_default);
   DBG("Sink default mute set %d", volume);

   VOLSET(volume, ((Emix_Sink *)_sink_default)->volume,
          (Emix_Sink *)_sink_default, emix_sink_volume_set);
   emix_config_save_state_get();
   if (emix_config_save_get()) e_config_save_queue();
}

EINTERN unsigned int
backend_volume_get(void)
{
   unsigned int volume = 0, i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_sink_default, 0);
   for (i = 0; i < _sink_default->volume.channel_count; i++)
     volume += _sink_default->volume.volumes[i];
   if (_sink_default->volume.channel_count)
     volume = volume / _sink_default->volume.channel_count;

   DBG("Sink default volume get %d", volume);
   return volume;
}

EINTERN void
backend_volume_decrease(void)
{
   _volume_decrease_cb(NULL, NULL);
}

EINTERN void
backend_volume_increase(void)
{
   _volume_increase_cb(NULL, NULL);
}

EINTERN void
backend_mute_set(Eina_Bool mute)
{
   EINA_SAFETY_ON_NULL_RETURN(_sink_default);

   DBG("Sink default mute set %d", mute);
   emix_sink_mute_set((Emix_Sink *)_sink_default, mute);
   emix_config_save_state_get();
   if (emix_config_save_get()) e_config_save_queue();
}

EINTERN Eina_Bool
backend_mute_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_sink_default, EINA_FALSE);
   DBG("Mute get %d", _sink_default->mute);
   return _sink_default->mute;
}

EINTERN void
backend_sink_default_set(const Emix_Sink *s)
{
   if (_sink_default != s)
     {
        _sink_default = s;
        if (s)
          {
             DBG("Sink default set %s", s->name);
             emix_sink_default_set((Emix_Sink *)s);
             emix_config_save_sink_set(s->name);
          }
        _backend_changed();
     }
}

EINTERN const Emix_Sink *
backend_sink_default_get(void)
{
   return _sink_default;
}

