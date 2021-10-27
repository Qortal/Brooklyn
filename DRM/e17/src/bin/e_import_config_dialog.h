#ifdef E_TYPEDEFS
typedef struct _E_Import_Config_Dialog E_Import_Config_Dialog;
#else
#ifndef E_IMPORT_CONFIG_DIALOG_H
#define E_IMPORT_CONFIG_DIALOG_H

#define E_IMPORT_CONFIG_DIALOG_TYPE 0xE0b01040
struct _E_Import_Config_Dialog
{
   E_Object             e_obj_inherit;
   Ecore_End_Cb         ok;
   Ecore_Cb             cancel;

   const char          *file;
   int                  method;
   int                  quality;
   E_Color              color;

   Ecore_Exe            *exe;
   Ecore_Event_Handler  *exe_handler;
   const char           *fdest;

   E_Dialog             *dia;
};

E_API E_Import_Config_Dialog *e_import_config_dialog_show(Evas_Object *parent, const char *path, Ecore_End_Cb ok, Ecore_Cb cancel);

#endif
#endif
