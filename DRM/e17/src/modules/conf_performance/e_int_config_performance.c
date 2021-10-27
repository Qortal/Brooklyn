#include "e.h"

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   int priority;
};

E_Config_Dialog *
e_int_config_performance(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "advanced/performance")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;
   v->basic.check_changed = _basic_check_changed;

   cfd = e_config_dialog_new(NULL, _("Performance Settings"),
                             "E", "advanced/performance",
                             "preferences-system-performance", 0, v, NULL);
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   if (!cfdata) return NULL;
   cfdata->priority = e_config->priority;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_FREE(cfdata);
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   e_config->priority = cfdata->priority;
   ecore_exe_run_priority_set(e_config->priority);
   e_config_save_queue();
   return 1;
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   return ((e_config->priority != cfdata->priority));
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *ob, *ol;

   ol = e_widget_list_add(evas, 0, 0);

   ob = e_widget_label_add(evas, _("Application priority"));
   e_widget_list_object_append(ol, ob, 1, 1, 0.5);
   ob = e_widget_slider_add(evas, 1, 0, "%1.0f", 0, 19, 1, 0, NULL,
                            &(cfdata->priority), 100);
   e_widget_list_object_append(ol, ob, 1, 0, 0.5);

   return ol;
}
