#include "e.h"
#ifdef __linux__
# include <sys/prctl.h>
#endif

#include "valgrind.h"

#define MAX_LEVEL 80

#define TS_DO
#ifdef TS_DO
# define TS(x)                                                    \
  {                                                               \
     t1 = ecore_time_unix_get();                                  \
     printf("ESTART: %1.5f [%1.5f] - %s\n", t1 - t0, t1 - t2, x); \
     t2 = t1;                                                     \
  }
static double t0, t1, t2;
#else
# define TS(x)
#endif

/*
 * i need to make more use of these when i'm baffled as to when something is
 * up. other hooks:
 *
 *      void *(*__malloc_hook)(size_t size, const void *caller);
 *
 *      void *(*__realloc_hook)(void *ptr, size_t size, const void *caller);
 *
 *      void *(*__memalign_hook)(size_t alignment, size_t size,
 *                               const void *caller);
 *
 *      void (*__free_hook)(void *ptr, const void *caller);
 *
 *      void (*__malloc_initialize_hook)(void);
 *
 *      void (*__after_morecore_hook)(void);
 *

   static void my_init_hook(void);
   static void my_free_hook(void *p, const void *caller);

   static void (*old_free_hook)(void *ptr, const void *caller) = NULL;
   void (*__free_hook)(void *ptr, const void *caller);

   void (*__malloc_initialize_hook) (void) = my_init_hook;
   static void
   my_init_hook(void)
   {
   old_free_hook = __free_hook;
   __free_hook = my_free_hook;
   }

   //void *magicfree = NULL;

   static void
   my_free_hook(void *p, const void *caller)
   {
   __free_hook = old_free_hook;
   //   if ((p) && (p == magicfree))
   //     {
   //	printf("CAUGHT!!!!! %p ...\n", p);
   //	abort();
   //     }
   free(p);
   __free_hook = my_free_hook;
   }
 */

/* local function prototypes */
static void      _e_main_shutdown(int errcode);
static void      _e_main_shutdown_push(int (*func)(void));
static void      _e_main_parse_arguments(int argc, char **argv);
static Eina_Bool _e_main_cb_signal_exit(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev EINA_UNUSED);
static Eina_Bool _e_main_cb_signal_hup(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev EINA_UNUSED);
static Eina_Bool _e_main_cb_signal_user(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev);
static int       _e_main_dirs_init(void);
static int       _e_main_dirs_shutdown(void);
static int       _e_main_path_init(void);
static int       _e_main_path_shutdown(void);
static int       _e_main_screens_init(void);
static int       _e_main_screens_shutdown(void);
static void      _e_main_desk_save(void);
static void      _e_main_desk_restore(void);
static void      _e_main_efreet_paths_init(void);
static void      _e_main_modules_load(Eina_Bool safe_mode);
static Eina_Bool _e_main_cb_idle_before(void *data EINA_UNUSED);
static Eina_Bool _e_main_cb_idle_after(void *data EINA_UNUSED);
static Eina_Bool _e_main_cb_startup_fake_end(void *data EINA_UNUSED);

/* local variables */
static Eina_Bool really_know = EINA_FALSE;
static Eina_Bool locked = EINA_FALSE;
static Eina_Bool inloop = EINA_FALSE;
static jmp_buf x_fatal_buff;

static int _e_main_lvl = 0;
static int(*_e_main_shutdown_func[MAX_LEVEL]) (void);

static Ecore_Idle_Enterer *_idle_before = NULL;
static Ecore_Idle_Enterer *_idle_after = NULL;

static Ecore_Event_Handler *mod_init_end = NULL;

/* external variables */
E_API Eina_Bool e_precache_end = EINA_FALSE;
E_API Eina_Bool x_fatal = EINA_FALSE;
E_API Eina_Bool good = EINA_FALSE;
E_API Eina_Bool evil = EINA_FALSE;
E_API Eina_Bool starting = EINA_TRUE;
E_API Eina_Bool stopping = EINA_FALSE;
E_API Eina_Bool restart = EINA_FALSE;
E_API Eina_Bool e_nopause = EINA_FALSE;
E_API Eina_Bool after_restart = EINA_FALSE;
E_API Eina_Bool e_main_loop_running = EINA_FALSE;
EINTERN const char *e_first_frame = NULL;
EINTERN double e_first_frame_start_time = -1;

static Eina_Bool
_xdg_check_str(const char *env, const char *str)
{
   const char *p;
   size_t len;

   len = strlen(str);
   for (p = strstr(env, str); p; p++, p = strstr(p, str))
     {
        if ((!p[len]) || (p[len] == ':')) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_xdg_data_dirs_augment(void)
{
   const char *s;
   const char *p = e_prefix_get();
   char newpath[PATH_MAX], buf[PATH_MAX + PATH_MAX + 200];

   if (!p) return;

   s = getenv("XDG_DATA_DIRS");
   if (s)
     {
        Eina_Bool pfxdata, pfx;

        pfxdata = !_xdg_check_str(s, e_prefix_data_get());
        snprintf(newpath, sizeof(newpath), "%s/share", p);
        pfx = !_xdg_check_str(s, newpath);
        if (pfxdata || pfx)
          {
             snprintf(buf, sizeof(buf), "%s%s%s%s%s",
               pfxdata ? e_prefix_data_get() : "",
               pfxdata ? ":" : "",
               pfx ? newpath : "",
               pfx ? ":" : "",
               s);
             e_util_env_set("XDG_DATA_DIRS", buf);
          }
     }
   else
     {
        snprintf(buf, sizeof(buf), "%s:%s/share:/usr/local/share:/usr/share", e_prefix_data_get(), p);
        e_util_env_set("XDG_DATA_DIRS", buf);
     }

   s = getenv("XDG_CONFIG_DIRS");
   snprintf(newpath, sizeof(newpath), "%s/etc/xdg", p);
   if (s)
     {
        if (!_xdg_check_str(s, newpath))
          {
             snprintf(buf, sizeof(buf), "%s:%s", newpath, s);
             e_util_env_set("XDG_CONFIG_DIRS", buf);
          }
     }
   else
     {
        snprintf(buf, sizeof(buf), "%s:/etc/xdg", newpath);
        e_util_env_set("XDG_CONFIG_DIRS", buf);
     }

   // N.B. XDG_RUNTIME_DIR is taken care of in e_start

   /* set menu prefix so we get our e menu */
   if (!getenv("XDG_MENU_PREFIX"))
     {
        e_util_env_set("XDG_MENU_PREFIX", "e-");
     }
}

static Eina_Bool
_precache_file(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data EINA_UNUSED, void *fdata)
{
   Eina_List **precache_files = fdata;
   *precache_files = eina_list_append(*precache_files, strdup(key));
   return EINA_TRUE;
}

static void *
_precache_thread(void *data, Eina_Thread thr EINA_UNUSED)
{
   Eina_List *precache_files = data;
   char *path;
   double t = ecore_time_get();
   unsigned int sum = 0;
   unsigned int reads = 0;


   printf("PRECACHE: BEGIN\n");
   EINA_LIST_FREE(precache_files, path)
     {
        double tt = ecore_time_get();
        FILE *f = fopen(path, "r");
        unsigned char buf[4096];

        if (f)
          {
             size_t sz;

             while ((sz = fread(buf, 1, sizeof(buf), f)) > 0)
               {
                  reads ++;
                  sum = ((sum << 1) ^ buf[0]) + reads;
               }
             fclose(f);
          }
        printf("PRECACHE: [%1.5f] [%s] DONE\n", ecore_time_get() - tt, path);
        free(path);
     }
   printf("PRECACHE: TOTAL [%1.5f]\n", ecore_time_get() - t);
   printf("PRECACHE: SUM=%08x, READS=%i\n", sum, reads);
   return NULL;
}

/* externally accessible functions */
int
main(int argc, char **argv)
{
   Eina_Bool nostartup = EINA_FALSE;
   Eina_Bool safe_mode = EINA_FALSE;
   Eina_Bool waslocked = EINA_FALSE;
   Eina_Stringshare *strshare;
   char *s = NULL;
   struct sigaction action;

#ifdef __linux__
# ifdef PR_SET_PTRACER
#  ifdef PR_SET_PTRACER_ANY
   prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
#  endif
# endif
#endif

#ifdef TS_DO
   t0 = t1 = t2 = ecore_time_unix_get();
#endif
   TS("Begin Startup");

   /* trap deadly bug signals and allow some form of sane recovery */
   /* or ability to gdb attach and debug at this point - better than your */
   /* wm/desktop vanishing and not knowing what happened */
   if (!getenv("NOTIFY_SOCKET"))
     {
        TS("Signal Trap");
        action.sa_sigaction = e_sigseg_act;
	action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGSEGV, &action, NULL);

	action.sa_sigaction = e_sigill_act;
	action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGILL, &action, NULL);

	action.sa_sigaction = e_sigfpe_act;
	action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGFPE, &action, NULL);

#ifndef HAVE_WAYLAND_ONLY
	action.sa_sigaction = e_sigbus_act;
	action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGBUS, &action, NULL);
#endif

	action.sa_sigaction = e_sigabrt_act;
	action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGABRT, &action, NULL);
	TS("Signal Trap Done");
     }

   TS("Eina Init");
   if (!eina_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Eina!\n"));
        _e_main_shutdown(101);
     }
   eina_file_statgen_enable();

   _e_main_shutdown_push(eina_shutdown);

   if (!e_log_init())
     {
        e_error_message_show(_("Enlightenment could not create a logging domain!\n"));
        _e_main_shutdown(101);
     }
#ifdef TS_DO
#undef TS
# define TS(x)                                                    \
  {                                                               \
     t1 = ecore_time_unix_get();                                  \
     printf("ESTART: %1.5f [%1.5f] - %s\n", t1 - t0, t1 - t2, x); \
     t2 = t1;                                                     \
  }
#endif
   TS("Eina Init Done");
   _e_main_shutdown_push(e_log_shutdown);

   TS("Determine Prefix");
   if (!e_prefix_determine(argv[0]))
     {
        fprintf(stderr,
                "ERROR: Enlightenment cannot determine it's installed\n"
                "       prefix from the system or argv[0].\n"
                "       This is because it is not on Linux AND has been\n"
                "       executed strangely. This is unusual.\n");
     }
   TS("Determine Prefix Done");

   /* Eio's eio_init internally calls efreet_init. Set XDG_MENU_PREFIX here      */
   /* else efreet's efreet_menu_prefix symbol is set erroneously during eio_init. */
   _xdg_data_dirs_augment();

   /* for debugging by redirecting stdout of e to a log file to tail */
   setvbuf(stdout, NULL, _IONBF, 0);

   TS("Environment Variables");
   if (getenv("E_RESTART")) after_restart = EINA_TRUE;
   if (getenv("DESKTOP_STARTUP_ID"))
     e_util_env_set("DESKTOP_STARTUP_ID", NULL);
   e_util_env_set("E_RESTART_OK", NULL);
   e_util_env_set("PANTS", "ON");
   e_util_env_set("DESKTOP", "Enlightenment");
   e_util_env_set("XDG_CURRENT_DESKTOP", "Enlightenment");
   if (getenv("E_ALERT_FONT_DIR"))
     e_util_env_set("E_ALERT_FONT_DIR", NULL);
   if (getenv("E_ALERT_SYSTEM_BIN"))
     e_util_env_set("E_ALERT_SYSTEM_BIN", NULL);
   strshare = eina_stringshare_printf("%s/enlightenment_askpass",
                                      e_prefix_bin_get());
   if (strshare)
     {
        e_util_env_set("SUDO_ASKPASS", strshare);
        e_util_env_set("SSH_ASKPASS", strshare);
        eina_stringshare_del(strshare);
     }
   TS("Environment Variables Done");

   TS("Parse Arguments");
   _e_main_parse_arguments(argc, argv);
   TS("Parse Arguments Done");

   /*** Initialize Core EFL Libraries We Need ***/

   TS("Eet Init");
   if (!eet_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Eet!\n"));
        _e_main_shutdown(101);
     }
   TS("Eet Init Done");
   _e_main_shutdown_push(eet_shutdown);

   TS("Ecore Init");
   if (!ecore_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Ecore!\n"));
        _e_main_shutdown(101);
     }
   TS("Ecore Init Done");
   _e_main_shutdown_push(ecore_shutdown);

   TS("E Comp Canvas Intercept Init");
   e_comp_canvas_intercept();
   TS("E Comp Canvas Intercept Init Done");

   e_first_frame = getenv("E_FIRST_FRAME");
   if (e_first_frame && e_first_frame[0])
     e_first_frame_start_time = ecore_time_get();
   else
     e_first_frame = NULL;

   TS("EFX Init");
   if (!e_efx_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize EFX!\n"));
        _e_main_shutdown(101);
     }
   TS("EFX Init Done");
   _e_main_shutdown_push((void*)e_efx_shutdown);

   TS("EIO Init");
   if (!eio_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize EIO!\n"));
        _e_main_shutdown(101);
     }
   TS("EIO Init Done");
   _e_main_shutdown_push(eio_shutdown);

   ecore_app_args_set(argc, (const char **)argv);

   TS("Ecore Event Handlers");
   if (!ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT,
                                _e_main_cb_signal_exit, NULL))
     {
        e_error_message_show(_("Enlightenment cannot set up an exit signal handler.\n"
                               "Perhaps you are out of memory?"));
        _e_main_shutdown(101);
     }
   if (!ecore_event_handler_add(ECORE_EVENT_SIGNAL_HUP,
                                _e_main_cb_signal_hup, NULL))
     {
        e_error_message_show(_("Enlightenment cannot set up a HUP signal handler.\n"
                               "Perhaps you are out of memory?"));
        _e_main_shutdown(101);
     }
   if (!ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                _e_main_cb_signal_user, NULL))
     {
        e_error_message_show(_("Enlightenment cannot set up a USER signal handler.\n"
                               "Perhaps you are out of memory?"));
        _e_main_shutdown(101);
     }
   TS("Ecore Event Handlers Done");

   TS("Ecore_File Init");
   if (!ecore_file_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Ecore_File!\n"));
        _e_main_shutdown(101);
     }
   TS("Ecore_File Init Done");
   _e_main_shutdown_push(ecore_file_shutdown);

   TS("Ecore_Con Init");
   if (!ecore_con_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Ecore_Con!\n"));
        _e_main_shutdown(101);
     }
   TS("Ecore_Con Init Done");
   _e_main_shutdown_push(ecore_con_shutdown);

   TS("Ecore_Ipc Init");
   if (!ecore_ipc_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Ecore_Ipc!\n"));
        _e_main_shutdown(101);
     }
   TS("Ecore_Ipc Init Done");
   _e_main_shutdown_push(ecore_ipc_shutdown);

   _idle_before = ecore_idle_enterer_before_add(_e_main_cb_idle_before, NULL);

   TS("Ecore_Evas Init");
   if (!ecore_evas_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Ecore_Evas!\n"));
        _e_main_shutdown(101);
     }
   TS("Ecore_Evas Init Done");
//   _e_main_shutdown_push(ecore_evas_shutdown);

   TS("Elementary Init");
   if (!elm_init(argc, argv))
     {
        e_error_message_show(_("Enlightenment cannot initialize Elementary!\n"));
        _e_main_shutdown(101);
     }
   TS("Elementary Init Done");
   //_e_main_shutdown_push(elm_shutdown);

   TS("Emotion Init");
   if (!emotion_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize Emotion!\n"));
        _e_main_shutdown(101);
     }
   TS("Emotion Init Done");
   /* triggers event flush: do not call */
   //_e_main_shutdown_push((void *)emotion_shutdown);

   /* e doesn't sync to compositor - it should be one */
   ecore_evas_app_comp_sync_set(0);

   TS("Ecore_Evas Engine Check");
#ifdef HAVE_WAYLAND_ONLY
   if (!ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_WAYLAND_SHM))
     {
        e_error_message_show(_("Enlightenment found ecore_evas doesn't support the Wayland SHM\n"
                               "rendering in Evas. Please check your installation of Evas and\n"
                                "Ecore and check they support the Wayland SHM rendering engine."));
        _e_main_shutdown(101);
     }
#else
   if (!ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_SOFTWARE_XCB))
     {
        if (!ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_SOFTWARE_XLIB))
          {
             e_error_message_show(_("Enlightenment found ecore_evas doesn't support the Software X11\n"
                                    "rendering in Evas. Please check your installation of Evas and\n"
                                    "Ecore and check they support the Software X11 rendering engine."));
             _e_main_shutdown(101);
          }
     }
#endif
   if (!ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_SOFTWARE_BUFFER))
     {
        e_error_message_show(_("Enlightenment found ecore_evas doesn't support the Software Buffer\n"
                               "rendering in Evas. Please check your installation of Evas and\n"
                               "Ecore and check they support the Software Buffer rendering engine."));
        _e_main_shutdown(101);
     }
   TS("Ecore_Evas Engine Check Done");

   edje_freeze();

   /*** Initialize E Subsystems We Need ***/

   TS("E Intl Init");
   if (!e_intl_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize E_Intl!\n"));
        _e_main_shutdown(101);
     }
   TS("E Intl Init Done");
   _e_main_shutdown_push(e_intl_shutdown);

#ifndef HAVE_WAYLAND_ONLY
   /* init white box of death alert */
   TS("E_Alert Init");
   if (!e_alert_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize its emergency alert system.\n"
                               "Have you set your DISPLAY variable?"));
        _e_main_shutdown(101);
     }
   TS("E_Alert Init Done");
   _e_main_shutdown_push(e_alert_shutdown);
#endif

   TS("E Directories Init");
   /* setup directories we will be using for configurations storage etc. */
   if (!_e_main_dirs_init())
     {
        e_error_message_show(_("Enlightenment cannot create directories in your home directory.\n"
                               "Perhaps you have no home directory or the disk is full?"));
        _e_main_shutdown(101);
     }
   TS("E Directories Init Done");
   _e_main_shutdown_push(_e_main_dirs_shutdown);

   TS("E_Filereg Init");
   if (!e_filereg_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its file registry system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Filereg Init Done");
   _e_main_shutdown_push(e_filereg_shutdown);

   TS("E_Config Init");
   if (!e_config_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its config system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Config Init Done");
   _e_main_shutdown_push(e_config_shutdown);

   if (e_config->xsettings.match_e17_theme)
     {

        /* KDE5 applications don't understand anything other then gnome or kde     */
        /* They expect everyone else to set QT_QPA_PLATFORMTHEME to tell them how  */
        /* to theme there apps otherwise they use a fallback mode which results in */
        /* missing icons and a inability to change the appearance of applications  */
        /* see https://bugzilla.suse.com/show_bug.cgi?id=920792 for more info.     */
        /* There are two sensible defaults for this variable, "kde" which will     */
        /* make apps appear the same as they do if they are run in kde. and gtk2   */
        /* which will make kde applications follow the gtk/gnome theme, we have    */
        /* decided on choosing gtk2 as it means that kde/qt apps will follow the   */
        /* app and icon theme set in the enlightenment settings dialog. Some users */
        /* who wish to use Qt apps without any gnome or gtk usage may choose to    */
        /* install qt5ct and overwrite this variable with qt5ct and use that to    */
        /* configure there Qt5 applications.                                       */
        e_util_env_set("QT_QPA_PLATFORMTHEME", "gtk2");
        e_util_env_set("QT_STYLE_OVERRIDE", "gtk2");
     }
   // make fonts NOT BLURRY. after 35 (v 38, v40 of interpreter) fonts become
   // horizontally blurry - they seemingly want the interpreter for hinting to
   // sub-pixel position on sub-pixel boundaries. this ends up with blurry
   // horizontal positioning/hinting that is on a sub-pixel. yes - this
   // requires logging out and logging in to get e to not set this env var.
   // for now that's good enough. the aim is to get everyone to render the
   // same way and this does it. efl, gtk, qt, chromium, firtefox, ...
   if (e_config->scale.set_xapp_dpi)
     {
        s = getenv("FREETYPE_PROPERTIES");
        if (!s)
          {
             e_util_env_set("FREETYPE_PROPERTIES", "truetype:interpreter-version=35");
          }
     }

   TS("E_Env Init");
   if (!e_env_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its environment.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Env Init Done");
   _e_main_shutdown_push(e_env_shutdown);

   efreet_desktop_environment_set(e_config->desktop_environment);
   e_util_env_set("E_ICON_THEME", e_config->icon_theme);
   ecore_exe_run_priority_set(e_config->priority);
   locked |= e_config->desklock_start_locked;

   s = getenv("E_DESKLOCK_LOCKED");
   if ((s) && (!strcmp(s, "locked"))) waslocked = EINA_TRUE;
   putenv("E_DESKLOCK_LOCKED=");

   TS("E Paths Init");
   if (!_e_main_path_init())
     {
        e_error_message_show(_("Enlightenment cannot set up paths for finding files.\n"
                               "Perhaps you are out of memory?"));
        _e_main_shutdown(101);
     }
   TS("E Paths Init Done");
   _e_main_shutdown_push(_e_main_path_shutdown);

   TS("E_Precache");
   if (!getenv("E_NO_PRECACHE"))
     {
        const Eina_List *l;
        const Eina_List *theme_items = elm_theme_list_get(NULL);
        Eina_List *precache_files = NULL;
        Eina_Hash *files = eina_hash_string_superfast_new(NULL);
        Eina_Thread thr;
        int scr, dx, dy;
        // find all theme edj files to precache
        EINA_LIST_FOREACH(theme_items, l, s)
          {
             Eina_Bool search = EINA_FALSE;
             char *path = elm_theme_list_item_path_get(s, &search);
             if (path)
               {
                  eina_hash_del(files, path, files);
                  eina_hash_add(files, path, files);
                  free(path);
               }
          }
        // go over the first 4 screens and all desks and find possible
        // background files and add them to our hash to precache
        for (scr = 0; scr < 4; scr++)
          {
             for (dy = 0; dy < e_config->zone_desks_y_count; dy++)
               {
                  for (dx = 0; dx < e_config->zone_desks_x_count; dx++)
                    {
                       const char *bgfile = e_bg_file_get(scr, dx, dy);
                       eina_hash_del(files, bgfile, files);
                       eina_hash_add(files, bgfile, files);
                       eina_stringshare_del(bgfile);
                    }
               }
          }
        eina_hash_foreach(files, _precache_file, &precache_files);
        eina_hash_free(files);
        printf("PRECACHE: SPAWN\n");
        if (!eina_thread_create(&thr, EINA_THREAD_BACKGROUND, -1,
                                _precache_thread, precache_files))
          {
             ERR("Can't spawn file precache thread");
          }
     }
   TS("E_Precache Done");

   TS("E_Ipc Init");
   if (!e_ipc_init()) _e_main_shutdown(101);
   TS("E_Ipc Init Done");
   _e_main_shutdown_push(e_ipc_shutdown);

   TS("E_Font Init");
   if (!e_font_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its font system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Font Init Done");
   _e_main_shutdown_push(e_font_shutdown);

   TS("E_Font Apply");
   e_font_apply();
   TS("E_Font Apply Done");

   TS("E_Theme Init");
   if (!e_theme_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its theme system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Theme Init Done");
   _e_main_shutdown_push(e_theme_shutdown);

   TS("E_Moveresize Init");
   e_moveresize_init();
   TS("E_Moveresize Init Done");
   _e_main_shutdown_push(e_moveresize_shutdown);

   TS("E_Msgbus Init");
   if (!e_msgbus_init())
     {
        if (!getenv("E_NO_DBUS_SESSION"))
          {
             e_error_message_show(_("Enlightenment cannot create a dbus session connection.\n"
                                    "At best this will break many things, at worst it will hard lock your machine.\n"
                                    "If you're sure you know what you're doing, export E_NO_DBUS_SESSION=1"));
             _e_main_shutdown(101);
          }
     }
   else
     _e_main_shutdown_push(e_msgbus_shutdown);
   TS("E_Msgbus Init Done");

   TS("Efreet Init");
   if (!efreet_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize the FDO desktop system.\n"
                               "Perhaps you lack permissions on ~/.cache/efreet or are\n"
                               "out of memory or disk space?"));
        _e_main_shutdown(101);
     }
   TS("Efreet Init Done");
   _e_main_shutdown_push(efreet_shutdown);

   TS("E_Intl Post Init");
   if (!e_intl_post_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its intl system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Intl Post Init Done");
   _e_main_shutdown_push(e_intl_post_shutdown);

   TS("E_Configure Init");
   e_configure_init();
   TS("E_Configure Init Done");

   e_screensaver_preinit();

   TS("E_Actions Init");
   if (!e_actions_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its actions system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Actions Init Done");
   _e_main_shutdown_push(e_actions_shutdown);

   /* these just add event handlers and can't fail
    * timestamping them is dumb.
    */
   e_zone_init();
   e_desk_init();
   e_exehist_init();

   TS("E_System Init");
   if (!e_system_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize the Privilege System access system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_System Init Done");
   _e_main_shutdown_push(e_system_shutdown);

   TS("E_Powersave Init");
   if (!e_powersave_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its powersave modes.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Powersave Init Done");
   _e_main_shutdown_push(e_powersave_shutdown);

   TS("Screens Init");
   if (!_e_main_screens_init())
     {
        e_error_message_show(_("Enlightenment set up window management for all the screens on your system\n"
                               "failed. Perhaps another window manager is running?\n"));
        _e_main_shutdown(101);
     }
   TS("Screens Init Done");
   _e_main_shutdown_push(_e_main_screens_shutdown);

   TS("E_Pointer Init");
   if (!e_pointer_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its pointer system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Pointer Init Done");
   _e_main_shutdown_push(e_pointer_shutdown);
   e_menu_init();

   TS("E_Scale Init");
   if (!e_scale_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its scale system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Scale Init Done");
   _e_main_shutdown_push(e_scale_shutdown);

   TS("E_Splash Init");
   if (!e_init_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its init screen.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Splash Init Done");
   _e_main_shutdown_push(e_init_shutdown);
   if (!after_restart)
     {
        TS("E_Splash Show");
        e_init_show();
        TS("E_Splash Show Done");
     }

   TS("Add Icon Extensions");
   efreet_icon_extension_add(".svg");
   efreet_icon_extension_add(".svgz");
   efreet_icon_extension_add(".svg.gz");
   efreet_icon_extension_add(".jpg");
   efreet_icon_extension_add(".jpeg");
   efreet_icon_extension_add(".png");
   efreet_icon_extension_add(".edj");
   TS("Add Icon Extensions Done");

   TS("E_Acpi Init");
   e_acpi_init();
   TS("E_Acpi Init Done");
   _e_main_shutdown_push(e_acpi_shutdown);

   TS("E_Backlight Init");
   if (!e_backlight_init())
     {
        e_error_message_show(_("Enlightenment cannot configure the backlight.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Backlight Init Done");

   TS("E_Dpms Init");
   if (!e_dpms_init())
     {
        e_error_message_show(_("Enlightenment cannot configure the DPMS settings.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Dpms Init Done");
   _e_main_shutdown_push(e_dpms_shutdown);

   TS("E_Desklock Init");
   if (!e_desklock_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its desk locking system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Desklock Init Done");
   _e_main_shutdown_push(e_desklock_shutdown);

   if (waslocked || (locked && ((!after_restart))))
     {
        e_desklock_show_manual(EINA_TRUE);
        e_screensaver_update();
     }

   TS("Efreet Paths");
   _e_main_efreet_paths_init();
   TS("Efreet Paths Done");

   TS("E_Sys Init");
   if (!e_sys_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize the System Command system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Sys Init Done");
   _e_main_shutdown_push(e_sys_shutdown);

   TS("E_Exec Init");
   if (!e_exec_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its exec system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Exec Init Done");

   TS("E_Comp Freeze");
   e_comp_all_freeze();
   TS("E_Comp Freeze Done");

   TS("E_Fm2 Init");
   if (!e_fm2_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize the File manager.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Fm2 Init Done");
   _e_main_shutdown_push(e_fm2_shutdown);

   TS("E_Msg Init");
   if (!e_msg_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its msg system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Msg Init Done");
   _e_main_shutdown_push(e_msg_shutdown);

   TS("E_Grabinput Init");
   if (!e_grabinput_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its grab input handling system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Grabinput Init Done");
   _e_main_shutdown_push(e_grabinput_shutdown);

   TS("E_Module Init");
   if (!e_module_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its module system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Module Init Done");
   _e_main_shutdown_push(e_module_shutdown);

   TS("E_Remember Init");
   if (!e_remember_init(after_restart ? E_STARTUP_RESTART : E_STARTUP_START))
     {
        e_error_message_show(_("Enlightenment cannot setup remember settings.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Remember Init Done");
   _e_main_shutdown_push(e_remember_shutdown);

   TS("E_Gadcon Init");
   if (!e_gadcon_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its gadget control system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Gadcon Init Done");
   _e_main_shutdown_push(e_gadcon_shutdown);

   TS("E_Toolbar Init");
   if (!e_toolbar_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its toolbars.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Toolbar Init Done");
   _e_main_shutdown_push(e_toolbar_shutdown);

   TS("E_Bg Init");
   if (!e_bg_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its desktop background system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Bg Init Done");
   _e_main_shutdown_push(e_bg_shutdown);

   TS("E_Mouse Init");
   if (!e_mouse_update())
     {
        e_error_message_show(_("Enlightenment cannot configure the mouse settings.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Mouse Init Done");

   TS("E_Bindings Init");
   if (!e_bindings_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its bindings system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Bindings Init Done");
   _e_main_shutdown_push(e_bindings_shutdown);

   TS("E_Thumb Init");
   if (!e_thumb_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize the Thumbnailing system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Thumb Init Done");
   _e_main_shutdown_push(e_thumb_shutdown);

   TS("E_Icon Init");
   if (!e_icon_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize the Icon Cache system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Icon Init Done");
   _e_main_shutdown_push(e_icon_shutdown);

   TS("E_Update Init");
   if (!e_update_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize the Update system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Update Init Done");
   _e_main_shutdown_push(e_update_shutdown);

   TS("E_Deskenv Init");
   if (!e_deskenv_init())
     {
        e_error_message_show(_("Enlightenment cannot initialize its desktop environment.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Deskenv Init Done");
   _e_main_shutdown_push(e_deskenv_shutdown);

   TS("E_Order Init");
   if (!e_order_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its order file system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Order Init Done");
   _e_main_shutdown_push(e_order_shutdown);

   TS("E_Comp_Canvas Keys Grab");
   e_comp_canvas_keys_grab();
   TS("E_Comp_Canvas Keys Grab Done");
#ifdef HAVE_ELPUT
   TS("E_Gesture Init");
   e_gesture_init();
   TS("E_Gesture Init Done");
   _e_main_shutdown_push(e_gesture_shutdown);
#endif

   TS("Run Startup Apps");
   if (!nostartup)
     {
        if (after_restart)
          e_startup_mode_set(E_STARTUP_RESTART);
        else
          e_startup_mode_set(E_STARTUP_START);
     }
   TS("Run Startup Apps Done");

   TS("E_Comp Thaw");
   e_comp_all_thaw();
   TS("E_Comp Thaw Done");

   TS("E_Test Init");
   e_test();
   TS("E_Test Done");

   TS("Load Modules");
   _e_main_modules_load(safe_mode);
   TS("Load Modules Done");

   TS("E_Shelf Init");
   if (!e_shelf_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its module system.\n"));
        _e_main_shutdown(101);
     }
   e_shelf_config_update();
   TS("E_Shelf Init Done");

   _idle_after = ecore_idle_enterer_add(_e_main_cb_idle_after, NULL);

   e_init_status_set(_("Welcome to Enlightenment"));

   starting = EINA_FALSE;
   inloop = EINA_TRUE;

   e_util_env_set("E_RESTART", "1");

   if (!after_restart)
     ecore_timer_add(2.0, _e_main_cb_startup_fake_end, NULL);

   if (after_restart)
     {
        E_LIST_FOREACH(e_comp->zones, e_comp_canvas_zone_restarted);
     }

//   e_watchdog_begin();
   TS("MAIN LOOP AT LAST");
   if (!setjmp(x_fatal_buff))
     {
        e_main_loop_running = EINA_TRUE;
        ecore_main_loop_begin();
     }
   else
     CRI("FATAL: X Died. Connection gone. Abbreviated Shutdown\n");
//   e_watchdog_end();
   e_main_loop_running = EINA_FALSE;

   inloop = EINA_FALSE;
   stopping = EINA_TRUE;

   //if (!x_fatal) e_canvas_idle_flush();

   e_config_save_flush();
   _e_main_desk_save();
   e_remember_internal_save();
   e_comp_internal_save();

   _e_main_shutdown(0);

   if (restart)
     {
        e_system_shutdown();
        exit(111); // return code so e_start restrts e freshly
     }

   e_prefix_shutdown();

   return 0;
}

E_API double
e_main_ts(const char *str)
{
   double ret;
   t1 = ecore_time_unix_get();
   printf("ESTART: %1.5f [%1.5f] - %s\n", t1 - t0, t1 - t2, str);
   ret = t1 - t2;
   t2 = t1;
   return ret;
}

/* local functions */
static void
_e_main_shutdown(int errcode)
{
   int i = 0;

   printf("E: Begin Shutdown Procedure!\n");

   if (_idle_before) ecore_idle_enterer_del(_idle_before);
   _idle_before = NULL;
   if (_idle_after) ecore_idle_enterer_del(_idle_after);
   _idle_after = NULL;

   for (i = (_e_main_lvl - 1); i >= 0; i--)
     (*_e_main_shutdown_func[i])();
   if (errcode != 0) exit(errcode);
}

static void
_e_main_shutdown_push(int (*func)(void))
{
   _e_main_lvl++;
   if (_e_main_lvl > MAX_LEVEL)
     {
        _e_main_lvl--;
        e_error_message_show("WARNING: too many init levels. MAX = %i\n",
                             MAX_LEVEL);
        return;
     }
   _e_main_shutdown_func[_e_main_lvl - 1] = func;
}

static void
_e_main_parse_arguments(int argc, char **argv)
{
   char *s = NULL;
   int i = 0;

   /* handle some command-line parameters */
   for (i = 1; i < argc; i++)
     {
        if ((!strcmp(argv[i], "-display")) && (i < (argc - 1)))
          {
             i++;
             e_util_env_set("DISPLAY", argv[i]);
          }
        else if ((!strcmp(argv[i], "-fake-xinerama-screen")) && (i < (argc - 1)))
          {
             int x, y, w, h;

             i++;
             if (sscanf(argv[i], "%ix%i+%i+%i", &w, &h, &x, &y) == 4)
               e_xinerama_fake_screen_add(x, y, w, h);
          }
        else if (!strcmp(argv[i], "-good"))
          {
             good = EINA_TRUE;
             evil = EINA_FALSE;
             printf("LA LA LA\n");
          }
        else if (!strcmp(argv[i], "-evil"))
          {
             good = EINA_FALSE;
             evil = EINA_TRUE;
             printf("MUHAHAHAHHAHAHAHAHA\n");
          }
        else if (!strcmp(argv[i], "-psychotic"))
          {
             good = EINA_TRUE;
             evil = EINA_TRUE;
             printf("MUHAHALALALALALALALA\n");
          }
        else if ((!strcmp(argv[i], "-profile")) && (i < (argc - 1)))
          {
             i++;
             if (!getenv("E_CONF_PROFILE"))
               e_util_env_set("E_CONF_PROFILE", argv[i]);
          }
        else if (!strcmp(argv[i], "-i-really-know-what-i-am-doing-and-accept-full-responsibility-for-it"))
          really_know = EINA_TRUE;
        else if (!strcmp(argv[i], "-locked"))
          locked = EINA_TRUE;
        else if (!strcmp(argv[i], "-nopause"))
          e_nopause = EINA_TRUE;
        else if ((!strcmp(argv[i], "-version")) ||
                 (!strcmp(argv[i], "--version")))
          {
             printf(_("Version: %s\n"), PACKAGE_VERSION);
             _e_main_shutdown(0);
          }
        else if ((!strcmp(argv[i], "-h")) ||
                 (!strcmp(argv[i], "-help")) ||
                 (!strcmp(argv[i], "--help")))
          {
             printf
               (_(
                 "Options:\n"
                 "\t-display DISPLAY\n"
                 "\t\tConnect to display named DISPLAY.\n"
                 "\t\tEG: -display :1.0\n"
                 "\t-fake-xinerama-screen WxH+X+Y\n"
                 "\t\tAdd a FAKE xinerama screen (instead of the real ones)\n"
                 "\t\tgiven the geometry. Add as many as you like. They all\n"
                 "\t\treplace the real xinerama screens, if any. This can\n"
                 "\t\tbe used to simulate xinerama.\n"
                 "\t\tEG: -fake-xinerama-screen 800x600+0+0 -fake-xinerama-screen 800x600+800+0\n"
                 "\t-profile CONF_PROFILE\n"
                 "\t\tUse the configuration profile CONF_PROFILE instead of the user selected default or just \"default\".\n"
                 "\t-good\n"
                 "\t\tBe good.\n"
                 "\t-evil\n"
                 "\t\tBe evil.\n"
                 "\t-psychotic\n"
                 "\t\tBe psychotic.\n"
                 "\t-locked\n"
                 "\t\tStart with desklock on, so password will be asked.\n"
                 "\t-i-really-know-what-i-am-doing-and-accept-full-responsibility-for-it\n"
                 "\t\tIf you need this help, you don't need this option.\n"
                 "\t-version\n"
                 )
               );
             _e_main_shutdown(0);
          }
     }

   /* fix up DISPLAY to be :N.0 if no .screen is in it */
   s = getenv("DISPLAY");
   if (s)
     {
        char *p, buff[4096];

        if (!(p = strrchr(s, ':')))
          {
             snprintf(buff, sizeof(buff), "%s:0.0", s);
             e_util_env_set("DISPLAY", buff);
          }
        else
          {
             if (!strrchr(p, '.'))
               {
                  snprintf(buff, sizeof(buff), "%s.0", s);
                  e_util_env_set("DISPLAY", buff);
               }
          }
     }

   /* we want to have been launched by enlightenment_start. there is a very */
   /* good reason we want to have been launched this way, thus check */
   if ((!getenv("E_START")) && (!RUNNING_ON_VALGRIND))
     {
        e_error_message_show(_("You are executing enlightenment directly. This is\n"
                               "bad. Please do not execute the \"enlightenment\"\n"
                               "binary. Use the \"enlightenment_start\" launcher. It\n"
                               "will handle setting up environment variables, paths,\n"
                               "and launching any other required services etc.\n"
                               "before enlightenment itself begins running.\n"));
        _e_main_shutdown(101);
     }
}

EINTERN void
_e_main_cb_x_fatal(void *data EINA_UNUSED)
{
   fprintf(stderr, "X I/O Error - fatal. Exiting.\n");
   exit(101);
}

static Eina_Bool
_e_main_cb_signal_exit(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev EINA_UNUSED)
{
   /* called on ctrl-c, kill (pid) (also SIGINT, SIGTERM and SIGQIT) */
   e_sys_action_do(E_SYS_EXIT, NULL);
   return ECORE_CALLBACK_RENEW;
}

static Ecore_Timer *hup_timer = NULL;

static Eina_Bool
_cb_hup_timer(void *data EINA_UNUSED)
{
   hup_timer = NULL;
   if (!e_sys_on_the_way_out_get()) e_sys_action_do(E_SYS_RESTART, NULL);
   return EINA_FALSE;
}

static Eina_Bool
_e_main_cb_signal_hup(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev EINA_UNUSED)
{
   if (hup_timer) ecore_timer_del(hup_timer);
   hup_timer = ecore_timer_add(0.5, _cb_hup_timer, NULL);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_main_cb_signal_user(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   Ecore_Event_Signal_User *e = ev;

   if (e->number == 1)
     {
//        E_Action *a = e_action_find("configuration");
//        if ((a) && (a->func.go)) a->func.go(NULL, NULL);
     }
   else if (e->number == 2)
     {
        // comp module has its own handler for this for enabling/disabling fps debug
     }
   return ECORE_CALLBACK_RENEW;

}

static int
_e_main_dirs_init(void)
{
   const char *base;
   const char *dirs[] =
   {
      "images",
      "fonts",
      "themes",
      "icons",
      "backgrounds",
      "applications",
      "applications/menu",
      "applications/menu/favorite",
      "applications/menu/all",
      "applications/bar",
      "applications/bar/default",
      "applications/startup",
      "applications/restart",
      "applications/trash",
      "applications/desk-lock",
      "applications/desk-unlock",
      "modules",
      "config",
      "locale",
      "input_methods",
      NULL
   };

   base = e_user_dir_get();
   if (ecore_file_mksubdirs(base, dirs) != sizeof(dirs) / sizeof(dirs[0]) - 1)
     {
        e_error_message_show("Could not create one of the required "
                             "subdirectories of '%s'\n", base);
        return 0;
     }

   return 1;
}

static int
_e_main_dirs_shutdown(void)
{
   return 1;
}

static int
_e_main_path_init(void)
{
   char buf[PATH_MAX];

   /* setup data paths */
   path_data = e_path_new();
   if (!path_data)
     {
        e_error_message_show("Cannot allocate path for path_data\n");
        return 0;
     }
   e_prefix_data_concat_static(buf, "data");
   e_path_default_path_append(path_data, buf);
   e_path_user_path_set(path_data, &(e_config->path_append_data));

   /* setup image paths */
   path_images = e_path_new();
   if (!path_images)
     {
        e_error_message_show("Cannot allocate path for path_images\n");
        return 0;
     }
   e_user_dir_concat_static(buf, "/images");
   e_path_default_path_append(path_images, buf);
   e_prefix_data_concat_static(buf, "data/images");
   e_path_default_path_append(path_images, buf);
   e_path_user_path_set(path_images, &(e_config->path_append_images));

   /* setup font paths */
   path_fonts = e_path_new();
   if (!path_fonts)
     {
        e_error_message_show("Cannot allocate path for path_fonts\n");
        return 0;
     }
   e_user_dir_concat_static(buf, "/fonts");
   e_path_default_path_append(path_fonts, buf);
   e_prefix_data_concat_static(buf, "data/fonts");
   e_path_default_path_append(path_fonts, buf);
   e_path_user_path_set(path_fonts, &(e_config->path_append_fonts));

   /* setup icon paths */
   path_icons = e_path_new();
   if (!path_icons)
     {
        e_error_message_show("Cannot allocate path for path_icons\n");
        return 0;
     }
   e_user_dir_concat_static(buf, "/icons");
   e_path_default_path_append(path_icons, buf);
   e_prefix_data_concat_static(buf, "data/icons");
   e_path_default_path_append(path_icons, buf);
   e_path_user_path_set(path_icons, &(e_config->path_append_icons));

   /* setup module paths */
   path_modules = e_path_new();
   if (!path_modules)
     {
        e_error_message_show("Cannot allocate path for path_modules\n");
        return 0;
     }
   e_user_dir_concat_static(buf, "/modules");
   e_path_default_path_append(path_modules, buf);
   snprintf(buf, sizeof(buf), "%s/enlightenment/modules", e_prefix_lib_get());
   e_path_default_path_append(path_modules, buf);
   /* FIXME: eventually this has to go - moduels should have installers that
    * add appropriate install paths (if not installed to user homedir) to
    * e's module search dirs
    */
   snprintf(buf, sizeof(buf), "%s/enlightenment/modules_extra", e_prefix_lib_get());
   e_path_default_path_append(path_modules, buf);
   e_path_user_path_set(path_modules, &(e_config->path_append_modules));

   /* setup background paths */
   path_backgrounds = e_path_new();
   if (!path_backgrounds)
     {
        e_error_message_show("Cannot allocate path for path_backgrounds\n");
        return 0;
     }
   e_user_dir_concat_static(buf, "/backgrounds");
   e_path_default_path_append(path_backgrounds, buf);
   e_prefix_data_concat_static(buf, "data/backgrounds");
   e_path_default_path_append(path_backgrounds, buf);
   e_path_user_path_set(path_backgrounds, &(e_config->path_append_backgrounds));

   path_messages = e_path_new();
   if (!path_messages)
     {
        e_error_message_show("Cannot allocate path for path_messages\n");
        return 0;
     }
   e_user_dir_concat_static(buf, "/locale");
   e_path_default_path_append(path_messages, buf);
   e_path_default_path_append(path_messages, e_prefix_locale_get());
   e_path_user_path_set(path_messages, &(e_config->path_append_messages));

   return 1;
}

static int
_e_main_path_shutdown(void)
{
   if (path_data)
     {
        e_object_del(E_OBJECT(path_data));
        path_data = NULL;
     }
   if (path_images)
     {
        e_object_del(E_OBJECT(path_images));
        path_images = NULL;
     }
   if (path_fonts)
     {
        e_object_del(E_OBJECT(path_fonts));
        path_fonts = NULL;
     }
   if (path_icons)
     {
        e_object_del(E_OBJECT(path_icons));
        path_icons = NULL;
     }
   if (path_modules)
     {
        e_object_del(E_OBJECT(path_modules));
        path_modules = NULL;
     }
   if (path_backgrounds)
     {
        e_object_del(E_OBJECT(path_backgrounds));
        path_backgrounds = NULL;
     }
   if (path_messages)
     {
        e_object_del(E_OBJECT(path_messages));
        path_messages = NULL;
     }
   return 1;
}

static int
_e_main_screens_init(void)
{
   TS("\tscreens: client");
   if (!e_client_init()) return 0;
   TS("E_Screensaver Init");
   if (!e_screensaver_init()) return 0;
   TS("\tscreens: client volume");
   if (!e_client_volume_init()) return 0;
   TS("\tscreens: win");
   if (!e_win_init()) return 0;
   TS("Compositor Init");
   if (!e_comp_init())
     {
        e_error_message_show(_("Enlightenment cannot create a compositor.\n"));
        _e_main_shutdown(101);
     }
   TS("Compositor Init Done");

   TS("Desk Restore");
   _e_main_desk_restore();
   TS("Desk Restore Done");

#ifndef HAVE_WAYLAND_ONLY
   TS("E_Dnd Init");
   if (!e_dnd_init())
     {
        e_error_message_show(_("Enlightenment cannot set up its dnd system.\n"));
        _e_main_shutdown(101);
     }
   TS("E_Dnd Init Done");
   _e_main_shutdown_push(e_dnd_shutdown);
#endif

   return 1;
}

static int
_e_main_screens_shutdown(void)
{
   e_win_shutdown();
   e_menu_shutdown();
   e_shelf_shutdown();
   e_comp_shutdown();
   e_client_volume_shutdown();
   e_screensaver_shutdown();
   e_client_shutdown();
   e_exehist_shutdown();
   e_backlight_shutdown();
   e_exec_shutdown();

   e_desk_shutdown();
   e_zone_shutdown();
   return 1;
}

static void
_e_main_desk_save(void)
{
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        const Eina_List *l;
        E_Zone *zone;
        char buf[256];
        unsigned int desk[2];
        Ecore_X_Atom a;

        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          {
             desk[0] = zone->desk_x_current;
             desk[1] = zone->desk_y_current;
             snprintf(buf, sizeof(buf), "E_DESK_%i", zone->num);
             a = ecore_x_atom_get(buf);
             ecore_x_window_prop_card32_set(e_comp->root, a, desk, 2);
          }
     }
#endif
}

static void
_e_main_desk_restore(void)
{
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        const Eina_List *l;
        E_Zone *zone;
        E_Client *ec;
        E_Desk *desk;
        int ret;
        char buf[256];
        unsigned int desks[2];
        Ecore_X_Atom a;

        // hide all windows first as desk show will restore
        E_CLIENT_FOREACH(ec)
          {
             ec->hidden = 1;
             evas_object_show(ec->frame);
             ec->changes.visible = 0;
             evas_object_hide(ec->frame);
             e_client_comp_hidden_set(ec, EINA_TRUE);
          }
        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          {
             desk = NULL;

             snprintf(buf, sizeof(buf), "E_DESK_%i", zone->num);
             a = ecore_x_atom_get(buf);
             ret = ecore_x_window_prop_card32_get(e_comp->root, a, desks, 2);
             if (ret == 2)
               {
                  desk = e_desk_at_xy_get(zone, desks[0], desks[1]);
                  if (desk) e_desk_show(desk);
               }
             if (!desk) desk = e_desk_current_get(zone);
             if (desk)
               {
                  // ensure windows for this desk are shown
                  E_CLIENT_REVERSE_FOREACH(ec)
                    {
                       if (ec->iconic) continue;
                       if ((ec->desk == desk) || (ec->sticky))
                         {
                            ec->hidden = 0;
                            e_client_comp_hidden_set(ec, ec->hidden || ec->shaded);
                            evas_object_show(ec->frame);
                         }
                    }
               }
          }
        E_CLIENT_REVERSE_FOREACH(ec)
          {
             if ((!e_client_util_ignored_get(ec)) &&
                 e_client_util_desk_visible(ec, e_desk_current_get(ec->zone)))
               {
                  ec->want_focus = ec->take_focus = 1;
                  break;
               }
          }
     }
#endif
}

static void
_e_main_efreet_paths_init(void)
{
   Eina_List **list;

   if ((list = efreet_icon_extra_list_get()))
     {
        char buff[PATH_MAX];

        e_user_dir_concat_static(buff, "icons");
        *list = eina_list_prepend(*list, (void *)eina_stringshare_add(buff));
        e_prefix_data_concat_static(buff, "data/icons");
        *list = eina_list_prepend(*list, (void *)eina_stringshare_add(buff));
     }
}

static Eina_Bool
_e_main_modules_load_after(void *d EINA_UNUSED, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   e_int_config_modules(NULL, NULL);
   E_FREE_FUNC(mod_init_end, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

static void
_e_main_modules_load(Eina_Bool safe_mode)
{
   if (!safe_mode)
     e_module_all_load();
   else
     {
        E_Module *m;
        char *crashmodule;

        crashmodule = getenv("E_MODULE_LOAD");
        if (crashmodule) m = e_module_new(crashmodule);

        if ((crashmodule) && (m))
          {
             e_module_disable(m);
             e_object_del(E_OBJECT(m));

             e_error_message_show
               (_("Enlightenment crashed early on start and has<ps/>"
                  "been restarted. There was an error loading the<ps/>"
                  "module named: %s. This module has been disabled<ps/>"
                  "and will not be loaded."), crashmodule);
             e_util_dialog_show
               (_("Enlightenment crashed early on start and has been restarted"),
               _("Enlightenment crashed early on start and has been restarted.<ps/>"
                 "There was an error loading the module named: %s<ps/><ps/>"
                 "This module has been disabled and will not be loaded."), crashmodule);
             e_module_all_load();
          }
        else
          {
             e_error_message_show
               (_("Enlightenment crashed early on start and has<ps/>"
                  "been restarted. All modules have been disabled<ps/>"
                  "and will not be loaded to help remove any problem<ps/>"
                  "modules from your configuration. The module<ps/>"
                  "configuration dialog should let you select your<ps/>"
                  "modules again.\n"));
             e_util_dialog_show
               (_("Enlightenment crashed early on start and has been restarted"),
               _("Enlightenment crashed early on start and has been restarted.<ps/>"
                 "All modules have been disabled and will not be loaded to help<ps/>"
                 "remove any problem modules from your configuration.<ps/><ps/>"
                 "The module configuration dialog should let you select your<ps/>"
                 "modules again."));
          }
        mod_init_end = ecore_event_handler_add(E_EVENT_MODULE_INIT_END, _e_main_modules_load_after, NULL);
     }
}

static Eina_Bool
_e_main_cb_idle_before(void *data EINA_UNUSED)
{
   e_menu_idler_before();
   e_client_idler_before();
   e_pointer_idler_before();
   edje_thaw();
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_main_cb_idle_after(void *data EINA_UNUSED)
{
   static int first_idle = 1;

   eet_clearcache();
   edje_freeze();

   if (first_idle)
     {
        TS("SLEEP");
        first_idle = 0;
     }
   e_precache_end = EINA_TRUE;

// every now and again trim malloc memory to stay lean-ish
#ifdef HAVE_MALLOC_TRIM
   static double t_last_clean = 0.0;
   double t = ecore_time_get();
   if ((t - t_last_clean) > 10.0)
     {
        t_last_clean = t;
        malloc_trim(0);
     }
#endif

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_main_cb_startup_fake_end(void *data EINA_UNUSED)
{
   e_init_done();
   return ECORE_CALLBACK_CANCEL;
}
