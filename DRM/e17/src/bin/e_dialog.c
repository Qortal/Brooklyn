#include "e.h"

/* local subsystem functions */
static void _e_dialog_free(E_Dialog *dia);
static void _e_dialog_del_func_cb(void *data, E_Dialog *dia);
static void _e_dialog_cb_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void _e_dialog_cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event);
static void _e_dialog_cb_wid_on_focus(void *data, Evas_Object *obj);

/* local subsystem globals */

/* externally accessible functions */

static E_Dialog *
_e_dialog_internal_new(Evas_Object *parent, const char *name, const char *class, int dialog)
{
   E_Dialog *dia;
   Evas_Object *o;
   Evas_Modifier_Mask mask;
   Eina_Bool kg;
   Evas *e;

   dia = E_OBJECT_ALLOC(E_Dialog, E_DIALOG_TYPE, _e_dialog_free);
   if (!dia) return NULL;
   if (dialog)
     dia->win = elm_win_add(parent, name, ELM_WIN_DIALOG_BASIC);
   else
     dia->win = elm_win_add(parent, name, ELM_WIN_BASIC);
   if (!dia->win)
     {
        free(dia);
        return NULL;
     }
   elm_win_modal_set(dia->win, !!parent);
   e = evas_object_evas_get(dia->win);
   evas_object_event_callback_add(dia->win, EVAS_CALLBACK_DEL, _e_dialog_cb_delete, dia);
   ecore_evas_name_class_set(ecore_evas_ecore_evas_get(e), name, class);
   dia->bg_object = o = elm_layout_add(dia->win);
   E_EXPAND(o);
   E_FILL(o);
   elm_win_resize_object_add(dia->win, o);
   e_theme_edje_object_set(o, "base/theme/dialog",
                           "e/widgets/dialog/main");
   elm_layout_signal_emit(dia->bg_object, "e,state,buttons,off", "e");
   evas_object_show(o);

   o = e_widget_list_add(evas_object_evas_get(dia->win), 1, 1);
   e_widget_on_focus_hook_set(o, _e_dialog_cb_wid_on_focus, dia);
   dia->box_object = o;
   elm_object_part_content_set(dia->bg_object, "e.swallow.buttons", o);
   elm_layout_signal_emit(dia->bg_object, "e,state,no_resizable", "e");

   o = evas_object_rectangle_add(e);
   dia->event_object = o;
   mask = 0;
   kg = evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Tab\" key events to object %p.\n", o);
   mask = evas_key_modifier_mask_get(evas_object_evas_get(dia->win), "Shift");
   kg = evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Tab\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "Return", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Return\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "Escape", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Escape\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "KP_Enter", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"KP_Enter\" key events to object %p.\n", o);

   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, _e_dialog_cb_key_down, dia);

   return dia;
}

E_API E_Dialog *
e_dialog_new(Evas_Object *parent, const char *name, const char *class)
{
   return _e_dialog_internal_new(parent, name, class, 1);
}

E_API E_Dialog *
e_dialog_normal_win_new(Evas_Object *parent, const char *name, const char *class)
{
   return _e_dialog_internal_new(parent, name, class, 0);
}

E_API void
e_dialog_button_add(E_Dialog *dia, const char *label, const char *icon, E_Dialog_Cb func, void *data)
{
   Evas_Object *o;

   if (!func) func = _e_dialog_del_func_cb;
   o = e_widget_button_add(evas_object_evas_get(dia->win), label, icon, (void (*)(void *, void *))func, data, dia);
   e_widget_list_object_append(dia->box_object, o, 1, 0, 0.5);
   if (!dia->buttons)
     elm_layout_signal_emit(dia->bg_object, "e,state,buttons,on", "e");
   dia->buttons = eina_list_append(dia->buttons, o);
}

E_API int
e_dialog_button_focus_num(E_Dialog *dia, int button)
{
   Evas_Object *o;

   o = eina_list_nth(dia->buttons, button);
   if (o) e_widget_focus_steal(o);
   return 1;
}

E_API int
e_dialog_button_disable_num_set(E_Dialog *dia, int button, int disabled)
{
   Evas_Object *o;

   o = eina_list_nth(dia->buttons, button);
   if (o) e_widget_disabled_set(o, disabled);
   return 1;
}

E_API int
e_dialog_button_disable_num_get(E_Dialog *dia, int button)
{
   Evas_Object *o;
   int ret = 0;

   o = eina_list_nth(dia->buttons, button);
   if (o) ret = e_widget_disabled_get(o);
   return ret;
}

E_API void
e_dialog_title_set(E_Dialog *dia, const char *title)
{
   elm_win_title_set(dia->win, title);
}

E_API void
e_dialog_text_set(E_Dialog *dia, const char *text)
{
   if (!dia->text_object)
     {
        Evas_Object *o;

        o = elm_layout_add(dia->win);
        dia->text_object = o;
        e_theme_edje_object_set(o, "base/theme/dialog",
                                "e/widgets/dialog/text");
        elm_object_part_content_set(dia->bg_object, "e.swallow.content", o);
        evas_object_show(o);
     }
   elm_object_part_text_set(dia->text_object, "e.textblock.message", text);
}

E_API void
e_dialog_icon_set(E_Dialog *dia, const char *icon, Evas_Coord size)
{
   if (!icon)
     {
        if (dia->icon_object)
          {
             evas_object_del(dia->icon_object);
             dia->icon_object = NULL;
          }
        elm_layout_signal_emit(dia->bg_object, "e,state,icon", "e");
        elm_layout_signal_emit(dia->bg_object, "e,icon,disabled", "e");
     }
   else
     {
        if (!dia->icon_object)
          {
             dia->icon_object = e_icon_add(evas_object_evas_get(dia->win));
             elm_object_part_content_set(dia->bg_object, "e.swallow.icon", dia->icon_object);
             evas_object_show(dia->icon_object);
          }
        if (!e_util_icon_theme_set(dia->icon_object, icon))
          e_icon_file_edje_set(dia->icon_object, icon, "icon");
        evas_object_size_hint_min_set(dia->icon_object, size * e_scale, size * e_scale);
        elm_layout_signal_emit(dia->bg_object, "e,state,icon", "e");
        elm_layout_signal_emit(dia->bg_object, "e,icon,enabled", "e");
     }
   edje_object_message_signal_process(elm_layout_edje_get(dia->bg_object));
}

E_API void
e_dialog_border_icon_set(E_Dialog *dia, const char *icon)
{
   E_Client *ec;

   ec = e_win_client_get(dia->win);
   if (ec)
     eina_stringshare_replace(&ec->internal_icon, icon);
}

E_API void
e_dialog_content_set(E_Dialog *dia, Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   dia->content_object = obj;
   e_widget_on_focus_hook_set(obj, _e_dialog_cb_wid_on_focus, dia);
   evas_object_size_hint_min_set(obj, minw, minh);
   elm_object_part_content_set(dia->bg_object, "e.swallow.content", obj);
   evas_object_show(obj);
}

E_API void
e_dialog_resizable_set(E_Dialog *dia, int resizable)
{
   dia->resizable = resizable;
   if (dia->win)
     {
        if (resizable)
          {
             E_EXPAND(dia->bg_object);
             e_util_win_auto_resize_fill(dia->win);
             elm_layout_signal_emit(dia->bg_object, "e,state,resizable", "e");
          }
        else
          {
             evas_object_resize(dia->win, dia->min_w, dia->min_h);
             evas_object_size_hint_weight_set(dia->bg_object, 0, 0);
             elm_layout_signal_emit(dia->bg_object, "e,state,no_resizable", "e");
          }
     }
}

E_API void
e_dialog_show(E_Dialog *dia)
{
   Evas_Coord mw, mh;
   Evas_Object *o;

   o = dia->text_object;
   if (o)
     elm_object_part_content_set(dia->bg_object, "e.swallow.content", o);

   edje_object_message_signal_process(elm_layout_edje_get(dia->bg_object));
   elm_layout_sizing_eval(dia->bg_object);
   evas_object_smart_calculate(dia->bg_object);
   evas_object_size_hint_min_get(dia->bg_object, &mw, &mh);
   dia->min_w = mw;
   dia->min_h = mh;

   evas_object_resize(dia->win, mw, mh);
   if (!dia->resizable)
     {
        evas_object_size_hint_weight_set(dia->bg_object, 0, 0);
        e_util_win_auto_resize_fill(dia->win);
     }
   else
     {
        E_EXPAND(dia->bg_object);
        e_util_win_auto_resize_fill(dia->win);
     }
   evas_object_show(dia->win);

   if (!e_widget_focus_get(dia->box_object))
     e_widget_focus_set(dia->box_object, 1);
   {
      E_Client *ec = e_win_client_get(dia->win);
      if (ec->focused && (e_config->pointer_slide || (!e_client_focus_policy_click(ec))))
        e_client_pointer_warp_to_center_now(ec);
   }
}

/* local subsystem functions */
static void
_e_dialog_free(E_Dialog *dia)
{
   if (dia->buttons) eina_list_free(dia->buttons);
   if (dia->text_object) evas_object_del(dia->text_object);
   if (dia->icon_object) evas_object_del(dia->icon_object);
   if (dia->box_object) evas_object_del(dia->box_object);
   if (dia->bg_object) evas_object_del(dia->bg_object);
   if (dia->content_object) evas_object_del(dia->content_object);
   if (dia->event_object) evas_object_del(dia->event_object);
   evas_object_del(dia->win);
   free(dia);
}

static void
_e_dialog_del_func_cb(void *data EINA_UNUSED, E_Dialog *dia)
{
   e_util_defer_object_del(E_OBJECT(dia));
}

static void
_e_dialog_cb_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Evas_Event_Key_Down *ev;
   E_Dialog *dia;

   ev = event;
   dia = data;
   if (!strcmp(ev->key, "Tab"))
     {
        if (evas_key_modifier_is_set(evas_key_modifier_get(evas_object_evas_get(dia->win)), "Shift"))
          {
             if (e_widget_focus_get(dia->box_object))
               {
                  if (!e_widget_focus_jump(dia->box_object, 0))
                    {
                       if (dia->text_object)
                         e_widget_focus_set(dia->box_object, 0);
                       else
                         {
                            e_widget_focus_set(dia->content_object, 0);
                            if (!e_widget_focus_get(dia->content_object))
                              e_widget_focus_set(dia->box_object, 0);
                         }
                    }
               }
             else
               {
                  if (!e_widget_focus_jump(dia->content_object, 0))
                    e_widget_focus_set(dia->box_object, 0);
               }
          }
        else
          {
             if (e_widget_focus_get(dia->box_object))
               {
                  if (!e_widget_focus_jump(dia->box_object, 1))
                    {
                       if (dia->text_object)
                         e_widget_focus_set(dia->box_object, 1);
                       else
                         {
                            e_widget_focus_set(dia->content_object, 1);
                            if (!e_widget_focus_get(dia->content_object))
                              e_widget_focus_set(dia->box_object, 1);
                         }
                    }
               }
             else
               {
                  if (!e_widget_focus_jump(dia->content_object, 1))
                    e_widget_focus_set(dia->box_object, 1);
               }
          }
     }
   else if (((!strcmp(ev->key, "Return")) ||
             (!strcmp(ev->key, "KP_Enter"))))
     {
        Evas_Object *o = NULL;

        if ((dia->content_object) && (e_widget_focus_get(dia->content_object)))
          o = e_widget_focused_object_get(dia->content_object);
        else
          o = e_widget_focused_object_get(dia->box_object);
        if (o) e_widget_activate(o);
     }
   else if (!strcmp(ev->key, "Escape"))
     e_object_del(E_OBJECT(dia));
}

static void
_e_dialog_cb_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_object_del(E_OBJECT(data));
}

static void
_e_dialog_cb_wid_on_focus(void *data, Evas_Object *obj)
{
   E_Dialog *dia;

   dia = data;
   if (obj == dia->content_object)
     e_widget_focused_object_clear(dia->box_object);
   else if (dia->content_object)
     e_widget_focused_object_clear(dia->content_object);
}

