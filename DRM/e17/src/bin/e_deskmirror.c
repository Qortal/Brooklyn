#include "e.h"

#define SMART_NAME "e_deskmirror"

#define INTERNAL_ENTRY E_Smart_Data *sd; sd = evas_object_smart_data_get(obj); if (!sd) return;

#define API_ENTRY(X)   E_Smart_Data *sd; \
                       sd = evas_object_smart_data_get(X); \
                       if ((!X) || (!sd) || (e_util_strcmp(evas_object_type_get(X), SMART_NAME))) return

typedef struct E_Smart_Data
{
   Evas *e;
   Evas_Object *obj;
   Evas_Object *clip;
   Evas_Object *bgpreview;
   Evas_Object *layout;
   Evas_Object *events;
   Eina_Inlist *mirrors;
   Eina_Hash *mirror_hash;

   Eina_List *handlers;

   Evas_Coord x, y;
   int w, h;

   E_Desk *desk;
   E_Object_Delfn *desk_delfn;

   Eina_Bool pager E_BITFIELD;
   Eina_Bool taskbar E_BITFIELD;

   Eina_Bool resize E_BITFIELD;
   Eina_Bool force E_BITFIELD;
} E_Smart_Data;

typedef struct Mirror
{
   EINA_INLIST;
   E_Smart_Data *sd;
   E_Client *ec;
   Evas_Object *comp_object;
   Evas_Object *mirror;
   int x, y, w, h;
   int ref;
   Eina_Bool added E_BITFIELD;
} Mirror;

typedef struct Mirror_Border
{
   Mirror *m;
   Evas_Object *mirror;
   Evas_Object *frame;
   Evas_Object *obj;
} Mirror_Border;

static Evas_Smart *_e_deskmirror_smart = NULL;
static Evas_Smart *_mirror_client_smart = NULL;

static void _e_deskmirror_mirror_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void _e_deskmirror_mirror_color_set_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED);

static void _e_deskmirror_mirror_setup(Mirror *m);
static void _comp_object_dirty(void *data, Evas_Object *obj, void *event_info EINA_UNUSED);
static void _comp_object_hide(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void _comp_object_show(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void _comp_object_stack(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED);
static void _comp_object_configure(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);

static Eina_Bool
_e_deskmirror_visible_get(E_Smart_Data *sd, Mirror *m)
{
   Eina_Bool visible = evas_object_visible_get(m->comp_object);
   if (m->ec && (!e_object_is_del(E_OBJECT(m->ec))))
     {
        visible = m->ec->visible;
        /* all iconic visibility changes occur with iconic flag set:
         * visibility here is determined by frame visibility
         */
        if (m->sd->handlers && m->ec->iconic)
          visible = evas_object_visible_get(m->ec->frame);
        if (visible)
          {
             visible = (sd->desk == m->ec->desk) || (m->ec->sticky && (!m->ec->hidden));
             if (sd->pager)
               visible = !m->ec->netwm.state.skip_pager;
             if (visible && sd->taskbar)
               visible = !m->ec->netwm.state.skip_taskbar;
          }
     }
   if ((m->w < 1) || (m->h < 1)) visible = EINA_FALSE;
   return visible;
}

static void
_mirror_visible_apply(Mirror *m)
{
   if (_e_deskmirror_visible_get(m->sd, m))
     {
        evas_object_show(m->mirror);
     }
   else
     {
        evas_object_hide(m->mirror);
     }
}

static void
_mirror_scale_set(Mirror *m, float sc)
{
   Edje_Message_Float msg;
   Mirror_Border *mb;

   if (!m->mirror) return;
   mb = evas_object_smart_data_get(m->mirror);
   if (!mb) return;
   msg.val = sc;
   edje_object_message_send(mb->frame, EDJE_MESSAGE_FLOAT, 0, &msg);
}

static void
_mirror_ref(Mirror *m)
{
   m->ref++;
}

static void
_mirror_unref(Mirror *m)
{
   m->ref--;
   if (m->ref > 0) return;
   free(m);
}

static void
_e_deskmirror_smart_reconfigure(E_Smart_Data *sd)
{
   e_layout_freeze(sd->layout);
   evas_object_move(sd->clip, sd->x, sd->y);
   evas_object_move(sd->bgpreview, sd->x, sd->y);
   evas_object_move(sd->layout, sd->x, sd->y);
   if (sd->events)
     evas_object_move(sd->events, sd->x, sd->y);

   if (sd->resize)
     {
        Mirror *m;

        evas_object_resize(sd->clip, sd->w, sd->h);
        evas_object_resize(sd->bgpreview, sd->w, sd->h);
        evas_object_resize(sd->layout, sd->w, sd->h);
        if (sd->events)
          evas_object_resize(sd->events, sd->w, sd->h);
        EINA_INLIST_FOREACH(sd->mirrors, m)
          _mirror_scale_set(m, (float)sd->h / (float)sd->desk->zone->h);
     }
   e_layout_thaw(sd->layout);
   sd->resize = 0;
}

///////////////////////////////////////////////

static void
_e_deskmirror_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd;

   sd = E_NEW(E_Smart_Data, 1);
   if (!sd) return;
   sd->obj = obj;
   sd->e = evas_object_evas_get(obj);
   sd->x = sd->y = sd->w = sd->h = 0;
   sd->clip = evas_object_rectangle_add(sd->e);
   evas_object_smart_member_add(sd->clip, sd->obj);
   evas_object_smart_data_set(obj, sd);
}

static void
_e_deskmirror_smart_del(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   e_object_unref(E_OBJECT(sd->desk));
   if (sd->desk_delfn)
     {
        e_object_delfn_del(E_OBJECT(sd->desk), sd->desk_delfn);
        sd->desk_delfn = NULL;
        sd->desk = NULL;
     }
   E_FREE_LIST(sd->handlers, ecore_event_handler_del);
   eina_hash_free(sd->mirror_hash);
   evas_object_del(sd->clip);
   evas_object_del(sd->bgpreview);
   evas_object_del(sd->layout);
   evas_object_del(sd->events);
   free(sd);
}

static void
_e_deskmirror_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   sd->x = x;
   sd->y = y;
   _e_deskmirror_smart_reconfigure(sd);
}

static void
_e_deskmirror_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   sd->w = w;
   sd->h = h;
   sd->resize = 1;
   _e_deskmirror_smart_reconfigure(sd);
}

static void
_e_deskmirror_smart_show(Evas_Object *obj)
{
   Mirror *m;

   INTERNAL_ENTRY;
   EINA_INLIST_FOREACH(sd->mirrors, m)
     {
        Mirror_Border *mb;

        if ((!m->ec) || (!m->mirror)) continue;
        mb = evas_object_smart_data_get(m->mirror);
        if (!mb) continue;
        edje_object_thaw(mb->frame);
        edje_object_play_set(mb->frame, 1);
        evas_object_show(mb->mirror);
     }
   evas_object_show(sd->clip);
}

static void
_e_deskmirror_smart_hide(Evas_Object *obj)
{
   Mirror *m;

   INTERNAL_ENTRY;
   EINA_INLIST_FOREACH(sd->mirrors, m)
     {
        Mirror_Border *mb;

        if ((!m->ec) || (!m->mirror)) continue;
        mb = evas_object_smart_data_get(m->mirror);
        if (!mb) continue;
        edje_object_freeze(mb->frame);
        edje_object_play_set(mb->frame, 0);
        evas_object_hide(mb->mirror);
     }
   evas_object_hide(sd->clip);
}

static void
_e_deskmirror_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   evas_object_color_set(sd->clip, r, g, b, a);
}

static void
_e_deskmirror_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   INTERNAL_ENTRY;
   evas_object_clip_set(sd->clip, clip);
}

static void
_e_deskmirror_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_clip_unset(sd->clip);
}

////////////////////////////////////////////////////////

static void
_mirror_client_theme_setup(Mirror_Border *mb, Evas_Object *o)
{
   char buf[4096];

   if ((!mb) || (!mb->m) || (!mb->m->ec)) return;
   if (e_comp_object_frame_exists(mb->m->ec->frame))
     snprintf(buf, sizeof(buf), "e/deskmirror/frame/%s", mb->m->ec->border.name);
   else
     snprintf(buf, sizeof(buf), "e/deskmirror/frame/borderless");
   e_theme_edje_object_set(o, "base/theme/borders", buf);
   if (e_client_util_shadow_state_get(mb->m->ec))
     edje_object_signal_emit(o, "e,state,shadow,on", "e");
   else
     edje_object_signal_emit(o, "e,state,shadow,off", "e");
   if (mb->m->ec->focused)
     edje_object_signal_emit(o, "e,state,focused", "e");
   if (mb->m->ec->shaded)
     edje_object_signal_emit(o, "e,state,shaded", "e");
   if (mb->m->ec->maximized)
     edje_object_signal_emit(o, "e,action,maximize", "e");
   if (mb->m->ec->sticky)
     edje_object_signal_emit(o, "e,state,sticky", "e");
   if (mb->m->ec->iconic)
     edje_object_signal_emit(o, "e,action,iconify", "e");
}

static void
_e_deskmirror_mirror_frame_recalc_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Mirror *m = data;
   Mirror_Border *mb;

   if (!m->mirror) return;
   mb = evas_object_smart_data_get(m->mirror);
   if (mb->m->ec && (!e_object_is_del(E_OBJECT(mb->m->ec))))
     {
        _mirror_client_theme_setup(mb, mb->frame);
        _mirror_scale_set(mb->m, (float)mb->m->sd->h / (float)mb->m->sd->desk->zone->h);
     }
   else
     {
        m = mb->m;
        if (m->comp_object)
          {
             evas_object_smart_callback_del_full(m->comp_object, "dirty", _comp_object_dirty, m);
             evas_object_smart_callback_del_full(m->comp_object, "frame_recalc_done", _e_deskmirror_mirror_frame_recalc_cb, m);
             evas_object_smart_callback_del_full(m->comp_object, "color_set", _e_deskmirror_mirror_color_set_cb, m);
             evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_DEL, _e_deskmirror_mirror_del_cb, m);
             evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_SHOW, (Evas_Object_Event_Cb)_comp_object_show, m);
             evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_HIDE, (Evas_Object_Event_Cb)_comp_object_hide, m);
             evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_RESTACK, (Evas_Object_Event_Cb)_comp_object_stack, m);
             evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)_comp_object_configure, m);
             evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_MOVE, (Evas_Object_Event_Cb)_comp_object_configure, m);
             m->comp_object = NULL;
          }
     }
}

static void
_e_deskmirror_mirror_color_set_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Mirror *m = data;
   int r, g, b, a;

   if (!m->mirror) return;
   evas_object_color_get(obj, &r, &g, &b, &a);
   evas_object_color_set(m->mirror, r, g, b, a);
}

static void
_mirror_client_smart_add(Evas_Object *obj)
{
   Mirror_Border *mb;

   mb = E_NEW(Mirror_Border, 1);
   mb->obj = obj;
   evas_object_smart_data_set(obj, mb);
}

static void
_mirror_client_signal_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *emission, const char *src)
{
   if (!strncmp(emission, "e,state,shadow", sizeof("e,state,shadow") - 1)) return;
   edje_object_signal_emit(data, emission, src);
   edje_object_message_signal_process(data);
   edje_object_calc_force(data);
}

static void
_mirror_client_shadow_change(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   E_Client *ec = event_info;

   if (e_client_util_shadow_state_get(ec))
     edje_object_signal_emit(data, "e,state,shadow,on", "e");
   else
     edje_object_signal_emit(data, "e,state,shadow,off", "e");
}

static void
_mirror_client_smart_del(Evas_Object *obj)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);
   if (mb->m->comp_object && mb->m->ec)
     {
        e_comp_object_signal_callback_del_full(mb->m->ec->frame, "*", "*", _mirror_client_signal_cb, mb->frame);
        evas_object_smart_callback_del_full(mb->m->ec->frame, "shadow_change", _mirror_client_shadow_change, mb->frame);
     }
   evas_object_del(mb->frame);
   mb->frame = NULL;
   evas_object_del(mb->mirror);
   mb->mirror = NULL;
   _mirror_unref(mb->m);
   free(mb);
}

static void
_mirror_client_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);

   evas_object_move(mb->frame, x, y);
}

static void
_mirror_client_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);
   evas_object_resize(mb->frame, w, h);
}

static void
_mirror_client_smart_show(Evas_Object *obj)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);
   evas_object_show(mb->frame);
}

static void
_mirror_client_smart_hide(Evas_Object *obj)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);
   evas_object_hide(mb->frame);
}

static void
_mirror_client_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);
   evas_object_color_set(mb->frame, r, g, b, a);
}

static void
_mirror_client_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);
   evas_object_clip_set(mb->frame, clip);
}

static void
_mirror_client_smart_clip_unset(Evas_Object *obj)
{
   Mirror_Border *mb = evas_object_smart_data_get(obj);
   evas_object_clip_unset(mb->frame);
}

static void
_mirror_client_smart_init(void)
{
   static const Evas_Smart_Class sc =
     {
        "mirror_border", EVAS_SMART_CLASS_VERSION,
        _mirror_client_smart_add, _mirror_client_smart_del, _mirror_client_smart_move, _mirror_client_smart_resize,
        _mirror_client_smart_show, _mirror_client_smart_hide, _mirror_client_smart_color_set, _mirror_client_smart_clip_set,
        _mirror_client_smart_clip_unset, NULL, NULL, NULL, NULL, NULL, NULL, NULL
     };
   if (_mirror_client_smart) return;
   _mirror_client_smart = evas_smart_class_new(&sc);
}

static void
_e_deskmirror_smart_init(void)
{
   static const Evas_Smart_Class sc =
     {
        SMART_NAME, EVAS_SMART_CLASS_VERSION,
        _e_deskmirror_smart_add, _e_deskmirror_smart_del, _e_deskmirror_smart_move, _e_deskmirror_smart_resize,
        _e_deskmirror_smart_show, _e_deskmirror_smart_hide, _e_deskmirror_smart_color_set, _e_deskmirror_smart_clip_set,
        _e_deskmirror_smart_clip_unset, NULL, NULL, NULL, NULL, NULL, NULL, NULL
     };
   if (_e_deskmirror_smart) return;
   _e_deskmirror_smart = evas_smart_class_new(&sc);
}

static void
_e_deskmirror_delfn(E_Smart_Data *sd, void *desk EINA_UNUSED)
{
   sd->desk_delfn = NULL;
   sd->desk = NULL;
   evas_object_del(sd->obj);
}

static void
_e_deskmirror_mirror_del(Mirror *m)
{
   void *val = m->comp_object;

   m->comp_object = NULL;
   eina_hash_del_by_key(m->sd->mirror_hash, &val);
}

static void
_e_deskmirror_mirror_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _e_deskmirror_mirror_del(data);
}

static void
_e_deskmirror_mirror_geometry_get(Mirror *m)
{
   E_Zone *zone;

   evas_object_geometry_get(m->comp_object, &m->x, &m->y, &m->w, &m->h);
   zone = e_comp_object_util_zone_get(m->comp_object);
   if (zone)
     m->x -= zone->x, m->y -= zone->y;
   /* double check here if we get zeroes */
   if ((!m->w) || (!m->h))
     {
        ERR("ACK!");
     }
}

static void
_e_deskmirror_mirror_reconfigure(Mirror *m)
{
   _e_deskmirror_mirror_geometry_get(m);
   if (m->mirror)
     {
        e_layout_child_move(m->mirror, m->x, m->y);
        e_layout_child_resize(m->mirror, m->w, m->h);
        /* assume that anything happening here is the result of a drag */
        if (((!m->sd->force) && (!e_drag_current_get())) || (m->sd->force))
          _mirror_visible_apply(m);
     }
}

static void
_e_deskmirror_mirror_del_hash(Mirror *m)
{
   m->sd->mirrors = eina_inlist_remove(m->sd->mirrors, EINA_INLIST_GET(m));
   if (m->mirror)
     {
        evas_object_del(m->mirror);
        m->mirror = NULL;
     }
   if (m->comp_object)
     {
        evas_object_smart_callback_del_full(m->comp_object, "dirty", _comp_object_dirty, m);
        evas_object_smart_callback_del_full(m->comp_object, "frame_recalc_done", _e_deskmirror_mirror_frame_recalc_cb, m);
        evas_object_smart_callback_del_full(m->comp_object, "color_set", _e_deskmirror_mirror_color_set_cb, m);
        evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_DEL, _e_deskmirror_mirror_del_cb, m);
        evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_SHOW, (Evas_Object_Event_Cb)_comp_object_show, m);
        evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_HIDE, (Evas_Object_Event_Cb)_comp_object_hide, m);
        evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_RESTACK, (Evas_Object_Event_Cb)_comp_object_stack, m);
        evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)_comp_object_configure, m);
        evas_object_event_callback_del_full(m->comp_object, EVAS_CALLBACK_MOVE, (Evas_Object_Event_Cb)_comp_object_configure, m);
        m->comp_object = NULL;
     }
   _mirror_unref(m);
}

static Evas_Object *
_mirror_client_new(Mirror *m)
{
   Evas_Object *o;
   Mirror_Border *mb;

   _mirror_client_smart_init();
   o = evas_object_smart_add(m->sd->e, _mirror_client_smart);
   mb = evas_object_smart_data_get(o);
   mb->m = m;
   _mirror_ref(m);
   mb->frame = edje_object_add(m->sd->e);
   evas_object_name_set(mb->frame, "mirror_border");
   _mirror_client_theme_setup(mb, mb->frame);
   if (m->comp_object)
     {
        e_comp_object_signal_callback_add(m->comp_object, "*", "*", _mirror_client_signal_cb, mb->frame);
        evas_object_smart_callback_add(m->comp_object, "shadow_change", _mirror_client_shadow_change, mb->frame);
     }

   mb->mirror = m->mirror;
   evas_object_smart_member_add(mb->frame, o);
   evas_object_name_set(mb->mirror, "mirror");
   edje_object_part_swallow(mb->frame, "e.swallow.client", m->mirror);
   edje_object_part_text_set(mb->frame, "e.text.title", e_client_util_name_get(m->ec));
   return o;
}

static void
_e_deskmirror_mirror_setup(Mirror *m)
{
   if (!m->mirror) return;
   if (m->ec)
     {
        m->mirror = _mirror_client_new(m);
        _mirror_scale_set(m, (double)m->sd->h / (double)m->sd->desk->zone->h);
     }
   else
     {
        evas_object_pass_events_set(m->mirror, !m->ec);
     }
   if (m->ec) evas_object_data_set(m->mirror, "E_Client", m->ec);
   evas_object_precise_is_inside_set(m->mirror, m->ec && (m->ec->shaped || m->ec->shaped_input));
   e_layout_pack(m->sd->layout, m->mirror);
   _e_deskmirror_mirror_reconfigure(m);
   if (m->sd->handlers) // no handlers = we're setting up = there's no possible listeners
     {
        _comp_object_stack(m, NULL, m->comp_object, NULL);
        if (!m->added)
          evas_object_smart_callback_call(m->sd->obj, "mirror_add", m->mirror);
     }
   else
     e_layout_child_raise(m->mirror);
   m->added = 1;
}

static Eina_Bool
_comp_object_check(Mirror *m)
{
   int w, h;

   evas_object_geometry_get(m->comp_object, NULL, NULL, &w, &h);
   if ((w < 2) || (h < 2)) return EINA_FALSE;
   if (!m->mirror)
     {
        m->mirror = e_comp_object_util_mirror_add(m->comp_object);
        if (!m->mirror) return EINA_FALSE;
     }
   evas_object_smart_callback_del(m->comp_object, "dirty", _comp_object_dirty);
   if (m->added) return EINA_TRUE;
   evas_object_name_set(m->mirror, "m->mirror");
   _e_deskmirror_mirror_setup(m);
   return EINA_TRUE;
}

static void
_comp_object_hide(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _mirror_visible_apply(m);
}

static void
_comp_object_show(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!m->mirror)
     {
        if (!_comp_object_check(m)) return;
     }
   _mirror_visible_apply(m);
}

static void
_comp_object_stack(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *comp_object;

   if (!m->mirror)
     {
        if (!_comp_object_check(m)) return;
     }
   comp_object = evas_object_below_get(obj);
   while (comp_object)
     {
        Mirror *m2;

        m2 = eina_hash_find(m->sd->mirror_hash, &comp_object);
        if (m2 && m2->mirror)
          {
             e_layout_child_raise_above(m->mirror, m2->mirror);
             return;
          }
        comp_object = evas_object_below_get(comp_object);
     }
   e_layout_child_lower(m->mirror);
}

static void
_comp_object_configure(Mirror *m, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!m->mirror)
     {
        if (!_comp_object_check(m)) return;
     }
   _e_deskmirror_mirror_reconfigure(m);
}

static void
_comp_object_dirty(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _comp_object_check(data);
}

static Mirror *
_e_deskmirror_mirror_add(E_Smart_Data *sd, Evas_Object *obj)
{
   Mirror *m;
   Evas_Object *o = NULL;
   E_Client *ec;
   int w, h;

   if (sd->desk->zone != e_comp_object_util_zone_get(obj)) return NULL;

   ec = e_comp_object_client_get(obj);
   if (ec)
     {
        if ((ec->desk != sd->desk) && (!ec->sticky))
          return NULL;
        if (ec->input_only || ec->ignored)
          return NULL;
     }
   else
     {
        E_Shelf *es;

        if (sd->pager || sd->taskbar) return NULL;
        es = evas_object_data_get(obj, "E_Shelf");
        if (es)
          {
             if (!e_shelf_desk_visible(es, sd->desk)) return NULL;
          }
        else
          {
             if (sd->desk != e_desk_current_get(sd->desk->zone)) return NULL;
          }
     }
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if ((w > 1) && (h > 1))
     {
        o = e_comp_object_util_mirror_add(obj);
        evas_object_name_set(o, "m->mirror");
     }
   m = calloc(1, sizeof(Mirror));
   m->comp_object = obj;
   m->ec = ec;
   m->sd = sd;
   m->mirror = o;
   m->ref = 1;
   evas_object_event_callback_add(m->comp_object, EVAS_CALLBACK_DEL, _e_deskmirror_mirror_del_cb, m);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, (Evas_Object_Event_Cb)_comp_object_show, m);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, (Evas_Object_Event_Cb)_comp_object_hide, m);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESTACK, (Evas_Object_Event_Cb)_comp_object_stack, m);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)_comp_object_configure, m);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, (Evas_Object_Event_Cb)_comp_object_configure, m);
   evas_object_smart_callback_add(obj, "frame_recalc_done", _e_deskmirror_mirror_frame_recalc_cb, m);
   evas_object_smart_callback_add(obj, "color_set", _e_deskmirror_mirror_color_set_cb, m);
   if (ec && (!ec->redirected) && (!ec->new_client) && e_pixmap_usable_get(ec->pixmap))
     evas_object_smart_callback_add(obj, "dirty", _comp_object_dirty, m);
   sd->mirrors = eina_inlist_append(sd->mirrors, EINA_INLIST_GET(m));
   eina_hash_add(sd->mirror_hash, &obj, m);
   _e_deskmirror_mirror_setup(m);
   return m;
}

static Eina_Bool
_comp_object_add(E_Smart_Data *sd, int type EINA_UNUSED, E_Event_Comp_Object *ev)
{
   if (eina_hash_find(sd->mirror_hash, &ev->comp_object)) return ECORE_CALLBACK_RENEW;
   _e_deskmirror_mirror_add(sd, ev->comp_object);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_del(E_Smart_Data *sd, int type EINA_UNUSED, E_Event_Client *ev)
{
   Mirror *m = eina_hash_find(sd->mirror_hash, &ev->ec->frame);

   if (m) m->ec = NULL;

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_add(E_Smart_Data *sd, int type EINA_UNUSED, E_Event_Client *ev)
{
   if (!eina_hash_find(sd->mirror_hash, &ev->ec->frame))
     _e_deskmirror_mirror_add(sd, ev->ec->frame);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_desk_set(E_Smart_Data *sd, int type EINA_UNUSED, E_Event_Client_Desk_Set *ev)
{
   Mirror *m;

   m = eina_hash_find(sd->mirror_hash, &ev->ec->frame);
   if (m)
     {
        /* ev->desk is previous desk */
        _mirror_visible_apply(m);
        if (!e_client_util_desk_visible(ev->ec, sd->desk))
          eina_hash_del_by_key(sd->mirror_hash, &ev->ec->frame);
     }
   if ((!m) && (sd->desk == ev->ec->desk))
     _e_deskmirror_mirror_add(sd, ev->ec->frame);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_client_property(E_Smart_Data *sd, int type EINA_UNUSED, E_Event_Client_Property *ev)
{
   Mirror *m;

   if ((!(ev->property & E_CLIENT_PROPERTY_NETWM_STATE)) &&
       (!(ev->property & E_CLIENT_PROPERTY_STICKY)))
     return ECORE_CALLBACK_RENEW;

   m = eina_hash_find(sd->mirror_hash, &ev->ec->frame);
   if (m)
     {
        _mirror_visible_apply(m);
        if (ev->property & E_CLIENT_PROPERTY_STICKY)
          {
             if ((!ev->ec->sticky) && (ev->ec->desk != sd->desk))
               eina_hash_del_by_key(sd->mirror_hash, &ev->ec->frame);
          }
     }
   else if (ev->ec->sticky)
     _e_deskmirror_mirror_add(sd, ev->ec->frame);
   return ECORE_CALLBACK_RENEW;
}

/* externally accessible functions */
E_API Evas_Object *
e_deskmirror_add(E_Desk *desk, Eina_Bool pager, Eina_Bool taskbar)
{
   E_Smart_Data *sd;
   Evas_Object *o, *l;
   Evas *e;

   e = e_comp->evas;
   _e_deskmirror_smart_init();
   o = evas_object_smart_add(e, _e_deskmirror_smart);
   e_object_ref(E_OBJECT(desk));
   sd = evas_object_smart_data_get(o);
   sd->pager = !!pager;
   sd->taskbar = !!taskbar;
   sd->desk = desk;
   sd->mirror_hash = eina_hash_pointer_new((Eina_Free_Cb)_e_deskmirror_mirror_del_hash);
   sd->desk_delfn = e_object_delfn_add(E_OBJECT(desk), (Ecore_End_Cb)_e_deskmirror_delfn, sd);
   if (pager || taskbar)
     {
        sd->events = evas_object_rectangle_add(e);
        evas_object_color_set(sd->events, 0, 0, 0, 0);
        evas_object_clip_set(sd->events, sd->clip);
        evas_object_smart_member_add(sd->events, o);
        evas_object_show(sd->events);
     }
   sd->bgpreview = e_widget_bgpreview_desk_add(e, desk->zone, desk->x, desk->y);
   evas_object_pass_events_set(sd->bgpreview, 1);
   evas_object_clip_set(sd->bgpreview, sd->clip);
   evas_object_smart_member_add(sd->bgpreview, o);
   evas_object_show(sd->bgpreview);
   sd->layout = e_layout_add(e);
   evas_object_clip_set(sd->layout, sd->clip);
   e_layout_virtual_size_set(sd->layout, desk->zone->w, desk->zone->h);
   evas_object_smart_member_add(sd->layout, o);
   evas_object_show(sd->layout);

   e_comp_object_util_center_on(o, sd->desk->zone->bg_clip_object);

   e_layout_freeze(sd->layout);

   l = evas_object_bottom_get(e_comp->evas);
   do
     {
        if (evas_object_data_get(l, "comp_object"))
          _e_deskmirror_mirror_add(sd, l);
        l = evas_object_above_get(l);
     } while (l);

   e_layout_thaw(sd->layout);

   E_LIST_HANDLER_APPEND(sd->handlers, E_EVENT_COMP_OBJECT_ADD, (Ecore_Event_Handler_Cb)_comp_object_add, sd);
   E_LIST_HANDLER_APPEND(sd->handlers, E_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)_client_add, sd);
   E_LIST_HANDLER_APPEND(sd->handlers, E_EVENT_CLIENT_REMOVE, (Ecore_Event_Handler_Cb)_client_del, sd);
   E_LIST_HANDLER_APPEND(sd->handlers, E_EVENT_CLIENT_PROPERTY, (Ecore_Event_Handler_Cb)_client_property, sd);
   E_LIST_HANDLER_APPEND(sd->handlers, E_EVENT_CLIENT_DESK_SET, (Ecore_Event_Handler_Cb)_client_desk_set, sd);
   return o;
}

E_API Evas_Object *
e_deskmirror_mirror_find(Evas_Object *deskmirror, Evas_Object *comp_object)
{
   Mirror *m;
   EINA_SAFETY_ON_NULL_RETURN_VAL(deskmirror, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(comp_object, NULL);
   API_ENTRY(deskmirror) NULL;

   m = eina_hash_find(sd->mirror_hash, &comp_object);
   return m ? m->mirror : NULL;
}

E_API Eina_List *
e_deskmirror_mirror_list(Evas_Object *deskmirror)
{
   Eina_List *l = NULL;
   Mirror *m;

   EINA_SAFETY_ON_NULL_RETURN_VAL(deskmirror, NULL);
   API_ENTRY(deskmirror) NULL;

   EINA_INLIST_FOREACH(sd->mirrors, m)
     {
        if (m->mirror)
          l = eina_list_append(l, m->mirror);
     }
   return l;
}

static void
_mirror_copy_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   e_comp_object_signal_callback_del_full(data, "*", "*", _mirror_client_signal_cb, obj);
   evas_object_smart_callback_del_full(data, "shadow_change", _mirror_client_shadow_change, obj);
}

E_API Evas_Object *
e_deskmirror_mirror_copy(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   if (!e_util_strcmp(evas_object_type_get(obj), "mirror_border"))
     {
        Evas_Object *o, *oo;
        Edje_Message_Float msg;
        Mirror_Border *mb;

        mb = evas_object_smart_data_get(obj);
        o = edje_object_add(evas_object_evas_get(obj));
        _mirror_client_theme_setup(mb, o);
        if (mb->m->comp_object)
          {
             e_comp_object_signal_callback_add(mb->m->comp_object, "*", "*", _mirror_client_signal_cb, o);
             evas_object_smart_callback_add(mb->m->comp_object, "shadow_change", _mirror_client_shadow_change, o);
             evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _mirror_copy_del, mb->m->comp_object);
          }
        msg.val = mb->m->sd->h / mb->m->sd->desk->zone->h;
        edje_object_message_send(o, EDJE_MESSAGE_FLOAT, 0, &msg);

        oo = e_comp_object_util_mirror_add(mb->m->comp_object);
        edje_object_part_swallow(o, "e.swallow.client", oo);
        if (e_comp_object_frame_exists(mb->m->ec->frame))
          edje_object_part_text_set(o, "e.text.title", e_client_util_name_get(mb->m->ec));
        e_comp_object_util_del_list_append(o, oo);
        return o;
     }
   else if (!e_util_strcmp(evas_object_type_get(obj), "image"))
     return e_comp_object_util_mirror_add(obj);
   CRI("NOT A DESKMIRROR CLIENT");
   return NULL;
}

E_API void
e_deskmirror_coord_canvas_to_virtual(Evas_Object *obj, Evas_Coord cx, Evas_Coord cy, Evas_Coord *vx, Evas_Coord *vy)
{
   API_ENTRY(obj);

   e_layout_coord_canvas_to_virtual(sd->layout, cx, cy, vx, vy);
}

E_API void
e_deskmirror_coord_virtual_to_canvas(Evas_Object *obj, Evas_Coord vx, Evas_Coord vy, Evas_Coord *cx, Evas_Coord *cy)
{
   API_ENTRY(obj);

   e_layout_coord_virtual_to_canvas(sd->layout, vx, vy, cx, cy);
}

E_API E_Desk *
e_deskmirror_desk_get(Evas_Object *obj)
{
   API_ENTRY(obj) NULL;
   return sd->desk;
}

E_API void
e_deskmirror_util_wins_print(Evas_Object *obj)
{
   E_Smart_Data *sd;
   Mirror *m;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   sd = evas_object_smart_data_get(obj);
   EINA_INLIST_FOREACH(sd->mirrors, m)
     {
        if (m->ec)
          {
             if (m->ec->override)
               fprintf(stderr, "MIRROR OVERRIDE: %p - %p%s\n", m->comp_object, m->ec, m->ec->input_only ? " INPUT" : "");
             else
               fprintf(stderr, "MIRROR EC:  %p - %p '%s:%s'\n", m->comp_object, m->ec, m->ec->icccm.name, m->ec->icccm.class);
          }
        else
          fprintf(stderr, "MIRROR OBJ: %p\n", m->comp_object);
     }
}

E_API void
e_deskmirror_update_force(Evas_Object *obj)
{
   Mirror *m;

   API_ENTRY(obj);

   e_layout_freeze(sd->layout);
   sd->force = EINA_TRUE;
   _e_deskmirror_smart_hide(obj);
   EINA_INLIST_FOREACH(sd->mirrors, m)
     _e_deskmirror_mirror_reconfigure(m);
   _e_deskmirror_smart_reconfigure(sd);
   _e_deskmirror_smart_show(obj);
   sd->force = EINA_FALSE;
   e_layout_thaw(sd->layout);
}
