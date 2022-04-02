#ifdef E_TYPEDEFS

#else
#ifndef E_FILEREG_H
#define E_FILEREG_H

EINTERN int e_filereg_init(void);
EINTERN int e_filereg_shutdown(void);

E_API int e_filereg_register(const char * path);
E_API void e_filereg_deregister(const char * path);
E_API Eina_Bool e_filereg_file_protected(const char * path);

#endif
#endif
