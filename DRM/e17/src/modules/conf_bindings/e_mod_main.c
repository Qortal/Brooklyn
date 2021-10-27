#include "e.h"
#include "e_mod_main.h"

/* actual module specifics */
static E_Module *conf_module = NULL;

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Settings - Input Controls"
};

E_API void *
e_modapi_init(E_Module *m)
{
   e_configure_registry_category_add("keyboard_and_mouse", 40, _("Input"),
                                     NULL, "preferences-behavior");

   e_configure_registry_item_add("keyboard_and_mouse/key_bindings", 10,
                                 _("Key Bindings"), NULL,
                                 "preferences-desktop-keyboard-shortcuts",
                                 e_int_config_keybindings);
   e_configure_registry_item_add("keyboard_and_mouse/mouse_bindings", 20,
                                 _("Mouse Bindings"), NULL,
                                 "preferences-desktop-mouse",
                                 e_int_config_mousebindings);
   e_configure_registry_item_add("keyboard_and_mouse/acpi_bindings", 30,
                                 _("ACPI Bindings"), NULL,
                                 "preferences-system-power-management",
                                 e_int_config_acpibindings);
   e_configure_registry_item_add("keyboard_and_mouse/edge_bindings", 10,
                                 _("Edge Bindings"), NULL,
                                 "preferences-desktop-edge-bindings",
                                 e_int_config_edgebindings);
#ifdef HAVE_ELPUT
   e_configure_registry_item_add("keyboard_and_mouse/swipe_bindings", 10,
                                 _("Swipe Bindings"), NULL,
                                 "preferences-desktop-swipe-bindings",
                                 e_int_config_swipebindings);
#endif

   e_configure_registry_category_add("advanced", 80, _("Advanced"), NULL, "preferences-advanced");
   e_configure_registry_item_add("advanced/signal_bindings", 10,
                                 _("Signal Bindings"), NULL,
                                 "preferences-desktop-signal-bindings",
                                 e_int_config_signalbindings);
   conf_module = m;

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   E_Config_Dialog *cfd;

   while ((cfd = e_config_dialog_get("E", "keyboard_and_mouse/acpi_bindings")))
     e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "keyboard_and_mouse/mouse_bindings")))
     e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "keyboard_and_mouse/key_bindings")))
     e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "keyboard_and_mouse/edge_bindings")))
     e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "advanced/signal_bindings")))
     e_object_del(E_OBJECT(cfd));

   e_configure_registry_item_del("keyboard_and_mouse/acpi_bindings");
   e_configure_registry_item_del("keyboard_and_mouse/mouse_bindings");
   e_configure_registry_item_del("keyboard_and_mouse/key_bindings");
   e_configure_registry_item_del("keyboard_and_mouse/edge_bindings");
   e_configure_registry_item_del("advanced/signal_bindings");
   e_configure_registry_category_del("keyboard_and_mouse");
   e_configure_registry_category_del("advanced");

   conf_module = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}

