#define E_COMP_WL
#include "e.h"
#include "text-input-unstable-v1-server-protocol.h"
#include "input-method-unstable-v1-server-protocol.h"

typedef struct _E_Text_Input E_Text_Input;
typedef struct _E_Input_Method E_Input_Method;
typedef struct _E_Input_Method_Context E_Input_Method_Context;

struct _E_Text_Input
{
   struct wl_resource *resource;

   E_Client *ec;
   Eina_List *input_methods;
   Eina_Rectangle *cursor_rect;
   Eina_Bool input_panel_visible;
};

struct _E_Input_Method
{
   struct wl_resource *resource;

   E_Text_Input *model;
   E_Input_Method_Context *context;
   Eina_List *handlers;
};

struct _E_Input_Method_Context
{
   struct wl_resource *resource;

   E_Text_Input *model;
   E_Input_Method *input_method;

   struct
   {
      struct wl_resource *resource;
      Eina_List *handlers;
      Eina_Bool grabbed;
   } kbd;
};

static struct wl_global *text_input_manager_global;

static void
_e_text_input_event_visible_change_send(Eina_Bool visible)
{
   E_Event_Text_Input_Panel_Visibility_Change *ev;

   ev = E_NEW(E_Event_Text_Input_Panel_Visibility_Change, 1);
   ev->visible = visible;
   ecore_event_add(E_EVENT_TEXT_INPUT_PANEL_VISIBILITY_CHANGE, ev, NULL, NULL);
}

static void
_e_text_input_method_context_key_send(E_Input_Method_Context *context, unsigned int keycode, unsigned int timestamp, enum wl_keyboard_key_state state)
{
   uint32_t serial, nk;

   nk = keycode - 8;

   serial = wl_display_next_serial(e_comp->wl_comp_data->wl.disp);
   wl_keyboard_send_key(context->kbd.resource, serial, timestamp, nk, state);
}

static Eina_Bool
_e_text_input_method_context_ecore_cb_key_down(void *data, int ev_type EINA_UNUSED, Ecore_Event_Key *ev)
{
   E_Input_Method_Context *context = data;

   _e_text_input_method_context_key_send(context, ev->keycode, ev->timestamp,
                                         WL_KEYBOARD_KEY_STATE_PRESSED);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_text_input_method_context_ecore_cb_key_up(void *data, int ev_type EINA_UNUSED, Ecore_Event_Key *ev)
{
   E_Input_Method_Context *context = data;

   _e_text_input_method_context_key_send(context, ev->keycode, ev->timestamp,
                                         WL_KEYBOARD_KEY_STATE_RELEASED);

   return ECORE_CALLBACK_RENEW;
}

static void
_e_text_input_method_context_grab_set(E_Input_Method_Context *context, Eina_Bool set)
{
   if (set == context->kbd.grabbed)
     return;

   context->kbd.grabbed = set;

   if (set)
     {
        E_LIST_HANDLER_APPEND(context->kbd.handlers, ECORE_EVENT_KEY_DOWN,
                              _e_text_input_method_context_ecore_cb_key_down,
                              context);
        E_LIST_HANDLER_APPEND(context->kbd.handlers, ECORE_EVENT_KEY_UP,
                              _e_text_input_method_context_ecore_cb_key_up,
                              context);

        e_comp_grab_input(0, 1);
     }
   else
     {
        E_FREE_LIST(context->kbd.handlers, ecore_event_handler_del);
        e_comp_ungrab_input(0, 1);
     }
}

static void
_e_text_input_deactivate(E_Text_Input *text_input, E_Input_Method *input_method)
{
   if (input_method->model == text_input)
     {
        if ((input_method->context) && (input_method->resource))
          {
             if (input_method->context)
               _e_text_input_method_context_grab_set(input_method->context,
                                                     EINA_FALSE);

             zwp_input_method_v1_send_deactivate(input_method->resource,
                                             input_method->context->resource);
          }

        input_method->model = NULL;
        input_method->context = NULL;

        text_input->input_methods = eina_list_remove(text_input->input_methods, input_method);

        zwp_text_input_v1_send_leave(text_input->resource);
     }
}

static void
_e_text_input_method_context_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static void
_e_text_input_method_context_cb_string_commit(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial, const char *text)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_commit_string(context->model->resource,
                                      serial, text);
}

static void
_e_text_input_method_context_cb_preedit_string(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial, const char *text, const char *commit)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_preedit_string(context->model->resource,
                                       serial, text, commit);
}

static void
_e_text_input_method_context_cb_preedit_styling(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t index, uint32_t length, uint32_t style)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_preedit_styling(context->model->resource,
                                        index, length, style);
}

static void
_e_text_input_method_context_cb_preedit_cursor(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t cursor)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_preedit_cursor(context->model->resource,
                                       cursor);
}

static void
_e_text_input_method_context_cb_surrounding_text_delete(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t index, uint32_t length)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_delete_surrounding_text(context->model->resource,
                                                index, length);
}

static void
_e_text_input_method_context_cb_cursor_position(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t index, int32_t anchor)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_cursor_position(context->model->resource,
                                        index, anchor);
}

static void
_e_text_input_method_context_cb_modifiers_map(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_array *map)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_modifiers_map(context->model->resource, map);
}

static void
_e_text_input_method_context_cb_keysym(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_keysym(context->model->resource,
                               serial, time, sym, state, modifiers);
}

static void
_e_text_input_method_context_keyboard_unbind(struct wl_resource *resource)
{
   E_Input_Method_Context *context;

   context = wl_resource_get_user_data(resource);

   _e_text_input_method_context_grab_set(context, EINA_FALSE);

   context->kbd.resource = NULL;
}

static void
_e_text_input_method_context_cb_keyboard_grab(struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
   E_Input_Method_Context *context;
   struct wl_resource *new_resource;

   DBG("Input Method Context - grab keyboard %d", wl_resource_get_id(resource));

   context = wl_resource_get_user_data(resource);
   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   new_resource = wl_resource_create(client, &wl_keyboard_interface, 1, id);
   if (!new_resource)
     {
        wl_resource_post_no_memory(resource);
        return;
     }

   wl_resource_set_implementation(new_resource, NULL, context,
                                  _e_text_input_method_context_keyboard_unbind);

   e_comp_wl_input_keymap_send(new_resource);

   context->kbd.resource = new_resource;

   _e_text_input_method_context_grab_set(context, EINA_TRUE);
}

static void
_e_text_input_method_context_cb_key(struct wl_client *client EINA_UNUSED, struct wl_resource *resource EINA_UNUSED, uint32_t serial EINA_UNUSED, uint32_t time EINA_UNUSED, uint32_t key EINA_UNUSED, uint32_t state_w EINA_UNUSED)
{
   DBG("Input Method Context - key %d", wl_resource_get_id(resource));
}

static void
_e_text_input_method_context_cb_modifiers(struct wl_client *client EINA_UNUSED, struct wl_resource *resource EINA_UNUSED, uint32_t serial EINA_UNUSED, uint32_t mods_depressed EINA_UNUSED, uint32_t mods_latched EINA_UNUSED, uint32_t mods_locked EINA_UNUSED, uint32_t group EINA_UNUSED)
{
   DBG("Input Method Context - modifiers %d", wl_resource_get_id(resource));
}

static void
_e_text_input_method_context_cb_language(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial, const char *language)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_language(context->model->resource,
                                 serial, language);
}

static void
_e_text_input_method_context_cb_text_direction(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial, uint32_t direction)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->model)
     zwp_text_input_v1_send_text_direction(context->model->resource,
                                       serial, direction);
}

static const struct zwp_input_method_context_v1_interface _e_text_input_method_context_implementation = {
     _e_text_input_method_context_cb_destroy,
     _e_text_input_method_context_cb_string_commit,
     _e_text_input_method_context_cb_preedit_string,
     _e_text_input_method_context_cb_preedit_styling,
     _e_text_input_method_context_cb_preedit_cursor,
     _e_text_input_method_context_cb_surrounding_text_delete,
     _e_text_input_method_context_cb_cursor_position,
     _e_text_input_method_context_cb_modifiers_map,
     _e_text_input_method_context_cb_keysym,
     _e_text_input_method_context_cb_keyboard_grab,
     _e_text_input_method_context_cb_key,
     _e_text_input_method_context_cb_modifiers,
     _e_text_input_method_context_cb_language,
     _e_text_input_method_context_cb_text_direction
};

static void
_e_text_input_method_context_cb_resource_destroy(struct wl_resource *resource)
{
   E_Input_Method_Context *context = wl_resource_get_user_data(resource);

   if (!context)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method Context For Resource");
        return;
     }

   if (context->kbd.resource)
     wl_resource_destroy(context->kbd.resource);

   if ((context->input_method) &&
       (context->input_method->context == context))
     context->input_method->context = NULL;

   free(context);
}

static Eina_Bool
_e_text_input_cb_event_client_focus_in(void *data, int type EINA_UNUSED, void *event)
{
   E_Input_Method *input_method = data;
   E_Event_Client *ev = event;

   if (!input_method->model)
     return ECORE_CALLBACK_RENEW;

   if ((!ev->ec) ||
       (input_method->model->ec != ev->ec))
     {
        _e_text_input_deactivate(input_method->model, input_method);
        _e_text_input_event_visible_change_send(EINA_FALSE);
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_e_text_input_cb_activate(struct wl_client *client, struct wl_resource *resource, struct wl_resource *seat, struct wl_resource *surface)
{
   E_Text_Input *text_input;
   E_Input_Method *input_method;
   E_Text_Input *old;
   E_Input_Method_Context *context;

   text_input = wl_resource_get_user_data(resource);
   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   input_method =
      wl_resource_get_user_data(e_comp->wl_comp_data->seat.im.resource);
   if (!input_method)
     {
        wl_resource_post_error(seat,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method For Seat");
        return;
     }

   old = input_method->model;
   if (old == text_input)
     return;

   if (old)
     _e_text_input_deactivate(old, input_method);

   input_method->model = text_input;
   text_input->input_methods =
      eina_list_append(text_input->input_methods, input_method);

   text_input->ec = wl_resource_get_user_data(surface);

   if (input_method->resource)
     {
        context = E_NEW(E_Input_Method_Context, 1);
        if (!context)
          {
             wl_client_post_no_memory(client);
             ERR("Could not allocate space for Input_Method_Context");
             return;
          }

        context->resource =
           wl_resource_create(wl_resource_get_client(input_method->resource),
                              &zwp_input_method_context_v1_interface, 1, 0);
        wl_resource_set_implementation(context->resource,
                                       &_e_text_input_method_context_implementation,
                                       context, _e_text_input_method_context_cb_resource_destroy);

        context->model = text_input;
        context->input_method = input_method;
        input_method->context = context;

        zwp_input_method_v1_send_activate(input_method->resource, context->resource);
     }

   if (text_input->input_panel_visible)
     _e_text_input_event_visible_change_send(EINA_TRUE);

   zwp_text_input_v1_send_enter(text_input->resource, surface);
}

static void
_e_text_input_cb_deactivate(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *seat)
{
   E_Text_Input *text_input;
   E_Input_Method *input_method;

   text_input = wl_resource_get_user_data(resource);
   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   input_method =
      wl_resource_get_user_data(e_comp->wl_comp_data->seat.im.resource);
   if (!input_method)
     {
        wl_resource_post_error(seat,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method For Seat");
        return;
     }

   _e_text_input_deactivate(text_input, input_method);
   _e_text_input_event_visible_change_send(EINA_FALSE);

}

static void
_e_text_input_cb_input_panel_show(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   text_input->input_panel_visible = 1;

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (input_method->model == text_input)
          _e_text_input_event_visible_change_send(EINA_TRUE);
     }
}

static void
_e_text_input_cb_input_panel_hide(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   text_input->input_panel_visible = 0;

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (input_method->model == text_input)
          _e_text_input_event_visible_change_send(EINA_FALSE);
     }
}

static void
_e_text_input_cb_reset(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (!input_method->context) continue;
        zwp_input_method_context_v1_send_reset(input_method->context->resource);
     }
}

static void
_e_text_input_cb_surrounding_text_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *text, uint32_t cursor, uint32_t anchor)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (!input_method->context) continue;
        zwp_input_method_context_v1_send_surrounding_text(input_method->context->resource,
                                                      text, cursor, anchor);
     }
}

static void
_e_text_input_cb_content_type_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t hint, uint32_t purpose)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (!input_method->context) continue;
        zwp_input_method_context_v1_send_content_type(input_method->context->resource,
                                                  hint, purpose);
     }
}

static void
_e_text_input_cb_cursor_rectangle_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   text_input->cursor_rect = eina_rectangle_new(x, y, width, height);

   // TODO: issue event update input_panel
}

static void
_e_text_input_cb_preferred_language_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, const char *language)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (!input_method->context) continue;
        zwp_input_method_context_v1_send_preferred_language(input_method->context->resource,
                                                        language);
     }
}

static void
_e_text_input_cb_state_commit(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t serial)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (!input_method->context) continue;
        zwp_input_method_context_v1_send_commit_state(input_method->context->resource, serial);
     }
}

static void
_e_text_input_cb_action_invoke(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, uint32_t button, uint32_t index)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;
   Eina_List *l;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   EINA_LIST_FOREACH(text_input->input_methods, l, input_method)
     {
        if (!input_method->context) continue;
        zwp_input_method_context_v1_send_invoke_action(input_method->context->resource,
                                                   button, index);
     }
}

static const struct zwp_text_input_v1_interface _e_text_input_implementation = {
     _e_text_input_cb_activate,
     _e_text_input_cb_deactivate,
     _e_text_input_cb_input_panel_show,
     _e_text_input_cb_input_panel_hide,
     _e_text_input_cb_reset,
     _e_text_input_cb_surrounding_text_set,
     _e_text_input_cb_content_type_set,
     _e_text_input_cb_cursor_rectangle_set,
     _e_text_input_cb_preferred_language_set,
     _e_text_input_cb_state_commit,
     _e_text_input_cb_action_invoke
};

static void
_e_text_input_cb_destroy(struct wl_resource *resource)
{
   E_Text_Input *text_input = wl_resource_get_user_data(resource);
   E_Input_Method *input_method;

   if (!text_input)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Text Input For Resource");
        return;
     }

   EINA_LIST_FREE(text_input->input_methods, input_method)
      _e_text_input_deactivate(text_input, input_method);

   eina_rectangle_free(text_input->cursor_rect);
   free(text_input);
}

static void
_e_text_input_manager_cb_text_input_create(struct wl_client *client, struct wl_resource *resource EINA_UNUSED, uint32_t id)
{
   E_Text_Input *text_input;

   text_input = E_NEW(E_Text_Input, 1);
   if (!text_input)
     {
        wl_client_post_no_memory(client);
        ERR("Could not allocate space for Text_Input");
        return;
     }

   text_input->resource = wl_resource_create(client,
                                             &zwp_text_input_v1_interface,
                                             1, id);
   if (!text_input->resource)
     {
        free(text_input);
        wl_client_post_no_memory(client);
        ERR("could not create wl_resource for text input");
        return;
     }

   wl_resource_set_implementation(text_input->resource,
                                  &_e_text_input_implementation,
                                  text_input, _e_text_input_cb_destroy);
}

static const struct zwp_text_input_manager_v1_interface _e_text_input_manager_implementation = {
     _e_text_input_manager_cb_text_input_create
};

static void
_e_text_cb_bind_text_input_manager(struct wl_client *client, void *data EINA_UNUSED, uint32_t version EINA_UNUSED, uint32_t id)
{
   struct wl_resource *resource;

   resource = wl_resource_create(client, &zwp_text_input_manager_v1_interface, 1, id);
   if (!resource)
     {
        wl_client_post_no_memory(client);
        ERR("could not create resource for text input manager");
        return;
     }

   wl_resource_set_implementation(resource, &_e_text_input_manager_implementation, NULL, NULL);
}

static void
_e_text_input_method_cb_unbind(struct wl_resource *resource)
{
   E_Input_Method *input_method;

   e_comp->wl_comp_data->seat.im.resource = NULL;

   input_method = wl_resource_get_user_data(resource);
   if (!input_method)
     {
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "No Input Method For Resource");
        return;
     }

   if (input_method->model)
     _e_text_input_deactivate(input_method->model, input_method);

   E_FREE_FUNC(input_method->handlers, ecore_event_handler_del);

   input_method->resource = NULL;
   input_method->context = NULL;

   free(input_method);
}

static void
_e_text_cb_bind_input_method(struct wl_client *client, void *data EINA_UNUSED, uint32_t version EINA_UNUSED, uint32_t id)
{
   E_Input_Method *input_method;
   struct wl_resource *resource;
   pid_t pid;

   resource = wl_resource_create(client, &zwp_input_method_v1_interface, 1, id);
   if (!resource)
     {
        wl_client_post_no_memory(client);
        ERR("could not create wl_resource for input method");
        return;
     }

   if (e_comp->wl_comp_data->seat.im.resource)
     {
        wl_resource_post_error(resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "interface object already bound");
        wl_resource_destroy(resource);
        return;
     }

   wl_client_get_credentials(client, &pid, NULL, NULL);
   if (pid != getpid())
     {
        ERR("Permission to bind input method denied");
        wl_resource_post_error(resource,
                               WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "permission to bind input_method denied");
        wl_resource_destroy(resource);
        return;
     }

   input_method = E_NEW(E_Input_Method, 1);
   if (!input_method)
     {
        wl_client_post_no_memory(client);
        wl_resource_destroy(resource);
        ERR("Could not allocate space for Input_Method");
        return;
     }

   wl_resource_set_implementation(resource, NULL, input_method,
                                  _e_text_input_method_cb_unbind);

   input_method->model = NULL;
   input_method->context = NULL;
   input_method->resource = resource;

   e_comp->wl_comp_data->seat.im.resource = resource;

   E_LIST_HANDLER_APPEND(input_method->handlers, E_EVENT_CLIENT_FOCUS_IN,
                         _e_text_input_cb_event_client_focus_in, input_method);
}

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Wl_Text_Input" };

E_API void *
e_modapi_init(E_Module *m)
{
   // FIXME: create only one input method object per seat.
   e_comp->wl_comp_data->seat.im.global =
      wl_global_create(e_comp->wl_comp_data->wl.disp, &zwp_input_method_v1_interface, 1,
                       NULL, _e_text_cb_bind_input_method);
   if (!e_comp->wl_comp_data->seat.im.global)
     {
        ERR("failed to create wl_global for input method");
        return NULL;
     }

   text_input_manager_global =
      wl_global_create(e_comp->wl_comp_data->wl.disp, &zwp_text_input_manager_v1_interface, 1,
                       NULL, _e_text_cb_bind_text_input_manager);
   if (!text_input_manager_global)
     {
        ERR("failed to create wl_global for text input manager");
        E_FREE_FUNC(e_comp->wl_comp_data->seat.im.global, wl_global_destroy);
        return NULL;
     }

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   E_FREE_FUNC(e_comp->wl_comp_data->seat.im.global, wl_global_destroy);
   E_FREE_FUNC(text_input_manager_global, wl_global_destroy);

   return 1;
}
