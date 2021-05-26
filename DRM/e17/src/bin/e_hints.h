#ifdef E_TYPEDEFS
#else
#ifndef E_HINTS_H
#define E_HINTS_H

EINTERN void e_hints_init(Ecore_Window win, Ecore_Window propwin);
//EINTERN void e_hints_manager_init(E_Manager *man);
E_API void e_hints_client_list_set(void);
E_API void e_hints_client_stacking_set(void);

E_API void e_hints_active_window_set(E_Client *ec);

EINTERN void e_hints_window_init(E_Client *ec);
E_API void e_hints_window_state_set(E_Client *ec);
E_API void e_hints_window_state_get(E_Client *ec);
E_API void e_hints_window_type_set(E_Client *ec);
E_API void e_hints_window_type_get(E_Client *ec);

E_API void e_hints_window_visible_set(E_Client *ec);
E_API void e_hints_window_iconic_set(E_Client *ec);
E_API void e_hints_window_hidden_set(E_Client *ec);

E_API void e_hints_window_shade_direction_set(E_Client *ec, E_Direction dir);
E_API E_Direction e_hints_window_shade_direction_get(E_Client *ec);

E_API void e_hints_window_size_set(E_Client *ec);
E_API void e_hints_window_size_unset(E_Client *ec);
E_API int  e_hints_window_size_get(E_Client *ec);

E_API void e_hints_window_shaded_set(E_Client *ec, int on);
E_API void e_hints_window_maximized_set(E_Client *ec, int horizontal, int vertical);
E_API void e_hints_window_fullscreen_set(E_Client *ec, int on);
E_API void e_hints_window_sticky_set(E_Client *ec, int on);
E_API void e_hints_window_stacking_set(E_Client *ec, E_Stacking stacking);
E_API void e_hints_window_desktop_set(E_Client *ec);

E_API void e_hints_window_e_state_set(E_Client *ec);
E_API void e_hints_window_e_state_get(E_Client *ec);

E_API void e_hints_window_qtopia_soft_menu_get(E_Client *ec);
E_API void e_hints_window_qtopia_soft_menus_get(E_Client *ec);

E_API void e_hints_window_steam_game_get(E_Client *ec);

E_API void e_hints_window_virtual_keyboard_state_get(E_Client *ec);
E_API void e_hints_window_virtual_keyboard_get(E_Client *ec);


E_API void e_hints_scale_update(void);


#ifdef E_COMP_X_H
E_API void e_hints_window_state_update(E_Client *ec, int state, int action);
extern E_API Ecore_X_Atom ATM__QTOPIA_SOFT_MENU;
extern E_API Ecore_X_Atom ATM__QTOPIA_SOFT_MENUS;
extern E_API Ecore_X_Atom ATM_GNOME_SM_PROXY;
extern E_API Ecore_X_Atom ATM_ENLIGHTENMENT_COMMS;
extern E_API Ecore_X_Atom ATM_ENLIGHTENMENT_VERSION;
extern E_API Ecore_X_Atom ATM_ENLIGHTENMENT_SCALE;
extern E_API Ecore_X_Atom ATM_NETWM_SHOW_WINDOW_MENU;
extern E_API Ecore_X_Atom ATM_NETWM_PERFORM_BUTTON_ACTION;
extern E_API Ecore_X_Atom ATM_GTK_FRAME_EXTENTS;
extern E_API Ecore_X_Atom ATM_STEAM_GAME;
#endif

#ifdef HAVE_WAYLAND
E_API const Eina_List * e_hints_aux_hint_supported_add(const char *hint);
E_API const Eina_List * e_hints_aux_hint_supported_del(const char *hint);
E_API const Eina_List * e_hints_aux_hint_supported_get(void);

E_API Eina_Bool e_hints_aux_hint_add(E_Client *ec, int32_t id, const char *name, const char *val);
E_API Eina_Bool e_hints_aux_hint_change(E_Client *ec, int32_t id, const char *val);
E_API Eina_Bool e_hints_aux_hint_add_fd(E_Client *ec, int32_t id, const char *name, int32_t fd);
E_API Eina_Bool e_hints_aux_hint_change_fd(E_Client *ec, int32_t id, int32_t fd);
E_API Eina_Bool e_hints_aux_hint_del(E_Client *ec, int32_t id);
E_API const char * e_hints_aux_hint_value_get(E_Client *ec, const char *name);

E_API Eina_Bool e_hints_aux_hint_add_with_pixmap(E_Pixmap *cp, int32_t id, const char *name, const char *val, int32_t fd);
E_API Eina_Bool e_hints_aux_hint_change_with_pixmap(E_Pixmap *cp, int32_t id, const char *val, int32_t fd);
E_API Eina_Bool e_hints_aux_hint_del_with_pixmap(E_Pixmap *cp, int32_t id);
E_API const char * e_hints_aux_hint_value_get_with_pixmap(E_Pixmap *cp, const char *name);
#endif
#endif
#endif
