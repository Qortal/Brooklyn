#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "e.h"

typedef struct
{
   const char *cookie;
   const char *message;
   const char *icon_name;
   const char *action;
   unsigned int target_uid;
   int auth_pid;
   Ecore_Event_Handler *exe_exit_handler;
   Eldbus_Message *reply;
   Eldbus_Pending *pend_reply;
   Evas_Object *win;
   Evas_Object *entry;
} Polkit_Session;

void session_reply(Polkit_Session *ps);

void auth_ui(Polkit_Session *ps);

void e_mod_polkit_register(void);
void e_mod_polkit_unregister(void);

#endif
