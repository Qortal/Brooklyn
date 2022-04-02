#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

E_Config_Dialog *e_int_config_desk(Evas_Object *parent, const char *params);
E_Config_Dialog *e_int_config_screensaver(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_dpms(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_display(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_desks(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_desklock(Evas_Object *parent, const char *params EINA_UNUSED);
void e_int_config_desklock_fsel_done(E_Config_Dialog *cfd, Evas_Object *bg, const char *bg_file, Eina_Bool hide_logo);
E_Config_Dialog *e_int_config_desklock_fsel(E_Config_Dialog *parent, Evas_Object *bg);
void e_int_config_desklock_fsel_del(E_Config_Dialog *cfd);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Display Display Configuration
 *
 * Configures the physical and virtual display, including screen
 * saver, screen lock and power saving settings (DPMS).
 *
 * @}
 */
#endif
