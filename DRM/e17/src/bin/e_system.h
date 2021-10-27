#ifdef E_TYPEDEFS

#else
#ifndef E_SYSTEM_H
#define E_SYSTEM_H

EINTERN int e_system_init(void);
EINTERN int e_system_shutdown(void);

E_API void e_system_send(const char *cmd, const char *fmt, ...) EINA_PRINTF(2, 3);
E_API void e_system_handler_add(const char *cmd, void (*func) (void *data, const char *params), void *data);
E_API void e_system_handler_del(const char *cmd, void (*func) (void *data, const char *params), void *data);

#endif
#endif
