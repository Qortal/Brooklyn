#include "e.h"

#define DBUS_MENU_IFACE "com.canonical.dbusmenu"

struct _E_DBusMenu_Ctx
{
   Eldbus_Proxy              *proxy;
   E_DBusMenu_Item          *root_menu;
   void                     *data;
   E_DBusMenu_Pop_Request_Cb pop_request_cb;
   E_DBusMenu_Update_Cb      update_cb;
   Eina_Bool  hacks E_BITFIELD;
};

static const char *Menu_Item_Type_Names[] =
{
   "standard", "separator"
};

static const char *Menu_Item_Toggle_Type_Names[] =
{
   "", "checkmark", "radio"
};

static const char *Menu_Item_Dispostion_Names[] =
{
   "normal", "informative", "warning", "alert"
};

static const char *Menu_Item_Event_Names[] =
{
   "clicked", "hovered", "opened", "closed"
};

static void proxy_init(E_DBusMenu_Ctx *ctx);

static int
id_find(const char *text, const char *array_of_names[], unsigned max)
{
   unsigned int i;
   for (i = 0; i < max; i++)
     {
        if (strcmp(text, array_of_names[i])) continue;
        return i;
     }
   return 0;
}

static void
dbus_menu_prop_dict_cb(void *data, const void *key, Eldbus_Message_Iter *var)
{
   E_DBusMenu_Item *m = data;
   if (!strcmp(key, "label"))
     {
        const char *label;
        Eina_Strbuf *label_buf = eina_strbuf_new();
        int i;

        eldbus_message_iter_arguments_get(var, "s", &label);
        for (i = 0; label[i]; i++)
          {
             if (label[i] != '_')
               {
                  eina_strbuf_append_char(label_buf, label[i]);
                  continue;
               }

             if (label[i + 1] == '_')
               {
                  eina_strbuf_append_char(label_buf, label[i]);
                  i++;
                  continue;
               }
          }
        eina_stringshare_replace(&m->label, eina_strbuf_string_get(label_buf));
        eina_strbuf_free(label_buf);
     }
   else if (!strcmp(key, "type"))
     {
        const char *type;
        eldbus_message_iter_arguments_get(var, "s", &type);
        m->type = id_find(type, Menu_Item_Type_Names, E_DBUSMENU_ITEM_TYPE_LAST);
     }
   else if (!strcmp(key, "icon-data"))
     {
        Eldbus_Message_Iter *array;
        int size;
        const unsigned char *img_data;

        eldbus_message_iter_arguments_get(var, "ay", &array);
        eldbus_message_iter_fixed_array_get(array, 'y', &img_data, &size);
        if (!size) return;
        m->icon_data = malloc(sizeof(unsigned char) * size);
        EINA_SAFETY_ON_FALSE_RETURN(m->icon_data);
        memcpy(m->icon_data, img_data, size);
        m->icon_data_size = size;
     }
   else if (!strcmp(key, "icon-name"))
     {
        const char *icon_name;
        eldbus_message_iter_arguments_get(var, "s", &icon_name);
        eina_stringshare_replace(&m->icon_name, icon_name);
     }
   else if (!strcmp(key, "toggle-type"))
     {
        const char *toggle_type;
        eldbus_message_iter_arguments_get(var, "s", &toggle_type);
        m->toggle_type = id_find(toggle_type, Menu_Item_Toggle_Type_Names,
                                 E_DBUSMENU_ITEM_TOGGLE_TYPE_LAST);
     }
   else if (!strcmp(key, "toggle-state"))
     {
        int state;
        eldbus_message_iter_arguments_get(var, "i", &state);
        if (state == 1) m->toggle_state = EINA_TRUE;
        else m->toggle_state = EINA_FALSE;
     }
   else if (!strcmp(key, "children-display"))
     {
        const char *display;
        eldbus_message_iter_arguments_get(var, "s", &display);
        if (!strcmp(display, "submenu")) m->is_submenu = EINA_TRUE;
        else m->is_submenu = EINA_FALSE;
     }
   else if (!strcmp(key, "disposition"))
     {
        const char *disposition;
        eldbus_message_iter_arguments_get(var, "s", &disposition);
        m->disposition = id_find(disposition, Menu_Item_Dispostion_Names,
                                 E_DBUSMENU_ITEM_DISPOSTION_LAST);
     }
   else if (!strcmp(key, "enabled"))
     eldbus_message_iter_arguments_get(var, "b", &m->enabled);
   else if (!strcmp(key, "visible"))
     eldbus_message_iter_arguments_get(var, "b", &m->visible);
}

static E_DBusMenu_Item *
parse_layout(Eldbus_Message_Iter *layout, E_DBusMenu_Item *parent, E_DBusMenu_Ctx *ctx)
{
   Eldbus_Message_Iter *menu_item_prop, *sub_menu_items_prop, *var;
   E_DBusMenu_Item *m = calloc(1, sizeof(E_DBusMenu_Item));
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);
   m->references = 1;
   m->ctx = ctx;
   m->enabled = EINA_TRUE;
   m->visible = EINA_TRUE;

   if (!eldbus_message_iter_arguments_get(layout, "ia{sv}av", &m->id,
                                         &menu_item_prop, &sub_menu_items_prop))
     {
        ERR("Error reading message");
        free(m);
        return NULL;
     }

   eldbus_message_iter_dict_iterate(menu_item_prop, "sv", dbus_menu_prop_dict_cb, m);

   while (eldbus_message_iter_get_and_next(sub_menu_items_prop, 'v', &var))
     {
        Eldbus_Message_Iter *st;
        if (!eldbus_message_iter_arguments_get(var, "(ia{sv}av)", &st))
          {
             ERR("Error readding message.");
             continue;
          }
        parse_layout(st, m, ctx);
     }

   if (!parent) return m;

   parent->sub_items = eina_inlist_append(parent->sub_items, EINA_INLIST_GET(m));
   m->parent = parent;
   return NULL;
}

static void
dbus_menu_free(E_DBusMenu_Item *m)
{
   Eina_Inlist *inlist;
   E_DBusMenu_Item *child;

   EINA_INLIST_FOREACH_SAFE(m->sub_items, inlist, child)
     {
        e_dbusmenu_item_unref(child);
     }
   EINA_INLIST_FREE(m->sub_items, child)
     {
        m->sub_items = eina_inlist_remove
          (m->sub_items, EINA_INLIST_GET(child));
        child->parent = NULL;
     }
   if (m->icon_name) eina_stringshare_del(m->icon_name);
   if (m->label) eina_stringshare_del(m->label);
   if (m->parent) m->parent->sub_items = eina_inlist_remove
     (m->parent->sub_items, EINA_INLIST_GET(m));
   if (m->icon_data_size) free(m->icon_data);
   free(m);
}

static Eina_Bool
attempt_hacks(E_DBusMenu_Ctx *ctx)
{
   /* https://phab.enlightenment.org/T3139 */
   Eldbus_Object *obj;
   Eldbus_Connection *conn;
   const char *bus, *p;
   int n;
   char buf[1024] = {0}, buf2[1024 + 12] = {0};

   if (ctx->hacks) return EINA_FALSE;
   obj = eldbus_proxy_object_get(ctx->proxy);
   conn = eldbus_object_connection_get(obj);
   bus = eldbus_object_bus_name_get(obj);
   if (bus[0] != ':') return EINA_FALSE;
   /* if this is a qt5 app, menu bus is $bus + 2
    * ...probably
    */

   p = strchr(bus + 1, '.');
   if (!p) return EINA_FALSE;
   p++;
   if (!p[0]) return EINA_FALSE;
   n = strtol(p, NULL, 10);
   if (n == -1) return EINA_FALSE;
   n += 2;
   if ((unsigned int)(p - bus) > sizeof(buf) - 1) return EINA_FALSE;
   strncpy(buf, bus, p - bus);
   snprintf(buf2, sizeof(buf2), "%s%d", buf, n);
   E_FREE_FUNC(ctx->root_menu, e_dbusmenu_item_unref);
   eldbus_proxy_unref(ctx->proxy);
   eldbus_object_unref(obj);

   obj = eldbus_object_get(conn, buf2, "/MenuBar");
   ctx->proxy = eldbus_proxy_get(obj, DBUS_MENU_IFACE);
   proxy_init(ctx);
   ctx->hacks = 1;
   return EINA_TRUE;
}

static void
layout_get_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   E_DBusMenu_Item *m;
   const char *error, *error_msg;
   Eldbus_Message_Iter *layout;
   unsigned revision;
   E_DBusMenu_Ctx *ctx = data;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        ERR("%s %s", error, error_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u(ia{sv}av)", &revision, &layout))
     {
        ERR("Error reading message");
        return;
     }

   m = parse_layout(layout, NULL, ctx);
   m->revision = revision;
   if (m->is_submenu && (!m->parent) && (!m->sub_items))
     {
        if (attempt_hacks(ctx))
          {
             e_dbusmenu_item_unref(m);
             return;
          }
     }

   if (ctx->update_cb) ctx->update_cb(ctx->data, m);
   if (ctx->root_menu) e_dbusmenu_item_unref(ctx->root_menu);
   ctx->root_menu = m;
}

static E_DBusMenu_Item *
dbus_menu_find(E_DBusMenu_Ctx *ctx, int id)
{
   E_DBusMenu_Item *m;
   if (!ctx)
     return NULL;

   EINA_INLIST_FOREACH(ctx->root_menu, m)
     {
        E_DBusMenu_Item *child, *found;
        if (m->id == id) return m;
        EINA_INLIST_FOREACH(m->sub_items, child)
          {
             found = dbus_menu_find(ctx, id);
             if (found) return found;
          }
     }
   return NULL;
}

static void
menu_pop_request(void *data, const Eldbus_Message *msg)
{
   E_DBusMenu_Ctx *ctx = data;
   int id;
   unsigned timestamp;
   E_DBusMenu_Item *m;

   if (!eldbus_message_arguments_get(msg, "iu", &id, &timestamp))
     {
        ERR("Error reading values.");
        return;
     }

   m = dbus_menu_find(ctx, id);
   if (!m) return;
   if (ctx->pop_request_cb) ctx->pop_request_cb(ctx->data, m);
}

static void
prop_changed_cb(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *interface, *propname;
   Eldbus_Message_Iter *variant, *array;

   if (!eldbus_message_arguments_get(msg, "ssv", &interface, &propname, &variant))
     {
        ERR("Error getting values");
        return;
     }

   if (strcmp(propname, "IconThemePath")) return;

   if (!eldbus_message_iter_arguments_get(variant, "as", &array))
     {
        //TODO
     }
}

static void
layout_update(E_DBusMenu_Ctx *ctx)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *main_iter, *array;

   msg = eldbus_proxy_method_call_new(ctx->proxy, "GetLayout");
   main_iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(main_iter, "iias", 0, -1, &array);
   eldbus_message_iter_container_close(main_iter, array);
   eldbus_proxy_send(ctx->proxy, msg, layout_get_cb, ctx, -1);
}

static void
layout_updated_cb(void *data, const Eldbus_Message *msg EINA_UNUSED)
{
   E_DBusMenu_Ctx *ctx = data;
   layout_update(ctx);
}

static void
proxy_init(E_DBusMenu_Ctx *ctx)
{
   layout_update(ctx);
   eldbus_proxy_signal_handler_add(ctx->proxy,
                                  "ItemActivationRequested",
                                  menu_pop_request, ctx);

   eldbus_proxy_properties_changed_callback_add(ctx->proxy,
                                               prop_changed_cb, ctx);

   eldbus_proxy_signal_handler_add(ctx->proxy, "ItemsPropertiesUpdated",
                                  layout_updated_cb, ctx);
   eldbus_proxy_signal_handler_add(ctx->proxy, "LayoutUpdated",
                                  layout_updated_cb, ctx);
}

E_API E_DBusMenu_Ctx *
e_dbusmenu_load(Eldbus_Connection *conn, const char *bus, const char *path, const void *data)
{
   Eldbus_Object *obj;
   E_DBusMenu_Ctx *ctx;
   EINA_SAFETY_ON_NULL_RETURN_VAL(bus, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   ctx = calloc(1, sizeof(E_DBusMenu_Ctx));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);

   ctx->data = (void *)data;

   eldbus_connection_ref(conn);
   obj = eldbus_object_get(conn, bus, path);
   ctx->proxy = eldbus_proxy_get(obj, DBUS_MENU_IFACE);
   proxy_init(ctx);
   return ctx;
}

E_API void
e_dbusmenu_event_send(E_DBusMenu_Item *m, E_DBusMenu_Item_Event event)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *main_iter, *var;
   unsigned int timestamp = (unsigned int)time(NULL);

   EINA_SAFETY_ON_NULL_RETURN(m);
   EINA_SAFETY_ON_FALSE_RETURN(event < E_DBUSMENU_ITEM_EVENT_LAST);
   EINA_SAFETY_ON_NULL_RETURN(m->ctx);
   m->ctx->proxy = eldbus_proxy_ref(m->ctx->proxy);

   msg = eldbus_proxy_method_call_new(m->ctx->proxy, "Event");
   main_iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(main_iter, "is", m->id,
                                       Menu_Item_Event_Names[event]);

   var = eldbus_message_iter_container_new(main_iter, 'v', "s");
   /* dummy data */
   eldbus_message_iter_arguments_append(var, "s", "");
   eldbus_message_iter_container_close(main_iter, var);

   eldbus_message_iter_arguments_append(main_iter, "u", timestamp);

   eldbus_proxy_send(m->ctx->proxy, msg, NULL, NULL, -1);
}

E_API void
e_dbusmenu_item_ref(E_DBusMenu_Item *m)
{
   m->references++;
}

E_API void
e_dbusmenu_item_unref(E_DBusMenu_Item *m)
{
   m->references--;
   if (m->references == 0) dbus_menu_free(m);
}

E_API void
e_dbusmenu_unload(E_DBusMenu_Ctx *ctx)
{
   Eldbus_Connection *conn;
   Eldbus_Object *obj;
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   if (ctx->root_menu) e_dbusmenu_item_unref(ctx->root_menu);
   obj = eldbus_proxy_object_get(ctx->proxy);
   conn = eldbus_object_connection_get(obj);
   eldbus_proxy_unref(ctx->proxy);
   eldbus_object_unref(obj);
   eldbus_connection_unref(conn);
   free(ctx);
}

E_API void
e_dbusmenu_pop_request_cb_set(E_DBusMenu_Ctx *ctx, E_DBusMenu_Pop_Request_Cb cb)
{
   ctx->pop_request_cb = cb;
}

E_API void
e_dbusmenu_update_cb_set(E_DBusMenu_Ctx *ctx, E_DBusMenu_Update_Cb cb)
{
   ctx->update_cb = cb;
}

