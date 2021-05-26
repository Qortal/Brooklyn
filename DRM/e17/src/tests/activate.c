#include <Ecore_Evas.h>

int
main(void)
{
   Ecore_Evas *ee;

   ecore_evas_init();
   ee = ecore_evas_new(NULL, 0, 0, 200, 200, NULL);
   ecore_evas_name_class_set(ee, "activate_test", "activate_test");
   ecore_evas_show(ee);
   ecore_evas_activate(ee);
   ecore_main_loop_begin();
   return 0;
}
