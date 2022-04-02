#include "e_mod_main.h"

static int _log_dom = -1;
#undef DBG
#undef WARN
#undef INF
#undef ERR
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

#define E_MSGBUS_WIN_ACTION_CB_PROTO(NAME) \
  static Eldbus_Message * _e_msgbus_window_##NAME##_cb(const Eldbus_Service_Interface * iface, const Eldbus_Message * msg)

E_MSGBUS_WIN_ACTION_CB_PROTO(list);
E_MSGBUS_WIN_ACTION_CB_PROTO(close);
E_MSGBUS_WIN_ACTION_CB_PROTO(kill);
E_MSGBUS_WIN_ACTION_CB_PROTO(focus);
E_MSGBUS_WIN_ACTION_CB_PROTO(iconify);
E_MSGBUS_WIN_ACTION_CB_PROTO(uniconify);
E_MSGBUS_WIN_ACTION_CB_PROTO(maximize);
E_MSGBUS_WIN_ACTION_CB_PROTO(unmaximize);
E_MSGBUS_WIN_ACTION_CB_PROTO(sendtodesktop);

static const Eldbus_Method window_methods[] = {
   { "List", NULL, ELDBUS_ARGS({"a(si)", "array_of_window"}), _e_msgbus_window_list_cb, 0 },
   { "Close", ELDBUS_ARGS({"i", "window_id"}), NULL, _e_msgbus_window_close_cb, 0 },
   { "Kill", ELDBUS_ARGS({"i", "window_id"}), NULL, _e_msgbus_window_kill_cb, 0 },
   { "Focus", ELDBUS_ARGS({"i", "window_id"}), NULL, _e_msgbus_window_focus_cb, 0 },
   { "Iconify", ELDBUS_ARGS({"i", "window_id"}), NULL, _e_msgbus_window_iconify_cb, 0 },
   { "Uniconify", ELDBUS_ARGS({"i", "window_id"}), NULL, _e_msgbus_window_uniconify_cb, 0 },
   { "Maximize", ELDBUS_ARGS({"i", "window_id"}), NULL, _e_msgbus_window_maximize_cb, 0 },
   { "Unmaximize", ELDBUS_ARGS({"i", "window_id"}), NULL, _e_msgbus_window_unmaximize_cb, 0 },
   { "SendToDesktop", ELDBUS_ARGS({"i","window_id"},{"i","zone"},{"i","desk_x"},{"i","desk_y"}), NULL, _e_msgbus_window_sendtodesktop_cb, 0 },
   { NULL, NULL, NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc window = {
   "org.enlightenment.wm.Window", window_methods, NULL, NULL, NULL, NULL
};

/* Window handlers */
static Eldbus_Message *
_e_msgbus_window_list_cb(const Eldbus_Service_Interface *iface EINA_UNUSED,
                         const Eldbus_Message *msg)
{
   const Eina_List *l;
   E_Client *ec;
   Eldbus_Message *reply;
   Eldbus_Message_Iter *main_iter, *array;

   reply = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(reply, NULL);

   main_iter = eldbus_message_iter_get(reply);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(main_iter, reply);

   eldbus_message_iter_arguments_append(main_iter, "a(si)", &array);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(array, reply);

   EINA_LIST_FOREACH(e_comp->clients, l, ec)
     {
        Eldbus_Message_Iter *s;

        if (e_client_util_ignored_get(ec)) continue;

        eldbus_message_iter_arguments_append(array, "(si)", &s);
        if (!s) continue;
        eldbus_message_iter_arguments_append(s, "si", ec->icccm.name,
                                            e_client_util_win_get(ec));
        eldbus_message_iter_container_close(array, s);
     }
   eldbus_message_iter_container_close(main_iter, array);

   return reply;
}

#define E_MSGBUS_WIN_ACTION_CB_BEGIN(NAME) \
   static Eldbus_Message * \
   _e_msgbus_window_##NAME##_cb(const Eldbus_Service_Interface *iface EINA_UNUSED, \
                                const Eldbus_Message *msg) { \
      E_Client *ec; \
      int xwin; \
      if (!eldbus_message_arguments_get(msg, "i", &xwin)) \
        return eldbus_message_method_return_new(msg); \
      ec = e_pixmap_find_client(E_PIXMAP_TYPE_X, xwin); \
      if (ec) {
#define E_MSGBUS_WIN_ACTION_CB_END \
      } \
      return eldbus_message_method_return_new(msg); \
   }

E_MSGBUS_WIN_ACTION_CB_BEGIN(close)
  e_client_act_close_begin(ec);
E_MSGBUS_WIN_ACTION_CB_END

E_MSGBUS_WIN_ACTION_CB_BEGIN(kill)
  e_client_act_kill_begin(ec);
E_MSGBUS_WIN_ACTION_CB_END

E_MSGBUS_WIN_ACTION_CB_BEGIN(focus)
  e_client_activate(ec, 1);
E_MSGBUS_WIN_ACTION_CB_END

E_MSGBUS_WIN_ACTION_CB_BEGIN(iconify)
  e_client_iconify(ec);
E_MSGBUS_WIN_ACTION_CB_END

E_MSGBUS_WIN_ACTION_CB_BEGIN(uniconify)
  e_client_uniconify(ec);
E_MSGBUS_WIN_ACTION_CB_END

E_MSGBUS_WIN_ACTION_CB_BEGIN(maximize)
  e_client_maximize(ec, e_config->maximize_policy);
E_MSGBUS_WIN_ACTION_CB_END

E_MSGBUS_WIN_ACTION_CB_BEGIN(unmaximize)
  e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
E_MSGBUS_WIN_ACTION_CB_END

static Eldbus_Message *
_e_msgbus_window_sendtodesktop_cb( const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   E_Client *ec;
   E_Zone * zone;
   E_Desk * desk;
   Eina_List *l = NULL;
   int xwin, zonenum, xdesk, ydesk;

   if (!eldbus_message_arguments_get(msg, "iiii", &xwin, &zonenum, &xdesk, &ydesk))
     return eldbus_message_method_return_new(msg);

   ec = e_pixmap_find_client(E_PIXMAP_TYPE_X, xwin);

   if (ec)
     {
        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          {
             if ((int)zone->num == zonenum)
               {
                  if (xdesk < zone->desk_x_count && ydesk < zone->desk_y_count)
                    {
                       E_Desk *old_desk = ec->desk;
                       Eina_Bool was_focused = e_client_stack_focused_get(ec);

                       desk = e_desk_at_xy_get(zone, xdesk, ydesk);
                       if ((desk) && (desk != old_desk))
                         {
                            e_client_desk_set(ec, desk);
                            if (was_focused)
                              e_desk_last_focused_focus(old_desk);
                         }
                    }
               }
          }
     }

   return eldbus_message_method_return_new(msg);

}


void msgbus_window_init(Eina_Array *ifaces)
{
   Eldbus_Service_Interface *iface;

   if (_log_dom == -1)
     {
        _log_dom = eina_log_domain_register("msgbus_window", EINA_COLOR_BLUE);
        if (_log_dom < 0)
          EINA_LOG_ERR("could not register msgbus_window log domain!");
     }

   iface = e_msgbus_interface_attach(&window);
   if (iface) eina_array_push(ifaces, iface);
}
