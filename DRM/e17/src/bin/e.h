#ifndef E_H
# define E_H

# define E_VERSION_MAJOR 22

/**
 * @defgroup API Enlightenment API
 *
 * Application programming interface to be used by modules to extend
 * Enlightenment.
 *
 * @{
 */

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

#ifdef HAVE_WAYLAND
# ifndef EFL_EO_API_SUPPORT
#  define EFL_EO_API_SUPPORT
# endif
# ifndef EFL_BETA_API_SUPPORT
#  define EFL_BETA_API_SUPPORT
# endif
#endif

# define USE_IPC
# if 0
#  define OBJECT_PARANOIA_CHECK
# endif
# define OBJECT_CHECK

# ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
# endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined _AIX
#  define alloca __alloca
# elif defined _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# elif !defined HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (size_t);
# endif
#endif

# ifdef __linux__
#  include <features.h>
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
# include <pwd.h>
# include <grp.h>
# include <glob.h>
# include <locale.h>
# include <errno.h>
# include <signal.h>
# include <inttypes.h>
# include <assert.h>

# ifdef HAVE_GETTEXT
#  include <libintl.h>
# endif

# ifndef _POSIX_HOST_NAME_MAX
#  define _POSIX_HOST_NAME_MAX 255
# endif

# ifdef HAVE_VALGRIND
#  include <memcheck.h>
# endif

# ifdef HAVE_EXECINFO_H
#  include <execinfo.h>
# endif

# if !defined (__FreeBSD__) && !defined (__OpenBSD__)
#  ifdef HAVE_MALLOC_H
#   include <malloc.h>
#  endif
# endif

// XXX: FIXME: TMP solution for window stack until api is settled
# ifndef EFL_BETA_API_SUPPORT
#  define EFL_BETA_API_SUPPORT
# endif


# include <setjmp.h>
# include <Eo.h>
# include <Eina.h>
# include <Eet.h>
# include <Evas.h>
# include <Evas_Engine_Buffer.h>
# include <Ecore.h>
# include <Ecore_Getopt.h>
# include <Ecore_Evas.h>
# include <Ecore_Input.h>
# include <Ecore_Input_Evas.h>
# include <Ecore_Con.h>
# include <Ecore_Ipc.h>
# include <Ecore_File.h>
# include <Efreet.h>
# include <Efreet_Mime.h>
# include <Edje.h>
# include <Eldbus.h>
# include <Eio.h>
# include <Emotion.h>
# include <Elementary.h>
# include "e_Efx.h"

# ifdef HAVE_WAYLAND
#  include <Ecore_Wl2.h>
# endif


typedef struct _E_Before_Idler E_Before_Idler;
typedef struct _E_Rect         E_Rect;

#include "e_macros.h"

# define E_REMOTE_OPTIONS 1
# define E_REMOTE_OUT     2
# define E_WM_IN          3
# define E_REMOTE_IN      4
# define E_ENUM           5
# define E_LIB_IN         6


/* if you see a deprecated warning for a YOLO function,
 * you are attempting to use an extremely dangerous function.
 */
#ifdef EXECUTIVE_MODE_ENABLED
 #define YOLO
#else
 #define YOLO EINA_DEPRECATED
#endif

# define E_TYPEDEFS       1
# include "e_includes.h"
# undef E_TYPEDEFS
# include "e_includes.h"

E_API double          e_main_ts(const char *str);

#define E_EFL_VERSION_MINIMUM(MAJ, MIN, MIC) \
  ((eina_version->major > MAJ) || (eina_version->minor > MIN) ||\
   ((eina_version->minor == MIN) && (eina_version->micro >= MIC)))

struct _E_Rect
{
   int x, y, w, h;
};

extern E_API E_Path *path_data;
extern E_API E_Path *path_images;
extern E_API E_Path *path_fonts;
extern E_API E_Path *path_themes;
extern E_API E_Path *path_icons;
extern E_API E_Path *path_modules;
extern E_API E_Path *path_backgrounds;
extern E_API E_Path *path_messages;
extern E_API Eina_Bool good;
extern E_API Eina_Bool evil;
extern E_API Eina_Bool starting;
extern E_API Eina_Bool stopping;
extern E_API Eina_Bool restart;
extern E_API Eina_Bool e_nopause;

extern E_API Eina_Bool e_precache_end;
extern E_API Eina_Bool x_fatal;

extern E_API Eina_Bool after_restart;
extern E_API Eina_Bool e_main_loop_running;

extern EINTERN const char *e_first_frame;
extern EINTERN double e_first_frame_start_time;

//#define SMARTERR(args...) abort()
#define SMARTERRNR() return
#define SMARTERR(x)  return x

/**
 * @}
 */

/**
 * @defgroup Optional_Modules Optional Modules
 * @{
 *
 * @defgroup Optional_Conf Configurations
 * @defgroup Optional_Control Controls
 * @defgroup Optional_Devices Devices & Hardware
 * @defgroup Optional_Fileman File Managers
 * @defgroup Optional_Gadgets Gadgets
 * @defgroup Optional_Launcher Launchers
 * @defgroup Optional_Layouts Layout Managers
 * @defgroup Optional_Look Look & Feel
 * @defgroup Optional_Monitors Monitors & Notifications
 * @defgroup Optional_Mobile Mobile Specific Extensions
 * @}
 */

#if 0
#define REFD(obj, num) \
   do { \
      printf("%p <- %5i <- ref   | %s-%i\n", \
             obj, E_OBJECT(obj)->references, \
             __FILE__, num); \
   } while (0)

#define UNREFD(obj, num) \
   do { \
      printf("%p <- %5i <- unref | %s-%i\n", \
             obj, E_OBJECT(obj)->references, \
             __FILE__, num); \
   } while (0)

#define DELD(obj, num) \
   do { \
      printf("%p <- %5i <- del   | %s-%i\n", \
             obj, E_OBJECT(obj)->references, \
             __FILE__, num); \
   } while (0)
#else
# define REFD(obj, num)
# define UNREFD(obj, num)
# define DELD(obj, num)
#endif

/* for disabling compiler warnings
 * http://stackoverflow.com/questions/3378560/how-to-disable-gcc-warnings-for-a-few-lines-of-code
 */
#define DIAG_STR(s) #s
#define DIAG_JOINSTR(x,y) DIAG_STR(x ## y)
#ifdef _MSC_VER
#define DIAG_DO_PRAGMA(x) __pragma (#x)
#define DIAG_PRAGMA(compiler,x) DIAG_DO_PRAGMA(warning(x))
#else
#define DIAG_DO_PRAGMA(x) _Pragma (#x)
#define DIAG_PRAGMA(compiler,x) DIAG_DO_PRAGMA(compiler diagnostic x)
#endif
#if defined(__clang__)
# define DISABLE_WARNING(gcc_unused,clang_option,msvc_unused) DIAG_PRAGMA(clang,push) DIAG_PRAGMA(clang,ignored DIAG_JOINSTR(-W,clang_option))
# define ENABLE_WARNING(gcc_unused,clang_option,msvc_unused) DIAG_PRAGMA(clang,pop)
#elif defined(_MSC_VER)
# define DISABLE_WARNING(gcc_unused,clang_unused,msvc_errorcode) DIAG_PRAGMA(msvc,push) DIAG_DO_PRAGMA(warning(disable:##msvc_errorcode))
# define ENABLE_WARNING(gcc_unused,clang_unused,msvc_errorcode) DIAG_PRAGMA(msvc,pop)
#elif defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
# define DISABLE_WARNING(gcc_option,clang_unused,msvc_unused) DIAG_PRAGMA(GCC,push) DIAG_PRAGMA(GCC,ignored DIAG_JOINSTR(-W,gcc_option))
# define ENABLE_WARNING(gcc_option,clang_unused,msvc_unused) DIAG_PRAGMA(GCC,pop)
#else
# define DISABLE_WARNING(gcc_option,clang_unused,msvc_unused) DIAG_PRAGMA(GCC,ignored DIAG_JOINSTR(-W,gcc_option))
# define ENABLE_WARNING(gcc_option,clang_option,msvc_unused) DIAG_PRAGMA(GCC,warning DIAG_JOINSTR(-W,gcc_option))
#endif
#endif

#endif
