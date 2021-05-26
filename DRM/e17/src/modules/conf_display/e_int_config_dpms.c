#include "e.h"

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int  _advanced_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int  _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object  *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas,
                                              E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;

   Evas_Object *backlight_slider_idle;
   Evas_Object *backlight_battery_slider_idle;
   Evas_Object *backlight_battery_label;
   Evas_Object *backlight_slider_fade;

   int enable_idle_dim;

   double backlight_normal;
   double backlight_dim;
   double backlight_timeout;
   double backlight_battery_timeout;
   double backlight_transition;
};

E_Config_Dialog *
e_int_config_dpms(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "screen/power_management"))
     return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _advanced_apply_data;
   v->basic.create_widgets = _advanced_create_widgets;
   v->basic.check_changed = _advanced_check_changed;
   v->override_auto_apply = 1;

   cfd = e_config_dialog_new(NULL, _("Backlight Settings"), "E",
                             "screen/power_management", "preferences-system-power-management",
                             0, v, NULL);
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->backlight_normal = e_config->backlight.normal * 100.0;
   cfdata->backlight_dim = e_config->backlight.dim * 100.0;
   cfdata->backlight_transition = e_config->backlight.transition;
   cfdata->enable_idle_dim = e_config->backlight.idle_dim;
   cfdata->backlight_timeout = e_config->backlight.timer;
   cfdata->backlight_battery_timeout = e_config->backlight.battery_timer;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->cfd = cfd;

   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_FREE(cfdata);
}

static int
_apply_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   e_config->backlight.normal = cfdata->backlight_normal / 100.0;
   e_config->backlight.dim = cfdata->backlight_dim / 100.0;
   e_config->backlight.transition = cfdata->backlight_transition;
   e_config->backlight.timer = lround(cfdata->backlight_timeout);
   e_config->backlight.battery_timer = lround(cfdata->backlight_battery_timeout);
   e_config->backlight.idle_dim = cfdata->enable_idle_dim;

   e_backlight_mode_set(NULL, E_BACKLIGHT_MODE_NORMAL);
   e_backlight_level_set(NULL, e_config->backlight.normal, -1.0);

   if ((e_config->backlight.idle_dim) &&
       (e_config->backlight.timer > (e_config->screensaver_timeout)))
     {
        e_config->screensaver_timeout = e_config->backlight.timer;
        e_config->dpms_standby_timeout = e_config->screensaver_timeout;
        e_config->dpms_suspend_timeout = e_config->screensaver_timeout;
        e_config->dpms_off_timeout = e_config->screensaver_timeout;
     }
   e_screensaver_update();
   e_dpms_update();
   e_backlight_update();
   e_config_save_queue();
   return 1;
}

/* advanced window */
static int
_advanced_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   // set state from saved config
   e_widget_disabled_set(cfdata->backlight_slider_idle, !cfdata->enable_idle_dim);
   e_widget_disabled_set(cfdata->backlight_battery_slider_idle, !cfdata->enable_idle_dim);
   e_widget_disabled_set(cfdata->backlight_battery_label, !cfdata->enable_idle_dim);
   e_widget_disabled_set(cfdata->backlight_slider_fade, !cfdata->enable_idle_dim);

   return (!EINA_DBL_EQ(e_config->backlight.normal * 100.0, cfdata->backlight_normal)) ||
          (!EINA_DBL_EQ(e_config->backlight.dim * 100.0, cfdata->backlight_dim)) ||
          (!EINA_DBL_EQ(e_config->backlight.transition, cfdata->backlight_transition)) ||
          (!EINA_DBL_EQ(e_config->backlight.timer, cfdata->backlight_timeout)) ||
          (!EINA_DBL_EQ(e_config->backlight.battery_timer, cfdata->backlight_battery_timeout)) ||
          (e_config->backlight.idle_dim != cfdata->enable_idle_dim);
}

static int
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   _apply_data(cfd, cfdata);
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *otb, *o, *ob;

   otb = e_widget_toolbook_add(evas, (24 * e_scale), (24 * e_scale));

   /* Dimming */
   o = e_widget_list_add(evas, 0, 0);
   ob = e_widget_label_add(evas, _("Normal Backlight"));
   e_widget_list_object_append(o, ob, 1, 1, 0.5);
   ob = e_widget_slider_add(evas, 1, 0, _("%3.0f"), 0.0, 100.0, 1.0, 0,
                            &(cfdata->backlight_normal), NULL, 100);
   e_widget_list_object_append(o, ob, 1, 1, 0.5);

   ob = e_widget_label_add(evas, _("Dim Backlight"));
   e_widget_list_object_append(o, ob, 1, 1, 0.5);
   ob = e_widget_slider_add(evas, 1, 0, _("%3.0f"), 0.0, 100.0, 1.0, 0,
                            &(cfdata->backlight_dim), NULL, 100);
   e_widget_list_object_append(o, ob, 1, 1, 0.5);

   ob = e_widget_check_add(evas, _("Idle Fade Time"), &(cfdata->enable_idle_dim));
   e_widget_list_object_append(o, ob, 1, 1, 0.5);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f second(s)"), 5.0, 300.0, 1.0, 0,
                            &(cfdata->backlight_timeout), NULL, 100);
   cfdata->backlight_slider_idle = ob;
   e_widget_disabled_set(ob, !cfdata->enable_idle_dim); // set state from saved config
   e_widget_list_object_append(o, ob, 1, 1, 0.5);
   ob = e_widget_label_add(evas, _("Fade Time on Battery"));
   cfdata->backlight_battery_label = ob;
   e_widget_disabled_set(ob, !cfdata->enable_idle_dim); // set state from saved config
   e_widget_list_object_append(o, ob, 1, 1, 0.5);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f second(s)"), 0.0, 300.0, 1.0, 0,
                            &(cfdata->backlight_battery_timeout), NULL, 100);
   cfdata->backlight_battery_slider_idle = ob;
   e_widget_disabled_set(ob, !cfdata->enable_idle_dim); // set state from saved config
   e_widget_list_object_append(o, ob, 1, 1, 0.5);

   ob = e_widget_label_add(evas, _("Fade Time"));
   e_widget_list_object_append(o, ob, 1, 1, 0.5);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f second(s)"), 0.0, 5.0, 0.1, 0,
                            &(cfdata->backlight_transition), NULL, 100);
   cfdata->backlight_slider_fade = ob;
   e_widget_disabled_set(ob, !cfdata->enable_idle_dim); // set state from saved config
   e_widget_list_object_append(o, ob, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Dimming"), o,
                                 1, 0, 1, 0, 0.5, 0.0);
   e_widget_toolbook_page_show(otb, 0);
   return otb;
}
