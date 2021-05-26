#include <Ecore_X.h>

static Ecore_X_Window a = 0;

static Eina_Bool
test_state(void *d EINA_UNUSED)
{
   int w, h;

   ecore_x_window_geometry_get(a, NULL, NULL, &w, &h);
   if ((w == 100) || (h == 100))
     fprintf(stderr, "FAIL\n");
   return EINA_FALSE;
}

int
main(void)
{


   ecore_x_init(NULL);

   a = ecore_x_window_override_new(0, 0, 0, 100, 100);
   ecore_x_window_show(a);
   ecore_x_netwm_state_request_send(a, 0,
                                      ECORE_X_WINDOW_STATE_FULLSCREEN, -1, 1);
   ecore_x_icccm_name_class_set(a, "override_fullscreen", "test");
   ecore_timer_loop_add(1.0, test_state, NULL);
   ecore_main_loop_begin();
   return 0;
}
