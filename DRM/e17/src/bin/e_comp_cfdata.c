#include "e.h"
#include "e_comp_cfdata.h"

E_API void
e_comp_cfdata_edd_init(E_Config_DD **conf_edd, E_Config_DD **match_edd)
{
   *match_edd = E_CONFIG_DD_NEW("Comp_Match", E_Comp_Match);
#undef T
#undef D
#define T E_Comp_Match
#define D *match_edd
   E_CONFIG_VAL(D, T, title, STR);
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, clas, STR);
   E_CONFIG_VAL(D, T, role, STR);
   E_CONFIG_VAL(D, T, primary_type, INT);
   E_CONFIG_VAL(D, T, borderless, CHAR);
   E_CONFIG_VAL(D, T, dialog, CHAR);
   E_CONFIG_VAL(D, T, accepts_focus, CHAR);
   E_CONFIG_VAL(D, T, argb, CHAR);
   E_CONFIG_VAL(D, T, fullscreen, CHAR);
   E_CONFIG_VAL(D, T, modal, CHAR);
   E_CONFIG_VAL(D, T, focus, CHAR);
   E_CONFIG_VAL(D, T, urgent, CHAR);
   E_CONFIG_VAL(D, T, no_shadow, CHAR);
   E_CONFIG_VAL(D, T, shadow_style, STR);
   E_CONFIG_VAL(D, T, visibility_effect, STR);

   *conf_edd = E_CONFIG_DD_NEW("Comp_Config", E_Comp_Config);
#undef T
#undef D
#define T E_Comp_Config
#define D *conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, shadow_style, STR);
   E_CONFIG_VAL(D, T, engine, INT);
   //E_CONFIG_VAL(D, T, max_unmapped_pixels, INT);
   E_CONFIG_VAL(D, T, max_unmapped_time, INT);
   E_CONFIG_VAL(D, T, min_unmapped_time, INT);
   E_CONFIG_VAL(D, T, fps_corner, UCHAR);
   E_CONFIG_VAL(D, T, fps_show, UCHAR);
   E_CONFIG_VAL(D, T, indirect, UCHAR);
   E_CONFIG_VAL(D, T, texture_from_pixmap, UCHAR);
//   E_CONFIG_VAL(D, T, lock_fps, UCHAR);
   E_CONFIG_VAL(D, T, grab, UCHAR);
   E_CONFIG_VAL(D, T, vsync, UCHAR);
   E_CONFIG_VAL(D, T, no_dither, UCHAR);
   E_CONFIG_VAL(D, T, swap_mode, UCHAR);
   //E_CONFIG_VAL(D, T, keep_unmapped, UCHAR);
   E_CONFIG_VAL(D, T, send_flush, UCHAR);
   E_CONFIG_VAL(D, T, send_dump, UCHAR);
   E_CONFIG_VAL(D, T, nocomp_fs, UCHAR);
   E_CONFIG_VAL(D, T, nofade, UCHAR);
   E_CONFIG_VAL(D, T, smooth_windows, UCHAR);
   E_CONFIG_VAL(D, T, first_draw_delay, DOUBLE);
   E_CONFIG_VAL(D, T, enable_advanced_features, UCHAR);
   E_CONFIG_LIST(D, T, match.popups, *match_edd);
   E_CONFIG_LIST(D, T, match.borders, *match_edd);
   E_CONFIG_LIST(D, T, match.overrides, *match_edd);
   E_CONFIG_LIST(D, T, match.menus, *match_edd);
   E_CONFIG_LIST(D, T, match.objects, *match_edd);
}

E_API E_Comp_Config *
e_comp_cfdata_config_new(void)
{
   E_Comp_Config *cfg;
   E_Comp_Match *mat;

   cfg = E_NEW(E_Comp_Config, 1);
   cfg->version = E_COMP_VERSION;
   cfg->shadow_style = eina_stringshare_add("default");
   cfg->engine = E_COMP_ENGINE_SW;
   cfg->max_unmapped_pixels = 32 * 1024;  // implement
   cfg->max_unmapped_time = 10 * 3600; // implement
   cfg->min_unmapped_time = 5 * 60; // implement
   cfg->fps_corner = 0;
   cfg->fps_show = 0;
   cfg->indirect = 0;
   cfg->texture_from_pixmap = 1;
//   cfg->lock_fps = 0;
   cfg->grab = 0;
   cfg->vsync = 1;
   cfg->no_dither = 0;
#ifdef ECORE_EVAS_GL_X11_OPT_SWAP_MODE
   cfg->swap_mode = ECORE_EVAS_GL_X11_SWAP_MODE_AUTO;
#else
   cfg->swap_mode = 0;
#endif
   cfg->keep_unmapped = 1;
   cfg->send_flush = 1; // implement
   cfg->send_dump = 1; // implement
   cfg->nocomp_fs = 1;
   cfg->nofade = 0;
   cfg->smooth_windows = 0; // 1 if gl, 0 if not
   cfg->first_draw_delay = 0.15;

   cfg->match.popups = NULL;

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.popups = eina_list_append(cfg->match.popups, mat);
   mat->name = eina_stringshare_add("shelf");
   mat->shadow_style = eina_stringshare_add("popup");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.popups = eina_list_append(cfg->match.popups, mat);
   mat->name = eina_stringshare_add("_e_popup_desklock");
   mat->shadow_style = eina_stringshare_add("still");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.popups = eina_list_append(cfg->match.popups, mat);
   mat->name = eina_stringshare_add("_e_popup_notification");
   mat->shadow_style = eina_stringshare_add("still");
   mat->no_shadow = 1;
   mat->focus = 1;

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.popups = eina_list_append(cfg->match.popups, mat);
   mat->name = eina_stringshare_add("E Drag");
   mat->shadow_style = eina_stringshare_add("still");
   mat->no_shadow = 1;

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.popups = eina_list_append(cfg->match.popups, mat);
   mat->shadow_style = eina_stringshare_add("popup");

   cfg->match.borders = NULL;

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.borders = eina_list_append(cfg->match.borders, mat);
   mat->fullscreen = 1;
   mat->shadow_style = eina_stringshare_add("fullscreen");

   cfg->match.overrides = NULL;

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->name = eina_stringshare_add("E");
   mat->clas = eina_stringshare_add("everything");
   mat->shadow_style = eina_stringshare_add("everything");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->primary_type = E_WINDOW_TYPE_DROPDOWN_MENU;
   mat->shadow_style = eina_stringshare_add("menu");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->primary_type = E_WINDOW_TYPE_POPUP_MENU;
   mat->shadow_style = eina_stringshare_add("menu");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->primary_type = E_WINDOW_TYPE_COMBO;
   mat->shadow_style = eina_stringshare_add("menu");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->primary_type = E_WINDOW_TYPE_TOOLTIP;
   mat->shadow_style = eina_stringshare_add("menu");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->primary_type = E_WINDOW_TYPE_MENU;
   mat->shadow_style = eina_stringshare_add("menu");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->primary_type = E_WINDOW_TYPE_DND;
   mat->shadow_style = eina_stringshare_add("still");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->primary_type = E_WINDOW_TYPE_DOCK;
   mat->shadow_style = eina_stringshare_add("none");

   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.overrides = eina_list_append(cfg->match.overrides, mat);
   mat->shadow_style = eina_stringshare_add("popup");

   cfg->match.menus = NULL;
   mat = E_NEW(E_Comp_Match, 1);
   cfg->match.menus = eina_list_append(cfg->match.menus, mat);
   mat->shadow_style = eina_stringshare_add("menu");

   return cfg;
}

E_API void
e_comp_cfdata_match_free(E_Comp_Match *m)
{
   if (!m) return;
   eina_stringshare_del(m->title);
   eina_stringshare_del(m->name);
   eina_stringshare_del(m->clas);
   eina_stringshare_del(m->role);
   eina_stringshare_del(m->shadow_style);
   eina_stringshare_del(m->visibility_effect);
   free(m);
}

E_API void
e_comp_cfdata_config_free(E_Comp_Config *cfg)
{
   if (!cfg) return;
   eina_stringshare_del(cfg->shadow_style);

   E_FREE_LIST(cfg->match.popups, e_comp_cfdata_match_free);
   E_FREE_LIST(cfg->match.borders, e_comp_cfdata_match_free);
   E_FREE_LIST(cfg->match.overrides, e_comp_cfdata_match_free);
   E_FREE_LIST(cfg->match.menus, e_comp_cfdata_match_free);
   E_FREE_LIST(cfg->match.objects, e_comp_cfdata_match_free);

   free(cfg);
}

