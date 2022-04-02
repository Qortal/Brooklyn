#ifdef E_TYPEDEFS

#define E_MENU_POP_DIRECTION_NONE  0
#define E_MENU_POP_DIRECTION_LEFT  1
#define E_MENU_POP_DIRECTION_RIGHT 2
#define E_MENU_POP_DIRECTION_UP    3
#define E_MENU_POP_DIRECTION_DOWN  4
#define E_MENU_POP_DIRECTION_AUTO  5
#define E_MENU_POP_DIRECTION_LAST  6

typedef struct _E_Menu         E_Menu;
typedef struct _E_Menu_Item    E_Menu_Item;
typedef struct _E_Menu_Category_Callback E_Menu_Category_Callback;


#else
#ifndef E_MENU_H
#define E_MENU_H

#define E_MENU_TYPE 0xE0b01009

#define E_MENU_ITEM_TYPE 0xE0b0100a

typedef void (*E_Menu_Cb) (void *data, E_Menu *m, E_Menu_Item *mi);


struct _E_Menu
{
   E_Object             e_obj_inherit;

   const char	       *category;

   struct {
      Eina_Bool         visible E_BITFIELD;
      int               x, y, w, h;
   } cur, prev;

   int                  frozen;

   struct {
      const char       *title;
      const char       *icon_file;
      Evas_Object      *icon;
   } header;

   Eina_List           *items;

   /* the zone it belongs to */
   E_Zone              *zone;

   /* if a menu item spawned this menu, what item is it? */
   E_Menu_Item         *parent_item;

   /* only useful if realized != 0 (ie menu is ACTUALLY realized) */
   Ecore_Job           *dangling_job;
   Evas                 *evas;
   Evas_Object         *comp_object;
   Evas_Object         *bg_object;
   Evas_Object         *container_object;
   Evas_Coord           container_x, container_y, container_w, container_h;

   struct {
      void *data;
      void (*func) (void *data, E_Menu *m);
   } pre_activate_cb, post_deactivate_cb;

   Eina_Bool        realized E_BITFIELD; /* 1 if it is realized */
   Eina_Bool        active E_BITFIELD; /* 1 if it is in active list */
   Eina_Bool        changed E_BITFIELD;
   Eina_Bool        fast_mouse E_BITFIELD;
   Eina_Bool        pending_new_submenu E_BITFIELD;
   Eina_Bool        have_submenu E_BITFIELD;
   Eina_Bool        in_active_list E_BITFIELD;
   Eina_Bool        hold_mode E_BITFIELD;
};

struct _E_Menu_Item
{
   E_Object       e_obj_inherit;
   E_Menu        *menu;
   const char    *icon;
   const char    *icon_key;
   const char    *label;
   E_Menu        *submenu;

   Evas_Object   *separator_object;

   Evas_Object   *bg_object;

   Evas_Object   *container_object;

   Evas_Object   *toggle_object;
   Evas_Object   *icon_bg_object;
   Evas_Object   *icon_object;
   Evas_Object   *label_object;
   Evas_Object   *submenu_object;

   Eina_List	 *list_position;

   int            label_w, label_h;
   int            icon_w, icon_h;
   int            separator_w, separator_h;
   int            submenu_w, submenu_h;
   int            toggle_w, toggle_h;
   int            radio_group;
   int            x, y, w, h;

   struct {
      void *data;
      E_Menu_Cb func;
   } cb; /* Callback for menu item activation */

   struct {
      void *data;
      E_Menu_Cb func;
   } realize_cb; /* Callback for menu item icon realization */

    struct {
      void *data;
      E_Menu_Cb func;
   } submenu_pre_cb;

    struct {
      void *data;
      E_Menu_Cb func;
   } submenu_post_cb;

   struct {
      void *data;
      E_Menu_Cb func;
   } drag_cb; /* Callback for menu item dragging */

   struct {
      int x, y;
   } drag;

   Eina_Bool  separator E_BITFIELD;
   Eina_Bool  radio E_BITFIELD;
   Eina_Bool  check E_BITFIELD;
   Eina_Bool  toggle E_BITFIELD;
   Eina_Bool  changed E_BITFIELD;
   Eina_Bool  active E_BITFIELD;
   Eina_Bool  disable E_BITFIELD;
   Eina_Bool  hilighted E_BITFIELD;
};

struct _E_Menu_Category_Callback
{
   const char *category;
   void *data;
   void (*create) (void *data, E_Menu *m, void *category_data);
   Ecore_Cb free;
};


EINTERN int          e_menu_init(void);
EINTERN int          e_menu_shutdown(void);

E_API void         e_menu_hide_all(void);
E_API E_Menu      *e_menu_new(void);
E_API void         e_menu_hold_mode_set(E_Menu *m, Eina_Bool hold_mode);
E_API void         e_menu_activate_key(E_Menu *m, E_Zone *zone, int x, int y, int w, int h, int dir);
E_API void         e_menu_activate_mouse(E_Menu *m, E_Zone *zone, int x, int y, int w, int h, int dir, unsigned int activate_time);
E_API void         e_menu_activate(E_Menu *m, E_Zone *zone, int x, int y, int w, int h, int dir);
E_API void         e_menu_deactivate(E_Menu *m);
E_API int          e_menu_freeze(E_Menu *m);
E_API int          e_menu_thaw(E_Menu *m);
E_API void         e_menu_title_set(E_Menu *m, const char *title);
E_API void         e_menu_icon_file_set(E_Menu *m, const char *icon);

/* menu categories functions */
E_API void         e_menu_category_set(E_Menu *m, const char *category);
E_API void         e_menu_category_data_set(char *category, void *data);
E_API E_Menu_Category_Callback  *e_menu_category_callback_add(char *category, void (*create_cb) (void *data, E_Menu *m, void *category_data), Ecore_Cb free_cb, void *data);
E_API void         e_menu_category_callback_del(E_Menu_Category_Callback *cb);


E_API void         e_menu_pre_activate_callback_set(E_Menu *m,  void (*func) (void *data, E_Menu *m), void *data);
E_API void         e_menu_post_deactivate_callback_set(E_Menu *m,  void (*func) (void *data, E_Menu *m), void *data);

E_API E_Menu      *e_menu_root_get(E_Menu *m);

E_API E_Menu_Item *e_menu_item_new(E_Menu *m);
E_API E_Menu_Item *e_menu_item_new_relative(E_Menu *m, E_Menu_Item *rel);
E_API E_Menu_Item *e_menu_item_nth(E_Menu *m, int n);
E_API int          e_menu_item_num_get(const E_Menu_Item *mi);
E_API void         e_menu_item_icon_file_set(E_Menu_Item *mi, const char *icon);
E_API void         e_menu_item_icon_edje_set(E_Menu_Item *mi, const char *icon, const char *key);
E_API void         e_menu_item_label_set(E_Menu_Item *mi, const char *label);
E_API void         e_menu_item_submenu_set(E_Menu_Item *mi, E_Menu *sub);
E_API void         e_menu_item_separator_set(E_Menu_Item *mi, int sep);
E_API void         e_menu_item_check_set(E_Menu_Item *mi, int chk);
E_API void         e_menu_item_radio_set(E_Menu_Item *mi, int rad);
E_API void         e_menu_item_radio_group_set(E_Menu_Item *mi, int radg);
E_API void         e_menu_item_toggle_set(E_Menu_Item *mi, int tog);
E_API int          e_menu_item_toggle_get(E_Menu_Item *mi);
E_API void         e_menu_item_callback_set(E_Menu_Item *mi,  E_Menu_Cb func, const void *data);
E_API void         e_menu_item_realize_callback_set(E_Menu_Item *mi,  E_Menu_Cb func, void *data);
E_API void         e_menu_item_submenu_pre_callback_set(E_Menu_Item *mi,  E_Menu_Cb func, const void *data);
E_API void         e_menu_item_submenu_post_callback_set(E_Menu_Item *mi,  E_Menu_Cb func, const void *data);
E_API void         e_menu_item_drag_callback_set(E_Menu_Item *mi,  E_Menu_Cb func, void *data);
E_API E_Menu_Item *e_menu_item_active_get(void);
E_API void         e_menu_active_item_activate(void);
E_API void         e_menu_item_active_set(E_Menu_Item *mi, int active);
E_API void         e_menu_item_disabled_set(E_Menu_Item *mi, int disable);

E_API void         e_menu_idler_before(void);

E_API Eina_Bool    e_menu_is_active(void);
E_API E_Menu      *e_menu_active_get(void);

#endif
#endif
