#ifndef _E_DBUSMENU_H_
#define _E_DBUSMENU_H_

#include <Eina.h>
#include <Eldbus.h>

typedef enum {
   E_DBUSMENU_ITEM_TYPE_STANDARD = 0,
   E_DBUSMENU_ITEM_TYPE_SEPARATOR,
   E_DBUSMENU_ITEM_TYPE_LAST
} E_DBusMenu_Item_Type;

typedef enum {
   E_DBUSMENU_ITEM_TOGGLE_TYPE_NONE = 0,
   E_DBUSMENU_ITEM_TOGGLE_TYPE_CHECKMARK,
   E_DBUSMENU_ITEM_TOGGLE_TYPE_RADIO,
   E_DBUSMENU_ITEM_TOGGLE_TYPE_LAST
} E_DBusMenu_Item_Toggle_Type;

typedef enum {
   E_DBUSMENU_ITEM_DISPOSITION_NORMAL = 0,
   E_DBUSMENU_ITEM_DISPOSITION_INFORMATIVE,
   E_DBUSMENU_ITEM_DISPOSITION_WARNING,
   E_DBUSMENU_ITEM_DISPOSITION_ALERT,
   E_DBUSMENU_ITEM_DISPOSTION_LAST
} E_DBusMenu_Item_Disposition;

typedef enum
{
   E_DBUSMENU_ITEM_EVENT_CLICKED = 0,
   E_DBUSMENU_ITEM_EVENT_HOVERED,
   E_DBUSMENU_ITEM_EVENT_OPENED,
   E_DBUSMENU_ITEM_EVENT_CLOSED,
   E_DBUSMENU_ITEM_EVENT_LAST
} E_DBusMenu_Item_Event;

typedef struct _E_DBusMenu_Item  E_DBusMenu_Item;
typedef struct _E_DBusMenu_Ctx E_DBusMenu_Ctx;

struct _E_DBusMenu_Item
{
   EINA_INLIST;
   unsigned int revision;
   int id;
   unsigned int icon_data_size;
   int references;
   const char *label;
   E_DBusMenu_Item_Type type;
   E_DBusMenu_Item_Toggle_Type toggle_type;
   E_DBusMenu_Item_Disposition disposition;
   Eina_Bool toggle_state;
   Eina_Bool enabled;
   Eina_Bool visible;
   Eina_Bool is_submenu;
   const char *icon_name;
   unsigned char *icon_data;
   Eina_Inlist *sub_items;
   E_DBusMenu_Item *parent;
   E_DBusMenu_Ctx *ctx;
};

typedef void (*E_DBusMenu_Pop_Request_Cb)(void *data, const E_DBusMenu_Item *item);
typedef void (*E_DBusMenu_Update_Cb)(void *data, E_DBusMenu_Item *new_root_item);

E_API E_DBusMenu_Ctx * e_dbusmenu_load(Eldbus_Connection *conn, const char *bus, const char *path, const void *data);
E_API void e_dbusmenu_unload(E_DBusMenu_Ctx *ctx);
E_API void e_dbusmenu_update_cb_set(E_DBusMenu_Ctx *menu_data, E_DBusMenu_Update_Cb cb);
E_API void e_dbusmenu_pop_request_cb_set(E_DBusMenu_Ctx *menu_data, E_DBusMenu_Pop_Request_Cb cb);

E_API void e_dbusmenu_event_send(E_DBusMenu_Item *m, E_DBusMenu_Item_Event event);
E_API void e_dbusmenu_item_ref(E_DBusMenu_Item *m);
E_API void e_dbusmenu_item_unref(E_DBusMenu_Item *m);

#endif
