#include "e.h"

#define TEXT_PRESS_KEY_SEQUENCE            _("Please press a key combination.<ps/><ps/>" \
                                             "Press <hilight>Escape</hilight> to abort.")
#define TEXT_PRESS_MOUSE_BINDING_SEQUENCE _("Please hold any modifier you want<ps/>"            \
                                             "and press any button on your mouse<ps/>" \
                                             "or roll a wheel to assign mouse binding.<ps/><ps/>"      \
                                             "Press <hilight>Escape</hilight> to abort.")
static Eina_Bool
_e_grab_dialog_key_handler(void *data, int type EINA_UNUSED, Ecore_Event_Key *ev)
{
   E_Grab_Dialog *eg = data;

   if (ev->window != eg->grab_win) return ECORE_CALLBACK_RENEW;
   if (!strcmp(ev->key, "Escape") &&
       !(ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT) &&
       !(ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) &&
       !(ev->modifiers & ECORE_EVENT_MODIFIER_ALT) &&
       !(ev->modifiers & ECORE_EVENT_MODIFIER_WIN))
     {
        e_object_del(data);
        return ECORE_CALLBACK_RENEW;
     }

   if (eg->key)
     {
        e_object_ref(data);
        eg->key(eg->data ? : eg, type, ev);
        e_object_unref(data);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_grab_dialog_wheel_handler(void *data, int type EINA_UNUSED, Ecore_Event_Mouse_Wheel *ev)
{
   E_Grab_Dialog *eg = data;

   if (ev->window != eg->grab_win) return ECORE_CALLBACK_RENEW;

   if (eg->wheel)
     {
        e_object_ref(data);
        eg->wheel(eg->data ? : eg, type, ev);
        e_object_unref(data);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_grab_dialog_mouse_handler(void *data, int type EINA_UNUSED, Ecore_Event_Mouse_Button *ev)
{
   E_Grab_Dialog *eg = data;

   if (ev->window != eg->grab_win) return ECORE_CALLBACK_RENEW;

   if (eg->mouse)
     {
        e_object_ref(data);
        eg->mouse(eg->data ? : eg, type, ev);
        e_object_unref(data);
     }
   else
     e_object_del(data);
   return ECORE_CALLBACK_RENEW;
}

static void
_e_grab_dialog_free(E_Grab_Dialog *eg)
{
   if (eg->grab_win)
     {
        e_grabinput_release(eg->grab_win, eg->grab_win);
#ifndef HAVE_WAYLAND_ONLY
        if (e_comp->comp_type == E_PIXMAP_TYPE_X)
          ecore_x_window_free(eg->grab_win);
        else
#endif
          e_comp_ungrab_input(1, 1);
     }
   E_FREE_LIST(eg->handlers, ecore_event_handler_del);
   e_bindings_disabled_set(0);

   e_object_del(E_OBJECT(eg->dia));
   free(eg);
}

static void
_e_grab_dialog_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_object_del(data);
}

static void
_e_grab_dialog_dia_del(void *data)
{
   E_Dialog *dia = data;

   e_object_del(dia->data);
}

E_API E_Grab_Dialog *
e_grab_dialog_show(Evas_Object *parent, Eina_Bool is_mouse, Ecore_Event_Handler_Cb key, Ecore_Event_Handler_Cb mouse, Ecore_Event_Handler_Cb wheel, const void *data)
{
   E_Grab_Dialog *eg;
   Ecore_Event_Handler *eh;

   if (parent)
     evas_object_focus_set(e_win_client_get(parent)->frame, 0);

   eg = E_OBJECT_ALLOC(E_Grab_Dialog, E_GRAB_DIALOG_TYPE, _e_grab_dialog_free);
   if (!eg) return NULL;

   if (is_mouse)
     {
        eg->dia = e_dialog_new(parent, "E", "_mousebind_getmouse_dialog");
        e_dialog_title_set(eg->dia, _("Mouse Binding Combination"));
        e_dialog_icon_set(eg->dia, "preferences-desktop-mouse", 48);
        e_dialog_text_set(eg->dia, TEXT_PRESS_MOUSE_BINDING_SEQUENCE);
     }
   else
     {
        eg->dia = e_dialog_new(parent, "E", "_keybind_getkey_dialog");
        e_dialog_title_set(eg->dia, _("Key Binding Combination"));
        e_dialog_icon_set(eg->dia, "preferences-desktop-keyboard-shortcuts", 48);
        e_dialog_text_set(eg->dia, TEXT_PRESS_KEY_SEQUENCE);
     }
   eg->dia->data = eg;
   elm_win_center(eg->dia->win, 1, 1);
   elm_win_borderless_set(eg->dia->win, 1);
   e_dialog_resizable_set(eg->dia, 0);
   e_object_del_attach_func_set(E_OBJECT(eg->dia), _e_grab_dialog_dia_del);
   evas_object_event_callback_add(eg->dia->win, EVAS_CALLBACK_DEL, _e_grab_dialog_delete, eg);

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        eg->grab_win = ecore_x_window_input_new(e_comp->root, 0, 0, 1, 1);
        ecore_x_window_show(eg->grab_win);
        e_grabinput_get(eg->grab_win, 0, eg->grab_win);
     }
   else
#endif
     {
        e_comp_grab_input(1, 1);
        eg->grab_win = e_comp->ee_win;
     }

   eg->key = key;
   eg->mouse = mouse;
   eg->wheel = wheel;
   eg->data = (void *)data;

   eh = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, (Ecore_Event_Handler_Cb)_e_grab_dialog_key_handler, eg);
   eg->handlers = eina_list_append(eg->handlers, eh);
   eh = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, (Ecore_Event_Handler_Cb)_e_grab_dialog_mouse_handler, eg);
   eg->handlers = eina_list_append(eg->handlers, eh);
   if (wheel)
     {
        eh = ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL, (Ecore_Event_Handler_Cb)_e_grab_dialog_wheel_handler, eg);
        eg->handlers = eina_list_append(eg->handlers, eh);
     }
   e_dialog_show(eg->dia);
   evas_object_layer_set(e_win_client_get(eg->dia->win)->frame, E_LAYER_CLIENT_PRIO);
   e_bindings_disabled_set(1);
   return eg;
}

