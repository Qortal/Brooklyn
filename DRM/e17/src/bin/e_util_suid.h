#include "config.h"

#define __USE_MISC
#define _SVID_SOURCE
#define _DEFAULT_SOURCE

# ifdef __linux__
#  include <features.h>
# endif

# ifdef HAVE_ENVIRON
#  define _GNU_SOURCE 1
# endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <inttypes.h>
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif
#include <signal.h>
#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#elif defined(HAVE_PROCCTL)
# include <sys/procctl.h>
#endif


#define E_UTIL_SUID_ERR(args...) do { fprintf(stderr, "E_SUID_ERR: "); fprintf(stderr, ##args); } while (0)

static inline int
e_setuid_setup(uid_t *caller_uid, gid_t *caller_gid, char **caller_user, char **caller_group)
{
   struct passwd *pwent;
   struct group *grent;
   static char buf[PATH_MAX];

   *caller_uid = getuid();
   *caller_gid = getgid();

   pwent = getpwuid(*caller_uid);
   if (!pwent)
     {
        E_UTIL_SUID_ERR("Unable to obtain passwd entry for calling user\n");
        return 31;
     }
   if (!pwent->pw_name)
     {
        E_UTIL_SUID_ERR("Blank username for user\n");
        return 32;
     }
   *caller_user = strdup(pwent->pw_name);
   if (!(*caller_user))
     {
        E_UTIL_SUID_ERR("Unable to allocate memory for username\n");
        return 33;
     }
   grent = getgrgid(*caller_gid);
   if (!grent)
     {
        E_UTIL_SUID_ERR("Unable to obtain group entry for calling group\n");
        return 34;
     }
   if (!grent->gr_name)
     {
        E_UTIL_SUID_ERR("Blank groupname for group\n");
        return 35;
     }
   *caller_group = strdup(grent->gr_name);
   if (!(*caller_group))
     {
        E_UTIL_SUID_ERR("Unable to allocate memory for groupname\n");
        return 36;
     }

   if (setuid(0) != 0)
     {
        E_UTIL_SUID_ERR("Unable to assume root user privileges\n");
        return 37;
     }
   if (setgid(0) != 0)
     {
        E_UTIL_SUID_ERR("Unable to assume root group privileges\n");
        return 38;
     }

   pwent = getpwuid(getuid());
   if (!pwent)
     {
        E_UTIL_SUID_ERR("Unable to obtain passwd entry\n");
        return 39;
     }
   if (!pwent->pw_dir)
     {
        E_UTIL_SUID_ERR("No home dir for root\n");
        return 40;
     }
   if (strlen(pwent->pw_dir) > (sizeof(buf) - 8))
     {
        E_UTIL_SUID_ERR("Root homedir too long\n");
        return 41;
     }
   if (pwent->pw_dir[0] != '/')
     {
        E_UTIL_SUID_ERR("Root homedir %s is not a full path\n", pwent->pw_dir);
        return 42;
     }
   if (!realpath(pwent->pw_dir, buf))
     {
        E_UTIL_SUID_ERR("Root homedir %s does not resolve\n", pwent->pw_dir);
        return 43;
     }
   snprintf(buf, sizeof(buf), "HOME=%s", pwent->pw_dir);
   if (putenv(buf) == -1)
     {
        E_UTIL_SUID_ERR("Unable to set $HOME environment\n");
        return 44;
     }

   // change CWD to / to avoid path search dlopens finding libs in ./
   if (chdir("/") != 0)
     {
        E_UTIL_SUID_ERR("Unable to change working dir to /\n");
        return 45;
     }

   // die with parent - special as this is setuid
#ifdef HAVE_PRCTL
   prctl(PR_SET_PDEATHSIG, SIGTERM);
#elif defined(HAVE_PROCCTL)
   int sig = SIGTERM;
   procctl(P_PID, 0, PROC_PDEATHSIG_CTL, &sig);
#endif

#ifdef HAVE_UNSETENV
# define NOENV(x) unsetenv(x)
   // pass 1 - just nuke known dangerous env vars brutally if possible via
   // unsetenv(). if you don't have unsetenv... there's pass 2 and 3
   NOENV("IFS");
   NOENV("CDPATH");
   NOENV("LOCALDOMAIN");
   NOENV("RES_OPTIONS");
   NOENV("HOSTALIASES");
   NOENV("NLSPATH");
   NOENV("PATH_LOCALE");
   NOENV("COLORTERM");
   NOENV("LANG");
   NOENV("LANGUAGE");
   NOENV("LINGUAS");
   NOENV("TERM");
   NOENV("LD_PRELOAD");
   NOENV("LD_LIBRARY_PATH");
   NOENV("SHLIB_PATH");
   NOENV("LIBPATH");
   NOENV("AUTHSTATE");
   NOENV("DYLD_*");
   NOENV("KRB_CONF*");
   NOENV("KRBCONFDIR");
   NOENV("KRBTKFILE");
   NOENV("KRB5_CONFIG*");
   NOENV("KRB5_KTNAME");
   NOENV("VAR_ACE");
   NOENV("USR_ACE");
   NOENV("DLC_ACE");
   NOENV("TERMINFO");
   NOENV("TERMINFO_DIRS");
   NOENV("TERMPATH");
   NOENV("TERMCAP");
   NOENV("ENV");
   NOENV("BASH_ENV");
   NOENV("PS4");
   NOENV("GLOBIGNORE");
   NOENV("SHELLOPTS");
   NOENV("JAVA_TOOL_OPTIONS");
   NOENV("PERLIO_DEBUG");
   NOENV("PERLLIB");
   NOENV("PERL5LIB");
   NOENV("PERL5OPT");
   NOENV("PERL5DB");
   NOENV("FPATH");
   NOENV("NULLCMD");
   NOENV("READNULLCMD");
   NOENV("ZDOTDIR");
   NOENV("TMPPREFIX");
   NOENV("PYTHONPATH");
   NOENV("PYTHONHOME");
   NOENV("PYTHONINSPECT");
   NOENV("RUBYLIB");
   NOENV("RUBYOPT");
# ifdef HAVE_ENVIRON
   if (environ)
     {
        int i, again;
        // go over environment array again and again... safely
        do
          {
             again = 0;
             // walk through and find first entry that we don't like
             for (i = 0; environ[i]; i++)
               {
                  // if it begins with any of these, it's possibly nasty
                  if ((!strncmp(environ[i], "LD_",   3)) ||
                      (!strncmp(environ[i], "_RLD_", 5)) ||
                      (!strncmp(environ[i], "LC_",   3)) ||
                      (!strncmp(environ[i], "LDR_",  3)))
                    {
                       // unset it
                       char *tmp, *p;

                       tmp = strdup(environ[i]);
                       if (!tmp) abort();
                       p = strchr(tmp, '=');
                       if (!p) abort();
                       *p = 0;
                       NOENV(tmp);
                       free(tmp);
                       // and mark our do to try again from the start in case
                       // unsetenv changes environ ptr
                       again = 1;
                       break;
                    }
               }
          }
        while (again);
     }
# endif
#endif
   // pass 2 - clear entire environment so it doesn't exist at all. if you
   // can't do this... you're possibly in trouble... but the worst is still
   // fixed in pass 3
#ifdef HAVE_CLEARENV
   clearenv();
#else
# ifdef HAVE_ENVIRON
   environ = NULL;
# endif
#endif
   // pass 3 - set path and ifs to minimal defaults
   putenv("PATH=/bin:/usr/bin:/sbin:/usr/sbin");
   putenv("IFS= \t\n");
   return 0;
}

