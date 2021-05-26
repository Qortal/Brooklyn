#if 1
#ifdef E_TYPEDEFS

typedef struct _E_Desktop_Edit E_Desktop_Edit;

#else
#ifndef E_DESKTOP_EDIT_H
#define E_DESKTOP_EDIT_H

#define E_DESKTOP_EDIT_TYPE 0xE0b01019

struct _E_Desktop_Edit
{
   E_Object                     e_obj_inherit;

   Efreet_Desktop       *desktop;
   Evas        *evas;

   Evas_Object *img;
   Evas_Object *img_widget;
   Evas_Object *icon_fsel;
   E_Dialog    *icon_fsel_dia;
   Evas_Object *entry_widget_exec;
   Evas_Object *entry_widget_url;
   Evas_Object *exec_fsel;
   E_Dialog    *exec_fsel_dia;
   //int          img_set;

   char *tmp_image_path;
   int saved; /* whether desktop has been saved or not */

   E_Config_Dialog *cfd;
};

E_API Efreet_Desktop *e_desktop_client_create(E_Client *ec);
E_API E_Desktop_Edit *e_desktop_client_edit(E_Client *ec);
E_API E_Desktop_Edit *e_desktop_edit(Efreet_Desktop *desktop);

#endif
#endif
#endif
