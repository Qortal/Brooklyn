#include "e_mod_main.h"

static Eina_Bool
_auth_cb_exit(void *data, int type EINA_UNUSED, void *event)
{
   Polkit_Session *ps = data;
   Ecore_Exe_Event_Del *ev = event;

   if (ev->pid != ps->auth_pid) return ECORE_CALLBACK_PASS_ON;
   ps->auth_pid = 0;
   if (ps->exe_exit_handler) ecore_event_handler_del(ps->exe_exit_handler);
   ps->exe_exit_handler = NULL;
   session_reply(ps);
   return ECORE_CALLBACK_PASS_ON;
}

static void
_cb_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj,
        void *event_info EINA_UNUSED)
{
   Polkit_Session *ps = evas_object_data_get(obj, "session");
   if (!ps) return;
   if (ps->exe_exit_handler)
     {
        ecore_event_handler_del(ps->exe_exit_handler);
        ps->exe_exit_handler = NULL;
     }
   if (ps->win)
     {
        ps->win = NULL;
        session_reply(ps);
     }
}

static void
_cb_ok(void *data EINA_UNUSED, Evas_Object *obj,
       void *event_info EINA_UNUSED)
{
   Polkit_Session *ps = evas_object_data_get(obj, "session");
   const char *str = elm_object_text_get(obj);

   if (!ps) return;
   if (ps->exe_exit_handler) return;
   elm_object_disabled_set(evas_object_data_get(obj, "label"), EINA_TRUE);
   elm_object_disabled_set(evas_object_data_get(obj, "label2"), EINA_TRUE);
   elm_object_disabled_set(evas_object_data_get(obj, "entry"), EINA_TRUE);
   e_dialog_button_disable_num_set(evas_object_data_get(obj, "dia"), 0, 1);
   e_dialog_button_disable_num_set(evas_object_data_get(obj, "dia"), 1, 1);
   ps->exe_exit_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                                  _auth_cb_exit, ps);
   if (str)
     {
        char *passwd = elm_entry_markup_to_utf8(str);
        if (passwd)
          {
             ps->auth_pid = e_auth_polkit_begin(passwd, ps->cookie, ps->target_uid);
             free(passwd);
             return;
          }
     }
   evas_object_del(ps->win);
}

static void
_cb_cancel(void *data EINA_UNUSED, Evas_Object *obj,
           void *event_info EINA_UNUSED)
{
   Polkit_Session *ps = evas_object_data_get(obj, "session");
   if (!ps) return;
   if (ps->exe_exit_handler) return;
   session_reply(ps);
}

static void
_cb_button_ok(void *data, E_Dialog *dia EINA_UNUSED)
{
   _cb_ok(NULL, data, NULL);
}

static void
_cb_button_cancel(void *data, E_Dialog *dia EINA_UNUSED)
{
   _cb_cancel(NULL, data, NULL);
}

void
auth_ui(Polkit_Session *ps)
{
   E_Dialog *dia;
   Evas_Object *o, *win, *box, *fr, *lab, *lab2, *ent;
   char buf[512];
   struct passwd *pass;

   dia = e_dialog_new(NULL, "E", "_polkit_auth");
   e_dialog_title_set(dia, _("Please enter password"));

   win = dia->win;

   if ((!ps->icon_name) || (!ps->icon_name[0]))
     e_dialog_icon_set(dia, "enlightenment", 64);
   else
     e_dialog_icon_set(dia, ps->icon_name, 64);

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cb_del, NULL);
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_data_set(win, "session", ps);

   box = o = elm_box_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, -1.0, -1.0);
   elm_box_horizontal_set(o, EINA_FALSE);
   e_dialog_content_set(dia, o, 0, 0);
   evas_object_show(o);

/* XXX: lookup action and display something useful for it in future.
   o = elm_label_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, 0.0, 0.0);
   elm_object_text_set(o, ps->action);
   elm_box_pack_end(box, o);
   evas_object_show(o);
 */

   fr = o = elm_frame_add(win);
   elm_object_style_set(o, "pad_medium");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, 0.0, 0.0);
   elm_box_pack_end(box, o);
   evas_object_show(o);

   lab = o = elm_label_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.0);
   elm_object_text_set(o, ps->message);
   elm_object_content_set(fr, o);
   evas_object_show(o);
   evas_object_data_set(win, "label", o);

   fr = o = elm_frame_add(win);
   elm_object_style_set(o, "pad_medium");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, 0.0, 0.0);
   elm_box_pack_end(box, o);
   evas_object_show(o);

   pass = getpwuid(ps->target_uid);
   if ((pass) && (pass->pw_name))
     snprintf(buf, sizeof(buf), _("Enter password for <b>%s</b>"), pass->pw_name);
   else
     snprintf(buf, sizeof(buf), _("Enter passowrd for UID %u"), ps->target_uid);
   lab2 = o = elm_label_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.0);
   elm_object_text_set(o, buf);
   elm_object_content_set(fr, o);
   evas_object_show(o);
   evas_object_data_set(win, "label2", o);

   fr = o = elm_frame_add(win);
   elm_object_style_set(o, "pad_medium");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 1.0);
   elm_box_pack_end(box, o);
   evas_object_show(o);

   ent = o = elm_entry_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.0);
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   elm_entry_password_set(o, EINA_TRUE);
   elm_object_part_text_set(o, "elm.guide", "Enter Password");
   evas_object_data_set(o, "session", ps);
   evas_object_data_set(o, "label", lab);
   evas_object_data_set(o, "label2", lab2);
   evas_object_data_set(o, "entry", ent);
   evas_object_data_set(o, "dia", dia);
   evas_object_smart_callback_add(o, "activated", _cb_ok, win);
   evas_object_smart_callback_add(o, "aborted", _cb_cancel, win);
   elm_object_content_set(fr, o);
   evas_object_show(o);
   evas_object_data_set(win, "entry", o);

   e_dialog_button_add(dia, _("OK"), NULL, _cb_button_ok, ent);
   e_dialog_button_add(dia, _("Cancel"), NULL, _cb_button_cancel, ent);
   e_dialog_button_focus_num(dia, 0);

   elm_object_focus_set(ent, EINA_TRUE);

   ps->win = win;
   ps->entry = ent;

   elm_win_center(win, 1, 1);
   e_dialog_show(dia);
   elm_win_activate(win);
}
