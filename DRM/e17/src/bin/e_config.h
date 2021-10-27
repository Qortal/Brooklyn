#ifdef E_TYPEDEFS

#define E_CONFIG_LIMIT(v, min, max) {if (v >= max) v = max; else if (v <= min) v = min; }

typedef struct _E_Config                    E_Config;
typedef struct _E_Config_Module             E_Config_Module;
typedef struct _E_Config_Binding_Mouse      E_Config_Binding_Mouse;
typedef struct _E_Config_Binding_Key        E_Config_Binding_Key;
typedef struct _E_Config_Binding_Edge       E_Config_Binding_Edge;
typedef struct _E_Config_Binding_Signal     E_Config_Binding_Signal;
typedef struct _E_Config_Binding_Wheel      E_Config_Binding_Wheel;
typedef struct _E_Config_Binding_Acpi       E_Config_Binding_Acpi;
typedef struct _E_Config_Binding_Swipe      E_Config_Binding_Swipe;
typedef struct _E_Config_Desktop_Background E_Config_Desktop_Background;
typedef struct _E_Config_Desklock_Background E_Config_Desklock_Background;
typedef struct _E_Config_Desktop_Name       E_Config_Desktop_Name;
typedef struct _E_Config_Gadcon             E_Config_Gadcon;
typedef struct _E_Config_Gadcon_Client      E_Config_Gadcon_Client;
typedef struct _E_Config_Shelf              E_Config_Shelf;
typedef struct _E_Config_Shelf_Desk         E_Config_Shelf_Desk;
typedef struct _E_Config_Mime_Icon          E_Config_Mime_Icon;
typedef struct _E_Config_Syscon_Action      E_Config_Syscon_Action;
typedef struct _E_Config_Env_Var            E_Config_Env_Var;
typedef struct _E_Config_XKB_Layout         E_Config_XKB_Layout;
typedef struct _E_Config_XKB_Option         E_Config_XKB_Option;

typedef struct _E_Event_Config_Icon_Theme   E_Event_Config_Icon_Theme;

typedef struct E_Config_Bindings E_Config_Bindings;

typedef enum
{
   E_CONFIG_PROFILE_TYPE_NONE,
   E_CONFIG_PROFILE_TYPE_MOBILE,
   E_CONFIG_PROFILE_TYPE_TABLET,
   E_CONFIG_PROFILE_TYPE_DESKTOP
} E_Config_Profile_Type;

#else
#ifndef E_CONFIG_H
#define E_CONFIG_H

/* increment this whenever we change config enough that you need new
 * defaults for e to work.
 */
#define E_CONFIG_FILE_EPOCH      1
/* increment this whenever a new set of config values are added but the users
 * config doesn't need to be wiped - simply new values need to be put in
 */
#define E_CONFIG_FILE_GENERATION 34
#define E_CONFIG_FILE_VERSION    ((E_CONFIG_FILE_EPOCH * 1000000) + E_CONFIG_FILE_GENERATION)

#define E_CONFIG_BINDINGS_VERSION 0 // DO NOT INCREMENT UNLESS YOU WANT TO WIPE ALL BINDINGS!!!!!

struct _E_Config
{
   int         config_version; // INTERNAL
   E_Config_Profile_Type config_type; // INTERNAL
   const char *desktop_default_background; // GUI
   Eina_List  *desktop_backgrounds; // GUI
   const char *desktop_default_name; // DEAD
   Eina_List  *desktop_names; // GUI
   double      menus_scroll_speed; // GUI
   double      menus_fast_mouse_move_threshhold; // GUI
   double      menus_click_drag_timeout; // GUI
   int         window_maximize_animate; // GUI
   int         window_maximize_transition; // GUI
   double      window_maximize_time; // GUI
   int         border_shade_animate; // GUI
   int         border_shade_transition; // GUI
   double      border_shade_speed; // GUI
   int         priority; // GUI
   int         zone_desks_x_count; // GUI
   int         zone_desks_y_count; // GUI
   int         show_desktop_icons; // GUI
   int         edge_flip_dragging; // GUI
   const char *language; // GUI
   const char *desklock_language; // GUI
   Eina_List  *modules; // GUI
   Eina_List  *bad_modules; // GUI
   Eina_List  *font_fallbacks; // GUI
   Eina_List  *font_defaults; // GUI

   /* NO LONGER SAVED WITH THIS STRUCT */
   Eina_List  *mouse_bindings; // GUI
   Eina_List  *key_bindings; // GUI
   Eina_List  *edge_bindings; // GUI
   Eina_List  *signal_bindings; // GUI
   Eina_List  *wheel_bindings; // GUI
   Eina_List  *acpi_bindings; // GUI
   Eina_List  *swipe_bindings; // GUI

   Eina_List  *path_append_data; // GUI
   Eina_List  *path_append_images; // GUI
   Eina_List  *path_append_fonts; // GUI
   Eina_List  *path_append_init; // GUI
   Eina_List  *path_append_icons; // GUI
   Eina_List  *path_append_modules; // GUI
   Eina_List  *path_append_backgrounds; // GUI
   Eina_List  *path_append_messages; // GUI
   int         window_placement_policy; // GUI
   int         window_grouping; // GUI
   int         focus_policy; // GUI
   int         focus_setting; // GUI
   int         pass_click_on; // GUI
   int         window_activehint_policy; // GUI
   int         always_click_to_raise; // GUI
   int         always_click_to_focus; // GUI
   int         use_auto_raise; // GUI
   double      auto_raise_delay; // GUI
   int         use_resist; // GUI
   int         drag_resist;
   int         desk_resist; // GUI
   int         window_resist; // GUI
   int         gadget_resist; // GUI
   int         geometry_auto_move; // GUI
   int         geometry_auto_resize_limit; // GUI
   int         winlist_warp_while_selecting; // GUI
   int         winlist_warp_at_end; // GUI
   int         winlist_no_warp_on_direction; // GUI
   double      winlist_warp_speed; // GUI **** NO LONGER USED!!!
   int         winlist_scroll_animate; // GUI
   double      winlist_scroll_speed; // GUI
   int         winlist_list_show_iconified; // GUI
   int         winlist_list_show_other_desk_iconified; // GUI
   int         winlist_list_show_other_screen_iconified; // GUI
   int         winlist_list_show_other_desk_windows; // GUI
   int         winlist_list_show_other_screen_windows; // GUI
   int         winlist_list_uncover_while_selecting; // GUI
   int         winlist_list_jump_desk_while_selecting; // GUI
   int         winlist_list_focus_while_selecting; // GUI
   int         winlist_list_raise_while_selecting; // GUI
   int         winlist_list_move_after_select; // GUI
   int         winlist_mode; // GUI
   double      winlist_list_size; // GUI
   double      winlist_large_size; // GUI
   Eina_Bool   winlist_list_no_miniatures; // GUI
   int         maximize_policy; // GUI
   int         allow_manip; // GUI
   int         border_fix_on_shelf_toggle; // GUI
   int         allow_above_fullscreen; // GUI
   int         kill_if_close_not_possible; // GUI
   int         kill_process; // GUI
   double      kill_timer_wait; // GUI
   int         ping_clients; // GUI
   const char *transition_start; // GUI
   const char *transition_desk; // GUI
   const char *transition_change; // GUI
   Eina_List  *remembers; // GUI
   int         remember_internal_windows; // GUI
   Eina_Bool  remember_internal_fm_windows; // GUI
   Eina_Bool  remember_internal_fm_windows_globally; // GUI
   int         move_info_follows; // GUI
   int         resize_info_follows; // GUI
   int         move_info_visible; // GUI
   int         resize_info_visible; // GUI
   int         focus_last_focused_per_desktop; // GUI
   int         focus_revert_on_hide_or_close; // GUI
   int         focus_revert_allow_sticky; // GUI
   int         disable_all_pointer_warps; // GUI
   int         pointer_slide; // GUI
   double      pointer_warp_speed; // GUI
   int         use_e_cursor; // GUI
   int         cursor_size; // GUI
   int         menu_autoscroll_margin; // GUI
   int         menu_autoscroll_cursor_margin; // GUI
   const char *input_method; // GUI
   struct {
      int move;      // GUI
      int resize;      // GUI
      int raise;      // GUI
      int lower;      // GUI
      int layer;      // GUI
      int desktop;      // GUI
      int iconify;      // GUI
   } transient;
   int                       menu_eap_name_show; // GUI
   int                       menu_eap_generic_show; // GUI
   int                       menu_eap_comment_show; // GUI
   int                       menu_favorites_show; // GUI
   int                       menu_apps_show; // GUI
   Eina_Bool                 menu_icons_hide; // GUI
   int                       menu_gadcon_client_toplevel; // GUI
   int                       fullscreen_policy; // GUI
   const char               *exebuf_term_cmd; // GUI
   Eina_List                *color_classes; // dead
   int                       use_app_icon; // GUI
   int                       cnfmdlg_disabled; // GUI
   int                       cfgdlg_auto_apply; // GUI
   int                       cfgdlg_default_mode; // GUI
   Eina_List                *gadcons; // GUI
   Eina_List                *shelves; // GUI
   int                       font_hinting; // GUI

   int                       desklock_passwd; // GUI // hashed
   int                       desklock_pin; // GUI // hashed
   Eina_List                *desklock_backgrounds; // GUI
   int                       desklock_auth_method; // GUI
   int                       desklock_login_box_zone; // GUI
   int                       desklock_start_locked; // GUI
   int                       desklock_on_suspend; // GUI
   int                       desklock_autolock_screensaver; // GUI
   double                    desklock_post_screensaver_time; // GUI
   unsigned char             desklock_ask_presentation; // GUI
   double                    desklock_ask_presentation_timeout; // GUI

   int                       screensaver_enable; // GUI
   int                       screensaver_timeout; // GUI
   int                       screensaver_interval; // GUI
   int                       screensaver_blanking; // GUI
   int                       screensaver_expose; // GUI
   unsigned char             screensaver_dpms_off; // GUI
   unsigned char             screensaver_ask_presentation; // GUI
   double                    screensaver_ask_presentation_timeout; // GUI
   int                       screensaver_desklock_timeout; // GUI

   int                       screensaver_wake_on_notify; // GUI
   int                       screensaver_wake_on_urgent; // GUI

   unsigned char             suspend_connected_standby; // GUI

   unsigned char             screensaver_suspend; // GUI
   unsigned char             screensaver_hibernate; // GUI
   unsigned char             screensaver_suspend_on_ac; // GUI
   double                    screensaver_suspend_delay; // GUI

   int                       dpms_enable; // GUI
   int                       dpms_standby_enable; // GUI
   int                       dpms_standby_timeout; // GUI
   int                       dpms_suspend_enable; // GUI
   int                       dpms_suspend_timeout; // GUI
   int                       dpms_off_enable; // GUI
   int                       dpms_off_timeout; // GUI
   unsigned char             no_dpms_on_fullscreen; // GUI

   int                       clientlist_group_by; // GUI
   int                       clientlist_include_all_zones; // GUI
   int                       clientlist_separate_with; // GUI
   int                       clientlist_sort_by; // GUI
   int                       clientlist_separate_iconified_apps; // GUI
   int                       clientlist_warp_to_iconified_desktop; // GUI
   int                       clientlist_limit_caption_len; // GUI
   int                       clientlist_max_caption_len; // GUI

   int                       mouse_hand; //GUI
   double                    mouse_accel; // GUI
   int                       mouse_accel_threshold; // GUI
   unsigned char             mouse_natural_scroll; // GUI
   unsigned char             mouse_emulate_middle_button; // GUI

   double                    touch_accel; // GUI
   unsigned char             touch_natural_scroll; // GUI
   unsigned char             touch_emulate_middle_button; // GUI
   unsigned char             touch_tap_to_click; // GUI
   unsigned char             touch_clickpad; // GUI
   unsigned char             touch_scrolling_2finger; // GUI
   unsigned char             touch_scrolling_edge; // GUI
   unsigned char             touch_scrolling_circular; // GUI
   unsigned char             touch_scrolling_horiz; // GUI
   unsigned char             touch_palm_detect; // GUI

   int                       border_raise_on_mouse_action; // GUI
   int                       border_raise_on_focus; // GUI
   int                       raise_on_revert_focus; // GUI
   int                       desk_flip_wrap; // GUI
   int                       fullscreen_flip; // GUI
   int                       multiscreen_flip; // GUI

   const char               *icon_theme; // GUI
   unsigned char             icon_theme_overrides; // GUI
   const char               *desktop_environment; // GUI

   /* modes:
    * 1-"pane") horizontal or vertical movement to/from next/previous "screen"
    * 2-"zoom") 45degree diagonal movement based on border position
    */
   int                       desk_flip_animate_mode; // GUI
   /* types based on theme */
   Eina_Stringshare         *desk_flip_animate_type; // GUI
   int                       desk_flip_animate_interpolation; // GUI

   const char               *wallpaper_import_last_dev; // INTERNAL
   const char               *wallpaper_import_last_path; // INTERNAL

   const char               *theme_default_border_style; // GUI

   Eina_List                *mime_icons; // GUI
   int                       desk_auto_switch; // GUI;

   int                       screen_limits;

   int                       thumb_nice;

   int                       ping_clients_interval; // GUI

   int                       thumbscroll_enable; // GUI
   int                       thumbscroll_threshhold; // GUI
   double                    thumbscroll_momentum_threshhold; // GUI
   double                    thumbscroll_friction; // GUI

   unsigned char             filemanager_single_click; // GUI
   int                       device_desktop; // GUI
   int                       device_auto_mount; // GUI
   int                       device_auto_open; // GUI
   Efm_Mode                  device_detect_mode; /* not saved, display-only */
   unsigned char             filemanager_copy; // GUI
   unsigned char             filemanager_secure_rm; // GUI

   struct
   {
      double timeout; // GUI
      struct
      {
         unsigned char dx; // GUI
         unsigned char dy; // GUI
      } move;
      struct
      {
         unsigned char dx; // GUI
         unsigned char dy; // GUI
      } resize;
   } border_keyboard;

   struct
   {
      double        min; // GUI
      double        max; // GUI
      double        factor; // GUI
      int           base_dpi; // GUI
      int           xapp_base_dpi; // GUI
      unsigned char use_dpi; // GUI
      unsigned char use_custom; // GUI
      unsigned char set_xapp_dpi; // GUI
   } scale;

   unsigned char show_cursor; // GUI
   unsigned char idle_cursor; // GUI

   const char   *default_system_menu; // GUI

   unsigned char cfgdlg_normal_wins; // GUI

   struct
   {
      struct
      {
         int icon_size;         // GUI
      } main, secondary, extra;
      double        timeout;  // GUI
      unsigned char do_input;  // GUI
      Eina_List    *actions;
   } syscon;

   struct
   {
      double        expire_timeout;
      unsigned char show_run_dialog;
      unsigned char show_exit_dialog;
   } exec;

   unsigned char null_container_win; // HYPER-ADVANCED-ONLY - TURNING ON KILLS DESKTOP BG

   Eina_List    *env_vars; // GUI

   struct
   {
      double        normal; // GUI
      double        dim; // GUI
      double        transition; // GUI
      double        timer; // GUI
      double        battery_timer; // GUI
      const char   *sysdev; // GUI
      unsigned char idle_dim; // GUI
   } backlight;

   struct
   {
      double           none;
      double           low;
      double           medium;
      double           high;
      double           extreme;
      E_Powersave_Mode min;
      E_Powersave_Mode max;
   } powersave;

   struct
   {
      unsigned char load_xrdb; // GUI
      unsigned char load_xmodmap; // GUI
      unsigned char load_gnome; // GUI
      unsigned char load_kde; // GUI
   } deskenv;

   struct
   {
      unsigned char enabled;  // GUI
      unsigned char match_e17_theme;  // GUI
      unsigned char match_e17_icon_theme;  // GUI
      int           xft_antialias;
      int           xft_hinting;
      const char   *xft_hint_style;
      const char   *xft_rgba;
      const char   *net_theme_name;  // GUI
      const char   *net_theme_name_detected; // not saved
      const char   *net_icon_theme_name;
      const char   *gtk_font_name;
   } xsettings;

   struct
   {
      unsigned char check; // INTERNAL
      unsigned char later; // INTERNAL
   } update;

   struct
   {
      Eina_List  *used_layouts;
      Eina_List  *used_options;
      int         only_label;
      const char *default_model;
      int         cur_group;
      E_Config_XKB_Layout *current_layout;
      E_Config_XKB_Layout *sel_layout;
      E_Config_XKB_Layout *lock_layout;
      Eina_Bool dont_touch_my_damn_keyboard;

      /* NO LONGER USED BECAUSE I SUCK
       * -zmike, 31 January 2013
       */
      const char *cur_layout; // whatever the current layout is
      const char *selected_layout; // whatever the current layout that the user has selected is
      const char *desklock_layout;
      Eina_Bool use_cache;
   } xkb;

   struct
   {
      int repeat_delay;//delay in milliseconds since key down until repeating starts
      int repeat_rate;//the rate of repeating keys in characters per second
   } keyboard;

   Eina_List  *menu_applications;
   unsigned char exe_always_single_instance; // GUI
   unsigned char explicit_date_time; // GUI
   Eina_List *screen_profiles;
};

struct E_Config_Bindings
{
   unsigned int config_version;
   Eina_List  *mouse_bindings; // GUI
   Eina_List  *key_bindings; // GUI
   Eina_List  *edge_bindings; // GUI
   Eina_List  *signal_bindings; // GUI
   Eina_List  *wheel_bindings; // GUI
   Eina_List  *acpi_bindings; // GUI
   Eina_List  *swipe_bindings;
};

struct _E_Config_Desklock_Background
{
   const char *file;
   Eina_Bool hide_logo;
};

struct _E_Config_Env_Var
{
   const char   *var;
   const char   *val;
   unsigned char unset;
};

struct _E_Config_Syscon_Action
{
   const char *action;
   const char *params;
   const char *button;
   const char *icon;
   int         is_main;
};

struct _E_Config_Module
{
   const char   *name;
   unsigned char enabled;
};

struct _E_Config_Binding_Mouse
{
   int           context;
   int           modifiers;
   const char   *action;
   const char   *params;
   unsigned char button;
   unsigned char any_mod;
};

struct _E_Config_Binding_Key
{
   int           context;
   unsigned int  modifiers;
   const char   *key;
   const char   *action;
   const char   *params;
   unsigned char any_mod;
};

struct _E_Config_Binding_Edge
{
   int           context;
   int           modifiers;
   float         delay;
   const char   *action;
   const char   *params;
   unsigned char edge;
   unsigned char any_mod;
   Eina_Bool    drag_only;
};

struct _E_Config_Binding_Signal
{
   int           context;
   const char   *signal;
   const char   *source;
   int           modifiers;
   unsigned char any_mod;
   const char   *action;
   const char   *params;
};

struct _E_Config_Binding_Wheel
{
   int           context;
   int           direction;
   int           z;
   int           modifiers;
   unsigned char any_mod;
   const char   *action;
   const char   *params;
};

struct _E_Config_Binding_Acpi
{
   int         context, type, status;
   const char *action, *params;
};

struct _E_Config_Binding_Swipe
{
   int context;
   unsigned int fingers;
   double direction, length, error;
   const char *action, *params;
};

struct _E_Config_Desktop_Background
{
   int         zone;
   int         desk_x;
   int         desk_y;
   const char *file;
};

struct _E_Config_Desktop_Name
{
   int         zone;
   int         desk_x;
   int         desk_y;
   const char *name;
};

struct _E_Config_Gadcon
{
   const char  *name;
   int          id;
   unsigned int zone;
   Eina_List   *clients;
};

struct _E_Config_Gadcon_Client
{
   const char   *name;
   const char   *id;
   struct
   {
      int    pos, size, res; //gadcon
      double pos_x, pos_y, size_w, size_h;  //gadman
   } geom;
   struct
   {
      int seq, flags;
   } state_info;
   const char   *style;
   int           orient;
   unsigned char autoscroll;
   unsigned char resizable;
   const char   *theme;
};

struct _E_Config_Shelf
{
   const char   *name;
   int           id;
   int           zone;
   int           layer; //E_Layer
   unsigned char popup; //DEAD
   int           orient;
   unsigned char fit_along;
   unsigned char fit_size;
   const char   *style;
   int           size;
   int           overlap;
   int           autohide;
   int           autohide_show_action;
   float         hide_timeout;
   float         hide_duration;
   int           desk_show_mode;
   Eina_List    *desk_list;
};

struct _E_Config_Shelf_Desk
{
   int x, y;
};

struct _E_Config_Mime_Icon
{
   const char *mime;
   const char *icon;
};

struct _E_Event_Config_Icon_Theme
{
   const char *icon_theme;
};

struct _E_Config_XKB_Layout
{
   const char *name;
   const char *model;
   const char *variant;
};

struct _E_Config_XKB_Option
{
   const char *name;
};

EINTERN int                   e_config_init(void);
EINTERN int                   e_config_shutdown(void);

E_API void                     e_config_load(void);

E_API int                      e_config_save(void);
E_API void                     e_config_save_flush(void);
E_API void                     e_config_save_queue(void);

E_API const char              *e_config_profile_get(void);
E_API char                    *e_config_profile_dir_get(const char *prof);
E_API void                     e_config_profile_set(const char *prof);
E_API Eina_List               *e_config_profile_list(void);
E_API void                     e_config_profile_add(const char *prof);
E_API void                     e_config_profile_del(const char *prof);

E_API void                     e_config_save_block_set(int block);
E_API int                      e_config_save_block_get(void);

E_API void                    *e_config_domain_load(const char *domain, E_Config_DD *edd);
E_API void                    *e_config_domain_system_load(const char *domain, E_Config_DD *edd);
E_API int                      e_config_profile_save(void);
E_API int                      e_config_domain_save(const char *domain, E_Config_DD *edd, const void *data);

E_API E_Config_Binding_Mouse  *e_config_binding_mouse_match(E_Config_Binding_Mouse *eb_in);
E_API E_Config_Binding_Key    *e_config_binding_key_match(E_Config_Binding_Key *eb_in);
E_API E_Config_Binding_Edge   *e_config_binding_edge_match(E_Config_Binding_Edge *eb_in);
E_API E_Config_Binding_Signal *e_config_binding_signal_match(E_Config_Binding_Signal *eb_in);
E_API E_Config_Binding_Wheel  *e_config_binding_wheel_match(E_Config_Binding_Wheel *eb_in);
E_API E_Config_Binding_Acpi   *e_config_binding_acpi_match(E_Config_Binding_Acpi *eb_in);
E_API void                     e_config_mode_changed(void);


E_API void e_config_bindings_free(E_Config_Bindings *ecb);
E_API void e_config_binding_signal_free(E_Config_Binding_Signal *ebs);
E_API void e_config_binding_wheel_free(E_Config_Binding_Wheel *ebw);
E_API void e_config_binding_mouse_free(E_Config_Binding_Mouse *ebm);
E_API void e_config_binding_edge_free(E_Config_Binding_Edge *ebe);
E_API void e_config_binding_key_free(E_Config_Binding_Key *ebk);
E_API void e_config_binding_acpi_free(E_Config_Binding_Acpi *eba);
E_API void e_config_binding_swipe_free(E_Config_Binding_Swipe *eba);

extern E_API E_Config *e_config;
extern E_API E_Config_Bindings *e_bindings;

extern E_API int E_EVENT_CONFIG_ICON_THEME;
extern E_API int E_EVENT_CONFIG_MODE_CHANGED;
extern E_API int E_EVENT_CONFIG_LOADED;

#endif
#endif
