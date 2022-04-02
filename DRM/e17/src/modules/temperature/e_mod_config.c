#include "e.h"
#include "e_mod_main.h"

#define FAR_2_CEL(x) ((x - 32) / 9.0) * 5.0
#define CEL_2_FAR(x) (x * 9.0 / 5.0) + 32

struct _E_Config_Dialog_Data
{
   struct {
      int interval;
   } poll;

   int unit_method;
   struct {
      int low, high;
   } temp;

   int sensor;
   Eina_List *sensors;

   Evas_Object *o_high, *o_low;

   Config_Face *inst;
};

/* local function prototypes */
static void *_create_data(E_Config_Dialog *cfd);
static void _fill_data_tempget(E_Config_Dialog_Data *cfdata);
static void _free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _cb_display_changed(void *data, Evas_Object *obj EINA_UNUSED);

void
config_temperature_module(Config_Face *inst)
{
   E_Config_Dialog_View *v;
   char buff[PATH_MAX];

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   snprintf(buff, sizeof(buff),
            "%s/e-module-temperature.edj", inst->module->dir);
   inst->config_dialog =
     e_config_dialog_new(NULL,
                         _("Temperature Settings"), "E",
                         "_e_mod_temperature_config_dialog", buff, 0, v, inst);
}

/* local function prototypes */
static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->inst = cfd->data;
   _fill_data_tempget(cfdata);
   return cfdata;
}

static void
_fill_data_tempget(E_Config_Dialog_Data *cfdata)
{
   cfdata->unit_method = cfdata->inst->units;
   cfdata->poll.interval = cfdata->inst->poll_interval;
   cfdata->temp.low = cfdata->inst->low;
   cfdata->temp.high = cfdata->inst->high;
   cfdata->sensor = 0;

   Eina_List *sensors;
   Sensor *sen;
   int n;

   sensors = temperature_tempget_sensor_list();
   n = 0;
   EINA_LIST_FREE(sensors, sen)
     {
        if ((cfdata->inst->sensor_name) &&
            (!strcmp(sen->name, cfdata->inst->sensor_name)))
          cfdata->sensor = n;
        cfdata->sensors = eina_list_append(cfdata->sensors, sen);
        n++;
     }
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Sensor *sen;

   cfdata->inst->config_dialog = NULL;
   EINA_LIST_FREE(cfdata->sensors, sen)
     {
        eina_stringshare_replace(&(sen->name), NULL);
        eina_stringshare_replace(&(sen->label), NULL);
        free(sen);
     }
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *otb, *ol, *ow;
   E_Radio_Group *rg;

   e_dialog_resizable_set(cfd->dia, 1);

   otb = e_widget_toolbook_add(evas, 24, 24);

   if (cfdata->sensors)
     {
        Sensor *sen;
        Eina_List *l;
        int n = 0;

        ol = e_widget_list_add(evas, 0, 0);
        rg = e_widget_radio_group_new(&(cfdata->sensor));
        EINA_LIST_FOREACH(cfdata->sensors, l, sen)
          {
             ow = e_widget_radio_add(evas, sen->label, n, rg);
             e_widget_list_object_append(ol, ow, 1, 0, 0.5);
             n++;
          }
        e_widget_toolbook_page_append(otb, NULL, _("Sensors"), ol,
                                      1, 1, 1, 0, 0.0, 0.0);
     }

   ol = e_widget_list_add(evas, 0, 0);
   rg = e_widget_radio_group_new(&(cfdata->unit_method));
   ow = e_widget_radio_add(evas, _("Celsius"), CELSIUS, rg);
   e_widget_on_change_hook_set(ow, _cb_display_changed, cfdata);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   ow = e_widget_radio_add(evas, _("Fahrenheit"), FAHRENHEIT, rg);
   e_widget_on_change_hook_set(ow, _cb_display_changed, cfdata);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Display Units"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   ol = e_widget_list_add(evas, 0, 0);
   ow = e_widget_slider_add(evas, 1, 0, _("%1.0f ticks"), 1, 1024, 4, 0,
                            NULL, &(cfdata->poll.interval), 150);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Check Interval"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   ol = e_widget_list_add(evas, 0, 0);
   ow = e_widget_label_add(evas, _("High Temperature"));
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   if (cfdata->unit_method == FAHRENHEIT)
     cfdata->o_high =
     e_widget_slider_add(evas, 1, 0, _("%1.0f F"), 0, 230, 5, 0,
                         NULL, &(cfdata->temp.high), 150);
   else
     cfdata->o_high =
     e_widget_slider_add(evas, 1, 0, _("%1.0f C"), 0, 110, 5, 0,
                         NULL, &(cfdata->temp.high), 150);
   e_widget_list_object_append(ol, cfdata->o_high, 1, 1, 0.5);

   ow = e_widget_label_add(evas, _("Low Temperature"));
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   if (cfdata->unit_method == FAHRENHEIT)
     cfdata->o_low =
     e_widget_slider_add(evas, 1, 0, _("%1.0f F"), 0, 200, 5, 0,
                         NULL, &(cfdata->temp.low), 150);
   else
     cfdata->o_low =
     e_widget_slider_add(evas, 1, 0, _("%1.0f C"), 0, 95, 5, 0,
                         NULL, &(cfdata->temp.low), 150);
   e_widget_list_object_append(ol, cfdata->o_low, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Temperatures"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);
   e_widget_toolbook_page_show(otb, 0);
   return otb;
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Sensor *sen;

   cfdata->inst->poll_interval = cfdata->poll.interval;
   cfdata->inst->units = cfdata->unit_method;
   cfdata->inst->low = cfdata->temp.low;
   cfdata->inst->high = cfdata->temp.high;

   sen = eina_list_nth(cfdata->sensors, cfdata->sensor);
   if (sen)
     eina_stringshare_replace(&(cfdata->inst->sensor_name), sen->name);

   e_config_save_queue();
   temperature_face_update_config(cfdata->inst);
   return 1;
}

static void
_cb_display_changed(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   int val;

   if (!(cfdata = data)) return;
   if (cfdata->unit_method == FAHRENHEIT)
     {
        e_widget_slider_value_range_set(cfdata->o_low, 0, 200);
        e_widget_slider_value_range_set(cfdata->o_high, 0, 230);
        e_widget_slider_value_int_get(cfdata->o_low, &val);
        e_widget_slider_value_int_set(cfdata->o_low, CEL_2_FAR(val));
        e_widget_slider_value_int_get(cfdata->o_high, &val);
        e_widget_slider_value_int_set(cfdata->o_high, CEL_2_FAR(val));
        e_widget_slider_value_format_display_set(cfdata->o_low, _("%1.0f F"));
        e_widget_slider_value_format_display_set(cfdata->o_high, _("%1.0f F"));
     }
   else
     {
        e_widget_slider_value_range_set(cfdata->o_low, 0, 95);
        e_widget_slider_value_range_set(cfdata->o_high, 0, 110);
        e_widget_slider_value_int_get(cfdata->o_low, &val);
        e_widget_slider_value_int_set(cfdata->o_low, FAR_2_CEL(val));
        e_widget_slider_value_int_get(cfdata->o_high, &val);
        e_widget_slider_value_int_set(cfdata->o_high, FAR_2_CEL(val));
        e_widget_slider_value_format_display_set(cfdata->o_low, _("%1.0f C"));
        e_widget_slider_value_format_display_set(cfdata->o_high, _("%1.0f C"));
     }
}
