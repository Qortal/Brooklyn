#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - Search Directories"
};

E_API void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("advanced", 80, _("Advanced"), NULL,
                                     "preferences-advanced");
   e_configure_registry_item_add("advanced/search_directories", 50,
                                 _("Search Directories"), NULL,
                                 "preferences-directories",
                                 e_int_config_paths);
   e_configure_registry_item_add("advanced/environment_variables", 120,
                                 _("Environment Variables"), NULL,
                                 "preferences-variables",
                                 e_int_config_env);
   conf_module = m;

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   E_Config_Dialog *cfd;

   while ((cfd = e_config_dialog_get("E", "advanced/environment_variables")))
     e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "advanced/search_directories")))
     e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("advanced/environment_variables");
   e_configure_registry_item_del("advanced/search_directories");
   e_configure_registry_category_del("advanced");

   conf_module = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}
