#ifdef E_TYPEDEFS
typedef enum _E_Client_Screen_Limit
{
    E_CLIENT_OFFSCREEN_LIMIT_ALLOW_PARTIAL = 0,
    E_CLIENT_OFFSCREEN_LIMIT_ALLOW_FULL = 1,
    E_CLIENT_OFFSCREEN_LIMIT_ALLOW_NONE = 2
} E_Client_Screen_Limit;

typedef enum _E_Icon_Preference
{
   E_ICON_PREF_E_DEFAULT,
   E_ICON_PREF_NETWM,
   E_ICON_PREF_USER
} E_Icon_Preference;

typedef enum _E_Direction
{
   E_DIRECTION_UP,
   E_DIRECTION_DOWN,
   E_DIRECTION_LEFT,
   E_DIRECTION_RIGHT
} E_Direction;

typedef enum _E_Transition
{
   E_TRANSITION_LINEAR = 0,
   E_TRANSITION_SINUSOIDAL = 1,
   E_TRANSITION_ACCELERATE = 2,
   E_TRANSITION_DECELERATE = 3,
   E_TRANSITION_ACCELERATE_LOTS = 4,
   E_TRANSITION_DECELERATE_LOTS = 5,
   E_TRANSITION_SINUSOIDAL_LOTS = 6,
   E_TRANSITION_BOUNCE = 7,
   E_TRANSITION_BOUNCE_LOTS = 8
} E_Transition;

typedef enum _E_Stacking
{
   E_STACKING_NONE,
   E_STACKING_ABOVE,
   E_STACKING_BELOW
} E_Stacking;

typedef enum _E_Focus_Policy
{
   E_FOCUS_CLICK,
   E_FOCUS_MOUSE,
   E_FOCUS_SLOPPY,
   E_FOCUS_LAST,
} E_Focus_Policy;

typedef enum
{
   /* same as ecore-x types */
   E_WINDOW_TYPE_UNKNOWN = 0,
   E_WINDOW_TYPE_DESKTOP,
   E_WINDOW_TYPE_DOCK,
   E_WINDOW_TYPE_TOOLBAR,
   E_WINDOW_TYPE_MENU,
   E_WINDOW_TYPE_UTILITY,
   E_WINDOW_TYPE_SPLASH,
   E_WINDOW_TYPE_DIALOG,
   E_WINDOW_TYPE_NORMAL,
   E_WINDOW_TYPE_DROPDOWN_MENU,
   E_WINDOW_TYPE_POPUP_MENU,
   E_WINDOW_TYPE_TOOLTIP,
   E_WINDOW_TYPE_NOTIFICATION,
   E_WINDOW_TYPE_COMBO,
   E_WINDOW_TYPE_DND,
   /* since UNKNOWN was used improperly in comp matching,
    * this value is a placeholder to indicate that we want
    * only type UNKNOWN windows
    */
   E_WINDOW_TYPE_REAL_UNKNOWN = 999
} E_Window_Type;

typedef enum _E_Urgency_Policy
{
   E_ACTIVEHINT_POLICY_IGNORE,
   E_ACTIVEHINT_POLICY_ANIMATE,
   E_ACTIVEHINT_POLICY_ACTIVATE,
   E_ACTIVEHINT_POLICY_ACTIVATE_EXCLUDE,
   E_ACTIVEHINT_POLICY_LAST,
} E_Urgency_Policy;

typedef enum _E_Focus_Setting
{
   E_FOCUS_NONE,
   E_FOCUS_NEW_WINDOW,
   E_FOCUS_NEW_DIALOG,
   E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED
} E_Focus_Setting;

typedef enum _E_Maximize
{
   E_MAXIMIZE_NONE = 0x00000000,
   E_MAXIMIZE_FULLSCREEN = 0x00000001,
   E_MAXIMIZE_SMART = 0x00000002,
   E_MAXIMIZE_EXPAND = 0x00000003,
   E_MAXIMIZE_FILL = 0x00000004,
   E_MAXIMIZE_TYPE = 0x0000000f,
   E_MAXIMIZE_VERTICAL = 0x00000010,
   E_MAXIMIZE_HORIZONTAL = 0x00000020,
   E_MAXIMIZE_BOTH = 0x00000030,
   E_MAXIMIZE_LEFT = 0x00000070,
   E_MAXIMIZE_RIGHT = 0x000000b0,
   E_MAXIMIZE_DIRECTION = 0x000000f0
} E_Maximize;

typedef enum _E_Fullscreen
{
   /* Resize window */
   E_FULLSCREEN_RESIZE,
   /* Change screen resolution and resize window */
   E_FULLSCREEN_ZOOM
} E_Fullscreen;

typedef enum _E_Window_Placement
{
   E_WINDOW_PLACEMENT_SMART,
   E_WINDOW_PLACEMENT_ANTIGADGET,
   E_WINDOW_PLACEMENT_CURSOR,
   E_WINDOW_PLACEMENT_MANUAL
} E_Window_Placement;

typedef enum E_Client_Property
{
   E_CLIENT_PROPERTY_NONE = 0,
   E_CLIENT_PROPERTY_SIZE = (1 << 0),
   E_CLIENT_PROPERTY_POS = (1 << 1),
   E_CLIENT_PROPERTY_TITLE = (1 << 2),
   E_CLIENT_PROPERTY_ICON = (1 << 3),
   E_CLIENT_PROPERTY_URGENCY = (1 << 4),
   E_CLIENT_PROPERTY_GRAVITY = (1 << 5),
   E_CLIENT_PROPERTY_NETWM_STATE = (1 << 6),
   E_CLIENT_PROPERTY_STICKY = (1 << 7),
   E_CLIENT_PROPERTY_MISC = (1 << 31),
} E_Client_Property;

typedef struct E_Client E_Client;

typedef struct E_Event_Client E_Event_Client;
typedef struct _E_Event_Client_Property E_Event_Client_Property;
typedef struct _E_Client_Pending_Resize E_Client_Pending_Resize;
typedef struct E_Event_Client_Zone_Set E_Event_Client_Zone_Set;
typedef struct E_Event_Client_Desk_Set E_Event_Client_Desk_Set;
typedef struct _E_Client_Hook E_Client_Hook;

typedef enum _E_Client_Hook_Point
{
   E_CLIENT_HOOK_EVAL_PRE_FETCH,
   E_CLIENT_HOOK_EVAL_FETCH,
   E_CLIENT_HOOK_EVAL_PRE_POST_FETCH,
   E_CLIENT_HOOK_EVAL_POST_FETCH,
   E_CLIENT_HOOK_EVAL_PRE_FRAME_ASSIGN,
   E_CLIENT_HOOK_EVAL_POST_FRAME_ASSIGN,
   E_CLIENT_HOOK_EVAL_PRE_NEW_CLIENT,
   E_CLIENT_HOOK_EVAL_POST_NEW_CLIENT,
   E_CLIENT_HOOK_EVAL_END,
   E_CLIENT_HOOK_FOCUS_SET,
   E_CLIENT_HOOK_FOCUS_UNSET,
   E_CLIENT_HOOK_NEW_CLIENT,
   E_CLIENT_HOOK_DESK_SET,
   E_CLIENT_HOOK_MOVE_BEGIN,
   E_CLIENT_HOOK_MOVE_UPDATE,
   E_CLIENT_HOOK_MOVE_END,
   E_CLIENT_HOOK_RESIZE_BEGIN,
   E_CLIENT_HOOK_RESIZE_UPDATE,
   E_CLIENT_HOOK_RESIZE_END,
   E_CLIENT_HOOK_DEL,
   E_CLIENT_HOOK_UNREDIRECT,
   E_CLIENT_HOOK_REDIRECT,
   E_CLIENT_HOOK_UNIGNORE,
   E_CLIENT_HOOK_LAST,
} E_Client_Hook_Point;

typedef void (*E_Client_Move_Intercept_Cb)(E_Client *, int x, int y);
typedef void (*E_Client_Hook_Cb)(void *data, E_Client *ec);
typedef void (*E_Client_Layout_Cb)(void);
#else

#ifndef HAVE_WAYLAND_ONLY
# include <Ecore_X.h>
#endif

#define E_CLIENT_TYPE (int)0xE0b01002

struct E_Event_Client
{
   E_Client *ec;
};

struct E_Event_Client_Desk_Set
{
   E_Client *ec;
   E_Desk *desk;
};

struct E_Event_Client_Zone_Set
{
   E_Client *ec;
   E_Zone *zone;
};

struct _E_Event_Client_Property
{
   E_Client *ec;
   unsigned int property;
};

struct _E_Client_Hook
{
   EINA_INLIST;
   E_Client_Hook_Point hookpoint;
   E_Client_Hook_Cb func;
   void               *data;
   unsigned char       delete_me E_BITFIELD;
};

struct _E_Client_Pending_Resize
{
   int           w, h;
   unsigned int  serial;
};

struct E_Client
{
   E_Object e_obj_inherit;
   EINA_INLIST;

   E_Pixmap *pixmap;
   int depth;
   int x, y, w, h; //frame+client geom
   struct {
      int x, y, w, h; //frame+client geom before move or resize callback
   } pre_cb;
   Eina_Rectangle client; //client geom
   Evas_Object *frame; //comp object
   Evas_Object *frame_object; //frame border comp object
   Evas_Object *agent; //resize agent;
   E_Zone *zone;
   E_Desk *desk;

   const char *restore_zone_id;

   Ecore_Poller              *ping_poller;
   Ecore_Timer               *kill_timer;

   E_Client                  *modal;

   E_Client                  *leader;
   Eina_List                 *group;

   E_Client                  *parent;
   Eina_List                 *transients;

   E_Layer                    layer;

   Eina_Rectangle           *shape_rects;
   unsigned int              shape_rects_num;

   Eina_Rectangle           *shape_input_rects;
   unsigned int              shape_input_rects_num;

   Eina_Stringshare         *internal_icon;
   Eina_Stringshare         *internal_icon_key;

   E_Direction               shade_dir;

   E_Comp_Client_Data       *comp_data;

   E_Action                  *cur_mouse_action;

   struct {
      E_Client               *next;
      E_Client               *prev;
      int                     ignore;
      Eina_Bool               focus_skip E_BITFIELD;
   } stack;

   int               border_size; //size of client's border

   struct
   {
      struct
      {
         int x, y, w, h;
         int mx, my;
      } current, last_down[3], last_up[3];
      Eina_Bool in E_BITFIELD;
   } mouse;

   struct
   {
      struct
      {
         int x, y, w, h;
         int mx, my;
         int button;
      } down;
   } moveinfo;

   unsigned char      ignore_first_unmap;
   E_Pointer_Mode     resize_mode;

   struct
   {
      Eina_Bool mapping_change E_BITFIELD;
      Eina_Bool iconic_shading E_BITFIELD;
   } hacks;

   struct
   {
      unsigned char changed E_BITFIELD;
      unsigned char user_selected E_BITFIELD;
      Eina_Stringshare *name;
   } border;

   struct
   {
      int          x, y, w, h;
      E_Layer      layer;
      int          zone;
      E_Maximize   maximized;
      Eina_Bool    frame E_BITFIELD;
      Eina_Bool    set E_BITFIELD;
   } saved;

   struct
   {
      unsigned char valid E_BITFIELD;
      int           x, y, w, h;
      struct
      {
         int x, y, w, h;
      } saved;
   } pre_res_change;

   unsigned char  shaped E_BITFIELD;
   unsigned char  argb E_BITFIELD;

   /* ICCCM */
   struct
   {
      Eina_Stringshare        *title;
      Eina_Stringshare        *name;
      Eina_Stringshare        *class;
      Eina_Stringshare        *icon_name;
      Eina_Stringshare        *machine;
      int                       min_w, min_h;
      int                       max_w, max_h;
      int                       base_w, base_h;
      int                       step_w, step_h;
      int                       start_x, start_y;
      double                    min_aspect, max_aspect;
      Ecore_Window            icon_window;
      Ecore_Window            window_group;
      uint64_t                transient_for;
      Ecore_Window            client_leader;
#ifndef HAVE_WAYLAND_ONLY
      Ecore_X_Window_State_Hint initial_state;
      Ecore_X_Window_State_Hint state;
      Ecore_X_Pixmap            icon_pixmap;
      Ecore_X_Pixmap            icon_mask;
      Ecore_X_Gravity           gravity;
#endif
      Eina_Stringshare         *window_role;
      unsigned char             take_focus E_BITFIELD;
      unsigned char             accepts_focus E_BITFIELD;
      unsigned char             urgent E_BITFIELD;
      unsigned char             delete_request E_BITFIELD;
      unsigned char             request_pos E_BITFIELD;
      struct
      {
         int    argc;
         char **argv;
      } command;
      struct
      {
         unsigned char title E_BITFIELD;
         unsigned char name_class E_BITFIELD;
         unsigned char icon_name E_BITFIELD;
         unsigned char machine E_BITFIELD;
         unsigned char hints E_BITFIELD;
         unsigned char size_pos_hints E_BITFIELD;
         unsigned char protocol E_BITFIELD;
         unsigned char transient_for E_BITFIELD;
         unsigned char client_leader E_BITFIELD;
         unsigned char window_role E_BITFIELD;
         unsigned char state E_BITFIELD;
         unsigned char command E_BITFIELD;
      } fetch;
   } icccm;

   /* MWM */
   struct
   {
#ifndef HAVE_WAYLAND_ONLY
      Ecore_X_MWM_Hint_Func  func;
      Ecore_X_MWM_Hint_Decor decor;
      Ecore_X_MWM_Hint_Input input;
#endif
      unsigned char          exists E_BITFIELD;
      unsigned char          borderless E_BITFIELD;
      struct
      {
         unsigned char hints E_BITFIELD;
      } fetch;
   } mwm;

   /* NetWM */
   struct
   {
      pid_t         pid;
      unsigned int  desktop;
      Eina_Stringshare *name;
      Eina_Stringshare *icon_name;
#ifndef HAVE_WAYLAND_ONLY
      Ecore_X_Icon *icons;
#endif
      int           num_icons;
      unsigned int  user_time;
      unsigned char opacity;
      Eina_Bool     opacity_changed E_BITFIELD; // prevent fetching opacity next prop change
      struct
      {
         int left;
         int right;
         int top;
         int bottom;
         int left_start_y;
         int left_end_y;
         int right_start_y;
         int right_end_y;
         int top_start_x;
         int top_end_x;
         int bottom_start_x;
         int bottom_end_x;
      } strut;
      unsigned char ping E_BITFIELD;
      struct
      {
         unsigned char        request E_BITFIELD;
         unsigned char        alarm E_BITFIELD;
         unsigned int         wait;
         unsigned int         serial;
         double               send_time;
      } sync;

      /* NetWM Window state */
      struct
      {
         unsigned char modal E_BITFIELD;
         unsigned char sticky E_BITFIELD;
         unsigned char maximized_v E_BITFIELD;
         unsigned char maximized_h E_BITFIELD;
         unsigned char shaded E_BITFIELD;
         unsigned char skip_taskbar E_BITFIELD;
         unsigned char skip_pager E_BITFIELD;
         unsigned char hidden E_BITFIELD;
         unsigned char fullscreen E_BITFIELD;
         E_Stacking    stacking;
      } state;

      /* NetWM Window allowed actions */
      struct
      {
         unsigned char move E_BITFIELD;
         unsigned char resize E_BITFIELD;
         unsigned char minimize E_BITFIELD;
         unsigned char shade E_BITFIELD;
         unsigned char stick E_BITFIELD;
         unsigned char maximized_h E_BITFIELD;
         unsigned char maximized_v E_BITFIELD;
         unsigned char fullscreen E_BITFIELD;
         unsigned char change_desktop E_BITFIELD;
         unsigned char close E_BITFIELD;
      } action;
      E_Window_Type  type;
      E_Window_Type *extra_types;
      int                  extra_types_num;
      int                  startup_id;

      struct
      {
         unsigned char name E_BITFIELD;
         unsigned char icon_name E_BITFIELD;
         unsigned char icon E_BITFIELD;
         unsigned char user_time E_BITFIELD;
         unsigned char strut E_BITFIELD;
         unsigned char type E_BITFIELD;
         unsigned char state E_BITFIELD;
         unsigned char opacity E_BITFIELD;
         /* No, fetch on new_client, shouldn't be changed after map.
            unsigned char pid E_BITFIELD;
          */
         /* No, ignore this
            unsigned char desktop E_BITFIELD;
          */
      } fetch;

      struct
      {
         unsigned char state E_BITFIELD;
      } update;
   } netwm;

   /* Extra e stuff */
   struct
   {
      struct
      {
         struct
         {
            int           x, y;

            unsigned char updated E_BITFIELD;
         } video_position;
         Ecore_Window video_parent;
         E_Client      *video_parent_client;
         Eina_List     *video_child;
         struct
         {
            Eina_Stringshare *name;
            Eina_Stringshare **available_list;
            Eina_Stringshare *set;
            Eina_Stringshare *wait;
            E_Desk           *wait_desk;
            E_Object_Delfn   *wait_desk_delfn;
            int               num;
            unsigned char     wait_for_done E_BITFIELD;
            unsigned char     use E_BITFIELD;
         } profile;
         Eina_Bool stack E_BITFIELD;
         unsigned char      centered E_BITFIELD;
         unsigned char      video E_BITFIELD;
      } state;

      struct
      {
         unsigned char state E_BITFIELD;
         unsigned char video_parent E_BITFIELD;
         unsigned char video_position E_BITFIELD;
         unsigned char profile E_BITFIELD;
         unsigned char stack E_BITFIELD;
      } fetch;
   } e;

   struct
   {
      struct
      {
         unsigned char soft_menu E_BITFIELD;
         unsigned char soft_menus E_BITFIELD;
      } fetch;

      unsigned char soft_menu E_BITFIELD;
      unsigned char soft_menus E_BITFIELD;
   } qtopia;

   struct
   {
      struct
      {
         unsigned char state E_BITFIELD;
         unsigned char vkbd E_BITFIELD;
      } fetch;
#ifndef HAVE_WAYLAND_ONLY
      Ecore_X_Virtual_Keyboard_State state;
#endif
      unsigned char                  have_property E_BITFIELD;
      unsigned char                  vkbd E_BITFIELD;
   } vkbd;

   struct {
      unsigned int steam_game_id;
   } steam;

   struct
   {
      unsigned char visible E_BITFIELD;
      unsigned char pos E_BITFIELD;
      unsigned char size E_BITFIELD;
      unsigned char stack E_BITFIELD;
      unsigned char prop E_BITFIELD;
      unsigned char border E_BITFIELD;
      unsigned char reset_gravity E_BITFIELD;
      unsigned char shading E_BITFIELD;
      unsigned char shaded E_BITFIELD;
      unsigned char shape E_BITFIELD;
      unsigned char shape_input E_BITFIELD;
      unsigned char icon E_BITFIELD;
      Eina_Bool internal_state E_BITFIELD;
      Eina_Bool need_maximize E_BITFIELD;
      Eina_Bool need_unmaximize E_BITFIELD;
      Eina_Bool need_rescale E_BITFIELD;
   } changes;

   unsigned int       visible E_BITFIELD; // client is set to be visible by display server (never use this)
   unsigned int       hidden E_BITFIELD; // set when window has been hidden by api and should not be shown
   unsigned int       moving E_BITFIELD;
   unsigned int       focused E_BITFIELD;
   unsigned int       new_client E_BITFIELD;
   unsigned int       re_manage E_BITFIELD; // client is persisting from before E restart
   unsigned int       placed E_BITFIELD;
   unsigned int       shading E_BITFIELD;
   unsigned int       shaded E_BITFIELD;
   unsigned int       iconic E_BITFIELD;
   unsigned int       deskshow E_BITFIELD;
   unsigned int       sticky E_BITFIELD;
   unsigned int       urgent E_BITFIELD;
   unsigned int       shaped_input E_BITFIELD;
   unsigned int       need_shape_merge E_BITFIELD;
   unsigned int       need_shape_export E_BITFIELD;
   unsigned int       fullscreen E_BITFIELD;
   unsigned int       unfullscreen_forced E_BITFIELD;
   unsigned int       need_fullscreen E_BITFIELD;
   unsigned int       already_unparented E_BITFIELD;
   unsigned int       need_reparent E_BITFIELD;
   unsigned int       button_grabbed E_BITFIELD;
   unsigned int       delete_requested E_BITFIELD;
   unsigned int       ping_ok E_BITFIELD;
   unsigned int       hung E_BITFIELD;
   unsigned int       take_focus E_BITFIELD;
   unsigned int       want_focus E_BITFIELD;
   unsigned int       user_skip_winlist E_BITFIELD;
   E_Maximize         maximized;
   E_Fullscreen       fullscreen_policy;
   unsigned int       borderless E_BITFIELD;
   unsigned char      offer_resistance E_BITFIELD;
   Eina_Stringshare  *bordername;

   unsigned int       lock_user_location E_BITFIELD; /*DONE*/
   unsigned int       lock_client_location E_BITFIELD; /*DONE*/
   unsigned int       lock_user_size E_BITFIELD; /*DONE*/
   unsigned int       lock_client_size E_BITFIELD; /*DONE*/
   unsigned int       lock_user_stacking E_BITFIELD; /*DONE*/
   unsigned int       lock_client_stacking E_BITFIELD; /*DONE*/
   unsigned int       lock_user_iconify E_BITFIELD; /*DONE*/
   unsigned int       lock_client_iconify E_BITFIELD; /*DONE*/
   unsigned int       lock_user_desk E_BITFIELD;
   unsigned int       lock_client_desk E_BITFIELD;
   unsigned int       lock_user_sticky E_BITFIELD; /*DONE*/
   unsigned int       lock_client_sticky E_BITFIELD; /*DONE*/
   unsigned int       lock_user_shade E_BITFIELD; /*DONE*/
   unsigned int       lock_client_shade E_BITFIELD; /*DONE*/
   unsigned int       lock_user_maximize E_BITFIELD; /*DONE*/
   unsigned int       lock_client_maximize E_BITFIELD; /*DONE*/
   unsigned int       lock_user_fullscreen E_BITFIELD; /*DONE*/
   unsigned int       lock_client_fullscreen E_BITFIELD; /*DONE*/
   unsigned int       lock_border E_BITFIELD; /*DONE*/
   unsigned int       lock_close E_BITFIELD; /*DONE*/
   unsigned int       lock_focus_in E_BITFIELD; /*DONE*/
   unsigned int       lock_focus_out E_BITFIELD; /*DONE*/
   unsigned int       lock_life E_BITFIELD; /*DONE*/

   unsigned int       stolen E_BITFIELD;

   unsigned int       frozen E_BITFIELD;

   unsigned int       internal E_BITFIELD;
   unsigned int       internal_no_remember E_BITFIELD;
   unsigned int       internal_no_reopen E_BITFIELD;

   unsigned int       next_mouse_action_ignore E_BITFIELD;

   Evas_Object       *internal_elm_win;

   double             ping;

   unsigned char      changed E_BITFIELD;

   unsigned char      icon_preference;

   struct
   {
      int x, y;
      int modified;
   } shelf_fix;

   Eina_List       *stick_desks;
   E_Menu          *border_menu;
   Evas_Object     *color_editor;
   E_Config_Dialog *border_locks_dialog;
   E_Config_Dialog *border_remember_dialog;
   E_Config_Dialog *border_border_dialog;
   E_Dialog        *border_prop_dialog;
   Eina_List       *pending_resize;

   struct
   {
      unsigned char start E_BITFIELD;
      int           x, y;
   } drag;

   Ecore_Timer               *raise_timer;
   E_Client_Move_Intercept_Cb move_intercept_cb;
   E_Remember                *remember;
   E_Remember                *sr_remember;

   Efreet_Desktop            *desktop;
   E_Exec_Instance           *exe_inst;

   unsigned char              comp_hidden   E_BITFIELD;

   unsigned char              post_move   E_BITFIELD;
   unsigned char              post_resize E_BITFIELD;
   unsigned char              post_show E_BITFIELD;
   unsigned char              during_lost E_BITFIELD;

   Ecore_Idle_Enterer        *post_job;

   E_Focus_Policy             focus_policy_override;

   Eina_Stringshare *uuid;

   Eina_List *sinks;
   int volume;
   int volume_min;
   int volume_max;
   unsigned char mute E_BITFIELD;
   unsigned char volume_control_enabled E_BITFIELD;

   Eina_Bool override E_BITFIELD;
   Eina_Bool input_only E_BITFIELD;
   Eina_Bool dialog E_BITFIELD;
   Eina_Bool tooltip E_BITFIELD;
   Eina_Bool redirected E_BITFIELD;
   Eina_Bool unredirected_single E_BITFIELD; //window has been selectively unredirected
   Eina_Bool shape_changed E_BITFIELD;
   Eina_Bool layer_block E_BITFIELD; // client is doing crazy stuff and should not be relayered in protocol
   Eina_Bool ignored E_BITFIELD; // client is comp-ignored
   Eina_Bool no_shape_cut E_BITFIELD; // client shape should not be cut
   Eina_Bool maximize_override E_BITFIELD; // client is doing crazy stuff and should "just do it" when moving/resizing
   Eina_Bool maximize_anims_disabled E_BITFIELD; // client cannot invoke anims or it will break
   Eina_Bool keyboard_resizing E_BITFIELD;

   Eina_Bool on_post_updates E_BITFIELD; // client is on the post update list
};

#define e_client_focus_policy_click(ec) \
  ((ec->focus_policy_override == E_FOCUS_CLICK) || (e_config->focus_policy == E_FOCUS_CLICK))

/* macro for finding misuse of changed flag */
#if 0
# define EC_CHANGED(EC) \
  do { \
     if (e_object_is_del(E_OBJECT(EC))) \
       EINA_LOG_CRIT("CHANGED SET ON DELETED CLIENT!"); \
     EC->changed = 1; \
     INF("%s:%d - EC CHANGED: %p", __FILE__, __LINE__, EC); \
  } while (0)
#else
# define EC_CHANGED(EC) EC->changed = 1
#endif

#define E_CLIENT_FOREACH(EC) \
  for (EC = e_client_bottom_get(); EC; EC = e_client_above_get(EC))

#define E_CLIENT_REVERSE_FOREACH(EC) \
  for (EC = e_client_top_get(); EC; EC = e_client_below_get(EC))


E_API extern int E_EVENT_CLIENT_ADD;
E_API extern int E_EVENT_CLIENT_REMOVE;
E_API extern int E_EVENT_CLIENT_ZONE_SET;
E_API extern int E_EVENT_CLIENT_DESK_SET;
E_API extern int E_EVENT_CLIENT_RESIZE;
E_API extern int E_EVENT_CLIENT_MOVE;
E_API extern int E_EVENT_CLIENT_SHOW;
E_API extern int E_EVENT_CLIENT_HIDE;
E_API extern int E_EVENT_CLIENT_ICONIFY;
E_API extern int E_EVENT_CLIENT_UNICONIFY;
E_API extern int E_EVENT_CLIENT_STACK;
E_API extern int E_EVENT_CLIENT_FOCUS_IN;
E_API extern int E_EVENT_CLIENT_FOCUS_OUT;
E_API extern int E_EVENT_CLIENT_PROPERTY;
E_API extern int E_EVENT_CLIENT_FULLSCREEN;
E_API extern int E_EVENT_CLIENT_UNFULLSCREEN;



EINTERN void e_client_idler_before(void);
EINTERN Eina_Bool e_client_init(void);
EINTERN void e_client_shutdown(void);
E_API E_Client *e_client_new(E_Pixmap *cp, int first_map, int internal);
E_API void e_client_unignore(E_Client *ec);
E_API void e_client_desk_set(E_Client *ec, E_Desk *desk);
E_API Eina_Bool e_client_comp_grabbed_get(void);
E_API E_Client *e_client_action_get(void);
E_API E_Client *e_client_warping_get(void);
E_API Eina_List *e_clients_immortal_list(void);
E_API void e_client_mouse_in(E_Client *ec, int x, int y);
E_API void e_client_mouse_out(E_Client *ec, int x, int y);
E_API void e_client_mouse_wheel(E_Client *ec, Evas_Point *output, E_Binding_Event_Wheel *ev);
E_API void e_client_mouse_down(E_Client *ec, int button, Evas_Point *output, E_Binding_Event_Mouse_Button *ev);
E_API void e_client_mouse_up(E_Client *ec, int button, Evas_Point *output, E_Binding_Event_Mouse_Button* ev);
E_API void e_client_mouse_move(E_Client *ec, Evas_Point *output);
E_API void e_client_res_change_geometry_save(E_Client *bd);
E_API void e_client_res_change_geometry_restore(E_Client *ec);
E_API void e_client_rescale(E_Client *ec);
E_API void e_client_zone_set(E_Client *ec, E_Zone *zone);
E_API void e_client_geometry_get(E_Client *ec, int *x, int *y, int *w, int *h);
E_API E_Client *e_client_above_get(const E_Client *ec);
E_API E_Client *e_client_below_get(const E_Client *ec);
E_API E_Client *e_client_bottom_get(void);
E_API E_Client *e_client_top_get(void);
E_API unsigned int e_clients_count(void);
E_API void e_client_move_intercept_cb_set(E_Client *ec, E_Client_Move_Intercept_Cb cb);
E_API E_Client_Hook *e_client_hook_add(E_Client_Hook_Point hookpoint, E_Client_Hook_Cb func, const void *data);
E_API void e_client_hook_del(E_Client_Hook *ch);
E_API void e_client_focus_latest_set(E_Client *ec);
E_API void e_client_raise_latest_set(E_Client *ec);
E_API Eina_Bool e_client_focus_track_enabled(void);
E_API void e_client_focus_track_freeze(void);
E_API void e_client_focus_track_thaw(void);
E_API void e_client_refocus(void);
E_API void e_client_focus_set_with_pointer(E_Client *ec);
E_API void e_client_activate(E_Client *ec, Eina_Bool just_do_it);
E_API E_Client *e_client_focused_get(void);
E_API Eina_List *e_client_focus_stack_get(void);
E_API Eina_List *e_client_raise_stack_get(void);
E_API Eina_List *e_client_lost_windows_get(E_Zone *zone);
E_API void e_client_shade(E_Client *ec, E_Direction dir);
E_API void e_client_unshade(E_Client *ec, E_Direction dir);
E_API Eina_Bool e_client_maximize_geometry_get(const E_Client *ec, E_Maximize max, int *mx, int *my, int *mw, int *mh);
E_API void e_client_maximize(E_Client *ec, E_Maximize max);
E_API Eina_Bool e_client_unmaximize_geometry_get(const E_Client *ec, E_Maximize max, int *mx, int *my, int *mw, int *mh);
E_API void e_client_unmaximize(E_Client *ec, E_Maximize max);
E_API void e_client_fullscreen(E_Client *ec, E_Fullscreen policy);
E_API void e_client_unfullscreen(E_Client *ec);
E_API void e_client_iconify(E_Client *ec);
E_API void e_client_uniconify(E_Client *ec);
E_API void e_client_urgent_set(E_Client *ec, Eina_Bool urgent);
E_API void e_client_stick(E_Client *ec);
E_API void e_client_unstick(E_Client *ec);
E_API void e_client_pinned_set(E_Client *ec, Eina_Bool set);
E_API void e_client_prop_misc_changed(E_Client *ec);
E_API void e_client_comp_hidden_set(E_Client *ec, Eina_Bool hidden);
E_API Eina_Bool e_client_border_set(E_Client *ec, const char *name);
E_API void e_client_act_move_keyboard(E_Client *ec);
E_API void e_client_act_resize_keyboard(E_Client *ec);
E_API void e_client_act_move_begin(E_Client *ec, E_Binding_Event_Mouse_Button *ev);
E_API void e_client_act_move_end(E_Client *ec, E_Binding_Event_Mouse_Button *ev EINA_UNUSED);
E_API void e_client_act_resize_begin(E_Client *ec, E_Binding_Event_Mouse_Button *ev);
E_API void e_client_act_resize_end(E_Client *ec, E_Binding_Event_Mouse_Button *ev EINA_UNUSED);
E_API void e_client_act_menu_begin(E_Client *ec, E_Binding_Event_Mouse_Button *ev, int key);
E_API void e_client_act_close_begin(E_Client *ec);
E_API void e_client_act_kill_begin(E_Client *ec);
E_API Evas_Object *e_client_icon_add(E_Client *ec, Evas *evas);
E_API void e_client_ping(E_Client *cw);
E_API void e_client_move_cancel(void);
E_API void e_client_resize_cancel(void);
E_API Eina_Bool e_client_resize_begin(E_Client *ec);
E_API void e_client_frame_recalc(E_Client *ec);
E_API void e_client_signal_move_begin(E_Client *ec, const char *sig, const char *src EINA_UNUSED);
E_API void e_client_signal_move_end(E_Client *ec, const char *sig EINA_UNUSED, const char *src EINA_UNUSED);
E_API void e_client_signal_resize_begin(E_Client *ec, const char *dir, const char *sig, const char *src EINA_UNUSED);
E_API void e_client_signal_resize_end(E_Client *ec, const char *dir EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED);
E_API void e_client_resize_limit(const E_Client *ec, int *w, int *h);
E_API E_Client *e_client_under_pointer_get(E_Desk *desk, E_Client *exclude);
E_API int e_client_pointer_warp_to_center_now(E_Client *ec);
E_API int e_client_pointer_warp_to_center(E_Client *ec);
E_API void e_client_redirected_set(E_Client *ec, Eina_Bool set);
E_API void e_client_next_mouse_action_ignore(E_Client *ec);
E_API Eina_Bool e_client_is_stacking(const E_Client *ec);
E_API Eina_Bool e_client_has_xwindow(const E_Client *ec);
E_API Eina_Bool e_client_desk_window_profile_available_check(E_Client *ec, const char *profile);
E_API void      e_client_desk_window_profile_wait_desk_set(E_Client *ec, E_Desk *desk);
E_API void      e_client_layout_cb_set(E_Client_Layout_Cb cb);
E_API Eina_List *e_client_stack_list_prepare(E_Client *ec);
E_API void       e_client_stack_list_finish(Eina_List *list);
E_API E_Client  *e_client_stack_top_get(E_Client *ec);
E_API E_Client  *e_client_stack_bottom_get(E_Client *ec);
E_API E_Client  *e_client_stack_active_adjust(E_Client *ec);
E_API Eina_Bool  e_client_stack_focused_get(E_Client *ec);
E_API Eina_Bool  e_client_stack_iconified_get(E_Client *ec);
E_API Eina_Bool  e_client_stack_urgent_get(E_Client *ec);

YOLO E_API void e_client_focus_stack_set(Eina_List *l);

E_API void e_client_parent_set(E_Client *ec, E_Client *parent);
E_API void e_client_transients_restack(E_Client *ec);
#include "e_client.x"
#endif
