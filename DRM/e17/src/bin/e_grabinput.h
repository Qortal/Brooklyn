#ifdef E_TYPEDEFS

typedef enum _E_Focus_Method
{
   E_FOCUS_METHOD_NO_INPUT,
   E_FOCUS_METHOD_LOCALLY_ACTIVE,
   E_FOCUS_METHOD_GLOBALLY_ACTIVE,
   E_FOCUS_METHOD_PASSIVE
} E_Focus_Method;

#else
#ifndef E_GRABINPUT_H
#define E_GRABINPUT_H

EINTERN int         e_grabinput_init(void);
EINTERN int         e_grabinput_shutdown(void);
E_API void           e_grabinput_lost_cb_set(void (*cb) (void *data), void *data);
E_API int            e_grabinput_get(Ecore_Window mouse_win, int confine_mouse, Ecore_Window key_win);
E_API void           e_grabinput_release(Ecore_Window mouse_win, Ecore_Window key_win);
E_API void           e_grabinput_focus(Ecore_Window win, E_Focus_Method method);
E_API double         e_grabinput_last_focus_time_get(void);
E_API Ecore_Window e_grabinput_last_focus_win_get(void);
E_API Ecore_Window e_grabinput_key_win_get(void);
E_API Ecore_Window e_grabinput_mouse_win_get(void);

#endif
#endif
