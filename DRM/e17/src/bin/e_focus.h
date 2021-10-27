#ifdef E_TYPEDEFS
#else
#ifndef E_FOCUS_H
#define E_FOCUS_H

E_API void e_focus_event_mouse_in(E_Client* ec);
E_API void e_focus_event_mouse_out(E_Client* ec);
E_API void e_focus_event_mouse_down(E_Client* ec);
E_API void e_focus_event_mouse_up(E_Client* ec);
E_API void e_focus_event_focus_in(E_Client *ec);
E_API void e_focus_event_focus_out(E_Client *ec);
E_API void e_focus_setup(E_Client *ec);
E_API void e_focus_setdown(E_Client *ec);

#endif
#endif
