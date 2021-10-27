#include "e_mod_tiling.h"

/* There are two major concepts, (un)track and add/remove client.
 * track - track all windows regardless if we are interested in them or not.
 *    We need that in order to keep proper track of things as they change.
 * add/remove: Clients should be tiled/untiled.
 */

/* types {{{ */

#define TILING_POPUP_TIMEOUT 0.8
#define TILING_POPUP_SIZE 100

static Eina_Bool started = EINA_FALSE;

typedef struct geom_t
{
   int x, y, w, h;
} geom_t;

typedef enum {
   POSITION_TOP = 0,
   POSITION_RIGHT = 1,
   POSITION_BOTTOM = 2,
   POSITION_LEFT = 3
} Position_On_Client;

typedef struct Client_Extra
{
   E_Client *client;
   geom_t    expected;
   struct
   {
      Eina_Bool drag;
      Evas_Object *hint, *ic;
      Ecore_Event_Handler *move, *up;
      int x,y; /* start points */
   } drag;
   struct
   {
      geom_t      geom;
      E_Maximize  maximized;
      const char *bordername;
   } orig;
   int       last_frame_adjustment; // FIXME: Hack for frame resize bug.
   Eina_Bool floating E_BITFIELD;
   Eina_Bool tiled E_BITFIELD;
   Eina_Bool tracked E_BITFIELD;
} Client_Extra;

typedef struct _Instance
{
   E_Gadcon_Client  *gcc;
   Evas_Object      *gadget;
   Eina_Stringshare *gad_id;

   E_Menu           *lmenu;
} Instance;

typedef struct {
   E_Desk *desk;
   Tiling_Split_Type type;
} Desk_Split_Type;

struct tiling_g tiling_g = {
   .module = NULL,
   .config = NULL,
   .log_domain = -1,
};

static void _client_track(E_Client *ec);
static void _client_untrack(E_Client *ec);
static Eina_Bool _add_client(E_Client *ec, Tiling_Split_Type type);
static void             _remove_client(E_Client *ec);
static void             _client_apply_settings(E_Client *ec, Client_Extra *extra);
static void             _foreach_desk(void (*func)(E_Desk *desk));
static Eina_Bool _toggle_tiling_based_on_state(E_Client *ec, Eina_Bool restore);
static void _edje_tiling_icon_set(Evas_Object *o);
static void _desk_config_apply(E_Desk *d, int old_nb_stacks, int new_nb_stacks);
static void _update_current_desk(E_Desk *new);
static void _client_drag_terminate(E_Client *ec);

/* Func Proto Requirements for Gadcon */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void             _gc_shutdown(E_Gadcon_Client *gcc);
static void             _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);

static const char  *_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas);
static const char  *_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED);

static void         _gadget_icon_set(Instance *inst);

/* }}} */
/* Globals {{{ */

static struct tiling_mod_main_g
{
   char                 edj_path[PATH_MAX];
   E_Config_DD         *config_edd, *vdesk_edd;
   Ecore_Event_Handler *handler_client_resize, *handler_client_move,
                       *handler_client_iconify, *handler_client_uniconify,
                       *handler_desk_set, *handler_compositor_resize,
                       *handler_desk_show;
   E_Client_Hook       *handler_client_resize_begin, *handler_client_add,
                       *handler_move_begin, *handler_move_end;
   E_Client_Menu_Hook  *client_menu_hook;

   Tiling_Info         *tinfo;
   Eina_Hash           *info_hash;
   Eina_Hash           *client_extras;
   Eina_Hash           *desk_type;

   E_Action            *act_togglefloat, *act_move_up, *act_move_down, *act_move_left,
                       *act_move_right, *act_toggle_split_mode, *act_swap_window;

   Desk_Split_Type     *current_split_type;
   Ecore_Job           *apply_tree_job;

   struct {
        Evas_Object *comp_obj;
        Evas_Object *obj;
        Ecore_Timer *timer;
        E_Desk *desk;
   } split_popup;
} _G =
{

};

/* Define the class and gadcon functions this module provides */
static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, "tiling",
   { _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new,
     NULL, NULL },
   E_GADCON_CLIENT_STYLE_PLAIN
};

/* }}} */
/* Utils {{{ */

/* I wonder why no one has implemented the following one yet? */
static E_Desk *
get_current_desk(void)
{
   E_Zone *z = e_zone_current_get();

   return e_desk_current_get(z);
}

static Tiling_Split_Type
_current_tiled_state(Eina_Bool allow_float)
{
   //update the current desk in case something has changed it
   _update_current_desk(get_current_desk());

   if (!_G.current_split_type)
     {
        ERR("Invalid state, the current field can never be NULL");
        return TILING_SPLIT_HORIZONTAL;
     }

   if (!allow_float &&
       _G.current_split_type->type == TILING_SPLIT_FLOAT)
     return TILING_SPLIT_HORIZONTAL;
   return _G.current_split_type->type;
}

static Tiling_Info *
_initialize_tinfo(const E_Desk *desk)
{
   Tiling_Info *tinfo;

   tinfo = E_NEW(Tiling_Info, 1);
   tinfo->desk = desk;
   eina_hash_direct_add(_G.info_hash, &tinfo->desk, tinfo);

   tinfo->conf =
     get_vdesk(tiling_g.config->vdesks, desk->x, desk->y, desk->zone->num);

   return tinfo;
}

static void
check_tinfo(const E_Desk *desk)
{
   if (!desk) return;
   if (!_G.tinfo || _G.tinfo->desk != desk)
     {
        _G.tinfo = eina_hash_find(_G.info_hash, &desk);
        if (!_G.tinfo)
          {
             /* lazy init */
             _G.tinfo = _initialize_tinfo(desk);
          }
        if (!_G.tinfo->conf)
          {
             _G.tinfo->conf =
               get_vdesk(tiling_g.config->vdesks, desk->x, desk->y,
                         desk->zone->num);
          }
     }
}

static Eina_Bool
desk_should_tile_check(const E_Desk *desk)
{
   check_tinfo(desk);
   return _G.tinfo && _G.tinfo->conf && _G.tinfo->conf->nb_stacks;
}

static int
is_ignored_window(const Client_Extra *extra)
{
   if (extra->client->sticky || extra->client->maximized ||
         extra->client->fullscreen || extra->floating)
     return true;

   return false;
}

static int
is_tilable(const E_Client *ec)
{
   if (ec->icccm.min_h == ec->icccm.max_h && ec->icccm.max_h > 0)
     return false;

   if (ec->e.state.centered || e_win_centered_get(ec->internal_elm_win))
     return false;

   if (!tiling_g.config->tile_dialogs && ((ec->icccm.transient_for != 0) ||
                                          (ec->netwm.type == E_WINDOW_TYPE_DIALOG)))
     return false;

   if (ec->fullscreen)
      return false;

   if (ec->maximized)
      return false;

   if (ec->iconic)
      return false;

   if (ec->sticky)
      return false;

   if (e_client_util_ignored_get(ec))
     return false;

   if (e_object_is_del(E_OBJECT(ec)))
     return false;

   return true;
}

static void
change_window_border(E_Client *ec, const char *bordername)
{
   if (ec->mwm.borderless)
      return;

   ec->border.changed = 0;
   if (e_client_border_set(ec, bordername))
     eina_stringshare_refplace(&ec->bordername, ec->border.name);

   DBG("%p -> border %s", ec, bordername);
}

static Eina_Bool
_info_hash_update(const Eina_Hash *hash EINA_UNUSED,
                  const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Tiling_Info *tinfo = data;
   int old_nb_stacks = 0, new_nb_stacks = 0;

   if (tinfo->conf)
     {
        old_nb_stacks = tinfo->conf->nb_stacks;
     }

   if (tinfo->desk)
     {
        tinfo->conf =
          get_vdesk(tiling_g.config->vdesks, tinfo->desk->x, tinfo->desk->y,
                    tinfo->desk->zone->num);

        if (tinfo->conf)
          {
             new_nb_stacks = tinfo->conf->nb_stacks;
          }

        _desk_config_apply((E_Desk *) tinfo->desk, old_nb_stacks, new_nb_stacks);
     }
   else
     {
        tinfo->conf = NULL;
     }

   return true;
}

void
e_tiling_update_conf(void)
{
   eina_hash_foreach(_G.info_hash, _info_hash_update, NULL);
}

static void
_e_client_move_resize(E_Client *ec, int x, int y, int w, int h)
{
   Client_Extra *extra;

   extra = eina_hash_find(_G.client_extras, &ec);
   if (!extra)
     {
        ERR("No extra for %p", ec);
        return;
     }

   extra->last_frame_adjustment =
     MAX(ec->h - ec->client.h, ec->w - ec->client.w);
   DBG("%p -> %dx%d+%d+%d", ec, w, h, x, y);
   evas_object_geometry_set(ec->frame, x, y, w, h);
}

static void
_e_client_unmaximize(E_Client *ec, E_Maximize max)
{
   DBG("%p -> %s", ec,
       (max & E_MAXIMIZE_DIRECTION) ==
       E_MAXIMIZE_NONE ? "NONE" : (max & E_MAXIMIZE_DIRECTION) ==
       E_MAXIMIZE_VERTICAL ? "VERTICAL" : (max & E_MAXIMIZE_DIRECTION) ==
       E_MAXIMIZE_HORIZONTAL ? "HORIZONTAL" : "BOTH");
   e_client_unmaximize(ec, max);
}

static Client_Extra *
_restore_client(E_Client *ec)
{
   Client_Extra *extra;

   extra = eina_hash_find(_G.client_extras, &ec);
   if (!extra)
     {
        ERR("No extra for %p", ec);
        return NULL;
     }

   if (!extra->tiled)
     return NULL;

   _client_untrack(ec);
   if (!ec->maximized && !ec->fullscreen)
     {
        _e_client_move_resize(ec, extra->orig.geom.x, extra->orig.geom.y,
              extra->orig.geom.w, extra->orig.geom.h);
        if (extra->orig.maximized != ec->maximized)
          {
             e_client_maximize(ec, extra->orig.maximized);
             ec->maximized = extra->orig.maximized;
          }
     }

   DBG("Change window border back to %s for %p", extra->orig.bordername, ec);
   change_window_border(ec,
                        (extra->orig.bordername) ? extra->orig.bordername : "default");

   return extra;
}

static Client_Extra *
_get_or_create_client_extra(E_Client *ec)
{
   Client_Extra *extra;

   extra = eina_hash_find(_G.client_extras, &ec);
   if (!extra)
     {
        extra = E_NEW(Client_Extra, 1);
        *extra = (Client_Extra)
        {
           .client = ec, .expected =
           {
              .x = ec->x, .y = ec->y, .w = ec->w, .h = ec->h,
           }

           , .orig =
           {
              .geom =
              {
                 .x = ec->x, .y = ec->y, .w = ec->w, .h = ec->h,
              }

              , .maximized = ec->maximized, .bordername =
                eina_stringshare_add(ec->bordername),
           }

           ,
        };
        eina_hash_direct_add(_G.client_extras, &extra->client, extra);
     }
   else
     {
        extra->expected = (geom_t)
        {
           .x = ec->x, .y = ec->y, .w = ec->w, .h = ec->h,
        };
        extra->orig.geom = extra->expected;
        extra->orig.maximized = ec->maximized;
        eina_stringshare_replace(&extra->orig.bordername, ec->bordername);
     }

   return extra;
}

void
tiling_e_client_move_resize_extra(E_Client *ec, int x, int y, int w, int h)
{
   Client_Extra *extra = eina_hash_find(_G.client_extras, &ec);

   if (!extra)
     {
        ERR("No extra for %p", ec);
        return;
     }

   extra->expected = (geom_t)
   {
      .x = x, .y = y, .w = w, .h = h,
   };

   _e_client_move_resize(ec, x, y, w, h);
}

static Client_Extra *
tiling_entry_no_desk_func(E_Client *ec)
{
   if (!ec)
     return NULL;

   Client_Extra *extra = eina_hash_find(_G.client_extras, &ec);

   if (!extra)
     ERR("No extra for %p", ec);

   return extra;
}

static Client_Extra *
tiling_entry_func(E_Client *ec)
{
   Client_Extra *extra;

   if (!is_tilable(ec))
     return NULL;

   extra = tiling_entry_no_desk_func(ec);

   if (!extra)
      return NULL;

   if (!desk_should_tile_check(ec->desk))
     return NULL;

   return extra;
}

/* }}} */
/* Reorganize Stacks {{{ */

static void
_reapply_tree(void)
{
   int zx, zy, zw, zh;

   if (_G.tinfo->tree)
     {
        e_zone_desk_useful_geometry_get(_G.tinfo->desk->zone, _G.tinfo->desk, &zx, &zy, &zw, &zh);

        if (zw > 0 && zh > 0)
          tiling_window_tree_apply(_G.tinfo->tree, zx, zy, zw, zh,
                                   tiling_g.config->window_padding,
                                   EINA_TRUE);
        else
          ERR("The zone desk geometry was not useful at all (%d,%d,%d,%d)", zx, zy, zw, zh);
     }
}

void
_restore_free_client(void *_item)
{
   Window_Tree *item = _item;

   if (item->client)
     {
        _restore_client(item->client);

        Client_Extra *extra = eina_hash_find(_G.client_extras, &item->client);

        if (extra)
          {
             extra->tiled = EINA_FALSE;
          }
     }
   free(item);
}

void
change_desk_conf(struct _Config_vdesk *newconf)
{
   E_Zone *z;
   E_Desk *d;
   int old_nb_stacks, new_nb_stacks = newconf->nb_stacks;

   z = e_comp_zone_number_get(newconf->zone_num);
   if (!z)
     return;
   d = e_desk_at_xy_get(z, newconf->x, newconf->y);
   if (!d)
     return;

   check_tinfo(d);
   old_nb_stacks = _G.tinfo->conf->nb_stacks;

   _G.tinfo->conf = newconf;
   _G.tinfo->conf->nb_stacks = new_nb_stacks;

   _desk_config_apply(d, old_nb_stacks, new_nb_stacks);
}

static void
_desk_config_apply(E_Desk *d, int old_nb_stacks, int new_nb_stacks)
{
   check_tinfo(d);

   if (new_nb_stacks == 0)
     {
        tiling_window_tree_walk(_G.tinfo->tree, _restore_free_client);
        _G.tinfo->tree = NULL;
     }
   else if (new_nb_stacks == old_nb_stacks)
     {
        E_Client *ec;

        E_CLIENT_FOREACH(ec)
          {
             _client_apply_settings(ec, NULL);
          }

        _reapply_tree();
     }
   else
     {
        /* Add all the existing windows. */
        E_Client *ec;

        E_CLIENT_FOREACH(ec)
          {
             _add_client(ec, _current_tiled_state(EINA_TRUE));
          }

        _reapply_tree();
     }
}

/* }}} */
/* Reorganize windows {{{ */

static void
_client_apply_settings(E_Client *ec, Client_Extra *extra)
{
   const char *bdname = "pixel";

   if (!extra) extra = tiling_entry_func(ec);

   if ((!extra) || (!extra->tiled)) return;

   if (ec->maximized) _e_client_unmaximize(ec, E_MAXIMIZE_BOTH);

   if (e_theme_border_find("tiling")) bdname = "tiling";

   if ((!tiling_g.config->show_titles) &&
       (!ec->bordername || strcmp(ec->bordername, bdname)))
     change_window_border(ec, bdname);
   else if ((tiling_g.config->show_titles) &&
            (ec->bordername && !strcmp(ec->bordername, bdname)))
     change_window_border(ec, (extra->orig.bordername) ?
                          extra->orig.bordername : "default");
}

static void
_e_client_check_based_on_state_cb(void *data, Evas_Object *obj EINA_UNUSED,
      void *event_info EINA_UNUSED)
{
   E_Client *ec = data;
   _toggle_tiling_based_on_state(ec, EINA_TRUE);
}

/**
 * Find the next tiled client under the current coordinates
 */
static Window_Tree*
_tilable_client(int x, int y)
{
   E_Client *ec;

   E_CLIENT_FOREACH(ec)
     {
        Eina_Rectangle c;
        Window_Tree *wt;

        e_client_geometry_get(ec, &c.x, &c.y, &c.w, &c.h);

        if (!eina_rectangle_coords_inside(&c, x, y)) continue;

        if (!(wt = tiling_window_tree_client_find(_G.tinfo->tree, ec))) continue;

        return wt;
     }
  return NULL;
}

static Position_On_Client
_calculate_position_preference(E_Client *ec)
{
   int x,y;
   float bounded_x, bounded_y;
   Eina_Rectangle rect;
   evas_pointer_canvas_xy_get(e_comp->evas, &x, &y);

   e_client_geometry_get(ec, &rect.x, &rect.y, &rect.w, &rect.h);

   if (!eina_rectangle_coords_inside(&rect, x, y))
     {
        ERR("Coorinates are not in there");
        return -1;
     }

   //for the calculation we think of a X cross in the rectangle
   bounded_x = ((float)x - rect.x)/((float)rect.w);
   bounded_y = ((float)y - rect.y)/((float)rect.h);

   if (bounded_y < bounded_x)
     {
        //right upper part
        if (bounded_y < (1.0 - bounded_x))
          {
             //left upper
             return POSITION_TOP;
          }
        else
          {
             //right lower
             return POSITION_RIGHT;
          }
     }
   else
     {
        //lower left part
        if (bounded_y < (1.0 - bounded_x))
          {
             //left upper
             return POSITION_LEFT;
          }
        else
          {
             //right lower
             return POSITION_BOTTOM;
          }
     }


}

static void
_insert_client_preferred(E_Client *ec)
{
   Window_Tree *parent;
   Tiling_Split_Type type = TILING_SPLIT_VERTICAL;
   Eina_Bool before;
   int x,y;

   evas_pointer_canvas_xy_get(e_comp->evas, &x, &y);
   parent = _tilable_client(x,y);

   if (parent)
     {
        //calculate a good position where we would like to stay
        Position_On_Client c;

        c = _calculate_position_preference(parent->client);
        if (c == POSITION_TOP || c == POSITION_BOTTOM)
          {
             before = (c == POSITION_TOP);
             type = TILING_SPLIT_VERTICAL;
          }
        else
          {
             before = (c == POSITION_LEFT);
             type = TILING_SPLIT_HORIZONTAL;
          }

        _G.tinfo->tree = tiling_window_tree_insert(_G.tinfo->tree, parent, ec, type, before);
     }
   else
     {
        _G.tinfo->tree = tiling_window_tree_insert(_G.tinfo->tree, NULL, ec, _current_tiled_state(EINA_FALSE), EINA_FALSE);
     }
}

static void
_insert_client(E_Client *ec, Tiling_Split_Type type)
{
   E_Client *ec_focused = e_client_focused_get();
   Window_Tree *place = NULL;

   if (ec_focused == ec)
     {
        _insert_client_preferred(ec);
     }
   else
     {
        //otherwise place next to the given client
        place = tiling_window_tree_client_find(_G.tinfo->tree,
                                               ec_focused);
        _G.tinfo->tree = tiling_window_tree_insert(_G.tinfo->tree, place, ec, type, EINA_FALSE);

     }

}

static Eina_Bool
_add_client(E_Client *ec, Tiling_Split_Type type)
{
   /* Should I need to check that the client is not already added? */
   if (!ec)
     {
        return EINA_FALSE;
     }

   Client_Extra *extra = _get_or_create_client_extra(ec);
   _client_track(ec);

   if (!is_tilable(ec))
     {
        return EINA_FALSE;
     }

   if (!desk_should_tile_check(ec->desk))
      return EINA_FALSE;

   if (is_ignored_window(extra))
      return EINA_FALSE;

   if (type == TILING_SPLIT_FLOAT)
     {
        extra->floating = EINA_TRUE;
        return EINA_FALSE;
     }

   if (extra->tiled)
      return EINA_FALSE;

   extra->tiled = EINA_TRUE;

   DBG("adding %p", ec);

   _client_apply_settings(ec, extra);

   /* Window tree updating. */
   _insert_client(ec, type);

   if (started)
     _reapply_tree();

   return EINA_TRUE;
}

static Eina_Bool
_client_remove_no_apply(E_Client *ec)
{
   if (!ec)
      return EINA_FALSE;

   DBG("removing %p", ec);

   Client_Extra *extra = eina_hash_find(_G.client_extras, &ec);

   if (!extra)
     {
        if (is_tilable(ec))
          {
             ERR("No extra for %p", ec);
          }
        return EINA_FALSE;
     }

   if (extra->drag.drag)
     {
        _client_drag_terminate(ec);
     }

   if (!extra->tiled)
      return EINA_FALSE;

   extra->tiled = EINA_FALSE;

   /* Window tree updating. */
     {
        /* If focused is NULL, it should return the root. */
        Window_Tree *item = tiling_window_tree_client_find(_G.tinfo->tree, ec);

        if (!item)
          {
             ERR("Couldn't find tree item for client %p!", ec);
             return EINA_FALSE;
          }

        _G.tinfo->tree = tiling_window_tree_remove(_G.tinfo->tree, item);
     }

   return EINA_TRUE;
}

static void
_remove_client(E_Client *ec)
{
   if (_client_remove_no_apply(ec))
     _reapply_tree();
}

/* }}} */
/* Toggle Floating {{{ */

static void
toggle_floating(E_Client *ec)
{
   Client_Extra *extra = tiling_entry_no_desk_func(ec);

   if (!extra)
     {
        return;
     }

   extra->floating = !extra->floating;

   if (!desk_should_tile_check(ec->desk))
     return;

   /* This is the new state, act accordingly. */
   if (extra->floating)
     {
        _restore_client(ec);
        _remove_client(ec);
     }
   else
     {
        _add_client(ec, _current_tiled_state(EINA_FALSE));
     }
}

static void
_window_tree_apply_delayed(void *data EINA_UNUSED)
{
   _reapply_tree();
   ecore_job_del(_G.apply_tree_job);
   _G.apply_tree_job = NULL;
}

void
tiling_e_client_does_not_fit(E_Client *ec)
{
   E_Notification_Notify n;
   Eina_Strbuf *buf;
   Client_Extra *extra = tiling_entry_no_desk_func(ec);

   EINA_SAFETY_ON_NULL_RETURN(extra);

   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, _("Window %s cannot be tiled\n"),
                             e_client_util_name_get(ec));

   memset(&n, 0, sizeof(n));
   n.app_name = _("Tiling");
   n.icon.icon = "dialog-error";
   n.summary = _("Window cannot be tiled");
   n.body = eina_strbuf_string_get(buf);
   n.timeout = 8000;
   e_notification_client_send(&n, NULL, NULL);
   eina_strbuf_string_free(buf);

   EINA_SAFETY_ON_TRUE_RETURN(extra->floating);

   //remove the client here without applying the tree again to break maybe possible recursions
   {
      extra->floating = EINA_TRUE;
      _restore_client(ec);
      _client_remove_no_apply(ec);
      _G.apply_tree_job = ecore_job_add(_window_tree_apply_delayed, NULL);
   }
}

static void
_e_mod_action_toggle_floating_cb(E_Object *obj EINA_UNUSED,
                                 const char *params EINA_UNUSED)
{
   toggle_floating(e_client_focused_get());
}

static E_Client *_go_mouse_client = NULL;

static Eina_Bool
_e_mod_action_swap_window_go_mouse(E_Object *obj EINA_UNUSED,
                                   const char *params EINA_UNUSED,
                                   E_Binding_Event_Mouse_Button *ev EINA_UNUSED)
{
   E_Client *ec = e_client_under_pointer_get(get_current_desk(), NULL);

   Client_Extra *extra = tiling_entry_func(ec);

   if (!extra || !extra->tiled)
     return EINA_FALSE;

   _go_mouse_client = ec;
   return EINA_TRUE;
}

static Eina_Bool
_e_mod_action_swap_window_end_mouse(E_Object *obj EINA_UNUSED,
                                    const char *params EINA_UNUSED,
                                    E_Binding_Event_Mouse_Button *ev EINA_UNUSED)
{
   E_Client *ec = e_client_under_pointer_get(get_current_desk(), NULL);
   E_Client *first_ec = _go_mouse_client;

   _go_mouse_client = NULL;

   if (!first_ec)
     return EINA_FALSE;

   Client_Extra *extra = tiling_entry_func(ec);

   if (!extra || !extra->tiled)
     return EINA_FALSE;

   /* XXX: Only support swap on the first desk for now. */
   if (ec->desk != first_ec->desk)
     return EINA_FALSE;

   Window_Tree *item, *first_item;

   item = tiling_window_tree_client_find(_G.tinfo->tree, ec);

   if (!item)
     return EINA_FALSE;

   first_item = tiling_window_tree_client_find(_G.tinfo->tree, first_ec);

   if (!first_item)
     return EINA_FALSE;

   item->client = first_ec;
   first_item->client = ec;

   _reapply_tree();
   return EINA_TRUE;
}

static void
_e_mod_menu_border_cb(void *data, E_Menu *m EINA_UNUSED,
                      E_Menu_Item *mi EINA_UNUSED)
{
   E_Client *ec = data;

   toggle_floating(ec);
}

/* }}} */
/* {{{ Move windows */

static void
_action_move(int cross_edge)
{
   E_Desk *desk;
   E_Client *focused_ec;

   desk = get_current_desk();
   if (!desk)
     return;

   focused_ec = e_client_focused_get();
   if (!focused_ec || focused_ec->desk != desk)
     return;

   if (!desk_should_tile_check(desk))
     return;

   Window_Tree *item =
     tiling_window_tree_client_find(_G.tinfo->tree, focused_ec);

   if (item)
     {
        tiling_window_tree_node_change_pos(item, cross_edge);

        _reapply_tree();
     }
}

static void
_e_mod_action_move_left_cb(E_Object *obj EINA_UNUSED,
                           const char *params EINA_UNUSED)
{
   _action_move(TILING_WINDOW_TREE_EDGE_LEFT);
}

static void
_e_mod_action_move_right_cb(E_Object *obj EINA_UNUSED,
                            const char *params EINA_UNUSED)
{
   _action_move(TILING_WINDOW_TREE_EDGE_RIGHT);
}

static void
_e_mod_action_move_up_cb(E_Object *obj EINA_UNUSED,
                         const char *params EINA_UNUSED)
{
   _action_move(TILING_WINDOW_TREE_EDGE_TOP);
}

static void
_e_mod_action_move_down_cb(E_Object *obj EINA_UNUSED,
                           const char *params EINA_UNUSED)
{
   _action_move(TILING_WINDOW_TREE_EDGE_BOTTOM);
}

/* }}} */
/* Toggle split mode {{{ */

static Eina_Bool
_split_type_popup_timer_del_cb(void *data EINA_UNUSED)
{
   evas_object_hide(_G.split_popup.comp_obj);
   evas_object_del(_G.split_popup.comp_obj);
   _G.split_popup.comp_obj = NULL;
   _G.split_popup.obj = NULL;
   _G.split_popup.timer = NULL;
   _G.split_popup.desk = NULL;

   return EINA_FALSE;
}

static void
_tiling_split_type_changed_popup(void)
{
   Evas_Object *comp_obj = _G.split_popup.comp_obj;
   Evas_Object *o = _G.split_popup.obj;
   E_Desk *desk = NULL;

   /* If this is not NULL, the rest isn't either. */

   /* check for the current desk we have */
   if (e_client_focused_get())
     {
        E_Client *c;

        c = e_client_focused_get();
        desk = c->desk;
     }

   if (!o)
     {
        _G.split_popup.obj = o = edje_object_add(e_comp->evas);
        if (!e_theme_edje_object_set(o, "base/theme/modules/tiling",
                 "e/modules/tiling/main"))
           edje_object_file_set(o, _G.edj_path, "modules/tiling/main");
        evas_object_resize(o, TILING_POPUP_SIZE, TILING_POPUP_SIZE);

        _G.split_popup.comp_obj = comp_obj = e_comp_object_util_add(o, E_COMP_OBJECT_TYPE_POPUP);

        if (desk)
          e_comp_object_util_center_on_zone(comp_obj, e_zone_current_get());
        else
          e_comp_object_util_center(comp_obj);
        _G.split_popup.desk = desk;
        evas_object_layer_set(comp_obj, E_LAYER_POPUP);
        evas_object_pass_events_set(comp_obj, EINA_TRUE);

        evas_object_show(comp_obj);

        _G.split_popup.timer = ecore_timer_loop_add(TILING_POPUP_TIMEOUT, _split_type_popup_timer_del_cb, NULL);
     }
   else
     {
        if (desk != _G.split_popup.desk)
          e_comp_object_util_center_on_zone(comp_obj, e_zone_current_get());
        ecore_timer_loop_reset(_G.split_popup.timer);
     }


   _edje_tiling_icon_set(o);
}

static void
_tiling_gadgets_update(void)
{
   Instance *inst;
   Eina_List *itr;

   EINA_LIST_FOREACH(tiling_g.gadget_instances, itr, inst)
     {
        _gadget_icon_set(inst);
     }
}

static void
_tiling_split_type_next(void)
{
   //update the current desk in case something has changed it
   _update_current_desk(get_current_desk());

   if (!_G.current_split_type)
     {
        ERR("Invalid state, current split type is NULL");
        return;
     }

   _G.current_split_type->type = (_G.current_split_type->type + 1) % TILING_SPLIT_LAST;

   /* If we don't allow floating, skip it. */
   if (!tiling_g.config->have_floating_mode &&
       (_G.current_split_type->type == TILING_SPLIT_FLOAT))
     {
        _G.current_split_type->type = (_G.current_split_type->type + 1) % TILING_SPLIT_LAST;
     }

   _tiling_gadgets_update();
   _tiling_split_type_changed_popup();
}

static void
_e_mod_action_toggle_split_mode(E_Object *obj EINA_UNUSED,
                                const char *params EINA_UNUSED)
{
   _tiling_split_type_next();
}

/* }}} */
/* Hooks {{{ */

static void
_move_or_resize(E_Client *ec)
{
   Client_Extra *extra = tiling_entry_func(ec);

   if (!extra || !extra->tiled)
     {
        return;
     }

   if ((ec->x == extra->expected.x) && (ec->y == extra->expected.y) &&
       (ec->w == extra->expected.w) && (ec->h == extra->expected.h))
     {
        return;
     }
/* This seems to create more issues than it seemingly solves? it stops
 * resizes when no borders are there....
   if (!extra->last_frame_adjustment)
     {
        printf
          ("This is probably because of the frame adjustment bug. Return\n");
        _reapply_tree();
        return;
     }
 */
   Window_Tree *item = tiling_window_tree_client_find(_G.tinfo->tree, ec);

   if (!item)
     {
        ERR("Couldn't find tree item for resized client %p!", ec);
        return;
     }

   {
      int w_dir = 1, h_dir = 1;
      double w_diff = 1.0, h_diff = 1.0;

      if (abs(extra->expected.w - ec->w) >= 1)
        {
           w_diff = ((double)ec->w) / extra->expected.w;
        }
      if (abs(extra->expected.h - ec->h) >= 1)
        {
           h_diff = ((double)ec->h) / extra->expected.h;
        }
      switch (ec->resize_mode)
        {
         case E_POINTER_RESIZE_L:
         case E_POINTER_RESIZE_BL:
           w_dir = -1;
           break;

         case E_POINTER_RESIZE_T:
         case E_POINTER_RESIZE_TR:
           h_dir = -1;
           break;

         case E_POINTER_RESIZE_TL:
           w_dir = -1;
           h_dir = -1;
           break;

         default:
           break;
        }
      if ((!eina_dbl_exact(w_diff, 1.0)) || (!eina_dbl_exact(h_diff, 1.0)))
        {
           if (!tiling_window_tree_node_resize(item, w_dir, w_diff, h_dir,
                                               h_diff))
             {
                /* FIXME: Do something? */
             }
        }
   }

   _reapply_tree();
}

static void
_resize_begin_hook(void *data EINA_UNUSED, E_Client *ec)
{
   Client_Extra *extra = tiling_entry_func(ec);

   if (!extra || !extra->tiled)
     {
        return;
     }

   Window_Tree *item = tiling_window_tree_client_find(_G.tinfo->tree, ec);

   if (!item)
     {
        ERR("Couldn't find tree item for resized client %p!", ec);
        return;
     }

   int edges = tiling_window_tree_edges_get(item);

   if (edges & TILING_WINDOW_TREE_EDGE_LEFT)
     {
        switch (ec->resize_mode)
          {
           case E_POINTER_RESIZE_L:
             ec->resize_mode = E_POINTER_RESIZE_NONE;
             break;

           case E_POINTER_RESIZE_TL:
             ec->resize_mode = E_POINTER_RESIZE_T;
             break;

           case E_POINTER_RESIZE_BL:
             ec->resize_mode = E_POINTER_RESIZE_B;
             break;

           default:
             break;
          }
     }
   if (edges & TILING_WINDOW_TREE_EDGE_RIGHT)
     {
        switch (ec->resize_mode)
          {
           case E_POINTER_RESIZE_R:
             ec->resize_mode = E_POINTER_RESIZE_NONE;
             break;

           case E_POINTER_RESIZE_TR:
             ec->resize_mode = E_POINTER_RESIZE_T;
             break;

           case E_POINTER_RESIZE_BR:
             ec->resize_mode = E_POINTER_RESIZE_B;
             break;

           default:
             break;
          }
     }
   if (edges & TILING_WINDOW_TREE_EDGE_TOP)
     {
        switch (ec->resize_mode)
          {
           case E_POINTER_RESIZE_T:
             ec->resize_mode = E_POINTER_RESIZE_NONE;
             break;

           case E_POINTER_RESIZE_TL:
             ec->resize_mode = E_POINTER_RESIZE_L;
             break;

           case E_POINTER_RESIZE_TR:
             ec->resize_mode = E_POINTER_RESIZE_R;
             break;

           default:
             break;
          }
     }
   if (edges & TILING_WINDOW_TREE_EDGE_BOTTOM)
     {
        switch (ec->resize_mode)
          {
           case E_POINTER_RESIZE_B:
             ec->resize_mode = E_POINTER_RESIZE_NONE;
             break;

           case E_POINTER_RESIZE_BL:
             ec->resize_mode = E_POINTER_RESIZE_L;
             break;

           case E_POINTER_RESIZE_BR:
             ec->resize_mode = E_POINTER_RESIZE_R;
             break;

           default:
             break;
          }
     }

   if (!e_client_util_resizing_get(ec))
     e_client_resize_cancel();
}

static Eina_Bool
_resize_hook(void *data EINA_UNUSED, int type EINA_UNUSED,
             E_Event_Client *event)
{
   E_Client *ec = event->ec;

   _move_or_resize(ec);

   return true;
}

static Eina_Bool
_move_hook(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Client *event)
{
   E_Client *ec = event->ec;
   Client_Extra *extra = tiling_entry_func(ec);

   if (!extra || !extra->tiled)
     {
        return true;
     }

   /* A hack because e doesn't trigger events for all property changes */
   if (!is_tilable(ec))
     {
        toggle_floating(ec);

        return true;
     }

   e_client_act_move_end(event->ec, NULL);

   _reapply_tree();

   return true;
}

static void
_frame_del_cb(void *data, Evas *evas EINA_UNUSED,
      Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *ec = data;

   if (desk_should_tile_check(ec->desk))
     {
        _client_remove_no_apply(ec);
     }

   _client_untrack(ec);

   eina_hash_del(_G.client_extras, &ec, NULL);

   _reapply_tree();
}

static void
_e_client_extra_unregister_callbacks(void *_client_extra)
{
   Client_Extra *extra = _client_extra;

   _client_untrack(extra->client);
}

static void
_client_untrack(E_Client *ec)
{
   Client_Extra *extra = eina_hash_find(_G.client_extras, &ec);

   if (!extra->tracked)
      return;

   extra->tracked = EINA_FALSE;

   evas_object_event_callback_del_full(ec->frame, EVAS_CALLBACK_DEL,
         _frame_del_cb, ec);
   evas_object_smart_callback_del_full(ec->frame, "maximize_done",
         _e_client_check_based_on_state_cb, ec);
   evas_object_smart_callback_del_full(ec->frame, "frame_recalc_done",
         _e_client_check_based_on_state_cb, ec);
   evas_object_smart_callback_del_full(ec->frame, "stick",
         _e_client_check_based_on_state_cb, ec);
   evas_object_smart_callback_del_full(ec->frame, "unstick",
         _e_client_check_based_on_state_cb, ec);
}

static void
_client_track(E_Client *ec)
{
   Client_Extra *extra = eina_hash_find(_G.client_extras, &ec);

   if (extra->tracked)
      return;

   extra->tracked = EINA_TRUE;

   evas_object_event_callback_add(ec->frame, EVAS_CALLBACK_DEL,
         _frame_del_cb, ec);
   evas_object_smart_callback_add(ec->frame, "maximize_done",
         _e_client_check_based_on_state_cb, ec);
   evas_object_smart_callback_add(ec->frame, "frame_recalc_done",
         _e_client_check_based_on_state_cb, ec);
   evas_object_smart_callback_add(ec->frame, "stick",
         _e_client_check_based_on_state_cb, ec);
   evas_object_smart_callback_add(ec->frame, "unstick",
         _e_client_check_based_on_state_cb, ec);
}

static void
_add_hook(void *data EINA_UNUSED, E_Client *ec)
{
   if (!ec)
     return;

   if (!ec->new_client)
     return;

   if (e_object_is_del(E_OBJECT(ec)))
     return;

   _add_client(ec, _current_tiled_state(EINA_TRUE));
}

static Eina_Bool
_toggle_tiling_based_on_state(E_Client *ec, Eina_Bool restore)
{
   Client_Extra *extra = eina_hash_find(_G.client_extras, &ec);

   if (!extra)
     {
        return EINA_FALSE;
     }

   /* This is the new state, act accordingly. */
   if (extra->tiled && !is_tilable(ec))
     {
        if (restore)
          {
             _restore_client(ec);
          }
        if (desk_should_tile_check(ec->desk))
          {
             _remove_client(ec);
          }

        return EINA_TRUE;
     }
   else if (!extra->tiled && is_tilable(ec))
     {
        _add_client(ec, _current_tiled_state(EINA_FALSE));

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static bool
_iconify_hook(void *data EINA_UNUSED, int type EINA_UNUSED,
                E_Event_Client *event)
{
   E_Client *ec = event->ec;

   if (ec->deskshow)
     return true;

   _toggle_tiling_based_on_state(ec, EINA_TRUE);

   return true;
}

static bool
_desk_set_hook(void *data EINA_UNUSED, int type EINA_UNUSED,
               E_Event_Client_Desk_Set *ev)
{
   DBG("%p: from (%d,%d) to (%d,%d)", ev->ec, ev->desk->x, ev->desk->y,
       ev->ec->desk->x, ev->ec->desk->y);
   Client_Extra *extra = eina_hash_find(_G.client_extras, &ev->ec);

   if (!extra)
     {
        return true;
     }

   //check the state of the new desk
   if (desk_should_tile_check(ev->ec->desk))
     {
        if (extra->drag.drag)
          {
             ev->ec->hidden = EINA_TRUE;
             e_client_comp_hidden_set(ev->ec, EINA_TRUE);
             evas_object_hide(ev->ec->frame);
             return true;
          }
     }
   else
     {
        if (extra->drag.drag)
          {
             _client_drag_terminate(ev->ec);
             extra->floating = EINA_TRUE;
          }
     }

   //check if we should remove that here
   if (desk_should_tile_check(ev->desk))
     {
        if (tiling_window_tree_client_find(_G.tinfo->tree, ev->ec))
          {
             _restore_client(ev->ec);
             _remove_client(ev->ec);
          }
     }
   if (desk_should_tile_check(ev->ec->desk))
     {
        _add_client(ev->ec, _current_tiled_state(EINA_FALSE));
     }

   return true;
}

static void
_compositor_resize_hook_desk_reapply(E_Desk *desk)
{
   check_tinfo(desk);
   if (!desk_should_tile_check(desk))
     return;

   _reapply_tree();
}

static bool
_compositor_resize_hook(void *data EINA_UNUSED, int type EINA_UNUSED,
                        void *ev EINA_UNUSED)
{
   _foreach_desk(_compositor_resize_hook_desk_reapply);

   return true;
}

static void
_bd_hook(void *d EINA_UNUSED, E_Client *ec)
{
   E_Menu_Item *mi;
   E_Menu *m;
   Eina_List *l;

   if (!ec->border_menu)
     return;
   m = ec->border_menu;

   Client_Extra *extra = eina_hash_find(_G.client_extras, &ec);

   if (!extra)
     {
        return;
     }

   /* position menu item just before the last separator */
   EINA_LIST_REVERSE_FOREACH(m->items, l, mi)
     if (mi->separator)
       break;
   if ((!mi) || (!mi->separator))
     return;
   l = eina_list_prev(l);
   mi = eina_list_data_get(l);
   if (!mi)
     return;

   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_label_set(mi, _("Floating"));
   e_menu_item_check_set(mi, true);
   e_menu_item_toggle_set(mi, (extra->floating) ? true : false);
   e_menu_item_callback_set(mi, _e_mod_menu_border_cb, ec);
}

/* }}} */
/* Module setup {{{ */

static void
_clear_info_hash(void *data)
{
   Tiling_Info *ti = data;

   tiling_window_tree_free(ti->tree);
   ti->tree = NULL;
   E_FREE(ti);
}

static void
_clear_border_extras(void *data)
{
   Client_Extra *extra = data;

   eina_stringshare_del(extra->orig.bordername);

   E_FREE(extra);
}

static void
_clear_desk_types(void *data)
{
   free(data);
}

E_API E_Module_Api e_modapi = {
   E_MODULE_API_VERSION,
   "Tiling"
};

static unsigned char
_client_drag_mouse_up(void *data, int event EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *ec = data;
   Client_Extra *extra = tiling_entry_func(ec);

   if (!extra) return ECORE_CALLBACK_PASS_ON;

   if (extra->drag.drag)
     _client_drag_terminate(data);

   //remove the events
   E_FREE_FUNC(extra->drag.move, ecore_event_handler_del);
   E_FREE_FUNC(extra->drag.up, ecore_event_handler_del);

   return ECORE_CALLBACK_PASS_ON;
}

static unsigned char
_client_drag_mouse_move(void *data, int event EINA_UNUSED, void *event_info)
{
   Ecore_Event_Mouse_Move *ev = event_info;
   Window_Tree *client;
   int x,y;
   E_Client *ec = data;
   Client_Extra *extra = tiling_entry_no_desk_func(data);

   if (!extra)
     {
        return ECORE_CALLBACK_PASS_ON;
     }

   if (evas_object_visible_get(ec->frame))
     {
        /*only initiaze the drag when x and y is different */
        if (extra->drag.x == ev->x && extra->drag.y == ev->y) return ECORE_CALLBACK_PASS_ON;

        _client_remove_no_apply(ec);

        extra->drag.drag = EINA_TRUE;
        e_comp_grab_input(EINA_TRUE, EINA_FALSE);

        ec->hidden = EINA_TRUE;
        e_client_comp_hidden_set(ec, EINA_TRUE);
        evas_object_hide(ec->frame);

        _reapply_tree();
     }

   //now check if we can hint somehow
   evas_pointer_canvas_xy_get(e_comp->evas, &x, &y);


   //create hint if not there
   if (!extra->drag.hint)
     {
        extra->drag.hint = edje_object_add(e_comp->evas);
        if (!e_theme_edje_object_set(extra->drag.hint,
                                     "base/theme/modules/tiling",
                                     "e/modules/tiling/indicator"))
          edje_object_file_set(extra->drag.hint, _G.edj_path, "modules/tiling/indicator");
        evas_object_layer_set(extra->drag.hint, E_LAYER_CLIENT_DRAG);
        evas_object_show(extra->drag.hint);

        extra->drag.ic = e_client_icon_add(ec, evas_object_evas_get(e_comp->evas));
        edje_object_part_swallow(extra->drag.hint, "e.client.icon", extra->drag.ic);
        evas_object_show(extra->drag.ic);
     }

   //if there is nothing below, we cannot hint to anything
   client = _tilable_client(x, y);
   if (client)
     {
        Position_On_Client c;

        c = _calculate_position_preference(client->client);

        Eina_Rectangle pos = client->client->client;
        if (c == POSITION_LEFT)
          evas_object_geometry_set(extra->drag.hint, pos.x, pos.y, pos.w/2, pos.h);
        else if (c == POSITION_RIGHT)
          evas_object_geometry_set(extra->drag.hint, pos.x+pos.w/2, pos.y, pos.w/2, pos.h);
        else if (c == POSITION_BOTTOM)
          evas_object_geometry_set(extra->drag.hint, pos.x, pos.y + pos.h/2, pos.w, pos.h/2);
        else if (c == POSITION_TOP)
          evas_object_geometry_set(extra->drag.hint, pos.x, pos.y, pos.w, pos.h/2);
        evas_object_show(extra->drag.hint);
     }
   else
     {
        //if there is no client, just highlight the zone
        Eina_Rectangle geom;
        E_Zone *zone = e_zone_current_get();
        e_zone_useful_geometry_get(zone, &geom.x, &geom.y, &geom.w, &geom.h);
        evas_object_geometry_set(extra->drag.hint, EINA_RECTANGLE_ARGS(&geom));
        evas_object_show(extra->drag.hint);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_client_drag_terminate(E_Client *ec)
{
   Client_Extra *extra = tiling_entry_no_desk_func(ec);

   if (!extra)
     {
        return;
     }

   //we grappend the comp when we started the drag
   e_comp_ungrab_input(EINA_TRUE, EINA_FALSE);

   //insert the client at the position where the up was
   if (desk_should_tile_check(get_current_desk()))
     {
        _insert_client_preferred(ec);
        extra->tiled = EINA_TRUE;
     }

   //remove the hint object
   E_FREE_FUNC(extra->drag.hint, evas_object_del);
   E_FREE_FUNC(extra->drag.ic, evas_object_del);

   //bring up the client again
   ec->hidden = EINA_FALSE;
   e_client_comp_hidden_set(ec, EINA_FALSE);
   evas_object_show(ec->frame);

   //remove the events
   E_FREE_FUNC(extra->drag.move, ecore_event_handler_del);
   E_FREE_FUNC(extra->drag.up, ecore_event_handler_del);

   _reapply_tree();

   evas_object_focus_set(ec->frame, EINA_TRUE);

   extra->drag.drag = EINA_FALSE;
}

static void
_client_move_begin(void *data EINA_UNUSED, E_Client *ec)
{
   Client_Extra *extra = tiling_entry_func(ec);

   if (!extra || !extra->tiled)
     {
        return;
     }

   //listen for mouse moves when the move starts we are starting a drag
   evas_pointer_canvas_xy_get(e_comp->evas, &extra->drag.x, &extra->drag.y);
   extra->drag.move = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _client_drag_mouse_move, ec);
   extra->drag.up = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, _client_drag_mouse_up, ec);

}

static void
_update_current_desk(E_Desk *new)
{
   Desk_Split_Type *type;

   type = eina_hash_find(_G.desk_type, &new);

   if (!type)
     {
        type = calloc(1, sizeof(Desk_Split_Type));
        type->desk = new;
        type->type = TILING_SPLIT_HORIZONTAL;
        eina_hash_add(_G.desk_type, &new, type);
     }

   _G.current_split_type = type;
}

static bool
_desk_shown(void *data EINA_UNUSED, int types EINA_UNUSED, void *event_info)
{
   E_Event_Desk_Show *ev = event_info;

   if (!ev->desk)
     {
        ERR("The shown desk can never be NULL!");
        return ECORE_CALLBACK_PASS_ON;
     }

   _update_current_desk(ev->desk);
   _tiling_gadgets_update();

   return ECORE_CALLBACK_PASS_ON;
}

E_API void *
e_modapi_init(E_Module *m)
{
   E_Desk *desk;
   Eina_List *l;

   tiling_g.module = m;

   if (tiling_g.log_domain < 0)
     {
        tiling_g.log_domain = eina_log_domain_register("tiling", NULL);
        if (tiling_g.log_domain < 0)
          {
             EINA_LOG_CRIT("could not register log domain 'tiling'");
          }
     }

   _G.info_hash = eina_hash_pointer_new(_clear_info_hash);
   _G.client_extras = eina_hash_pointer_new(_clear_border_extras);
   _G.desk_type = eina_hash_pointer_new(_clear_desk_types);
#define HANDLER(_h, _e, _f) \
  _h = ecore_event_handler_add(E_EVENT_##_e, (void *)_f, NULL)

   _G.handler_client_resize_begin =
      e_client_hook_add(E_CLIENT_HOOK_RESIZE_BEGIN, _resize_begin_hook, NULL);
   _G.handler_move_begin =
      e_client_hook_add(E_CLIENT_HOOK_MOVE_BEGIN, _client_move_begin, NULL);

   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     _G.handler_client_add =
        e_client_hook_add(E_CLIENT_HOOK_EVAL_PRE_FRAME_ASSIGN, _add_hook, NULL);
   else
     _G.handler_client_add =
        e_client_hook_add(E_CLIENT_HOOK_UNIGNORE, _add_hook, NULL);
   HANDLER(_G.handler_client_resize, CLIENT_RESIZE, _resize_hook);
   HANDLER(_G.handler_client_move, CLIENT_MOVE, _move_hook);

   HANDLER(_G.handler_client_iconify, CLIENT_ICONIFY, _iconify_hook);
   HANDLER(_G.handler_client_uniconify, CLIENT_UNICONIFY, _iconify_hook);

   HANDLER(_G.handler_desk_set, CLIENT_DESK_SET, _desk_set_hook);
   HANDLER(_G.handler_compositor_resize, COMPOSITOR_UPDATE,
           _compositor_resize_hook);
   HANDLER(_G.handler_desk_show, DESK_SHOW, _desk_shown);
#undef HANDLER

#define ACTION_ADD(_action, _cb, _title, _value, _params, _example, _editable) \
  {                                                                            \
     const char *_name = _value;                                               \
     if ((_action = e_action_add(_name))) {                                    \
          _action->func.go = _cb;                                              \
          e_action_predef_name_set(N_("Tiling"), _title, _name,                \
                                   _params, _example, _editable);              \
       }                                                                       \
  }

   /* Module's actions */
   ACTION_ADD(_G.act_togglefloat, _e_mod_action_toggle_floating_cb,
              N_("Toggle floating"), "toggle_floating", NULL, NULL, 0);

   ACTION_ADD(_G.act_move_up, _e_mod_action_move_up_cb,
              N_("Move the focused window up"), "move_up", NULL, NULL, 0);
   ACTION_ADD(_G.act_move_down, _e_mod_action_move_down_cb,
              N_("Move the focused window down"), "move_down", NULL, NULL, 0);
   ACTION_ADD(_G.act_move_left, _e_mod_action_move_left_cb,
              N_("Move the focused window left"), "move_left", NULL, NULL, 0);
   ACTION_ADD(_G.act_move_right, _e_mod_action_move_right_cb,
              N_("Move the focused window right"), "move_right", NULL, NULL, 0);

   ACTION_ADD(_G.act_toggle_split_mode, _e_mod_action_toggle_split_mode,
              N_("Toggle split mode for new windows."), "toggle_split_mode", NULL, NULL, 0);

   ACTION_ADD(_G.act_swap_window, NULL, N_("Swap window"), "swap_window", NULL,
              NULL, 0);
   _G.act_swap_window->func.go_mouse = _e_mod_action_swap_window_go_mouse;
   _G.act_swap_window->func.end_mouse = _e_mod_action_swap_window_end_mouse;

#undef ACTION_ADD

   /* Configuration entries */
   snprintf(_G.edj_path, sizeof(_G.edj_path), "%s/e-module-tiling.edj",
            e_module_dir_get(m));
   e_configure_registry_category_add("windows", 50, _("Windows"), NULL,
                                     "preferences-system-windows");
   e_configure_registry_item_add("windows/tiling", 150, _("Tiling"), NULL,
                                 _G.edj_path, e_int_config_tiling_module);

   /* Configuration itself */
   _G.config_edd = E_CONFIG_DD_NEW("Tiling_Config", Config);
   _G.vdesk_edd = E_CONFIG_DD_NEW("Tiling_Config_VDesk", struct _Config_vdesk);

   E_CONFIG_VAL(_G.config_edd, Config, tile_dialogs, INT);
   E_CONFIG_VAL(_G.config_edd, Config, show_titles, INT);
   E_CONFIG_VAL(_G.config_edd, Config, have_floating_mode, INT);
   E_CONFIG_VAL(_G.config_edd, Config, window_padding, INT);

   E_CONFIG_LIST(_G.config_edd, Config, vdesks, _G.vdesk_edd);
   E_CONFIG_VAL(_G.vdesk_edd, struct _Config_vdesk, x, INT);
   E_CONFIG_VAL(_G.vdesk_edd, struct _Config_vdesk, y, INT);
   E_CONFIG_VAL(_G.vdesk_edd, struct _Config_vdesk, zone_num, INT);
   E_CONFIG_VAL(_G.vdesk_edd, struct _Config_vdesk, nb_stacks, INT);

   tiling_g.config = e_config_domain_load("module.tiling", _G.config_edd);
   if (!tiling_g.config)
     {
        tiling_g.config = E_NEW(Config, 1);
        tiling_g.config->tile_dialogs = 1;
        tiling_g.config->show_titles = 1;
        tiling_g.config->have_floating_mode = 1;
        tiling_g.config->window_padding = 0;
     }

   E_CONFIG_LIMIT(tiling_g.config->tile_dialogs, 0, 1);
   E_CONFIG_LIMIT(tiling_g.config->show_titles, 0, 1);
   E_CONFIG_LIMIT(tiling_g.config->have_floating_mode, 0, 1);
   E_CONFIG_LIMIT(tiling_g.config->window_padding, 0, TILING_MAX_PADDING);

   for (l = tiling_g.config->vdesks; l; l = l->next)
     {
        struct _Config_vdesk *vd;

        vd = l->data;

        E_CONFIG_LIMIT(vd->nb_stacks, 0, 1);
     }

   _G.client_menu_hook = e_int_client_menu_hook_add(_bd_hook, NULL);

   desk = get_current_desk();
   _G.tinfo = _initialize_tinfo(desk);

   _update_current_desk(get_current_desk());

   /* Add all the existing windows. */
   {
      E_Client *ec;

      E_CLIENT_FOREACH(ec)
      {
         _add_client(ec, _current_tiled_state(EINA_TRUE));
      }
   }
   started = EINA_TRUE;
   _reapply_tree();
   e_gadcon_provider_register(&_gc_class);


   return m;
}

static void
_disable_desk(E_Desk *desk)
{
   check_tinfo(desk);
   if (!_G.tinfo->conf)
     return;

   tiling_window_tree_walk(_G.tinfo->tree, _restore_free_client);
   _G.tinfo->tree = NULL;
}

static void
_disable_all_tiling(void)
{
   _foreach_desk(_disable_desk);
}

static void
_foreach_desk(void (*func)(E_Desk *desk))
{
   const Eina_List *l;
   E_Zone *zone;
   E_Desk *desk;
   int x, y;

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        for (x = 0; x < zone->desk_x_count; x++)
          {
             for (y = 0; y < zone->desk_y_count; y++)
               {
                  desk = zone->desks[x + (y * zone->desk_x_count)];

                  func(desk);
               }
          }
     }
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_gadcon_provider_unregister(&_gc_class);
   started = EINA_FALSE;
   _disable_all_tiling();

   e_int_client_menu_hook_del(_G.client_menu_hook);

   if (tiling_g.log_domain >= 0)
     {
        eina_log_domain_unregister(tiling_g.log_domain);
        tiling_g.log_domain = -1;
     }
#define SAFE_FREE(x, freefunc) \
   if (x) \
     { \
        freefunc(x); \
        x = NULL; \
     }
#define FREE_HANDLER(x)            \
   SAFE_FREE(x, ecore_event_handler_del);

   FREE_HANDLER(_G.handler_client_resize);
   FREE_HANDLER(_G.handler_client_move);

   FREE_HANDLER(_G.handler_client_iconify);
   FREE_HANDLER(_G.handler_client_uniconify);

   FREE_HANDLER(_G.handler_desk_set);

   SAFE_FREE(_G.handler_client_resize_begin, e_client_hook_del);
   SAFE_FREE(_G.handler_client_add, e_client_hook_del);
#undef FREE_HANDLER
#undef SAFE_FREE

#define ACTION_DEL(act, title, value)             \
  if (act) {                                      \
       e_action_predef_name_del("Tiling", title); \
       e_action_del(value);                       \
       act = NULL;                                \
    }
   ACTION_DEL(_G.act_togglefloat, "Toggle floating", "toggle_floating");
   ACTION_DEL(_G.act_move_up, "Move the focused window up", "move_up");
   ACTION_DEL(_G.act_move_down, "Move the focused window down", "move_down");
   ACTION_DEL(_G.act_move_left, "Move the focused window left", "move_left");
   ACTION_DEL(_G.act_move_right, "Move the focused window right", "move_right");

   ACTION_DEL(_G.act_toggle_split_mode, "Toggle split mode for new windows.",
              "toggle_split_mode");
   ACTION_DEL(_G.act_swap_window, "Swap window", "swap_window");
#undef ACTION_DEL

   e_configure_registry_item_del("windows/tiling");
   e_configure_registry_category_del("windows");

   E_FREE(tiling_g.config);
   E_CONFIG_DD_FREE(_G.config_edd);
   E_CONFIG_DD_FREE(_G.vdesk_edd);

   tiling_g.module = NULL;

   eina_hash_free(_G.info_hash);
   _G.info_hash = NULL;

   eina_hash_free_cb_set(_G.client_extras, _e_client_extra_unregister_callbacks);
   eina_hash_free(_G.client_extras);
   _G.client_extras = NULL;

   _G.tinfo = NULL;

   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.tiling", _G.config_edd, tiling_g.config);

   return true;
}

/* GADGET STUFF. */

/* Hack to properly save and free the gadget id. */
static Eina_Stringshare *_current_gad_id = NULL;

static void
_edje_tiling_icon_set(Evas_Object *o)
{
   switch (_current_tiled_state(EINA_TRUE))
     {
      case TILING_SPLIT_HORIZONTAL:
        edje_object_signal_emit(o, "tiling,mode,horizontal", "e");
        break;

      case TILING_SPLIT_VERTICAL:
        edje_object_signal_emit(o, "tiling,mode,vertical", "e");
        break;

      case TILING_SPLIT_FLOAT:
        edje_object_signal_emit(o, "tiling,mode,floating", "e");
        break;

      default:
        ERR("Unknown split type.");
     }
}

static void
_gadget_icon_set(Instance *inst)
{
   _edje_tiling_icon_set(inst->gadget);
}

static void
_tiling_cb_menu_configure(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   // FIXME here need to be some checks and return ?
   e_int_config_tiling_module(NULL, NULL);
}

static void
_gadget_mouse_down_cb(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Instance *inst = data;

   if (ev->button == 1) /* Change on left-click. */
     {
        _tiling_split_type_next();
     }
   else if (ev->button == 3)
     {
        E_Zone *zone;
        E_Menu *m;
        E_Menu_Item *mi;
        int x, y;

        zone = e_zone_current_get();

        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("Settings"));
        e_util_menu_item_theme_icon_set(mi, "configure");
        e_menu_item_callback_set(mi, _tiling_cb_menu_configure, NULL);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse(m, zone, x + ev->output.x, y + ev->output.y,
                              1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(e, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;

   inst = E_NEW(Instance, 1);

   o = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(o, "base/theme/modules/tiling",
                                "e/modules/tiling/main"))
     edje_object_file_set(o, _G.edj_path, "modules/tiling/main");
   evas_object_show(o);

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->gad_id = _current_gad_id;
   _current_gad_id = NULL;

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
                                  _gadget_mouse_down_cb, inst);

   inst->gadget = o;

   _gadget_icon_set(inst);

   tiling_g.gadget_instances = eina_list_append(tiling_g.gadget_instances, inst);

   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;
   Evas_Object *o;

   if (!(inst = gcc->data)) return;

   o = inst->gadget;

   evas_object_event_callback_del_full(o, EVAS_CALLBACK_MOUSE_DOWN,
                                       _gadget_mouse_down_cb, inst);

   if (inst->gadget)
     evas_object_del(inst->gadget);

   tiling_g.gadget_instances = eina_list_remove(tiling_g.gadget_instances, inst);

   eina_stringshare_del(inst->gad_id);

   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Tiling");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;

   o = edje_object_add(evas);
   edje_object_file_set(o, _G.edj_path, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   char buf[1024];

   snprintf(buf, sizeof(buf), "%s %d", _("Tiling"), tiling_g.gadget_number);

   tiling_g.gadget_number++;

   return _current_gad_id = eina_stringshare_add(buf);
}

/* }}} */
