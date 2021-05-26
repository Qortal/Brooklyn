#include <Ecore.h>
#include <Ecore_Evas.h>

static Eina_Bool
_focus(Ecore_Evas *ee)
{
   ecore_evas_focus_set(ee, 1);
   return 1;
}

int
main(void)
{
   Ecore_Evas *ee;

   ecore_evas_init();
   ee = ecore_evas_new(NULL, 0, 0, 200, 200, NULL);
   ecore_evas_show(ee);
   ecore_timer_loop_add(3.0, (Ecore_Task_Cb)_focus, ee);
   ecore_main_loop_begin();
   return 0;
}
