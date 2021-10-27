#ifndef _E_MOD_MAIN_H
#define _E_MOD_MAIN_H

EINTERN Eina_Bool   e_input_panel_init(void);
EINTERN void        e_input_panel_shutdown(void);

EINTERN extern Eina_Hash *shell_resources;
EINTERN extern Eina_Hash *xdg_shell_resources;
EINTERN extern Eina_List *hooks;


EINTERN void e_shell_surface_destroy(struct wl_resource *resource);
EINTERN void e_shell_surface_cb_destroy(struct wl_resource *resource);
EINTERN void e_shell_surface_parent_set(E_Client *ec, struct wl_resource *parent_resource);
EINTERN void e_shell_surface_mouse_down_helper(E_Client *ec, E_Binding_Event_Mouse_Button *ev, Eina_Bool move);
EINTERN E_Shell_Data *e_shell_data_new(unsigned int version);

EINTERN Eina_Bool e_xdg_shell_v6_init(void);
EINTERN Eina_Bool e_xdg_shell_init(void);
EINTERN void wl_shell_cb_bind(struct wl_client *client, void *data EINA_UNUSED, uint32_t version, uint32_t id);

struct E_Shell_Data
{
   uint32_t edges;
   int32_t width;
   int32_t height;
   Eina_List *pending;
   struct wl_resource *surface;
   void *shell;
   unsigned int version;
   Eina_Bool fullscreen E_BITFIELD;
   Eina_Bool maximized E_BITFIELD;
   Eina_Bool activated E_BITFIELD;
};

#endif
