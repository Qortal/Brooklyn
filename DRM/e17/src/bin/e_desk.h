#ifdef E_TYPEDEFS

typedef struct _E_Desk E_Desk;
typedef struct _E_Event_Desk_Show E_Event_Desk_Show;
typedef struct _E_Event_Desk_Before_Show E_Event_Desk_Before_Show;
typedef struct _E_Event_Desk_After_Show E_Event_Desk_After_Show;
typedef struct _E_Event_Desk_Name_Change E_Event_Desk_Name_Change;
typedef struct _E_Event_Desk_Window_Profile_Change E_Event_Desk_Window_Profile_Change;

typedef void (*E_Desk_Flip_Cb)(void *data, E_Desk *desk, int dx, int dy, Eina_Bool show);

#else
#ifndef E_DESK_H
#define E_DESK_H

#define E_DESK_TYPE (int)0xE0b01005

typedef enum
{
   E_DESKFLIP_ANIMATION_MODE_OFF,
   E_DESKFLIP_ANIMATION_MODE_PANE,
   E_DESKFLIP_ANIMATION_MODE_ZOOM
} E_Deskflip_Animation_Mode;

struct _E_Desk
{
   E_Object             e_obj_inherit;

   E_Zone              *zone;
   Eina_Stringshare    *name;
   Eina_Stringshare    *window_profile;
   int                  x, y;
   unsigned char        visible E_BITFIELD;
   unsigned int         deskshow_toggle E_BITFIELD;
   Eina_List            *fullscreen_clients;

   Evas_Object         *bg_object;

   unsigned int animate_count;
   Eina_Inlist *obstacles;
};

struct _E_Event_Desk_Show
{
   E_Desk   *desk;
};

struct _E_Event_Desk_Before_Show
{
   E_Desk   *desk;
};

struct _E_Event_Desk_After_Show
{
   E_Desk   *desk;
};

struct _E_Event_Desk_Name_Change
{
   E_Desk   *desk;
};

struct _E_Event_Desk_Window_Profile_Change
{
   E_Desk   *desk;
};

EINTERN int          e_desk_init(void);
EINTERN int          e_desk_shutdown(void);
E_API E_Desk      *e_desk_new(E_Zone *zone, int x, int y);
E_API void         e_desk_name_set(E_Desk *desk, const char *name);
E_API void         e_desk_name_add(int zone, int desk_x, int desk_y, const char *name);
E_API void         e_desk_name_del(int zone, int desk_x, int desk_y);
E_API void         e_desk_name_update(void);
E_API void         e_desk_show(E_Desk *desk);
E_API void         e_desk_deskshow(E_Zone *zone);
E_API E_Client    *e_desk_last_focused_focus(E_Desk *desk);
E_API E_Client    *e_desk_client_top_visible_get(const E_Desk *desk);
E_API E_Desk      *e_desk_current_get(E_Zone *zone);
E_API E_Desk      *e_desk_at_xy_get(const E_Zone *zone, int x, int y);
E_API E_Desk      *e_desk_at_pos_get(E_Zone *zone, int pos);
E_API void         e_desk_xy_get(E_Desk *desk, int *x, int *y);
E_API void         e_desk_next(E_Zone *zone);
E_API void         e_desk_prev(E_Zone *zone);
E_API void         e_desk_row_add(E_Zone *zone);
E_API void         e_desk_row_remove(E_Zone *zone);
E_API void         e_desk_col_add(E_Zone *zone);
E_API void         e_desk_col_remove(E_Zone *zone);
E_API void         e_desk_window_profile_set(E_Desk *desk, const char *profile);
E_API void         e_desk_window_profile_update(E_Zone *zone);

E_API void         e_desk_flip_cb_set(E_Desk_Flip_Cb cb, const void *data);
E_API void         e_desk_flip_end(E_Desk *desk);

E_API unsigned int e_desks_count(void);

extern E_API int E_EVENT_DESK_SHOW;
extern E_API int E_EVENT_DESK_BEFORE_SHOW;
extern E_API int E_EVENT_DESK_AFTER_SHOW;
extern E_API int E_EVENT_DESK_DESKSHOW;
extern E_API int E_EVENT_DESK_NAME_CHANGE;
extern E_API int E_EVENT_DESK_WINDOW_PROFILE_CHANGE;

#endif
#endif
