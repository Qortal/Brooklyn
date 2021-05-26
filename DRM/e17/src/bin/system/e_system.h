#ifndef E_SYSTEM_H
# define E_SYSTEM_H 1
# include "config.h"

# ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
# endif

# ifdef STDC_HEADERS
#  include <stdlib.h>
#  include <stddef.h>
# else
#  ifdef HAVE_STDLIB_H
#   include <stdlib.h>
#  endif
# endif
# ifdef HAVE_ALLOCA_H
#  include <alloca.h>
# elif !defined alloca
#  ifdef __GNUC__
#   define alloca __builtin_alloca
#  elif defined _AIX
#   define alloca __alloca
#  elif defined _MSC_VER
#   include <malloc.h>
#   define alloca _alloca
#  elif !defined HAVE_ALLOCA
#   ifdef  __cplusplus
extern "C"
#   endif
void *alloca (size_t);
#  endif
# endif

# ifdef __linux__
#  include <features.h>
# endif

# ifdef HAVE_ENVIRON
#  define _GNU_SOURCE 1
# endif
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/param.h>
# include <sys/resource.h>
# include <utime.h>
# include <dlfcn.h>
# include <math.h>
# include <fcntl.h>
# include <limits.h>
# include <ctype.h>
# include <time.h>
# include <dirent.h>
# include <grp.h>
# include <glob.h>
# include <locale.h>
# include <errno.h>
# include <signal.h>
# include <inttypes.h>
# include <assert.h>
# if !defined (__FreeBSD__) && !defined (__OpenBSD__)
#  ifdef HAVE_MALLOC_H
#   include <malloc.h>
#  endif
# endif

#if defined (__FreeBSD__) || defined (__OpenBSD__)
# include <sys/sysctl.h>
#endif

#if defined (__FreeBSD__) && (__FreeBSD__ > 12)
# include <sys/ioctl.h>
# include <sys/backlight.h>
#endif

# ifndef _POSIX_HOST_NAME_MAX
#  define _POSIX_HOST_NAME_MAX 255
# endif

# include <Eina.h>
# include <Ecore.h>
# include <Ecore_File.h>
# include <Eet.h>
# ifdef HAVE_EEZE
#  include <Eeze.h>
# endif

#define ERR(args...) do { fprintf(stderr, "E_SYSTEM_ERR: "); fprintf(stderr, ##args); } while (0)
#define INF(args...) do { fprintf(stderr, "E_SYSTEM_INF: "); fprintf(stderr, ##args); } while (0)

extern Eina_Bool alert_backlight_reset;

extern uid_t uid;
extern gid_t gid;
extern char *user_name;
extern char *group_name;

void e_system_inout_init(void);
void e_system_inout_shutdown(void);
void e_system_inout_command_register(const char *cmd, void (*func) (void *data, const char *aprams), void *data);
void e_system_inout_command_send(const char *cmd, const char *fmt, ...) EINA_PRINTF(2, 3);

void e_system_backlight_init(void);
void e_system_backlight_shutdown(void);

void e_system_storage_init(void);
void e_system_storage_shutdown(void);

void e_system_power_init(void);
void e_system_power_shutdown(void);

void e_system_rfkill_init(void);
void e_system_rfkill_shutdown(void);

void e_system_l2ping_init(void);
void e_system_l2ping_shutdown(void);

void e_system_cpufreq_init(void);
void e_system_cpufreq_shutdown(void);

void e_system_ddc_init(void);
void e_system_ddc_shutdown(void);

extern Ecore_Exe *e_system_run(const char *exe);

#endif

