#ifdef E_TYPEDEFS
#else
#ifndef E_SCALE_H
#define E_SCALE_H

EINTERN int  e_scale_init(void);
EINTERN int  e_scale_shutdown(void);
E_API double e_scale_dpi_get(void);
E_API void   e_scale_update(void);

extern E_API double e_scale;

#endif
#endif
