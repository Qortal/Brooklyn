#include "e.h"

EINTERN int
e_deskenv_init(void)
{
#ifndef HAVE_WAYLAND_ONLY
   char buf[PATH_MAX], buf2[PATH_MAX + sizeof("xrdb -override ")];

   // run xdrb -load .Xdefaults & .Xresources
   // NOTE: one day we should replace this with an e based config + service
   if (e_config->deskenv.load_xrdb)
     {
        Eina_Bool exists = EINA_FALSE;

        e_user_homedir_concat(buf, sizeof(buf), ".Xdefaults");
        exists = ecore_file_exists(buf);
        if (!exists)
          {
             e_user_homedir_concat(buf, sizeof(buf), ".Xresources");
             exists = ecore_file_exists(buf);
          }
        if (exists)
          {
#if 1
             snprintf(buf2, sizeof(buf2), "xrdb -override %s", buf);
             if (system(buf2) != 0)
               fprintf(stderr, "Execution of [%s] did not exit cleanly\n", buf2);
#else
             // while this SHOULD work.. it ends up with mysterious problems
             // inside xlib that i seem to not be able to trap easily...
             ecore_x_rersource_load(buf);
             ecore_x_resource_db_flush();
#endif
          }
     }

   e_deskenv_xmodmap_run();
   // make gnome apps happy
   // NOTE: one day we should replace this with an e based config + service
   if (e_config->deskenv.load_gnome)
     {
        ecore_exe_run("gnome-settings-daemon", NULL);
     }

   // make kde apps happy
   // NOTE: one day we should replace this with an e based config + service ??
   if (e_config->deskenv.load_kde)
     {
        ecore_exe_run("kdeinit", NULL);
     }
#endif
   return 1;
}

EINTERN int
e_deskenv_shutdown(void)
{
   return 1;
}

E_API void
e_deskenv_xmodmap_run(void)
{
#ifndef HAVE_WAYLAND_ONLY
   char buf[PATH_MAX], buf2[PATH_MAX + sizeof("xmodmap ")];
   // load ~/.Xmodmap
   // NOTE: one day we should replace this with an e based config + service
   if (!e_config->deskenv.load_xmodmap) return;
   e_user_homedir_concat(buf, sizeof(buf), ".Xmodmap");
   if (!ecore_file_exists(buf)) return;
   snprintf(buf2, sizeof(buf2), "xmodmap %s", buf);
   ecore_exe_run(buf2, NULL);
#endif
}

