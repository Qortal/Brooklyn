#include "e.h"

static void
_e_entry_dialog_free(E_Entry_Dialog *ed)
{
   e_object_del(E_OBJECT(ed->dia));
   E_FREE(ed->text);
   free(ed);
}

static void
_e_entry_dialog_delete(E_Dialog *dia, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_object_del(E_OBJECT(dia->data));
}

static void
_e_entry_dialog_del(void *data)
{
   E_Dialog *dia;

   dia = data;
   evas_object_event_callback_add(dia->win, EVAS_CALLBACK_DEL,
                                  (Evas_Object_Event_Cb)_e_entry_dialog_delete,
                                  dia);
   e_object_del(dia->data);
}

static void
_e_entry_dialog_ok(void *data, E_Dialog *dia EINA_UNUSED)
{
   E_Entry_Dialog *ed;

   ed = data;
   e_object_ref(E_OBJECT(ed));
   if (ed->ok.func) ed->ok.func(ed->ok.data, ed->text);
   e_object_del(E_OBJECT(ed));
   e_object_unref(E_OBJECT(ed));
}
static void
_e_entry_dialog_cancel(void *data, E_Dialog *dia EINA_UNUSED)
{
   E_Entry_Dialog *ed;

   ed = data;
   e_object_ref(E_OBJECT(ed));
   if (ed->cancel.func) ed->cancel.func(ed->cancel.data);
   e_object_del(E_OBJECT(ed));
   e_object_unref(E_OBJECT(ed));
}

static void
_e_entry_cb_key_down(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev;
   E_Entry_Dialog *ed;

   ev = event_info;
   if (!(ed = data)) return;
   if (!strcmp(ev->key, "Return"))
     _e_entry_dialog_ok(data, ed->dia);
   else if (!strcmp(ev->key, "Escape"))
     _e_entry_dialog_cancel(data, ed->dia);
}

E_API E_Entry_Dialog *
e_entry_dialog_show(Evas_Object *parent, const char *title, const char *icon, const char *text, const char *initial_text, const char *button_text, const char *button2_text, void (*ok_func) (void *data, char *text), void (*cancel_func) (void *data), void *data)
{
   E_Entry_Dialog *ed;
   E_Dialog *dia;
   Evas_Object *o, *obj;
   Evas_Modifier_Mask mask;
   Evas *e;
   int w, h;

   ed = E_OBJECT_ALLOC(E_Entry_Dialog, E_ENTRY_DIALOG_TYPE,
                       _e_entry_dialog_free);

   ed->ok.func = ok_func;
   ed->ok.data = data;
   ed->cancel.func = cancel_func;
   ed->cancel.data = data;

   if (initial_text) ed->text = strdup(initial_text);

   dia = e_dialog_new(parent, "E", "_entry_dialog");
   if (!dia)
     {
        e_object_del(E_OBJECT(ed));
        return NULL;
     }

   dia->data = ed;
   ed->dia = dia;

   mask = 0;
   evas_object_key_ungrab(dia->event_object, "space", mask, ~mask);
   e_object_del_attach_func_set(E_OBJECT(dia), _e_entry_dialog_del);
   evas_object_event_callback_add(dia->win, EVAS_CALLBACK_DEL,
                                  (Evas_Object_Event_Cb)_e_entry_dialog_delete,
                                  dia);

   if (title) e_dialog_title_set(dia, title);
   if (icon) e_dialog_icon_set(dia, icon, 64);

   e = evas_object_evas_get(dia->win);

   /* create main packing table */
   o = e_widget_table_add(e, 0);

   if (text)
     {
        obj = e_widget_label_add(e, text);
        e_widget_table_object_append(o, obj, 0, 0, 1, 1, 1, 1, 0, 0);
     }

   ed->entry = e_widget_entry_add(dia->win, &(ed->text), NULL, NULL, NULL);
   evas_object_smart_callback_add(ed->entry, "key_down",
                                  _e_entry_cb_key_down, ed);
   evas_object_size_hint_weight_set(ed->entry, EVAS_HINT_EXPAND, 0.5);

   e_widget_table_object_append(o, ed->entry, 1, 0, 1, 1, 1, 1, 1, 0);
   e_widget_size_min_get(o, &w, &h);
   e_dialog_content_set(dia, o, w, h);

   e_dialog_button_add(dia, !button_text ? _("OK") : button_text, NULL,
                       _e_entry_dialog_ok, ed);
   e_dialog_button_add(dia, !button2_text ? _("Cancel") : button2_text, NULL,
                       _e_entry_dialog_cancel, ed);

   elm_win_center(dia->win, 1, 1);
   e_dialog_resizable_set(dia, 1);
   e_dialog_show(dia);
   e_widget_focus_set(ed->entry, 1);
   e_widget_entry_select_all(ed->entry);

   return ed;
}
