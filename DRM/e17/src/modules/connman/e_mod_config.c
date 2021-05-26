#include "e_mod_main.h"

extern const char _e_connman_Name[];

struct _E_Config_Dialog_Data
{
   E_Connman_Module_Context *ctxt;
};

static Evas_Object *
_basic_create(E_Config_Dialog *dialog EINA_UNUSED, Evas *evas EINA_UNUSED,
              E_Config_Dialog_Data *cfdata EINA_UNUSED)
{
   return NULL;
}

static int
_basic_apply(E_Config_Dialog *dialog EINA_UNUSED,
             E_Config_Dialog_Data *cfdata EINA_UNUSED)
{
   return 1;
}

static void
_free_data(E_Config_Dialog      *dialog,
           E_Config_Dialog_Data *cfdata)
{
   E_Connman_Module_Context *ctxt = dialog->data;
   ctxt->conf_dialog = NULL;
   E_FREE(cfdata);
}

static inline void
_fill_data(E_Config_Dialog_Data *cfdata,
           E_Connman_Module_Context *ctxt)
{
   cfdata->ctxt = ctxt;
}

static void *
_create_data(E_Config_Dialog *dialog)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   if (!cfdata)
     return NULL;
   _fill_data(cfdata, dialog->data);
   return cfdata;
}

E_Config_Dialog *
e_connman_config_dialog_new(Evas_Object *parent EINA_UNUSED,
                            E_Connman_Module_Context *ctxt)
{
   E_Config_Dialog *dialog;
   E_Config_Dialog_View *view;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(!!ctxt->conf_dialog, ctxt->conf_dialog);

   view = E_NEW(E_Config_Dialog_View, 1);
   if (!view)
     return NULL;

   view->create_cfdata = _create_data;
   view->free_cfdata = _free_data;
   view->basic.create_widgets = _basic_create;
   view->basic.apply_cfdata = _basic_apply;

   dialog = e_config_dialog_new(NULL, _("Connection Manager"),
       _e_connman_Name, "e_connman_config_dialog_new",
       "preferences-network", 0, view, ctxt);

   return dialog;
}
