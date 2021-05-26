#include "e.h"
#ifdef USE_MODULE_WL_DRM
# include <Ecore_Drm2.h>
#endif

E_API int
e_mouse_update(void)
{
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     e_comp_x_devices_config_apply(EINA_TRUE);
#endif

#ifdef USE_MODULE_WL_DRM
   if (strstr(ecore_evas_engine_name_get(e_comp->ee), "drm"))
     {
        Ecore_Drm2_Device *dev;

        dev = ecore_evas_data_get(e_comp->ee, "device");
        if (dev)
          {
             ecore_drm2_device_pointer_left_handed_set(dev, (Eina_Bool)!e_config->mouse_hand);
             ecore_drm2_device_pointer_accel_speed_set(dev, e_config->mouse_accel);
             ecore_drm2_device_touch_tap_to_click_enabled_set(dev, e_config->touch_tap_to_click);
          }
     }
#endif
   return 1;
}
