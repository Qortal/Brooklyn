#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include "config.h"
#include <e.h>

E_API extern E_Module_Api e_modapi;
E_API void *e_modapi_init(E_Module *m);
E_API int e_modapi_shutdown(E_Module *m);
E_API int e_modapi_save(E_Module *m);

#endif
