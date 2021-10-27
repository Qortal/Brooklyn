#include "e_mod_main.h"

static Eldbus_Connection *pk_conn = NULL;
static Eldbus_Service_Interface *agent_iface = NULL;
static Eldbus_Object *pk_obj = NULL;
static Eldbus_Proxy *pk_proxy = NULL;

static Eldbus_Pending *pend_call = NULL;

static Eldbus_Object *ses_obj = NULL;
static Eldbus_Object *ses_obj2 = NULL;
static Eldbus_Proxy *ses_proxy = NULL;
static Eldbus_Proxy *ses_proxy2 = NULL;

static Eina_Bool     agent_request = EINA_FALSE;
static Eina_Bool     agent_ok = EINA_FALSE;
static const char   *session_path = NULL;
static const char   *session_id = NULL;
static unsigned int  session_uid = 0;
static const char   *session_user = NULL;

//////////////////////////////////////////////////////////////////////////////

static Eina_Hash *sessions = NULL;

static void
_session_free(Polkit_Session *ps)
{
   if (ps->reply) eldbus_connection_send(pk_conn, ps->reply, NULL, NULL, -1);
   ps->reply = NULL;
   if (ps->pend_reply) eldbus_pending_cancel(ps->pend_reply);
   ps->pend_reply = NULL;
   eina_stringshare_del(ps->cookie);
   ps->cookie = NULL;
   eina_stringshare_del(ps->message);
   ps->message = NULL;
   eina_stringshare_del(ps->icon_name);
   ps->icon_name = NULL;
   eina_stringshare_del(ps->action);
   ps->action = NULL;
   if (ps->win)
     {
        Evas_Object *win = ps->win;

        ps->win = NULL;
        evas_object_del(win);
     }
   free(ps);
}

static void
session_init(void)
{
   if (sessions) return;
   sessions = eina_hash_string_superfast_new((void *)_session_free);
}

static void
session_shutdown(void)
{
   if (sessions) eina_hash_free(sessions);
   sessions = NULL;
}

static Polkit_Session *
session_new(void)
{
   return calloc(1, sizeof(Polkit_Session));
}

static void
session_free(Polkit_Session *ps)
{
   eina_hash_del(sessions, ps->cookie, ps);
}

static void
session_register(Polkit_Session *ps)
{
   eina_hash_add(sessions, ps->cookie, ps);
}

static Polkit_Session *
session_find(const char *cookie)
{
   return eina_hash_find(sessions, cookie);
}

void
session_reply(Polkit_Session *ps)
{
   if (ps->reply)
     {
        ps->pend_reply = eldbus_connection_send(pk_conn, ps->reply, NULL, NULL, -1);
        ps->reply = NULL;
     }
   session_free(ps);
}

void
session_show(Polkit_Session *ps)
{
   auth_ui(ps);
   // display some auth dialog to enter a password, show ps->message
   // and ps->action specific ui ps->icon_name
   // when we get the password call
   // e_auth_polkit_begin(pass, ps->cookie, ps->target_uid);
   // when this returns call session_reply(ps);
}

//////////////////////////////////////////////////////////////////////////////

static void
iterate_dict(void *data, const void *key, Eldbus_Message_Iter *var)
{
   Polkit_Session *ps = data;
   const char *skey = key;

   if (!strcmp(skey, "uid"))
     {
        unsigned int uid = 0;

        if (eldbus_message_iter_arguments_get(var, "u", &uid))
          ps->target_uid = uid;
     }
}

static Eldbus_Message *
cb_agent_begin_authentication(const Eldbus_Service_Interface *iface EINA_UNUSED,
                              const Eldbus_Message *msg)
{
   // sssa{ss}sa(sa{sv})
   const char *action_id = NULL, *message = NULL, *icon_name = NULL,
     *cookie = NULL;
   Eldbus_Message_Iter *details = NULL, *ident = NULL, *item = NULL;
   Polkit_Session *ps, *ps2;

   ps = session_new();
   if (!ps) goto err;
   ps->reply = eldbus_message_method_return_new(msg);

   if (!eldbus_message_arguments_get(msg, "sssa{ss}sa(sa{sv})",
                                     &action_id, &message, &icon_name,
                                     &details, &cookie, &ident))
     goto err;
   ps->cookie = eina_stringshare_add(cookie);
   ps->message = eina_stringshare_add(message);
   ps->icon_name = eina_stringshare_add(icon_name);
   ps->action = eina_stringshare_add(action_id);
   // actions in: /usr/share/polkit-1/actions

/* XXX: Haven't seen details content yet - not sure what to do with it
   while (eldbus_message_iter_get_and_next(details, 'r', &item))
     {
        const char *v1, *v2;

        v1 = NULL;
        v2 = NULL;
        eldbus_message_iter_arguments_get(item, "ss", &v1, &v2);
     }
 */
   while (eldbus_message_iter_get_and_next(ident, 'r', &item))
     {
        const char *v1;
        Eldbus_Message_Iter *dict = NULL;

        v1 = NULL;
        eldbus_message_iter_arguments_get(item, "sa{sv}", &v1, &dict);
        if (!strcmp(v1, "unix-user"))
          eldbus_message_iter_dict_iterate(dict, "sv", iterate_dict, ps);
        else
          {
             printf("PK: Unhandled ident type.\n");
          }
     }
   ps2 = session_find(ps->cookie);
   if (ps2) session_free(ps2);
   session_register(ps);
   session_show(ps);
   return NULL;
err:
   if (ps) session_free(ps);
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
cb_agent_cancel_authentication(const Eldbus_Service_Interface *iface EINA_UNUSED,
                               const Eldbus_Message *msg)
{
   const char *cookie;
   Polkit_Session *ps;

   // s
   if (!eldbus_message_arguments_get(msg, "s", &cookie)) return NULL;
   ps = session_find(cookie);
   if (ps) session_free(ps);
   return eldbus_message_method_return_new(msg);
}

//////////////////////////////////////////////////////////////////////////////

static void
cb_register(void *data EINA_UNUSED, const Eldbus_Message *msg,
            Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;

   pend_call = NULL;
   if (eldbus_message_error_get(msg, &name, &text)) return;
   agent_request = EINA_FALSE;
   agent_ok = EINA_TRUE;
}

static const Eldbus_Method agent_methods[] = {
   { "BeginAuthentication",
      ELDBUS_ARGS({ "s",         "action_id" },
                    { "s",         "message" },
                    { "s",         "icon_name" },
                    { "a{ss}",     "details" },
                    { "s",         "cookie" },
                    { "a(sa{sv})", "identities" }),
      NULL,
      cb_agent_begin_authentication, 0
   },
   { "CancelAuthentication",
      ELDBUS_ARGS({ "s", "cookie" }),
      NULL,
      cb_agent_cancel_authentication, 0
   },
   { NULL, NULL, NULL, NULL, 0 }
};
static const Eldbus_Service_Interface_Desc agent_desc = {
   "org.freedesktop.PolicyKit1.AuthenticationAgent", agent_methods, NULL, NULL, NULL, NULL
};

static void
pk_agent_register(void)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *subj, *array, *dict, *vari;
   const char *locale = NULL;

   agent_request = EINA_TRUE;
   // set up agent interface
   agent_iface = eldbus_service_interface_register
     (pk_conn, "/org/enlightenment/polkit/Agent", &agent_desc);

   // register agent interface with polkit
   if (!locale) locale = getenv("LC_MESSAGES");
   if (!locale) locale = getenv("LC_ALL");
   if (!locale) locale = getenv("LANG");
   if (!locale) locale = getenv("LANGUAGE");
   if (!locale) locale = "C";

   pk_obj = eldbus_object_get(pk_conn, "org.freedesktop.PolicyKit1",
                              "/org/freedesktop/PolicyKit1/Authority");
   if (!pk_obj) return;
   pk_proxy = eldbus_proxy_get(pk_obj, "org.freedesktop.PolicyKit1.Authority");
   if (!pk_proxy) return;
   msg = eldbus_proxy_method_call_new(pk_proxy, "RegisterAuthenticationAgent");
   // (sa{sv})ss
   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(iter, "(sa{sv})", &subj);
    eldbus_message_iter_basic_append(subj, 's', "unix-session");
    eldbus_message_iter_arguments_append(subj, "a{sv}", &array);
     eldbus_message_iter_arguments_append(array, "{sv}", &dict);
      eldbus_message_iter_basic_append(dict, 's', "session-id");
      vari = eldbus_message_iter_container_new(dict, 'v', "s");
       eldbus_message_iter_basic_append(vari, 's', session_id);
      eldbus_message_iter_container_close(dict, vari);
     eldbus_message_iter_container_close(array, dict);
    eldbus_message_iter_container_close(subj, array);
   eldbus_message_iter_container_close(iter, subj);

   eldbus_message_iter_basic_append(iter, 's', locale);
   eldbus_message_iter_basic_append(iter, 's', "/org/enlightenment/polkit/Agent");
   pend_call = eldbus_proxy_send(pk_proxy, msg, cb_register, NULL, -1);
}

///////////////////////////////////////////////////////////////////////////////

static void
cb_login_prop_entry(void *data EINA_UNUSED, const void *key, Eldbus_Message_Iter *var)
{
   const char *skey = key;

   if (!strcmp(skey, "Id"))
     {
        const char *val = NULL;
        if (eldbus_message_iter_arguments_get(var, "s", &val))
          eina_stringshare_replace(&session_id, val);
     }
   else if (!strcmp(skey, "User"))
     {
        Eldbus_Message_Iter *iter = NULL;

        eldbus_message_iter_arguments_get(var, "(uo)", &iter);
        if (iter)
          {
             unsigned int uid = 0;
             const char *val = NULL;

             if (eldbus_message_iter_arguments_get(iter, "uo", &uid, &val))
               {
                  session_uid = uid;
                  eina_stringshare_replace(&session_user, val);
               }
          }
     }
}

static void
cb_login_prop(void *data EINA_UNUSED, const Eldbus_Message *msg,
              Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *array;

   pend_call = NULL;
   if (eldbus_message_error_get(msg, NULL, NULL)) return;
   if (eldbus_message_arguments_get(msg, "a{sv}", &array))
     {
        eldbus_message_iter_dict_iterate(array, "sv",
                                         cb_login_prop_entry, NULL);
        if ((session_id) && (session_user) && (session_path))
          pk_agent_register();
     }
   if (ses_proxy2) eldbus_proxy_unref(ses_proxy2);
   ses_proxy2 = NULL;
   if (ses_proxy) eldbus_proxy_unref(ses_proxy);
   ses_proxy = NULL;
   if (ses_obj) eldbus_object_unref(ses_obj);
   ses_obj = NULL;
   if (ses_obj2) eldbus_object_unref(ses_obj2);
   ses_obj2 = NULL;
}

static void
cb_login_session(void *data EINA_UNUSED, const Eldbus_Message *msg,
                 Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;
   const char *s;

   pend_call = NULL;
   if (eldbus_message_error_get(msg, &name, &text)) return;
   if (!eldbus_message_arguments_get(msg, "o", &s)) return;
   eina_stringshare_replace(&session_path, s);
   ses_obj2 = eldbus_object_get(pk_conn, "org.freedesktop.login1", s);
   if (!ses_obj2) return;
   ses_proxy2 = eldbus_proxy_get(ses_obj2, "org.freedesktop.login1.Session");
   if (!ses_proxy2) return;
   pend_call = eldbus_proxy_property_get_all(ses_proxy2, cb_login_prop, NULL);
}

static void
pk_session_init(void)
{
   ses_obj = eldbus_object_get(pk_conn, "org.freedesktop.login1",
                           "/org/freedesktop/login1");
   if (!ses_obj) return;
   ses_proxy = eldbus_proxy_get(ses_obj, "org.freedesktop.login1.Manager");
   if (!ses_proxy) return;
   pend_call = eldbus_proxy_call(ses_proxy, "GetSessionByPID",
                                 cb_login_session, NULL, -1,
                                 "u", (unsigned int)getpid());
}

/////////////////////////////////////////////////////////////////////////////

static Ecore_Timer *owner_gain_timer = NULL;

static Eina_Bool
cb_name_owner_new(void *data EINA_UNUSED)
{
   owner_gain_timer = NULL;
   pk_session_init();
   session_init();
   return EINA_FALSE;
}

static void
cb_name_owner_changed(void *data EINA_UNUSED,
                      const char *bus EINA_UNUSED,
                      const char *from EINA_UNUSED,
                      const char *to)
{
   static Eina_Bool first = EINA_TRUE;

   if (to[0])
     {
        if (owner_gain_timer) ecore_timer_del(owner_gain_timer);
        // on first start try and re-init quickly because we get a name
        // owner change even if all is good when we register to listen for it,
        // so start fast
        if (first)
          owner_gain_timer = ecore_timer_add(0.1, cb_name_owner_new, NULL);
        // but if we gegt a name owner change later it's probably because
        // bluez was restarted or crashed. a new bz daemon will (or should)
        // come up. so re-init more slowly here giving the daemon some time
        // to come up before pestering it.
        else
          owner_gain_timer = ecore_timer_add(1.0, cb_name_owner_new, NULL);
        first = EINA_FALSE;
     }
   else
     {
        session_shutdown();
        if (pend_call) eldbus_pending_cancel(pend_call);
        pend_call = NULL;
        if (agent_iface) eldbus_service_object_unregister(agent_iface);
        agent_iface = NULL;
        if (owner_gain_timer) ecore_timer_del(owner_gain_timer);
        owner_gain_timer = NULL;

        if (pk_proxy) eldbus_proxy_unref(pk_proxy);
        pk_proxy = NULL;
        if (pk_obj) eldbus_object_unref(pk_obj);
        pk_obj = NULL;

        if (pk_proxy) eldbus_proxy_unref(pk_proxy);
        pk_proxy = NULL;
        if (pk_obj) eldbus_object_unref(pk_obj);
        pk_obj = NULL;
        if (ses_proxy2) eldbus_proxy_unref(ses_proxy2);
        ses_proxy2 = NULL;
        if (ses_proxy) eldbus_proxy_unref(ses_proxy);
        ses_proxy = NULL;
        if (ses_obj) eldbus_object_unref(ses_obj);
        ses_obj = NULL;
        if (ses_obj2) eldbus_object_unref(ses_obj2);
        ses_obj2 = NULL;
        agent_request = EINA_FALSE;
        agent_ok = EINA_FALSE;
        eina_stringshare_replace(&session_path, NULL);
        eina_stringshare_replace(&session_id, NULL);
        eina_stringshare_replace(&session_user, NULL);
        session_uid = 0;
     }
}

void
e_mod_polkit_register(void)
{
   agent_request = EINA_FALSE;
   agent_ok = EINA_FALSE;
   pk_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (pk_conn)
     {
        eldbus_name_owner_changed_callback_add(pk_conn,
                                               "org.freedesktop.PolicyKit1",
                                               cb_name_owner_changed, NULL,
                                               EINA_TRUE);
     }
}

void
e_mod_polkit_unregister(void)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *subj, *array, *dict, *vari;

   if (!pk_conn) return;
   eldbus_name_owner_changed_callback_del(pk_conn,
                                          "org.freedesktop.PolicyKit1",
                                          cb_name_owner_changed, NULL);
   if (pend_call) eldbus_pending_cancel(pend_call);
   pend_call = NULL;

   if ((agent_request || agent_ok) && (session_id) && (pk_proxy))
     {
        msg = eldbus_proxy_method_call_new(pk_proxy,
                                           "UnregisterAuthenticationAgent");
        // (sa{sv})s
        iter = eldbus_message_iter_get(msg);
        eldbus_message_iter_arguments_append(iter, "(sa{sv})", &subj);
         eldbus_message_iter_basic_append(subj, 's', "unix-session");
         eldbus_message_iter_arguments_append(subj, "a{sv}", &array);
          eldbus_message_iter_arguments_append(array, "{sv}", &dict);
           eldbus_message_iter_basic_append(dict, 's', "session-id");
           vari = eldbus_message_iter_container_new(dict, 'v', "s");
            eldbus_message_iter_basic_append(vari, 's', session_id);
           eldbus_message_iter_container_close(dict, vari);
          eldbus_message_iter_container_close(array, dict);
         eldbus_message_iter_container_close(subj, array);
        eldbus_message_iter_container_close(iter, subj);
        eldbus_message_iter_basic_append(iter, 's', "/org/enlightenment/polkit/Agent");
        eldbus_proxy_send(pk_proxy, msg, NULL, NULL, -1);
     }

   session_shutdown();

   if (agent_iface) eldbus_service_object_unregister(agent_iface);
   agent_iface = NULL;
   if (owner_gain_timer) ecore_timer_del(owner_gain_timer);
   owner_gain_timer = NULL;

   if (pk_proxy) eldbus_proxy_unref(pk_proxy);
   pk_proxy = NULL;
   if (pk_obj) eldbus_object_unref(pk_obj);
   pk_obj = NULL;

   if (pk_proxy) eldbus_proxy_unref(pk_proxy);
   pk_proxy = NULL;
   if (pk_obj) eldbus_object_unref(pk_obj);
   pk_obj = NULL;
   if (ses_proxy2) eldbus_proxy_unref(ses_proxy2);
   ses_proxy2 = NULL;
   if (ses_proxy) eldbus_proxy_unref(ses_proxy);
   ses_proxy = NULL;
   if (ses_obj) eldbus_object_unref(ses_obj);
   ses_obj = NULL;
   if (ses_obj2) eldbus_object_unref(ses_obj2);
   ses_obj2 = NULL;

   eldbus_connection_unref(pk_conn);
   pk_conn = NULL;

   agent_request = EINA_FALSE;
   agent_ok = EINA_FALSE;
   eina_stringshare_replace(&session_path, NULL);
   eina_stringshare_replace(&session_id, NULL);
   eina_stringshare_replace(&session_user, NULL);
   session_uid = 0;
}
