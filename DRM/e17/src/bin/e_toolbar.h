#ifdef E_TYPEDEFS

typedef struct _E_Toolbar E_Toolbar;

#else
#ifndef E_TOOLBAR_H
#define E_TOOLBAR_H

#define E_TOOLBAR_TYPE 0xE0b0101f

struct _E_Toolbar
{
   E_Object         e_obj_inherit;

   int              minw, minh, id;
   const char      *name;

   Evas            *evas;
   E_Gadcon        *gadcon;
   E_Menu          *menu;

   Evas_Object     *fwin;
   Evas_Object     *fm2;

   E_Config_Dialog *cfg_dlg;
   Evas_Object     *o_base, *o_event;
};

EINTERN int       e_toolbar_init(void);
EINTERN int       e_toolbar_shutdown(void);
E_API E_Toolbar   *e_toolbar_new(Evas *evas, const char *name, Evas_Object *fwin, Evas_Object *fm2);
E_API void         e_toolbar_fwin_set(E_Toolbar *tbar, Evas_Object *fwin);
E_API Evas_Object *e_toolbar_fwin_get(E_Toolbar *tbar);
E_API void         e_toolbar_fm2_set(E_Toolbar *tbar, Evas_Object *fm2);
E_API Evas_Object *e_toolbar_fm2_get(E_Toolbar *tbar);
E_API void         e_toolbar_orient(E_Toolbar *tbar, E_Gadcon_Orient orient);
E_API void         e_toolbar_populate(E_Toolbar *tbar);

#endif
#endif
