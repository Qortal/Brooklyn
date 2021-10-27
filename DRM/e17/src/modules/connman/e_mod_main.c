#include "e.h"
#include "e_mod_main.h"

#include "E_Connman.h"

E_Module *connman_mod = NULL;
static char tmpbuf[4096]; /* general purpose buffer, just use immediately */

const char _e_connman_name[] = "connman";
const char _e_connman_Name[] = N_("Connection Manager");
int _e_connman_log_dom = -1;

const char *
e_connman_theme_path(void)
{
#define TF "/e-module-connman.edj"
   size_t dirlen;

   dirlen = strlen(connman_mod->dir);
   if (dirlen >= sizeof(tmpbuf) - sizeof(TF))
     return NULL;

   memcpy(tmpbuf, connman_mod->dir, dirlen);
   memcpy(tmpbuf + dirlen, TF, sizeof(TF));

   return tmpbuf;
#undef TF
}

static Evas_Object *
_econnman_service_new_icon(struct Connman_Service *cs, Evas *evas)
{
   const char *type = econnman_service_type_to_str(cs->type);
   Edje_Message_Int_Set *msg;
   Evas_Object *icon;
   char buf[128];

   snprintf(buf, sizeof(buf), "e/modules/connman/icon/%s", type);
   icon = edje_object_add(evas);
   e_theme_edje_object_set(icon, "base/theme/modules/connman", buf);

   msg = malloc(sizeof(*msg) + sizeof(int));
   msg->count = 2;
   msg->val[0] = cs->state;
   msg->val[1] = cs->strength;

   edje_object_message_send(icon, EDJE_MESSAGE_INT_SET, 1, msg);
   free(msg);

   return icon;
}

static Evas_Object *
_econnman_service_new_end(struct Connman_Service *cs, Evas *evas)
{
   Eina_Iterator *iter;
   Evas_Object *end;
   void *security;
   char buf[128];

   end = edje_object_add(evas);
   e_theme_edje_object_set(end, "base/theme/modules/connman",
                           "e/modules/connman/end");

   if (!cs->security)
     return end;

   iter = eina_array_iterator_new(cs->security);
   while (eina_iterator_next(iter, &security))
     {
        snprintf(buf, sizeof(buf), "e,security,%s", (const char *)security);
        edje_object_signal_emit(end, buf, "e");
     }
   eina_iterator_free(iter);

   return end;
}

static void
_econnman_disconnect_cb(void *data, const char *error)
{
   const char *path = data;

   if (error == NULL)
     return;

   ERR("Could not disconnect %s: %s", path, error);
}

static void
_econnman_connect_cb(void *data, const char *error)
{
   const char *path = data;

   if (error == NULL)
     return;

   ERR("Could not connect %s: %s", path, error);
}

static void
_econnman_popup_selected_cb(void *data)
{
   E_Connman_Instance *inst = data;
   const char *path;
   struct Connman_Service *cs;

   path = e_widget_ilist_selected_value_get(inst->ui.popup.list);
   if (path == NULL)
     return;

   cs = econnman_manager_find_service(inst->ctxt->cm, path);
   if (!cs)
     return;

   switch (cs->state)
     {
      case CONNMAN_STATE_READY:
      case CONNMAN_STATE_ONLINE:
         INF("Disconnect %s", path);
         econnman_service_disconnect(cs, _econnman_disconnect_cb,
                                     (void *) path);
         break;
      default:
         INF("Connect %s", path);
         econnman_service_connect(cs, _econnman_connect_cb, (void *) path);
         break;
     }
}

static void
_econnman_popup_update(struct Connman_Manager *cm, E_Connman_Instance *inst)
{
   Evas_Object *list = inst->ui.popup.list;
   Evas_Object *powered = inst->ui.popup.powered;
   Evas *evas = evas_object_evas_get(list);
   struct Connman_Service *cs;
   const char *hidden = "«hidden»";

   EINA_SAFETY_ON_NULL_RETURN(cm);

   e_widget_ilist_freeze(list);
   e_widget_ilist_clear(list);

   EINA_INLIST_FOREACH(cm->services, cs)
     {
        Evas_Object *icon = _econnman_service_new_icon(cs, evas);
        Evas_Object *end = _econnman_service_new_end(cs, evas);
        e_widget_ilist_append_full(list, icon, end, cs->name ?: hidden,
                                   _econnman_popup_selected_cb,
                                   inst, cs->path);
     }

   e_widget_ilist_thaw(list);
   e_widget_ilist_go(list);

   if (inst->ctxt)
     {
        inst->ctxt->powered = cm->powered;
     }
   e_widget_check_checked_set(powered, cm->powered);
}

void
econnman_mod_services_changed(struct Connman_Manager *cm)
{
   E_Connman_Module_Context *ctxt = connman_mod->data;
   const Eina_List *l;
   E_Connman_Instance *inst;

   EINA_LIST_FOREACH(ctxt->instances, l, inst)
     {
        if (!inst->popup)
          continue;

        _econnman_popup_update(cm, inst);
     }
}

static void
_econnman_app_launch(E_Connman_Instance *inst)
{
   Efreet_Desktop *desktop;
   E_Zone *zone;

   desktop = efreet_util_desktop_file_id_find("econnman.desktop");
   if (!desktop)
     {
        e_util_dialog_internal
          (_("Missing Application"),
          _("This module wants to execute an external application "
            "EConnMan that does not exist.<ps/>"
            "Please install <b>EConnMan</b> application."));
        return;
     }

   zone = e_gadcon_client_zone_get(inst->gcc);
   if (!zone)
     zone = e_zone_current_get();

   e_exec(zone, desktop, NULL, NULL, "econnman/app");
   efreet_desktop_free(desktop);
}

static void
_econnman_configure_cb(void *data, void *data2 EINA_UNUSED)
{
   E_Connman_Instance *inst = data;
   econnman_popup_del(inst);
   _econnman_app_launch(inst);
}

static void
_econnman_powered_changed(void *data, Evas_Object *obj EINA_UNUSED, void *info EINA_UNUSED)
{
   E_Connman_Instance *inst = data;
   E_Connman_Module_Context *ctxt = inst->ctxt;

   if (!ctxt) return;
   if (!ctxt->cm) return;
   econnman_powered_set(ctxt->cm, ctxt->powered);
}

static void
_e_connman_widget_size_set(E_Connman_Instance *inst, Evas_Object *widget, Evas_Coord percent_w, Evas_Coord percent_h, Evas_Coord min_w, Evas_Coord min_h, Evas_Coord max_w, Evas_Coord max_h)
{
   Evas_Coord w, h, zw, zh;
   E_Zone *zone;

   zone = e_gadcon_client_zone_get(inst->gcc);
   e_zone_useful_geometry_get(zone, NULL, NULL, &zw, &zh);

   w = zw * percent_w / 100.0;
   h = zh * percent_h / 100.0;

   if (w < min_w)
     w = min_w;
   else if (w > max_w)
     w = max_w;
   if (h < min_h)
     h = min_h;
   else if (h > max_h)
     h = max_h;

   e_widget_size_min_set(widget, w, h);
}

static void
_econnman_popup_del_cb(void *obj)
{
   econnman_popup_del(e_object_data_get(obj));
}

static void
_econnman_popup_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Connman_Instance *inst = data;

   E_FREE_FUNC(inst->popup, e_object_del);
}

static void
_econnman_popup_new(E_Connman_Instance *inst)
{
   E_Connman_Module_Context *ctxt = inst->ctxt;
   Evas_Object *list, *bt, *ck;
   Evas *evas;

   EINA_SAFETY_ON_FALSE_RETURN(inst->popup == NULL);

   if (!ctxt->cm)
     return;

   e_connman_scan(ctxt->cm);

   inst->popup = e_gadcon_popup_new(inst->gcc, 0);
   evas = e_comp->evas;

   list = e_widget_list_add(evas, 0, 0);
   inst->ui.popup.list = e_widget_ilist_add(evas, 24, 24, NULL);
   e_widget_size_min_set(inst->ui.popup.list, 60, 100);
   e_widget_list_object_append(list, inst->ui.popup.list, 1, 1, 0.5);

   ck = e_widget_check_add(evas, _("Wifi On"), &(ctxt->powered));
   inst->ui.popup.powered = ck;
   e_widget_list_object_append(list, ck, 1, 0, 0.5);
   evas_object_smart_callback_add(ck, "changed",
                                  _econnman_powered_changed, inst);

   _econnman_popup_update(ctxt->cm, inst);

   if (efreet_util_desktop_file_id_find("econnman.desktop"))
     {
        bt = e_widget_button_add(evas, _("Configure"), NULL,
                                 _econnman_configure_cb, inst, NULL);
        e_widget_list_object_append(list, bt, 1, 0, 0.5);
     }

   /* 30,40 % -- min vga, max uvga */
   _e_connman_widget_size_set(inst, list, 10, 30, 192, 192, 384, 384);
   e_gadcon_popup_content_set(inst->popup, list);
   e_comp_object_util_autoclose(inst->popup->comp_object, _econnman_popup_del, NULL, inst);
   e_gadcon_popup_show(inst->popup);
   e_object_data_set(E_OBJECT(inst->popup), inst);
   E_OBJECT_DEL_SET(inst->popup, _econnman_popup_del_cb);
}

void
econnman_popup_del(E_Connman_Instance *inst)
{
   E_FREE_FUNC(inst->popup, e_object_del);
   inst->ui.popup.powered = inst->ui.popup.list = NULL;
}

static void
_econnman_mod_manager_update_inst(E_Connman_Module_Context *ctxt EINA_UNUSED,
                                  E_Connman_Instance *inst,
                                  enum Connman_State state,
                                  enum Connman_Service_Type type)
{
   Evas_Object *o = inst->ui.gadget;
   Edje_Message_Int_Set *msg;
   const char *typestr;
   char buf[128];

   msg = malloc(sizeof(*msg) + sizeof(int));
   msg->count = 2;
   msg->val[0] = state;
   /* FIXME check if it's possible to receive strength as props of cm */
   if (type == -1)
       msg->val[1] = 0;
   else
       msg->val[1] = 100;

   edje_object_message_send(o, EDJE_MESSAGE_INT_SET, 1, msg);
   free(msg);

   typestr = econnman_service_type_to_str(type);
   snprintf(buf, sizeof(buf), "e,changed,technology,%s", typestr);
   edje_object_signal_emit(o, buf, "e");

   DBG("state=%d type=%d", state, type);
}

static enum Connman_Service_Type _econnman_manager_service_type_get(
                                                   struct Connman_Manager *cm)
{
   DBG("cm->services=%p", cm->services);

   if ((cm->services) && ((cm->state == CONNMAN_STATE_ONLINE) ||
                          (cm->state == CONNMAN_STATE_READY)))
     /* FIXME would be nice to represent "configuring state".
        theme already supports it */
     /*                 (cm->state == CONNMAN_STATE_ASSOCIATION) ||
                          (cm->state == CONNMAN_STATE_CONFIGURATION))) */
     {
        struct Connman_Service *cs = EINA_INLIST_CONTAINER_GET(cm->services,
                                                      struct Connman_Service);
        return cs->type;
     }
   return CONNMAN_SERVICE_TYPE_NONE;
}

void
econnman_mod_manager_update(struct Connman_Manager *cm)
{
   enum Connman_Service_Type type;
   E_Connman_Module_Context *ctxt = connman_mod->data;
   E_Connman_Instance *inst;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(cm);

   type = _econnman_manager_service_type_get(cm);
   EINA_LIST_FOREACH(ctxt->instances, l, inst)
     _econnman_mod_manager_update_inst(ctxt, inst, cm->state, type);
}

static void
_econnman_gadget_setup(E_Connman_Instance *inst)
{
   E_Connman_Module_Context *ctxt = inst->ctxt;
   Evas_Object *o = inst->ui.gadget;

   DBG("has_manager=%d", ctxt->cm != NULL);

   if (!ctxt->cm)
     {
        edje_object_signal_emit(o, "e,unavailable", "e");
        edje_object_signal_emit(o, "e,changed,connected,no", "e");
     }
   else
     edje_object_signal_emit(o, "e,available", "e");

   return;
}

void
econnman_mod_manager_inout(struct Connman_Manager *cm)
{
   E_Connman_Module_Context *ctxt = connman_mod->data;
   const Eina_List *l;
   E_Connman_Instance *inst;

   DBG("Manager %s", cm ? "in" : "out");
   ctxt->cm = cm;

   EINA_LIST_FOREACH(ctxt->instances, l, inst)
     _econnman_gadget_setup(inst);

   if (ctxt->cm)
     econnman_mod_manager_update(cm);
}

static void
_econnman_menu_cb_configure(void *data, E_Menu *menu EINA_UNUSED,
                            E_Menu_Item *mi EINA_UNUSED)
{
   E_Connman_Instance *inst = data;
   _econnman_app_launch(inst);
}

static void
_econnman_menu_new(E_Connman_Instance *inst, Evas_Event_Mouse_Down *ev)
{
   E_Menu *m;
   E_Menu_Item *mi;
   int x, y;

   m = e_menu_new();
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Settings"));
   e_util_menu_item_theme_icon_set(mi, "configure");
   e_menu_item_callback_set(mi, _econnman_menu_cb_configure, inst);

   m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
   e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
   e_menu_activate_mouse(m,
                         e_zone_current_get(),
                         x + ev->output.x, y + ev->output.y, 1, 1,
                         E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
}

static void
_econnman_cb_mouse_down(void *data, Evas *evas EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Connman_Instance *inst = data;
   Evas_Event_Mouse_Down *ev = event;

   if (!inst)
     return;

   if (ev->button == 1)
     {
        if (!inst->popup)
          _econnman_popup_new(inst);
     }
   else if (ev->button == 3)
     _econnman_menu_new(inst, ev);
}

/* Gadcon Api Functions */

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   E_Connman_Instance *inst;
   E_Connman_Module_Context *ctxt;
   enum Connman_Service_Type type;

   if (!connman_mod)
     return NULL;

   ctxt = connman_mod->data;

   inst = E_NEW(E_Connman_Instance, 1);
   inst->ctxt = ctxt;
   inst->ui.gadget = edje_object_add(gc->evas);
   e_theme_edje_object_set(inst->ui.gadget, "base/theme/modules/connman",
                           "e/modules/connman/main");

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->ui.gadget);
   inst->gcc->data = inst;

   evas_object_event_callback_add
     (inst->ui.gadget, EVAS_CALLBACK_MOUSE_DOWN, _econnman_cb_mouse_down, inst);

   _econnman_gadget_setup(inst);

   if (ctxt->cm)
     {
        /* Update main icon with the current state */
        type = _econnman_manager_service_type_get(ctxt->cm);
        _econnman_mod_manager_update_inst(ctxt, inst, ctxt->cm->state, type);
     }
   ctxt->instances = eina_list_append(ctxt->instances, inst);

   return inst->gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   E_Connman_Module_Context *ctxt;
   E_Connman_Instance *inst;

   if (!connman_mod)
     return;

   ctxt = connman_mod->data;
   if (!ctxt)
     return;

   inst = gcc->data;
   if (!inst)
     return;

   if (inst->popup)
     econnman_popup_del(inst);

   evas_object_del(inst->ui.gadget);

   ctxt->instances = eina_list_remove(ctxt->instances, inst);

   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _(_e_connman_Name);
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;

   o = edje_object_add(evas);
   edje_object_file_set(o, e_connman_theme_path(), "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   E_Connman_Module_Context *ctxt;
   Eina_List *instances;

   if (!connman_mod)
     return NULL;

   ctxt = connman_mod->data;
   if (!ctxt)
     return NULL;

   instances = ctxt->instances;
   snprintf(tmpbuf, sizeof(tmpbuf), "connman.%d", eina_list_count(instances));
   return tmpbuf;
}

static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, _e_connman_name,
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, _e_connman_Name };

static E_Config_Dialog *
_econnman_config(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Connman_Module_Context *ctxt;

   if (!connman_mod)
     return NULL;

   ctxt = connman_mod->data;
   if (!ctxt)
     return NULL;

   if (!ctxt->conf_dialog)
     ctxt->conf_dialog = e_connman_config_dialog_new(NULL, ctxt);

   return ctxt->conf_dialog;
}

static const char _reg_cat[] = "extensions";
static const char _reg_item[] = "extensions/connman";

static void
_econnman_configure_registry_register(void)
{
   e_configure_registry_category_add(_reg_cat, 90, _("Extensions"), NULL,
                                     "preferences-extensions");
   e_configure_registry_item_add(_reg_item, 110, _(_e_connman_Name), NULL,
                                 "preferences-network",
                                 _econnman_config);
}

static void
_econnman_configure_registry_unregister(void)
{
   e_configure_registry_item_del(_reg_item);
   e_configure_registry_category_del(_reg_cat);
}

E_API void *
e_modapi_init(E_Module *m)
{
   E_Connman_Module_Context *ctxt;
   Eldbus_Connection *c;

   if (_e_connman_log_dom < 0)
     {
        _e_connman_log_dom = eina_log_domain_register("econnman",
                                                      EINA_COLOR_ORANGE);
        if (_e_connman_log_dom < 0)
          {
             EINA_LOG_CRIT("could not register logging domain econnman");
             goto error_log_domain;
          }
     }

   ctxt = E_NEW(E_Connman_Module_Context, 1);
   if (!ctxt)
     goto error_connman_context;
   c = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!c)
     goto error_dbus_bus_get;
   if (!e_connman_system_init(c))
     goto error_connman_system_init;

   ctxt->conf_dialog = NULL;
   connman_mod = m;

   _econnman_configure_registry_register();
   e_gadcon_provider_register(&_gc_class);

   return ctxt;

error_connman_system_init:
   eldbus_connection_unref(c);
error_dbus_bus_get:
   E_FREE(ctxt);
error_connman_context:
   eina_log_domain_unregister(_e_connman_log_dom);
error_log_domain:
   _e_connman_log_dom = -1;
   return NULL;
}

static void
_econnman_instances_free(E_Connman_Module_Context *ctxt)
{
   while (ctxt->instances)
     {
        E_Connman_Instance *inst = ctxt->instances->data;
        ctxt->instances = eina_list_remove_list(ctxt->instances,
                                                ctxt->instances);
        e_object_del(E_OBJECT(inst->gcc));
     }
}

E_API int
e_modapi_shutdown(E_Module *m)
{
   E_Connman_Module_Context *ctxt;

   ctxt = m->data;
   if (!ctxt)
     return 0;

   e_connman_system_shutdown();

   _econnman_instances_free(ctxt);
   _econnman_configure_registry_unregister();
   e_gadcon_provider_unregister(&_gc_class);

   E_FREE(ctxt);
   connman_mod = NULL;

   eina_log_domain_unregister(_e_connman_log_dom);
   _e_connman_log_dom = -1;

   return 1;
}

E_API int
e_modapi_save(E_Module *m)
{
   E_Connman_Module_Context *ctxt;

   ctxt = m->data;
   if (!ctxt)
     return 0;
   return 1;
}
