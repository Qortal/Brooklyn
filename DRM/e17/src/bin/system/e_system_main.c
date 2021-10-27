#include "e_util_suid.h"

#include "e_system.h"

Eina_Bool alert_backlight_reset = EINA_FALSE;

uid_t uid = -1; // uid of person running me
gid_t gid = -1; // gid of person running me
char *user_name = NULL;
char *group_name = NULL;

static int
_conf_allow_deny(const char *cmd, const char *glob, const char *sys)
{
   if (!strcmp(cmd, "allow:"))
     {
        if (!strcmp(glob, "*")) return 1; // allow
        if (eina_fnmatch(glob, sys, 0)) return 1; // allow this sys
     }
   else if (!strcmp(cmd, "deny:"))
     {
        if (!strcmp(glob, "*")) return -1; // deny
        if (eina_fnmatch(glob, sys, 0)) return -1; // deny this sys
     }
   return 0; // unknown
}

static int
_etc_enlightenment_system_conf_check(const char *sys)
{
#define MAXGROUPS 1024
   int gn, i;
   gid_t gl[MAXGROUPS];
   char type[32], usergroup[256], cmd[32], glob[256], buf[1024];
   Eina_Bool in_usergroup;
   FILE *f = fopen("/etc/enlightenment/system.conf", "r");
   if (!f) return 1; // if the config doesn't exist - allow by policy

   gn = getgroups(MAXGROUPS, gl);
   if (gn < 0)
     {
        ERR("User %i member of too many groups\n", uid);
        goto deny;
     }
   while (fgets(buf, sizeof(buf), f))
     {
        int num;

        if (buf[0] == '#') continue;
        num = sscanf(buf, "%31s %255s %31s %255s\n",
                     type, usergroup, cmd, glob);
        if (num == 4)
          {
             if (!strcmp(type, "user:"))
               {
                  struct passwd *pw = getpwuid(uid);

                  in_usergroup = EINA_FALSE;
                  if (pw)
                    {
                       if (eina_fnmatch(usergroup, pw->pw_name, 0))
                         {
                            in_usergroup = EINA_TRUE;
                         }
                    }
                  if (in_usergroup)
                    {
                       int ok = _conf_allow_deny(cmd, glob, sys);
                       if (ok == 1) goto allow;
                       else if (ok == -1)
                         {
                            INF("Deny rule: %s\n", buf);
                            goto deny;
                         }
                    }
               }
             else if (!strcmp(type, "group:"))
               {
                  struct group *gp;

                  in_usergroup = EINA_FALSE;
                  gp = getgrgid(gid);
                  if (gp)
                    {
                       for (i = 0; i < gn; i++)
                         {
                            gp = getgrgid(gl[i]);
                            if (gp)
                              {
                                 if (eina_fnmatch(usergroup, gp->gr_name, 0))
                                   {
                                      in_usergroup = EINA_TRUE;
                                      break;
                                   }
                              }
                         }
                    }
                  if (in_usergroup)
                    {
                       int ok = _conf_allow_deny(cmd, glob, sys);
                       if (ok == 1) goto allow;
                       else if (ok == -1)
                         {
                            INF("Deny rule: %s\n", buf);
                            goto deny;
                         }
                    }
               }
          }
     }
allow:
   fclose(f);
   return 1;
deny:
   fclose(f);
   return 0;
}

static Eina_Bool
_cb_idle_enterer(void *data EINA_UNUSED)
{
   // welcome to unportable code land... :)
   // trim process down as much as possible when going idle
#ifdef HAVE_MALLOC_TRIM
   malloc_trim(0);
#endif
   return ECORE_CALLBACK_RENEW;
}

int
main(int argc EINA_UNUSED, const char **argv EINA_UNUSED)
{
   const char *s;
   int ok, systems = 0;

   // special mode to reset all newly found bl devices to max on
   // discovery because we were run by the e alert crash handler and
   // the user needs to see it...
   s = getenv("E_ALERT_BACKLIGHT_RESET");
   if ((s) && (s[0]  == '1')) alert_backlight_reset = EINA_TRUE;

   ok = e_setuid_setup(&uid, &gid, &user_name, &group_name);
   if (ok != 0) exit(ok);

   ecore_app_no_system_modules();

   eina_init();
   ecore_init();
   ecore_file_init();
#ifdef HAVE_EEZE
   eeze_init();
#endif
   eet_init();

   e_system_inout_init();

#define CONF_INIT_CHECK(sys, fn, flag) \
   Eina_Bool flag = EINA_FALSE; \
   do { \
      if (_etc_enlightenment_system_conf_check(sys)) { \
         fn(); \
         flag = EINA_TRUE; \
         systems++; \
      } \
   } while (0)
#define CONF_SHUTDOWN(fn, flag) \
   if (flag) fn()

   CONF_INIT_CHECK("backlight", e_system_backlight_init, init_backlight);
   CONF_INIT_CHECK("ddc",       e_system_ddc_init,       init_ddc);
   CONF_INIT_CHECK("storage",   e_system_storage_init,   init_storage);
   CONF_INIT_CHECK("power",     e_system_power_init,     init_power);
   CONF_INIT_CHECK("rfkill",    e_system_rfkill_init,    init_rfkill);
   CONF_INIT_CHECK("l2ping",    e_system_l2ping_init,    init_l2ping);
   CONF_INIT_CHECK("cpufreq",   e_system_cpufreq_init,   init_cpufreq);

   if (systems == 0)
     {
        ERR("Permission denied to use this tool\n");
        exit(46);
     }

   ecore_idle_enterer_add(_cb_idle_enterer, NULL);

   ecore_main_loop_begin();

   CONF_SHUTDOWN(e_system_cpufreq_shutdown,   init_cpufreq);
   CONF_SHUTDOWN(e_system_l2ping_shutdown,    init_l2ping);
   CONF_SHUTDOWN(e_system_rfkill_shutdown,    init_rfkill);
   CONF_SHUTDOWN(e_system_power_shutdown,     init_power);
   CONF_SHUTDOWN(e_system_storage_shutdown,   init_storage);
   CONF_SHUTDOWN(e_system_ddc_shutdown,       init_ddc);
   CONF_SHUTDOWN(e_system_backlight_shutdown, init_backlight);

   e_system_inout_shutdown();

   eet_shutdown();
#ifdef HAVE_EEZE
   eeze_shutdown();
#endif
   ecore_file_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return 0;
}

Ecore_Exe *
e_system_run(const char *cmd)
{
   Ecore_Exe_Flags flags = ECORE_EXE_NONE;
#if (ECORE_VERSION_MAJOR >= 1) && (ECORE_VERSION_MINOR >= 21)
   flags |= ECORE_EXE_ISOLATE_IO;
#else
   flags |= 1024; // isolate_io is bit 10 .... it will be ignored if
   // efl doesn't do it, so harmless
#endif
   return ecore_exe_pipe_run(cmd, flags, NULL);
}
