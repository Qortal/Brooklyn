#define E_COMP_WL
#include "e.h"
#include "e_mod_main.h"

EINTERN Eina_Hash *shell_resources;
EINTERN Eina_Hash *xdg_shell_resources;
EINTERN Eina_List *hooks;

EINTERN void
e_shell_surface_destroy(struct wl_resource *resource)
{
   E_Client *ec;
   E_Shell_Data *shd;

   /* DBG("Shell Surface Destroy: %d", wl_resource_get_id(resource)); */

   /* get the client for this resource */
   ec = wl_resource_get_user_data(resource);
   if (!ec) return;
   wl_resource_set_user_data(resource, NULL);
   /* client may be passed here during DEL hook */
   if (!ec->comp_data) return;

   if (ec->comp_data->grab)
     {
        e_comp_wl_grab_client_del(ec, 0);
        ec->comp_data->grab = 0;
     }


   if (resource == ec->comp_data->shell.surface)
     ec->comp_data->shell.surface = NULL;
   shd = ec->comp_data->shell.data;
   if (shd)
     {
        E_FREE_LIST(shd->pending, free);
        if ((resource == shd->surface) || (!shd->surface))
          E_FREE(ec->comp_data->shell.data);
     }

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

   e_object_unref(E_OBJECT(ec));
}

EINTERN void
e_shell_surface_cb_destroy(struct wl_resource *resource)
{
   /* DBG("Shell Surface Destroy: %d", wl_resource_get_id(resource)); */

   e_shell_surface_destroy(resource);
}

EINTERN void
e_shell_surface_parent_set(E_Client *ec, struct wl_resource *parent_resource)
{
   E_Client *pc;
   uint64_t pwin = 0;

   if (!parent_resource)
     {
        ec->icccm.fetch.transient_for = EINA_FALSE;
        ec->icccm.transient_for = 0;
        if (ec->parent)
          {
             ec->parent->transients =
                eina_list_remove(ec->parent->transients, ec);
             if (ec->parent->modal == ec) ec->parent->modal = NULL;
             ec->parent = NULL;
          }
        return;
     }

   pc = wl_resource_get_user_data(parent_resource);
   if (!pc)
     {
        ERR("Could not get parent resource client");
        return;
     }

   pwin = e_pixmap_window_get(pc->pixmap);

   e_pixmap_parent_window_set(ec->pixmap, pwin);
   e_client_parent_set(ec, pc);

   ec->icccm.fetch.transient_for = EINA_TRUE;
   ec->icccm.transient_for = pwin;
}

EINTERN void
e_shell_surface_mouse_down_helper(E_Client *ec, E_Binding_Event_Mouse_Button *ev, Eina_Bool move)
{
   if (move)
     {
        /* tell E to start moving the client */
        e_client_act_move_begin(ec, ev);

        /* we have to get a reference to the window_move action here, or else
         * when e_client stops the move we will never get notified */
        ec->cur_mouse_action = e_action_find("window_move");
        if (ec->cur_mouse_action)
          e_object_ref(E_OBJECT(ec->cur_mouse_action));
     }
   else
     {
        /* tell E to start resizing the client */
        e_client_act_resize_begin(ec, ev);

        /* we have to get a reference to the window_resize action here,
         * or else when e_client stops the resize we will never get notified */
        ec->cur_mouse_action = e_action_find("window_resize");
        if (ec->cur_mouse_action)
          e_object_ref(E_OBJECT(ec->cur_mouse_action));
     }

   e_focus_event_mouse_down(ec);
}

EINTERN E_Shell_Data *
e_shell_data_new(unsigned int version)
{
   E_Shell_Data *shd;

   shd = E_NEW(E_Shell_Data, 1);
   shd->version = version;
   return shd;
}

static Eina_Bool
desktop_xwayland_startup()
{
   e_startup();
   return ECORE_CALLBACK_RENEW;
}

static void
desktop_startup()
{
   if (e_module_find("xwayland"))
     ecore_event_handler_add(E_EVENT_COMPOSITOR_XWAYLAND_INIT, desktop_xwayland_startup, NULL);
   else
     e_startup();
}

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Wl_Desktop_Shell" };

E_API void *
e_modapi_init(E_Module *m)
{
   Eina_Bool have_shell;

   /* try to create global shell interface */
   if (!wl_global_create(e_comp_wl->wl.disp, &wl_shell_interface, 1,
                         NULL, wl_shell_cb_bind))
     {
        ERR("Could not create shell global");
        return NULL;
     }

   have_shell = e_xdg_shell_v6_init() | e_xdg_shell_init();
   if (!have_shell) return NULL;

#ifdef HAVE_WL_TEXT_INPUT
   if (!e_input_panel_init())
     {
        ERR("Could not init input panel");
        return NULL;
     }
#endif
   ecore_job_add(desktop_startup, NULL);

   shell_resources = eina_hash_pointer_new(NULL);
   xdg_shell_resources = eina_hash_pointer_new(NULL);

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_input_panel_shutdown();
   eina_hash_free(shell_resources);
   eina_hash_free(xdg_shell_resources);
   E_FREE_LIST(hooks, e_client_hook_del);
   return 1;
}
