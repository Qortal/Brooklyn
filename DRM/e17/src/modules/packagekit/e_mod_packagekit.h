#ifndef PACKAGEKIT_H
#define PACKAGEKIT_H

#include <e.h>


#define PKITV07 (ctxt->v_maj == 0) && (ctxt->v_min == 7)
#define PKITV08 (ctxt->v_maj == 0) && (ctxt->v_min == 8)

// PackageKit dbus interface contains bitfield constants which
// aren't introspectable. we define here. still cannot find the proper doc!
#define PK_PROVIDES_ANY 1
#define PK_FILTER_ENUM_NOT_INSTALLED 1 << 3
#define PK_FILTER_ENUM_NEWEST 1 << 16
#define PK_FILTER_ENUM_ARCH 1 << 18
#define PK_TRANSACTION_FLAG_ENUM_ONLY_TRUSTED 1 << 1

typedef enum {
   PK_INFO_ENUM_UNKNOWN,
   PK_INFO_ENUM_INSTALLED,
   PK_INFO_ENUM_AVAILABLE,
   PK_INFO_ENUM_LOW,
   PK_INFO_ENUM_ENHANCEMENT,
   PK_INFO_ENUM_NORMAL,
   PK_INFO_ENUM_BUGFIX,
   PK_INFO_ENUM_IMPORTANT,
   PK_INFO_ENUM_SECURITY,
   PK_INFO_ENUM_BLOCKED,
   PK_INFO_ENUM_DOWNLOADING,
   PK_INFO_ENUM_UPDATING,
   PK_INFO_ENUM_INSTALLING,
   PK_INFO_ENUM_REMOVING,
   PK_INFO_ENUM_CLEANUP,
   PK_INFO_ENUM_OBSOLETING,
   PK_INFO_ENUM_COLLECTION_INSTALLED,
   PK_INFO_ENUM_COLLECTION_AVAILABLE,
   PK_INFO_ENUM_FINISHED,
   PK_INFO_ENUM_REINSTALLING,
   PK_INFO_ENUM_DOWNGRADING,
   PK_INFO_ENUM_PREPARING,
   PK_INFO_ENUM_DECOMPRESSING,
   PK_INFO_ENUM_UNTRUSTED,
   PK_INFO_ENUM_TRUSTED,
   PK_INFO_ENUM_LAST,
} PackageKit_Package_Info;

typedef struct _PackageKit_Config
{
   int update_interval;
   int last_update;
   const char *manager_command;
   int show_description;
} PackageKit_Config;

typedef struct _E_PackageKit_Module_Context
{
   E_Module *module;
   Eina_List *instances;
   Eina_List *packages;
   Ecore_Timer *refresh_timer;
   const char *error;
   int v_maj;
   int v_min;
   int v_mic;

   Eldbus_Connection *conn;
   Eldbus_Proxy *packagekit;
   Eldbus_Proxy *transaction;
   double transaction_progress;

   E_Config_DD *conf_edd;
   PackageKit_Config *config;

} E_PackageKit_Module_Context;

typedef struct _E_PackageKit_Instance
{
   E_PackageKit_Module_Context *ctxt;
   E_Gadcon_Client *gcc;
   Evas_Object *gadget;
   E_Gadcon_Popup *popup;
   Evas_Object *popup_title_entry;
   Evas_Object *popup_error_label;
   Evas_Object *popup_install_button;
   Evas_Object *popup_progressbar;
   Evas_Object *popup_progressbar_frame;
   Evas_Object *popup_genlist;
   Elm_Genlist_Item_Class *popup_genlist_itc;
   Eina_Bool popup_help_mode;
} E_PackageKit_Instance;

typedef struct _E_PackageKit_Package
{
   const char *pkg_id;
   const char *name;
   const char *summary;
   const char *version;
   PackageKit_Package_Info info;
   Eina_Bool to_be_installed;
} E_PackageKit_Package;


typedef void (*E_PackageKit_Transaction_Func)(E_PackageKit_Module_Context *ctxt,
                                              const char *transaction);


Eina_Bool packagekit_dbus_connect(E_PackageKit_Module_Context *ctxt);
void      packagekit_dbus_disconnect(E_PackageKit_Module_Context *ctxt);
void      packagekit_create_transaction_and_exec(E_PackageKit_Module_Context *ctxt,
                                                 E_PackageKit_Transaction_Func func);
void      packagekit_get_updates(E_PackageKit_Module_Context *ctxt, const char *transaction);
void      packagekit_refresh_cache(E_PackageKit_Module_Context *ctxt, const char *transaction);
void      packagekit_update_packages(E_PackageKit_Module_Context *ctxt, const char *transaction);
void      packagekit_icon_update(E_PackageKit_Module_Context *ctxt, Eina_Bool working);
void      packagekit_popup_new(E_PackageKit_Instance *inst);
void      packagekit_popup_del(E_PackageKit_Instance *inst);
void      packagekit_popup_update(E_PackageKit_Instance *inst, Eina_Bool rebuild_list);


#endif
