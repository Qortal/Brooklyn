#ifdef E_TYPEDEFS

typedef struct _E_Exec_Instance E_Exec_Instance;

#else
#ifndef E_EXEC_H
#define E_EXEC_H

struct _E_Exec_Instance
{
   Efreet_Desktop *desktop;
   Eina_List      *clients;
   const char     *key;
   Ecore_Exe      *exe;
   int             startup_id;
   double          launch_time;
   Ecore_Timer    *expire_timer;
   int             screen;
   int             desk_x, desk_y;
   int             used;
   int             ref;
   Eina_List      *watchers;
   Eina_Bool       phony E_BITFIELD;
   Eina_Bool       deleted E_BITFIELD;
};

typedef enum
{
   E_EXEC_WATCH_STARTED,
   E_EXEC_WATCH_STOPPED,
   E_EXEC_WATCH_TIMEOUT
} E_Exec_Watch_Type;

EINTERN int  e_exec_init(void);
EINTERN int  e_exec_shutdown(void);
E_API void e_exec_executor_set(E_Exec_Instance *(*func) (void *data, E_Zone *zone, Efreet_Desktop *desktop, const char *exec, Eina_List *files, const char *launch_method), const void *data);
E_API E_Exec_Instance *e_exec(E_Zone *zone, Efreet_Desktop *desktop, const char *exec, Eina_List *files, const char *launch_method);
E_API E_Exec_Instance *e_exec_phony(E_Client *ec);
E_API Eina_Bool e_exec_phony_del(E_Exec_Instance *inst);
E_API E_Exec_Instance *e_exec_startup_id_pid_instance_find(int id, pid_t pid);
E_API Efreet_Desktop *e_exec_startup_id_pid_find(int startup_id, pid_t pid);
E_API E_Exec_Instance *e_exec_startup_desktop_instance_find(Efreet_Desktop *desktop);
E_API void e_exec_instance_found(E_Exec_Instance *inst);
E_API void e_exec_instance_watcher_add(E_Exec_Instance *inst, void (*func) (void *data, E_Exec_Instance *inst, E_Exec_Watch_Type type), const void *data);
E_API void e_exec_instance_watcher_del(E_Exec_Instance *inst, void (*func) (void *data, E_Exec_Instance *inst, E_Exec_Watch_Type type), const void *data);
E_API const Eina_List *e_exec_desktop_instances_find(const Efreet_Desktop *desktop);

E_API const Eina_Hash *e_exec_instances_get(void);
E_API void e_exec_instance_client_add(E_Exec_Instance *inst, E_Client *ec);

/* sends E_Exec_Instance */
E_API extern int E_EVENT_EXEC_NEW;
E_API extern int E_EVENT_EXEC_NEW_CLIENT;
E_API extern int E_EVENT_EXEC_DEL;

#endif
#endif
