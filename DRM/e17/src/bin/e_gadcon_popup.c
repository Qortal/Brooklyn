#include "e.h"

/* local subsystem functions */

static void _e_gadcon_popup_changed_size_hints_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED);
static void _e_gadcon_popup_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED);

static Eina_Bool
_e_popup_autoclose_deskafter_show_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Desk_After_Show *ev = event;
   E_Gadcon_Popup *pop = data;

   if (!pop) return ECORE_CALLBACK_RENEW;
   if (!pop->visible) return ECORE_CALLBACK_RENEW;
   if (!e_gadcon_client_visible_get(pop->gcc, ev->desk))
     e_object_del(E_OBJECT(pop));

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_popup_autoclose_client_fullscreen_cb(void *data, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   E_Gadcon_Popup *pop = data;

   if (!pop) return ECORE_CALLBACK_RENEW;
   if (!pop->visible) return ECORE_CALLBACK_RENEW;
   if (!ev->ec->fullscreen) return ECORE_CALLBACK_RENEW;
   if (e_gadcon_client_visible_get(pop->gcc, ev->ec->desk))
     e_object_del(E_OBJECT(pop));
   return ECORE_CALLBACK_RENEW;
}

static void
_e_gadcon_popup_locked_set(E_Gadcon_Popup *pop, Eina_Bool locked)
{
   if (!pop->gcc)
     return;

   e_gadcon_locked_set(pop->gcc->gadcon, locked);
   pop->gadcon_was_locked = locked;
}

static void
_e_gadcon_popup_delay_del(void *obj)
{
   E_Gadcon_Popup *pop = obj;

   if (pop->comp_object) evas_object_hide(pop->comp_object);
}

static void
_e_gadcon_popup_free(E_Gadcon_Popup *pop)
{
   if (pop->content)
     {
        evas_object_event_callback_del_full(pop->content, EVAS_CALLBACK_DEL,
                                            _e_gadcon_popup_del_cb, pop);
        evas_object_event_callback_del_full(pop->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _e_gadcon_popup_changed_size_hints_cb, pop);
        pop->content = NULL;
     }
   E_FREE_FUNC(pop->autoclose_handlers[0], ecore_event_handler_del);
   E_FREE_FUNC(pop->autoclose_handlers[1], ecore_event_handler_del);

   if (pop->gadcon_was_locked)
     _e_gadcon_popup_locked_set(pop, 0);
   pop->gcc = NULL;
   evas_object_hide(pop->comp_object);
   E_FREE_FUNC(pop->comp_object, evas_object_del);
   E_FREE_FUNC(pop->show_idler, ecore_idler_del);
   E_FREE_FUNC(pop->show_animator, ecore_animator_del);
   free(pop);
}

static void
_e_gadcon_popup_position(E_Gadcon_Popup *pop)
{
   Evas_Coord gx = 0, gy = 0, gw, gh, zw, zh, zx, zy, px, py;
   E_Zone *zone;

   /* Popup positioning */
   e_gadcon_client_geometry_get(pop->gcc, &gx, &gy, &gw, &gh);
   zone = e_gadcon_zone_get(pop->gcc->gadcon);
   zx = zone->x;
   zy = zone->y;
   zw = zone->w;
   zh = zone->h;
   switch (pop->gcc->gadcon->orient)
     {
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_RB:
      case E_GADCON_ORIENT_RIGHT:
        px = gx - pop->w;
        py = gy;
        if (py + pop->h >= (zy + zh))
          py = gy + gh - pop->h;
        px = MIN(zx + zw - gw - pop->w - 3, px);
        break;

      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_LB:
        px = gx + gw;
        py = gy;
        if (py + pop->h >= (zy + zh))
          py = gy + gh - pop->h;
        px = MIN(zx + zw - gw - pop->w + 3, px);
        break;

      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
        py = gy + gh;
        px = (gx + (gw / 2)) - (pop->w / 2);
        if ((px + pop->w) >= (zx + zw))
          px = gx + gw - pop->w;
        else if (px < zx)
          px = zx;
        py = MIN(zy + zh - gh - pop->h + 3, py);
        break;

      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
        py = gy - pop->h;
        px = (gx + (gw / 2)) - (pop->w / 2);
        if ((px + pop->w) >= (zx + zw))
          px = gx + gw - pop->w;
        else if (px < zx)
          px = zx;
        py = MIN(zy + zh - gh - pop->h - 3, py);
        break;

      case E_GADCON_ORIENT_FLOAT:
        px = (gx + (gw / 2)) - (pop->w / 2);
        if (gy >= (zy + (zh / 2)))
          py = gy - pop->h;
        else
          py = gy + gh;
        if ((px + pop->w) >= (zx + zw))
          px = gx + gw - pop->w;
        else if (px < zx)
          px = zx;
        break;

      default:
        evas_object_move(pop->comp_object, 50, 50);
        return;
     }
   if (px - zx < 0)
     px = zx;
   if (py - zy < 0)
     py = zy;
   evas_object_move(pop->comp_object, px, py);

   if (pop->gadcon_lock && (!pop->gadcon_was_locked))
     _e_gadcon_popup_locked_set(pop, 1);
}

static void
_e_gadcon_popup_size_recalc(E_Gadcon_Popup *pop, Evas_Object *obj)
{
   Evas_Coord pw, ph, w = 0, h = 0;

   if (!isedje(obj))
     evas_object_size_hint_min_get(obj, &w, &h);
   else
     {
        edje_object_size_min_get(obj, &w, &h);
        edje_object_size_min_restricted_calc(obj, &w, &h, w, h);
        evas_object_size_hint_min_set(obj, w, h);
     }
   edje_object_size_min_calc(pop->o_bg, &pw, &ph);
   pop->w = MAX(pw, pop->w);
   pop->h = MAX(ph, pop->h);
   evas_object_resize(pop->comp_object, pop->w, pop->h);

   _e_gadcon_popup_position(pop);
}

static void
_e_gadcon_popup_changed_size_hints_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Gadcon_Popup *pop = data;

   /* edje calc bug: hint updates on child are not updated for min_calc unless this happens first */
   edje_object_part_unswallow(pop->o_bg, obj);
   edje_object_part_swallow(pop->o_bg, "e.swallow.content", obj);
   _e_gadcon_popup_size_recalc(data, obj);
}

static void
_e_gadcon_popup_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Gadcon_Popup *pop = data;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _e_gadcon_popup_del_cb, pop);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _e_gadcon_popup_changed_size_hints_cb, pop);
   pop->content = NULL;
}

/* externally accessible functions */

E_API E_Gadcon_Popup *
e_gadcon_popup_new(E_Gadcon_Client *gcc, Eina_Bool noshadow)
{
   E_Gadcon_Popup *pop;
   Evas_Object *o;

   pop = E_OBJECT_ALLOC(E_Gadcon_Popup, E_GADCON_POPUP_TYPE, _e_gadcon_popup_free);
   if (!pop) return NULL;
   e_object_delay_del_set(E_OBJECT(pop), _e_gadcon_popup_delay_del);

   o = edje_object_add(e_comp->evas);
   e_theme_edje_object_set(o, "base/theme/gadman", "e/gadman/popup");
   pop->o_bg = o;

   pop->comp_object = e_comp_object_util_add(o, noshadow ? E_COMP_OBJECT_TYPE_NONE : E_COMP_OBJECT_TYPE_POPUP);
   if (e_comp->nocomp)
     evas_object_layer_set(pop->comp_object, E_LAYER_CLIENT_NORMAL);
   else
     evas_object_layer_set(pop->comp_object, E_LAYER_POPUP);

   pop->gcc = gcc;
   pop->gadcon_lock = 1;
   pop->gadcon_was_locked = 0;

   return pop;
}

E_API void
e_gadcon_popup_content_set(E_Gadcon_Popup *pop, Evas_Object *o)
{
   Evas_Object *old_o;

   if (!pop) return;
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);

   old_o = edje_object_part_swallow_get(pop->o_bg, "e.swallow.content");
   if (old_o != o)
     {
        if (old_o)
          {
             edje_object_part_unswallow(pop->o_bg, old_o);
             evas_object_del(old_o);
          }
        edje_object_part_swallow(pop->o_bg, "e.swallow.content", o);
        evas_object_event_callback_add(o, EVAS_CALLBACK_DEL,
                                       _e_gadcon_popup_del_cb, pop);
        evas_object_event_callback_add(o, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _e_gadcon_popup_changed_size_hints_cb, pop);
     }
   pop->content = o;
   _e_gadcon_popup_size_recalc(pop, o);
}

static Eina_Bool
_cb_show_animator(void *data)
{
   E_Gadcon_Popup *pop = data;
   pop->show_animator = NULL;
   E_FREE_FUNC(pop->show_idler, ecore_idler_del);
   evas_object_show(pop->comp_object);
   return EINA_FALSE;
}

static Eina_Bool
_cb_show_idler(void *data)
{
   E_Gadcon_Popup *pop = data;
   pop->show_idler = NULL;
   E_FREE_FUNC(pop->show_animator, ecore_animator_del);
   pop->show_animator = ecore_animator_add(_cb_show_animator, pop);
   return EINA_FALSE;
}

E_API void
e_gadcon_popup_show(E_Gadcon_Popup *pop)
{
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);

   if (pop->visible) return;

   _e_gadcon_popup_position(pop);
   pop->autoclose_handlers[0] = ecore_event_handler_add(E_EVENT_DESK_AFTER_SHOW, _e_popup_autoclose_deskafter_show_cb, NULL);
   pop->autoclose_handlers[1] = ecore_event_handler_add(E_EVENT_CLIENT_FULLSCREEN, _e_popup_autoclose_client_fullscreen_cb, NULL);
   if (pop->content)
     e_comp_object_util_del_list_append(pop->comp_object, pop->content);
   pop->show_idler = ecore_idler_add(_cb_show_idler, pop);
   pop->visible = EINA_TRUE;
}

E_API void
e_gadcon_popup_hide(E_Gadcon_Popup *pop)
{
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);
   if (pop->pinned) return;
   if (!pop->visible) return;
   E_FREE_FUNC(pop->show_idler, ecore_idler_del);
   E_FREE_FUNC(pop->show_animator, ecore_animator_del);
   evas_object_hide(pop->comp_object);
   if (pop->gadcon_was_locked)
     _e_gadcon_popup_locked_set(pop, 0);
   pop->visible = EINA_FALSE;
}

E_API void
e_gadcon_popup_toggle_pinned(E_Gadcon_Popup *pop)
{
   if (!pop) return;
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);

   if (pop->pinned)
     {
        pop->pinned = 0;
        edje_object_signal_emit(pop->o_bg, "e,state,unpinned", "e");
     }
   else
     {
        pop->pinned = 1;
        edje_object_signal_emit(pop->o_bg, "e,state,pinned", "e");
     }
}

E_API void
e_gadcon_popup_lock_set(E_Gadcon_Popup *pop, Eina_Bool setting)
{
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);

   setting = !!setting;
   if (pop->gadcon_lock == setting) return;
   pop->gadcon_lock = setting;

   if (setting != pop->gadcon_was_locked)
     _e_gadcon_popup_locked_set(pop, setting);
}
