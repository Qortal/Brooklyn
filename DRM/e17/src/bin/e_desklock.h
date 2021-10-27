#ifdef E_TYPEDEFS

typedef struct _E_Event_Desklock E_Event_Desklock;
typedef Eina_Bool (*E_Desklock_Show_Cb)(void);
typedef void (*E_Desklock_Hide_Cb)(void);

typedef enum _E_Desklock_Background_Method {
    E_DESKLOCK_BACKGROUND_METHOD_THEME_DESKLOCK = 0,
    E_DESKLOCK_BACKGROUND_METHOD_THEME,
    E_DESKLOCK_BACKGROUND_METHOD_WALLPAPER,
    E_DESKLOCK_BACKGROUND_METHOD_CUSTOM,
} E_Desklock_Background_Method;

typedef enum
{
   E_DESKLOCK_AUTH_METHOD_SYSTEM = 0,
   E_DESKLOCK_AUTH_METHOD_PERSONAL = 1,
   E_DESKLOCK_AUTH_METHOD_PIN = 3,
} E_Desklock_Auth_Method;

typedef struct E_Desklock_Interface E_Desklock_Interface;

#else
#ifndef E_DESKLOCK_H
#define E_DESKLOCK_H

typedef Eina_Bool (*E_Desklock_Key_Cb)(Ecore_Event_Key*);

struct E_Desklock_Interface
{
   const char *name;
   E_Desklock_Show_Cb show;
   E_Desklock_Hide_Cb hide;
   E_Desklock_Key_Cb key_down;
   E_Desklock_Key_Cb key_up;
   Eina_Bool active E_BITFIELD; //interface is currently being used for locking
};

struct _E_Event_Desklock
{
   int on;
   int suspend;
};

EINTERN int e_desklock_init(void);
EINTERN int e_desklock_shutdown(void);

E_API Eina_Bool e_desklock_demo(void);
E_API int e_desklock_show(Eina_Bool suspend);
E_API int e_desklock_show_autolocked(void);
E_API int e_desklock_show_manual(Eina_Bool suspend);
E_API Eina_Bool e_desklock_manual_get(void);
E_API void e_desklock_hide(void);
E_API Eina_Bool e_desklock_state_get(void);

E_API void e_desklock_block(void);
E_API void e_desklock_unblock(void);

E_API void e_desklock_interface_append(E_Desklock_Interface *iface);
E_API void e_desklock_interface_remove(E_Desklock_Interface *iface);
EINTERN E_Desklock_Interface *e_desklock_interface_current_get(void);
E_API Eina_Stringshare *e_desklock_user_wallpaper_get(E_Zone *zone);
E_API void e_desklock_show_hook_add(E_Desklock_Show_Cb cb);
E_API void e_desklock_show_hook_del(E_Desklock_Show_Cb cb);
E_API void e_desklock_hide_hook_add(E_Desklock_Hide_Cb cb);
E_API void e_desklock_hide_hook_del(E_Desklock_Hide_Cb cb);

E_API void e_desklock_zone_block_set(const E_Zone *zone, Eina_Bool block);

extern E_API int E_EVENT_DESKLOCK;

static inline Eina_Bool
e_desklock_is_personal(void)
{
   return e_config->desklock_auth_method == E_DESKLOCK_AUTH_METHOD_PERSONAL;
}

static inline Eina_Bool
e_desklock_is_system(void)
{
   return e_config->desklock_auth_method == E_DESKLOCK_AUTH_METHOD_SYSTEM;
}

#endif
#endif
