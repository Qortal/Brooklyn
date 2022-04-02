#include "e.h"

E_API double e_scale = 1.0;

EINTERN int
e_scale_init(void)
{
   e_scale_update();
   return 1;
}

EINTERN int
e_scale_shutdown(void)
{
   return 1;
}

E_API double
e_scale_dpi_get(void)
{
   // this is a general dpi across all screens thing
   if ((e_randr2) && (e_randr2->screens))
     {
        Eina_List *l;
        E_Randr2_Screen *sc;
        double total_dpi = 0.0;
        int total_screens = 0;

        EINA_LIST_FOREACH(e_randr2->screens, l, sc)
          {
             if ((sc->config.enabled) &&
                 (sc->config.mode.w > 0) && (sc->config.mode.h > 0) &&
                 (sc->info.size.w > 0) && (sc->info.size.h > 0))
               {
                  double dpi = ((((double)sc->config.mode.w * 25.4) /
                                 (double)sc->info.size.w) +
                                (((double)sc->config.mode.h * 25.4) /
                                 (double)sc->info.size.h)) / 2.0;
                  total_dpi += dpi;
                  total_screens++;
               }
          }
        if ((total_dpi > 0.0) && (total_screens > 0))
          return total_dpi / (double)total_screens;
     }
   // fall back to old way
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        int x_core_dpi = ecore_x_dpi_get();
        return x_core_dpi;
     }
#endif
#ifdef HAVE_WAYLAND
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     {
        double dpi;
        int xdpi = 0, ydpi = 0;

        ecore_evas_screen_dpi_get(e_comp->ee, &xdpi, &ydpi);
        if (xdpi == 0) xdpi = 75;
        if (ydpi == 0) ydpi = 75;
        dpi = ((double)(xdpi + ydpi) / 2.0);
        return dpi;
     }
#endif
   return 75.0;
}

E_API void
e_scale_update(void)
{
   char buf[128];

   if (e_config->scale.use_dpi)
     {
        e_scale = e_scale_dpi_get() / (double)e_config->scale.base_dpi;
        if      (e_scale > e_config->scale.max) e_scale = e_config->scale.max;
        else if (e_scale < e_config->scale.min) e_scale = e_config->scale.min;
     }
   else if (e_config->scale.use_custom)
     {
        e_scale = e_config->scale.factor;
        if      (e_scale > e_config->scale.max) e_scale = e_config->scale.max;
        else if (e_scale < e_config->scale.min) e_scale = e_config->scale.min;
     }
   elm_config_scale_set(e_scale);
   elm_config_all_flush();
   edje_scale_set(e_scale);
   snprintf(buf, sizeof(buf), "%1.3f", e_scale);
   e_util_env_set("E_SCALE", buf);
   e_hints_scale_update();
   e_pointers_size_set(e_config->cursor_size);
#ifndef HAVE_WAYLAND_ONLY
   e_xsettings_config_update();
   if (e_config->scale.set_xapp_dpi)
     {
        snprintf(buf, sizeof(buf), "%i",
                 (int)((double)e_config->scale.xapp_base_dpi * e_scale));
        ecore_x_resource_db_string_set("Xft.dpi", buf);
        ecore_x_resource_db_string_set("Xft.hinting", "1");
        ecore_x_resource_db_string_set("Xft.antialias", "1");
        ecore_x_resource_db_string_set("Xft.autohint", "0");
        ecore_x_resource_db_string_set("Xft.hintstyle", "hintfull");
        ecore_x_resource_db_string_set("Xft.rgba", "none");
        ecore_x_resource_db_flush();
     }
#endif
}
