#include "e.h"

/* PROTOTYPES - same all the time */

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _advanced_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _advanced_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

/* Actual config data we will be playing with whil the dialog is active */
struct _E_Config_Dialog_Data
{
   /*- BASIC -*/
   int    mode;
   int    use_auto_raise;
   /*- ADVANCED -*/
   int    focus_policy;
   int    focus_setting;
   int    pass_click_on;
   int    window_activehint_policy;
   int    always_click_to_raise;
   int    always_click_to_focus;
   int    focus_last_focused_per_desktop;
   int    focus_revert_on_hide_or_close;
   int    focus_revert_allow_sticky;
   int    pointer_slide;
   int    disable_all_pointer_warps;
   double pointer_warp_speed;
   double auto_raise_delay;
   int    border_raise_on_mouse_action;
   int    raise_on_revert_focus;
};

/* a nice easy setup function that does the dirty work */
E_Config_Dialog *
e_int_config_focus(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "windows/window_focus")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   /* methods */
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;
   v->basic.check_changed = _basic_check_changed;
   v->advanced.apply_cfdata = _advanced_apply;
   v->advanced.create_widgets = _advanced_create;
   v->advanced.check_changed = _advanced_check_changed;

   /* create config diaolg for NULL object/data */
   cfd = e_config_dialog_new(NULL, _("Focus Settings"), "E",
                             "windows/window_focus", "preferences-focus",
                             0, v, NULL);
   return cfd;
}

/**--CREATE--**/
static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->focus_policy = e_config->focus_policy;
   cfdata->focus_setting = e_config->focus_setting;
   cfdata->pass_click_on = e_config->pass_click_on;
   cfdata->window_activehint_policy = e_config->window_activehint_policy;
   cfdata->always_click_to_raise = e_config->always_click_to_raise;
   cfdata->always_click_to_focus = e_config->always_click_to_focus;
   cfdata->focus_last_focused_per_desktop =
     e_config->focus_last_focused_per_desktop;
   cfdata->focus_revert_on_hide_or_close =
     e_config->focus_revert_on_hide_or_close;
   cfdata->focus_revert_allow_sticky =
     e_config->focus_revert_allow_sticky;
   cfdata->pointer_slide = e_config->pointer_slide;
   cfdata->disable_all_pointer_warps = e_config->disable_all_pointer_warps;
   cfdata->pointer_warp_speed = e_config->pointer_warp_speed;

   cfdata->mode = cfdata->focus_policy;

   cfdata->use_auto_raise = e_config->use_auto_raise;
   cfdata->auto_raise_delay = e_config->auto_raise_delay;
   cfdata->border_raise_on_mouse_action =
     e_config->border_raise_on_mouse_action;
   cfdata->raise_on_revert_focus = e_config->raise_on_revert_focus;
}

static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   /* Create cfdata - cfdata is a temporary block of config data that this
    * dialog will be dealing with while configuring. it will be applied to
    * the running systems/config in the apply methods
    */
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   free(cfdata);
}

/**--APPLY--**/
static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   e_comp_button_bindings_ungrab_all();
   if (cfdata->mode == E_FOCUS_CLICK)
     {
        e_config->focus_policy = E_FOCUS_CLICK;
        e_config->focus_setting = E_FOCUS_NEW_WINDOW;
        e_config->pass_click_on = 1;
        e_config->window_activehint_policy = 2;
        e_config->always_click_to_raise = 1;
        e_config->always_click_to_focus = 1;
        e_config->focus_last_focused_per_desktop = 1;
        e_config->focus_revert_on_hide_or_close = 1;
        e_config->pointer_slide = 0;
        e_config->disable_all_pointer_warps = 1;
     }
   else if (cfdata->mode == E_FOCUS_MOUSE)
     {
        e_config->focus_policy = E_FOCUS_MOUSE;
        e_config->focus_setting = E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED;
        e_config->pass_click_on = 1;
        e_config->window_activehint_policy = 2;
        e_config->always_click_to_raise = 0;
        e_config->always_click_to_focus = 0;
        e_config->focus_last_focused_per_desktop = 0;
        e_config->focus_revert_on_hide_or_close = 0;
        e_config->pointer_slide = 1;
        e_config->disable_all_pointer_warps = 0;
     }
   else
     {
        e_config->focus_policy = E_FOCUS_SLOPPY;
        e_config->focus_setting = E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED;
        e_config->pass_click_on = 1;
        e_config->window_activehint_policy = 2;
        e_config->always_click_to_raise = 0;
        e_config->always_click_to_focus = 0;
        e_config->focus_last_focused_per_desktop = 1;
        e_config->focus_revert_on_hide_or_close = 1;
        e_config->pointer_slide = 1;
        e_config->disable_all_pointer_warps = 0;
     }
   e_config->use_auto_raise = cfdata->use_auto_raise;
   e_comp_button_bindings_grab_all();
   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   return (e_config->focus_policy != cfdata->mode) ||
          (e_config->use_auto_raise != cfdata->use_auto_raise);
}

static int
_advanced_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   /* Actually take our cfdata settings and apply them in real life */
   e_comp_button_bindings_ungrab_all();
   e_config->focus_policy = cfdata->focus_policy;
   e_config->focus_setting = cfdata->focus_setting;
   e_config->pass_click_on = cfdata->pass_click_on;
   e_config->window_activehint_policy = cfdata->window_activehint_policy;
   e_config->always_click_to_raise = cfdata->always_click_to_raise;
   e_config->always_click_to_focus = cfdata->always_click_to_focus;
   e_config->focus_last_focused_per_desktop =
     cfdata->focus_last_focused_per_desktop;
   e_config->focus_revert_on_hide_or_close =
     cfdata->focus_revert_on_hide_or_close;
   e_config->focus_revert_allow_sticky =
     cfdata->focus_revert_allow_sticky;
   e_config->pointer_slide = cfdata->pointer_slide;
   e_config->disable_all_pointer_warps = cfdata->disable_all_pointer_warps;
   e_config->pointer_warp_speed = cfdata->pointer_warp_speed;

   e_config->use_auto_raise = cfdata->use_auto_raise;
   e_config->auto_raise_delay = cfdata->auto_raise_delay;
   e_config->border_raise_on_mouse_action = cfdata->border_raise_on_mouse_action;
   e_config->raise_on_revert_focus = cfdata->raise_on_revert_focus;
   e_comp_button_bindings_grab_all();
   e_config_save_queue();
   return 1; /* Apply was OK */
}

static int
_advanced_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   return (e_config->focus_policy != cfdata->focus_policy) ||
          (e_config->focus_setting != cfdata->focus_setting) ||
          (e_config->pass_click_on != cfdata->pass_click_on) ||
          (e_config->window_activehint_policy != cfdata->window_activehint_policy) ||
          (e_config->always_click_to_raise != cfdata->always_click_to_raise) ||
          (e_config->always_click_to_focus != cfdata->always_click_to_focus) ||
          (e_config->focus_last_focused_per_desktop != cfdata->focus_last_focused_per_desktop) ||
          (e_config->focus_revert_on_hide_or_close != cfdata->focus_revert_on_hide_or_close) ||
          (e_config->focus_revert_allow_sticky != cfdata->focus_revert_allow_sticky) ||
          (e_config->pointer_slide != cfdata->pointer_slide) ||
          (e_config->disable_all_pointer_warps != cfdata->disable_all_pointer_warps) ||
          (fabs(e_config->pointer_warp_speed - cfdata->pointer_warp_speed) > DBL_EPSILON) ||
          (e_config->use_auto_raise != cfdata->use_auto_raise) ||
          (!EINA_DBL_EQ(e_config->auto_raise_delay, cfdata->auto_raise_delay)) ||
          (e_config->border_raise_on_mouse_action != cfdata->border_raise_on_mouse_action) ||
          (e_config->raise_on_revert_focus != cfdata->raise_on_revert_focus);
}

/**--GUI--**/
static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for a basic dialog */
   Evas_Object *o, *ob;
   E_Radio_Group *rg;

   e_dialog_resizable_set(cfd->dia, 1);
   o = e_widget_list_add(evas, 0, 0);
   rg = e_widget_radio_group_new(&(cfdata->mode));
   ob = e_widget_radio_add(evas, _("Click Window to Focus"), E_FOCUS_CLICK, rg);
   e_widget_list_object_append(o, ob, 1, 0, 0.0);
   ob = e_widget_radio_add(evas, _("Window under the Mouse"), E_FOCUS_MOUSE, rg);
   e_widget_list_object_append(o, ob, 1, 0, 0.0);
   ob = e_widget_radio_add(evas, _("Most recent Window under the Mouse"),
                           E_FOCUS_SLOPPY, rg);
   e_widget_list_object_append(o, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Raise windows on mouse over"),
                           &(cfdata->use_auto_raise));
   e_widget_list_object_append(o, ob, 1, 0, 0.5);
   return o;
}

static Evas_Object *
_advanced_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   /* generate the core widget layout for an advanced dialog */
   Evas_Object *otb, *ol, *ob, *obp, *of;
   Evas_Object *autoraise_check;
   E_Radio_Group *rg;

   e_dialog_resizable_set(cfd->dia, 1);
   otb = e_widget_toolbook_add(evas, (24 * e_scale), (24 * e_scale));

   /* Focus */
   ol = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Focus Policy"), 0);
   rg = e_widget_radio_group_new(&(cfdata->focus_policy));
   ob = e_widget_radio_add(evas, _("Click"), E_FOCUS_CLICK, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Pointer"), E_FOCUS_MOUSE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Sloppy"), E_FOCUS_SLOPPY, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.5);

   of = e_widget_framelist_add(evas, _("New Window Focus"), 0);
   rg = e_widget_radio_group_new(&(cfdata->focus_setting));
   ob = e_widget_radio_add(evas, _("No window"), E_FOCUS_NONE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("All windows"), E_FOCUS_NEW_WINDOW, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Only dialogs"), E_FOCUS_NEW_DIALOG, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Only dialogs with focused parent"),
                           E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Focus"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Stacking */
   ol = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Autoraise"), 0);
   autoraise_check = e_widget_check_add(evas, _("Raise windows on mouse over"),
                                        &(cfdata->use_auto_raise));
   e_widget_framelist_object_append(of, autoraise_check);
   ob = e_widget_label_add(evas, _("Delay before raising:"));
   e_widget_check_widget_disable_on_unchecked_add(autoraise_check, ob);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f s"), 0.0, 9.9, 0.1, 0,
                            &(cfdata->auto_raise_delay), NULL, 100);
   e_widget_check_widget_disable_on_unchecked_add(autoraise_check, ob);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.5);

   of = e_widget_framelist_add(evas, _("Raise Window"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   ob = e_widget_check_add(evas, _("Raise when starting to move or resize"),
                           &(cfdata->border_raise_on_mouse_action));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Raise when reverting focus"),
                           &(cfdata->raise_on_revert_focus));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Stacking"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Urgency */
   of = e_widget_framelist_add(evas, _("Active Window Hint Policy"), 0);
   rg = e_widget_radio_group_new(&cfdata->window_activehint_policy);
   ob = e_widget_radio_add(evas, _("Ignore hint"), E_ACTIVEHINT_POLICY_IGNORE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Animate on hint"), E_ACTIVEHINT_POLICY_ANIMATE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Activate on hint"), E_ACTIVEHINT_POLICY_ACTIVATE, rg);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_radio_add(evas, _("Activate if on visible desk"), E_ACTIVEHINT_POLICY_ACTIVATE_EXCLUDE, rg);
   e_widget_framelist_object_append(of, ob);
   e_widget_toolbook_page_append(otb, NULL, _("Hints"), of,
                                 1, 1, 1, 0, 0.0, 0.0);
   /* Pointer */
   ol = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Warping"), 0);
   /* NOTE/TODO:
    *
    * IMHO all these slide-pointer-to-window, warp and all should have
    * an unique and consistent setting. In some cases it just do not
    * make sense to have one but not the other.
    */

   obp = ob = e_widget_check_add(evas, _("Prevent all forms of pointer warping"),
                                 &(cfdata->disable_all_pointer_warps));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Slide pointer to a new window which is focused"),
                           &(cfdata->pointer_slide));
   e_widget_framelist_object_append(of, ob);
   e_widget_check_widget_disable_on_checked_add(obp, ob);

   ob = e_widget_label_add(evas, _("Warp speed"));
   e_widget_check_widget_disable_on_checked_add(obp, ob);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.2f"), 0.0, 1.0, 0.01, 0,
                            &(cfdata->pointer_warp_speed), NULL, 100);
   e_widget_check_widget_disable_on_checked_add(obp, ob);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Pointer"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Misc */
   ol = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Other Settings"), 0);
   ob = e_widget_check_add(evas, _("Always pass click events to programs"),
                           &(cfdata->pass_click_on));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Click raises the window"),
                           &(cfdata->always_click_to_raise));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Click focuses the window"),
                           &(cfdata->always_click_to_focus));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Refocus last window on desktop switch"),
                           &(cfdata->focus_last_focused_per_desktop));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Allow focusing of sticky windows when reverting focus"),
                           &(cfdata->focus_revert_allow_sticky));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Focus last focused window on lost focus"),
                           &(cfdata->focus_revert_on_hide_or_close));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(ol, of, 1, 0, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Miscellaneous"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   e_widget_toolbook_page_show(otb, 0);
   return otb;
}

