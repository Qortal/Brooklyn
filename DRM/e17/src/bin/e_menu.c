#include "e.h"

/* TODO List:
 *
 * * support change of menu items after realize
 * * support add/del of menu items after realize
 * * support text/color classes
 * * refcount menu up while looping thru and calling other fn's
 * * support alignment (x, y) as well as spawn direction
 * * need different menu style support for different menus
 * * add menu icon/title support
 * * use event timestamps not clock for "click and release" detect
 * * menu icons can set if/how they will be scaled
 * * support move/resize of "box" that spawned the menu
 * * add image item (label is replaced by image/icon)
 * * add generic evas object item type (label replaced by object)
 * * allow menus to stretch width/height to fit spawner widget/box
 * * allow menus to auto-shrink (horizontally) if forced to
 * * support auto left/right direction spawn
 * * support obscures to indicate offscreen/not visible menu parts
 */

/* local subsystem data types */
typedef struct _E_Menu_Category E_Menu_Category;

struct _E_Menu_Category
{
   void      *data;
   Eina_List *callbacks;
};

/* local subsystem functions */
static void         _e_menu_free(E_Menu *m);
static void         _e_menu_item_free(E_Menu_Item *mi);
static void         _e_menu_item_realize(E_Menu_Item *mi);
static void         _e_menu_realize(E_Menu *m);
static void         _e_menu_items_layout_update(E_Menu *m);
static void         _e_menu_item_unrealize(E_Menu_Item *mi);
static void         _e_menu_unrealize(E_Menu *m);
static void         _e_menu_activate_internal(E_Menu *m, E_Zone *zone);
static void         _e_menu_deactivate_all(void);
static void         _e_menu_deactivate_above(E_Menu *m);
static void         _e_menu_submenu_activate(E_Menu_Item *mi);
static void         _e_menu_submenu_deactivate(E_Menu_Item *mi);
static void         _e_menu_reposition(E_Menu *m);
static int          _e_menu_active_call(void);
static int          _e_menu_realize_call(E_Menu_Item *mi);
static void         _e_menu_item_activate_next(void);
static void         _e_menu_item_activate_previous(void);
static void         _e_menu_item_activate_first(void);
static void         _e_menu_item_activate_last(void);
static void         _e_menu_item_activate_nth(int n);
static void         _e_menu_item_activate_char(const char *key_compose);
static void         _e_menu_activate_next(void);
static void         _e_menu_activate_previous(void);
static void         _e_menu_activate_first(void);
static void         _e_menu_activate_last(void);
#if 0
static void         _e_menu_activate_nth(int n);
#endif
static E_Menu      *_e_menu_active_get(void);
static E_Menu_Item *_e_menu_item_active_get(void);
static Eina_List   *_e_menu_list_item_active_get(void);
static int          _e_menu_outside_bounds_get(int xdir, int ydir);
static void         _e_menu_scroll_by(int dx, int dy);
static void         _e_menu_mouse_autoscroll_check(void);
static void         _e_menu_item_ensure_onscreen(E_Menu_Item *mi);
static int          _e_menu_auto_place(E_Menu *m, int x, int y, int w, int h);
static void         _e_menu_cb_intercept_item_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y);
static void         _e_menu_cb_intercept_item_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h);
static void         _e_menu_cb_intercept_container_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y);
static void         _e_menu_cb_intercept_container_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h);
static void         _e_menu_cb_item_in(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void         _e_menu_cb_item_out(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static Eina_Bool    _e_menu_cb_key_down(void *data EINA_UNUSED, Ecore_Event_Key *ev);
static Eina_Bool    _e_menu_cb_mouse_down(void *data, int type, void *event);
static Eina_Bool    _e_menu_cb_mouse_up(void *data, int type, void *event);
static Eina_Bool    _e_menu_cb_mouse_move(void *data, int type, void *event);
static Eina_Bool    _e_menu_cb_mouse_wheel(void *data, int type, void *event);
static Eina_Bool    _e_menu_cb_scroll_animator(void *data);
static void         _e_menu_cb_item_submenu_post_default(void *data, E_Menu *m, E_Menu_Item *mi);
static void         _e_menu_category_free_cb(E_Menu_Category *cat);
static void         _e_menu_cb_mouse_evas_down(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED);
static void         _e_menu_hide_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);

typedef enum
{
   NAV_BY_NONE,
   NAV_BY_MOUSE,
   NAV_BY_WHEEL,
   NAV_BY_KEY
} Nav_By;

/* local subsystem globals */
static Ecore_Window _e_menu_win = UINT_MAX;
static Eina_Bool _e_menu_grabbed = EINA_FALSE;
static Eina_List *_e_active_menus = NULL;
static E_Menu_Item *_e_active_menu_item = NULL;
static E_Menu_Item *_e_prev_active_menu_item = NULL;
/*static Eina_Hash	   *_e_menu_category_items	= NULL;*/
static Eina_Hash *_e_menu_categories = NULL;
static unsigned int _e_menu_activate_time = 0;
static int _e_menu_activate_floating = 0;
static int _e_menu_activate_maybe_drag = 0;
static int _e_menu_activate_dragging = 0;
static Ecore_Animator *_e_menu_scroll_animator = NULL;
static double _e_menu_scroll_start = 0.0;
static int _e_menu_x = 0;
static int _e_menu_y = 0;
static unsigned int _e_menu_time = 0;
static int _e_menu_autoscroll_x = 0;
static int _e_menu_autoscroll_y = 0;
static Eina_List *handlers = NULL;
static Eina_Bool _e_menu_lock = EINA_FALSE;
static Nav_By _e_menu_nav_by = NAV_BY_NONE;

static Eina_Bool pending_feed;
static unsigned int pending_activate_time;

static Eina_List *
_e_active_menus_copy_ref(void)
{
   E_Object *o;
   Eina_List *l, *ret = NULL;

   EINA_LIST_FOREACH(_e_active_menus, l, o)
     {
        ret = eina_list_append(ret, o);
        e_object_ref(o);
     }
   return ret;
}

static Eina_List *
_e_menu_list_free_unref(Eina_List *l)
{
   E_Object *o;
   Eina_List *ll, *lll;

   /* list must be freed in reverse to ensure that submenus
    * (which are added to the end of the list)
    * are deleted before their parents
    */
   EINA_LIST_REVERSE_FOREACH_SAFE(l, ll, lll, o)
   {
      e_object_unref(o);
      l = eina_list_remove_list(l, ll);
   }
   return l;
}

/* macros for debugging menu refcounts */
#if 0
#define e_object_ref(X)   do {      \
       int xx;                      \
       xx = e_object_ref(X);        \
       INF("REF: %p || %d", X, xx); \
  } while (0)
#define e_object_unref(X) do {        \
       int xx;                        \
       xx = e_object_unref(X);        \
       INF("UNREF: %p || %d", X, xx); \
  } while (0)
#endif

/* externally accessible functions */
EINTERN int
e_menu_init(void)
{
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_MOUSE_BUTTON_DOWN, _e_menu_cb_mouse_down, NULL);
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_MOUSE_BUTTON_UP, _e_menu_cb_mouse_up, NULL);
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_MOUSE_MOVE, _e_menu_cb_mouse_move, NULL);
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_MOUSE_WHEEL, _e_menu_cb_mouse_wheel, NULL);
   _e_menu_categories = eina_hash_string_superfast_new((Eina_Free_Cb)_e_menu_category_free_cb);

   e_int_menus_init();
   return 1;
}

EINTERN int
e_menu_shutdown(void)
{
   E_FREE_LIST(handlers, ecore_event_handler_del);

   if (!x_fatal)
     e_menu_hide_all();
   _e_active_menus = NULL;
   E_FREE_FUNC(_e_menu_categories, eina_hash_free);

   _e_menu_lock = EINA_FALSE;
   e_int_menus_shutdown();

   return 1;
}

E_API void
e_menu_hide_all(void)
{
   E_Menu *m;

   EINA_LIST_FREE(_e_active_menus, m)
     {
        if (m->post_deactivate_cb.func)
          m->post_deactivate_cb.func(m->post_deactivate_cb.data, m);
        m->active = 0;
        if (m->comp_object == e_comp->autoclose.obj)
          e_comp_object_util_autoclose(NULL, NULL, NULL, NULL);
        _e_menu_unrealize(m);
        m->in_active_list = 0;
        e_object_unref(E_OBJECT(m));
     }
   if (_e_menu_grabbed)
     {
        e_comp_ungrab_input(1, 1);
        _e_menu_grabbed = EINA_FALSE;
     }
}

E_API E_Menu *
e_menu_new(void)
{
   E_Menu *m;

   m = E_OBJECT_ALLOC(E_Menu, E_MENU_TYPE, _e_menu_free);
   if (!m) return NULL;
   m->cur.w = 1;
   m->cur.h = 1;
   m->category = NULL;
   m->hold_mode = EINA_TRUE;
   return m;
}

E_API void
e_menu_hold_mode_set(E_Menu *m, Eina_Bool hold_mode)
{
   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   m->hold_mode = hold_mode;
}

E_API void
e_menu_activate_key(E_Menu *m, E_Zone *zone, int x, int y, int w, int h, int dir)
{
   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   if (_e_active_menus) e_menu_hide_all();
   _e_menu_activate_time = 0;
   _e_menu_activate_floating = 0;
   _e_menu_activate_internal(m, zone);
   if (!m->zone)
     {
        e_menu_deactivate(m);
        return;
     }
   _e_menu_nav_by = NAV_BY_KEY;
   switch (dir)
     {
      case E_MENU_POP_DIRECTION_LEFT:
        _e_menu_realize(m);
        m->cur.x = x - m->cur.w;
        m->cur.y = y;
        if ((m->cur.y + m->cur.h) > (m->zone->y + m->zone->h))
          m->cur.y = y + h - m->cur.h;
        e_menu_idler_before();
        _e_menu_activate_first();
        break;

      case E_MENU_POP_DIRECTION_RIGHT:
        _e_menu_realize(m);
        m->cur.x = x + w;
        m->cur.y = y;
        e_menu_idler_before();
        _e_menu_activate_first();
        break;

      case E_MENU_POP_DIRECTION_UP:
        _e_menu_realize(m);
        m->cur.x = x;
        if ((m->cur.x + m->cur.w) > (m->zone->x + m->zone->w))
          m->cur.x = x + w - m->cur.w;
        m->cur.y = y - m->cur.h;
        e_menu_idler_before();
        _e_menu_activate_last();
        break;

      case E_MENU_POP_DIRECTION_DOWN:
        _e_menu_realize(m);
        m->cur.x = x;
        if ((m->cur.x + m->cur.w) > (m->zone->x + m->zone->w))
          m->cur.x = x + w - m->cur.w;
        m->cur.y = y + h;
        if ((m->cur.y + m->cur.h) > (m->zone->y + m->zone->h))
          m->cur.y = y + h - m->cur.h;
        e_menu_idler_before();
        _e_menu_activate_first();
        break;

      case E_MENU_POP_DIRECTION_AUTO:
          {
             int pos = 0;

             pos = _e_menu_auto_place(m, x, y, w, h);
             e_menu_idler_before();
             if (pos == 4) _e_menu_activate_last();
             else _e_menu_activate_first();
          }
      break;

      default:
        m->cur.x = x + w;
        m->cur.y = y + h;
        e_menu_idler_before();
        _e_menu_activate_first();
        break;
     }
}

E_API void
e_menu_activate_mouse(E_Menu *m, E_Zone *zone, int x, int y, int w, int h, int dir, unsigned int activate_time)
{
   E_Menu_Item *pmi;

   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);

   e_object_ref(E_OBJECT(m));
   e_menu_hide_all();
   _e_menu_activate_time = 0;
   _e_menu_activate_floating = 0;
   _e_menu_activate_internal(m, zone);
   if (!m->zone)
     {
        e_menu_deactivate(m);
        e_object_unref(E_OBJECT(m));
        return;
     }
   _e_menu_nav_by = NAV_BY_MOUSE;
   switch (dir)
     {
      case E_MENU_POP_DIRECTION_LEFT:
        _e_menu_realize(m);
        m->cur.x = x - m->cur.w;
        m->cur.y = y;
        if ((m->cur.y + m->cur.h) > (m->zone->y + m->zone->h))
          m->cur.y = y + h - m->cur.h;
        e_menu_idler_before();
        break;

      case E_MENU_POP_DIRECTION_RIGHT:
        _e_menu_realize(m);
        m->cur.x = x + w;
        m->cur.y = y;
        e_menu_idler_before();
        break;

      case E_MENU_POP_DIRECTION_UP:
        _e_menu_realize(m);
        m->cur.x = x;
        if ((m->cur.x + m->cur.w) > (m->zone->x + m->zone->w))
          m->cur.x = x + w - m->cur.w;
        m->cur.y = y - m->cur.h;
        e_menu_idler_before();
        break;

      case E_MENU_POP_DIRECTION_DOWN:
        _e_menu_realize(m);
        m->cur.x = x;
        if ((m->cur.x + m->cur.w) > (m->zone->x + m->zone->w))
          m->cur.x = x + w - m->cur.w;
        m->cur.y = y + h;
        if ((m->cur.y + m->cur.h) > (m->zone->y + m->zone->h))
          m->cur.y = y + h - m->cur.h;
        e_menu_idler_before();
        break;

      case E_MENU_POP_DIRECTION_AUTO:
        e_menu_idler_before();
        _e_menu_auto_place(m, x, y, w, h);
        break;

      default:
        m->cur.x = x + w;
        m->cur.y = y + h;
        e_menu_idler_before();
        break;
     }
   pmi = _e_menu_item_active_get();
   if (pmi) e_menu_item_active_set(pmi, 0);
   pending_feed = 1;
   if (!activate_time) activate_time = lround(ecore_loop_time_get() * 1000);
   _e_menu_activate_time = pending_activate_time = activate_time;
   e_object_unref(E_OBJECT(m));
}

E_API void
e_menu_activate(E_Menu *m, E_Zone *zone, int x, int y, int w, int h, int dir)
{
   E_Menu_Item *pmi;

   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   E_OBJECT_CHECK(zone);
   E_OBJECT_TYPE_CHECK(zone, E_ZONE_TYPE);
   if (_e_active_menus) e_menu_hide_all();
   _e_menu_activate_time = 0;
   _e_menu_activate_floating = 0;
   _e_menu_activate_internal(m, zone);
   if (!m->zone)
     {
        e_menu_deactivate(m);
        return;
     }
   _e_menu_nav_by = NAV_BY_NONE;
   switch (dir)
     {
      case E_MENU_POP_DIRECTION_LEFT:
        _e_menu_realize(m);
        m->cur.x = x - m->cur.w;
        m->cur.y = y;
        if ((m->cur.y + m->cur.h) > (m->zone->y + m->zone->h))
          m->cur.y = y + h - m->cur.h;
        e_menu_idler_before();
        _e_menu_activate_first();
        break;

      case E_MENU_POP_DIRECTION_RIGHT:
        _e_menu_realize(m);
        m->cur.x = x + w;
        m->cur.y = y;
        e_menu_idler_before();
        _e_menu_activate_first();
        break;

      case E_MENU_POP_DIRECTION_UP:
        _e_menu_realize(m);
        m->cur.x = x;
        if ((m->cur.x + m->cur.w) > (m->zone->x + m->zone->w))
          m->cur.x = x + w - m->cur.w;
        m->cur.y = y - m->cur.h;
        e_menu_idler_before();
        _e_menu_activate_last();
        break;

      case E_MENU_POP_DIRECTION_DOWN:
        _e_menu_realize(m);
        m->cur.x = x;
        if ((m->cur.x + m->cur.w) > (m->zone->x + m->zone->w))
          m->cur.x = x + w - m->cur.w;
        m->cur.y = y + h;
        if ((m->cur.y + m->cur.h) > (m->zone->y + m->zone->h))
          m->cur.y = y + h - m->cur.h;
        e_menu_idler_before();
        _e_menu_activate_first();
        break;

      case E_MENU_POP_DIRECTION_AUTO:
        e_menu_idler_before();
        _e_menu_auto_place(m, x, y, w, h);
        break;

      default:
        m->cur.x = x + w;
        m->cur.y = y + h;
        e_menu_idler_before();
        break;
     }
   pmi = _e_menu_item_active_get();
   if (pmi) e_menu_item_active_set(pmi, 0);
}

static void
_e_menu_item_unhilight(E_Menu_Item *mi)
{
   if (!mi->hilighted) return;
   mi->hilighted = 0;
   if (mi->bg_object)
     edje_object_signal_emit(mi->bg_object, "e,state,unselected", "e");
   if (mi->icon_bg_object)
     edje_object_signal_emit(mi->icon_bg_object, "e,state,unselected", "e");
   if (isedje(mi->label_object))
     edje_object_signal_emit(mi->label_object, "e,state,unselected", "e");
   if (isedje(mi->submenu_object))
     edje_object_signal_emit(mi->submenu_object, "e,state,unselected", "e");
   if (isedje(mi->toggle_object))
     edje_object_signal_emit(mi->toggle_object, "e,state,unselected", "e");
   if (mi->icon_key)
     {
        if (mi->icon_object)
          {
             if (isedje(mi->icon_object))
               edje_object_signal_emit(mi->icon_object, "e,state,unselected", "e");
             else
               e_icon_selected_set(mi->icon_object, EINA_FALSE);
          }
     }
   edje_object_signal_emit(mi->menu->bg_object, "e,state,unselected", "e");
}

E_API void
e_menu_deactivate(E_Menu *m)
{
   Eina_List *l;
   E_Menu_Item *mi;

   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   if (!m->active) return;

   EINA_LIST_FOREACH(m->items, l, mi)
     {
        if (mi->submenu) e_menu_deactivate(mi->submenu);
     }

   m->cur.visible = 0;
   m->active = 0;
   if (m->post_deactivate_cb.func)
     m->post_deactivate_cb.func(m->post_deactivate_cb.data, m);
   if (m->parent_item)
     _e_menu_item_unhilight(m->parent_item);
}

E_API int
e_menu_freeze(E_Menu *m)
{
   E_OBJECT_CHECK_RETURN(m, 0);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MENU_TYPE, 0);
   m->frozen++;
   return m->frozen;
}

E_API int
e_menu_thaw(E_Menu *m)
{
   E_OBJECT_CHECK_RETURN(m, 0);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MENU_TYPE, 0);
   m->frozen--;
   if (m->frozen < 0) m->frozen = 0;
   return m->frozen;
}

E_API void
e_menu_title_set(E_Menu *m, const char *title)
{
   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   if ((m->header.title) && (title) && (!strcmp(m->header.title, title)))
     return;
   if (m->header.title)
     {
        eina_stringshare_del(m->header.title);
        m->header.title = NULL;
     }
   if (title) m->header.title = eina_stringshare_add(title);
   else m->header.title = NULL;
   m->changed = 1;
   if (!m->realized) return;
   edje_object_part_text_set(m->bg_object, "e.text.title", m->header.title);
   if (m->header.title)
     edje_object_signal_emit(m->bg_object, "e,action,show,title", "e");
   else
     edje_object_signal_emit(m->bg_object, "e,action,hide,title", "e");
   edje_object_message_signal_process(m->bg_object);
}

E_API void
e_menu_icon_file_set(E_Menu *m EINA_UNUSED, const char *icon EINA_UNUSED)
{
   /* FIXME: support menu icons
      E_OBJECT_CHECK(m);
      E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
    */
}

E_API void
e_menu_category_set(E_Menu *m, const char *category)
{
   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   if (m->category)
     {
        eina_stringshare_del(m->category);
        m->category = NULL;
     }
   if (category)
     m->category = eina_stringshare_add(category);
   else
     m->category = NULL;
   m->changed = 1;
}

E_API void
e_menu_category_data_set(char *category, void *data)
{
   E_Menu_Category *cat;

   cat = eina_hash_find(_e_menu_categories, category);
   if (cat)
     cat->data = data;
   else   /* if it isn't found create the new hash */
     {
        cat = calloc(1, sizeof(E_Menu_Category));
        cat->data = data;
        eina_hash_add(_e_menu_categories, category, cat);
     }
}

E_API E_Menu_Category_Callback *
e_menu_category_callback_add(char *category, void (*create_cb)(void *data, E_Menu *m, void *category_data), Ecore_Cb free_cb, void *data)
{
   E_Menu_Category *cat;
   E_Menu_Category_Callback *cb = NULL;

   cat = eina_hash_find(_e_menu_categories, category);
   if (!cat)   /* if it isn't found create the new hash */
     {
        cat = calloc(1, sizeof(E_Menu_Category));
        eina_hash_add(_e_menu_categories, category, cat);
     }
   if (cat)
     {
        cb = calloc(1, sizeof(E_Menu_Category_Callback));
        if (cb)
          {
             cb->data = data;
             cb->create = create_cb;
             cb->free = free_cb;
             cb->category = eina_stringshare_add(category);
             cat->callbacks = eina_list_append(cat->callbacks, cb);
          }
     }
   return cb;
}

E_API void
e_menu_category_callback_del(E_Menu_Category_Callback *cb)
{
   E_Menu_Category *cat;

   if (cb)
     {
        cat = eina_hash_find(_e_menu_categories, cb->category);
        if (cat)
          cat->callbacks = eina_list_remove(cat->callbacks, cb);
        eina_stringshare_del(cb->category);
        free(cb);
     }
}

E_API void
e_menu_pre_activate_callback_set(E_Menu *m, void (*func)(void *data, E_Menu *m), void *data)
{
   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   m->pre_activate_cb.func = func;
   m->pre_activate_cb.data = data;
}

E_API void
e_menu_post_deactivate_callback_set(E_Menu *m, void (*func)(void *data, E_Menu *m), void *data)
{
   E_OBJECT_CHECK(m);
   E_OBJECT_TYPE_CHECK(m, E_MENU_TYPE);
   m->post_deactivate_cb.func = func;
   m->post_deactivate_cb.data = data;
}

E_API E_Menu *
e_menu_root_get(E_Menu *m)
{
   E_Menu *ret;

   E_OBJECT_CHECK_RETURN(m, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MENU_TYPE, NULL);
   ret = m;
   while ((ret->parent_item) && (ret->parent_item->menu))
     ret = ret->parent_item->menu;

   return ret;
}

E_API E_Menu_Item *
e_menu_item_new(E_Menu *m)
{
   E_Menu_Item *mi;

   E_OBJECT_CHECK_RETURN(m, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MENU_TYPE, NULL);
   mi = E_OBJECT_ALLOC(E_Menu_Item, E_MENU_ITEM_TYPE, _e_menu_item_free);
   mi->menu = m;
   mi->menu->items = eina_list_append(mi->menu->items, mi);
   mi->list_position = eina_list_last(mi->menu->items);
   return mi;
}

E_API E_Menu_Item *
e_menu_item_new_relative(E_Menu *m, E_Menu_Item *rel)
{
   E_Menu_Item *mi;
   E_OBJECT_CHECK_RETURN(m, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MENU_TYPE, NULL);
   if (rel)
     {
        E_OBJECT_CHECK_RETURN(rel, NULL);
        E_OBJECT_TYPE_CHECK_RETURN(rel, E_MENU_ITEM_TYPE, NULL);
        if (rel->menu != m) return NULL;
     }

   mi = E_OBJECT_ALLOC(E_Menu_Item, E_MENU_ITEM_TYPE, _e_menu_item_free);
   mi->menu = m;

   if (rel)
     {
        Eina_List *l;

        l = eina_list_data_find_list(m->items, rel);
        m->items = eina_list_append_relative_list(m->items, mi, l);
        mi->list_position = l->next;
     }
   else
     {
        m->items = eina_list_prepend(m->items, mi);
        mi->list_position = m->items;
     }

   return mi;
}

E_API E_Menu_Item *
e_menu_item_nth(E_Menu *m, int n)
{
   E_OBJECT_CHECK_RETURN(m, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(m, E_MENU_TYPE, NULL);
   return (E_Menu_Item *)eina_list_nth(m->items, n);
}

E_API int
e_menu_item_num_get(const E_Menu_Item *mi)
{
   const Eina_List *l;
   const E_Menu_Item *mi2;
   int i = 0;

   E_OBJECT_CHECK_RETURN(mi, -1);
   E_OBJECT_CHECK_RETURN(mi->menu, -1);
   E_OBJECT_TYPE_CHECK_RETURN(mi, E_MENU_TYPE, -1);
   EINA_LIST_FOREACH(mi->menu->items, l, mi2)
     {
        if (mi2 == mi) return i;
        i++;
     }
   return -1;
}

E_API void
e_menu_item_icon_file_set(E_Menu_Item *mi, const char *icon)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (((mi->icon) && (icon) && (!strcmp(icon, mi->icon))) ||
       ((!mi->icon) && (!icon)))
     return;
   if (mi->icon) eina_stringshare_del(mi->icon);
   if (mi->icon_key) eina_stringshare_del(mi->icon_key);
   mi->icon = NULL;
   mi->icon_key = NULL;
   if (icon)
     {
        mi->icon = eina_stringshare_add(icon);
        if (eina_str_has_extension(mi->icon, ".edj"))
          mi->icon_key = eina_stringshare_add("icon");
     }
   mi->changed = 1;
   mi->menu->changed = 1;
}

E_API void
e_menu_item_icon_edje_set(E_Menu_Item *mi, const char *icon, const char *key)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (((mi->icon) && (icon) && (!strcmp(icon, mi->icon))) ||
       ((!mi->icon) && (!icon)) ||
       ((key) && (mi->icon_key) && (!strcmp(key, mi->icon_key))))
     return;
   eina_stringshare_replace(&mi->icon, icon);
   eina_stringshare_replace(&mi->icon_key, key);
   mi->changed = 1;
   mi->menu->changed = 1;
}

E_API void
e_menu_item_label_set(E_Menu_Item *mi, const char *label)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (((mi->label) && (label) && (!strcmp(label, mi->label))) ||
       ((!mi->label) && (!label)))
     return;
   if (mi->label) eina_stringshare_del(mi->label);
   mi->label = NULL;
   if (label) mi->label = eina_stringshare_add(label);
   mi->changed = 1;
   mi->menu->changed = 1;
}

E_API void
e_menu_item_submenu_set(E_Menu_Item *mi, E_Menu *sub)
{
   Eina_Bool submenu = EINA_FALSE;
   Evas_Object *o;
   Eina_List *tmp = NULL;
   int ww, hh;
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);

   if (mi->submenu == sub) return;
   tmp = _e_active_menus_copy_ref();
   submenu = !!mi->submenu;
   if (mi->submenu) e_object_unref(E_OBJECT(mi->submenu));
   if (sub) e_object_ref(E_OBJECT(sub));
   mi->submenu = sub;
   mi->changed = 1;
   mi->menu->changed = 1;
   if (!!sub == submenu) goto out;
   if (!mi->bg_object) goto out;
   if (sub) e_object_ref(E_OBJECT(sub));
   _e_menu_lock = EINA_TRUE;
   if ((mi->submenu) || (mi->submenu_pre_cb.func))
     {
        if (mi->submenu_object)
          {
             if (isedje(mi->submenu_object))
               {
                  /* already have a correct submenu object, don't re-set it */
                  _e_menu_lock = EINA_FALSE;
                  if (sub) e_object_unref(E_OBJECT(sub));
                  edje_object_size_min_calc(mi->submenu_object, &ww, &hh);
                  mi->submenu_w = ww;
                  mi->submenu_h = hh;
                  E_WEIGHT(mi->submenu_object, 0, 1);
                  E_FILL(mi->submenu_object);
                  evas_object_size_hint_min_set(mi->submenu_object, ww, hh);
                  goto out;
               }
             evas_object_del(mi->submenu_object);
          }
        o = edje_object_add(mi->menu->evas);
        if (sub && (mi->submenu != sub)) e_object_ref(E_OBJECT(sub));
        mi->submenu = sub;
        mi->submenu_object = o;
        e_theme_edje_object_set(o, "base/theme/menus",
                                "e/widgets/menu/default/submenu");
        evas_object_pass_events_set(o, 1);
        elm_box_pack_end(mi->container_object, o);
        evas_object_show(o);
        edje_object_size_min_calc(mi->submenu_object, &ww, &hh);
        mi->submenu_w = ww;
        mi->submenu_h = hh;
        E_WEIGHT(mi->submenu_object, 0, 1);
        E_FILL(mi->submenu_object);
        evas_object_size_hint_min_set(mi->submenu_object, ww, hh);
        edje_object_part_swallow(mi->bg_object, "e.swallow.content",
                                 mi->container_object);
        edje_object_size_min_calc(mi->bg_object, &ww, &hh);
        E_WEIGHT(mi->bg_object, 1, 0);
        E_FILL(mi->bg_object);
        evas_object_size_hint_min_set(mi->bg_object, ww, hh);
     }
   else
     {
        if (mi->submenu_object) evas_object_del(mi->submenu_object);
        o = evas_object_rectangle_add(mi->menu->evas);
        mi->submenu_object = o;
        evas_object_color_set(o, 0, 0, 0, 0);
        evas_object_pass_events_set(o, 1);
        elm_box_pack_end(mi->container_object, o);
     }
   _e_menu_lock = EINA_FALSE;
   if (sub) e_object_unref(E_OBJECT(sub));
   if ((mi->submenu) || (mi->submenu_pre_cb.func))
     {
        if (e_theme_edje_object_set(mi->bg_object, "base/theme/menus",
                                    "e/widgets/menu/default/submenu_bg"))
          goto out;
     }

   e_theme_edje_object_set(mi->bg_object, "base/theme/menus",
                           "e/widgets/menu/default/item_bg");
out:
   _e_menu_list_free_unref(tmp);
}

E_API void
e_menu_item_separator_set(E_Menu_Item *mi, int sep)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (((mi->separator) && (sep)) || ((!mi->separator) && (!sep))) return;
   mi->separator = sep;
   mi->changed = 1;
   mi->menu->changed = 1;
}

E_API void
e_menu_item_check_set(E_Menu_Item *mi, int chk)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (((mi->check) && (chk)) || ((!mi->check) && (!chk))) return;
   mi->check = chk;
   mi->changed = 1;
   mi->menu->changed = 1;
}

E_API void
e_menu_item_radio_set(E_Menu_Item *mi, int rad)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (((mi->radio) && (rad)) || ((!mi->radio) && (!rad))) return;
   mi->radio = rad;
   mi->changed = 1;
   mi->menu->changed = 1;
}

E_API void
e_menu_item_radio_group_set(E_Menu_Item *mi, int radg)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (mi->radio_group == radg) return;
   mi->radio_group = radg;
   mi->changed = 1;
   mi->menu->changed = 1;
}

E_API void
e_menu_item_toggle_set(E_Menu_Item *mi, int tog)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (mi->separator) return;
   if (tog)
     {
        mi->toggle = 1;
        if (mi->bg_object)
          edje_object_signal_emit(mi->bg_object, "e,state,on", "e");
        if (mi->icon_bg_object)
          edje_object_signal_emit(mi->icon_bg_object, "e,state,on", "e");
        if (isedje(mi->label_object))
          edje_object_signal_emit(mi->label_object, "e,state,on", "e");
        if (isedje(mi->submenu_object))
          edje_object_signal_emit(mi->submenu_object, "e,state,on", "e");
        if (isedje(mi->toggle_object))
          edje_object_signal_emit(mi->toggle_object, "e,state,on", "e");
        if (mi->menu->bg_object)
          edje_object_signal_emit(mi->menu->bg_object, "e,state,on", "e");
     }
   else
     {
        mi->toggle = 0;
        if (mi->bg_object)
          edje_object_signal_emit(mi->bg_object, "e,state,off", "e");
        if (mi->icon_bg_object)
          edje_object_signal_emit(mi->icon_bg_object, "e,state,off", "e");
        if (isedje(mi->label_object))
          edje_object_signal_emit(mi->label_object, "e,state,off", "e");
        if (isedje(mi->submenu_object))
          edje_object_signal_emit(mi->submenu_object, "e,state,off", "e");
        if (isedje(mi->toggle_object))
          edje_object_signal_emit(mi->toggle_object, "e,state,off", "e");
        if (mi->menu->bg_object)
          edje_object_signal_emit(mi->menu->bg_object, "e,state,off", "e");
     }
   if (tog)
     {
        if (mi->radio)
          {
             const Eina_List *l;
             E_Menu_Item *mi2;

             EINA_LIST_FOREACH(mi->menu->items, l, mi2)
               {
                  if ((mi2 != mi) && (mi2->radio) &&
                      (mi2->radio_group == mi->radio_group))
                    e_menu_item_toggle_set(mi2, 0);
               }
          }
     }
}

E_API int
e_menu_item_toggle_get(E_Menu_Item *mi)
{
   E_OBJECT_CHECK_RETURN(mi, 0);
   E_OBJECT_TYPE_CHECK_RETURN(mi, E_MENU_ITEM_TYPE, 0);
   return mi->toggle;
}

E_API void
e_menu_item_callback_set(E_Menu_Item *mi, void (*func)(void *data, E_Menu *m, E_Menu_Item *mi), const void *data)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   mi->cb.func = func;
   mi->cb.data = (void *)data;
}

E_API void
e_menu_item_realize_callback_set(E_Menu_Item *mi, void (*func)(void *data, E_Menu *m, E_Menu_Item *mi), void *data)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   mi->realize_cb.func = func;
   mi->realize_cb.data = data;
}

E_API void
e_menu_item_submenu_pre_callback_set(E_Menu_Item *mi, void (*func)(void *data, E_Menu *m, E_Menu_Item *mi), const void *data)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   mi->submenu_pre_cb.func = func;
   mi->submenu_pre_cb.data = (void *)data;
   if (!mi->submenu_post_cb.func)
     mi->submenu_post_cb.func = _e_menu_cb_item_submenu_post_default;
}

E_API void
e_menu_item_submenu_post_callback_set(E_Menu_Item *mi, void (*func)(void *data, E_Menu *m, E_Menu_Item *mi), const void *data)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   mi->submenu_post_cb.func = func;
   mi->submenu_post_cb.data = (void *)data;
}

E_API void
e_menu_item_drag_callback_set(E_Menu_Item *mi, void (*func)(void *data, E_Menu *m, E_Menu_Item *mi), void *data)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   mi->drag_cb.func = func;
   mi->drag_cb.data = data;
}

E_API void
e_menu_item_active_set(E_Menu_Item *mi, int active)
{
   Eina_List *tmp = NULL;

   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (mi->separator) return;
   active = !!active;
   if (active == mi->active) return;
   if ((active) && (!mi->active))
     {
        E_Menu_Item *pmi;

        if (mi->disable) return;
        pmi = _e_menu_item_active_get();
        if (mi == pmi) return;
        if (pmi)
          {
             tmp = _e_active_menus_copy_ref();
             e_menu_item_active_set(pmi, 0);
          }
        if (_e_prev_active_menu_item && (mi != _e_prev_active_menu_item))
          {
             if (mi->menu->parent_item && (_e_prev_active_menu_item != mi->menu->parent_item))
               _e_menu_submenu_deactivate(_e_prev_active_menu_item);
          }
        mi->active = 1;
        mi->hilighted = 1;
        _e_active_menu_item = mi;
        if (mi->bg_object)
          edje_object_signal_emit(mi->bg_object, "e,state,selected", "e");
        if (mi->icon_bg_object)
          edje_object_signal_emit(mi->icon_bg_object, "e,state,selected", "e");
        if (isedje(mi->label_object))
          edje_object_signal_emit(mi->label_object, "e,state,selected", "e");
        if (isedje(mi->submenu_object))
          edje_object_signal_emit(mi->submenu_object, "e,state,selected", "e");
        if (isedje(mi->toggle_object))
          edje_object_signal_emit(mi->toggle_object, "e,state,selected", "e");
        if (mi->icon_key)
          {
             if (mi->icon_object)
               {
                  if (isedje(mi->icon_object))
                    edje_object_signal_emit(mi->icon_object, "e,state,selected", "e");
                  else
                    e_icon_selected_set(mi->icon_object, EINA_TRUE);
               }
          }
        edje_object_signal_emit(mi->menu->bg_object, "e,state,selected", "e");
        _e_menu_submenu_activate(mi);
     }
   else if ((!active) && (mi->active))
     {
        tmp = _e_active_menus_copy_ref();
        mi->active = 0;
        _e_prev_active_menu_item = mi;
        _e_active_menu_item = NULL;
        if (!((mi->submenu) && (mi->submenu->active)))
          _e_menu_item_unhilight(mi);
     }
   _e_menu_list_free_unref(tmp);
}

E_API E_Menu_Item *
e_menu_item_active_get(void)
{
   return _e_active_menu_item;
}

E_API void
e_menu_active_item_activate(void)
{
   _e_menu_active_call();
   _e_menu_deactivate_all();
}

E_API void
e_menu_item_disabled_set(E_Menu_Item *mi, int disable)
{
   E_OBJECT_CHECK(mi);
   E_OBJECT_TYPE_CHECK(mi, E_MENU_ITEM_TYPE);
   if (mi->separator) return;
   if ((disable))
     {
        if (mi->active) e_menu_item_active_set(mi, 0);
        mi->disable = 1;
        if (mi->icon_bg_object)
          edje_object_signal_emit(mi->icon_bg_object, "e,state,disable", "e");
        if (isedje(mi->label_object))
          edje_object_signal_emit(mi->label_object, "e,state,disable", "e");
        if (isedje(mi->toggle_object))
          edje_object_signal_emit(mi->toggle_object, "e,state,disable", "e");
     }
   else
     {
        mi->disable = 0;
        if (mi->icon_bg_object)
          edje_object_signal_emit(mi->icon_bg_object, "e,state,enable", "e");
        if (isedje(mi->label_object))
          edje_object_signal_emit(mi->label_object, "e,state,enable", "e");
        if (isedje(mi->toggle_object))
          edje_object_signal_emit(mi->toggle_object, "e,state,enable", "e");
     }
}

E_API void
e_menu_idler_before(void)
{
   /* when e goes "idle" this gets called so leave all our hard work till */
   /* idle time to avoid falling behind the user. just evaluate the high */
   /* level state machine */
   Eina_List *l, *removals = NULL, *tmp;
   E_Menu *m;
   int active_count = 0;

   if (pending_feed)
     {
        e_comp_canvas_feed_mouse_up(pending_activate_time);
        pending_feed = 0;
     }

   /* add refcount to all menus we will work with */
   tmp = _e_active_menus_copy_ref();
   /* phase 1. hide all the menus that want to be hidden */
   EINA_LIST_FOREACH(_e_active_menus, l, m)
     {
        if ((!m->cur.visible) && (m->prev.visible))
          {
             m->prev.visible = m->cur.visible;
             e_object_ref(E_OBJECT(m));
             evas_object_pass_events_set(m->comp_object, 1);
             if (m->container_object)
               {
                  evas_object_intercept_move_callback_del(m->container_object, _e_menu_cb_intercept_container_move);
                  evas_object_intercept_resize_callback_del(m->container_object, _e_menu_cb_intercept_container_resize);
               }
             evas_object_hide(m->comp_object);
          }
     }
   /* phase 2. move & reisze all the menus that want to moves/resized */
   EINA_LIST_FOREACH(_e_active_menus, l, m)
     {
        if (m->frozen || (!m->active) || (!m->zone)) continue;
        if (!m->realized) _e_menu_realize(m);
        if (!m->realized) continue;
        if (((m->cur.w) != (m->prev.w)) ||
            ((m->cur.h) != (m->prev.h)))
          {
             int w, h;

             m->prev.w = m->cur.w;
             m->prev.h = m->cur.h;
             w = m->cur.w;
             h = m->cur.h;
             evas_object_resize(m->comp_object, w, h);
          }
        if (((m->cur.x) != (m->prev.x)) ||
            ((m->cur.y) != (m->prev.y)))
          {
             if (!m->parent_item)
               {
                  int x, y, w, h;

                  e_zone_useful_geometry_get(m->zone, &x, &y, &w, &h);
                  if (m->cur.w <= w)
                    {
                       if ((m->cur.x + m->cur.w) > (x + w))
                         m->cur.x = x + w - m->cur.w;
                    }
                  if (m->cur.h <= h)
                    {
                       if ((m->cur.y + m->cur.h) > (y + h))
                         m->cur.y = y + h - m->cur.h;
                    }
               }
             m->prev.x = m->cur.x;
             m->prev.y = m->cur.y;
             _e_menu_lock = 1;
             evas_object_move(m->comp_object, m->cur.x, m->cur.y);
             _e_menu_lock = 0;
          }
     }
   /* phase 3. show all the menus that want to be shown */
   EINA_LIST_FOREACH(_e_active_menus, l, m)
     {
        if (m->frozen) continue;
        if (!m->realized) continue;
        if (m->cur.visible)
          {
             m->prev.visible = m->cur.visible;
             evas_object_pass_events_set(m->comp_object, 0);
             evas_object_show(m->comp_object);
          }
     }
   /* phase 4. de-activate... */
   EINA_LIST_REVERSE_FOREACH(_e_active_menus, l, m)
     {
        if (!m->active)
          {
             if ((m->realized) &&
                 (!evas_object_visible_get(m->comp_object)))
               {
                  _e_menu_unrealize(m);
                  removals = eina_list_append(removals, m);
               }
          }
     }
   EINA_LIST_FREE(removals, m)
     {
        if (m->in_active_list)
          {
             _e_active_menus = eina_list_remove(_e_active_menus, m);
             m->in_active_list = 0;
             e_object_unref(E_OBJECT(m));
          }
     }
   /* del refcount to all menus we worked with */
   _e_menu_list_free_unref(tmp);

   EINA_LIST_FOREACH(_e_active_menus, l, m)
     {
        if (m->active) active_count++;
     }
   if (!active_count)
     {
        if (_e_menu_win == e_comp->ee_win)
          {
             if (_e_menu_grabbed)
               {
                  e_comp_ungrab_input(1, 1);
                  _e_menu_grabbed = EINA_FALSE;
               }
             _e_menu_win = UINT_MAX;
             e_bindings_disabled_set(0);
          }
     }
}

E_API Eina_Bool
e_menu_is_active(void)
{
   return _e_menu_win == e_comp->ee_win;
}

E_API E_Menu *
e_menu_active_get(void)
{
   return _e_active_menus ? eina_list_last_data_get(_e_active_menus) : NULL;
}

/* local subsystem functions */
static void
_e_menu_dangling_cb(void *data)
{
   E_Menu *m = data;

   WRN("DANGLING SUBMENU: REF(%d)||MENU(%p)", e_object_ref_get(data), data);
   m->dangling_job = NULL;
}

static void
_e_menu_free(E_Menu *m)
{
   Eina_List *l, *l_next;
   E_Menu_Item *mi;
   E_Menu_Category *cat = NULL;

   /* the foreign menu items */
   if (m->category) cat = eina_hash_find(_e_menu_categories, m->category);
   if (cat)
     {
        E_Menu_Category_Callback *cb;

        EINA_LIST_FOREACH(cat->callbacks, l, cb)
          {
             if (cb->free) cb->free(cb->data);
          }
     }
   eina_stringshare_replace(&m->category, NULL);
   if (m->parent_item)
     m->parent_item->submenu = NULL;
   _e_menu_unrealize(m);
   if (m->realized) return;
   EINA_LIST_FOREACH_SAFE(m->items, l, l_next, mi)
     e_object_del(E_OBJECT(mi));
   if (m->in_active_list)
     {
        _e_active_menus = eina_list_remove(_e_active_menus, m);
        m->in_active_list = 0;
     }
   if (m->header.title) eina_stringshare_del(m->header.title);
   if (m->header.icon_file) eina_stringshare_del(m->header.icon_file);
   if (m->dangling_job) ecore_job_del(m->dangling_job);
   free(m);
}

static void
_e_menu_item_free(E_Menu_Item *mi)
{
   if (mi == _e_active_menu_item) _e_active_menu_item = NULL;
   if (mi == _e_prev_active_menu_item) _e_prev_active_menu_item = NULL;
   if (mi->submenu)
     {
        int ref = 0;

        /* parent_item gets unset in a few places, reapply it for use in cleanup */
        if (!mi->submenu->parent_item)
          mi->submenu->parent_item = mi;
        /* menu may not have been deactivated, ensure deactivate callback is called */
        if (mi->active)
          _e_menu_submenu_deactivate(mi);
        /* submenus CANNOT exist without their parent menu+item, so ensure that they get deleted */
        if (mi->submenu)
          {
             ref = e_object_ref_get(E_OBJECT(mi->submenu)) - 1;
             e_object_unref(E_OBJECT(mi->submenu));
          }
        if (ref)
          {
             if (!mi->submenu->dangling_job)
               mi->submenu->dangling_job = ecore_job_add(_e_menu_dangling_cb, mi->submenu);
             mi->submenu->parent_item = NULL;
          }
     }
   if (mi->menu->realized) _e_menu_item_unrealize(mi);
   mi->menu->items = eina_list_remove(mi->menu->items, mi);
   if (mi->icon) eina_stringshare_del(mi->icon);
   if (mi->icon_key) eina_stringshare_del(mi->icon_key);
   if (mi->label) eina_stringshare_del(mi->label);
   free(mi);
}

static void
_e_menu_cb_intercept_item_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y)
{
   E_Menu_Item *mi;

   mi = data;
   mi->x = x;
   mi->y = y;
   evas_object_move(o, x, y);
   if ((mi->submenu) && (mi->submenu->parent_item))
     {
        mi->submenu->zone = mi->menu->zone;
        _e_menu_reposition(mi->submenu);
     }
}

static void
_e_menu_cb_intercept_item_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
   E_Menu_Item *mi;

   mi = data;
   mi->w = w;
   mi->h = h;
   evas_object_resize(o, w, h);
   if ((mi->submenu) && (mi->submenu->parent_item))
     _e_menu_reposition(mi->submenu);
}

static void
_e_menu_cb_intercept_container_move(void *data, Evas_Object *o, Evas_Coord x, Evas_Coord y)
{
   E_Menu *m;

   m = data;
   m->container_x = x;
   m->container_y = y;
   if (m->parent_item) _e_menu_reposition(m);
   evas_object_move(o, x, y);
}

static void
_e_menu_cb_intercept_container_resize(void *data, Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
   E_Menu *m;

   m = data;
   m->container_w = w;
   m->container_h = h;
   if (m->parent_item) _e_menu_reposition(m);
   evas_object_resize(o, w, h);
}

static void
_e_menu_hide_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_object_unref(data);
}

static void
_e_menu_item_realize(E_Menu_Item *mi)
{
   Evas_Object *o;
   Evas_Coord ww = 1, hh = 1;

   /* and set up initial item state */
   if (mi->separator)
     {
        o = edje_object_add(mi->menu->evas);
        mi->separator_object = o;
        e_theme_edje_object_set(o, "base/theme/menus",
                                "e/widgets/menu/default/separator");
        edje_object_size_min_calc(mi->separator_object, &ww, &hh);
        E_FILL(mi->separator_object);
        mi->separator_w = ww;
        mi->separator_h = hh;
        evas_object_size_hint_min_set(mi->separator_object, ww, hh);
        elm_box_pack_end(mi->menu->container_object, mi->separator_object);
        evas_object_show(o);
     }
   else
     {
        o = edje_object_add(mi->menu->evas);
        mi->bg_object = o;
        evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_IN, _e_menu_cb_item_in, mi);
        evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_OUT, _e_menu_cb_item_out, mi);
        evas_object_intercept_move_callback_add(o, _e_menu_cb_intercept_item_move, mi);
        evas_object_intercept_resize_callback_add(o, _e_menu_cb_intercept_item_resize, mi);
        if ((mi->submenu) || (mi->submenu_pre_cb.func))
          {
             if (!e_theme_edje_object_set(mi->bg_object, "base/theme/menus",
                                          "e/widgets/menu/default/submenu_bg"))
               goto no_submenu_item;
          }
        else
          {
no_submenu_item:
             e_theme_edje_object_set(mi->bg_object, "base/theme/menus",
                                     "e/widgets/menu/default/item_bg");
          }
        o = elm_box_add(e_comp->elm);
        mi->container_object = o;
        elm_box_horizontal_set(o, 1);
        evas_object_show(o);


        if (mi->check)
          {
             o = edje_object_add(mi->menu->evas);
             mi->toggle_object = o;
             e_theme_edje_object_set(o, "base/theme/menus",
                                     "e/widgets/menu/default/check");
             edje_object_size_min_calc(mi->toggle_object, &ww, &hh);
             mi->toggle_w = ww;
             mi->toggle_h = hh;
             E_WEIGHT(mi->toggle_object, 0, 1);
             E_FILL(mi->toggle_object);
             evas_object_size_hint_min_set(mi->toggle_object, ww, hh);
             elm_box_pack_end(mi->container_object, o);
             evas_object_show(o);
          }
        else if (mi->radio)
          {
             o = edje_object_add(mi->menu->evas);
             mi->toggle_object = o;
             e_theme_edje_object_set(o, "base/theme/menus",
                                     "e/widgets/menu/default/radio");
             edje_object_size_min_calc(mi->toggle_object, &ww, &hh);
             mi->toggle_w = ww;
             mi->toggle_h = hh;
             E_WEIGHT(mi->toggle_object, 0, 1);
             E_FILL(mi->toggle_object);
             evas_object_size_hint_min_set(mi->toggle_object, ww, hh);
             elm_box_pack_end(mi->container_object, o);
             evas_object_show(o);
          }
        else
          {
             o = evas_object_rectangle_add(mi->menu->evas);
             mi->toggle_object = o;
             evas_object_color_set(o, 0, 0, 0, 0);
             elm_box_pack_end(mi->container_object, o);
          }
        if ((!e_config->menu_icons_hide) && ((mi->icon) || (mi->realize_cb.func)))
          {
             int icon_w = 0, icon_h = 0;

             o = edje_object_add(mi->menu->evas);
             if (e_theme_edje_object_set(o, "base/theme/menus",
                                         "e/widgets/menu/default/icon"))
               {
                  mi->icon_bg_object = o;
               }
             else
               {
                  evas_object_del(o);
                  o = NULL;
               }
             //if (o) evas_object_pass_events_set(o, 1);

             /* FIXME: Not sure why there are two different tries to get the icon size, surely only the last one is needed. */
             /* FIXME: Do it this way later, when e_app_icon_add() just registers a request for an icon to be filled in when it's ready.
                if (mi->app)
                {
                  o = e_app_icon_add(mi->menu->evas, mi->app);
                  mi->icon_object = o;
                  e_icon_size_get(mi->icon_object, &icon_w, &icon_h);
                }
                else
              */
             if (mi->icon)
               {
                  /* This is done this way to match up with how e_app_icon_add does it. */
                  if (mi->icon_key)
                    {
                       Evas_Coord iww, ihh;

                       o = edje_object_add(mi->menu->evas);
                       if (edje_object_file_set(o, mi->icon, mi->icon_key))
                         {
                            mi->icon_object = o;
                            edje_object_size_max_get(o, &iww, &ihh);
                            icon_w = iww;
                            icon_h = ihh;
                         }
                       else
                         {
                            evas_object_del(o);
                            o = NULL;
                         }
                    }
                  if (!mi->icon_object)
                    {
                       o = e_icon_add(mi->menu->evas);
                       mi->icon_object = o;
                       e_icon_scale_size_set(o, e_util_icon_size_normalize(96 * e_scale));
                       e_icon_preload_set(mi->icon_object, 1);
                       e_icon_file_set(o, mi->icon);
                       e_icon_fill_inside_set(mi->icon_object, 1);
                       e_icon_size_get(mi->icon_object, &icon_w, &icon_h);
                    }
               }
             if (_e_menu_realize_call(mi))
               {
                  o = mi->icon_object;
                  e_icon_fill_inside_set(o, 1);
                  e_icon_size_get(o, &icon_w, &icon_h);
               }

             evas_object_show(o);

             if (mi->icon_bg_object)
               {
                  evas_object_size_hint_min_set(mi->icon_object, 0, 0);
                  edje_object_part_swallow(mi->icon_bg_object,
                                           "e.swallow.content",
                                           mi->icon_object);
                  edje_object_size_min_calc(mi->icon_bg_object, &ww, &hh);
                  mi->icon_w = ww;
                  mi->icon_h = hh;
                  evas_object_size_hint_min_set(mi->icon_bg_object, ww, hh);
                  E_WEIGHT(mi->icon_bg_object, 0, 1);
                  E_FILL(mi->icon_bg_object);
                  elm_box_pack_end(mi->container_object, mi->icon_bg_object);
                  evas_object_show(mi->icon_bg_object);
               }
             else
               {
                  o = edje_object_add(mi->menu->evas);
                  e_icon_size_get(mi->icon_object, &icon_w, &icon_h);
                  mi->icon_w = icon_w;
                  mi->icon_h = icon_h;
                  E_WEIGHT(mi->icon_object, 0, 1);
                  E_FILL(mi->icon_object);
                  evas_object_size_hint_min_set(mi->icon_object, ww, hh);
                  elm_box_pack_end(mi->container_object, o);
                  evas_object_show(o);
               }
          }
        else
          {
             o = evas_object_rectangle_add(mi->menu->evas);
             mi->icon_object = o;
             evas_object_color_set(o, 0, 0, 0, 0);
             elm_box_pack_end(mi->container_object, o);
          }

        if (mi->label)
          {
             o = edje_object_add(mi->menu->evas);
             mi->label_object = o;
             e_theme_edje_object_set(o, "base/theme/menus",
                                     "e/widgets/menu/default/label");
             /* default label */
             edje_object_part_text_set(o, "e.text.label", mi->label);
             edje_object_size_min_calc(mi->label_object, &ww, &hh);
             mi->label_w = ww;
             mi->label_h = hh;
             evas_object_size_hint_min_set(mi->label_object, ww, hh);
             E_EXPAND(mi->label_object);
             E_FILL(mi->label_object);
             elm_box_pack_end(mi->container_object, o);
             evas_object_show(o);
          }
        else
          {
             o = evas_object_rectangle_add(mi->menu->evas);
             mi->label_object = o;
             evas_object_color_set(o, 0, 0, 0, 0);
             elm_box_pack_end(mi->container_object, o);
          }
        if ((mi->submenu) || (mi->submenu_pre_cb.func))
          {
             o = edje_object_add(mi->menu->evas);
             mi->submenu_object = o;
             e_theme_edje_object_set(o, "base/theme/menus",
                                     "e/widgets/menu/default/submenu");
             edje_object_size_min_calc(mi->submenu_object, &ww, &hh);
             mi->submenu_w = ww;
             mi->submenu_h = hh;
             E_WEIGHT(mi->submenu_object, 0, 1);
             E_FILL(mi->submenu_object);
             evas_object_size_hint_min_set(mi->submenu_object, ww, hh);
             elm_box_pack_end(mi->container_object, o);
             evas_object_show(o);
          }
        else
          {
             o = evas_object_rectangle_add(mi->menu->evas);
             mi->submenu_object = o;
             evas_object_color_set(o, 0, 0, 0, 0);
             elm_box_pack_end(mi->container_object, o);
          }

        edje_object_part_swallow(mi->bg_object, "e.swallow.content",
                                 mi->container_object);

        elm_box_pack_end(mi->menu->container_object, mi->bg_object);
        evas_object_show(mi->container_object);
        evas_object_show(mi->bg_object);
     }
   if (mi->active) e_menu_item_active_set(mi, 1);
   if (mi->toggle) e_menu_item_toggle_set(mi, 1);
   if (mi->disable) e_menu_item_disabled_set(mi, 1);
}

static void
_e_menu_realize(E_Menu *m)
{
   Evas_Object *o;
   Eina_List *l;
   E_Menu_Item *mi;

   if (m->realized || (!m->items)) return;

   if (m->parent_item && m->parent_item->menu)
     m->zone = m->parent_item->menu->zone;
   if (!m->zone) return; //menu not ready!
   m->evas = evas_object_evas_get(e_comp->elm);
   evas_event_freeze(m->evas);

   o = edje_object_add(m->evas);
   m->bg_object = o;
   e_theme_edje_object_set(o, "base/theme/menus", "e/widgets/menu/default/background");
   if (m->header.title)
     {
        edje_object_part_text_set(o, "e.text.title", m->header.title);
        edje_object_signal_emit(o, "e,action,show,title", "e");
        edje_object_message_signal_process(o);
     }

   m->comp_object = e_comp_object_util_add(o, E_COMP_OBJECT_TYPE_MENU);
   if (!m->parent_item)
     e_comp_object_util_autoclose(m->comp_object, _e_menu_cb_mouse_evas_down, _e_menu_cb_key_down, m);
   evas_object_event_callback_add(m->comp_object, EVAS_CALLBACK_HIDE, _e_menu_hide_cb, m);
   evas_object_layer_set(m->comp_object, E_LAYER_MENU);
   evas_object_geometry_set(m->comp_object, m->cur.x, m->cur.y, m->cur.w, m->cur.h);

   o = elm_box_add(e_comp->elm);
   m->container_object = o;
   evas_object_intercept_move_callback_add(o, _e_menu_cb_intercept_container_move, m);
   evas_object_intercept_resize_callback_add(o, _e_menu_cb_intercept_container_resize, m);

   EINA_LIST_FOREACH(m->items, l, mi)
     _e_menu_item_realize(mi);

   edje_object_part_swallow(m->bg_object, "e.swallow.content", m->container_object);

   _e_menu_items_layout_update(m);


   evas_event_thaw(m->evas);
   evas_event_thaw_eval(m->evas);
   m->realized = 1;
}

static void
_e_menu_items_layout_update(E_Menu *m)
{
   Eina_List *l;
   E_Menu_Item *mi;
   Evas_Coord bw, bh, mw = 0, mh = 0;
   int toggles_on = 0;
   int icons_on = 0;
   int labels_on = 0;
   int submenus_on = 0;
   int min_icon_w = 0, min_icon_h = 0;
   int min_label_w = 0, min_label_h = 0;
   int min_submenu_w = 0, min_submenu_h = 0;
   int min_toggle_w = 0, min_toggle_h = 0;
   int min_w = 0, min_h = 1;
   int zh = 0, ms = 0, maxh = 0;
   unsigned int cur_items = 0, max_items = -1;

   if (!m->zone) return;
   EINA_LIST_FOREACH(m->items, l, mi)
     {
        if (mi->icon) icons_on = 1;
        if (mi->icon_object) icons_on = 1;
        if (mi->label) labels_on = 1;
        if (mi->submenu) submenus_on = 1;
        if (mi->submenu_pre_cb.func) submenus_on = 1;
        if (mi->check) toggles_on = 1;
        if (mi->radio) toggles_on = 1;

        if (mi->icon_w > min_icon_w) min_icon_w = mi->icon_w;
        if (mi->icon_h > min_icon_h) min_icon_h = mi->icon_h;
        if (mi->label_w > min_label_w) min_label_w = mi->label_w;
        if (mi->label_h > min_label_h) min_label_h = mi->label_h;
        if (mi->submenu_w > min_submenu_w) min_submenu_w = mi->submenu_w;
        if (mi->submenu_h > min_submenu_h) min_submenu_h = mi->submenu_h;
        if (mi->toggle_w > min_toggle_w) min_toggle_w = mi->toggle_w;
        if (mi->toggle_h > min_toggle_h) min_toggle_h = mi->toggle_h;
     }
   if (labels_on)
     {
        if (submenus_on)
          {
             if (min_label_h < min_submenu_h)
               min_label_h = min_submenu_h;
          }
        if (toggles_on)
          {
             if (min_label_h < min_toggle_h)
               min_label_h = min_toggle_h;
          }
        if ((icons_on) && (min_icon_h > 0))
          {
             min_icon_w = (min_icon_w * min_label_h) / min_icon_h;
             min_icon_h = min_label_h;
          }
        min_w = min_label_w + min_icon_w + min_submenu_w + min_toggle_w;
        min_h = min_label_h;
     }
   else if (icons_on)
     {
        if (submenus_on)
          {
             if (min_icon_h < min_submenu_h)
               min_icon_h = min_submenu_h;
          }
        if (toggles_on)
          {
             if (min_icon_h < min_toggle_h)
               min_icon_h = min_toggle_h;
          }
        min_w = min_icon_w + min_toggle_w + min_submenu_w;
        min_h = min_icon_h;
     }
   else if (toggles_on)
     {
        if (submenus_on)
          {
             if (min_toggle_h < min_submenu_h)
               min_toggle_h = min_submenu_h;
          }
        min_w = min_toggle_w + min_submenu_w;
        min_h = min_toggle_h;
     }
   if (min_h * eina_list_count(m->items) >= (unsigned int)m->zone->h)
     {
        e_zone_useful_geometry_get(m->zone, NULL, NULL, NULL, &zh);
        maxh = zh * 4;
        if (maxh > 30000) maxh = 30000;  // 32k x 32k mx coord limit for wins
        max_items = (maxh / min_h) - 1;
     }
   EINA_LIST_FOREACH(m->items, l, mi)
     {
        if (cur_items >= max_items)
          {
             _e_menu_item_unrealize(mi);
             continue;
          }
        cur_items++;
        if (mi->separator)
          {
             E_WEIGHT(mi->separator_object, 1, 0);
             E_FILL(mi->separator_object);
             evas_object_size_hint_min_set(mi->separator_object, mi->separator_w, mi->separator_h);
             evas_object_size_hint_max_set(mi->separator_object, -1, mi->separator_h);
             ms += mi->separator_h;
             continue;
          }
        E_WEIGHT(mi->toggle_object, 0, toggles_on);
        E_FILL(mi->toggle_object);
        evas_object_size_hint_min_set(mi->toggle_object, min_toggle_w * toggles_on, min_toggle_h * toggles_on);
        if (icons_on)
          {
             E_WEIGHT(mi->icon_bg_object ?: mi->icon_object, 0, 1);
             E_FILL(mi->icon_bg_object ?: mi->icon_object);
             evas_object_size_hint_min_set(mi->icon_bg_object ?: mi->icon_object, min_icon_w, min_icon_h);
          }
        else
          {
             E_WEIGHT(mi->icon_object, 0, 1);
             E_FILL(mi->icon_object);
             evas_object_size_hint_min_set(mi->icon_object, 0, 0);
          }

        E_WEIGHT(mi->label_object, 0, 1 * labels_on);
        E_FILL(mi->label_object);
        evas_object_size_hint_min_set(mi->label_object, min_label_w * labels_on, min_label_h * labels_on);

        E_WEIGHT(mi->submenu_object, 0, 1 * submenus_on);
        E_FILL(mi->submenu_object);
        evas_object_size_hint_min_set(mi->submenu_object, min_submenu_w * submenus_on, min_submenu_h * submenus_on);

        evas_object_size_hint_min_set(mi->container_object,
                                        min_w, min_h);
        edje_object_size_min_calc(mi->bg_object, &mw, &mh);
        E_WEIGHT(mi->bg_object, 0, 1);
        E_FILL(mi->bg_object);
        evas_object_size_hint_min_set(mi->bg_object, mw, mh);
        ms += mh;
     }
   elm_box_recalculate(m->container_object);
   evas_object_size_hint_min_get(m->container_object, &bw, &bh);
   evas_object_size_hint_max_set(m->container_object, bw, bh);
   edje_object_size_min_calc(m->bg_object, &mw, &mh);
   m->cur.w = mw;
   m->cur.h = mh;
}

static void
_e_menu_item_unrealize(E_Menu_Item *mi)
{
   if (mi->separator_object) evas_object_del(mi->separator_object);
   mi->separator_object = NULL;
   if (mi->bg_object) evas_object_del(mi->bg_object);
   mi->bg_object = NULL;
   if (mi->container_object) evas_object_del(mi->container_object);
   mi->container_object = NULL;
   if (mi->toggle_object) evas_object_del(mi->toggle_object);
   mi->toggle_object = NULL;
   if (mi->icon_bg_object) evas_object_del(mi->icon_bg_object);
   mi->icon_bg_object = NULL;
   if (mi->icon_object) evas_object_del(mi->icon_object);
   mi->icon_object = NULL;
   if (mi->label_object) evas_object_del(mi->label_object);
   mi->label_object = NULL;
   if (mi->submenu_object) evas_object_del(mi->submenu_object);
   mi->submenu_object = NULL;
}

static void
_e_menu_unrealize(E_Menu *m)
{
   Eina_List *l;
   E_Menu_Item *mi;

   if (!m->realized) return;
   /* freeze+thaw here breaks the universe. don't do it. */
   //evas_event_freeze(m->evas);
   if (m->comp_object)
     evas_object_event_callback_del(m->comp_object, EVAS_CALLBACK_HIDE, _e_menu_hide_cb);
   if (m->cur.visible && m->comp_object && (!stopping))
     {
        /* force unref in smart object */
        if (m->container_object)
          {
             evas_object_intercept_move_callback_del(m->container_object, _e_menu_cb_intercept_container_move);
             evas_object_intercept_resize_callback_del(m->container_object, _e_menu_cb_intercept_container_resize);
          }
        evas_object_pass_events_set(m->comp_object, 1);
        evas_object_hide(m->comp_object);
        E_FREE_FUNC(m->comp_object, evas_object_del);
        return;
     }
   evas_object_hide(m->comp_object);
   evas_object_del(m->comp_object);
   if (stopping && m->comp_object) evas_object_unref(m->comp_object);
   EINA_LIST_FOREACH(m->items, l, mi)
     _e_menu_item_unrealize(mi);
   E_FREE_FUNC(m->header.icon, evas_object_del);
   E_FREE_FUNC(m->bg_object, evas_object_del);
   E_FREE_FUNC(m->container_object, evas_object_del);
   m->cur.visible = 0;
   m->prev.visible = 0;
   m->realized = 0;
   m->zone = NULL;
   //evas_event_thaw(m->evas);
   m->evas = NULL;
}

static void
_e_menu_activate_internal(E_Menu *m, E_Zone *zone)
{
   E_Menu_Category *cat = NULL;
   Eina_List *l;

   if (m->pre_activate_cb.func)
     m->pre_activate_cb.func(m->pre_activate_cb.data, m);
   m->fast_mouse = 0;
   m->pending_new_submenu = 0;
   if (_e_menu_win != e_comp->ee_win)
     {
        _e_menu_win = e_comp->ee_win;
        if (!e_comp_grab_input(1, 1))
          {
             _e_menu_win = UINT_MAX;
             return;
          }
        _e_menu_grabbed = EINA_TRUE;
        e_bindings_disabled_set(1);
     }
   m->zone = zone;
   if (!m->active)
     {
        /* this remove is in case the menu is marked as inactive but hasn't */
        /* been removed from the list yet */
        if (m->in_active_list)
          _e_active_menus = eina_list_remove(_e_active_menus, m);
        _e_active_menus = eina_list_append(_e_active_menus, m);
        if (!m->in_active_list)
          e_object_ref(E_OBJECT(m));
        m->in_active_list = 1;
        m->active = 1;
     }
   /* the foreign menu items */
   if (m->category)
     {
        cat = eina_hash_find(_e_menu_categories, m->category);
        if (cat)
          {
             E_Menu_Category_Callback *cb;
             EINA_LIST_FOREACH(cat->callbacks, l, cb)
               if (cb->create) cb->create(cb->data, m, cat->data);
          }
     }
   m->cur.visible = 1;
}

static void
_e_menu_deactivate_all(void)
{
   Eina_List *tmp;
   E_Menu *m;

   tmp = _e_active_menus_copy_ref();

   EINA_LIST_FREE(tmp, m)
     {
        e_menu_deactivate(m);
        m->parent_item = NULL;
        e_object_unref(E_OBJECT(m));
     }
   _e_menu_activate_floating = 0;
   _e_menu_activate_maybe_drag = 0;
   _e_menu_activate_dragging = 0;
}

static void
_e_menu_deactivate_above(E_Menu *ma)
{
   Eina_List *tmp;
   int above = 0;
   E_Menu *m;

   tmp = _e_active_menus_copy_ref();

   EINA_LIST_FREE(tmp, m)
     {
        if (above)
          {
             e_menu_deactivate(m);
             m->parent_item = NULL;
          }
        if (ma == m) above = 1;
        e_object_unref(E_OBJECT(m));
     }
}

static void
_e_menu_submenu_activate(E_Menu_Item *mi)
{
   if (!mi->menu->active) return;
   if (mi->submenu && mi->submenu->active) return;
   if (mi->menu->fast_mouse)
     {
        mi->menu->pending_new_submenu = 1;
        return;
     }
   mi->menu->pending_new_submenu = 0;
   _e_menu_deactivate_above(mi->menu);
   if (mi->submenu_pre_cb.func)
     mi->submenu_pre_cb.func(mi->submenu_pre_cb.data, mi->menu, mi);
   if (mi->submenu)
     {
        E_Menu *m;

        m = mi->submenu;
        e_object_ref(E_OBJECT(m));
        m->parent_item = mi;
        _e_menu_activate_internal(m, mi->menu->zone);
        _e_menu_reposition(m);
        e_object_unref(E_OBJECT(m));
        mi->menu->have_submenu = 1;
     }
}

static void
_e_menu_submenu_deactivate(E_Menu_Item *mi)
{
   if (!mi->menu->active) return;
   mi->menu->have_submenu = 0;
   if (mi->submenu_post_cb.func)
     mi->submenu_post_cb.func(mi->submenu_post_cb.data, mi->menu, mi);
}

static void
_e_menu_reposition(E_Menu *m)
{
   Eina_List *l, *tmp;
   E_Menu_Item *mi;
   int parent_item_bottom;

   if (!m->zone) return;
   if (!m->parent_item) return;
   m->cur.x = m->parent_item->menu->cur.x + m->parent_item->menu->cur.w;

   parent_item_bottom = m->parent_item->y;
   if (m->cur.h > m->zone->h)
     {
#if 0 // we can't win - we just flip back and forth, so let it go off and use scrolling
        /* menu is larger than screen */
        if (parent_item_bottom > (m->zone->y + (m->zone->h / 2)))
          /* more is shown if menu goes up */
          m->cur.y = (parent_item_bottom - (m->zone->h + 1));
        else
          /* more is shown if menu goes down */
          m->cur.y = parent_item_bottom - m->zone->y;
#endif
     }
   else
     {
        /* menu is on top or bottom half of screen */
        if (parent_item_bottom > (m->zone->y + (m->zone->h / 2)))
          m->cur.y = parent_item_bottom - m->cur.h + m->parent_item->h;
        else
          m->cur.y = parent_item_bottom;
     }

   /* FIXME: this will suck for big menus */
   tmp = _e_active_menus_copy_ref();

   EINA_LIST_FOREACH(m->items, l, mi)
     if ((mi->active) && (mi->submenu)) _e_menu_reposition(mi->submenu);

   _e_menu_list_free_unref(tmp);
}

static int
_e_menu_active_call(void)
{
   E_Menu_Item *mi;

   mi = _e_menu_item_active_get();
   if (mi)
     {
        if (mi->check)
          e_menu_item_toggle_set(mi, !mi->toggle);
        if ((mi->radio) && (!e_menu_item_toggle_get(mi)))
          e_menu_item_toggle_set(mi, 1);
        if (mi->cb.func)
          mi->cb.func(mi->cb.data, mi->menu, mi);
        return 1;
     }
   return -1;
}

static int
_e_menu_realize_call(E_Menu_Item *mi)
{
   if (mi)
     {
        if (mi->realize_cb.func)
          {
             mi->realize_cb.func(mi->realize_cb.data, mi->menu, mi);
             return 1;
          }
     }
   return 0;
}

static void
_e_menu_item_activate_next(void)
{
   E_Menu_Item *mi;
   Eina_List *ll;

   ll = _e_menu_list_item_active_get();
   mi = _e_menu_item_active_get();
   if (ll && mi)
     {
        /* Look at the next item and then cycle until we're not on
         * a separator. */
        do
          {
             if (!eina_list_next(ll))
               ll = mi->menu->items;
             else
               ll = eina_list_next(ll);
             mi = eina_list_data_get(ll);
          }
        while (mi->separator || mi->disable);

        e_menu_item_active_set(mi, 1);
        _e_menu_item_ensure_onscreen(mi);
        return;
     }

   _e_menu_activate_first();
}

static void
_e_menu_item_activate_previous(void)
{
   E_Menu_Item *mi;
   Eina_List *ll;

   ll = _e_menu_list_item_active_get();
   mi = _e_menu_item_active_get();
   if (ll && mi)
     {
        /* Look at the prev item and then cycle until we're not on
         * a separator. */
        do
          {
             if (!eina_list_prev(ll))
               ll = eina_list_last(ll);
             else
               ll = eina_list_prev(ll);
             mi = eina_list_data_get(ll);
          }
        while ((mi->separator) || (mi->disable));

        e_menu_item_active_set(mi, 1);
        _e_menu_item_ensure_onscreen(mi);
        return;
     }

   _e_menu_activate_first();
}

static void
_e_menu_item_activate_first(void)
{
   E_Menu *m;
   Eina_List *ll;
   E_Menu_Item *mi;

   m = _e_menu_active_get();
   if (m)
     {
        ll = m->items;
        mi = eina_list_data_get(ll);
        while ((mi->separator) && eina_list_next(ll))
          {
             ll = eina_list_next(ll);
             mi = eina_list_data_get(ll);
          }
        if (mi->separator) return;
        e_menu_item_active_set(mi, 1);
        _e_menu_item_ensure_onscreen(mi);
        return;
     }
   _e_menu_activate_first();
}

static void
_e_menu_item_activate_last(void)
{
   E_Menu *m;
   Eina_List *ll;
   E_Menu_Item *mi;

   m = _e_menu_active_get();
   if (m)
     {
        ll = eina_list_last(m->items);
        mi = eina_list_data_get(ll);
        while ((mi->separator) && eina_list_prev(ll))
          {
             ll = eina_list_prev(ll);
             mi = eina_list_data_get(ll);
          }
        if (mi->separator) return;
        e_menu_item_active_set(mi, 1);
        _e_menu_item_ensure_onscreen(mi);
        return;
     }
   _e_menu_activate_first();
}

static void
_e_menu_item_activate_nth(int n)
{
   E_Menu *m;
   E_Menu_Item *mi;
   Eina_List *ll;
   int i = -1;

   mi = _e_menu_item_active_get();
   if (!mi)
     {
        _e_menu_activate_first();
        mi = _e_menu_item_active_get();
        if (!mi) return;
     }
   m = mi->menu;
   EINA_LIST_FOREACH(m->items, ll, mi)
     {
        if (!mi->separator) i++;
        if (i == n) break;
     }
   if (!mi) return;
   e_menu_item_active_set(mi, 1);
   _e_menu_item_ensure_onscreen(mi);
}

static void
_e_menu_item_activate_char(const char *key_compose)
{
   E_Menu *m;
   E_Menu_Item *mi;
   Eina_List *ll, *ll_orig;

   /* Ignore modifiers and such. */
   if (!key_compose) return;

   /* Check we've got a menu and it's active. */
   m = _e_menu_active_get();
   if (!m)
     {
        if (!_e_active_menus) return;
        m = eina_list_data_get(_e_active_menus);
        if (!m) return;
     }

   ll = _e_menu_list_item_active_get();
   /* If we don't have an active item, start from the top of the list. */
   if (!ll)
     {
        ll = m->items;
        mi = eina_list_data_get(ll);
        /* Only check the current item if it wasn't active before. */
        if (!mi->separator && mi->label && !strncasecmp(key_compose, mi->label, strlen(key_compose)))
          {
             e_menu_item_active_set(mi, 1);
             _e_menu_item_ensure_onscreen(mi);
             return;
          }
     }

   ll_orig = ll;

   mi = eina_list_data_get(ll);
   if (!eina_list_next(ll))
     ll = mi->menu->items;
   else
     ll = eina_list_next(ll);
   mi = eina_list_data_get(ll);

   /* While we don't have a label OR we don't match  AND we haven't
    * wrapped around */
   while ((!mi->label || strncasecmp(key_compose, mi->label, strlen(key_compose)))
          && ll != ll_orig)
     {
        do
          {
             if (!eina_list_next(ll))
               ll = mi->menu->items;
             else
               ll = eina_list_next(ll);
             mi = eina_list_data_get(ll);
          }
        while (mi->separator);
     }

   e_menu_item_active_set(mi, 1);
   _e_menu_item_ensure_onscreen(mi);
   return;
}

static void
_e_menu_activate_next(void)
{
   E_Menu_Item *mi;

   mi = _e_menu_item_active_get();
   if (mi)
     {
        if (mi->submenu)
          {
             if (mi->submenu->items)
               {
                  Eina_List *l;
                  EINA_LIST_FOREACH(mi->submenu->items, l, mi)
                    if (!mi->disable)
                      {
                         e_menu_item_active_set(mi, 1);
                         _e_menu_item_ensure_onscreen(mi);
                         break;
                      }
               }
          }
        return;
     }
   _e_menu_activate_first();
}

static void
_e_menu_activate_previous(void)
{
   E_Menu_Item *mi;

   mi = _e_menu_item_active_get();
   if (mi)
     {
        if (mi->menu->parent_item)
          {
             Eina_List *l;
             E_Menu_Item *mi2;

             EINA_LIST_FOREACH(mi->menu->items, l, mi2)
               {
                  if (!mi2->disable)
                    {
                       e_menu_item_active_set(mi2, 0);
                       _e_menu_item_unhilight(mi2);
                    }
               }
             mi = mi->menu->parent_item;
             e_menu_item_active_set(mi, 1);
             _e_menu_item_ensure_onscreen(mi);
          }
        return;
     }
   _e_menu_activate_last();
}

static void
_e_menu_activate_first(void)
{
   E_Menu *m;
   E_Menu_Item *mi;
   Eina_List *ll;

   if (!_e_active_menus) return;
   m = eina_list_data_get(_e_active_menus);
   if (!m->items) return;
   ll = m->items;
   mi = eina_list_data_get(ll);
   while ((mi->separator) && eina_list_next(ll))
     {
        ll = eina_list_next(ll);
        mi = eina_list_data_get(ll);
     }
   if (mi->separator) return;
   e_menu_item_active_set(mi, 1);
   _e_menu_item_ensure_onscreen(mi);
}

static void
_e_menu_activate_last(void)
{
   E_Menu *m;
   E_Menu_Item *mi;
   Eina_List *ll;

   if (!_e_active_menus) return;
   m = eina_list_data_get(_e_active_menus);
   if (!m->items) return;
   ll = eina_list_last(m->items);
   mi = eina_list_data_get(ll);
   while ((mi->separator) && eina_list_prev(ll))
     {
        ll = eina_list_prev(ll);
        mi = eina_list_data_get(ll);
     }
   if (mi->separator) return;
   e_menu_item_active_set(mi, 1);
   _e_menu_item_ensure_onscreen(mi);
}

#if 0
static void
_e_menu_activate_nth(int n)
{
   E_Menu *m;
   E_Menu_Item *mi;
   Eina_List *ll;
   int i = -1;

   mi = _e_menu_item_active_get();
   if (!mi)
     {
        _e_menu_activate_first();
        mi = _e_menu_item_active_get();
        if (!mi) return;
     }
   m = mi->menu;
   EINA_LIST_FOREACH(m->items, ll, mi)
     {
        if (!mi->separator) i++;
        if (i == n)
          {
             e_menu_item_active_set(mi, 1);
             _e_menu_item_ensure_onscreen(mi);
             return;
          }
     }
}

#endif

static E_Menu *
_e_menu_active_get(void)
{
   if (_e_active_menu_item) return _e_active_menu_item->menu;
   return NULL;
}

static E_Menu_Item *
_e_menu_item_active_get(void)
{
   return _e_active_menu_item;
}

static Eina_List *
_e_menu_list_item_active_get(void)
{
   if (_e_active_menu_item)
     return _e_active_menu_item->list_position;
   else
     return NULL;
}

static int
_e_menu_outside_bounds_get(int xdir, int ydir)
{
   Eina_List *l;
   E_Menu *m;
   int outl = 0;
   int outr = 0;
   int outt = 0;
   int outb = 0;
   int i = 0;

   EINA_LIST_FOREACH(_e_active_menus, l, m)
     {
        if (!m->zone) continue;
        if (m->cur.x < m->zone->x + e_config->menu_autoscroll_margin)
          {
             i = m->zone->x - m->cur.x + e_config->menu_autoscroll_margin;
             if (i > outl) outl = i;
          }
        if (m->cur.y < m->zone->y + e_config->menu_autoscroll_margin)
          {
             i = m->zone->y - m->cur.y + e_config->menu_autoscroll_margin;
             if (i > outt) outt = i;
          }
        if ((m->cur.x + m->cur.w) > (m->zone->w - e_config->menu_autoscroll_margin))
          {
             i = m->cur.x + m->cur.w - (m->zone->x + m->zone->w - e_config->menu_autoscroll_margin);
             if (i > outr) outr = i;
          }
        if ((m->cur.y + m->cur.h) > (m->zone->h - e_config->menu_autoscroll_margin))
          {
             i = m->cur.y + m->cur.h - (m->zone->y + m->zone->h - e_config->menu_autoscroll_margin);
             if (i > outb) outb = i;
          }
     }
   if (xdir == -1)
     {
        if (outl) return outl;
     }
   else if (xdir == 1)
     {
        if (outr) return outr;
     }
   else if (ydir == -1)
     {
        if (outt) return outt;
     }
   else if (ydir == 1)
     {
        if (outb) return outb;
     }
   return 0;
}

static void
_e_menu_scroll_by(int dx, int dy)
{
   Eina_List *l;
   E_Menu *m;

   EINA_LIST_FOREACH(_e_active_menus, l, m)
     {
        m->cur.x += dx;
        m->cur.y += dy;
     }
}

static void
_e_menu_mouse_autoscroll_check(void)
{
   int autoscroll_x = 0;
   int autoscroll_y = 0;
   E_Menu *m = NULL;
   int mx, my;

   mx = _e_menu_x;
   my = _e_menu_y;
   if (_e_active_menus)
     {
        m = eina_list_data_get(_e_active_menus);
        if ((m) && (m->zone))
          {
             mx -= m->zone->x;
             my -= m->zone->y;
          }
     }
   if (mx - e_config->menu_autoscroll_cursor_margin <= 0)
     {
        if (_e_menu_outside_bounds_get(-1, 0)) autoscroll_x = -1;
     }
   if (my - e_config->menu_autoscroll_cursor_margin <= 0)
     {
        if (_e_menu_outside_bounds_get(0, -1)) autoscroll_y = -1;
     }
   if ((!autoscroll_x) && (!autoscroll_y))
     {
        if ((m) && (m->zone))
          {
             if (mx + e_config->menu_autoscroll_cursor_margin >= (m->zone->w - 1))
               {
                  if (_e_menu_outside_bounds_get(1, 0)) autoscroll_x = 1;
               }
             if (my + e_config->menu_autoscroll_cursor_margin >= (m->zone->h - 1))
               {
                  if (_e_menu_outside_bounds_get(0, 1)) autoscroll_y = 1;
               }
          }
     }
   _e_menu_autoscroll_x = autoscroll_x;
   _e_menu_autoscroll_y = autoscroll_y;
   if ((!autoscroll_x) && (!autoscroll_y)) return;
   if (_e_menu_scroll_animator) return;
   _e_menu_scroll_animator = ecore_animator_add(_e_menu_cb_scroll_animator,
                                                NULL);
   _e_menu_scroll_start = ecore_loop_time_get();
}

static void
_e_menu_item_ensure_onscreen(E_Menu_Item *mi)
{
   int x = 0, y = 0, w = 0, h = 0;
   int dx = 0, dy = 0;

   if (!mi->menu) return;
   if (!mi->menu->zone) return;
   evas_object_geometry_get(mi->container_object, &x, &y, &w, &h);
   if ((x + w) > (mi->menu->zone->x + mi->menu->zone->w))
     dx = (mi->menu->zone->x + mi->menu->zone->w) - (x + w);
   else if (x < mi->menu->zone->x)
     dx = mi->menu->zone->x - x;
   if ((y + h) > (mi->menu->zone->y + mi->menu->zone->h))
     dy = (mi->menu->zone->y + mi->menu->zone->h) - (y + h);
   else if (y < mi->menu->zone->y)
     dy = mi->menu->zone->y - y;
   if ((dx != 0) || (dy != 0))
     _e_menu_scroll_by(dx, dy);
}

static void
_e_menu_auto_place_vert(E_Menu *m, int x, int y, int w, int h)
{
   int zx, zy, zw, zh;

   if (!m->zone) return;
   e_zone_useful_geometry_get(m->zone, &zx, &zy, &zw, &zh);
   if (E_CONTAINS(zx, zy, zw, zh, m->cur.x, y, m->cur.w, m->cur.h))
     {
        if (w + h > 2)
          {
             if (E_INTERSECTS(m->cur.x, y, m->cur.w, m->cur.h, x, y, w, h))
               m->cur.y = y + h - m->cur.h;
             else
               m->cur.y = y;
          }
        else
          m->cur.y = y;
     }
   else
     m->cur.y = y + h - m->cur.h;
}

static int
_e_menu_auto_place(E_Menu *m, int x, int y, int w, int h)
{
   double xr, yr;

   _e_menu_realize(m);
   /* +-----+
    * |\ T /|
    * | \ / |
    * |L X R|
    * | / \ |
    * |/ B \|
    * +-----+
    *
    * quadrants... which one
    */
   if (!m->zone) return 0;
   if (w != m->zone->w)
     xr = (double)(x - m->zone->x) /
       (double)(m->zone->w - w);
   else
     xr = 0.0;
   if (h != m->zone->h)
     yr = (double)(y - m->zone->y) /
       (double)(m->zone->h - h);
   else
     yr = 0.0;

   if ((xr + yr) < 0.99) /* top or left */
     {
        if (((1.0 - yr) + xr) <= 1.0)
          {
             /* L */
             m->cur.x = x + w;
             _e_menu_auto_place_vert(m, x, y, w, h);
             return 1;
          }
        else
          {
             /* T */
             m->cur.y = y + h;
             if (x < (m->zone->x + ((m->zone->w * 1) / 3)))
               m->cur.x = x;
             else if (x < (m->zone->x + ((m->zone->w * 2) / 3)))
               m->cur.x = x + ((w - m->cur.w) / 2);
             else
               m->cur.x = x + w - m->cur.w;
             return 3;
          }
     }
   else /* bottom or right */
     {
        if (((1.0 - yr) + xr) <= 1.01)
          {
             /* B */
             m->cur.y = y - m->cur.h;
             if (x < (m->zone->x + ((m->zone->w * 1) / 3)))
               m->cur.x = x;
             else if (x < (m->zone->x + ((m->zone->w * 2) / 3)))
               m->cur.x = x + ((w - m->cur.w) / 2);
             else
               m->cur.x = x + w - m->cur.w;
             return 4;
          }
        else
          {
             /* R */
             m->cur.x = x - m->cur.w;
             _e_menu_auto_place_vert(m, x, y, w, h);
             return 2;
          }
     }
   return 0;
}

static void
_e_menu_cb_item_in(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Menu_Item *mi = data;

   if (_e_menu_lock) return;
   // ignore in/out not due to deliberate mouse move by user
   if (_e_menu_nav_by != NAV_BY_MOUSE) return;
   /* this can be triggered when creating menus if the new menu is on top of its parent */
   if (!mi->menu->realized) return;
   e_menu_item_active_set(mi, 1);
}

static void
_e_menu_cb_item_out(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Menu_Item *mi = data;
   // ignore in/out not due to deliberate mouse move by user
   if (_e_menu_nav_by != NAV_BY_MOUSE) return;
   /* this can be triggered when creating menus if the new menu is on top of its parent */
   if (!mi->menu->realized) return;
   e_menu_item_active_set(mi, 0);
}

static Eina_Bool
_e_menu_cb_key_down(void *data EINA_UNUSED, Ecore_Event_Key *ev)
{
   _e_menu_nav_by = NAV_BY_KEY;
   if ((!strcmp(ev->key, "Up")) || (!strcmp(ev->key, "KP_Up")))
     _e_menu_item_activate_previous();
   else if ((!strcmp(ev->key, "Down")) || (!strcmp(ev->key, "KP_Down")))
     _e_menu_item_activate_next();
   else if ((!strcmp(ev->key, "Left")) || (!strcmp(ev->key, "KP_Left")))
     _e_menu_activate_previous();
   else if ((!strcmp(ev->key, "Right")) || (!strcmp(ev->key, "KP_Right")))
     _e_menu_activate_next();
   else if ((!strcmp(ev->key, "Home")) || (!strcmp(ev->key, "KP_Home")))
     _e_menu_item_activate_first();
   else if ((!strcmp(ev->key, "End")) || (!strcmp(ev->key, "KP_End")))
     _e_menu_item_activate_last();
   else if (!strcmp(ev->key, "space"))
     _e_menu_active_call();
   else if ((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter")))
     {
        _e_menu_active_call();
        _e_menu_deactivate_all();
     }
   else if (!strcmp(ev->key, "Escape"))
     _e_menu_deactivate_all();
   else if ((!strcmp(ev->key, "1")) || (!strcmp(ev->key, "KP_1")))
     _e_menu_item_activate_first();
   else if ((!strcmp(ev->key, "2")) || (!strcmp(ev->key, "KP_2")))
     _e_menu_item_activate_nth(1);
   else if ((!strcmp(ev->key, "3")) || (!strcmp(ev->key, "KP_3")))
     _e_menu_item_activate_nth(2);
   else if ((!strcmp(ev->key, "4")) || (!strcmp(ev->key, "KP_4")))
     _e_menu_item_activate_nth(3);
   else if ((!strcmp(ev->key, "5")) || (!strcmp(ev->key, "KP_5")))
     _e_menu_item_activate_nth(4);
   else if ((!strcmp(ev->key, "6")) || (!strcmp(ev->key, "KP_6")))
     _e_menu_item_activate_nth(5);
   else if ((!strcmp(ev->key, "7")) || (!strcmp(ev->key, "KP_7")))
     _e_menu_item_activate_nth(6);
   else if ((!strcmp(ev->key, "8")) || (!strcmp(ev->key, "KP_8")))
     _e_menu_item_activate_nth(7);
   else if ((!strcmp(ev->key, "9")) || (!strcmp(ev->key, "KP_9")))
     _e_menu_item_activate_nth(8);
   else if ((!strcmp(ev->key, "0")) || (!strcmp(ev->key, "KP_0")))
     _e_menu_item_activate_last();
   else if (ev->compose)
     _e_menu_item_activate_char(ev->compose);
   return ECORE_CALLBACK_PASS_ON;
}

/* we need all of these because menus are special and grab the mouse and
 * keyboard and thus the normal event mechanism doesn't work, so we feed
 * events directly to the canvases from our grab window
 */

static void
_e_menu_cb_mouse_evas_down(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Menu *m = data;

   while (m->parent_item)
     m = m->parent_item->menu;
   e_menu_deactivate(m);
}

static Eina_Bool
_e_menu_cb_mouse_down(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Button *ev;

   ev = event;
   _e_menu_nav_by = NAV_BY_MOUSE;
   if (ev->window != _e_menu_win)
     {
        if (_e_menu_active_get())
          _e_menu_deactivate_all();
        return ECORE_CALLBACK_PASS_ON;
     }

   _e_menu_lock = 1;
   e_comp_canvas_feed_mouse_up(0);
   _e_menu_lock = 0;

   /* Only allow dragging from floating menus for now.
    * The reason for this is that for non floating menus,
    * the mouse is already down and dragging, so the decision
    * to start a drag is much more complex.
    */
   if (_e_menu_activate_floating)
     _e_menu_activate_maybe_drag = 1;

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_menu_cb_mouse_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Button *ev;
   E_Menu *m;
   Eina_List *l;
   unsigned int t;
   int ret = 0;

   if (_e_menu_lock) return ECORE_CALLBACK_RENEW;
   ev = event;
   if (ev->window != _e_menu_win) return ECORE_CALLBACK_RENEW;

   _e_menu_nav_by = NAV_BY_MOUSE;
   if (!_e_menu_activate_floating)
     {
        EINA_LIST_FOREACH(_e_active_menus, l, m)
          {
             if (!m->hold_mode)
               {
                  _e_menu_activate_floating = 1;
                  return ECORE_CALLBACK_PASS_ON;
               }
          }
     }

   t = ev->timestamp - _e_menu_activate_time;
   if ((_e_menu_activate_time != 0) &&
       (t < (e_config->menus_click_drag_timeout * 1000)))
     {
        _e_menu_activate_floating = 1;
        return ECORE_CALLBACK_PASS_ON;
     }

   if (_e_menu_activate_dragging)
     {
        /* FIXME: This is a drop, which is not allowed for now.
         * Once dragging is working, this will be subject to some experimentation.
         */
     }
   else
     {
        E_Menu_Item *mi;

        mi = _e_menu_item_active_get();
        if ((!mi) ||
            (E_INSIDE(e_comp_canvas_x_root_adjust(ev->root.x),
                      e_comp_canvas_y_root_adjust(ev->root.y),
                      mi->x, mi->y, mi->w, mi->h))
           )
          ret = _e_menu_active_call();
     }
   _e_menu_activate_maybe_drag = 0;
   _e_menu_activate_dragging = 0;
   if (ret == 1)
     {
/* allow mouse to pop down menu if clicked elsewhere */
/*	if (_e_menu_activate_time != 0) */
        _e_menu_deactivate_all();
     }
   else if (ret == -1)
     _e_menu_deactivate_all();
   else if (!_e_menu_activate_floating)
     _e_menu_deactivate_all();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_menu_cb_mouse_move(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Move *ev;
   Eina_List *l, *tmp;
   E_Menu *m;
   int dx, dy, d;
   double dt;
   double fast_move_threshold;
   int is_fast = 0;

   ev = event;
   if (ev->window != _e_menu_win) return ECORE_CALLBACK_PASS_ON;
   _e_menu_nav_by = NAV_BY_MOUSE;
   fast_move_threshold = e_config->menus_fast_mouse_move_threshhold;
   dx = ev->x - _e_menu_x;
   dy = ev->y - _e_menu_y;
   d = (dx * dx) + (dy * dy);
   dt = (double)(ev->timestamp - _e_menu_time) / 1000.0;
   dt = dt * dt;
   if ((dt > 0.0) && ((d / dt) >= (fast_move_threshold * fast_move_threshold)))
     is_fast = 1;

   tmp = _e_active_menus_copy_ref();

   EINA_LIST_FOREACH(_e_active_menus, l, m)
     {
        if ((!m->realized) || (!m->cur.visible)) continue;
        if (is_fast)
          m->fast_mouse = 1;
        else if (dt > 0.0)
          {
             m->fast_mouse = 0;
             if (m->pending_new_submenu)
               {
                  E_Menu_Item *mi;

                  mi = _e_menu_item_active_get();
                  if (mi)
                    _e_menu_submenu_activate(mi);
               }
          }
       if (!_e_menu_activate_maybe_drag)
         /* this is useless while the mouse is down */
         evas_event_feed_mouse_move(m->evas, ev->x, ev->y, ev->timestamp, NULL);
     }
   if (_e_menu_activate_maybe_drag)
     {
        if (_e_active_menu_item)
          {
             if (!E_INSIDE(ev->x, ev->y, _e_active_menu_item->x, _e_active_menu_item->y, _e_active_menu_item->w, _e_active_menu_item->h))
               {
                  if (_e_active_menu_item->drag_cb.func)
                    {
                       /* User is dragging a draggable item elsewhere. */
                       _e_active_menu_item->drag.x = ev->x - (ev->x - _e_active_menu_item->x);
                       _e_active_menu_item->drag.y = ev->y - (ev->y - _e_active_menu_item->y);
                       _e_menu_deactivate_all();
                       _e_active_menu_item->drag_cb.func(_e_active_menu_item->drag_cb.data, _e_active_menu_item->menu, _e_active_menu_item);
                    }
                  /* Either way, the maybe drag stops here. */
                  _e_menu_activate_maybe_drag = 0;
               }
          }
     }

   _e_menu_list_free_unref(tmp);

   _e_menu_x = ev->x;
   _e_menu_y = ev->y;
   _e_menu_time = ev->timestamp;
   _e_menu_mouse_autoscroll_check();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_menu_cb_mouse_wheel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Wheel *ev;

   ev = event;
   if (ev->window != _e_menu_win) return ECORE_CALLBACK_PASS_ON;
   _e_menu_nav_by = NAV_BY_WHEEL;
   if (ev->z < 0) /* up */
     {
        int i;

        for (i = ev->z; i < 0; i++)
          _e_menu_item_activate_previous();
     }
   else if (ev->z > 0) /* down */
     {
        int i;

        for (i = ev->z; i > 0; i--)
          _e_menu_item_activate_next();
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_menu_cb_scroll_animator(void *data EINA_UNUSED)
{
   double t, dt;
   double dx, dy;
   int out;
   double spd;

   t = ecore_loop_time_get();
   spd = e_config->menus_scroll_speed;
   dt = t - _e_menu_scroll_start;
   _e_menu_scroll_start = t;
   dx = 0;
   dy = 0;
   if (_e_menu_autoscroll_x)
     {
        out = _e_menu_outside_bounds_get(_e_menu_autoscroll_x, 0);
        dx = (-_e_menu_autoscroll_x) * spd * dt;
        if (_e_menu_autoscroll_x == -1)
          {
             if (dx > out) dx = out;
          }
        else
          {
             if (dx < -out) dx = -out;
          }
     }
   if (_e_menu_autoscroll_y)
     {
        out = _e_menu_outside_bounds_get(0, _e_menu_autoscroll_y);
        dy = (-_e_menu_autoscroll_y) * spd * dt;
        if (_e_menu_autoscroll_y == -1)
          {
             if (dy > out) dy = out;
          }
        else
          {
             if (dy < -out) dy = -out;
          }
     }
   _e_menu_scroll_by(dx, dy);
   _e_menu_mouse_autoscroll_check();
   if ((_e_menu_autoscroll_x == 0) && (_e_menu_autoscroll_y == 0))
     {
        _e_menu_scroll_animator = NULL;
        return 0;
     }
   return 1;
}

static void
_e_menu_cb_item_submenu_post_default(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi)
{
   if (!mi->submenu) return;
   e_menu_item_submenu_set(mi, NULL); // unis unrefs previous submenu
}

static void
_e_menu_category_free_cb(E_Menu_Category *cat)
{
   E_FREE_LIST(cat->callbacks, free);
   free(cat);
}

