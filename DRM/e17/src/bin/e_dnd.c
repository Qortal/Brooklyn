#include "e.h"

/* local subsystem functions */

static void           _e_drag_move(E_Drag *drag, int x, int y);
static void           _e_drag_coords_update(const E_Drop_Handler *h, int *dx, int *dy);
static Ecore_Window _e_drag_win_get(const E_Drop_Handler *h, int xdnd);
static int            _e_drag_win_matches(E_Drop_Handler *h, Ecore_Window win, int xdnd);
static void           _e_drag_win_show(E_Drop_Handler *h);
static void           _e_drag_win_hide(E_Drop_Handler *h);
static int            _e_drag_update(Ecore_Window root, int x, int y, Ecore_X_Atom action);
static void           _e_drag_end(int x, int y);
static void           _e_drag_xdnd_end(Ecore_Window root, int x, int y);
static void           _e_drag_free(E_Drag *drag);

static Eina_Bool      _e_dnd_cb_key_down(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_key_up(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_mouse_up(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_mouse_move(void *data, int type, void *event);
#ifndef HAVE_WAYLAND_ONLY
static Eina_Bool      _e_dnd_cb_event_dnd_enter(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_event_dnd_leave(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_event_dnd_position(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_event_dnd_finished(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_event_dnd_drop(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_event_dnd_selection(void *data, int type, void *event);
static Eina_Bool      _e_dnd_cb_event_hide(void *data, int type, Ecore_X_Event_Window_Hide *ev);
#endif

/* local subsystem globals */

typedef struct _XDnd XDnd;

struct _XDnd
{
   int         x, y;
   const char *type;
   void       *data;
};

static Eina_List *_event_handlers = NULL;
static Eina_List *_drop_handlers = NULL;
static Eina_List *_active_handlers = NULL;
static Eina_Hash *_drop_win_hash = NULL;

static Ecore_Window _drag_win = 0;
static Ecore_Window _drag_win_root = 0;

static Eina_List *_drag_list = NULL;
static E_Drag *_drag_current = NULL;
static Ecore_Window drop_win;

static XDnd *_xdnd = NULL;
static Ecore_X_Atom _text_atom = 0;

static Eina_Stringshare *_type_text_uri_list = NULL;
static Eina_Stringshare *_type_xds = NULL;
static Eina_Stringshare *_type_text_x_moz_url = NULL;
static Eina_Stringshare *_type_enlightenment_x_file = NULL;

static Eina_Stringshare **_e_dnd_types[] =
{
   &_type_text_uri_list,
   &_type_xds,
   &_type_text_x_moz_url,
   //&_type_enlightenment_x_file,
   NULL
};

static Eina_Hash *_drop_handlers_responsives;
static Ecore_X_Atom _action;

static void
_e_drop_handler_active_check(E_Drop_Handler *h, const E_Drag *drag, Eina_Stringshare *type)
{
   unsigned int i, j;

   if (h->hidden) return;
   for (i = 0; i < h->num_types; i++)
     {
        if (drag)
          {
             for (j = 0; j < drag->num_types; j++)
               {
                  if (h->types[i] != drag->types[j]) continue;
                  h->active = 1;
                  h->active_type = eina_stringshare_ref(h->types[i]);
                  return;
               }
          }
        else
          {
             if (h->types[i] != type) continue;
             h->active = 1;
             h->active_type = eina_stringshare_ref(h->types[i]);
             return;
          }
     }
}

static int
_e_drag_finalize(E_Drag *drag, E_Drag_Type type, int x, int y)
{
   const Eina_List *l;
   E_Drop_Handler *h;

   if (_drag_win) return 0;
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        _drag_win = ecore_x_window_input_new(e_comp->win,
                                             0, 0,
                                             e_comp->w, e_comp->h);
        ecore_event_window_register(_drag_win, e_comp->ee, e_comp->evas,
                                      NULL, NULL, NULL, NULL);
        ecore_x_window_show(_drag_win);
        _drag_win_root = e_comp->root;
        if (!e_grabinput_get(_drag_win, 0, _drag_win))
          {
             ecore_x_window_free(_drag_win);
             _drag_win = _drag_win_root = 0;
             return 0;
          }
     }
   else
#endif
     {
        _drag_win = _drag_win_root = e_comp->ee_win;
        if (!e_comp_grab_input(1, 1))
          {
             _drag_win = _drag_win_root = 0;
             return 0;
          }
     }

   if (!drag->object)
     {
        e_drag_object_set(drag, evas_object_rectangle_add(drag->evas));
        evas_object_color_set(drag->object, 0, 0, 0, 0);
        //evas_object_color_set(drag->object, 255, 0, 0, 255);
     }
   evas_object_move(drag->comp_object, drag->x, drag->y);
   evas_object_resize(drag->comp_object, drag->w, drag->h);
   drag->visible = 1;
   evas_object_show(drag->comp_object);
   drag->type = type;

   drag->dx = x - drag->x;
   drag->dy = y - drag->y;

   _active_handlers = eina_list_free(_active_handlers);
   EINA_LIST_FOREACH(_drop_handlers, l, h)
     {
        Eina_Bool active = h->active;

        h->active = 0;
        eina_stringshare_replace(&h->active_type, NULL);
        _e_drop_handler_active_check(h, drag, NULL);
        if (h->active != active)
          {
             if (h->active)
               _active_handlers = eina_list_append(_active_handlers, h);
             else
               _active_handlers = eina_list_remove(_active_handlers, h);
          }
        h->entered = 0;
     }

   if (type == E_DRAG_XDND)
     {
#ifndef HAVE_WAYLAND_ONLY
        if (e_comp->comp_type == E_PIXMAP_TYPE_X)
          {
             Ecore_X_Atom actions[] = {
                ECORE_X_DND_ACTION_MOVE, ECORE_X_DND_ACTION_PRIVATE,
                ECORE_X_DND_ACTION_COPY, ECORE_X_DND_ACTION_ASK,
                ECORE_X_DND_ACTION_LINK
             };

             ecore_x_dnd_aware_set(_drag_win, 1);
             ecore_x_dnd_types_set(_drag_win, drag->types, drag->num_types);
             ecore_x_dnd_actions_set(_drag_win, actions, 5);
             ecore_x_dnd_begin(_drag_win, drag->data, drag->data_size);
          }
#endif
#ifdef HAVE_WAYLAND
        if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
          {
          }
#endif
     }
   e_bindings_disabled_set(1);
   _drag_current = drag;
   return 1;
}

/* externally accessible functions */

EINTERN int
e_dnd_init(void)
{
   if (!_event_handlers)
     {
        _type_text_uri_list = eina_stringshare_add("text/uri-list");
        _type_xds = eina_stringshare_add("XdndDirectSave0");
        _type_text_x_moz_url = eina_stringshare_add("text/x-moz-url");
        _type_enlightenment_x_file = eina_stringshare_add("enlightenment/x-file");

        _drop_win_hash = eina_hash_int32_new(NULL);
        _drop_handlers_responsives = eina_hash_int32_new(NULL);

        E_LIST_HANDLER_APPEND(_event_handlers, ECORE_EVENT_MOUSE_BUTTON_UP, _e_dnd_cb_mouse_up, NULL);
        E_LIST_HANDLER_APPEND(_event_handlers, ECORE_EVENT_MOUSE_MOVE, _e_dnd_cb_mouse_move, NULL);
        E_LIST_HANDLER_APPEND(_event_handlers, ECORE_EVENT_KEY_DOWN, _e_dnd_cb_key_down, NULL);
        E_LIST_HANDLER_APPEND(_event_handlers, ECORE_EVENT_KEY_UP, _e_dnd_cb_key_up, NULL);
     }
   if (!e_comp_util_has_x()) return 1;
#ifndef HAVE_WAYLAND_ONLY
   if (_text_atom) return 1;
   _text_atom = ecore_x_atom_get("text/plain");
   E_LIST_HANDLER_APPEND(_event_handlers, ECORE_X_EVENT_XDND_ENTER, _e_dnd_cb_event_dnd_enter, NULL);
   E_LIST_HANDLER_APPEND(_event_handlers, ECORE_X_EVENT_XDND_LEAVE, _e_dnd_cb_event_dnd_leave, NULL);
   E_LIST_HANDLER_APPEND(_event_handlers, ECORE_X_EVENT_XDND_POSITION, _e_dnd_cb_event_dnd_position, NULL);
   E_LIST_HANDLER_APPEND(_event_handlers, ECORE_X_EVENT_XDND_FINISHED, _e_dnd_cb_event_dnd_finished, NULL);
   E_LIST_HANDLER_APPEND(_event_handlers, ECORE_X_EVENT_XDND_DROP, _e_dnd_cb_event_dnd_drop, NULL);
   E_LIST_HANDLER_APPEND(_event_handlers, ECORE_X_EVENT_SELECTION_NOTIFY, _e_dnd_cb_event_dnd_selection, NULL);
   E_LIST_HANDLER_APPEND(_event_handlers, ECORE_X_EVENT_WINDOW_HIDE, _e_dnd_cb_event_hide, NULL);

   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     e_drop_xdnd_register_set(e_comp->ee_win, 1);
   else
     e_drop_xdnd_register_set(e_comp->cm_selection, 1);

   _action = ECORE_X_ATOM_XDND_ACTION_PRIVATE;
#endif
   return 1;
}

EINTERN int
e_dnd_shutdown(void)
{
   E_FREE_LIST(_drag_list, e_object_del);

   _active_handlers = eina_list_free(_active_handlers);
   E_FREE_LIST(_drop_handlers, e_drop_handler_del);

   E_FREE_LIST(_event_handlers, ecore_event_handler_del);

   eina_hash_free(_drop_win_hash);

   eina_hash_free(_drop_handlers_responsives);

   eina_stringshare_del(_type_text_uri_list);
   eina_stringshare_del(_type_xds);
   eina_stringshare_del(_type_text_x_moz_url);
   eina_stringshare_del(_type_enlightenment_x_file);
   _type_text_uri_list = NULL;
   _type_xds = NULL;
   _type_text_x_moz_url = NULL;
   _type_enlightenment_x_file = NULL;
   _text_atom = 0;

   return 1;
}

E_API E_Drag *
e_drag_current_get(void)
{
   return _drag_current;
}

E_API E_Drag *
e_drag_new(int x, int y,
           const char **types, unsigned int num_types,
           void *data, int size,
           void *(*convert_cb)(E_Drag * drag, const char *type),
           void (*finished_cb)(E_Drag *drag, int dropped))
{
   E_Drag *drag;
   unsigned int i;

   drag = e_object_alloc(sizeof(E_Drag) + num_types * sizeof(char *),
                         E_DRAG_TYPE, E_OBJECT_CLEANUP_FUNC(_e_drag_free));
   if (!drag) return NULL;

   drag->x = x;
   drag->y = y;
   drag->w = 24;
   drag->h = 24;
   drag->layer = E_LAYER_CLIENT_DRAG;

   drag->evas = e_comp->evas;

   drag->type = E_DRAG_NONE;

   for (i = 0; i < num_types; i++)
     drag->types[i] = eina_stringshare_add(types[i]);
   drag->num_types = num_types;
   drag->data = data;
   drag->data_size = size;
   drag->cb.convert = convert_cb;
   drag->cb.finished = finished_cb;

   _drag_list = eina_list_append(_drag_list, drag);

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     ecore_x_window_shadow_tree_flush();
#endif

   _drag_win_root = e_comp->root;

   drag->cb.key_down = NULL;
   drag->cb.key_up = NULL;

   return drag;
}

E_API Evas *
e_drag_evas_get(const E_Drag *drag)
{
   return drag->evas;
}

E_API void
e_drag_object_set(E_Drag *drag, Evas_Object *object)
{
   EINA_SAFETY_ON_NULL_RETURN(object);
   EINA_SAFETY_ON_TRUE_RETURN(!!drag->object);
   if (drag->visible)
     evas_object_show(object);
   else
     evas_object_hide(object);
   drag->object = object;
   drag->comp_object = e_comp_object_util_add(object, E_COMP_OBJECT_TYPE_NONE);
   evas_object_layer_set(drag->comp_object, drag->layer);
   evas_object_name_set(drag->comp_object, "E Drag");
   evas_object_pass_events_set(drag->comp_object, 1);
}

E_API void
e_drag_move(E_Drag *drag, int x, int y)
{
   if ((drag->x == x) && (drag->y == y)) return;
   drag->x = x;
   drag->y = y;
   if (_drag_current == drag)
     evas_object_move(drag->comp_object, x, y);
}

E_API void
e_drag_resize(E_Drag *drag, int w, int h)
{
   if ((drag->w == w) && (drag->h == h)) return;
   drag->h = h;
   drag->w = w;
   if (_drag_current == drag)
     evas_object_resize(drag->comp_object, w, h);
}

E_API int
e_dnd_active(void)
{
   return _drag_win != 0;
}

E_API int
e_drag_start(E_Drag *drag, int x, int y)
{
   return _e_drag_finalize(drag, E_DRAG_INTERNAL, x, y);
}

E_API int
e_drag_xdnd_start(E_Drag *drag, int x, int y)
{
   return _e_drag_finalize(drag, E_DRAG_XDND, x, y);
}

E_API void
e_drop_handler_xds_set(E_Drop_Handler *handler, Eina_Bool (*cb)(void *data, const char *type))
{
   handler->cb.xds = cb;
}

/* should only be used for windows */
E_API void
e_drop_xds_update(Eina_Bool enable, const char *value)
{
#ifndef HAVE_WAYLAND_ONLY
   Ecore_Window xwin;
   char buf[PATH_MAX + 8];
   char *file;
   int size;
   size_t len;

   if (!e_comp_util_has_x()) return;
   enable = !!enable;

   xwin = ecore_x_selection_owner_get(ECORE_X_ATOM_SELECTION_XDND);
   if (enable)
     {
        if (!ecore_x_window_prop_property_get(xwin, ECORE_X_ATOM_XDND_DIRECTSAVE0, _text_atom, 8, (unsigned char **)&file, &size))
          return;
        len = strlen(value);
        if (size + len + 8 + 1 > sizeof(buf))
          {
             free(file);
             return;
          }
        snprintf(buf, sizeof(buf), "file://%s/", value);
        strncat(buf, file, size);
        free(file);
        ecore_x_window_prop_property_set(xwin, ECORE_X_ATOM_XDND_DIRECTSAVE0, _text_atom, 8, (void *)buf, size + len + 8);
     }
   else
     ecore_x_window_prop_property_del(xwin, ECORE_X_ATOM_XDND_DIRECTSAVE0);
#else
   (void)enable;
   (void)value;
#endif
}

E_API E_Drop_Handler *
e_drop_handler_add(E_Object *obj, Evas_Object *win,
                   void *data,
                   void (*enter_cb)(void *data, const char *type, void *event),
                   void (*move_cb)(void *data, const char *type, void *event),
                   void (*leave_cb)(void *data, const char *type, void *event),
                   void (*drop_cb)(void *data, const char *type, void *event),
                   const char **types, unsigned int num_types, int x, int y, int w, int h)
{
   E_Drop_Handler *handler;
   unsigned int i;

   handler = calloc(1, sizeof(E_Drop_Handler) + num_types * sizeof(char *));
   if (!handler) return NULL;

   handler->cb.data = data;
   handler->cb.enter = enter_cb;
   handler->cb.move = move_cb;
   handler->cb.leave = leave_cb;
   handler->cb.drop = drop_cb;
   handler->num_types = num_types;
   for (i = 0; i < num_types; i++)
     handler->types[i] = eina_stringshare_add(types[i]);
   handler->x = x;
   handler->y = y;
   handler->w = w;
   handler->h = h;

   handler->obj = obj;
   handler->win = win;
   handler->entered = 0;

   _drop_handlers = eina_list_append(_drop_handlers, handler);

   return handler;
}

E_API void
e_drop_handler_geometry_set(E_Drop_Handler *handler, int x, int y, int w, int h)
{
   handler->x = x;
   handler->y = y;
   handler->w = w;
   handler->h = h;
}

E_API int
e_drop_inside(const E_Drop_Handler *handler, int x, int y)
{
   int dx, dy;

   _e_drag_coords_update(handler, &dx, &dy);
   x -= dx;
   y -= dy;
   return E_INSIDE(x, y, handler->x, handler->y, handler->w, handler->h);
}

E_API void
e_drop_handler_del(E_Drop_Handler *handler)
{
   unsigned int i;
   Eina_List *l;
   Ecore_Window hwin;

   if (!handler)
     return;

   hwin = _e_drag_win_get(handler, 1);
   if (hwin)
     {
        l = eina_hash_find(_drop_handlers_responsives, &hwin);
        if (l)
          eina_hash_set(_drop_handlers_responsives, &hwin, eina_list_remove(l, handler));
     }
   _drop_handlers = eina_list_remove(_drop_handlers, handler);
   if (handler->active)
     _active_handlers = eina_list_remove(_active_handlers, handler);
   for (i = 0; i < handler->num_types; i++)
     eina_stringshare_del(handler->types[i]);
   eina_stringshare_del(handler->active_type);
   free(handler);
}

E_API int
e_drop_xdnd_register_set(Ecore_Window win, int reg)
{
   if (!e_comp_util_has_x()) return 0;
   if (reg)
     {
        if (!eina_hash_find(_drop_win_hash, &win))
          {
#ifndef HAVE_WAYLAND_ONLY
             ecore_x_dnd_aware_set(win, 1);
#endif
             eina_hash_add(_drop_win_hash, &win, (void *)1);
          }
     }
   else
     {
#ifndef HAVE_WAYLAND_ONLY
        ecore_x_dnd_aware_set(win, 0);
#endif
        eina_hash_del(_drop_win_hash, &win, (void *)1);
     }
   return 1;
}

E_API void
e_drop_handler_responsive_set(E_Drop_Handler *handler)
{
   Ecore_Window hwin = _e_drag_win_get(handler, 1);
   Eina_List *l;

   l = eina_hash_find(_drop_handlers_responsives, &hwin);
   eina_hash_set(_drop_handlers_responsives, &hwin, eina_list_append(l, handler));
}

E_API int
e_drop_handler_responsive_get(const E_Drop_Handler *handler)
{
   Ecore_Window hwin = _e_drag_win_get(handler, 1);
   Eina_List *l;

   l = eina_hash_find(_drop_handlers_responsives, &hwin);
   return l && eina_list_data_find(l, handler);
}

E_API void
e_drop_handler_action_set(unsigned int action)
{
   _action = action;
}

E_API unsigned int
e_drop_handler_action_get(void)
{
   return _action;
}

E_API void
e_drag_key_down_cb_set(E_Drag *drag, void (*func)(E_Drag *drag, Ecore_Event_Key *e))
{
   drag->cb.key_down = func;
}

E_API void
e_drag_key_up_cb_set(E_Drag *drag, void (*func)(E_Drag *drag, Ecore_Event_Key *e))
{
   drag->cb.key_up = func;
}

/* from ecore_x_selection.c */
E_API Eina_List *
e_dnd_util_text_uri_list_convert(char *data, int size)
{
   char *tmp;
   int i, is;
   Eina_List *ret = NULL;

   if ((!data) || (!size)) return NULL;
   tmp = malloc(size);
   is = i = 0;
   while ((is < size) && (data[is]))
     {
        if ((i == 0) && (data[is] == '#'))
          for (; ((data[is]) && (data[is] != '\n')); is++) ;
        else
          {
             if ((data[is] != '\r') &&
                 (data[is] != '\n'))
               tmp[i++] = data[is++];
             else
               {
                  while ((data[is] == '\r') || (data[is] == '\n'))
                    is++;
                  tmp[i] = 0;
                  ret = eina_list_append(ret, strdup(tmp));
                  tmp[0] = 0;
                  i = 0;
               }
          }
     }
   if (i > 0)
     {
        tmp[i] = 0;
        ret = eina_list_append(ret, strdup(tmp));
     }

   free(tmp);

   return ret;
}

/* local subsystem functions */

static Eina_Stringshare *
_e_dnd_type_implemented(const char *type)
{
   const char ***t;

   for (t = _e_dnd_types; *t; t++)
     {
        if (!strcmp(type, **t))
          return **t;
     }
   return NULL;
}

static void
_e_drag_move(E_Drag *drag, int x, int y)
{
   E_Zone *zone;

   if (((drag->x + drag->dx) == x) && ((drag->y + drag->dy) == y)) return;

   zone = e_comp_zone_xy_get(x, y);
   if (zone) e_zone_flip_coords_handle(zone, x, y);

   drag->x = x - drag->dx;
   drag->y = y - drag->dy;
   evas_object_move(drag->comp_object, drag->x, drag->y);
}

static void
_e_drag_coords_update(const E_Drop_Handler *h, int *dx, int *dy)
{
   int px = 0, py = 0;

   *dx = 0;
   *dy = 0;
   if (h->win)
     {
        E_Client *ec;

        ec = e_win_client_get(h->win);
        px = ec->x;
        py = ec->y;
     }
   else if (h->obj)
     {
        switch (h->obj->type)
          {
             E_Gadcon *gc;

           case E_GADCON_TYPE:
             gc = (E_Gadcon *)h->obj;
             if (!gc->toolbar) return;
             evas_object_geometry_get(gc->toolbar->fwin, &px, &py, NULL, NULL);
             break;

           case E_GADCON_CLIENT_TYPE:
             gc = ((E_Gadcon_Client *)(void *)(h->obj))->gadcon;
             e_gadcon_canvas_zone_geometry_get(gc, &px, &py, NULL, NULL);
             if (!gc->toolbar) break;
             {
                int x, y;

                evas_object_geometry_get(gc->toolbar->fwin, &x, &y, NULL, NULL);
                px += x, py += y;
             }
             break;

           case E_ZONE_TYPE:
// zone based drag targets are in a comp thus their coords should be
// screen-relative as containers just cover the screen
//	     px = ((E_Zone *)(h->obj))->x;
//	     py = ((E_Zone *)(h->obj))->y;
             break;

           case E_CLIENT_TYPE:
             px = ((E_Client *)(void *)(h->obj))->x;
             py = ((E_Client *)(void *)(h->obj))->y;
             break;

           /* FIXME: add more types as needed */
           default:
             break;
          }
     }
   *dx += px;
   *dy += py;
}

static Ecore_Window
_e_drag_win_get(const E_Drop_Handler *h, int xdnd)
{
   Ecore_Window hwin = 0;

   if (h->win)
     return elm_win_window_id_get(h->win);
   if (h->obj)
     {
        E_Gadcon *gc = NULL;

        switch (h->obj->type)
          {
           case E_GADCON_CLIENT_TYPE:
             gc = ((E_Gadcon_Client *)(void *)(h->obj))->gadcon;
             if (!gc) return 0;
             EINA_FALLTHROUGH;
             /* no break */

           case E_GADCON_TYPE:
             if (!gc) gc = (E_Gadcon *)h->obj;

             if (gc->toolbar) hwin = e_client_util_pwin_get(e_win_client_get(gc->toolbar->fwin)); //double check for xdnd...
             else
               {
                  if (xdnd) hwin = e_gadcon_xdnd_window_get(gc);
                  else hwin = e_gadcon_dnd_window_get(gc);
               }
             break;

           case E_CLIENT_TYPE:
           case E_ZONE_TYPE:
           default:
             /* protect against crashes during shutdown */
             if (e_comp)
               hwin = e_comp->ee_win;
             break;
          }
     }

   return hwin;
}

static int
_e_drag_win_matches(E_Drop_Handler *h, Ecore_Window win, int xdnd)
{
   Ecore_Window hwin = _e_drag_win_get(h, xdnd);

   if (win == hwin) return 1;
   return 0;
}

static void
_e_drag_win_show(E_Drop_Handler *h)
{
   E_Shelf *shelf;

   if (h->win) return;
   if (h->obj)
     {
        switch (h->obj->type)
          {
           case E_GADCON_TYPE:
             shelf = e_gadcon_shelf_get((E_Gadcon *)(h->obj));
             if (shelf) e_shelf_toggle(shelf, 1);
             break;

           case E_GADCON_CLIENT_TYPE:
             shelf = e_gadcon_shelf_get(((E_Gadcon_Client *)(void *)(h->obj))->gadcon);
             if (shelf) e_shelf_toggle(shelf, 1);
             break;

           /* FIXME: add more types as needed */
           default:
             break;
          }
     }
}

static void
_e_drag_win_hide(E_Drop_Handler *h)
{
   E_Shelf *shelf;

   if (h->win) return;
   if (h->obj)
     {
        switch (h->obj->type)
          {
           case E_GADCON_TYPE:
             shelf = e_gadcon_shelf_get((E_Gadcon *)(h->obj));
             if (shelf) e_shelf_toggle(shelf, 0);
             break;

           case E_GADCON_CLIENT_TYPE:
             shelf = e_gadcon_shelf_get(((E_Gadcon_Client *)(void *)(h->obj))->gadcon);
             if (shelf) e_shelf_toggle(shelf, 0);
             break;

           /* FIXME: add more types as needed */
           default:
             break;
          }
     }
}

static unsigned int
_e_dnd_object_layer_get(E_Drop_Handler *h)
{
   unsigned int adjust = 0;
   E_Object *obj = h->obj;

   if (h->base) return evas_object_layer_get(h->base);
   if (!obj) return 0;
   if (h->win)
     obj = (E_Object*)e_win_client_get(h->win);
   switch (obj->type)
     {
      case E_GADCON_CLIENT_TYPE:
        /* add 1 to ensure we're above a potential receiving gadcon */
        adjust = 1;
        EINA_FALLTHROUGH;
        /* no break */

      default:
        adjust += e_comp_e_object_layer_get(obj);
     }
   return adjust;
}

static Ecore_Window
_dnd_top_window_at_xy_get(Evas_Coord x, Evas_Coord y)
{
   E_Client *ec;
   Eina_List *objs, *l;
   Evas_Object *o;

   if (_drag_current->type == E_DRAG_INTERNAL)
     return e_comp_top_window_at_xy_get(x, y);
   objs = evas_objects_at_xy_get(e_comp->evas, x, y, 0, 0);
   if (!objs) return e_comp->ee_win;
   EINA_LIST_FOREACH(objs, l, o)
     {
        ec = evas_object_data_get(o, "E_Client");
        if (ec)
          {
             eina_list_free(objs);
             return e_client_util_pwin_get(ec);
          }
     }
   eina_list_free(objs);
   return e_comp->ee_win;
}

static int
_e_drag_update(Ecore_Window root, int x, int y, unsigned int action)
{
   const Eina_List *l;
   Eina_List *entered = NULL;
   E_Event_Dnd_Enter enter_ev;
   E_Event_Dnd_Move move_ev;
   E_Event_Dnd_Leave leave_ev;
   E_Drop_Handler *h, *top = NULL;
   unsigned int top_layer = 0;
   int dx, dy;
   Ecore_Window win;
   int responsive = 0;

//   double t1 = ecore_time_get(); ////
   if (_drag_current && !_xdnd)
     win = _dnd_top_window_at_xy_get(x, y);
   else
     win = root;

   if (_drag_current)
     {
        if (_drag_current->ended) return 0;
        if (_drag_current->visible) evas_object_show(_drag_current->comp_object);
        else evas_object_hide(_drag_current->comp_object);
        _e_drag_move(_drag_current, x, y);
     }
   EINA_LIST_FOREACH(_active_handlers, l, h)
     {
        _e_drag_coords_update(h, &dx, &dy);
        enter_ev.x = x - dx;
        enter_ev.y = y - dy;
        enter_ev.data = NULL;
        enter_ev.action = action;
        move_ev.x = x - dx;
        move_ev.y = y - dy;
        move_ev.action = action;
        leave_ev.x = x - dx;
        leave_ev.y = y - dy;

        if (E_INSIDE(enter_ev.x, enter_ev.y, h->x, h->y, h->w, h->h) &&
            ((!_drag_current) || _e_drag_win_matches(h, win, 0)))
          entered = eina_list_append(entered, h);
        else
          {
             if (h->entered)
               {
                  if (h->cb.leave)
                    h->cb.leave(h->cb.data, h->active_type, &leave_ev);
                  if (_drag_current)
                    _e_drag_win_hide(h);
                  h->entered = 0;
               }
          }
     }
   if (!entered) return 0;

   EINA_LIST_FREE(entered, h)
     {
        unsigned int layer;
        E_Drop_Handler *h2;

        _e_drag_coords_update(h, &dx, &dy);
        leave_ev.x = x - dx;
        leave_ev.y = y - dy;

        layer = _e_dnd_object_layer_get(h);
        if (!top)
          {
             top = h;
             top_layer = layer;
             enter_ev.x = x - dx;
             enter_ev.y = y - dy;
             enter_ev.data = NULL;
             enter_ev.action = action;
             move_ev.x = x - dx;
             move_ev.y = y - dy;
             move_ev.action = action;
             continue;
          }
        if (layer > top_layer)
          {
             h2 = top, top = h, h = h2;
             enter_ev.x = x - dx;
             enter_ev.y = y - dy;
             enter_ev.data = NULL;
             enter_ev.action = action;
             move_ev.x = x - dx;
             move_ev.y = y - dy;
             move_ev.action = action;
          }
        if (h == top) continue;
        if (h->entered)
          {
             if (h->cb.leave)
               h->cb.leave(h->cb.data, h->active_type, &leave_ev);
             if (_drag_current)
               _e_drag_win_hide(h);
             h->entered = 0;
          }
     }
   responsive = !!e_drop_handler_responsive_get(top);
   if (!top->entered)
     {
        _e_drag_win_show(top);
        if (top->cb.enter)
          {
             if (_drag_current)
               {
                  if (_drag_current->cb.convert)
                    {
                       enter_ev.data = _drag_current->cb.convert(_drag_current,
                                                                 top->active_type);
                    }
                  else
                    enter_ev.data = _drag_current->data;
               }
             top->cb.enter(top->cb.data, top->active_type, &enter_ev);
          }
        top->entered = 1;
     }
   if (top->cb.move)
     top->cb.move(top->cb.data, top->active_type, &move_ev);
   return responsive;
//   double t2 = ecore_time_get() - t1; ////
//   printf("DND UPDATE %3.7f\n", t2); ////
}

static Eina_Bool
_drag_timeout(void *data)
{
   E_Drag *drag = data;

   drag->timeout = NULL;
   ERR("Drop finished response timeout...");
   e_object_del(E_OBJECT(drag));
   return EINA_FALSE;
}

static void
_e_drag_end(int x, int y)
{
   E_Zone *zone;
   const Eina_List *l;
   E_Event_Dnd_Drop ev;
   int dx, dy;
   Ecore_Window win;
   E_Drop_Handler *h;
   int dropped = 0;

   if (!_drag_current) return;
   win = _dnd_top_window_at_xy_get(x, y);
   zone = e_comp_zone_xy_get(x, y);
   /* Pass -1, -1, so that it is possible to drop at the edge. */
   if (zone) e_zone_flip_coords_handle(zone, -1, -1);

   evas_object_hide(_drag_current->comp_object);

   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     e_grabinput_release(_drag_win, _drag_win);

   while (_drag_current->type == E_DRAG_XDND)
     {
#ifndef HAVE_WAYLAND_ONLY
        if (e_comp->comp_type == E_PIXMAP_TYPE_X)
          {
             if (!(dropped = ecore_x_dnd_drop()))
               break;
             else
               drop_win = win;
          }
        else
#endif
          if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
            break;
        if (_drag_current->cb.finished)
          _drag_current->cb.finished(_drag_current, dropped);
        _drag_current->cb.finished = NULL;
        _drag_current->ended = 1;
        _drag_current->timeout = ecore_timer_add(2.0, _drag_timeout, _drag_current);
        return;
     }

   dropped = 0;
   if (!_drag_current->data)
     {
        /* Just leave */
        E_Event_Dnd_Leave leave_ev;

        leave_ev.x = x;
        leave_ev.y = y;

        EINA_LIST_FOREACH(_active_handlers, l, h)
          {
             if (h->entered)
               {
                  if (h->cb.leave)
                    h->cb.leave(h->cb.data, h->active_type, &leave_ev);
                  h->entered = 0;
               }
          }
     }

   EINA_LIST_FOREACH(_active_handlers, l, h)
     {
        if (!h->entered) continue;
        _e_drag_coords_update(h, &dx, &dy);
        ev.x = x - dx;
        ev.y = y - dy;
        if ((_e_drag_win_matches(h, win, 0)) &&
            ((h->cb.drop) && (E_INSIDE(ev.x, ev.y, h->x, h->y, h->w, h->h))))
          {
             Eina_Bool need_free = EINA_FALSE;
             Eina_List *list;

             if (_drag_current->cb.convert)
               {
                  ev.data = _drag_current->cb.convert(_drag_current,
                                                      h->active_type);
               }
             else
               {
                  unsigned int i;

                  for (i = 0; i < _drag_current->num_types; i++)
                    if (_drag_current->types[i] == _type_text_uri_list)
                      {
                         char *data = _drag_current->data;
                         int size = _drag_current->data_size;

                         if (data && data[size - 1])
                           {
                              /* Isn't nul terminated */
                              size++;
                              data = realloc(data, size);
                              data[size - 1] = 0;
                           }
                         _drag_current->data = data;
                         _drag_current->data_size = size;
                         ev.data = e_dnd_util_text_uri_list_convert(_drag_current->data, _drag_current->data_size);
                         need_free = EINA_TRUE;
                         break;
                      }
                  if (!need_free)
                    ev.data = _drag_current->data;
               }
             h->cb.drop(h->cb.data, h->active_type, &ev);
             list = ev.data;
             if (need_free) E_FREE_LIST(list, free);
             dropped = 1;
          }
        h->entered = 0;
        if (dropped) break;
     }
   if (_drag_current->cb.finished)
     _drag_current->cb.finished(_drag_current, dropped);
   _drag_current->cb.finished = NULL;

   e_object_del(E_OBJECT(_drag_current));
}

static void
_e_drag_xdnd_end(Ecore_Window win, int x, int y)
{
   const Eina_List *l;
   E_Event_Dnd_Drop ev;
   int dx, dy;

   if (!_xdnd) return;

   ev.data = _xdnd->data;

   if (ev.data)
     {
        E_Drop_Handler *h;

        EINA_LIST_FOREACH(_active_handlers, l, h)
          {
             _e_drag_coords_update(h, &dx, &dy);
             ev.x = x - dx;
             ev.y = y - dy;
             if (_e_drag_win_matches(h, win, 1) && h->cb.drop
                 && E_INSIDE(ev.x, ev.y, h->x, h->y, h->w, h->h))
               {
                  h->cb.drop(h->cb.data, h->active_type, &ev);
               }
          }
     }
   if (_drag_current) e_object_del(E_OBJECT(_drag_current));
}

static void
_e_drag_free(E_Drag *drag)
{
   unsigned int i;

   if (drag->timeout)
     {
        ecore_timer_del(drag->timeout);
        drag->timeout = NULL;
     }
   if (drag == _drag_current)
     {
        E_Event_Dnd_Leave leave_ev;
        E_Drop_Handler *h;

        e_grabinput_release(_drag_win, _drag_win);
        _drag_win_root = 0;

        leave_ev.x = 0;
        leave_ev.y = 0;
        EINA_LIST_FREE(_active_handlers, h)
          {
             if (h->entered)
               {
                  if (h->cb.leave)
                    h->cb.leave(h->cb.data, h->active_type, &leave_ev);
                  _e_drag_win_hide(h);
               }
             h->active = 0;
          }
        if (drag->cb.finished)
          drag->cb.finished(drag, 0);
        drag->cb.finished = NULL;
        drop_win = 0;
     }

   _drag_current = NULL;

   _drag_list = eina_list_remove(_drag_list, drag);

   evas_object_hide(drag->comp_object);
   E_FREE_FUNC(drag->comp_object, evas_object_del);
   for (i = 0; i < drag->num_types; i++)
     eina_stringshare_del(drag->types[i]);
   free(drag);
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        ecore_event_window_unregister(_drag_win);
        if (_drag_win != e_comp->ee_win)
          ecore_x_window_free(_drag_win);
        ecore_x_window_shadow_tree_flush();
     }
   else
#endif
     {
        e_comp_ungrab_input(1, 1);
     }
   e_bindings_disabled_set(0);
   _drag_win = 0;
}

static Eina_Bool
_e_dnd_cb_key_down(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Key *ev = event;

   if (ev->window != _drag_win) return ECORE_CALLBACK_PASS_ON;

   if (!_drag_current) return ECORE_CALLBACK_PASS_ON;

   if (_drag_current->cb.key_down)
     _drag_current->cb.key_down(_drag_current, ev);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_key_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Key *ev = event;

   if (ev->window != _drag_win) return ECORE_CALLBACK_PASS_ON;

   if (!_drag_current) return ECORE_CALLBACK_PASS_ON;

   if (_drag_current->cb.key_up)
     _drag_current->cb.key_up(_drag_current, ev);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_mouse_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Button *ev = event;

   if (ev->window != _drag_win) return ECORE_CALLBACK_PASS_ON;

   if (_drag_current && _drag_current->button_mask)
     {
        _drag_current->button_mask &= ~(1 << (ev->buttons - 1));
        if (_drag_current->button_mask) return ECORE_CALLBACK_RENEW;
     }
   _e_drag_end(ev->x, ev->y);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_mouse_move(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Move *ev = event;

   if (ev->window != _drag_win) return ECORE_CALLBACK_PASS_ON;

#ifndef HAVE_WAYLAND_ONLY
   if (!_xdnd)
     _e_drag_update(_drag_win_root, ev->x, ev->y,
                    _action ?: ECORE_X_ATOM_XDND_ACTION_PRIVATE);
# ifdef HAVE_WAYLAND
   if (e_comp_util_has_xwayland())
     {
        if (e_comp_wl->drag != _drag_current) return ECORE_CALLBACK_RENEW;
        if (!e_comp_wl->ptr.ec) return ECORE_CALLBACK_RENEW;
        if (!e_client_has_xwindow(e_comp_wl->ptr.ec)) return ECORE_CALLBACK_RENEW;
        if (e_client_has_xwindow(e_comp_wl->drag_client)) return ECORE_CALLBACK_RENEW;
        ecore_x_client_message32_send(e_client_util_win_get(e_comp_wl->ptr.ec),
          ECORE_X_ATOM_XDND_POSITION, ECORE_X_EVENT_MASK_NONE,
          e_comp->cm_selection, 0, ((ev->x << 16) & 0xffff0000) | (ev->y & 0xffff),
          ev->timestamp, ECORE_X_ATOM_XDND_ACTION_COPY);
     }
# endif
#endif

   return ECORE_CALLBACK_PASS_ON;
}

#ifndef HAVE_WAYLAND_ONLY
static Eina_Bool
_e_dnd_cb_event_dnd_enter(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_X_Event_Xdnd_Enter *ev = event;
   E_Drop_Handler *h;
   const Eina_List *l;
   int i;

   if (!eina_hash_find(_drop_win_hash, &ev->win)) return ECORE_CALLBACK_PASS_ON;

   EINA_LIST_FREE(_active_handlers, h)
     {
        h->active = 0;
        eina_stringshare_replace(&h->active_type, NULL);
        h->entered = 0;
     }
   for (i = 0; i < ev->num_types; i++)
     {
        Eina_Stringshare *t;

        t = eina_stringshare_ref(_e_dnd_type_implemented(ev->types[i]));
        if (!t) continue;
        _xdnd = E_NEW(XDnd, 1);
        _xdnd->type = t;
        EINA_LIST_FOREACH(_drop_handlers, l, h)
          {
             _e_drop_handler_active_check(h, NULL, _xdnd->type);
             if (h->active)
               _active_handlers = eina_list_append(_active_handlers, h);
             h->entered = 0;
          }
        break;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_event_dnd_leave(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_X_Event_Xdnd_Leave *ev = event;
   E_Event_Dnd_Leave leave_ev;
   const Eina_List *l;

   if (!eina_hash_find(_drop_win_hash, &ev->win)) return ECORE_CALLBACK_PASS_ON;

   leave_ev.x = 0;
   leave_ev.y = 0;

   if (_xdnd)
     {
        E_Drop_Handler *h;

        EINA_LIST_FOREACH(_active_handlers, l, h)
          {
             if (h->entered)
               {
                  if (h->cb.leave)
                    h->cb.leave(h->cb.data, h->active_type, &leave_ev);
                  h->entered = 0;
               }
          }

        eina_stringshare_del(_xdnd->type);
        E_FREE(_xdnd);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_event_hide(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_X_Event_Window_Hide *ev)
{
   E_Event_Dnd_Leave leave_ev;
   const char *id;
   const Eina_List *l;

   id = e_util_winid_str_get(ev->win);
   if (!eina_hash_find(_drop_win_hash, id))
     {
        if (_drag_current && _drag_current->ended && (drop_win == ev->win))
          e_object_del(E_OBJECT(_drag_current));
        return ECORE_CALLBACK_RENEW;
     }
   leave_ev.x = 0;
   leave_ev.y = 0;

   if (_xdnd)
     {
        unsigned int entered = 0;
        E_Drop_Handler *h;

        EINA_LIST_FOREACH(_active_handlers, l, h)
          {
             if (h->entered && (_e_drag_win_get(h, 1) == ev->win))
               {
                  if (h->cb.leave)
                    h->cb.leave(h->cb.data, h->active_type, &leave_ev);
                  h->entered = 0;
               }
             entered += h->entered;
          }

        if (!entered)
          {
             eina_stringshare_del(_xdnd->type);
             E_FREE(_xdnd);
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_dnd_cb_event_dnd_position(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_X_Event_Xdnd_Position *ev = event;
   Ecore_X_Rectangle rect;
   int responsive;

//   double t1 = ecore_time_get(); ////
   if (!eina_hash_find(_drop_win_hash, &ev->win))
     {
//	double t2 = ecore_time_get() - t1; ////
//	printf("DND POS EV 1 %3.7f\n", t2); ////
        return ECORE_CALLBACK_PASS_ON;
     }

   rect.x = 0;
   rect.y = 0;
   rect.width = 0;
   rect.height = 0;

   if (!_active_handlers)
     ecore_x_dnd_send_status(0, 0, rect, ECORE_X_DND_ACTION_PRIVATE);
   else
     {
        responsive = _e_drag_update(ev->win, ev->position.x, ev->position.y, ev->action);
        if (responsive)
          ecore_x_dnd_send_status(1, 0, rect, _action);
        else
          ecore_x_dnd_send_status(1, 0, rect, ECORE_X_ATOM_XDND_ACTION_PRIVATE);
     }
//   double t2 = ecore_time_get() - t1; ////
//   printf("DND POS EV 2 %3.7f\n", t2); ////
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_event_dnd_finished(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
/*
 * this is broken since the completed flag doesn't tell us anything with current
 * ecore-x and results in never-ending dnd operation which breaks the window
 * 18 September 2012
 * BORKER CERTIFICATION: BRONZE
 * -discomfitor
   Ecore_X_Event_Xdnd_Finished *ev;

   ev = event;

   if (!ev->completed) return ECORE_CALLBACK_PASS_ON;
 */

   if (_drag_current && (!_xdnd))
     e_object_del(E_OBJECT(_drag_current));
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_event_dnd_drop(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_X_Event_Xdnd_Drop *ev = event;

   if (!eina_hash_find(_drop_win_hash, &ev->win)) return ECORE_CALLBACK_PASS_ON;

   if (_xdnd)
     {
        E_Drop_Handler *h;
        Eina_Bool req = EINA_TRUE;
        Eina_List *l;

        EINA_LIST_FOREACH(_active_handlers, l, h)
          {
             if (_e_drag_win_matches(h, ev->win, 1) && h->entered && h->cb.xds)
               {
                  req = h->cb.xds(h->cb.data, _xdnd->type);
               }
          }
        if (req) ecore_x_selection_xdnd_request(ev->win, _xdnd->type);

        _xdnd->x = ev->position.x;
        _xdnd->y = ev->position.y;
        if (!req)
          {
             _e_drag_xdnd_end(ev->win, _xdnd->x, _xdnd->y);
             ecore_x_dnd_send_finished();
             eina_stringshare_del(_xdnd->type);
             E_FREE(_xdnd);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dnd_cb_event_dnd_selection(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_X_Event_Selection_Notify *ev = event;
   int i;

   if (!eina_hash_find(_drop_win_hash, &ev->win)) return ECORE_CALLBACK_PASS_ON;
   if (ev->selection != ECORE_X_SELECTION_XDND) return ECORE_CALLBACK_PASS_ON;
   if (e_comp->comp_type != E_PIXMAP_TYPE_X) return ECORE_CALLBACK_RENEW;

   if (!_xdnd)
     {
        /* something crazy happened */
        ecore_x_dnd_send_finished();
        return ECORE_CALLBACK_RENEW;
     }

   if (_type_text_uri_list == _xdnd->type)
     {
        Ecore_X_Selection_Data_Files *files;
        Eina_List *l = NULL;

        files = ev->data;
        for (i = 0; i < files->num_files; i++)
          {
             /* TODO: Check if hostname is in file:// uri */
             /* if (!strncmp(files->files[i], "file://", 7)) */
             /*   l = eina_list_append(l, files->files[i]); */
             /* TODO: download files
                else if (!strncmp(files->files[i], "http://", 7))
                else if (!strncmp(files->files[i], "ftp://", 6))
              */
             /* else */
             l = eina_list_append(l, files->files[i]);
          }
        _xdnd->data = l;
        _e_drag_xdnd_end(ev->win, _xdnd->x, _xdnd->y);
        eina_list_free(l);
     }
   else if (_type_text_x_moz_url == _xdnd->type)
     {
        Ecore_X_Selection_Data_X_Moz_Url *sel;
        E_Dnd_X_Moz_Url moz;

        sel = ev->data;
        moz.links = sel->links;
        moz.link_names = sel->link_names;
        _xdnd->data = &moz;
        _e_drag_xdnd_end(ev->win, _xdnd->x, _xdnd->y);
     }
   else
     _e_drag_xdnd_end(ev->win, _xdnd->x, _xdnd->y);
   /* FIXME: When to execute this? It could be executed in ecore_x after getting
    * the drop property... */
   ecore_x_dnd_send_finished();
   eina_stringshare_del(_xdnd->type);
   E_FREE(_xdnd);
   return ECORE_CALLBACK_PASS_ON;
}
#endif

