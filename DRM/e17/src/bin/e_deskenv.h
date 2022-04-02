#ifdef E_TYPEDEFS
#else
#ifndef E_DESKENV_H
#define E_DESKENV_H

EINTERN int  e_deskenv_init(void);
EINTERN int  e_deskenv_shutdown(void);
E_API void e_deskenv_xmodmap_run(void);
#endif
#endif
