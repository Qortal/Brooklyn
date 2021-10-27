#include "e.h"
#include "e_mod_main.h"

typedef struct _Instance Instance;

struct _Instance
{
   EINA_INLIST;

   E_Gadcon_Client *gcc;
   Evas_Object     *o_button;

   Evry_Window     *win;
   Gadget_Config   *cfg;
   E_Config_Dialog *cfd;

   Eina_Bool        mouse_down E_BITFIELD;
   Evas_Coord       mouse_down_x, mouse_down_y;

   double           hide_start;
   int              hide_x, hide_y;

   Eina_List       *handlers;

   Eina_Bool        hidden;
   Eina_Bool        animating;
   Eina_Bool        illume_mode;
};

static void             _button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void             _button_cb_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool        _cb_focus_out(void *data, int type EINA_UNUSED, void *event);
static void _del_func(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void             _gc_shutdown(E_Gadcon_Client *gcc);
static void             _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char      *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object     *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char      *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static Gadget_Config   *_conf_item_get(const char *id);

static void             _conf_dialog(Instance *inst);
static Eina_Inlist *instances = NULL;

static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
   "evry-starter",
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

static Eina_Bool
_illume_running(void)
{
   /* hack to find out out if illume is running, dont grab if
      this is the case... */

   Eina_List *l;
   E_Module *m;

   EINA_LIST_FOREACH (e_module_list(), l, m)
     if (!strcmp(m->name, "illume2") && m->enabled)
       return EINA_TRUE;

   return EINA_FALSE;
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   Evry_Plugin *p;

   inst = E_NEW(Instance, 1);
   inst->cfg = _conf_item_get(id);

   o = edje_object_add(gc->evas);
   e_theme_edje_object_set(o, "base/theme/modules/everything", "e/modules/everything/gadget");

   if (((inst->cfg->plugin) && (strcmp(inst->cfg->plugin, "Start") != 0)) &&
       (p = evry_plugin_find(inst->cfg->plugin)))
     {
        Evas_Object *oo = evry_util_icon_get(EVRY_ITEM(p), gc->evas);
        if (oo)
          {
             edje_object_part_swallow(o, "e.swallow.icon", oo);
             edje_object_signal_emit(o, "e,state,icon,plugin", "e");
          }
     }

   edje_object_signal_emit(o, "e,state,unfocused", "e");

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;

   inst->gcc = gcc;
   inst->o_button = o;

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
                                  _button_cb_mouse_down, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,
                                  _button_cb_mouse_up, inst);

   if (_illume_running())
     {
        inst->illume_mode = EINA_TRUE;

        inst->handlers = eina_list_append(inst->handlers,
                                          ecore_event_handler_add(E_EVENT_CLIENT_FOCUS_OUT,
                                                                  _cb_focus_out, inst));
     }

   instances = eina_inlist_append(instances, EINA_INLIST_GET(inst));

   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;
   Ecore_Event_Handler *h;

   inst = gcc->data;

   instances = eina_inlist_remove(instances, EINA_INLIST_GET(inst));

   EINA_LIST_FREE (inst->handlers, h)
     ecore_event_handler_del(h);

   if (inst->win)
     {
        evas_object_event_callback_del(inst->win->ewin, EVAS_CALLBACK_DEL, _del_func);
        evry_hide(inst->win, 0);
     }

   evas_object_del(inst->o_button);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   Instance *inst;
   Evas_Coord mw, mh;

   inst = gcc->data;
   mw = 0, mh = 0;
   edje_object_size_min_get(inst->o_button, &mw, &mh);
   if ((mw < 1) || (mh < 1))
     edje_object_size_min_calc(inst->o_button, &mw, &mh);
   if (mw < 4) mw = 4;
   if (mh < 4) mh = 4;
   e_gadcon_client_aspect_set(gcc, mw, mh);
   e_gadcon_client_min_size_set(gcc, mw, mh);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Everything Starter");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas EINA_UNUSED)
{
   Evas_Object *o;
   char buf[PATH_MAX];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-everything-start.edj",
            e_module_dir_get(_mod_evry));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static Gadget_Config *
_conf_item_get(const char *id)
{
   Gadget_Config *ci;

   GADCON_CLIENT_CONFIG_GET(Gadget_Config, evry_conf->gadgets, _gadcon_class, id);

   ci = E_NEW(Gadget_Config, 1);
   ci->id = eina_stringshare_add(id);

   evry_conf->gadgets = eina_list_append(evry_conf->gadgets, ci);

   e_config_save_queue();

   return ci;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   Gadget_Config *ci = NULL;

   ci = _conf_item_get(NULL);
   return ci->id;
}

/***************************************************************************/

static void
_del_func(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst = data;

   e_gadcon_locked_set(inst->gcc->gadcon, 0);

   inst->win = NULL;
   edje_object_signal_emit(inst->o_button, "e,state,unfocused", "e");
}

static void
_cb_menu_configure(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   _conf_dialog(data);
}

static void
_hide_done(void *data, Evas_Object *obj EINA_UNUSED, const char *s EINA_UNUSED, const char *ss EINA_UNUSED)
{
   Instance *inst = data;
   E_Client *ec;

   /* go bac to subject selector */
   evry_selectors_switch(inst->win, -1, 0);
   evry_selectors_switch(inst->win, -1, 0);

   ec = e_win_client_get(inst->win->ewin);
   e_client_iconify(ec);
   e_comp_object_effect_set(ec->frame, "none");
   inst->animating = 0;
}

static void
_evry_hide_func(Evry_Window *win, int finished EINA_UNUSED)
{
   Instance *inst = win->data;
   E_Client *ec;
   int x, y, w, h;

   ec = e_win_client_get(inst->win->ewin);;
   e_comp_object_effect_set(ec->frame, "pane");
   /* set geoms */
   evas_object_geometry_get(inst->win->ewin, &x, &y, &w, &h);
   e_comp_object_effect_params_set(ec->frame, 1,
     (int[]){x, y, w, h,
             ec->zone->w, ec->zone->h,
             inst->hide_x, inst->hide_y}, 8);
   e_comp_object_effect_params_set(ec->frame, 0, (int[]){0}, 1);
   e_comp_object_effect_start(ec->frame, _hide_done, inst);
   inst->hidden = inst->animating = EINA_TRUE;
}

static Eina_Bool
_cb_focus_out(void *data, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   Instance *inst;

   ev = event;

   EINA_INLIST_FOREACH (instances, inst)
     if (inst == data) break;

   if ((!inst) || (!inst->win))
     return ECORE_CALLBACK_PASS_ON;

   if (e_win_client_get(inst->win->ewin) != ev->ec)
     return ECORE_CALLBACK_PASS_ON;

   _evry_hide_func(inst->win, 0);

   return ECORE_CALLBACK_PASS_ON;
}

static void
_gadget_popup_show(Instance *inst)
{
   Evas_Coord x, y, w, h;
   int cx, cy, px, py, pw, ph;
   Evas_Object *ewin = inst->win->ewin;

   evas_object_geometry_get(ewin, &px, &py, &pw, &ph);

   evas_object_geometry_get(inst->o_button, &x, &y, &w, &h);
   e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy, NULL, NULL);
   x += cx;
   y += cy;

   switch (inst->gcc->gadcon->orient)
     {
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
        y += h;
        inst->hide_y = -1;
        break;

      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_BR:
      case E_GADCON_ORIENT_CORNER_BL:
        y -= ph;
        inst->hide_y = 1;
        break;

      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_LB:
        x += w;
        inst->hide_x = -1;
        break;

      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_RB:
        x -= pw;
        inst->hide_x = 1;
        break;

      case E_GADCON_ORIENT_FLOAT:
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_VERT:
      default:
        break;
     }

   if (x + pw > inst->win->zone->x + inst->win->zone->w)
     x = inst->win->zone->x + inst->win->zone->w - pw;

   if (y + ph > inst->win->zone->y + inst->win->zone->h)
     y = inst->win->zone->y + inst->win->zone->h - ph;

   evas_object_move(ewin, x, y);
}

static void
_gadget_window_show(Instance *inst)
{
   int zx, zy, zw, zh;
   int gx, gy, gw, gh;
   int cx, cy;
   int pw, ph;

   Evas_Object *ewin = inst->win->ewin;

   inst->win->func.hide = &_evry_hide_func;

   e_zone_useful_geometry_get(inst->win->zone, &zx, &zy, &zw, &zh);

   evas_object_geometry_get(inst->o_button, &gx, &gy, &gw, &gh);
   e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy, NULL, NULL);
   gx += cx;
   gy += cy;

   switch (inst->gcc->gadcon->orient)
     {
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
        pw = zw / 2;
        ph = zh / 2;
        inst->hide_y = -1;
        evas_object_move(ewin, zx, gy + gh);
        break;

      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_BR:
      case E_GADCON_ORIENT_CORNER_BL:
        pw = zw / 2;
        ph = zh / 2;
        inst->hide_y = 1;
        evas_object_move(ewin, zx, gy - ph);
        break;

      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_LB:
        pw = zw / 2.5;
        ph = zh;
        inst->hide_x = -1;
        evas_object_move(ewin, gx + gw, zy);
        break;

      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_RB:
        pw = zw / 2.5;
        ph = zh;
        inst->hide_x = 1;
        evas_object_move(ewin, gx - pw, zy);
        break;

      case E_GADCON_ORIENT_FLOAT:
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_VERT:
      default:
        pw = ph = 1;
        break;
     }

   evas_object_resize(ewin, pw, ph);
   evas_object_show(ewin);

   {
      E_Client *ec = e_win_client_get(ewin);

      evas_object_focus_set(ec->frame, 1);
      ec->netwm.state.skip_pager = 1;
      ec->netwm.state.skip_taskbar = 1;
      ec->sticky = 1;
   }

   inst->hidden = EINA_FALSE;
}

static void
_button_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev = event_info;

   inst = data;
   inst->mouse_down = EINA_TRUE;
   inst->mouse_down_x = ev->canvas.x;
   inst->mouse_down_y = ev->canvas.y;
}

static void
_button_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord dx, dy;

   inst = data;
   if (!inst->mouse_down) return;

   inst->mouse_down = EINA_FALSE;
   dx = ev->canvas.x - inst->mouse_down_x;
   dy = ev->canvas.y - inst->mouse_down_y;
   if (((dx * dx) + (dy * dy)) > (5 * 5)) return;

   ev = event_info;
   if (ev->button == 1)
     {
        Evry_Window *win;
        E_Client *ec;

        if (inst->win)
          {
             win = inst->win;
             ec = e_win_client_get(win->ewin);

             if (inst->hidden || !ec->focused)
               {
                  if (inst->animating)
                    e_comp_object_effect_stop(ec->frame, NULL);
                  e_comp_object_effect_set(ec->frame, "none");
                  e_client_uniconify(ec);
                  evas_object_raise(ec->frame);
                  evas_object_focus_set(ec->frame, 1);
                  inst->hidden = EINA_FALSE;
                  return;
               }
             else
               {
                  evry_hide(win, 1);
                  return;
               }
          }

        win = evry_show(e_gadcon_zone_get(inst->gcc->gadcon),
                        0, inst->cfg->plugin, !inst->illume_mode);
        if (!win) return;

        inst->win = win;
        win->data = inst;


        if (inst->illume_mode)
          _gadget_window_show(inst);
        else
          _gadget_popup_show(inst);

        e_gadcon_locked_set(inst->gcc->gadcon, 1);

        evas_object_event_callback_add(win->ewin, EVAS_CALLBACK_DEL, _del_func, inst);

        edje_object_signal_emit(inst->o_button, "e,state,focused", "e");
     }
   else if (ev->button == 3)
     {
        E_Menu *m;
        E_Menu_Item *mi;
        int cx, cy;

        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("Settings"));
        e_util_menu_item_theme_icon_set(mi, "configure");
        e_menu_item_callback_set(mi, _cb_menu_configure, inst);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy,
                                          NULL, NULL);
        e_menu_activate_mouse(m,
                              e_zone_current_get(),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

int
evry_gadget_init(void)
{
   e_gadcon_provider_register(&_gadcon_class);
   return 1;
}

void
evry_gadget_shutdown(void)
{
   e_gadcon_provider_unregister(&_gadcon_class);
}

/***************************************************************************/

struct _E_Config_Dialog_Data
{
   const char  *plugin;
   int          hide_after_action;
   int          popup;
   Evas_Object *list;
};

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

static void
_conf_dialog(Instance *inst)
{
   E_Config_Dialog_View *v = NULL;

   if (inst->cfd)
     return;

   /* if (e_config_dialog_find("everything-gadgets", "launcher/everything-gadgets"))
    *   return; */

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   inst->cfd = e_config_dialog_new(NULL, _("Everything Gadgets"), "everything-gadgets",
                                   "launcher/everything-gadgets", NULL, 0, v, inst);

   /* _conf->cfd = cfd; */
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata = NULL;
   Instance *inst = cfd->data;
   Gadget_Config *gc = inst->cfg;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);

#define CP(_name) cfdata->_name = (gc->_name ? strdup(gc->_name) : NULL);
#define C(_name)  cfdata->_name = gc->_name;
   /* CP(plugin); */
   C(hide_after_action);
   C(popup);
#undef CP
#undef C

   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Instance *inst = cfd->data;

   inst->cfd = NULL;
   /* if (cfdata->plugin) free(cfdata->plugin); */
   E_FREE(cfdata);
}

static void
_cb_button_settings(void *data EINA_UNUSED, void *data2 EINA_UNUSED)
{
   /* evry_collection_conf_dialog(NULL, "Start"); */
}

static void
_fill_list(Eina_List *plugins, Evas_Object *obj, E_Config_Dialog_Data *cfdata)
{
   Evas *evas;
   Evas_Coord w;
   Eina_List *l;
   Plugin_Config *pc;
   int sel = 0, cnt = 1;

   evas = evas_object_evas_get(obj);
   evas_event_freeze(evas);
   edje_freeze();
   e_widget_ilist_freeze(obj);
   e_widget_ilist_clear(obj);

   e_widget_ilist_append(obj, NULL, _("All"), NULL, NULL, NULL);

   EINA_LIST_FOREACH (plugins, l, pc)
     {
        if (!pc->plugin) continue;
        e_widget_ilist_append(obj, NULL, pc->plugin->base.label, NULL, NULL, pc->name);
        if (cfdata->plugin && !strcmp(pc->name, cfdata->plugin))
          sel = cnt;

        cnt++;
     }

   e_widget_ilist_selected_set(obj, sel);

   e_widget_ilist_go(obj);
   e_widget_size_min_get(obj, &w, NULL);
   e_widget_size_min_set(obj, w > 180 ? w : 180, 140);
   e_widget_ilist_thaw(obj);
   edje_thaw();
   evas_event_thaw(evas);
}

#if 0
static void
_list_select_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   int sel = e_widget_ilist_selected_get(obj);

   e_widget_ilist_nth_data_get(obj, sel);
}

#endif

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *e, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;
   Instance *inst = cfd->data;

   o = e_widget_list_add(e, 0, 0);

   of = e_widget_framelist_add(e, _("Plugin"), 0);
   /* ow = e_widget_entry_add(cfd->dia->win, &(cfdata->plugin), NULL, NULL, NULL);
    * e_widget_framelist_object_append(of, ow); */

   ow = e_widget_ilist_add(e, 24, 24, &cfdata->plugin);
   /* e_widget_on_change_hook_set(ow, _list_select_cb, cfdata); */
   _fill_list(evry_conf->conf_subjects, ow, cfdata);
   e_widget_framelist_object_append(of, ow);
   cfdata->list = ow;

   ow = e_widget_button_add(e, _("Settings"), NULL, _cb_button_settings, inst, NULL);
   e_widget_framelist_object_append(of, ow);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Instance *inst = cfd->data;
   Gadget_Config *gc = inst->cfg;
   Evry_Plugin *p;
   Evas_Object *oo;
   const char *plugin;

#define CP(_name)                     \
  if (gc->_name)                      \
    eina_stringshare_del(gc->_name);  \
  gc->_name = eina_stringshare_add(cfdata->_name);
#define C(_name) gc->_name = cfdata->_name;
   eina_stringshare_del(gc->plugin); \
   plugin = e_widget_ilist_selected_label_get(cfdata->list);
   if (plugin && plugin[0])
     gc->plugin = eina_stringshare_add(cfdata->plugin);
   else
     gc->plugin = NULL;
   C(hide_after_action);
   C(popup);
#undef CP
#undef C

   e_config_save_queue();

   oo = edje_object_part_swallow_get(inst->o_button, "e.swallow.icon");
   if (oo) evas_object_del(oo);

   edje_object_signal_emit(inst->o_button, "e,state,icon,default", "e");

   if ((p = evry_plugin_find(inst->cfg->plugin)))
     {
        oo = evry_util_icon_get(EVRY_ITEM(p), evas_object_evas_get(inst->o_button));
        if (oo)
          {
             edje_object_part_swallow(inst->o_button, "e.swallow.icon", oo);
             edje_object_signal_emit(inst->o_button, "e,state,icon,plugin", "e");
          }
     }

   return 1;
}

