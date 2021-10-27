#ifdef E_TYPEDEFS

typedef struct _E_Obj_Dialog E_About;

#else
#ifndef E_ABOUT_H
#define E_ABOUT_H

E_API E_About  *e_about_new         (void);
E_API void      e_about_show        (E_About *about);

#endif
#endif
