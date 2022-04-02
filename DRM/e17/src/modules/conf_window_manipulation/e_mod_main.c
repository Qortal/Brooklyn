#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Settings - Window Manipulation"
};

E_API void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("windows", 50, _("Windows"), NULL, "preferences-system-windows");
   e_configure_registry_item_add("windows/window_display", 10, _("Window Display"), NULL, "preferences-system-windows", e_int_config_window_display);
   e_configure_registry_item_add("windows/window_focus", 20, _("Window Focus"), NULL, "preferences-focus", e_int_config_focus);
   e_configure_registry_item_add("windows/window_geometry", 30, _("Window Geometry"), NULL, "preferences-window-geometry", e_int_config_window_geometry);
   e_configure_registry_item_add("windows/window_list_menu", 40, _("Window List Menu"), NULL, "preferences-winlist", e_int_config_clientlist);
   e_configure_registry_item_add("windows/window_process", 50, _("Window Process Management"), NULL, "preferences-window-process", e_int_config_window_process);
   conf_module = m;
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   while ((cfd = e_config_dialog_get("E", "windows/window_process"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "windows/window_list_menu"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "windows/window_geometry"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "windows/window_focus"))) e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "windows/window_display"))) e_object_del(E_OBJECT(cfd));
   e_configure_registry_item_del("windows/window_process");
   e_configure_registry_item_del("windows/window_list_menu");
   e_configure_registry_item_del("windows/window_geometry");
   e_configure_registry_item_del("windows/window_focus");
   e_configure_registry_item_del("windows/window_display");
   e_configure_registry_category_del("windows");
   conf_module = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}
