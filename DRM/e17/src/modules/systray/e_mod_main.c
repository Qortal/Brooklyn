#include "e_mod_main.h"

static const char _Name[] = "Systray";
static const char _name[] = "systray";
static const char _group_gadget[] = "e/modules/systray/main";
static const char _sig_source[] = "e";

static E_Module *systray_mod = NULL;
static Systray_Context *ctx = NULL;
static char tmpbuf[4096]; /* general purpose buffer, just use immediately */

#define SYSTRAY_MIN_W 4
#define SYSTRAY_MIN_H 4

static Eina_Bool
_systray_site_is_safe(E_Gadcon_Site site)
{
   /* NB: filter out sites we know are not safe for a systray to sit.
    * This was done so that systray could be put into illume indicator
    * (or anywhere else really) that is 'safe' for systray to be.
    * Pretty much, this is anywhere but toolbars at the moment */
   return !e_gadcon_site_is_any_toolbar(site);
}

static const char *
_systray_theme_path(void)
{
#define TF "/e-module-systray.edj"
   unsigned int dirlen;
   const char *moddir = e_module_dir_get(systray_mod);

   dirlen = strlen(moddir);
   if (dirlen >= sizeof(tmpbuf) - sizeof(TF))
     return NULL;

   memcpy(tmpbuf, moddir, dirlen);
   memcpy(tmpbuf + dirlen, TF, sizeof(TF));

   return tmpbuf;
#undef TF
}

#if 0
static void *
_cfg_data_create(E_Config_Dialog *cfd EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = calloc(1, sizeof(E_Config_Dialog_Data));
   return cfdata;
}

static void
_cfg_data_free(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   free(cfdata);
}

static Evas_Object *
_cfg_widgets_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of;
   o = e_widget_list_add(evas, 0, 0);

   return o;
}

static int
_cfg_data_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Instance *inst = cfd->data;

   systray_size_updated(inst);

   return 1;
}

static void
_cb_menu_cfg(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Config_Dialog_View *v;

   v = calloc(1, sizeof(E_Config_Dialog_View));
   v->create_cfdata = _cfg_data_create;
   v->free_cfdata = _cfg_data_free;
   v->basic.create_widgets = _cfg_widgets_create;
   v->basic.apply_cfdata = _cfg_data_apply;

   e_config_dialog_new(NULL, _("Systray Settings"), "E",
                       "_e_mod_systray_config_dialog",
                       NULL, 0, v, data);
}
#endif
static Eina_Bool
_is_horiz(Instance *inst)
{
   switch (inst->gcc->gadcon->orient)
     {
      case E_GADCON_ORIENT_FLOAT:
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
        return EINA_TRUE;
        break;
      case E_GADCON_ORIENT_VERT:
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_RB:
      default:
        return EINA_FALSE;
        break;
     }
   return EINA_TRUE;
}

static void
_redo_sizing(Instance *inst)
{
   Eina_List *l;
   Evas_Object *o;
   Evas_Coord w, h;

   evas_object_geometry_get(inst->ui.gadget, NULL, NULL, &w, &h);
   EINA_LIST_FOREACH(inst->icons, l, o)
     {
        if (_is_horiz(inst)) evas_object_size_hint_min_set(o, h, 0);
        else evas_object_size_hint_min_set(o, 0, w);
     }
}

static void
_systray_menu_new(Instance *inst, Evas_Event_Mouse_Down *ev)
{
   E_Zone *zone;
   E_Menu *m;
   //E_Menu_Item *mi;
   int x, y;

   zone = e_zone_current_get();

   m = e_menu_new();
   //mi = e_menu_item_new(m);
   //e_menu_item_label_set(mi, _("Settings"));
   //e_util_menu_item_theme_icon_set(mi, "configure");
   //e_menu_item_callback_set(mi, _cb_menu_cfg, inst);
   m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
   e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
   e_menu_activate_mouse(m, zone, x + ev->output.x, y + ev->output.y,
                         1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
   evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                            EVAS_BUTTON_NONE, ev->timestamp, NULL);
}

static void
_systray_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Instance *inst = data;
   Evas_Event_Mouse_Down *ev = event;

   if (ev->button == 3)
     _systray_menu_new(inst, ev);
}

static void
_systray_cb_resize(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Instance *inst = data;
   _redo_sizing(inst);
   systray_size_updated(inst);
}

static void
_systray_theme(Evas_Object *o, const char *shelf_style, const char *gc_style)
{
   char buf[4096];

   if (shelf_style && gc_style)
     {
        snprintf(buf, sizeof(buf), "%s/%s/%s", _group_gadget, shelf_style, gc_style);
        if (e_theme_edje_object_set(o, NULL, buf))
          return;
     }

   if (shelf_style)
     {
        snprintf(buf, sizeof(buf), "%s/%s", _group_gadget, shelf_style);
        if (e_theme_edje_object_set(o, NULL, buf))
          return;
     }

   if (gc_style)
     {
        snprintf(buf, sizeof(buf), "%s/%s", _group_gadget, gc_style);
        if (e_theme_edje_object_set(o, NULL, buf))
          return;
     }

   e_theme_edje_object_set(o, NULL, _group_gadget);
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Instance *inst;

   // fprintf(stderr, "SYSTRAY: init name=%s, id=%s, style=%s\n", name, id, style);

   if (!systray_mod)
     return NULL;

   inst = E_NEW(Instance, 1);
   if (!inst)
     return NULL;
   inst->evas = gc->evas;
   if (!e_comp)
     {
        E_FREE(inst);
        return NULL;
     }

   inst->ui.gadget = edje_object_add(inst->evas);
   evas_object_event_callback_add(inst->ui.gadget, EVAS_CALLBACK_RESIZE,
                                  _systray_cb_resize, inst);

   _systray_theme(inst->ui.gadget, gc->shelf ? gc->shelf->style : NULL, style);

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->ui.gadget);
   if (!inst->gcc)
     {
        evas_object_del(inst->ui.gadget);
        E_FREE(inst);
        return NULL;
     }
   e_gadcon_client_min_size_set(inst->gcc, SYSTRAY_MIN_W, SYSTRAY_MIN_H);

   inst->gcc->data = inst;

   evas_object_event_callback_add(inst->ui.gadget, EVAS_CALLBACK_MOUSE_DOWN,
                                  _systray_cb_mouse_down, inst);

   inst->notifier = systray_notifier_host_new(inst, inst->gcc->gadcon);

   return inst->gcc;
}

/* Called when Gadget_Container says stop */
static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst = gcc->data;

   // fprintf(stderr, "SYSTRAY: shutdown %p, inst=%p\n", gcc, inst);

   if (!inst)
     return;

   systray_notifier_host_free(inst->notifier);

   evas_object_del(inst->ui.gadget);

   if (inst->job.size_apply)
     ecore_job_del(inst->job.size_apply);

   inst->icons = eina_list_free(inst->icons);
   E_FREE(inst);
   gcc->data = NULL;
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   Instance *inst = gcc->data;
   const char *sig;

   if (!inst)
     return;

   switch (orient)
     {
      case E_GADCON_ORIENT_FLOAT:
        sig = "e,action,orient,float";
        break;

      case E_GADCON_ORIENT_HORIZ:
        sig = "e,action,orient,horiz";
        break;

      case E_GADCON_ORIENT_VERT:
        sig = "e,action,orient,vert";
        break;

      case E_GADCON_ORIENT_LEFT:
        sig = "e,action,orient,left";
        break;

      case E_GADCON_ORIENT_RIGHT:
        sig = "e,action,orient,right";
        break;

      case E_GADCON_ORIENT_TOP:
        sig = "e,action,orient,top";
        break;

      case E_GADCON_ORIENT_BOTTOM:
        sig = "e,action,orient,bottom";
        break;

      case E_GADCON_ORIENT_CORNER_TL:
        sig = "e,action,orient,corner_tl";
        break;

      case E_GADCON_ORIENT_CORNER_TR:
        sig = "e,action,orient,corner_tr";
        break;

      case E_GADCON_ORIENT_CORNER_BL:
        sig = "e,action,orient,corner_bl";
        break;

      case E_GADCON_ORIENT_CORNER_BR:
        sig = "e,action,orient,corner_br";
        break;

      case E_GADCON_ORIENT_CORNER_LT:
        sig = "e,action,orient,corner_lt";
        break;

      case E_GADCON_ORIENT_CORNER_RT:
        sig = "e,action,orient,corner_rt";
        break;

      case E_GADCON_ORIENT_CORNER_LB:
        sig = "e,action,orient,corner_lb";
        break;

      case E_GADCON_ORIENT_CORNER_RB:
        sig = "e,action,orient,corner_rb";
        break;

      default:
        sig = "e,action,orient,horiz";
     }

   edje_object_signal_emit(inst->ui.gadget, sig, _sig_source);
   edje_object_message_signal_process(inst->ui.gadget);
   _redo_sizing(inst);
   systray_size_updated(inst);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Systray");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;

   o = edje_object_add(evas);
   edje_object_file_set(o, _systray_theme_path(), "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _name;
}

static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, _name,
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      _systray_site_is_safe
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

E_API E_Module_Api e_modapi = {E_MODULE_API_VERSION, _Name};

E_API void *
e_modapi_init(E_Module *m)
{
   systray_mod = m;

   ctx = calloc(1, sizeof(Systray_Context));
   ctx->conf_edd = E_CONFIG_DD_NEW("Systray_Config", Systray_Config);
   ctx->notifier_item_edd = E_CONFIG_DD_NEW("Notifier_Item_Cache", Notifier_Item_Cache);
   #undef T
   #undef D
   #define T Notifier_Item_Cache
   #define D ctx->notifier_item_edd
   E_CONFIG_VAL(D, T, path, STR);
   #undef T
   #undef D
   #define T Systray_Config
   #define D ctx->conf_edd
   E_CONFIG_VAL(D, T, dbus, STR);
   E_CONFIG_HASH(D, T, items, ctx->notifier_item_edd);

   ctx->config = e_config_domain_load(_name, ctx->conf_edd);
   if (!ctx->config)
     ctx->config = calloc(1, sizeof(Systray_Config));

   e_gadcon_provider_register(&_gc_class);

   systray_notifier_host_init();

   return ctx;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_gadcon_provider_unregister(&_gc_class);
   systray_mod = NULL;

   systray_notifier_host_shutdown();

   E_CONFIG_DD_FREE(ctx->conf_edd);
   E_CONFIG_DD_FREE(ctx->notifier_item_edd);
   free(ctx->config);
   free(ctx);
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save(_name, ctx->conf_edd, ctx->config);
   return 1;
}

const E_Gadcon *
systray_gadcon_get(const Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(inst, NULL);
   return inst->gcc->gadcon;
}

Evas_Object *
systray_edje_get(const Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(inst, NULL);
   return inst->ui.gadget;
}

void
systray_edje_box_append(Instance *inst, Evas_Object *child)
{
   Evas_Coord w, h;

   inst->icons = eina_list_append(inst->icons, child);
   evas_object_size_hint_aspect_set(child, EVAS_ASPECT_CONTROL_BOTH, 1.0, 1.0);
   evas_object_geometry_get(inst->ui.gadget, NULL, NULL, &w, &h);
   if (_is_horiz(inst)) evas_object_size_hint_min_set(child, h, 0);
   else evas_object_size_hint_min_set(child, 0, w);
   edje_object_part_box_append(inst->ui.gadget, "box", child);
}

void
systray_edje_box_prepend(Instance *inst, Evas_Object *child)
{
   Evas_Coord w, h;

   inst->icons = eina_list_prepend(inst->icons, child);
   evas_object_size_hint_aspect_set(child, EVAS_ASPECT_CONTROL_BOTH, 1.0, 1.0);
   evas_object_geometry_get(inst->ui.gadget, NULL, NULL, &w, &h);
   if (_is_horiz(inst)) evas_object_size_hint_min_set(child, h, 0);
   else evas_object_size_hint_min_set(child, 0, w);
   edje_object_part_box_prepend(inst->ui.gadget, "box", child);
}

void
systray_edje_box_remove(Instance *inst, Evas_Object *child)
{
   inst->icons = eina_list_remove(inst->icons, child);
   edje_object_part_box_remove(inst->ui.gadget, "box", child);
}

static void
_systray_size_apply_do(Instance *inst)
{
   Evas_Coord w, h;

   edje_object_message_signal_process(inst->ui.gadget);
   edje_object_size_min_calc(inst->ui.gadget, &w, &h);
   e_gadcon_client_min_size_set(inst->gcc, MAX(w, SYSTRAY_MIN_W), MAX(h, SYSTRAY_MIN_H));
}

static void
_systray_size_apply_delayed(void *data)
{
   Instance *inst = data;
   _systray_size_apply_do(inst);
   inst->job.size_apply = NULL;
}

void
systray_size_updated(Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN(inst);
   if (inst->job.size_apply) return;
   inst->job.size_apply = ecore_job_add(_systray_size_apply_delayed, inst);
}

EINTERN Systray_Context *
systray_ctx_get(void)
{
   return ctx;
}
