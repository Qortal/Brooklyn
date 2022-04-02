#include "bz.h"
#include "e_mod_main.h"

static Eldbus_Object *objman_obj = NULL;
static Eldbus_Proxy *objman_proxy = NULL;
static Eldbus_Signal_Handler *sig_ifadd = NULL;
static Eldbus_Signal_Handler *sig_ifdel = NULL;
static Eldbus_Pending *pend_getobj = NULL;
static Eina_Hash *obj_table = NULL;
static void (*fn_obj_add) (Obj *o) = NULL;

/*
static void
cb_obj_prop_mandata(void *data, const void *key, Eldbus_Message_Iter *var)
{
   Obj *o = data;
   unsigned short *skey = key;

   printf("    M KEY %x\n", (int)*skey);
}
*/

static void
cb_obj_prop_entry(void *data, const void *key, Eldbus_Message_Iter *var)
{
   Obj *o = data;
   const char *skey = key;

   if (!strcmp(skey, "Paired"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->paired = val;
     }
   else if (!strcmp(skey, "Connected"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->connected = val;
     }
   else if (!strcmp(skey, "Trusted"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->trusted = val;
     }
   else if (!strcmp(skey, "Blocked"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->blocked = val;
     }
   else if (!strcmp(skey, "LegacyPairing"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->legacy_pairing = val;
     }
   else if (!strcmp(skey, "ServicesResolved"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->services_resolved = val;
     }
   else if (!strcmp(skey, "Address"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "s", &val))
          o->address = eina_stringshare_add(val);
     }
   else if (!strcmp(skey, "AddressType"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "s", &val))
          o->address_type = eina_stringshare_add(val);
     }
   else if (!strcmp(skey, "Name"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "s", &val))
          o->name = eina_stringshare_add(val);
     }
   else if (!strcmp(skey, "Icon"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "s", &val))
          o->icon = eina_stringshare_add(val);
     }
   else if (!strcmp(skey, "Alias"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "s", &val))
          o->alias = eina_stringshare_add(val);
     }
   else if (!strcmp(skey, "Modalias"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "s", &val))
          o->modalias = eina_stringshare_add(val);
     }
   else if (!strcmp(skey, "Adapter"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "o", &val))
          o->adapter = eina_stringshare_add(val);
     }
   else if (!strcmp(skey, "Class"))
     {
        unsigned int val = 0;
        if (eldbus_message_iter_arguments_get(var, "u", &val))
          o->klass = val;
     }
   else if (!strcmp(skey, "Appearance"))
     {
        unsigned short val = 0;
        if (eldbus_message_iter_arguments_get(var, "q", &val))
          o->appearance = val;
     }
   else if (!strcmp(skey, "RSSI"))
     {
        short val = 0;
        if (eldbus_message_iter_arguments_get(var, "n", &val))
          o->rssi = val;
     }
   else if (!strcmp(skey, "TxPower"))
     {
        unsigned short val = 0;
        if (eldbus_message_iter_arguments_get(var, "n", &val))
          o->txpower = val;
     }
   else if (!strcmp(skey, "UUIDs"))
     {
        Eldbus_Message_Iter *array = NULL;

        if (eldbus_message_iter_arguments_get(var, "as", &array))
          {
             const char *val = NULL;

             while (eldbus_message_iter_get_and_next(array, 's', &val))
               {
                  if (!o->uuids) o->uuids = eina_array_new(1);
                  eina_array_push(o->uuids, eina_stringshare_add(val));
               }
          }
     }
   else if (!strcmp(skey, "Discoverable"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->discoverable = val;
     }
   else if (!strcmp(skey, "Discovering"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->discovering = val;
     }
   else if (!strcmp(skey, "Pairable"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->pairable = val;
     }
   else if (!strcmp(skey, "Powered"))
     {
        Eina_Bool val = EINA_FALSE;
        if (eldbus_message_iter_arguments_get(var, "b", &val))
          o->powered = val;
     }
   else if (!strcmp(skey, "DiscoverableTimeout"))
     {
        unsigned int val = 0;
        if (eldbus_message_iter_arguments_get(var, "u", &val))
          o->discoverable_timeout = val;
     }
   else if (!strcmp(skey, "PairableTimeout"))
     {
        unsigned int val = 0;
        if (eldbus_message_iter_arguments_get(var, "u", &val))
          o->pairable_timeout = val;
     }
   // dict ManufacturerData [readonly, optional]
   //  Manufacturer specific advertisement data. Keys are
   //  16 bits Manufacturer ID followed by its byte array
   //  value.
/*
   if (!strcmp(skey, "ManufacturerData"))
     {
        Eldbus_Message_Iter *array = NULL;

        if (eldbus_message_iter_arguments_get(var, "a{qv}", &array))
          eldbus_message_iter_dict_iterate(array, "qv", cb_obj_prop_mandata, o);
     }
 */
   // dict ServiceData [readonly, optional]
   //  Service advertisement data. Keys are the UUIDs in
   //  string format followed by its byte array value.
   //
   // array{byte} AdvertisingFlags [readonly, experimental]
   //  The Advertising Data Flags of the remote device.
   //
   // dict AdvertisingData [readonly, experimental]
   //  The Advertising Data of the remote device. Keys are
   //  are 8 bits AD Type followed by data as byte array.
}

static void
_obj_clear(Obj *o)
{
   o->paired = EINA_FALSE;
   o->connected = EINA_FALSE;
   o->trusted = EINA_FALSE;
   o->blocked = EINA_FALSE;
   o->legacy_pairing = EINA_FALSE;
   o->services_resolved = EINA_FALSE;
   eina_stringshare_del(o->address);
   o->address = NULL;
   eina_stringshare_del(o->address_type);
   o->address_type = NULL;
   eina_stringshare_del(o->name);
   o->name = NULL;
   eina_stringshare_del(o->icon);
   o->icon = NULL;
   eina_stringshare_del(o->alias);
   o->alias = NULL;
   eina_stringshare_del(o->adapter);
   o->adapter = NULL;
   eina_stringshare_del(o->modalias);
   o->modalias = NULL;
   eina_stringshare_del(o->modalias);
   o->modalias = NULL;
   o->klass = 0;
   o->appearance = 0;
   o->txpower = 0;
   o->rssi = 0;
   if (o->uuids)
     {
        const char *val;

        while ((val = eina_array_pop(o->uuids)))
          eina_stringshare_del(val);
        eina_array_free(o->uuids);
        o->uuids = NULL;
     }
}

#define ERR_PRINT(str) \
   do { const char *name, *text; \
      if (eldbus_message_error_get(msg, &name, &text)) { \
         printf("Error: %s.\n %s:\n %s\n", str, name, text); \
         return; \
      } \
   } while(0)


static void
cb_obj_prop(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Obj *o = data;
   Eldbus_Message_Iter *array;

   if (eldbus_message_error_get(msg, NULL, NULL)) return;
   _obj_clear(o);
   if (eldbus_message_arguments_get(msg, "a{sv}", &array))
     eldbus_message_iter_dict_iterate(array, "sv", cb_obj_prop_entry, o);
   bz_obj_ref(o);
   if ((o->powered) && (o->path))
     {
        const char *s = strrchr(o->path, '/');

        if (s) ebluez5_rfkill_unblock(s + 1);
     }
   if (!o->add_called)
     {
        o->add_called = EINA_TRUE;
        if (fn_obj_add) fn_obj_add(o);
     }
   if (o->fn_change) o->fn_change(o);
   bz_obj_unref(o);
}

static void
cb_obj_prop_changed(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED)
{
   Obj *o = data;
   if (!o->proxy) return;
   eldbus_proxy_property_get_all(o->proxy, cb_obj_prop, o);
}

static void
cb_obj_prop_bat_entry(void *data, const void *key, Eldbus_Message_Iter *var)
{
   Obj *o = data;
   const char *skey = key;

   if (!strcmp(skey, "Percentage"))
     {
        char val;
        if (eldbus_message_iter_arguments_get(var, "y", &val))
          o->bat_percent = val;
     }
}

static void
cb_obj_prop_bat(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Obj *o = data;
   Eldbus_Message_Iter *array;

   if (eldbus_message_error_get(msg, NULL, NULL)) return;
   if (eldbus_message_arguments_get(msg, "a{sv}", &array))
     eldbus_message_iter_dict_iterate(array, "sv", cb_obj_prop_bat_entry, o);
   bz_obj_ref(o);
   if (o->fn_change) o->fn_change(o);
   bz_obj_unref(o);
}

static void
cb_obj_prop_bat_changed(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED)
{
   Obj *o = data;
   if (!o->proxy_bat) return;
   eldbus_proxy_property_get_all(o->proxy_bat, cb_obj_prop_bat, o);
}

//static void
//cb_obj_discovery_filter(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
//{
//   ERR_PRINT("Discovery Filter Clear");
//}

Obj *
bz_obj_add(const char *path)
{
   Obj *o = calloc(1, sizeof(Obj));
   o->ref = 1;
   o->path = eina_stringshare_add(path);
   o->obj = eldbus_object_get(bz_conn, "org.bluez", o->path);
   o->type = BZ_OBJ_UNKNOWN;
   o->in_table = EINA_TRUE;
   o->bat_percent = -1;
   eina_hash_add(obj_table, o->path, o);
   if (!strcmp(o->path, "/org/bluez"))
     {
        o->proxy = eldbus_proxy_get(o->obj, "org.bluez.AgentManager1");
        o->type = BZ_OBJ_BLUEZ;
        o->add_called = EINA_TRUE;
        bz_obj_ref(o);
        if (fn_obj_add) fn_obj_add(o);
        bz_obj_unref(o);
        goto done;
     }
   // all devices are /org/bluez/XXX/dev_XXX so look for /dev_
   else if (strstr(o->path, "/dev_"))
     {
        o->type = BZ_OBJ_DEVICE;
        // device properties
        o->proxy = eldbus_proxy_get(o->obj, "org.bluez.Device1");
        if (o->proxy)
          {
             eldbus_proxy_property_get_all(o->proxy, cb_obj_prop, o);
             o->prop_proxy = eldbus_proxy_get(o->obj,
                                              "org.freedesktop.DBus.Properties");
             if (o->prop_proxy)
               o->prop_sig = eldbus_proxy_signal_handler_add(o->prop_proxy,
                                                             "PropertiesChanged",
                                                             cb_obj_prop_changed, o);
          }
        // battery properties - if it supports it
        o->proxy_bat = eldbus_proxy_get(o->obj, "org.bluez.Battery1");
        if (o->proxy_bat)
          {
             eldbus_proxy_property_get_all(o->proxy_bat, cb_obj_prop_bat, o);
             o->prop_proxy_bat = eldbus_proxy_get(o->obj,
                                              "org.freedesktop.DBus.Properties");
             if (o->prop_proxy_bat)
               o->prop_sig_bat = eldbus_proxy_signal_handler_add(o->prop_proxy_bat,
                                                                 "PropertiesChanged",
                                                                 cb_obj_prop_bat_changed, o);
          }
        goto done;
     }
   // all dadapters begin with /org/bluez/
   else if (!strncmp(o->path, "/org/bluez/", 11))
     {
        o->proxy = eldbus_proxy_get(o->obj, "org.bluez.Adapter1");
        o->type = BZ_OBJ_ADAPTER;
        if (o->proxy)
          {
             eldbus_proxy_property_get_all(o->proxy, cb_obj_prop, o);
             o->prop_proxy = eldbus_proxy_get(o->obj,
                                              "org.freedesktop.DBus.Properties");
             if (o->prop_proxy)
               o->prop_sig = eldbus_proxy_signal_handler_add(o->prop_proxy,
                                                             "PropertiesChanged",
                                                             cb_obj_prop_changed, o);
             // disable the filter for discovery later
             // XXX: this doesn't seem to exist on the bluez daemons I see
             // so don't do this to avoid error noise and it's useless it seems
             // eldbus_proxy_call
             //  (o->proxy, "SetDiscoveryFilter", cb_obj_discovery_filter, o, -1, "");
          }
        goto done;
     }
done:
   return o;
}

Obj *
bz_obj_find(const char *path)
{
   return eina_hash_find(obj_table, path);
}

static Eina_Bool
_cb_power_again(void *data)
{
   Obj *o = data;

   o->power_retry_timer = NULL;
   printf("Retry power on...");
   bz_obj_power_on(o);
   return EINA_FALSE;
}

static void
cb_power_on(void *data, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   Obj *o = data;
   const char *name, *text;

   if (eldbus_message_error_get(msg, &name, &text))
     {
        printf("Error: %s.\n %s:\n %s\n", "Power On", name, text);
        if (!strcmp(name, "org.bluez.Error.Busy"))
          {
             o->power_retry_timer = ecore_timer_add(0.5, _cb_power_again, o);
          }
     }
}

void
bz_obj_power_on(Obj *o)
{
   Eina_Bool val = EINA_TRUE;
   if (!o->proxy) return;
   eldbus_proxy_property_set
     (o->proxy, "Powered", "b", (void *)(uintptr_t)val, cb_power_on, o);
}

static void
cb_power_off(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Power Off");
}

void
bz_obj_power_off(Obj *o)
{
   Eina_Bool val = EINA_FALSE;
   if (!o->proxy) return;
   if (o->power_retry_timer)
     {
        ecore_timer_del(o->power_retry_timer);
        o->power_retry_timer = NULL;
     }
   eldbus_proxy_property_set
     (o->proxy, "Powered", "b", (void *)(uintptr_t)val, cb_power_off, o);
}

static void
cb_discoverable(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Discoverable");
}

void
bz_obj_discoverable(Obj *o)
{
   Eina_Bool val = EINA_TRUE;
   if (!o->proxy) return;
   eldbus_proxy_property_set
     (o->proxy, "Discoverable", "b", (void *)(uintptr_t)val, cb_discoverable, o);
}

static void
cb_undiscoverable(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Undiscoverable");
}

void
bz_obj_undiscoverable(Obj *o)
{
   Eina_Bool val = EINA_FALSE;
   if (!o->proxy) return;
   eldbus_proxy_property_set
     (o->proxy, "Discoverable", "b", (void *)(uintptr_t)val, cb_undiscoverable, o);
}

static void
cb_pairable(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Pairable");
}

void
bz_obj_pairable(Obj *o)
{
   Eina_Bool val = EINA_TRUE;
   if (!o->proxy) return;
   eldbus_proxy_property_set
     (o->proxy, "Pairable", "b", (void *)(uintptr_t)val, cb_pairable, o);
}

static void
cb_unpairable(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Unpairable");
}

void
bz_obj_unpairable(Obj *o)
{
   Eina_Bool val = EINA_FALSE;
   if (!o->proxy) return;
   eldbus_proxy_property_set
     (o->proxy, "Pairable", "b", (void *)(uintptr_t)val, cb_unpairable, o);
}

static void
cb_trust(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Trust");
}

void
bz_obj_trust(Obj *o)
{
   Eina_Bool val = EINA_TRUE;
   if (!o->proxy) return;
   eldbus_proxy_property_set
     (o->proxy, "Trusted", "b", (void *)(uintptr_t)val, cb_trust, o);
}

static void
cb_distrust(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Distrust");
}

void
bz_obj_distrust(Obj *o)
{
   Eina_Bool val = EINA_FALSE;
   if (!o->proxy) return;
   eldbus_proxy_property_set
     (o->proxy, "Trusted", "b", (void *)(uintptr_t)val, cb_distrust, o);
}

static void
cb_pair(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Pair");
}

void
bz_obj_pair(Obj *o)
{
   if (!o->proxy) return;
   eldbus_proxy_call(o->proxy, "Pair", cb_pair, o, -1, "");
}

static void
cb_pair_cancel(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Pair Cancel");
}

void
bz_obj_pair_cancel(Obj *o)
{
   if (!o->proxy) return;
   eldbus_proxy_call
     (o->proxy, "CancelPairing", cb_pair_cancel, o, -1, "");
}

static void
cb_connect(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Connect");
}

void
bz_obj_connect(Obj *o)
{
   if (!o->proxy) return;
   eldbus_proxy_call
     (o->proxy, "Connect", cb_connect, o, -1, "");
}

static void
cb_disconnect(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Disconnect");
}

void
bz_obj_disconnect(Obj *o)
{
   if (!o->proxy) return;
   eldbus_proxy_call
     (o->proxy, "Disconnect", cb_disconnect, o, -1, "");
}

static void
_cb_l2ping(void *data, const char *params)
{
   Obj *o = data;
   char addr[256];
   int timeout = 0;

   if (sscanf(params, "%255s %i", addr, &timeout) == 2)
     {
        if (!strcmp(o->address, addr))
          {
             if (o->ping_busy)
               {
                  e_system_handler_del("l2ping-ping", _cb_l2ping, o);
                  o->ping_busy = EINA_FALSE;
               }
             if (timeout >= 0)
               {
                  if (!o->ping_ok)
                    {
                       printf("@@@PING SUCCEED\n");
                       o->ping_ok = EINA_TRUE;
                       if (o->fn_change) o->fn_change(o);
                    }
               }
             else
               {
                  if (o->ping_ok)
                    {
                       printf("@@@PING FAIL\n");
                       o->ping_ok = EINA_FALSE;
                       if (o->fn_change) o->fn_change(o);
                    }
               }
          }
     }
}

static int
ping_powersave_timeout_get(void)
{
   int timeout = 10;
   E_Powersave_Mode pm = e_powersave_mode_get();

   if      (pm <= E_POWERSAVE_MODE_LOW)     timeout = 5;
   else if (pm <= E_POWERSAVE_MODE_MEDIUM)  timeout = 8;
   else if (pm <= E_POWERSAVE_MODE_HIGH)    timeout = 12;
   else if (pm <= E_POWERSAVE_MODE_EXTREME) timeout = 30;
   return timeout;
}

static void
ping_do(Obj *o)
{
   int timeout = 1000 * ping_powersave_timeout_get();
   if (o->ping_busy)
     e_system_handler_del("l2ping-ping", _cb_l2ping, o);
   o->ping_busy = EINA_TRUE;
   e_system_handler_add("l2ping-ping", _cb_l2ping, o);
   e_system_send("l2ping-ping", "%s %i", o->address, timeout);
   printf("@@@ run new ping %s %i\n", o->address, timeout);
}

static Eina_Bool cb_ping_timer(void *data);

static void
ping_schedule(Obj *o)
{
   double timeout = ping_powersave_timeout_get() + 1.0;

   if (o->ping_timer) ecore_timer_del(o->ping_timer);
   printf("@@@ new ping in %1.3f\n", timeout);
   o->ping_timer = ecore_timer_add(timeout, cb_ping_timer, o);
}

static Eina_Bool
cb_ping_timer(void *data)
{
   Obj *o = data;

   printf("@@@ ping timer %s\n", o->address);
   o->ping_timer = NULL;
   o->ping_block = EINA_TRUE;
   if (o->ping_busy)
     {
        o->ping_busy = EINA_FALSE;
        e_system_handler_del("l2ping-ping", _cb_l2ping, o);
        if (o->ping_ok)
          {
             o->ping_ok = EINA_FALSE;
             if (o->fn_change) o->fn_change(o);
          }
     }
   o->ping_block = EINA_FALSE;
   ping_do(o);
   ping_schedule(o);
   return EINA_FALSE;
}

void
bz_obj_ping_begin(Obj *o)
{
   if ((o->ping_timer) || (o->ping_block)) return;
   if (o->ping_busy)
     {
        o->ping_busy = EINA_FALSE;
        e_system_handler_del("l2ping-ping", _cb_l2ping, o);
     }
   ping_do(o);
   ping_schedule(o);
 }

void
bz_obj_ping_end(Obj *o)
{
   if (o->ping_timer)
     {
        ecore_timer_del(o->ping_timer);
        o->ping_timer = NULL;
     }
   if (o->ping_busy)
     {
        o->ping_busy = EINA_FALSE;
        e_system_handler_del("l2ping-ping", _cb_l2ping, o);
     }
   if (o->ping_ok)
     {
        printf("@@@PING END %s\n", o->address);
        o->ping_ok = EINA_FALSE;
        if (o->fn_change) o->fn_change(o);
     }
}

static void
cb_remove(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Remove");
}

/*
void
bz_obj_profile_connect(Obj *o, const char *uuid)
{
   if (!o->proxy) return;
   eldbus_proxy_call(o->proxy, "ConnectProfile", NULL, NULL, -1, "s", uuid);
}

void
bz_obj_profile_disconnect(Obj *o, const char *uuid)
{
   if (!o->proxy) return;
   eldbus_proxy_call(o->proxy, "DisconnectProfile", NULL, NULL, -1, "s", uuid);
}
*/

void
bz_obj_remove(Obj *o)
{
   if (o->adapter)
     {
        Obj *adapter = bz_obj_find(o->adapter);
        if (adapter)
          {
             if (!adapter->proxy) return;
             eldbus_proxy_call(adapter->proxy, "RemoveDevice",
                               cb_remove, adapter, -1,
                               "o", o->path);
          }
     }
}

void
bz_obj_ref(Obj *o)
{
   o->ref++;
}

void
bz_obj_unref(Obj *o)
{
   o->ref--;
   if (o->ref > 0) return;
   if (o->in_table)
     {
        o->in_table = EINA_FALSE;
        eina_hash_del(obj_table, o->path, o);
     }
   if (o->ping_busy)
     {
        o->ping_busy = EINA_FALSE;
        e_system_handler_del("l2ping-ping", _cb_l2ping, o);
     }
   _obj_clear(o);
   if (o->prop_sig)
     {
        eldbus_signal_handler_del(o->prop_sig);
        o->prop_sig = NULL;
     }
   if (o->path)
     {
        eina_stringshare_del(o->path);
        o->path = NULL;
     }
   if (o->agent_request)
     {
        eina_stringshare_del(o->agent_request);
        o->agent_request = NULL;
     }
   if (o->agent_msg_err)
     {
        bz_agent_msg_drop(o->agent_msg_err);
        o->agent_msg_err = NULL;
     }
   if (o->agent_msg_ok)
     {
        bz_agent_msg_drop(o->agent_msg_ok);
        o->agent_msg_ok = NULL;
     }
   if (o->ping_timer)
     {
        ecore_timer_del(o->ping_timer);
        o->ping_timer = NULL;
     }
   if (o->power_retry_timer)
     {
        ecore_timer_del(o->power_retry_timer);
        o->power_retry_timer = NULL;
     }
   if (o->proxy)
     {
        eldbus_proxy_unref(o->proxy);
        o->proxy = NULL;
     }
   if (o->prop_proxy)
     {
        eldbus_proxy_unref(o->prop_proxy);
        o->prop_proxy = NULL;
     }
   if (o->proxy_bat)
     {
        eldbus_proxy_unref(o->proxy_bat);
        o->proxy_bat = NULL;
     }
   if (o->prop_proxy_bat)
     {
        eldbus_proxy_unref(o->prop_proxy_bat);
        o->prop_proxy_bat = NULL;
     }
   if (o->obj)
     {
        eldbus_object_unref(o->obj);
        o->obj = NULL;
     }
   free(o);
}

static void
cb_discovery_start(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Discovery Start");
}

void
bz_obj_discover_start(Obj *o)
{
   if (!o->proxy) return;
   eldbus_proxy_call
     (o->proxy, "StartDiscovery", cb_discovery_start, o, -1, "");
}

static void
cb_discovery_stop(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   ERR_PRINT("Discovery Stop");
}

void
bz_obj_discover_stop(Obj *o)
{
   if (!o->proxy) return;
   eldbus_proxy_call
     (o->proxy, "StopDiscovery", cb_discovery_stop, o, -1, "");
}

void
bz_obj_agent_request(Obj *o, const char *req, void (*fn) (Eldbus_Message *msg, const char *str), Eldbus_Message *msg_ok, Eldbus_Message *msg_err)
{
   if (o->agent_msg_ok) bz_agent_msg_drop(o->agent_msg_ok);
   if (o->agent_msg_err) bz_agent_msg_reply(o->agent_msg_err);
   o->agent_msg_ok = msg_ok;
   o->agent_msg_err = msg_err;
   o->agent_entry_fn = fn;
   o->agent_alert = EINA_TRUE;
   eina_stringshare_replace(&(o->agent_request), req);
   bz_obj_ref(o);
   if (o->fn_change) o->fn_change(o);
   bz_obj_unref(o);
}

static void
cb_obj_add(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *path = NULL;

   if (!eldbus_message_arguments_get(msg, "o", &path)) return;
   if (bz_obj_find(path)) return;
   bz_obj_add(path);
}

static void
cb_obj_del(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   Obj *o;
   const char *path = NULL;

   if (!eldbus_message_arguments_get(msg, "o", &path)) return;
   o = bz_obj_find(path);
   if (o)
     {
        bz_obj_ref(o);
        if (o->fn_del) o->fn_del(o);
        bz_obj_unref(o);
        bz_obj_unref(o);
     }
}

static void
cb_getobj(void *data EINA_UNUSED, const Eldbus_Message *msg,
          Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *prop, *dict;

   pend_getobj = NULL;
   ERR_PRINT("Get Objects");
   if (eldbus_message_arguments_get(msg, "a{oa{sa{sv}}}", &prop))
     {
        while (eldbus_message_iter_get_and_next(prop, 'e', &dict))
          {
             const char *path;

             path = NULL;
             if (!eldbus_message_iter_arguments_get(dict, "o", &path))
               {
                  return;
               }
             bz_obj_add(path);
          }
     }
}

static void
_obj_hash_free(Obj *o)
{
   o->in_table = 0;
   bz_obj_unref(o);
}

void
bz_obj_init(void)
{
   obj_table = eina_hash_string_superfast_new((void *)_obj_hash_free);

   objman_obj = eldbus_object_get(bz_conn, "org.bluez", "/");
   objman_proxy = eldbus_proxy_get(objman_obj, "org.freedesktop.DBus.ObjectManager");
   sig_ifadd = eldbus_proxy_signal_handler_add(objman_proxy, "InterfacesAdded",
                                               cb_obj_add, NULL);
   sig_ifdel = eldbus_proxy_signal_handler_add(objman_proxy, "InterfacesRemoved",
                                               cb_obj_del, NULL);
   pend_getobj = eldbus_proxy_call(objman_proxy, "GetManagedObjects",
                                   cb_getobj, NULL, -1, "");
}

void
bz_obj_shutdown(void)
{
   eina_hash_free(obj_table);
   obj_table = NULL;
   if (pend_getobj)
     {
        eldbus_pending_cancel(pend_getobj);
        pend_getobj = NULL;
     }
   if (sig_ifadd)
     {
        eldbus_signal_handler_del(sig_ifadd);
        sig_ifadd = NULL;
     }
   if (sig_ifdel)
     {
        eldbus_signal_handler_del(sig_ifdel);
        sig_ifdel = NULL;
     }
   if (objman_proxy)
     {
        eldbus_proxy_unref(objman_proxy);
        objman_proxy = NULL;
     }
   if (objman_obj)
     {
        eldbus_object_unref(objman_obj);
        objman_obj = NULL;
     }
}

void
bz_obj_add_func_set(void (*fn) (Obj *o))
{
   fn_obj_add = fn;
}
