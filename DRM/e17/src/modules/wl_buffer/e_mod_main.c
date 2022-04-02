#include "e.h"

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Wl_Buffer" };

E_API void *
e_modapi_init(E_Module *m)
{
   int w = 1024, h = 768;

   printf("LOAD Wl_Buffer MODULE\n");

   e_comp->ee = ecore_evas_buffer_new(w, h);

   if (!e_comp->ee)
     {
        ERR("Could not create ecore_evas canvas");
        return NULL;
     }
   e_comp_gl_set(EINA_FALSE);
   elm_config_accel_preference_set("none");
   elm_config_accel_preference_override_set(EINA_TRUE);
   ecore_evas_data_set(e_comp->ee, "comp", e_comp);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!e_comp_wl_init(), NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!e_comp_canvas_init(1024, 768), NULL);

   ecore_event_evas_init();
   ecore_evas_input_event_register(e_comp->ee);

   e_comp_wl_input_pointer_enabled_set(EINA_TRUE);
   e_comp_wl_input_keyboard_enabled_set(EINA_TRUE);
   e_comp_wl_input_touch_enabled_set(EINA_TRUE);

   /* e_comp->pointer =  */
   /*   e_pointer_window_new(ecore_evas_window_get(e_comp->ee), EINA_TRUE); */
   e_comp->pointer = e_pointer_canvas_new(e_comp->ee, EINA_TRUE);
   e_comp->pointer->color = EINA_TRUE;

   e_comp_wl->dmabuf_disable = EINA_TRUE;
   ecore_evas_title_set(e_comp->ee, "Enlightenment: WL-Buffer");
   ecore_evas_pointer_warp(e_comp->ee, w / 2, h / 2);
   ecore_evas_pointer_xy_get(e_comp->ee, &e_comp_wl->ptr.x,
                             &e_comp_wl->ptr.y);
   evas_event_feed_mouse_in(e_comp->evas, 0, NULL);

   return m;
}
