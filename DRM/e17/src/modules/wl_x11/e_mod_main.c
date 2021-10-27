#include "e.h"
#include <Ecore_X.h>

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Wl_X11" };

static Ecore_Event_Handler *kbd_hdlr;

E_API void *
e_modapi_init(E_Module *m)
{
   int w = 0, h = 0;
   Ecore_X_Window root, win, win2;
   Eina_Bool managed = EINA_FALSE;

   printf("LOAD WL_X11 MODULE\n");

   if (!ecore_x_init(NULL))
     {
        fprintf(stderr, "X11 connect failed!\n");
        return NULL;
     }
   root = ecore_x_window_root_first_get();

   /* get root and setup canvas first */
   e_comp_x_randr_canvas_new(root, 1, 1);

   /* then check if it's 'managed' or not */
   if (ecore_x_window_prop_window_get(root,
                                      ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK,
                                      &win, 1) == 1)
     {
        if (ecore_x_window_prop_window_get(win,
                                           ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK,
                                           &win2, 1) == 1)
          {
             if (win == win2) managed = EINA_TRUE;
          }
     }
   if (managed) e_randr2_stop();

   if (!e_comp->ee)
     {
        ERR("Could not create ecore_evas canvas");
        return NULL;
     }
   ecore_evas_title_set(e_comp->ee, "Enlightenment: WL-X11");
   ecore_evas_name_class_set(e_comp->ee, "E", "compositor");

   ecore_evas_screen_geometry_get(e_comp->ee, NULL, NULL, &w, &h);
   if (managed) e_comp_x_randr_screen_iface_set();
   if (!e_comp_wl_init()) return NULL;
   if (managed)
     {
        w = (w * 2) / 3;
        h = (h * 2) / 3;
     }
   if (!e_comp_canvas_init(w, h)) return NULL;

   e_comp_wl_input_pointer_enabled_set(EINA_TRUE);
   e_comp_wl_input_keyboard_enabled_set(EINA_TRUE);
   e_comp_wl_input_touch_enabled_set(EINA_TRUE);

   /* e_comp->pointer =  */
   /*   e_pointer_window_new(ecore_evas_window_get(e_comp->ee), EINA_TRUE); */
   e_comp->pointer = e_pointer_canvas_new(e_comp->ee, EINA_TRUE);
   e_comp->pointer->color = EINA_TRUE;

   e_comp_wl->dmabuf_disable = EINA_TRUE;

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   /* delete handler for keymap change */
   if (kbd_hdlr) ecore_event_handler_del(kbd_hdlr);
   ecore_x_shutdown();

   return 1;
}
