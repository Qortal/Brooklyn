#include <Ecore_X.h>

static Ecore_X_Window a;
static unsigned int i, j;
static Ecore_X_Rectangle rects[10000];

static Eina_Bool
_shape(void *d EINA_UNUSED)
{
   unsigned int num = (i * 100) + j;
   fprintf(stderr, "%u: %d,%d %dx%d\n", i, j, 1, 1);
   rects[num].x = i;
   rects[num].y = j;
   rects[num].width = 1;
   rects[num].height = 1;
   ecore_x_window_shape_input_rectangles_set(a, (Ecore_X_Rectangle*)rects, 10000);
   j++;
   if (j == 100)
     {
        i++;
        j = 0;
     }
   if (i == 100)
     {
        i = 0;
        memset(&rects, 0, sizeof(rects));
        ecore_x_window_shape_input_rectangles_set(a, (Ecore_X_Rectangle*)rects, 10000);
     }
   return EINA_TRUE;
}

int
main(void)
{
   ecore_x_init(NULL);

   a = ecore_x_window_input_new(0, 0, 0, 100, 100);
   ecore_x_window_show(a);

   ecore_timer_loop_add(0.01, _shape, NULL);


   ecore_x_icccm_name_class_set(a, "shaped_input", "test");
   ecore_main_loop_begin();
   return 0;
}
