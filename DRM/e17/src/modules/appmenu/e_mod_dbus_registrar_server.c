#include "e_mod_appmenu_private.h"

#define REGISTRAR_BUS "com.canonical.AppMenu.Registrar"
#define REGISTRAR_PATH "/com/canonical/AppMenu/Registrar"
#define REGISTRAR_IFACE "com.canonical.AppMenu.Registrar"

#define ERROR_WINDOW_NOT_FOUND "com.canonical.AppMenu.Registrar.WindowNotFound"

enum
{
   SIGNAL_WINDOW_REGISTERED = 0,
   SIGNAL_WINDOW_UNREGISTERED,
};

void
appmenu_application_monitor(void *data, const char *bus EINA_UNUSED, const char *old EINA_UNUSED, const char *new EINA_UNUSED)
{
   E_AppMenu_Window *window = data;
   eldbus_service_signal_emit(window->ctxt->iface, SIGNAL_WINDOW_UNREGISTERED,
                             window->window_id);
   appmenu_window_free(window);
}

static void
menu_update_cb(void *data, E_DBusMenu_Item *item)
{
   E_AppMenu_Window *window = data;
   window->root_item = item;
   if (!item)
     return;
   if (window->window_id == window->ctxt->window_with_focus)
     appmenu_menu_render(window->ctxt, window);
}

static void
menu_pop_cb(void *data EINA_UNUSED, const E_DBusMenu_Item *new_root_item EINA_UNUSED)
{
   //TODO
}

static Eldbus_Message *
_on_register_window(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Connection *conn = eldbus_service_connection_get(iface);
   E_AppMenu_Context *ctxt = eldbus_service_object_data_get(iface, "ctxt");
   unsigned window_id;
   const char *path, *bus_id;
   E_AppMenu_Window *window;

   if (!eldbus_message_arguments_get(msg, "uo", &window_id, &path))
     {
        ERR("Error reading message");
        return NULL;
     }

   window = calloc(1, sizeof(E_AppMenu_Window));
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);


   bus_id = eldbus_message_sender_get(msg);

   window->window_id = window_id;
   window->dbus_menu = e_dbusmenu_load(conn, bus_id, path, window);
   e_dbusmenu_update_cb_set(window->dbus_menu, menu_update_cb);
   e_dbusmenu_pop_request_cb_set(window->dbus_menu, menu_pop_cb);
   window->bus_id = eina_stringshare_add(bus_id);
   window->path = eina_stringshare_add(path);

   eldbus_name_owner_changed_callback_add(conn, bus_id, appmenu_application_monitor,
                                         window, EINA_FALSE);
   ctxt->windows = eina_list_append(ctxt->windows, window);
   window->ctxt = ctxt;

   eldbus_service_signal_emit(iface, SIGNAL_WINDOW_REGISTERED, window_id,
                             bus_id, path);
   return eldbus_message_method_return_new(msg);
}

static E_AppMenu_Window *
window_find(E_AppMenu_Context *ctxt, unsigned window_id)
{
   Eina_List *l;
   E_AppMenu_Window *w;
   EINA_LIST_FOREACH(ctxt->windows, l, w)
     {
        if (w->window_id == window_id)
          return w;
     }
   return NULL;
}

static Eldbus_Message *
_on_unregister_window(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   E_AppMenu_Context *ctxt = eldbus_service_object_data_get(iface, "ctxt");
   E_AppMenu_Window *w;
   unsigned window_id;

   if (!eldbus_message_arguments_get(msg, "u", &window_id))
     {
        ERR("Error reading message.");
        return NULL;
     }

   w = window_find(ctxt, window_id);
   if (w)
     appmenu_window_free(w);
   eldbus_service_signal_emit(iface, SIGNAL_WINDOW_UNREGISTERED, window_id);
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_on_getmenu(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   unsigned window_id;
   Eina_List *l;
   E_AppMenu_Window *w;
   E_AppMenu_Context *ctxt = eldbus_service_object_data_get(iface, "ctxt");

   if (!eldbus_message_arguments_get(msg, "u", &window_id))
     {
        ERR("Error reading message");
        return NULL;
     }
   EINA_LIST_FOREACH(ctxt->windows, l, w)
     {
        if (w->window_id == window_id)
          {
             Eldbus_Message *reply;
             reply = eldbus_message_method_return_new(msg);
             eldbus_message_arguments_append(reply, "so", w->bus_id, w->path);
             return reply;
          }
     }
   return eldbus_message_error_new(msg, ERROR_WINDOW_NOT_FOUND, "");
}

static Eldbus_Message *
_on_getmenus(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eina_List *l;
   E_AppMenu_Window *w;
   E_AppMenu_Context *ctxt = eldbus_service_object_data_get(iface, "ctxt");
   Eldbus_Message *reply;
   Eldbus_Message_Iter *array, *main_iter;

   reply = eldbus_message_method_return_new(msg);
   main_iter = eldbus_message_iter_get(reply);
   eldbus_message_iter_arguments_append(main_iter, "a(uso)", &array);

   EINA_LIST_FOREACH(ctxt->windows, l, w)
     {
        Eldbus_Message_Iter *entry;
        eldbus_message_iter_arguments_append(array, "(uso)", &entry);
        eldbus_message_iter_arguments_append(entry, "uso", w->window_id,
                                            w->bus_id, w->path);
        eldbus_message_iter_container_close(array, entry);
     }

   eldbus_message_iter_container_close(main_iter, array);
   return reply;
}

static const Eldbus_Method registrar_methods[] = {
   { "RegisterWindow", ELDBUS_ARGS({"u", "windowId"},{"o", "menuObjectPath"}),
      NULL, _on_register_window, 0 },
   { "UnregisterWindow", ELDBUS_ARGS({"u", "windowId"}),
      NULL, _on_unregister_window, 0 },
   { "GetMenuForWindow", ELDBUS_ARGS({"u", "windowId"}),
     ELDBUS_ARGS({"s", "bus_id"},{"o", "menu_path"}), _on_getmenu, 0 },
   { "GetMenus", NULL, ELDBUS_ARGS({"a(uso)", "array_of_menu"}), _on_getmenus, 0 },
   { NULL, NULL, NULL, NULL, 0}
};

static const Eldbus_Signal registrar_signals[] = {
   [SIGNAL_WINDOW_REGISTERED] =
     { "WindowRegistered",
        ELDBUS_ARGS({"u", "windowId"}, {"s", "bus_id"}, {"o", "menu_path"}), 0 },
   [SIGNAL_WINDOW_UNREGISTERED] =
     { "WindowUnregistered", ELDBUS_ARGS({"u", "windowId"}), 0 },
   { NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc registrar_iface = {
   REGISTRAR_IFACE, registrar_methods, registrar_signals, NULL, NULL, NULL
};

void
appmenu_dbus_registrar_server_init(E_AppMenu_Context *ctx)
{
   ctx->iface = eldbus_service_interface_register(ctx->conn,
                                                 REGISTRAR_PATH,
                                                 &registrar_iface);
   eldbus_service_object_data_set(ctx->iface, "ctxt", ctx);
   eldbus_name_request(ctx->conn, REGISTRAR_BUS,
                      ELDBUS_NAME_REQUEST_FLAG_REPLACE_EXISTING, NULL, NULL);
}

void
appmenu_dbus_registrar_server_shutdown(E_AppMenu_Context *ctx)
{
   if (ctx->iface) eldbus_service_interface_unregister(ctx->iface);
   if (ctx->conn) eldbus_name_release(ctx->conn, REGISTRAR_BUS, NULL, NULL);
   ctx->iface = NULL;
}
