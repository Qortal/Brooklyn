#ifdef E_TYPEDEFS

//typedef struct _E_Screen E_Screen;

typedef enum _E_Backlight_Mode
{
   E_BACKLIGHT_MODE_NORMAL = 0,
   E_BACKLIGHT_MODE_OFF = 1,
   E_BACKLIGHT_MODE_MIN = 2,
   E_BACKLIGHT_MODE_DIM = 3,
   E_BACKLIGHT_MODE_MAX = 4
      // for the future. right now not working as we'd need an optical
      // sensor support framework
//   E_BACKLIGHT_MODE_AUTO = 5
} E_Backlight_Mode;

#else
#ifndef E_BACKLIGHT_H
#define E_BACKLIGHT_H

extern E_API int E_EVENT_BACKLIGHT_CHANGE;

//struct _E_Screen
//{
//   int screen, escreen;
//   int x, y, w, h;
//};

EINTERN int              e_backlight_init(void);
EINTERN int              e_backlight_shutdown(void);
E_API Eina_Bool           e_backlight_exists(void);
E_API void                e_backlight_update(void);
E_API void                e_backlight_level_set(E_Zone *zone, double val, double tim);
E_API double              e_backlight_level_get(E_Zone *zone);
E_API void                e_backlight_mode_set(E_Zone *zone, E_Backlight_Mode mode);
E_API E_Backlight_Mode    e_backlight_mode_get(E_Zone *zone);
E_API const Eina_List    *e_backlight_devices_get(void);
E_API void                e_backlight_suspend_set(Eina_Bool suspend);

#endif
#endif
