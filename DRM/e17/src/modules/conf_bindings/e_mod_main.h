#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

E_Config_Dialog *e_int_config_acpibindings(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_keybindings(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_mousebindings(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_edgebindings(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_signalbindings(Evas_Object *parent, const char *params);
E_Config_Dialog *e_int_config_swipebindings(Evas_Object *parent EINA_UNUSED, const char *params);
/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_KeyBinding Key Bindings (Shortcuts) Configuration
 *
 * Configure global keyboard shortcuts.
 *
 * @see Module_Conf_MouseBinding
 * @see Module_Conf_EdgeBinding
 * @}
 */
#endif
