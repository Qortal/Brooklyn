#include "e.h"

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   int    windows_other_desks;
   int    windows_other_screens;
   int    iconified;
   int    iconified_other_desks;
   int    iconified_other_screens;

   int    focus, raise, uncover;
   int    warp_while_selecting;
   int    warp_at_end;
   int    no_warp_on_direction;
   int    jump_desk;
   int    move_after_select;

   int    mode;
   int    list_miniatures;
   double large_size;
   double list_size;

   int    scroll_animate;
   double scroll_speed;

   struct {
      Evas_Object *min_w, *min_h;
   } gui;
};

E_Config_Dialog *
e_int_config_winlist(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "windows/window_list")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.create_widgets = _basic_create;
   v->basic.check_changed = _basic_check_changed;

   cfd = e_config_dialog_new(NULL, _("Window Switcher Settings"),
                             "E", "windows/window_list",
                             "preferences-winlist", 0, v, NULL);
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   cfdata->focus = e_config->winlist_list_focus_while_selecting;
   cfdata->raise = e_config->winlist_list_raise_while_selecting;
   cfdata->uncover = e_config->winlist_list_uncover_while_selecting;
   cfdata->jump_desk = e_config->winlist_list_jump_desk_while_selecting;
   cfdata->move_after_select = e_config->winlist_list_move_after_select;
   cfdata->mode = e_config->winlist_mode;
   cfdata->large_size = e_config->winlist_large_size;
   cfdata->list_size = e_config->winlist_list_size;
   cfdata->list_miniatures = !(!!e_config->winlist_list_no_miniatures);

   cfdata->windows_other_desks =
     e_config->winlist_list_show_other_desk_windows;
   cfdata->windows_other_screens =
     e_config->winlist_list_show_other_screen_windows;

   cfdata->iconified = e_config->winlist_list_show_iconified;
   cfdata->iconified_other_desks =
     e_config->winlist_list_show_other_desk_iconified;
   cfdata->iconified_other_screens =
     e_config->winlist_list_show_other_screen_iconified;

   cfdata->warp_while_selecting = e_config->winlist_warp_while_selecting;
   cfdata->warp_at_end = e_config->winlist_warp_at_end;
   cfdata->no_warp_on_direction = e_config->winlist_no_warp_on_direction;

   cfdata->scroll_animate = e_config->winlist_scroll_animate;
   cfdata->scroll_speed = e_config->winlist_scroll_speed;
}

static void *
_create_data(E_Config_Dialog *cfd EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   if (!cfdata) return NULL;
   _fill_data(cfdata);
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
#define DO(_e_config, _cfdata) \
  e_config->winlist_##_e_config = cfdata->_cfdata

   DO(list_show_iconified, iconified);
   DO(list_show_other_desk_iconified, iconified_other_desks);
   DO(list_show_other_screen_iconified, iconified_other_screens);
   DO(list_show_other_desk_windows, windows_other_desks);
   DO(list_show_other_screen_windows, windows_other_screens);
   DO(list_uncover_while_selecting, uncover);
   DO(list_jump_desk_while_selecting, jump_desk);
   DO(list_move_after_select, move_after_select);
   DO(mode, mode);
   DO(large_size, large_size);
   DO(list_size, list_size);
   e_config->winlist_list_no_miniatures = !(!!cfdata->list_miniatures);
   DO(warp_while_selecting, warp_while_selecting);
   DO(warp_at_end, warp_at_end);
   DO(no_warp_on_direction, no_warp_on_direction);
   DO(scroll_animate, scroll_animate);
   DO(scroll_speed, scroll_speed);
   DO(list_focus_while_selecting, focus);
   DO(list_raise_while_selecting, raise);
#undef DO

   e_config_save_queue();

   return 1;
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
#define DO(_e_config, _cfdata) \
  if (e_config->winlist_##_e_config != cfdata->_cfdata) return 1
#define DO_DBL(_e_config, _cfdata) \
  if (!EINA_DBL_EQ(e_config->winlist_##_e_config, cfdata->_cfdata)) return 1

   DO(list_show_iconified, iconified);
   DO(list_show_other_desk_iconified, iconified_other_desks);
   DO(list_show_other_screen_iconified, iconified_other_screens);
   DO(list_show_other_desk_windows, windows_other_desks);
   DO(list_show_other_screen_windows, windows_other_screens);
   DO(list_uncover_while_selecting, uncover);
   DO(list_jump_desk_while_selecting, jump_desk);
   DO(list_move_after_select, move_after_select);
   DO(mode, mode);
   DO_DBL(large_size, large_size);
   DO_DBL(list_size, list_size);
   if (e_config->winlist_list_no_miniatures != !(!!cfdata->list_miniatures)) return 1;
   DO(warp_while_selecting, warp_while_selecting);
   DO(warp_at_end, warp_at_end);
   DO(no_warp_on_direction, no_warp_on_direction);
   DO(scroll_animate, scroll_animate);
   DO_DBL(scroll_speed, scroll_speed);
   DO(list_focus_while_selecting, focus);
   DO(list_raise_while_selecting, raise);
#undef DO

   return 0;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *otb, *ol, *ob, *iconified, *scroll_animate, *ck;

   e_dialog_resizable_set(cfd->dia, 1);
   otb = e_widget_toolbook_add(evas, (48 * e_scale), (48 * e_scale));

   ol = e_widget_list_add(evas, 0, 0);
   ob = e_widget_check_add(evas, _("Large"),
                           &(cfdata->mode));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_label_add(evas, _("Large Mode Size"));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.2f"), 0.0, 1.0, 0.01, 0,
                            &(cfdata->large_size), NULL, 100);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_label_add(evas, _("List Mode Size"));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.2f"), 0.0, 1.0, 0.01, 0,
                            &(cfdata->list_size), NULL, 100);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("List Window Miniatures"),
                           &(cfdata->list_miniatures));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Windows from other desks"),
                           &(cfdata->windows_other_desks));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Windows from other screens"),
                           &(cfdata->windows_other_screens));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   iconified = ob = e_widget_check_add(evas, _("Iconified"), &(cfdata->iconified));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Iconified from other desks"),
                           &(cfdata->iconified_other_desks));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   e_widget_check_widget_disable_on_unchecked_add(iconified, ob);
   ob = e_widget_check_add(evas, _("Iconified from other screens"),
                           &(cfdata->iconified_other_screens));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   e_widget_check_widget_disable_on_unchecked_add(iconified, ob);
   e_widget_toolbook_page_append(otb, NULL, _("Display"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   ol = e_widget_list_add(evas, 0, 0);
   ob = e_widget_check_add(evas, _("Focus"), &(cfdata->focus));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Raise"), &(cfdata->raise));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Uniconify/Unshade"), &(cfdata->uncover));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Warp mouse while selecting"),
                           &(cfdata->warp_while_selecting));
   e_widget_disabled_set(ob, e_config->disable_all_pointer_warps);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Warp mouse at end"),
                           &(cfdata->warp_at_end));
   e_widget_disabled_set(ob, e_config->disable_all_pointer_warps);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Disable mouse warp on directional focus change"),
                           &(cfdata->no_warp_on_direction));
   e_widget_disabled_set(ob, e_config->disable_all_pointer_warps);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ck = ob = e_widget_check_add(evas, _("Jump to desk"), &(cfdata->jump_desk));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_check_add(evas, _("Move to current desk after switch"), &(cfdata->move_after_select));
   e_widget_check_widget_disable_on_checked_add(ck, ob);
   e_widget_check_widget_disable_on_checked_add(ob, ck);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   e_widget_toolbook_page_append(otb, NULL, _("Selecting"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   ol = e_widget_list_add(evas, 0, 0);
   scroll_animate = ob = e_widget_check_add(evas, _("Scroll Animation"),
                           &(cfdata->scroll_animate));
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_label_add(evas, _("Scroll speed"));
   e_widget_check_widget_disable_on_unchecked_add(scroll_animate, ob);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.2f"), 0.0, 1.0, 0.01, 0,
                            &(cfdata->scroll_speed), NULL, 100);
   e_widget_check_widget_disable_on_unchecked_add(scroll_animate, ob);
   e_widget_list_object_append(ol, ob, 1, 0, 0.0);
   e_widget_toolbook_page_append(otb, NULL, _("Animations"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   e_widget_toolbook_page_show(otb, 0);

   return otb;
}
