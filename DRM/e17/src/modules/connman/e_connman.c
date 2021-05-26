#include "e_mod_main.h"

#define CONNMAN_BUS_NAME "net.connman"
#define CONNMAN_MANAGER_IFACE CONNMAN_BUS_NAME ".Manager"
#define CONNMAN_SERVICE_IFACE CONNMAN_BUS_NAME ".Service"
#define CONNMAN_TECHNOLOGY_IFACE CONNMAN_BUS_NAME ".Technology"

#define MILLI_PER_SEC 1000
#define CONNMAN_CONNECTION_TIMEOUT 60 * MILLI_PER_SEC

static unsigned int init_count;
static Eldbus_Connection *conn;
static struct Connman_Manager *connman_manager;
static E_Connman_Agent *agent;

E_API int E_CONNMAN_EVENT_MANAGER_IN;
E_API int E_CONNMAN_EVENT_MANAGER_OUT;

/* utility functions */

static void _eina_str_array_clean(Eina_Array *array)
{
   const char *item;
   Eina_Array_Iterator itr;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(array, i, item, itr)
     eina_stringshare_del(item);

   eina_array_clean(array);
}

static void _dbus_str_array_to_eina(Eldbus_Message_Iter *value, Eina_Array **old,
                                    unsigned nelem)
{
   Eldbus_Message_Iter *itr_array;
   Eina_Array *array;
   const char *s;
   EINA_SAFETY_ON_NULL_RETURN(value);
   EINA_SAFETY_ON_NULL_RETURN(old);

   EINA_SAFETY_ON_FALSE_RETURN(eldbus_message_iter_arguments_get(value, "as",
                                                                &itr_array));

   array = *old;
   if (array == NULL)
     {
        array = eina_array_new(nelem);
        *old = array;
     }
   else
     _eina_str_array_clean(array);

   while (eldbus_message_iter_get_and_next(itr_array, 's', &s))
     {
        eina_array_push(array, eina_stringshare_add(s));
        DBG("Push %s", s);
     }

   return;
}

static enum Connman_State str_to_state(const char *s)
{
   if (!strcmp(s, "offline"))
     return CONNMAN_STATE_OFFLINE;
   if (!strcmp(s, "idle"))
     return CONNMAN_STATE_IDLE;
   if (!strcmp(s, "association"))
     return CONNMAN_STATE_ASSOCIATION;
   if (!strcmp(s, "configuration"))
     return CONNMAN_STATE_CONFIGURATION;
   if (!strcmp(s, "ready"))
     return CONNMAN_STATE_READY;
   if (!strcmp(s, "online"))
     return CONNMAN_STATE_ONLINE;
   if (!strcmp(s, "disconnect"))
     return CONNMAN_STATE_DISCONNECT;
   if (!strcmp(s, "failure"))
     return CONNMAN_STATE_FAILURE;

   ERR("Unknown state %s", s);
   return CONNMAN_STATE_NONE;
}

const char *econnman_state_to_str(enum Connman_State state)
{
   switch (state)
     {
      case CONNMAN_STATE_OFFLINE:
         return "offline";
      case CONNMAN_STATE_IDLE:
         return "idle";
      case CONNMAN_STATE_ASSOCIATION:
         return "association";
      case CONNMAN_STATE_CONFIGURATION:
         return "configuration";
      case CONNMAN_STATE_READY:
         return "ready";
      case CONNMAN_STATE_ONLINE:
         return "online";
      case CONNMAN_STATE_DISCONNECT:
         return "disconnect";
      case CONNMAN_STATE_FAILURE:
         return "failure";
      case CONNMAN_STATE_NONE:
         break;
     }

   return NULL;
}

static enum Connman_Service_Type str_to_type(const char *s)
{
   if (!strcmp(s, "ethernet"))
     return CONNMAN_SERVICE_TYPE_ETHERNET;
   else if (!strcmp(s, "wifi"))
     return CONNMAN_SERVICE_TYPE_WIFI;
   else if (!strcmp(s, "bluetooth"))
     return CONNMAN_SERVICE_TYPE_BLUETOOTH;
   else if (!strcmp(s, "cellular"))
     return CONNMAN_SERVICE_TYPE_CELLULAR;

   DBG("Unknown type %s", s);
   return CONNMAN_SERVICE_TYPE_NONE;
}

const char *econnman_service_type_to_str(enum Connman_Service_Type type)
{
   switch (type)
     {
      case CONNMAN_SERVICE_TYPE_ETHERNET:
         return "ethernet";
      case CONNMAN_SERVICE_TYPE_WIFI:
         return "wifi";
      case CONNMAN_SERVICE_TYPE_BLUETOOTH:
         return "bluetooth";
      case CONNMAN_SERVICE_TYPE_CELLULAR:
         return "cellular";
      case CONNMAN_SERVICE_TYPE_NONE:
         break;
     }

   return "other";
}

/* ---- */

static void _service_parse_prop_changed(struct Connman_Service *cs,
                                        const char *prop_name,
                                        Eldbus_Message_Iter *value)
{
   DBG("service %p %s prop %s", cs, cs->path, prop_name);

   if (strcmp(prop_name, "State") == 0)
     {
        const char *state;
        EINA_SAFETY_ON_FALSE_RETURN(eldbus_message_iter_arguments_get(value,
                                                                     "s",
                                                                     &state));
        cs->state = str_to_state(state);
        DBG("New state: %s %d", state, cs->state);
     }
   else if (strcmp(prop_name, "Name") == 0)
     {
        const char *name;
        EINA_SAFETY_ON_FALSE_RETURN(eldbus_message_iter_arguments_get(value,
                                                                     "s",
                                                                     &name));
        free(cs->name);
        cs->name = strdup(name);
        DBG("New name: %s", cs->name);
     }
   else if (strcmp(prop_name, "Type") == 0)
     {
        const char *type;
        EINA_SAFETY_ON_FALSE_RETURN(eldbus_message_iter_arguments_get(value,
                                                                     "s",
                                                                     &type));
        cs->type = str_to_type(type);
        DBG("New type: %s %d", type, cs->type);
     }
   else if (strcmp(prop_name, "Strength") == 0)
     {
        uint8_t strength;
        EINA_SAFETY_ON_FALSE_RETURN(eldbus_message_iter_arguments_get(value,
                                                                     "y",
                                                                     &strength));
        cs->strength = strength;
        DBG("New strength: %d", strength);
     }
   else if (strcmp(prop_name, "Security") == 0)
     {
        DBG("Old security count: %d",
            cs->security ? eina_array_count(cs->security) : 0);
        _dbus_str_array_to_eina(value, &cs->security, 2);
        DBG("New security count: %d", eina_array_count(cs->security));
     }
   else if (strcmp(prop_name, "Proxy") == 0)
      {
         Eldbus_Message_Iter* proxy_dict = NULL;
         const char* method = "direct";
         Eina_Array *proxy_servers = NULL, *proxy_excludes = NULL;
         Eldbus_Message_Iter* proxy_dict_entry = NULL;

         //  'Proxy': {'Excludes': [], 'Method': 'manual', 'Servers': []}
         if (!eldbus_message_iter_arguments_get(value, "a{sv}", &proxy_dict))
            {
               ERR("Error getting arguments.");
               return;
            }

         while (eldbus_message_iter_get_and_next(proxy_dict, 'e',
             &proxy_dict_entry))
            {
               Eldbus_Message_Iter* proxy_dict_entry_val = NULL;
               char* proxy_dict_entry_key = NULL;
               if (!eldbus_message_iter_arguments_get(proxy_dict_entry, "sv",
                       &proxy_dict_entry_key,
                       &proxy_dict_entry_val))
                  continue;
               if (strcmp(proxy_dict_entry_key, "Excludes") == 0)
                  {
                     _dbus_str_array_to_eina(proxy_dict_entry_val, &proxy_excludes, 4);
                  }
               else if (strcmp(proxy_dict_entry_key, "Method") == 0)
                  {
                     EINA_SAFETY_ON_FALSE_RETURN(eldbus_message_iter_arguments_get(
                         proxy_dict_entry_val, "s", &method));
                  }
               else if (strcmp(proxy_dict_entry_key, "Servers") == 0)
                  {
                     _dbus_str_array_to_eina(proxy_dict_entry_val, &proxy_servers, 4);
                  }
            }

         if (strcmp(method, "manual") == 0)
            {
               if (proxy_servers && (eina_array_count(proxy_servers) > 0))
                  {
                     DBG("New {all,http{,s}}_proxy: %s",
                         (const char*)eina_array_data_get(proxy_servers, 0));
                     e_env_set("ALL_PROXY", eina_array_data_get(proxy_servers, 0));
                     e_env_set("http_proxy", eina_array_data_get(proxy_servers, 0));
                     e_env_set("HTTPS_PROXY", eina_array_data_get(proxy_servers, 0));
                  }
               if (proxy_excludes && (eina_array_count(proxy_excludes) > 0))
                  {
                     Eina_Strbuf *buf;
                     Eina_Array_Iterator it;
                     unsigned int i;
                     const char *p;

                     buf = eina_strbuf_new();
                     EINA_ARRAY_ITER_NEXT(proxy_excludes, i, p, it)
                        {
                           if (i)
                             eina_strbuf_append_char(buf, ',');
                           eina_strbuf_append(buf, p);
                        }
                     DBG("New no_proxy: %s", eina_strbuf_string_get(buf));
                     e_env_set("NO_PROXY", eina_strbuf_string_get(buf));
                     eina_strbuf_free(buf);
                  }
            }
         else if (strcmp(method, "direct") == 0)
            {
               e_env_unset("ALL_PROXY");
               e_env_unset("http_proxy");
               e_env_unset("HTTPS_PROXY");
               e_env_unset("NO_PROXY");
            }

         if (proxy_excludes)
            {
               _eina_str_array_clean(proxy_excludes);
               eina_array_free(proxy_excludes);
            }
         if (proxy_servers)
            {
               _eina_str_array_clean(proxy_servers);
               eina_array_free(proxy_servers);
            }
      }
}

static void _service_prop_dict_changed(struct Connman_Service *cs,
                                       Eldbus_Message_Iter *props)
{
   Eldbus_Message_Iter *dict;
   while (eldbus_message_iter_get_and_next(props, 'e', &dict))
     {
        char *name;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(dict, "sv", &name, &var))
          continue;
        _service_parse_prop_changed(cs, name, var);
     }
}

static void _service_prop_changed(void *data, const Eldbus_Message *msg)
{
   struct Connman_Service *cs = data;
   Eldbus_Message_Iter *var;
   const char *name;

   if (!eldbus_message_arguments_get(msg, "sv", &name, &var))
     return;

   _service_parse_prop_changed(cs, name, var);
}

struct connection_data {
   struct Connman_Service *cs;
   Econnman_Simple_Cb cb;
   void *user_data;
};

static void _service_free(struct Connman_Service *cs)
{
   Eldbus_Object *obj;
   if (!cs)
     return;

   if (cs->pending.connect)
     {
        eldbus_pending_cancel(cs->pending.connect);
        free(cs->pending.data);
     }
   else if (cs->pending.disconnect)
     {
        eldbus_pending_cancel(cs->pending.disconnect);
        free(cs->pending.data);
     }
   else if (cs->pending.remov)
     {
        eldbus_pending_cancel(cs->pending.remov);
        free(cs->pending.data);
     }

   free(cs->name);
   _eina_str_array_clean(cs->security);
   eina_array_free(cs->security);
   eina_stringshare_del(cs->path);
   obj = eldbus_proxy_object_get(cs->service_iface);
   eldbus_proxy_unref(cs->service_iface);
   eldbus_object_unref(obj);

   free(cs);
}

static struct Connman_Service *_service_new(const char *path, Eldbus_Message_Iter *props)
{
   struct Connman_Service *cs;
   Eldbus_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   cs = calloc(1, sizeof(*cs));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cs, NULL);

   cs->path = eina_stringshare_add(path);

   obj = eldbus_object_get(conn, CONNMAN_BUS_NAME, path);
   cs->service_iface = eldbus_proxy_get(obj, CONNMAN_SERVICE_IFACE);
   eldbus_proxy_signal_handler_add(cs->service_iface, "PropertyChanged",
                                  _service_prop_changed, cs);

   _service_prop_dict_changed(cs, props);
   return cs;
}

static void _service_connection_cb(void *data, const Eldbus_Message *msg,
                                   Eldbus_Pending *pending EINA_UNUSED)
{
   struct connection_data *cd = data;

   if (cd->cb)
     {
        const char *s = NULL;
        eldbus_message_error_get(msg, NULL, &s);
        cd->cb(cd->user_data, s);
     }

   cd->cs->pending.connect = NULL;
   cd->cs->pending.disconnect = NULL;
   cd->cs->pending.remov = NULL;
   cd->cs->pending.data = NULL;

   free(cd);
}

bool econnman_service_connect(struct Connman_Service *cs,
                              Econnman_Simple_Cb cb, void *data)
{
   struct connection_data *cd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cs, false);

   if (cs->pending.connect || cs->pending.disconnect || cs->pending.remov)
     {
        ERR("Pending connection: connect=%p disconnect=%p remov=%p", cs->pending.connect,
            cs->pending.disconnect, cs->pending.remov);
        return false;
     }

   cd = calloc(1, sizeof(*cd));
   EINA_SAFETY_ON_NULL_GOTO(cd, fail);

   cd->cs = cs;
   cd->cb = cb;
   cd->user_data = data;

   cs->pending.connect = eldbus_proxy_call(cs->service_iface, "Connect",
                                          _service_connection_cb, cd,
                                          CONNMAN_CONNECTION_TIMEOUT, "");
   return true;

fail:
   return false;
}

bool econnman_service_disconnect(struct Connman_Service *cs,
                                 Econnman_Simple_Cb cb, void *data)
{
   struct connection_data *cd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cs, false);

   if (cs->pending.connect || cs->pending.disconnect || cs->pending.remov)
     {
        ERR("Pending connection: connect=%p disconnect=%p remov=%p", cs->pending.connect,
            cs->pending.disconnect, cs->pending.remov);
        return false;
     }

   cd = calloc(1, sizeof(*cd));
   EINA_SAFETY_ON_NULL_GOTO(cd, fail);

   cd->cs = cs;
   cd->cb = cb;
   cd->user_data = data;

   cs->pending.disconnect = eldbus_proxy_call(cs->service_iface, "Disconnect",
                                          _service_connection_cb, cd,
                                          -1, "");
   return true;

fail:
   return false;
}

bool econnman_service_remove(struct Connman_Service *cs,
                             Econnman_Simple_Cb cb, void *data)
{
   struct connection_data *cd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cs, false);

   if (cs->pending.connect || cs->pending.disconnect || cs->pending.remov)
     {
        ERR("Pending connection: connect=%p disconnect=%p remov=%p", cs->pending.connect,
            cs->pending.disconnect, cs->pending.remov);
        return false;
     }

   cd = calloc(1, sizeof(*cd));
   EINA_SAFETY_ON_NULL_GOTO(cd, fail);

   cd->cs = cs;
   cd->cb = cb;
   cd->user_data = data;

   cs->pending.remov = eldbus_proxy_call(cs->service_iface, "Remove",
                                         _service_connection_cb, cd,
                                         -1, "");
   return true;

fail:
   return false;
}

static struct Connman_Service *_manager_find_service_stringshared(
                                 struct Connman_Manager *cm, const char *path)
{
   struct Connman_Service *cs, *found = NULL;

   EINA_INLIST_FOREACH(cm->services, cs)
     {
        if (cs->path == path)
          {
             found = cs;
             break;
          }
     }
   return found;
}

struct Connman_Service *econnman_manager_find_service(struct Connman_Manager *cm,
                                                      const char *path)
{
   struct Connman_Service *cs;

   path = eina_stringshare_add(path);
   cs = _manager_find_service_stringshared(cm, path);
   eina_stringshare_del(path);
   return cs;
}

static void _manager_services_remove(struct Connman_Manager *cm,
                                     Eldbus_Message_Iter *array)
{
   const char *path;
   while (eldbus_message_iter_get_and_next(array, 'o', &path))
     {
        struct Connman_Service *cs;
        cs = econnman_manager_find_service(cm, path);
        if (cs == NULL)
          {
             ERR("Received object path '%s' to remove, but it's not in list",
                 path);
             continue;
          }
        cm->services = eina_inlist_remove(cm->services, EINA_INLIST_GET(cs));
        DBG("Removed service: %p %s", cs, path);
        _service_free(cs);
     }
}

static void _manager_services_changed(void *data, const Eldbus_Message *msg)
{
   struct Connman_Manager *cm = data;
   Eldbus_Message_Iter *changed, *removed, *s;
   Eina_Inlist *tmp = NULL;

   if (cm->pending.get_services)
     return;

   if (!eldbus_message_arguments_get(msg, "a(oa{sv})ao", &changed, &removed))
     {
        ERR("Error getting arguments");
        return;
     }

   _manager_services_remove(cm, removed);

   while (eldbus_message_iter_get_and_next(changed, 'r', &s))
     {
        struct Connman_Service *cs;
        const char *path;
        Eldbus_Message_Iter *array;

        if (!eldbus_message_iter_arguments_get(s, "oa{sv}", &path, &array))
          continue;

        cs = econnman_manager_find_service(cm, path);
        if (!cs)
          {
             cs = _service_new(path, array);
             DBG("Added service: %p %s", cs, path);
          }
        else
          {
             _service_prop_dict_changed(cs, array);
             cm->services = eina_inlist_remove(cm->services,
                                               EINA_INLIST_GET(cs));
             DBG("Changed service: %p %s", cs, path);
          }
        tmp = eina_inlist_append(tmp, EINA_INLIST_GET(cs));
     }

   cm->services = tmp;
   econnman_mod_services_changed(cm);
}

static void _manager_get_services_cb(void *data, const Eldbus_Message *msg,
                                     Eldbus_Pending *pending EINA_UNUSED)
{
   struct Connman_Manager *cm = data;
   Eldbus_Message_Iter *array, *s;
   const char *name, *text;

   cm->pending.get_services = NULL;

   if (eldbus_message_error_get(msg, &name, &text))
     {
        ERR("Could not get services. %s: %s", name, text);
        return;
     }
   DBG("cm->services=%p", cm->services);

   if (!eldbus_message_arguments_get(msg, "a(oa{sv})", &array))
     {
        ERR("Error getting array");
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 'r', &s))
     {
        struct Connman_Service *cs;
        const char *path;
        Eldbus_Message_Iter *inner_array;

        if (!eldbus_message_iter_arguments_get(s, "oa{sv}", &path, &inner_array))
          continue;

        cs = _service_new(path, inner_array);
        if (cs == NULL)
          continue;

        cm->services = eina_inlist_append(cm->services, EINA_INLIST_GET(cs));
        DBG("Added service: %p %s", cs, path);
     }
   econnman_mod_services_changed(cm);
}

static bool _manager_parse_prop_changed(struct Connman_Manager *cm,
                                        const char *name,
                                        Eldbus_Message_Iter *value)
{
   if (strcmp(name, "State") == 0)
     {
        const char *state;
        if (!eldbus_message_iter_arguments_get(value, "s", &state))
          return false;
        DBG("New state: %s", state);
        cm->state = str_to_state(state);
     }
   else if (strcmp(name, "OfflineMode") == 0)
     {
        if (!eldbus_message_iter_arguments_get(value, "b", &cm->offline_mode))
          return false;
     }
   else
     return false;

   econnman_mod_manager_update(cm);
   return true;
}

static void
_manager_prop_changed(void *data, const Eldbus_Message *msg)
{
   struct Connman_Manager *cm = data;
   Eldbus_Message_Iter *var;
   const char *name;

   if (!eldbus_message_arguments_get(msg, "sv", &name, &var))
     {
        ERR("Could not parse message %p", msg);
        return;
     }

   _manager_parse_prop_changed(cm, name, var);
}

static void _manager_get_prop_cb(void *data, const Eldbus_Message *msg,
                                 Eldbus_Pending *pending EINA_UNUSED)
{
   struct Connman_Manager *cm = data;
   Eldbus_Message_Iter *array, *dict;
   const char *name, *text;

   if (eldbus_message_error_get(msg, &name, &text))
     {
        WRN("Could not get properties. %s: %s", name, text);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a{sv}", &array))
     {
        WRN("Error getting arguments.");
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 'e', &dict))
     {
        const char *key;
        Eldbus_Message_Iter *var;

        if (!eldbus_message_iter_arguments_get(dict, "sv", &key, &var))
          continue;
        _manager_parse_prop_changed(cm, key, var);
     }
}

static bool _manager_parse_wifi_prop_changed(struct Connman_Manager *cm,
                                             const char *name,
                                             Eldbus_Message_Iter *value)
{
   if (!strcmp(name, "Powered"))
     return eldbus_message_iter_arguments_get(value, "b", &cm->powered);
   else
     return false;
}

static void _manager_wifi_prop_changed(void *data, const Eldbus_Message *msg)
{
   struct Connman_Manager *cm = data;
   Eldbus_Message_Iter *var;
   const char *name;

   if (!eldbus_message_arguments_get(msg, "sv", &name, &var))
     {
        ERR("Could not parse message %p", msg);
        return;
     }

   _manager_parse_wifi_prop_changed(cm, name, var);
}

static void _manager_get_wifi_prop_cb(void *data, const Eldbus_Message *msg,
                                      Eldbus_Pending *pending EINA_UNUSED)
{
   struct Connman_Manager *cm = data;
   Eldbus_Message_Iter *array, *dict;
   const char *name, *message;

   cm->pending.get_wifi_properties = NULL;

   if (eldbus_message_error_get(msg, &name, &message))
     {
        WRN("Could not get properties. %s: %s", name, message);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a{sv}", &array))
     {
        WRN("Error getting arguments.");
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 'e', &dict))
     {
        Eldbus_Message_Iter *var;
        const char *key;

         if (!eldbus_message_iter_arguments_get(dict, "sv", &key, &var))
           continue;
        _manager_parse_wifi_prop_changed(cm, key, var);
     }
}

static void
_manager_agent_unregister(struct Connman_Manager *cm)
{
   eldbus_proxy_call(cm->manager_iface, "UnregisterAgent", NULL, NULL, -1, "o",
                    AGENT_PATH);
}

static void
_manager_agent_register_cb(void *data EINA_UNUSED, const Eldbus_Message *msg,
                           Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;

   if (eldbus_message_error_get(msg, &name, &text))
     {
        WRN("Could not register agent. %s: %s", name, text);
        return;
     }

   INF("Agent registered");
}

static void
_manager_agent_register(struct Connman_Manager *cm)
{
   if (!cm)
     return;

   eldbus_proxy_call(cm->manager_iface, "RegisterAgent",
                    _manager_agent_register_cb, NULL, -1, "o", AGENT_PATH);
}

static void _manager_free(struct Connman_Manager *cm)
{
   Eldbus_Object *obj;
   if (!cm)
     return;

   while (cm->services)
     {
        struct Connman_Service *cs = EINA_INLIST_CONTAINER_GET(cm->services,
                                                      struct Connman_Service);
        cm->services = eina_inlist_remove(cm->services, cm->services);
        _service_free(cs);
     }

   if (cm->pending.get_services)
     eldbus_pending_cancel(cm->pending.get_services);

   if (cm->pending.get_wifi_properties)
     eldbus_pending_cancel(cm->pending.get_wifi_properties);

   if (cm->pending.set_powered)
     eldbus_pending_cancel(cm->pending.set_powered);

   eina_stringshare_del(cm->path);
   obj = eldbus_proxy_object_get(cm->manager_iface);
   eldbus_proxy_unref(cm->manager_iface);
   eldbus_object_unref(obj);
   obj = eldbus_proxy_object_get(cm->technology_iface);
   eldbus_proxy_unref(cm->technology_iface);
   eldbus_object_unref(obj);
   free(cm);
}

static void _manager_powered_cb(void *data, const Eldbus_Message *msg,
                                Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Pending *p;
   struct Connman_Manager *cm = data;
   const char *error_name, *error_msg;

   cm->pending.set_powered = NULL;

   if (eldbus_message_error_get(msg, &error_name, &error_msg))
     {
        ERR("Error: %s %s", error_name, error_msg);
        return;
     }
   if (cm->pending.get_wifi_properties)
     eldbus_pending_cancel(cm->pending.get_wifi_properties);
   p = eldbus_proxy_call(cm->technology_iface, "GetProperties",
                        _manager_get_wifi_prop_cb, cm, -1, "");
   cm->pending.get_wifi_properties = p;
}

void econnman_powered_set(struct Connman_Manager *cm, Eina_Bool powered)
{
   Eldbus_Message_Iter *main_iter, *var;
   Eldbus_Message *msg;

   if (cm->pending.set_powered)
     eldbus_pending_cancel(cm->pending.set_powered);

   msg = eldbus_proxy_method_call_new(cm->technology_iface, "SetProperty");
   main_iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_basic_append(main_iter, 's', "Powered");
   var = eldbus_message_iter_container_new(main_iter, 'v', "b");
   eldbus_message_iter_basic_append(var, 'b', powered);
   eldbus_message_iter_container_close(main_iter, var);

   cm->pending.set_powered = eldbus_proxy_send(cm->technology_iface, msg,
                                              _manager_powered_cb, cm, -1);
}

static struct Connman_Manager *_manager_new(void)
{
   const char *path = "/";
   struct Connman_Manager *cm;
   Eldbus_Object *obj;

   cm = calloc(1, sizeof(*cm));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cm, NULL);


   obj = eldbus_object_get(conn, CONNMAN_BUS_NAME, "/");
   cm->manager_iface = eldbus_proxy_get(obj, CONNMAN_MANAGER_IFACE);
   obj = eldbus_object_get(conn, CONNMAN_BUS_NAME,
                          "/net/connman/technology/wifi");
   cm->technology_iface = eldbus_proxy_get(obj, CONNMAN_TECHNOLOGY_IFACE);

   cm->path = eina_stringshare_add(path);

   eldbus_proxy_signal_handler_add(cm->manager_iface, "PropertyChanged",
                                  _manager_prop_changed, cm);
   eldbus_proxy_signal_handler_add(cm->manager_iface, "ServicesChanged",
                                  _manager_services_changed, cm);
   eldbus_proxy_signal_handler_add(cm->technology_iface, "PropertyChanged",
                                  _manager_wifi_prop_changed, cm);

   /*
    * PropertyChanged signal in service's path is guaranteed to arrive only
    * after ServicesChanged above. So we only add the handler later, in a per
    * service manner.
    */
   cm->pending.get_services = eldbus_proxy_call(cm->manager_iface,
                                               "GetServices",
                                               _manager_get_services_cb, cm,
                                               -1, "");
   eldbus_proxy_call(cm->manager_iface, "GetProperties", _manager_get_prop_cb, cm,
                    -1, "");
   cm->pending.get_wifi_properties = eldbus_proxy_call(cm->technology_iface,
                                                      "GetProperties",
                                                      _manager_get_wifi_prop_cb,
                                                      cm, -1, "");

   return cm;
}

static inline void _e_connman_system_name_owner_exit(Eina_Bool shutdown)
{
   if (!connman_manager)
     return;
   _manager_agent_unregister(connman_manager);
   econnman_mod_manager_inout(NULL);
   _manager_free(connman_manager);
   connman_manager = NULL;

   ecore_event_add(E_CONNMAN_EVENT_MANAGER_OUT, NULL, NULL, NULL);
   if (!shutdown)
     e_util_dialog_show(_("Connman Service Missing"),
                        _("The Connman service is not able to be found.<br>"
                          "Is <b>connmand</b> daemon running?"));
}

static inline void _e_connman_system_name_owner_enter(const char *owner EINA_UNUSED)
{
   connman_manager = _manager_new();
   _manager_agent_register(connman_manager);
   ecore_event_add(E_CONNMAN_EVENT_MANAGER_IN, NULL, NULL, NULL);
   econnman_mod_manager_inout(connman_manager);
}

static void
_e_connman_system_name_owner_changed(void *data EINA_UNUSED,
                                     const char *bus EINA_UNUSED,
                                     const char *from EINA_UNUSED,
                                     const char *to)
{
   if (to[0])
     _e_connman_system_name_owner_enter(to);
   else
     _e_connman_system_name_owner_exit(EINA_FALSE);
}

/**
 * Initialize E Connection Manager (E_Connman) system.
 *
 * This will connect to ConnMan through DBus and watch for it going in and out.
 *
 * Interesting events are:
 *   - E_CONNMAN_EVENT_MANAGER_IN: issued when connman is available.
 *   - E_CONNMAN_EVENT_MANAGER_OUT: issued when connman connection is lost.
 */
unsigned int
e_connman_system_init(Eldbus_Connection *eldbus_conn)
{
   init_count++;

   if (init_count > 1)
      return init_count;

   E_CONNMAN_EVENT_MANAGER_IN = ecore_event_type_new();
   E_CONNMAN_EVENT_MANAGER_OUT = ecore_event_type_new();

   conn = eldbus_conn;
   eldbus_name_owner_changed_callback_add(conn, CONNMAN_BUS_NAME,
                                         _e_connman_system_name_owner_changed,
                                         NULL, EINA_TRUE);
   agent = econnman_agent_new(eldbus_conn);

   return init_count;
}

/**
 * Shutdown ConnMan system
 *
 * When count drops to 0 resources will be released and no calls should be
 * made anymore.
 */
unsigned int
e_connman_system_shutdown(void)
{
   if (init_count == 0)
     {
        ERR("connman system already shut down.");
        return 0;
     }

   init_count--;
   if (init_count > 0)
      return init_count;

   eldbus_name_owner_changed_callback_del(conn, CONNMAN_BUS_NAME,
                                         _e_connman_system_name_owner_changed,
                                         NULL);
   _e_connman_system_name_owner_exit(EINA_TRUE);
   if (agent)
     econnman_agent_del(agent);
   if (conn)
     eldbus_connection_unref(conn);
   agent = NULL;
   conn = NULL;

   E_CONNMAN_EVENT_MANAGER_OUT = 0;
   E_CONNMAN_EVENT_MANAGER_IN = 0;

   return init_count;
}

void
e_connman_scan(struct Connman_Manager *cm)
{
   eldbus_proxy_call(cm->technology_iface, "Scan", NULL, NULL, -1, "");
}
