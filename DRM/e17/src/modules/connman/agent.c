#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "e.h"
#include "E_Connman.h"
#include "e_mod_main.h"
#define AGENT_IFACE "net.connman.Agent"
#define AGENT_KEY "agent"

typedef struct _E_Connman_Agent_Input E_Connman_Agent_Input;

struct Connman_Field
{
   const char *name;

   const char *type;
   const char *requirement;
   const char *value;
   Eina_Array *alternates;
};

struct _E_Connman_Agent_Input
{
   char *key;
   char *value;
   int show_password;
};

struct _E_Connman_Agent
{
   E_Dialog *dialog;
   Eldbus_Service_Interface *iface;
   Eldbus_Message *msg;
   Eldbus_Connection *conn;
   Eina_Bool canceled E_BITFIELD;
};

static void
_dict_append_basic(Eldbus_Message_Iter *array, const char *key, void *val)
{
   Eldbus_Message_Iter *dict, *variant;

   eldbus_message_iter_arguments_append(array, "{sv}", &dict);
   eldbus_message_iter_basic_append(dict, 's', key);
   variant = eldbus_message_iter_container_new(dict, 'v', "s");
   eldbus_message_iter_basic_append(variant, 's', val ?: "");
   eldbus_message_iter_container_close(dict, variant);
   eldbus_message_iter_container_close(array, dict);
}

static void
_dialog_ok_cb(void *data, E_Dialog *dialog)
{
   E_Connman_Agent *agent = data;
   E_Connman_Agent_Input *input;
   Evas_Object *toolbook, *list;
   Eldbus_Message_Iter *iter, *array;
   Eina_List *input_list, *l;
   Eldbus_Message *reply;

   toolbook = agent->dialog->content_object;

   /* fugly - no toolbook page get */
   list = evas_object_data_get(toolbook, "mandatory");
   if ((!list) || (!evas_object_visible_get(list)))
     {
        list = evas_object_data_get(toolbook, "alternate");
        if ((!list) || (!evas_object_visible_get(list)))
          {
             ERR("Couldn't get user input.");
             e_object_del(E_OBJECT(dialog));
             return;
          }
     }

   agent->canceled = EINA_FALSE;
   input_list = evas_object_data_get(list, "input_list");

   reply = eldbus_message_method_return_new(agent->msg);
   iter = eldbus_message_iter_get(reply);
   eldbus_message_iter_arguments_append(iter, "a{sv}", &array);

   EINA_LIST_FOREACH(input_list, l, input)
     _dict_append_basic(array, input->key, input->value);
   eldbus_message_iter_container_close(iter, array);

   eldbus_connection_send(agent->conn, reply, NULL, NULL, -1);

   e_object_del(E_OBJECT(dialog));
}

static void
_dialog_cancel_cb(void *data, E_Dialog *dialog)
{
   E_Connman_Agent *agent = data;
   agent->canceled = EINA_TRUE;
   e_object_del(E_OBJECT(dialog));
}

static void
_dialog_key_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED,
                    void *event)
{
   Evas_Event_Key_Down *ev = event;
   E_Connman_Agent *agent = data;

   if (!strcmp(ev->key, "Return"))
     _dialog_ok_cb(agent, agent->dialog);
   else if (strcmp(ev->key, "Escape") == 0)
     _dialog_cancel_cb(agent, agent->dialog);
}

static void
_dialog_cancel(E_Connman_Agent *agent)
{
   Eldbus_Message *reply;

   reply = eldbus_message_error_new(agent->msg,
                                  "net.connman.Agent.Error.Canceled",
                                  "User canceled dialog");
   eldbus_connection_send(agent->conn, reply, NULL, NULL, -1);
}

static void
_dialog_del_cb(void *data)
{
   E_Dialog *dialog = data;
   E_Connman_Agent *agent = e_object_data_get(E_OBJECT(dialog));

   if (agent->canceled)
     _dialog_cancel(agent);

   // FIXME need to mark cs->pending_connect = NULL;
   eldbus_message_unref(agent->msg);
   agent->msg = NULL;
   agent->dialog = NULL;
}

static void
_page_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Connman_Agent_Input *input;
   Eina_List *input_list;

   input_list = evas_object_data_get(obj, "input_list");
   EINA_LIST_FREE(input_list, input)
     {
        free(input->key);
        free(input);
     }
}

static void
_show_password_cb(void *data, Evas_Object *obj, void *event  EINA_UNUSED)
{
   Evas_Object *entry = data;
   int hidden;

   hidden = !e_widget_check_checked_get(obj);
   e_widget_entry_password_set(entry, hidden);
}

static void
_dialog_field_add(E_Connman_Agent *agent, struct Connman_Field *field)
{
   Evas_Object *toolbook, *list, *framelist, *entry, *check;
   E_Connman_Agent_Input *input;
   Eina_List *input_list;
   Eina_Bool mandatory;
   char header[64];
   Evas *evas;

   evas = evas_object_evas_get(agent->dialog->win);
   toolbook = agent->dialog->content_object;
   mandatory = !strcmp(field->requirement, "mandatory");

   if ((!mandatory) && (strcmp(field->requirement, "alternate")))
     {
        WRN("Field not handled: %s %s", field->name, field->type);
        return;
     }

   input = E_NEW(E_Connman_Agent_Input, 1);
   input->key = strdup(field->name);
   entry = e_widget_entry_add(agent->dialog->win, &(input->value), NULL, NULL, NULL);
   evas_object_show(entry);

   list = evas_object_data_get(toolbook, field->requirement);
   if (!list)
     {
        list = e_widget_list_add(evas, 0, 0);
        e_widget_toolbook_page_append(toolbook, NULL, field->name,
                                      list, 1, 1, 1, 1, 0.5, 0.0);
        evas_object_data_set(toolbook, field->requirement, list);

        e_widget_toolbook_page_show(toolbook, 0);
        evas_object_event_callback_add(list, EVAS_CALLBACK_DEL,
                                       _page_del, NULL);

        if (mandatory)
          e_widget_focus_set(entry, 1);
     }

   input_list = evas_object_data_get(list, "input_list");
   input_list = eina_list_append(input_list, input);
   evas_object_data_set(list, "input_list", input_list);

   snprintf(header, sizeof(header), "%s required to access network:",
            field->name);
   framelist = e_widget_framelist_add(evas, header, 0);
   evas_object_show(framelist);
   e_widget_list_object_append(list, framelist, 1, 1, 0.5);

   e_widget_framelist_object_append(framelist, entry);

   if ((!strcmp(field->name, "Passphrase")) ||
       (!strcmp(field->name, "Password")))
     {
        e_widget_entry_password_set(entry, 1);

        check = e_widget_check_add(evas, _("Show password"),
                                   &(input->show_password));
        evas_object_show(check);
        e_widget_framelist_object_append(framelist, check);

        evas_object_smart_callback_add(check, "changed", _show_password_cb,
                                       entry);
     }
   e_util_win_auto_resize_fill(agent->dialog->win);
}

static E_Dialog *
_dialog_new(E_Connman_Agent *agent)
{
   Evas_Object *toolbook;
   E_Dialog *dialog;
   Evas *evas;
   int mw, mh;

   dialog = e_dialog_new(NULL, "E", "connman_request_input");
   if (!dialog)
     return NULL;
   e_dialog_resizable_set(dialog, 1);

   e_dialog_title_set(dialog, _("Input requested"));
   e_dialog_border_icon_set(dialog, "dialog-ask");

   e_dialog_button_add(dialog, _("Ok"), NULL, _dialog_ok_cb, agent);
   e_dialog_button_add(dialog, _("Cancel"), NULL, _dialog_cancel_cb, agent);
   agent->canceled = EINA_TRUE; /* if win is closed it works like cancel */

   evas = evas_object_evas_get(dialog->win);

   toolbook = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);
   evas_object_show(toolbook);

   e_widget_size_min_get(toolbook, &mw, &mh);
   /* is it a hack ? */
   if (mw < 260)
     mw = 260;
   if (mh < 130)
     mh = 130;
   e_dialog_content_set(dialog, toolbook, mw, mh);
   e_dialog_show(dialog);

   evas_object_event_callback_add(dialog->bg_object, EVAS_CALLBACK_KEY_DOWN,
                                  _dialog_key_down_cb, agent);

   e_object_del_attach_func_set(E_OBJECT(dialog), _dialog_del_cb);
   e_object_data_set(E_OBJECT(dialog), agent);

   e_dialog_button_focus_num(dialog, 0);
   elm_win_center(dialog->win, 1, 1);

   return dialog;
}

static Eldbus_Message *
_agent_release(const Eldbus_Service_Interface *iface,
               const Eldbus_Message *msg)
{
   E_Connman_Agent *agent;
   Eldbus_Message *reply;

   DBG("Agent released");

   reply = eldbus_message_method_return_new(msg);

   agent = eldbus_service_object_data_get(iface, AGENT_KEY);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(agent, reply);

   if (agent->dialog)
     e_object_del(E_OBJECT(agent->dialog));

   return reply;
}

static Eldbus_Message *
_agent_report_error(const Eldbus_Service_Interface *iface EINA_UNUSED,
                    const Eldbus_Message *msg)
{
   const char *txt = NULL;

   if (eldbus_message_arguments_get(msg, "s", &txt))
     {
        e_util_dialog_show(_("Connman Error"), "%s", txt);
     }
   return NULL;
}

static Eldbus_Message *
_agent_request_browser(const Eldbus_Service_Interface *iface EINA_UNUSED,
                       const Eldbus_Message *msg)
{
   const char *url = NULL;

   if (eldbus_message_arguments_get(msg, "s", &url))
     {
        char buf[PATH_MAX * 2];

        snprintf(buf, sizeof(buf), "%s/enlightenment_open %s",
                 e_prefix_bin_get(), e_util_filename_escape(url));
        e_util_exe_safe_run(buf, NULL);
     }
   return NULL;
}

static Eina_Bool
_parse_field_value(struct Connman_Field *field, const char *key,
                   Eldbus_Message_Iter *value, const char *signature)
{
   if (!strcmp(key, "Type"))
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(signature[0] == 's', EINA_FALSE);
        eldbus_message_iter_basic_get(value, &field->type);
        return EINA_TRUE;
     }

   if (!strcmp(key, "Requirement"))
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(signature[0] == 's', EINA_FALSE);
        eldbus_message_iter_basic_get(value, &field->requirement);
        return EINA_TRUE;
     }

   if (!strcmp(key, "Alternates"))
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(signature[0] == 'a', EINA_FALSE);
        /* ignore alternates */
        return EINA_TRUE;
     }

   if (!strcmp(key, "Value"))
     {
        EINA_SAFETY_ON_FALSE_RETURN_VAL(signature[0] == 's', EINA_FALSE);
        eldbus_message_iter_basic_get(value, &field->value);
        return EINA_TRUE;
     }

   DBG("Ignored unknown argument: %s", key);
   return EINA_FALSE;
}

static Eina_Bool
_parse_field(struct Connman_Field *field, Eldbus_Message_Iter *value,
             const char *signature EINA_UNUSED)
{
   Eldbus_Message_Iter *array, *dict;

   eldbus_message_iter_arguments_get(value, "a{sv}", &array);
   EINA_SAFETY_ON_NULL_RETURN_VAL(array, EINA_FALSE);

   while (eldbus_message_iter_get_and_next(array, 'e', &dict))
     {
        Eldbus_Message_Iter *var;
        const char *key;
        char *sig2;

        if (!eldbus_message_iter_arguments_get(dict, "sv", &key, &var))
          return EINA_FALSE;
        sig2 = eldbus_message_iter_signature_get(var);
        if (!sig2)
          return EINA_FALSE;

        if (!_parse_field_value(field, key, var, sig2))
          {
             free(sig2);
             return EINA_FALSE;
          }
        free(sig2);
     }

   return EINA_TRUE;
}

static Eldbus_Message *
_agent_request_input(const Eldbus_Service_Interface *iface,
                     const Eldbus_Message *msg)
{
   E_Connman_Module_Context *ctxt = connman_mod->data;
   const Eina_List *l;
   E_Connman_Instance *inst;
   Eldbus_Message_Iter *array, *dict;
   E_Connman_Agent *agent;
   const char *path;

   agent = eldbus_service_object_data_get(iface, AGENT_KEY);

   /* Discard previous requests */
   // if msg is the current agent msg? eek.
   if (agent->msg == msg)
     {
        ERR("agent->msg == msg -> this should not be happening?");
        return NULL;
     }

   if (agent->msg) eldbus_message_unref(agent->msg);
   agent->msg = eldbus_message_ref((Eldbus_Message *)msg);

   EINA_LIST_FOREACH(ctxt->instances, l, inst)
     econnman_popup_del(inst);

   if (agent->dialog)
     e_object_del(E_OBJECT(agent->dialog));
   agent->dialog = _dialog_new(agent);
   EINA_SAFETY_ON_NULL_GOTO(agent->dialog, err);

   if (!eldbus_message_arguments_get(msg, "oa{sv}", &path, &array))
     goto err;

   while (eldbus_message_iter_get_and_next(array, 'e', &dict))
     {
        Eldbus_Message_Iter *var;
        char *signature;
        struct Connman_Field field = { NULL, NULL, NULL, NULL, NULL };

        if (!eldbus_message_iter_arguments_get(dict, "sv", &field.name, &var))
          goto err;
        signature = eldbus_message_iter_signature_get(var);
        if (!signature) goto err;

        if (!_parse_field(&field, var, signature))
          {
             free(signature);
	     goto err;
          }
        free(signature);

        DBG("AGENT Got field:\n"
            "\tName: %s\n"
            "\tType: %s\n"
            "\tRequirement: %s\n"
            "\tAlternates: (omit array)\n"
            "\tValue: %s",
            field.name, field.type, field.requirement, field.value);
        if (!field.requirement)
          {
             ERR("Field '%s' missing requirement string", field.name);
          }
        else if (!field.name)
          {
             ERR("Field missing name string");
          }
        else _dialog_field_add(agent, &field);
     }

   return NULL;

err:
   eldbus_message_unref((Eldbus_Message *)msg);
   agent->msg = NULL;
   WRN("Failed to parse msg");
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_agent_cancel(const Eldbus_Service_Interface *iface,
              const Eldbus_Message *msg)
{
   E_Connman_Agent *agent;
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);

   DBG("Agent canceled");

   agent = eldbus_service_object_data_get(iface, AGENT_KEY);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(agent, reply);

   if (agent->dialog)
     e_object_del(E_OBJECT(agent->dialog));

   return reply;
}

static const Eldbus_Method methods[] = {
   { "Release", NULL, NULL, _agent_release, 0 },
   {
    "ReportError", ELDBUS_ARGS({"o", "service"}, {"s", "error"}), NULL,
    _agent_report_error, 0
   },
   {
    "RequestBrowser", ELDBUS_ARGS({"o", "service"}, {"s", "url"}), NULL,
     _agent_request_browser, 0
   },
   {
    "RequestInput", ELDBUS_ARGS({"o", "service"}, {"a{sv}", "fields"}),
    ELDBUS_ARGS({"a{sv}", ""}), _agent_request_input, 0
   },
   { "Cancel", NULL, NULL, _agent_cancel, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc desc = {
   AGENT_IFACE, methods, NULL, NULL, NULL, NULL
};

E_Connman_Agent *
econnman_agent_new(Eldbus_Connection *eldbus_conn)
{
   Eldbus_Service_Interface *iface;
   E_Connman_Agent *agent;

   agent = E_NEW(E_Connman_Agent, 1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(agent, NULL);

   iface = eldbus_service_interface_register(eldbus_conn, AGENT_PATH, &desc);
   if (!iface)
     {
        ERR("Failed to create eldbus interface");
        free(agent);
        return NULL;
     }
   eldbus_service_object_data_set(iface, AGENT_KEY, agent);

   agent->iface = iface;
   agent->conn = eldbus_conn;

   return agent;
}

void
econnman_agent_del(E_Connman_Agent *agent)
{
   EINA_SAFETY_ON_NULL_RETURN(agent);
   if (agent->msg) eldbus_message_unref(agent->msg);
   agent->msg = NULL;
   eldbus_service_object_unregister(agent->iface);
   agent->iface = NULL;
   free(agent);
}
