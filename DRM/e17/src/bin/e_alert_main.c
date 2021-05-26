#include "config.h"

#include <Elementary.h>
#ifndef HAVE_WAYLAND_ONLY
# include <Ecore_X.h>
#endif

static int         ret = 0, sig = 0, exit_gdb = 0;
static pid_t       pid;
static Eina_Bool   tainted = EINA_FALSE;
static const char *backtrace_str = NULL;

//#define FONT           "TopazPlus_a500_v1.0.pcf"
//#define FONT           "terminus-16.pcf"
#define FONT           "Topaz_a500_v1.0.ttf"
#define FONT_FALLBACK  "Mono"
#define COL_BG         0,   0,   0, 255
#define COL_FG         255,   0,   0, 255
#define HDIV           10
#define PDIV           20
#define BLINK          0.8
#define RECOVER_BUTTON 1
#define RECOVER_KEY    "F1"
#define ABORT_BUTTON   3
#define ABORT_KEY      "F12"

static inline const char *title1(void) {
   return "Software Failure. Press left mouse button / F1 to recover, right mouse button / F12 to abort.";
}
static inline const char *title2(void) { static char buf[512];
   if (tainted) snprintf
     (buf, sizeof(buf), "Tainted by unsupported modules");
   else if (exit_gdb) snprintf
     (buf, sizeof(buf), "Couldn't run gdb to collect a backtrace");
   else if (backtrace_str) snprintf
     (buf, sizeof(buf), "Backtrace log: %s", backtrace_str);
   else snprintf
     (buf, sizeof(buf), " ");
   return buf;
}
static inline const char *title3(void) { static char buf[512];
   snprintf(buf, sizeof(buf), "Guru Meditation #%08d.%08d", pid, sig);
   return buf;
}

/////////////////////////////////////////////////////////////////////////////

static Evas_Object *obj_base = NULL;
static Evas_Object *obj_outer = NULL;
static Evas_Object *obj_inner = NULL;
static Evas_Object *obj_line1 = NULL;
static Evas_Object *obj_line2 = NULL;
static Evas_Object *obj_line3 = NULL;

static void
mouse_up(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *info)
{
   Evas_Event_Mouse_Up *ev = info;

   if (ev->button == RECOVER_BUTTON)
     {
        ret = 111;
        elm_exit();
     }
   else if (ev->button == ABORT_BUTTON)
     {
        ret = 101;
        elm_exit();
     }
}

static void
key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *info)
{
   Evas_Event_Key_Down *ev = info;

   if (!strcmp(ev->key, RECOVER_KEY))
     {
        ret = 111;
        elm_exit();
     }
   else if (!strcmp(ev->key, ABORT_KEY))
     {
        ret = 101;
        elm_exit();
     }
}

static const char *
font_get(void)
{
   const char *s = getenv("E_ALERT_FONT_DIR");
   static char buf[4096];

   if (s) snprintf(buf, sizeof(buf), "%s/"FONT, s);
   else snprintf(buf, sizeof(buf), "%s", FONT_FALLBACK);
   return buf;
}

static void
resize(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *o, void *info EINA_UNUSED)
{
   Evas_Coord w, h, tw, th;
   const char *font = font_get();

   evas_object_geometry_get(o, NULL, NULL, &w, &h);
   h = w / HDIV;

   evas_object_geometry_set(obj_base, 0, 0, w, h * HDIV * 2);
   evas_object_geometry_set(obj_outer, (1 * h) / PDIV, (1 * h) / PDIV,
                            w - ((h * 2) / PDIV), h - ((h * 2) / PDIV));
   evas_object_geometry_set(obj_inner, (2 * h) / PDIV, (2 * h) / PDIV,
                            w - ((h * 4) / PDIV), h - ((h * 4) / PDIV));

   evas_object_text_font_set(obj_line1, font, h / 8);
   evas_object_geometry_get(obj_line1, NULL, NULL, &tw, &th);
   evas_object_move(obj_line1, (w - tw) / 2, (3 * h) / PDIV);

   evas_object_text_font_set(obj_line2, font, h / 8);
   evas_object_geometry_get(obj_line2, NULL, NULL, &tw, &th);
   evas_object_move(obj_line2, (w - tw) / 2, (h - th) / 2);

   evas_object_text_font_set(obj_line3, font, h / 8);
   evas_object_geometry_get(obj_line3, NULL, NULL, &tw, &th);
   evas_object_move(obj_line3, (w - tw) / 2, h - th - (3 * h) / PDIV);
}

static Eina_Bool
timer(void *data EINA_UNUSED)
{
   if (evas_object_visible_get(obj_outer)) evas_object_hide(obj_outer);
   else evas_object_show(obj_outer);
   return EINA_TRUE;
}

static Eina_Bool
setup_display(void)
{
   Evas *e;
   Evas_Object *win, *o;
   const char *font = font_get();

   win = o = elm_win_add(NULL, "e-alert", ELM_WIN_SPLASH);
   if (!win) return EINA_FALSE;
   e = evas_object_evas_get(win);
   elm_win_override_set(o, EINA_TRUE);

   obj_base = o = evas_object_rectangle_add(e);
   evas_object_color_set(o, COL_BG);
   evas_object_show(o);

   obj_outer = o = evas_object_rectangle_add(e);
   evas_object_color_set(o, COL_FG);
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_show(o);

   obj_inner = o = evas_object_rectangle_add(e);
   evas_object_color_set(o, COL_BG);
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_show(o);

   obj_line1 = o = evas_object_text_add(e);
   evas_object_color_set(o, COL_FG);
   evas_object_text_font_set(o, font, 10);
   evas_object_text_text_set(o, title1());
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_show(o);

   obj_line2 = o = evas_object_text_add(e);
   evas_object_color_set(o, COL_FG);
   evas_object_text_font_set(o, font, 10);
   evas_object_text_text_set(o, title2());
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_show(o);

   obj_line3 = o = evas_object_text_add(e);
   evas_object_color_set(o, COL_FG);
   evas_object_text_font_set(o, font, 10);
   evas_object_text_text_set(o, title3());
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_show(o);

   evas_object_event_callback_add(obj_base, EVAS_CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_DOWN, key_down, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, resize, NULL);

   ecore_timer_add(BLINK, timer, NULL);

   resize(NULL, e, win, NULL);

#ifndef HAVE_WAYLAND_ONLY
   if (getenv("DISPLAY"))
     {
        Ecore_X_Window root = ecore_x_window_root_first_get();
        Ecore_X_Atom atom_composite_win = ecore_x_atom_get("_E_COMP_WINDOW");
        unsigned int id;
        if (ecore_x_window_prop_card32_get(root, atom_composite_win, &id, 1) > 0)
          {
             Ecore_X_Window elmwin = elm_win_xwindow_get(win);
             ecore_x_window_reparent(elmwin, id, 0, 0);
             Ecore_X_Randr_Output output = ecore_x_randr_primary_output_get(root);
             Ecore_X_Randr_Crtc crct = ecore_x_randr_output_crtc_get(root, output);
             int x = 0, y = 0, w = 1, h = 1;
             ecore_x_randr_crtc_geometry_get(root, crct, &x, &y, &w, &h);
             ecore_x_window_move_resize(elmwin, x, y, w, w / HDIV);
          }
     }
#endif

   evas_object_show(win);

#ifndef HAVE_WAYLAND_ONLY
   if (getenv("DISPLAY"))
     {
        Ecore_X_Window elmwin = elm_win_xwindow_get(win);
        ecore_x_pointer_grab(elmwin);
        ecore_x_keyboard_grab(elmwin);
     }
#endif
   return EINA_TRUE;
}

int
main(int argc, char **argv)
{
   const char *s;
   int i = 0;

   for (i = 1; i < argc; i++)
     {
        if ((!strcmp(argv[i], "-h")) ||
            (!strcmp(argv[i], "-help")) ||
            (!strcmp(argv[i], "--help")))
          {
             printf("This is an internal tool for Enlightenment.\n"
                    "do not use it.\n");
             exit(0);
          }
        else if (i == 1) sig           = atoi(argv[i]);  // signal
        else if (i == 2) pid           = atoi(argv[i]);  // E's pid
        else if (i == 3) exit_gdb      = atoi(argv[i]);
        else if (i == 4) backtrace_str = argv[i];
     }
   fprintf(stderr, "exit_gdb: %i\n", exit_gdb);

   s = getenv("E_TAINTED");
   if      (s && !strcmp(s, "NO")) tainted = EINA_FALSE;
   else if (s && !strcmp(s, "YES")) tainted = EINA_TRUE;

   ecore_app_no_system_modules();
   elm_init(argc, argv);

   if (setup_display())
     {
        s = getenv("E_ALERT_SYSTEM_BIN");
        if (s && s[0])
          {
             putenv("E_ALERT_BACKLIGHT_RESET=1");
             ecore_exe_pipe_run
               (s, ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE |
                ECORE_EXE_NOT_LEADER | ECORE_EXE_TERM_WITH_PARENT, NULL);
          }

        elm_run();
     }

   return ret;
}
