#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _E_Config_Wallpaper E_Config_Wallpaper;

E_Config_Dialog *e_int_config_xsettings(Evas_Object *parent, const char *params EINA_UNUSED);

/**
 * @addtogroup Optional_Conf
 * @{
 *
 * @defgroup Module_Conf_Theme Theme Configuration
 *
 * Changes the graphical user interface theme and wallpaper.
 *
 * @}
 */

E_Config_Dialog *e_int_config_borders(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_borders_border(Evas_Object *parent, const char *params);

E_Config_Dialog *e_int_config_color_classes(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_fonts(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_scale(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_theme(Evas_Object *parent, const char *params EINA_UNUSED);

void             e_int_config_theme_import_done(E_Config_Dialog *dia);
void             e_int_config_theme_update(E_Config_Dialog *dia, char *file);

Evas_Object *e_int_config_theme_import(E_Config_Dialog *parent);

E_Config_Dialog *e_int_config_transitions(Evas_Object *parent, const char *params EINA_UNUSED);

E_Config_Dialog *e_int_config_wallpaper(Evas_Object *parent, const char *params EINA_UNUSED);
E_Config_Dialog *e_int_config_wallpaper_desk(Evas_Object *parent, const char *params);

void e_int_config_wallpaper_update(E_Config_Dialog *dia, char *file);
void e_int_config_wallpaper_import_done(E_Config_Dialog *dia);

#endif
