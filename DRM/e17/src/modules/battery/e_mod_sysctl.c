#include "e.h"
#include "e_mod_main.h"

#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
# include <err.h>
# include <sys/types.h>
# include <sys/sysctl.h>

#if defined(__FreeBSD__) || defined(__DragonFly__)
# include <sys/ioctl.h>
# include <dev/acpica/acpiio.h>
#endif

#if defined(__OpenBSD__)
# include <sys/param.h>
# include <sys/sensors.h>
#endif

static Eina_Bool   _battery_sysctl_battery_update_poll(void *data EINA_UNUSED);
static int         _battery_sysctl_battery_update();

extern Eina_List  *device_batteries;
extern Eina_List  *device_ac_adapters;
extern double      init_time;

static Ac_Adapter *ac = NULL;

int
_battery_sysctl_start(void)
{
   Battery *bat;

   if (device_batteries) return 1;

# if defined(__OpenBSD__)
   int mib[] = {CTL_HW, HW_SENSORS, 0, 0, 0};
   int devn;
   struct sensordev snsrdev;
   size_t sdlen = sizeof(struct sensordev);

   for (devn = 0;; devn++)
     {
        mib[2] = devn;
        if (sysctl(mib, 3, &snsrdev, &sdlen, NULL, 0) == -1)
          {
             if (errno == ENXIO)
               continue;
             if (errno == ENOENT)
               break;
          }

        if (!strncmp("acpibat", snsrdev.xname, 7))
          {
             if (!(bat = E_NEW(Battery, 1)))
               return 0;
             bat->udi = eina_stringshare_add(snsrdev.xname);
             bat->mib = malloc(sizeof(int) * 5);
             if (!bat->mib) return 0;
             bat->mib[0] = mib[0];
             bat->mib[1] = mib[1];
             bat->mib[2] = mib[2];
             bat->technology = eina_stringshare_add("Unknown");
             bat->model = eina_stringshare_add("Unknown");
             bat->vendor = eina_stringshare_add("Unknown");
             bat->poll = ecore_poller_add(ECORE_POLLER_CORE,
                                          battery_config->poll_interval,
                                          _battery_sysctl_battery_update_poll, NULL);
             device_batteries = eina_list_append(device_batteries, bat);
          }
        else if (!strcmp("acpiac0", snsrdev.xname))
          {
             if (!(ac = E_NEW(Ac_Adapter, 1)))
               return 0;
             ac->udi = eina_stringshare_add("acpiac0");
             ac->mib = malloc(sizeof(int) * 5);
             if (!ac->mib) return 0;
             ac->mib[0] = mib[0];
             ac->mib[1] = mib[1];
             ac->mib[2] = mib[2];
             device_ac_adapters = eina_list_append(device_ac_adapters, ac);
          }
     }
# elif defined(__FreeBSD__) || defined(__DragonFly__)
   int fd, n_units, val;
   union acpi_battery_ioctl_arg battio;
   char buf[128];

   if ((fd = open("/dev/acpi", O_RDONLY)) == -1) return 0;

   if (ioctl(fd, ACPIIO_BATT_GET_UNITS, &n_units) == -1) return 0;

   for (int i = 0; i < n_units; i++)
     {
        battio.unit = i;
        if (ioctl(fd, ACPIIO_BATT_GET_BIX, &battio) != -1)
          {
             if (battio.bst.state == ACPI_BATT_STAT_NOT_PRESENT)
               continue;

             if (!(bat = E_NEW(Battery, 1)))
               return 0;

             snprintf(buf, sizeof(buf), "hw.acpi.battery.%i", i);
             bat->udi = eina_stringshare_add(buf);
             bat->technology = eina_stringshare_add(battio.bix.type);
             bat->vendor = eina_stringshare_add(battio.bix.oeminfo);
             bat->model = eina_stringshare_add(battio.bix.model);
             bat->design_charge = battio.bix.dcap;
             bat->last_full_charge = battio.bix.lfcap;
             bat->poll = ecore_poller_add(ECORE_POLLER_CORE,
                                          battery_config->poll_interval,
                                          _battery_sysctl_battery_update_poll, NULL);
             bat->unit = i;
             device_batteries = eina_list_append(device_batteries, bat);
          }
     }

    if (ioctl(fd, ACPIIO_ACAD_GET_STATUS, &val) != -1)
      {
         if (!(ac = E_NEW(Ac_Adapter, 1)))
           return 0;

         ac->udi = eina_stringshare_add("hw.acpi.acline");
         ac->present = val;
         device_ac_adapters = eina_list_append(device_ac_adapters, ac);
      }
    close(fd);
# endif
   _battery_sysctl_battery_update();

   if (bat)
     bat->last_update = ecore_time_get();

   return 1;
}

void
_battery_sysctl_stop(void)
{
   Battery *bat;

   if (ac)
     {
        eina_stringshare_del(ac->udi);
#if defined(__OpenBSD__)
        E_FREE(ac->mib);
#endif
        E_FREE(ac);
     }

   EINA_LIST_FREE(device_batteries, bat)
     {
        eina_stringshare_del(bat->udi);
        eina_stringshare_del(bat->technology);
        eina_stringshare_del(bat->model);
        eina_stringshare_del(bat->vendor);
        ecore_poller_del(bat->poll);
#if defined(__OpenBSD__)
        E_FREE(bat->mib);
#endif
        E_FREE(bat);
     }
}

static Eina_Bool
_battery_sysctl_battery_update_poll(void *data EINA_UNUSED)
{
   Battery *bat;
   Eina_List *l;

   EINA_LIST_FOREACH(device_batteries, l, bat)
     _battery_sysctl_battery_update(bat);

   return EINA_TRUE;
}

static int
_battery_sysctl_battery_update(Battery *bat)
{
   double _time;
# if defined(__OpenBSD__)
   double charge;
   struct sensor s;
   size_t slen = sizeof(struct sensor);
# elif defined(__FreeBSD__) || defined(__DragonFly__)
   int fd, val;
   union acpi_battery_ioctl_arg battio;
# endif

   if (bat)
     {
       /* update the poller interval */
       ecore_poller_poller_interval_set(bat->poll,
                                        battery_config->poll_interval);
# if defined(__OpenBSD__)
       charge = 0;

       /* last full capacity */
       bat->mib[3] = SENSOR_WATTHOUR;
       bat->mib[4] = 0;
       if (sysctl(bat->mib, 5, &s, &slen, NULL, 0) != -1)
         {
            bat->last_full_charge = (double)s.value;
         }

       /* remaining capacity */
       bat->mib[3] = SENSOR_WATTHOUR;
       bat->mib[4] = 3;
       if (sysctl(bat->mib, 5, &s, &slen, NULL, 0) != -1)
         {
            charge = (double)s.value;
         }

       if ((EINA_FLT_EQ(charge, 0.0)) || (EINA_FLT_EQ(bat->last_full_charge, 0.0)))
         {
            /* last full capacity */
            bat->mib[3] = SENSOR_AMPHOUR;
            bat->mib[4] = 0;
            if (sysctl(bat->mib, 5, &s, &slen, NULL, 0) != -1)
              {
                 bat->last_full_charge = (double)s.value;
              }

            /* remaining capacity */
            bat->mib[3] = SENSOR_AMPHOUR;
            bat->mib[4] = 3;
            if (sysctl(bat->mib, 5, &s, &slen, NULL, 0) != -1)
              {
                 charge = (double)s.value;
              }
         }

       bat->got_prop = 1;

       _time = ecore_time_get();
       if ((bat->got_prop) && (!EINA_FLT_EQ(charge, bat->current_charge)))
         bat->charge_rate = ((charge - bat->current_charge) / (_time - bat->last_update));
       bat->last_update = _time;
       bat->current_charge = charge;
       bat->percent = 100 * (bat->current_charge / bat->last_full_charge);
       if (bat->current_charge >= bat->last_full_charge)
         bat->percent = 100;

       if (bat->got_prop)
         {
            if (bat->charge_rate > 0)
              {
                 if (battery_config->fuzzy && (++battery_config->fuzzcount <= 10) && (bat->time_full > 0))
                   bat->time_full = (((bat->last_full_charge - bat->current_charge) / bat->charge_rate) + bat->time_full) / 2;
                 else
                   bat->time_full = (bat->last_full_charge - bat->current_charge) / bat->charge_rate;
                 bat->time_left = -1;
              }
            else
              {
                 if (battery_config->fuzzy && (battery_config->fuzzcount <= 10) && (bat->time_left > 0))
                   bat->time_left = (((0 - bat->current_charge) / bat->charge_rate) + bat->time_left) / 2;
                 else
                   bat->time_left = (0 - bat->current_charge) / bat->charge_rate;
                 bat->time_full = -1;
              }
         }
       else
         {
            bat->time_full = -1;
            bat->time_left = -1;
         }

       /* battery state 1: discharge, 2: charge */
       bat->mib[3] = 10;
       bat->mib[4] = 0;
       if (sysctl(bat->mib, 5, &s, &slen, NULL, 0) == -1)
         {
            if (s.value == 2)
              bat->charging = 1;
            else
              bat->charging = 0;
         }

# elif defined(__FreeBSD__) || defined(__DragonFly__)
       if ((fd = open("/dev/acpi", O_RDONLY)) == -1) return 0;

       battio.unit = bat->unit;
       if (ioctl(fd, ACPIIO_BATT_GET_BATTINFO, &battio) == -1)
         {
            if (fd != -1)
              close(fd);
            return 0;
         }
       bat->got_prop = 1;

       bat->percent = battio.battinfo.cap;
       _time = ecore_time_get();
       bat->last_update = _time;

       bat->charging = (battio.battinfo.state == ACPI_BATT_STAT_CHARGING) ? 1 : 0;
       bat->time_min = bat->time_full = battio.battinfo.min;

       if (bat->time_min >= 0) bat->time_left = bat->time_min * 60;
       close(fd);
# endif
   }

   if (ac)
     {
# if defined(__OpenBSD__)
       /* AC State */
       ac->mib[3] = 9;
       ac->mib[4] = 0;
       if (sysctl(ac->mib, 5, &s, &slen, NULL, 0) != -1)
         {
            if (s.value)
              ac->present = 1;
            else
              ac->present = 0;
         }
# elif defined(__FreeBSD__) || defined(__DragonFly__)
       if ((fd = open("/dev/acpi", O_RDONLY)) == -1) return 0;
       if (ioctl(fd, ACPIIO_ACAD_GET_STATUS, &val) == -1)
         {
            close(fd);
            return 0;
         }
       ac->present = val;
       close(fd);
# endif
     }

   if (bat)
     {
        if (bat->got_prop)
          _battery_device_update();
        bat->got_prop = 1;
     }
   return 1;
}
#endif



