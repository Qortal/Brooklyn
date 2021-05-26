#include "e.h"
#include "e_mod_main.h"

// XXX: need some way to break a grab for desklock to work in x like wl _cb_lost
// XXX: newly added windows while up come up black
// XXX: ARGB windows when scaled down with zoomap have their base colors messed up (black)


/* local subsystem functions */
typedef struct _E_Winlist_Win E_Winlist_Win;

struct _E_Winlist_Win
{
   Evas_Object  *bg_object;
   Evas_Object  *icon_object;
   Evas_Object  *win_object;
   E_Client     *client;
   unsigned char was_iconified E_BITFIELD;
   unsigned char was_shaded E_BITFIELD;
};

static void      _e_winlist_client_resize_cb(void *data, Evas_Object *obj, void *info);
static void      _e_winlist_size_adjust(void);
static Eina_Bool _e_winlist_client_add(E_Client *ec, E_Zone *zone, E_Desk *desk);
static void      _e_winlist_client_del(E_Client *ec);
static void      _e_winlist_activate_nth(int n);
static void      _e_winlist_activate(void);
static void      _e_winlist_deactivate(void);
static void      _e_winlist_show_active(void);
static Eina_Bool _e_winlist_cb_event_border_add(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_event_border_remove(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_key_down(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_key_up(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_mouse_down(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_mouse_up(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_mouse_wheel(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_mouse_move(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_zone_del(void *data, int type, void *event);
static Eina_Bool _e_winlist_cb_zone_move_resize(void *data, int type, void *event);
static Eina_Bool _e_winlist_scroll_timer(void *data);
static Eina_Bool _e_winlist_animator(void *data);

/* local subsystem globals */
static Evas_Object *_winlist = NULL;
static E_Zone *_winlist_zone = NULL;
static Evas_Object *_bg_object = NULL;
static Evas_Object *_list_object = NULL;
static Evas_Object *_icon_object = NULL;
static Evas_Object *_win_object = NULL;
static Evas_Object *_winlist_bg_object = NULL;
static Evas_Object *_winlist_fg_object = NULL;
static Eina_List *_wins = NULL;
static Eina_List *_win_selected = NULL;
static E_Desk *_last_desk = NULL;
static int _last_pointer_x = 0;
static int _last_pointer_y = 0;
static E_Client *_last_client = NULL;
static int _hold_count = 0;
static int _hold_mod = 0;
static E_Winlist_Activate_Type _activate_type = 0;
static Eina_List *_handlers = NULL;
static Ecore_Window _input_window = 0;
static int _scroll_to = 0;
static double _scroll_align_to = 0.0;
static double _scroll_align = 0.0;
static Ecore_Timer *_scroll_timer = NULL;
static Ecore_Animator *_animator = NULL;
static Eina_Bool _mouse_pressed = EINA_FALSE;

static Eina_Bool
_wmclass_picked(const Eina_List *lst, const char *wmclass)
{
   const Eina_List *l;
   const char *s;

   if (!wmclass) return EINA_FALSE;
   EINA_LIST_FOREACH(lst, l, s)
     {
        if (s == wmclass) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_cb_lost(void *data EINA_UNUSED)
{
   e_winlist_hide();
}

/* externally accessible functions */
int
e_winlist_init(void)
{
   return 1;
}

int
e_winlist_shutdown(void)
{
   e_winlist_hide();
   return 1;
}

static void
_cb_bgfg_signal_hide_done(void *data EINA_UNUSED, Evas_Object *obj, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   evas_object_del(obj);
}

int
e_winlist_show(E_Zone *zone, E_Winlist_Filter filter)
{
   Evas_Object *o;
   Eina_List *l, *ll;
   E_Desk *desk;
   E_Client *ec;
   E_Winlist_Win *ww;
   Eina_List *wmclasses = NULL;
   int ok;

   E_OBJECT_CHECK_RETURN(zone, 0);
   E_OBJECT_TYPE_CHECK_RETURN(zone, E_ZONE_TYPE, 0);

   if (_winlist) return 1;

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        Ecore_X_Window mouse_grab = 0;
        _input_window = ecore_x_window_input_new(e_comp->root, 0, 0, 1, 1);
        ecore_x_window_show(_input_window);
        mouse_grab = _input_window;
//        if (_activate_type == E_WINLIST_ACTIVATE_TYPE_MOUSE) mouse_grab = _input_window;
        if (!e_grabinput_get(mouse_grab, 0, _input_window))
          {
             ecore_x_window_free(_input_window);
             _input_window = 0;
             return 0;
          }
        e_grabinput_lost_cb_set(_cb_lost, NULL);
     }
#endif
   if (e_comp->comp_type != E_PIXMAP_TYPE_X)
     {
        Eina_Bool mouse_grab = EINA_FALSE;
        mouse_grab = EINA_TRUE;
//        if (_activate_type == E_WINLIST_ACTIVATE_TYPE_MOUSE) mouse_grab = EINA_TRUE;
        if (!e_comp_grab_input(mouse_grab, EINA_TRUE)) return 0;
        _input_window = e_comp->ee_win;
     }

   _winlist_zone = zone;
   e_client_move_cancel();
   e_client_resize_cancel();
   e_client_focus_track_freeze();

#ifndef HAVE_WAYLAND_ONLY
   evas_event_feed_mouse_in(e_comp->evas, 0, NULL);
   evas_event_feed_mouse_move(e_comp->evas, -1000000, -1000000, 0, NULL);
#endif

   evas_event_freeze(e_comp->evas);

   o = edje_object_add(e_comp->evas);
   evas_object_layer_set(o, E_LAYER_CLIENT_POPUP);
   evas_object_pass_events_set(o, EINA_TRUE);
   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
     ok = e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/large/bg");
   else
     ok = e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/bg");
   if (ok)
     {
        // nothing need be done here yet
        //   edje_object_signal_callback_add(o, "e,action,show,done", "e", _cb_bgfg_signal_hide_done, NULL);
        edje_object_signal_callback_add(o, "e,action,hide,done", "e", _cb_bgfg_signal_hide_done, NULL);
        edje_object_signal_emit(o, "e,state,visible,on", "e");
        evas_object_geometry_set(o, zone->x, zone->y, zone->w, zone->h);
        evas_object_show(o);
        _winlist_bg_object = o;
     }
   else
     {
        evas_object_del(o);
        _winlist_bg_object = NULL;
     }

   o = edje_object_add(e_comp->evas);
   evas_object_pass_events_set(o, EINA_TRUE);
   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
     {
        if (!e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/large"))
          e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/main");
     }
   else
     e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/main");
   if (edje_object_data_get(o, "noshadow"))
     evas_object_data_set(o, "noshadow", o);
   _winlist = e_comp_object_util_add(o, E_COMP_OBJECT_TYPE_POPUP);
   evas_object_layer_set(_winlist, E_LAYER_CLIENT_POPUP);
   _bg_object = o;

   o = edje_object_add(e_comp->evas);
   evas_object_layer_set(o, E_LAYER_CLIENT_POPUP);
   evas_object_pass_events_set(o, EINA_TRUE);
   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
     ok = e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/large/fg");
   else
     ok = e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/fg");
   if (ok)
     {
        // nothing need be done here yet
        //   edje_object_signal_callback_add(o, "e,action,show,done", "e", _cb_bgfg_signal_hide_done, NULL);
        edje_object_signal_callback_add(o, "e,action,hide,done", "e", _cb_bgfg_signal_hide_done, NULL);
        edje_object_signal_emit(o, "e,state,visible,on", "e");
        evas_object_geometry_set(o, zone->x, zone->y, zone->w, zone->h);
        evas_object_show(o);
        _winlist_fg_object = o;
     }
   else
     {
        evas_object_del(o);
        _winlist_fg_object = NULL;
     }

   o = elm_box_add(e_comp->elm);
   _list_object = o;
   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
     {
     }
   else
     {
        elm_box_homogeneous_set(o, 1);
     }
   e_comp_object_util_del_list_append(_winlist, o);
   edje_object_part_swallow(_bg_object, "e.swallow.list", o);
   edje_object_part_text_set(_bg_object, "e.text.title", _("Select a window"));
   evas_object_show(o);

   _last_client = e_client_focused_get();

   desk = e_desk_current_get(_winlist_zone);
   EINA_LIST_FOREACH(e_client_focus_stack_get(), l, ec)
     {
        Eina_Bool pick;

        // skip if we already have it in winlist
        EINA_LIST_FOREACH(_wins, ll, ww)
          {
             if (e_client_stack_bottom_get(ww->client) ==
                 e_client_stack_bottom_get(ec)) break;
          }
        if (ll) continue;
        switch (filter)
          {
           case E_WINLIST_FILTER_CLASS_WINDOWS:
             if (!_last_client) pick = EINA_FALSE;
             else pick = _last_client->icccm.class == ec->icccm.class;
             break;
           case E_WINLIST_FILTER_CLASSES:
             pick = (!_wmclass_picked(wmclasses, ec->icccm.class));
             if (pick) wmclasses = eina_list_append(wmclasses, ec->icccm.class);
             break;

           default:
             pick = EINA_TRUE;
          }
        if (pick) _e_winlist_client_add(ec, _winlist_zone, desk);
     }
   eina_list_free(wmclasses);

   if (!_wins)
     {
        if (_winlist_bg_object)
          {
             evas_object_del(_winlist_bg_object);
             _winlist_bg_object = NULL;
          }
        if (_winlist_fg_object)
          {
             evas_object_del(_winlist_fg_object);
             _winlist_fg_object = NULL;
          }
        e_winlist_hide();
        evas_event_thaw(e_comp->evas);
        return 1;
     }

   if (e_config->winlist_list_show_other_desk_windows ||
       e_config->winlist_list_show_other_screen_windows)
     _last_desk = e_desk_current_get(_winlist_zone);
   if (e_config->winlist_warp_while_selecting)
     ecore_evas_pointer_xy_get(e_comp->ee,
                            &_last_pointer_x, &_last_pointer_y);

   _e_winlist_activate_nth(0);

   evas_event_thaw(e_comp->evas);
   _e_winlist_size_adjust();

   E_LIST_HANDLER_APPEND(_handlers, E_EVENT_CLIENT_ADD, _e_winlist_cb_event_border_add, NULL);
   E_LIST_HANDLER_APPEND(_handlers, E_EVENT_CLIENT_REMOVE, _e_winlist_cb_event_border_remove, NULL);
   E_LIST_HANDLER_APPEND(_handlers, ECORE_EVENT_KEY_DOWN, _e_winlist_cb_key_down, NULL);
   E_LIST_HANDLER_APPEND(_handlers, ECORE_EVENT_KEY_UP, _e_winlist_cb_key_up, NULL);
   E_LIST_HANDLER_APPEND(_handlers, ECORE_EVENT_MOUSE_BUTTON_DOWN, _e_winlist_cb_mouse_down, NULL);
   E_LIST_HANDLER_APPEND(_handlers, ECORE_EVENT_MOUSE_BUTTON_UP, _e_winlist_cb_mouse_up, NULL);
   E_LIST_HANDLER_APPEND(_handlers, ECORE_EVENT_MOUSE_WHEEL, _e_winlist_cb_mouse_wheel, NULL);
   E_LIST_HANDLER_APPEND(_handlers, ECORE_EVENT_MOUSE_MOVE, _e_winlist_cb_mouse_move, NULL);
   E_LIST_HANDLER_APPEND(_handlers, E_EVENT_ZONE_DEL, _e_winlist_cb_zone_del, NULL);
   E_LIST_HANDLER_APPEND(_handlers, E_EVENT_ZONE_MOVE_RESIZE, _e_winlist_cb_zone_move_resize, NULL);

   evas_object_show(_winlist);
   return 1;
}

void
e_winlist_hide(void)
{
   E_Client *ec = NULL;
   E_Winlist_Win *ww;

   if (!_winlist) return;
   if (_win_selected)
     {
        ww = _win_selected->data;
        ec = ww->client;
     }
   evas_object_hide(_winlist);
   EINA_LIST_FREE(_wins, ww)
     {
        if (ww->client->frame)
          evas_object_smart_callback_del_full(ww->client->frame, "client_resize",
                                              _e_winlist_client_resize_cb, ww);
        if ((!ec) || (ww->client != ec)) e_object_unref(E_OBJECT(ww->client));
        free(ww);
     }
   _win_selected = NULL;
   _icon_object = NULL;
   _win_object = NULL;

   if (_winlist_bg_object)
     edje_object_signal_emit(_winlist_bg_object, "e,state,visible,off", "e");
   evas_object_del(_winlist);
   if (_winlist_fg_object)
     edje_object_signal_emit(_winlist_fg_object, "e,state,visible,off", "e");
   e_client_focus_track_thaw();
   _winlist = NULL;
   _winlist_bg_object = NULL;
   _winlist_fg_object = NULL;
   _winlist_zone = NULL;
   _hold_count = 0;
   _hold_mod = 0;
   _activate_type = 0;

   E_FREE_LIST(_handlers, ecore_event_handler_del);

   E_FREE_FUNC(_scroll_timer, ecore_timer_del);
   E_FREE_FUNC(_animator, ecore_animator_del);

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        if (_input_window)
          {
             e_grabinput_release(_input_window, _input_window);
             ecore_x_window_free(_input_window);
          }
     }
#endif
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL) e_comp_ungrab_input(1, 1);
   _input_window = 0;
   if (ec)
     {
        Eina_Bool set = !ec->lock_focus_out;

        if ((ec->shaded) && (!ec->lock_user_shade))
          e_client_unshade(ec, ec->shade_dir);
        if (e_config->winlist_list_move_after_select)
          {
             e_client_zone_set(ec, e_zone_current_get());
             e_client_desk_set(ec, e_desk_current_get(ec->zone));
          }
        else if (ec->desk)
          {
             if (!ec->sticky) e_desk_show(ec->desk);
          }
        if (!ec->lock_user_stacking)
          {
             evas_object_raise(ec->frame);
             e_client_raise_latest_set(ec);
          }
        if (ec->iconic) e_client_uniconify(ec);
        if (ec->shaded) e_client_unshade(ec, ec->shade_dir);
        if ((e_config->focus_policy != E_FOCUS_CLICK) ||
            (e_config->winlist_warp_at_end) ||
            (e_config->winlist_warp_while_selecting))
          {
             set |= !e_client_pointer_warp_to_center_now(ec);
          }
        if (set)
          {
             evas_object_focus_set(ec->frame, 1);
             e_client_focus_latest_set(ec);
          }
        e_object_unref(E_OBJECT(ec));
     }
}

void
e_winlist_next(void)
{
   if (!_winlist) return;
   if (eina_list_count(_wins) == 1)
     {
        if (!_win_selected)
          {
             _win_selected = _wins;
             _e_winlist_show_active();
             _e_winlist_activate();
          }
        return;
     }
   _e_winlist_deactivate();
   if (!_win_selected) _win_selected = _wins;
   else _win_selected = _win_selected->next;
   if (!_win_selected) _win_selected = _wins;
   _e_winlist_show_active();
   _e_winlist_activate();
}

void
e_winlist_prev(void)
{
   if (!_winlist) return;
   if (eina_list_count(_wins) == 1)
     {
        if (!_win_selected)
          {
             _win_selected = _wins;
             _e_winlist_show_active();
             _e_winlist_activate();
          }
        return;
     }
   _e_winlist_deactivate();
   if (!_win_selected) _win_selected = _wins;
   else _win_selected = _win_selected->prev;
   if (!_win_selected) _win_selected = eina_list_last(_wins);
   _e_winlist_show_active();
   _e_winlist_activate();
}

static int
point_line_dist(int x, int y, int lx1, int ly1, int lx2, int ly2)
{
   int xx, yy, dx, dy;
   int a = x - lx1;
   int b = y - ly1;
   int c = lx2 - lx1;
   int d = ly2 - ly1;

   int dot = (a * c) + (b * d);
   int len_sq = (c * c) + (d * d);
   double dist, param = -1.0;

   // if line is 0 length
   if (len_sq) param = (double)dot / len_sq;

   if (param < 0)
     {
        xx = lx1;
        yy = ly1;
     }
   else if (param > 1)
     {
        xx = lx2;
        yy = ly2;
     }
   else
     {
        xx = lx1 + lround(param * c);
        yy = ly1 + lround(param * d);
     }

   dx = x - xx;
   dy = y - yy;
   dist = sqrt((dx * dx) + (dy * dy));
   return lround(dist);
}

void
e_winlist_direction_large_select(int dir)
{
   E_Winlist_Win *ww;
   Eina_List *l, *closest = NULL;
   Evas_Coord x, y, w, h;
   Evas_Coord sx, sy, sw, sh;
   int lx1, ly1, lx2, ly2;
   int dist, dist2, distmin = -1, dx, dy;

   if (!_win_selected) _win_selected = _wins;
   if (!_win_selected) return;
   ww = _win_selected->data;
   evas_object_geometry_get(ww->bg_object, &sx, &sy, &sw, &sh);
   lx1 = sx + (sw / 2);
   ly1 = sy + (sh / 2);
   switch (dir)
     {
      case 0: /* up */
        lx2 = lx1;
        ly2 = ly1 - 30000;
        break;
      case 1: /* down */
        lx2 = lx1;
        ly2 = ly1 + 30000;
        break;
      case 2: /* left */
        lx2 = lx1 - 30000;
        ly2 = ly1;
        break;
      case 3: /* right */
        lx2 = lx1 + 30000;
        ly2 = ly1;
        break;
      default:
        return;
     }
   EINA_LIST_FOREACH(_wins, l, ww)
     {
        if (l == _win_selected) continue;
        evas_object_geometry_get(ww->bg_object, &x, &y, &w, &h);
        dist = point_line_dist(x + (w / 2), y + (h / 2), lx1, ly1, lx2, ly2);
        dx = (x + (w / 2)) - (sx + (sw / 2));
        dy = (y + (h / 2)) - (sy + (sh / 2));
        dist2 = sqrt((dx * dx) + (dy * dy));
        dist += dist2 * 2;
        switch (dir)
          {
           case 0: /* up */
             if (dy > -1) continue;
             break;
           case 1: /* down */
             if (dy < 1) continue;
             break;
           case 2: /* left */
             if (dx > -1) continue;
             break;
           case 3: /* right */
             if (dx < 1) continue;
             break;
           default:
             break;
          }
        if ((dist < distmin) || (distmin == -1))
          {
             distmin = dist;
             closest = l;
          }
     }
   if (closest)
     {
        _e_winlist_deactivate();
        _win_selected = closest;
        _e_winlist_show_active();
        _e_winlist_activate();
     }
}

void
e_winlist_direction_select(E_Zone *zone, int dir)
{
   E_Client *ec;
   Eina_List *l;
   E_Desk *desk;
   E_Client *ec_orig, *ec_next = NULL;
   int distance = INT_MAX;
   int cx, cy;
   E_Winlist_Win *ww;

   ec_next = NULL;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   ec_orig = e_client_focused_get();
   if (!ec_orig) return;

   cx = ec_orig->x + (ec_orig->w / 2);
   cy = ec_orig->y + (ec_orig->h / 2);

   desk = e_desk_current_get(zone);
   EINA_LIST_FOREACH(e_client_focus_stack_get(), l, ec)
     {
        int a = 0, d = 0;

        if (ec == ec_orig) continue;
        if ((!ec->icccm.accepts_focus) && (!ec->icccm.take_focus)) continue;
        if (ec->netwm.state.skip_taskbar) continue;
        if (ec->user_skip_winlist) continue;
        if (ec->iconic)
          {
             if (!e_config->winlist_list_show_iconified) continue;
             if ((ec->zone != zone) &&
                 (!e_config->winlist_list_show_other_screen_iconified))
               continue;
             if ((ec->desk != desk) &&
                 (!e_config->winlist_list_show_other_desk_iconified)) continue;
          }
        else
          {
             if (ec->sticky)
               {
                  if ((ec->zone != zone) &&
                      (!e_config->winlist_list_show_other_screen_windows))
                    continue;
               }
             else
               {
                  if (ec->desk != desk)
                    {
                       if ((ec->zone) && (ec->zone != zone))
                         {
                            if (!e_config->winlist_list_show_other_screen_windows)
                              continue;
                         }
                       else if (!e_config->winlist_list_show_other_desk_windows)
                         continue;
                    }
               }
          }

        switch (dir)
          {
           case 0: /* up */
             d = point_line_dist(cx, cy,
                                 ec->x,         ec->y + ec->h,
                                 ec->x + ec->w, ec->y + ec->h);
             if (d >= distance) continue;
             d = point_line_dist(cx, cy,
                                 ec->x,         ec->y + (ec->h / 2),
                                 ec->x + ec->w, ec->y + (ec->h / 2));
             if (d >= distance) continue;
             if (cy <= (ec->y + (ec->h / 2))) continue;
             a = abs(cx - (ec->x + (ec->w / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
           case 1: /* down */
             d = point_line_dist(cx, cy,
                                 ec->x,         ec->y,
                                 ec->x + ec->w, ec->y);
             if (d >= distance) continue;
             d = point_line_dist(cx, cy,
                                 ec->x,         ec->y + (ec->h / 2),
                                 ec->x + ec->w, ec->y + (ec->h / 2));
             if (d >= distance) continue;
             if (cy >= (ec->y + (ec->h / 2))) continue;
             a = abs(cx - (ec->x + (ec->w / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
           case 2: /* left */
             d = point_line_dist(cx, cy,
                                 ec->x + ec->w, ec->y,
                                 ec->x + ec->w, ec->y + ec->h);
             if (d >= distance) continue;
             d = point_line_dist(cx, cy,
                                 ec->x + (ec->w / 2), ec->y,
                                 ec->x + (ec->w / 2), ec->y + ec->h);
             if (d >= distance) continue;
             if (cx <= (ec->x + (ec->w / 2))) continue;
             a = abs(cy - (ec->y + (ec->h / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
           case 3: /* right */
             d = point_line_dist(cx, cy,
                                 ec->x, ec->y,
                                 ec->x, ec->y + ec->h);
             if (d >= distance) continue;
             d = point_line_dist(cx, cy,
                                 ec->x + (ec->w / 2), ec->y,
                                 ec->x + (ec->w / 2), ec->y + ec->h);
             if (d >= distance) continue;
             if (cx >= (ec->x + (ec->w / 2))) continue;
             a = abs(cy - (ec->y + (ec->h / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
          }
        ec_next = ec;
        distance = d;
     }

   if (!ec_next) return;
   _e_winlist_deactivate();
   EINA_LIST_FOREACH(_wins, l, ww)
     {
        if (ww->client != ec_next) continue;
        _win_selected = l;
        break;
     }
   _e_winlist_show_active();
   _e_winlist_activate();
}

void
e_winlist_modifiers_set(int mod, E_Winlist_Activate_Type type)
{
   _hold_mod = mod;
   _hold_count = 0;
   _activate_type = type;
   if (type == E_WINLIST_ACTIVATE_TYPE_MOUSE) _hold_count++;
   if (_hold_mod & ECORE_EVENT_MODIFIER_SHIFT) _hold_count++;
   if (_hold_mod & ECORE_EVENT_MODIFIER_CTRL) _hold_count++;
   if (_hold_mod & ECORE_EVENT_MODIFIER_ALT) _hold_count++;
   if (_hold_mod & ECORE_EVENT_MODIFIER_WIN) _hold_count++;
}

/* local subsystem functions */
static void
_e_winlist_size_list_adjust(void)
{
   Evas_Coord mw, mh;
   E_Zone *zone = _winlist_zone;
   int x, y, w, h;

   elm_box_recalculate(_list_object);
   edje_object_part_swallow(_bg_object, "e.swallow.list", _list_object);
   edje_object_size_min_calc(_bg_object, &mw, &mh);
   evas_object_size_hint_min_set(_list_object, -1, -1);
   edje_object_part_swallow(_bg_object, "e.swallow.list", _list_object);

   zone = _winlist_zone;
   w = zone->w * e_config->winlist_list_size;
   if (w < mw) w = mw;
   if (w > zone->w) w = zone->w;

   h = mh;
   if (h > zone->h) h = zone->h;

   x = zone->x + ((zone->w - w) / 2);
   y = zone->y + ((zone->h - h) / 2);
   evas_object_geometry_set(_winlist, x, y, w, h);
}

static int
_e_winlist_large_item_height_set(Evas_Coord h)
{
   E_Winlist_Win *ww;
   Eina_List *l, *boxes, *bl;
   Evas_Coord mw, mh, lw, lh;
   Evas_Object *o, *box;
   int rows = 1;
   int rowlen = 0;

   evas_event_freeze(evas_object_evas_get(_list_object));
   evas_object_geometry_get(_list_object, NULL, NULL, &lw, &lh);
   bl = elm_box_children_get(_list_object);
   EINA_LIST_FOREACH(bl, l, o)
     {
        elm_box_unpack_all(o);
     }
   bl = elm_box_children_get(_list_object);
   if (!bl) return 0;
   EINA_LIST_FOREACH(_wins, l, ww)
     {
        mh = h;
        if (mh > ww->client->h)
          mh = ww->client->h;
        if (ww->client->h > 0)
          mw = (ww->client->w * mh) / ww->client->h;
        else
          mw = mh;
        edje_object_part_unswallow(ww->bg_object, ww->win_object);
        evas_object_size_hint_min_set(ww->win_object, mw, mh);
        evas_object_size_hint_max_set(ww->win_object, mw, mh);
        edje_object_part_swallow(ww->bg_object, "e.swallow.win", ww->win_object);
        edje_object_size_min_calc(ww->bg_object, &mw, &mh);
        evas_object_size_hint_min_set(ww->bg_object, mw, mh);
        rowlen += mw;
        if (rowlen > lw)
          {
             rowlen = 0;
             boxes = elm_box_children_get(bl->data);
             if (!boxes) break;
             // no more boxes to fill? break the loop trying - should not happen
             bl = bl->next;
             if (!bl) break;
             rows++;
          }
        box = bl->data;
        elm_box_pack_end(box, ww->bg_object);
//        evas_smart_objects_calculate(evas_object_evas_get(box));
     }
   evas_event_thaw(evas_object_evas_get(_list_object));
   return rows;
}

static void
_e_winlist_size_large_adjust(void)
{
   Evas_Coord mw = 0, mh = 0;
   E_Zone *zone = _winlist_zone;
   Evas_Coord x, y, w, h, h1, h2, maxw, maxh, prevh;
   Eina_Bool expand = EINA_FALSE;

// for optimizing - time things
//   double t0 = ecore_time_get(), t;

   evas_event_freeze(evas_object_evas_get(_list_object));
   maxw = zone->w * e_config->winlist_large_size;
   maxh = zone->h * e_config->winlist_large_size;
   evas_object_resize(_bg_object, maxw, maxh);
   evas_smart_objects_calculate(evas_object_evas_get(_bg_object));
   evas_object_geometry_get(_list_object, NULL, NULL, &maxw, &maxh);
   if (maxw < 64) maxw = 64;
   if (maxh < 64) maxh = 64;
   // we will bisect sizes using the interval h1 -> h2 until h == h2
   // then switch to expand mode where we go up bu 20% in size until
   // we get too big
   h1 = 0;
   h2 = maxh;
   h = (h1 + h2) / 2;
//   t = ecore_time_get(); printf("WINLIST: start %1.5f\n", t - t0); t0 = t;
//   printf("SZ:\n");
   for (;;)
     {
        prevh = h;
        if (expand)
          {
             // get bigger by a bit
             int newh = ((h * 120) / 100);
             if (newh == h) h = newh + 1;
             else h  = newh;
          }
        // pick midpoint in interval (bisect)
        else h = (h1 + h2) / 2;
//        printf("SZ: %i [%i -> %i] expand=%i\n", h, h1, h2, expand);
        _e_winlist_large_item_height_set(h);
        evas_smart_objects_calculate(evas_object_evas_get(_bg_object));
        evas_object_size_hint_min_get(_list_object, &mw, &mh);
        if (expand)
          {
//             printf("SZ:  exp %ix%i > %ix%i || %i >= %i\n", mw, mh, maxw, maxh, h, maxh);
             if ((mw > maxw) || (mh > maxh) || (h >= maxh))
               {
                  h = prevh;
//                  printf("SZ:    chose %i\n", h);
                  _e_winlist_large_item_height_set(h);
//                  t = ecore_time_get(); printf("WINLIST: grow %1.5f\n", t - t0); t0 = t;
                  break;
               }
          }
        else
          {
//             printf("SZ:  shrink %ix%i > %ix%i\n", mw, mh, maxw, maxh);
             if ((mw > maxw) || (mh > maxh)) h2 = h;
             else h1 = h;
             if ((h2 - h1) <= 1)
               {
//                  printf("SZ:    switch to expand\n");
//                  t = ecore_time_get(); printf("WINLIST: took shrink %1.5f\n", t - t0); t0 = t;
                  expand = EINA_TRUE;
                  h = h1;
               }
          }
     }
//   t = ecore_time_get(); printf("WINLIST: loop done %1.5f\n", t - t0); t0 = t;
   evas_smart_objects_calculate(evas_object_evas_get(_bg_object));
   edje_object_part_swallow(_bg_object, "e.swallow.list", _list_object);
   edje_object_size_min_calc(_bg_object, &mw, &mh);

   w = mw;
   h = mh;
   x = zone->x + ((zone->w - w) / 2);
   y = zone->y + ((zone->h - h) / 2);
   evas_object_geometry_set(_winlist, -1, -1, 1, 1);
   evas_object_geometry_set(_winlist, x, y, w, h);
   evas_event_thaw(evas_object_evas_get(_list_object));
//   t = ecore_time_get(); printf("WINLIST: done %1.5f\n", t - t0); t0 = t;
}

static void
_e_winlist_size_adjust(void)
{
   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE) _e_winlist_size_large_adjust();
   else _e_winlist_size_list_adjust();
}

static void
_e_winlist_client_resize_cb(void *data, Evas_Object *obj EINA_UNUSED, void *info EINA_UNUSED)
{
   E_Winlist_Win *ww = data;

   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
     {
        _e_winlist_size_large_adjust();
     }
   else
     {
        evas_object_size_hint_aspect_set(ww->win_object, EVAS_ASPECT_CONTROL_BOTH,
                                         ww->client->w, ww->client->h);
        edje_object_part_swallow(ww->bg_object, "e.swallow.win", ww->win_object);
     }
}

static Eina_Bool
_e_winlist_client_add(E_Client *ec, E_Zone *zone, E_Desk *desk)
{
   E_Winlist_Win *ww;
   Evas_Coord mw, mh;
   Evas_Object *o;

   if ((!ec->icccm.accepts_focus) && (!ec->icccm.take_focus))
     return EINA_FALSE;
   if (ec->netwm.state.skip_taskbar) return EINA_FALSE;
   if (ec->user_skip_winlist) return EINA_FALSE;
   if (ec->iconic)
     {
        if (!e_config->winlist_list_show_iconified) return EINA_FALSE;
        if ((ec->zone != zone) &&
            (!e_config->winlist_list_show_other_screen_iconified)) return EINA_FALSE;
        if ((ec->desk != desk) &&
            (!e_config->winlist_list_show_other_desk_iconified)) return EINA_FALSE;
     }
   else
     {
        if (ec->sticky)
          {
             if ((ec->zone != zone) &&
                 (!e_config->winlist_list_show_other_screen_windows))
               return EINA_FALSE;
          }
        else
          {
             if (ec->desk != desk)
               {
                  if ((ec->zone) && (ec->zone != zone))
                    {
                       if (!e_config->winlist_list_show_other_screen_windows)
                         return EINA_FALSE;
                       if ((ec->zone) && (ec->desk) &&
                           (ec->desk != e_desk_current_get(ec->zone)) &&
                           (!e_config->winlist_list_show_other_desk_windows))
                         return EINA_FALSE;
                    }
                  else if (!e_config->winlist_list_show_other_desk_windows)
                    return EINA_FALSE;
               }
          }
     }

   ww = E_NEW(E_Winlist_Win, 1);
   if (!ww) return EINA_FALSE;
   ww->client = ec;
   _wins = eina_list_append(_wins, ww);
   o = edje_object_add(e_comp->evas);
   E_FILL(o);
   e_comp_object_util_del_list_append(_winlist, o);
   ww->bg_object = o;
   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
     {
        if (!e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/item_large"))
          e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/item");
     }
   else
     e_theme_edje_object_set(o, "base/theme/winlist", "e/widgets/winlist/item");
   edje_object_part_text_set(o, "e.text.label",
                             e_client_util_name_get
                             (e_client_stack_active_adjust(ww->client)));
   evas_object_show(o);
   if (edje_object_part_exists(ww->bg_object, "e.swallow.icon"))
     {
        o = e_client_icon_add(ec, e_comp->evas);
        ww->icon_object = o;
        e_comp_object_util_del_list_append(_winlist, o);
        edje_object_part_swallow(ww->bg_object, "e.swallow.icon", o);
        evas_object_show(o);
     }
   if (edje_object_part_exists(ww->bg_object, "e.swallow.win") &&
      (!e_config->winlist_list_no_miniatures))
     {
        o = e_comp_object_util_frame_mirror_add(ec->frame);
        ww->win_object = o;
        e_comp_object_util_del_list_append(_winlist, o);
        if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
          {
             mh = e_config->winlist_large_size * zone->h;
             if (mh > ec->h) mh = ec->h;
             if (ec->h > 0) mw = (ec->w * mh) / ec->h;
             else mw = e_config->winlist_large_size;
             evas_object_size_hint_min_set(o, mw, mh);
             evas_object_size_hint_max_set(o, mw, mh);
          }
        else
          evas_object_size_hint_aspect_set(o, EVAS_ASPECT_CONTROL_BOTH,
                                           ec->w, ec->h);
        edje_object_part_swallow(ww->bg_object, "e.swallow.win", o);
        evas_object_show(o);
     }
   if (ec->shaded)
     edje_object_signal_emit(ww->bg_object, "e,state,shaded", "e");
   else if (ec->iconic)
     edje_object_signal_emit(ww->bg_object, "e,state,iconified", "e");
   else if ((ec->desk != desk) &&
            (!((ec->sticky) && (ec->zone == zone))))
     edje_object_signal_emit(ww->bg_object, "e,state,invisible", "e");

   if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
     {
        // a hboz per item
        Evas_Object *o2 = elm_box_add(e_comp->elm);
        elm_box_horizontal_set(o2, EINA_TRUE);
        elm_box_pack_end(_list_object, o2);
        evas_object_show(o2);

        edje_object_size_min_calc(ww->bg_object, &mw, &mh);
        E_WEIGHT(ww->bg_object, 0, 0);
        evas_object_size_hint_min_set(ww->bg_object, mw, mh);
        elm_box_pack_end(o2, ww->bg_object);
     }
   else
     {
        edje_object_size_min_calc(ww->bg_object, &mw, &mh);
        E_WEIGHT(ww->bg_object, 1, 0);
        E_FILL(ww->bg_object);
        evas_object_size_hint_min_set(ww->bg_object, mw, mh);
        evas_object_size_hint_max_set(ww->bg_object, 9999, mh);
        elm_box_pack_end(_list_object, ww->bg_object);
     }
   evas_object_smart_callback_add(ww->client->frame, "client_resize",
                                  _e_winlist_client_resize_cb, ww);
   e_object_ref(E_OBJECT(ww->client));
   return EINA_TRUE;
}

static void
_e_winlist_client_del(E_Client *ec)
{
   E_Winlist_Win *ww;
   Eina_List *l;

   if (ec == _last_client) _last_client = NULL;
   EINA_LIST_FOREACH(_wins, l, ww)
     {
        if (ww->client == ec)
          {
             if (ec->frame)
               evas_object_smart_callback_del_full(ec->frame, "client_resize",
                                                   _e_winlist_client_resize_cb, ww);
             e_object_unref(E_OBJECT(ww->client));
             if (l == _win_selected)
               {
                  _win_selected = l->next;
                  if (!_win_selected) _win_selected = l->prev;
                  _e_winlist_show_active();
                  _e_winlist_activate();
               }
             e_comp_object_util_del_list_remove(_winlist, ww->bg_object);
             evas_object_del(ww->bg_object);
             if (ww->icon_object)
               {
                  e_comp_object_util_del_list_remove(_winlist, ww->icon_object);
                  evas_object_del(ww->icon_object);
               }
             if (ww->win_object)
               {
                  e_comp_object_util_del_list_remove(_winlist, ww->win_object);
                  evas_object_del(ww->win_object);
               }
             E_FREE(ww);
             _wins = eina_list_remove_list(_wins, l);
             return;
          }
     }
}

static void
_e_winlist_client_replace(E_Client *ec, E_Client *ec_new)
{
   E_Winlist_Win *ww;
   Eina_List *l;

   EINA_LIST_FOREACH(_wins, l, ww)
     {
        if (ww->client == ec)
          {
             Evas_Coord mw, mh;

             edje_object_part_text_set(ww->bg_object, "e.text.label",
                                       e_client_util_name_get(ec_new));
             edje_object_size_min_calc(ww->bg_object, &mw, &mh);
             if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
               {
               }
             else
               {
                  E_WEIGHT(ww->bg_object, 1, 0);
                  E_FILL(ww->bg_object);
                  evas_object_size_hint_min_set(ww->bg_object, mw, mh);
                  evas_object_size_hint_max_set(ww->bg_object, 9999, mh);
               }
             return;
          }
     }
}

static void
_e_winlist_activate_nth(int n)
{
   Eina_List *l;
   int cnt;

   _e_winlist_deactivate();
   cnt = eina_list_count(_wins);
   if (n >= cnt) n = cnt - 1;
   l = eina_list_nth_list(_wins, n);
   if (!l) return;

   _win_selected = l;
   _e_winlist_show_active();
   _e_winlist_activate();
}

static void
_e_winlist_activate(void)
{
   E_Winlist_Win *ww;
   Evas_Object *o;
   int ok = 0;

   if (!_win_selected) return;
   ww = _win_selected->data;
   edje_object_signal_emit(ww->bg_object, "e,state,selected", "e");
   if (ww->icon_object && e_icon_edje_get(ww->icon_object))
     e_icon_edje_emit(ww->icon_object, "e,state,selected", "e");

   if ((ww->client->iconic) &&
       (e_config->winlist_list_uncover_while_selecting))
     {
        if (!ww->client->lock_user_iconify)
          e_client_uniconify(ww->client);
        ww->was_iconified = 1;
        ok = 1;
     }
   if ((!ww->client->sticky) &&
       (ww->client->desk != e_desk_current_get(_winlist_zone)) &&
       (e_config->winlist_list_jump_desk_while_selecting))
     {
        if (ww->client->desk) e_desk_show(ww->client->desk);
        ok = 1;
     }
   if ((ww->client->shaded || ww->client->shading) &&
       (ww->client->desk == e_desk_current_get(_winlist_zone)) &&
       (e_config->winlist_list_uncover_while_selecting))
     {
        if (!ww->client->lock_user_shade)
          e_client_unshade(ww->client, ww->client->shade_dir);
        ww->was_shaded = 1;
        ok = 1;
     }
   if ((!ww->client->iconic) &&
       ((ww->client->desk == e_desk_current_get(_winlist_zone)) ||
        (ww->client->sticky)))
     ok = 1;
   if (ok)
     {
        int set = 1;
        if (e_config->winlist_warp_while_selecting)
          {
            if (!e_client_pointer_warp_to_center_now(ww->client))
              {
                 evas_object_focus_set(ww->client->frame, 1);
                 set = 0;
              }
          }

        if ((!ww->client->lock_user_stacking) &&
            (e_config->winlist_list_raise_while_selecting))
          evas_object_raise(ww->client->frame);
        if ((!ww->client->lock_focus_out) &&
            (e_config->winlist_list_focus_while_selecting))
          {
             if (set)
               evas_object_focus_set(ww->client->frame, 1);
          }
     }
   edje_object_part_text_set(_bg_object, "e.text.label",
                             e_client_util_name_get(ww->client));
   if (_icon_object)
     {
        e_comp_object_util_del_list_remove(_winlist, _icon_object);
        evas_object_del(_icon_object);
        _icon_object = NULL;
     }
   if (edje_object_part_exists(_bg_object, "e.swallow.icon"))
     {
        o = e_client_icon_add(ww->client, evas_object_evas_get(_winlist));
        _icon_object = o;
        e_comp_object_util_del_list_append(_winlist, o);
        edje_object_part_swallow(_bg_object, "e.swallow.icon", o);
        evas_object_show(o);
     }
   if (_win_object)
     {
        e_comp_object_util_del_list_remove(_winlist, _win_object);
        evas_object_del(_win_object);
        _win_object = NULL;
     }
   if (edje_object_part_exists(_bg_object, "e.swallow.win"))
     {
        o = e_comp_object_util_frame_mirror_add(ww->client->frame);
        _win_object = o;
        e_comp_object_util_del_list_append(_winlist, o);
        evas_object_size_hint_aspect_set(o, EVAS_ASPECT_CONTROL_BOTH,
                                         ww->client->w,
                                         ww->client->h);
        edje_object_part_swallow(_bg_object, "e.swallow.win", o);
        evas_object_show(o);
     }

   edje_object_signal_emit(_bg_object, "e,state,selected", "e");
}

static void
_e_winlist_deactivate(void)
{
   E_Winlist_Win *ww;

   if (!_win_selected) return;
   ww = _win_selected->data;
   if (ww->was_shaded)
     {
        if (!ww->client->lock_user_shade)
          e_client_shade(ww->client, ww->client->shade_dir);
     }
   if (ww->was_iconified)
     {
        if (!ww->client->lock_user_iconify)
          e_client_iconify(ww->client);
     }
   ww->was_shaded = 0;
   ww->was_iconified = 0;
   edje_object_part_text_set(_bg_object, "e.text.label", "");
   edje_object_signal_emit(ww->bg_object, "e,state,unselected", "e");
   if (ww->icon_object && e_icon_edje_get(ww->icon_object))
     e_icon_edje_emit(ww->icon_object, "e,state,unselected", "e");
   if (!ww->client->lock_focus_in)
     evas_object_focus_set(ww->client->frame, 0);
}

static void
_e_winlist_show_active(void)
{
   Eina_List *l;
   int i, n;

   if (!_wins) return;

   for (i = 0, l = _wins; l; l = l->next, i++)
     {
        if (l == _win_selected) break;
     }

   n = eina_list_count(_wins);
   if (n <= 1) return;
   _scroll_align_to = (double)i / (double)(n - 1);
   if (e_config->winlist_scroll_animate)
     {
        _scroll_to = 1;
        if (!_scroll_timer)
          _scroll_timer = ecore_timer_loop_add(0.01, _e_winlist_scroll_timer, NULL);
        if (!_animator)
          _animator = ecore_animator_add(_e_winlist_animator, NULL);
     }
   else
     {
        if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
          {
          }
        else
          {
             _scroll_align = _scroll_align_to;
             elm_box_align_set(_list_object, 0.5, fabs(1.0 - _scroll_align));
          }
     }
}

static void
_e_winlist_restore_desktop(void)
{
   if (_last_desk &&
       (e_config->winlist_list_show_other_desk_windows ||
        e_config->winlist_list_show_other_screen_windows))
     e_desk_show(_last_desk);
   if (e_config->winlist_warp_while_selecting)
     ecore_evas_pointer_warp(e_comp->ee,
                          _last_pointer_x, _last_pointer_y);
   _e_winlist_deactivate();
   _win_selected = NULL;
   e_winlist_hide();
   if (_last_client)
     {
        evas_object_focus_set(_last_client->frame, 1);
        _last_client = NULL;
     }
}

static Eina_Bool
_e_winlist_cb_event_border_add(void *data EINA_UNUSED, int type EINA_UNUSED,
                               void *event)
{
   E_Event_Client *ev = event;
   E_Client *ec = ev->ec;
   Eina_List *l;
   E_Winlist_Win *ww;

   // get base client
   ec = e_client_stack_bottom_get(ec);
   // skip if we already have it in winlist
   EINA_LIST_FOREACH(_wins, l, ww)
     {
        if (ww->client == ec)
          {
             _e_winlist_client_replace(ec, e_client_stack_active_adjust(ec));
             goto done;
          }
     }
   if (_e_winlist_client_add(ec, _winlist_zone,
                             e_desk_current_get(_winlist_zone)))
     _e_winlist_size_adjust();
done:
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_event_border_remove(void *data EINA_UNUSED, int type EINA_UNUSED,
                                  void *event)
{
   E_Event_Client *ev = event;
   E_Client *ec = ev->ec;

   if (!ec->stack.prev) _e_winlist_client_del(ec);
   else _e_winlist_client_replace(ec, e_client_stack_active_adjust(ec));
   _e_winlist_size_adjust();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_key_down(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Key *ev = event;
   E_Util_Action act;
   int x = 0, y = 0;

   if (ev->window != _input_window) return ECORE_CALLBACK_PASS_ON;
   act = e_util_key_geometry_action_get(ev->key, &x, &y, 1, 1);
   if (act == E_UTIL_ACTION_DONE)
     e_winlist_hide();
   else if (act == E_UTIL_ACTION_ABORT)
     _e_winlist_restore_desktop();
   else if (act == E_UTIL_ACTION_DO)
     {
        if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
          {
             if      (y < 0) e_winlist_direction_large_select(0);
             else if (y > 0) e_winlist_direction_large_select(1);
             else if (x < 0) e_winlist_direction_large_select(2);
             else if (x > 0) e_winlist_direction_large_select(3);
          }
        else
          {
             if      (y < 0) e_winlist_direction_select(_winlist_zone, 0);
             else if (y > 0) e_winlist_direction_select(_winlist_zone, 1);
             else if (x < 0) e_winlist_direction_select(_winlist_zone, 2);
             else if (x > 0) e_winlist_direction_select(_winlist_zone, 3);
          }
     }
   else if (!strcmp(ev->key, "1"))
     _e_winlist_activate_nth(0);
   else if (!strcmp(ev->key, "2"))
     _e_winlist_activate_nth(1);
   else if (!strcmp(ev->key, "3"))
     _e_winlist_activate_nth(2);
   else if (!strcmp(ev->key, "4"))
     _e_winlist_activate_nth(3);
   else if (!strcmp(ev->key, "5"))
     _e_winlist_activate_nth(4);
   else if (!strcmp(ev->key, "6"))
     _e_winlist_activate_nth(5);
   else if (!strcmp(ev->key, "7"))
     _e_winlist_activate_nth(6);
   else if (!strcmp(ev->key, "8"))
     _e_winlist_activate_nth(7);
   else if (!strcmp(ev->key, "9"))
     _e_winlist_activate_nth(8);
   else if (!strcmp(ev->key, "0"))
     _e_winlist_activate_nth(9);
   else
     {
        Eina_List *l;
        E_Config_Binding_Key *binding;
        E_Binding_Modifier mod;

        EINA_LIST_FOREACH(e_bindings->key_bindings, l, binding)
          {
             if (binding->action != _winlist_act) continue;

             mod = 0;

             if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
               mod |= E_BINDING_MODIFIER_SHIFT;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)
               mod |= E_BINDING_MODIFIER_CTRL;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_ALT)
               mod |= E_BINDING_MODIFIER_ALT;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_WIN)
               mod |= E_BINDING_MODIFIER_WIN;

             if (binding->key && ((!strcmp(binding->key, ev->key)) || (!strcmp(binding->key, ev->keyname))) &&
                 ((binding->modifiers == mod) || (binding->any_mod)))
               {
                  if (!_act_winlist) continue;
                  if (_act_winlist->func.go_key)
                    _act_winlist->func.go_key(E_OBJECT(_winlist_zone), binding->params, ev);
                  else if (_act_winlist->func.go)
                    _act_winlist->func.go(E_OBJECT(_winlist_zone), binding->params);
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_key_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Key *ev;
   Eina_List *l;
   E_Config_Binding_Key *binding;
   E_Binding_Modifier mod;

   ev = event;
   if (ev->window != _input_window) return ECORE_CALLBACK_PASS_ON;
   if (!_winlist) return ECORE_CALLBACK_PASS_ON;
   if ((_hold_mod) && (_hold_count > 0))
     {
#define KEY_CHECK(MOD, NAME) \
        if ((_hold_mod & ECORE_EVENT_MODIFIER_##MOD) && (!strcmp(ev->key, NAME))) \
          _hold_count--, _hold_mod &= ~ECORE_EVENT_MODIFIER_##MOD
        KEY_CHECK(SHIFT, "Shift_L");
        else KEY_CHECK(SHIFT, "Shift_R");
        else KEY_CHECK(CTRL, "Control_L");
        else KEY_CHECK(CTRL, "Control_R");
        else KEY_CHECK(ALT, "Alt_L");
        else KEY_CHECK(ALT, "Alt_R");
        else KEY_CHECK(ALT, "Meta_L");
        else KEY_CHECK(ALT, "Meta_R");
        else KEY_CHECK(WIN, "Meta_L");
        else KEY_CHECK(WIN, "Meta_R");
        else KEY_CHECK(ALT, "Super_L");
        else KEY_CHECK(ALT, "Super_R");
        else KEY_CHECK(WIN, "Super_L");
        else KEY_CHECK(WIN, "Super_R");
        else KEY_CHECK(WIN, "Mode_switch");

        if ((_hold_count <= 0) || ((!_hold_mod) && (_activate_type == E_WINLIST_ACTIVATE_TYPE_KEY)))
          {
             e_winlist_hide();
             return 1;
          }
     }

   EINA_LIST_FOREACH(e_bindings->key_bindings, l, binding)
     {
        if (binding->action != _winlist_act) continue;
        mod = 0;

        if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
          mod |= E_BINDING_MODIFIER_SHIFT;
        if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)
          mod |= E_BINDING_MODIFIER_CTRL;
        if (ev->modifiers & ECORE_EVENT_MODIFIER_ALT)
          mod |= E_BINDING_MODIFIER_ALT;
        if (ev->modifiers & ECORE_EVENT_MODIFIER_WIN)
          mod |= E_BINDING_MODIFIER_WIN;

        if (binding->key && (!strcmp(binding->key, ev->key)) &&
            ((binding->modifiers == mod) || (binding->any_mod)))
          {
             if (!_act_winlist) continue;
             if (_act_winlist->func.end_key)
               _act_winlist->func.end_key(E_OBJECT(_winlist_zone), binding->params, ev);
             else if (_act_winlist->func.end)
               _act_winlist->func.end(E_OBJECT(_winlist_zone), binding->params);
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_List *
_e_winlist_win_at_xy_get(Evas_Coord x, Evas_Coord y)
{
   E_Winlist_Win *ww;
   Eina_List *l;
   Evas_Coord ox, oy, ow, oh;

   EINA_LIST_FOREACH(_wins, l, ww)
     {
        evas_object_geometry_get(ww->bg_object, &ox, &oy, &ow, &oh);
        if ((x >= ox) && (x < (ox + ow)) && (y >= oy) && (y < (oy + oh)))
          return l;
     }
   return NULL;
}

static Eina_Bool
_e_winlist_cb_mouse_down(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Button *ev;

   ev = event;
   if (ev->window != _input_window) return ECORE_CALLBACK_PASS_ON;
   e_bindings_mouse_down_ecore_event_handle(E_BINDING_CONTEXT_WINLIST,
                                      E_OBJECT(_winlist_zone), ev);
   if (ev->buttons == 1)
     {
        Eina_List *l = _e_winlist_win_at_xy_get(ev->x, ev->y);
        _mouse_pressed = EINA_TRUE;
        if (l)
          {
             _e_winlist_deactivate();
             _win_selected = l;
             _e_winlist_show_active();
             _e_winlist_activate();
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_mouse_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Button *ev;
   Eina_Bool was_pressed = EINA_FALSE;

   ev = event;
   if (ev->window != _input_window) return ECORE_CALLBACK_PASS_ON;
   if (e_bindings_mouse_up_ecore_event_handle(E_BINDING_CONTEXT_WINLIST, E_OBJECT(_winlist_zone), ev))
     return ECORE_CALLBACK_RENEW;
   if (ev->buttons == 1)
     {
        if (_mouse_pressed) was_pressed = EINA_TRUE;
        _mouse_pressed = EINA_FALSE;
        if ((_win_selected) && (was_pressed))
          {
             _e_winlist_deactivate();
             e_winlist_hide();
             _hold_count = 0;
             return ECORE_CALLBACK_PASS_ON;
          }
     }
   if (_activate_type != E_WINLIST_ACTIVATE_TYPE_MOUSE) return ECORE_CALLBACK_RENEW;
   if ((!--_hold_count) && (was_pressed)) e_winlist_hide();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_mouse_wheel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Wheel *ev;
   int i;

   ev = event;
   if (ev->window != _input_window) return ECORE_CALLBACK_PASS_ON;
   e_bindings_wheel_ecore_event_handle(E_BINDING_CONTEXT_WINLIST,
                                 E_OBJECT(_winlist_zone), ev);
   if (ev->z < 0) /* up */
     {
        for (i = ev->z; i < 0; i++)
          {
             e_winlist_prev();
          }
     }
   else if (ev->z > 0) /* down */
     {
        for (i = ev->z; i > 0; i--)
          {
             e_winlist_next();
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_mouse_move(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Move *ev;
   int x, y, w, h;

   ev = event;
   if (ev->window != _input_window) return ECORE_CALLBACK_PASS_ON;
   evas_object_geometry_get(_winlist, &x, &y, &w, &h);
   /* only feed mouse move if it's within the winlist popup */
   if (E_INSIDE(ev->x, ev->y, x, y, w, h))
     evas_event_feed_mouse_move(evas_object_evas_get(_winlist), ev->x, ev->y, ev->timestamp, NULL);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_zone_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Zone_Del *ev = event;

   if (ev->zone != _winlist_zone) return ECORE_CALLBACK_PASS_ON;
   _e_winlist_deactivate();
   e_winlist_hide();
   _hold_count = 0;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_cb_zone_move_resize(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Zone_Move_Resize *ev = event;
   Evas_Coord x, y, w, h;

   if (ev->zone != _winlist_zone) return ECORE_CALLBACK_PASS_ON;
   evas_object_geometry_set(_winlist_bg_object, ev->zone->x, ev->zone->y, ev->zone->w, ev->zone->h);
   evas_object_geometry_get(_winlist, NULL, NULL, &w, &h);
   x = ev->zone->x + ((ev->zone->w - w) / 2);
   y = ev->zone->y + ((ev->zone->h - h) / 2);
   evas_object_geometry_set(_winlist, x, y, w, h);
   evas_object_geometry_set(_winlist_fg_object, ev->zone->x, ev->zone->y, ev->zone->w, ev->zone->h);
   _e_winlist_deactivate();
   e_winlist_hide();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_winlist_scroll_timer(void *data EINA_UNUSED)
{
   if (_scroll_to)
     {
        double spd;

        spd = e_config->winlist_scroll_speed;
        _scroll_align = (_scroll_align * (1.0 - spd)) +
          (_scroll_align_to * spd);
        return 1;
     }
   _scroll_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_e_winlist_animator(void *data EINA_UNUSED)
{
   if (_scroll_to)
     {
        double da;

        da = _scroll_align - _scroll_align_to;
        if (da < 0.0) da = -da;
        if (da < 0.01)
          {
             _scroll_align = _scroll_align_to;
             _scroll_to = 0;
          }
        if (e_config->winlist_mode == E_WINLIST_MODE_LARGE)
          {
          }
        else
          elm_box_align_set(_list_object, 0.5, fabs(1.0 - _scroll_align));
     }
   if (!_scroll_to) _animator = NULL;
   return _scroll_to;
}
