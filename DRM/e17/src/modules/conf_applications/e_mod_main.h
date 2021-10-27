#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

E_Config_Dialog *e_int_config_apps_favs(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_apps_add(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_apps_ibar(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_apps_ibar_other(Evas_Object *parent, const char *path);
E_Config_Dialog *e_int_config_apps_startup(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_apps_restart(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_apps_desk_lock(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_apps_desk_unlock(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_defapps(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_apps_personal(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_deskenv(Evas_Object *parent, const char *params EINA_UNUSED);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Applications Applications Configuration
 *
 * Configure application icons (launchers ".desktop"), which
 * applications to use on start or restart of Enlightenment.
 *
 * @}
 */
#endif
