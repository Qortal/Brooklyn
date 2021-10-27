#include "e.h"

/* TODO:
 * - Need some kind of "wait for exit" system, maybe register with
 *   e_config? startup and restart apps could also be in e_config
 */

/* local subsystem functions */
static void      _e_startup(void);
static void      _e_startup_next_cb(void *data);
static Eina_Bool _e_startup_event_cb(void *data, int ev_type, void *ev);
static Eina_Bool _e_startup_time_exceeded(void *data);

/* local subsystem globals */
static E_Order *startup_apps = NULL;
static int start_app_pos = -1;
static Ecore_Event_Handler *desktop_cache_update_handler = NULL;
static Ecore_Timer *timer = NULL;
static Eina_Bool desktop_cache_update = EINA_FALSE;
static Eina_Bool started = EINA_FALSE;
static E_Startup_Mode startup_mode = 0;

/* externally accessible functions */


E_API void
e_startup_mode_set(E_Startup_Mode mode)
{
   startup_mode = mode;
   desktop_cache_update_handler =
     ecore_event_handler_add(EFREET_EVENT_DESKTOP_CACHE_BUILD,
                             _e_startup_event_cb, NULL);
   if (timer) ecore_timer_del(timer);
   timer = ecore_timer_add(20.0, _e_startup_time_exceeded, NULL);
   e_init_undone();
}

E_API void
e_startup(void)
{
   if (!desktop_cache_update)
     started = EINA_TRUE;
   else
     _e_startup();
}

/* local subsystem functions */
static Eina_Bool
_e_startup_delay(void *data)
{
   Efreet_Desktop *desktop = data;
   e_exec(NULL, desktop, NULL, NULL, NULL);
   efreet_desktop_unref(desktop);
   return EINA_FALSE;
}

static void
_e_startup(void)
{
   Efreet_Desktop *desktop;
   const char *s;
   double delay = 0.0;

   if (!startup_apps)
     {
        e_init_done();
        return;
     }
   desktop = eina_list_nth(startup_apps->desktops, start_app_pos);
   start_app_pos++;
   if (!desktop)
     {
        e_object_del(E_OBJECT(startup_apps));
        startup_apps = NULL;
        start_app_pos = -1;
        e_init_done();
        return;
     }
   if (desktop->x)
     {
        s = eina_hash_find(desktop->x, "X-GNOME-Autostart-Delay");
        if (s) delay = eina_convert_strtod_c(s, NULL);
     }
   if (delay > 0.0)
     {
        efreet_desktop_ref(desktop);
        ecore_timer_add(delay, _e_startup_delay, desktop);
     }
   else e_exec(NULL, desktop, NULL, NULL, NULL);
   ecore_job_add(_e_startup_next_cb, NULL);
}

static void
_e_startup_next_cb(void *data EINA_UNUSED)
{
   _e_startup();
}

static void
_e_startup_error_dialog(const char *msg)
{
   E_Dialog *dia;

   dia = e_dialog_new(NULL, "E", "_startup_error_dialog");
   EINA_SAFETY_ON_NULL_RETURN(dia);

   e_dialog_title_set(dia, "ERROR!");
   e_dialog_icon_set(dia, "enlightenment", 64);
   e_dialog_text_set(dia, msg);
   e_dialog_button_add(dia, _("Close"), NULL, NULL, NULL);
   elm_win_center(dia->win, 1, 1);
   e_win_no_remember_set(dia->win, 1);
   e_dialog_show(dia);
}

static E_Order *
_e_startup_load(const char *file)
{
   E_Order *o = e_order_new(file);
   if (!o) return NULL;
   if (!o->desktops)
     {
        e_object_del(E_OBJECT(o));
        o = NULL;
     }
   return o;
}

static Eina_Bool
_e_startup_event_cb(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   char buf[PATH_MAX];
   Efreet_Event_Cache_Update *e;

   if (timer) ecore_timer_del(timer);
   timer = NULL;

   e = ev;
   if ((e) && (e->error))
     {
        fprintf(stderr, "E: efreet couldn't build cache\n");
        _e_startup_error_dialog("E: Efreetd cannot be connected to.<br>"
                                "Please check:<br>"
                                "$XDG_RUTIME_DIR/.ecore/efreetd/0<br>"
                                "or<br>"
                                "~/.ecore/efreetd/0<br>"
                                "Is your system very slow on start so<br>"
                                "efreetd cannot be connected to within<br>"
                                "0.5sec after launch??");
     }
   desktop_cache_update = EINA_TRUE;
   E_FREE_FUNC(desktop_cache_update_handler, ecore_event_handler_del);

   if (startup_mode == E_STARTUP_START)
     {
        e_user_dir_concat_static(buf, "applications/startup/.order");
        startup_apps = _e_startup_load(buf);
        if (!startup_apps)
          {
             e_prefix_data_concat_static(buf, "data/applications/startup/.order");
             startup_apps = e_order_new(buf);
          }
     }
   else if (startup_mode == E_STARTUP_RESTART)
     {
        e_user_dir_concat_static(buf, "applications/restart/.order");
        startup_apps = _e_startup_load(buf);
        if (!startup_apps)
          {
             e_prefix_data_concat_static(buf, "data/applications/restart/.order");
             startup_apps = e_order_new(buf);
          }
     }
   if (startup_apps) start_app_pos = 0;
   if (started) _e_startup();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_startup_time_exceeded(void *data EINA_UNUSED)
{
   fprintf(stderr, "E: efreet didn't notify about cache update\n");
   _e_startup_error_dialog("E: Efreet did not update cache.<br>"
                           "Please check your Efreet setup.<br>"
                           "Is efreetd running?<br>"
                           "Can ~/.cache/efreet be written to?");
   timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}
