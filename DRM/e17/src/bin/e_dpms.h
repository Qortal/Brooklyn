#ifdef E_TYPEDEFS
#else
#ifndef E_DPMS_H
#define E_DPMS_H

EINTERN int e_dpms_init(void);
EINTERN int e_dpms_shutdown(void);

E_API void e_dpms_update(void);
E_API void e_dpms_force_update(void);

#endif
#endif
