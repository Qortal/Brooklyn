#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

E_Config_Dialog *e_int_config_env(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_paths(Evas_Object *parent, const char *params EINA_UNUSED);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Paths Paths & Environment Configuration
 *
 * Configures where to search for fonts, icons, images, themes,
 * walpapers and others.
 *
 * Can also configure environment variables used and propagated by
 * Enlightenment to child process and applications.
 *
 * @}
 */
#endif
