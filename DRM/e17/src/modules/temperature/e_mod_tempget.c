#include "e.h"
#include "e_mod_main.h"

#if defined (__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
#else
// https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface
typedef enum
{
   TEMP_UNKNOWN,
   TEMP_CPU, // CPU embedded diode
   TEMP_TRANSISTOR, // 3904 transistor
   TEMP_THERMAL, // thermal diode
   TEMP_THERMISTOR, // thermistor
   TEMP_AMD_AMDSI, // AMD AMDSI
   TEMP_INTEL_PECI // Intel PECI
} Temp_Type;

typedef enum
{
   PWM_OFF, // full speed
   PWM_ON, // enabled
   PWM_AUTO // automatic mode
} Pwm_Mode;

typedef struct
{
   Temp_Type type;
   const char *path;
   const char *name;
   const char *label;
   int enable;
   int fault;
   double temp;
   double min;
   double max;
   double crit;
   double emergency;
   double offset;
} Temp;

typedef struct
{
   const char *path;
   const char *name;
   const char *label;
   int enable;
   int fault;
   int rpm;
   int min;
   int max;
   int target;
} Fan;

typedef struct
{
   const char *path;
   const char *name;
   Pwm_Mode mode;
   double val;
   double min;
   double max;
} Pwm;

typedef struct
{
   const char *path;
   const char *name;
   const char *label;
   double mhz;
} Freq;

typedef struct
{
   const char *path;
   const char *name;
   int enable;
   double watts;
   double min;
   double max;
   double cap;
   double cap_min;
   double cap_max;
   double crit;
} Power;

typedef struct
{
   const char *path;
   const char *name;
   const char *label;
   int enable;
   double volts;
   double min;
   double max;
   double crit_min;
   double crit;
} Volt;

typedef struct
{
   const char *path;
   const char *name;
   int enable;
   double amps;
   double min;
   double max;
   double crit_min;
   double crit;
} Amp;

typedef struct
{
   const char *path;
   const char *name;
   int enable;
   double joules;
} Energy;

typedef struct
{
   const char *path;
   const char *name;
   int enable;
   double percent;
} Humid;

typedef struct
{
   const char *path;
   const char *name;
   int alarm;
} Intrusion;

typedef struct
{
   const char *path;
   const char *name;
   const char *label;
   Eina_List *temps;
   Eina_List *fans;
   Eina_List *pwms;
   Eina_List *freqs;
   Eina_List *powers;
   Eina_List *volts;
   Eina_List *amps;
   Eina_List *energies;
   Eina_List *humids;
   Eina_List *intrusions;
} Mon;

static Eina_Lock mons_lock;
static Eina_List *mons = NULL;

static char *
_file_str_get(const char *path)
{
   int fd = open(path, O_RDONLY);
   char buf[4096], *str = NULL, *s;
   ssize_t sz;

   if (fd < 0) return NULL;
   sz = read(fd, buf, sizeof(buf) - 1);
   if (sz < 1) goto err;
   buf[sz] = '\0';
   for (s = buf; s[0] != '\0'; s++)
     {
        if ((s[0] == '\n') || (s[0] == '\r'))
          {
             s[0] = '\0';
             break;
          }
     }
   str = strdup(buf);
err:
   close(fd);
   return str;
}

#define NOVAL -999999999

static int
_file_int_get(const char *path)
{
   char *str = _file_str_get(path);
   if (!str) return NOVAL;
   int val = atoi(str);
   free(str);
   return val;
}

#define GETVAL_STR(_base, _name, _field) do { char *_s; char _b[1024 + 512]; \
   snprintf(_b, sizeof(_b), "%s_%s", _base, _name); \
   if ((_s = _file_str_get(_b))) { \
      _field = eina_stringshare_add(_s); \
      free(_s); \
   } \
} while (0)
#define GETVAL_INT(_base, _name, _field) do { int _v; char _b[1024 + 512]; \
   snprintf(_b, sizeof(_b), "%s_%s", _base, _name); \
   if ((_v = _file_int_get(_b)) >= 0) _field = _v; \
} while (0)
#define GETVAL_DBL(_base, _name, _field, _div) do { int _v; char _b[1024 + 512]; \
   snprintf(_b, sizeof(_b), "%s_%s", _base, _name); \
   if ((_v = _file_int_get(_b)) >= 0) _field = (double)_v / (_div); \
} while (0)

static void
_hwmon_init(void)
{
   Eina_List *list;
   char *s, *str, buf[1024], buf2[1024 + 256];
   int i, x;

   eina_lock_take(&mons_lock);
   list = ecore_file_ls("/sys/class/hwmon");
   EINA_LIST_FREE(list, s)
     {
        Mon *mon = calloc(1, sizeof(Mon));
        if (mon)
          {
             snprintf(buf, sizeof(buf), "/sys/class/hwmon/%s", s);
             mon->path = eina_stringshare_add(buf);
             mon->name = eina_stringshare_add(s);
             snprintf(buf, sizeof(buf), "/sys/class/hwmon/%s/name", s);
             str = _file_str_get(buf);
             if (str)
               {
                  mon->label = eina_stringshare_add(str);
                  free(str);
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/temp%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x == NOVAL) break;
                  Temp *t = calloc(1, sizeof(Temp));
                  if (t)
                    {
                       t->temp = (double)x / 1000.0;
                       t->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "temp%i", i);
                       t->name = eina_stringshare_add(buf2);
                       GETVAL_STR(buf, "label", t->label);
                       t->enable = 1;
                       GETVAL_INT(buf, "enable", t->enable);
                       GETVAL_INT(buf, "fault", t->fault);
                       GETVAL_INT(buf, "type", t->type);
                       GETVAL_DBL(buf, "min", t->min, 1000.0);
                       GETVAL_DBL(buf, "max", t->max, 1000.0);
                       GETVAL_DBL(buf, "crit", t->crit, 1000.0);
                       GETVAL_DBL(buf, "emergency", t->emergency, 1000.0);
                       mon->temps = eina_list_append(mon->temps, t);
                    }
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/fan%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x == NOVAL) break;
                  Fan *f = calloc(1, sizeof(Fan));
                  if (f)
                    {
                       f->rpm = x;
                       f->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "fan%i", i);
                       f->name = eina_stringshare_add(buf2);
                       GETVAL_STR(buf, "label", f->label);
                       f->enable = 1;
                       GETVAL_INT(buf, "enable", f->enable);
                       GETVAL_INT(buf, "fault", f->fault);
                       GETVAL_INT(buf, "min", f->min);
                       GETVAL_INT(buf, "max", f->max);
                       GETVAL_INT(buf, "target", f->target);
                       mon->fans = eina_list_append(mon->fans, f);
                    }
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/pwm%i", s, i);
                  x = _file_int_get(buf);
                  if (x == NOVAL) break;
                  Pwm *p = calloc(1, sizeof(Pwm));
                  if (p)
                    {
                       p->val = (double)x / 255.0;
                       p->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "pwm%i", i);
                       p->name = eina_stringshare_add(buf2);
                       GETVAL_INT(buf, "enable", p->mode);
                       GETVAL_DBL(buf, "min", p->min, 255.0);
                       GETVAL_DBL(buf, "max", p->max, 255.0);
                       mon->pwms = eina_list_append(mon->pwms, p);
                    }
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/freq%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x == NOVAL) break;
                  Freq *f = calloc(1, sizeof(Freq));
                  if (f)
                    {
                       f->mhz = (double)x / 1000000.0;
                       f->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "freq%i", i);
                       f->name = eina_stringshare_add(buf2);
                       GETVAL_STR(buf, "label", f->label);
                       mon->freqs = eina_list_append(mon->freqs, f);
                    }
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/power%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x < 0)
                    {
                       snprintf(buf2, sizeof(buf2), "%s_%s", buf, "average");
                       x = _file_int_get(buf2);
                    }
                  if (x == NOVAL) break;
                  Power *p = calloc(1, sizeof(Power));
                  if (p)
                    {
                       p->watts = (double)x / 1000000.0;
                       p->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "power%i", i);
                       p->name = eina_stringshare_add(buf2);
                       p->enable = 1;
                       GETVAL_INT(buf, "enable", p->enable);
                       GETVAL_DBL(buf, "min", p->min, 1000000.0);
                       GETVAL_DBL(buf, "max", p->max, 1000000.0);
                       if (p->min <= 0.0)
                         GETVAL_DBL(buf, "average_min", p->min, 1000000.0);
                       if (p->max <= 0.0)
                         GETVAL_DBL(buf, "average_max", p->max, 1000000.0);
                       GETVAL_DBL(buf, "cap", p->cap, 1000000.0);
                       GETVAL_DBL(buf, "cap_min", p->cap_min, 1000000.0);
                       GETVAL_DBL(buf, "cap_max", p->cap_max, 1000000.0);
                       GETVAL_DBL(buf, "crit", p->crit, 1000000.0);
                       mon->powers = eina_list_append(mon->powers, p);
                    }
               }
             for (i = 0; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/in%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x < 0)
                    {
                       snprintf(buf2, sizeof(buf2), "%s_%s", buf, "average");
                       x = _file_int_get(buf2);
                    }
                  if (x == NOVAL) break;
                  Volt *v = calloc(1, sizeof(Volt));
                  if (v)
                    {
                       v->volts = (double)x / 1000.0;
                       v->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "in%i", i);
                       v->name = eina_stringshare_add(buf2);
                       GETVAL_STR(buf, "label", v->label);
                       v->enable = 1;
                       GETVAL_INT(buf, "enable", v->enable);
                       GETVAL_DBL(buf, "min", v->min, 1000.0);
                       GETVAL_DBL(buf, "max", v->max, 1000.0);
                       GETVAL_DBL(buf, "lcrit", v->crit_min, 1000.0);
                       GETVAL_DBL(buf, "crit", v->crit, 1000.0);
                       mon->volts = eina_list_append(mon->volts, v);
                    }
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/curr%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x < 0)
                    {
                       snprintf(buf2, sizeof(buf2), "%s_%s", buf, "average");
                       x = _file_int_get(buf2);
                    }
                  if (x == NOVAL) break;
                  Amp *a = calloc(1, sizeof(Amp));
                  if (a)
                    {
                       a->amps = (double)x / 1000.0;
                       a->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "curr%i", i);
                       a->name = eina_stringshare_add(buf2);
                       a->enable = 1;
                       GETVAL_INT(buf, "enable", a->enable);
                       GETVAL_DBL(buf, "min", a->min, 1000.0);
                       GETVAL_DBL(buf, "max", a->max, 1000.0);
                       GETVAL_DBL(buf, "lcrit", a->crit_min, 1000.0);
                       GETVAL_DBL(buf, "crit", a->crit, 1000.0);
                       mon->amps = eina_list_append(mon->amps, a);
                    }
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/energy%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x == NOVAL) break;
                  Energy *e = calloc(1, sizeof(Energy));
                  if (e)
                    {
                       e->joules = (double)x / 1000000.0;
                       e->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "energy%i", i);
                       e->name = eina_stringshare_add(buf2);
                       e->enable = 1;
                       GETVAL_INT(buf, "enable", e->enable);
                       mon->energies = eina_list_append(mon->energies, e);
                    }
               }
             for (i = 1; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/humidity%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "input");
                  x = _file_int_get(buf2);
                  if (x == NOVAL) break;
                  Humid *h = calloc(1, sizeof(Humid));
                  if (h)
                    {
                       h->percent = (double)x / 1000.0;
                       h->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "humidity%i", i);
                       h->name = eina_stringshare_add(buf2);
                       h->enable = 1;
                       GETVAL_INT(buf, "enable", h->enable);
                       mon->humids = eina_list_append(mon->humids, h);
                    }
               }
             for (i = 0; ; i++)
               {
                  snprintf(buf,  sizeof(buf),  "/sys/class/hwmon/%s/intrusion%i", s, i);
                  snprintf(buf2, sizeof(buf2), "%s_%s", buf, "alarm");
                  x = _file_int_get(buf2);
                  if (x == NOVAL) break;
                  Intrusion *n = calloc(1, sizeof(Intrusion));
                  if (n)
                    {
                       n->alarm = x;
                       n->path = eina_stringshare_add(buf);
                       snprintf(buf2,  sizeof(buf2),  "intrusion%i", i);
                       n->name = eina_stringshare_add(buf2);
                       mon->intrusions = eina_list_append(mon->intrusions, n);
                    }
               }
             mons = eina_list_append(mons, mon);
          }
        free(s);
     }
   eina_lock_release(&mons_lock);
}

static void
_hwmon_update(void)
{
   Eina_List *l, *ll;
   Mon *mon;
   Temp *temp;
   Fan *fan;
   Pwm *pwm;
   Freq *freq;
   Power *power;
   Volt *volt;
   Amp *amp;
   Energy *energy;
   Humid *humid;
   Intrusion *intrusion;
   int x;
   char buf[4096];

   eina_lock_take(&mons_lock);
   EINA_LIST_FOREACH(mons, l, mon)
     {
        EINA_LIST_FOREACH(mon->temps, ll, temp)
          {
             snprintf(buf, sizeof(buf), "%s_input", temp->path);
             temp->temp = (double)_file_int_get(buf) / 1000.0;
             snprintf(buf, sizeof(buf), "%s_enable", temp->path);
             x = _file_int_get(buf);
             if (x != NOVAL) temp->enable = x;
             snprintf(buf, sizeof(buf), "%s_fault", temp->path);
             temp->fault = _file_int_get(buf);
          }
        EINA_LIST_FOREACH(mon->fans, ll, fan)
          {
             snprintf(buf, sizeof(buf), "%s_input", fan->path);
             fan->rpm = _file_int_get(buf);
             snprintf(buf, sizeof(buf), "%s_target", fan->path);
             fan->target = _file_int_get(buf);
             snprintf(buf, sizeof(buf), "%s_enable", fan->path);
             x = _file_int_get(buf);
             if (x != NOVAL) fan->enable = x;
             snprintf(buf, sizeof(buf), "%s_fault", fan->path);
             fan->fault = _file_int_get(buf);
          }
        EINA_LIST_FOREACH(mon->pwms, ll, pwm)
          {
             snprintf(buf, sizeof(buf), "%s", pwm->path);
             pwm->val = (double)_file_int_get(buf) / 255.0;
             snprintf(buf, sizeof(buf), "%s_enable", pwm->path);
             x = _file_int_get(buf);
             if (x != NOVAL) pwm->mode = x;
          }
        EINA_LIST_FOREACH(mon->freqs, ll, freq)
          {
             snprintf(buf, sizeof(buf), "%s_input", freq->path);
             freq->mhz = (double)_file_int_get(buf) / 1000000.0;
          }
        EINA_LIST_FOREACH(mon->powers, ll, power)
          {
             snprintf(buf, sizeof(buf), "%s_input", power->path);
             x = _file_int_get(buf);
             if (x == NOVAL)
               {
                  snprintf(buf, sizeof(buf), "%s_average", power->path);
                  x = _file_int_get(buf);
               }
             power->watts = (double)x / 1000000.0;
             snprintf(buf, sizeof(buf), "%s_enable", power->path);
             x = _file_int_get(buf);
             if (x != NOVAL) power->enable = x;
          }
        EINA_LIST_FOREACH(mon->volts, ll, volt)
          {
             snprintf(buf, sizeof(buf), "%s_input", volt->path);
             x = _file_int_get(buf);
             if (x == NOVAL)
               {
                  snprintf(buf, sizeof(buf), "%s_average", volt->path);
                  x = _file_int_get(buf);
               }
             volt->volts = (double)x / 1000.0;
             snprintf(buf, sizeof(buf), "%s_enable", volt->path);
             x = _file_int_get(buf);
             if (x != NOVAL) volt->enable = x;
          }
        EINA_LIST_FOREACH(mon->amps, ll, amp)
          {
             snprintf(buf, sizeof(buf), "%s_input", amp->path);
             x = _file_int_get(buf);
             if (x == NOVAL)
               {
                  snprintf(buf, sizeof(buf), "%s_average", amp->path);
                  x = _file_int_get(buf);
               }
             amp->amps = (double)x / 1000.0;
             snprintf(buf, sizeof(buf), "%s_enable", amp->path);
             x = _file_int_get(buf);
             if (x != NOVAL) amp->enable = x;
          }
        EINA_LIST_FOREACH(mon->energies, ll, energy)
          {
             snprintf(buf, sizeof(buf), "%s_input", energy->path);
             energy->joules = (double)_file_int_get(buf) / 1000000.0;
             snprintf(buf, sizeof(buf), "%s_enable", energy->path);
             x = _file_int_get(buf);
             if (x != NOVAL) energy->enable = x;
         }
        EINA_LIST_FOREACH(mon->humids, ll, humid)
          {
             snprintf(buf, sizeof(buf), "%s_input", humid->path);
             humid->percent = (double)_file_int_get(buf) / 1000.0;
             snprintf(buf, sizeof(buf), "%s_enable", humid->path);
             x = _file_int_get(buf);
             if (x != NOVAL) humid->enable = x;
          }
        EINA_LIST_FOREACH(mon->intrusions, ll, intrusion)
          {
             snprintf(buf, sizeof(buf), "%s_alarm", intrusion->path);
             intrusion->alarm = _file_int_get(buf);
          }
     }
   eina_lock_release(&mons_lock);
}

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
             Eina_List *l, *ll;
             Mon *mon;
             Temp *temp;

             eina_lock_take(&mons_lock);
             EINA_LIST_FOREACH(mons, l, mon)
               {
                  EINA_LIST_FOREACH(mon->temps, ll, temp)
                    {
                       tth->sensor_name = eina_stringshare_add(temp->path);
                       break;
                    }
               }
             eina_lock_release(&mons_lock);
          }
     }
   // we couldn't find the sensor - so init failed - try again next time
   if (((!tth->sensor_name) || (tth->sensor_name[0] == 0)))
     tth->initted = EINA_FALSE;
}

static int
check(Tempthread *tth)
{
   Eina_List *l, *ll;
   Mon *mon;
   Temp *temp;

   _hwmon_update();
   if (tth->sensor_name)
     {
        double t = 0.0;

        eina_lock_take(&mons_lock);
        EINA_LIST_FOREACH(mons, l, mon)
          {
             EINA_LIST_FOREACH(mon->temps, ll, temp)
               {
                  if (!strcmp(tth->sensor_name, temp->path))
                    {
                       t =  temp->temp;
                       break;
                    }
               }
          }
        eina_lock_release(&mons_lock);
        return t;
     }
   return -999;
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
   Eina_List *sensors = NULL, *l, *ll;
   Sensor *sen;
   Mon *mon;
   Temp *temp;
   char buf[256];

   EINA_LIST_FOREACH(mons, l, mon)
     {
        EINA_LIST_FOREACH(mon->temps, ll, temp)
          {
             sen = calloc(1, sizeof(Sensor));
             sen->name = eina_stringshare_add(temp->path);
             snprintf(buf, sizeof(buf), "%s - %s",
                      mon->label ? mon->label : mon->name,
                      temp->label ? temp->label : temp->name);
             sen->label = eina_stringshare_add(buf);
             sensors = eina_list_append(sensors, sen);
          }
     }
   return sensors;
}

void
temperature_tempget_setup(void)
{
   eina_lock_new(&mons_lock);
   _hwmon_init();
}

void
temperature_tempget_clear(void)
{
   Mon *mon;
   Temp *temp;
   Fan *fan;
   Pwm *pwm;
   Freq *freq;
   Power *power;
   Volt *volt;
   Amp *amp;
   Energy *energy;
   Humid *humid;
   Intrusion *intrusion;

   eina_lock_take(&mons_lock);
   EINA_LIST_FREE(mons, mon)
     {
        eina_stringshare_replace(&(mon->path), NULL);
        eina_stringshare_replace(&(mon->name), NULL);
        eina_stringshare_replace(&(mon->label), NULL);
        EINA_LIST_FREE(mon->temps, temp)
          {
             eina_stringshare_replace(&(temp->path), NULL);
             eina_stringshare_replace(&(temp->name), NULL);
             eina_stringshare_replace(&(temp->label), NULL);
             free(temp);
          }
        EINA_LIST_FREE(mon->fans, fan)
          {
             eina_stringshare_replace(&(fan->path), NULL);
             eina_stringshare_replace(&(fan->name), NULL);
             eina_stringshare_replace(&(fan->label), NULL);
             free(fan);
          }
        EINA_LIST_FREE(mon->pwms, pwm)
          {
             eina_stringshare_replace(&(pwm->path), NULL);
             eina_stringshare_replace(&(pwm->name), NULL);
             free(fan);
          }
        EINA_LIST_FREE(mon->freqs, freq)
          {
             eina_stringshare_replace(&(freq->path), NULL);
             eina_stringshare_replace(&(freq->name), NULL);
             eina_stringshare_replace(&(freq->label), NULL);
             free(freq);
          }
        EINA_LIST_FREE(mon->powers, power)
          {
             eina_stringshare_replace(&(power->path), NULL);
             eina_stringshare_replace(&(power->name), NULL);
             free(power);
          }
        EINA_LIST_FREE(mon->volts, volt)
          {
             eina_stringshare_replace(&(volt->path), NULL);
             eina_stringshare_replace(&(volt->name), NULL);
             eina_stringshare_replace(&(volt->label), NULL);
             free(volt);
          }
        EINA_LIST_FREE(mon->amps, amp)
          {
             eina_stringshare_replace(&(amp->path), NULL);
             eina_stringshare_replace(&(amp->name), NULL);
             free(amp);
          }
        EINA_LIST_FREE(mon->energies, energy)
          {
             eina_stringshare_replace(&(energy->path), NULL);
             eina_stringshare_replace(&(energy->name), NULL);
             free(energy);
          }
        EINA_LIST_FREE(mon->humids, humid)
          {
             eina_stringshare_replace(&(humid->path), NULL);
             eina_stringshare_replace(&(humid->name), NULL);
             free(humid);
          }
        EINA_LIST_FREE(mon->intrusions, intrusion)
          {
             eina_stringshare_replace(&(intrusion->path), NULL);
             eina_stringshare_replace(&(intrusion->name), NULL);
             free(intrusion);
          }
        free(mon);
     }
   eina_lock_release(&mons_lock);
   // extra lock take to cover race cond for thread
   eina_lock_take(&mons_lock);
   eina_lock_release(&mons_lock);
   eina_lock_free(&mons_lock);
}
#endif
