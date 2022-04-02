/* Setup if we need temperature? */
#include "e_wizard.h"
#include "e_wizard_api.h"

#ifdef __FreeBSD__
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

#ifdef HAVE_EEZE
# include <Eeze.h>
#endif
/*
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
 */

#if defined (__FreeBSD__) || defined (__OpenBSD__)
#else
static Eina_List *
_wizard_temperature_get_bus_files(const char *bus)
{
   Eina_List *result;
   Eina_List *therms;
   char path[PATH_MAX + PATH_MAX + 3];
   char busdir[PATH_MAX];
   char *name;

   result = NULL;

   snprintf(busdir, sizeof(busdir), "/sys/bus/%s/devices", bus);
   /* Look through all the devices for the given bus. */
   therms = ecore_file_ls(busdir);

   EINA_LIST_FREE(therms, name)
     {
        Eina_List *files;
        char *file;

        /* Search each device for temp*_input, these should be
         * temperature devices. */
        snprintf(path, sizeof(path), "%s/%s", busdir, name);
        files = ecore_file_ls(path);
        EINA_LIST_FREE(files, file)
          {
             if ((!strncmp("temp", file, 4)) &&
                 (!strcmp("_input", &file[strlen(file) - 6])))
               {
                  char *f;

                  snprintf(path, sizeof(path),
                           "%s/%s/%s", busdir, name, file);
                  f = strdup(path);
                  if (f) result = eina_list_append(result, f);
               }
             free(file);
          }
        free(name);
     }
   return result;
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
#ifdef HAVE_EEZE
   Eina_List *tempdevs = NULL;
#endif
   int hav_temperature = 0;
#if defined (__FreeBSD__) || defined (__OpenBSD__)
   // figure out on bsd if we have temp sensors
#else
   const char *sensor_path[] = {
      "/proc/omnibook/temperature",
      "/proc/acpi/thermal_zone",    //LINUX_ACPI Directory
      "/sys/class/thermal",         //LINUX_SYS Directory
      "/sys/devices/temperatures/cpu_temperature",
      "/sys/devices/temperatures/sensor1_temperature",
      "/sys/devices/platform/coretemp.0/temp1_input",
      "/sys/devices/platform/thinkpad_hwmon/temp1_input",
      NULL
   };

#ifdef HAVE_EEZE
   tempdevs = eeze_udev_find_by_type(EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR, NULL);
#endif
   if (tempdevs && (eina_list_count(tempdevs)))
     hav_temperature = 1;
   else
     {
        int i = 0;

        while(sensor_path[i] != NULL)
          {
             if (ecore_file_exists(sensor_path[i]))
               {
                  hav_temperature = 1;
                  break;
               }
             i++;
          }

        if (!hav_temperature)
          {
             Eina_List *therms;

             therms = _wizard_temperature_get_bus_files("i2c");
             if (therms)
               {
                  char *name;
                  if ((name = eina_list_data_get(therms)))
                    {
                       if (ecore_file_exists(name))
                         {
                            hav_temperature = 1;
                         }
                    }
                  eina_list_free(therms);
               }
          }

        if (!hav_temperature)
          {
             Eina_List *therms;

             therms = _wizard_temperature_get_bus_files("pci");
             if (therms)
               {
                  char *name;
                  if ((name = eina_list_data_get(therms)))
                    {
                       if (ecore_file_exists(name))
                         {
                            hav_temperature = 1;
                         }
                    }
                  eina_list_free(therms);
               }
          }
     }
#endif
   if (!hav_temperature)
     {
        E_Config_Module *em;
        Eina_List *l;

        EINA_LIST_FOREACH(e_config->modules, l, em)
          {
             if (!em->name) continue;
             if (!strcmp(em->name, "temperature"))
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
