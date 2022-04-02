#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "e.h"

/* Increment for Major Changes */
#define MOD_CONFIG_FILE_EPOCH      1
/* Increment for Minor Changes (ie: user doesn't need a new config) */
#define MOD_CONFIG_FILE_GENERATION 0
#define MOD_CONFIG_FILE_VERSION    ((MOD_CONFIG_FILE_EPOCH * 1000000) + MOD_CONFIG_FILE_GENERATION)

typedef struct E_Quick_Access_Entry
{
   const char *id; /* entry identifier (config, actions...), stringshared */
   const char *name; /* icccm name, stringshared */
   const char *class; /* icccm class, stringshared */
   const char *cmd; /* stringshared */
   Ecore_X_Window win; /* current window */
   E_Client       *client; /* associated client, if any */
   Ecore_Event_Handler *exe_handler; /* for catching exe delete */
   Ecore_Exe      *exe; /* if executed cmd but still no border associated */
   E_Dialog *dia; // used for option handling
   void *cfg_entry; // created by config dialog

   struct
   {
      Eina_Bool autohide; // hide when focus lost
      Eina_Bool hide_when_behind; // hide when window is not focused instead of raising
      Eina_Bool hidden; // FIXME: used for tracking current state to restore on restart
      Eina_Bool relaunch; // reopen on exit
      Eina_Bool jump; // don't hide window, just jump to it when activated
   } config;
   Eina_Bool transient;
   Eina_Bool help_watch; // tutorial is currently watching for this entry
} E_Quick_Access_Entry;

typedef struct Config
{
   unsigned int config_version;
   Eina_List *entries;
   Eina_List *transient_entries;

   Eina_Bool autohide;
   Eina_Bool hide_when_behind;
   Eina_Bool skip_window_list;
   Eina_Bool skip_taskbar;
   Eina_Bool skip_pager;
   Eina_Bool dont_bug_me;
   Eina_Bool first_run; // set to disable help popups
} Config;

typedef struct Mod
{
   E_Module *module;
   E_Config_Dialog *cfd;
   E_Object *help_dia;
   E_Dialog *demo_dia;
   unsigned int demo_state;
   Ecore_Timer *help_timer;
   Ecore_Timer *help_timeout;
   E_Menu *menu;
} Mod;

extern Config *qa_config;
extern Mod *qa_mod;
extern int _e_quick_access_log_dom;
extern const char *_act_toggle;
#undef DBG
#undef INF
#undef WRN
#undef ERR
#undef CRIT
#define DBG(...)  EINA_LOG_DOM_DBG(_e_quick_access_log_dom, __VA_ARGS__)
#define INF(...)  EINA_LOG_DOM_INFO(_e_quick_access_log_dom, __VA_ARGS__)
#define WRN(...)  EINA_LOG_DOM_WARN(_e_quick_access_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(_e_quick_access_log_dom, __VA_ARGS__)
#define CRIT(...) EINA_LOG_DOM_CRIT(_e_quick_access_log_dom, __VA_ARGS__)

Eina_Bool e_qa_init(void);
void e_qa_shutdown(void);
void e_qa_entry_free(E_Quick_Access_Entry *entry);
E_Quick_Access_Entry *e_qa_entry_new(const char *id, Eina_Bool transient);
void e_qa_entries_update(void);
Eina_Bool e_qa_entry_rename(E_Quick_Access_Entry *entry, const char *name);

E_Config_DD *e_qa_config_dd_new(void);
void e_qa_config_free(Config *conf);
Config *e_qa_config_new(void);
void *e_qa_config_dd_free(void);
void e_qa_config_entry_free(E_Quick_Access_Entry *entry);
void e_qa_config_entry_add(E_Quick_Access_Entry *entry);
void e_qa_config_entry_transient_convert(E_Quick_Access_Entry *entry);
E_Config_Dialog *e_int_config_qa_module(Evas_Object *parent, const char *params EINA_UNUSED);

char *e_qa_db_class_lookup(const char *class);

void e_qa_entry_bindings_cleanup(E_Quick_Access_Entry *entry);
void e_qa_entry_bindings_rename(E_Quick_Access_Entry *entry, const char *name);

E_API int e_modapi_shutdown(E_Module *m EINA_UNUSED);
#endif
