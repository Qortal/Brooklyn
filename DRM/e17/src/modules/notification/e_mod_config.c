#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int    show_low;
   int    show_normal;
   int    show_critical;
   int    force_timeout;
   int    ignore_replacement;
   Popup_Display_Policy dual_screen;
   double timeout;
   int    corner;
   Evas_Object *force_timeout_slider;
};

/* local function protos */
static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog      *cfd,
                               E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog      *cfd,
                                  Evas                 *evas,
                                  E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog      *cfd,
                        E_Config_Dialog_Data *cfdata);
static int _basic_check_changed(E_Config_Dialog      *cfd,
                                E_Config_Dialog_Data *cfdata);
static void _force_timeout_changed(void        *data,
                                   Evas_Object *obj EINA_UNUSED);

E_Config_Dialog *
e_int_config_notification_module(Evas_Object *parent EINA_UNUSED,
                                 const char  *params EINA_UNUSED)
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   char buf[4096];

   if (e_config_dialog_find("Notification", "extensions/notification")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.check_changed = _basic_check_changed;

   snprintf(buf, sizeof(buf), "%s/e-module-notification.edj", notification_mod->dir);
   cfd = e_config_dialog_new(NULL, _("Notification Settings"), "Notification",
                             "extensions/notification", buf, 0, v, NULL);
   notification_cfg->cfd = cfd;
   return cfd;
}

/* local functions */
static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = NULL;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog      *cfd EINA_UNUSED,
           E_Config_Dialog_Data *cfdata)
{
   notification_cfg->cfd = NULL;
   E_FREE(cfdata);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->show_low = notification_cfg->show_low;
   cfdata->show_normal = notification_cfg->show_normal;
   cfdata->show_critical = notification_cfg->show_critical;
   cfdata->timeout = notification_cfg->timeout;
   cfdata->corner = notification_cfg->corner;
   cfdata->force_timeout = notification_cfg->force_timeout;
   cfdata->ignore_replacement = notification_cfg->ignore_replacement;
   cfdata->dual_screen = notification_cfg->dual_screen;
}

static Evas_Object *
_basic_create(E_Config_Dialog      *cfd EINA_UNUSED,
              Evas                 *evas,
              E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;
   E_Radio_Group *rg;
//   E_Manager *man;

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Urgency"), 0);
   ow = e_widget_label_add(evas, _("Levels of urgency to display:"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_check_add(evas, _("Low"), &(cfdata->show_low));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_check_add(evas, _("Normal"), &(cfdata->show_normal));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_check_add(evas, _("Critical"), &(cfdata->show_critical));
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Default Timeout"), 0);
   ow = e_widget_check_add(evas, _("Force timeout for all notifications"), &(cfdata->force_timeout));
   e_widget_on_change_hook_set(ow, _force_timeout_changed, cfdata);
   e_widget_framelist_object_append(of, ow);
   cfdata->force_timeout_slider = ow =
     e_widget_slider_add(evas, 1, 0, _("%.1f seconds"), 0.0, 15.0, 0.1, 0, &(cfdata->timeout), NULL, 200);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   _force_timeout_changed(cfdata, NULL);

   /* man = e_manager_current_get();
    * of = e_widget_framelist_add(evas, _("Placement"), 0);
    * ow = e_widget_slider_add(evas, 1, 0, _("%2.0f x"), 0.0, man->w, 1.0, 0,
    *                          NULL, &(cfdata->placement.x), 200);
    * e_widget_framelist_object_append(of, ow);
    * ow = e_widget_slider_add(evas, 1, 0, _("%2.0f y"), 0.0, man->h, 1.0, 0,
    *                          NULL, &(cfdata->placement.y), 200);
    * e_widget_framelist_object_append(of, ow);
    * e_widget_list_object_append(o, of, 1, 1, 0.5); */

   of = e_widget_framelist_add(evas, _("Screen Policy"), 0);
   rg = e_widget_radio_group_new((int*)&cfdata->dual_screen);
   ow = e_widget_radio_add(evas, _("Primary screen"), POPUP_DISPLAY_POLICY_FIRST, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Current screen"), POPUP_DISPLAY_POLICY_CURRENT, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("All screens"), POPUP_DISPLAY_POLICY_ALL, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Xinerama"), POPUP_DISPLAY_POLICY_MULTI, rg);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Popup Corner"), 0);
   rg = e_widget_radio_group_new(&(cfdata->corner));
   ow = e_widget_radio_add(evas, _("Top left"), CORNER_TL, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Top right"), CORNER_TR, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Bottom left"), CORNER_BL, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Bottom right"), CORNER_BR, rg);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   /* of = e_widget_framelist_add(evas, _("Gap"), 0);
   * ow = e_widget_label_add(evas, _("Size of the gap between two popups : "));
   * e_widget_framelist_object_append(of, ow);
   * ow = e_widget_slider_add(evas, 1, 0, _("%2.0f pixels"), 0.0, 50, 1.0, 0,
   *                          NULL, &(cfdata->gap), 200);
   * e_widget_framelist_object_append(of, ow);
   * e_widget_list_object_append(o, of, 1, 1, 0.5); */
   of = e_widget_framelist_add(evas, _("Miscellaneous"), 0);
   ow = e_widget_check_add(evas, _("Ignore replace ID"), &(cfdata->ignore_replacement));
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 1, 0.5);


   return o;
}

static int
_basic_apply(E_Config_Dialog      *cfd EINA_UNUSED,
             E_Config_Dialog_Data *cfdata)
{
   notification_cfg->show_low = cfdata->show_low;
   notification_cfg->show_normal = cfdata->show_normal;
   notification_cfg->show_critical = cfdata->show_critical;
   notification_cfg->timeout = cfdata->timeout;
   notification_cfg->corner = cfdata->corner;
   notification_cfg->force_timeout = cfdata->force_timeout;
   notification_cfg->ignore_replacement = cfdata->ignore_replacement;
   notification_cfg->dual_screen = cfdata->dual_screen;

   e_modapi_save(notification_mod);
   return 1;
}

static int
_basic_check_changed(E_Config_Dialog      *cfd EINA_UNUSED,
                     E_Config_Dialog_Data *cfdata)
{
   return
     (cfdata->show_low != notification_cfg->show_low) ||
     (cfdata->show_normal != notification_cfg->show_normal) ||
     (cfdata->show_critical != notification_cfg->show_critical) ||
     (!EINA_DBL_EQ(cfdata->timeout, notification_cfg->timeout)) ||
     (cfdata->corner != (int)notification_cfg->corner) ||
     (cfdata->force_timeout != notification_cfg->force_timeout) ||
     (cfdata->ignore_replacement != notification_cfg->ignore_replacement) ||
     (cfdata->dual_screen != notification_cfg->dual_screen);
}

static void
_force_timeout_changed(void        *data,
                       Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   e_widget_disabled_set(cfdata->force_timeout_slider, !cfdata->force_timeout);
}
