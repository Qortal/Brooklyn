/* Setup if we need cpufreq? */
#include "e_wizard.h"
#include "e_wizard_api.h"

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifndef __FreeBSD__
static char *
read_file(const char *file)
{
   FILE *f = fopen(file, "r");
   size_t len;
   char buf[4096], *p;
   if (!f) return NULL;
   len = fread(buf, 1, sizeof(buf) - 1, f);
   if (len == 0)
     {
        fclose(f);
        return NULL;
     }
   buf[len] = 0;
   for (p = buf; *p; p++)
     {
        if (p[0] == '\n') p[0] = 0;
     }
   fclose(f);
   return strdup(buf);
}
#endif
/*
E_API int
wizard_page_init(E_Wizard_Page *pg EINA_UNUSED, Eina_Bool *need_xdg_desktops EINA_UNUSED, Eina_Bool *need_xdg_icons EINA_UNUSED)
{
   return 1;
}

E_API int
wizard_page_shutdown(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   int hav_cpufreq = 0;
#ifdef __FreeBSD__
   char buf[PATH_MAX];
   size_t len = 0;

   len = sizeof(buf);
   if (sysctlbyname("dev.cpu.0.freq_levels", buf, &len, NULL, 0) == 0)
     hav_cpufreq = 1;
#else
   char *str, *p;

   str = read_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies");
   if (str)
     {
        for (p = str; *p; p++)
          {
             if (isspace(*p))
               {
                  hav_cpufreq = 1;
                  break;
               }
          }
        free(str);
     }
#endif
   if (!hav_cpufreq)
     {
        E_Config_Module *em;
        Eina_List *l;

        EINA_LIST_FOREACH(e_config->modules, l, em)
          {
             if (!em->name) continue;
             if (!strcmp(em->name, "cpufreq"))
               {
                  e_config->modules = eina_list_remove_list
                      (e_config->modules, l);
                  if (em->name) eina_stringshare_del(em->name);
                  free(em);
                  break;
               }
          }
        e_config_save_queue();
     }
   return 0; /* 1 == show ui, and wait for user, 0 == just continue */
}
/*
E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}

E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
