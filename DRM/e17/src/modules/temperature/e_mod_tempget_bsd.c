#include "e.h"
#include "e_mod_main.h"

#if defined (__FreeBSD__) || defined(__DragonFly__) || defined (__OpenBSD__)
static Eina_Lock mons_lock;
static Eina_List *mons = NULL;
#endif

#if defined (__FreeBSD__) || defined(__DragonFly__)
# include <sys/types.h>
# include <sys/sysctl.h>
# include <errno.h>
static const char *sources[] =
{
   "hw.acpi.thermal.tz0.temperature",
   "dev.aibs.0.temp.0",
   "dev.lm75.0.temperature",
   NULL
};

typedef struct
{
   const char  *name;
   const char  *label;
   double       temp;
   int          mib[CTL_MAXNAME];
   unsigned int miblen;
} Temp;

static void
_sysctl_init(void)
{
   size_t len;
   int mib[CTL_MAXNAME];
   char buf[128];

   for (int i = 0; sources[i] != NULL; i++)
     {
        len = 4;
        if (sysctlnametomib(sources[i], mib, &len) != -1)
          {
             Temp *temp = malloc(sizeof(Temp));
             temp->name = eina_stringshare_add(sources[i]);
             temp->label = eina_stringshare_add(sources[i]);
             memcpy(temp->mib, &mib, sizeof(mib));
             temp->miblen = len;
             mons = eina_list_append(mons, temp);
         }
     }
   for (int i = 0; i < 256; i++)
     {
        len = 4;
        snprintf(buf, sizeof(buf), "dev.cpu.%i.temperature", i);
        if (sysctlnametomib(buf, mib, &len) == -1) break;

        Temp *temp = malloc(sizeof(Temp));
        temp->name = eina_stringshare_add(buf);
        temp->label = eina_stringshare_add(buf);
        memcpy(temp->mib, &mib, sizeof(mib));
        temp->miblen = len;
        mons = eina_list_append(mons, temp);
    }
}

static void
_sysctl_update(void)
{
   Eina_List *l;
   Temp *temp;
   int val;
   size_t len = sizeof(val);

   eina_lock_take(&mons_lock);
   EINA_LIST_FOREACH(mons, l, temp)
     {
        if (sysctl(temp->mib, temp->miblen, &val, &len, NULL, 0) != -1)
          temp->temp = (val - 2732) / 10;
     }
   eina_lock_release(&mons_lock);
}

#elif defined (__OpenBSD__)
# include <sys/param.h>
# include <sys/sysctl.h>
# include <sys/sensors.h>
# include <errno.h>
# include <err.h>
typedef struct
{
   const char *name;
   const char *label;
   double      temp;
   int         mib[5];
} Temp;

static void
_sysctl_init(void)
{
   int dev, numt;
   struct sensordev snsrdev;
   struct sensor snsr;
   size_t sdlen = sizeof(snsrdev);
   size_t slen = sizeof(snsr);
   int mib[5] = { CTL_HW, HW_SENSORS, 0, 0, 0 };
   enum sensor_type type;

   for (dev = 0;; dev++)
     {
        mib[2] = dev;
        if (sysctl(mib, 3, &snsrdev, &sdlen, NULL, 0) == -1)
          {
             if (errno == ENOENT) /* no further sensors */
               break;
             else
               continue;
          }
        for (type = 0; type < SENSOR_MAX_TYPES; type++)
          {
             mib[3] = type;
             for (numt = 0; numt < snsrdev.sensors_count; numt++)
               {
                  mib[4] = numt;
                  slen = sizeof(snsr);
                  if (sysctl(mib, 5, &snsr, &slen, NULL, 0) == -1)
                    continue;
                  if ((!slen) || (snsr.flags & SENSOR_FINVALID))
                    continue;
                  if (snsr.type != SENSOR_TEMP)
                    continue;

                  Temp *temp = malloc(sizeof(Temp));
                  if (temp)
                    {
                       char buf[64];
                       temp->name = eina_stringshare_add(snsrdev.xname);
                       snprintf(buf, sizeof(buf), "%s.temp%i", snsrdev.xname, numt);
                       temp->label = eina_stringshare_add(buf);
                       memcpy(temp->mib, &mib, sizeof(mib));
                       mons = eina_list_append(mons, temp);
                    }
               }
          }
     }
}

static void
_sysctl_update(void)
{
   Eina_List *l;
   Temp *temp;
   struct sensor snsr;
   size_t slen = sizeof(struct sensor);

   eina_lock_take(&mons_lock);
   EINA_LIST_FOREACH(mons, l, temp)
     {
        if (sysctl(temp->mib, 5, &snsr, &slen, NULL, 0) != -1)
          temp->temp = (snsr.value - 273150000) / 1000000.0;
     }
   eina_lock_release(&mons_lock);
}
#endif

#if defined (__FreeBSD__) || defined(__DragonFly__) || defined (__OpenBSD__)
static void
init(Tempthread *tth)
{
   if (tth->initted) return;
   tth->initted = EINA_TRUE;

   if (((!tth->sensor_name) || (tth->sensor_name[0] == 0)))
     {
        eina_stringshare_replace(&(tth->sensor_name), NULL);
        if (!tth->sensor_name)
          {
             Eina_List *l;
             Temp *temp;

             eina_lock_take(&mons_lock);
             EINA_LIST_FOREACH(mons, l, temp)
               {
                  tth->sensor_name = eina_stringshare_add(temp->name);
                  break;
               }
             eina_lock_release(&mons_lock);
          }
     }
}

static int
check(Tempthread *tth)
{
   Eina_List *l;
   Temp *temp;
   double t = 0.0;

   _sysctl_update();
   if (!tth->sensor_name) return -999;

   eina_lock_take(&mons_lock);
   EINA_LIST_FOREACH(mons, l, temp)
     {
        if (!strcmp(tth->sensor_name, temp->name))
          {
             t = temp->temp;
             break;
          }
     }
   eina_lock_release(&mons_lock);
   return t;
}

int
temperature_tempget_get(Tempthread *tth)
{
   init(tth);
   return check(tth);
}

Eina_List *
temperature_tempget_sensor_list(void)
{
   Eina_List *sensors = NULL, *l;
   Sensor *sen;
   Temp *temp;

   EINA_LIST_FOREACH(mons, l, temp)
     {
        sen = calloc(1, sizeof(Sensor));
        sen->name = eina_stringshare_add(temp->name);
        sen->label = eina_stringshare_add(temp->label);
        sensors = eina_list_append(sensors, sen);
     }
   return sensors;
}

void
temperature_tempget_setup(void)
{
   eina_lock_new(&mons_lock);
   _sysctl_init();
}

void
temperature_tempget_clear(void)
{
   Temp *temp;

   eina_lock_take(&mons_lock);
   EINA_LIST_FREE(mons, temp)
     {
        eina_stringshare_replace(&(temp->name), NULL);
        eina_stringshare_replace(&(temp->label), NULL);
        free(temp);
     }
   eina_lock_release(&mons_lock);
   // extra lock take to cover race cond for thread
   eina_lock_take(&mons_lock);
   eina_lock_release(&mons_lock);
   eina_lock_free(&mons_lock);
}
#endif
