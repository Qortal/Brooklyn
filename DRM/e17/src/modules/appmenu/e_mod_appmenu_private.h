#ifndef APPMENU_PRIVATE_H
#define APPMENU_PRIVATE_H

#include "e.h"

typedef struct _E_AppMenu_Window E_AppMenu_Window;

typedef struct _E_AppMenu_Context
{
   Eina_List *instances;
   Eldbus_Connection *conn;
   Eldbus_Service_Interface *iface;
   Eina_List *windows;
   unsigned window_with_focus;
   E_AppMenu_Window *window;
   Ecore_Event_Handler *events[2];
} E_AppMenu_Context;

typedef struct _E_AppMenu_Instance
{
   Evas_Object *box;
   Evas *evas;
   E_Gadcon_Client *gcc;
   E_AppMenu_Context *ctx;
   Eina_Bool orientation_horizontal;
} E_AppMenu_Instance;

struct _E_AppMenu_Window
{
   unsigned window_id;
   const char *bus_id;
   const char *path;
   E_DBusMenu_Ctx *dbus_menu;
   E_AppMenu_Context *ctxt;
   E_DBusMenu_Item *root_item;
};

void appmenu_window_free(E_AppMenu_Window *window);
void appmenu_dbus_registrar_server_init(E_AppMenu_Context *ctx);
void appmenu_dbus_registrar_server_shutdown(E_AppMenu_Context *ctx);
void appmenu_application_monitor(void *data, const char *bus, const char *old, const char *new);
void appmenu_menu_render(E_AppMenu_Context *ctxt EINA_UNUSED, E_AppMenu_Window *w);
void appmenu_menu_of_instance_render(E_AppMenu_Instance *inst, E_AppMenu_Window *window);
int  appmenu_menu_count_get(void);
void appmenu_cancel(void);

#endif
