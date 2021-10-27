#define E_COMP_WL
#include "e.h"

/* handle include for printing uint64_t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define COMPOSITOR_VERSION 4

E_API int E_EVENT_WAYLAND_GLOBAL_ADD = -1;

#ifndef EGL_HEIGHT
# define EGL_HEIGHT			0x3056
#endif
#ifndef EGL_WIDTH
# define EGL_WIDTH			0x3057
#endif

/* Resource Data Mapping: (wl_resource_get_user_data)
 *
 * wl_surface == e_pixmap
 * wl_region == eina_tiler
 * wl_subsurface == e_client
 *
 */

static void _e_comp_wl_subsurface_parent_commit(E_Client *ec, Eina_Bool parent_synchronized);

EINTERN Eina_Bool e_comp_wl_extensions_init(void);

/* local variables */
/* static Eina_Hash *clients_win_hash = NULL; */
static Eina_List *handlers = NULL;
static double _last_event_time = 0.0;

static int64_t surface_id = 0;

static Eina_List *grab_clients;
static Eina_List *grab_cbs;

/* local functions */

static Eina_Bool
_parent_client_contains_pointer(E_Client *ec)
{
   Eina_List *l;
   E_Client *c, *top = ec;

   while (top->parent) top = top->parent;

   if (top == ec) return EINA_FALSE;

   if (top->mouse.in) return EINA_TRUE;

   EINA_LIST_FOREACH(top->comp_data->sub.list, l, c)
     if (c->mouse.in) return EINA_TRUE;

   EINA_LIST_FOREACH(top->transients, l, c)
     if ((ec != c) && c->mouse.in) return EINA_TRUE;

   return EINA_FALSE;
}

static struct wl_resource *
_output_resource_find(Eina_List *reslist, struct wl_resource *surface)
{
   Eina_List *l;
   struct wl_client *client;
   struct wl_resource *res;

   client = wl_resource_get_client(surface);
   EINA_LIST_FOREACH(reslist, l, res)
     if (wl_resource_get_client(res) == client) return res;

   return NULL;
}

static void
_e_comp_wl_surface_outputs_update(E_Client *ec)
{
   Eina_List *l;
   E_Zone *zone;
   int32_t obits = 0;

   if (ec->visible)
     EINA_LIST_FOREACH(e_comp->zones, l, zone)
       if (E_INTERSECTS(zone->x, zone->y, zone->w, zone->h,
                        ec->x, ec->y, ec->w, ec->h)) obits |= 1 << zone->id;

   if (obits != ec->comp_data->on_outputs)
     {
        int32_t leave = (obits ^ ec->comp_data->on_outputs) & ec->comp_data->on_outputs;
        int32_t enter = (obits ^ ec->comp_data->on_outputs) & obits;

        ec->comp_data->on_outputs = obits;
        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          {
             struct wl_resource *s, *res;
             E_Comp_Wl_Output *wlo;

             s = ec->comp_data->surface;
             wlo = zone->output;
             if (!wlo) continue;

             res = _output_resource_find(wlo->resources, s);
             if (!res) continue;

             if (leave & (1 << zone->id)) wl_surface_send_leave(s, res);
             if (enter & (1 << zone->id)) wl_surface_send_enter(s, res);
          }
     }
}

static void
_e_comp_wl_configure_send(E_Client *ec, Eina_Bool edges)
{
   int w = 0, h = 0;

   if (e_object_is_del(E_OBJECT(ec))) return;
   if (ec->changes.size || e_pixmap_usable_get(ec->pixmap))
     {
        if (e_comp_object_frame_exists(ec->frame))
          w = ec->client.w, h = ec->client.h;
        else
          w = ec->w, h = ec->h;
     }
   ec->comp_data->shell.configure_send(ec->comp_data->shell.surface,
                                       edges * e_comp_wl->resize.edges,
                                       w, h);
}

/* static void */
/* _e_comp_wl_log_cb_print(const char *format, va_list args) */
/* { */
/*    EINA_LOG_DOM_INFO(e_log_dom, format, args); */
/* } */

static void
_e_comp_wl_modules_load(void)
{
   const char **m, *mods[] =
   {
#ifdef USE_MODULE_WL_DESKTOP_SHELL
      "wl_desktop_shell",
#endif
#ifdef USE_MODULE_XWAYLAND
      "xwayland",
#endif
      NULL
   };

   for (m = mods; *m; m++)
     {
        E_Module *mod = e_module_find(*m);

        if (!mod)
          mod = e_module_new(*m);

        if (mod)
          e_module_enable(mod);
     }
}

static void
_e_comp_wl_evas_cb_show(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec = data;

   if (e_object_is_del(E_OBJECT(ec))) return;

   if (!ec->override) e_hints_window_visible_set(ec);
}

static void
_e_comp_wl_evas_cb_hide(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec = data;

   if (!e_object_is_del(E_OBJECT(ec))) return;
   if (!e_object_ref_get(E_OBJECT(ec))) return;

   e_comp_object_damage(ec->frame, 0, 0, ec->w, ec->h);
   e_comp_object_dirty(ec->frame);
   if (e_comp_object_render(ec->frame))
     e_comp_client_post_update_add(ec);
   e_object_unref(E_OBJECT(ec));
}

static void
_e_comp_wl_mouse_in(E_Client *ec, Evas_Event_Mouse_In *ev)
{
   struct wl_resource *res;
   struct wl_client *wc;
   Eina_List *l;
   uint32_t serial;

   if (e_object_is_del(E_OBJECT(ec))) return;

   if (!ec->comp_data->surface) return;

   e_comp_wl->ptr.ec = ec;
   if (e_comp_wl->drag)
     {
        e_comp_wl_data_device_send_enter(ec);
        return;
     }
   if (!eina_list_count(e_comp_wl->ptr.resources)) return;
   wc = wl_resource_get_client(ec->comp_data->surface);
   serial = wl_display_next_serial(e_comp_wl->wl.disp);
   EINA_LIST_FOREACH(e_comp_wl->ptr.resources, l, res)
     {
        E_Comp_Wl_Pointer *ptr = wl_resource_get_user_data(res);

        if (!e_comp_wl_input_pointer_check(res)) continue;
        if (wl_resource_get_client(res) != wc) continue;
        ptr->entered = 1;
        wl_pointer_send_enter(res, serial, ec->comp_data->surface,
                              wl_fixed_from_int(ev->canvas.x - ec->client.x),
                              wl_fixed_from_int(ev->canvas.y - ec->client.y));
        e_comp_wl_input_pointer_cursor_update(ptr);
     }
}

static void
_e_comp_wl_evas_cb_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   if (e_comp_object_frame_exists(obj)) return;
   _e_comp_wl_mouse_in(data, event_info);
}

static void
_e_comp_wl_cb_ssd_mouse_in(void *data, Evas_Object *obj, void *event_info)
{
   E_Client *ec = data;

   if (!e_comp_object_frame_exists(obj)) return;
   if (!e_object_is_del(data))
     ec->comp_data->ssd_mouse_in = 1;
   _e_comp_wl_mouse_in(data, event_info);
}

static void
_e_comp_wl_mouse_out(E_Client *ec)
{
   struct wl_resource *res;
   struct wl_client *wc;
   Eina_List *l;
   uint32_t serial;

   if ((ec == e_client_action_get()) && e_grabinput_mouse_win_get()) return;
   /* FIXME? this is a hack to just reset the cursor whenever we mouse out. not sure if accurate */
   {
      e_pointer_object_set(e_comp->pointer, NULL, 0, 0);
      evas_object_show(e_comp->pointer->o_ptr);
   }
   if (e_comp_wl->ptr.ec == ec)
     e_comp_wl->ptr.ec = NULL;
   if (!ec->comp_data) return;

   if (!ec->comp_data->surface) return;
   if (e_comp_wl->drag)
     {
        e_comp_wl_data_device_send_leave(ec);
        return;
     }
   if (!eina_list_count(e_comp_wl->ptr.resources)) return;

   wc = wl_resource_get_client(ec->comp_data->surface);
   serial = wl_display_next_serial(e_comp_wl->wl.disp);
   EINA_LIST_FOREACH(e_comp_wl->ptr.resources, l, res)
     {
        E_Comp_Wl_Pointer *ptr = wl_resource_get_user_data(res);
        if (!e_comp_wl_input_pointer_check(res)) continue;
        if (wl_resource_get_client(res) != wc) continue;
        ptr->entered = 0;
        wl_pointer_send_leave(res, serial, ec->comp_data->surface);
     }
}

static void
_e_comp_wl_evas_cb_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (e_comp_object_frame_exists(obj)) return;
   _e_comp_wl_mouse_out(data);
}

static void
_e_comp_wl_cb_ssd_mouse_out(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *ec = data;

   if (!e_comp_object_frame_exists(obj)) return;
   if (!e_object_is_del(data))
     ec->comp_data->ssd_mouse_in = 0;
   _e_comp_wl_mouse_out(data);
}

static void
_e_comp_wl_send_mouse_move(E_Client *ec, int x, int y, unsigned int timestamp)
{
   struct wl_resource *res;
   struct wl_client *wc;
   Eina_List *l;
   E_Client *tec, *tec_in = NULL;
   Eina_Bool found = EINA_FALSE;
   Evas_Event_Mouse_In ev;

   /* begin hacks:
    *
    * this should be handled by setting NOGRAB pointer mode on comp object internals,
    * ie. 03a4ecbdb0972f8267c07ea14d752fcee69bd9fa
    * but doing so breaks dnd eventing.
    *
    * instead, walk the transients list, assuming (correctly) that they are listed from
    * bottom to top, and send mouse events to whichever surface the pointer is in
    */
   ev.canvas.x = x;
   ev.canvas.y = y;
   EINA_LIST_FOREACH(ec->transients, l, tec)
     {
        if (e_client_util_ignored_get(tec) || (!evas_object_visible_get(tec->frame)))
          continue;
        if (tec->mouse.in) tec_in = tec;
        if (E_INSIDE(x, y, tec->x, tec->y, tec->w, tec->h))
          {
             if (tec_in && (tec != tec_in))
               {
                  e_client_mouse_out(tec_in, x, y);
                  _e_comp_wl_mouse_out(tec_in);
                  tec_in = NULL;
               }
             e_client_mouse_in(tec, x, y);
             _e_comp_wl_mouse_in(tec, &ev);
             e_client_mouse_move(tec, &(Evas_Point){x, y});
             _e_comp_wl_send_mouse_move(tec, x, y, timestamp);
             found = EINA_TRUE;
          }
        else if (tec->mouse.in)
          {
             e_client_mouse_out(tec, x, y);
             _e_comp_wl_mouse_out(tec);
          }
     }
   if (found) return;
   /* if a transient previously had mouse.in, re-set mouse.in to the parent */
   if (tec_in)
     {
        e_client_mouse_in(ec, x, y);
        _e_comp_wl_mouse_in(ec, &ev);
        e_client_mouse_move(ec, &(Evas_Point){x, y});
     }
   /* end hacks */
   wc = wl_resource_get_client(ec->comp_data->surface);
   EINA_LIST_FOREACH(e_comp_wl->ptr.resources, l, res)
     {
        if (!e_comp_wl_input_pointer_check(res)) continue;
        if (wl_resource_get_client(res) != wc) continue;
        wl_pointer_send_motion(res, timestamp,
                               wl_fixed_from_int(x - ec->client.x),
                               wl_fixed_from_int(y - ec->client.y));
     }
}

static Eina_Bool
_e_comp_wl_evas_cb_mouse_internal_check(E_Client *ec, int x, int y)
{
   if (!e_comp_object_frame_exists(ec->frame)) return EINA_TRUE;
   if (E_INSIDE(x, y, ec->client.x, ec->client.y, ec->client.w, ec->client.h)) return EINA_TRUE;
   return EINA_FALSE;
}

static void
_e_comp_wl_evas_cb_mouse_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec = data;
   Evas_Event_Mouse_Move *ev = event;

   if (ec == e_client_action_get()) return;
   if (!ec->mouse.in) return;
   if (!_e_comp_wl_evas_cb_mouse_internal_check(ec, ev->cur.canvas.x, ev->cur.canvas.y))
     {
        e_client_mouse_move(ec, &(Evas_Point){ev->cur.canvas.x, ev->cur.canvas.y});
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (ec->ignored) return;
   if (!ec->comp_data->surface) return;

   if ((!e_comp_wl->drag_client) ||
       (!e_client_has_xwindow(e_comp_wl->drag_client)) ||
       e_comp_wl->drag_client->override)
     _e_comp_wl_send_mouse_move(ec, ev->cur.canvas.x, ev->cur.canvas.y, ev->timestamp);
}

static void
_e_comp_wl_evas_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec = data;
   Evas_Event_Mouse_Down *ev = event;

   if (!_e_comp_wl_evas_cb_mouse_internal_check(ec, ev->output.x, ev->output.y)) return;
   e_comp_wl_evas_handle_mouse_button(ec, ev->timestamp, ev->button,
                                      WL_POINTER_BUTTON_STATE_PRESSED);
}

static void
_e_comp_wl_evas_cb_mouse_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec = data;
   Evas_Event_Mouse_Up *ev = event;

   if (!_e_comp_wl_evas_cb_mouse_internal_check(ec, ev->output.x, ev->output.y)) return;
   e_comp_wl_evas_handle_mouse_button(ec, ev->timestamp, ev->button,
                                      WL_POINTER_BUTTON_STATE_RELEASED);
}

static void
_e_comp_wl_evas_cb_mouse_wheel(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Client *ec;
   Evas_Event_Mouse_Wheel *ev;
   struct wl_resource *res;
   struct wl_client *wc;
   Eina_List *l;
   uint32_t axis, dir;

   ev = event;
   if (!(ec = data)) return;
   if (ec == e_client_action_get()) return;
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (ec->ignored) return;
   if (!ec->mouse.in) return;

   if (ev->direction == 0)
     axis = WL_POINTER_AXIS_VERTICAL_SCROLL;
   else
     axis = WL_POINTER_AXIS_HORIZONTAL_SCROLL;

   if (ev->z < 0)
     dir = -wl_fixed_from_int(abs(10 * ev->z));
   else
     dir = wl_fixed_from_int(10 * ev->z);

   if (!ec->comp_data->surface) return;

   wc = wl_resource_get_client(ec->comp_data->surface);
   EINA_LIST_FOREACH(e_comp_wl->ptr.resources, l, res)
     {
        if (!e_comp_wl_input_pointer_check(res)) continue;
        if (wl_resource_get_client(res) != wc) continue;
        wl_pointer_send_axis(res, ev->timestamp, axis, dir);
     }
}

static void
_e_comp_wl_evas_cb_multi_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Eina_List *l;
   struct wl_client *wc;
   uint32_t serial;
   struct wl_resource *res;
   E_Client *ec = data;
   Evas_Event_Multi_Down *ev = event;
   wl_fixed_t x, y;

   if (e_object_is_del(E_OBJECT(ec))) return;
   if (!ec->comp_data->surface) return;
   if (!ec->mouse.in) return;

   wc = wl_resource_get_client(ec->comp_data->surface);
   serial = wl_display_next_serial(e_comp_wl->wl.disp);

   x = wl_fixed_from_int(ev->canvas.x - ec->client.x);
   y = wl_fixed_from_int(ev->canvas.y - ec->client.y);

   EINA_LIST_FOREACH(e_comp_wl->touch.resources, l, res)
     {
        if (wl_resource_get_client(res) != wc) continue;
        if (!e_comp_wl_input_touch_check(res)) continue;
        wl_touch_send_down(res, serial, ev->timestamp,
                           ec->comp_data->surface, ev->device, x, y);
     }
}

static void
_e_comp_wl_evas_cb_multi_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Eina_List *l;
   struct wl_client *wc;
   uint32_t serial;
   struct wl_resource *res;
   E_Client *ec = data;
   Evas_Event_Multi_Up *ev = event;

   if (e_object_is_del(E_OBJECT(ec))) return;
   if (!ec->comp_data->surface) return;
   if (!ec->mouse.in) return;

   wc = wl_resource_get_client(ec->comp_data->surface);
   serial = wl_display_next_serial(e_comp_wl->wl.disp);

   EINA_LIST_FOREACH(e_comp_wl->touch.resources, l, res)
     {
        if (wl_resource_get_client(res) != wc) continue;
        if (!e_comp_wl_input_touch_check(res)) continue;
        wl_touch_send_up(res, serial, ev->timestamp, ev->device);
     }
}

static void
_e_comp_wl_evas_cb_multi_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Eina_List *l;
   struct wl_client *wc;
   struct wl_resource *res;
   E_Client *ec = data;
   Evas_Event_Multi_Move *ev = event;
   wl_fixed_t x, y;

   if (e_object_is_del(E_OBJECT(ec))) return;
   if (!ec->comp_data->surface) return;
   if (!ec->mouse.in) return;

   wc = wl_resource_get_client(ec->comp_data->surface);

   x = wl_fixed_from_int(ev->cur.canvas.x - ec->client.x);
   y = wl_fixed_from_int(ev->cur.canvas.y - ec->client.y);

   EINA_LIST_FOREACH(e_comp_wl->touch.resources, l, res)
     {
        if (wl_resource_get_client(res) != wc) continue;
        if (!e_comp_wl_input_touch_check(res)) continue;
        wl_touch_send_motion(res, ev->timestamp, ev->device, x, y);
     }
}

static void
_e_comp_wl_client_priority_adjust(int pid, int set, int adj, Eina_Bool use_adj, Eina_Bool adj_child, Eina_Bool do_child)
{
   Eina_List *files;
   char *file, buff[PATH_MAX];
   FILE *f;
   int pid2, ppid;
   int num_read;
   int n;

   if (use_adj)
     n = (getpriority(PRIO_PROCESS, pid) + adj);
   else
     n = set;

   setpriority(PRIO_PROCESS, pid, n);

   if (adj_child)
     use_adj = EINA_TRUE;

   if (!do_child) return;

   files = ecore_file_ls("/proc");
   EINA_LIST_FREE(files, file)
      {
         if (!isdigit(file[0]))
           continue;

         snprintf(buff, sizeof(buff), "/proc/%s/stat", file);
         if ((f = fopen(buff, "r")))
           {
              pid2 = -1;
              ppid = -1;
              num_read = fscanf(f, "%i %*s %*s %i %*s", &pid2, &ppid);
              fclose(f);
              if (num_read == 2 && ppid == pid)
                _e_comp_wl_client_priority_adjust(pid2, set,
                                                  adj, use_adj,
                                                  adj_child, do_child);
           }

         free(file);
      }
}

static void
_e_comp_wl_client_priority_raise(E_Client *ec)
{
   if (ec->netwm.pid <= 0) return;
   if (ec->netwm.pid == getpid()) return;
   _e_comp_wl_client_priority_adjust(ec->netwm.pid,
                                     e_config->priority - 1, -1,
                                     EINA_FALSE, EINA_TRUE, EINA_FALSE);
}

static void
_e_comp_wl_client_priority_normal(E_Client *ec)
{
   if (ec->netwm.pid <= 0) return;
   if (ec->netwm.pid == getpid()) return;
   _e_comp_wl_client_priority_adjust(ec->netwm.pid, e_config->priority, 1,
                                     EINA_FALSE, EINA_TRUE, EINA_FALSE);
}

static Eina_Bool
_e_comp_wl_evas_cb_focus_in_timer(E_Client *ec)
{
   if (e_object_is_del(E_OBJECT(ec))) return EINA_FALSE;

   ec->comp_data->on_focus_timer = NULL;

   if (!e_comp_wl->kbd.focused) return EINA_FALSE;
   e_comp_wl_input_keyboard_modifiers_update();
   return EINA_FALSE;
}

static Eina_Bool
_keyboard_resource_add(struct wl_resource *newres)
{
  Eina_List *l;
  struct wl_resource *res;

  EINA_LIST_FOREACH(e_comp_wl->kbd.focused, l, res)
    if (res == newres) return EINA_FALSE;

  e_comp_wl->kbd.focused = eina_list_append(e_comp_wl->kbd.focused, newres);
  return EINA_TRUE;
}

static void
_e_comp_wl_evas_cb_focus_in(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec, *focused;
   struct wl_resource *res;
   struct wl_client *wc;
   Eina_List *l;
   int added = 0;

   if (!(ec = data)) return;
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (ec->iconic) return;

   /* block spurious focus events */
   focused = e_client_focused_get();
   if ((focused) && (ec != focused)) return;

   /* raise client priority */
   _e_comp_wl_client_priority_raise(ec);

   wc = wl_resource_get_client(ec->comp_data->surface);
   if (ec->comp_data->is_xdg_surface)
     {
        /* We only send kbd focus to xdg top levels */
        while (ec->parent)
          {
             ec = ec->parent;

             /* If an xdg shell popup's parent already has focus we don't
              * need to do anything more.
              */
             if (ec->focused) return;
          }
     }

   EINA_LIST_FOREACH(e_comp_wl->kbd.resources, l, res)
     if (wl_resource_get_client(res) == wc)
       added |= _keyboard_resource_add(res);
   if (!e_comp_wl->kbd.focused) return;
   if (!added) return;
   e_comp_wl_input_keyboard_enter_send(ec);
   if (e_comp_util_kbd_grabbed()) return;
   e_comp_wl_data_device_keyboard_focus_set();
   ec->comp_data->on_focus_timer =
     ecore_timer_loop_add(0.8, (Ecore_Task_Cb)_e_comp_wl_evas_cb_focus_in_timer, ec);
}

static void
_e_comp_wl_keyboard_leave(E_Client *ec)
{
   struct wl_resource *res;
   Eina_List *l, *ll;
   uint32_t serial;

   if (!eina_list_count(e_comp_wl->kbd.resources)) return;
   if (!ec->comp_data) return;

   if (ec->comp_data->is_xdg_surface)
     {
        /* If we left an xdg popup to enter some other (sub)surface
         * of the same top level, we don't need to do anything.
         * We also test e_client_focused_get() because it will be NULL
         * on moves which trip up _parent_client_contains_pointer() */
        if (e_client_focused_get() &&
            _parent_client_contains_pointer(ec)) return;

        /* We only kbd focus top level xdg */
        while (ec->parent) ec = ec->parent;
     }

   serial = wl_display_next_serial(e_comp_wl->wl.disp);

   EINA_LIST_FOREACH_SAFE(e_comp_wl->kbd.focused, l, ll, res)
     {
        if (ec->comp_data->surface)
          wl_keyboard_send_leave(res, serial, ec->comp_data->surface);
        e_comp_wl->kbd.focused = eina_list_remove_list(e_comp_wl->kbd.focused, l);
     }
   if ((!e_comp_wl->wl.client_ec) || e_object_is_del(E_OBJECT(e_comp_wl->wl.client_ec))) return;
   if (e_client_focused_get()) return;
   e_comp_wl->kbd.focus = e_comp_wl->wl.client_ec->comp_data->surface;
   e_comp_wl_data_device_keyboard_focus_set();
}

static void
_e_comp_wl_evas_cb_focus_out(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec = data;

   /* lower client priority */
   if (!e_object_is_del(data))
     {
        _e_comp_wl_client_priority_normal(ec);
        E_FREE_FUNC(ec->comp_data->on_focus_timer, ecore_timer_del);
     }
   _e_comp_wl_keyboard_leave(ec);
}

static void
_e_comp_wl_evas_cb_restack(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *sec, *ec = data;
   Eina_List *l, *ll;

   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_client_has_xwindow(ec)) return;

   /* only restack to enforce shell spec if config would otherwise not force restack */
   if (!e_config->transient.raise)
     e_client_transients_restack(ec);

   if (!ec->comp_data->sub.list) return;
   EINA_LIST_FOREACH(ec->comp_data->sub.list, l, sec)
     evas_object_layer_set(sec->frame, evas_object_layer_get(ec->frame));
   sec = eina_list_last_data_get(ec->comp_data->sub.list);
   evas_object_stack_above(sec->frame, ec->frame);
   EINA_LIST_REVERSE_FOREACH_SAFE(ec->comp_data->sub.list, l, ll, sec)
     {
        E_Client *nsec = eina_list_data_get(ll);

        if (nsec)
          evas_object_stack_below(nsec->frame, sec->frame);
     }
}

static void
_e_comp_wl_evas_cb_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *sec, *ec = data;
   Eina_List *l;

   if (e_object_is_del(E_OBJECT(ec))) return;
   ec->comp_data->moved = 1;
   EINA_LIST_FOREACH(ec->comp_data->sub.list, l, sec)
     {
        if (!sec->comp_data->sub.data->position.set)
          evas_object_move(sec->frame, ec->client.x + sec->comp_data->sub.data->position.x,
                           ec->client.y + sec->comp_data->sub.data->position.y);
     }
   _e_comp_wl_surface_outputs_update(ec);
}

static void
_e_comp_wl_evas_cb_unmaximize_pre(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   E_Client *ec = data;
   E_Maximize *max = event_info;

   if (ec->need_fullscreen || e_object_is_del(E_OBJECT(ec))) return;
   if (ec->comp_data->in_commit)
     ec->comp_data->maximizing = 1;
   else if (!e_client_has_xwindow(ec))
     {
        int w, h, ew = 0, eh = 0, *ecw, *ech;
        unsigned int pmax = ec->maximized;
        ec->comp_data->unmax = *max;
        if (ec->internal)
          ecw = &ec->client.w, ech = &ec->client.h;
        else
          ecw = &ec->w, ech = &ec->h;
        if ((!e_config->window_maximize_animate) || ec->maximize_anims_disabled)
          {
             e_client_unmaximize_geometry_get(ec, *max, NULL, NULL, &w, &h);
             if (ec->internal)
               e_comp_object_frame_wh_unadjust(ec->frame, w, h, &w, &h);
             ew = *ecw, eh = *ech;
             *ecw = w, *ech = h;
          }
        ec->maximized = 0;
        _e_comp_wl_configure_send(ec, 0);
        if ((!e_config->window_maximize_animate) || ec->maximize_anims_disabled)
          *ecw = ew, *ech = eh;
        ec->maximized = pmax;
        *max = 0;
     }
}

static void
_e_comp_wl_evas_cb_maximize_pre(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   E_Client *ec = data;
   E_Maximize *max = event_info;

   if (e_object_is_del(E_OBJECT(ec))) return;
   if (!ec->comp_data->buffer_commit)
     ec->maximize_anims_disabled = 1;
   if (ec->comp_data->in_commit)
     ec->comp_data->maximizing = 1;
   else if (!e_client_has_xwindow(ec))
     {
        int w, h, ew = 0, eh = 0, *ecw, *ech;
        unsigned int pmax = ec->maximized;
        ec->comp_data->max = *max;
        if (ec->internal)
          ecw = &ec->client.w, ech = &ec->client.h;
        else
          ecw = &ec->w, ech = &ec->h;
        if ((!e_config->window_maximize_animate) ||
            ec->maximize_anims_disabled || (!ec->comp_data->buffer_commit))
          {
             e_client_maximize_geometry_get(ec, *max, NULL, NULL, &w, &h);
             if (ec->internal)
               e_comp_object_frame_wh_unadjust(ec->frame, w, h, &w, &h);
             ew = *ecw, eh = *ech;
             *ecw = w, *ech = h;
          }
        ec->maximized = *max;
        _e_comp_wl_configure_send(ec, 0);
        if ((!e_config->window_maximize_animate) || ec->maximize_anims_disabled)
          *ecw = ew, *ech = eh;
        ec->maximized = pmax;
        *max = 0;
     }
}

static void
_e_comp_wl_evas_cb_resize(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec;

   if (!(ec = data)) return;
   if (e_object_is_del(E_OBJECT(ec))) return;

   if ((ec->shading) || (ec->shaded)) return;
   if (!ec->comp_data->shell.configure_send) return;
   if (ec->comp_data->maximizing) return;
   if (e_client_util_resizing_get(ec) && e_comp_wl->resize.edges)
     {
        int x, y;

        x = ec->mouse.last_down[ec->moveinfo.down.button - 1].w;
        y = ec->mouse.last_down[ec->moveinfo.down.button - 1].h;
        if (e_comp_object_frame_exists(ec->frame))
          e_comp_object_frame_wh_unadjust(ec->frame, x, y, &x, &y);

        switch (ec->resize_mode)
          {
           case E_POINTER_RESIZE_TL:
           case E_POINTER_RESIZE_L:
           case E_POINTER_RESIZE_BL:
             x += ec->mouse.last_down[ec->moveinfo.down.button - 1].mx -
               ec->mouse.current.mx;
             break;
           case E_POINTER_RESIZE_TR:
           case E_POINTER_RESIZE_R:
           case E_POINTER_RESIZE_BR:
             x += ec->mouse.current.mx - ec->mouse.last_down[ec->moveinfo.down.button - 1].mx;
             break;
           default:
             break;;
          }
        switch (ec->resize_mode)
          {
           case E_POINTER_RESIZE_TL:
           case E_POINTER_RESIZE_T:
           case E_POINTER_RESIZE_TR:
             y += ec->mouse.last_down[ec->moveinfo.down.button - 1].my -
               ec->mouse.current.my;
             break;
           case E_POINTER_RESIZE_BL:
           case E_POINTER_RESIZE_B:
           case E_POINTER_RESIZE_BR:
             y += ec->mouse.current.my - ec->mouse.last_down[ec->moveinfo.down.button - 1].my;
             break;
           default:
             break;
          }
        x = E_CLAMP(x, 1, x);
        y = E_CLAMP(y, 1, y);
        ec->comp_data->shell.configure_send(ec->comp_data->shell.surface,
                                            e_comp_wl->resize.edges,
                                            x, y);
     }
   else
     _e_comp_wl_configure_send(ec, 1);
}

static void
_e_comp_wl_evas_cb_state_update(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec = data;

   if (e_object_is_del(E_OBJECT(ec))) return;

   /* check for wayland pixmap */

   if (ec->comp_data->shell.configure_send)
     _e_comp_wl_configure_send(ec, 0);
}

static void
_e_comp_wl_evas_cb_maximize_done(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *ec = data;

   if (e_object_is_del(E_OBJECT(ec))) return;
   ec->comp_data->maximizing = 0;
   ec->maximize_anims_disabled = ec->comp_data->maximize_anims_disabled;
}

static void
_e_comp_wl_evas_cb_delete_request(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec = data;

   e_comp_ignore_win_del(E_PIXMAP_TYPE_WL, e_pixmap_window_get(ec->pixmap));

   if (!e_client_has_xwindow(ec))
     {
        if (ec->internal_elm_win)
          evas_object_del(ec->internal_elm_win);
        else
          e_object_del(E_OBJECT(ec));
     }
}

static void
_e_comp_wl_evas_cb_kill_request(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec;

   if (!(ec = data)) return;

   e_comp_ignore_win_del(E_PIXMAP_TYPE_WL, e_pixmap_window_get(ec->pixmap));

   evas_object_pass_events_set(ec->frame, EINA_TRUE);
   if (ec->visible) evas_object_hide(ec->frame);
   if (ec->internal)
     {
        ec->ignored = 1;
        if (!e_object_is_del(E_OBJECT(ec)))
          ec->comp_data->mapped = EINA_FALSE;
     }
   else e_object_del(E_OBJECT(ec));
}

static void
_e_comp_wl_evas_cb_ping(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Client *ec;

   if (!(ec = data)) return;
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (!(ec->comp_data->shell.ping)) return;
   if (!(ec->comp_data->shell.surface)) return;

   ec->comp_data->shell.ping(ec->comp_data->shell.surface);
}

static void
_e_comp_wl_evas_cb_color_set(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   E_Client *ec;
   int a = 0;

   if (!(ec = data)) return;
   evas_object_color_get(obj, NULL, NULL, NULL, &a);
   if (ec->netwm.opacity == a) return;
   ec->netwm.opacity = a;
   ec->netwm.opacity_changed = EINA_TRUE;
}

static void
_e_comp_wl_buffer_cb_destroy(struct wl_listener *listener, void *data EINA_UNUSED)
{
   E_Comp_Wl_Buffer *buffer;

   buffer = container_of(listener, E_Comp_Wl_Buffer, destroy_listener);
   wl_signal_emit(&buffer->destroy_signal, buffer);
   buffer->destroyed = EINA_TRUE;

   if (!buffer->busy)
     free(buffer);
}

static void
_e_comp_wl_client_evas_init(E_Client *ec)
{
   if (ec->comp_data->evas_init) return;
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_SHOW,
                                  EVAS_CALLBACK_PRIORITY_AFTER,
                                  _e_comp_wl_evas_cb_show, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_HIDE,
                                  EVAS_CALLBACK_PRIORITY_AFTER,
                                  _e_comp_wl_evas_cb_hide, ec);

   /* setup input callbacks */
   evas_object_smart_callback_add(ec->frame, "mouse_in",
     (Evas_Smart_Cb)_e_comp_wl_cb_ssd_mouse_in, ec);
   evas_object_smart_callback_add(ec->frame, "mouse_out",
     (Evas_Smart_Cb)_e_comp_wl_cb_ssd_mouse_out, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MOUSE_IN,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           (Evas_Object_Event_Cb)_e_comp_wl_evas_cb_mouse_in, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MOUSE_OUT,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           (Evas_Object_Event_Cb)_e_comp_wl_evas_cb_mouse_out, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MOUSE_MOVE,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_mouse_move, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MOUSE_DOWN,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_mouse_down, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MOUSE_UP,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_mouse_up, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MOUSE_WHEEL,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_mouse_wheel, ec);

   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MULTI_DOWN,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_multi_down, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MULTI_UP,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_multi_up, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MULTI_MOVE,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_multi_move, ec);


   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_FOCUS_IN,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_focus_in, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_FOCUS_OUT,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_focus_out, ec);

   if (!ec->override)
     {
        evas_object_smart_callback_add(ec->frame, "client_resize",
                                       _e_comp_wl_evas_cb_resize, ec);
        evas_object_smart_callback_add(ec->frame, "maximize_pre",
                                       _e_comp_wl_evas_cb_maximize_pre, ec);
        evas_object_smart_callback_add(ec->frame, "unmaximize_pre",
                                       _e_comp_wl_evas_cb_unmaximize_pre, ec);
        evas_object_smart_callback_add(ec->frame, "maximize_done",
                                       _e_comp_wl_evas_cb_maximize_done, ec);
        evas_object_smart_callback_add(ec->frame, "unmaximize_done",
                                       _e_comp_wl_evas_cb_maximize_done, ec);
        evas_object_smart_callback_add(ec->frame, "fullscreen",
                                       _e_comp_wl_evas_cb_state_update, ec);
     }
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_MOVE,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_move, ec);
   evas_object_event_callback_priority_add(ec->frame, EVAS_CALLBACK_RESTACK,
                                           EVAS_CALLBACK_PRIORITY_AFTER,
                                           _e_comp_wl_evas_cb_restack, ec);

   if (ec->internal_elm_win)
     {
        /* setup delete/kill callbacks */
        evas_object_smart_callback_add(ec->frame, "delete_request",
                                       _e_comp_wl_evas_cb_delete_request, ec);
        evas_object_smart_callback_add(ec->frame, "kill_request",
                                       _e_comp_wl_evas_cb_kill_request, ec);
     }

   /* setup ping callback */
   evas_object_smart_callback_add(ec->frame, "ping",
                                  _e_comp_wl_evas_cb_ping, ec);

   evas_object_smart_callback_add(ec->frame, "color_set",
                                  _e_comp_wl_evas_cb_color_set, ec);

   ec->comp_data->evas_init = EINA_TRUE;
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_SCREENSAVER_ON, _e_comp_wl_screensaver_on, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_SCREENSAVER_OFF, _e_comp_wl_screensaver_off, NULL);
   e_comp_wl_notidle();

   e_screensaver_attrs_set(e_screensaver_timeout_get(EINA_TRUE),
                           e_config->screensaver_blanking,
                           e_config->screensaver_expose);
// XXX: maybe later like x work on explicit suspend of compositor stuff?
//   e_desklock_show_hook_add(_e_comp_x_desklock_show);
//   e_desklock_hide_hook_add(_e_comp_x_desklock_hide);
}

static inline int
_rotation_get(int rotation)
{
   int transform;
   switch (rotation)
     {
      case 90:
        transform = WL_OUTPUT_TRANSFORM_90;
        break;
      case 180:
        transform = WL_OUTPUT_TRANSFORM_180;
        break;
      case 270:
        transform = WL_OUTPUT_TRANSFORM_270;
        break;
      case 0:
      default:
        transform = WL_OUTPUT_TRANSFORM_NORMAL;
        break;
     }
   return transform;
}

static Eina_Bool
_e_comp_wl_cb_randr_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   const Eina_List *l;
   E_Randr2_Screen *screen;

   if (e_xinerama_fake_screens_exist())
     {
        E_Screen *scr;
        EINA_LIST_FOREACH(e_xinerama_screens_get(), l, scr)
          {
             e_comp_wl_output_init(NULL, NULL, NULL,
               scr->x, scr->y, scr->w, scr->h,
               0, 0, 0, 0, _rotation_get(scr->rotation), scr->escreen);
          }
        return ECORE_CALLBACK_RENEW;
     }
   if (!e_randr2) return ECORE_CALLBACK_RENEW;
   EINA_LIST_FOREACH(e_randr2->screens, l, screen)
     {
        if (!screen->config.enabled)
          {
             e_comp_wl_output_remove(screen->id);
             continue;
          }

        if (!e_comp_wl_output_init(screen->id, screen->info.name,
                                   screen->info.screen,
                                   screen->config.geom.x, screen->config.geom.y,
                                   screen->config.geom.w, screen->config.geom.h,
                                   screen->info.size.w, screen->info.size.h,
                                   screen->config.mode.refresh, screen->info.subpixel,
                                   _rotation_get(screen->config.rotation), 0))
          ERR("Could not initialize screen %s", screen->info.name);
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_comp_wl_cb_comp_object_add(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Comp_Object *ev)
{
   E_Client *ec;

   /* try to get the client from the object */
   if (!(ec = e_comp_object_client_get(ev->comp_object)))
     return ECORE_CALLBACK_RENEW;

   /* check for client being deleted */
   if (e_object_is_del(E_OBJECT(ec))) return ECORE_CALLBACK_RENEW;

   /* check for wayland pixmap */
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL)
     return ECORE_CALLBACK_RENEW;

   /* if we have not setup evas callbacks for this client, do it */
   if (!ec->comp_data->evas_init) _e_comp_wl_client_evas_init(ec);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_comp_wl_cb_mouse_move(void *d EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Mouse_Move *ev)
{
   _last_event_time = ecore_loop_time_get();

   e_comp_wl->ptr.x = ev->x;
   e_comp_wl->ptr.y = ev->y;
   e_comp_canvas_notidle();
   if (e_comp_wl->selection.target &&
       (!e_client_has_xwindow(e_comp_wl->selection.target)) &&
       e_comp_wl->drag)
     {
        struct wl_resource *res;
        int x, y;

        res = e_comp_wl_data_find_for_client(wl_resource_get_client(e_comp_wl->selection.target->comp_data->surface));
        x = ev->x - e_comp_wl->selection.target->client.x;
        y = ev->y - e_comp_wl->selection.target->client.y;
        wl_data_device_send_motion(res, ev->timestamp, wl_fixed_from_int(x), wl_fixed_from_int(y));
     }
   if (e_comp_wl->drag &&
       e_comp_wl->drag_client &&
       e_client_has_xwindow(e_comp_wl->drag_client) &&
       (!e_comp_wl->drag_client->override))
     _e_comp_wl_send_mouse_move(e_comp_wl->drag_client, ev->x, ev->y, ev->timestamp);
   return ECORE_CALLBACK_RENEW;
}

static void
_e_comp_wl_surface_state_size_update(E_Client *ec, E_Comp_Wl_Surface_State *state)
{
   Eina_Rectangle *window;
   /* double scale = 0.0; */

   /* scale = e_comp_wl->output.scale; */
   /* switch (e_comp_wl->output.transform) */
   /*   { */
   /*    case WL_OUTPUT_TRANSFORM_90: */
   /*    case WL_OUTPUT_TRANSFORM_270: */
   /*    case WL_OUTPUT_TRANSFORM_FLIPPED_90: */
   /*    case WL_OUTPUT_TRANSFORM_FLIPPED_270: */
   /*      w = ec->comp_data->buffer_ref.buffer->h / scale; */
   /*      h = ec->comp_data->buffer_ref.buffer->w / scale; */
   /*      break; */
   /*    default: */
   /*      w = ec->comp_data->buffer_ref.buffer->w / scale; */
   /*      h = ec->comp_data->buffer_ref.buffer->h / scale; */
   /*      break; */
   /*   } */

   if (!e_pixmap_size_get(ec->pixmap, &state->bw, &state->bh)) return;
   if (e_client_has_xwindow(ec) || e_comp_object_frame_exists(ec->frame)) return;
   window = &ec->comp_data->shell.window;
   if (window->x || window->y || window->w || window->h)
     e_comp_object_frame_geometry_set(ec->frame, -window->x,
                                      (window->x + window->w) - state->bw,
                                      -window->y,
                                      (window->y + window->h) - state->bh);
   else
     e_comp_object_frame_geometry_set(ec->frame, 0, 0, 0, 0);
}

static void
_e_comp_wl_surface_state_cb_buffer_destroy(struct wl_listener *listener, void *data EINA_UNUSED)
{
   E_Comp_Wl_Surface_State *state;

   state =
     container_of(listener, E_Comp_Wl_Surface_State, buffer_destroy_listener);
   state->buffer = NULL;
}

static void
_e_comp_wl_surface_state_init(E_Comp_Wl_Surface_State *state)
{
   state->new_attach = EINA_FALSE;
   state->buffer = NULL;
   state->buffer_destroy_listener.notify =
     _e_comp_wl_surface_state_cb_buffer_destroy;
   state->sx = state->sy = 0;

   state->input = NULL;

   state->opaque = NULL;
}

static void
_e_comp_wl_surface_state_finish(E_Comp_Wl_Surface_State *state)
{
   struct wl_resource *cb;
   Eina_Rectangle *dmg;
   Eina_List *free_list;

   /* The resource destroy callback will walk the state->frames list,
    * so move the list to a temporary first.
    */
   free_list = state->frames;
   state->frames = NULL;
   EINA_LIST_FREE(free_list, cb)
     wl_resource_destroy(cb);

   EINA_LIST_FREE(state->damages, dmg)
     eina_rectangle_free(dmg);

   if (state->opaque) eina_tiler_free(state->opaque);
   state->opaque = NULL;

   if (state->input) eina_tiler_free(state->input);
   state->input = NULL;

   if (state->buffer) wl_list_remove(&state->buffer_destroy_listener.link);
   state->buffer = NULL;
}

static void
_e_comp_wl_surface_state_buffer_set(E_Comp_Wl_Surface_State *state, E_Comp_Wl_Buffer *buffer)
{
   if (state->buffer == buffer) return;
   if (state->buffer)
     wl_list_remove(&state->buffer_destroy_listener.link);
   state->buffer = buffer;
   if (state->buffer)
     wl_signal_add(&state->buffer->destroy_signal,
                   &state->buffer_destroy_listener);
}

static void
_e_comp_wl_surface_state_attach(E_Client *ec, E_Comp_Wl_Surface_State *state)
{
   /* set usable early because shell module checks this */
   e_pixmap_usable_set(ec->pixmap, (state->buffer != NULL));
   e_pixmap_resource_set(ec->pixmap, state->buffer);
   e_pixmap_dirty(ec->pixmap);
   e_pixmap_refresh(ec->pixmap);
}

static void
_e_comp_wl_surface_state_commit(E_Client *ec, E_Comp_Wl_Surface_State *state)
{
   Eina_Bool first = EINA_FALSE;
   Eina_Rectangle *dmg;
   int x = 0, y = 0, w, h;

   first = !e_pixmap_usable_get(ec->pixmap);
#ifndef HAVE_WAYLAND_ONLY
   if (first && e_client_has_xwindow(ec))
     first = !e_pixmap_usable_get(e_comp_x_client_pixmap_get(ec));
#endif

   if (e_client_util_is_popup(ec) && (!ec->parent) && (!e_client_has_xwindow(ec)))
     {
        wl_resource_post_error(ec->comp_data->surface,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "Popup requires a parent shell surface before commit");
        return;
     }

   ec->comp_data->in_commit = 1;
   if (ec->ignored && ec->comp_data->shell.surface)
     {
        EC_CHANGED(ec);
        ec->new_client = 1;
        e_comp->new_clients++;
        e_client_unignore(ec);
     }

   if (state->new_attach)
     {
        _e_comp_wl_surface_state_attach(ec, state);
        if (first && (!ec->comp_data->cursor) && (!e_client_util_is_popup(ec)))
          {
             if (!e_client_has_xwindow(ec))
               {
                  ec->take_focus = !starting || ec->internal_elm_win;
                  ec->want_focus = ec->override && !ec->parent;
                  if (ec->parent && (!ec->lock_user_location))
                    e_comp_object_util_center_on(ec->frame, ec->parent->frame);
               }
          }
     }

   _e_comp_wl_surface_state_buffer_set(state, NULL);


   if (ec->comp_data->shell.surface)
     {
        if (ec->comp_data->shell.set.min_size.w)
          ec->icccm.min_w = ec->comp_data->shell.set.min_size.w;
        ec->comp_data->shell.set.min_size.w = 0;
        if (ec->comp_data->shell.set.min_size.h)
          ec->icccm.min_h = ec->comp_data->shell.set.min_size.h;
        ec->comp_data->shell.set.min_size.h = 0;
        if (ec->comp_data->shell.set.max_size.w)
          ec->icccm.max_w = ec->comp_data->shell.set.max_size.w;
        ec->comp_data->shell.set.max_size.w = 0;
        if (ec->comp_data->shell.set.max_size.h)
          ec->icccm.max_h = ec->comp_data->shell.set.max_size.h;
        ec->comp_data->shell.set.max_size.h = 0;
        if (ec->icccm.min_w && ec->icccm.max_w && (ec->icccm.min_w > ec->icccm.max_w))
          wl_resource_post_error(ec->comp_data->shell.surface,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "min surface width cannot be larger than max surface width");
        if (ec->icccm.min_h && ec->icccm.max_h && (ec->icccm.min_h > ec->icccm.max_h))
          wl_resource_post_error(ec->comp_data->shell.surface,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "min surface height cannot be larger than max surface height");
        if (ec->comp_data->shell.set.fullscreen && (!ec->fullscreen))
          {
             E_Zone *zone = e_comp_zone_id_get(ec->comp_data->shell.set.fs_zone);

             if (zone && (zone != ec->zone))
               e_client_zone_set(ec, zone);
             e_client_fullscreen(ec, E_FULLSCREEN_RESIZE);
          }
        if (ec->comp_data->shell.set.unfullscreen)
           e_client_unfullscreen(ec);
        if (ec->comp_data->shell.set.maximize)
          {
             e_client_maximize(ec, (e_config->maximize_policy & E_MAXIMIZE_TYPE) | ec->comp_data->max);
             ec->comp_data->max = 0;
          }
        if (ec->comp_data->shell.set.unmaximize)
          {
             e_client_unmaximize(ec, (e_config->maximize_policy & E_MAXIMIZE_TYPE) | ec->comp_data->unmax);
             ec->comp_data->unmax = 0;
          }
        if (ec->comp_data->shell.set.minimize)
          e_client_iconify(ec);
        ec->comp_data->shell.set.fullscreen =
        ec->comp_data->shell.set.unfullscreen =
        ec->comp_data->shell.set.maximize =
        ec->comp_data->shell.set.unmaximize =
        ec->comp_data->shell.set.minimize = 0;
     }
   _e_comp_wl_surface_state_size_update(ec, state);

   if (state->new_attach)
     {
        if (ec->changes.pos || ec->internal_elm_win)
          e_comp_object_frame_xy_unadjust(ec->frame, ec->x, ec->y, &x, &y);
        else
          {
             x = ec->client.x, y = ec->client.y;
             if (first)
               x -= ec->comp_data->shell.window.x, y -= ec->comp_data->shell.window.y;
          }

        if ((!ec->comp_data->buffer_commit) && e_client_has_xwindow(ec))
          /* use client geometry to avoid race condition from x11 configure request */
          x = ec->x, y = ec->y;
        else
          {
             ec->client.w = state->bw;
             ec->client.h = state->bh;
             if (!ec->changes.size)
               e_comp_object_frame_wh_adjust(ec->frame, ec->client.w, ec->client.h, &ec->w, &ec->h);
          }
        w = ec->client.w;
        h = ec->client.h;
        if (ec->comp_data->need_center)
          {
             Eina_List *l;
             E_Client *cec;
             EINA_LIST_FOREACH(ec->transients, l, cec)
               {
                  cec->placed = 0;
                  if (!cec->ignored) EC_CHANGED(cec);
               }
             ec->comp_data->need_center = 0;
          }
     }
   else
     w = state->bw, h = state->bh;
   if (!e_pixmap_usable_get(ec->pixmap))
     {
        if (ec->comp_data->mapped)
          {
             if ((ec->comp_data->shell.surface) && (ec->comp_data->shell.unmap))
               ec->comp_data->shell.unmap(ec->comp_data->shell.surface);
             else if (ec->comp_data->cursor || e_client_has_xwindow(ec) || ec->internal_elm_win ||
                      (ec->comp_data->sub.data && ec->comp_data->sub.data->parent->comp_data->mapped) ||
                      (ec == e_comp_wl->drag_client))
               {
                  ec->visible = EINA_FALSE;
                  evas_object_hide(ec->frame);
                  ec->comp_data->mapped = 0;
               }
          }
     }
   else
     {
        if (!ec->comp_data->mapped)
          {
             if ((ec->comp_data->shell.surface) && (ec->comp_data->shell.map))
               ec->comp_data->shell.map(ec->comp_data->shell.surface);
             else if ((ec == e_comp->pointer->client.ec) || e_client_has_xwindow(ec) || ec->internal_elm_win ||
                      (ec->comp_data->sub.data && ec->comp_data->sub.data->parent->comp_data->mapped) ||
                      (ec == e_comp_wl->drag_client))
               {
                  ec->visible = EINA_TRUE;
                  ec->ignored = 0;
                  evas_object_show(ec->frame);
                  ec->comp_data->mapped = 1;
                  if (e_client_has_xwindow(ec))
                    {
                       if (!e_client_util_desk_visible(ec, e_desk_current_get(ec->zone)))
                         evas_object_hide(ec->frame);
                    }
               }
          }
     }

   if (state->new_attach)
     {
        if ((ec->comp_data->shell.surface) && (ec->comp_data->shell.configure))
          ec->comp_data->shell.configure(ec->comp_data->shell.surface,
                                         x, y, state->bw, state->bh);
        else
          {
             if (ec->netwm.sync.wait)
               {
                  E_Client_Pending_Resize *pnd = NULL;

                  ec->netwm.sync.wait--;

                  /* skip pending for which we didn't get a reply */
                  while (ec->pending_resize)
                    {
                       pnd = eina_list_data_get(ec->pending_resize);
                       ec->pending_resize = eina_list_remove(ec->pending_resize, pnd);

                       if ((state->bw == pnd->w) && (state->bh == pnd->h))
                         break;

                       E_FREE(pnd);
                    }

                  if (pnd)
                    {
                       e_comp_object_frame_wh_adjust(ec->frame, pnd->w, pnd->h, &ec->w, &ec->h);
                       E_FREE(pnd);
                    }
                  ecore_evas_pointer_xy_get(e_comp->ee, &ec->mouse.current.mx, &ec->mouse.current.my);
                  ec->netwm.sync.send_time = ecore_loop_time_get();
               }
             if (e_comp_wl->drag && e_comp_wl->drag_client &&
                 (e_comp_wl->drag_client == ec))
               {
                  e_comp_wl->drag->dx -= state->sx;
                  e_comp_wl->drag->dy -= state->sy;
                  e_drag_move(e_comp_wl->drag,
                    e_comp_wl->drag->x + state->sx, e_comp_wl->drag->y + state->sy);
                  e_drag_resize(e_comp_wl->drag, state->bw, state->bh);
               }
             else
               e_client_util_move_resize_without_frame(ec, x, y, w, h);
          }

        if ((!ec->comp_data->sub.data) && (!ec->comp_data->buffer_commit))
          {
             if (e_client_has_xwindow(ec))
               ec->placed = 0;
             else if (!ec->placed)
               ec->placed = ec->netwm.type != E_WINDOW_TYPE_NORMAL;
             ec->want_focus |= ec->icccm.accepts_focus && (!ec->override);
          }
     }
   else if (ec->comp_data->need_xdg_configure && ec->comp_data->shell.surface && !ec->iconic)
     _e_comp_wl_configure_send(ec, 0);

   state->sx = 0;
   state->sy = 0;
   if (state->new_attach)
     ec->comp_data->buffer_commit = 1;
   state->new_attach = EINA_FALSE;

   /* insert state frame callbacks into comp_data->frames
    * NB: This clears state->frames list */
   ec->comp_data->frames = eina_list_merge(ec->comp_data->frames,
                                           state->frames);
   state->frames = NULL;

   /* put state damages into surface */
   if ((!e_comp->nocomp) && (ec->frame))
     {
        EINA_LIST_FREE(state->damages, dmg)
          {
             e_comp_object_damage(ec->frame, dmg->x, dmg->y, dmg->w, dmg->h);
             eina_rectangle_free(dmg);
          }
     }

   /* put state opaque into surface */
   if (state->opaque)
     {
        if (!eina_tiler_empty(state->opaque))
          {
             Eina_Rectangle *rect;
             Eina_Iterator *itr;

             /* This is seriously wrong and results in only the first
              * rectangle in the region being set, but in the usual
              * case there's only one rectangle.
              */
             itr = eina_tiler_iterator_new(state->opaque);
             EINA_ITERATOR_FOREACH(itr, rect)
               {
                  Eina_Rectangle r;

                  EINA_RECTANGLE_SET(&r, rect->x, rect->y, rect->w, rect->h);
                  E_RECTS_CLIP_TO_RECT(r.x, r.y, r.w, r.h, 0, 0, state->bw, state->bh);
                  e_pixmap_image_opaque_set(ec->pixmap, r.x, r.y, r.w, r.h);
                  break;
               }

             eina_iterator_free(itr);
             eina_tiler_free(state->opaque);
             state->opaque = NULL;
          }
        else
          e_pixmap_image_opaque_set(ec->pixmap, 0, 0, 0, 0);
     }

   /* put state input into surface */
   if (e_client_has_xwindow(ec))
     {
        Eina_Tiler *t = eina_tiler_new(ec->client.w, ec->client.h);
        eina_tiler_tile_size_set(t, 1, 1);
        if (ec->shaped_input)
          {
             Eina_Rectangle *r;
             unsigned int i;

             for (i = 0, r = ec->shape_input_rects; i < ec->shape_input_rects_num; i++, r++)
               eina_tiler_rect_add(t, r);
          }
        else
          eina_tiler_rect_add(t, &(Eina_Rectangle){0, 0, 65535, 65535});
        e_comp_object_input_area_set(ec->frame, t);
        eina_tiler_free(t);
     }
   else if (state->input)
     {
        e_comp_object_input_area_set(ec->frame, state->input);

        eina_tiler_free(state->input);
        state->input = NULL;
     }
   ec->comp_data->in_commit = 0;

   _e_comp_wl_surface_outputs_update(ec);
}

static void
_e_comp_wl_surface_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec;
   DBG("Surface Cb Destroy: %d", wl_resource_get_id(resource));
   ec = wl_resource_get_user_data(resource);
   if (ec && (!e_object_is_del(E_OBJECT(ec)))) ec->comp_data->surface = NULL;
   wl_resource_destroy(resource);
}

static void
_e_comp_wl_surface_cb_attach(struct wl_client *client, struct wl_resource *resource, struct wl_resource *buffer_resource, int32_t sx, int32_t sy)
{
   E_Client *ec;
   E_Comp_Wl_Buffer *buffer = NULL;

   if (!(ec = wl_resource_get_user_data(resource))) return;
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (ec->comp_data->need_xdg_configure)
     {
        ec->comp_data->shell.buffer_attach_error(ec);
        return;
     }

   if (buffer_resource)
     {
        if (!(buffer = e_comp_wl_buffer_get(buffer_resource)))
          {
             ERR("Could not get buffer from resource");
             wl_client_post_no_memory(client);
             return;
          }
     }

   _e_comp_wl_surface_state_buffer_set(&ec->comp_data->pending, buffer);

   ec->comp_data->pending.sx = sx;
   ec->comp_data->pending.sy = sy;
   ec->comp_data->pending.new_attach = EINA_TRUE;
}

static void
_e_comp_wl_surface_cb_damage_buffer(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   E_Client *ec;
   Eina_Rectangle *dmg = NULL;

   if (!(ec = wl_resource_get_user_data(resource))) return;
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (!(dmg = eina_rectangle_new(x, y, w, h))) return;

   ec->comp_data->pending.damages =
     eina_list_append(ec->comp_data->pending.damages, dmg);
}

/*
 * Currently damage and damage_buffer are the same because we don't support
 * buffer_scale, transform, or viewport.  Once we support those we'll have
 * to make surface_cb_damage handle damage in surface co-ordinates.
 */
static void
_e_comp_wl_surface_cb_damage(struct wl_client *client, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   _e_comp_wl_surface_cb_damage_buffer(client, resource, x, y, w, h);
}

static void
_e_comp_wl_frame_cb_destroy(struct wl_resource *resource)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource))) return;
   if (e_object_is_del(E_OBJECT(ec))) return;

   ec->comp_data->frames =
     eina_list_remove(ec->comp_data->frames, resource);

   ec->comp_data->pending.frames =
     eina_list_remove(ec->comp_data->pending.frames, resource);

   if (!ec->comp_data->sub.data) return;

   ec->comp_data->sub.data->cached.frames =
     eina_list_remove(ec->comp_data->sub.data->cached.frames, resource);
}

static void
_e_comp_wl_surface_cb_frame(struct wl_client *client, struct wl_resource *resource, uint32_t callback)
{
   E_Client *ec;
   struct wl_resource *res;

   if (!(ec = wl_resource_get_user_data(resource))) return;
   if (e_object_is_del(E_OBJECT(ec))) return;

   /* create frame callback */
   if (!(res =
         wl_resource_create(client, &wl_callback_interface, 1, callback)))
     {
        wl_resource_post_no_memory(resource);
        return;
     }

   wl_resource_set_implementation(res, NULL, ec, _e_comp_wl_frame_cb_destroy);

   ec->comp_data->pending.frames =
     eina_list_append(ec->comp_data->pending.frames, res);
}

static void
_e_comp_wl_surface_cb_opaque_region_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource))) return;
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (ec->comp_data->pending.opaque)
     eina_tiler_free(ec->comp_data->pending.opaque);
   ec->comp_data->pending.opaque = eina_tiler_new(65535, 65535);
   eina_tiler_tile_size_set(ec->comp_data->pending.opaque, 1, 1);
   if (region_resource)
     {
        Eina_Tiler *tmp;

        if (!(tmp = wl_resource_get_user_data(region_resource)))
          return;

        eina_tiler_union(ec->comp_data->pending.opaque, tmp);
     }
}

static void
_e_comp_wl_surface_cb_input_region_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource))) return;
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (ec->comp_data->cursor || (ec == e_comp_wl->drag_client)) return;

   if (ec->comp_data->pending.input)
     eina_tiler_free(ec->comp_data->pending.input);
   ec->comp_data->pending.input = eina_tiler_new(65535, 65535);
   eina_tiler_tile_size_set(ec->comp_data->pending.input, 1, 1);
   if (region_resource)
     {
        Eina_Tiler *tmp;

        if (!(tmp = wl_resource_get_user_data(region_resource)))
          return;

        eina_tiler_union(ec->comp_data->pending.input, tmp);
     }
   else
     {
        eina_tiler_rect_add(ec->comp_data->pending.input,
                            &(Eina_Rectangle){0, 0, 65535, 65535});
     }
}

static void
_e_comp_wl_surface_cb_commit(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec, *subc;
   Eina_List *l;

   if (!(ec = wl_resource_get_user_data(resource))) return;
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (e_comp_wl_subsurface_commit(ec)) return;

   e_comp_wl_surface_commit(ec);

   EINA_LIST_FOREACH(ec->comp_data->sub.list, l, subc)
     {
        if (ec != subc)
          _e_comp_wl_subsurface_parent_commit(subc, EINA_FALSE);
     }
}

static void
_e_comp_wl_surface_cb_buffer_transform_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource EINA_UNUSED, int32_t transform EINA_UNUSED)
{
   /* DBG("Surface Buffer Transform: %d", wl_resource_get_id(resource)); */
}

static void
_e_comp_wl_surface_cb_buffer_scale_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource EINA_UNUSED, int32_t scale EINA_UNUSED)
{
   /* DBG("Surface Buffer Scale: %d", wl_resource_get_id(resource)); */
}

static const struct wl_surface_interface _e_surface_interface =
{
   _e_comp_wl_surface_cb_destroy,
   _e_comp_wl_surface_cb_attach,
   _e_comp_wl_surface_cb_damage,
   _e_comp_wl_surface_cb_frame,
   _e_comp_wl_surface_cb_opaque_region_set,
   _e_comp_wl_surface_cb_input_region_set,
   _e_comp_wl_surface_cb_commit,
   _e_comp_wl_surface_cb_buffer_transform_set,
   _e_comp_wl_surface_cb_buffer_scale_set,
   _e_comp_wl_surface_cb_damage_buffer
};


static void
_e_comp_wl_surface_destroy(struct wl_resource *resource)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource))) return;

   if (!e_object_is_del(E_OBJECT(ec)))
     {
        if (ec->comp_data->mapped)
          ec->comp_data->mapped = EINA_FALSE;
        evas_object_hide(ec->frame);
     }

   if (ec->internal_elm_win)
     {
        e_pixmap_alias(ec->pixmap, E_PIXMAP_TYPE_WL, wl_resource_get_id(resource));
        ec->ignored = 1;
     }
   else
     e_object_del(E_OBJECT(ec));
}

static void
_e_comp_wl_compositor_cb_surface_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   struct wl_resource *res;
   E_Client *wc, *ec = NULL;
   Eina_List *l;
   pid_t pid;

   DBG("Compositor Cb Surface Create: %d", id);

   /* try to create an internal surface */
   if (!(res = wl_resource_create(client, &wl_surface_interface,
                                  wl_resource_get_version(resource), id)))
     {
        ERR("Could not create compositor surface");
        wl_client_post_no_memory(client);
        return;
     }

   DBG("\tCreated Resource: %d", wl_resource_get_id(res));

   /* set implementation on resource */
   wl_resource_set_implementation(res, &_e_surface_interface, NULL,
                                  _e_comp_wl_surface_destroy);

   wl_client_get_credentials(client, &pid, NULL, NULL);
   if ((client != e_comp_wl->xwl_client) && (pid == getpid())) //internal!
     ec = e_pixmap_find_client(E_PIXMAP_TYPE_WL, (int64_t)id);
   if (ec)
     {
        if (e_object_is_del(E_OBJECT(ec))) return;
     }
   else
     {
        E_Pixmap *ep = NULL;

        /* try to create new pixmap */
        do
          {
             if (--surface_id >= 0) surface_id = -1;
             ep = e_pixmap_find(E_PIXMAP_TYPE_WL, surface_id);
          } while (ep);
        if (!(ep = e_pixmap_new(E_PIXMAP_TYPE_WL, surface_id)))
          {
             ERR("Could not create new pixmap");
             wl_resource_destroy(res);
             wl_client_post_no_memory(client);
             return;
          }
        DBG("\tUsing Pixmap: %p", ep);

        ec = e_client_new(ep, 0, 0);
     }
   if (ec->new_client)
     e_comp->new_clients--;
   ec->new_client = 0;
   if ((!ec->client.w) && (!ec->client.h))
     ec->client.w = ec->client.h = 1;
   ec->comp_data->surface = res;
   ec->netwm.pid = pid;
   if (!ec->internal_elm_win)
     {
        ec->icccm.min_w = ec->icccm.min_h =
        ec->icccm.max_w = ec->icccm.max_h = 0;
     }
   if (client != e_comp_wl->xwl_client)
     ec->internal = pid == getpid();

   if (ec->internal && (!e_comp_wl->wl.client_ec))
     {
        e_comp_wl->wl.client_ec = ec;
        if (!e_client_focused_get())
          {
             e_comp_wl->kbd.focus = e_comp_wl->wl.client_ec->comp_data->surface;
             e_comp_wl_data_device_keyboard_focus_set();
          }
     }
   ec->icccm.delete_request |= ec->internal;

   /* set reference to pixmap so we can fetch it later */
   DBG("\tUsing Client: %p", ec);
   wl_resource_set_user_data(res, ec);
#ifndef HAVE_WAYLAND_ONLY
   EINA_LIST_FOREACH(e_comp_wl->xwl_pending, l, wc)
     {
        if (!e_pixmap_is_x(wc->pixmap)) continue;
        if (wl_resource_get_id(res) !=
            ((E_Comp_X_Client_Data*)wc->comp_data)->surface_id) continue;
        // wc, ec is correct! first arg should be the x client, 2nd wl one
        e_comp_x_xwayland_client_setup(wc, ec);
        break;
     }
#endif
   /* emit surface create signal */
   wl_signal_emit(&e_comp_wl->signals.surface.create, res);

   e_object_ref(E_OBJECT(ec));
}

static void
_e_comp_wl_region_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   DBG("Region Destroy: %d", wl_resource_get_id(resource));
   wl_resource_destroy(resource);
}

static void
_e_comp_wl_region_cb_add(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Eina_Tiler *tiler;

   DBG("Region Add: %d", wl_resource_get_id(resource));
   DBG("\tGeom: %d %d %d %d", x, y, w, h);

   /* get the tiler from the resource */
   if ((tiler = wl_resource_get_user_data(resource)))
     eina_tiler_rect_add(tiler, &(Eina_Rectangle){x, y, w, h});
}

static void
_e_comp_wl_region_cb_subtract(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Eina_Tiler *tiler;

   DBG("Region Subtract: %d", wl_resource_get_id(resource));
   DBG("\tGeom: %d %d %d %d", x, y, w, h);

   /* get the tiler from the resource */
   if ((tiler = wl_resource_get_user_data(resource)))
     eina_tiler_rect_del(tiler, &(Eina_Rectangle){x, y, w, h});
}

static const struct wl_region_interface _e_region_interface =
{
   _e_comp_wl_region_cb_destroy,
   _e_comp_wl_region_cb_add,
   _e_comp_wl_region_cb_subtract
};

static void
_e_comp_wl_compositor_cb_region_destroy(struct wl_resource *resource)
{
   Eina_Tiler *tiler;

   DBG("Compositor Region Destroy: %d", wl_resource_get_id(resource));

   /* try to get the tiler from the region resource */
   if ((tiler = wl_resource_get_user_data(resource)))
     eina_tiler_free(tiler);
}

static void
_e_comp_wl_compositor_cb_region_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   Eina_Tiler *tiler;
   struct wl_resource *res;

   DBG("Region Create: %d", wl_resource_get_id(resource));

   /* try to create new tiler */
   if (!(tiler = eina_tiler_new(65535, 65535)))
     {
        ERR("Could not create Eina_Tiler");
        wl_resource_post_no_memory(resource);
        return;
     }

   /* set tiler size */
   eina_tiler_tile_size_set(tiler, 1, 1);

   if (!(res = wl_resource_create(client, &wl_region_interface, 1, id)))
     {
        ERR("\tFailed to create region resource");
        wl_resource_post_no_memory(resource);
        return;
     }

   wl_resource_set_implementation(res, &_e_region_interface, tiler,
                                  _e_comp_wl_compositor_cb_region_destroy);
}

static const struct wl_compositor_interface _e_comp_interface =
{
   _e_comp_wl_compositor_cb_surface_create,
   _e_comp_wl_compositor_cb_region_create
};

static void
_e_comp_wl_compositor_cb_bind(struct wl_client *client, void *data EINA_UNUSED, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!(res =
         wl_resource_create(client, &wl_compositor_interface,
                            version, id)))
     {
        ERR("Could not create compositor resource");
        wl_client_post_no_memory(client);
        return;
     }

   wl_resource_set_implementation(res, &_e_comp_interface, e_comp, NULL);
}

static void
_e_comp_wl_subsurface_destroy(struct wl_resource *resource)
{
   E_Client *ec;
   E_Comp_Wl_Subsurf_Data *sdata;

   /* try to get the client from resource data */
   if (!(ec = wl_resource_get_user_data(resource))) return;

   if (!ec->comp_data) return;

   if (!(sdata = ec->comp_data->sub.data)) return;

   if (sdata->parent)
     {
        /* remove this client from parents sub list */
        sdata->parent->comp_data->sub.list =
          eina_list_remove(sdata->parent->comp_data->sub.list, ec);
     }

   _e_comp_wl_surface_state_finish(&sdata->cached);

   /* the client is getting deleted, which means the pixmap will be getting
    * freed. We need to unset the surface user data */
   /* wl_resource_set_user_data(ec->comp_data->surface, NULL); */

   E_FREE(sdata);

   ec->comp_data->sub.data = NULL;
}

static Eina_Bool
_e_comp_wl_subsurface_synchronized_get(E_Comp_Wl_Subsurf_Data *sdata)
{
   while (sdata)
     {
        if (sdata->synchronized) return EINA_TRUE;
        if (!sdata->parent) return EINA_FALSE;
        sdata = sdata->parent->comp_data->sub.data;
     }

   return EINA_FALSE;
}

static void
_e_comp_wl_subsurface_commit_to_cache(E_Client *ec)
{
   E_Comp_Client_Data *cdata;
   E_Comp_Wl_Subsurf_Data *sdata;

   if (!(cdata = ec->comp_data)) return;
   if (!(sdata = cdata->sub.data)) return;

   DBG("Subsurface Commit to Cache");

   /* move pending damage to cached */
   sdata->cached.damages = eina_list_merge(sdata->cached.damages,
                                           cdata->pending.damages);
   cdata->pending.damages = NULL;

   if (cdata->pending.new_attach)
     {
        sdata->cached.new_attach = EINA_TRUE;
        _e_comp_wl_surface_state_buffer_set(&sdata->cached,
                                            cdata->pending.buffer);
        e_pixmap_resource_set(ec->pixmap, cdata->pending.buffer);
        e_pixmap_dirty(ec->pixmap);
        e_pixmap_refresh(ec->pixmap);
     }

   sdata->cached.sx = cdata->pending.sx;
   sdata->cached.sy = cdata->pending.sy;
   /* sdata->cached.buffer = cdata->pending.buffer; */
   sdata->cached.new_attach = cdata->pending.new_attach;

   /* _e_comp_wl_surface_state_buffer_set(&cdata->pending, NULL); */
   /* cdata->pending.sx = 0; */
   /* cdata->pending.sy = 0; */
   /* cdata->pending.new_attach = EINA_FALSE; */

   sdata->cached.opaque = cdata->pending.opaque;
   cdata->pending.opaque = NULL;

   sdata->cached.input = cdata->pending.input;
   cdata->pending.input = NULL;

   sdata->cached.frames = eina_list_merge(sdata->cached.frames,
                                          cdata->pending.frames);
   cdata->pending.frames = NULL;
   sdata->cached.has_data = EINA_TRUE;
}

static void
_cb_early_frame_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   E_Client *ec;

   ec = data;
   if (ec->comp_data->early_frame == 1)
     e_pixmap_image_clear(ec->pixmap, 1);

   /* If some client does a surface frame and never follows with
    * a render, it's just an idiot, so stop trying to help it with
    * timing.
    */
   if (ec->comp_data->early_frame++ > 15)
     {
       ec->comp_data->early_frame = 0;
       efl_event_callback_del(e_comp->evas, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK,
                              _cb_early_frame_cb, data);
     }
}

static void
_e_comp_wl_surface_early_frame(E_Client *ec)
{
   if (e_comp_object_damage_exists(ec->frame))
     {
        efl_event_callback_del(e_comp->evas, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK,
                               _cb_early_frame_cb, ec);
        ec->comp_data->early_frame = 0;
        return;
     }
   if (ec->on_post_updates) return;
   if (!ec->comp_data->early_frame)
       efl_event_callback_add(e_comp->evas, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK,
                              _cb_early_frame_cb, ec);
   ec->comp_data->early_frame = 1;
}

static void
_e_comp_wl_subsurface_commit_from_cache(E_Client *ec)
{
   E_Comp_Client_Data *cdata;
   E_Comp_Wl_Subsurf_Data *sdata;

   if (!(cdata = ec->comp_data)) return;
   if (!(sdata = cdata->sub.data)) return;

   DBG("Subsurface Commit from Cache");

   _e_comp_wl_surface_state_commit(ec, &sdata->cached);
   _e_comp_wl_surface_early_frame(ec);
}

static void
_e_comp_wl_subsurface_parent_commit(E_Client *ec, Eina_Bool parent_synchronized)
{
   E_Client *parent;
   E_Comp_Wl_Subsurf_Data *sdata;

   if (!(sdata = ec->comp_data->sub.data)) return;
   if (!(parent = sdata->parent)) return;

   if (sdata->position.set)
     {
        evas_object_move(ec->frame, parent->client.x + sdata->position.x,
                         parent->client.y + sdata->position.y);
        sdata->position.set = EINA_FALSE;
     }

   if ((parent_synchronized) || (sdata->synchronized))
     {
        E_Client *subc;
        Eina_List *l;

        if (sdata->cached.has_data)
          _e_comp_wl_subsurface_commit_from_cache(ec);

        EINA_LIST_FOREACH(ec->comp_data->sub.list, l, subc)
          {
             if (ec != subc)
               _e_comp_wl_subsurface_parent_commit(subc, EINA_TRUE);
          }
     }
}

static void
_e_comp_wl_subsurface_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static void
_e_comp_wl_subsurface_cb_position_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y)
{
   E_Client *ec;
   E_Comp_Wl_Subsurf_Data *sdata;

   DBG("Subsurface Cb Position Set: %d", wl_resource_get_id(resource));

   /* try to get the client from resource data */
   if (!(ec = wl_resource_get_user_data(resource))) return;

   if (!(sdata = ec->comp_data->sub.data)) return;

   sdata->position.x = x;
   sdata->position.y = y;
   sdata->position.set = EINA_TRUE;
}

static void
_e_comp_wl_subsurface_cb_place_above(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *sibling_resource)
{
   E_Client *ec, *ecs;
   E_Client *parent;

   DBG("Subsurface Cb Place Above: %d", wl_resource_get_id(resource));

   /* try to get the client from resource data */
   if (!(ec = wl_resource_get_user_data(resource))) return;

   if (!ec->comp_data->sub.data) return;

   /* try to get the client from the sibling resource */
   if (!(ecs = wl_resource_get_user_data(sibling_resource))) return;

   if (!ecs->comp_data->sub.data) return;

   if (!(parent = ec->comp_data->sub.data->parent)) return;

   parent->comp_data->sub.list =
     eina_list_remove(parent->comp_data->sub.list, ec);

   parent->comp_data->sub.list =
     eina_list_append_relative(parent->comp_data->sub.list, ec, ecs);

   parent->comp_data->sub.restack_target = parent;
}

static void
_e_comp_wl_subsurface_cb_place_below(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *sibling_resource)
{
   E_Client *ec, *ecs;
   E_Client *parent;

   DBG("Subsurface Cb Place Below: %d", wl_resource_get_id(resource));

   /* try to get the client from resource data */
   if (!(ec = wl_resource_get_user_data(resource))) return;

   if (!ec->comp_data->sub.data) return;

   /* try to get the client from the sibling resource */
   if (!(ecs = wl_resource_get_user_data(sibling_resource))) return;

   if (!ecs->comp_data->sub.data) return;

   if (!(parent = ec->comp_data->sub.data->parent)) return;

   parent->comp_data->sub.list =
     eina_list_remove(parent->comp_data->sub.list, ec);

   parent->comp_data->sub.list =
     eina_list_prepend_relative(parent->comp_data->sub.list, ec, ecs);

   parent->comp_data->sub.restack_target = parent;
}

static void
_e_comp_wl_subsurface_cb_sync_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec;
   E_Comp_Wl_Subsurf_Data *sdata;

   DBG("Subsurface Cb Sync Set: %d", wl_resource_get_id(resource));

   /* try to get the client from resource data */
   if (!(ec = wl_resource_get_user_data(resource))) return;

   if (!(sdata = ec->comp_data->sub.data)) return;

   sdata->synchronized = EINA_TRUE;
}

static void
_e_comp_wl_subsurface_cb_desync_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec;
   E_Comp_Wl_Subsurf_Data *sdata;

   DBG("Subsurface Cb Desync Set: %d", wl_resource_get_id(resource));

   /* try to get the client from resource data */
   if (!(ec = wl_resource_get_user_data(resource))) return;

   if (!(sdata = ec->comp_data->sub.data)) return;

   sdata->synchronized = EINA_FALSE;
}

static const struct wl_subsurface_interface _e_subsurface_interface =
{
   _e_comp_wl_subsurface_cb_destroy,
   _e_comp_wl_subsurface_cb_position_set,
   _e_comp_wl_subsurface_cb_place_above,
   _e_comp_wl_subsurface_cb_place_below,
   _e_comp_wl_subsurface_cb_sync_set,
   _e_comp_wl_subsurface_cb_desync_set
};

static Eina_Bool
_e_comp_wl_subsurface_create(E_Client *ec, E_Client *epc, uint32_t id, struct wl_resource *surface_resource)
{
   struct wl_client *client;
   struct wl_resource *res;
   E_Comp_Wl_Subsurf_Data *sdata;

   /* try to get the wayland client from the surface resource */
   if (!(client = wl_resource_get_client(surface_resource)))
     {
        ERR("Could not get client from resource %d",
            wl_resource_get_id(surface_resource));
        return EINA_FALSE;
     }

   /* try to allocate subsurface data */
   if (!(sdata = E_NEW(E_Comp_Wl_Subsurf_Data, 1)))
     {
        ERR("Could not allocate space for subsurface data");
        goto dat_err;
     }

   /* try to create the subsurface resource */
   if (!(res = wl_resource_create(client, &wl_subsurface_interface, 1, id)))
     {
        ERR("Failed to create subsurface resource");
        wl_resource_post_no_memory(surface_resource);
        goto res_err;
     }

   /* set resource implementation */
   wl_resource_set_implementation(res, &_e_subsurface_interface, ec,
                                  _e_comp_wl_subsurface_destroy);

   _e_comp_wl_surface_state_init(&sdata->cached);

   /* set subsurface data properties */
   sdata->resource = res;
   sdata->synchronized = EINA_TRUE;
   sdata->parent = epc;

   /* set subsurface client properties */
   ec->borderless = EINA_TRUE;
   ec->argb = EINA_TRUE;
   ec->lock_border = EINA_TRUE;
   ec->lock_focus_in = ec->lock_focus_out = EINA_TRUE;
   ec->netwm.state.skip_taskbar = EINA_TRUE;
   ec->netwm.state.skip_pager = EINA_TRUE;
   ec->no_shape_cut = EINA_TRUE;
   ec->border_size = 0;
   ec->layer_block = 1;

   if (epc)
     {
        if (epc->comp_data)
          {
             /* append this client to the parents subsurface list */
             epc->comp_data->sub.list =
               eina_list_append(epc->comp_data->sub.list, ec);
          }

        /* TODO: add callbacks ?? */
     }

   ec->comp_data->surface = surface_resource;
   ec->comp_data->sub.data = sdata;
   if (epc)
     {
        evas_object_layer_set(ec->frame, evas_object_layer_get(epc->frame));
        evas_object_stack_above(ec->frame, epc->frame);
     }

   return EINA_TRUE;

res_err:
   free(sdata);
dat_err:
   return EINA_FALSE;
}

static void
_e_comp_wl_subcompositor_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static void
_e_comp_wl_subcompositor_cb_subsurface_get(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t id, struct wl_resource *surface_resource, struct wl_resource *parent_resource)
{
   E_Client *ec, *epc = NULL;
   static const char where[] = "get_subsurface: wl_subsurface@";

   if (!(ec = wl_resource_get_user_data(surface_resource))) return;
   if (!(epc = wl_resource_get_user_data(parent_resource))) return;

   if (ec == epc)
     {
        wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE,
                               "%s%d: wl_surface@%d cannot be its own parent",
                               where, id, wl_resource_get_id(surface_resource));
        return;
     }

   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_object_is_del(E_OBJECT(epc))) return;

   /* check if this surface is already a sub-surface */
   if ((ec->comp_data) && (ec->comp_data->sub.data))
     {
        wl_resource_post_error(resource,
                               WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE,
                               "%s%d: wl_surface@%d is already a sub-surface",
                               where, id, wl_resource_get_id(surface_resource));
        return;
     }

   /* try to create a new subsurface */
   if (!_e_comp_wl_subsurface_create(ec, epc, id, surface_resource))
     ERR("Failed to create subsurface for surface %d",
         wl_resource_get_id(surface_resource));
}

static const struct wl_subcompositor_interface _e_subcomp_interface =
{
   _e_comp_wl_subcompositor_cb_destroy,
   _e_comp_wl_subcompositor_cb_subsurface_get
};

static void
_e_comp_wl_subcompositor_cb_bind(struct wl_client *client, void *data EINA_UNUSED, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!(res =
         wl_resource_create(client, &wl_subcompositor_interface,
                            version, id)))
     {
        ERR("Could not create subcompositor resource");
        wl_client_post_no_memory(client);
        return;
     }

   wl_resource_set_implementation(res, &_e_subcomp_interface, e_comp, NULL);

   /* TODO: add handlers for client iconify/uniconify */
}

static void
_e_comp_wl_client_cb_new(void *data EINA_UNUSED, E_Client *ec)
{
   int64_t win;

   /* make sure this is a wayland client */
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL) return;

   /* get window id from pixmap */
   win = e_pixmap_window_get(ec->pixmap);

   if (!(ec->comp_data = E_NEW(E_Comp_Client_Data, 1)))
     {
        ERR("Could not allocate new client data structure");
        return;
     }

   wl_signal_init(&ec->comp_data->destroy_signal);
   _e_comp_wl_surface_state_init(&ec->comp_data->pending);

   /* set initial client properties */
   ec->argb = EINA_TRUE;
   ec->no_shape_cut = EINA_TRUE;
   ec->redirected = ec->ignored = 1;
   ec->border_size = 0;

   /* NB: could not find a better place to do this, BUT for internal windows,
    * we need to set delete_request else the close buttons on the frames do
    * basically nothing */
   if (ec->internal_elm_win)
     {
        ec->icccm.delete_request = EINA_TRUE;
        ec->override = elm_win_override_get(e_win_evas_win_get(ecore_evas_get(ecore_event_window_match(win))));
     }

   /* set initial client data properties */
   ec->comp_data->mapped = EINA_FALSE;
   ec->comp_data->pending.input = eina_tiler_new(65535, 65535);
   eina_tiler_tile_size_set(ec->comp_data->pending.input, 1, 1);
   eina_tiler_rect_add(ec->comp_data->pending.input,
                            &(Eina_Rectangle){0, 0, 65535, 65535});

   /* add this client to the hash */
   /* eina_hash_add(clients_win_hash, &win, ec); */
}

static void
_e_comp_wl_client_cb_del(void *data EINA_UNUSED, E_Client *ec)
{
   /* Eina_Rectangle *dmg; */
   struct wl_resource *cb;
   E_Client *subc;
   Eina_List *free_list;

   /* make sure this is a wayland client */
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL) return;

   efl_event_callback_del(e_comp->evas, EFL_CANVAS_OBJECT_EVENT_ANIMATOR_TICK,
                          _cb_early_frame_cb, ec);
   ec->comp_data->early_frame = 0;

   if (ec == e_comp_wl->wl.client_ec)
     e_comp_wl->wl.client_ec = NULL;
   e_comp_wl_extension_pointer_unconstrain(ec);

   /* remove sub list */
   EINA_LIST_FREE(ec->comp_data->sub.list, subc)
     {
        if (!e_object_is_del(E_OBJECT(subc)))
          subc->comp_data->sub.data->parent = NULL;
     }

   if ((!e_client_has_xwindow(ec)) && (ec->parent) && (ec->parent->modal == ec))
     {
        ec->parent->lock_close = EINA_FALSE;
        ec->parent->modal = NULL;
     }
   if (e_comp_wl->selection.target == ec) e_comp_wl->selection.target = NULL;

   if ((ec == e_client_focused_get()) && ec->visible) _e_comp_wl_keyboard_leave(ec);

   wl_signal_emit(&ec->comp_data->destroy_signal, &ec->comp_data->surface);

   _e_comp_wl_surface_state_finish(&ec->comp_data->pending);
   E_FREE_FUNC(ec->comp_data->on_focus_timer, ecore_timer_del);
   if (ec->mouse.in)
     _e_comp_wl_mouse_out(ec);
   else if (ec->comp_data->cursor)
     {
        Eina_List *l;
        struct wl_resource *res;

        EINA_LIST_FOREACH(e_comp_wl->ptr.resources, l, res)
          {
             E_Comp_Wl_Pointer *ptr = wl_resource_get_user_data(res);

             if (ptr->cursor_set && (ptr->cursor == ec))
               {
                  ptr->cursor = NULL;
                  ptr->cursor_set = 0;
                  if (ptr->entered)
                    e_comp_wl_input_pointer_cursor_update(ptr);
               }
          }
     }

   if (ec->comp_data->aux_hint.hints)
     {
        E_Comp_Wl_Aux_Hint *hint;
        EINA_LIST_FREE(ec->comp_data->aux_hint.hints, hint)
          {
             eina_stringshare_del(hint->hint);
             eina_stringshare_del(hint->val);
             E_FREE(hint);
          }
     }

   /* The resource destroy callback will walk the state->frames list,
    * so move the list to a temporary first.
    */
   free_list = ec->comp_data->frames;
   ec->comp_data->frames = NULL;
   EINA_LIST_FREE(free_list, cb)
     wl_resource_destroy(cb);

   if (ec->comp_data->surface)
     wl_resource_set_user_data(ec->comp_data->surface, NULL);

   if (ec->internal_elm_win)
     evas_object_hide(ec->frame);

   /* WL clients take an extra ref at startup so they don't get deleted while
    * visible.  Since we drop that in the render loop we need to make sure
    * it's dropped here if the client isn't going to be rendered.
    */
   if (!evas_object_visible_get(ec->frame)) e_object_unref(E_OBJECT(ec));
}

static void
_e_comp_wl_client_cb_focus_set(void *data EINA_UNUSED, E_Client *ec)
{
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL) return;

   /* send configure */
   if (ec->comp_data->shell.configure_send)
     {
        if (ec->comp_data->shell.surface)
          _e_comp_wl_configure_send(ec, 1);
     }

   //if ((ec->icccm.take_focus) && (ec->icccm.accepts_focus))
     //e_grabinput_focus(e_client_util_win_get(ec),
                       //E_FOCUS_METHOD_LOCALLY_ACTIVE);
   //else if (!ec->icccm.accepts_focus)
     //e_grabinput_focus(e_client_util_win_get(ec),
                       //E_FOCUS_METHOD_GLOBALLY_ACTIVE);
   //else if (!ec->icccm.take_focus)
     //e_grabinput_focus(e_client_util_win_get(ec), E_FOCUS_METHOD_PASSIVE);

   e_comp_wl->kbd.focus = ec->comp_data->surface;
}

static void
_e_comp_wl_client_cb_focus_unset(void *data EINA_UNUSED, E_Client *ec)
{
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL) return;

   /* send configure */
   if (ec->comp_data->shell.configure_send)
     {
        if (ec->comp_data->shell.surface)
          _e_comp_wl_configure_send(ec, 1);
     }

   if (e_comp_wl->kbd.focus == ec->comp_data->surface)
     e_comp_wl->kbd.focus = NULL;
}

static void
_e_comp_wl_client_cb_move_begin(void *data EINA_UNUSED, E_Client *ec EINA_UNUSED)
{
   e_comp_wl->ptr.button_mask = 0;
}

static void
_e_comp_wl_client_cb_resize_begin(void *data EINA_UNUSED, E_Client *ec)
{
   e_comp_wl->ptr.button_mask = 0;
   if (e_client_has_xwindow(ec)) return;
   e_comp_wl->resize.edges = 0;
   if (ec->keyboard_resizing) return;
   switch (ec->resize_mode)
     {
      case E_POINTER_RESIZE_T: // 1
        e_comp_wl->resize.edges = 1;
        break;
      case E_POINTER_RESIZE_B: // 2
        e_comp_wl->resize.edges = 2;
        break;
      case E_POINTER_RESIZE_L: // 4
        e_comp_wl->resize.edges = 4;
        break;
      case E_POINTER_RESIZE_R: // 8
        e_comp_wl->resize.edges = 8;
        break;
      case E_POINTER_RESIZE_TL: // 5
        e_comp_wl->resize.edges = 5;
        break;
      case E_POINTER_RESIZE_TR: // 9
        e_comp_wl->resize.edges = 9;
        break;
      case E_POINTER_RESIZE_BL: // 6
        e_comp_wl->resize.edges = 6;
        break;
      case E_POINTER_RESIZE_BR: // 10
        e_comp_wl->resize.edges = 10;
        break;
      default:
        break;
     }
   ec->comp_data->shell.configure_send(ec->comp_data->shell.surface,
                                       e_comp_wl->resize.edges,
                                       0, 0);
}

static void
_e_comp_wl_client_cb_resize_end(void *data EINA_UNUSED, E_Client *ec)
{
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_client_has_xwindow(ec)) return;

   e_comp_wl->resize.edges = 0;
   e_comp_wl->resize.resource = NULL;

   ec->comp_data->shell.configure_send(ec->comp_data->shell.surface,
                                       e_comp_wl->resize.edges,
                                       0, 0);

   if (ec->pending_resize)
     {

        ec->changes.pos = EINA_TRUE;
        ec->changes.size = EINA_TRUE;
        EC_CHANGED(ec);
     }

   E_FREE_LIST(ec->pending_resize, free);
}

static void
_e_comp_wl_cb_output_unbind(struct wl_resource *resource)
{
   E_Comp_Wl_Output *output;

   if (!(output = wl_resource_get_user_data(resource))) return;

   output->resources = eina_list_remove(output->resources, resource);
}

static void
_e_comp_wl_cb_output_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
   E_Comp_Wl_Output *output;
   struct wl_resource *resource;

   if (!(output = data)) return;

   resource =
     wl_resource_create(client, &wl_output_interface, version, id);
   if (!resource)
     {
        wl_client_post_no_memory(client);
        return;
     }

   DBG("Bound Output: %s", output->id);
   DBG("\tGeom: %d %d %d %d", output->x, output->y, output->w, output->h);

   output->resources = eina_list_append(output->resources, resource);

   wl_resource_set_implementation(resource, NULL, output,
                                  _e_comp_wl_cb_output_unbind);
   wl_resource_set_user_data(resource, output);

   wl_output_send_geometry(resource, output->x, output->y,
                           output->phys_width, output->phys_height,
                           output->subpixel, output->make ?: "",
                           output->model ?: "", output->transform);

   if (version >= WL_OUTPUT_SCALE_SINCE_VERSION)
     wl_output_send_scale(resource, output->scale);

   /* 3 == preferred + current */
   wl_output_send_mode(resource, 3, output->w, output->h, output->refresh);

   if (version >= WL_OUTPUT_DONE_SINCE_VERSION)
     wl_output_send_done(resource);
}

static Eina_Bool
_e_comp_wl_compositor_create(void)
{
   E_Comp_Wl_Data *cdata;

   /* create new compositor data */
   if (!(cdata = E_NEW(E_Comp_Wl_Data, 1)))
     {
       ERR("Could not create compositor data");
       return EINA_FALSE;
     }

   /* set compositor wayland data */
   e_comp_wl = e_comp->wl_comp_data = cdata;

   /* set wayland log handler */
   /* wl_log_set_handler_server(_e_comp_wl_log_cb_print); */

   /* try to create an ecore_wl2 display */
   cdata->ewd = ecore_wl2_display_create(NULL);
   if (!cdata->ewd)
     {
        ERR("Could not create a Wayland display");
        free(cdata);
        return EINA_FALSE;
     }

   cdata->wl.disp = ecore_wl2_display_get(cdata->ewd);
   if (!cdata->wl.disp)
     {
        ERR("Could not create a Wayland display");
        goto disp_err;
     }

   /* e_env_set("WAYLAND_DISPLAY", name); */

   /* initialize compositor signals */
   wl_signal_init(&cdata->signals.surface.create);
   wl_signal_init(&cdata->signals.surface.activate);
   wl_signal_init(&cdata->signals.surface.kill);

   /* cdata->output.transform = WL_OUTPUT_TRANSFORM_NORMAL; */
   /* cdata->output.scale = e_scale; */

   /* try to add compositor to wayland globals */
   if (!wl_global_create(cdata->wl.disp, &wl_compositor_interface,
                         COMPOSITOR_VERSION, e_comp,
                         _e_comp_wl_compositor_cb_bind))
     {
        ERR("Could not add compositor to wayland globals");
        goto comp_global_err;
     }

   /* try to add subcompositor to wayland globals */
   if (!wl_global_create(cdata->wl.disp, &wl_subcompositor_interface, 1,
                         e_comp, _e_comp_wl_subcompositor_cb_bind))
     {
        ERR("Could not add subcompositor to wayland globals");
        goto comp_global_err;
     }

   if (!e_comp_wl_extensions_init()) goto comp_global_err;

   /* initialize shm mechanism */
   wl_display_init_shm(cdata->wl.disp);

   /* _e_comp_wl_cb_randr_change(NULL, 0, NULL); */

   /* try to init data manager */
   if (!e_comp_wl_data_manager_init())
     {
        ERR("Could not initialize data manager");
        goto data_err;
     }

   /* try to init input */
   if (!e_comp_wl_input_init())
     {
        ERR("Could not initialize input");
        goto input_err;
     }
   e_comp_wl->wl.client_disp = ecore_wl2_display_connect(NULL);

   _e_comp_wl_modules_load();

   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        e_comp_wl_input_pointer_enabled_set(EINA_TRUE);
        e_comp_wl_input_keyboard_enabled_set(EINA_TRUE);
        e_comp_wl_input_touch_enabled_set(EINA_TRUE);
     }

   return EINA_TRUE;

input_err:
   e_comp_wl_data_manager_shutdown();
data_err:
comp_global_err:
   /* e_env_unset("WAYLAND_DISPLAY"); */
/* sock_err: */
   ecore_wl2_display_destroy(cdata->ewd);
disp_err:
   free(cdata);
   return EINA_FALSE;
}

static Eina_Bool
_e_comp_wl_desklock_show(void)
{
   return e_comp_grab_input(1, 1);
}

static void
_e_comp_wl_desklock_hide(void)
{
   e_comp_ungrab_input(1, 1);
}

static void
_e_comp_wl_gl_shutdown(void)
{
   if (!e_comp->gl) return;
   if (e_comp_wl->wl.glapi && e_comp_wl->wl.glapi->evasglUnbindWaylandDisplay)
     e_comp_wl->wl.glapi->evasglUnbindWaylandDisplay(e_comp_wl->wl.gl, e_comp_wl->wl.disp);
   if (e_comp_wl->wl.glsfc)
     evas_gl_surface_destroy(e_comp_wl->wl.gl, e_comp_wl->wl.glsfc);
   if (e_comp_wl->wl.glctx)
     evas_gl_context_destroy(e_comp_wl->wl.gl, e_comp_wl->wl.glctx);
   evas_gl_free(e_comp_wl->wl.gl);
   evas_gl_config_free(e_comp_wl->wl.glcfg);
}

Eina_Bool
e_comp_wl_query_dmabuf_formats(int max_formats, int *formats, int *num_formats)
{
   *num_formats = 0;
#if EVAS_GL_API_VERSION >= 7
   if (!e_comp_wl->wl.glapi->evasglQueryDmaBufFormats) return EINA_FALSE;
   return e_comp_wl->wl.glapi->evasglQueryDmaBufFormats(e_comp_wl->wl.gl, max_formats, formats, num_formats);
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
e_comp_wl_query_dmabuf_modifiers(int format, int max_modifiers, uint64_t *modifiers, Eina_Bool *external_only, int *num_modifiers)
{
   *num_modifiers = 0;
#if EVAS_GL_API_VERSION >= 7
   if (!e_comp_wl->wl.glapi->evasglQueryDmaBufModifiers) return EINA_FALSE;
   return e_comp_wl->wl.glapi->evasglQueryDmaBufModifiers(e_comp_wl->wl.gl, format, max_modifiers, modifiers, external_only, num_modifiers);
#else
   return EINA_FALSE;
#endif
}

static void
_e_comp_wl_gl_init(void)
{
   e_comp_wl->wl.gl = evas_gl_new(ecore_evas_get(e_comp->ee));
   if (!e_comp_wl->wl.gl) return;
   e_comp_wl->wl.glctx = evas_gl_context_create(e_comp_wl->wl.gl, NULL);
   if (!e_comp_wl->wl.glctx) goto end;
   e_comp_wl->wl.glcfg = evas_gl_config_new();
   if (!e_comp_wl->wl.glcfg) goto end;
   e_comp_wl->wl.glsfc = evas_gl_surface_create(e_comp_wl->wl.gl, e_comp_wl->wl.glcfg, 1, 1);
   if (!e_comp_wl->wl.glsfc) goto end;
   if (!evas_gl_make_current(e_comp_wl->wl.gl, e_comp_wl->wl.glsfc, e_comp_wl->wl.glctx)) goto end;
   e_comp_wl->wl.glapi = evas_gl_context_api_get(e_comp_wl->wl.gl, e_comp_wl->wl.glctx);
   if (e_comp_wl->wl.glapi->evasglBindWaylandDisplay)
     e_comp->gl = e_comp_wl->wl.glapi->evasglBindWaylandDisplay(e_comp_wl->wl.gl, e_comp_wl->wl.disp);
   if (e_comp->gl)
     {
        e_util_env_set("ELM_ACCEL", "gl");
        return;
     }
end:
   _e_comp_wl_gl_shutdown();
}

static Eina_Bool
_e_comp_wl_cb_uniconify(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   E_Client *ec = ev->ec;

   if (ec->comp_data->need_xdg_configure)
      ec->comp_data->shell.configure_send(ec->comp_data->shell.surface, 0, 0, 0);
   return ECORE_CALLBACK_PASS_ON;
}

/* public functions */

/**
 * Creates and initializes a Wayland compositor with ecore.
 * Registers callback handlers for keyboard and mouse activity
 * and other client events.
 *
 * @returns true on success, false if initialization failed.
 */
E_API Eina_Bool
e_comp_wl_init(void)
{
   /* try to init ecore_wayland */
   if (!ecore_wl2_init())
     {
        fprintf(stderr, _("Enlightenment cannot initialize Wayland client connection.\n"));
        return EINA_FALSE;
     }

   /* Block session recovery for internal windows */
   ecore_wl2_session_recovery_disable();

   /* set gl available if we have ecore_evas support */
   if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_WAYLAND_EGL) ||
       ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_OPENGL_DRM))
     e_comp_gl_set(EINA_TRUE);

   /* try to create a wayland compositor */
   if (!_e_comp_wl_compositor_create())
     {
        fprintf(stderr, _("Enlightenment cannot create a Wayland Compositor.\n"));
        return EINA_FALSE;
     }

   /* create hash to store clients */
   /* clients_win_hash = eina_hash_int64_new(NULL); */

   /* add event handlers to catch E events */
   if (e_comp->comp_type != E_PIXMAP_TYPE_X)
     {
        if (e_randr2_init() && (!e_xinerama_fake_screens_exist()))
          e_randr2_screens_setup(-1, -1);
        elm_config_preferred_engine_set("wayland_shm");
     }
   e_util_env_set("WAYLAND_DEBUG", "0");
   e_util_env_set("ELM_DISPLAY", "wl");
   if (e_comp_gl_get())
     _e_comp_wl_gl_init();

   E_LIST_HANDLER_APPEND(handlers, E_EVENT_RANDR_CHANGE,
                                _e_comp_wl_cb_randr_change, NULL);

   E_LIST_HANDLER_APPEND(handlers, E_EVENT_COMP_OBJECT_ADD,
                         _e_comp_wl_cb_comp_object_add, NULL);

   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_MOUSE_MOVE,
                         _e_comp_wl_cb_mouse_move, NULL);

   E_LIST_HANDLER_APPEND(handlers, E_EVENT_CLIENT_UNICONIFY,
                         _e_comp_wl_cb_uniconify, NULL);

   /* add hooks to catch e_client events */
   e_client_hook_add(E_CLIENT_HOOK_NEW_CLIENT, _e_comp_wl_client_cb_new, NULL);
   e_client_hook_add(E_CLIENT_HOOK_DEL, _e_comp_wl_client_cb_del, NULL);

   e_client_hook_add(E_CLIENT_HOOK_FOCUS_SET,
                     _e_comp_wl_client_cb_focus_set, NULL);
   e_client_hook_add(E_CLIENT_HOOK_FOCUS_UNSET,
                     _e_comp_wl_client_cb_focus_unset, NULL);

   e_client_hook_add(E_CLIENT_HOOK_MOVE_BEGIN,
                     _e_comp_wl_client_cb_move_begin, NULL);
   e_client_hook_add(E_CLIENT_HOOK_RESIZE_BEGIN,
                     _e_comp_wl_client_cb_resize_begin, NULL);
   e_client_hook_add(E_CLIENT_HOOK_RESIZE_END,
                     _e_comp_wl_client_cb_resize_end, NULL);

   e_desklock_show_hook_add(_e_comp_wl_desklock_show);
   e_desklock_hide_hook_add(_e_comp_wl_desklock_hide);

   E_EVENT_WAYLAND_GLOBAL_ADD = ecore_event_type_new();

   _last_event_time = ecore_loop_time_get();

   return EINA_TRUE;
}

/**
 * Get the signal that is fired for the creation of a Wayland surface.
 *
 * @returns the corresponding Wayland signal
 */
E_API struct wl_signal
e_comp_wl_surface_create_signal_get(void)
{
   return e_comp_wl->signals.surface.create;
}

/* internal functions */
EINTERN void
e_comp_wl_shutdown(void)
{
   E_Comp_Wl_Output *output;

   EINA_LIST_FREE(e_comp_wl->outputs, output)
     {
        if (output->id) eina_stringshare_del(output->id);
        if (output->make) eina_stringshare_del(output->make);
        if (output->model) eina_stringshare_del(output->model);
        free(output);
     }

   /* free handlers */
   E_FREE_LIST(handlers, ecore_event_handler_del);

   /* while (e_comp_wl->wl.globals) */
   /*   { */
   /*      Ecore_Wl_Global *global; */

   /*      global = */
   /*        EINA_INLIST_CONTAINER_GET(e_comp_wl->wl.globals, Ecore_Wl_Global); */

   /*      e_comp_wl->wl.globals = */
   /*        eina_inlist_remove(e_comp_wl->wl.globals, e_comp_wl->wl.globals); */

   /*      free(global->interface); */
   /*      free(global); */
   /*   } */

   if (e_comp_wl->wl.shm) wl_shm_destroy(e_comp_wl->wl.shm);
   _e_comp_wl_gl_shutdown();

   ecore_wl2_display_destroy(e_comp_wl->ewd);

   /* shutdown ecore_wayland */
   ecore_wl2_shutdown();

   /* delete fd handler */
   /* if (e_comp_wl->fd_hdlr) ecore_main_fd_handler_del(e_comp_wl->fd_hdlr); */

   /* free allocated data structure */
   free(e_comp_wl->extensions);
   free(e_comp_wl);
}

EINTERN struct wl_resource *
e_comp_wl_surface_create(struct wl_client *client, int version, uint32_t id)
{
   struct wl_resource *ret = NULL;

   if ((ret = wl_resource_create(client, &wl_surface_interface, version, id)))
     DBG("Created Surface: %d", wl_resource_get_id(ret));

   return ret;
}

EINTERN Eina_Bool
e_comp_wl_surface_commit(E_Client *ec)
{
   _e_comp_wl_surface_state_commit(ec, &ec->comp_data->pending);
   _e_comp_wl_surface_early_frame(ec);
   e_comp_wl_extension_pointer_constraints_commit(ec);

   return EINA_TRUE;
}

EINTERN Eina_Bool
e_comp_wl_subsurface_commit(E_Client *ec)
{
   E_Comp_Wl_Subsurf_Data *sdata;

   /* check for valid subcompositor data */
   if (!(sdata = ec->comp_data->sub.data)) return EINA_FALSE;

   if (_e_comp_wl_subsurface_synchronized_get(sdata))
     _e_comp_wl_subsurface_commit_to_cache(ec);
   else
     {
        E_Client *subc;
        Eina_List *l;

        if (sdata->cached.has_data)
          {
             _e_comp_wl_subsurface_commit_to_cache(ec);
             _e_comp_wl_subsurface_commit_from_cache(ec);
          }
        else
          e_comp_wl_surface_commit(ec);

        EINA_LIST_FOREACH(ec->comp_data->sub.list, l, subc)
          {
             if (ec != subc)
               _e_comp_wl_subsurface_parent_commit(subc, EINA_FALSE);
          }
     }

   return EINA_TRUE;
}

/**
 * Get the buffer for a given resource.
 *
 * Retrieves the Wayland SHM buffer for the resource and
 * uses it to create a new E_Comp_Wl_Buffer object. This
 * buffer will be freed when the resource is destroyed.
 *
 * @param resource that owns the desired buffer
 * @returns a new E_Comp_Wl_Buffer object
 */
E_API E_Comp_Wl_Buffer *
e_comp_wl_buffer_get(struct wl_resource *resource)
{
   E_Comp_Wl_Buffer *buffer;
   struct wl_listener *listener;
   struct wl_shm_buffer *shmbuff;
   struct linux_dmabuf_buffer *dmabuf;

   listener =
     wl_resource_get_destroy_listener(resource, _e_comp_wl_buffer_cb_destroy);
   if (listener)
     return container_of(listener, E_Comp_Wl_Buffer, destroy_listener);

   if (!(buffer = E_NEW(E_Comp_Wl_Buffer, 1))) return NULL;
   shmbuff = wl_shm_buffer_get(resource);
   dmabuf = linux_dmabuf_buffer_get(resource);
   if (shmbuff)
     {
        buffer->w = wl_shm_buffer_get_width(shmbuff);
        buffer->h = wl_shm_buffer_get_height(shmbuff);
     }
   else if (dmabuf)
     {
        buffer->w = dmabuf->attributes.width;
        buffer->h = dmabuf->attributes.height;
     }
   else if (e_comp->gl)
     {
        e_comp_wl->wl.glapi->evasglQueryWaylandBuffer(e_comp_wl->wl.gl, resource, EGL_WIDTH, &buffer->w);
        e_comp_wl->wl.glapi->evasglQueryWaylandBuffer(e_comp_wl->wl.gl, resource, EGL_HEIGHT, &buffer->h);
     }
   buffer->shm_buffer = shmbuff;
   buffer->dmabuf_buffer = dmabuf;

   buffer->resource = resource;
   wl_signal_init(&buffer->destroy_signal);
   buffer->destroy_listener.notify = _e_comp_wl_buffer_cb_destroy;
   wl_resource_add_destroy_listener(resource, &buffer->destroy_listener);

   return buffer;
}

/**
 * Computes the time since the last input event.
 *
 * @returns time in seconds.
 */
E_API double
e_comp_wl_idle_time_get(void)
{
   return (ecore_loop_time_get() - _last_event_time);
}

static E_Comp_Wl_Output *
_e_comp_wl_output_get(Eina_List *outputs, const char *id)
{
   Eina_List *l;
   E_Comp_Wl_Output *output;

   EINA_LIST_FOREACH(outputs, l, output)
     {
       if (!strcmp(output->id, id))
         return output;
     }

   return NULL;
}

/**
 * Initializes information about one display output.
 *
 * Adds or updates the given data about a single display output,
 * with an id matching the provided id.
 *
 * @param id         identification of output to be added or changed
 * @param make       manufacturer name of the display output
 * @param model      model name of the display output
 * @param x          output's top left corner x coordinate
 * @param y          output's top left corner y coordinate
 * @param w          output's width in pixels
 * @param h          output's height in pixels
 * @param pw         output's physical width in millimeters
 * @param ph         output's physical height in millimeters
 * @param refresh    output's refresh rate in Hz
 * @param subpixel   output's subpixel layout
 * @param transform  output's rotation and/or mirror transformation
 *
 * @returns True if a display output object could be added or updated
 */
E_API Eina_Bool
e_comp_wl_output_init(const char *id, const char *make, const char *model,
                      int x, int y, int w, int h, int pw, int ph,
                      unsigned int refresh, unsigned int subpixel,
                      unsigned int transform, unsigned int num)
{
   E_Comp_Wl_Output *output;
   Eina_List *l2;
   struct wl_resource *resource;
   E_Zone *zone;

   /* retrieve named output; or create it if it doesn't exist */
   output = _e_comp_wl_output_get(e_comp_wl->outputs, id);
   if (!output)
     {
        if (e_xinerama_fake_screens_exist())
          zone = e_comp_zone_number_get(num);
        else
          zone = e_zone_for_id_get(id);
        if (!zone) return EINA_FALSE;
        if (!(output = E_NEW(E_Comp_Wl_Output, 1))) return EINA_FALSE;

        if (id) output->id = eina_stringshare_add(id);
        if (make) output->make = eina_stringshare_add(make);
        if (model) output->model = eina_stringshare_add(model);

        e_comp_wl->outputs = eina_list_append(e_comp_wl->outputs, output);

        output->global =
          wl_global_create(e_comp_wl->wl.disp, &wl_output_interface,
                           2, output, _e_comp_wl_cb_output_bind);

        output->resources = NULL;
        output->scale = e_scale;

        zone->output = output;
     }

   /* update the output details */
   output->x = x;
   output->y = y;
   output->w = w;
   output->h = h;
   output->phys_width = pw;
   output->phys_height = ph;
   output->refresh = refresh * 1000;
   output->subpixel = subpixel;
   output->transform = transform;

   if (output->scale <= 0)
     output->scale = e_scale;

   /* if we have bound resources, send updates */
   EINA_LIST_FOREACH(output->resources, l2, resource)
     {
        wl_output_send_geometry(resource,
                                output->x, output->y,
                                output->phys_width,
                                output->phys_height,
                                output->subpixel,
                                output->make ?: "", output->model ?: "",
                                output->transform);

        if (wl_resource_get_version(resource) >= WL_OUTPUT_SCALE_SINCE_VERSION)
          wl_output_send_scale(resource, output->scale);

        /* 3 == preferred + current */
        wl_output_send_mode(resource, 3, output->w, output->h, output->refresh);

        if (wl_resource_get_version(resource) >= WL_OUTPUT_DONE_SINCE_VERSION)
          wl_output_send_done(resource);
     }

   return EINA_TRUE;
}

E_API void
e_comp_wl_output_remove(const char *id)
{
   E_Comp_Wl_Output *output;

   output = _e_comp_wl_output_get(e_comp_wl->outputs, id);
   if (output)
     {
        e_comp_wl->outputs = eina_list_remove(e_comp_wl->outputs, output);

        /* wl_global_destroy(output->global); */

        /* eina_stringshare_del(output->id); */
        /* eina_stringshare_del(output->make); */
        /* eina_stringshare_del(output->model); */

        /* free(output); */
     }
}

EINTERN Eina_Bool
e_comp_wl_key_down(Ecore_Event_Key *ev, E_Client *ec)
{
   uint32_t serial, *end, *k, keycode;

   if ((e_comp->comp_type != E_PIXMAP_TYPE_WL) || (ev->window != e_comp->ee_win)) return EINA_FALSE;
   _last_event_time = ecore_loop_time_get();

   keycode = (ev->keycode - 8);
   if (!(e_comp_wl = e_comp->wl_comp_data)) return EINA_FALSE;

#ifndef E_RELEASE_BUILD
   if ((ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) &&
       ((ev->modifiers & ECORE_EVENT_MODIFIER_ALT) ||
       (ev->modifiers & ECORE_EVENT_MODIFIER_ALTGR)) &&
       eina_streq(ev->key, "BackSpace"))
     {
        if (!e_desklock_state_get()) exit(0);
     }
#endif

   end = (uint32_t *)e_comp_wl->kbd.keys.data + (e_comp_wl->kbd.keys.size / sizeof(*k));

   for (k = e_comp_wl->kbd.keys.data; k < end; k++)
     {
        /* ignore server-generated key repeats */
        if (*k == keycode) return EINA_FALSE;
     }

   e_comp_wl->kbd.keys.size = (const char *)end - (const char *)e_comp_wl->kbd.keys.data;
   if (!(k = wl_array_add(&e_comp_wl->kbd.keys, sizeof(*k))))
     {
        DBG("wl_array_add: Out of memory\n");
        return EINA_FALSE;
     }
   *k = keycode;

   if ((!e_client_action_get()) && (!e_comp->input_key_grabs) &&
       (!e_menu_is_active()))
     {
        if (!ec) ec = e_client_focused_get();
        if (ec && ec->comp_data->surface && e_comp_wl->kbd.focused)
          {
             struct wl_resource *res;
             Eina_List *l;

             serial = wl_display_next_serial(e_comp_wl->wl.disp);
             EINA_LIST_FOREACH(e_comp_wl->kbd.focused, l, res)
               wl_keyboard_send_key(res, serial, ev->timestamp,
                               keycode, WL_KEYBOARD_KEY_STATE_PRESSED);
          }
     }

   /* update modifier state */
   e_comp_wl_input_keyboard_state_update(keycode, EINA_TRUE);
   return !!ec;
}

EINTERN Eina_Bool
e_comp_wl_key_up(Ecore_Event_Key *ev, E_Client *ec)
{
   uint32_t serial, *end, *k, keycode;
   struct wl_resource *res;
   Eina_List *l;

   if ((e_comp->comp_type != E_PIXMAP_TYPE_WL) ||
       (ev->window != e_comp->ee_win)) return EINA_FALSE;

   _last_event_time = ecore_loop_time_get();

   keycode = (ev->keycode - 8);
   if (!(e_comp_wl = e_comp->wl_comp_data)) return EINA_FALSE;

   end = (uint32_t *)e_comp_wl->kbd.keys.data + (e_comp_wl->kbd.keys.size / sizeof(*k));
   for (k = e_comp_wl->kbd.keys.data; k < end; k++)
     {
        if (*k == keycode)
          {
             *k = *--end;
             break;
          }
     }

   e_comp_wl->kbd.keys.size =
     (const char *)end - (const char *)e_comp_wl->kbd.keys.data;

   if ((!e_client_action_get()) && (!e_comp->input_key_grabs) &&
       (!e_menu_is_active()))
     {
        if (!ec) ec = e_client_focused_get();

        if (e_comp_wl->kbd.focused)
          {
             serial = wl_display_next_serial(e_comp_wl->wl.disp);
             EINA_LIST_FOREACH(e_comp_wl->kbd.focused, l, res)
               wl_keyboard_send_key(res, serial, ev->timestamp,
                                    keycode, WL_KEYBOARD_KEY_STATE_RELEASED);
          }
     }

   /* update modifier state */
   e_comp_wl_input_keyboard_state_update(keycode, EINA_FALSE);
   return !!ec;
}

E_API Eina_Bool
e_comp_wl_evas_handle_mouse_button(E_Client *ec, uint32_t timestamp, uint32_t button_id, uint32_t state)
{
   Eina_List *l;
   struct wl_client *wc;
   uint32_t serial, btn, *state_serial;
   struct wl_resource *res;

   if (ec->cur_mouse_action || ec->border_menu || e_comp_wl->drag)
     return EINA_FALSE;
   if (e_object_is_del(E_OBJECT(ec))) return EINA_FALSE;
   if (ec->ignored) return EINA_FALSE;
   if (!ec->mouse.in) return EINA_FALSE;

   switch (button_id)
     {
      case 1:
        btn = BTN_LEFT;
        break;
      case 2:
        btn = BTN_MIDDLE;
        break;
      case 3:
        btn = BTN_RIGHT;
        break;
      case 4:
        btn = BTN_SIDE;
        break;
      case 5:
        btn = BTN_EXTRA;
        break;
      case 6:
        btn = BTN_FORWARD;
        break;
      case 7:
        btn = BTN_BACK;
        break;
      default:
        btn = button_id + BTN_SIDE - 8;
        break;
     }

   if (state == WL_POINTER_BUTTON_STATE_PRESSED)
     {
        e_comp_wl->ptr.button_mask |= 1 << button_id;
        state_serial = &e_comp_wl->ptr.serial[0];
     }
   else
     {
        /* reject release events if button is not pressed */
        if (!(e_comp_wl->ptr.button_mask & (1 << button_id))) return EINA_FALSE;
        e_comp_wl->ptr.button_mask &= ~(1 << button_id);
        state_serial = &e_comp_wl->ptr.serial[1];
     }
   e_comp_wl->ptr.button = btn;

   if (!ec->comp_data->surface) return EINA_FALSE;

   if (!eina_list_count(e_comp_wl->ptr.resources))
     return EINA_TRUE;

   wc = wl_resource_get_client(ec->comp_data->surface);
   *state_serial = serial = wl_display_next_serial(e_comp_wl->wl.disp);

   EINA_LIST_FOREACH(e_comp_wl->ptr.resources, l, res)
     {
        if (wl_resource_get_client(res) != wc) continue;
        if (!e_comp_wl_input_pointer_check(res)) continue;
        wl_pointer_send_button(res, serial, timestamp, btn, state);
     }
   return EINA_TRUE;
}

EINTERN void
e_comp_wl_xwayland_client_queue(E_Client *ec)
{
   e_comp_wl->xwl_pending = eina_list_append(e_comp_wl->xwl_pending, ec);
}

E_API void
e_comp_wl_grab_client_add(E_Client *ec, E_Comp_Wl_Grab_End_Cb cb)
{
   E_Client *gec, *parent = e_client_util_top_parent_get(ec);
   E_Comp_Wl_Grab_End_Cb grabcb;

   if (grab_clients && (parent != e_client_util_top_parent_get(eina_list_data_get(grab_clients))))
     {
        /* dismiss grabs in order when grabbing from new toplevel */
        EINA_LIST_FREE(grab_clients, gec)
          {
             grabcb = eina_list_data_get(grab_cbs);
             if (grabcb) grabcb(gec);
             grab_cbs = eina_list_remove_list(grab_cbs, grab_cbs);
          }
     }
   grab_clients = eina_list_prepend(grab_clients, ec);
   grab_cbs = eina_list_prepend(grab_cbs, cb);
   ec->comp_data->grab = 1;
   if (eina_list_count(grab_clients) == 1) e_bindings_disabled_set(1);
}

E_API void
e_comp_wl_grab_client_del(E_Client *ec, Eina_Bool dismiss)
{
   Eina_List *l, *ll;
   E_Client *gec;
   int n = -1;
   E_Comp_Wl_Grab_End_Cb cb;

   EINA_LIST_FOREACH(grab_clients, l, gec)
     {
        n++;
        if (gec != ec) continue;
        ll = eina_list_nth_list(grab_cbs, n);
        cb = eina_list_data_get(ll);
        if (dismiss && cb) cb(gec);
        grab_cbs = eina_list_remove_list(grab_cbs, ll);
        grab_clients = eina_list_remove_list(grab_clients, l);
        break;
     }
   if (!grab_clients)
     e_bindings_disabled_set(0);
}

E_API Eina_Bool
e_comp_wl_client_is_grabbed(const E_Client *ec)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ec, EINA_FALSE);
   if (e_object_is_del(E_OBJECT(ec))) return EINA_FALSE;
   return ec->comp_data->grab && evas_object_visible_get(ec->frame);
}

static Eina_Bool
_check_grab_coords(E_Client *ec, int x, int y)
{
   E_Client *tec, *pec;

   E_CLIENT_REVERSE_FOREACH(tec)
     {
        if (!e_comp_object_coords_inside_input_area(tec->frame, x, y)) continue;
        if (tec == ec) return EINA_TRUE;
        pec = ec->parent;
        while (pec)
          {
             if (tec == pec) return EINA_TRUE;
             pec = pec->parent;
          }
        return EINA_FALSE;
     }
   return EINA_FALSE;
}

E_API Eina_Bool
e_comp_wl_grab_client_mouse_move(const Ecore_Event_Mouse_Move *ev)
{
   E_Client *ec;
   Evas_Object *o;

   if (e_comp->comp_type != E_PIXMAP_TYPE_WL) return ECORE_CALLBACK_RENEW;
   ec = e_client_focused_get();
   if (!ec) return ECORE_CALLBACK_RENEW;
   if (!e_client_util_is_popup(ec)) return ECORE_CALLBACK_RENEW;
   if (!e_comp_wl_client_is_grabbed(ec)) return ECORE_CALLBACK_RENEW;
   /* reject mouse moves from outside the popup */
   if (_check_grab_coords(ec, ev->x, ev->y)) return ECORE_CALLBACK_RENEW;
   /* manually move the pointer since we're about to block the event globally */
   ecore_evas_cursor_get(e_comp->ee, &o, NULL, NULL, NULL);
   evas_object_move(o, ev->x, ev->y);
   return ECORE_CALLBACK_DONE;
}

E_API Eina_Bool
e_comp_wl_grab_client_mouse_button(const Ecore_Event_Mouse_Button *ev)
{
   E_Client *ec;

   if (e_comp->comp_type != E_PIXMAP_TYPE_WL) return ECORE_CALLBACK_RENEW;
   ec = e_client_focused_get();
   if (!ec) return ECORE_CALLBACK_RENEW;
   if (!e_client_util_is_popup(ec)) return ECORE_CALLBACK_RENEW;
   if (!e_comp_wl_client_is_grabbed(ec)) return ECORE_CALLBACK_RENEW;
   /* reject mouse moves from outside the popup */
   if (_check_grab_coords(ec, ev->x, ev->y)) return ECORE_CALLBACK_RENEW;
   e_comp_wl_grab_client_del(ec, 1);
   while (grab_clients)
     e_comp_wl_grab_client_del(eina_list_last_data_get(grab_clients), 1);
   evas_event_feed_mouse_move(e_comp->evas, ev->x, ev->y, 0, NULL);
   e_comp_canvas_feed_mouse_up(0);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool saver_inhibit = EINA_FALSE;
static Eina_Bool saver_on = EINA_FALSE;
static Ecore_Timer *screensaver_eval_timer = NULL;
static Ecore_Timer *screensaver_idle_timer = NULL;

static Eina_Bool
_e_comp_wl_screensaver_eval_cb(void *d EINA_UNUSED)
{
   screensaver_eval_timer = NULL;
   if (!saver_on)
     {
        if (e_comp->screen && e_comp->screen->dpms)
          e_comp->screen->dpms(0);
     }
   e_screensaver_eval(saver_on);
   return EINA_FALSE;
}

static Eina_Bool
_e_comp_wl_screensaver_idle_cb(void *data EINA_UNUSED)
{
   screensaver_idle_timer = NULL;
   if (e_screensaver_ignore_get()) return EINA_FALSE;
   if (!saver_on)
     {
        saver_on = EINA_TRUE;
        E_FREE_FUNC(screensaver_eval_timer, ecore_timer_del);
        screensaver_eval_timer = ecore_timer_loop_add
          (0.3, _e_comp_wl_screensaver_eval_cb, NULL);
     }
   return EINA_FALSE;
}

static void
_e_comp_cb_pointer_suspend_resume_done(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   edje_object_signal_callback_del(obj, emission, source,
                                   _e_comp_cb_pointer_suspend_resume_done);
   if (!data)
     {
        e_pointer_grab_set(e_comp->pointer, EINA_FALSE);
     }
}

EINTERN Eina_Bool
_e_comp_wl_screensaver_on()
{
   const char *s;

   if ((!e_comp->pointer) || (!e_comp->pointer->o_ptr)) return ECORE_CALLBACK_RENEW;
   s = edje_object_data_get(e_comp->pointer->o_ptr, "can_suspend");

   if ((s) && (atoi(s) == 1))
     {
        if (!e_desklock_state_get())
          {
             e_pointer_grab_set(e_comp->pointer, EINA_FALSE);
             e_pointer_grab_set(e_comp->pointer, EINA_TRUE);
          }
        edje_object_signal_callback_del(e_comp->pointer->o_ptr,
                                        "e,state,mouse,suspend,done", "e",
                                        _e_comp_cb_pointer_suspend_resume_done);
        edje_object_signal_callback_del(e_comp->pointer->o_ptr,
                                        "e,state,mouse,resume,done", "e",
                                        _e_comp_cb_pointer_suspend_resume_done);
        edje_object_signal_callback_add(e_comp->pointer->o_ptr,
                                        "e,state,mouse,suspend,done",
                                        "e",
                                        _e_comp_cb_pointer_suspend_resume_done,
                                        e_comp);
        edje_object_signal_emit(e_comp->pointer->o_ptr,
                                "e,state,mouse,suspend", "e");
     }
   return ECORE_CALLBACK_RENEW;
}

EINTERN Eina_Bool
_e_comp_wl_screensaver_off()
{
   const char *s;

   e_pointer_grab_set(e_comp->pointer, EINA_FALSE);
   if ((!e_comp->pointer) || (!e_comp->pointer->o_ptr)) return ECORE_CALLBACK_RENEW;
   s = edje_object_data_get(e_comp->pointer->o_ptr, "can_suspend");

   if ((s) && (atoi(s) == 1))
     {
        if (!e_desklock_state_get())
          {
             e_pointer_grab_set(e_comp->pointer, EINA_TRUE);
          }
        edje_object_signal_callback_del(e_comp->pointer->o_ptr,
                                        "e,state,mouse,suspend,done", "e",
                                        _e_comp_cb_pointer_suspend_resume_done);
        edje_object_signal_callback_del(e_comp->pointer->o_ptr,
                                        "e,state,mouse,resume,done", "e",
                                        _e_comp_cb_pointer_suspend_resume_done);
        edje_object_signal_callback_add(e_comp->pointer->o_ptr,
                                        "e,state,mouse,resume,done",
                                        "e",
                                        _e_comp_cb_pointer_suspend_resume_done,
                                        NULL);
        edje_object_signal_emit(e_comp->pointer->o_ptr,
                                "e,state,mouse,resume", "e");
     }
   return ECORE_CALLBACK_RENEW;
}

E_API void
e_comp_wl_notidle(void)
{
   int timeout;

   if (saver_on)
     {
        saver_on = EINA_FALSE;
        E_FREE_FUNC(screensaver_eval_timer, ecore_timer_del);
        screensaver_eval_timer = ecore_timer_loop_add
          (0.3, _e_comp_wl_screensaver_eval_cb, NULL);
     }
   E_FREE_FUNC(screensaver_idle_timer, ecore_timer_del);
   timeout = e_screensaver_timeout_get(EINA_TRUE);
   if (timeout > 0)
     screensaver_idle_timer = ecore_timer_add
       (timeout, _e_comp_wl_screensaver_idle_cb, NULL);
}

E_API void
e_comp_wl_screensaver_activate(void)
{
   saver_on = EINA_TRUE;
   E_FREE_FUNC(screensaver_eval_timer, ecore_timer_del);
   E_FREE_FUNC(screensaver_idle_timer, ecore_timer_del);
   e_screensaver_eval(saver_on);
}

E_API void
e_comp_wl_screensaver_inhibit(Eina_Bool inhibit)
{
   if (inhibit == saver_inhibit) return;
   saver_inhibit = inhibit;
   e_comp_wl_notidle();
   if (inhibit)
     E_FREE_FUNC(screensaver_idle_timer, ecore_timer_del);
}
