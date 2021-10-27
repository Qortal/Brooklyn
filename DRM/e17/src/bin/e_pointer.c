#include "e.h"

typedef struct _E_Pointer_Stack E_Pointer_Stack;
struct _E_Pointer_Stack
{
   void *obj;
   const char *type;
};

/* local variables */
static Eina_List *_hdlrs = NULL;
static Eina_List *_ptrs = NULL;

static Eina_Bool init = EINA_FALSE;

static inline void
_e_pointer_theme_buf(E_Pointer *ptr, char cursor[1024])
{
   if (ptr->color)
     snprintf(cursor, 1024, "e/pointer/enlightenment/%s/color", ptr->type);
   else
     snprintf(cursor, 1024, "e/pointer/enlightenment/%s/mono", ptr->type);
}

static inline void
_e_pointer_hot_update(E_Pointer *ptr, int x, int y)
{
   if ((ptr->hot.x != x) || (ptr->hot.y != y))
     {
        ptr->hot.x = x;
        ptr->hot.y = y;
        ptr->hot.update = EINA_TRUE;
     }
}

static void
_e_pointer_active(E_Pointer *ptr)
{
   if (!ptr->idle) return;
   if (ptr->o_ptr)
     edje_object_signal_emit(ptr->o_ptr, "e,state,mouse,active", "e");
   ptr->idle = EINA_FALSE;
}

static void
_e_pointer_idle(E_Pointer *ptr)
{
   if (ptr->idle) return;
   if (ptr->o_ptr)
     edje_object_signal_emit(ptr->o_ptr, "e,state,mouse,idle", "e");
   ptr->idle = EINA_TRUE;
}

static Eina_Bool
_e_pointer_cb_idle_poller(void *data)
{
   E_Pointer *ptr;
   int x = 0, y = 0;

   if (!(ptr = data)) return ECORE_CALLBACK_RENEW;

   if ((e_powersave_mode_get() >= E_POWERSAVE_MODE_MEDIUM) ||
       (!e_config->idle_cursor))
     {
        ptr->idle_poll = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   if (ptr->canvas)
     ecore_evas_pointer_xy_get(ptr->ee, &x, &y);
#ifndef HAVE_WAYLAND_ONLY
   else
     ecore_x_pointer_xy_get(ptr->win, &x, &y);
#endif

   if ((ptr->x != x) || (ptr->y != y))
     {
        ptr->x = x;
        ptr->y = y;
        if (ptr->idle) _e_pointer_active(ptr);
        return ECORE_CALLBACK_RENEW;
     }

   if (!ptr->idle) _e_pointer_idle(ptr);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_pointer_cb_idle_wait(void *data)
{
   E_Pointer *ptr;

   if (!(ptr = data)) return ECORE_CALLBACK_RENEW;
   ptr->idle_tmr = NULL;
   if ((e_powersave_mode_get() >= E_POWERSAVE_MODE_MEDIUM) ||
       (!e_config->idle_cursor))
     {
        E_FREE_FUNC(ptr->idle_poll, ecore_poller_del);
        return ECORE_CALLBACK_CANCEL;
     }

   if (!ptr->idle_poll)
     ptr->idle_poll = ecore_poller_add(ECORE_POLLER_CORE, 64,
                                       _e_pointer_cb_idle_poller, ptr);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_e_pointer_cb_idle_pre(void *data)
{
   E_Pointer *ptr;

   if (!(ptr = data)) return ECORE_CALLBACK_RENEW;

   if (ptr->canvas)
     ecore_evas_pointer_xy_get(ptr->ee, &ptr->x, &ptr->y);
#ifndef HAVE_WAYLAND_ONLY
   else
     ecore_x_pointer_xy_get(ptr->win, &ptr->x, &ptr->y);
#endif

   ptr->idle_tmr = ecore_timer_loop_add(4.0, _e_pointer_cb_idle_wait, ptr);

   return ECORE_CALLBACK_CANCEL;
}

static void
_e_pointer_active_handle(E_Pointer *ptr)
{
   _e_pointer_active(ptr);
   if (ptr->idle_tmr)
     ecore_timer_loop_reset(ptr->idle_tmr);
   else
     {
        E_FREE_FUNC(ptr->idle_poll, ecore_poller_del);
        if (e_powersave_mode_get() >= E_POWERSAVE_MODE_MEDIUM) return;
        if (!e_config->idle_cursor) return;
        ptr->idle_tmr = ecore_timer_loop_add(1.0, _e_pointer_cb_idle_pre, ptr);
     }
}

static Eina_Bool
_e_pointer_cb_mouse_down(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eina_List *l;
   E_Pointer *ptr;

   EINA_LIST_FOREACH(_ptrs, l, ptr)
     {
        _e_pointer_active_handle(ptr);
        if (e_powersave_mode_get() < E_POWERSAVE_MODE_EXTREME)
          {
             if (ptr->o_ptr)
               edje_object_signal_emit(ptr->o_ptr, "e,action,mouse,down", "e");
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_pointer_cb_mouse_up(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eina_List *l;
   E_Pointer *ptr;

   EINA_LIST_FOREACH(_ptrs, l, ptr)
     {
        _e_pointer_active_handle(ptr);
        if (e_powersave_mode_get() < E_POWERSAVE_MODE_EXTREME)
          {
             if (ptr->o_ptr)
               edje_object_signal_emit(ptr->o_ptr, "e,action,mouse,up", "e");
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_pointer_cb_mouse_move(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eina_List *l;
   E_Pointer *ptr;

   EINA_LIST_FOREACH(_ptrs, l, ptr)
     {
        _e_pointer_active_handle(ptr);
        if (e_powersave_mode_get() < E_POWERSAVE_MODE_HIGH)
          {
             if (ptr->o_ptr)
               edje_object_signal_emit(ptr->o_ptr, "e,action,mouse,move", "e");
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_pointer_cb_mouse_wheel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eina_List *l;
   E_Pointer *ptr;

   EINA_LIST_FOREACH(_ptrs, l, ptr)
     {
        _e_pointer_active_handle(ptr);
        if (e_powersave_mode_get() < E_POWERSAVE_MODE_EXTREME)
          {
             if (ptr->o_ptr)
               edje_object_signal_emit(ptr->o_ptr, "e,action,mouse,wheel", "e");
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_hot_update(E_Pointer *ptr)
{
   int x = 0, y = 0, ox = 0, oy = 0;

   if (ptr->e_cursor && (e_comp->comp_type != E_PIXMAP_TYPE_WL))
     evas_object_geometry_get(ptr->buffer_o_ptr, &ox, &oy, NULL, NULL);
   else
     evas_object_geometry_get(ptr->o_ptr, &ox, &oy, NULL, NULL);
   evas_object_geometry_get(ptr->o_hot, &x, &y, NULL, NULL);
   _e_pointer_hot_update(ptr, x - ox, y - oy);
}

static void
_e_pointer_cb_hot_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Pointer *ptr = data;

   if (!ptr->e_cursor) return;
   if (!evas_object_visible_get(ptr->o_ptr)) return;
   _hot_update(ptr);
}

static void
_e_pointer_cb_hot_show(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Pointer *ptr = data;

   if (!ptr->e_cursor) return;
   _hot_update(ptr);
}

static void
_e_pointer_pointer_canvas_init(E_Pointer *ptr, Evas *e, Evas_Object **o_ptr, Evas_Object **o_hot)
{
   /* create pointer object */
   *o_ptr = edje_object_add(e);

   /* create hotspot object */
   *o_hot = evas_object_rectangle_add(e);
   evas_object_color_set(*o_hot, 0, 0, 0, 0);

   evas_object_event_callback_add(*o_hot, EVAS_CALLBACK_MOVE,
                                  _e_pointer_cb_hot_move, ptr);
   evas_object_event_callback_add(*o_ptr, EVAS_CALLBACK_MOVE,
                                  _e_pointer_cb_hot_move, ptr);
   evas_object_event_callback_add(*o_hot, EVAS_CALLBACK_SHOW,
                                  _e_pointer_cb_hot_show, ptr);

   evas_object_move(*o_ptr, 0, 0);
   evas_object_resize(*o_ptr, ptr->w, ptr->h);
}

static void
_e_pointer_canvas_del(E_Pointer *ptr)
{
   E_FREE_FUNC(ptr->buffer_o_hot, evas_object_del);
   E_FREE_FUNC(ptr->buffer_o_ptr, evas_object_del);
   E_FREE_FUNC(ptr->buffer_evas, evas_free);
   E_FREE(ptr->pixels);
}

static void
_e_pointer_canvas_add(E_Pointer *ptr)
{
   Evas_Engine_Info_Buffer *einfo;
   int method = 0;

   /* try to create new canvas */
   if (!(ptr->buffer_evas = evas_new())) goto err;

   method = evas_render_method_lookup("buffer");
   evas_output_method_set(ptr->buffer_evas, method);
   evas_output_size_set(ptr->buffer_evas, ptr->w, ptr->h);
   evas_output_viewport_set(ptr->buffer_evas, 0, 0, ptr->w, ptr->h);

   /* try to allocate space for pixels */
   if (!(ptr->pixels = malloc(ptr->w * ptr->h * sizeof(int))))
     goto err;

   /* try to get the buffer engine info */
   einfo = (Evas_Engine_Info_Buffer *)(void *)
     evas_engine_info_get(ptr->buffer_evas);
   if (!einfo) goto err;

   /* fill in buffer engine info */
   einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
   einfo->info.dest_buffer = ptr->pixels;
   einfo->info.dest_buffer_row_bytes = (ptr->w * sizeof(int));
   einfo->info.use_color_key = 0;
   einfo->info.alpha_threshold = 0;
   einfo->info.func.new_update_region = NULL;
   einfo->info.func.free_update_region = NULL;

   /* set buffer engine info */
   evas_engine_info_set(ptr->buffer_evas, (Evas_Engine_Info *)einfo);

   _e_pointer_pointer_canvas_init(ptr, ptr->buffer_evas, &ptr->buffer_o_ptr, &ptr->buffer_o_hot);
   if (!ptr->evas)
     {
        ptr->evas = ptr->buffer_evas;
        ptr->o_ptr = ptr->buffer_o_ptr;
        ptr->o_hot = ptr->buffer_o_hot;
     }
   return;

err:
   _e_pointer_canvas_del(ptr);
}

static void
_e_pointer_canvas_resize(E_Pointer *ptr, int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;

   if ((ptr->w == w) && (ptr->h == h)) return;
   ptr->w = w;
   ptr->h = h;
   evas_output_size_set(ptr->buffer_evas, w, h);
   evas_output_viewport_set(ptr->buffer_evas, 0, 0, w, h);

   ptr->pixels = realloc(ptr->pixels, (ptr->w * ptr->h * sizeof(int)));

   einfo = (Evas_Engine_Info_Buffer *)(void *)
     evas_engine_info_get(ptr->buffer_evas);
   EINA_SAFETY_ON_NULL_RETURN(einfo);

   einfo->info.dest_buffer = ptr->pixels;
   einfo->info.dest_buffer_row_bytes = (ptr->w * sizeof(int));
   evas_engine_info_set(ptr->buffer_evas, (Evas_Engine_Info *)einfo);

   evas_object_move(ptr->buffer_o_ptr, 0, 0);
   evas_object_resize(ptr->buffer_o_ptr, ptr->w, ptr->h);
}

static void
_e_pointer_stack_free(E_Pointer_Stack *stack)
{
   if (stack->type) eina_stringshare_del(stack->type);
   free(stack);
}

static void
_e_pointer_cb_free(E_Pointer *ptr)
{
   _ptrs = eina_list_remove(_ptrs, ptr);

   E_FREE_LIST(ptr->stack, _e_pointer_stack_free);

   eina_stringshare_del(ptr->type);
   eina_stringshare_del(ptr->deferred_type);

   E_FREE_FUNC(ptr->idle_tmr, ecore_timer_del);
   E_FREE_FUNC(ptr->idle_poll, ecore_poller_del);

   if (ptr->buffer_evas) _e_pointer_canvas_del(ptr);

   free(ptr);
}

static void
_e_pointer_x11_setup(E_Pointer *ptr, const char *cursor)
{
   if (ptr->e_cursor && (e_comp->comp_type != E_PIXMAP_TYPE_WL))
     {
        /* create a pointer canvas if we need to */
        if ((!ptr->buffer_evas) && ptr->win) _e_pointer_canvas_add(ptr);
        if (ptr->buffer_o_ptr && (ptr->buffer_o_ptr != ptr->o_ptr))
          {
             e_theme_edje_object_set(ptr->buffer_o_ptr, "base/theme/pointer", cursor);
             edje_object_part_swallow(ptr->buffer_o_ptr, "e.swallow.hotspot", ptr->buffer_o_hot);
             if (!init)
               {
                  edje_object_signal_emit(ptr->o_ptr, "e,state,init", "e");
                  edje_object_message_signal_process(ptr->o_ptr);
                  init = EINA_TRUE;
               }
          }
        return;
     }
   if (ptr->buffer_evas) _e_pointer_canvas_del(ptr);
#ifndef HAVE_WAYLAND_ONLY
   if (!e_comp_util_has_x()) return;
   Ecore_X_Cursor curs = 0;

   if (!strcmp(ptr->type, "move"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_FLEUR);
# if 0
   else if (!strcmp(ptr->type, "resize"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_SIZING);
# endif
   else if (!strcmp(ptr->type, "resize_tl"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_TOP_LEFT_CORNER);
   else if (!strcmp(ptr->type, "resize_t"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_TOP_SIDE);
   else if (!strcmp(ptr->type, "resize_tr"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_TOP_RIGHT_CORNER);
   else if (!strcmp(ptr->type, "resize_r"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_RIGHT_SIDE);
   else if (!strcmp(ptr->type, "resize_br"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_BOTTOM_RIGHT_CORNER);
   else if (!strcmp(ptr->type, "resize_b"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_BOTTOM_SIDE);
   else if (!strcmp(ptr->type, "resize_bl"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_BOTTOM_LEFT_CORNER);
   else if (!strcmp(ptr->type, "resize_l"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_LEFT_SIDE);
   else if (!strcmp(ptr->type, "entry"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_XTERM);
   else if (!strcmp(ptr->type, "default"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_LEFT_PTR);
   else if (!strcmp(ptr->type, "plus"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_PLUS);
   else if (!strcmp(ptr->type, "hand"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_HAND1);
   else if (!strcmp(ptr->type, "rotate"))
     curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_EXCHANGE);
   else
     {
        WRN("Unknown pointer ptr->type: %s\n", ptr->type);
        curs = ecore_x_cursor_shape_get(ECORE_X_CURSOR_ARROW);
     }
   if (!curs) WRN("X Cursor for %s is missing\n", ptr->type);
   ecore_x_window_cursor_set(ptr->win, curs);
   if (curs) ecore_x_cursor_free(curs);
#endif
}

static void
_e_pointer_type_set(E_Pointer *ptr, const char *type)
{
   /* check if pointer type is already set */
   if (!e_util_strcmp(ptr->type, type)) return;

   if (ptr->grabcount > 0)
     {
        eina_stringshare_replace(&(ptr->deferred_type), type);
        return;
     }
   eina_stringshare_replace(&ptr->type, type);

   /* don't show cursor if in hidden mode */
   if (!e_config->show_cursor)
     {
        e_pointer_hide(ptr);
        return;
     }

   if (ptr->e_cursor)
     {
        char cursor[1024];

        if ((!ptr->buffer_evas) && ptr->win) _e_pointer_canvas_add(ptr);
        _e_pointer_theme_buf(ptr, cursor);

        /* try to set the edje object theme */
        if (!e_theme_edje_object_set(ptr->o_ptr, "base/theme/pointer", cursor))
          cursor[0] = 0;
        edje_object_part_swallow(ptr->o_ptr, "e.swallow.hotspot", ptr->o_hot);
        if (!init)
          {
             edje_object_signal_emit(ptr->o_ptr, "e,state,init", "e");
             edje_object_message_signal_process(ptr->o_ptr);
             init = EINA_TRUE;
          }
        _e_pointer_x11_setup(ptr, cursor);
        if (!cursor[0]) return;

        _hot_update(ptr);

        if (ptr->canvas)
          e_pointer_object_set(ptr, NULL, 0, 0);
        else
          evas_object_show(ptr->o_ptr);
     }
   else
     _e_pointer_x11_setup(ptr, NULL);
}

EINTERN int
e_pointer_init(void)
{
   E_LIST_HANDLER_APPEND(_hdlrs, ECORE_EVENT_MOUSE_BUTTON_DOWN,
                         _e_pointer_cb_mouse_down, NULL);
   E_LIST_HANDLER_APPEND(_hdlrs, ECORE_EVENT_MOUSE_BUTTON_UP,
                         _e_pointer_cb_mouse_up, NULL);
   E_LIST_HANDLER_APPEND(_hdlrs, ECORE_EVENT_MOUSE_MOVE,
                         _e_pointer_cb_mouse_move, NULL);
   E_LIST_HANDLER_APPEND(_hdlrs, ECORE_EVENT_MOUSE_WHEEL,
                         _e_pointer_cb_mouse_wheel, NULL);
   init = EINA_FALSE;
   return 1;
}

EINTERN int
e_pointer_shutdown(void)
{
   E_FREE_LIST(_hdlrs, ecore_event_handler_del);
   init = EINA_FALSE;
   return 1;
}

E_API E_Pointer *
e_pointer_window_new(Ecore_Window win, Eina_Bool filled)
{
   E_Pointer *ptr = NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(win, NULL);

   /* allocate space for new pointer */
   if (!(ptr = E_OBJECT_ALLOC(E_Pointer, E_POINTER_TYPE, _e_pointer_cb_free)))
     return NULL;

   /* set default pointer properties */
   ptr->w = ptr->h = (double)e_config->cursor_size * e_scale;
   ptr->e_cursor = e_config->use_e_cursor;
   ptr->win = win;
   ptr->color = EINA_FALSE;
   if (e_comp->pointer)
     ptr->color = e_comp->pointer->color;

   /* set pointer default type */
   if (filled) e_pointer_type_push(ptr, ptr, "default");

   /* append this pointer to the list */
   _ptrs = eina_list_append(_ptrs, ptr);

   return ptr;
}

E_API E_Pointer *
e_pointer_canvas_new(Ecore_Evas *ee, Eina_Bool filled)
{
   E_Pointer *ptr = NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(ee, NULL);

   /* allocate space for new pointer */
   if (!(ptr = E_OBJECT_ALLOC(E_Pointer, E_POINTER_TYPE, _e_pointer_cb_free)))
     return NULL;

   /* set default pointer properties */
   ptr->color = EINA_TRUE;
   ptr->canvas = EINA_TRUE;
   ptr->w = ptr->h = (double)e_config->cursor_size * e_scale;
   ptr->e_cursor = 1;

   ptr->ee = ee;
   ptr->evas = ecore_evas_get(ee);
   _e_pointer_pointer_canvas_init(ptr, ptr->evas, &ptr->o_ptr, &ptr->o_hot);

   /* set pointer default type */
   if (filled) e_pointer_type_push(ptr, ptr, "default");

     /* append this pointer to the list */
   _ptrs = eina_list_append(_ptrs, ptr);

   _e_pointer_active_handle(ptr);

   return ptr;
}

E_API void
e_pointers_size_set(int size)
{
   Eina_List *l;
   E_Pointer *ptr;

   if (!e_config->show_cursor) return;

   size = (double)size * e_scale;
   EINA_LIST_FOREACH(_ptrs, l, ptr)
     {
        if ((ptr->w == size) && (ptr->h == size)) continue;
        if (ptr->buffer_evas)
          _e_pointer_canvas_resize(ptr, size, size);
        if (ptr->canvas)
          {
             ptr->w = size;
             ptr->h = size;
             evas_object_resize(ptr->o_ptr, size, size);
          }
     }
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     ecore_x_cursor_size_set(e_config->cursor_size * 3 / 4);
#endif
}

E_API void
e_pointer_hide(E_Pointer *ptr)
{
   if (ptr->buffer_evas)
     _e_pointer_canvas_del(ptr);
   if (ptr->canvas)
     evas_object_hide(ptr->o_ptr);
#ifndef HAVE_WAYLAND_ONLY
   if (ptr->win)
     ecore_x_window_cursor_set(ptr->win, 0);
#endif
}

E_API void
e_pointer_show(E_Pointer *ptr)
{
   if ((!ptr->buffer_evas) && ptr->win) _e_pointer_canvas_add(ptr);
   if (ptr->canvas)
     evas_object_show(ptr->o_ptr);
}

E_API void
e_pointer_type_push(E_Pointer *ptr, void *obj, const char *type)
{
   E_Pointer_Stack *stack;

   EINA_SAFETY_ON_NULL_RETURN(ptr);

   _e_pointer_type_set(ptr, type);

   if (!(stack = E_NEW(E_Pointer_Stack, 1))) return;
   stack->type = eina_stringshare_ref(ptr->type);
   stack->obj = obj;
   ptr->stack = eina_list_prepend(ptr->stack, stack);
}

E_API void
e_pointer_type_pop(E_Pointer *ptr, void *obj, const char *type)
{
   Eina_List *l, *ll;
   E_Pointer_Stack *stack;

   EINA_SAFETY_ON_NULL_RETURN(ptr);

   EINA_LIST_FOREACH_SAFE(ptr->stack, l, ll, stack)
     {
        if (((stack->obj == obj) || (!obj)) &&
            ((!type) || (!e_util_strcmp(stack->type, type))))
          {
             _e_pointer_stack_free(stack);
             ptr->stack = eina_list_remove_list(ptr->stack, l);
             if (type) break;
          }
     }

   if ((!obj) && (!type))
     {
        if (!ptr->stack)
          {
             e_pointer_type_push(ptr, ptr, "default");
          }
     }
   else
     {
        if (!ptr->stack)
          {
             e_pointer_hide(ptr);
             eina_stringshare_replace(&ptr->type, NULL);
             return;
          }
     }

   if (!(stack = eina_list_data_get(ptr->stack))) return;

   _e_pointer_type_set(ptr, stack->type);

   /* eina_stringshare_replace(&ptr->type, stack->type); */
}

E_API void
e_pointer_mode_push(void *obj, E_Pointer_Mode mode)
{
   switch (mode)
     {
      case E_POINTER_RESIZE_TL:
        e_pointer_type_push(e_comp->pointer, obj, "resize_tl");
        break;

      case E_POINTER_RESIZE_T:
        e_pointer_type_push(e_comp->pointer, obj, "resize_t");
        break;

      case E_POINTER_RESIZE_TR:
        e_pointer_type_push(e_comp->pointer, obj, "resize_tr");
        break;

      case E_POINTER_RESIZE_R:
        e_pointer_type_push(e_comp->pointer, obj, "resize_r");
        break;

      case E_POINTER_RESIZE_BR:
        e_pointer_type_push(e_comp->pointer, obj, "resize_br");
        break;

      case E_POINTER_RESIZE_B:
        e_pointer_type_push(e_comp->pointer, obj, "resize_b");
        break;

      case E_POINTER_RESIZE_BL:
        e_pointer_type_push(e_comp->pointer, obj, "resize_bl");
        break;

      case E_POINTER_RESIZE_L:
        e_pointer_type_push(e_comp->pointer, obj, "resize_l");
        break;

      case E_POINTER_MOVE:
        e_pointer_type_push(e_comp->pointer, obj, "move");
        break;

      default: break;
     }
}

E_API void
e_pointer_mode_pop(void *obj, E_Pointer_Mode mode)
{
   switch (mode)
     {
      case E_POINTER_RESIZE_TL:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_tl");
        break;

      case E_POINTER_RESIZE_T:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_t");
        break;

      case E_POINTER_RESIZE_TR:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_tr");
        break;

      case E_POINTER_RESIZE_R:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_r");
        break;

      case E_POINTER_RESIZE_BR:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_br");
        break;

      case E_POINTER_RESIZE_B:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_b");
        break;

      case E_POINTER_RESIZE_BL:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_bl");
        break;

      case E_POINTER_RESIZE_L:
        e_pointer_type_pop(e_comp->pointer, obj, "resize_l");
        break;

      case E_POINTER_MOVE:
        e_pointer_type_pop(e_comp->pointer, obj, "move");
        break;

      default: break;
     }
}

E_API void
e_pointer_idler_before(void)
{
   Eina_List *l;
   E_Pointer *ptr;

   if (!e_config->show_cursor) return;

   if (ecore_evas_manual_render_get(e_comp->ee)) return;
   EINA_LIST_FOREACH(_ptrs, l, ptr)
     {
        if ((!ptr->e_cursor) || (!ptr->buffer_evas)) continue;

        if (ptr->hot.update)
          _e_pointer_type_set(ptr, ptr->type);

        if (ptr->buffer_evas)
          {
             Eina_List *updates;

             if ((updates = evas_render_updates(ptr->buffer_evas)))
               {
#ifndef HAVE_WAYLAND_ONLY
                  Ecore_X_Cursor cur;
                  int hotx = ptr->hot.x;
                  int hoty = ptr->hot.y;

                  if      (hotx < 0)       hotx = 0;
                  else if (hotx >= ptr->w) hotx = ptr->w - 1;
                  if      (hoty < 0)       hoty = 0;
                  else if (hoty >= ptr->h) hoty = ptr->h - 1;
                  cur = ecore_x_cursor_new(ptr->win, ptr->pixels,
                                           ptr->w, ptr->h, hotx, hoty);
                  ecore_x_window_cursor_set(ptr->win, cur);
                  ecore_x_cursor_free(cur);
#endif
                  evas_render_updates_free(updates);
               }
          }

        ptr->hot.update = EINA_FALSE;
     }
}

E_API void
e_pointer_object_set(E_Pointer *ptr, Evas_Object *obj, int x, int y)
{
   Evas_Object *o;
   E_Client *ec;
   int px, py;

   ecore_evas_cursor_get(ptr->ee, &o, NULL, &px, &py);
   if (o)
     {
        if (o == obj)
          {
             ecore_evas_object_cursor_set(ptr->ee, obj, E_LAYER_MAX - 1, x, y);
             return;
          }
        ec = e_comp_object_client_get(o);
        if (ec)
          {
             ec->hidden = 1;
             evas_object_hide(ec->frame);
          }
     }
   ec = NULL;
   if (obj)
     {
        ec = e_comp_object_client_get(obj);
        if (ec)
          ec->hidden = 1;
        ecore_evas_cursor_unset(ptr->ee);
        ecore_evas_object_cursor_set(ptr->ee, obj, E_LAYER_MAX - 1, x, y);
     }
   else if ((o != ptr->o_ptr) || (x != px) || (y != py))
     {
        ecore_evas_cursor_unset(ptr->ee);
        ecore_evas_object_cursor_set(ptr->ee, ptr->o_ptr, E_LAYER_MAX - 1,
                                     ptr->hot.x, ptr->hot.y);
        evas_object_show(ptr->o_ptr);
     }
   ptr->client.ec = ec;
   ptr->client.x = x;
   ptr->client.y = y;
}

E_API void
e_pointer_window_add(E_Pointer *ptr, Ecore_Window win)
{
   ptr->win = win;
   _e_pointer_x11_setup(ptr, "default");
}

E_API void
e_pointer_grab_set(E_Pointer *ptr, Eina_Bool grab)
{
   if (grab) ptr->grabcount++;
   else
     {
        if (ptr->grabcount > 0)
          {
             ptr->grabcount--;
             if ((ptr->grabcount == 0) && (ptr->deferred_type))
               {
                  _e_pointer_type_set(ptr, ptr->deferred_type);
                  eina_stringshare_del(ptr->deferred_type);
                  ptr->deferred_type = NULL;
               }
          }
     }
}

E_API void
e_pointer_reset(E_Pointer *ptr)
{
   EINA_SAFETY_ON_NULL_RETURN(ptr);

   /* free stack of pointer types */
   E_FREE_LIST(ptr->stack, _e_pointer_stack_free);

   eina_stringshare_del(ptr->type);
   eina_stringshare_del(ptr->deferred_type);

   /* reset pointer to default */
   e_pointer_type_push(ptr, ptr, "default");
}

EINTERN void
e_pointers_freeze_set(Eina_Bool set)
{
   Eina_List *l;
   E_Pointer *ptr;

   EINA_LIST_FOREACH(_ptrs, l, ptr)
     {
        if (isedje(ptr->o_ptr))
          edje_object_play_set(ptr->o_ptr, !set);
        if (isedje(ptr->buffer_o_ptr))
          edje_object_play_set(ptr->buffer_o_ptr, !set);
     }
}
