#ifdef E_TYPEDEFS

typedef enum _E_Config_Dialog_CFData_Type
{
   E_CONFIG_DIALOG_CFDATA_TYPE_BASIC,
   E_CONFIG_DIALOG_CFDATA_TYPE_ADVANCED,
   E_CONFIG_DIALOG_CFDATA_TYPE_NORMAL_WINDOW
} E_Config_Dialog_CFData_Type;

typedef struct _E_Config_Dialog      E_Config_Dialog;
typedef struct _E_Config_Dialog_View E_Config_Dialog_View;
typedef struct _E_Config_Dialog_Data E_Config_Dialog_Data;

#else
#ifndef E_CONFIG_DIALOG_H
#define E_CONFIG_DIALOG_H

#define E_CONFIG_DIALOG_TYPE 0xE0b01017

struct _E_Config_Dialog_View
{
   unsigned char override_auto_apply E_BITFIELD;
   unsigned char basic_only E_BITFIELD;
   unsigned char normal_win E_BITFIELD;
   unsigned char scroll E_BITFIELD;

   void           *(*create_cfdata)     (E_Config_Dialog *cfd);
   void            (*free_cfdata)       (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
   int             (*close_cfdata)      (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
   struct {
      int          (*apply_cfdata)      (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
      Evas_Object *(*create_widgets)    (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
      int          (*check_changed)     (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
   } basic, advanced;
};

struct _E_Config_Dialog
{
   E_Object                     e_obj_inherit;

   E_Config_Dialog_CFData_Type  view_type;
   E_Config_Dialog_View        *view;
   E_Config_Dialog_Data        *cfdata;
   Evas_Object                 *parent;
   const char                  *title;
   const char                  *icon;
   const char                  *name;
   const char                  *class;
   int                          icon_size;
   E_Dialog                    *dia;
   void                        *data;
   Ecore_Timer                 *auto_apply_timer;
   unsigned char                hide_buttons E_BITFIELD;
   unsigned char                cfg_changed E_BITFIELD;
   unsigned char                cfg_changed_auto E_BITFIELD;
};

E_API E_Config_Dialog *e_config_dialog_new(Evas_Object *parent, const char *title, const char *name, const char *class, const char *icon, int icon_size, E_Config_Dialog_View *view, void *data);
E_API int e_config_dialog_find(const char *name, const char *class);
E_API E_Config_Dialog *e_config_dialog_get(const char *name, const char *class);

E_API void e_config_dialog_changed_auto_set(E_Config_Dialog *cfd, unsigned char value);
E_API void e_config_dialog_changed_set(E_Config_Dialog *cfd, unsigned char value);

#endif
#endif
