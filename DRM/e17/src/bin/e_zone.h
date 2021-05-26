#ifdef E_TYPEDEFS

typedef enum _E_Zone_Edge
{
   E_ZONE_EDGE_NONE,
   E_ZONE_EDGE_LEFT,
   E_ZONE_EDGE_RIGHT,
   E_ZONE_EDGE_TOP,
   E_ZONE_EDGE_BOTTOM,
   E_ZONE_EDGE_TOP_LEFT,
   E_ZONE_EDGE_TOP_RIGHT,
   E_ZONE_EDGE_BOTTOM_RIGHT,
   E_ZONE_EDGE_BOTTOM_LEFT
} E_Zone_Edge;

typedef struct _E_Zone                      E_Zone;

typedef struct _E_Event_Zone_Generic        E_Event_Zone_Desk_Count_Set;
typedef struct _E_Event_Zone_Generic        E_Event_Zone_Move_Resize;
typedef struct _E_Event_Zone_Generic        E_Event_Zone_Add;
typedef struct _E_Event_Zone_Generic        E_Event_Zone_Del;
/* TODO: Move this to a general place? */
typedef struct _E_Event_Pointer_Warp        E_Event_Pointer_Warp;
typedef struct _E_Event_Zone_Edge           E_Event_Zone_Edge;
typedef struct _E_Event_Zone_Generic        E_Event_Zone_Stow;
typedef struct _E_Event_Zone_Generic        E_Event_Zone_Unstow;

typedef struct _E_Zone_Obstacle             E_Zone_Obstacle;

#else
#ifndef E_ZONE_H
#define E_ZONE_H

#define E_ZONE_TYPE (int)0xE0b0100d
#define E_ZONE_OBSTACLE_TYPE (int)0xE0b0110d

struct _E_Zone
{
   E_Object     e_obj_inherit;

   int          x, y, w, h;
   const char  *name;
   /* num matches the id of the xinerama screen
    * this zone belongs to. */
   unsigned int num;
   int          fullscreen;

   Evas_Object *bg_object;
   Evas_Object *bg_event_object;
   Evas_Object *bg_clip_object;
   Evas_Object *prev_bg_object;
   Evas_Object *transition_object;

   int          desk_x_count, desk_y_count;
   int          desk_x_current, desk_y_current;
   int          desk_x_prev, desk_y_prev;
   E_Desk     **desks;
   Eina_Inlist *obstacles;

   Eina_List   *handlers;

   /* formerly E_Comp_Zone */
   Evas_Object *base;
   Evas_Object *over;

   E_Backlight_Mode bl_mode;
   double       bl;
   Eina_Bool    bloff;

   struct
   {
      E_Zone_Edge        switching;
      E_Shelf           *es;
      E_Event_Zone_Edge *ev;
      E_Binding_Edge    *bind;
   } flip;

   struct
   {
      Evas_Object *top, *right, *bottom, *left;
   } edge;
   struct
   {
      Evas_Object *left_top, *top_left, *top_right, *right_top,
                  *right_bottom, *bottom_right, *bottom_left, *left_bottom;
   } corner;

   E_Action      *cur_mouse_action;

   int            id;

   void *output;

   Eina_Rectangle useful_geometry[2];
   char *randr2_id; // same id we get from randr2 so look it up there
   Eina_Bool useful_geometry_changed E_BITFIELD;
   Eina_Bool useful_geometry_dirty E_BITFIELD;
   Eina_Bool      stowed E_BITFIELD;
   Eina_Bool desk_flip_sync E_BITFIELD;
};

struct _E_Event_Zone_Generic
{
   E_Zone *zone;
};

struct _E_Event_Pointer_Warp
{
   struct
   {
      int x, y;
   } prev;
   struct
   {
      int x, y;
   } curr;
};

struct _E_Event_Zone_Edge
{
   E_Zone     *zone;
   E_Zone_Edge edge;
   int         x, y;
   int         modifiers;
   int         button;
   Eina_Bool  drag E_BITFIELD;
};

struct _E_Zone_Obstacle
{
   E_Object     e_obj_inherit;
   EINA_INLIST;
   int x, y, w, h;
   E_Object *owner;
   Eina_Bool vertical E_BITFIELD;
};

EINTERN int    e_zone_init(void);
EINTERN int    e_zone_shutdown(void);
E_API E_Zone   *e_zone_new(int num, int id, int x, int y, int w, int h);
E_API void      e_zone_name_set(E_Zone *zone, const char *name);
E_API void      e_zone_move(E_Zone *zone, int x, int y);
E_API void      e_zone_resize(E_Zone *zone, int w, int h);
E_API Eina_Bool  e_zone_move_resize(E_Zone *zone, int x, int y, int w, int h);
E_API E_Zone   *e_zone_current_get(void);
E_API void      e_zone_bg_reconfigure(E_Zone *zone);
E_API void      e_zone_flip_coords_handle(E_Zone *zone, int x, int y);
E_API void      e_zone_desk_count_set(E_Zone *zone, int x_count, int y_count);
E_API void      e_zone_desk_count_get(E_Zone *zone, int *x_count, int *y_count);
E_API void      e_zone_desk_flip_by(E_Zone *zone, int dx, int dy);
E_API void      e_zone_desk_flip_to(E_Zone *zone, int x, int y);
E_API void      e_zone_desk_linear_flip_by(E_Zone *zone, int dx);
E_API void      e_zone_desk_linear_flip_to(E_Zone *zone, int x);
E_API void      e_zone_edge_flip_eval(E_Zone *zone);
E_API void      e_zone_edge_new(E_Zone_Edge edge);
E_API void      e_zone_edge_free(E_Zone_Edge edge);
E_API void      e_zone_edge_enable(void);
E_API void      e_zone_edge_disable(void);
E_API void      e_zone_edges_desk_flip_capable(E_Zone *zone, Eina_Bool l, Eina_Bool r, Eina_Bool t, Eina_Bool b);
E_API Eina_Bool e_zone_exists_direction(E_Zone *zone, E_Zone_Edge edge);
E_API void      e_zone_edge_win_layer_set(E_Zone *zone, E_Layer layer);

E_API void      e_zone_useful_geometry_dirty(E_Zone *zone);
E_API Eina_Bool e_zone_useful_geometry_get(E_Zone *zone, int *x, int *y, int *w, int *h);
E_API void      e_zone_desk_useful_geometry_get(const E_Zone *zone, const E_Desk *desk, int *x, int *y, int *w, int *h);
E_API void      e_zone_stow(E_Zone *zone);
E_API void      e_zone_unstow(E_Zone *zone);

E_API void      e_zone_fade_handle(E_Zone *zone, int out, double tim);

E_API E_Zone_Obstacle *e_zone_obstacle_add(E_Zone *zone, E_Desk *desk, Eina_Rectangle *geom, Eina_Bool vertical);
E_API void e_zone_obstacle_modify(E_Zone_Obstacle *obs, Eina_Rectangle *geom, Eina_Bool vertical);

E_API E_Zone *e_zone_for_id_get(const char *id);

extern E_API int E_EVENT_ZONE_DESK_COUNT_SET;
extern E_API int E_EVENT_ZONE_MOVE_RESIZE;
extern E_API int E_EVENT_ZONE_USEFUL_GEOMETRY_CHANGED;
extern E_API int E_EVENT_ZONE_ADD;
extern E_API int E_EVENT_ZONE_DEL;
extern E_API int E_EVENT_POINTER_WARP;
extern E_API int E_EVENT_ZONE_EDGE_IN;
extern E_API int E_EVENT_ZONE_EDGE_OUT;
extern E_API int E_EVENT_ZONE_EDGE_MOVE;
extern E_API int E_EVENT_ZONE_STOW;
extern E_API int E_EVENT_ZONE_UNSTOW;

#endif
#endif
