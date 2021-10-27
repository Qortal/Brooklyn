#define E_COMP_WL
#include "e.h"
#include "e_mod_main.h"

static void
_wl_shell_surface_cb_pong(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial EINA_UNUSED)
{
   E_Client *ec;

   if ((ec = wl_resource_get_user_data(resource)))
     {
        ec->ping_ok = EINA_TRUE;
        ec->hung = EINA_FALSE;
     }
}

static void
_wl_shell_surface_cb_move(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat_resource EINA_UNUSED, uint32_t serial EINA_UNUSED)
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
_wl_shell_surface_cb_resize(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat_resource EINA_UNUSED, uint32_t serial EINA_UNUSED, uint32_t edges)
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

   if ((edges == 0) || (edges > 15) ||
       ((edges & 3) == 3) || ((edges & 12) == 12)) return;

   if ((ec->maximized) || (ec->fullscreen)) return;

   DBG("Comp Resize Edges Set: %d", edges);

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
_wl_shell_surface_cb_toplevel_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
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

   /* set toplevel client properties */
   ec->icccm.accepts_focus = 1;
   if (!ec->internal)
     ec->borderless = !ec->internal;

   ec->lock_border = EINA_TRUE;
   if ((!ec->internal) || (!ec->borderless))
     ec->border.changed = ec->changes.border = !ec->borderless;
   ec->netwm.type = E_WINDOW_TYPE_NORMAL;
   if ((!ec->lock_user_maximize) && (ec->maximized))
     e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
   if ((!ec->lock_user_fullscreen) && (ec->fullscreen))
     e_client_unfullscreen(ec);
   EC_CHANGED(ec);
}

static void
_wl_shell_surface_cb_transient_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *parent_resource, int32_t x EINA_UNUSED, int32_t y EINA_UNUSED, uint32_t flags EINA_UNUSED)
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

   ec->icccm.accepts_focus = 1;
   if (!ec->internal)
     ec->borderless = !ec->internal;

   ec->lock_border = EINA_TRUE;
   if ((!ec->internal) || (!ec->borderless))
     ec->border.changed = ec->changes.border = !ec->borderless;
   ec->netwm.type = E_WINDOW_TYPE_DIALOG;
   ec->dialog = EINA_TRUE;
   if ((!ec->lock_user_maximize) && (ec->maximized))
     e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
   if ((!ec->lock_user_fullscreen) && (ec->fullscreen))
     e_client_unfullscreen(ec);
   EC_CHANGED(ec);
}

static void
_wl_shell_surface_cb_fullscreen_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t method EINA_UNUSED, uint32_t framerate EINA_UNUSED, struct wl_resource *output_resource EINA_UNUSED)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   if (!ec->lock_user_fullscreen)
     e_client_fullscreen(ec, e_config->fullscreen_policy);
}

static void
_wl_shell_surface_cb_popup_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat_resource EINA_UNUSED, uint32_t serial EINA_UNUSED, struct wl_resource *parent_resource, int32_t x, int32_t y, uint32_t flags EINA_UNUSED)
{
   E_Client *ec;
   E_Comp_Client_Data *cdata;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   ec->comp_data->popup.x = x;
   ec->comp_data->popup.y = y;

   if (!ec->internal)
     ec->borderless = !ec->internal_elm_win;
   ec->lock_border = EINA_TRUE;
   if (!ec->internal)
     ec->border.changed = ec->changes.border = !ec->borderless;
   ec->changes.icon = !!ec->icccm.class;
   ec->netwm.type = E_WINDOW_TYPE_POPUP_MENU;

   cdata = ec->comp_data;
   if (ec->parent)
     {
        cdata->popup.x = E_CLAMP(x, 0, ec->parent->client.w);
        cdata->popup.y = E_CLAMP(y, 0, ec->parent->client.h);
     }
   else
     {
        cdata->popup.x = x;
        cdata->popup.y = y;
     }

   /* set this client as a transient for parent */
   e_shell_surface_parent_set(ec, parent_resource);

   EC_CHANGED(ec);
}

static void
_wl_shell_surface_cb_maximized_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *output_resource EINA_UNUSED)
{
   E_Client *ec;

   /* DBG("WL_SHELL: Surface Maximize: %d", wl_resource_get_id(resource)); */

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   /* tell E to maximize this client */
   if (!ec->lock_user_maximize)
     {
        unsigned int edges = 0;

        e_client_maximize(ec, ((e_config->maximize_policy & E_MAXIMIZE_TYPE) |
                               E_MAXIMIZE_BOTH));

        edges = (WL_SHELL_SURFACE_RESIZE_TOP | WL_SHELL_SURFACE_RESIZE_LEFT);
        wl_shell_surface_send_configure(resource, edges, ec->w, ec->h);
     }
}

static void
_wl_shell_surface_cb_title_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *title)
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
_wl_shell_surface_cb_class_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *clas)
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
   eina_stringshare_replace(&ec->icccm.class, clas);
   ec->changes.icon = !!ec->icccm.class;
   EC_CHANGED(ec);
}

static const struct wl_shell_surface_interface _wl_shell_surface_interface =
{
   _wl_shell_surface_cb_pong,
   _wl_shell_surface_cb_move,
   _wl_shell_surface_cb_resize,
   _wl_shell_surface_cb_toplevel_set,
   _wl_shell_surface_cb_transient_set,
   _wl_shell_surface_cb_fullscreen_set,
   _wl_shell_surface_cb_popup_set,
   _wl_shell_surface_cb_maximized_set,
   _wl_shell_surface_cb_title_set,
   _wl_shell_surface_cb_class_set,
};

static void
_wl_shell_surface_configure_send(struct wl_resource *resource, uint32_t edges, int32_t width, int32_t height)
{
   E_Client *ec;

   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_client_util_is_popup(ec)) return;

   wl_shell_surface_send_configure(resource, edges, width, height);
}

static void
_wl_shell_surface_configure(struct wl_resource *resource, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   E_Client *ec;

   /* DBG("WL_SHELL: Surface Configure: %d \t%d %d %d %d",  */
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

   if (ec->parent)
     {
        if (e_client_util_is_popup(ec))
          {
             x = E_CLAMP(ec->parent->client.x + ec->comp_data->popup.x,
                         ec->parent->client.x,
                         ec->parent->client.x +
                         ec->parent->client.w);
             y = E_CLAMP(ec->parent->client.y + ec->comp_data->popup.y,
                         ec->parent->client.y,
                         ec->parent->client.y +
                         ec->parent->client.h);
          }
     }

   if (ec->placed || ec->parent)
     e_client_util_move_resize_without_frame(ec, x, y, w, h);
   else
     e_client_util_resize_without_frame(ec, w, h);
}

static void
_wl_shell_surface_ping(struct wl_resource *resource)
{
   E_Client *ec;
   uint32_t serial;

   /* get the client for this resource */
   if (!(ec = wl_resource_get_user_data(resource)))
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client For Shell Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;

   serial = wl_display_next_serial(e_comp_wl->wl.disp);
   wl_shell_surface_send_ping(ec->comp_data->shell.surface, serial);
}

static void
_wl_shell_surface_map(struct wl_resource *resource)
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

   /* map this surface if needed */
   if ((!ec->comp_data->mapped) && (e_pixmap_usable_get(ec->pixmap)))
     {
        ec->visible = EINA_TRUE;
        evas_object_geometry_set(ec->frame, ec->x, ec->y, ec->w, ec->h);
        ec->comp_data->mapped = EINA_TRUE;
        evas_object_show(ec->frame);
        if (!e_client_util_desk_visible(ec, e_desk_current_get(ec->zone)))
          evas_object_hide(ec->frame);
     }
}

static void
_wl_shell_surface_unmap(struct wl_resource *resource)
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

   if (ec->comp_data->mapped)
     {
        ec->visible = EINA_FALSE;
        evas_object_hide(ec->frame);
        ec->comp_data->mapped = EINA_FALSE;
     }
}

static void
_wl_shell_cb_shell_surface_get(struct wl_client *client, struct wl_resource *resource EINA_UNUSED, uint32_t id, struct wl_resource *surface_resource)
{
   E_Client *ec;
   E_Comp_Client_Data *cdata;

   /* get the pixmap from this surface so we can find the client */
   if (!(ec = wl_resource_get_user_data(surface_resource)))
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Pixmap Set On Surface");
        return;
     }
   if (e_object_is_del(E_OBJECT(ec))) return;
   cdata = ec->comp_data;
   ec->netwm.ping = 1;

   /* check for existing shell surface */
   if (cdata->shell.surface)
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "Client already has shell surface");
        return;
     }

   /* try to create a shell surface */
   if (!(cdata->shell.surface =
         wl_resource_create(client, &wl_shell_surface_interface, 1, id)))
     {
        ERR("Could not create wl_shell surface");
        wl_resource_post_no_memory(surface_resource);
        return;
     }

   wl_resource_set_implementation(cdata->shell.surface,
                                  &_wl_shell_surface_interface,
                                  ec, e_shell_surface_cb_destroy);

   e_object_ref(E_OBJECT(ec));

   cdata->shell.configure_send = _wl_shell_surface_configure_send;
   cdata->shell.configure = _wl_shell_surface_configure;
   cdata->shell.ping = _wl_shell_surface_ping;
   cdata->shell.map = _wl_shell_surface_map;
   cdata->shell.unmap = _wl_shell_surface_unmap;
   if (!ec->internal)
     e_client_ping(ec);
}

static const struct wl_shell_interface _e_shell_interface =
{
   _wl_shell_cb_shell_surface_get
};

static void
_wl_shell_cb_unbind(struct wl_resource *resource)
{
   struct wl_client *client = wl_resource_get_client(resource);
   eina_hash_set(shell_resources, &client, NULL);
}

EINTERN void
wl_shell_cb_bind(struct wl_client *client, void *data EINA_UNUSED, uint32_t version, uint32_t id)
{
   struct wl_resource *res;

   if (!(res = wl_resource_create(client, &wl_shell_interface, version, id)))
     {
        wl_client_post_no_memory(client);
        return;
     }

   eina_hash_set(shell_resources, &client, res);
   wl_resource_set_implementation(res, &_e_shell_interface,
                                  NULL, _wl_shell_cb_unbind);
}
