#define E_COMP_WL
#include "e.h"
#include "e_mod_main.h"
#include "input-method-unstable-v1-server-protocol.h"

typedef struct _E_Input_Panel E_Input_Panel;
typedef struct _E_Input_Panel_Surface E_Input_Panel_Surface;

struct _E_Input_Panel
{
   struct wl_resource *resource;
   Eina_List *surfaces;
};

struct _E_Input_Panel_Surface
{
   E_Client *ec;

   Eina_Bool panel;
   Eina_Bool showing;
};

static E_Input_Panel input_panel;
static Eina_List *handlers = NULL;
static struct wl_global *input_panel_global = NULL;

static void
_e_input_panel_surface_cb_toplevel_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *output_resource EINA_UNUSED, uint32_t position EINA_UNUSED)
{
   E_Input_Panel_Surface *ips;

   ips = wl_resource_get_user_data(resource);
   ips->panel = EINA_FALSE;
}

static void
_e_input_panel_surface_cb_overlay_panel_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Input_Panel_Surface *ips;

   ips = wl_resource_get_user_data(resource);
   ips->panel = EINA_TRUE;
}

static const struct zwp_input_panel_surface_v1_interface _e_input_panel_surface_implementation = {
     _e_input_panel_surface_cb_toplevel_set,
     _e_input_panel_surface_cb_overlay_panel_set
};

static void
_e_input_panel_surface_resource_destroy(struct wl_resource *resource)
{
   E_Input_Panel_Surface *ips;
   E_Client *ec;

   ips = wl_resource_get_user_data(resource);
   ec = ips->ec;
   if (!e_object_is_del(E_OBJECT(ec)))
     {
        if (ec->comp_data->mapped)
          {
             if ((ec->comp_data->shell.surface) &&
                 (ec->comp_data->shell.unmap))
               ec->comp_data->shell.unmap(ec->comp_data->shell.surface);
          }
        if (ec->parent)
          {
             ec->parent->transients =
                eina_list_remove(ec->parent->transients, ec);
          }
        ec->comp_data->shell.surface = NULL;
     }

   input_panel.surfaces = eina_list_remove(input_panel.surfaces, ips);

   free(ips);
}

static void
_e_input_panel_position_set(E_Client *ec, int w, int h)
{
   int nx, ny;
   int zx, zy, zw, zh;

   e_zone_useful_geometry_get(ec->zone, &zx, &zy, &zw, &zh);

   nx = zx + (zw - w) / 2;
   ny = zy + zh - h;

   e_client_util_move_without_frame(ec, nx, ny);
}

static void
_e_input_panel_surface_visible_update(E_Input_Panel_Surface *ips)
{
   E_Client *ec;

   ec = ips->ec;
   if ((ips->showing) &&
       (e_pixmap_usable_get(ec->pixmap)))
     {
        if (!ips->panel)
          _e_input_panel_position_set(ec, ec->client.w, ec->client.h);

        ec->visible = EINA_TRUE;
        evas_object_geometry_set(ec->frame, ec->x, ec->y, ec->w, ec->h);
        evas_object_show(ec->frame);
        e_comp_object_damage(ec->frame, ec->x, ec->y, ec->w, ec->h);
     }
   else
     {
        ec->visible = EINA_FALSE;
        evas_object_hide(ec->frame);
     }
}

static void
_e_input_panel_surface_configure(struct wl_resource *resource, Evas_Coord x EINA_UNUSED, Evas_Coord y EINA_UNUSED, Evas_Coord w, Evas_Coord h)
{
   E_Input_Panel_Surface *ips;

   ips = wl_resource_get_user_data(resource);

   e_client_util_resize_without_frame(ips->ec, w, h);

   if (ips->showing)
     _e_input_panel_surface_visible_update(ips);
}

static void
_e_input_panel_surface_map(struct wl_resource *resource)
{
   E_Input_Panel_Surface *ips;
   E_Client *ec;

   ips = wl_resource_get_user_data(resource);
   ec = ips->ec;

   if (e_object_is_del(E_OBJECT(ec)))
     return;

   // NOTE: we need to set mapped, so that avoid showing evas_object and continue buffer's commit process.
   if ((!ec->comp_data->mapped) && (e_pixmap_usable_get(ec->pixmap)))
     ec->comp_data->mapped = EINA_TRUE;
}

static void
_e_input_panel_surface_unmap(struct wl_resource *resource)
{
   E_Input_Panel_Surface *ips;
   E_Client *ec;

   ips = wl_resource_get_user_data(resource);
   ec = ips->ec;

   if (e_object_is_del(E_OBJECT(ec)))
     return;

   if (ec->comp_data->mapped)
     {
        ec->visible = EINA_FALSE;
        evas_object_hide(ec->frame);
        ec->comp_data->mapped = EINA_FALSE;
     }
}

static void
_e_input_panel_cb_surface_get(struct wl_client *client, struct wl_resource *resource EINA_UNUSED, uint32_t id, struct wl_resource *surface_resource)
{
   E_Client *ec;
   E_Input_Panel_Surface *ips;
   E_Comp_Client_Data *cd;

   ec = wl_resource_get_user_data(surface_resource);
   if (!ec)
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Client Set On Surface");
        return;
     }

   cd = ec->comp_data;
   if (!cd)
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Comp Data For Client");
        return;
     }

   /* check for existing shell surface */
   if (ec->comp_data->shell.surface)
     {
        wl_resource_post_error(surface_resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "Client already has shell surface");
        return;
     }

   ips = E_NEW(E_Input_Panel_Surface, 1);
   if (!ips)
     {
        wl_client_post_no_memory(client);
        return;
     }

   cd->shell.surface = wl_resource_create(client,
                                          &zwp_input_panel_surface_v1_interface,
                                          1, id);
   if (!cd->shell.surface)
     {
        wl_client_post_no_memory(client);
        free(ips);
        return;
     }

   ips->ec = ec;

   EC_CHANGED(ec);
   if (!ec->new_client)
     {
        ec->new_client = EINA_TRUE;
        e_comp->new_clients++;
     }
   if (ec->ignored)
     e_client_unignore(ec);

   /* set input panel client properties */
   ec->borderless = EINA_TRUE;
   ec->argb = EINA_TRUE;
   ec->lock_border = EINA_TRUE;
   ec->lock_focus_in = ec->lock_focus_out = EINA_TRUE;
   ec->netwm.state.skip_taskbar = EINA_TRUE;
   ec->netwm.state.skip_pager = EINA_TRUE;
   ec->no_shape_cut = EINA_TRUE;
   ec->border_size = 0;
   ec->netwm.type = E_WINDOW_TYPE_UTILITY;


   cd->surface = surface_resource;
   cd->shell.configure_send = NULL;
   cd->shell.configure = _e_input_panel_surface_configure;
   cd->shell.ping = NULL;
   cd->shell.map = _e_input_panel_surface_map;
   cd->shell.unmap = _e_input_panel_surface_unmap;


   wl_resource_set_implementation(cd->shell.surface,
                                  &_e_input_panel_surface_implementation,
                                  ips, _e_input_panel_surface_resource_destroy);

   input_panel.surfaces = eina_list_append(input_panel.surfaces, ips);
}

static const struct zwp_input_panel_v1_interface _e_input_panel_implementation = {
     _e_input_panel_cb_surface_get
};

static void
_e_input_panel_unbind(struct wl_resource *resource EINA_UNUSED)
{
   input_panel.resource = NULL;

   E_FREE_FUNC(input_panel.surfaces, eina_list_free);
}

static void
_e_input_panel_bind(struct wl_client *client, void *data EINA_UNUSED, uint32_t version EINA_UNUSED, uint32_t id)
{
   struct wl_resource *resource;

   resource = wl_resource_create(client, &zwp_input_panel_v1_interface, 1, id);
   if (!resource)
     {
        wl_client_post_no_memory(client);
        return;
     }

   if (input_panel.resource)
     {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "interface object already bound");
        return;
     }

   input_panel.resource = resource;

   wl_resource_set_implementation(resource,
                                  &_e_input_panel_implementation,
                                  NULL, _e_input_panel_unbind);
}

static Eina_Bool
_e_input_panel_cb_visible_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Text_Input_Panel_Visibility_Change *ev = event;
   E_Input_Panel_Surface *ips;
   Eina_List *l;

   EINA_LIST_FOREACH(input_panel.surfaces, l, ips)
     {
        if (!ips->ec) continue;
        ips->showing = ev->visible;
        _e_input_panel_surface_visible_update(ips);
     }

   return ECORE_CALLBACK_RENEW;
}

EINTERN Eina_Bool
e_input_panel_init(void)
{
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_TEXT_INPUT_PANEL_VISIBILITY_CHANGE,
                         _e_input_panel_cb_visible_change, NULL);
   // TODO: add signal handler - update input panel

   input_panel_global = wl_global_create(e_comp->wl_comp_data->wl.disp,
                                         &zwp_input_panel_v1_interface, 1,
                                         NULL, _e_input_panel_bind);
   if (!input_panel_global)
     {
        ERR("failed to create wl_global for input panel");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EINTERN void
e_input_panel_shutdown(void)
{
   E_FREE_FUNC(input_panel_global, wl_global_destroy);
   E_FREE_LIST(handlers, ecore_event_handler_del);
}
