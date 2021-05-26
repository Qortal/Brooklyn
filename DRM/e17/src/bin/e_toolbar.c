#include "e.h"

/* local function protos */
static void        _e_toolbar_free(E_Toolbar *tbar);
static void        _e_toolbar_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void        _e_toolbar_menu_cb_post(void *data, E_Menu *mn);
static void        _e_toolbar_menu_cb_pre(void *data, E_Menu *mn);
static void        _e_toolbar_menu_append(E_Toolbar *tbar, E_Menu *mn);
//static void        _e_toolbar_menu_cb_edit(void *data, E_Menu *mn, E_Menu_Item *mi);
static void        _e_toolbar_menu_cb_config(void *data, E_Menu *mn, E_Menu_Item *mi);
//static void        _e_toolbar_menu_cb_contents(void *data, E_Menu *mn, E_Menu_Item *mi);
static void        _e_toolbar_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h);
static const char *_e_toolbar_orient_string_get(E_Toolbar *tbar);
static void        _e_toolbar_fm2_changed(void *data, Evas_Object *obj, void *event_info);
static void        _e_toolbar_fm2_dir_changed(void *data, Evas_Object *obj, void *event_info);
static void        _e_toolbar_fm2_dir_deleted(void *data, Evas_Object *obj, void *event_info);
static void        _e_toolbar_fm2_files_deleted(void *data, Evas_Object *obj, void *event_info);
static void        _e_toolbar_fm2_selected(void *data, Evas_Object *obj, void *event_info);
static void        _e_toolbar_fm2_selection_changed(void *data, Evas_Object *obj, void *event_info);
static void        _e_toolbar_menu_items_append(void *data, E_Gadcon_Client *gcc, E_Menu *mn);

/* local vars */
static Eina_List *toolbars = NULL;
static E_Gadcon_Location *tb_location = NULL;

static void
_tb_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   E_Toolbar *tbar = data;
   Evas_Coord w, h;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (tbar->gadcon) e_gadcon_swallowed_min_size_set(tbar->gadcon, w, h);
}
/*
static void
_e_toolbar_gadget_remove(void *data EINA_UNUSED, E_Gadcon_Client *gcc)
{
   E_Toolbar *tbar = eina_list_data_get(toolbars);
   Eina_List *l;

   if (!tbar)
     {
        E_Config_Gadcon *cf_gc;
        EINA_LIST_FOREACH(e_config->gadcons, l, cf_gc)
          {
             if (e_util_strcmp(cf_gc->name, "toolbar")) continue;
             cf_gc->clients = eina_list_remove(cf_gc->clients, gcc->cf);
             break;
          }
        if (!cf_gc) return;
     }
   else if (gcc->cf)
     e_gadcon_client_config_del(tbar->gadcon->cf, gcc->cf);
   EINA_LIST_FOREACH(toolbars, l, tbar)
     e_gadcon_repopulate(tbar->gadcon);
   e_config_save_queue();
}

static int
_e_toolbar_gadget_add(void *data EINA_UNUSED, E_Gadcon_Client *gcc, const E_Gadcon_Client_Class *cc)
{
   E_Toolbar *tbar = eina_list_data_get(toolbars);
   E_Config_Gadcon *cf_gc = NULL;
   E_Gadcon *gc = NULL;
   Eina_List *l;
   E_Config_Gadcon_Client *cf_gcc = gcc->cf;

   if (!tbar)
     {
        E_Config_Gadcon *cf_gc2;
        EINA_LIST_FOREACH(e_config->gadcons, l, cf_gc2)
          {
             if (e_util_strcmp(cf_gc2->name, "toolbar")) continue;
             cf_gc = cf_gc2;
             break;
          }
        if (!cf_gc) return 0;
     }
   else
     gc = tbar->gadcon, cf_gc = gc->cf;
   if (gcc)
     {
        gcc->gadcon->cf->clients = eina_list_remove(gcc->gadcon->cf->clients, cf_gcc);
        if (gc && gc->zone)
          cf_gcc->geom.res = gc->zone->w;
        else if (gc && gc->o_container)
          {
             int w, h;
             evas_object_geometry_get(gc->o_container, NULL, NULL, &w, &h);
             switch (gc->orient)
               {
                case E_GADCON_ORIENT_VERT:
                case E_GADCON_ORIENT_LEFT:
                case E_GADCON_ORIENT_RIGHT:
                case E_GADCON_ORIENT_CORNER_LT:
                case E_GADCON_ORIENT_CORNER_RT:
                case E_GADCON_ORIENT_CORNER_LB:
                case E_GADCON_ORIENT_CORNER_RB:
                  cf_gcc->geom.res = h;
                  break;

                default:
                  cf_gcc->geom.res = w;
               }
          }
        else
          cf_gcc->geom.res = 800;
        cf_gcc->geom.size = 80;
        cf_gcc->geom.pos = cf_gcc->geom.res - cf_gcc->geom.size;
        cf_gc->clients = eina_list_append(cf_gc->clients, cf_gcc);
     }
   else
     {
        if (!gc); // FIXME: okay...need to create a gcc from nothing and add it to nothing...
        else if (!e_gadcon_client_config_new(gc, cc->name))
          return 0;
     }
   EINA_LIST_FOREACH(toolbars, l, tbar)
     e_gadcon_repopulate(tbar->gadcon);
   if (gc || gcc)
     e_config_save_queue();
   return (gc || gcc);
}
*/
EINTERN int
e_toolbar_init(void)
{
   tb_location = e_gadcon_location_new(_("EFM Toolbar"), E_GADCON_SITE_EFM_TOOLBAR,
                                       NULL, NULL, NULL, NULL);
                                    //_e_toolbar_gadget_add, NULL,
                                    //_e_toolbar_gadget_remove, NULL);
   e_gadcon_location_set_icon_name(tb_location, "configure-toolbars");
   e_gadcon_location_register(tb_location);
   return 1;
}

EINTERN int
e_toolbar_shutdown(void)
{
   while (toolbars)
     {
        E_Toolbar *tbar;

        tbar = eina_list_data_get(toolbars);
        e_object_del(E_OBJECT(tbar));
     }
   e_gadcon_location_unregister(tb_location);
   E_FREE_FUNC(tb_location, e_gadcon_location_free);
   return 1;
}

E_API E_Toolbar *
e_toolbar_new(Evas *evas, const char *name, Evas_Object *fwin, Evas_Object *fm2)
{
   E_Toolbar *tbar = NULL;

   if (!name) return NULL;
   if ((!fwin) || (!fm2)) return NULL;

   tbar = E_OBJECT_ALLOC(E_Toolbar, E_TOOLBAR_TYPE, _e_toolbar_free);
   if (!tbar) return NULL;
   tbar->id = 1;
//   tbar->id = eina_list_count(toolbars) + 1;
   tbar->evas = evas;
   tbar->name = eina_stringshare_add(name);
   tbar->fwin = fwin;
   tbar->fm2 = fm2;

   evas_object_smart_callback_add(tbar->fm2, "changed",
                                  _e_toolbar_fm2_changed, tbar);
   evas_object_smart_callback_add(tbar->fm2, "dir_changed",
                                  _e_toolbar_fm2_dir_changed, tbar);
   evas_object_smart_callback_add(tbar->fm2, "dir_deleted",
                                  _e_toolbar_fm2_dir_deleted, tbar);
   evas_object_smart_callback_add(tbar->fm2, "files_deleted",
                                  _e_toolbar_fm2_files_deleted, tbar);
   evas_object_smart_callback_add(tbar->fm2, "selected",
                                  _e_toolbar_fm2_selected, tbar);
   evas_object_smart_callback_add(tbar->fm2, "selection_change",
                                  _e_toolbar_fm2_selection_changed, tbar);

   tbar->o_base = edje_object_add(evas);
   e_theme_edje_object_set(tbar->o_base, "base/theme/fileman/toolbar",
                           "e/fileman/toolbar/default/base");
   evas_object_event_callback_add(tbar->o_base, EVAS_CALLBACK_RESIZE,
                                  _tb_resize, tbar);
   tbar->o_event = evas_object_rectangle_add(evas);
   evas_object_color_set(tbar->o_event, 0, 0, 0, 0);
   evas_object_show(tbar->o_event);
   edje_object_part_swallow(tbar->o_base, "e.swallow.event", tbar->o_event);
   evas_object_event_callback_add(tbar->o_event, EVAS_CALLBACK_MOUSE_DOWN,
                                  _e_toolbar_cb_mouse_down, tbar);

   tbar->gadcon = e_gadcon_swallowed_new(tbar->name, tbar->id, tbar->o_base,
                                         "e.swallow.content");
   e_gadcon_size_request_callback_set(tbar->gadcon,
                                      _e_toolbar_gadcon_size_request, tbar);
   /* FIXME: We want to implement "styles" here ? */

   e_toolbar_orient(tbar, E_GADCON_ORIENT_TOP);

   e_gadcon_toolbar_set(tbar->gadcon, tbar);
   tbar->gadcon->location = tb_location;
   e_gadcon_ecore_evas_set(tbar->gadcon, ecore_evas_ecore_evas_get(evas_object_evas_get(tbar->fwin)));
   e_gadcon_util_menu_attach_func_set(tbar->gadcon,
                                      _e_toolbar_menu_items_append, tbar);
   e_gadcon_populate(tbar->gadcon);

   _e_toolbar_gadcon_size_request(tbar, tbar->gadcon, 0, 0);
   toolbars = eina_list_append(toolbars, tbar);
   return tbar;
}

E_API void
e_toolbar_fwin_set(E_Toolbar *tbar, Evas_Object *fwin)
{
   E_OBJECT_CHECK(tbar);
   E_OBJECT_TYPE_CHECK(tbar, E_TOOLBAR_TYPE);
   tbar->fwin = fwin;
}

E_API Evas_Object *
e_toolbar_fwin_get(E_Toolbar *tbar)
{
   E_OBJECT_CHECK_RETURN(tbar, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(tbar, E_TOOLBAR_TYPE, NULL);
   return tbar->fwin;
}

E_API void
e_toolbar_fm2_set(E_Toolbar *tbar, Evas_Object *fm2)
{
   E_OBJECT_CHECK(tbar);
   E_OBJECT_TYPE_CHECK(tbar, E_TOOLBAR_TYPE);
   tbar->fm2 = fm2;
}

E_API Evas_Object *
e_toolbar_fm2_get(E_Toolbar *tbar)
{
   E_OBJECT_CHECK_RETURN(tbar, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(tbar, E_TOOLBAR_TYPE, NULL);
   return tbar->fm2;
}

E_API void
e_toolbar_orient(E_Toolbar *tbar, E_Gadcon_Orient orient)
{
   char buf[4096];

   E_OBJECT_CHECK(tbar);
   E_OBJECT_TYPE_CHECK(tbar, E_TOOLBAR_TYPE);
   e_gadcon_orient(tbar->gadcon, orient);
   snprintf(buf, sizeof(buf), "e,state,orientation,%s",
            _e_toolbar_orient_string_get(tbar));
   edje_object_signal_emit(tbar->o_base, buf, "e");
   edje_object_message_signal_process(tbar->o_base);
}

E_API void
e_toolbar_populate(E_Toolbar *tbar)
{
   E_OBJECT_CHECK(tbar);
   E_OBJECT_TYPE_CHECK(tbar, E_TOOLBAR_TYPE);
   e_gadcon_populate(tbar->gadcon);
}

/* local functions */
static void
_e_toolbar_free(E_Toolbar *tbar)
{
   toolbars = eina_list_remove(toolbars, tbar);

   evas_object_smart_callback_del_full(tbar->fm2, "changed",
                                       _e_toolbar_fm2_changed, tbar);
   evas_object_smart_callback_del_full(tbar->fm2, "dir_changed",
                                       _e_toolbar_fm2_dir_changed, tbar);
   evas_object_smart_callback_del_full(tbar->fm2, "dir_deleted",
                                       _e_toolbar_fm2_dir_deleted, tbar);
   evas_object_smart_callback_del_full(tbar->fm2, "files_deleted",
                                       _e_toolbar_fm2_files_deleted, tbar);
   evas_object_smart_callback_del_full(tbar->fm2, "selected",
                                       _e_toolbar_fm2_selected, tbar);
   evas_object_smart_callback_del_full(tbar->fm2, "selection_change",
                                       _e_toolbar_fm2_selection_changed, tbar);
   if (tbar->menu)
     {
        e_menu_post_deactivate_callback_set(tbar->menu, NULL, NULL);
        e_object_del(E_OBJECT(tbar->menu));
        tbar->menu = NULL;
     }
   if (tbar->cfg_dlg) e_object_del(E_OBJECT(tbar->cfg_dlg));
   e_object_del(E_OBJECT(tbar->gadcon));
   if (tbar->name) eina_stringshare_del(tbar->name);
   evas_object_del(tbar->o_event);
   evas_object_del(tbar->o_base);
   E_FREE(tbar);
}

static void
_e_toolbar_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   E_Toolbar *tbar;
   E_Menu *mn;
   E_Zone *zone;
   int x, y;

   ev = event_info;
   tbar = data;
   if (ev->button != 3) return;
   mn = e_menu_new();
   e_menu_post_deactivate_callback_set(mn, _e_toolbar_menu_cb_post, tbar);
   tbar->menu = mn;
   _e_toolbar_menu_append(tbar, mn);
   zone = e_zone_current_get();
   ecore_evas_pointer_xy_get(e_comp->ee, &x, &y);
   e_menu_activate_mouse(mn, zone, x, y, 1, 1,
                         E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
}

static void
_e_toolbar_menu_cb_post(void *data, E_Menu *mn EINA_UNUSED)
{
   E_Toolbar *tbar;

   tbar = data;
   if (!tbar->menu) return;
   e_object_del(E_OBJECT(tbar->menu));
   tbar->menu = NULL;
}

static void
_e_toolbar_menu_cb_pre(void *data, E_Menu *mn)
{
   E_Toolbar *tbar;
   E_Menu_Item *mi;

   tbar = data;
   e_menu_pre_activate_callback_set(mn, NULL, NULL);
/*
   mi = e_menu_item_new(mn);
   if (tbar->gadcon->editing)
     e_menu_item_label_set(mi, _("Stop Moving Items"));
   else
     e_menu_item_label_set(mi, _("Begin Moving Items"));
   e_util_menu_item_theme_icon_set(mi, "transform-scale");
   e_menu_item_callback_set(mi, _e_toolbar_menu_cb_edit, tbar);

   mi = e_menu_item_new(mn);
   e_menu_item_separator_set(mi, 1);
*/
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Toolbar Settings"));
   e_util_menu_item_theme_icon_set(mi, "configure");
   e_menu_item_callback_set(mi, _e_toolbar_menu_cb_config, tbar);
/*
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Set Toolbar Contents"));
   e_util_menu_item_theme_icon_set(mi, "preferences-toolbar");
   e_menu_item_callback_set(mi, _e_toolbar_menu_cb_contents, tbar);
*/
}

static void
_e_toolbar_menu_items_append(void *data, E_Gadcon_Client *gcc EINA_UNUSED, E_Menu *mn)
{
   E_Toolbar *tbar;

   tbar = data;
   _e_toolbar_menu_append(tbar, mn);
}

static void
_e_toolbar_menu_append(E_Toolbar *tbar, E_Menu *mn)
{
   E_Menu_Item *mi;
   E_Menu *subm;

   subm = e_menu_new();
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, tbar->name);
   e_util_menu_item_theme_icon_set(mi, "preferences-toolbar");
   e_menu_pre_activate_callback_set(subm, _e_toolbar_menu_cb_pre, tbar);
   e_menu_item_submenu_set(mi, subm);
   e_object_unref(E_OBJECT(subm));
}
/*
static void
_e_toolbar_menu_cb_edit(void *data, E_Menu *mn EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Toolbar *tbar;

   tbar = data;
   if (tbar->gadcon->editing)
     e_gadcon_edit_end(tbar->gadcon);
   else
     e_gadcon_edit_begin(tbar->gadcon);
}
*/
static void
_e_toolbar_menu_cb_config(void *data, E_Menu *mn EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Toolbar *tbar;

   tbar = data;
   if (!tbar->cfg_dlg) e_int_toolbar_config(tbar);
}
/*
static void
_e_toolbar_menu_cb_contents(void *data, E_Menu *mn EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Toolbar *tbar;

   tbar = data;
   if (!tbar->gadcon->config_dialog) e_int_gadcon_config_toolbar(tbar->gadcon);
}
*/
static void
_e_toolbar_gadcon_size_request(void *data, E_Gadcon *gc, Evas_Coord w, Evas_Coord h)
{
   E_Toolbar *tbar;
   Evas_Coord ww, hh, nw, nh;

   tbar = data;
   ww = hh = 0;
   evas_object_geometry_get(gc->o_container, NULL, NULL, &ww, &hh);
   switch (gc->orient)
     {
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_BOTTOM:
        w = ww;
        h = 32;
        break;

      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_RIGHT:
        w = 32;
        h = hh;
        break;

      default:
        break;
     }
   e_gadcon_swallowed_min_size_set(gc, w, h);
   edje_object_size_min_calc(tbar->o_base, &nw, &nh);
   tbar->minw = nw;
   tbar->minh = nh;
}

static const char *
_e_toolbar_orient_string_get(E_Toolbar *tbar)
{
   const char *sig = "";

   switch (tbar->gadcon->orient)
     {
      case E_GADCON_ORIENT_HORIZ:
        sig = "horizontal";
        break;

      case E_GADCON_ORIENT_VERT:
        sig = "vertical";
        break;

      case E_GADCON_ORIENT_LEFT:
        sig = "left";
        break;

      case E_GADCON_ORIENT_RIGHT:
        sig = "right";
        break;

      case E_GADCON_ORIENT_TOP:
        sig = "top";
        break;

      case E_GADCON_ORIENT_BOTTOM:
        sig = "bottom";
        break;

      default:
        break;
     }
   return sig;
}

static void
_e_toolbar_fm2_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Toolbar *tbar;
   Eina_List *l = NULL;
   E_Gadcon_Client *gcc = NULL;

   tbar = data;
   if (!tbar) return;
   EINA_LIST_FOREACH(tbar->gadcon->clients, l, gcc)
     {
        if (!gcc) continue;
        evas_object_smart_callback_call(gcc->o_base, "changed", tbar);
     }
}

static void
_e_toolbar_fm2_dir_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Toolbar *tbar;
   Eina_List *l = NULL;
   E_Gadcon_Client *gcc = NULL;

   tbar = data;
   if (!tbar) return;
   EINA_LIST_FOREACH(tbar->gadcon->clients, l, gcc)
     {
        if (!gcc) continue;
        evas_object_smart_callback_call(gcc->o_base, "dir_changed", tbar);
     }
}

static void
_e_toolbar_fm2_dir_deleted(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Toolbar *tbar;
   Eina_List *l = NULL;
   E_Gadcon_Client *gcc = NULL;

   tbar = data;
   if (!tbar) return;
   EINA_LIST_FOREACH(tbar->gadcon->clients, l, gcc)
     {
        if (!gcc) continue;
        evas_object_smart_callback_call(gcc->o_base, "dir_deleted", tbar);
     }
}

static void
_e_toolbar_fm2_files_deleted(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Toolbar *tbar;
   Eina_List *l = NULL;
   E_Gadcon_Client *gcc = NULL;

   tbar = data;
   if (!tbar) return;
   EINA_LIST_FOREACH(tbar->gadcon->clients, l, gcc)
     {
        if (!gcc) continue;
        evas_object_smart_callback_call(gcc->o_base, "files_deleted", tbar);
     }
}

static void
_e_toolbar_fm2_selected(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Toolbar *tbar;
   Eina_List *l = NULL;
   E_Gadcon_Client *gcc = NULL;

   tbar = data;
   if (!tbar) return;
   EINA_LIST_FOREACH(tbar->gadcon->clients, l, gcc)
     {
        if (!gcc) continue;
        evas_object_smart_callback_call(gcc->o_base, "selected", tbar);
     }
}

static void
_e_toolbar_fm2_selection_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Toolbar *tbar;
   Eina_List *l = NULL;
   E_Gadcon_Client *gcc = NULL;

   tbar = data;
   if (!tbar) return;
   EINA_LIST_FOREACH(tbar->gadcon->clients, l, gcc)
     {
        if (!gcc) continue;
        evas_object_smart_callback_call(gcc->o_base, "selection_changed", tbar);
     }
}

