/* Setup if we need connman? */
#include "e_wizard.h"
#include "e_wizard_api.h"

static void
_recommend_connman(E_Wizard_Page *pg EINA_UNUSED)
{
   Evas_Object *of, *ob;

   api->wizard_title_set(_("Network Management"));

   of = elm_frame_add(e_comp->elm);
   ob = elm_label_add(of);
   elm_object_content_set(of, ob);
#if defined(USE_MODULE_CONNMAN) || defined(USE_MODULE_WIRELESS)
   elm_object_text_set(of, _("Connman network service not found"));


   elm_object_text_set(ob, _("Install/Enable Connman service for network management support"));
#else
   elm_object_text_set(of, _("Connman and Wireless modules disabled"));
   elm_object_text_set(ob, _("Install one of these modules for network management support"));
#endif
   evas_object_show(ob);
   evas_object_show(of);

   api->wizard_page_show(of);
//   pg->data = o;

   api->wizard_button_next_enable_set(1);
}

static Eldbus_Connection *conn;
static Eldbus_Pending *pending_connman;
static Ecore_Timer *connman_timeout = NULL;

static Eina_Bool
_connman_fail(void *data)
{
   E_Wizard_Page *pg = data;
   E_Config_Module *em;
   Eina_List *l;

   EINA_LIST_FOREACH(e_config->modules, l, em)
     {
        if (!em->name) continue;
        if (!strcmp(em->name, "connman"))
          {
             e_config->modules = eina_list_remove_list
                 (e_config->modules, l);
             if (em->name) eina_stringshare_del(em->name);
             free(em);
             break;
          }
     }

   e_config_save_queue();

   connman_timeout = NULL;
   _recommend_connman(pg);
   return EINA_FALSE;
}

static Eina_Bool
_page_next_call(void *data EINA_UNUSED)
{
   api->wizard_next();
   return ECORE_CALLBACK_CANCEL;
}

static void
_check_connman_owner(void *data, const Eldbus_Message *msg,
                     Eldbus_Pending *pending EINA_UNUSED)
{
   const char *id;
   pending_connman = NULL;

   if (connman_timeout)
     {
        ecore_timer_del(connman_timeout);
        connman_timeout = NULL;
     }

   if (eldbus_message_error_get(msg, NULL, NULL))
     goto fail;

   if (!eldbus_message_arguments_get(msg, "s", &id))
     goto fail;

   if (id[0] != ':')
     goto fail;

   api->wizard_button_next_enable_set(1);
   ecore_idler_add(_page_next_call, NULL);
   return;

fail:
   _connman_fail(data);
}
/*

E_API int
wizard_page_init(E_Wizard_Page *pg EINA_UNUSED, Eina_Bool *need_xdg_desktops EINA_UNUSED, Eina_Bool *need_xdg_icons EINA_UNUSED)
{
   return 1;
}

E_API int
wizard_page_shutdown(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
E_API int
wizard_page_show(E_Wizard_Page *pg)
{
   Eina_Bool have_connman = EINA_FALSE;

#if defined(USE_MODULE_CONNMAN) || defined(USE_MODULE_WIRELESS)
   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
#endif
   if (conn)
     {
        if (pending_connman)
          eldbus_pending_cancel(pending_connman);

        pending_connman = eldbus_name_owner_get(conn, "net.connman",
                                               _check_connman_owner, pg);
        if (connman_timeout)
          ecore_timer_del(connman_timeout);
        connman_timeout = ecore_timer_loop_add(0.5, _connman_fail, pg);
        have_connman = EINA_TRUE;
        api->wizard_button_next_enable_set(0);
     }
   if (!have_connman)
     {
        E_Config_Module *em;
        Eina_List *l;
        EINA_LIST_FOREACH(e_config->modules, l, em)
          {
             if (!em->name) continue;
             if (!strcmp(em->name, "connman"))
               {
                  e_config->modules = eina_list_remove_list
                      (e_config->modules, l);
                  if (em->name) eina_stringshare_del(em->name);
                  free(em);
                  break;
               }
          }
        e_config_save_queue();
        _recommend_connman(pg);
     }
   api->wizard_title_set(_("Checking to see if Connman exists"));
   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   if (pending_connman)
     {
        eldbus_pending_cancel(pending_connman);
        pending_connman = NULL;
     }
   if (connman_timeout)
     {
        ecore_timer_del(connman_timeout);
        connman_timeout = NULL;
     }
   if (conn)
     eldbus_connection_unref(conn);
   conn = NULL;

   return 1;
}
/*
E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
