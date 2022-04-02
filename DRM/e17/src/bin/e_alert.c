#include "e.h"
#include <sys/wait.h>

/* public variables */
E_API unsigned long _e_alert_composite_win = 0;

EINTERN int
e_alert_init(void)
{
   return 1;
}

EINTERN int
e_alert_shutdown(void)
{
   return 1;
}

E_API void
e_alert_composite_win(Ecore_X_Window root, Ecore_X_Window comp)
{
#ifndef HAVE_WAYLAND_ONLY
   Ecore_X_Atom composite_win;

   composite_win = ecore_x_atom_get("_E_COMP_WINDOW");

   if (comp == 0)
     ecore_x_window_prop_property_del(root, composite_win);
   else
     ecore_x_window_prop_card32_set(root, composite_win, &comp, 1);
#else
   (void)root;
   (void)comp;
#endif
}

E_API void
e_alert_show(void)
{
   if (!e_nopause)
     {
        fprintf(stderr, "PAUSE !\n");
        pause();
     }
}

