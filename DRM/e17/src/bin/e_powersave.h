#ifdef E_TYPEDEFS

typedef enum _E_Powersave_Mode
{
   E_POWERSAVE_MODE_NONE, // no powersaving
   E_POWERSAVE_MODE_LOW, // normal power management - AC plugged in
   E_POWERSAVE_MODE_MEDIUM, // lots of battery available but not on AC
   E_POWERSAVE_MODE_HIGH, // have enough battery but good to save powwr
   E_POWERSAVE_MODE_EXTREME, // low on battery save more
   E_POWERSAVE_MODE_FREEZE // really try and do as little as possible
} E_Powersave_Mode;

typedef struct _E_Powersave_Deferred_Action E_Powersave_Deferred_Action;
typedef struct _E_Event_Powersave_Update E_Event_Powersave_Update;
typedef struct _E_Powersave_Sleeper E_Powersave_Sleeper;

#else
#ifndef E_POWERSAVE_H
#define E_POWERSAVE_H

extern E_API int E_EVENT_POWERSAVE_UPDATE;
extern E_API int E_EVENT_POWERSAVE_CONFIG_UPDATE;

struct _E_Event_Powersave_Update
{
   E_Powersave_Mode mode;
};

EINTERN int e_powersave_init(void);
EINTERN int e_powersave_shutdown(void);

// do this thing somw time later - how much later depends on power save mode
// this is done in the mainloop
E_API E_Powersave_Deferred_Action *e_powersave_deferred_action_add(void (*func) (void *data), const void *data);
E_API void                         e_powersave_deferred_action_del(E_Powersave_Deferred_Action *pa);
E_API void                         e_powersave_mode_set(E_Powersave_Mode mode);
E_API E_Powersave_Mode             e_powersave_mode_get(void);
// force is used by e's system core to use "connected standby" where the system
// should be aslpee but not regular suspend - soits still alive but it needs
// to try and be as idle as possible
E_API void                         e_powersave_mode_force(E_Powersave_Mode mode);
E_API void                         e_powersave_mode_unforce(void);
// used by screen saving/blanking to set a specific power mode when screens
// are off/not visible
E_API void                         e_powersave_mode_screen_set(E_Powersave_Mode mode);
E_API void                         e_powersave_mode_screen_unset(void);
E_API E_Powersave_Mode             e_powersave_mode_screen_get(void);

E_API void                         e_powersave_defer_suspend(void);
E_API void                         e_powersave_defer_hibernate(void);
E_API void                         e_powersave_defer_cancel(void);

// sleepers are meant to be used in threads polling for things. create in
// mainloop then use and sleep in a thread, 
E_API E_Powersave_Sleeper         *e_powersave_sleeper_new(void);
E_API void                         e_powersave_sleeper_free(E_Powersave_Sleeper *sleeper);
// the below function is INTENDED to be called from a thread
E_API void                         e_powersave_sleeper_sleep(E_Powersave_Sleeper *sleeper, int poll_interval, Eina_Bool allow_save);

/* FIXME: in the powersave system add things like pre-loading entire files
 * int memory for pre-caching to avoid disk spinup, when in an appropriate
 * powersave mode */

/* FIXME: in powersave mode also add the ability to reduce framerate when
 * at a particular powersave mode */

/* FIXME: in powersave mode also reduce ecore timer precision, so timers
 * will be dispatched together and system will wakeup less. */

#endif
#endif
