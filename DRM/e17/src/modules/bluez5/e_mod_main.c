#include "e_mod_main.h"

static Eina_List *instances = NULL;
static E_Module *mod = NULL;

/* Local config */
static E_Config_DD *conf_adapter_edd = NULL;
static E_Config_DD *conf_device_edd = NULL;
static E_Config_DD *conf_edd = NULL;
Config *ebluez5_config = NULL;

static Ecore_Timer *zero_adapters_check_timer = NULL;

E_API E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Bluez5"};

static void
_mod_icon_set(Evas_Object *base, Eina_Bool gadget)
{
   char edj_path[4096], *group;

   // XXX: hack for now until we make the icon do things and have it
   // in theme in efl
   snprintf(edj_path, sizeof(edj_path), "%s/e-module-bluez5.edj", mod->dir);
   if (1) group = "e/modules/bluez5/main";
   else group = "e/modules/bluez5/inactive";

   if (!e_theme_edje_object_set(base, "base/theme/modules/bluez5", group))
     {
        if (gadget)
          elm_layout_file_set(base, edj_path, group);
        else
          edje_object_file_set(base, edj_path, group);
     }
}

/////////////////////////////////////////////////////////////////////////////

static void
_popup_del(Instance *inst)
{
   E_FREE_FUNC(inst->popup, e_object_del);
}

static void
_popup_del_cb(void *obj)
{
   _popup_del(e_object_data_get(obj));
}

static void
_popup_comp_del_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   _popup_del(data);
}

static void
_popup_new(Instance *inst)
{
   inst->popup = e_gadcon_popup_new(inst->gcc, 0);

   e_gadcon_popup_content_set(inst->popup, ebluez5_popup_content_add(e_comp->elm, inst));
   e_comp_object_util_autoclose(inst->popup->comp_object, _popup_comp_del_cb, NULL, inst);
   e_gadcon_popup_show(inst->popup);
   e_object_data_set(E_OBJECT(inst->popup), inst);
   E_OBJECT_DEL_SET(inst->popup, _popup_del_cb);
}

static void
_ebluez5_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Instance *inst = data;
   Evas_Event_Mouse_Down *ev = event;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (ev->button != 1) return;
   if (!inst->popup) _popup_new(inst);
   else _popup_del(inst);
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   Instance *inst = E_NEW(Instance, 1);

   if (!inst) return NULL;
   inst->o_bluez5 = o = edje_object_add(gc->evas);
   _mod_icon_set(o, EINA_FALSE);
   inst->gcc = e_gadcon_client_new(gc, name, id, style, o);
   inst->gcc->data = inst;
   e_gadcon_client_util_menu_attach(inst->gcc);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _ebluez5_cb_mouse_down, inst);
   instances = eina_list_append(instances, inst);
   ebluez5_instances_update();
   return inst->gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst = gcc->data;;

   if (!inst) return;
   instances = eina_list_remove(instances, inst);
   _popup_del(inst);
   E_FREE_FUNC(inst->o_bluez5, evas_object_del);
   E_FREE(inst);
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   static char tmpbuf[128];
   snprintf(tmpbuf, sizeof(tmpbuf), "bluez5.%d", eina_list_count(instances));
   return tmpbuf;
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
   return _("Bluez5");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o = NULL;
   char buf[4096];

   snprintf(buf, sizeof(buf), "%s/e-module-bluez5.edj", mod->dir);
   o = edje_object_add(evas);
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const E_Gadcon_Client_Class _gc_class = {
   GADCON_CLIENT_CLASS_VERSION, "bluez5",
     {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL},
   E_GADCON_CLIENT_STYLE_PLAIN
};

/////////////////////////////////////////////////////////////////////////////

void
ebluez5_conf_adapter_add(const char *addr, Eina_Bool powered, Eina_Bool pairable)
{
   Eina_List *l;
   Config_Adapter *ad;

   if (!ebluez5_config) ebluez5_config = E_NEW(Config, 1);
   if (!ebluez5_config) return;
   EINA_LIST_FOREACH(ebluez5_config->adapters, l, ad)
     {
        if (!ad->addr) continue;
        if (!strcmp(addr, ad->addr))
          {
             if ((ad->powered == powered) && (ad->pairable == pairable)) return;
             ad->powered = powered;
             ad->pairable = pairable;
             e_config_save_queue();
             return;
          }
     }
   ad = E_NEW(Config_Adapter, 1);
   if (ad)
     {
        ad->addr = eina_stringshare_add(addr);
        ad->powered = powered;
        ad->pairable = pairable;
        ebluez5_config->adapters = eina_list_append(ebluez5_config->adapters, ad);
     }
   e_config_save_queue();
}

void
ebluez5_popups_show(void)
{
   Eina_List *l;
   Instance *inst;

   EINA_LIST_FOREACH(instances, l, inst)
     {
        if (inst->gcc)
          {
             if (!inst->popup) _popup_new(inst);
          }
     }
}

static void
_cb_rfkill_unblock(void *datam EINA_UNUSED, const char *params)
{
   int ret_code = 0;

   if (sscanf(params, "%i %*s", &ret_code) != 1) return;

   if (ret_code == 0) return;

   e_util_dialog_show
     (_("Bluetooth rfkill run Error"),
      _("Trying to rfkill unblock the bluetooth adapter failed.<br>"
        "Do you have rfkill installed? Check sysactions.conf<br>"
        "to ensure the command is right and your user is<br>"
        "permitted to use the rfkill unblock action. Check the<br>"
        "users and groups there to be sure."));
}

static void
_cb_rfkill_list(void *datam EINA_UNUSED, const char *params)
{
   if ((!params) || (!strcmp(params, "-"))) return;
   // params is:
   // hci0\tBluetooth\n
   // phy0\tWireless LAN\n
   // ...
   // we got a list of possible rf-killable devices and this list callback
   // will only have happened if we requested a list which we only do if
   // tjhe list of adapters is empty still 5 seconds after init - something
   // possibly wrong with them being blocked and thbus bluez not even listing
   // them, so unblock them to get them listed
   char **lines = eina_str_split(params, "\n", 0);
   if (lines)
     {
        int i = 0;
        char *line = lines[i];

        while (line)
          {
             char **fields = eina_str_split(line, "\t", 0);
             if (fields)
               {
                  if ((fields[0]) && (fields[1]))
                    {
                       if (!strcasecmp(fields[1], "bluetooth"))
                         {
                            ebluez5_rfkill_unblock(fields[0]);
                         }
                    }
                  free(fields[0]);
                  free(fields);
               }
             i++;
             line = lines[i];
          }
        free(lines[0]);
        free(lines);
     }
}

void
ebluez5_rfkill_unblock(const char *name)
{
   e_system_send("rfkill-unblock", "%s", name);
}

void
ebluez5_instances_update(void)
{
   const Eina_List *l;
   Obj *o;
   Instance *inst;
   Eina_Bool exist = EINA_FALSE;
   Eina_Bool on = EINA_FALSE;
   Eina_Bool visible = EINA_FALSE;
   Eina_Bool scanning = EINA_FALSE;

   EINA_LIST_FOREACH(ebluez5_popup_adapters_get(), l, o)
     {
        exist = EINA_TRUE;
        if (o->powered) on = EINA_TRUE;
        if (o->discoverable) visible = EINA_TRUE;
        if (o->discovering) scanning = EINA_TRUE;
     }
   EINA_LIST_FOREACH(instances, l, inst)
     {
        if (exist)    edje_object_signal_emit(inst->o_bluez5, "e,state,exist", "e");
        else          edje_object_signal_emit(inst->o_bluez5, "e,state,noexist", "e");
        if (on)       edje_object_signal_emit(inst->o_bluez5, "e,state,on", "e");
        else          edje_object_signal_emit(inst->o_bluez5, "e,state,off", "e");
        if (visible)  edje_object_signal_emit(inst->o_bluez5, "e,state,visible", "e");
        else          edje_object_signal_emit(inst->o_bluez5, "e,state,invisible", "e");
        if (scanning) edje_object_signal_emit(inst->o_bluez5, "e,state,scanning", "e");
        else          edje_object_signal_emit(inst->o_bluez5, "e,state,unscanning", "e");
     }
}

static void
_device_prop_clean(Config_Device *dev)
{
   if ((!dev->unlock) && (!dev->force_connect))
     {
        ebluez5_config->devices = eina_list_remove(ebluez5_config->devices, dev);
        eina_stringshare_del(dev->addr);
        free(dev);
     }
}

static Config_Device *
_device_prop_new(const char *address)
{
   Config_Device *dev = calloc(1, sizeof(Config_Device));
   if (!dev) return NULL;
   dev->addr = eina_stringshare_add(address);
   if (!dev->addr)
     {
        free(dev);
        return NULL;
     }
   ebluez5_config->devices = eina_list_append(ebluez5_config->devices, dev);
   return dev;
}

Config_Device *
ebluez5_device_prop_find(const char *address)
{
   Config_Device *dev;
   Eina_List *l;

   if (!address) return NULL;
   EINA_LIST_FOREACH(ebluez5_config->devices, l, dev)
     {
        if ((dev->addr) && (!strcmp(address, dev->addr)))
          return dev;
     }
   return NULL;
}


void
ebluez5_device_prop_force_connect_set(const char *address, Eina_Bool enable)
{
   Config_Device *dev;

   if (!address) return;
   dev = ebluez5_device_prop_find(address);
   if (dev)
     {
        dev->force_connect = enable;
        _device_prop_clean(dev);
        return;
     }
   if (enable)
     {
        dev = _device_prop_new(address);
        dev->force_connect = enable;
     }
}

void
ebluez5_device_prop_unlock_set(const char *address, Eina_Bool enable)
{
   Config_Device *dev;

   if (!address) return;
   dev = ebluez5_device_prop_find(address);
   if (dev)
     {
        dev->unlock = enable;
        _device_prop_clean(dev);
        return;
     }
   if (enable)
     {
        dev = _device_prop_new(address);
        dev->unlock = enable;
     }
}

static Eina_Bool
_cb_zero_adapters_check(void *data EINA_UNUSED)
{
   zero_adapters_check_timer = NULL;
   if (!ebluez5_popup_adapters_get())
     e_system_send("rfkill-list", "-");
   return EINA_FALSE;
}

/////////////////////////////////////////////////////////////////////////////

/* Module Functions */
E_API void *
e_modapi_init(E_Module *m)
{
   mod = m;

   conf_adapter_edd = E_CONFIG_DD_NEW("Config_Adapter", Config_Adapter);
#undef T
#undef D
#define T Config_Adapter
#define D conf_adapter_edd
   E_CONFIG_VAL(D, T, addr, STR);
   E_CONFIG_VAL(D, T, powered, UCHAR);
   E_CONFIG_VAL(D, T, pairable, UCHAR);

   conf_device_edd = E_CONFIG_DD_NEW("Config_Device", Config_Device);
#undef T
#undef D
#define T Config_Device
#define D conf_device_edd
   E_CONFIG_VAL(D, T, addr, STR);
   E_CONFIG_VAL(D, T, force_connect, UCHAR);
   E_CONFIG_VAL(D, T, unlock, UCHAR);

   conf_edd = E_CONFIG_DD_NEW("Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_LIST(D, T, adapters, conf_adapter_edd);
   E_CONFIG_LIST(D, T, devices, conf_device_edd);

   e_system_handler_add("rfkill-unblock", _cb_rfkill_unblock, NULL);
   e_system_handler_add("rfkill-list", _cb_rfkill_list, NULL);

   ebluez5_config = e_config_domain_load("module.ebluez5", conf_edd);
   if (!ebluez5_config) ebluez5_config = E_NEW(Config, 1);

   ebluze5_popup_init();
   bz_init();

   e_gadcon_provider_register(&_gc_class);

   zero_adapters_check_timer = ecore_timer_add(5.0, _cb_zero_adapters_check, NULL);

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   Config_Adapter *ad;
   Config_Device *dev;

   if (zero_adapters_check_timer)
     {
        ecore_timer_del(zero_adapters_check_timer);
        zero_adapters_check_timer = NULL;
     }
   e_system_handler_del("rfkill-list", _cb_rfkill_list, NULL);
   e_system_handler_del("rfkill-unblock", _cb_rfkill_unblock, NULL);
   EINA_LIST_FREE(ebluez5_config->adapters, ad)
     {
        eina_stringshare_del(ad->addr);
        free(ad);
     }
   EINA_LIST_FREE(ebluez5_config->devices, dev)
     {
        eina_stringshare_del(dev->addr);
        free(dev);
     }
   free(ebluez5_config);
   ebluez5_config = NULL;

   bz_shutdown();
   ebluze5_popup_shutdown();

   E_CONFIG_DD_FREE(conf_edd);
   E_CONFIG_DD_FREE(conf_adapter_edd);
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.ebluez5", conf_edd, ebluez5_config);
   return 1;
}
