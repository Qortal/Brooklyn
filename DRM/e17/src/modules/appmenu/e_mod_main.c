#include "e_mod_appmenu_private.h"

static E_Module *appmenu_module = NULL;

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   E_AppMenu_Instance *inst = gcc->data;
   switch (orient)
     {
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
        inst->orientation_horizontal = EINA_TRUE;
        break;
      case E_GADCON_ORIENT_VERT:
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_RB:
      default:
        inst->orientation_horizontal = EINA_FALSE;
        break;
     }
   if (inst->orientation_horizontal)
     evas_object_box_layout_set(inst->box, evas_object_box_layout_horizontal, NULL, NULL);
   else
     evas_object_box_layout_set(inst->box, evas_object_box_layout_vertical, NULL, NULL);
   appmenu_menu_of_instance_render(inst, inst->ctx->window);
}

/* Gadcon Api Functions */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   E_AppMenu_Instance *inst;
   E_AppMenu_Context *ctxt;

   EINA_SAFETY_ON_NULL_RETURN_VAL(appmenu_module, NULL);
   inst = calloc(1, sizeof(E_AppMenu_Instance));
   EINA_SAFETY_ON_NULL_RETURN_VAL(inst, NULL);

   ctxt = appmenu_module->data;
   ctxt->instances = eina_list_append(ctxt->instances, inst);
   inst->evas = gc->evas;
   inst->ctx = ctxt;

   inst->box = evas_object_box_add(inst->evas);
   evas_object_show(inst->box);

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->box);
   if (!inst->gcc)
     {
        evas_object_del(inst->box);
        ctxt->instances = eina_list_remove(ctxt->instances, inst);
        free(inst);
        return NULL;
     }
   inst->gcc->data = inst;
   _gc_orient(inst->gcc, inst->gcc->gadcon->orient);

   if (!ctxt->iface)
     appmenu_dbus_registrar_server_init(ctxt);

   return inst->gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   E_AppMenu_Instance *inst = gcc->data;
   appmenu_cancel();
   evas_object_del(inst->box);
   inst->ctx->instances = eina_list_remove(inst->ctx->instances, inst);
   if (!inst->ctx->instances)
     appmenu_dbus_registrar_server_shutdown(inst->ctx);
   free(inst);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Application Menu");
}

static char tmpbuf[1024]; /* general purpose buffer, just use immediately */

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;
   EINA_SAFETY_ON_NULL_RETURN_VAL(appmenu_module, NULL);
   snprintf(tmpbuf, sizeof(tmpbuf), "%s/e-module-appmenu.edj",
            e_module_dir_get(appmenu_module));
   o = edje_object_add(evas);
   edje_object_file_set(o, tmpbuf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   E_AppMenu_Context *ctxt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(appmenu_module, NULL);
   ctxt = appmenu_module->data;
   snprintf(tmpbuf, sizeof(tmpbuf), "appmenu.%d",
            eina_list_count(ctxt->instances));
   return tmpbuf;
}

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "AppMenu" };

static Eina_Bool
cb_focus_in(void *data, int type EINA_UNUSED, void *event)
{
   E_AppMenu_Context *ctxt = data;
   E_Event_Client *ev = event;
   Eina_List *l;
   E_AppMenu_Window *w, *found = NULL;
   ctxt->window_with_focus = e_client_util_win_get(ev->ec);

   EINA_LIST_FOREACH(ctxt->windows, l, w)
     {
        if (w->window_id == ctxt->window_with_focus)
          {
             found = w;
             break;
          }
     }
   appmenu_menu_render(ctxt, found);
   return EINA_TRUE;
}

static Eina_Bool
cb_focus_out(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   E_AppMenu_Context *ctxt = data;
   if (appmenu_menu_count_get() == 0) appmenu_menu_render(ctxt, NULL);
   return EINA_TRUE;
}

static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, "appmenu",
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

E_API void *
e_modapi_init(E_Module *m)
{
   E_AppMenu_Context *ctxt;
   Ecore_Event_Handler *event;

   ctxt = calloc(1, sizeof(E_AppMenu_Context));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctxt, NULL);

   appmenu_module = m;

   ctxt->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   event = ecore_event_handler_add(E_EVENT_CLIENT_FOCUS_IN, cb_focus_in, ctxt);
   ctxt->events[0] = event;
   event = ecore_event_handler_add(E_EVENT_CLIENT_FOCUS_OUT, cb_focus_out, ctxt);
   ctxt->events[1] = event;

   e_gadcon_provider_register(&_gc_class);

   return ctxt;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}

E_API int
e_modapi_shutdown(E_Module *m)
{
   E_AppMenu_Context *ctxt = m->data;
   E_AppMenu_Window *w;
   Eina_List *l, *l2;

   ecore_event_handler_del(ctxt->events[0]);
   ecore_event_handler_del(ctxt->events[1]);

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctxt, 0);
   EINA_LIST_FOREACH_SAFE(ctxt->windows, l, l2, w)
     appmenu_window_free(w);

   appmenu_dbus_registrar_server_shutdown(ctxt);
   eldbus_connection_unref(ctxt->conn);
   free(ctxt);
   return 1;
}

void
appmenu_window_free(E_AppMenu_Window *window)
{
   window->ctxt->windows = eina_list_remove(window->ctxt->windows, window);
   e_dbusmenu_unload(window->dbus_menu);
   eldbus_name_owner_changed_callback_del(window->ctxt->conn, window->bus_id,
                                         appmenu_application_monitor, window);
   eina_stringshare_del(window->bus_id);
   eina_stringshare_del(window->path);
   free(window);
}
