#include "e.h"

/**************************** private data ******************************/

typedef struct _E_Desklock_Run E_Desklock_Run;


struct _E_Desklock_Run
{
   E_Order *desk_run;
   int      position;
};

static Ecore_Event_Handler *_e_desklock_run_handler = NULL;
static Ecore_Event_Handler *_e_desklock_randr_handler = NULL;
static Ecore_Job *job = NULL;
static Eina_List *tasks = NULL;

static Eina_List *show_hooks = NULL;
static Eina_List *hide_hooks = NULL;

static Evas_Object *block_rects[32] = {NULL};
static Eina_Bool block_zone[32] = {EINA_FALSE};

static Eina_List *desklock_ifaces = NULL;
static E_Desklock_Interface *current_iface = NULL;
static Eina_Bool demo = EINA_FALSE;

static Eina_Bool _e_desklock_want = EINA_FALSE;
static int _e_desklock_block = 0;
static Eina_Bool desklock_manual = EINA_FALSE;

/***********************************************************************/
static Eina_Bool _e_desklock_cb_run(void *data, int type, void *event);
static Eina_Bool _e_desklock_cb_randr(void *data, int type, void *event);

static Eina_Bool _e_desklock_state = EINA_FALSE;

E_API int E_EVENT_DESKLOCK = 0;

EINTERN int
e_desklock_init(void)
{
   Eina_List *l;
   E_Config_Desklock_Background *bg;

   EINA_LIST_FOREACH(e_config->desklock_backgrounds, l, bg)
     e_filereg_register(bg->file);

   E_EVENT_DESKLOCK = ecore_event_type_new();

   _e_desklock_run_handler = ecore_event_handler_add(E_EVENT_DESKLOCK,
                                                     _e_desklock_cb_run, NULL);

   _e_desklock_randr_handler = ecore_event_handler_add(E_EVENT_RANDR_CHANGE,
                                                       _e_desklock_cb_randr, NULL);
   return 1;
}

EINTERN int
e_desklock_shutdown(void)
{
   E_Desklock_Run *task;
   Eina_List *l;
   E_Config_Desklock_Background *bg;

   desklock_ifaces = eina_list_free(desklock_ifaces);
   if (!x_fatal)
     e_desklock_hide();

   ecore_event_handler_del(_e_desklock_run_handler);
   _e_desklock_run_handler = NULL;
   ecore_event_handler_del(_e_desklock_randr_handler);
   _e_desklock_randr_handler = NULL;

   if (job) ecore_job_del(job);
   job = NULL;

   EINA_LIST_FOREACH(e_config->desklock_backgrounds, l, bg)
     e_filereg_deregister(bg->file);

   EINA_LIST_FREE(tasks, task)
     {
        e_object_del(E_OBJECT(task->desk_run));
        free(task);
     }

   return 1;
}

E_API Eina_Stringshare *
e_desklock_user_wallpaper_get(E_Zone *zone)
{
   const E_Config_Desktop_Background *cdbg;
   const Eina_List *l;
   E_Desk *desk;

   desk = e_desk_current_get(zone);
   EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cdbg)
     {
        if ((cdbg->zone > -1) && (cdbg->zone != (int)zone->num)) continue;
        if ((cdbg->desk_x > -1) && (cdbg->desk_x != desk->x)) continue;
        if ((cdbg->desk_y > -1) && (cdbg->desk_y != desk->y)) continue;
        if (cdbg->file) return cdbg->file;
     }

   if (e_config->desktop_default_background)
     return e_config->desktop_default_background;

   return e_theme_edje_file_get("base/theme/desklock", "e/desklock/background");
}

E_API void
e_desklock_interface_append(E_Desklock_Interface *iface)
{
   EINA_SAFETY_ON_NULL_RETURN(iface);
   EINA_SAFETY_ON_NULL_RETURN(iface->show);
   EINA_SAFETY_ON_NULL_RETURN(iface->name);
   EINA_SAFETY_ON_TRUE_RETURN(iface->active); // fucking casuals
   /* make sure our module is first so it gets tried last */
   if (!e_util_strcmp(iface->name, "lokker"))
     desklock_ifaces = eina_list_prepend(desklock_ifaces, (void*)iface);
   else
     desklock_ifaces = eina_list_append(desklock_ifaces, (void*)iface);
   if (_e_desklock_state && (!current_iface))
     {
        if (iface->show())
          {
             iface->active = EINA_TRUE;
             current_iface = iface;
          }
     }
}

EINTERN E_Desklock_Interface *
e_desklock_interface_current_get(void)
{
   return current_iface;
}

E_API void
e_desklock_interface_remove(E_Desklock_Interface *iface)
{
   E_Desklock_Interface *diface;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(iface);
   if (!desklock_ifaces) return;
   desklock_ifaces = eina_list_remove(desklock_ifaces, (void*)iface);
   if (!iface->active) return;
   /* if it was active, hide it */
   if (iface->hide) iface->hide();
   iface->active = EINA_FALSE;
   current_iface = NULL;
   /* then try to find a replacement locker */
   EINA_LIST_REVERSE_FOREACH(desklock_ifaces, l, diface)
     {
        if (!diface->show()) continue;
        diface->active = EINA_TRUE;
        current_iface = diface;
        break;
     }
   /* XXX: if none of the remaining lockers can lock, we're left with a black screen.
    * I again blame the user for magically unloading the current locker module DURING
    * desklock, but I refuse to unlock their system.
   if (!current_iface) e_desklock_hide();
    */
}

E_API void
e_desklock_show_hook_add(E_Desklock_Show_Cb cb)
{
   EINA_SAFETY_ON_NULL_RETURN(cb);
   show_hooks = eina_list_append(show_hooks, cb);
}

E_API void
e_desklock_show_hook_del(E_Desklock_Show_Cb cb)
{
   EINA_SAFETY_ON_NULL_RETURN(cb);
   show_hooks = eina_list_remove(show_hooks, cb);
}

E_API void
e_desklock_hide_hook_add(E_Desklock_Hide_Cb cb)
{
   EINA_SAFETY_ON_NULL_RETURN(cb);
   hide_hooks = eina_list_append(hide_hooks, cb);
}

E_API void
e_desklock_hide_hook_del(E_Desklock_Hide_Cb cb)
{
   EINA_SAFETY_ON_NULL_RETURN(cb);
   hide_hooks = eina_list_remove(hide_hooks, cb);
}

E_API int
e_desklock_show_autolocked(void)
{
   return e_desklock_show(EINA_FALSE);
}

E_API Eina_Bool
e_desklock_demo(void)
{
   E_Desklock_Interface *iface;
   Eina_List *l;

   EINA_LIST_REVERSE_FOREACH(desklock_ifaces, l, iface)
     {
        if (iface->show())
          {
             demo = iface->active = EINA_TRUE;
             current_iface = iface;
             e_comp_shape_queue();
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static int
_desklock_show_internal(Eina_Bool suspend)
{
   const Eina_List *l;
   E_Event_Desklock *ev;
   E_Desklock_Show_Cb show_cb;
   E_Desklock_Hide_Cb hide_cb;
   E_Zone *zone;

#if !defined(HAVE_PAM) && !defined(__FreeBSD__)  && !defined(__OpenBSD__)
   if (!e_desklock_is_personal())
     {
        e_util_dialog_show(_("Error - no PAM support"),
                           _("No PAM support was built into Enlightenment, so<ps/>"
                             "desk locking is disabled."));
        return EINA_FALSE;
     }
#endif
   if (_e_desklock_state) return EINA_TRUE;

   if (e_desklock_is_personal())
     {
        if (!e_config->desklock_passwd)
          {
             zone = e_zone_current_get();
             if (zone)
               e_configure_registry_call("screen/screen_lock", NULL, NULL);
             return 0;
          }
     }

   e_menu_hide_all();
   EINA_LIST_FOREACH(show_hooks, l, show_cb)
     {
        if (!show_cb()) goto fail;
     }

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        Evas_Object *o;

        if (zone->num >= EINA_C_ARRAY_LENGTH(block_rects))
          {
             CRI("> %lu screens connected????",
                 (unsigned long)EINA_C_ARRAY_LENGTH(block_rects));
             break;
          }
        o = evas_object_rectangle_add(e_comp->evas);
        block_rects[zone->num] = o;
        evas_object_color_set(o, 0, 0, 0, 0);
        evas_object_geometry_set(o, zone->x, zone->y, zone->w, zone->h);
        evas_object_layer_set(o, E_LAYER_DESKLOCK);
        if (!block_zone[zone->num])
          evas_object_show(o);
     }
   if (e_config->desklock_language)
     e_intl_language_set(e_config->desklock_language);

   if (e_config->xkb.lock_layout)
     e_xkb_layout_set(e_config->xkb.lock_layout);

   if (demo)
     {
        if (current_iface->hide)
          current_iface->hide();
        current_iface->active = demo = EINA_FALSE;
        current_iface = NULL;
     }

   {
      E_Desklock_Interface *iface;
      Eina_Bool success = EINA_TRUE;

      EINA_LIST_REVERSE_FOREACH(desklock_ifaces, l, iface)
        {
           success = iface->show();
           if (success)
             {
                iface->active = EINA_TRUE;
                current_iface = iface;
                break;
             }
        }
      /* FIXME: if someone doesn't have a locking module loaded and has
       * lock-on-startup, this will result in a permanent black screen.
       * I blame the user in this case since lokker is a default module
       * which is really hard to disable (you have to work at it or be a gentoo user).
       */
      if (!success) goto lang_fail;
   }

   ev = E_NEW(E_Event_Desklock, 1);
   ev->on = 1;
   ev->suspend = suspend;
   ecore_event_add(E_EVENT_DESKLOCK, ev, NULL, NULL);

   if (getenv("E_START_MANAGER")) kill(getppid(), SIGUSR2);
   _e_desklock_state = EINA_TRUE;
   e_bindings_disabled_set(1);
   e_screensaver_update();
   e_dpms_force_update();
   return 1;
lang_fail:
   if (e_config->desklock_language)
     e_intl_language_set(e_config->language);

   if (e_config_xkb_layout_eq(e_config->xkb.current_layout, e_config->xkb.lock_layout))
     {
        if (e_config->xkb.sel_layout)
          e_xkb_layout_set(e_config->xkb.sel_layout);
     }
fail:
   EINA_LIST_FOREACH(hide_hooks, l, hide_cb)
     hide_cb();
   return 0;
}

E_API int
e_desklock_show(Eina_Bool suspend)
{
   _e_desklock_want = EINA_TRUE;
   if ((_e_desklock_block > 0) && (!desklock_manual)) return EINA_FALSE;
   return _desklock_show_internal(suspend);
}

static void
_desklock_hide_internal(void)
{
   Eina_List *l;
   E_Event_Desklock *ev;
   E_Desklock_Hide_Cb hide_cb;

   if (demo && current_iface)
     {
        if (current_iface->hide)
          current_iface->hide();
        demo = current_iface->active = EINA_FALSE;
        current_iface = NULL;
        return;
     }
   demo = EINA_FALSE;

   if (!_e_desklock_state) return;

   e_comp_override_del();
   e_comp_shape_queue();
   {
      unsigned int n;

      for (n = 0; n < EINA_C_ARRAY_LENGTH(block_rects); n++)
        {
           E_FREE_FUNC(block_rects[n], evas_object_del);
           block_zone[n] = EINA_FALSE;
        }
   }
   //e_comp_block_window_del();
   if (e_config->desklock_language)
     e_intl_language_set(e_config->language);

   if (e_config_xkb_layout_eq(e_config->xkb.current_layout, e_config->xkb.lock_layout))
     {
        if (e_config->xkb.sel_layout)
          e_xkb_layout_set(e_config->xkb.sel_layout);
     }

   _e_desklock_state = EINA_FALSE;
   e_bindings_disabled_set(0);
   ev = E_NEW(E_Event_Desklock, 1);
   ev->on = 0;
   ev->suspend = 1;
   ecore_event_add(E_EVENT_DESKLOCK, ev, NULL, NULL);

   e_screensaver_update();
   e_dpms_force_update();

   EINA_LIST_FOREACH(hide_hooks, l, hide_cb)
     hide_cb();

   if (current_iface)
     {
        if (current_iface->hide)
          current_iface->hide();
        current_iface->active = EINA_FALSE;
        current_iface = NULL;
     }

   if (getenv("E_START_MANAGER")) kill(getppid(), SIGHUP);

   e_pointer_reset(e_comp->pointer);
}

E_API int
e_desklock_show_manual(Eina_Bool suspend)
{
   desklock_manual = EINA_TRUE;
   return e_desklock_show(suspend);
}

E_API Eina_Bool
e_desklock_manual_get(void)
{
   return desklock_manual;
}

E_API void
e_desklock_hide(void)
{
   desklock_manual = EINA_FALSE;
   _e_desklock_want = EINA_FALSE;
   _desklock_hide_internal();
}

E_API Eina_Bool
e_desklock_state_get(void)
{
   return _e_desklock_state;
}

E_API void
e_desklock_block(void)
{
   _e_desklock_block++;
   if (_e_desklock_block == 1)
     {
        if (!desklock_manual)
          {
             if (_e_desklock_state) _desklock_hide_internal();
          }
     }
}

E_API void
e_desklock_unblock(void)
{
   _e_desklock_block--;
   if (_e_desklock_block == 0)
     {
        if (_e_desklock_want) e_desklock_show(EINA_FALSE);
     }
   else if (_e_desklock_block < 0)
     {
        ERR("desklock block going below zero");
     }
}

static Eina_Bool
_e_desklock_run(E_Desklock_Run *task)
{
   Efreet_Desktop *desktop;

   desktop = eina_list_nth(task->desk_run->desktops, task->position++);
   if (!desktop)
     {
        e_object_del(E_OBJECT(task->desk_run));
        free(task);
        return EINA_FALSE;
     }

   e_exec(NULL, desktop, NULL, NULL, NULL);
   return EINA_TRUE;
}

static void
_e_desklock_job(void *data EINA_UNUSED)
{
   E_Desklock_Run *task;

   job = NULL;
   if (!tasks) return;

   task = eina_list_data_get(tasks);
   if (!_e_desklock_run(task))
     tasks = eina_list_remove_list(tasks, tasks);

   if (tasks) job = ecore_job_add(_e_desklock_job, NULL);
}

static Eina_Bool
_e_desklock_cb_run(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Desklock_Run *task;
   E_Event_Desklock *ev = event;
   E_Order *desk_run;
   char buf[PATH_MAX];

   if (!ev->suspend) return ECORE_CALLBACK_PASS_ON;

   if (ev->on)
     {
        e_user_dir_concat_static(buf, "applications/desk-lock/.order");
        if (!ecore_file_exists(buf))
          e_prefix_data_concat_static(buf, "applications/desk-lock/.order");
     }
   else
     {
        e_user_dir_concat_static(buf, "applications/desk-unlock/.order");
        if (!ecore_file_exists(buf))
          e_prefix_data_concat_static(buf, "applications/desk-unlock/.order");
     }

   desk_run = e_order_new(buf);
   if (!desk_run) return ECORE_CALLBACK_PASS_ON;

   task = calloc(1, sizeof (E_Desklock_Run));
   if (!task)
     {
        e_object_del(E_OBJECT(desk_run));
        return ECORE_CALLBACK_PASS_ON;
     }

   task->desk_run = desk_run;
   tasks = eina_list_append(tasks, task);

   if (!job) ecore_job_add(_e_desklock_job, NULL);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_desklock_cb_randr(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   if (!_e_desklock_state) return ECORE_CALLBACK_PASS_ON;
   e_desklock_hide();
   e_desklock_show(EINA_FALSE);
   return ECORE_CALLBACK_PASS_ON;
}

E_API void
e_desklock_zone_block_set(const E_Zone *zone, Eina_Bool block)
{
   EINA_SAFETY_ON_NULL_RETURN(zone);
   if (zone->num >= EINA_C_ARRAY_LENGTH(block_rects))
     {
        CRI("> %lu screens connected????",
            (unsigned long)EINA_C_ARRAY_LENGTH(block_rects));
        return;
     }
   block_zone[zone->num] = !!block;
   if (!block_rects[zone->num]) return;
   if (block)
     evas_object_show(block_rects[zone->num]);
   else
     evas_object_hide(block_rects[zone->num]);
}
