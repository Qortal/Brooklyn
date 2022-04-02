#include "e.h"
#include "evry_api.h"

#define BORDER_SHOW       1
#define BORDER_HIDE       2
#define BORDER_FULLSCREEN 3
#define BORDER_TODESK     4
#define BORDER_CLOSE      5

typedef struct _Plugin      Plugin;
typedef struct _Border_Item Client_Item;

struct _Plugin
{
   Evry_Plugin base;
   Eina_List  *clients;
   Eina_List  *handlers;
   const char *input;
};

struct _Border_Item
{
   Evry_Item base;
   E_Client *client;
};

static const Evry_API *evry = NULL;
static Evry_Module *evry_module = NULL;
static Evry_Plugin *_plug;
static Eina_List *_actions = NULL;

static Evas_Object *_icon_get(Evry_Item *it, Evas *e);

/***************************************************************************/

#define GET_BORDER(_bd, _it) Client_Item * _bd = (Client_Item *)_it;

static void
_client_item_free(Evry_Item *it)
{
   GET_BORDER(bi, it);

   e_object_unref(E_OBJECT(bi->client));

   E_FREE(bi);
}

static int
_client_item_add(Plugin *p, E_Client *ec)
{
   Client_Item *bi;
   char buf[1024];

   if (ec->netwm.state.skip_taskbar)
     return 0;
   if (ec->netwm.state.skip_pager)
     return 0;
   if (e_client_util_ignored_get(ec))
     return 0;

   bi = EVRY_ITEM_NEW(Client_Item, p, e_client_util_name_get(ec),
                      _icon_get, _client_item_free);

   snprintf(buf, sizeof(buf), "%d:%d %s",
            ec->desk->x, ec->desk->y,
            (ec->desktop ? ec->desktop->name : ""));
   EVRY_ITEM_DETAIL_SET(bi, buf);

   bi->client = ec;
   e_object_ref(E_OBJECT(ec));

   p->clients = eina_list_append(p->clients, bi);

   return 1;
}

static Eina_Bool
_cb_border_remove(void *data, EINA_UNUSED int type, void *event)
{
   E_Event_Client *ev = event;
   Client_Item *bi;
   Eina_List *l;
   Plugin *p = data;

   EINA_LIST_FOREACH(p->clients, l, bi)
     if (bi->client == ev->ec)
       break;

   if (!bi) return ECORE_CALLBACK_PASS_ON;

   EVRY_PLUGIN_ITEMS_CLEAR(p);

   p->clients = eina_list_remove(p->clients, bi);
   EVRY_ITEM_FREE(bi);

   EVRY_PLUGIN_ITEMS_ADD(p, p->clients, p->input, 1, 0);

   EVRY_PLUGIN_UPDATE(p, EVRY_UPDATE_ADD);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_cb_client_add(void *data, EINA_UNUSED int type, void *event)
{
   E_Event_Client *ev = event;
   Plugin *p = data;

   if (e_client_util_ignored_get(ev->ec)) return ECORE_CALLBACK_RENEW;
   if (!_client_item_add(p, ev->ec))
     return ECORE_CALLBACK_PASS_ON;

   EVRY_PLUGIN_ITEMS_CLEAR(p);

   EVRY_PLUGIN_ITEMS_ADD(p, p->clients, p->input, 1, 0);

   EVRY_PLUGIN_UPDATE(p, EVRY_UPDATE_ADD);

   return ECORE_CALLBACK_PASS_ON;
}

static Evry_Plugin *
_begin(Evry_Plugin *plugin, const Evry_Item *item EINA_UNUSED)
{
   Plugin *p;
   E_Client *ec;
   Eina_List *l;

   EVRY_PLUGIN_INSTANCE(p, plugin);

   p->handlers = eina_list_append
       (p->handlers, ecore_event_handler_add
         (E_EVENT_CLIENT_REMOVE, _cb_border_remove, p));

   p->handlers = eina_list_append
       (p->handlers, ecore_event_handler_add
         (E_EVENT_CLIENT_ADD, _cb_client_add, p));

   EINA_LIST_FOREACH (e_client_focus_stack_get(), l, ec)
     _client_item_add(p, ec);

   return EVRY_PLUGIN(p);
}

static void
_finish(Evry_Plugin *plugin)
{
   Ecore_Event_Handler *h;
   Client_Item *bi;

   GET_PLUGIN(p, plugin);

   IF_RELEASE(p->input);

   EVRY_PLUGIN_ITEMS_CLEAR(p);

   EINA_LIST_FREE (p->clients, bi)
     EVRY_ITEM_FREE(bi);

   EINA_LIST_FREE (p->handlers, h)
     ecore_event_handler_del(h);

   E_FREE(p);
}

static int
_fetch(Evry_Plugin *plugin, const char *input)
{
   GET_PLUGIN(p, plugin);

   EVRY_PLUGIN_ITEMS_CLEAR(p);

   EVRY_PLUGIN_MIN_QUERY(p, input)
   {
      IF_RELEASE(p->input);

      if (input)
        p->input = eina_stringshare_add(input);

      return EVRY_PLUGIN_ITEMS_ADD(p, p->clients, input, 1, 0);
   }

   return 0;
}

static Evas_Object *
_icon_get(Evry_Item *it, Evas *e)
{
   GET_BORDER(bi, it);

   E_Client *ec = bi->client;

   return e_client_icon_add(ec, e);
}

/***************************************************************************/

static int
_check_border(Evry_Action *act, const Evry_Item *it)
{
   GET_BORDER(bi, it);

   int action = EVRY_ITEM_DATA_INT_GET(act);
   E_Client *ec = bi->client;
   E_Zone *zone = e_zone_current_get();

   if (!ec)
     {
        ERR("no client");
        return 0;
     }

   switch (action)
     {
      case BORDER_CLOSE:
        if (ec->lock_close)
          return 0;
        break;

      case BORDER_SHOW:
        if (ec->lock_focus_in)
          return 0;
        break;

      case BORDER_HIDE:
        if (ec->lock_user_iconify)
          return 0;
        break;

      case BORDER_FULLSCREEN:
        if (!ec->lock_user_fullscreen)
          return 0;
        break;

      case BORDER_TODESK:
        if (ec->desk == (e_desk_current_get(zone)))
          return 0;
        break;
     }

   return 1;
}

static int
_act_border(Evry_Action *act)
{
   GET_BORDER(bi, act->it1.item);

   int action = EVRY_ITEM_DATA_INT_GET(act);
   E_Client *ec = bi->client;
   E_Zone *zone = e_zone_current_get();
   int focus = 0;

   if (!ec)
     {
        ERR("no client");
        return 0;
     }

   switch (action)
     {
      case BORDER_CLOSE:
        e_client_act_close_begin(ec);
        break;

      case BORDER_SHOW:
        if (ec->desk != (e_desk_current_get(zone)))
          e_desk_show(ec->desk);
        focus = 1;
        break;

      case BORDER_HIDE:
        e_client_iconify(ec);
        break;

      case BORDER_FULLSCREEN:
        if (!ec->fullscreen)
          e_client_fullscreen(ec, E_FULLSCREEN_RESIZE);
        else
          e_client_unfullscreen(ec);
        break;

      case BORDER_TODESK:
        if (ec->desk != (e_desk_current_get(zone)))
          e_client_desk_set(ec, e_desk_current_get(zone));
        focus = 1;
        break;

      default:
        break;
     }

   if (focus)
     {
        if (ec->shaded)
          e_client_unshade(ec, ec->shade_dir);

        if (ec->iconic)
          e_client_uniconify(ec);
        else
          evas_object_raise(ec->frame);

        if (!ec->lock_focus_out)
          {
             evas_object_focus_set(ec->frame, 1);
             e_client_focus_latest_set(ec);
          }

        if ((e_config->focus_policy != E_FOCUS_CLICK) ||
            (e_config->winlist_warp_at_end) ||
            (e_config->winlist_warp_while_selecting))
          {
             int warp_to_x = ec->x + (ec->w / 2);
             if (warp_to_x < (ec->zone->x + 1))
               warp_to_x = ec->zone->x + ((ec->x + ec->w - ec->zone->x) / 2);
             else if (warp_to_x >= (ec->zone->x + ec->zone->w - 1))
               warp_to_x = (ec->zone->x + ec->zone->w + ec->x) / 2;

             int warp_to_y = ec->y + (ec->h / 2);
             if (warp_to_y < (ec->zone->y + 1))
               warp_to_y = ec->zone->y + ((ec->y + ec->h - ec->zone->y) / 2);
             else if (warp_to_y >= (ec->zone->y + ec->zone->h - 1))
               warp_to_y = (ec->zone->y + ec->zone->h + ec->y) / 2;

             ecore_evas_pointer_warp(e_comp->ee, warp_to_x, warp_to_y);
          }
        /* e_client_focus_set_with_pointer(ec); */
     }

   return 1;
}

static int
_plugins_init(const Evry_API *_api)
{
   Evry_Action *act;

   evry = _api;

   if (!evry->api_version_check(EVRY_API_VERSION))
     return EINA_FALSE;

   _plug = EVRY_PLUGIN_BASE(N_("Windows"), "preferences-system-windows",
                            EVRY_TYPE_CLIENT, _begin, _finish, _fetch);
   _plug->transient = EINA_TRUE;
   evry->plugin_register(_plug, EVRY_PLUGIN_SUBJECT, 2);

   act = EVRY_ACTION_NEW(N_("Switch to Window"),
                         EVRY_TYPE_CLIENT, 0, "go-next",
                         _act_border, _check_border);
   EVRY_ITEM_DATA_INT_SET(act, BORDER_SHOW);
   evry->action_register(act, 1);

   _actions = eina_list_append(_actions, act);

   act = EVRY_ACTION_NEW(N_("Iconify"),
                         EVRY_TYPE_CLIENT, 0, "go-down",
                         _act_border, _check_border);
   EVRY_ITEM_DATA_INT_SET(act, BORDER_HIDE);
   _actions = eina_list_append(_actions, act);
   evry->action_register(act, 2);

   act = EVRY_ACTION_NEW(N_("Toggle Fullscreen"),
                         EVRY_TYPE_CLIENT, 0, "view-fullscreen",
                         _act_border, _check_border);
   EVRY_ITEM_DATA_INT_SET(act, BORDER_FULLSCREEN);
   _actions = eina_list_append(_actions, act);
   evry->action_register(act, 4);

   act = EVRY_ACTION_NEW(N_("Close"),
                         EVRY_TYPE_CLIENT, 0, "list-remove",
                         _act_border, _check_border);
   EVRY_ITEM_DATA_INT_SET(act, BORDER_CLOSE);
   _actions = eina_list_append(_actions, act);
   evry->action_register(act, 3);

   act = EVRY_ACTION_NEW(N_("Send to Desktop"),
                         EVRY_TYPE_CLIENT, 0, "go-previous",
                         _act_border, _check_border);
   EVRY_ITEM_DATA_INT_SET(act, BORDER_TODESK);
   _actions = eina_list_append(_actions, act);
   evry->action_register(act, 3);

   return EINA_TRUE;
}

static void
_plugins_shutdown(void)
{
   Evry_Action *act;

   EVRY_PLUGIN_FREE(_plug);

   EINA_LIST_FREE (_actions, act)
     EVRY_ACTION_FREE(act);
}

/***************************************************************************/

Eina_Bool
evry_plug_windows_init(E_Module *m EINA_UNUSED)
{
   EVRY_MODULE_NEW(evry_module, evry, _plugins_init, _plugins_shutdown);

   return EINA_TRUE;
}

void
evry_plug_windows_shutdown(void)
{
   EVRY_MODULE_FREE(evry_module);
}

void
evry_plug_windows_save(void){}

