#include "e.h"

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   int        move_info_visible;
   int        move_info_follows;
   int        resize_info_visible;
   int        resize_info_follows;
   int        window_maximize_animate;
   int        window_maximize_transition;
   double     window_maximize_time;
   int        border_shade_animate;
   int        border_shade_transition;
   double     border_shade_speed;
   int        use_app_icon;
   int        window_placement_policy;
   int        window_grouping;
   int        desk_auto_switch;
   int        screen_limits;

};

/* a nice easy setup function that does the dirty work */
E_Config_Dialog *
e_int_config_window_display(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "windows/window_display")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   /* methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;
   v->basic.check_changed = _basic_check_changed;

   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(NULL, _("Window Display"),
                             "E", "windows/window_display",
                             "preferences-system-windows", 0, v, NULL);
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   if (!cfdata) return NULL;
   cfdata->move_info_visible = e_config->move_info_visible;
   cfdata->move_info_follows = e_config->move_info_follows;
   cfdata->resize_info_visible = e_config->resize_info_visible;
   cfdata->resize_info_follows = e_config->resize_info_follows;
   cfdata->use_app_icon = e_config->use_app_icon;

   cfdata->window_placement_policy =
     e_config->window_placement_policy;
   cfdata->window_grouping = e_config->window_grouping;
   cfdata->desk_auto_switch = e_config->desk_auto_switch;

   cfdata->screen_limits = e_config->screen_limits;

   cfdata->window_maximize_animate = e_config->window_maximize_animate;
   cfdata->window_maximize_transition = e_config->window_maximize_transition;
   cfdata->window_maximize_time = e_config->window_maximize_time;

   cfdata->border_shade_animate = e_config->border_shade_animate;
   cfdata->border_shade_transition = e_config->border_shade_transition;
   cfdata->border_shade_speed = e_config->border_shade_speed;

   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   free(cfdata);
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   e_config->window_placement_policy = cfdata->window_placement_policy;
   e_config->window_grouping = cfdata->window_grouping;
   e_config->move_info_visible = cfdata->move_info_visible;
   e_config->move_info_follows = cfdata->move_info_follows;
   e_config->resize_info_visible = cfdata->resize_info_visible;
   e_config->resize_info_follows = cfdata->resize_info_follows;
   e_config->window_maximize_animate = cfdata->window_maximize_animate;
   e_config->window_maximize_transition = cfdata->window_maximize_transition;
   e_config->window_maximize_time = cfdata->window_maximize_time;
   e_config->border_shade_animate = cfdata->border_shade_animate;
   e_config->border_shade_transition = cfdata->border_shade_transition;
   e_config->border_shade_speed = cfdata->border_shade_speed;
   e_config->use_app_icon = cfdata->use_app_icon;

   e_config->desk_auto_switch = cfdata->desk_auto_switch;
   e_config->screen_limits = cfdata->screen_limits;
   e_config_save_queue();
   return 1;
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   return (e_config->window_placement_policy != cfdata->window_placement_policy) ||
          (e_config->window_grouping != cfdata->window_grouping) ||
          (e_config->move_info_visible != cfdata->move_info_visible) ||
          (e_config->move_info_follows != cfdata->move_info_follows) ||
          (e_config->resize_info_visible != cfdata->resize_info_visible) ||
          (e_config->resize_info_follows != cfdata->resize_info_follows) ||
          (e_config->border_shade_animate != cfdata->border_shade_animate) ||
          (e_config->border_shade_transition != cfdata->border_shade_transition) ||
          (!EINA_DBL_EQ(e_config->border_shade_speed, cfdata->border_shade_speed)) ||
          (e_config->window_maximize_animate != cfdata->window_maximize_animate) ||
          (e_config->window_maximize_transition != cfdata->window_maximize_transition) ||
          (!dblequal(e_config->window_maximize_time, cfdata->window_maximize_time)) ||
          (e_config->use_app_icon != cfdata->use_app_icon) ||
          (e_config->desk_auto_switch != cfdata->desk_auto_switch) ||
          (e_config->screen_limits != cfdata->screen_limits);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *otb, *ol, *of, *ow, *oc;
   E_Radio_Group *rg;

   e_dialog_resizable_set(cfd->dia, 1);
   otb = e_widget_toolbook_add(evas, (24 * e_scale), (24 * e_scale));

   /* Display */
   ol = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Border Icon"), 0);
   rg = e_widget_radio_group_new(&(cfdata->use_app_icon));
   ow = e_widget_radio_add(evas, _("User defined"), 0, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Application provided"), 1, rg);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Move Geometry"), 0);
   oc = e_widget_check_add(evas, _("Display information"),
                           &(cfdata->move_info_visible));
   e_widget_framelist_object_append(of, oc);
   ow = e_widget_check_add(evas, _("Follows the window"),
                           &(cfdata->move_info_follows));
   e_widget_disabled_set(ow, !cfdata->move_info_visible);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Resize Geometry"), 0);
   oc = e_widget_check_add(evas, _("Display information"),
                           &(cfdata->resize_info_visible));
   e_widget_framelist_object_append(of, oc);
   ow = e_widget_check_add(evas, _("Follows the window"),
                           &(cfdata->resize_info_follows));
   e_widget_disabled_set(ow, !cfdata->resize_info_visible);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Display"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* New Windows */
   ol = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Placement"), 0);
   rg = e_widget_radio_group_new(&(cfdata->window_placement_policy));
   ow = e_widget_radio_add(evas, _("Smart Placement"),
                           E_WINDOW_PLACEMENT_SMART, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Don't hide Gadgets"),
                           E_WINDOW_PLACEMENT_ANTIGADGET, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Place at mouse pointer"),
                           E_WINDOW_PLACEMENT_CURSOR, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_radio_add(evas, _("Place manually with the mouse"),
                           E_WINDOW_PLACEMENT_MANUAL, rg);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_check_add(evas, _("Group with windows of the same application"),
                           &(cfdata->window_grouping));
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);
   ow = e_widget_check_add(evas, _("Switch to desktop of new window"),
                           &(cfdata->desk_auto_switch));
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("New Windows"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Shading */
   ol = e_widget_list_add(evas, 0, 0);
   oc = e_widget_check_add(evas, _("Animate"),
                           &(cfdata->border_shade_animate));
   e_widget_list_object_append(ol, oc, 1, 1, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, _("%4.0f pixels/s"),
                            100, 9900, 100, 0,
                            &(cfdata->border_shade_speed), NULL, 100);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   rg = e_widget_radio_group_new(&(cfdata->border_shade_transition));

   ow = e_widget_radio_add(evas, _("Linear"), E_TRANSITION_LINEAR, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Accelerate, then decelerate"), E_TRANSITION_SINUSOIDAL, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Accelerate"), E_TRANSITION_ACCELERATE, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Decelerate"), E_TRANSITION_DECELERATE, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Pronounced accelerate"), E_TRANSITION_ACCELERATE_LOTS, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Pronounced decelerate"), E_TRANSITION_DECELERATE_LOTS, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Pronounced accelerate, then decelerate"), E_TRANSITION_SINUSOIDAL_LOTS, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Bounce"), E_TRANSITION_BOUNCE, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Bounce more"), E_TRANSITION_BOUNCE_LOTS, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Shading"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Shading */
   ol = e_widget_list_add(evas, 0, 0);
   oc = e_widget_check_add(evas, _("Animate"),
                           &(cfdata->window_maximize_animate));
   e_widget_list_object_append(ol, oc, 1, 1, 0.5);
   ow = e_widget_slider_add(evas, 1, 0, _("%1.2f seconds"),
                            0.0, 1.0, 0.05, 0,
                            &(cfdata->window_maximize_time), NULL, 100);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   rg = e_widget_radio_group_new(&(cfdata->border_shade_transition));

   ow = e_widget_radio_add(evas, _("Linear"), E_EFX_EFFECT_SPEED_LINEAR, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Accelerate"), E_EFX_EFFECT_SPEED_ACCELERATE, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Decelerate"), E_EFX_EFFECT_SPEED_DECELERATE, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Accelerate, then decelerate"), E_EFX_EFFECT_SPEED_SINUSOIDAL, rg);
   e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Maximizing"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Screen Limits */
   ol = e_widget_list_add(evas, 0, 0);

   rg = e_widget_radio_group_new(&(cfdata->screen_limits));

   ow = e_widget_radio_add(evas, _("Keep windows within the visual screen limits"), E_CLIENT_OFFSCREEN_LIMIT_ALLOW_NONE, rg);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Allow windows partly out of the visual screen limits"), E_CLIENT_OFFSCREEN_LIMIT_ALLOW_PARTIAL, rg);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Allow windows completely out of visual screen limits"), E_CLIENT_OFFSCREEN_LIMIT_ALLOW_FULL, rg);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Screen Limits"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   e_widget_toolbook_page_show(otb, 0);
   return otb;
}
