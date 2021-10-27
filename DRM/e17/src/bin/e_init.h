#ifdef E_TYPEDEFS
#else
#ifndef E_INIT_H
#define E_INIT_H

EINTERN int            e_init_init(void);
EINTERN int            e_init_shutdown(void);
E_API void           e_init_show(void);
E_API void           e_init_hide(void);
E_API void           e_init_title_set(const char *str);
E_API void           e_init_version_set(const char *str);
E_API void           e_init_status_set(const char *str);
E_API void           e_init_done(void);
E_API void           e_init_undone(void);
E_API void           e_init_client_data(Ecore_Ipc_Event_Client_Data *e);
E_API void           e_init_client_del(Ecore_Ipc_Event_Client_Del *e);
E_API int            e_init_count_get(void);

extern E_API int E_EVENT_INIT_DONE;

#endif
#endif
