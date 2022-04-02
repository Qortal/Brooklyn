#ifdef E_TYPEDEFS

typedef struct _E_Sys_Con_Action E_Sys_Con_Action;

typedef enum _E_Sys_Action
{
   E_SYS_NONE,
   E_SYS_EXIT,
   E_SYS_RESTART,
   E_SYS_EXIT_NOW,
   E_SYS_LOGOUT,
   E_SYS_HALT,
   E_SYS_HALT_NOW,
   E_SYS_REBOOT,
   E_SYS_SUSPEND,
   E_SYS_HIBERNATE
} E_Sys_Action;

struct _E_Sys_Con_Action
{
   const char *label;
   const char *icon_group;
   const char *button_name;
   void (*func) (void *data);
   const void *data;
   Eina_Bool disabled E_BITFIELD;
};

#else
#ifndef E_SYS_H
#define E_SYS_H

E_API extern int E_EVENT_SYS_SUSPEND;
E_API extern int E_EVENT_SYS_HIBERNATE;
E_API extern int E_EVENT_SYS_RESUME;

EINTERN int e_sys_init(void);
EINTERN int e_sys_shutdown(void);
E_API int e_sys_action_possible_get(E_Sys_Action a);
E_API int e_sys_action_do(E_Sys_Action a, char *param);
E_API int e_sys_action_raw_do(E_Sys_Action a, char *param);

E_API E_Sys_Con_Action *e_sys_con_extra_action_register(const char *label,
                                                       const char *icon_group,
                                                       const char *button_name,
                                                       void (*func) (void *data),
                                                       const void *data);
E_API void e_sys_con_extra_action_unregister(E_Sys_Con_Action *sca);
E_API const Eina_List *e_sys_con_extra_action_list_get(void);
E_API Eina_Bool e_sys_on_the_way_out_get(void);

#endif
#endif
