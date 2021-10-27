#ifndef E_MOD_MAIN_H
# define E_MOD_MAIN_H

#if 0
//# define LOGFNS 1

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) printf("-CONF-RANDR: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif

E_API extern E_Module_Api e_modapi;

E_API void *e_modapi_init(E_Module *m);
E_API int e_modapi_shutdown(E_Module *m);
E_API int e_modapi_save(E_Module *m);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_RandR RandR (Screen Resize, Rotate and Mirror)
 *
 * Configures size, rotation and mirroring of screen. Uses the X11
 * RandR protocol (does not work with NVidia proprietary drivers).
 *
 * @}
 */

#endif
#endif
