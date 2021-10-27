#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

E_Config_Dialog *e_int_config_engine(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_performance(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_powermanagement(Evas_Object *parent, const char *params EINA_UNUSED);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Performance Performance Configuration
 *
 * Configures the frame rate, graphics engine and other performance
 * switches.
 *
 * @}
 */
#endif
