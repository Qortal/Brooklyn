#include <Ecore_Evas.h>

static void
_focus(Ecore_Evas *ee)
{
   ecore_evas_urgent_set(ee, 0);
}

static void
_focus_out(Ecore_Evas *ee)
{
   ecore_evas_urgent_set(ee, 1);
}

int
main(void)
{
   Ecore_Evas *ee;

   ecore_evas_init();
   ee = ecore_evas_new(NULL, 0, 0, 200, 200, NULL);
   ecore_evas_callback_focus_in_set(ee, _focus);
   ecore_evas_callback_focus_out_set(ee, _focus_out);
   ecore_evas_show(ee);
   ecore_evas_urgent_set(ee, 1);
   ecore_main_loop_begin();
   return 0;
}
