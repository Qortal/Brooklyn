#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include <e.h>
#include "bz.h"

typedef struct _Instance Instance;
struct _Instance
{
   // common info
   Evas_Object *o_bluez5;
   // e_gadcon info
   E_Gadcon_Client *gcc;
   E_Gadcon_Popup *popup;
   int id;
};

typedef struct _Config Config;
struct _Config
{
   Eina_List *adapters;
   Eina_List *devices;
};
typedef struct _Config_Adapter Config_Adapter;
struct _Config_Adapter
{
   const char *addr;
   Eina_Bool powered;
   Eina_Bool pairable;
};
typedef struct _Config_Device Config_Device;
struct _Config_Device
{
   const char *addr;
   Eina_Bool force_connect;
   Eina_Bool unlock;
};

extern Config *ebluez5_config;

E_API extern E_Module_Api e_modapi;

E_API void *e_modapi_init(E_Module *m);
E_API int e_modapi_shutdown(E_Module *m);
E_API int e_modapi_save(E_Module *m);

void ebluez5_conf_adapter_add(const char *addr, Eina_Bool powered, Eina_Bool pairable);
void ebluez5_popups_show(void);
void ebluez5_rfkill_unblock(const char *name);
void ebluez5_instances_update(void);

void ebluez5_device_prop_force_connect_set(const char *address, Eina_Bool enable);
void ebluez5_device_prop_unlock_set(const char *address, Eina_Bool enable);
Config_Device *ebluez5_device_prop_find(const char *address);

void ebluez5_popup_hide(Instance *inst);

Evas_Object *ebluez5_popup_content_add(Evas_Object *base, Instance *inst);
void ebluze5_popup_init(void);
void ebluze5_popup_shutdown(void);
void ebluze5_popup_clear(void);
void ebluez5_popup_adapter_add(Obj *o);
void ebluez5_popup_adapter_del(Obj *o);
void ebluez5_popup_adapter_change(Obj *o);
void ebluez5_popup_device_add(Obj *o);
void ebluez5_popup_device_del(Obj *o);
void ebluez5_popup_device_change(Obj *o);
const Eina_List *ebluez5_popup_adapters_get(void);

void ebluez5_agent_agent_release(void);
void ebluez5_agent_agent_cancel(void);
void ebluez5_agent_agent_req_pin(Eldbus_Message *msg);
void ebluez5_agent_agent_disp_pin(Eldbus_Message *msg);
void ebluez5_agent_req_pass(Eldbus_Message *msg);
void ebluez5_agent_disp_pass(Eldbus_Message *msg);
void ebluez5_agent_req_confirm(Eldbus_Message *msg);
void ebluez5_agent_req_auth(Eldbus_Message *msg);
void ebluez5_agent_auth_service(Eldbus_Message *msg);

Evas_Object *util_obj_icon_add(Evas_Object *base, Obj *o, int size);
Evas_Object *util_obj_icon_rssi_add(Evas_Object *base, Obj *o, int size);
Evas_Object *util_check_add(Evas_Object *base, const char *text, const char *tip, Eina_Bool state);
Evas_Object *util_button_icon_add(Evas_Object *base, const char *icon, const char *tip);
const char  *util_obj_name_get(Obj *o);

#endif
