#include <Ecore.h>
#include <Ecore_Input.h>
#include <Ecore_Evas.h>

static Eina_Bool
fs(void *ee)
{
   if (!ecore_evas_fullscreen_get(ee))
     ecore_evas_fullscreen_set(ee, 1);
   return EINA_TRUE;
}

static Eina_Bool
key(void *ee, int t EINA_UNUSED, Ecore_Event_Key *ev)
{
   if (!strcmp(ev->key, "Escape"))
     ecore_evas_fullscreen_set(ee, 0);
   else
     fprintf(stderr, "%s\n", ev->key);
   return ECORE_CALLBACK_RENEW;
}

int
main(void)
{
   Ecore_Evas *ee;

   ecore_evas_init();
   ee = ecore_evas_new(NULL, 0, 0, 200, 200, NULL);
   ecore_timer_loop_add(2, fs, ee);
   ecore_evas_show(ee);
   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, (Ecore_Event_Handler_Cb)key, ee);
   ecore_main_loop_begin();
   return 0;
}
