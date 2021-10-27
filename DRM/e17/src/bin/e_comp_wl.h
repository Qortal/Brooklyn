#ifdef E_TYPEDEFS
typedef struct _E_Comp_Wl_Aux_Hint  E_Comp_Wl_Aux_Hint;
typedef struct _E_Comp_Wl_Buffer E_Comp_Wl_Buffer;
typedef struct _E_Comp_Wl_Subsurf_Data E_Comp_Wl_Subsurf_Data;
typedef struct _E_Comp_Wl_Surface_State E_Comp_Wl_Surface_State;
typedef struct _E_Comp_Wl_Client_Data E_Comp_Wl_Client_Data;
typedef struct _E_Comp_Wl_Output E_Comp_Wl_Output;
typedef struct _E_Comp_Wl_Pointer E_Comp_Wl_Pointer;
typedef struct E_Shell_Data E_Shell_Data;
typedef struct Tizen_Extensions Tizen_Extensions;
typedef void (*E_Comp_Wl_Grab_End_Cb)(E_Client*);
#else
# ifndef E_COMP_WL_H
#  define E_COMP_WL_H

/* NB: Turn off shadow warnings for Wayland includes */
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wshadow"
#  define WL_HIDE_DEPRECATED
#  include <wayland-server.h>
#  pragma GCC diagnostic pop

#  include <xkbcommon/xkbcommon.h>

#  ifndef HAVE_WAYLAND_ONLY
#   include "e_comp_x.h"
#  endif

/* #  ifdef HAVE_WAYLAND_EGL */
/* #   include <EGL/egl.h> */
/* #   define GL_GLEXT_PROTOTYPES */
/* #  endif */

#  ifdef __linux__
#   include <linux/input.h>
#  else
#   define BTN_LEFT 0x110
#   define BTN_RIGHT 0x111
#   define BTN_MIDDLE 0x112
#   define BTN_SIDE 0x113
#   define BTN_EXTRA 0x114
#   define BTN_FORWARD 0x115
#   define BTN_BACK 0x116
#  endif

#  define container_of(ptr, type, member) \
   ({ \
      const __typeof__( ((type *)0)->member ) *__mptr = (ptr); \
      (type *)(void *)( (char *)__mptr - offsetof(type,member) ); \
   })

struct _E_Comp_Wl_Aux_Hint
{
   int           id;
   const char   *hint;
   const char   *val;
   int32_t       fd;
   Eina_Bool     changed;
   Eina_Bool     deleted;
};

struct _E_Comp_Wl_Buffer
{
   struct wl_resource *resource;
   struct wl_signal destroy_signal;
   struct wl_listener destroy_listener;
   struct wl_listener deferred_destroy_listener;
   struct wl_shm_buffer *shm_buffer;
   struct wl_shm_pool *pool;
   struct linux_dmabuf_buffer *dmabuf_buffer;
   E_Pixmap *discarding_pixmap;
   int32_t w, h;
   uint32_t busy;
   Eina_Bool destroyed;
};

struct _E_Comp_Wl_Surface_State
{
   int sx, sy;
   int bw, bh;
   E_Comp_Wl_Buffer *buffer;
   struct wl_listener buffer_destroy_listener;
   Eina_List *damages, *frames;
   Eina_Tiler *input, *opaque;
   Eina_Bool new_attach E_BITFIELD;
   Eina_Bool has_data E_BITFIELD;
};

struct _E_Comp_Wl_Subsurf_Data
{
   struct wl_resource *resource;

   E_Client *parent;

   struct
     {
        int x, y;
        Eina_Bool set;
     } position;

   E_Comp_Wl_Surface_State cached;

   Eina_Bool synchronized;
};

typedef struct E_Comp_Wl_Extension_Data
{
   struct
     {
        struct wl_global *global;
     } session_recovery;
   /* begin xdg-foreign */
   struct
     {
        struct wl_global *global;
        Eina_Hash *surfaces;
     } zxdg_exporter_v1;
   struct
     {
        struct wl_global *global;
     } zxdg_importer_v1;
   /* end xdg-foreign */
   struct
     {
        struct wl_global *global;
        Eina_List *resources;
     } zwp_relative_pointer_manager_v1;
   struct
     {
        struct wl_global *global;
        Eina_Hash *constraints;
     } zwp_pointer_constraints_v1;
   struct
   {
     struct wl_global *global;
   } action_route;
   struct
     {
        struct wl_global *global;
     } efl_aux_hints;
} E_Comp_Wl_Extension_Data;

struct _E_Comp_Wl_Data
{
   Ecore_Wl2_Display *ewd;

   struct
     {
        struct wl_display *disp;
        Ecore_Wl2_Display *client_disp;
        E_Client *client_ec;
        struct wl_registry *registry; // only used for nested wl compositors
        /* struct wl_event_loop *loop; */
        Eina_Inlist *globals;  // only used for nested wl compositors
        struct wl_shm *shm;  // only used for nested wl compositors
        Evas_GL *gl;
        Evas_GL_Config *glcfg;
        Evas_GL_Context *glctx;
        Evas_GL_Surface *glsfc;
        Evas_GL_API *glapi;
     } wl;

   struct
     {
        struct
          {
             struct wl_signal create;
             struct wl_signal activate;
             struct wl_signal kill;
          } surface;
        /* NB: At the moment, we don't need these */
        /*      struct wl_signal destroy; */
        /*      struct wl_signal activate; */
        /*      struct wl_signal transform; */
        /*      struct wl_signal kill; */
        /*      struct wl_signal idle; */
        /*      struct wl_signal wake; */
        /*      struct wl_signal session; */
        /*      struct  */
        /*        { */
        /*           struct wl_signal created; */
        /*           struct wl_signal destroyed; */
        /*           struct wl_signal moved; */
        /*        } seat, output; */
     } signals;

   struct
     {
        Eina_List *resources;
        Eina_List *focused;
        Eina_Bool enabled E_BITFIELD;
        xkb_mod_mask_t mod_depressed, mod_latched, mod_locked;
        xkb_layout_index_t mod_group;
        xkb_layout_index_t choosen_group;
        struct wl_array keys;
        struct wl_resource *focus;
        int mod_changed;
     } kbd;

   struct
     {
        Eina_List *resources;
        Evas_Coord x, y;
        uint32_t button;
        uint32_t serial[2]; //down/up
        uint32_t button_mask;
        E_Client *ec;
        Eina_Bool enabled E_BITFIELD;
     } ptr;

   struct
     {
        Eina_List *resources;
        Eina_Bool enabled E_BITFIELD;
     } touch;

   struct
     {
        struct wl_global *global;
        Eina_List *resources;
        uint32_t version;
        char *name;

        struct
          {
             struct wl_global *global;
             struct wl_resource *resource;
          } im;
     } seat;

   struct
     {
        struct wl_global *global;
        struct wl_resource *resource;
        Eina_Hash *data_resources;
     } mgr;

   struct
     {
        void *data_source;
        uint32_t serial;
        struct wl_signal signal;
        struct wl_listener data_source_listener;
        E_Client *target;
     } selection;

   struct
     {
        void *source;
        struct wl_listener listener;
        Ecore_Window xwl_owner;
     } clipboard;

   struct
     {
        struct wl_resource *resource;
        uint32_t edges;
     } resize;

   struct
     {
        struct xkb_keymap *keymap;
        struct xkb_context *context;
        struct xkb_state *state;
        char *map_string;
        int map_size;
     } xkb;

   E_Comp_Wl_Extension_Data *extensions;

   Eina_List *outputs;

   Ecore_Fd_Handler *fd_hdlr;
   Ecore_Idler *idler;

   struct wl_client *xwl_client;
   Eina_List *xwl_pending;

   E_Drag *drag;
   E_Client *drag_client;
   void *drag_source;

   Eina_List *efl_wls;

   Eina_Bool dmabuf_disable E_BITFIELD;
   Eina_Bool dmabuf_proxy E_BITFIELD;
};

struct _E_Comp_Wl_Client_Data
{
   Ecore_Timer *on_focus_timer;

   struct
     {
        E_Comp_Wl_Subsurf_Data *data;
        E_Client *restack_target;
        Eina_List *list;
     } sub;

   /* regular surface resource (wl_compositor_create_surface) */
   struct wl_resource *surface;
   struct wl_signal destroy_signal;

   struct
     {
        /* shell surface resource */
        struct wl_resource *surface;

        void (*configure_send)(struct wl_resource *resource, uint32_t edges, int32_t width, int32_t height);
        void (*configure)(struct wl_resource *resource, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
        void (*ping)(struct wl_resource *resource);
        void (*map)(struct wl_resource *resource);
        void (*unmap)(struct wl_resource *resource);
        void (*buffer_attach_error)(E_Client *ec);
        Eina_Rectangle window;
        E_Shell_Data *data;
        struct
        {
           Evas_Coord_Size min_size;
           Evas_Coord_Size max_size;
           Eina_Bool fullscreen E_BITFIELD;
           Eina_Bool unfullscreen E_BITFIELD;
           Eina_Bool maximize E_BITFIELD;
           Eina_Bool unmaximize E_BITFIELD;
           Eina_Bool minimize E_BITFIELD;
           int fs_zone;
        } set;
     } shell;

   E_Comp_Wl_Surface_State pending;

   Eina_List *frames;
   Eina_List *constraints;

   struct
     {
        int32_t x, y;
     } popup;

   Tizen_Extensions *tizen;
   struct
     {
        Eina_Bool  changed E_BITFIELD;
        Eina_List *hints;
        Eina_Bool  use_msg E_BITFIELD;
     } aux_hint;

   int32_t on_outputs; /* Bitfield of the outputs this client is present on */

   E_Maximize max;
   E_Maximize unmax;
#ifndef HAVE_WAYLAND_ONLY
   E_Pixmap *xwayland_pixmap;
   E_Comp_X_Client_Data *xwayland_data;
#endif

   int early_frame;

   Eina_Bool mapped E_BITFIELD;
   Eina_Bool evas_init E_BITFIELD;
   Eina_Bool cursor E_BITFIELD;
   Eina_Bool moved E_BITFIELD;
   Eina_Bool maximizing E_BITFIELD;
   Eina_Bool in_commit E_BITFIELD;
   Eina_Bool is_xdg_surface E_BITFIELD;
   Eina_Bool grab E_BITFIELD;
   Eina_Bool buffer_commit E_BITFIELD;
   Eina_Bool need_xdg_configure E_BITFIELD;
   Eina_Bool maximize_anims_disabled E_BITFIELD;
   Eina_Bool ssd_mouse_in E_BITFIELD;
   Eina_Bool need_center E_BITFIELD;
};

struct _E_Comp_Wl_Output
{
   struct wl_global *global;
   Eina_List *resources;
   const char *id, *make, *model;
   int x, y, w, h;
   int phys_width, phys_height;
   unsigned int refresh;
   unsigned int subpixel;
   unsigned int transform;
   double scale;

   /* added for screenshot ability */
   struct wl_output *wl_output;
   struct wl_buffer *buffer;
   void *data;
};

struct _E_Comp_Wl_Pointer
{
   E_Client *cursor;
   Evas_Point offset;
   Eina_Bool entered E_BITFIELD;
   Eina_Bool cursor_set E_BITFIELD;
};

E_API Eina_Bool e_comp_wl_init(void);
EINTERN void e_comp_wl_shutdown(void);

EINTERN struct wl_resource *e_comp_wl_surface_create(struct wl_client *client, int version, uint32_t id);
EINTERN void e_comp_wl_surface_destroy(struct wl_resource *resource);
EINTERN Eina_Bool e_comp_wl_surface_commit(E_Client *ec);
EINTERN Eina_Bool e_comp_wl_subsurface_commit(E_Client *ec);
E_API E_Comp_Wl_Buffer *e_comp_wl_buffer_get(struct wl_resource *resource);

E_API struct wl_signal e_comp_wl_surface_create_signal_get(void);
E_API double e_comp_wl_idle_time_get(void);
E_API Eina_Bool e_comp_wl_output_init(const char *id, const char *make, const char *model, int x, int y, int w, int h, int pw, int ph, unsigned int refresh, unsigned int subpixel, unsigned int transform, unsigned int num);
E_API void e_comp_wl_output_remove(const char *id);

EINTERN Eina_Bool e_comp_wl_key_down(Ecore_Event_Key *ev, E_Client *ec);
EINTERN Eina_Bool e_comp_wl_key_up(Ecore_Event_Key *ev, E_Client *ec);
E_API Eina_Bool e_comp_wl_evas_handle_mouse_button(E_Client *ec, uint32_t timestamp, uint32_t button_id, uint32_t state);

E_API extern int E_EVENT_WAYLAND_GLOBAL_ADD;

E_API void e_comp_wl_grab_client_add(E_Client *ec, E_Comp_Wl_Grab_End_Cb cb);
E_API void e_comp_wl_grab_client_del(E_Client *ec, Eina_Bool dismiss);
E_API Eina_Bool e_comp_wl_client_is_grabbed(const E_Client *ec);
E_API Eina_Bool e_comp_wl_grab_client_mouse_move(const Ecore_Event_Mouse_Move *ev);
E_API Eina_Bool e_comp_wl_grab_client_mouse_button(const Ecore_Event_Mouse_Button *ev);

E_API void e_comp_wl_extension_relative_motion_event(uint64_t time_usec, double dx, double dy, double dx_unaccel, double dy_unaccel);
E_API void e_comp_wl_extension_pointer_constraints_commit(E_Client *ec);
E_API Eina_Bool e_comp_wl_extension_pointer_constraints_update(E_Client *ec, int x, int y);
E_API void e_comp_wl_extension_pointer_unconstrain(E_Client *ec);
E_API void e_comp_wl_extension_action_route_pid_allowed_set(uint32_t pid, Eina_Bool allow);
E_API const void *e_comp_wl_extension_action_route_interface_get(int *version);

E_API void e_comp_wl_notidle(void);
E_API void e_comp_wl_screensaver_activate(void);
E_API void e_comp_wl_screensaver_inhibit(Eina_Bool inhibit);

EINTERN Eina_Bool _e_comp_wl_screensaver_on();
EINTERN Eina_Bool _e_comp_wl_screensaver_off();

E_API void
e_policy_wl_aux_message_send(E_Client *ec,
                             const char *key,
                             const char *val,
                             Eina_List *options);

E_API void e_comp_wl_input_keymap_send(struct wl_resource *res);

# ifndef HAVE_WAYLAND_ONLY
EINTERN void e_comp_wl_xwayland_client_queue(E_Client *ec);
static inline E_Comp_X_Client_Data *
e_comp_wl_client_xwayland_data(const E_Client *ec)
{
   return ec->comp_data ? ((E_Comp_Wl_Client_Data*)ec->comp_data)->xwayland_data : NULL;
}

static inline E_Pixmap *
e_comp_wl_client_xwayland_pixmap(const E_Client *ec)
{
   return ec->comp_data ?  ((E_Comp_Wl_Client_Data*)ec->comp_data)->xwayland_pixmap : NULL;
}

static inline void
e_comp_wl_client_xwayland_setup(E_Client *ec, E_Comp_X_Client_Data *cd, E_Pixmap *ep)
{
   if (cd && ep)
     {
        ((E_Comp_Wl_Client_Data*)ec->comp_data)->xwayland_data = cd;
        ((E_Comp_Wl_Client_Data*)ec->comp_data)->xwayland_pixmap = ep;
     }
   if (e_comp_wl->xwl_pending)
     e_comp_wl->xwl_pending = eina_list_remove(e_comp_wl->xwl_pending, ec);
}
#  endif
# endif
#endif
