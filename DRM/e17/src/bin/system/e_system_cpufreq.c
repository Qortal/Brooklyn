#include "e_system.h"

#if defined __OpenBSD__
// no local funcs
#elif defined __FreeBSD__
// no local funcs
#else
static int
sys_cpu_setall(const char *control, const char *value)
{
   int num = 0;
   char buf[4096];
   FILE *f;

   for (;;)
     {
        snprintf(buf, sizeof(buf),
                 "/sys/devices/system/cpu/cpu%i/cpufreq/%s", num, control);
        f = fopen(buf, "w");
        if (!f) return num;
        fprintf(f, "%s", value);
        fclose(f);
        num++;
     }
   return 1;
}

static int
sys_cpufreq_set(const char *control, const char *value)
{
   char buf[4096];
   FILE *f;

   snprintf(buf, sizeof(buf), "/sys/devices/system/cpu/cpufreq/%s", control);
   f = fopen(buf, "w");
   if (!f)
     {
        if (sys_cpu_setall(control, value) > 0) return 1;
        else return 0;
     }
   fprintf(f, "%s", value);
   fclose(f);
   return 1;
}

static int
sys_cpu_pstate(int min, int max, int turbo)
{
   FILE *f;

   f = fopen("/sys/devices/system/cpu/intel_pstate/min_perf_pct", "w");
   if (!f) return 0;
   fprintf(f, "%i", min);
   fclose(f);

   f = fopen("/sys/devices/system/cpu/intel_pstate/max_perf_pct", "w");
   if (!f) return 0;
   fprintf(f, "%i", max);
   fclose(f);

   f = fopen("/sys/devices/system/cpu/intel_pstate/no_turbo", "w");
   if (!f) return 0;
   fprintf(f, "%i", turbo ? 0 : 1);
   fclose(f);

   return 1;
}
#endif

static void
_cb_cpufreq_freq(void *data EINA_UNUSED, const char *params)
{
   // FREQ
   int f = atoi(params);
   if (f > 0)
     {
#if defined __OpenBSD__
        int mib[] = {CTL_HW, HW_SETPERF};
        size_t len = sizeof(f);

        if (sysctl(mib, 2, NULL, 0, &f, len) == 0)
          {
             e_system_inout_command_send("cpufreq-freq", "ok");
             return;
          }
#elif defined __FreeBSD__
        if (sysctlbyname("dev.cpu.0.freq", NULL, NULL, &f, sizeof(f)) == 0)
          {
             e_system_inout_command_send("cpufreq-freq", "ok");
             return;
          }
#else
        fprintf(stderr, "CPUFREQ: scaling_setspeed [%s]\n", params);
        if (sys_cpu_setall("scaling_setspeed", params) > 0)
          {
             fprintf(stderr, "CPUFREQ: scaling_setspeed OK\n");
             e_system_inout_command_send("cpufreq-freq", "ok");
             return;
          }
#endif
     }
   e_system_inout_command_send("cpufreq-freq", "err");
}

static void
_cb_cpufreq_governor(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   // NAME
#if defined __OpenBSD__
   e_system_inout_command_send("cpufreq-governor", "err");
#elif defined __FreeBSD__
   e_system_inout_command_send("cpufreq-governor", "err");
#else
   fprintf(stderr, "CPUFREQ: scaling_governor [%s]\n", params);
   if (sys_cpu_setall("scaling_governor", params) <= 0)
     {
        ERR("Unable to open governor interface for writing\n");
        e_system_inout_command_send("cpufreq-governor", "err");
        return;
     }
   else
     fprintf(stderr, "CPUFREQ: scaling_governor OK\n");
   if (!strcmp(params, "ondemand"))
     sys_cpufreq_set("ondemand/ignore_nice_load", "0");
   else if (!strcmp(params, "conservative"))
     sys_cpufreq_set("conservative/ignore_nice_load", "0");
   e_system_inout_command_send("cpufreq-governor", "ok");
#endif
}

static void
_cb_cpufreq_pstate(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   // MIN_PERC MAX_PERC TURBO
#if defined __OpenBSD__
   e_system_inout_command_send("cpufreq-pstate", "err");
#elif defined __FreeBSD__
   e_system_inout_command_send("cpufreq-pstate", "err");
#else
   int min = 0, max = 100, turbo = 1;
   FILE *f;

   f = fopen("/sys/devices/system/cpu/intel_pstate/min_perf_pct", "r");
   if (!f) return;
   fclose(f);
   if (sscanf(params, "%i %i %i", &min, &max, &turbo) == 3)
     {
        if (min < 0) min = 0;
        else if (min > 100) min = 100;
        if (max < 0) max = 0;
        else if (max > 100) max = 100;
        if (turbo < 0) turbo = 0;
        else if (turbo > 1) turbo = 1;
        fprintf(stderr, "CPUFREQ: pstate [min=%i max=%i turbo=%i]\n", min, max, turbo);
        if (sys_cpu_pstate(min, max, turbo) > 0)
          {
             e_system_inout_command_send("cpufreq-pstate", "ok");
             return;
          }
     }
   e_system_inout_command_send("cpufreq-pstate", "err");
#endif
}

void
e_system_cpufreq_init(void)
{
   e_system_inout_command_register("cpufreq-freq",     _cb_cpufreq_freq, NULL);
#if defined __FreeBSD__ || defined __OpenBSD__
   (void) _cb_cpufreq_governor;
   (void) _cb_cpufreq_pstate;
#else
   e_system_inout_command_register("cpufreq-governor", _cb_cpufreq_governor, NULL);
   e_system_inout_command_register("cpufreq-pstate",   _cb_cpufreq_pstate, NULL);
#endif
}

void
e_system_cpufreq_shutdown(void)
{
   // only shutdown things we really have to - no need to free mem etc.
}
