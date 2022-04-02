#include "e.h"
#include "e_mod_main.h"

static E_Module *conf_module = NULL;

E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION, "Settings - Window Remembers"
};

E_API void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("windows", 50, _("Windows"), NULL,
                                     "preferences-system-windows");
   e_configure_registry_item_add("windows/window_remembers", 40,
                                 _("Window Remembers"), NULL,
                                 "preferences-desktop-window-remember",
                                 e_int_config_remembers);
   conf_module = m;

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   E_Config_Dialog *cfd;

   while ((cfd = e_config_dialog_get("E", "windows/window_remembers")))
     e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("windows/window_remembers");
   e_configure_registry_category_del("windows");

   conf_module = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}
