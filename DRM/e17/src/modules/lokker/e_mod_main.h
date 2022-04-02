#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "e.h"

#undef DBG
#undef INF
#undef WRN
#undef ERR
#undef CRIT
#define DBG(...)  EINA_LOG_DOM_DBG(_e_lokker_log_dom, __VA_ARGS__)
#define INF(...)  EINA_LOG_DOM_INFO(_e_lokker_log_dom, __VA_ARGS__)
#define WRN(...)  EINA_LOG_DOM_WARN(_e_lokker_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(_e_lokker_log_dom, __VA_ARGS__)
#define CRIT(...) EINA_LOG_DOM_CRIT(_e_lokker_log_dom, __VA_ARGS__)


typedef enum
{
   E_DESKLOCK_AUTH_METHOD_LINES = 4,
} E_Desklock_Auth_Method2;

EINTERN Eina_Bool lokker_lock(void);
EINTERN void lokker_unlock(void);
E_API E_Config_Dialog *e_int_config_lokker(Evas_Object *parent, const char *params EINA_UNUSED);
EINTERN Eina_Bool lokker_key_down(Ecore_Event_Key*);
EINTERN Eina_Bool lokker_key_up(Ecore_Event_Key*);
#endif
