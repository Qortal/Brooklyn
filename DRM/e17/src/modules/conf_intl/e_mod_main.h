#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

E_Config_Dialog *e_int_config_imc(Evas_Object *parent, const char *params EINA_UNUSED);
void e_int_config_imc_import_done(E_Config_Dialog *dia);
void e_int_config_imc_update(E_Config_Dialog *dia, const char *file);

Evas_Object *e_int_config_imc_import(E_Config_Dialog *parent);
void e_int_config_imc_import_del(Evas_Object *);

E_Config_Dialog *e_int_config_intl(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_desklock_intl(Evas_Object *parent, const char *params EINA_UNUSED);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Intl Internationalization Configuration
 *
 * Configure language and input method (e.g. Asian languages)
 *
 * @}
 */
#endif
