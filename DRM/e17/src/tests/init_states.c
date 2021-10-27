#include <Ecore_Evas.h>
#include <Ecore_X.h>



int
main(void)
{
   Ecore_Evas *ee;

   ecore_evas_init();
   ee = ecore_evas_new(NULL, 0, 0, 200, 200, NULL);
   ecore_evas_show(ee);
   ecore_evas_urgent_set(ee, 1);
   ecore_evas_iconified_set(ee, 1);
   ecore_evas_sticky_set(ee, 1);
   ecore_x_netwm_user_time_set(ecore_evas_window_get(ee), 0);
   ecore_main_loop_begin();
   return 0;
}
