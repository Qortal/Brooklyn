#include "e.h"

#define PATH "/org/enlightenment/wm/RemoteObject"

/* local subsystem functions */
static void            _e_msgbus_request_name_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending);

static Eldbus_Message *_e_msgbus_core_version_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);
static Eldbus_Message *_e_msgbus_core_restart_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);
static Eldbus_Message *_e_msgbus_core_shutdown_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);

static const Eldbus_Method core_methods[] =
{
   { "Version", NULL, ELDBUS_ARGS({"s", "version"}), _e_msgbus_core_version_cb, 0 },
   { "Restart", NULL, NULL, _e_msgbus_core_restart_cb, 0 },
   { "Shutdown", NULL, NULL, _e_msgbus_core_shutdown_cb, 0 },
   { NULL, NULL, NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc core_desc = {
   "org.enlightenment.wm.Core", core_methods, NULL, NULL, NULL, NULL
};

/* local subsystem globals */
static E_Msgbus_Data *_e_msgbus_data = NULL;

/* externally accessible functions */
EINTERN int
e_msgbus_init(void)
{
   _e_msgbus_data = E_NEW(E_Msgbus_Data, 1);

   eldbus_init();

   _e_msgbus_data->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   if (!_e_msgbus_data->conn)
     {
        WRN("Cannot get ELDBUS_CONNECTION_TYPE_SESSION");
        return 0;
     }

   _e_msgbus_data->iface = eldbus_service_interface_register
     (_e_msgbus_data->conn, PATH, &core_desc);
   eldbus_name_request(_e_msgbus_data->conn,
                       "org.enlightenment.wm.service",
                       0, _e_msgbus_request_name_cb, NULL);
   return 1;
}

EINTERN int
e_msgbus_shutdown(void)
{
   if (_e_msgbus_data->iface)
     eldbus_service_object_unregister(_e_msgbus_data->iface);
   if (_e_msgbus_data->conn)
     {
        eldbus_name_release(_e_msgbus_data->conn,
                            "org.enlightenment.wm.service",
                            NULL, NULL);
        eldbus_connection_unref(_e_msgbus_data->conn);
     }
   eldbus_shutdown();

   E_FREE(_e_msgbus_data);
   _e_msgbus_data = NULL;
   return 1;
}

E_API Eldbus_Service_Interface *
e_msgbus_interface_attach(const Eldbus_Service_Interface_Desc *desc)
{
   if (!_e_msgbus_data->iface) return NULL;
   return eldbus_service_interface_register(_e_msgbus_data->conn, PATH, desc);
}

static void
_e_msgbus_request_name_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
                          Eldbus_Pending *pending EINA_UNUSED)
{
   unsigned int flag;

   if (eldbus_message_error_get(msg, NULL, NULL))
     {
        ERR("Could not request bus name");
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u", &flag))
     {
        ERR("Could not get arguments on on_name_request");
        return;
     }

   if (!(flag & ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER))
     WRN("Name already in use\n");
}

/* Core Handlers */
static Eldbus_Message *
_e_msgbus_core_version_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);
   eldbus_message_arguments_append(reply, "s", VERSION);
   return reply;
}

static Eldbus_Message *
_e_msgbus_core_restart_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const Eldbus_Message *msg)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     ERR("DBus restart API disabled for security reasons");
   else
     e_sys_action_do(E_SYS_RESTART, NULL);
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_e_msgbus_core_shutdown_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                           const Eldbus_Message *msg)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     ERR("DBus shutdown API disabled for security reasons");
   else
     e_sys_action_do(E_SYS_EXIT, NULL);
   return eldbus_message_method_return_new(msg);
}
