#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include <e.h>

#include "E_Connman.h"

#define AGENT_PATH "/org/enlightenment/connman/agent"

#define MOD_CONF_VERSION 3

extern E_Module *connman_mod;
extern int _e_connman_log_dom;

typedef struct E_Connman_Instance E_Connman_Instance;
typedef struct E_Connman_Module_Context E_Connman_Module_Context;

struct E_Connman_Instance
{
   E_Connman_Module_Context *ctxt;
   E_Gadcon_Client *gcc;

   E_Gadcon_Popup *popup;

   struct
     {
        Evas_Object *gadget;

        struct
          {
             Evas_Object *list;
             Evas_Object *powered;
          } popup;
     } ui;
};

struct E_Connman_Module_Context
{
   Eina_List *instances;
   E_Config_Dialog *conf_dialog;

   struct
     {
        Ecore_Event_Handler *manager_in;
        Ecore_Event_Handler *manager_out;
     } event;

   struct Connman_Manager *cm;
   Eina_Bool offline_mode;
   int powered;
};

E_API extern E_Module_Api e_modapi;
E_API void *e_modapi_init(E_Module *m);
E_API int e_modapi_shutdown(E_Module *m);
E_API int e_modapi_save(E_Module *m);

void econnman_popup_del(E_Connman_Instance *inst);
const char *e_connman_theme_path(void);
E_Config_Dialog *e_connman_config_dialog_new(Evas_Object *parent,
                                             E_Connman_Module_Context *ctxt);

E_Connman_Agent *econnman_agent_new(Eldbus_Connection *eldbus_conn) EINA_ARG_NONNULL(1);
void econnman_agent_del(E_Connman_Agent *agent);

/**
 * @addtogroup Optional_Devices
 * @{
 *
 * @defgroup Module_Connman ConnMan (Connection Manager)
 *
 * Controls network connections for ethernet, wifi, 3G, GSM and
 * bluetooth (PAN).
 *
 * @see http://connman.net/
 * @}
 */

#endif
