#include <Ecore_X.h>

int
main(void)
{
   Ecore_X_Window a, b;

   ecore_x_init(NULL);

   a = ecore_x_window_new(0, 0, 0, 100, 100);
   ecore_x_window_show(a);

   b = ecore_x_window_new(0, 200, 200, 100, 100);
   ecore_x_window_show(b);
   ecore_x_icccm_transient_for_set(b, a);
   ecore_x_netwm_state_request_send(b, 0,
                                      ECORE_X_WINDOW_STATE_MODAL, -1, 1);
   ecore_x_icccm_name_class_set(b, "modal", "test");
   ecore_main_loop_begin();
   return 0;
}
