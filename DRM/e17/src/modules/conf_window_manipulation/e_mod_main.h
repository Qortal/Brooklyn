#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

E_Config_Dialog *e_int_config_window_geometry(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_window_process(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_window_display(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_focus(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_clientlist(Evas_Object *parent, const char *params EINA_UNUSED);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Window_Manipulation Window Manipulation Configuration
 *
 * Configures how windows are placed, maximized, focused, stacked and
 * so on.
 *
 * @see Module_Conf_Window_Remembers
 * @}
 */
#endif
