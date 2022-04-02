#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include "e.h"

E_API extern E_Module_Api e_modapi;

E_API void *e_modapi_init(E_Module *m);
E_API int   e_modapi_shutdown(E_Module *m);
E_API int   e_modapi_save(E_Module *m);

typedef struct _Instance Instance;
typedef struct _Context_Notifier_Host Context_Notifier_Host;
typedef struct _Instance_Notifier_Host Instance_Notifier_Host;
typedef struct _Notifier_Item Notifier_Item;
typedef struct _Systray_Context Systray_Context;
typedef struct Systray_Config Systray_Config;

struct _E_Config_Dialog_Data
{
};

struct _Systray_Context
{
   Systray_Config *config;
   E_Config_DD *conf_edd;
   E_Config_DD *notifier_item_edd;
};

struct _Instance
{
   E_Gadcon_Client        *gcc;
   Evas                   *evas;
   Instance_Notifier_Host *notifier;
   Eina_List              *icons;
   struct {
      Evas_Object         *gadget;
   } ui;
   struct {
      Ecore_Job           *size_apply;
   } job;
};

typedef struct Notifier_Item_Cache
{
   Eina_Stringshare       *path;
} Notifier_Item_Cache;

struct Systray_Config
{
   Eina_Stringshare       *dbus;
   Eina_Hash              *items;
};

const E_Gadcon *systray_gadcon_get(const Instance *inst);
void systray_size_updated(Instance *inst);
Evas_Object *systray_edje_get(const Instance *inst);
void systray_edje_box_append(Instance *inst, Evas_Object *child);
void systray_edje_box_remove(Instance *inst, Evas_Object *child);
void systray_edje_box_prepend(Instance *inst, Evas_Object *child);

Instance_Notifier_Host *systray_notifier_host_new(Instance *inst, E_Gadcon *gadcon);
void systray_notifier_host_free(Instance_Notifier_Host *notifier);
void systray_notifier_host_init(void);
void systray_notifier_host_shutdown(void);


EINTERN Systray_Context *systray_ctx_get(void);
extern Instance *instance;
/**
 * @addtogroup Optional_Gadgets
 * @{
 *
 * @defgroup Module_Systray Systray (System Icons Tray)
 *
 * Shows system icons in a box.
 *
 * The icons come from the FreeDesktop.Org systray specification.
 *
 * @see http://standards.freedesktop.org/systemtray-spec/systemtray-spec-latest.html
 * @}
 */
#endif
