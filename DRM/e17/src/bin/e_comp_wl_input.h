#ifdef E_TYPEDEFS
#else
# ifndef E_COMP_WL_INPUT_H
#  define E_COMP_WL_INPUT_H

E_API extern int E_EVENT_TEXT_INPUT_PANEL_VISIBILITY_CHANGE;

typedef struct _E_Event_Text_Input_Panel_Visibility_Change E_Event_Text_Input_Panel_Visibility_Change;

struct _E_Event_Text_Input_Panel_Visibility_Change
{
   Eina_Bool visible;
};

EINTERN Eina_Bool e_comp_wl_input_init(void);
EINTERN void e_comp_wl_input_shutdown(void);
EINTERN Eina_Bool e_comp_wl_input_pointer_check(struct wl_resource *res);
EINTERN Eina_Bool e_comp_wl_input_keyboard_check(struct wl_resource *res);
EINTERN Eina_Bool e_comp_wl_input_touch_check(struct wl_resource *res);

EINTERN void e_comp_wl_input_pointer_cursor_update(E_Comp_Wl_Pointer *ptr);

EINTERN Eina_Bool e_comp_wl_input_keyboard_modifiers_serialize(void);
EINTERN void e_comp_wl_input_keyboard_modifiers_update(void);
EINTERN void e_comp_wl_input_keyboard_state_update(uint32_t keycode, Eina_Bool pressed);
EINTERN void e_comp_wl_input_keyboard_enter_send(E_Client *client);

EINTERN void e_comp_wl_input_keyboard_modifers_clear(void);

E_API void e_comp_wl_input_pointer_enabled_set(Eina_Bool enabled);
E_API void e_comp_wl_input_keyboard_enabled_set(Eina_Bool enabled);
E_API void e_comp_wl_input_touch_enabled_set(Eina_Bool enabled);


E_API void e_comp_wl_input_keymap_index_set(xkb_layout_index_t index);
E_API void e_comp_wl_input_keymap_set(const char *rules, const char *model, const char *layout, const char *variant, const char *options);

E_API void e_comp_wl_input_keyboard_event_generate(const char *key, int mods, Eina_Bool up);

EINTERN int _e_comp_wl_input_keymap_fd_get(void);

# endif
#endif
