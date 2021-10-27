#include "e.h"

static Eina_List *handlers;
static Ecore_Timer *timer_post_screensaver_lock = NULL;
static Ecore_Timer *timer_post_screensaver_on = NULL;
static Ecore_Timer *timer_pointer_freeze = NULL;

static void
_e_comp_canvas_cb_del()
{
   E_FREE_LIST(handlers, ecore_event_handler_del);
   E_FREE_FUNC(timer_post_screensaver_lock, ecore_timer_del);
   E_FREE_FUNC(timer_post_screensaver_on, ecore_timer_del);
   E_FREE_FUNC(timer_pointer_freeze, ecore_timer_del);
}

static void
_e_comp_canvas_event_compositor_resize_free(void *data EINA_UNUSED, void *event EINA_UNUSED)
{
   e_object_unref(E_OBJECT(e_comp));
}

///////////////////////////////////

static void
_e_comp_canvas_cb_first_frame(void *data EINA_UNUSED, Evas *e, void *event_info EINA_UNUSED)
{
   double now = ecore_time_get();

   switch (e_first_frame[0])
     {
      case 'A': abort();
      case 'E':
      case 'D': exit(101);
      case 'T': fprintf(stderr, "Startup time: '%f' - '%f' = '%f'\n", now, e_first_frame_start_time, now - e_first_frame_start_time);
         break;
     }

   evas_event_callback_del_full(e, EVAS_CALLBACK_RENDER_POST, _e_comp_canvas_cb_first_frame, NULL);
}

static Ecore_Job *_e_comp_update_job = NULL;

static void
_e_comp_canvas_fps_update_job(void *data EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();
   _e_comp_update_job = NULL;
   if (conf->fps_show) e_comp_fps_update();
}

static Ecore_Job *_e_comp_post_render_job = NULL;

static void
_e_comp_canvas_render_post_job(void *data EINA_UNUSED)
{
   E_Client *ec;

   _e_comp_post_render_job = NULL;
   EINA_LIST_FREE(e_comp->post_updates, ec)
     {
        //INF("POST %p", ec);
        ec->on_post_updates = EINA_FALSE;
        if (!e_object_is_del(E_OBJECT(ec)))
          e_pixmap_image_clear(ec->pixmap, 1);
        evas_object_smart_callback_call(ec->frame, "post_render", NULL);
        UNREFD(ec, 111);
        e_object_unref(E_OBJECT(ec));
     }
}

static void
_e_comp_canvas_render_track_pre(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();
   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_RENDER_BEGIN, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_time_get());
     }
}

static void
_e_comp_canvas_render_track_post(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();
   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_RENDER_END, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_time_get());
     }
}

static void
_e_comp_canvas_render_track_flush_pre(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();
   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_RENDER2_BEGIN, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_time_get());
     }
}

static void
_e_comp_canvas_render_track_flush_post(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();
   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_RENDER2_END, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_time_get());
     }
}

static void
_e_comp_canvas_render_post(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();

   // NOTE: don't modify objects here as we may lose updates - defer...
   if (conf->fps_show)
     {
        if (_e_comp_update_job) ecore_job_del(_e_comp_update_job);
        ecore_job_add(_e_comp_canvas_fps_update_job, NULL);
     }

   e_comp->rendering = EINA_FALSE;

   if (_e_comp_post_render_job) ecore_job_del(_e_comp_post_render_job);
   _e_comp_post_render_job = ecore_job_add(_e_comp_canvas_render_post_job, NULL);

   if (conf->grab && e_comp->grabbed)
     {
        if (e_comp->grab_cb) e_comp->grab_cb();
        e_comp->grabbed = 0;
     }
}

///////////////////////////////////

static void
_e_comp_canvas_cb_mouse_in(void *d EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *ec;

   e_comp_canvas_notidle();
   if (e_client_action_get() || e_grabinput_mouse_win_get()) return;
   ec = e_client_focused_get();
   if (ec && (!ec->border_menu)) e_focus_event_mouse_out(ec);
}

static void
_e_comp_canvas_cb_mouse_down(void *d EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   e_comp_canvas_notidle();
   if (e_client_action_get() || e_grabinput_mouse_win_get()) return;
   e_bindings_mouse_down_evas_event_handle(E_BINDING_CONTEXT_COMPOSITOR, E_OBJECT(e_comp), event_info);
}

static void
_e_comp_canvas_cb_mouse_up(void *d EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   e_comp_canvas_notidle();
   if (e_client_action_get() || e_grabinput_mouse_win_get()) return;
   e_bindings_mouse_up_evas_event_handle(E_BINDING_CONTEXT_COMPOSITOR, E_OBJECT(e_comp), event_info);
}

static void
_e_comp_canvas_cb_mouse_wheel(void *d EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   e_comp_canvas_notidle();
   if (e_client_action_get() || e_grabinput_mouse_win_get()) return;
   e_bindings_wheel_evas_event_handle(E_BINDING_CONTEXT_COMPOSITOR, E_OBJECT(e_comp), event_info);
}

static Eina_Bool
_key_down(int ctx, Ecore_Event_Key *ev)
{
   e_comp_canvas_notidle();
   if (e_desklock_state_get() && (ctx == E_BINDING_CONTEXT_MANAGER))
     {
        E_Desklock_Interface *iface = e_desklock_interface_current_get();
        if (iface && iface->key_down)
          {
#ifdef HAVE_WAYLAND
             if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
               e_comp_wl_key_down(ev, NULL);
#endif
             return iface->key_down(ev);
          }
     }
   if ((e_comp->comp_type == E_PIXMAP_TYPE_X) && (ev->event_window != e_comp->root))
     {
        E_Client *ec;

        ec = e_client_focused_get();
        if (!ec) ec = e_client_action_get();
        /* *block actions when no client is focused (probably something else did a grab here so we'll play nice)
         * *block actions when menu is up
         * *block actions when event (grab) window isn't comp window
         * *other cases?
         */
        if ((!ec) || (ev->event_window != e_comp->ee_win)) return ECORE_CALLBACK_RENEW;
     }
   return ((!e_comp->screen) ||
           (!e_comp->screen->key_down) || (!e_comp->screen->key_down(ev))) &&
     !e_bindings_key_down_event_handle(ctx, E_OBJECT(e_comp), ev)
#ifdef HAVE_WAYLAND
       && !e_comp_wl_key_down(ev, NULL)
#endif
         ;
}

static Eina_Bool
_e_comp_canvas_cb_key_down(void *data EINA_UNUSED, int ev_type EINA_UNUSED, Ecore_Event_Key *ev)
{
   return _key_down(E_BINDING_CONTEXT_COMPOSITOR, ev);
}

static Eina_Bool
_e_comp_cb_key_down(void *data EINA_UNUSED, int ev_type EINA_UNUSED, Ecore_Event_Key *ev)
{
   return _key_down(E_BINDING_CONTEXT_MANAGER, ev);
}

static Eina_Bool
_key_up(int ctx, Ecore_Event_Key *ev)
{
   e_comp_canvas_notidle();
   if (e_desklock_state_get() && (ctx == E_BINDING_CONTEXT_MANAGER))
     {
        E_Desklock_Interface *iface = e_desklock_interface_current_get();
        if (iface && iface->key_up)
          {
#ifdef HAVE_WAYLAND
             if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
               e_comp_wl_key_up(ev, NULL);
#endif
             return iface->key_up(ev);
          }
     }
   if ((e_comp->comp_type == E_PIXMAP_TYPE_X) && (ev->event_window != e_comp->root)) return ECORE_CALLBACK_PASS_ON;
   return ((!e_comp->screen) ||
           (!e_comp->screen->key_up) || (!e_comp->screen->key_up(ev))) &&
     !e_bindings_key_up_event_handle(ctx, E_OBJECT(e_comp), ev)
#ifdef HAVE_WAYLAND
       && !e_comp_wl_key_up(ev, NULL)
#endif
         ;
}

static Eina_Bool
_e_comp_canvas_cb_key_up(void *data EINA_UNUSED, int ev_type EINA_UNUSED, Ecore_Event_Key *ev)
{
   return _key_up(E_BINDING_CONTEXT_COMPOSITOR, ev);
}

static Eina_Bool
_e_comp_cb_key_up(void *data EINA_UNUSED, int ev_type EINA_UNUSED, Ecore_Event_Key *ev)
{
   return _key_up(E_BINDING_CONTEXT_MANAGER, ev);
}

static Eina_Bool
_e_comp_cb_mouse_up(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Mouse_Button *ev)
{
   if ((e_comp->comp_type == E_PIXMAP_TYPE_X) && (ev->event_window != e_comp->root))
     return ECORE_CALLBACK_PASS_ON;
   if (e_bindings_mouse_down_ecore_event_handle(E_BINDING_CONTEXT_MANAGER, E_OBJECT(e_comp), ev))
     return ECORE_CALLBACK_DONE;

#ifdef HAVE_WAYLAND
   return e_comp_wl_grab_client_mouse_button(ev);
#else
   return ECORE_CALLBACK_RENEW;
#endif
}

static Eina_Bool
_e_comp_cb_mouse_down(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Mouse_Button *ev)
{
   if ((e_comp->comp_type == E_PIXMAP_TYPE_X) && (ev->event_window != e_comp->root))
     return ECORE_CALLBACK_PASS_ON;
   if (e_bindings_mouse_down_ecore_event_handle(E_BINDING_CONTEXT_MANAGER, E_OBJECT(e_comp), ev))
     return ECORE_CALLBACK_DONE;
#ifdef HAVE_WAYLAND
   return e_comp_wl_grab_client_mouse_button(ev);
#else
   return ECORE_CALLBACK_RENEW;
#endif
}

static Eina_Bool
_e_comp_cb_mouse_wheel(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Mouse_Wheel *ev)
{
   if ((e_comp->comp_type == E_PIXMAP_TYPE_X) && (ev->event_window != e_comp->root))
     return ECORE_CALLBACK_PASS_ON;
   return !e_bindings_wheel_ecore_event_handle(E_BINDING_CONTEXT_MANAGER, E_OBJECT(e_comp), ev);
}

#ifdef HAVE_WAYLAND
static Eina_Bool
_e_comp_cb_mouse_move(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Mouse_Move *ev)
{
   static int x = -9999, y = -9999;

   if (e_comp->comp_type != E_PIXMAP_TYPE_WL) return ECORE_CALLBACK_RENEW;
   if (x == -9999)
     x = ev->x, y = ev->y;
   if ((x == ev->x) && (y == ev->y)) return ECORE_CALLBACK_RENEW;
   if (e_client_focused_get() && (!e_comp_util_mouse_grabbed()))
     {
        if ((!e_comp->screen) || (!e_comp->screen->relative_motion))
          {
             e_comp_wl_extension_relative_motion_event(ecore_time_unix_get() * 1000ULL,
               ev->x - x, ev->y - y, 0, 0);
          }
        x = ev->x, y = ev->y;
        if (e_comp_wl_extension_pointer_constraints_update(e_client_focused_get(), ev->x, ev->y))
          return ECORE_CALLBACK_CANCEL;
     }
   x = ev->x, y = ev->y;
   return e_comp_wl_grab_client_mouse_move(ev);
}
#endif

////////////////////////////////////

static Eina_Bool
_e_comp_cb_zone_change(void *d EINA_UNUSED, int type, void *event)
{
   if (type == E_EVENT_ZONE_ADD)
     {
        E_Event_Zone_Add *ev = event;
        E_Client *ec;

        E_CLIENT_FOREACH(ec)
          {
             if (ec->restore_zone_id)
               {
                  E_Zone *restore_zone =
                    e_zone_for_id_get(ec->restore_zone_id);
                  if ((restore_zone) && (ev->zone) &&
                      (restore_zone == ev->zone))
                    {
                       int dx, dy;
                       E_Desk *desk;

                       dx = dy = -1;
                       if (ec->desk)
                         {
                            dx = ec->desk->x;
                            dy = ec->desk->y;
                         }
                       e_client_zone_set(ec, restore_zone);
                       if ((dx >= 0) && (dy >= 0))
                         {
                            desk = e_desk_at_xy_get(restore_zone, dx, dy);
                            if (desk)
                              {
                                 e_client_desk_set(ec, desk);
                                 if (desk == e_desk_current_get(restore_zone))
                                   {
                                      if ((ec->visible) && (!ec->hidden) && (!ec->iconic))
                                        evas_object_show(ec->frame);
                                   }
                              }
                         }
                       e_client_res_change_geometry_restore(ec);
                    }
               }
          }
     }
   e_comp_canvas_update();
   return ECORE_CALLBACK_PASS_ON;
}

////////////////////////////////////

static Eina_Bool
_e_comp_cb_screensaver_active_delay(void *data EINA_UNUSED)
{
   if (!e_comp->nocomp)
     ecore_evas_manual_render_set(e_comp->ee, EINA_TRUE);
   timer_post_screensaver_on = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_e_comp_canvas_screensaver_active(void *d EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   if (timer_post_screensaver_on) return;
   /* thawed in _e_comp_screensaver_off() */
   timer_post_screensaver_on = ecore_timer_loop_add
     (1.0, _e_comp_cb_screensaver_active_delay, NULL);
}

static Eina_Bool
_e_comp_cb_timer_post_screensaver_lock(void *data EINA_UNUSED)
{
   // XXX: the below should instantly show desklock and have no transition
   // or animation
   e_desklock_show_autolocked();
   timer_post_screensaver_lock = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_e_comp_cb_screensaver_ponter_freeze_cb(void *data EINA_UNUSED)
{
   e_pointers_freeze_set(EINA_TRUE);
   timer_pointer_freeze = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_e_comp_cb_screensaver_on()
{
   // XXX: this is not quite right. this here should be called after e_comp.c
   // and freinds have emitted "e,state,screensaver,on" AND gotten a signal
   // reply from the theme to say it's done with this animation before
   // doing the below
   if (e_config->desklock_autolock_screensaver)
     {
        E_FREE_FUNC(timer_post_screensaver_lock, ecore_timer_del);
        if (e_config->desklock_post_screensaver_time <= 1.0)
          e_desklock_show_autolocked();
        else
          timer_post_screensaver_lock = ecore_timer_loop_add
              (e_config->desklock_post_screensaver_time,
              _e_comp_cb_timer_post_screensaver_lock, NULL);
     }
   if (timer_pointer_freeze) ecore_timer_del(timer_pointer_freeze);
   timer_pointer_freeze = ecore_timer_add
     (10.0, _e_comp_cb_screensaver_ponter_freeze_cb, NULL);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_comp_cb_screensaver_off()
{
   E_FREE_FUNC(timer_post_screensaver_lock, ecore_timer_del);
   E_FREE_FUNC(timer_post_screensaver_on, ecore_timer_del);
   e_pointers_freeze_set(EINA_FALSE);
   if (timer_pointer_freeze)
     {
        ecore_timer_del(timer_pointer_freeze);
        timer_pointer_freeze = NULL;
     }
   return ECORE_CALLBACK_PASS_ON;
}
////////////////////////////////////

static int
_e_comp_canvas_cb_zone_sort(const void *data1, const void *data2)
{
   const E_Zone *z1 = data1, *z2 = data2;

   return z1->num - z2->num;
}

static void
_e_comp_canvas_prerender(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Comp_Cb cb;
   Eina_List *l;
   E_Comp_Config *conf = e_comp_config_get();

   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_RENDER_BEGIN, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_time_get());
     }

   e_comp->rendering = EINA_TRUE;

   if (conf->grab && (!e_comp->grabbed))
     {
        if (e_comp->grab_cb) e_comp->grab_cb();
        e_comp->grabbed = 1;
     }

   EINA_LIST_FOREACH(e_comp->pre_render_cbs, l, cb)
     cb();
}

static Eina_Bool
_e_comp_canvas_cb_idle_enterer(void *data EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();
   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_IDLE_ENTER, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_time_get());
     }
   return EINA_TRUE;
}

static Eina_Bool
_e_comp_canvas_cb_idle_exiter(void *data EINA_UNUSED)
{
   E_Comp_Config *conf = e_comp_config_get();
   if (conf->fps_show)
   {
      int info[4] = { E_COMP_FRAME_EVENT_IDLE_EXIT, 0, 0, 0 };
      e_comp_frame_event_add(info, ecore_time_get());
   }
   return EINA_TRUE;
}

E_API Eina_Bool
e_comp_canvas_init(int w, int h)
{
   Evas_Object *o;
   Eina_List *screens;
   char *bsize;

   e_comp->w = w;
   e_comp->h = h;
   bsize = getenv("E_COMP_SIZE");
   if (bsize)
     {
        int ew = w, eh = h;

        if (sscanf(bsize, "%dx%d", &ew, &eh) == 2)
          {
             if ((w > 0) && (h > 0))
               e_comp->w = ew, e_comp->h = eh;
          }
     }

   e_comp->evas = ecore_evas_get(e_comp->ee);

   static int inited = 0;
   if (!inited)
     {
        inited = 1;
        ecore_idle_enterer_before_add(_e_comp_canvas_cb_idle_enterer, NULL);
        ecore_idle_exiter_add(_e_comp_canvas_cb_idle_exiter, NULL);
     }
   evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_PRE, _e_comp_canvas_render_track_pre, NULL);
   evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_POST, _e_comp_canvas_render_track_post, NULL);
   evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_FLUSH_PRE, _e_comp_canvas_render_track_flush_pre, NULL);
   evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_FLUSH_POST, _e_comp_canvas_render_track_flush_post, NULL);
   if (e_first_frame)
     evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_POST, _e_comp_canvas_cb_first_frame, NULL);
   o = evas_object_rectangle_add(e_comp->evas);
   e_comp->canvas->resize_object = o;
   evas_object_layer_set(o, E_LAYER_BOTTOM);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, e_comp->w, e_comp->h);
   evas_object_color_set(o, 0, 0, 0, 255);
   evas_object_name_set(o, "comp->canvas->resize_object");
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, (Evas_Object_Event_Cb)_e_comp_canvas_cb_mouse_down, NULL);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, (Evas_Object_Event_Cb)_e_comp_canvas_cb_mouse_up, NULL);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN, (Evas_Object_Event_Cb)_e_comp_canvas_cb_mouse_in, NULL);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, (Evas_Object_Event_Cb)_e_comp_canvas_cb_mouse_wheel, NULL);
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _e_comp_canvas_cb_del, NULL);
   evas_object_show(o);

   ecore_evas_name_class_set(e_comp->ee, "E", "Comp_EE");
   //   ecore_evas_manual_render_set(e_comp->ee, conf->lock_fps);
   ecore_evas_show(e_comp->ee);

   evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_FLUSH_POST, _e_comp_canvas_render_post, NULL);

   e_comp->ee_win = ecore_evas_window_get(e_comp->ee);

   screens = (Eina_List *)e_xinerama_screens_get();
   if (screens)
     {
        E_Screen *scr;
        Eina_List *l;

        EINA_LIST_FOREACH(screens, l, scr)
          {
             E_Zone *zone = e_zone_new(scr->screen, scr->escreen,
                                       scr->x, scr->y, scr->w, scr->h);
             if (scr->id) zone->randr2_id = strdup(scr->id);
             e_desk_window_profile_update(zone);
          }
     }
   else
     e_zone_new(0, 0, 0, 0, e_comp->w, e_comp->h);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_ZONE_MOVE_RESIZE, _e_comp_cb_zone_change, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_ZONE_ADD, _e_comp_cb_zone_change, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_ZONE_DEL, _e_comp_cb_zone_change, NULL);
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_KEY_DOWN, _e_comp_canvas_cb_key_down, NULL);
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_KEY_UP, _e_comp_canvas_cb_key_up, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_SCREENSAVER_ON, _e_comp_cb_screensaver_on, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_SCREENSAVER_OFF, _e_comp_cb_screensaver_off, NULL);

   evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_PRE, _e_comp_canvas_prerender, NULL);
   ecore_evas_resize(e_comp->ee, w, h);
   if ((!after_restart) || (!e_comp_x))
     ecore_evas_pointer_warp(e_comp->ee, e_comp->w / 2, e_comp->h / 2);

#ifdef HAVE_WAYLAND
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     e_comp_wl_notidle();
#endif
   return EINA_TRUE;
}

EINTERN void
e_comp_canvas_clear(void)
{
   evas_event_freeze(e_comp->evas);
   edje_freeze();

   E_FREE_FUNC(e_comp->canvas->fps_fg, evas_object_del);
   E_FREE_FUNC(e_comp->canvas->fps_bg, evas_object_del);
   E_FREE_FUNC(e_comp->canvas->fps_gr, evas_object_del);
   E_FREE_FUNC(e_comp->autoclose.rect, evas_object_del);
   E_FREE_FUNC(e_comp->shape_job, ecore_job_del);
   E_FREE_FUNC(e_comp->pointer, e_object_del);
}

//////////////////////////////////////////////

E_API void
e_comp_canvas_resize(int w, int h)
{
   e_comp->w = w;
   e_comp->h = h;
   ecore_evas_resize(e_comp->ee, w, h);
}

E_API void
e_comp_all_freeze(void)
{
   evas_event_freeze(e_comp->evas);
}

E_API void
e_comp_all_thaw(void)
{
   evas_event_thaw(e_comp->evas);
}

E_API E_Zone *
e_comp_zone_xy_get(Evas_Coord x, Evas_Coord y)
{
   const Eina_List *l;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     if (E_INSIDE(x, y, zone->x, zone->y, zone->w, zone->h)) return zone;
   return NULL;
}

E_API E_Zone *
e_comp_zone_number_get(int num)
{
   Eina_List *l = NULL;
   E_Zone *zone = NULL;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if ((int)zone->num == num) return zone;
     }
   return NULL;
}

E_API E_Zone *
e_comp_zone_id_get(int id)
{
   Eina_List *l = NULL;
   E_Zone *zone = NULL;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if (zone->id == id) return zone;
     }
   return NULL;
}

E_API E_Desk *
e_comp_desk_window_profile_get(const char *profile)
{
   Eina_List *l = NULL;
   E_Zone *zone = NULL;
   int x, y;

   EINA_SAFETY_ON_NULL_RETURN_VAL(profile, NULL);

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        for (x = 0; x < zone->desk_x_count; x++)
          {
             for (y = 0; y < zone->desk_y_count; y++)
               {
                  E_Desk *desk = e_desk_at_xy_get(zone, x, y);
                  if (desk)
                    {
                       if (e_object_is_del(E_OBJECT(desk))) continue;
                       if (!e_util_strcmp(desk->window_profile, profile))
                         return desk;
                    }
               }
          }
     }

   return NULL;
}

E_API void
e_comp_canvas_zone_restarted(E_Zone *zone)
{
   if (edje_object_data_get(zone->over, "restarted"))
     {
        edje_object_signal_emit(zone->over, "e,state,sys,restarted,show", "e");
     }
}

E_API void
e_comp_canvas_zone_update(E_Zone *zone)
{
   Evas_Object *o;

   if (zone->over && zone->base)
     {
        e_theme_edje_object_set(zone->base, "base/theme/comp",
                                "e/comp/screen/base/default");
        edje_object_part_swallow(zone->base, "e.swallow.background",
                                 zone->transition_object ?: zone->bg_object);
        e_theme_edje_object_set(zone->over, "base/theme/comp",
                                "e/comp/screen/overlay/default");
        return;
     }
   E_FREE_FUNC(zone->base, evas_object_del);
   E_FREE_FUNC(zone->over, evas_object_del);
   zone->base = o = edje_object_add(e_comp->evas);
   evas_object_repeat_events_set(o, 1);
   evas_object_name_set(zone->base, "zone->base");
   e_theme_edje_object_set(o, "base/theme/comp", "e/comp/screen/base/default");
   edje_object_part_swallow(zone->base, "e.swallow.background", zone->transition_object ?: zone->bg_object);
   evas_object_move(o, zone->x, zone->y);
   evas_object_resize(o, zone->w, zone->h);
   evas_object_layer_set(o, E_LAYER_BG);
   evas_object_show(o);

   zone->over = o = edje_object_add(e_comp->evas);
   edje_object_signal_callback_add(o, "e,state,screensaver,active", "e", _e_comp_canvas_screensaver_active, NULL);
   evas_object_layer_set(o, E_LAYER_MAX);
   evas_object_raise(o);
   evas_object_name_set(zone->over, "zone->over");
   evas_object_pass_events_set(o, 1);
   e_theme_edje_object_set(o, "base/theme/comp", "e/comp/screen/overlay/default");
   evas_object_move(o, zone->x, zone->y);
   evas_object_resize(o, zone->w, zone->h);
   evas_object_raise(o);
   evas_object_show(o);
   if (starting && after_restart &&
       (edje_object_data_get(zone->over, "restarted")))
     {
        edje_object_signal_emit(zone->over, "e,state,sys,restarted", "e");
        edje_object_message_signal_process(zone->over);
     }
}

E_API void
e_comp_canvas_update(void)
{
   Eina_List *l, *screens, *zones = NULL, *ll;
   E_Zone *zone;
   E_Screen *scr;
   Eina_Bool changed = EINA_FALSE;

   screens = (Eina_List *)e_xinerama_screens_get();

   if (screens)
     {
        zones = e_comp->zones;
        e_comp->zones = NULL;
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        EINA_LIST_FOREACH(screens, l, scr)
          {
             zone = NULL;

             printf("@ match screens %p[%i] = %i %i %ix%i -- %i\n",
                    scr, scr->escreen, scr->x, scr->y, scr->w, scr->h, scr->escreen);
             EINA_LIST_FOREACH(zones, ll, zone)
               {
                  if (zone->id == scr->escreen) break;
                  zone = NULL;
               }
             printf("@ matches existing zone %p\n", zone);
             if (zone)
               {
                  printf("   move resize %i %i %ix%i -> %i %i %ix%i\n",
                         zone->x, zone->y, zone->w, zone->h,
                         scr->x, scr->y, scr->w, scr->h);
                  changed |= e_zone_move_resize(zone, scr->x, scr->y, scr->w, scr->h);
                  if (changed)
                    printf("@@@ FOUND ZONE %i %i [%p]\n", zone->num, zone->id, zone);
                  zones = eina_list_remove(zones, zone);
                  e_comp->zones = eina_list_append(e_comp->zones, zone);
                  zone->num = scr->screen;
                  free(zone->randr2_id);
                  zone->randr2_id = NULL;
                  if (scr->id) zone->randr2_id = strdup(scr->id);
                  e_desk_window_profile_update(zone);
               }
             else
               {
                  zone = e_zone_new(scr->screen, scr->escreen,
                                    scr->x, scr->y, scr->w, scr->h);
                  if (scr->id) zone->randr2_id = strdup(scr->id);
                  e_desk_window_profile_update(zone);
                  printf("@@@ NEW ZONE = %p\n", zone);
                  changed = EINA_TRUE;
               }
             if (changed)
               printf("@@@ SCREENS: %i %i | %i %i %ix%i\n",
                      scr->screen, scr->escreen, scr->x, scr->y, scr->w, scr->h);
          }
        e_comp->zones = eina_list_sort(e_comp->zones, 0, _e_comp_canvas_cb_zone_sort);
        if (zones)
          {
             E_Zone *spare_zone;

             printf("@zones have been deleted....\n");
             changed = EINA_TRUE;
             spare_zone = eina_list_data_get(e_comp->zones);

             EINA_LIST_FREE(zones, zone)
               {
                  E_Client *ec;

                  printf("reassign all clients from deleted zone %p\n", zone);
                  E_CLIENT_FOREACH(ec)
                    {
                       if (ec->zone == zone)
                         {
                            const char *tmp;

                            if (ec->restore_zone_id)
                              tmp = eina_stringshare_add(ec->restore_zone_id);
                            else
                              tmp = NULL;
                            if (spare_zone)
                              {
                                 int dx, dy;
                                 E_Desk *desk;

                                 dx = dy = -1;
                                 if (ec->desk)
                                   {
                                      dx = ec->desk->x;
                                      dy = ec->desk->y;
                                   }
                                 e_client_res_change_geometry_save(ec);
                                 e_client_zone_set(ec, spare_zone);
                                 if ((dx >= 0) && (dy >= 0))
                                   {
                                      desk = e_desk_at_xy_get(spare_zone, dx, dy);
                                      if (desk) e_client_desk_set(ec, desk);
                                   }
                              }
                            else
                              printf("EEEK! should not be here - but no\n"
                                     "spare zones exist to move this\n"
                                     "window to!!! help!\n");
                            eina_stringshare_replace(&(ec->restore_zone_id), tmp);
                            if (tmp) eina_stringshare_del(tmp);
                         }
                    }
                  e_object_del(E_OBJECT(zone));
               }
          }
        if (changed) e_shelf_config_update();
     }
   else
     {
        E_Zone *z;

        z = e_comp_zone_number_get(0);
        if (z)
          {
             changed |= e_zone_move_resize(z, 0, 0, e_comp->w, e_comp->h);
             if (changed) e_shelf_zone_move_resize_handle(z);
          }
     }

   e_comp_clients_rescale();
   if (!changed) return;
   if (!starting)
     {
        e_object_ref(E_OBJECT(e_comp));
        ecore_event_add(E_EVENT_COMPOSITOR_UPDATE, NULL, _e_comp_canvas_event_compositor_resize_free, NULL);
     }

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        E_FREE_FUNC(zone->base, evas_object_del);
        E_FREE_FUNC(zone->over, evas_object_del);
        if (zone->bloff)
          {
             if (!e_comp_config_get()->nofade)
               {
                  if (e_backlight_mode_get(zone) != E_BACKLIGHT_MODE_NORMAL)
                    e_backlight_mode_set(zone, E_BACKLIGHT_MODE_NORMAL);
                  e_backlight_level_set(zone, e_config->backlight.normal, -1.0);
               }
          }
        e_comp_canvas_zone_update(zone);
        if (!starting)
          e_bindings_edge_reset();
     }
   evas_object_resize(e_comp->canvas->resize_object, e_comp->w, e_comp->h);
}

E_API void
e_comp_canvas_fake_layers_init(void)
{
   unsigned int layer;

   /* init layers */
   for (layer = e_comp_canvas_layer_map(E_LAYER_CLIENT_DESKTOP); layer <= e_comp_canvas_layer_map(E_LAYER_CLIENT_PRIO); layer++)
     {
        Evas_Object *o2;

        o2 = e_comp->layers[layer].obj = evas_object_rectangle_add(e_comp->evas);
        evas_object_layer_set(o2, e_comp_canvas_layer_map_to(layer));
        evas_object_name_set(o2, "layer_obj");
     }
}

E_API void
e_comp_canvas_fps_toggle(void)
{
   E_Comp_Config *conf = e_comp_config_get();

   conf->fps_show = !conf->fps_show;
   e_comp_internal_save();
   e_comp_render_queue();
}

E_API E_Layer
e_comp_canvas_layer_map_to(unsigned int layer)
{
   switch (layer)
     {
      case 0: return E_LAYER_BOTTOM;
      case 1: return E_LAYER_BG;
      case 2: return E_LAYER_DESKTOP;
      case 3: return E_LAYER_DESKTOP_TOP;
      case 4: return E_LAYER_CLIENT_DESKTOP;
      case 5: return E_LAYER_CLIENT_BELOW;
      case 6: return E_LAYER_CLIENT_NORMAL;
      case 7: return E_LAYER_CLIENT_ABOVE;
      case 8: return E_LAYER_CLIENT_EDGE;
      case 9: return E_LAYER_CLIENT_FULLSCREEN;
      case 10: return E_LAYER_CLIENT_EDGE_FULLSCREEN;
      case 11: return E_LAYER_CLIENT_POPUP;
      case 12: return E_LAYER_CLIENT_TOP;
      case 13: return E_LAYER_CLIENT_DRAG;
      case 14: return E_LAYER_CLIENT_PRIO;
      case 15: return E_LAYER_POPUP;
      case 16: return E_LAYER_MENU;
      case 17: return E_LAYER_DESKLOCK;
      case 18: return E_LAYER_MAX;
      default: break;
     }
   return -INT_MAX;
}

E_API unsigned int
e_comp_canvas_layer_map(E_Layer layer)
{
   switch (layer)
     {
      case E_LAYER_BOTTOM: return 0;
      case E_LAYER_BG: return 1;
      case E_LAYER_DESKTOP: return 2;
      case E_LAYER_DESKTOP_TOP: return 3;
      case E_LAYER_CLIENT_DESKTOP: return 4;
      case E_LAYER_CLIENT_BELOW: return 5;
      case E_LAYER_CLIENT_NORMAL: return 6;
      case E_LAYER_CLIENT_ABOVE: return 7;
      case E_LAYER_CLIENT_EDGE: return 8;
      case E_LAYER_CLIENT_FULLSCREEN: return 9;
      case E_LAYER_CLIENT_EDGE_FULLSCREEN: return 10;
      case E_LAYER_CLIENT_POPUP: return 11;
      case E_LAYER_CLIENT_TOP: return 12;
      case E_LAYER_CLIENT_DRAG: return 13;
      case E_LAYER_CLIENT_PRIO: return 14;
      case E_LAYER_POPUP: return 15;
      case E_LAYER_MENU: return 16;
      case E_LAYER_DESKLOCK: return 17;
      case E_LAYER_MAX: return 18;
      default: break;
     }
   return 9999;
}

E_API unsigned int
e_comp_canvas_client_layer_map(E_Layer layer)
{
   switch (layer)
     {
      case E_LAYER_CLIENT_DESKTOP: return 0;
      case E_LAYER_CLIENT_BELOW: return 1;
      case E_LAYER_CLIENT_NORMAL: return 2;
      case E_LAYER_CLIENT_ABOVE: return 3;
      case E_LAYER_CLIENT_EDGE: return 4;
      case E_LAYER_CLIENT_FULLSCREEN: return 5;
      case E_LAYER_CLIENT_EDGE_FULLSCREEN: return 6;
      case E_LAYER_CLIENT_POPUP: return 7;
      case E_LAYER_CLIENT_TOP: return 8;
      case E_LAYER_CLIENT_DRAG: return 9;
      case E_LAYER_CLIENT_PRIO: return 10;
      default: break;
     }
   return 9999;
}

E_API E_Layer
e_comp_canvas_client_layer_map_nearest(int layer)
{
#define LAYER_MAP(X) \
   if (layer <= X) return X

   LAYER_MAP(E_LAYER_CLIENT_DESKTOP);
   LAYER_MAP(E_LAYER_CLIENT_BELOW);
   LAYER_MAP(E_LAYER_CLIENT_NORMAL);
   LAYER_MAP(E_LAYER_CLIENT_ABOVE);
   LAYER_MAP(E_LAYER_CLIENT_EDGE);
   LAYER_MAP(E_LAYER_CLIENT_FULLSCREEN);
   LAYER_MAP(E_LAYER_CLIENT_EDGE_FULLSCREEN);
   LAYER_MAP(E_LAYER_CLIENT_POPUP);
   LAYER_MAP(E_LAYER_CLIENT_TOP);
   LAYER_MAP(E_LAYER_CLIENT_DRAG);
   return E_LAYER_CLIENT_PRIO;
}

E_API void
e_comp_canvas_keys_grab(void)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     e_bindings_key_grab(E_BINDING_CONTEXT_ANY, e_comp->root);
}

E_API void
e_comp_canvas_keys_ungrab(void)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     e_bindings_key_ungrab(E_BINDING_CONTEXT_ANY, e_comp->root);
}

E_API void
e_comp_canvas_feed_mouse_up(unsigned int activate_time)
{
   int button_mask, i;

   button_mask = evas_pointer_button_down_mask_get(e_comp->evas);
   for (i = 0; i < 32; i++)
     {
       if ((button_mask & (1 << i)))
         evas_event_feed_mouse_up(e_comp->evas, i + 1, EVAS_BUTTON_NONE, activate_time, NULL);
     }
}

E_API void
e_comp_canvas_notidle(void)
{
#ifdef HAVE_WAYLAND
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL) e_comp_wl_notidle();
#endif
}

E_API Evas_Object *
e_comp_canvas_event_grabber_add(void)
{
   E_Zone *zone;
   Eina_List *l;
   Evas_Object *o;

   o = evas_object_event_grabber_add(e_comp->evas);
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
#define EDGE_ADD(E) \
        if (zone->E) \
          evas_object_smart_member_add(zone->E, o)

        EDGE_ADD(edge.top);
        EDGE_ADD(edge.right);
        EDGE_ADD(edge.bottom);
        EDGE_ADD(edge.left);

        EDGE_ADD(corner.left_top);
        EDGE_ADD(corner.right_top);
        EDGE_ADD(corner.top_left);
        EDGE_ADD(corner.top_right);
        EDGE_ADD(corner.left_bottom);
        EDGE_ADD(corner.right_bottom);
        EDGE_ADD(corner.bottom_left);
        EDGE_ADD(corner.bottom_right);
     }
   return o;
}

EINTERN void
e_comp_canvas_intercept(void)
{
   ecore_event_init();
   E_LIST_HANDLER_APPEND_PRE(handlers, ECORE_EVENT_MOUSE_BUTTON_DOWN, _e_comp_cb_mouse_down, NULL);
   E_LIST_HANDLER_APPEND_PRE(handlers, ECORE_EVENT_MOUSE_BUTTON_UP, _e_comp_cb_mouse_up, NULL);
   E_LIST_HANDLER_APPEND_PRE(handlers, ECORE_EVENT_MOUSE_WHEEL, _e_comp_cb_mouse_wheel, NULL);
#ifdef HAVE_WAYLAND
   E_LIST_HANDLER_APPEND_PRE(handlers, ECORE_EVENT_MOUSE_MOVE, _e_comp_cb_mouse_move, NULL);
#endif
   E_LIST_HANDLER_APPEND_PRE(handlers, ECORE_EVENT_KEY_DOWN, _e_comp_cb_key_down, NULL);
   E_LIST_HANDLER_APPEND_PRE(handlers, ECORE_EVENT_KEY_UP, _e_comp_cb_key_up, NULL);
}
