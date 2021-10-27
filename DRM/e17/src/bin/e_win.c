#include "e.h"

/* intercept elm_win operations so we talk directly to e_client */
#undef elm_win_add
#undef elm_win_util_dialog_add
#undef elm_win_util_standard_add

typedef struct _Elm_Win_Trap_Ctx
{
   E_Client      *client;
   E_Pointer     *pointer;
   int x, y, w, h;
   Eina_Bool      centered E_BITFIELD;
   Eina_Bool      placed E_BITFIELD;
   Eina_Bool      sized E_BITFIELD;
   Eina_Bool      internal_no_remember E_BITFIELD;
   Eina_Bool      internal_no_reopen E_BITFIELD;
   Eina_Bool      visible E_BITFIELD;
   Eina_Bool      override E_BITFIELD;
} Elm_Win_Trap_Ctx;


static Elm_Win_Trap_Ctx *current_win = NULL;

static void *
_e_elm_win_trap_add(Evas_Object *o)
{
   if (elm_win_type_get(o) == ELM_WIN_INLINED_IMAGE) return NULL;
   Elm_Win_Trap_Ctx *ctx = calloc(1, sizeof(Elm_Win_Trap_Ctx));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   evas_object_name_set(o, "E_Win");
   return ctx;
}

static void
_e_elm_win_trap_del(void *data, Evas_Object *o)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return;
   if (ctx->client)
     {
        ctx->client->internal_elm_win = NULL;
        e_object_del(E_OBJECT(ctx->client));
        evas_object_data_set(o, "E_Client", NULL);
     }
   free(ctx);
}

static Eina_Bool
_e_elm_win_trap_hide(void *data, Evas_Object *o)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   if (!ctx->visible) return EINA_FALSE;
   if (strncmp(ecore_evas_engine_name_get(e_win_ee_get(o)), "wayland", 7))
     E_FREE_FUNC(ctx->pointer, e_object_del);
#ifdef HAVE_WAYLAND
   else if (ctx->client)
     e_pixmap_alias(NULL, E_PIXMAP_TYPE_WL, ecore_wl2_window_surface_id_get(elm_win_wl_window_get(o)));
#endif

   if (!ctx->client) return EINA_TRUE;
   ctx->visible = 0;
   evas_object_hide(ctx->client->frame);
   return EINA_FALSE;
}

static Eina_Bool
_e_elm_win_trap_show(void *data, Evas_Object *o)
{
   Elm_Win_Trap_Ctx *ctx = data;
   Evas *e = evas_object_evas_get(o);
   Ecore_Evas *ee = ecore_evas_ecore_evas_get(e);
   Eina_Bool borderless;

   if (!ctx) return EINA_TRUE;
   borderless = elm_win_borderless_get(o);
   if (!ctx->client)
     {
        E_Client *ec;
        Ecore_Window win;
#ifdef HAVE_WAYLAND
        int64_t wl_win_id = -1;
#endif
        E_Pixmap_Type type = E_PIXMAP_TYPE_X;

        win = elm_win_window_id_get(o);
#ifdef HAVE_WAYLAND
        if (!strncmp(ecore_evas_engine_name_get(ee), "wayland", 7))
          {
             Ecore_Wl2_Window *ewin = elm_win_wl_window_get(o);

             type = E_PIXMAP_TYPE_WL;
             ecore_evas_object_cursor_set(ee, NULL, 0, 0, 0);
             ctx->pointer = e_comp->pointer;
             elm_win_borderless_set(o, 1);
             wl_win_id = ecore_wl2_window_surface_id_get(ewin);
          }
        else
#endif
          {
             type = E_PIXMAP_TYPE_X;
             ctx->pointer = e_pointer_window_new(win, EINA_TRUE);
          }

        if (type == E_PIXMAP_TYPE_WL)
          ec = e_pixmap_find_client(type, (int64_t)win);
        else
          ec = e_pixmap_find_client(type, win);

        if (ec)
          ctx->client = ec;
        else
          {
             E_Pixmap *cp;
             const char *title, *name, *clas;

             ecore_evas_name_class_get(ee, &name, &clas);
             if (!name) name = "E";
             if (!clas) clas = "_e_internal_window";
             ecore_evas_name_class_set(ee, name, clas);
             title = elm_win_title_get(o);
             if ((!title) || (!title[0]))
               title = "E";
             ecore_evas_title_set(ee, title);

             if (type == E_PIXMAP_TYPE_WL)
               cp = e_pixmap_new(type, (int64_t)win);
             else
               cp = e_pixmap_new(type, win);

             EINA_SAFETY_ON_NULL_RETURN_VAL(cp, EINA_TRUE);
#ifdef HAVE_WAYLAND
             if (wl_win_id >= 0)
               e_pixmap_alias(cp, type, wl_win_id);
#endif

             current_win = ctx;
             ctx->client = e_client_new(cp, 0, 1);
             current_win = NULL;
             EINA_SAFETY_ON_NULL_RETURN_VAL(ctx->client, EINA_TRUE);
             eina_stringshare_replace(&ctx->client->icccm.name, name);
             eina_stringshare_replace(&ctx->client->icccm.class, clas);
             eina_stringshare_replace(&ctx->client->icccm.title, title);
          }
        ctx->client->placed = ctx->placed | ctx->centered;
        ctx->client->internal_no_remember = ctx->internal_no_remember;
        ctx->client->internal_no_reopen = ctx->internal_no_reopen;
        ctx->client->internal_elm_win = o;
        elm_win_autodel_set(o, 1);
        evas_object_data_set(o, "E_Client", ctx->client);
        ctx->client->dialog = elm_win_type_get(o) == ELM_WIN_DIALOG_BASIC;
        ctx->client->tooltip = elm_win_type_get(o) == ELM_WIN_TOOLTIP;

        evas_object_size_hint_min_get(o, &ctx->client->icccm.min_w, &ctx->client->icccm.min_h);
        ctx->client->icccm.min_w = MAX(ctx->client->icccm.min_w, 0);
        ctx->client->icccm.min_h = MAX(ctx->client->icccm.min_h, 0);
        evas_object_size_hint_max_get(o, &ctx->client->icccm.max_w, &ctx->client->icccm.max_h);
        ctx->client->icccm.max_w = MAX(ctx->client->icccm.max_w, 0);
        ctx->client->icccm.max_h = MAX(ctx->client->icccm.max_h, 0);
     }
//#endif
   evas_object_geometry_get(o, &ctx->client->client.x, &ctx->client->client.y, &ctx->client->client.w, &ctx->client->client.h);
   ecore_evas_show(ee);
   eina_stringshare_replace(&ctx->client->internal_icon, elm_win_icon_name_get(o));
   if (!ctx->visible)
     {
        ctx->visible = 1;
        evas_object_show(ctx->client->frame);
     }
   ctx->client->borderless |= borderless;
   e_comp_object_frame_xy_adjust(ctx->client->frame, ctx->client->client.x, ctx->client->client.y, &ctx->client->x, &ctx->client->y);
   e_comp_object_frame_wh_adjust(ctx->client->frame, ctx->client->client.w, ctx->client->client.h, &ctx->client->w, &ctx->client->h);
   if (ctx->sized) evas_object_resize(ctx->client->frame, ctx->w, ctx->h);
   if (ctx->placed) evas_object_move(ctx->client->frame, ctx->x, ctx->y);
   if (ctx->centered)
     {
        e_comp_object_util_center(o);
        ctx->centered = 0;
     }
   return EINA_TRUE;
}

static Eina_Bool
_e_elm_win_trap_move(void *data, Evas_Object *o, int x, int y)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     {
        int ex, ey;

        /* if the ee coords match the requested coords, this is coming from
         * a configure event which the compositor sent. failing to reject this
         * move will result in bad window positioning
         */
        ecore_evas_geometry_get(e_win_ee_get(o), &ex, &ey, NULL, NULL);
        if ((x == ex) && (y == ey)) return EINA_FALSE;
     }
   ctx->placed = 1;
   ctx->x = x, ctx->y = y;
   if (!ctx->client) return EINA_TRUE;
   if ((ctx->client->client.x != x) || (ctx->client->client.y != y))
     e_client_util_move_without_frame(ctx->client, x, y);
   return EINA_TRUE;
}

static Eina_Bool
_e_elm_win_trap_resize(void *data, Evas_Object *o EINA_UNUSED, int w, int h)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   ctx->sized = 1;
   ctx->w = w, ctx->h = h;
   if (!ctx->client) return EINA_TRUE;
   e_comp_object_frame_wh_adjust(ctx->client->frame, w, h, &w, &h);
   e_client_resize_limit(ctx->client, &w, &h);
   evas_object_resize(ctx->client->frame, w, h);
   return EINA_TRUE;
}

static Eina_Bool
_e_elm_win_trap_center(void *data, Evas_Object *o EINA_UNUSED, Eina_Bool h, Eina_Bool v)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   ctx->centered = h | v;
   ctx->placed = 1;
   if (!ctx->client) return EINA_FALSE;
   if (ctx->centered)
     {
        e_comp_object_util_center(ctx->client->frame);
        ctx->centered = 0;
     }
   return EINA_FALSE;
}

static Eina_Bool
_e_elm_win_trap_lower(void *data, Evas_Object *o EINA_UNUSED)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   if (!ctx->client) return EINA_TRUE;
   evas_object_lower(ctx->client->frame);
   return EINA_FALSE;
}

static Eina_Bool
_e_elm_win_trap_raise(void *data, Evas_Object *o EINA_UNUSED)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   if (!ctx->client) return EINA_TRUE;
   evas_object_raise(ctx->client->frame);
   return EINA_FALSE;
}

static Eina_Bool
_e_elm_win_trap_activate(void *data, Evas_Object *o EINA_UNUSED)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   if (!ctx->client) return EINA_TRUE;
   evas_object_focus_set(ctx->client->frame, 1);
   if (!ctx->client->lock_user_stacking)
     evas_object_raise(ctx->client->frame);
   return EINA_FALSE;
}

static Eina_Bool
_e_elm_win_trap_size_min_set(void *data, Evas_Object *o EINA_UNUSED, int w, int h)
{
   Elm_Win_Trap_Ctx *ctx = data;
   int mw = 0, mh = 0;

   if (!ctx) return EINA_TRUE;
   if (!ctx->client) return EINA_TRUE;
   ctx->client->icccm.min_w = w;
   ctx->client->icccm.min_h = h;
   e_client_resize_limit(ctx->client, &mw, &mh);
   mw = ctx->client->w < w ? w : ctx->client->w;
   mh = ctx->client->h < h ? h : ctx->client->h;
   if ((ctx->client->w != mw) || (ctx->client->h != mh))
     evas_object_resize(ctx->client->frame, mw, mh);

   return EINA_TRUE;
}

static Eina_Bool
_e_elm_win_trap_size_max_set(void *data, Evas_Object *o EINA_UNUSED, int w, int h)
{
   Elm_Win_Trap_Ctx *ctx = data;
   int mw = 0, mh = 0;

   if (!ctx) return EINA_TRUE;
   if (!ctx->client) return EINA_TRUE;
   w = MAX(w, 0);
   h = MAX(h, 0);
   ctx->client->icccm.max_w = w;
   ctx->client->icccm.max_h = h;
   e_client_resize_limit(ctx->client, &mw, &mh);
   mw = ctx->client->w > w ? w : ctx->client->w;
   mh = ctx->client->h > h ? h : ctx->client->h;
   if ((ctx->client->w != mw) || (ctx->client->h != mh))
     evas_object_resize(ctx->client->frame, mw, mh);

   return EINA_TRUE;
}

static Eina_Bool
_e_elm_win_trap_size_base_set(void *data, Evas_Object *o EINA_UNUSED, int w, int h)
{
   Elm_Win_Trap_Ctx *ctx = data;
   int mw = 0, mh = 0;

   if (!ctx) return EINA_TRUE;
   if (!ctx->client) return EINA_TRUE;
   ctx->client->icccm.base_w = w;
   ctx->client->icccm.base_h = h;
   e_client_resize_limit(ctx->client, &mw, &mh);
   mw = ctx->client->w < w ? w : ctx->client->w;
   mh = ctx->client->h < h ? h : ctx->client->h;
   if ((ctx->client->w != mw) || (ctx->client->h != mh))
     evas_object_resize(ctx->client->frame, mw, mh);

   return EINA_TRUE;
}

static Eina_Bool
_e_elm_win_trap_borderless_set(void *data, Evas_Object *o EINA_UNUSED, Eina_Bool borderless)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;
   if (!ctx->client) return EINA_TRUE;

   borderless = !!borderless;

   ctx->client->borderless = borderless;
   EC_CHANGED(ctx->client);
   ctx->client->changes.border = 1;
   return EINA_TRUE;
}

static Eina_Bool
_e_elm_win_trap_override_set(void *data, Evas_Object *o EINA_UNUSED, Eina_Bool override)
{
   Elm_Win_Trap_Ctx *ctx = data;
   if (!ctx) return EINA_TRUE;

   if (ctx->client)
     CRI("Override being set too late on internal client!");
   ctx->override = !!override;
   return EINA_TRUE;
}

static const Elm_Win_Trap _e_elm_win_trap = {
   ELM_WIN_TRAP_VERSION,
   _e_elm_win_trap_add,
   _e_elm_win_trap_del,
   _e_elm_win_trap_hide,
   _e_elm_win_trap_show,
   _e_elm_win_trap_move,
   _e_elm_win_trap_resize,
   _e_elm_win_trap_center,
   _e_elm_win_trap_lower,
   _e_elm_win_trap_raise,
   _e_elm_win_trap_activate,
   /* alpha_set */ NULL,
   /* aspect_set */ NULL,
   /* avoid_damage_set */ NULL,
   _e_elm_win_trap_borderless_set,
   /* demand_attention_set */ NULL,
   /* focus_skip_set */ NULL,
   /* fullscreen_set */ NULL,
   /* iconified_set */ NULL,
   /* layer_set */ NULL,
   /* manual_render_set */ NULL,
   /* maximized_set */ NULL,
   /* modal_set */ NULL,
   /* name_class_set */ NULL,
   /* object_cursor_set */ NULL,
   _e_elm_win_trap_override_set,
   /* rotation_set */ NULL,
   /* rotation_with_resize_set */ NULL,
   /* shaped_set */ NULL,
   _e_elm_win_trap_size_base_set,
   /* size_step_set */ NULL,
   _e_elm_win_trap_size_min_set,
   _e_elm_win_trap_size_max_set,
   /* sticky_set */ NULL,
   /* title_set */ NULL,
   /* urgent_set */ NULL,
   /* withdrawn_set */ NULL
};

static void
_e_win_client_hook_new(void *d EINA_UNUSED, E_Client *ec)
{
   if (!ec->internal) return;
   if (current_win)
     ec->override = current_win->override;
}

/* externally accessible functions */
EINTERN int
e_win_init(void)
{
   if (!elm_win_trap_set(&_e_elm_win_trap)) return 0;
   e_client_hook_add(E_CLIENT_HOOK_NEW_CLIENT, _e_win_client_hook_new, NULL);
   return 1;
}

EINTERN int
e_win_shutdown(void)
{
   return 1;
}

E_API E_Client *
e_win_client_get(Evas_Object *obj)
{
   Elm_Win_Trap_Ctx *ctx;

   if (!e_obj_is_win(obj)) return NULL;
   ctx = elm_win_trap_data_get(obj);

   return ctx ? ctx->client : NULL;
}

E_API Ecore_Evas *
e_win_ee_get(Evas_Object *obj)
{
   return ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
}

E_API E_Pointer *
e_win_pointer_get(Evas_Object *obj)
{
   Elm_Win_Trap_Ctx *ctx = elm_win_trap_data_get(obj);

   if (!ctx) return NULL;
   return ctx->pointer;
}

E_API Eina_Bool
e_win_centered_get(Evas_Object *obj)
{
   Elm_Win_Trap_Ctx *ctx = elm_win_trap_data_get(obj);

   if (!ctx) return EINA_FALSE;
   return ctx->centered;
}

E_API void
e_win_client_icon_set(Evas_Object *obj, const char *icon)
{
   Elm_Win_Trap_Ctx *ctx = elm_win_trap_data_get(obj);

   if (!ctx) return;
   if (ctx->client)
     eina_stringshare_replace(&ctx->client->internal_icon, icon);
}

E_API void
e_win_client_icon_key_set(Evas_Object *obj, const char *key)
{
   Elm_Win_Trap_Ctx *ctx = elm_win_trap_data_get(obj);

   if (!ctx) return;
   if (ctx->client)
     eina_stringshare_replace(&ctx->client->internal_icon_key, key);
}

E_API void
e_win_placed_set(Evas_Object *obj, Eina_Bool placed)
{
   Elm_Win_Trap_Ctx *ctx = elm_win_trap_data_get(obj);

   if (!ctx) return;
   EINA_SAFETY_ON_NULL_RETURN(obj);
   ctx->placed = !!placed;
   if (ctx->client)
     {
        ctx->client->placed = !!placed;
        EC_CHANGED(ctx->client);
     }
}

E_API void
e_win_no_remember_set(Evas_Object *obj, Eina_Bool no_rem)
{
   Elm_Win_Trap_Ctx *ctx = elm_win_trap_data_get(obj);

   if (!ctx) return;
   EINA_SAFETY_ON_NULL_RETURN(obj);
   ctx->internal_no_remember = !!no_rem;
   if (ctx->client)
     ctx->client->internal_no_remember = !!no_rem;
}

E_API void
e_win_no_reopen_set(Evas_Object *obj, Eina_Bool no_reopen)
{
   Elm_Win_Trap_Ctx *ctx = elm_win_trap_data_get(obj);

   if (!ctx) return;
   EINA_SAFETY_ON_NULL_RETURN(obj);
   ctx->internal_no_reopen = !!no_reopen;
   if (ctx->client)
     ctx->client->internal_no_reopen = !!no_reopen;
}

E_API Evas_Object *
e_elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type)
{
   char *eng;
   Evas_Object *o;

   if (type == ELM_WIN_INLINED_IMAGE) return elm_win_add(parent, name, type);
   eng = eina_strdup(getenv("ELM_ACCEL"));
   e_util_env_set("ELM_ACCEL", "none");
   o = elm_win_add(parent, name, type);
   e_util_env_set("ELM_ACCEL", eng);
   elm_win_alpha_set(o, 1);
   free(eng);
   return o;
}

E_API Evas_Object *
e_elm_win_util_standard_add(const char *name, const char *title)
{
   char *eng;
   Evas_Object *o;

   eng = eina_strdup(getenv("ELM_ACCEL"));
   e_util_env_set("ELM_ACCEL", "none");
   o = elm_win_util_standard_add(name, title);
   e_util_env_set("ELM_ACCEL", eng);
   elm_win_alpha_set(o, 1);
   free(eng);
   return o;
}

E_API Evas_Object *
e_elm_win_util_dialog_add(Evas_Object *parent, const char *name, const char *title)
{
   char *eng;
   Evas_Object *o;

   eng = eina_strdup(getenv("ELM_ACCEL"));
   e_util_env_set("ELM_ACCEL", "none");
   o = elm_win_util_dialog_add(parent, name, title);
   e_util_env_set("ELM_ACCEL", eng);
   elm_win_alpha_set(o, 1);
   free(eng);
   return o;
}
