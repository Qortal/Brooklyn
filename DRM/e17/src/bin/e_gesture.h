#ifdef HAVE_ELPUT

#ifdef E_TYPEDEFS

typedef void (*E_Bindings_Swipe_Live_Update)(void *data, Eina_Bool end, double direction, double length, double error, unsigned int fingers);

#else
#ifndef E_GESTURES_H
#define E_GESTURES_H
E_API int e_gesture_init(void);
E_API int e_gesture_shutdown(void);
E_API void e_bindings_swipe_live_update_hook_set(E_Bindings_Swipe_Live_Update update, void *data);
E_API int e_bindings_gesture_capable_devices_get(void);
#endif
#endif

#endif
