#include "e_mod_main.h"

static int _log_dom = -1;
#undef DBG
#undef WARN
#undef INF
#undef ERR
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

static Eldbus_Message *_e_msgbus_profile_set_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);
static Eldbus_Message *_e_msgbus_profile_get_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);
static Eldbus_Message *_e_msgbus_profile_list_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);
static Eldbus_Message *_e_msgbus_profile_add_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);
static Eldbus_Message *_e_msgbus_profile_delete_cb(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg);

static const Eldbus_Method profile_methods[] = {
   { "Set", ELDBUS_ARGS({"s", "profile"}), NULL, _e_msgbus_profile_set_cb, 0 },
   { "Get", NULL, ELDBUS_ARGS({"s", "profile"}), _e_msgbus_profile_get_cb, 0 },
   { "List", NULL, ELDBUS_ARGS({"as", "array_profiles"}), _e_msgbus_profile_list_cb, 0 },
   { "Add", ELDBUS_ARGS({"s", "profile"}), NULL, _e_msgbus_profile_add_cb, 0 },
   { "Delete", ELDBUS_ARGS({"s", "profile"}), NULL, _e_msgbus_profile_delete_cb, 0 },
   { NULL, NULL, NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc profile = {
   "org.enlightenment.wm.Profile", profile_methods, NULL, NULL, NULL, NULL
};

/* Profile Handlers */
static Eldbus_Message *
_e_msgbus_profile_set_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                         const Eldbus_Message *msg)
{
   char *prof;
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);

   if (!eldbus_message_arguments_get(msg, "s", &prof))
     return reply;

   e_config_save_flush();
   e_config_profile_set(prof);
   e_config_profile_save();
   e_config_save_block_set(1);
   e_sys_action_do(E_SYS_RESTART, NULL);

   return reply;
}

static Eldbus_Message *
_e_msgbus_profile_get_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                         const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   const char *prof;

   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);
   prof = e_config_profile_get();
   eldbus_message_arguments_append(reply, "s", prof);
   return reply;
}

static Eldbus_Message *
_e_msgbus_profile_list_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                          const Eldbus_Message *msg)
{
   Eina_List *l;
   char *name;
   Eldbus_Message *reply;
   Eldbus_Message_Iter *array, *main_iter;

   reply = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(reply, NULL);

   main_iter = eldbus_message_iter_get(reply);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(main_iter, reply);

   eldbus_message_iter_arguments_append(main_iter, "as", &array);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(array, reply);

   l = e_config_profile_list();
   EINA_LIST_FREE(l, name)
     {
        eldbus_message_iter_basic_append(array, 's', name);
        free(name);
     }
   eldbus_message_iter_container_close(main_iter, array);

   return reply;
}

static Eldbus_Message *
_e_msgbus_profile_add_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                         const Eldbus_Message *msg)
{
   char *prof;
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);

   if (!eldbus_message_arguments_get(msg, "s", &prof))
     return reply;
   e_config_profile_add(prof);

   return reply;
}

static Eldbus_Message *
_e_msgbus_profile_delete_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                            const Eldbus_Message *msg)
{
   char *prof;

   if (!eldbus_message_arguments_get(msg, "s", &prof))
     return eldbus_message_method_return_new(msg);
   if (!strcmp(e_config_profile_get(), prof))
     return eldbus_message_error_new(msg,
                                    "org.enlightenment.DBus.InvalidArgument",
                                    "Can't delete active prof");
   e_config_profile_del(prof);
   return eldbus_message_method_return_new(msg);
}

void msgbus_profile_init(Eina_Array *ifaces)
{
   Eldbus_Service_Interface *iface;

   if (_log_dom == -1)
     {
        _log_dom = eina_log_domain_register("msgbus_profile", EINA_COLOR_BLUE);
        if (_log_dom < 0)
          EINA_LOG_ERR("could not register msgbus_profile log domain!");
     }

   iface = e_msgbus_interface_attach(&profile);
   if (iface) eina_array_push(ifaces, iface);
}
