#include "e_mod_main.h"

E_API E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Polkit"};

E_API void *
e_modapi_init(E_Module *m)
{
   e_mod_polkit_register();
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_mod_polkit_unregister();
   return 1;
}
