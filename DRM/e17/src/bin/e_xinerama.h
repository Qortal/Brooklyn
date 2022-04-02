#ifdef E_TYPEDEFS

typedef struct _E_Screen E_Screen;

#else
#ifndef E_XINERAMA_H
#define E_XINERAMA_H

struct _E_Screen
{
   int screen, escreen;
   int x, y, w, h;
   int rotation;
   char *id; // this is the same id we get from randr2 so look it up there
};

EINTERN int           e_xinerama_init(void);
EINTERN int           e_xinerama_shutdown(void);
E_API void             e_xinerama_update(void);
E_API const Eina_List *e_xinerama_screens_get(void);
E_API const Eina_List *e_xinerama_screens_all_get(void);
E_API void e_xinerama_screens_set(Eina_List *screens);
E_API void             e_xinerama_fake_screen_add(int x, int y, int w, int h);
E_API Eina_Bool        e_xinerama_fake_screens_exist(void);

#endif
#endif
