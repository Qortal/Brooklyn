#include "e.h"
#include "e_mod_main.h"

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void             _gc_shutdown(E_Gadcon_Client *gcc);
static void             _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char      *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object     *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char      *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static void             _gc_id_del(const E_Gadcon_Client_Class *client_class, const char *id);
/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
   "ibox",
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, _gc_id_del,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_INSET
};

/* actual module specifics */

typedef struct _Instance  Instance;

typedef struct _IBox      IBox;
typedef struct _IBox_Icon IBox_Icon;

struct _Instance
{
   E_Gadcon_Client *gcc;
   E_Comp_Object_Mover *iconify_provider;
   Evas_Object     *o_ibox;
   IBox            *ibox;
   E_Drop_Handler  *drop_handler;
   Config_Item     *ci;
   E_Gadcon_Orient  orient;
};

struct _IBox
{
   Instance    *inst;
   Evas_Object *o_box;
   Evas_Object *o_drop;
   Evas_Object *o_drop_over;
   Evas_Object *o_empty;
   IBox_Icon   *ic_drop_before;
   int          drop_before;
   Eina_List   *icons;
   E_Zone      *zone;
   Evas_Coord   dnd_x, dnd_y;
};

struct _IBox_Icon
{
   IBox        *ibox;
   Evas_Object *o_holder;
   Evas_Object *o_icon;
   Evas_Object *o_holder2;
   Evas_Object *o_icon2;
   E_Client    *client;
   struct
   {
      unsigned char start E_BITFIELD;
      unsigned char dnd E_BITFIELD;
      int           x, y;
      int           dx, dy;
   } drag;
};

static IBox        *_ibox_new(Evas_Object *parent, E_Zone *zone);
static void         _ibox_free(IBox *b);
static void         _ibox_cb_empty_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_empty_handle(IBox *b);
static void         _ibox_fill(IBox *b);
static void         _ibox_empty(IBox *b);
static void         _ibox_orient_set(IBox *b, int horizontal);
static void         _ibox_resize_handle(IBox *b);
static void         _ibox_instance_drop_zone_recalc(Instance *inst);
static IBox_Icon   *_ibox_icon_find(IBox *b, E_Client *ec);
static IBox_Icon   *_ibox_icon_at_coord(IBox *b, Evas_Coord x, Evas_Coord y);
static IBox_Icon   *_ibox_icon_new(IBox *b, E_Client *ec);
static void         _ibox_icon_free(IBox_Icon *ic);
static void         _ibox_icon_fill(IBox_Icon *ic);
static void         _ibox_icon_fill_label(IBox_Icon *ic);
static void         _ibox_icon_fill_preview(IBox_Icon *ic, Eina_Bool is_retry);
static void         _ibox_icon_empty(IBox_Icon *ic);
static void         _ibox_icon_signal_emit(IBox_Icon *ic, char *sig, char *src);
static Eina_List   *_ibox_zone_find(E_Zone *zone);
static void         _ibox_cb_obj_moveresize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_menu_configuration(void *data, E_Menu *m, E_Menu_Item *mi);
static void         _ibox_cb_icon_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_icon_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_icon_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_icon_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_icon_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_icon_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_icon_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _ibox_cb_drag_finished(E_Drag *drag, int dropped);
static void         _ibox_inst_cb_enter(void *data, const char *type, void *event_info);
static void         _ibox_inst_cb_move(void *data, const char *type, void *event_info);
static void         _ibox_inst_cb_leave(void *data, const char *type, void *event_info);
static void         _ibox_inst_cb_drop(void *data, const char *type, void *event_info);
static void         _ibox_drop_position_update(Instance *inst, Evas_Coord x, Evas_Coord y);
static void         _ibox_inst_cb_scroll(void *data);
static Eina_Bool    _ibox_cb_event_client_add(void *data, int type, void *event);
static Eina_Bool    _ibox_cb_event_client_remove(void *data, int type, void *event);
static Eina_Bool    _ibox_cb_event_client_iconify(void *data, int type, void *event);
static Eina_Bool    _ibox_cb_event_client_uniconify(void *data, int type, void *event);
static Eina_Bool    _ibox_cb_event_client_zone_set(void *data, int type, void *event);
static Eina_Bool    _ibox_cb_event_desk_show(void *data, int type, void *event);
static Config_Item *_ibox_config_item_get(const char *id);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

Config *ibox_config = NULL;

static void
_ibox_cb_iconify_end_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   E_Client *ec = data;

   evas_object_layer_set(ec->frame, ec->layer);
   ec->layer_block = 0;
   if (ec->iconic)
     evas_object_hide(ec->frame);
}

static Eina_Bool
_ibox_cb_iconify_provider(void *data, Evas_Object *obj, const char *signal)
{
   Instance *inst = data;
   IBox_Icon *ic;
   Evas_Coord ox, oy, ow, oh;
   Eina_List *l;
   E_Client *ec;
   ox = oy = ow = oh = 0;

   ec = e_comp_object_client_get(obj);
   if (ec->zone != inst->gcc->gadcon->zone) return EINA_FALSE;
   if (!strcmp(signal, "e,action,uniconify"))
     {
        EINA_LIST_FOREACH(inst->ibox->icons, l, ic)
          {
             if (ic->client == ec)
               {
                  evas_object_geometry_get(ic->o_holder, &ox, &oy, &ow, &oh);
                  break;
               }
          }
     }
   else
     {
        // XXX: ibox doesn't know yet where a new icon might go... so assume right
        // edge of ibox
        evas_object_geometry_get(inst->ibox->o_box, &ox, &oy, &ow, &oh);
        ox += ow - 1;
        oy += (oh / 2);
     }
   ec->layer_block = 1;
   evas_object_layer_set(ec->frame, E_LAYER_CLIENT_PRIO);
   e_comp_object_effect_set(ec->frame, "iconify/ibox");
   e_comp_object_effect_params_set(ec->frame, 1, (int[]){ec->x, ec->y, ec->w, ec->h, ox, oy, ow, oh}, 8);
   e_comp_object_effect_params_set(ec->frame, 0, (int[]){!!strcmp(signal, "e,action,iconify")}, 1);
   e_comp_object_effect_start(ec->frame, _ibox_cb_iconify_end_cb, ec);
   return EINA_TRUE;
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   IBox *b;
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   Evas_Coord x, y, w, h;
   const char *drop[] = { "enlightenment/border" };
   Config_Item *ci;

   inst = E_NEW(Instance, 1);

   ci = _ibox_config_item_get(id);
   inst->ci = ci;

   b = _ibox_new(gc->o_container ?: e_comp->elm, gc->zone);
   b->inst = inst;
   inst->ibox = b;
   o = b->o_box;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   ci->gcc = gcc;

   inst->gcc = gcc;
   inst->o_ibox = o;
   inst->orient = E_GADCON_ORIENT_HORIZ;
   _ibox_fill(b);

   evas_object_geometry_get(o, &x, &y, &w, &h);
   inst->drop_handler =
     e_drop_handler_add(E_OBJECT(inst->gcc), NULL, inst,
                        _ibox_inst_cb_enter, _ibox_inst_cb_move,
                        _ibox_inst_cb_leave, _ibox_inst_cb_drop,
                        drop, 1, x, y, w, h);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOVE,
                                  _ibox_cb_obj_moveresize, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE,
                                  _ibox_cb_obj_moveresize, inst);
   ibox_config->instances = eina_list_append(ibox_config->instances, inst);
   // add highest priority iconify provider - tasks and ibar can do this
   // but lower priority
   inst->iconify_provider = e_comp_object_effect_mover_add(100, "e,action,*iconify", _ibox_cb_iconify_provider, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   e_comp_object_effect_mover_del(inst->iconify_provider);
   inst->ci->gcc = NULL;
   ibox_config->instances = eina_list_remove(ibox_config->instances, inst);
   e_drop_handler_del(inst->drop_handler);
   _ibox_free(inst->ibox);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   Instance *inst;

   inst = gcc->data;
   if ((int)orient != -1) inst->orient = orient;

   switch (inst->orient)
     {
      case E_GADCON_ORIENT_FLOAT:
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
        _ibox_orient_set(inst->ibox, 1);
        e_gadcon_client_aspect_set(gcc, eina_list_count(inst->ibox->icons) * 16, 16);
        break;

      case E_GADCON_ORIENT_VERT:
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_RB:
        _ibox_orient_set(inst->ibox, 0);
        e_gadcon_client_aspect_set(gcc, 16, eina_list_count(inst->ibox->icons) * 16);
        break;

      default:
        break;
     }
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("IBox");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;
   char buf[PATH_MAX];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-ibox.edj",
            e_module_dir_get(ibox_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   static char buf[4096];

   snprintf(buf, sizeof(buf), "%s.%d", client_class->name,
            eina_list_count(ibox_config->instances) + 1);
   return buf;
}

static void
_gc_id_del(const E_Gadcon_Client_Class *client_class EINA_UNUSED, const char *id EINA_UNUSED)
{
/* yes - don't do this. on shutdown gadgets are deleted and this means config
 * for them is deleted - that means empty config is saved. keep them around
 * as if u add a gadget back it can pick up its old config again
   Config_Item *ci;

   ci = _ibox_config_item_get(id);
   if (ci)
     {
        if (ci->id) eina_stringshare_del(ci->id);
        ibox_config->items = eina_list_remove(ibox_config->items, ci);
     }
 */
}

static IBox *
_ibox_new(Evas_Object *parent, E_Zone *zone)
{
   IBox *b;

   b = E_NEW(IBox, 1);
   b->o_box = elm_box_add(e_win_evas_object_win_get(parent));
   elm_box_homogeneous_set(b->o_box, 1);
   elm_box_horizontal_set(b->o_box, 1);
   elm_box_align_set(b->o_box, 0.5, 0.5);
   b->zone = zone;
   return b;
}

static void
_ibox_free(IBox *b)
{
   _ibox_empty(b);
   evas_object_del(b->o_box);
   if (b->o_drop) evas_object_del(b->o_drop);
   if (b->o_drop_over) evas_object_del(b->o_drop_over);
   if (b->o_empty) evas_object_del(b->o_empty);
   free(b);
}

static void
_ibox_cb_empty_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   IBox *b;
   E_Menu *m;
   E_Menu_Item *mi;
   int cx, cy;

   ev = event_info;
   b = data;
   if (ev->button != 3) return;

   m = e_menu_new();
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Settings"));
   e_util_menu_item_theme_icon_set(mi, "configure");
   e_menu_item_callback_set(mi, _ibox_cb_menu_configuration, b);

   m = e_gadcon_client_util_menu_items_append(b->inst->gcc, m, 0);

   e_gadcon_canvas_zone_geometry_get(b->inst->gcc->gadcon,
                                     &cx, &cy, NULL, NULL);
   e_menu_activate_mouse(m,
                         e_zone_current_get(),
                         cx + ev->output.x, cy + ev->output.y, 1, 1,
                         E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
   evas_event_feed_mouse_up(b->inst->gcc->gadcon->evas, ev->button,
                            EVAS_BUTTON_NONE, ev->timestamp, NULL);
}

static void
_ibox_empty_handle(IBox *b)
{
   if (!b->o_empty)
     {
        b->o_empty = evas_object_rectangle_add(evas_object_evas_get(b->o_box));
        evas_object_event_callback_add(b->o_empty, EVAS_CALLBACK_MOUSE_DOWN, _ibox_cb_empty_mouse_down, b);
        evas_object_color_set(b->o_empty, 0, 0, 0, 0);
        E_EXPAND(b->o_empty);
        E_FILL(b->o_empty);
     }
   if (b->icons) return;
   evas_object_show(b->o_empty);
   elm_box_pack_end(b->o_box, b->o_empty);
}

static void
_ibox_fill(IBox *b)
{
   IBox_Icon *ic;
   E_Client *ec;
   int ok;
   int mw, mh, h;

   E_CLIENT_FOREACH(ec)
     {
        if (e_client_util_ignored_get(ec)) continue;
        ok = 0;
        if ((b->inst->ci->show_zone == 0) && (ec->iconic))
          {
             ok = 1;
          }
        else if ((b->inst->ci->show_zone == 1) && (ec->iconic))
          {
             if (ec->sticky)
               {
                  ok = 1;
               }
             else if ((b->inst->ci->show_desk == 0) && (ec->zone == b->zone))
               {
                  ok = 1;
               }
             else if ((b->inst->ci->show_desk == 1) && (ec->zone == b->zone) &&
                      (ec->desk == e_desk_current_get(b->zone)))
               {
                  ok = 1;
               }
          }

        if (ok)
          {
             ic = _ibox_icon_new(b, ec);
             b->icons = eina_list_append(b->icons, ic);
             elm_box_pack_end(b->o_box, ic->o_holder);
          }
     }

   _ibox_empty_handle(b);
   _ibox_resize_handle(b);
   if (!b->inst->gcc) return;
   if (!b->inst->ci->expand_on_desktop) return;
   if (!e_gadcon_site_is_desktop(b->inst->gcc->gadcon->location->site)) return;
   elm_box_recalculate(b->o_box);
   evas_object_size_hint_min_get(b->o_box, &mw, &mh);
   if ((!mw) && (!mh))
     evas_object_geometry_get(b->o_box, NULL, NULL, &mw, &mh);
   evas_object_geometry_get(b->inst->gcc->o_frame, NULL, NULL, NULL, &h);
   evas_object_resize(b->inst->gcc->o_frame, MIN(mw, b->inst->gcc->gadcon->zone->w), MAX(h, mh));
}

static void
_ibox_empty(IBox *b)
{
   E_FREE_LIST(b->icons, _ibox_icon_free);
   _ibox_empty_handle(b);
}

static void
_ibox_orient_set(IBox *b, int horizontal)
{
   elm_box_horizontal_set(b->o_box, horizontal);
   elm_box_align_set(b->o_box, 0.5, 0.5);
}

static void
_ibox_resize_handle(IBox *b)
{
   Eina_List *l;
   IBox_Icon *ic;
   int w, h;

   evas_object_geometry_get(b->o_box, NULL, NULL, &w, &h);
   if (elm_box_horizontal_get(b->o_box))
     w = h;
   else
     h = w;
   if (w || h)
     evas_object_size_hint_min_set(b->o_empty, w, h);
   if (b->icons && evas_object_visible_get(b->o_empty))
     {
        elm_box_unpack(b->o_box, b->o_empty);
        evas_object_hide(b->o_empty);
     }
   EINA_LIST_FOREACH(b->icons, l, ic)
     {
        evas_object_size_hint_min_set(ic->o_holder, w, h);
        evas_object_size_hint_max_set(ic->o_holder, w, h);
     }
}

static void
_ibox_instance_drop_zone_recalc(Instance *inst)
{
   Evas_Coord x, y, w, h;

   e_gadcon_client_viewport_geometry_get(inst->gcc, &x, &y, &w, &h);
   e_drop_handler_geometry_set(inst->drop_handler, x, y, w, h);
}

static IBox_Icon *
_ibox_icon_find(IBox *b, E_Client *ec)
{
   Eina_List *l;
   IBox_Icon *ic;

   EINA_LIST_FOREACH(b->icons, l, ic)
     {
        if (ic->client == ec) return ic;
     }
   return NULL;
}

static IBox_Icon *
_ibox_icon_at_coord(IBox *b, Evas_Coord x, Evas_Coord y)
{
   Eina_List *l;
   IBox_Icon *ic;

   EINA_LIST_FOREACH(b->icons, l, ic)
     {
        Evas_Coord dx, dy, dw, dh;

        evas_object_geometry_get(ic->o_holder, &dx, &dy, &dw, &dh);
        if (E_INSIDE(x, y, dx, dy, dw, dh)) return ic;
     }
   return NULL;
}

static Eina_Bool
_ibox_cb_icon_fill_timer(void *data)
{
   IBox_Icon *ic = data;

   _ibox_icon_fill(ic);

   return EINA_FALSE;
}

static IBox_Icon *
_ibox_icon_new(IBox *b, E_Client *ec)
{
   IBox_Icon *ic;

   ic = E_NEW(IBox_Icon, 1);
   e_object_ref(E_OBJECT(ec));
   ic->ibox = b;
   ic->client = ec;
   ic->o_holder = edje_object_add(evas_object_evas_get(b->o_box));
   E_FILL(ic->o_holder);
   e_theme_edje_object_set(ic->o_holder, "base/theme/modules/ibox",
                           "e/modules/ibox/icon");
   evas_object_event_callback_add(ic->o_holder, EVAS_CALLBACK_MOUSE_IN, _ibox_cb_icon_mouse_in, ic);
   evas_object_event_callback_add(ic->o_holder, EVAS_CALLBACK_MOUSE_OUT, _ibox_cb_icon_mouse_out, ic);
   evas_object_event_callback_add(ic->o_holder, EVAS_CALLBACK_MOUSE_DOWN, _ibox_cb_icon_mouse_down, ic);
   evas_object_event_callback_add(ic->o_holder, EVAS_CALLBACK_MOUSE_UP, _ibox_cb_icon_mouse_up, ic);
   evas_object_event_callback_add(ic->o_holder, EVAS_CALLBACK_MOUSE_MOVE, _ibox_cb_icon_mouse_move, ic);
   evas_object_event_callback_add(ic->o_holder, EVAS_CALLBACK_MOVE, _ibox_cb_icon_move, ic);
   evas_object_event_callback_add(ic->o_holder, EVAS_CALLBACK_RESIZE, _ibox_cb_icon_resize, ic);
   evas_object_show(ic->o_holder);

   ic->o_holder2 = edje_object_add(evas_object_evas_get(b->o_box));
   e_theme_edje_object_set(ic->o_holder2, "base/theme/modules/ibox",
                           "e/modules/ibox/icon_overlay");
   evas_object_layer_set(ic->o_holder2, 9999);
   evas_object_pass_events_set(ic->o_holder2, 1);
   evas_object_show(ic->o_holder2);

   ecore_timer_add(0.1, _ibox_cb_icon_fill_timer, ic);
   return ic;
}

static void
_ibox_icon_free(IBox_Icon *ic)
{
   if (ic->ibox->ic_drop_before == ic)
     ic->ibox->ic_drop_before = NULL;
   _ibox_icon_empty(ic);
   evas_object_del(ic->o_holder);
   evas_object_del(ic->o_holder2);
   e_object_unref(E_OBJECT(ic->client));
   free(ic);
}

static void
_ibox_icon_fill_icon(IBox_Icon *ic)
{
   ic->o_icon = e_client_icon_add(ic->client, evas_object_evas_get(ic->ibox->o_box));
   edje_object_part_swallow(ic->o_holder, "e.swallow.content", ic->o_icon);
   evas_object_pass_events_set(ic->o_icon, 1);
   evas_object_show(ic->o_icon);
   ic->o_icon2 = e_client_icon_add(ic->client, evas_object_evas_get(ic->ibox->o_box));
   edje_object_part_swallow(ic->o_holder2, "e.swallow.content", ic->o_icon2);
   evas_object_pass_events_set(ic->o_icon2, 1);
   evas_object_show(ic->o_icon2);
}

static Eina_Bool
_ibox_icon_fill_timer(void *data)
{
   IBox_Icon *ic = data;

   _ibox_icon_fill_preview(ic, EINA_TRUE);

   return EINA_FALSE;
}

static void
_ibox_icon_fill_preview(IBox_Icon *ic, Eina_Bool is_retry)
{
   E_Client *ec;
   Evas_Object *img, *img2;
   int w, h;

   ec = ic->client;

   img = e_comp_object_util_frame_mirror_add(ec->frame);
   if (!img && !is_retry)
     {
        ecore_timer_add(0.5, _ibox_icon_fill_timer, ic);
        return;
     }
   w = ec->client.w;
   h = ec->client.h;
   if (ec->shaded)
     evas_object_geometry_get(ec->frame_object, NULL, NULL, &w, &h);
   evas_object_size_hint_aspect_set(img, EVAS_ASPECT_CONTROL_BOTH, w, h);
   evas_object_size_hint_max_set(img, w, h);
   ic->o_icon = img;
   edje_object_part_swallow(ic->o_holder, "e.swallow.preview", ic->o_icon);
   evas_object_pass_events_set(ic->o_icon, 1);
   evas_object_show(ic->o_icon);

   img2 = e_comp_object_util_frame_mirror_add(ec->frame);
   evas_object_size_hint_aspect_set(img2, EVAS_ASPECT_CONTROL_BOTH, w, h);
   ic->o_icon2 = img2;
   edje_object_part_swallow(ic->o_holder2, "e.swallow.preview", ic->o_icon2);
   evas_object_pass_events_set(ic->o_icon2, 1);
   evas_object_show(ic->o_icon2);
}

static void
_ibox_icon_fill(IBox_Icon *ic)
{
   if ((ic->ibox->inst->ci->show_preview) && (edje_object_part_exists(ic->o_holder, "e.swallow.preview")))
     _ibox_icon_fill_preview(ic, EINA_FALSE);
   else
     _ibox_icon_fill_icon(ic);

   _ibox_icon_fill_label(ic);

   if (ic->client->urgent)
     {
        e_gadcon_urgent_show(ic->ibox->inst->gcc->gadcon);
        edje_object_signal_emit(ic->o_holder, "e,state,urgent", "e");
        edje_object_signal_emit(ic->o_holder2, "e,state,urgent", "e");
     }
}

static void
_ibox_icon_fill_label(IBox_Icon *ic)
{
   const char *label = NULL;

   switch (ic->ibox->inst->ci->icon_label)
     {
      case 0:
        label = ic->client->netwm.name;
        if (!label)
          label = ic->client->icccm.name;
        break;

      case 1:
        label = ic->client->icccm.title;
        break;

      case 2:
        label = ic->client->icccm.class;
        break;

      case 3:
        label = ic->client->netwm.icon_name;
        if (!label)
          label = ic->client->icccm.icon_name;
        break;

      case 4:
        label = e_client_util_name_get(ic->client);
        break;
     }

   if (!label) label = "?";
   edje_object_part_text_set(ic->o_holder2, "e.text.label", label);
}

static void
_ibox_icon_empty(IBox_Icon *ic)
{
   if (ic->o_icon) evas_object_del(ic->o_icon);
   if (ic->o_icon2) evas_object_del(ic->o_icon2);
   ic->o_icon = NULL;
   ic->o_icon2 = NULL;
}

static void
_ibox_icon_signal_emit(IBox_Icon *ic, char *sig, char *src)
{
   if (ic->o_holder)
     edje_object_signal_emit(ic->o_holder, sig, src);
   if (ic->o_icon && e_icon_edje_get(ic->o_icon))
     e_icon_edje_emit(ic->o_icon, sig, src);
   if (ic->o_holder2)
     edje_object_signal_emit(ic->o_holder2, sig, src);
   if (ic->o_icon2 && e_icon_edje_get(ic->o_icon2))
     e_icon_edje_emit(ic->o_icon2, sig, src);
}

static Eina_List *
_ibox_zone_find(E_Zone *zone)
{
   Eina_List *ibox = NULL;
   Eina_List *l;
   Instance *inst;

   EINA_LIST_FOREACH(ibox_config->instances, l, inst)
     {
        if (inst->ci->show_zone == 0)
          ibox = eina_list_append(ibox, inst->ibox);
        else if ((inst->ci->show_zone == 1) && (inst->ibox->zone == zone))
          ibox = eina_list_append(ibox, inst->ibox);
     }
   return ibox;
}

static void
_ibox_cb_obj_moveresize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst;

   inst = data;
   _ibox_resize_handle(inst->ibox);
   _ibox_instance_drop_zone_recalc(inst);
}

static void
_ibox_cb_icon_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   IBox_Icon *ic;

   ic = data;
   _ibox_icon_signal_emit(ic, "e,state,focused", "e");
   if (ic->ibox->inst->ci->show_label)
     {
        _ibox_icon_fill_label(ic);
        _ibox_icon_signal_emit(ic, "e,action,show,label", "e");
     }
}

static void
_ibox_cb_icon_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   IBox_Icon *ic;

   ic = data;
   _ibox_icon_signal_emit(ic, "e,state,unfocused", "e");
   if (ic->ibox->inst->ci->show_label)
     _ibox_icon_signal_emit(ic, "e,action,hide,label", "e");
}

static void
_ibox_cb_icon_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   IBox_Icon *ic;

   ev = event_info;
   ic = data;
   if (ev->button == 1)
     {
        ic->drag.x = ev->output.x;
        ic->drag.y = ev->output.y;
        ic->drag.start = 1;
        ic->drag.dnd = 0;
     }
   else if (ev->button == 3)
     {
        E_Menu *m;
        E_Menu_Item *mi;
        int cx, cy;

        m = e_menu_new();

        /* FIXME: other icon options go here too */
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("Settings"));
        e_util_menu_item_theme_icon_set(mi, "configure");
        e_menu_item_callback_set(mi, _ibox_cb_menu_configuration, ic->ibox);

        m = e_gadcon_client_util_menu_items_append(ic->ibox->inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(ic->ibox->inst->gcc->gadcon,
                                          &cx, &cy, NULL, NULL);
        e_menu_activate_mouse(m,
                              e_zone_current_get(),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
     }
}

static void
_ibox_cb_icon_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   IBox_Icon *ic;

   ev = event_info;
   ic = data;
   if ((ev->button == 1) && (!ic->drag.dnd))
     {
        e_client_uniconify(ic->client);
        evas_object_focus_set(ic->client->frame, 1);
     }
}

static void
_ibox_cb_icon_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   IBox_Icon *ic;

   ev = event_info;
   ic = data;
   if (ic->drag.start)
     {
        int dx, dy;

        dx = ev->cur.output.x - ic->drag.x;
        dy = ev->cur.output.y - ic->drag.y;
        if (((dx * dx) + (dy * dy)) >
            (e_config->drag_resist * e_config->drag_resist))
          {
             E_Drag *d;
             Evas_Object *o;
             Evas_Coord x, y, w, h;
             const char *drag_types[] = { "enlightenment/border" };
             E_Gadcon_Client *gcc;
             ic->drag.dnd = 1;
             ic->drag.start = 0;

             evas_object_geometry_get(ic->o_icon, &x, &y, &w, &h);
             d = e_drag_new(x, y, drag_types, 1,
                            ic->client, -1, NULL, _ibox_cb_drag_finished);
             d->button_mask = evas_pointer_button_down_mask_get(e_comp->evas);

             if (!ic->ibox->inst->ci->show_preview)
               o = e_client_icon_add(ic->client, e_drag_evas_get(d));
             else
               {
                  E_Client *ec = ic->client;
                  o = e_comp_object_util_frame_mirror_add(ec->frame);
                  evas_object_size_hint_aspect_set(o, EVAS_ASPECT_CONTROL_BOTH, ec->client.w, ec->client.h);
                  evas_object_size_hint_max_set(o, w, h);
               }
             e_drag_object_set(d, o);
             e_drag_resize(d, w, h);
             e_drag_start(d, ic->drag.x, ic->drag.y);
             e_object_ref(E_OBJECT(ic->client));
             gcc = ic->ibox->inst->gcc;
             _gc_orient(gcc, -1);
          }
     }
}

static void
_ibox_cb_icon_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   IBox_Icon *ic;
   Evas_Coord x, y;

   ic = data;
   evas_object_geometry_get(ic->o_holder, &x, &y, NULL, NULL);
   evas_object_move(ic->o_holder2, x, y);
   evas_object_raise(ic->o_holder2);
}

static void
_ibox_cb_icon_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   IBox_Icon *ic;
   Evas_Coord w, h;

   ic = data;
   evas_object_geometry_get(ic->o_holder, NULL, NULL, &w, &h);
   evas_object_resize(ic->o_holder2, w, h);
   evas_object_raise(ic->o_holder2);
}

static void
_ibox_cb_drag_finished(E_Drag *drag, int dropped)
{
   E_Client *ec;

   ec = drag->data;
   if (!dropped) e_client_uniconify(ec);
   e_object_unref(E_OBJECT(ec));
}

static void
_ibox_cb_drop_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   IBox *b;
   Evas_Coord x, y;

   b = data;
   evas_object_geometry_get(b->o_drop, &x, &y, NULL, NULL);
   evas_object_move(b->o_drop_over, x, y);
}

static void
_ibox_cb_drop_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   IBox *b;
   Evas_Coord w, h;

   b = data;
   evas_object_geometry_get(b->o_drop, NULL, NULL, &w, &h);
   evas_object_resize(b->o_drop_over, w, h);
}

static void
_ibox_inst_cb_scroll(void *data)
{
   Instance *inst;

   /* Update the position of the dnd to handle for autoscrolling
    * gadgets. */
   inst = data;
   _ibox_drop_position_update(inst, inst->ibox->dnd_x, inst->ibox->dnd_y);
}

static void
_ibox_drop_position_update(Instance *inst, Evas_Coord x, Evas_Coord y)
{
   IBox_Icon *ic;

   inst->ibox->dnd_x = x;
   inst->ibox->dnd_y = y;

   if (inst->ibox->o_drop)
     elm_box_unpack(inst->ibox->o_box, inst->ibox->o_drop);
   ic = _ibox_icon_at_coord(inst->ibox, x, y);
   inst->ibox->ic_drop_before = ic;
   if (ic)
     {
        Evas_Coord ix, iy, iw, ih;
        int before = 0;

        evas_object_geometry_get(ic->o_holder, &ix, &iy, &iw, &ih);
        if (elm_box_horizontal_get(inst->ibox->o_box))
          {
             if (x < (ix + iw)) before = 1;
          }
        else
          {
             if (y < (iy + ih)) before = 1;
          }
        if (before)
          elm_box_pack_before(inst->ibox->o_box, inst->ibox->o_drop, ic->o_holder);
        else
          elm_box_pack_after(inst->ibox->o_box, inst->ibox->o_drop, ic->o_holder);
        inst->ibox->drop_before = before;
     }
   else elm_box_pack_end(inst->ibox->o_box, inst->ibox->o_drop);
   evas_object_size_hint_min_set(inst->ibox->o_drop, 1, 1);
   _ibox_resize_handle(inst->ibox);
   _gc_orient(inst->gcc, -1);
}

static void
_ibox_inst_cb_enter(void *data, const char *type EINA_UNUSED, void *event_info)
{
   E_Event_Dnd_Enter *ev;
   Instance *inst;
   E_Client *ec;
   Evas_Object *o, *o2;

   ev = event_info;
   inst = data;

   ec = ev->data;

   if (_ibox_icon_find(inst->ibox, ec)) return;

   o = edje_object_add(evas_object_evas_get(inst->ibox->o_box));
   inst->ibox->o_drop = o;
   E_EXPAND(inst->ibox->o_drop);
   E_FILL(inst->ibox->o_drop);
   o2 = edje_object_add(evas_object_evas_get(inst->ibox->o_box));
   inst->ibox->o_drop_over = o2;
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOVE, _ibox_cb_drop_move, inst->ibox);
   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE, _ibox_cb_drop_resize, inst->ibox);
   e_theme_edje_object_set(o, "base/theme/modules/ibox",
                           "e/modules/ibox/drop");
   e_theme_edje_object_set(o2, "base/theme/modules/ibox",
                           "e/modules/ibox/drop_overlay");
   evas_object_layer_set(o2, 19999);
   evas_object_show(o);
   evas_object_show(o2);
   _ibox_drop_position_update(inst, ev->x, ev->y);
   e_gadcon_client_autoscroll_cb_set(inst->gcc, _ibox_inst_cb_scroll, inst);
   e_gadcon_client_autoscroll_update(inst->gcc, ev->x, ev->y);
}

static void
_ibox_inst_cb_move(void *data, const char *type EINA_UNUSED, void *event_info)
{
   E_Event_Dnd_Move *ev;
   Instance *inst;

   ev = event_info;
   inst = data;

   _ibox_drop_position_update(inst, ev->x, ev->y);
   e_gadcon_client_autoscroll_update(inst->gcc, ev->x, ev->y);
}

static void
_ibox_inst_cb_leave(void *data, const char *type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst;

   inst = data;
   inst->ibox->ic_drop_before = NULL;
   evas_object_del(inst->ibox->o_drop);
   inst->ibox->o_drop = NULL;
   evas_object_del(inst->ibox->o_drop_over);
   inst->ibox->o_drop_over = NULL;
   e_gadcon_client_autoscroll_cb_set(inst->gcc, NULL, NULL);
   _ibox_resize_handle(inst->ibox);
   _gc_orient(inst->gcc, -1);
}

static void
_ibox_inst_cb_drop(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Drop *ev;
   Instance *inst;
   E_Client *ec = NULL;
   IBox *b;
   IBox_Icon *ic, *ic2;
   Eina_List *l;

   ev = event_info;
   inst = data;
   if (!strcmp(type, "enlightenment/border"))
     {
        ec = ev->data;
        if (!ec) return;
     }
   else return;

   if (!ec->iconic) e_client_iconify(ec);

   ic2 = inst->ibox->ic_drop_before;
   if (ic2)
     {
        /* Add new eapp before this icon */
        if (!inst->ibox->drop_before)
          {
             EINA_LIST_FOREACH(inst->ibox->icons, l, ic)
               {
                  if (ic == ic2)
                    {
                       ic2 = eina_list_data_get(l->next);
                       break;
                    }
               }
          }
        if (!ic2) goto atend;
        b = inst->ibox;
        if (_ibox_icon_find(b, ec)) return;
        ic = _ibox_icon_new(b, ec);
        if (!ic) return;
        b->icons = eina_list_prepend_relative(b->icons, ic, ic2);
        elm_box_pack_before(b->o_box, ic->o_holder, ic2->o_holder);
     }
   else
     {
atend:
        b = inst->ibox;
        if (_ibox_icon_find(b, ec)) return;
        ic = _ibox_icon_new(b, ec);
        if (!ic) return;
        b->icons = eina_list_append(b->icons, ic);
        elm_box_pack_end(b->o_box, ic->o_holder);
     }

   evas_object_del(inst->ibox->o_drop);
   inst->ibox->o_drop = NULL;
   evas_object_del(inst->ibox->o_drop_over);
   inst->ibox->o_drop_over = NULL;
   _ibox_empty_handle(b);
   e_gadcon_client_autoscroll_cb_set(inst->gcc, NULL, NULL);
   _ibox_resize_handle(inst->ibox);
   _gc_orient(inst->gcc, -1);
}

static Eina_Bool
_ibox_cb_event_client_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   IBox *b;
   IBox_Icon *ic;
   E_Desk *desk;
   Eina_List *ibox;

   /* add if iconic */
   if (!ev->ec->iconic) return ECORE_CALLBACK_RENEW;
   if (!ev->ec->zone) return ECORE_CALLBACK_RENEW;
   desk = e_desk_current_get(ev->ec->zone);

   ibox = _ibox_zone_find(ev->ec->zone);
   EINA_LIST_FREE(ibox, b)
     {
        if (_ibox_icon_find(b, ev->ec)) continue;
        if ((b->inst->ci->show_desk) && (ev->ec->desk != desk) && (!ev->ec->sticky)) continue;
        ic = _ibox_icon_new(b, ev->ec);
        if (!ic) continue;
        b->icons = eina_list_append(b->icons, ic);
        elm_box_pack_end(b->o_box, ic->o_holder);
        _ibox_empty_handle(b);
        _ibox_resize_handle(b);
        _gc_orient(b->inst->gcc, -1);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ibox_cb_event_client_remove(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   IBox *b;
   IBox_Icon *ic;
   Eina_List *ibox;

   ev = event;
   /* find icon and remove if there */
   ibox = _ibox_zone_find(ev->ec->zone);
   EINA_LIST_FREE(ibox, b)
     {
        ic = _ibox_icon_find(b, ev->ec);
        if (!ic) continue;
        b->icons = eina_list_remove(b->icons, ic);
        _ibox_icon_free(ic);
        _ibox_empty_handle(b);
        _ibox_resize_handle(b);
        _gc_orient(b->inst->gcc, -1);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ibox_cb_event_client_iconify(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   IBox *b;
   IBox_Icon *ic;
   Eina_List *ibox;
   E_Desk *desk;

   ev = event;
   /* add icon for ibox for right zone */
   /* do some sort of anim when iconifying */
   desk = e_desk_current_get(ev->ec->zone);
   ibox = _ibox_zone_find(ev->ec->zone);
   EINA_LIST_FREE(ibox, b)
     {
        int h, mw, mh;
        if (_ibox_icon_find(b, ev->ec)) continue;
        if ((b->inst->ci->show_desk) && (ev->ec->desk != desk) && (!ev->ec->sticky)) continue;
        ic = _ibox_icon_new(b, ev->ec);
        if (!ic) continue;
        b->icons = eina_list_append(b->icons, ic);
        elm_box_pack_end(b->o_box, ic->o_holder);
        _ibox_empty_handle(b);
        _ibox_resize_handle(b);
        _gc_orient(b->inst->gcc, -1);
        if (!b->inst->ci->expand_on_desktop) continue;
        if (!e_gadcon_site_is_desktop(b->inst->gcc->gadcon->location->site)) continue;
        elm_box_recalculate(b->o_box);
        evas_object_size_hint_min_get(b->o_box, &mw, &mh);
        if ((!mw) && (!mh))
          evas_object_geometry_get(b->o_box, NULL, NULL, &mw, &mh);
        evas_object_geometry_get(b->inst->gcc->o_frame, NULL, NULL, NULL, &h);
        evas_object_resize(b->inst->gcc->o_frame, MIN(mw, b->inst->gcc->gadcon->zone->w), MAX(h, mh));
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ibox_cb_event_client_uniconify(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   IBox *b;
   IBox_Icon *ic;
   Eina_List *ibox;

   ev = event;
   /* del icon for ibox for right zone */
   /* do some sort of anim when uniconifying */
   ibox = _ibox_zone_find(ev->ec->zone);
   EINA_LIST_FREE(ibox, b)
     {
        int mw, mh, h;
        ic = _ibox_icon_find(b, ev->ec);
        if (!ic) continue;
        b->icons = eina_list_remove(b->icons, ic);
        _ibox_icon_free(ic);
        _ibox_empty_handle(b);
        _ibox_resize_handle(b);
        _gc_orient(b->inst->gcc, -1);
        if (!b->inst->ci->expand_on_desktop) continue;
        if (!e_gadcon_site_is_desktop(b->inst->gcc->gadcon->location->site)) continue;
        elm_box_recalculate(b->o_box);
        evas_object_size_hint_min_get(b->o_box, &mw, &mh);
        if ((!mw) && (!mh))
          evas_object_geometry_get(b->o_box, NULL, NULL, &mw, &mh);
        evas_object_geometry_get(b->inst->gcc->o_frame, NULL, NULL, NULL, &h);
        evas_object_resize(b->inst->gcc->o_frame, MIN(mw, b->inst->gcc->gadcon->zone->w), MAX(h, mh));
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ibox_cb_event_client_property(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Client_Property *ev)
{
   IBox *b;
   IBox_Icon *ic;
   Eina_List *ibox;

   if ((ev->property & ~E_CLIENT_PROPERTY_ICON) && (ev->property & ~E_CLIENT_PROPERTY_URGENCY)) return ECORE_CALLBACK_RENEW;
   ibox = _ibox_zone_find(ev->ec->zone);
   EINA_LIST_FREE(ibox, b)
     {
        ic = _ibox_icon_find(b, ev->ec);
        if (!ic) continue;
        if ((ev->property & E_CLIENT_PROPERTY_ICON))
          {
             _ibox_icon_empty(ic);
             _ibox_icon_fill(ic);
             continue;
          }
        if (ev->ec->urgent)
          {
             e_gadcon_urgent_show(b->inst->gcc->gadcon);
             edje_object_signal_emit(ic->o_holder, "e,state,urgent", "e");
             edje_object_signal_emit(ic->o_holder2, "e,state,urgent", "e");
          }
        else
          {
             edje_object_signal_emit(ic->o_holder, "e,state,not_urgent", "e");
             edje_object_signal_emit(ic->o_holder2, "e,state,not_urgent", "e");
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ibox_cb_event_client_zone_set(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client_Zone_Set *ev;

   ev = event;
   /* delete from current zone ibox, add to new one */
   if (ev->ec->iconic)
     {
     }
   return 1;
}

static Eina_Bool
_ibox_cb_event_desk_show(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Desk_Show *ev;
   IBox *b;
   Eina_List *ibox;

   ev = event;
   /* delete all wins from ibox and add only for current desk */
   ibox = _ibox_zone_find(ev->desk->zone);
   EINA_LIST_FREE(ibox, b)
     {
        if (b->inst->ci->show_desk)
          {
             _ibox_empty(b);
             _ibox_fill(b);
             _ibox_resize_handle(b);
             _gc_orient(b->inst->gcc, -1);
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Config_Item *
_ibox_config_item_get(const char *id)
{
   Config_Item *ci;

   GADCON_CLIENT_CONFIG_GET(Config_Item, ibox_config->items, _gadcon_class, id);

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->show_label = 0;
   ci->show_zone = 1;
   ci->show_desk = 0;
   ci->icon_label = 0;
   ibox_config->items = eina_list_append(ibox_config->items, ci);
   return ci;
}

void
_ibox_config_update(Config_Item *ci)
{
   Eina_List *l;
   Instance *inst;

   EINA_LIST_FOREACH(ibox_config->instances, l, inst)
     {
        if (inst->ci != ci) continue;

        _ibox_empty(inst->ibox);
        _ibox_fill(inst->ibox);
        _ibox_resize_handle(inst->ibox);
        _gc_orient(inst->gcc, -1);
     }
}

static void
_ibox_cb_menu_configuration(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   IBox *b;
   int ok = 1;
   Eina_List *l;
   E_Config_Dialog *cfd;

   b = data;
   EINA_LIST_FOREACH(ibox_config->config_dialog, l, cfd)
     {
        if (cfd->data == b->inst->ci)
          {
             ok = 0;
             break;
          }
     }
   if (ok) _config_ibox_module(b->inst->ci);
}

/***************************************************************************/
/**/
/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "IBox"
};

E_API void *
e_modapi_init(E_Module *m)
{
   conf_item_edd = E_CONFIG_DD_NEW("IBox_Config_Item", Config_Item);
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, expand_on_desktop, INT);
   E_CONFIG_VAL(D, T, show_label, INT);
   E_CONFIG_VAL(D, T, show_zone, INT);
   E_CONFIG_VAL(D, T, show_desk, INT);
   E_CONFIG_VAL(D, T, icon_label, INT);
   E_CONFIG_VAL(D, T, show_preview, INT);

   conf_edd = E_CONFIG_DD_NEW("IBox_Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);

   ibox_config = e_config_domain_load("module.ibox", conf_edd);
   if (!ibox_config)
     {
        Config_Item *ci;

        ibox_config = E_NEW(Config, 1);

        ci = E_NEW(Config_Item, 1);
        ci->id = eina_stringshare_add("ibox.1");
        ci->show_label = 0;
        ci->show_zone = 1;
        ci->show_desk = 0;
        ci->icon_label = 0;
        ibox_config->items = eina_list_append(ibox_config->items, ci);
     }

   ibox_config->module = m;

   E_LIST_HANDLER_APPEND(ibox_config->handlers, E_EVENT_CLIENT_ADD, _ibox_cb_event_client_add, NULL);
   E_LIST_HANDLER_APPEND(ibox_config->handlers, E_EVENT_CLIENT_REMOVE, _ibox_cb_event_client_remove, NULL);
   E_LIST_HANDLER_APPEND(ibox_config->handlers, E_EVENT_CLIENT_ICONIFY, _ibox_cb_event_client_iconify, NULL);
   E_LIST_HANDLER_APPEND(ibox_config->handlers, E_EVENT_CLIENT_UNICONIFY, _ibox_cb_event_client_uniconify, NULL);
   E_LIST_HANDLER_APPEND(ibox_config->handlers, E_EVENT_CLIENT_PROPERTY, _ibox_cb_event_client_property, NULL);
   E_LIST_HANDLER_APPEND(ibox_config->handlers, E_EVENT_CLIENT_ZONE_SET, _ibox_cb_event_client_zone_set, NULL);
   E_LIST_HANDLER_APPEND(ibox_config->handlers, E_EVENT_DESK_SHOW, _ibox_cb_event_desk_show, NULL);

/* FIXME: add these later for things taskbar-like functionality
   ibox_config->handlers = eina_list_append
     (ibox_config->handlers, ecore_event_handler_add
      (E_EVENT_CLIENT_DESK_SET, _ibox_cb_event_client_zone_set, NULL));
   ibox_config->handlers = eina_list_append
     (ibox_config->handlers, ecore_event_handler_add
      (E_EVENT_CLIENT_SHOW, _ibox_cb_event_client_zone_set, NULL));
   ibox_config->handlers = eina_list_append
     (ibox_config->handlers, ecore_event_handler_add
      (E_EVENT_CLIENT_HIDE, _ibox_cb_event_client_zone_set, NULL));
   ibox_config->handlers = eina_list_append
     (ibox_config->handlers, ecore_event_handler_add
      (E_EVENT_CLIENT_STACK, _ibox_cb_event_client_zone_set, NULL));
   ibox_config->handlers = eina_list_append
     (ibox_config->handlers, ecore_event_handler_add
      (E_EVENT_CLIENT_STICK, _ibox_cb_event_client_zone_set, NULL));
 */
   e_gadcon_provider_register(&_gadcon_class);
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   Config_Item *ci;
   e_gadcon_provider_unregister(&_gadcon_class);

   E_FREE_LIST(ibox_config->handlers, ecore_event_handler_del);

   while (ibox_config->config_dialog)
     /* there is no need to eves_list_remove_list. It is done implicitly in
      * dialog _free_data function
      */
     e_object_del(E_OBJECT(ibox_config->config_dialog->data));

   EINA_LIST_FREE(ibox_config->items, ci)
     {
        eina_stringshare_del(ci->id);
        free(ci);
     }

   E_FREE(ibox_config);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.ibox", conf_edd, ibox_config);
   return 1;
}

