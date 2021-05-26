#include "e.h"

static Ecore_Event_Handler *_e_screensaver_handler_on = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_off = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_config_mode = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_border_fullscreen = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_border_unfullscreen = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_border_remove = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_border_iconify = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_border_uniconify = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_border_desk_set = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_desk_show = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_powersave = NULL;

static int _e_screensaver_cfg_timeout = -1;
static int _e_screensaver_cfg_dim = -1;

static int _e_screensaver_timeout = 0;
static int _e_screensaver_blanking = 0;
static int _e_screensaver_expose = 0;

static Ecore_Timer *_e_screensaver_suspend_timer = NULL;
static Eina_Bool _e_screensaver_on = EINA_FALSE;

static Ecore_Timer *screensaver_idle_timer = NULL;
static Eina_Bool screensaver_dimmed = EINA_FALSE;

static Eina_Bool _screensaver_ignore = EINA_FALSE;
static Eina_Bool _screensaver_now = EINA_FALSE;

E_API int E_EVENT_SCREENSAVER_ON = -1;
E_API int E_EVENT_SCREENSAVER_OFF = -1;
E_API int E_EVENT_SCREENSAVER_OFF_PRE = -1;

E_API int
e_screensaver_timeout_get(Eina_Bool use_idle)
{
   int timeout = 0;
   Eina_Bool use_special_instead_of_dim = EINA_FALSE;

   if (_screensaver_now) return 1;
   if (e_config->screensaver_enable)
     {
        if ((e_desklock_state_get()) &&
            (e_config->screensaver_desklock_timeout > 0))
          {
             timeout = e_config->screensaver_desklock_timeout;
             use_special_instead_of_dim = EINA_TRUE;
          }
        else
          timeout = e_config->screensaver_timeout;
     }

   if ((use_idle) && (!use_special_instead_of_dim))
     {
        if (e_config->backlight.idle_dim)
          {
             double t2 = e_config->backlight.timer;

             if ((e_powersave_mode_get() > E_POWERSAVE_MODE_LOW) &&
                 (e_config->backlight.battery_timer > 0.0))
               t2 = e_config->backlight.battery_timer;
             if (timeout > 0)
               {
                  if (t2 < timeout) timeout = t2;
               }
             else timeout = t2;
          }
     }
   return timeout;
}

E_API void
e_screensaver_ignore(void)
{
   _screensaver_ignore = EINA_TRUE;
}

E_API void
e_screensaver_unignore(void)
{
   _screensaver_ignore = EINA_FALSE;
}

E_API Eina_Bool
e_screensaver_ignore_get(void)
{
   return _screensaver_ignore;
}

E_API void
e_screensaver_update(void)
{
   int timeout, interval = 0, blanking = 0, expose = 0;
   Eina_Bool changed = EINA_FALSE;
   Eina_Bool real_changed = EINA_FALSE;
   int dim_timeout = -1;
   E_Powersave_Mode pm = e_powersave_mode_get();

   if (pm > E_POWERSAVE_MODE_LOW)
     dim_timeout = e_config->backlight.battery_timer;
   else
     dim_timeout = e_config->backlight.timer;

   if (_e_screensaver_cfg_timeout != e_config->screensaver_timeout)
     real_changed = EINA_TRUE;
   else if (_e_screensaver_cfg_dim != dim_timeout)
     real_changed = EINA_TRUE;
   _e_screensaver_cfg_timeout = e_config->screensaver_timeout;
   _e_screensaver_cfg_dim = dim_timeout;

   timeout = e_screensaver_timeout_get(EINA_TRUE);
   if (!((e_config->screensaver_enable) &&
         (!((e_util_fullscreen_current_any()) &&
            (e_config->no_dpms_on_fullscreen)))))
     timeout = 0;

   if (_e_screensaver_timeout != timeout)
     {
        _e_screensaver_timeout = timeout;
        changed = EINA_TRUE;
     }

   interval = e_config->screensaver_interval;
   blanking = e_config->screensaver_blanking;
   expose = e_config->screensaver_expose;

   if (_e_screensaver_blanking != blanking)
     {
        _e_screensaver_blanking = blanking;
        changed = EINA_TRUE;
     }
   if (_e_screensaver_expose != expose)
     {
        _e_screensaver_expose = expose;
        changed = EINA_TRUE;
     }

   if (changed)
     {
#ifndef HAVE_WAYLAND_ONLY
        if (e_comp->comp_type != E_PIXMAP_TYPE_WL)
          {
             // this toggling of dpms is a bug workaround in x that i found
             // where if we change screensaver timeouts and force a manual
             // wake of the screen e.g. on lid open/close we have to toggle
             // it for x to stop thinking the monitor is off when it's
             // actually on and this causes later dpms issues where the
             // screen doesn't turn off at all because x thinks internally
             // that the monitor is still off... so this is odd, but it's
             // necessary on some hardware.
             if ((real_changed) && (!e_config->screensaver_dpms_off))
               {
                  ecore_x_dpms_enabled_set(!e_config->screensaver_enable);
                  ecore_x_dpms_enabled_set(e_config->screensaver_enable);
               }
             ecore_x_screensaver_set(timeout, interval, blanking, expose);
          }
#endif
     }
}

static Eina_Bool
_e_screensaver_handler_config_mode_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   e_screensaver_update();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_screensaver_suspend_cb(void *data EINA_UNUSED)
{
   _e_screensaver_suspend_timer = NULL;
   if (e_config->screensaver_suspend)
     {
        if ((e_config->screensaver_suspend_on_ac) ||
            (e_powersave_mode_get() > E_POWERSAVE_MODE_LOW))
          {
             if (e_config->screensaver_hibernate)
               e_sys_action_do(E_SYS_HIBERNATE, NULL);
             else
               e_sys_action_do(E_SYS_SUSPEND, NULL);
          }
     }
   return EINA_FALSE;
}

static Eina_Bool
_e_screensaver_handler_powersave_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   e_screensaver_update();
   if ((e_config->screensaver_suspend) && (_e_screensaver_on))
     {
        if (_e_screensaver_suspend_timer)
          ecore_timer_del(_e_screensaver_suspend_timer);
        _e_screensaver_suspend_timer =
          ecore_timer_loop_add(e_config->screensaver_suspend_delay,
                          _e_screensaver_suspend_cb, NULL);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_screensaver_handler_screensaver_on_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   _e_screensaver_on = EINA_TRUE;
   if (_e_screensaver_suspend_timer)
     {
        ecore_timer_del(_e_screensaver_suspend_timer);
        _e_screensaver_suspend_timer = NULL;
     }
   if (e_config->screensaver_suspend)
     _e_screensaver_suspend_timer =
       ecore_timer_loop_add(e_config->screensaver_suspend_delay,
                       _e_screensaver_suspend_cb, NULL);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_screensaver_handler_screensaver_off_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
//   e_dpms_force_update();

   _e_screensaver_on = EINA_FALSE;
   if (_e_screensaver_suspend_timer)
     {
        ecore_timer_del(_e_screensaver_suspend_timer);
        _e_screensaver_suspend_timer = NULL;
     }

   if (!e_desklock_state_get())
     e_pointer_reset(e_comp->pointer);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_screensaver_handler_border_fullscreen_check_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   e_screensaver_update();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_screensaver_handler_border_desk_set_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   e_screensaver_update();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_screensaver_handler_desk_show_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   e_screensaver_update();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_screensaver_idle_timer_cb(void *d EINA_UNUSED)
{
   e_pointer_reset(e_comp->pointer);
   e_powersave_mode_screen_set(E_POWERSAVE_MODE_FREEZE);
   ecore_event_add(E_EVENT_SCREENSAVER_ON, NULL, NULL, NULL);
   screensaver_idle_timer = NULL;
   return EINA_FALSE;
}

EINTERN void
e_screensaver_preinit(void)
{
   E_EVENT_SCREENSAVER_ON = ecore_event_type_new();
   E_EVENT_SCREENSAVER_OFF = ecore_event_type_new();
   E_EVENT_SCREENSAVER_OFF_PRE = ecore_event_type_new();
}

EINTERN int
e_screensaver_init(void)
{
   _e_screensaver_handler_on = ecore_event_handler_add
       (E_EVENT_SCREENSAVER_ON, _e_screensaver_handler_screensaver_on_cb, NULL);
   _e_screensaver_handler_off = ecore_event_handler_add
       (E_EVENT_SCREENSAVER_OFF, _e_screensaver_handler_screensaver_off_cb, NULL);
   _e_screensaver_handler_config_mode = ecore_event_handler_add
       (E_EVENT_CONFIG_MODE_CHANGED, _e_screensaver_handler_config_mode_cb, NULL);

   _e_screensaver_handler_border_fullscreen = ecore_event_handler_add
       (E_EVENT_CLIENT_FULLSCREEN, _e_screensaver_handler_border_fullscreen_check_cb, NULL);
   _e_screensaver_handler_border_unfullscreen = ecore_event_handler_add
       (E_EVENT_CLIENT_UNFULLSCREEN, _e_screensaver_handler_border_fullscreen_check_cb, NULL);
   _e_screensaver_handler_border_remove = ecore_event_handler_add
       (E_EVENT_CLIENT_REMOVE, _e_screensaver_handler_border_fullscreen_check_cb, NULL);
   _e_screensaver_handler_border_iconify = ecore_event_handler_add
       (E_EVENT_CLIENT_ICONIFY, _e_screensaver_handler_border_fullscreen_check_cb, NULL);
   _e_screensaver_handler_border_uniconify = ecore_event_handler_add
       (E_EVENT_CLIENT_UNICONIFY, _e_screensaver_handler_border_fullscreen_check_cb, NULL);
   _e_screensaver_handler_border_desk_set = ecore_event_handler_add
       (E_EVENT_CLIENT_DESK_SET, _e_screensaver_handler_border_desk_set_cb, NULL);
   _e_screensaver_handler_desk_show = ecore_event_handler_add
       (E_EVENT_DESK_SHOW, _e_screensaver_handler_desk_show_cb, NULL);

   _e_screensaver_handler_powersave = ecore_event_handler_add
       (E_EVENT_POWERSAVE_UPDATE, _e_screensaver_handler_powersave_cb, NULL);

   return 1;
}

EINTERN int
e_screensaver_shutdown(void)
{
   if (_e_screensaver_handler_on)
     {
        ecore_event_handler_del(_e_screensaver_handler_on);
        _e_screensaver_handler_on = NULL;
     }

   if (_e_screensaver_handler_off)
     {
        ecore_event_handler_del(_e_screensaver_handler_off);
        _e_screensaver_handler_off = NULL;
     }

   if (_e_screensaver_suspend_timer)
     {
        ecore_timer_del(_e_screensaver_suspend_timer);
        _e_screensaver_suspend_timer = NULL;
     }

   if (_e_screensaver_handler_powersave)
     {
        ecore_event_handler_del(_e_screensaver_handler_powersave);
        _e_screensaver_handler_powersave = NULL;
     }

   if (_e_screensaver_handler_config_mode)
     {
        ecore_event_handler_del(_e_screensaver_handler_config_mode);
        _e_screensaver_handler_config_mode = NULL;
     }

   if (_e_screensaver_handler_border_fullscreen)
     {
        ecore_event_handler_del(_e_screensaver_handler_border_fullscreen);
        _e_screensaver_handler_border_fullscreen = NULL;
     }

   if (_e_screensaver_handler_border_unfullscreen)
     {
        ecore_event_handler_del(_e_screensaver_handler_border_unfullscreen);
        _e_screensaver_handler_border_unfullscreen = NULL;
     }

   if (_e_screensaver_handler_border_remove)
     {
        ecore_event_handler_del(_e_screensaver_handler_border_remove);
        _e_screensaver_handler_border_remove = NULL;
     }

   if (_e_screensaver_handler_border_iconify)
     {
        ecore_event_handler_del(_e_screensaver_handler_border_iconify);
        _e_screensaver_handler_border_iconify = NULL;
     }

   if (_e_screensaver_handler_border_uniconify)
     {
        ecore_event_handler_del(_e_screensaver_handler_border_uniconify);
        _e_screensaver_handler_border_uniconify = NULL;
     }

   if (_e_screensaver_handler_border_desk_set)
     {
        ecore_event_handler_del(_e_screensaver_handler_border_desk_set);
        _e_screensaver_handler_border_desk_set = NULL;
     }

   if (_e_screensaver_handler_desk_show)
     {
        ecore_event_handler_del(_e_screensaver_handler_desk_show);
        _e_screensaver_handler_desk_show = NULL;
     }

   return 1;
}

E_API void
e_screensaver_attrs_set(int timeout, int blanking, int expose)
{
   _e_screensaver_timeout = timeout;
//   _e_screensaver_interval = ecore_x_screensaver_interval_get();
   _e_screensaver_blanking = blanking;
   _e_screensaver_expose = expose;
}

E_API Eina_Bool
e_screensaver_on_get(void)
{
   return _e_screensaver_on;
}

E_API void
e_screensaver_activate(void)
{
   if (e_screensaver_on_get()) return;

   E_FREE_FUNC(screensaver_idle_timer, ecore_timer_del);
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type != E_PIXMAP_TYPE_WL)
     ecore_x_screensaver_activate();
#endif
#ifdef HAVE_WAYLAND
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     e_comp_wl_screensaver_activate();
#endif
}

E_API void
e_screensaver_deactivate(void)
{
   if (!e_screensaver_on_get()) return;

   E_FREE_FUNC(screensaver_idle_timer, ecore_timer_del);
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type != E_PIXMAP_TYPE_WL)
     ecore_x_screensaver_reset();
#endif
#ifdef HAVE_WAYLAND
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     e_comp_canvas_notidle();
#endif
}

E_API void
e_screensaver_now_set(Eina_Bool now)
{
   _screensaver_now = now;
}

E_API void
e_screensaver_eval(Eina_Bool saver_on)
{
   Eina_Bool use_special_instead_of_dim = EINA_FALSE;

   if ((e_desklock_state_get()) &&
       (e_config->screensaver_desklock_timeout > 0))
     use_special_instead_of_dim = EINA_TRUE;
   if (saver_on)
     {
        if ((e_config->backlight.idle_dim) &&
            (!use_special_instead_of_dim))
          {
             double t2 = e_config->backlight.timer;

             if ((e_powersave_mode_get() > E_POWERSAVE_MODE_LOW) &&
                 (e_config->backlight.battery_timer > 0.0))
               t2 = e_config->backlight.battery_timer;
             double t = e_config->screensaver_timeout - t2;

             if (t < 1.0) t = 1.0;
             if (_screensaver_now) t = 1.0;
             E_FREE_FUNC(screensaver_idle_timer, ecore_timer_del);
             if (!_screensaver_ignore)
               {
                  if (e_config->screensaver_enable)
                    screensaver_idle_timer = ecore_timer_loop_add
                      (t, _e_screensaver_idle_timer_cb, NULL);
                  if (e_backlight_mode_get(NULL) != E_BACKLIGHT_MODE_DIM)
                    {
                       e_backlight_mode_set(NULL, E_BACKLIGHT_MODE_DIM);
                       screensaver_dimmed = EINA_TRUE;
                    }
               }
          }
        else
          {
             if (!_screensaver_ignore)
               {
                  if (!e_screensaver_on_get())
                    {
                       e_powersave_mode_screen_set(E_POWERSAVE_MODE_FREEZE);
                       ecore_event_add(E_EVENT_SCREENSAVER_ON, NULL, NULL, NULL);
                    }
               }
          }
        return;
     }
   else
     {
#ifdef HAVE_WAYLAND
        if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
          ecore_event_add(E_EVENT_SCREENSAVER_OFF_PRE, NULL, NULL, NULL);
#endif
     }
   if (screensaver_idle_timer)
     {
        E_FREE_FUNC(screensaver_idle_timer, ecore_timer_del);
        if ((e_config->backlight.idle_dim) &&
            (!use_special_instead_of_dim))
          {
             if (e_backlight_mode_get(NULL) != E_BACKLIGHT_MODE_NORMAL)
               e_backlight_mode_set(NULL, E_BACKLIGHT_MODE_NORMAL);
          }
        return;
     }
   if (screensaver_dimmed)
     {
        if (e_backlight_mode_get(NULL) != E_BACKLIGHT_MODE_NORMAL)
          e_backlight_mode_set(NULL, E_BACKLIGHT_MODE_NORMAL);
        screensaver_dimmed = EINA_FALSE;
     }
   if (!_screensaver_ignore)
     {
        if (e_screensaver_on_get())
          {
             e_powersave_mode_screen_unset();
             e_screensaver_update();
             ecore_event_add(E_EVENT_SCREENSAVER_OFF, NULL, NULL, NULL);
          }
     }
}

E_API void
e_screensaver_inhibit_toggle(Eina_Bool inhibit)
{
#ifdef HAVE_WAYLAND
   if (e_comp->comp_type != E_PIXMAP_TYPE_WL) return;
   e_comp_wl_screensaver_inhibit(inhibit);
#else
   (void)inhibit;
#endif
}

