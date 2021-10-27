#define E_COMP_WL
#include "e.h"
#include "e_mod_main.h"

#include "xdg-shell-server-protocol.h"

typedef enum
{
   STATE_MAXIMIZED = (1 << 0),
   STATE_UNMAXIMIZED = (1 << 1),
   STATE_FULLSCREEN = (1 << 2),
   STATE_UNFULLSCREEN = (1 << 3),
} State;

typedef struct Pending_State
{
   State state;
   uint32_t serial;
} Pending_State;

typedef struct vF_Shell_Data
{
   Eina_List *surfaces;
   Eina_List *positioners;
} vF_Shell_Data;

typedef struct Positioner
{
   vF_Shell_Data *v;
   struct wl_resource *res;
   Evas_Coord_Size size;
   Eina_Rectangle anchor_rect;
   enum xdg_positioner_anchor anchor;
   enum xdg_positioner_gravity gravity;
   enum xdg_positioner_constraint_adjustment constrain;
   Evas_Coord_Point offset;
} Positioner;


static void
_e_xdg_shell_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   vF_Shell_Data *v = wl_resource_get_user_data(resource);
   if (v->surfaces)
     wl_resource_post_error(resource, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "shell destroyed before surfaces");
   wl_resource_destroy(resource);
}

static void
_validate_size(struct wl_resource *resource, int32_t value)
{
   if (value <= 0)
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid size passed");
}

static void
_validate_size_negative(struct wl_resource *resource, int32_t value)
{
   if (value < 0)
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid size passed");
}

static void
_e_xdg_positioner_set_size(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, int32_t w, int32_t h)
{
   Positioner *p = wl_resource_get_user_data(resource);

   _validate_size(resource, w);
   _validate_size(resource, h);

   p->size.w = w;
   p->size.h = h;
}

static void
_e_xdg_positioner_set_anchor_rect(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   Positioner *p = wl_resource_get_user_data(resource);

   _validate_size_negative(resource, w);
   _validate_size_negative(resource, h);

   EINA_RECTANGLE_SET(&p->anchor_rect, x, y, w, h);
}

static void
_e_xdg_positioner_set_anchor(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, enum xdg_positioner_anchor anchor)
{
   Positioner *p = wl_resource_get_user_data(resource);

   p->anchor = anchor;
}

static void
_e_xdg_positioner_set_gravity(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, enum xdg_positioner_gravity gravity)
{
   Positioner *p = wl_resource_get_user_data(resource);

   if ((gravity & (XDG_POSITIONER_GRAVITY_TOP | XDG_POSITIONER_GRAVITY_BOTTOM)) ==
       (XDG_POSITIONER_GRAVITY_TOP | XDG_POSITIONER_GRAVITY_BOTTOM))
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid gravity values passed");
   else if ((gravity & (XDG_POSITIONER_GRAVITY_LEFT | XDG_POSITIONER_GRAVITY_RIGHT)) ==
       (XDG_POSITIONER_GRAVITY_LEFT | XDG_POSITIONER_GRAVITY_RIGHT))
     wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "Invalid gravity values passed");
   else
     p->gravity = gravity;
}

static void
_e_xdg_positioner_set_constraint_adjustment(struct wl_client *wl_client EINA_UNUSED, struct wl_resource *resource, enum xdg_positioner_constraint_adjustment constraint_adjustment)
{
   Positioner *p = wl_resource_get_user_data(resource);

   p->constrain = constraint_adjustment;
}

static void
_e_xdg_positioner_set_offset(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y)
{
   Positioner *p = wl_resource_get_user_data(resource);

   p->offset.x = x;
   p->offset.y = y;
}

static void
_e_xdg_positioner_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static const struct xdg_positioner_interface _e_xdg_positioner_interface =
{
   .destroy = _e_xdg_positioner_destroy,
   .set_size = _e_xdg_positioner_set_size,
   .set_anchor_rect = _e_xdg_positioner_set_anchor_rect,
   .set_anchor = _e_xdg_positioner_set_anchor,
   .set_gravity = _e_xdg_positioner_set_gravity,
   .set_constraint_adjustment = _e_xdg_positioner_set_constraint_adjustment,
   .set_offset = _e_xdg_positioner_set_offset,
};

static void
_e_xdg_shell_positioner_destroy(struct wl_resource *resource)
{
   Positioner *p;

   p = wl_resource_get_user_data(resource);
   if (!p) return;
   if (p->v) p->v->positioners = eina_list_remove(p->v->positioners, p);
   free(p);
}

static void
_e_xdg_shell_cb_positioner_create(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   struct wl_resource *res;
   vF_Shell_Data *v;
   Positioner *p;

   v = wl_resource_get_user_data(resource);
   res = wl_resource_create(client, &xdg_positioner_interface, 1, id);
   p = E_NEW(Positioner, 1);
   p->v = v;
   p->res = res;
   p->anchor_rect.w = p->anchor_rect.h = -1;
   v->positioners = eina_list_append(v->positioners, p);
   wl_resource_set_implementation(res, &_e_xdg_positioner_interface, p, _e_xdg_shell_positioner_destroy);
   wl_resource_set_user_data(res, p);
}

static void
_e_xdg_shell_surface_configure(struct wl_resource *resource, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   E_Client *ec;

   /* DBG("XDG_SHELL: Surface Configure: %d \t%d %d %d %d",  */
   /*     wl_resource_get_id(resource), x, y, w, h); */

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (ec->placed)
     e_client_util_move_resize_without_frame(ec, x, y, w, h);
   else
     e_client_util_resize_without_frame(ec, w, h);
}

static void
_e_xdg_shell_surface_ping(struct wl_resource *resource)
{
   E_Client *ec;
   uint32_t serial;
   struct wl_client *client;
   struct wl_resource *xdg_shell;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   client = wl_resource_get_client(resource);
   xdg_shell = eina_hash_find(xdg_shell_resources, &client);

   if (!xdg_shell) return;
   serial = wl_display_next_serial(e_comp_wl->wl.disp);
   xdg_wm_base_send_ping(xdg_shell, serial);
}

static void
_e_xdg_shell_surface_map(struct wl_resource *resource)
{
   E_Client *ec;

   /* DBG("XDG_SHELL: Map Surface: %d", wl_resource_get_id(resource)); */

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   if ((!ec->comp_data->mapped) && (e_pixmap_usable_get(ec->pixmap)))
     {
        /* map this surface if needed */
        ec->visible = EINA_TRUE;
        ec->comp_data->mapped = EINA_TRUE;
        evas_object_show(ec->frame);
        if (!e_client_util_desk_visible(ec, e_desk_current_get(ec->zone)))
          evas_object_hide(ec->frame);

        /* FIXME: sometimes popup surfaces Do Not raise above their
         * respective parents... */
        /* if (e_client_util_is_popup(ec)) */
        /*   e_client_raise_latest_set(ec); */
     }
}

static void
_e_xdg_shell_surface_unmap(struct wl_resource *resource)
{
   E_Client *ec;

   /* DBG("XDG_SHELL: Unmap Surface: %d", wl_resource_get_id(resource)); */

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (!ec->comp_data->mapped) return;
   ec->visible = EINA_FALSE;
   evas_object_hide(ec->frame);
   ec->comp_data->mapped = EINA_FALSE;
   while (ec->transients)
     {
        E_Client *cec = eina_list_data_get(ec->transients);
        e_shell_surface_parent_set(cec, ec->parent ? ec->parent->comp_data->surface : NULL);
     }
   ec->comp_data->need_xdg_configure = 1;
}

static void
_e_xdg_surface_state_add(struct wl_resource *resource, struct wl_array *states, uint32_t state)
{
  uint32_t *s;

   s = wl_array_add(states, sizeof(*s));
  if (s)
    *s = state;
  else
    wl_resource_post_no_memory(resource);
}

static void
_xdg_shell_surface_send_configure(struct wl_resource *resource, Eina_Bool fullscreen, Eina_Bool maximized, uint32_t edges, int32_t width, int32_t height)
{
   struct wl_array states;
   uint32_t serial;
   E_Client *focused, *ec;
   E_Shell_Data *shd;
   State pending = 0;
   Eina_Bool activated = EINA_FALSE;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_client_util_is_popup(ec)) return;
   focused = e_client_focused_get();
   if (ec == focused)
     activated = 1;
   else if (focused && focused->parent)
     {
        do
          {
             if (focused->parent != ec)
               focused = focused->parent;
             else
               activated = 1;
          } while (focused && (!activated));
     }
   shd = ec->comp_data->shell.data;
   if ((shd->edges == edges) && (shd->width == width) && (shd->height == height) &&
       (shd->fullscreen == fullscreen) &&
       (shd->maximized == maximized) &&
       (shd->activated == activated)) return;
   if (shd->edges && (shd->edges != edges))
     {
        if (shd->pending && (!width) && (!height))
          width = shd->width, height = shd->height;
     }
   //fprintf(stderr, "CONFIGURE %dx%d | max(%d) | full(%d)\n", width, height, maximized, fullscreen);
   shd->edges = edges;
   shd->width = width;
   shd->height = height;
   if (shd->fullscreen != fullscreen)
     {
        if (fullscreen)
          pending |= STATE_FULLSCREEN;
        else if (ec->fullscreen)
          pending |= STATE_UNFULLSCREEN;
     }
   shd->fullscreen = fullscreen;
   if (shd->maximized != maximized)
     {
        if (maximized)
          pending |= STATE_MAXIMIZED;
        else if (ec->maximized || ec->comp_data->unmax)
          pending |= STATE_UNMAXIMIZED;
     }
   shd->maximized = maximized;
   shd->activated = activated;
   wl_array_init(&states);

   if (fullscreen)
     _e_xdg_surface_state_add(resource, &states, XDG_TOPLEVEL_STATE_FULLSCREEN);
   else if (maximized)
     _e_xdg_surface_state_add(resource, &states, XDG_TOPLEVEL_STATE_MAXIMIZED);
   if (edges)
     _e_xdg_surface_state_add(resource, &states, XDG_TOPLEVEL_STATE_RESIZING);
   if (activated)
     _e_xdg_surface_state_add(resource, &states, XDG_TOPLEVEL_STATE_ACTIVATED);

   serial = wl_display_next_serial(e_comp_wl->wl.disp);
   xdg_toplevel_send_configure(resource, width, height, &states);
   {
      Pending_State *ps;

      ps = E_NEW(Pending_State, 1);
      ps->state = pending;
      ps->serial = serial;
      shd->pending = eina_list_append(shd->pending, ps);
   }
   xdg_surface_send_configure(shd->surface, serial);

   wl_array_release(&states);
   ec->comp_data->need_xdg_configure = 0;
}

static void
_e_xdg_shell_surface_configure_send(struct wl_resource *resource, uint32_t edges, int32_t width, int32_t height)
{
   E_Client *ec;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_client_util_is_popup(ec))
     {
        E_Shell_Data *shd;
        uint32_t serial;

        shd = ec->comp_data->shell.data;
        serial = wl_display_next_serial(e_comp_wl->wl.disp);
        xdg_popup_send_configure(resource, ec->x - ec->parent->x, ec->y - ec->parent->y, width ?: ec->w, height ?: ec->h);
        xdg_surface_send_configure(shd->surface, serial);
        ec->comp_data->need_xdg_configure = 0;
        return;
     }

   _xdg_shell_surface_send_configure(resource, ec->fullscreen, !!ec->maximized || ec->comp_data->max, edges, width, height);
}

static void
_e_xdg_popup_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static void
_e_xdg_popup_grab_dismiss(E_Client *ec)
{
   xdg_popup_send_popup_done(ec->comp_data->shell.surface);
   ec->comp_data->grab = 0;
}

static void
_e_xdg_popup_cb_grab(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat EINA_UNUSED, uint32_t serial EINA_UNUSED)
{
   E_Client *ec;

   ec = wl_resource_get_user_data(resource);
   if ((!ec) || e_object_is_del(E_OBJECT(ec)))
     {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                                "No Client For Shell Surface");
        return;
     }
   if (ec->comp_data->mapped)
     {
        wl_resource_post_error(resource, XDG_POPUP_ERROR_INVALID_GRAB,
                                "grab requested on mapped popup");
        return;
     }
   if (e_client_util_is_popup(ec->parent) && (!ec->parent->comp_data->grab))
     {
        wl_resource_post_error(resource, XDG_POPUP_ERROR_INVALID_GRAB,
                                "grab requested on ungrabbed nested popup");
        return;
     }
   e_comp_wl_grab_client_add(ec, _e_xdg_popup_grab_dismiss);
}

static const struct xdg_popup_interface _e_xdg_popup_interface = {
   _e_xdg_popup_cb_destroy,
   _e_xdg_popup_cb_grab,
};

static void
_e_xdg_surface_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static void
_e_xdg_surface_cb_ack_configure(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial)
{
   E_Client *ec;
   Pending_State *ps;
   E_Shell_Data *shd;
   Eina_List *l, *ll;

   ec = wl_resource_get_user_data(resource);
   if (!ec)
     {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                                "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   shd = ec->comp_data->shell.data;
   EINA_LIST_FOREACH_SAFE(shd->pending, l, ll, ps)
     {
        if (ps->serial > serial) break;
        if (ps->state & STATE_FULLSCREEN)
          {
             ec->comp_data->shell.set.fullscreen = 1;
             ec->comp_data->shell.set.unfullscreen = 0;
          }
        if (ps->state & STATE_UNFULLSCREEN)
          {
             ec->comp_data->shell.set.unfullscreen = 1;
             ec->comp_data->shell.set.fullscreen = 0;
          }
        if (ps->state & STATE_MAXIMIZED)
          {
             ec->comp_data->shell.set.maximize = 1;
             ec->comp_data->shell.set.unmaximize = 0;
             if (!ec->comp_data->max)
               ec->comp_data->max = (e_config->maximize_policy & E_MAXIMIZE_TYPE) | E_MAXIMIZE_BOTH;
          }
        if (ps->state & STATE_UNMAXIMIZED)
          {
             ec->comp_data->shell.set.unmaximize = 1;
             ec->comp_data->shell.set.maximize = 0;
             if (!ec->comp_data->unmax)
               ec->comp_data->unmax = (e_config->maximize_policy & E_MAXIMIZE_TYPE) | E_MAXIMIZE_BOTH;
          }
        shd->pending = eina_list_remove_list(shd->pending, l);
        free(ps);
     }
}

static void
_e_xdg_surface_cb_window_geometry_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t w, int32_t h)
{
   E_Client *ec;

   ec = wl_resource_get_user_data(resource);
   if (!ec)
     {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                                "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   EINA_RECTANGLE_SET(&ec->comp_data->shell.window, x, y, w, h);
   //DBG("XDG_SHELL: Window Geom Set: %d \t%d %d, %d %d", wl_resource_get_id(resource), x, y, w, h);
}

static void
_e_xdg_toplevel_cb_maximized_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec;
   int w, h;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (ec->lock_user_maximize) return;
   if (ec->fullscreen)
     {
        w = ec->w, h = ec->h;
        ec->saved.maximized = e_config->maximize_policy | E_MAXIMIZE_BOTH;
     }
   else
     {
        if (e_config->window_maximize_animate && (!ec->maximize_anims_disabled) && ec->comp_data->buffer_commit)
          w = ec->w, h = ec->h;
        else
          {
             switch (e_config->maximize_policy & E_MAXIMIZE_TYPE)
               {
                case E_MAXIMIZE_FULLSCREEN:
                  w = ec->zone->w, h = ec->zone->h;
                  break;
                default:
                  e_zone_useful_geometry_get(ec->zone, NULL, NULL, &w, &h);
               }
          }
     }
   _xdg_shell_surface_send_configure(resource, ec->fullscreen, 1, 0, w, h);
}

static void
_e_xdg_toplevel_cb_maximized_unset(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec;
   int w, h;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (ec->lock_user_maximize) return;
   if (ec->fullscreen)
     {
        w = ec->w, h = ec->h;
        ec->saved.maximized = 0;
     }
   else
     {
        if (e_config->window_maximize_animate && (!ec->maximize_anims_disabled))
          w = ec->w, h = ec->h;
        else
          w = ec->saved.w, h = ec->saved.h;
     }
   _xdg_shell_surface_send_configure(resource, ec->fullscreen, 0, 0, w, h);
}

static void
_e_xdg_toplevel_cb_fullscreen_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *output_resource)
{
   E_Client *ec;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }

   if (ec->lock_user_fullscreen) return;
   if (output_resource)
     {
        Eina_List *l;
        E_Zone *zone;
        E_Comp_Wl_Output *wlo = wl_resource_get_user_data(output_resource);
        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          if (zone->output == wlo)
            ec->comp_data->shell.set.fs_zone = zone->id;
     }
   else
     ec->comp_data->shell.set.fs_zone = ec->zone->id;
   _xdg_shell_surface_send_configure(resource, 1, !!ec->maximized || ec->comp_data->max, 0, ec->zone->w, ec->zone->h);
}

static void
_e_xdg_toplevel_cb_fullscreen_unset(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (ec->lock_user_fullscreen) return;
   e_client_unfullscreen(ec);
}

static void
_e_xdg_toplevel_cb_minimized_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Client *ec;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (ec->lock_user_iconify) return;
   ec->comp_data->shell.set.minimize = 1;
}


#define CONSTRAINED(EC, X, Y) \
   !E_CONTAINS(zx, zy, zw, zh, (X), (Y), (EC)->w, (EC)->h)

static int
_apply_positioner_x(int x, Positioner *p, Eina_Bool invert)
{
   enum xdg_positioner_anchor an = XDG_POSITIONER_ANCHOR_NONE;
   enum xdg_positioner_gravity grav = XDG_POSITIONER_GRAVITY_NONE;

   if (invert)
     {
        if (p->anchor == XDG_POSITIONER_ANCHOR_LEFT)
          an = XDG_POSITIONER_ANCHOR_RIGHT;
        else if (p->anchor == XDG_POSITIONER_ANCHOR_RIGHT)
          an = XDG_POSITIONER_ANCHOR_LEFT;
        if (p->gravity == XDG_POSITIONER_GRAVITY_LEFT)
          grav |= XDG_POSITIONER_GRAVITY_RIGHT;
        else if (p->gravity == XDG_POSITIONER_GRAVITY_RIGHT)
          grav |= XDG_POSITIONER_GRAVITY_LEFT;
     }
   else
     {
        an = p->anchor;
        grav = p->gravity;
     }

   switch (an)
     {
      case XDG_POSITIONER_ANCHOR_LEFT:
      case XDG_POSITIONER_ANCHOR_TOP_LEFT:
      case XDG_POSITIONER_ANCHOR_BOTTOM_LEFT:
        x += p->anchor_rect.x;
        break;
      case XDG_POSITIONER_ANCHOR_RIGHT:
      case XDG_POSITIONER_ANCHOR_TOP_RIGHT:
      case XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT:
        x += p->anchor_rect.x + p->anchor_rect.w;
        break;
      default:
        x += p->anchor_rect.x + (p->anchor_rect.w / 2);
        break;
     }

   switch (grav)
     {
      case XDG_POSITIONER_GRAVITY_LEFT:
      case XDG_POSITIONER_GRAVITY_TOP_LEFT:
      case XDG_POSITIONER_GRAVITY_BOTTOM_LEFT:
        x -= p->size.w;
        break;
      case XDG_POSITIONER_GRAVITY_RIGHT:
      case XDG_POSITIONER_GRAVITY_TOP_RIGHT:
      case XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT:
        x = x;
        break;
      default:
        x -= p->size.w / 2;
        break;
     }

   return x;
}

static int
_apply_positioner_y(int y, Positioner *p, Eina_Bool invert)
{
   enum xdg_positioner_anchor an = XDG_POSITIONER_ANCHOR_NONE;
   enum xdg_positioner_gravity grav = XDG_POSITIONER_GRAVITY_NONE;

   if (invert)
     {
        if (p->anchor == XDG_POSITIONER_ANCHOR_TOP)
          an = XDG_POSITIONER_ANCHOR_BOTTOM;
        else if (p->anchor == XDG_POSITIONER_ANCHOR_BOTTOM)
          an = XDG_POSITIONER_ANCHOR_TOP;
        if (p->gravity == XDG_POSITIONER_GRAVITY_TOP)
          grav |= XDG_POSITIONER_GRAVITY_BOTTOM;
        else if (p->gravity == XDG_POSITIONER_GRAVITY_BOTTOM)
          grav |= XDG_POSITIONER_GRAVITY_TOP;
     }
   else
     {
        an = p->anchor;
        grav = p->gravity;
     }

   switch (an)
     {
      case XDG_POSITIONER_ANCHOR_TOP:
      case XDG_POSITIONER_ANCHOR_TOP_LEFT:
      case XDG_POSITIONER_ANCHOR_TOP_RIGHT:
        y += p->anchor_rect.y;
        break;
      case XDG_POSITIONER_ANCHOR_BOTTOM:
      case XDG_POSITIONER_ANCHOR_BOTTOM_LEFT:
      case XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT:
        y += p->anchor_rect.y + p->anchor_rect.h;
        break;
      default:
        y += p->anchor_rect.y + (p->anchor_rect.h / 2);
        break;
     }

   switch (grav)
     {
      case XDG_POSITIONER_GRAVITY_TOP:
      case XDG_POSITIONER_GRAVITY_TOP_LEFT:
      case XDG_POSITIONER_GRAVITY_TOP_RIGHT:
        y -= p->size.h;
        break;
      case XDG_POSITIONER_GRAVITY_BOTTOM:
      case XDG_POSITIONER_GRAVITY_BOTTOM_LEFT:
      case XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT:
        y = y;
        break;
      default:
        y -= (p->size.h / 2);
     }

   return y;
}

static Eina_Bool
_apply_positioner_slide(E_Client *ec, Positioner *p, int zx, int zy, int zw, int zh)
{
   if ((p->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X) &&
       (!E_CONTAINS(zx, zy, zw, zh, ec->x, zy, ec->w, 1)))
     {
        int sx = ec->x;

        if (p->gravity & XDG_POSITIONER_GRAVITY_LEFT)
          {
             if (ec->x + ec->w > zx + zw)
               sx = MAX(zx + zw - ec->w, ec->parent->x + p->anchor_rect.x - ec->w);
             else if (ec->x < zx)
               sx = MIN(zx, ec->parent->x + p->anchor_rect.x + p->anchor_rect.w);
          }
        else if (p->gravity & XDG_POSITIONER_GRAVITY_RIGHT)
          {
             if (ec->x < zx)
               sx = MIN(zx, ec->parent->x + p->anchor_rect.x + p->anchor_rect.w);
             else if (ec->x + ec->w > zx + zw)
               sx = MAX(zx + zw - ec->w, ec->parent->x + p->anchor_rect.x - ec->w);
          }
        if (E_CONTAINS(zx, zy, zw, zh, sx, zy, ec->w, 1))
          ec->x = sx;
     }
   if (!CONSTRAINED(ec, ec->x, ec->y)) return EINA_TRUE;
   if ((p->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y) &&
       (!E_CONTAINS(zx, zy, zw, zh, zx, ec->y, 1, ec->h)))
     {
        int sy = ec->y;

        if (p->gravity & XDG_POSITIONER_GRAVITY_TOP)
          {
             if (ec->y + ec->h > zy + zh)
               sy = MAX(zy + zh - ec->h, ec->parent->y + p->anchor_rect.y - ec->h);
             else if (ec->y < zy)
               sy = MIN(zy, ec->parent->y + p->anchor_rect.y + p->anchor_rect.h);
          }
        else if (p->gravity & XDG_POSITIONER_GRAVITY_BOTTOM)
          {
             if (ec->y < zy)
               sy = MIN(zy, ec->parent->y + p->anchor_rect.y + p->anchor_rect.h);
             else if (ec->y + ec->h > zy + zh)
               sy = MAX(zy + zh - ec->h, ec->parent->y + p->anchor_rect.y - ec->h);
          }
        if (E_CONTAINS(zx, zy, zw, zh, zx, sy, 1, ec->h))
          ec->y = sy;
     }
   return !CONSTRAINED(ec, ec->x, ec->y);
}

static void
_apply_positioner(E_Client *ec, Positioner *p)
{
   int x, y;
   int zx, zy, zw, zh;

   /* apply base geometry:
    * coords are relative to parent
    */
   x = ec->x = ec->parent->x + p->offset.x;
   y = ec->y = ec->parent->y + p->offset.y;

   if (p->size.w && p->size.h)
     {
        ec->w = p->size.w;
        ec->h = p->size.h;
     }

   /* apply policies in order:
    - anchor (add anchor_rect using anchor point)
    - gravity (perform flips if gravity is not right|bottom)
    - constrain (adjust if popup does not fit)
    */
   ec->x = _apply_positioner_x(x, p, 0);
   ec->y = _apply_positioner_y(y, p, 0);

   e_zone_useful_geometry_get(ec->parent->zone, &zx, &zy, &zw, &zh);

   if (!CONSTRAINED(ec, ec->x, ec->y)) return;

   /* assume smart placement:
    - flip
    - slide
    - resize
    */
   if ((p->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X) &&
       (!E_CONTAINS(zx, zy, zw, zh, ec->x, zy, ec->w, 1)))
     {
        int fx;

        fx = _apply_positioner_x(x, p, 1);
        if (E_CONTAINS(zx, zy, zw, zh, fx, zy, ec->w, 1))
          ec->x = fx;
     }
   if (!CONSTRAINED(ec, ec->x, ec->y)) return;
   if ((p->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y) &&
       (!E_CONTAINS(zx, zy, zw, zh, zx, ec->y, 1, ec->h)))
     {
        int fy;

        fy = _apply_positioner_y(y, p, 1);
        if (E_CONTAINS(zx, zy, zw, zh, zx, fy, 1, ec->h))
          ec->y = fy;
     }
   if (!CONSTRAINED(ec, ec->x, ec->y)) return;
   if (_apply_positioner_slide(ec, p, zx, zy, zw, zh)) return;
   _apply_positioner_slide(ec, p, ec->zone->x, ec->zone->y, ec->zone->w, ec->zone->h);

   if (!CONSTRAINED(ec, ec->x, ec->y)) return;

   if ((p->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X) &&
       (!E_CONTAINS(zx, zy, zw, zh, ec->x, zy, ec->w, 1)))
     {
        if (ec->x < zx) ec->x = zx;
        ec->w = zx + zw - ec->x;
        e_client_resize_limit(ec, &ec->w, &ec->h);
        ec->changes.size = 1;
        if (!CONSTRAINED(ec, ec->x, ec->y)) return;
     }
   if ((p->constrain & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y) &&
       (!E_CONTAINS(zx, zy, zw, zh, zx, ec->y, 1, ec->h)))
     {
        if (ec->y < zy) ec->y = zy;
        ec->h = zy + zh - ec->y;
        e_client_resize_limit(ec, &ec->w, &ec->h);
        ec->changes.size = 1;
     }
}

static void
_e_xdg_surface_cb_popup_get(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *parent_resource, struct wl_resource *positioner_resource)
{
   E_Client *ec;
   E_Comp_Client_Data *cdata;
   Positioner *p;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Pixmap Set On Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_pixmap_usable_get(ec->pixmap))
     {
        wl_resource_post_error(resource,
                               XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                               "buffer attached/committed before configure");
        return;
     }
   p = wl_resource_get_user_data(positioner_resource);
   if (!p)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "invalid positioner");
        return;
     }

   if ((!p->size.w) || (p->anchor_rect.w < 0))
     {
        wl_resource_post_error(resource,
                               XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                               "invalid positioner");
        return;
     }

   cdata = ec->comp_data;

   if (cdata->shell.surface)
     {
        wl_resource_post_error(resource,
                               XDG_WM_BASE_ERROR_ROLE,
                               "surface already has assigned role");
        return;
     }

   /* try to create a shell surface */
   if (!(cdata->shell.surface =
         wl_resource_create(client, &xdg_popup_interface, 1, id)))
     {
        ERR("Could not create xdg shell surface");
        wl_resource_post_no_memory(resource);
        return;
     }

   wl_resource_set_implementation(cdata->shell.surface,
                                  &_e_xdg_popup_interface, ec, e_shell_surface_destroy);

   e_object_ref(E_OBJECT(ec));

   cdata->shell.configure_send = _e_xdg_shell_surface_configure_send;
   cdata->shell.configure = _e_xdg_shell_surface_configure;
   cdata->shell.map = _e_xdg_shell_surface_map;
   cdata->shell.unmap = _e_xdg_shell_surface_unmap;
   cdata->need_xdg_configure = 1;

   if (!ec->internal)
     ec->borderless = !ec->internal_elm_win;
   ec->lock_border = EINA_TRUE;
   if (!ec->internal)
     ec->border.changed = ec->changes.border = !ec->borderless;
   ec->changes.icon = 1;
   EC_CHANGED(ec);
   ec->netwm.type = E_WINDOW_TYPE_POPUP_MENU;
   ec->placed = EINA_TRUE;

   /* set this client as a transient for parent */
   if (parent_resource) e_shell_surface_parent_set(ec, parent_resource);

   _apply_positioner(ec, p);
   ec->client.x = ec->x;
   ec->client.y = ec->y;

   if (ec->internal_elm_win && evas_object_visible_get(ec->internal_elm_win))
     _e_xdg_shell_surface_map(resource);
}

static void
_e_xdg_toplevel_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}
static void
_e_xdg_toplevel_cb_parent_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *parent_resource)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   /* set this client as a transient for parent */
   e_shell_surface_parent_set(ec, parent_resource);
}

static void
_e_xdg_toplevel_cb_title_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *title)
{
   E_Client *ec;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   /* set title */
   eina_stringshare_replace(&ec->icccm.title, title);
   if (ec->frame) e_comp_object_frame_title_set(ec->frame, title);
}

static void
_e_xdg_toplevel_cb_app_id_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *id)
{
   E_Client *ec;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   /* use the wl_client to get the pid * and set it in the netwm props */
   wl_client_get_credentials(client, &ec->netwm.pid, NULL, NULL);

   /* set class */
   eina_stringshare_replace(&ec->icccm.class, id);
   /* eina_stringshare_replace(&ec->netwm.name, id); */
   ec->changes.icon = !!ec->icccm.class;
   EC_CHANGED(ec);
}

static void
_e_xdg_toplevel_cb_window_menu_show(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat_resource EINA_UNUSED, uint32_t serial EINA_UNUSED, int32_t x, int32_t y)
{
   E_Client *ec;
   double timestamp;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   timestamp = ecore_loop_time_get();
   e_int_client_menu_show(ec, ec->x + x, ec->y + y, 0, timestamp);
}

static void
_e_xdg_toplevel_cb_move(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat_resource EINA_UNUSED, uint32_t serial EINA_UNUSED)
{
   E_Client *ec;
   E_Binding_Event_Mouse_Button ev;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   if ((ec->maximized) || (ec->fullscreen)) return;
   if (serial != e_comp_wl->ptr.serial[0]) return;

   switch (e_comp_wl->ptr.button)
     {
      case BTN_LEFT:
        ev.button = 1;
        break;
      case BTN_MIDDLE:
        ev.button = 2;
        break;
      case BTN_RIGHT:
        ev.button = 3;
        break;
      default:
        ev.button = e_comp_wl->ptr.button;
        break;
     }

   e_comp_object_frame_xy_unadjust(ec->frame,
                                   e_comp_wl->ptr.x,
                                   e_comp_wl->ptr.y,
                                   &ev.canvas.x, &ev.canvas.y);

   e_shell_surface_mouse_down_helper(ec, &ev, EINA_TRUE);
}

static void
_e_xdg_toplevel_cb_resize(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat_resource EINA_UNUSED, uint32_t serial EINA_UNUSED, uint32_t edges)
{
   E_Client *ec;
   E_Binding_Event_Mouse_Button ev;

   /* DBG("XDG_SHELL: Surface Resize: %d\tEdges: %d",  */
   /*     wl_resource_get_id(resource), edges); */

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (serial != e_comp_wl->ptr.serial[0]) return;

   if ((edges == 0) || (edges > 15) ||
       ((edges & 3) == 3) || ((edges & 12) == 12)) return;

   if ((ec->maximized) || (ec->fullscreen)) return;

   e_comp_wl->resize.resource = resource;
   e_comp_wl->resize.edges = edges;

   switch (e_comp_wl->ptr.button)
     {
      case BTN_LEFT:
        ev.button = 1;
        break;
      case BTN_MIDDLE:
        ev.button = 2;
        break;
      case BTN_RIGHT:
        ev.button = 3;
        break;
      default:
        ev.button = e_comp_wl->ptr.button;
        break;
     }

   e_comp_object_frame_xy_unadjust(ec->frame,
                                   e_comp_wl->ptr.x,
                                   e_comp_wl->ptr.y,
                                   &ev.canvas.x, &ev.canvas.y);

   e_shell_surface_mouse_down_helper(ec, &ev, EINA_FALSE);
}

static void
_e_xdg_toplevel_cb_max_size_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t width, int32_t height)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   ec->comp_data->shell.set.max_size.w = width;
   ec->comp_data->shell.set.max_size.h = height;
}

static void
_e_xdg_toplevel_cb_min_size_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t width, int32_t height)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   ec->comp_data->shell.set.min_size.w = width;
   ec->comp_data->shell.set.min_size.h = height;
}

static const struct xdg_toplevel_interface _e_xdg_toplevel_interface = {
   _e_xdg_toplevel_cb_destroy,
   _e_xdg_toplevel_cb_parent_set,
   _e_xdg_toplevel_cb_title_set,
   _e_xdg_toplevel_cb_app_id_set,
   _e_xdg_toplevel_cb_window_menu_show,
   _e_xdg_toplevel_cb_move,
   _e_xdg_toplevel_cb_resize,
   _e_xdg_toplevel_cb_max_size_set,
   _e_xdg_toplevel_cb_min_size_set,
   _e_xdg_toplevel_cb_maximized_set,
   _e_xdg_toplevel_cb_maximized_unset,
   _e_xdg_toplevel_cb_fullscreen_set,
   _e_xdg_toplevel_cb_fullscreen_unset,
   _e_xdg_toplevel_cb_minimized_set,
};

static void
_e_xdg_surface_cb_toplevel_get(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t id)
{
   E_Client *ec;
   E_Comp_Client_Data *cdata;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "resource does not have xdg_shell surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_pixmap_usable_get(ec->pixmap))
     {
        wl_resource_post_error(resource,
                               XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                               "buffer attached/committed before configure");
        return;
     }

   cdata = ec->comp_data;
   if (cdata->shell.surface)
     {
        wl_resource_post_error(resource,
                               XDG_WM_BASE_ERROR_ROLE,
                               "surface already has assigned role");
        return;
     }

   if (!(cdata->shell.surface =
         wl_resource_create(client, &xdg_toplevel_interface, 1, id)))
     {
        ERR("Could not create xdg shell surface");
        wl_resource_post_no_memory(resource);
        return;
     }

   wl_resource_set_implementation(cdata->shell.surface,
                                  &_e_xdg_toplevel_interface, ec,
                                  e_shell_surface_cb_destroy);

   e_object_ref(E_OBJECT(ec));

   cdata->shell.configure_send = _e_xdg_shell_surface_configure_send;
   cdata->shell.configure = _e_xdg_shell_surface_configure;
   cdata->shell.map = _e_xdg_shell_surface_map;
   cdata->shell.unmap = _e_xdg_shell_surface_unmap;
   cdata->need_xdg_configure = 1;

   /* set toplevel client properties */
   ec->icccm.accepts_focus = 1;
   if (!ec->internal)
     ec->borderless = 1;
   ec->lock_border = EINA_TRUE;
   ec->changes.icon  = 1;
   EC_CHANGED(ec);
   if ((!ec->internal) || (!ec->borderless))
     ec->border.changed = ec->changes.border = !ec->borderless;
   ec->netwm.type = E_WINDOW_TYPE_NORMAL;

   if (ec->internal_elm_win && evas_object_visible_get(ec->internal_elm_win))
     _e_xdg_shell_surface_map(resource);
}

static const struct xdg_surface_interface _e_xdg_surface_interface =
{
   _e_xdg_surface_cb_destroy,
   _e_xdg_surface_cb_toplevel_get,
   _e_xdg_surface_cb_popup_get,
   _e_xdg_surface_cb_window_geometry_set,
   _e_xdg_surface_cb_ack_configure,
};

static void
_e_xdg_shell_surface_cb_destroy(struct wl_resource *resource)
{
   E_Shell_Data *shd;
   E_Client *ec = wl_resource_get_user_data(resource);

   if (e_object_is_del(E_OBJECT(ec))) return;

   if (ec->comp_data->shell.surface)
     {
        wl_resource_post_error(resource, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES, "shell surface destroyed before role surfaces");
        e_shell_surface_cb_destroy(ec->comp_data->shell.surface);
     }

   shd = ec->comp_data->shell.data;
   if (shd)
     ((vF_Shell_Data*)shd->shell)->surfaces = eina_list_remove(((vF_Shell_Data*)shd->shell)->surfaces, resource);
   e_shell_surface_cb_destroy(resource);
}

static void
_e_xdg_shell_surface_buffer_attach_error(E_Client *ec)
{
   wl_resource_post_error(ec->comp_data->surface,
                               XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                               "buffer attached/committed before configure");
}

static void
_e_xdg_shell_cb_surface_get(struct wl_client *client, struct wl_resource *resource, uint32_t id, struct wl_resource *surface_resource)
{
   E_Client *ec;
   E_Comp_Client_Data *cdata;
   E_Shell_Data *shd;
   vF_Shell_Data *v;

   /* DBG("XDG_SHELL: Surface Get %d", wl_resource_get_id(surface_resource)); */

   /* get the pixmap from this surface so we can find the client */
   if (!(ec = wl_resource_get_user_data(surface_resource)))
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Pixmap Set On Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   if (e_pixmap_usable_get(ec->pixmap))
     {
        wl_resource_post_error(surface_resource,
                               XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER,
                               "buffer attached/committed before configure");
        return;
     }

   ec->netwm.ping = 1;
   cdata = ec->comp_data;

   /* check for existing shell surface */
   if (cdata->shell.data)
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "Client already has XDG shell surface");
        return;
     }
   shd = cdata->shell.data = e_shell_data_new(6);
   shd->width = shd->height = -1;

   /* try to create a shell surface */
   if (!(shd->surface =
         wl_resource_create(client, &xdg_surface_interface, 1, id)))
     {
        ERR("Could not create xdg shell surface");
        wl_resource_post_no_memory(surface_resource);
        return;
     }

   wl_resource_set_implementation(shd->surface,
                                  &_e_xdg_surface_interface, ec,
                                  _e_xdg_shell_surface_cb_destroy);
   v = wl_resource_get_user_data(resource);
   v->surfaces = eina_list_append(v->surfaces, shd->surface);
   shd->shell = v;

   e_object_ref(E_OBJECT(ec));

   cdata->shell.ping = _e_xdg_shell_surface_ping;
   cdata->shell.buffer_attach_error = _e_xdg_shell_surface_buffer_attach_error;
   cdata->is_xdg_surface = EINA_TRUE;

   if (!ec->internal)
     e_client_ping(ec);
}

static void
_e_xdg_shell_cb_pong(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial EINA_UNUSED)
{
   vF_Shell_Data *v;
   Eina_List *l;
   struct wl_resource *res;

   v = wl_resource_get_user_data(resource);
   EINA_LIST_FOREACH(v->surfaces, l, res)
     {
        E_Client *ec = wl_resource_get_user_data(res);

        if (!ec) continue;
        ec->ping_ok = EINA_TRUE;
        ec->hung = EINA_FALSE;
     }
}

static const struct xdg_wm_base_interface _e_xdg_shell_interface =
{
   _e_xdg_shell_cb_destroy,
   _e_xdg_shell_cb_positioner_create,
   _e_xdg_shell_cb_surface_get,
   _e_xdg_shell_cb_pong
};

static void
_xdg_client_destroy(E_Client *ec, Eina_Bool do_list)
{
   E_Shell_Data *shd;

   /* make sure this is a wayland client */
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_WL) return;

   shd = ec->comp_data->shell.data;
   if (shd && (shd->version != 6)) return;
   if (do_list && shd)
     {
        vF_Shell_Data *v;
        struct wl_client *client;
        struct wl_resource *res;

        client = wl_resource_get_client(ec->comp_data->surface);
        res = eina_hash_find(xdg_shell_resources, &client);
        v = wl_resource_get_user_data(res);
        v->surfaces = eina_list_remove(v->surfaces, shd->surface);
     }
   if (ec->comp_data->shell.surface)
     e_shell_surface_destroy(ec->comp_data->shell.surface);
   if (shd)
     e_shell_surface_destroy(shd->surface);
}

static void
_e_xdg_shell_cb_unbind(struct wl_resource *resource)
{
   vF_Shell_Data *v;
   Positioner *p;
   struct wl_resource *res;
   Eina_List *l, *ll;
   struct wl_client *client = wl_resource_get_client(resource);

   v = wl_resource_get_user_data(resource);
   eina_hash_set(xdg_shell_resources, &client, NULL);
   EINA_LIST_REVERSE_FOREACH_SAFE(v->surfaces, l, ll, res)
     {
        E_Client *ec = wl_resource_get_user_data(res);

        if (!e_object_is_del(E_OBJECT(ec)))
          _xdg_client_destroy(ec, 0);
        v->surfaces = eina_list_remove_list(v->surfaces, l);
     }

   EINA_LIST_FREE(v->positioners, p)
     {
        wl_resource_set_user_data(p->res, NULL);
        free(p);
     }
   free(v);
}

static void
_e_xdg_shell_cb_bind(struct wl_client *client, void *data EINA_UNUSED, uint32_t version, uint32_t id)
{
   struct wl_resource *res;
   vF_Shell_Data *v;

   if (!(res = wl_resource_create(client, &xdg_wm_base_interface, version, id)))
     {
        wl_client_post_no_memory(client);
        return;
     }

   eina_hash_set(xdg_shell_resources, &client, res);
   v = E_NEW(vF_Shell_Data, 1);
   wl_resource_set_user_data(res, v);
   wl_resource_set_implementation(res, &_e_xdg_shell_interface,
                                  v, _e_xdg_shell_cb_unbind);
}

static void
_xdg_client_hook_del(void *d EINA_UNUSED, E_Client *ec)
{
   _xdg_client_destroy(ec, 1);
}

EINTERN Eina_Bool
e_xdg_shell_init(void)
{
   /* try to create global xdg_shell interface */
   if (!wl_global_create(e_comp_wl->wl.disp, &xdg_wm_base_interface, 1,
                         NULL, _e_xdg_shell_cb_bind))
     {
        ERR("Could not create xdg_shell global");
        return EINA_FALSE;
     }
   hooks = eina_list_append(hooks, e_client_hook_add(E_CLIENT_HOOK_DEL, _xdg_client_hook_del, NULL));
   return EINA_TRUE;
}
