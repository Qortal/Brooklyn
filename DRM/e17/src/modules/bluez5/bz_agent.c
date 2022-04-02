#include "bz.h"
#include "e.h"

static Eldbus_Service_Interface *agent_iface = NULL;
static Eldbus_Object *agent_obj = NULL;
static Eldbus_Proxy *agent_proxy = NULL;
static void (*fn_release) (void) = NULL;
static void (*fn_cancel) (void) = NULL;
static void (*fn_req_pin) (Eldbus_Message *msg) = NULL;
static void (*fn_disp_pin) (Eldbus_Message *msg) = NULL;
static void (*fn_req_pass) (Eldbus_Message *msg) = NULL;
static void (*fn_disp_pass) (Eldbus_Message *msg) = NULL;
static void (*fn_req_confirm) (Eldbus_Message *msg) = NULL;
static void (*fn_req_auth) (Eldbus_Message *msg) = NULL;
static void (*fn_auth_service) (Eldbus_Message *msg) = NULL;

static Eldbus_Message *
cb_agent_release(const Eldbus_Service_Interface *iface EINA_UNUSED,
                 const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);

   if (fn_release) fn_release();
   return reply;
}

static Eldbus_Message *
cb_agent_cancel(const Eldbus_Service_Interface *iface EINA_UNUSED,
                const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);

   if (fn_cancel) fn_cancel();
   return reply;
}

static Eldbus_Message *
cb_agent_request_pin_code(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const Eldbus_Message *msg)
{
   if (fn_req_pin) fn_req_pin((Eldbus_Message *)msg);
   return NULL;
}

static Eldbus_Message *
cb_agent_display_pin_code(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const Eldbus_Message *msg)
{
   if (fn_disp_pin) fn_disp_pin((Eldbus_Message *)msg);
   return NULL;
}

static Eldbus_Message *
cb_agent_request_pass_key(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const Eldbus_Message *msg)
{
   if (fn_req_pass) fn_req_pass((Eldbus_Message *)msg);
   return NULL;
}

static Eldbus_Message *
cb_agent_display_pass_key(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const Eldbus_Message *msg)
{
   if (fn_disp_pass) fn_disp_pass((Eldbus_Message *)msg);
   return NULL;
}

static Eldbus_Message *
cb_agent_request_confirmation(const Eldbus_Service_Interface *iface EINA_UNUSED,
                              const Eldbus_Message *msg)
{
   if (fn_req_confirm) fn_req_confirm((Eldbus_Message *)msg);
   return NULL;
}

static Eldbus_Message *
cb_agent_request_authorization(const Eldbus_Service_Interface *iface EINA_UNUSED,
                               const Eldbus_Message *msg)
{
   if (fn_req_auth) fn_req_auth((Eldbus_Message *)msg);
   return NULL;
}

static Eldbus_Message *
cb_agent_authorize_service(const Eldbus_Service_Interface *iface EINA_UNUSED,
                           const Eldbus_Message *msg)
{
   if (fn_auth_service) fn_auth_service((Eldbus_Message *)msg);
   return NULL;
/* Not done yet... don't even know what services are with bt...
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   const char *path = NULL, *uuid = NULL;

   printf("Agent authorize service\n");
   if (!eldbus_message_arguments_get(msg, "os", &path, &uuid)) return reply;
   printf("  %s %s\n", path, uuid);
   // if ok return this reply, or make it an error...
   // reply = eldbus_message_error_new(msg, "org.bluez.Error.Rejected", "");
   return reply;
   // really return NULL and defer above reply with:
   // eldbus_connection_send(bz_conn, reply, NULL, NULL, -1);
 */
}

static void
cb_default(void *data EINA_UNUSED, const Eldbus_Message *msg,
           Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;

   if (eldbus_message_error_get(msg, &name, &text))
     {
        // XXX: should have a visual e log...
        e_util_dialog_show(_("Bluetooth"),
                           _("Could not register default agent:<br>%s %s"),
                           name, text);
        return;
     }
}

static void
cb_register(void *data EINA_UNUSED, const Eldbus_Message *msg,
            Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;

   if (eldbus_message_error_get(msg, &name, &text))
     {
        // XXX: should have a visual e log...
        e_util_dialog_show(_("Bluetooth"),
                           _("Could not register agent:<br>%s %s\n"),
                           name, text);
        return;
     }
   if (!agent_proxy) return;
   eldbus_proxy_call(agent_proxy, "RequestDefaultAgent",
                     cb_default, NULL, -1,
                     "o", "/org/enlightenment/bluez5/agent");
}

static void
cb_unregister(void *data EINA_UNUSED, const Eldbus_Message *msg,
            Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;

   if (agent_proxy)
     {
        eldbus_proxy_unref(agent_proxy);
        agent_proxy = NULL;
     }
   if (agent_obj)
     {
        eldbus_object_unref(agent_obj);
        agent_obj = NULL;
     }
   if (agent_iface)
     {
        eldbus_service_object_unregister(agent_iface);
        agent_iface = NULL;
     }
   if (eldbus_message_error_get(msg, &name, &text))
     {
        // just debug for developers
        printf("Could not unregister agent.\n %s:\n %s\n", name, text);
        return;
     }
}

static const Eldbus_Method agent_methods[] = {
   { "Release",              NULL,                                                                   NULL,                            cb_agent_release, 0 },
   { "RequestPinCode",       ELDBUS_ARGS({ "o", "device" }),                                         ELDBUS_ARGS({ "s", "pincode" }), cb_agent_request_pin_code, 0 },
   { "DisplayPinCode",       ELDBUS_ARGS({ "o", "device" }, { "s", "pincode" }),                     NULL,                            cb_agent_display_pin_code, 0 },
   { "RequestPasskey",       ELDBUS_ARGS({ "o", "device" }),                                         ELDBUS_ARGS({ "u", "passkey" }), cb_agent_request_pass_key, 0 },
   { "DisplayPasskey",       ELDBUS_ARGS({ "o", "device" }, { "u", "passkey" }, { "q", "entered" }), NULL,                            cb_agent_display_pass_key, 0 },
   { "RequestConfirmation",  ELDBUS_ARGS({ "o", "device" }, { "u", "passkey" }),                     NULL,                            cb_agent_request_confirmation, 0 },
   { "RequestAuthorization", ELDBUS_ARGS({ "o", "device" }),                                         NULL,                            cb_agent_request_authorization, 0 },
   { "AuthorizeService",     ELDBUS_ARGS({ "o", "device" }, { "s", "uuid" }),                        NULL,                            cb_agent_authorize_service, 0 },
   { "Cancel",               NULL,                                                                   NULL,                            cb_agent_cancel, 0 },

   { NULL,                   NULL, NULL, NULL, 0 }
};
static const Eldbus_Service_Interface_Desc agent_desc = {
   "org.bluez.Agent1", agent_methods, NULL, NULL, NULL, NULL
};

void
bz_agent_msg_reply(Eldbus_Message *msg)
{
   if (!bz_conn)
     {
        eldbus_message_unref(msg);
        return;
     }
   eldbus_connection_send(bz_conn, msg, NULL, NULL, -1);
}

void
bz_agent_msg_drop(Eldbus_Message *msg)
{
   eldbus_message_unref(msg);
}

Eldbus_Message *
bz_agent_msg_err(Eldbus_Message *msg)
{
   return eldbus_message_error_new(msg, "org.bluez.Error.Rejected", "");
}

Eldbus_Message *
bz_agent_msg_ok(Eldbus_Message *msg)
{
   return eldbus_message_method_return_new(msg);
}

const char *
bz_agent_msg_path(Eldbus_Message *msg)
{
   const char *s = NULL;

   if (!eldbus_message_arguments_get(msg, "o", &s)) return NULL;
   return s;
}

const char *
bz_agent_msg_path_str(Eldbus_Message *msg, const char **str)
{
   const char *s = NULL, *s2 = NULL;

   if (!eldbus_message_arguments_get(msg, "os", &s, &s2)) return NULL;
   if (str) *str = s2;
   return s;
}

const char *
bz_agent_msg_path_u32(Eldbus_Message *msg, unsigned int *u32)
{
   const char *s = NULL;
   unsigned int uu32 = 0;

   if (!eldbus_message_arguments_get(msg, "ou", &s, &uu32)) return NULL;
   if (u32) *u32 = uu32;
   return s;
}

const char *
bz_agent_msg_path_u32_u16(Eldbus_Message *msg, unsigned int *u32, unsigned short *u16)
{
   const char *s = NULL;
   unsigned int uu32 = 0;
   unsigned short uu16 = 0;

   if (!eldbus_message_arguments_get(msg, "ouq", &s, &uu32, &uu16)) return NULL;
   if (u32) *u32 = uu32;
   if (u16) *u16 = uu16;
   return s;
}

void
bz_agent_msg_str_add(Eldbus_Message *msg, const char *str)
{
   eldbus_message_arguments_append(msg, "s", str);
}

void
bz_agent_msg_u32_add(Eldbus_Message *msg, unsigned int u32)
{
   eldbus_message_arguments_append(msg, "u", &u32);
}

void
bz_agent_init(void)
{
   agent_obj = eldbus_object_get(bz_conn, "org.bluez", "/org/bluez");
   agent_proxy = eldbus_proxy_get(agent_obj, "org.bluez.AgentManager1");
   agent_iface = eldbus_service_interface_register
     (bz_conn, "/org/enlightenment/bluez5/agent", &agent_desc);
   if (agent_proxy)
     eldbus_proxy_call(agent_proxy, "RegisterAgent",
                       cb_register, NULL, -1,
                       "os", "/org/enlightenment/bluez5/agent",
                       "KeyboardDisplay");
   else
     e_util_dialog_show(_("Bluetooth"),
                        _("Could not call RegisterAgent\n"));
}

void
bz_agent_shutdown(void)
{
   if (!agent_proxy) return;
   eldbus_proxy_call(agent_proxy, "UnregisterAgent",
                     cb_unregister, NULL, -1,
                     "o", "/org/enlightenment/bluez5/agent");
}

void
bz_agent_release_func_set(void (*fn) (void))
{
   fn_release = fn;
}

void
bz_agent_cancel_func_set(void (*fn) (void))
{
   fn_cancel = fn;
}

void
bz_agent_req_pin_func_set(void (*fn) (Eldbus_Message *msg))
{
   fn_req_pin = fn;
}

void
bz_agent_disp_pin_func_set(void (*fn) (Eldbus_Message *msg))
{
   fn_disp_pin = fn;
}

void
bz_agent_req_pass_func_set(void (*fn) (Eldbus_Message *msg))
{
   fn_req_pass = fn;
}

void
bz_agent_disp_pass_func_set(void (*fn) (Eldbus_Message *msg))
{
   fn_disp_pass = fn;
}

void
bz_agent_req_confirm_func_set(void (*fn) (Eldbus_Message *msg))
{
   fn_req_confirm = fn;
}

void
bz_agent_req_auth_func_set(void (*fn) (Eldbus_Message *msg))
{
   fn_req_auth = fn;
}

void
bz_agent_auth_service_func_set(void (*fn) (Eldbus_Message *msg))
{
   fn_auth_service = fn;
}
