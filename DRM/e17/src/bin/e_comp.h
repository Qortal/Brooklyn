#ifdef E_TYPEDEFS
typedef struct _E_Comp E_Comp;

#ifdef E_COMP_WL
typedef struct _E_Comp_Wl_Client_Data E_Comp_Client_Data;
#endif
typedef struct _E_Comp_Wl_Data E_Comp_Wl_Data;

#ifdef E_COMP_X
typedef struct _E_Comp_X_Client_Data E_Comp_Client_Data;
#endif
typedef struct _E_Comp_X_Data E_Comp_X_Data;

#if !defined(E_COMP_WL) && !defined(E_COMP_X)
typedef struct _E_Comp_Client_Data E_Comp_Client_Data;
#endif

typedef struct _E_Comp_Demo_Style_Item E_Comp_Demo_Style_Item;

# define E_COMP_TYPE (int) 0xE0b01003

# define E_LAYER_COUNT 19
# define E_CLIENT_LAYER_COUNT 11

typedef enum _E_Layer
{
   E_LAYER_BOTTOM = -100,
   E_LAYER_BG = -1, // zone bg stuff
   E_LAYER_DESKTOP = 0, // desktop objects: fileman, gadgets, shelves
   E_LAYER_DESKTOP_TOP = 10, // raised desktop objects: gadgets
   E_LAYER_CLIENT_DESKTOP = 100, //shelves
   E_LAYER_CLIENT_BELOW = 150,
   E_LAYER_CLIENT_NORMAL = 200,
   E_LAYER_CLIENT_ABOVE = 250,
   E_LAYER_CLIENT_EDGE = 300,
   E_LAYER_CLIENT_FULLSCREEN = 350,
   E_LAYER_CLIENT_EDGE_FULLSCREEN = 400,
   E_LAYER_CLIENT_TOP = 450,
   E_LAYER_CLIENT_POPUP = 500,
   E_LAYER_CLIENT_DRAG = 550,
   E_LAYER_CLIENT_PRIO = 600,
   E_LAYER_POPUP = 999, // popups
   E_LAYER_MENU = 5000, // menus
   E_LAYER_DESKLOCK = 9999, // desklock
   E_LAYER_MAX = 32767 // EVAS_LAYER_MAX
} E_Layer;

#else
# ifndef E_COMP_H
#  define E_COMP_H

# include "e_comp_cfdata.h"

extern E_API int E_EVENT_COMPOSITOR_DISABLE;
extern E_API int E_EVENT_COMPOSITOR_ENABLE;
extern E_API int E_EVENT_COMPOSITOR_XWAYLAND_INIT;

typedef void (*E_Comp_Cb)(void);

typedef struct E_Comp_Screen_Iface
{
   /* can screen changes be made at all */
   Eina_Bool (*available)(void);
   /* begin listening for screen events */
   void (*init)(void);
   /* stop listening for screen events */
   void (*shutdown)(void);
   /* gather screen info */
   E_Randr2 *(*create)(void);
   /* apply current config */
   void (*apply)(void);
   /* set dpms (on, standby, suspend, off) */
   void (*dpms)(int);
   /* is key event eaten */
   Eina_Bool (*key_down)(Ecore_Event_Key *ev);
   /* is key event eaten */
   Eina_Bool (*key_up)(Ecore_Event_Key *ev);
   Eina_Bool relative_motion E_BITFIELD;
   Eina_Bool backlight_enabled E_BITFIELD;
} E_Comp_Screen_Iface;

/* struct to hold canvas objects so that abi doesn't break
 * when new objects are added
 */
typedef struct E_Comp_Canvas
{
   Evas_Object *resize_object; //object to monitor for comp canvas resizes
   Evas_Object *fps_bg;
   Evas_Object *fps_fg;
   Evas_Object *fps_gr;
   Evas_Object *gadget_site; //desktop gadget site
} E_Comp_Canvas;

#define E_COMP_FRAME_EVENT_COUNT 4096

#define E_COMP_FRAME_EVENT_RENDER_BEGIN   1
#define E_COMP_FRAME_EVENT_RENDER_END     2
#define E_COMP_FRAME_EVENT_RENDER2_BEGIN  3
#define E_COMP_FRAME_EVENT_RENDER2_END    4
#define E_COMP_FRAME_EVENT_CLIENT_DAMAGE  5
#define E_COMP_FRAME_EVENT_HANDLE_DAMAGE  6
#define E_COMP_FRAME_EVENT_IDLE_EXIT      7
#define E_COMP_FRAME_EVENT_IDLE_ENTER     8

typedef struct E_Comp_Frame_Event
{
   int info[4];
   double t;
} E_Comp_Event;

struct _E_Comp
{
   E_Object e_obj_inherit;
   int w, h; //overall size of compositor

   Ecore_Window  win; // input overlay
   Ecore_Window  root; //x11 root window
   Ecore_Evas     *ee; //canvas
   Ecore_Window  ee_win; //canvas window
   E_Comp_Canvas *canvas;
   Evas_Object    *elm; //elm win base
   Evas           *evas; //canvas
   Eina_List      *zones; //list of E_Zones
   E_Pointer      *pointer;
   Eina_List *clients; //list of all E_Clients
   unsigned int new_clients; //number of clients with new_client set

   Eina_List *pre_render_cbs; /* E_Comp_Cb */

   E_Comp_X_Data *x_comp_data; //x11 compositor-specific data
   E_Comp_Wl_Data *wl_comp_data; //wl compositor-specific data

   E_Pixmap_Type comp_type; //for determining X/Wayland primary type

   struct {
      Ecore_Window win; //x11 layer stacking window
      Evas_Object *obj; //layer stacking object
      //Eina_Inlist *objs; /* E_Comp_Object; NOT to be exposed; seems pointless? */
      Eina_Inlist *clients; /* E_Client, bottom to top */
      unsigned int clients_count; //count of clients on layer
   } layers[E_LAYER_COUNT];

   struct //autoclose handler for e_comp_object_util_autoclose
   {
      Evas_Object *rect; //autoclose blocker rect
      Evas_Object *obj; //autoclose object
      Ecore_Event_Handler *key_handler;
      E_Comp_Object_Autoclose_Cb del_cb; //cb to call on autoclose delete
      E_Comp_Object_Key_Cb key_cb; //cb to call on key press
      void *data; //user data
   } autoclose;

   E_Comp_Screen_Iface *screen;

   Eina_List *debug_rects; //used when SHAPE_DEBUG is defined in e_comp.c
   Eina_List *ignore_wins; //windows to be ignored by the compositor

   Eina_List      *updates; //E_Clients with render updates
   Eina_List      *post_updates; //E_Clients awaiting post render flushing
   Ecore_Animator *render_animator; //animator for fixed time rendering
   Ecore_Job      *shape_job; //job to update x11 input shapes
   Ecore_Job      *update_job; //job to trigger render updates
   Ecore_Timer    *nocomp_delay_timer; //delay before activating nocomp in x11
   Ecore_Timer    *nocomp_override_timer; //delay before overriding nocomp in x11
   int             animating; //number of animating comp objects
   E_Comp_Event    frame_events[E_COMP_FRAME_EVENT_COUNT]; // history of frame events for debug
   int             frame_event_now;
   int             frameskip;

   int             nocomp_override; //number of times nocomp override has been requested
   Ecore_Window block_win;
   int             block_count; //number of times block window has been requested

   Ecore_Window  cm_selection; //FIXME: move to comp_x ?
   E_Client       *nocomp_ec; //window that triggered nocomp mode

   unsigned int    input_key_grabs; //number of active compositor key grabs
   unsigned int    input_mouse_grabs; //number of active compositor mouse grabs

   E_Comp_Cb        grab_cb; //callback for grabbing the xserver
   E_Comp_Cb        bindings_grab_cb; //callback for triggering binding input grab
   E_Comp_Cb        bindings_ungrab_cb; //callback for triggering binding input ungrab

   Eina_Bool       gl E_BITFIELD; //gl is active
   Eina_Bool       grabbed E_BITFIELD; //xserver is grabbed
   Eina_Bool       nocomp E_BITFIELD; //nocomp is active
   Eina_Bool       nocomp_want E_BITFIELD; //nocomp is pending
   Eina_Bool       saver E_BITFIELD; //screensaver is active
   Eina_Bool       shape_queue_blocked E_BITFIELD; //x11 input shape updates are blocked

   Eina_Bool       rendering E_BITFIELD; // we've received a pre-render callback but no post-render yet.
   Eina_Bool       updating E_BITFIELD; // running client updates

   Eina_Bool       frozen E_BITFIELD; // frozen - when power save freeze mode is on
};


struct _E_Comp_Demo_Style_Item
{
   Evas_Object *preview;
   Evas_Object *frame;
   Evas_Object *livethumb;
   Evas_Object *layout;
   Evas_Object *border;
   Evas_Object *client;
};

typedef enum
{
   E_COMP_ENGINE_NONE = 0,
   E_COMP_ENGINE_SW = 1,
   E_COMP_ENGINE_GL = 2
} E_Comp_Engine;

extern E_API E_Comp *e_comp;
extern E_API E_Comp_X_Data *e_comp_x;
extern E_API E_Comp_Wl_Data *e_comp_wl;

EINTERN Eina_Bool e_comp_init(void);
E_API E_Comp *e_comp_new(void);
E_API int e_comp_internal_save(void);
EINTERN int e_comp_shutdown(void);
E_API void e_comp_render_queue(void);
E_API void e_comp_client_post_update_add(E_Client *ec);
E_API void e_comp_shape_queue(void);
E_API void e_comp_shape_queue_block(Eina_Bool block);
E_API E_Comp_Config *e_comp_config_get(void);
E_API const Eina_List *e_comp_list(void);
E_API void e_comp_shadows_reset(void);
E_API Ecore_Window e_comp_top_window_at_xy_get(Evas_Coord x, Evas_Coord y);
E_API void e_comp_util_wins_print(void);
E_API void e_comp_ignore_win_add(E_Pixmap_Type type, Ecore_Window win);
E_API void e_comp_ignore_win_del(E_Pixmap_Type type, Ecore_Window win);
E_API Eina_Bool e_comp_ignore_win_find(Ecore_Window win);
E_API void e_comp_override_del(void);
E_API void e_comp_override_add(void);
E_API void e_comp_block_window_add(void);
E_API void e_comp_block_window_del(void);
E_API E_Comp *e_comp_find_by_window(Ecore_Window win);
E_API void e_comp_override_timed_pop(void);
E_API unsigned int e_comp_e_object_layer_get(const E_Object *obj);
E_API Eina_Bool e_comp_grab_input(Eina_Bool mouse, Eina_Bool kbd);
E_API void e_comp_ungrab_input(Eina_Bool mouse, Eina_Bool kbd);
E_API void e_comp_gl_set(Eina_Bool set);
E_API Eina_Bool e_comp_gl_get(void);
E_API void e_comp_frame_event_add(int info[4], double t);
E_API void e_comp_client_frame_add(Evas_Object *obj);

E_API void e_comp_button_bindings_grab_all(void);
E_API void e_comp_button_bindings_ungrab_all(void);
E_API void e_comp_client_redirect_toggle(E_Client *ec);
E_API Eina_Bool e_comp_util_object_is_above_nocomp(Evas_Object *obj);

EINTERN Evas_Object *e_comp_style_selector_create(Evas *evas, const char **source);
E_API E_Config_Dialog *e_int_config_comp(Evas_Object *parent, const char *params);
E_API E_Config_Dialog *e_int_config_comp_match(Evas_Object *parent, const char *params);

E_API Eina_Bool e_comp_util_kbd_grabbed(void);
E_API Eina_Bool e_comp_util_mouse_grabbed(void);

E_API void e_comp_clients_rescale(void);

static inline Eina_Bool
e_comp_util_client_is_fullscreen(const E_Client *ec)
{
   if ((!ec->visible) || (ec->input_only))
     return EINA_FALSE;
   return ((ec->client.x == 0) && (ec->client.y == 0) &&
       ((ec->client.w) >= e_comp->w) &&
       ((ec->client.h) >= e_comp->h) &&
       (!ec->argb) && (!ec->shaped)
       );
}

static inline Eina_Bool
e_comp_util_has_x(void)
{
   return !!e_comp->root;
}

static inline Eina_Bool
e_comp_util_has_xwayland(void)
{
   return (e_comp->comp_type != E_PIXMAP_TYPE_X) && e_comp_util_has_x();
}

#endif
#endif
