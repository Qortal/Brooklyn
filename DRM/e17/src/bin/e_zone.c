#include "e.h"

/* E_Zone is a child object of E_Comp. There is one zone per screen
 * in a xinerama setup. Each zone has one or more desktops.
 */

static void        _e_zone_free(E_Zone *zone);
static void        _e_zone_cb_bg_mouse_down(void *data,
                                            Evas *evas,
                                            Evas_Object *obj,
                                            void *event_info);
static void        _e_zone_cb_bg_mouse_up(void *data,
                                          Evas *evas,
                                          Evas_Object *obj,
                                          void *event_info);
static Eina_Bool   _e_zone_cb_edge_timer(void *data);
static void        _e_zone_event_generic_free(void *data, void *ev);
static void        _e_zone_object_del_attach(void *o);
static E_Zone_Edge _e_zone_detect_edge(E_Zone *zone, Evas_Object *obj);
static void        _e_zone_edge_move_resize(E_Zone *zone);
static void _e_zone_obstacle_free(E_Zone_Obstacle *obs);

E_API int E_EVENT_ZONE_DESK_COUNT_SET = 0;
E_API int E_EVENT_POINTER_WARP = 0;
E_API int E_EVENT_ZONE_USEFUL_GEOMETRY_CHANGED = 0;
E_API int E_EVENT_ZONE_MOVE_RESIZE = 0;
E_API int E_EVENT_ZONE_ADD = 0;
E_API int E_EVENT_ZONE_DEL = 0;
E_API int E_EVENT_ZONE_EDGE_IN = 0;
E_API int E_EVENT_ZONE_EDGE_OUT = 0;
E_API int E_EVENT_ZONE_EDGE_MOVE = 0;
E_API int E_EVENT_ZONE_STOW = 0;
E_API int E_EVENT_ZONE_UNSTOW = 0;

#define E_ZONE_FLIP_LEFT(zone)  (((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1)) || ((zone)->desk_x_current > 0)) && (zone)->edge.left)
#define E_ZONE_FLIP_RIGHT(zone) (((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1)) || (((zone)->desk_x_current + 1) < (zone)->desk_x_count)) && (zone)->edge.right)
#define E_ZONE_FLIP_UP(zone)    (((e_config->desk_flip_wrap && ((zone)->desk_y_count > 1)) || ((zone)->desk_y_current > 0)) && (zone)->edge.top)
#define E_ZONE_FLIP_DOWN(zone)  (((e_config->desk_flip_wrap && ((zone)->desk_y_count > 1)) || (((zone)->desk_y_current + 1) < (zone)->desk_y_count)) && (zone)->edge.bottom)

#define E_ZONE_CORNER_RATIO 0.025;

EINTERN int
e_zone_init(void)
{
   E_EVENT_ZONE_DESK_COUNT_SET = ecore_event_type_new();
   E_EVENT_POINTER_WARP = ecore_event_type_new();
   E_EVENT_ZONE_MOVE_RESIZE = ecore_event_type_new();
   E_EVENT_ZONE_USEFUL_GEOMETRY_CHANGED = ecore_event_type_new();
   E_EVENT_ZONE_ADD = ecore_event_type_new();
   E_EVENT_ZONE_DEL = ecore_event_type_new();
   E_EVENT_ZONE_EDGE_IN = ecore_event_type_new();
   E_EVENT_ZONE_EDGE_OUT = ecore_event_type_new();
   E_EVENT_ZONE_EDGE_MOVE = ecore_event_type_new();
   E_EVENT_ZONE_STOW = ecore_event_type_new();
   E_EVENT_ZONE_UNSTOW = ecore_event_type_new();
   return 1;
}

EINTERN int
e_zone_shutdown(void)
{
   return 1;
}

E_API void
e_zone_all_edge_flip_eval(void)
{
   const Eina_List *l;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     e_zone_edge_flip_eval(zone);
}

static void
_e_zone_cb_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_In *ev = event_info;
   E_Event_Zone_Edge *zev;
   E_Zone_Edge edge;
   E_Zone *zone = data;

   if (!ev->timestamp) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   edge = _e_zone_detect_edge(zone, obj);
   if (edge == E_ZONE_EDGE_NONE) return;

   zev = E_NEW(E_Event_Zone_Edge, 1);
   zev->zone = zone;
   zev->edge = edge;
   zev->x = ev->output.x;
   zev->y = ev->output.y;
   zev->modifiers = e_bindings_evas_modifiers_convert(ev->modifiers);
   zev->drag = !!evas_pointer_button_down_mask_get(e_comp->evas);

   ecore_event_add(E_EVENT_ZONE_EDGE_IN, zev, NULL, NULL);
   if (e_bindings_edge_in_event_handle(E_BINDING_CONTEXT_ZONE, E_OBJECT(zone), zev))
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
_e_zone_cb_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Out *ev = event_info;
   E_Event_Zone_Edge *zev;
   E_Zone_Edge edge;
   E_Zone *zone = data;

   if (!ev->timestamp) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   edge = _e_zone_detect_edge(zone, obj);
   if (edge == E_ZONE_EDGE_NONE) return;

   zev = E_NEW(E_Event_Zone_Edge, 1);
   zev->zone = zone;
   zev->edge = edge;
   zev->x = ev->output.x;
   zev->y = ev->output.y;
   zev->modifiers = e_bindings_evas_modifiers_convert(ev->modifiers);
   zev->drag = !!evas_pointer_button_down_mask_get(e_comp->evas);

   ecore_event_add(E_EVENT_ZONE_EDGE_OUT, zev, NULL, NULL);
   if (e_bindings_edge_out_event_handle(E_BINDING_CONTEXT_ZONE, E_OBJECT(zone), zev))
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
_e_zone_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   E_Event_Zone_Edge *zev;
   E_Zone_Edge edge;
   E_Zone *zone = data;

   if (!ev->timestamp) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   edge = _e_zone_detect_edge(zone, obj);
   if (edge == E_ZONE_EDGE_NONE) return;

   zev = E_NEW(E_Event_Zone_Edge, 1);
   zev->zone = zone;
   zev->edge = edge;
   zev->x = ev->output.x;
   zev->y = ev->output.y;
   zev->button = ev->button;
   zev->modifiers = e_bindings_evas_modifiers_convert(ev->modifiers);
   ecore_event_add(E_EVENT_ZONE_EDGE_OUT, zev, NULL, NULL);
   if (e_bindings_edge_down_event_handle(E_BINDING_CONTEXT_ZONE, E_OBJECT(zone), zev))
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
_e_zone_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   E_Event_Zone_Edge *zev;
   E_Zone_Edge edge;
   E_Zone *zone = data;

   if (!ev->timestamp) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   edge = _e_zone_detect_edge(zone, obj);
   if (edge == E_ZONE_EDGE_NONE) return;

   zev = E_NEW(E_Event_Zone_Edge, 1);
   zev->zone = zone;
   zev->edge = edge;
   zev->x = ev->output.x;
   zev->y = ev->output.y;
   zev->button = ev->button;
   zev->modifiers = e_bindings_evas_modifiers_convert(ev->modifiers);
   ecore_event_add(E_EVENT_ZONE_EDGE_OUT, zev, NULL, NULL);
   if (e_bindings_edge_up_event_handle(E_BINDING_CONTEXT_ZONE, E_OBJECT(zone), zev))
     ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
_e_zone_cb_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   E_Event_Zone_Edge *zev;
   E_Zone_Edge edge;
   E_Zone *zone = data;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   edge = _e_zone_detect_edge(zone, obj);
   if (edge == E_ZONE_EDGE_NONE) return;

   zev = E_NEW(E_Event_Zone_Edge, 1);
   zev->zone = zone;
   zev->edge = edge;
   zev->x = ev->cur.output.x;
   zev->y = ev->cur.output.y;
   zev->modifiers = e_bindings_evas_modifiers_convert(ev->modifiers);
   ecore_event_add(E_EVENT_ZONE_EDGE_MOVE, zev, NULL, NULL);
}

E_API E_Zone *
e_zone_new(int num, int id, int x, int y, int w, int h)
{
   E_Zone *zone;
   Evas_Object *o;
   E_Event_Zone_Add *ev;
   char name[40];

   zone = E_OBJECT_ALLOC(E_Zone, E_ZONE_TYPE, _e_zone_free);
   if (!zone) return NULL;

   zone->x = x;
   zone->y = y;
   zone->w = w;
   zone->h = h;
   zone->num = num;
   zone->id = id;

   zone->useful_geometry_dirty = 1;
   zone->useful_geometry[0].x = zone->useful_geometry[0].y =
     zone->useful_geometry[0].w = zone->useful_geometry[0].h = -1;
   zone->useful_geometry[1].x = zone->useful_geometry[1].y =
     zone->useful_geometry[1].w = zone->useful_geometry[1].h = -1;

   //printf("@@@@@@@@@@ e_zone_new: %i %i | %i %i %ix%i = %p\n", num, id, x, y, w, h, zone);

   snprintf(name, sizeof(name), "Zone %d", zone->num);
   zone->name = eina_stringshare_add(name);

   e_comp->zones = eina_list_append(e_comp->zones, zone);

   o = evas_object_rectangle_add(e_comp->evas);
   zone->bg_clip_object = o;
   evas_object_repeat_events_set(o, 1);
   evas_object_layer_set(o, E_LAYER_BG);
   evas_object_name_set(o, "zone->bg_clip_object");
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_show(o);

   o = evas_object_rectangle_add(e_comp->evas);
   zone->bg_event_object = o;
   evas_object_name_set(o, "zone->bg_event_object");
   evas_object_clip_set(o, zone->bg_clip_object);
   evas_object_layer_set(o, E_LAYER_BG);
   evas_object_repeat_events_set(o, 1);
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_repeat_events_set(o, 1);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_zone_cb_bg_mouse_down, zone);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _e_zone_cb_bg_mouse_up, zone);

   /* TODO: config the ecore_evas type. */

   zone->desk_x_count = 0;
   zone->desk_y_count = 0;
   zone->desk_x_current = 0;
   zone->desk_y_current = 0;
   e_zone_desk_count_set(zone, e_config->zone_desks_x_count,
                         e_config->zone_desks_y_count);

   e_object_del_attach_func_set(E_OBJECT(zone), _e_zone_object_del_attach);

   e_zone_all_edge_flip_eval();

   if (starting) return zone;

   ev = E_NEW(E_Event_Zone_Add, 1);
   ev->zone = zone;
   e_object_ref(E_OBJECT(ev->zone));
   ecore_event_add(E_EVENT_ZONE_ADD, ev, _e_zone_event_generic_free, NULL);

   return zone;
}

E_API void
e_zone_name_set(E_Zone *zone,
                const char *name)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   if (zone->name) eina_stringshare_del(zone->name);
   zone->name = eina_stringshare_add(name);
}

static void
e_zone_reconfigure_clients(E_Zone *zone, int dx, int dy)
{
   E_Client *ec;

   E_CLIENT_FOREACH(ec)
     {
        if (ec->zone != zone) continue;

        if ((dx != 0) || (dy != 0))
          evas_object_move(ec->frame, ec->x + dx, ec->y + dy);
     }
}

E_API void
e_zone_move(E_Zone *zone,
            int x,
            int y)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   e_zone_move_resize(zone, x, y, zone->w, zone->h);
}

E_API void
e_zone_resize(E_Zone *zone,
              int w,
              int h)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   e_zone_move_resize(zone, zone->x, zone->y, w, h);
}

E_API Eina_Bool
e_zone_move_resize(E_Zone *zone,
                   int x,
                   int y,
                   int w,
                   int h)
{
   E_Event_Zone_Move_Resize *ev;
   int dx, dy;

   E_OBJECT_CHECK_RETURN(zone, EINA_FALSE);
   E_OBJECT_TYPE_CHECK_RETURN(zone, E_ZONE_TYPE, EINA_FALSE);

   if ((x == zone->x) && (y == zone->y) && (w == zone->w) && (h == zone->h))
     return EINA_FALSE;

   dx = x - zone->x;
   dy = y - zone->y;
   zone->x = x;
   zone->y = y;
   zone->w = w;
   zone->h = h;

   evas_object_move(zone->bg_object, x, y);
   evas_object_move(zone->bg_event_object, x, y);
   evas_object_move(zone->bg_clip_object, x, y);
   evas_object_resize(zone->bg_object, w, h);
   evas_object_resize(zone->bg_event_object, w, h);
   evas_object_resize(zone->bg_clip_object, w, h);

   ev = E_NEW(E_Event_Zone_Move_Resize, 1);
   ev->zone = zone;
   e_object_ref(E_OBJECT(ev->zone));
   ecore_event_add(E_EVENT_ZONE_MOVE_RESIZE, ev,
                   _e_zone_event_generic_free, NULL);

   _e_zone_edge_move_resize(zone);
   e_zone_bg_reconfigure(zone);
   e_zone_reconfigure_clients(zone, dx, dy);
   e_zone_useful_geometry_dirty(zone);
   return EINA_TRUE;
}

E_API E_Zone *
e_zone_current_get(void)
{
   Eina_List *l = NULL;
   E_Zone *zone;

   if (!e_comp) return NULL;
   if (!starting)
     {
        int x, y;

        ecore_evas_pointer_xy_get(e_comp->ee, &x, &y);
        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          {
             if (E_INSIDE(x, y, zone->x, zone->y, zone->w, zone->h))
               return zone;
          }
     }
   if (!e_comp->zones) return NULL;
   return eina_list_data_get(e_comp->zones);
}

E_API void
e_zone_bg_reconfigure(E_Zone *zone)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   e_bg_zone_update(zone, E_BG_TRANSITION_CHANGE);
}

E_API void
e_zone_flip_coords_handle(E_Zone *zone,
                          int x,
                          int y)
{
   E_Event_Zone_Edge *zev;
   E_Binding_Edge *binding;
   E_Zone_Edge edge;
   Eina_List *l;
   E_Shelf *es;
   int ok = 0;
   int one_row = 1;
   int one_col = 1;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   if (zone->flip.switching)
     {
        int cx, cy, w, h;

        switch (zone->flip.switching)
          {
           case E_ZONE_EDGE_LEFT:
             evas_object_geometry_get(zone->edge.left, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               zone->flip.switching = E_ZONE_EDGE_NONE;
             break;
           case E_ZONE_EDGE_RIGHT:
             evas_object_geometry_get(zone->edge.right, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               zone->flip.switching = E_ZONE_EDGE_NONE;
             break;
           case E_ZONE_EDGE_TOP:
             evas_object_geometry_get(zone->edge.top, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               zone->flip.switching = E_ZONE_EDGE_NONE;
             break;
           case E_ZONE_EDGE_BOTTOM:
             evas_object_geometry_get(zone->edge.bottom, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               zone->flip.switching = E_ZONE_EDGE_NONE;
             break;

           case E_ZONE_EDGE_TOP_LEFT:
             evas_object_geometry_get(zone->corner.left_top, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               {
                  evas_object_geometry_get(zone->corner.top_left, &cx, &cy, &w, &h);
                  if (!E_INSIDE(x, y, cx, cy, w, h))
                    zone->flip.switching = E_ZONE_EDGE_NONE;
               }
             break;
           case E_ZONE_EDGE_TOP_RIGHT:
             evas_object_geometry_get(zone->corner.right_top, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               {
                  evas_object_geometry_get(zone->corner.top_right, &cx, &cy, &w, &h);
                  if (!E_INSIDE(x, y, cx, cy, w, h))
                    zone->flip.switching = E_ZONE_EDGE_NONE;
               }
             break;
           case E_ZONE_EDGE_BOTTOM_RIGHT:
             evas_object_geometry_get(zone->corner.right_bottom, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               {
                  evas_object_geometry_get(zone->corner.bottom_right, &cx, &cy, &w, &h);
                  if (!E_INSIDE(x, y, cx, cy, w, h))
                    zone->flip.switching = E_ZONE_EDGE_NONE;
               }
             break;
           case E_ZONE_EDGE_BOTTOM_LEFT:
             evas_object_geometry_get(zone->corner.left_bottom, &cx, &cy, &w, &h);
             if (!E_INSIDE(x, y, cx, cy, w, h))
               {
                  evas_object_geometry_get(zone->corner.bottom_left, &cx, &cy, &w, &h);
                  if (!E_INSIDE(x, y, cx, cy, w, h))
                    zone->flip.switching = E_ZONE_EDGE_NONE;
               }
             break;
             default: break;
          }
        if (zone->flip.switching) return;
     }

   if (!e_config->edge_flip_dragging) return;
   /* if we have only 1 row we can flip up/down even if we have xinerama */
   if (eina_list_count(e_comp->zones) > 1)
     {
        Eina_List *zones;
        E_Zone *next_zone;
        int cx, cy;

        zones = e_comp->zones;
        next_zone = (E_Zone *)eina_list_data_get(zones);
        cx = next_zone->x;
        cy = next_zone->y;
        zones = eina_list_next(zones);
        EINA_LIST_FOREACH(eina_list_next(zones), zones, next_zone)
          {
             if (next_zone->x != cx) one_col = 0;
             if (next_zone->y != cy) one_row = 0;
          }
     }
   if (!E_INSIDE(x, y, zone->x, zone->y, zone->w, zone->h))
     goto noflip;
   if ((one_row) && (y == 0))
     edge = E_ZONE_EDGE_TOP;
   else if ((one_col) && (x == (zone->w - 1)))
     edge = E_ZONE_EDGE_RIGHT;
   else if ((one_row) && (y == (zone->h - 1)))
     edge = E_ZONE_EDGE_BOTTOM;
   else if ((one_col) && (x == 0))
     edge = E_ZONE_EDGE_LEFT;
   else
     {
noflip:
        if (zone->flip.es)
          e_shelf_toggle(zone->flip.es, 0);
        zone->flip.es = NULL;
        return;
     }
   EINA_LIST_FOREACH(e_shelf_list(), l, es)
     {
        if (es->zone != zone) continue;
        switch (es->gadcon->orient)
          {
           case E_GADCON_ORIENT_TOP:
           case E_GADCON_ORIENT_CORNER_TL:
           case E_GADCON_ORIENT_CORNER_TR:
             if (edge == E_ZONE_EDGE_TOP) ok = 1;
             break;

           case E_GADCON_ORIENT_BOTTOM:
           case E_GADCON_ORIENT_CORNER_BL:
           case E_GADCON_ORIENT_CORNER_BR:
             if (edge == E_ZONE_EDGE_BOTTOM) ok = 1;
             break;

           case E_GADCON_ORIENT_LEFT:
           case E_GADCON_ORIENT_CORNER_LT:
           case E_GADCON_ORIENT_CORNER_LB:
             if (edge == E_ZONE_EDGE_LEFT) ok = 1;
             break;

           case E_GADCON_ORIENT_RIGHT:
           case E_GADCON_ORIENT_CORNER_RT:
           case E_GADCON_ORIENT_CORNER_RB:
             if (edge == E_ZONE_EDGE_RIGHT) ok = 1;
             break;

           default:
             ok = 0;
             break;
          }

        if (!ok) continue;
        if (!E_INSIDE(x, y, es->x, es->y, es->w, es->h))
          continue;

        if (zone->flip.es)
          e_shelf_toggle(zone->flip.es, 0);

        zone->flip.es = es;
        e_shelf_toggle(es, 1);
     }
   ok = 0;
   switch (edge)
     {
      case E_ZONE_EDGE_LEFT:
        if (E_ZONE_FLIP_LEFT(zone)) ok = 1;
        break;

      case E_ZONE_EDGE_TOP:
        if (E_ZONE_FLIP_UP(zone)) ok = 1;
        break;

      case E_ZONE_EDGE_RIGHT:
        if (E_ZONE_FLIP_RIGHT(zone)) ok = 1;
        break;

      case E_ZONE_EDGE_BOTTOM:
        if (E_ZONE_FLIP_DOWN(zone)) ok = 1;
        break;

      default:
        ok = 0;
        break;
     }
   if (!ok) return;
   binding = e_bindings_edge_get("desk_flip_in_direction", edge, 0);
   if (!binding) binding = e_bindings_edge_get("desk_flip_by", edge, 0);
   if (binding && (!binding->timer))
     {
        zev = E_NEW(E_Event_Zone_Edge, 1);
        zev->zone = zone;
        zev->x = x;
        zev->y = y;
        zev->edge = edge;
        zone->flip.ev = zev;
        zone->flip.bind = binding;
        zone->flip.switching = edge;
        binding->timer = ecore_timer_loop_add(((double)binding->delay), _e_zone_cb_edge_timer, zone);
     }
}

E_API void
e_zone_desk_count_set(E_Zone *zone,
                      int x_count,
                      int y_count)
{
   E_Desk **new_desks;
   E_Desk *desk, *new_desk;
   E_Client *ec;
   E_Event_Zone_Desk_Count_Set *ev;
   int x, y, xx, yy, moved, nx, ny;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   xx = x_count;
   if (xx < 1) xx = 1;
   yy = y_count;
   if (yy < 1) yy = 1;

   /* Orphaned window catcher; in case desk count gets reset */
   moved = 0;
   if (zone->desk_x_current >= xx) moved = 1;
   if (zone->desk_y_current >= yy) moved = 1;
   if (moved)
     {
        nx = zone->desk_x_current;
        ny = zone->desk_y_current;
        if (zone->desk_x_current >= xx) nx = xx - 1;
        if (zone->desk_y_current >= yy) ny = yy - 1;
        e_desk_show(e_desk_at_xy_get(zone, nx, ny));
     }

   new_desks = malloc(xx * yy * sizeof(E_Desk *));
   for (x = 0; x < xx; x++)
     {
        for (y = 0; y < yy; y++)
          {
             if ((x < zone->desk_x_count) && (y < zone->desk_y_count))
               desk = zone->desks[x + (y * zone->desk_x_count)];
             else
               desk = e_desk_new(zone, x, y);
             new_desks[x + (y * xx)] = desk;
          }
     }

   /* catch windows that have fallen off the end if we got smaller */
   if (xx < zone->desk_x_count)
     {
        for (y = 0; y < zone->desk_y_count; y++)
          {
             new_desk = zone->desks[xx - 1 + (y * zone->desk_x_count)];
             for (x = xx; x < zone->desk_x_count; x++)
               {
                  desk = zone->desks[x + (y * zone->desk_x_count)];

                  E_CLIENT_FOREACH(ec)
                    {
                       if (ec->desk == desk)
                         e_client_desk_set(ec, new_desk);
                    }
                  e_object_del(E_OBJECT(desk));
               }
          }
     }
   if (yy < zone->desk_y_count)
     {
        for (x = 0; x < zone->desk_x_count; x++)
          {
             new_desk = zone->desks[x + ((yy - 1) * zone->desk_x_count)];
             for (y = yy; y < zone->desk_y_count; y++)
               {
                  desk = zone->desks[x + (y * zone->desk_x_count)];

                  E_CLIENT_FOREACH(ec)
                    {
                       if (ec->desk == desk)
                         e_client_desk_set(ec, new_desk);
                    }
                  e_object_del(E_OBJECT(desk));
               }
          }
     }
   free(zone->desks);
   zone->desks = new_desks;

   zone->desk_x_count = xx;
   zone->desk_y_count = yy;
   e_config->zone_desks_x_count = xx;
   e_config->zone_desks_y_count = yy;
   e_config_save_queue();

   /* Cannot call desk_current_get until the zone desk counts have been set
    * or else we end up with a "white background" because desk_current_get will
    * return NULL.
    */
   desk = e_desk_current_get(zone);
   if (desk)
     {
        /* need to simulate "startup" conditions to force desk show to reevaluate here */
        int s = starting;
        desk->visible = 0;
        starting = 1;
        e_desk_show(desk);
        starting = s;
     }

   e_zone_edge_flip_eval(zone);

   ev = E_NEW(E_Event_Zone_Desk_Count_Set, 1);
   if (!ev) return;
   ev->zone = zone;
   e_object_ref(E_OBJECT(ev->zone));
   ecore_event_add(E_EVENT_ZONE_DESK_COUNT_SET, ev,
                   _e_zone_event_generic_free, NULL);
}

E_API void
e_zone_desk_count_get(E_Zone *zone,
                      int *x_count,
                      int *y_count)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   if (x_count) *x_count = zone->desk_x_count;
   if (y_count) *y_count = zone->desk_y_count;
}

E_API void
e_zone_desk_flip_by(E_Zone *zone,
                    int dx,
                    int dy)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   dx = zone->desk_x_current + dx;
   dy = zone->desk_y_current + dy;
   e_zone_desk_flip_to(zone, dx, dy);
   e_zone_edge_flip_eval(zone);
}

E_API void
e_zone_desk_flip_to(E_Zone *zone,
                    int x,
                    int y)
{
   E_Desk *desk;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   if (e_config->desk_flip_wrap)
     {
        x = x % zone->desk_x_count;
        y = y % zone->desk_y_count;
        if (x < 0) x += zone->desk_x_count;
        if (y < 0) y += zone->desk_y_count;
     }
   else
     {
        if (x < 0) x = 0;
        else if (x >= zone->desk_x_count)
          x = zone->desk_x_count - 1;
        if (y < 0) y = 0;
        else if (y >= zone->desk_y_count)
          y = zone->desk_y_count - 1;
     }
   desk = e_desk_at_xy_get(zone, x, y);
   if (!desk) return;
   e_desk_show(desk);
   e_zone_edge_flip_eval(zone);
}

E_API void
e_zone_desk_linear_flip_by(E_Zone *zone,
                           int dx)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   dx = zone->desk_x_current +
     (zone->desk_y_current * zone->desk_x_count) + dx;
   if ((!e_config->desk_flip_wrap) &&
     ((dx < 0) || (dx >= zone->desk_x_count * zone->desk_y_count))) return;
   dx = dx % (zone->desk_x_count * zone->desk_y_count);
   while (dx < 0)
     dx += (zone->desk_x_count * zone->desk_y_count);
   e_zone_desk_linear_flip_to(zone, dx);
}

E_API void
e_zone_desk_linear_flip_to(E_Zone *zone,
                           int x)
{
   int y;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   y = x / zone->desk_x_count;
   x = x - (y * zone->desk_x_count);
   e_zone_desk_flip_to(zone, x, y);
}

E_API void
e_zone_edge_enable(void)
{
   const Eina_List *l;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if (zone->edge.left) evas_object_show(zone->edge.left);
        if (zone->edge.right) evas_object_show(zone->edge.right);
        if (zone->edge.top) evas_object_show(zone->edge.top);
        if (zone->edge.bottom) evas_object_show(zone->edge.bottom);
        if (zone->corner.left_top) evas_object_show(zone->corner.left_top);
        if (zone->corner.top_left) evas_object_show(zone->corner.top_left);
        if (zone->corner.top_right) evas_object_show(zone->corner.top_right);
        if (zone->corner.right_top) evas_object_show(zone->corner.right_top);
        if (zone->corner.right_bottom) evas_object_show(zone->corner.right_bottom);
        if (zone->corner.bottom_right) evas_object_show(zone->corner.bottom_right);
        if (zone->corner.bottom_left) evas_object_show(zone->corner.bottom_left);
        if (zone->corner.left_bottom) evas_object_show(zone->corner.left_bottom);
        e_zone_edge_flip_eval(zone);
     }
}

E_API void
e_zone_edge_disable(void)
{
   const Eina_List *l;
   E_Zone *zone;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        zone->flip.switching = E_ZONE_EDGE_NONE;
        if (zone->edge.left) evas_object_hide(zone->edge.left);
        if (zone->edge.right) evas_object_hide(zone->edge.right);
        if (zone->edge.top) evas_object_hide(zone->edge.top);
        if (zone->edge.bottom) evas_object_hide(zone->edge.bottom);
        if (zone->corner.left_top) evas_object_hide(zone->corner.left_top);
        if (zone->corner.top_left) evas_object_hide(zone->corner.top_left);
        if (zone->corner.top_right) evas_object_hide(zone->corner.top_right);
        if (zone->corner.right_top) evas_object_hide(zone->corner.right_top);
        if (zone->corner.right_bottom) evas_object_hide(zone->corner.right_bottom);
        if (zone->corner.bottom_right) evas_object_hide(zone->corner.bottom_right);
        if (zone->corner.bottom_left) evas_object_hide(zone->corner.bottom_left);
        if (zone->corner.left_bottom) evas_object_hide(zone->corner.left_bottom);
     }
}

E_API void
e_zone_edges_desk_flip_capable(E_Zone *zone, Eina_Bool l, Eina_Bool r, Eina_Bool t, Eina_Bool b)
{
#define NEED_FLIP_EDGE(x) \
  (e_bindings_edge_flippable_get(x) || e_bindings_edge_non_flippable_get(x))
#define NEED_EDGE(x) \
  (e_bindings_edge_non_flippable_get(x))
#define CHECK_EDGE(v, ed, obj)                                                                   \
  do {                                                                                           \
       if (v) {                                                                                  \
            if (NEED_FLIP_EDGE(ed)) { if (zone->edge.obj) evas_object_show(zone->edge.obj); } \
            else if (zone->edge.obj)                                                             \
              evas_object_hide(zone->edge.obj);                                               \
         }                                                                                       \
       else {                                                                                    \
            if (NEED_EDGE(ed)) { if (zone->edge.obj) evas_object_show(zone->edge.obj); }      \
            else if (zone->edge.obj)                                                             \
              evas_object_hide(zone->edge.obj);                                               \
         }                                                                                       \
    } while (0)

   CHECK_EDGE(l, E_ZONE_EDGE_LEFT, left);
   CHECK_EDGE(r, E_ZONE_EDGE_RIGHT, right);
   CHECK_EDGE(t, E_ZONE_EDGE_TOP, top);
   CHECK_EDGE(b, E_ZONE_EDGE_BOTTOM, bottom);

#define CHECK_CORNER(v1, v2, ed, obj1, obj2)                                \
  if ((!v1) && (!v2)) {                                                     \
       if (NEED_EDGE(ed)) {                                                 \
            if (zone->corner.obj1) evas_object_show(zone->corner.obj1);  \
            if (zone->corner.obj2) evas_object_show(zone->corner.obj2);  \
         }                                                                  \
       else {                                                               \
            if (zone->corner.obj1) evas_object_hide(zone->corner.obj1);  \
            if (zone->corner.obj2) evas_object_hide(zone->corner.obj2);  \
         }                                                                  \
    }                                                                       \
  else {                                                                    \
       if (NEED_FLIP_EDGE(ed)) {                                            \
            if (zone->corner.obj1) evas_object_show(zone->corner.obj1);  \
            if (zone->corner.obj2) evas_object_show(zone->corner.obj2);  \
         }                                                                  \
       else {                                                               \
            if (zone->corner.obj1) evas_object_hide(zone->corner.obj1);  \
            if (zone->corner.obj2) evas_object_hide(zone->corner.obj2);  \
         }                                                                  \
    }

   CHECK_CORNER(l, t, E_ZONE_EDGE_TOP_LEFT, left_top, top_left);
   CHECK_CORNER(r, t, E_ZONE_EDGE_TOP_RIGHT, right_top, top_right);
   CHECK_CORNER(l, b, E_ZONE_EDGE_BOTTOM_LEFT, left_bottom, bottom_left);
   CHECK_CORNER(r, b, E_ZONE_EDGE_BOTTOM_RIGHT, right_bottom, bottom_right);
}

E_API Eina_Bool
e_zone_exists_direction(E_Zone *zone, E_Zone_Edge edge)
{
   Eina_List *l;
   E_Zone *z2;

   EINA_LIST_FOREACH(e_comp->zones, l, z2)
     {
        if (zone == z2) continue;

        switch (edge)
          {
           case E_ZONE_EDGE_TOP_LEFT:
             if (((E_SPANS_COMMON(0, zone->x + zone->w, z2->x, z2->w)) &&
                  (z2->y < zone->y)) ||
                 ((E_SPANS_COMMON(0, zone->y + zone->h, z2->y, z2->h)) &&
                  (z2->x < zone->x)))
               return EINA_TRUE;
             break;

           case E_ZONE_EDGE_TOP:
             if ((E_SPANS_COMMON(zone->x, zone->w, z2->x, z2->w)) &&
                 (z2->y < zone->y))
               return EINA_TRUE;
             break;

           case E_ZONE_EDGE_TOP_RIGHT:
             if (((E_SPANS_COMMON(zone->x, 99999, z2->x, z2->w)) &&
                  (z2->y < zone->y)) ||
                 ((E_SPANS_COMMON(0, zone->y + zone->h, z2->y, z2->h)) &&
                  (z2->x >= (zone->x + zone->w))))
               return EINA_TRUE;
             break;

           case E_ZONE_EDGE_LEFT:
             if ((E_SPANS_COMMON(zone->y, zone->h, z2->y, z2->h)) &&
                 (z2->x < zone->x))
               return EINA_TRUE;
             break;

           case E_ZONE_EDGE_RIGHT:
             if ((E_SPANS_COMMON(zone->y, zone->h, z2->y, z2->h)) &&
                 (z2->x >= (zone->x + zone->w)))
               return EINA_TRUE;
             break;

           case E_ZONE_EDGE_BOTTOM_LEFT:
             if (((E_SPANS_COMMON(0, zone->x + zone->w, z2->x, z2->w)) &&
                  (z2->y >= (zone->y + zone->h))) ||
                 ((E_SPANS_COMMON(zone->y, 99999, z2->y, z2->h)) &&
                  (z2->x < zone->x)))
               return EINA_TRUE;
             break;

           case E_ZONE_EDGE_BOTTOM:
             if ((E_SPANS_COMMON(zone->x, zone->w, z2->x, z2->w)) &&
                 (z2->y >= (zone->y + zone->h)))
               return EINA_TRUE;
             break;

           case E_ZONE_EDGE_BOTTOM_RIGHT:
             if (((E_SPANS_COMMON(zone->x, 99999, z2->x, z2->w)) &&
                  (z2->y >= (zone->y + zone->h))) ||
                 ((E_SPANS_COMMON(zone->y, 99999, z2->y, z2->h)) &&
                  (z2->x < zone->x)))
               return EINA_TRUE;
             break;

           default:
             break;
          }
     }

   return EINA_FALSE;
}

E_API void
e_zone_edge_flip_eval(E_Zone *zone)
{
   Eina_Bool lf, rf, tf, bf;

   lf = rf = tf = bf = EINA_TRUE;
   if (zone->desk_x_count <= 1) lf = rf = EINA_FALSE;
   else if (!e_config->desk_flip_wrap)
     {
        if (zone->desk_x_current == 0) lf = EINA_FALSE;
        if (zone->desk_x_current == (zone->desk_x_count - 1)) rf = EINA_FALSE;
     }
   if (zone->desk_y_count <= 1) tf = bf = EINA_FALSE;
   else if (!e_config->desk_flip_wrap)
     {
        if (zone->desk_y_current == 0) tf = EINA_FALSE;
        if (zone->desk_y_current == (zone->desk_y_count - 1)) bf = EINA_FALSE;
     }
   e_zone_edges_desk_flip_capable(zone, lf, rf, tf, bf);
}

E_API void
e_zone_edge_new(E_Zone_Edge edge)
{
   const Eina_List *l;
   E_Zone *zone;
   int cw, ch;

   if (edge == E_ZONE_EDGE_NONE) return;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        // don't allow bindings on edges that are on the boundary
        // between zones
        if (e_zone_exists_direction(zone, edge)) continue;
        cw = zone->w * E_ZONE_CORNER_RATIO;
        ch = zone->h * E_ZONE_CORNER_RATIO;
        switch (edge)
          {
#define EDGE_NEW(MEMBER, X, Y, W, H) do { \
             if (!zone->MEMBER) \
               { \
                  zone->MEMBER = evas_object_rectangle_add(e_comp->evas); \
                  evas_object_name_set(zone->MEMBER, #MEMBER); \
                  evas_object_move(zone->MEMBER, (X), (Y)); \
                  evas_object_resize(zone->MEMBER, (W), (H)); \
                  evas_object_repeat_events_set(zone->MEMBER, 1); \
                  evas_object_data_set(zone->MEMBER, "comp_repeat", (void*)1); \
                  evas_object_color_set(zone->MEMBER, 0, 0, 0, 0); \
                  evas_object_event_callback_add(zone->MEMBER, EVAS_CALLBACK_MOUSE_MOVE, _e_zone_cb_mouse_move, zone); \
                  evas_object_event_callback_add(zone->MEMBER, EVAS_CALLBACK_MOUSE_IN, _e_zone_cb_mouse_in, zone); \
                  evas_object_event_callback_add(zone->MEMBER, EVAS_CALLBACK_MOUSE_OUT, _e_zone_cb_mouse_out, zone); \
                  evas_object_event_callback_add(zone->MEMBER, EVAS_CALLBACK_MOUSE_DOWN, _e_zone_cb_mouse_down, zone); \
                  evas_object_event_callback_add(zone->MEMBER, EVAS_CALLBACK_MOUSE_UP, _e_zone_cb_mouse_up, zone); \
                  evas_object_show(zone->MEMBER); \
               } \
            } while (0)

           case E_ZONE_EDGE_LEFT:
             EDGE_NEW(edge.left, zone->x, zone->y + ch, 1, zone->h - 2 * ch);
             break;
           case E_ZONE_EDGE_RIGHT:
             EDGE_NEW(edge.right, zone->x + zone->w - 1, zone->y + ch, 1, zone->h - 2 * ch);
             break;
           case E_ZONE_EDGE_TOP:
             EDGE_NEW(edge.top, zone->x + 1 + cw, zone->y, zone->w - 2 * cw - 2, 1);
             break;
           case E_ZONE_EDGE_BOTTOM:
             EDGE_NEW(edge.bottom, zone->x + 1 + cw, zone->y + zone->h - 1, zone->w - 2 - 2 * cw, 1);
             break;
           case E_ZONE_EDGE_TOP_LEFT:
             EDGE_NEW(corner.left_top, zone->x, zone->y, 1, ch);
             EDGE_NEW(corner.top_left, zone->x + 1, zone->y, cw, 1);
             break;
           case E_ZONE_EDGE_TOP_RIGHT:
             EDGE_NEW(corner.top_right, zone->x + zone->w - cw - 2, zone->y, cw, 1);
             EDGE_NEW(corner.right_top, zone->x + zone->w - 1, zone->y, 1, ch);
             break;
           case E_ZONE_EDGE_BOTTOM_RIGHT:
             EDGE_NEW(corner.right_bottom, zone->x + zone->w - 1, zone->y + zone->h - ch, 1, ch);
             EDGE_NEW(corner.bottom_right, zone->x + zone->w - cw - 2, zone->y + zone->h - 1, cw, 1);
             break;
           case E_ZONE_EDGE_BOTTOM_LEFT:
             EDGE_NEW(corner.bottom_left, zone->x + 1, zone->y + zone->h - 1, cw, 1);
             EDGE_NEW(corner.left_bottom, zone->x, zone->y + zone->h - ch, 1, ch);
             break;
           default: continue;
          }
        if (e_config->fullscreen_flip)
          e_zone_edge_win_layer_set(zone, E_LAYER_CLIENT_EDGE_FULLSCREEN);
        else
          e_zone_edge_win_layer_set(zone, E_LAYER_CLIENT_EDGE);
     }
}

E_API void
e_zone_edge_free(E_Zone_Edge edge)
{
   const Eina_List *l;
   E_Zone *zone;

   if (edge == E_ZONE_EDGE_NONE) return;
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if (zone->flip.switching == edge)
          zone->flip.switching = E_ZONE_EDGE_NONE;
        switch (edge)
          {
           case E_ZONE_EDGE_NONE:
             /* noop */
             break;

           case E_ZONE_EDGE_LEFT:
             E_FREE_FUNC(zone->edge.left, evas_object_del);
             break;

           case E_ZONE_EDGE_RIGHT:
             E_FREE_FUNC(zone->edge.right, evas_object_del);
             break;

           case E_ZONE_EDGE_TOP:
             E_FREE_FUNC(zone->edge.top, evas_object_del);
             break;

           case E_ZONE_EDGE_BOTTOM:
             E_FREE_FUNC(zone->edge.bottom, evas_object_del);
             break;

           case E_ZONE_EDGE_TOP_LEFT:
             E_FREE_FUNC(zone->corner.left_top, evas_object_del);
             E_FREE_FUNC(zone->corner.top_left, evas_object_del);
             break;

           case E_ZONE_EDGE_TOP_RIGHT:
             E_FREE_FUNC(zone->corner.top_right, evas_object_del);
             E_FREE_FUNC(zone->corner.right_top, evas_object_del);
             break;

           case E_ZONE_EDGE_BOTTOM_RIGHT:
             E_FREE_FUNC(zone->corner.right_bottom, evas_object_del);
             E_FREE_FUNC(zone->corner.bottom_right, evas_object_del);
             break;

           case E_ZONE_EDGE_BOTTOM_LEFT:
             E_FREE_FUNC(zone->corner.bottom_left, evas_object_del);
             E_FREE_FUNC(zone->corner.left_bottom, evas_object_del);
             break;
          }
     }
}

E_API void
e_zone_edge_win_layer_set(E_Zone *zone, E_Layer layer)
{
#define EDGE_STACK(EDGE) do { \
   if (zone->EDGE) \
     { \
       evas_object_layer_set(zone->EDGE, layer); \
       evas_object_stack_below(zone->EDGE, e_comp->layers[e_comp_canvas_layer_map(layer)].obj); \
     } \
   } while (0)

   EDGE_STACK(corner.left_bottom);
   EDGE_STACK(corner.left_top);
   EDGE_STACK(corner.top_left);
   EDGE_STACK(corner.top_right);
   EDGE_STACK(corner.right_top);
   EDGE_STACK(corner.right_bottom);
   EDGE_STACK(corner.bottom_right);
   EDGE_STACK(corner.bottom_left);

   EDGE_STACK(edge.left);
   EDGE_STACK(edge.right);
   EDGE_STACK(edge.top);
   EDGE_STACK(edge.bottom);
}

E_API void
e_zone_fade_handle(E_Zone *zone, int out, double tim)
{
   EINA_SAFETY_ON_NULL_RETURN(zone);
   if (out == 1)
     {
        if ((e_backlight_exists()) && (!e_comp_config_get()->nofade))
          {
             e_backlight_update();
             zone->bloff = EINA_TRUE;
             zone->bl = e_backlight_level_get(zone);
             e_backlight_level_set(zone, 0.0, tim);
          }
     }
   else
     {
        if ((e_backlight_exists()) && (!e_comp_config_get()->nofade))
          {
             zone->bloff = EINA_FALSE;
             e_backlight_update();
             if (e_backlight_mode_get(zone) != E_BACKLIGHT_MODE_NORMAL)
               e_backlight_mode_set(zone, E_BACKLIGHT_MODE_NORMAL);
             else
               e_backlight_level_set(zone, e_config->backlight.normal, tim);
          }
     }
}

static void
_e_zone_useful_geometry_calc(const E_Zone *zone, int dx, int dy, int *x, int *y, int *w, int *h)
{
   E_Desk *desk;
   E_Zone_Obstacle *obs;
   Eina_Tiler *tiler;
   int zx, zy, zw, zh;
   Eina_Iterator *it;
   Eina_Rectangle geom = { 0 } , *rect;
   int size = 0;

   zx = zone->x;
   zy = zone->y;
   zw = zone->w;
   zh = zone->h;
   tiler = eina_tiler_new(zw, zh);
   eina_tiler_tile_size_set(tiler, 1, 1);
   eina_tiler_rect_add(tiler, &(Eina_Rectangle){0, 0, zw, zh});
   EINA_INLIST_FOREACH(zone->obstacles, obs)
     {
        if (!E_INTERSECTS(obs->x, obs->y, obs->w, obs->h, zx, zy, zw, zh)) continue;
        if (obs->vertical)
          eina_tiler_rect_del(tiler, &(Eina_Rectangle){obs->x - zx, 0, obs->w, zh});
        else
          eina_tiler_rect_del(tiler, &(Eina_Rectangle){0, obs->y - zy, zw, obs->h});
     }
   desk = e_desk_at_xy_get(zone, dx, dy);
   if (desk)
     {
        EINA_INLIST_FOREACH(desk->obstacles, obs)
          {
             if (!E_INTERSECTS(obs->x, obs->y, obs->w, obs->h, zx, zy, zw, zh)) continue;
             if (obs->vertical)
               eina_tiler_rect_del(tiler, &(Eina_Rectangle){obs->x - zx, 0, obs->w, zh});
             else
               eina_tiler_rect_del(tiler, &(Eina_Rectangle){0, obs->y - zy, zw, obs->h});
          }
     }
   it = eina_tiler_iterator_new(tiler);
   EINA_ITERATOR_FOREACH(it, rect)
     {
        if (rect->w * rect->h < size) continue;
        size = rect->w * rect->h;
        geom = *rect;
     }
   eina_iterator_free(it);
   eina_tiler_free(tiler);

   if (x) *x = geom.x + zx;
   if (y) *y = geom.y + zy;
   if (w) *w = geom.w;
   if (h) *h = geom.h;
}

/**
 * Get (or calculate) the useful (or free, without any shelves) area.
 */
E_API Eina_Bool
e_zone_useful_geometry_get(E_Zone *zone,
                           int *x,
                           int *y,
                           int *w,
                           int *h)
{
   int zx, zy, zw, zh;

   E_OBJECT_CHECK_RETURN(zone, EINA_FALSE);
   E_OBJECT_TYPE_CHECK_RETURN(zone, E_ZONE_TYPE, EINA_FALSE);

   if (zone->useful_geometry_dirty)
     {
        _e_zone_useful_geometry_calc(zone, zone->desk_x_current, zone->desk_y_current, &zx, &zy, &zw, &zh);
        memcpy(&zone->useful_geometry[0], &zone->useful_geometry[1], sizeof(Eina_Rectangle));
        zone->useful_geometry[1].x = zx;
        zone->useful_geometry[1].y = zy;
        zone->useful_geometry[1].w = zw;
        zone->useful_geometry[1].h = zh;
        zone->useful_geometry_changed =
          !!memcmp(&zone->useful_geometry[0], &zone->useful_geometry[1], sizeof(Eina_Rectangle));

     }
   zone->useful_geometry_dirty = 0;

   if (x) *x = zone->useful_geometry[1].x;
   if (y) *y = zone->useful_geometry[1].y;
   if (w) *w = zone->useful_geometry[1].w;
   if (h) *h = zone->useful_geometry[1].h;
   return zone->useful_geometry_changed;
}

E_API void
e_zone_desk_useful_geometry_get(const E_Zone *zone, const E_Desk *desk, int *x, int *y, int *w, int *h)
{
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   E_OBJECT_CHECK(desk);
   E_OBJECT_TYPE_CHECK(desk, E_DESK_TYPE);

   if (desk->zone != zone) CRI("zone/desk mismatch!");

   _e_zone_useful_geometry_calc(zone, desk->x, desk->y, x, y, w, h);
}

/**
 * Mark as dirty so e_zone_useful_geometry_get() will need to recalculate.
 *
 * Call this function when shelves are added or important properties changed.
 */
E_API void
e_zone_useful_geometry_dirty(E_Zone *zone)
{
   E_Event_Zone_Move_Resize *ev;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   /* ignore if pending event already exists */
   if (zone->useful_geometry_dirty) return;

   zone->useful_geometry_dirty = 1;
   if (!e_zone_useful_geometry_get(zone, NULL, NULL, NULL, NULL)) return;
   ev = E_NEW(E_Event_Zone_Move_Resize, 1);
   ev->zone = zone;
   e_object_ref(E_OBJECT(ev->zone));
   ecore_event_add(E_EVENT_ZONE_USEFUL_GEOMETRY_CHANGED, ev, _e_zone_event_generic_free, NULL);

   if (!stopping)
     e_comp_clients_rescale();
}

E_API E_Zone_Obstacle *
e_zone_obstacle_add(E_Zone *zone, E_Desk *desk, Eina_Rectangle *geom, Eina_Bool vertical)
{
   E_Zone_Obstacle *obs;

   E_OBJECT_CHECK_RETURN(zone, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(zone, E_ZONE_TYPE, NULL);
   if (desk)
     {
        E_OBJECT_CHECK_RETURN(desk, NULL);
        E_OBJECT_TYPE_CHECK_RETURN(desk, E_DESK_TYPE, NULL);
        if (desk->zone != zone)
          {
             ERR("zone != desk->zone");
             return NULL;
          }
     }
   obs = E_OBJECT_ALLOC(E_Zone_Obstacle, E_ZONE_OBSTACLE_TYPE, _e_zone_obstacle_free);
   obs->x = geom->x, obs->y = geom->y;
   obs->w = geom->w, obs->h = geom->h;
   obs->owner = E_OBJECT(desk) ?: E_OBJECT(zone);
   obs->vertical = !!vertical;
   if (desk)
     {
        desk->obstacles = eina_inlist_append(desk->obstacles, EINA_INLIST_GET(obs));
        if (desk->visible)
          e_zone_useful_geometry_dirty(desk->zone);
     }
   else
     {
        zone->obstacles = eina_inlist_append(zone->obstacles, EINA_INLIST_GET(obs));
        e_zone_useful_geometry_dirty(zone);
     }
   return obs;
}

E_API void
e_zone_obstacle_modify(E_Zone_Obstacle *obs, Eina_Rectangle *geom, Eina_Bool vertical)
{
   E_Zone *zone;
   E_Desk *desk;

   E_OBJECT_CHECK(obs);
   E_OBJECT_TYPE_CHECK(obs, E_ZONE_OBSTACLE_TYPE);
   EINA_SAFETY_ON_NULL_RETURN(geom);
   if ((obs->x == geom->x) && (obs->y == geom->y) && (obs->w == geom->w) && (obs->h == geom->h))
     return;
   obs->x = geom->x, obs->y = geom->y;
   obs->w = geom->w, obs->h = geom->h;
   obs->vertical = !!vertical;

   if (obs->owner->type == E_DESK_TYPE)
     {
        desk = (void *)obs->owner;
        if (desk->visible)
          e_zone_useful_geometry_dirty(desk->zone);
     }
   else
     {
        zone = (void *)obs->owner;
        e_zone_useful_geometry_dirty(zone);
     }
}

E_API void
e_zone_stow(E_Zone *zone)
{
   E_Event_Zone_Stow *ev;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   if (zone->stowed) return;
   ev = E_NEW(E_Event_Zone_Stow, 1);
   ev->zone = zone;
   e_object_ref(E_OBJECT(ev->zone));
   ecore_event_add(E_EVENT_ZONE_STOW, ev, _e_zone_event_generic_free, NULL);

   zone->stowed = EINA_TRUE;
}

E_API void
e_zone_unstow(E_Zone *zone)
{
   E_Event_Zone_Unstow *ev;

   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   if (!zone->stowed) return;
   ev = E_NEW(E_Event_Zone_Unstow, 1);
   ev->zone = zone;
   e_object_ref(E_OBJECT(ev->zone));
   ecore_event_add(E_EVENT_ZONE_UNSTOW, ev, _e_zone_event_generic_free, NULL);

   zone->stowed = EINA_FALSE;
}

/* local subsystem functions */
static void
_e_zone_free(E_Zone *zone)
{
   int x, y;

   //printf("@@@@@@@@@@ e_zone_free: %i %i | %i %i %ix%i = %p\n", zone->num, zone->id, zone->x, zone->y, zone->w, zone->h, zone);
   /* Delete the edge windows if they exist */
   E_FREE_FUNC(zone->edge.top, evas_object_del);
   E_FREE_FUNC(zone->edge.bottom, evas_object_del);
   E_FREE_FUNC(zone->edge.left, evas_object_del);
   E_FREE_FUNC(zone->edge.right, evas_object_del);
   E_FREE_FUNC(zone->corner.left_bottom, evas_object_del);
   E_FREE_FUNC(zone->corner.left_top, evas_object_del);
   E_FREE_FUNC(zone->corner.top_left, evas_object_del);
   E_FREE_FUNC(zone->corner.top_right, evas_object_del);
   E_FREE_FUNC(zone->corner.right_top, evas_object_del);
   E_FREE_FUNC(zone->corner.right_bottom, evas_object_del);
   E_FREE_FUNC(zone->corner.bottom_right, evas_object_del);
   E_FREE_FUNC(zone->corner.bottom_left, evas_object_del);

   /* Delete the object event callbacks */
   evas_object_event_callback_del(zone->bg_event_object,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _e_zone_cb_bg_mouse_down);
   evas_object_event_callback_del(zone->bg_event_object,
                                  EVAS_CALLBACK_MOUSE_UP,
                                  _e_zone_cb_bg_mouse_up);

   E_FREE_FUNC(zone->cur_mouse_action, e_object_unref);

   /* remove handlers */
   E_FREE_LIST(zone->handlers, ecore_event_handler_del);

   if (zone->name) eina_stringshare_del(zone->name);
   e_comp->zones = eina_list_remove(e_comp->zones, zone);
   evas_object_del(zone->bg_event_object);
   evas_object_del(zone->bg_clip_object);
   evas_object_del(zone->bg_object);
   if (zone->prev_bg_object) evas_object_del(zone->prev_bg_object);
   if (zone->transition_object) evas_object_del(zone->transition_object);

   evas_object_del(zone->base);
   evas_object_del(zone->over);
   if ((!stopping) && (!e_comp_config_get()->nofade))
     {
        if (zone->bloff)
          {
             if (e_backlight_mode_get(zone) != E_BACKLIGHT_MODE_NORMAL)
               e_backlight_mode_set(zone, E_BACKLIGHT_MODE_NORMAL);
             e_backlight_level_set(zone, e_config->backlight.normal, 0.0);
          }
     }

   /* free desks */
   while (zone->obstacles)
     {
        E_Object *obs = (void *)EINA_INLIST_CONTAINER_GET(zone->obstacles, E_Zone_Obstacle);
        e_object_del(obs);
     }
   for (x = 0; x < zone->desk_x_count; x++)
     {
        for (y = 0; y < zone->desk_y_count; y++)
          {
             e_object_del(E_OBJECT(zone->desks[x + (y * zone->desk_x_count)]));
             zone->desks[x + (y * zone->desk_x_count)] = NULL;
          }
     }
   free(zone->desks);
   free(zone->randr2_id);
   free(zone);
}

static void
_e_zone_cb_bg_mouse_down(void *data,
                         Evas *evas       EINA_UNUSED,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info)
{
   E_Zone *zone = data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (e_comp_util_mouse_grabbed()) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if (!zone->cur_mouse_action)
     {
        zone->cur_mouse_action =
          e_bindings_mouse_down_evas_event_handle(E_BINDING_CONTEXT_ZONE,
                                             E_OBJECT(zone), event_info);
        if (zone->cur_mouse_action)
          {
             if ((!zone->cur_mouse_action->func.end_mouse) &&
                 (!zone->cur_mouse_action->func.end))
               zone->cur_mouse_action = NULL;
             if (zone->cur_mouse_action)
               e_object_ref(E_OBJECT(zone->cur_mouse_action));
          }
     }
}

static void
_e_zone_cb_bg_mouse_up(void *data,
                       Evas *evas       EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info)
{
   E_Zone *zone = data;
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (zone->cur_mouse_action)
     {
        E_Binding_Event_Mouse_Button event;

        e_bindings_evas_event_mouse_button_convert(event_info, &event);
        if (zone->cur_mouse_action->func.end_mouse)
          zone->cur_mouse_action->func.end_mouse(E_OBJECT(zone), "", &event);
        else if (zone->cur_mouse_action->func.end)
          zone->cur_mouse_action->func.end(E_OBJECT(zone), "");

        e_object_unref(E_OBJECT(zone->cur_mouse_action));
        zone->cur_mouse_action = NULL;
     }
   else
     {
        E_Binding_Event_Mouse_Button event;

        e_bindings_ecore_event_mouse_button_convert(event_info, &event);
        e_bindings_mouse_up_event_handle(E_BINDING_CONTEXT_ZONE,
                                         E_OBJECT(zone), &event);
     }
}

static Eina_Bool
_e_zone_cb_edge_timer(void *data)
{
   E_Zone *zone;
   E_Action *act;

   zone = data;
   act = e_action_find(zone->flip.bind->action);
   if (!act)
     {
        E_FREE(zone->flip.ev);
        return ECORE_CALLBACK_CANCEL;
     }

   if (act->func.go_edge)
     act->func.go_edge(E_OBJECT(zone), zone->flip.bind->params, zone->flip.ev);
   else if (act->func.go)
     act->func.go(E_OBJECT(zone), zone->flip.bind->params);

   zone->flip.bind->timer = NULL;

   E_FREE(zone->flip.ev);
   return ECORE_CALLBACK_CANCEL;
}

static void
_e_zone_event_generic_free(void *data EINA_UNUSED, void *ev)
{
   struct _E_Event_Zone_Generic *e;
   // also handes E_Event_Zone_Add, E_Event_Zone_Del, E_Event_Zone_Stow,
   // E_Event_Zone_Unstow, E_Event_Zone_Desk_Count_Set due to them all
   // having the same content

   e = ev;
   e_object_unref(E_OBJECT(e->zone));
   free(e);
}

static void
_e_zone_object_del_attach(void *o)
{
   E_Zone *zone;
   E_Event_Zone_Del *ev;

   zone = o;
   if (stopping) return;
   ev = E_NEW(E_Event_Zone_Del, 1);
   ev->zone = zone;
   e_object_ref(E_OBJECT(ev->zone));
   ecore_event_add(E_EVENT_ZONE_DEL, ev, _e_zone_event_generic_free, NULL);
}

static E_Zone_Edge
_e_zone_detect_edge(E_Zone *zone, Evas_Object *obj)
{
   E_Zone_Edge edge = E_ZONE_EDGE_NONE;

   if (obj == zone->edge.left)
     edge = E_ZONE_EDGE_LEFT;
   else if (obj == zone->edge.top)
     edge = E_ZONE_EDGE_TOP;
   else if (obj == zone->edge.right)
     edge = E_ZONE_EDGE_RIGHT;
   else if (obj == zone->edge.bottom)
     edge = E_ZONE_EDGE_BOTTOM;
   else if ((obj == zone->corner.left_top) ||
            (obj == zone->corner.top_left))
     edge = E_ZONE_EDGE_TOP_LEFT;
   else if ((obj == zone->corner.right_top) ||
            (obj == zone->corner.top_right))
     edge = E_ZONE_EDGE_TOP_RIGHT;
   else if ((obj == zone->corner.right_bottom) ||
            (obj == zone->corner.bottom_right))
     edge = E_ZONE_EDGE_BOTTOM_RIGHT;
   else if ((obj == zone->corner.left_bottom) ||
            (obj == zone->corner.bottom_left))
     edge = E_ZONE_EDGE_BOTTOM_LEFT;
   return edge;
}

static void
_e_zone_edge_move_resize(E_Zone *zone)
{
   int cw;
   int ch;

   cw = zone->w * E_ZONE_CORNER_RATIO;
   ch = zone->h * E_ZONE_CORNER_RATIO;

   evas_object_geometry_set(zone->corner.left_bottom,
             zone->x, zone->y + zone->h - ch, 1, ch);
   evas_object_geometry_set(zone->edge.left,
             zone->x, zone->y + ch, 1, zone->h - 2 * ch);
   evas_object_geometry_set(zone->corner.left_top,
             zone->x, zone->y, 1, ch);

   evas_object_geometry_set(zone->corner.top_left,
             zone->x + 1, zone->y, cw, 1);
   evas_object_geometry_set(zone->edge.top,
             zone->x + 1 + cw, zone->y, zone->w - 2 * cw - 2, 1);
   evas_object_geometry_set(zone->corner.top_right,
             zone->x + zone->w - cw - 2, zone->y, cw, 1);

   evas_object_geometry_set(zone->corner.right_top,
             zone->x + zone->w - 1, zone->y, 1, ch);
   evas_object_geometry_set(zone->edge.right,
             zone->x + zone->w - 1, zone->y + ch, 1, zone->h - 2 * ch);
   evas_object_geometry_set(zone->corner.right_bottom,
             zone->x + zone->w - 1, zone->y + zone->h - ch, 1, ch);

   evas_object_geometry_set(zone->corner.bottom_right,
             zone->x + 1, zone->y + zone->h - 1, cw, 1);
   evas_object_geometry_set(zone->edge.bottom,
             zone->x + 1 + cw, zone->y + zone->h - 1, zone->w - 2 - 2 * cw, 1);
   evas_object_geometry_set(zone->corner.bottom_left,
             zone->x + zone->w - cw - 2, zone->y + zone->h - 1, cw, 1);
}

static void
_e_zone_obstacle_free(E_Zone_Obstacle *obs)
{
   E_Zone *zone;
   E_Desk *desk;

   if (obs->owner->type == E_DESK_TYPE)
     {
        desk = (void *)obs->owner;
        desk->obstacles = eina_inlist_remove(desk->obstacles, EINA_INLIST_GET(obs));
        if (desk->visible)
          e_zone_useful_geometry_dirty(desk->zone);
     }
   else
     {
        zone = (void *)obs->owner;
        zone->obstacles = eina_inlist_remove(zone->obstacles, EINA_INLIST_GET(obs));
        e_zone_useful_geometry_dirty(zone);
     }
   free(obs);
}

E_API E_Zone *
e_zone_for_id_get(const char *id)
{
   Eina_List *l = NULL;
   E_Zone *zone;

   if (!e_comp) return NULL;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if (eina_streq(zone->randr2_id, id))
          return zone;
     }

   return NULL;
}
