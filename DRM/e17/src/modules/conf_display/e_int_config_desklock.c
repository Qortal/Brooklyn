#include "e.h"
#include "e_mod_main.h"


static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static int          _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

static void         _cb_method_change(void *data, Evas_Object *obj, void *event_info);
static void         _login_method_change(void *data, Evas_Object *obj, void *event_info);
static void         _cb_login_change(void *data, Evas_Object *obj);

static void         _cb_bg_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);

struct _E_Config_Dialog_Data
{
   Evas_Object *passwd_entry, *pin_entry;
   E_Config_Dialog *cfd, *bg_fsel;

   /* Common vars */
   int              use_xscreensaver;
   int              zone_count;

   /* Locking */
   int              start_locked;
   int              lock_on_suspend;
   int              desklock_auth_method;
   int              login_zone;
   int              zone;
   char            *desklock_personal_passwd;
   char            *pin_str;

   /* Layout */
   const char     *desklock_layout;

   /* Timers */
   int              screensaver_lock;
   double           post_screensaver_time;

   /* Adv props */
   E_Desklock_Background_Method bg_method;
   int              bg_method_prev;
   Eina_List       *bgs;

   struct
   {
      Evas_Object *kbd_list;
      Evas_Object *loginbox_slider;
      Evas_Object *o_table;
      Eina_List *bgs;
   } gui;
};

E_Config_Dialog *
e_int_config_desklock(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "screen/screen_lock")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;
   v->basic.check_changed = _basic_check_changed;
   v->override_auto_apply = 1;

   cfd = e_config_dialog_new(NULL, _("Screen Lock Settings"), "E",
                             "screen/screen_lock", "preferences-system-lock-screen",
                             0, v, NULL);
   return cfd;
}

void
e_int_config_desklock_fsel_done(E_Config_Dialog *cfd, Evas_Object *bg, const char *bg_file, Eina_Bool hide_logo)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   E_Config_Desklock_Background *cbg;
   Eina_Stringshare *bg_name;
   Evas_Object *o;
   unsigned int n = 0;

   if (!(cfdata = cfd->cfdata)) return;
   cfdata->bg_fsel = NULL;
   if (!bg_file) return;
   e_widget_preview_file_get(bg, &bg_name, NULL);
   EINA_LIST_FOREACH(cfdata->gui.bgs, l, o)
     {
        if (o == bg) break;
        n++;
     }
   cbg = eina_list_nth(cfdata->bgs, n);
   if (!cbg) return; //wtf
   eina_stringshare_replace(&cbg->file, bg_file);
   cbg->hide_logo = hide_logo;
   evas_object_data_set(bg, "hide_logo", (void*)(long)cbg->hide_logo);
   e_widget_preview_edje_set(bg, bg_file, "e/desktop/background");
}

static E_Config_Desklock_Background *
desklock_bg_dup(const E_Config_Desklock_Background *cbg, const char *str)
{
   E_Config_Desklock_Background *ret;

   ret = E_NEW(E_Config_Desklock_Background, 1);
   if (cbg)
     {
        ret->file = eina_stringshare_ref(cbg->file);
        ret->hide_logo = cbg->hide_logo;
     }
   else
     ret->file = eina_stringshare_add(str);
   return ret;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   E_Config_Desklock_Background *bg;
   int x;

   cfdata->zone_count = eina_list_count(e_comp->zones);
   EINA_LIST_FOREACH(e_config->desklock_backgrounds, l, bg)
     cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(bg, NULL));
   if (!cfdata->bgs)
     for (x = 0; x < cfdata->zone_count; x++)
       cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(NULL, "theme_desklock_background"));

   bg = eina_list_data_get(cfdata->bgs);
   if (!e_util_strcmp(bg->file, "theme_desklock_background"))
     cfdata->bg_method = E_DESKLOCK_BACKGROUND_METHOD_THEME_DESKLOCK;
   else if (!e_util_strcmp(bg->file, "theme_background"))
     cfdata->bg_method = E_DESKLOCK_BACKGROUND_METHOD_THEME;
   else if (!e_util_strcmp(bg->file, "user_background"))
     cfdata->bg_method = E_DESKLOCK_BACKGROUND_METHOD_WALLPAPER;
   else
     cfdata->bg_method = E_DESKLOCK_BACKGROUND_METHOD_THEME_DESKLOCK;

   cfdata->bg_method_prev = cfdata->bg_method;
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     cfdata->use_xscreensaver = ecore_x_screensaver_event_available_get();
   else
#endif
     cfdata->use_xscreensaver = 1;

   cfdata->desklock_auth_method = e_config->desklock_auth_method;
   cfdata->desklock_layout = e_config->xkb.desklock_layout;
   cfdata->start_locked = e_config->desklock_start_locked;
   cfdata->lock_on_suspend = e_config->desklock_on_suspend;
   cfdata->screensaver_lock = e_config->desklock_autolock_screensaver;
   cfdata->post_screensaver_time = e_config->desklock_post_screensaver_time;
   if (e_config->desklock_login_box_zone >= 0)
     {
        cfdata->login_zone = 0;
        cfdata->zone = e_config->desklock_login_box_zone;
     }
   else
     {
        cfdata->login_zone = e_config->desklock_login_box_zone;
        cfdata->zone = 0;
     }
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
   E_Config_Desklock_Background *bg;
   if (cfdata->bg_fsel)
     e_object_del(E_OBJECT(cfdata->bg_fsel));
   E_FREE(cfdata->desklock_personal_passwd);
   E_FREE(cfdata->pin_str);
   EINA_LIST_FREE(cfdata->bgs, bg)
     {
        eina_stringshare_del(bg->file);
        free(bg);
     }
   free(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   E_Config_XKB_Layout *cl;
   int grp = 0;
   Evas_Object *otb, *ol, *ow, *of, *oc;
   const Eina_List *l;
   E_Zone *zone;
   E_Radio_Group *rg;
   int screen_count, x = 0;

   e_dialog_resizable_set(cfd->dia, 1);
   screen_count = eina_list_count(e_xinerama_screens_get());

   otb = e_widget_toolbook_add(evas, (24 * e_scale), (24 * e_scale));

   /* Locking */
   ol = e_widget_list_add(evas, 0, 0);
   ow = e_widget_check_add(evas, _("Lock on Startup"), &cfdata->start_locked);
   e_widget_disabled_set(ow, !cfdata->use_xscreensaver);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   ow = e_widget_check_add(evas, _("Lock on Suspend"), &cfdata->lock_on_suspend);
   e_widget_disabled_set(ow, !cfdata->use_xscreensaver);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   rg = e_widget_radio_group_new(&(cfdata->desklock_auth_method));
   ow = e_widget_radio_add(evas, _("Use System Authentication"), E_DESKLOCK_AUTH_METHOD_SYSTEM, rg);
   evas_object_smart_callback_add(ow, "changed", _login_method_change, cfdata);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   ow = e_widget_radio_add(evas, _("Use Personal Screenlock Password (insecure)"), E_DESKLOCK_AUTH_METHOD_PERSONAL, rg);
   evas_object_smart_callback_add(ow, "changed", _login_method_change, cfdata);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);
   ow = e_widget_radio_add(evas, _("Use PIN (insecure)"), E_DESKLOCK_AUTH_METHOD_PIN, rg);
   evas_object_smart_callback_add(ow, "changed", _login_method_change, cfdata);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Personal Screenlock Password (insecure)"), 0);
   cfdata->passwd_entry = ow = e_widget_entry_add(cfd->dia->win, &(cfdata->desklock_personal_passwd), NULL, NULL, NULL);
   e_widget_entry_password_set(ow, 1);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("PIN Entry (insecure)"), 0);
   cfdata->pin_entry = ow = e_widget_entry_add(cfd->dia->win, &(cfdata->pin_str), NULL, NULL, NULL);
   e_widget_entry_password_set(ow, 1);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   e_widget_disabled_set(cfdata->passwd_entry,
     (cfdata->desklock_auth_method != E_DESKLOCK_AUTH_METHOD_PERSONAL));
   e_widget_disabled_set(cfdata->pin_entry,
     (cfdata->desklock_auth_method != E_DESKLOCK_AUTH_METHOD_PIN));

   e_widget_toolbook_page_append(otb, NULL, _("Locking"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Keyboard Layout */
   cfdata->gui.kbd_list = ol = e_widget_ilist_add(evas, 40 * e_scale, 40 * e_scale, &cfdata->desklock_layout);
   EINA_LIST_FOREACH(e_config->xkb.used_layouts, l, cl)
     {
        Evas_Object *icon, *end;
        char buf[4096];
        const char *name = cl->name;

        end = edje_object_add(evas);
        if (e_theme_edje_object_set(end, "base/theme/widgets",
                                    "e/widgets/ilist/toggle_end"))
          {
             if (name == cfdata->desklock_layout)
               {
                  edje_object_signal_emit(end, "e,state,checked", "e");
                  e_widget_ilist_selected_set(ol, grp);
               }
             else
               edje_object_signal_emit(end, "e,state,unchecked", "e");
          }
        else
          {
             evas_object_del(end);
             end = NULL;
          }
        e_xkb_flag_file_get(buf, sizeof(buf), name);
        icon = e_icon_add(evas);
        e_icon_file_set(icon, buf);
        if (cl->variant)
          snprintf(buf, sizeof(buf), "%s (%s, %s)", cl->name, cl->model, cl->variant);
        else
          snprintf(buf, sizeof(buf), "%s (%s)", cl->name, cl->model);
        e_widget_ilist_append_full(ol, icon, end, buf, NULL, cfdata, cl->name);
        grp++;
     }
   e_widget_toolbook_page_append(otb, NULL, _("Keyboard Layout"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Login */
   ol = e_widget_list_add(evas, 0, 0);
   rg = e_widget_radio_group_new(&(cfdata->login_zone));
   ow = e_widget_radio_add(evas, _("Show on all screens"), -1, rg);
   e_widget_on_change_hook_set(ow, _cb_login_change, cfdata);
   e_widget_disabled_set(ow, (screen_count <= 0));
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Show on current screen"), -2, rg);
   e_widget_on_change_hook_set(ow, _cb_login_change, cfdata);
   e_widget_disabled_set(ow, (screen_count <= 0));
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   ow = e_widget_radio_add(evas, _("Show on screen #:"), 0, rg);
   e_widget_on_change_hook_set(ow, _cb_login_change, cfdata);
   e_widget_disabled_set(ow, (screen_count <= 0));
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   cfdata->gui.loginbox_slider =
     e_widget_slider_add(evas, 1, 0, _("%1.0f"), 0.0, (cfdata->zone_count - 1),
                         1.0, 0, NULL, &(cfdata->zone), 100);
   e_widget_disabled_set(cfdata->gui.loginbox_slider, (screen_count <= 0));
   e_widget_list_object_append(ol, cfdata->gui.loginbox_slider, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Login Box"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Timers */
   ol = e_widget_list_add(evas, 0, 0);
   oc = e_widget_check_add(evas, _("Lock after blanking"),
                           &cfdata->screensaver_lock);
   e_widget_disabled_set(oc, !cfdata->use_xscreensaver);
   e_widget_list_object_append(ol, oc, 1, 1, 0.5);

   ow = e_widget_slider_add(evas, 1, 0, _("%1.0f seconds"), 0.0, 300.0, 10.0, 0,
                            &(cfdata->post_screensaver_time), NULL, 100);
   e_widget_disabled_set(ow, !cfdata->use_xscreensaver);
   if (cfdata->use_xscreensaver)
     e_widget_check_widget_disable_on_unchecked_add(oc, ow);
   e_widget_list_object_append(ol, ow, 1, 1, 0.5);

   e_widget_toolbook_page_append(otb, NULL, _("Timers"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   /* Wallpapers */
   ol = e_widget_list_add(evas, 0, 0);
   of = e_widget_table_add(e_win_evas_win_get(evas), 1);
   rg = e_widget_radio_group_new((int *)&(cfdata->bg_method));
   ow = e_widget_radio_add(evas, _("Theme Defined"),
                           E_DESKLOCK_BACKGROUND_METHOD_THEME_DESKLOCK, rg);
   evas_object_smart_callback_add(ow, "changed", _cb_method_change, cfdata);
   e_widget_table_object_append(of, ow, 0, 0, 1, 1, 1, 0, 1, 0);
   ow = e_widget_radio_add(evas, _("Theme Wallpaper"),
                           E_DESKLOCK_BACKGROUND_METHOD_THEME, rg);
   evas_object_smart_callback_add(ow, "changed", _cb_method_change, cfdata);
   e_widget_table_object_append(of, ow, 0, 1, 1, 1, 1, 0, 1, 0);
   ow = e_widget_radio_add(evas, _("Current Wallpaper"),
                           E_DESKLOCK_BACKGROUND_METHOD_WALLPAPER, rg);
   evas_object_smart_callback_add(ow, "changed", _cb_method_change, cfdata);
   e_widget_table_object_append(of, ow, 1, 0, 1, 1, 1, 0, 1, 0);
   ow = e_widget_radio_add(evas, _("Custom"),
                           E_DESKLOCK_BACKGROUND_METHOD_CUSTOM, rg);
   evas_object_smart_callback_add(ow, "changed", _cb_method_change, cfdata);
   e_widget_table_object_append(of, ow, 1, 1, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   cfdata->gui.o_table = e_widget_table_add(e_win_evas_win_get(evas), 1);

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        ow = e_widget_preview_add(evas, 100, 140);
        cfdata->gui.bgs = eina_list_append(cfdata->gui.bgs, ow);
        evas_object_data_set(ow, "zone", zone);
        e_widget_disabled_set(ow,
          (cfdata->bg_method < E_DESKLOCK_BACKGROUND_METHOD_CUSTOM));
        evas_object_event_callback_add(ow, EVAS_CALLBACK_MOUSE_DOWN, _cb_bg_mouse_down, cfdata);
        e_widget_table_object_append(cfdata->gui.o_table, ow, x++, 0, 1, 1, 1, 1, 1, 1);
     }
   _cb_method_change(cfdata, NULL, NULL);
   e_widget_list_object_append(ol, cfdata->gui.o_table, 1, 1, 0.5);
   e_widget_toolbook_page_append(otb, NULL, _("Wallpaper"), ol,
                                 1, 1, 1, 0, 0.0, 0.0);

   e_widget_toolbook_page_show(otb, 0);

   return otb;
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   const Eina_List *l;
   E_Config_Desklock_Background *cbg;

   e_config->desklock_auth_method = cfdata->desklock_auth_method;
   if (e_config->desklock_auth_method == E_DESKLOCK_AUTH_METHOD_PERSONAL)
     {
        if ((!e_config->desklock_passwd) &&
            cfdata->desklock_personal_passwd && cfdata->desklock_personal_passwd[0])
          {
             e_config->desklock_passwd =
               e_auth_hash_djb2(cfdata->desklock_personal_passwd,
               strlen(cfdata->desklock_personal_passwd));
          }
     }
   else if (cfdata->desklock_auth_method == E_DESKLOCK_AUTH_METHOD_PIN)
     {
        if (cfdata->pin_str && cfdata->pin_str[0])
          {
             int test;
             char *pp;

             errno = 0;
             test = strtol(cfdata->pin_str, &pp, 10);
             if (errno) return 0; //NAN
             if (pp && pp[0]) return 0;
             if (test < 1) return 0;
             e_config->desklock_pin = e_auth_hash_djb2(cfdata->pin_str, strlen(cfdata->pin_str));
          }
        else
          /* dumb, but let them do what they want... */
          e_config->desklock_pin = 0;
     }
   e_config->desklock_start_locked = cfdata->start_locked;
   e_config->desklock_on_suspend = cfdata->lock_on_suspend;
   e_config->desklock_post_screensaver_time = cfdata->post_screensaver_time;
   e_config->desklock_autolock_screensaver = cfdata->screensaver_lock;
   if (e_config->xkb.desklock_layout != cfdata->desklock_layout)
     {
        e_config->xkb.desklock_layout = eina_stringshare_ref(cfdata->desklock_layout);
        if (cfdata->desklock_layout)
          {
             E_Ilist_Item *ili;

             EINA_LIST_FOREACH(e_widget_ilist_items_get(cfdata->gui.kbd_list), l, ili)
               {
                  if (ili->selected)
                    edje_object_signal_emit(ili->o_end, "e,state,checked", "e");
                  else
                    edje_object_signal_emit(ili->o_end, "e,state,unchecked", "e");
               }
          }
     }

   if (cfdata->bgs)
     {
        EINA_LIST_FREE(e_config->desklock_backgrounds, cbg)
          {
             e_filereg_deregister(cbg->file);
             eina_stringshare_del(cbg->file);
             free(cbg);
          }
        EINA_LIST_FOREACH(cfdata->bgs, l, cbg)
          {
             e_config->desklock_backgrounds = eina_list_append(e_config->desklock_backgrounds, desklock_bg_dup(cbg, NULL));
             e_filereg_register(cbg->file);
          }
     }

   if (cfdata->login_zone < 0)
     e_config->desklock_login_box_zone = cfdata->login_zone;
   else
     e_config->desklock_login_box_zone = cfdata->zone;

   cfdata->bg_method_prev = cfdata->bg_method;
   e_config_save_queue();
   return 1;
}

static int
_basic_check_changed(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Eina_List *l, *ll;
   E_Config_Desklock_Background *cbg;

   if (e_config->xkb.desklock_layout != cfdata->desklock_layout)
     return 1;
   if (e_config->desklock_start_locked != cfdata->start_locked)
     return 1;
   if (e_config->desklock_on_suspend != cfdata->lock_on_suspend)
     return 1;
   if (e_config->desklock_auth_method != cfdata->desklock_auth_method)
     return 1;
   if (e_config->desklock_auth_method == E_DESKLOCK_AUTH_METHOD_PERSONAL)
     {
        if ((!e_config->desklock_passwd) &&
            cfdata->desklock_personal_passwd && cfdata->desklock_personal_passwd[0])
          {
             if (e_config->desklock_passwd !=
               e_auth_hash_djb2(cfdata->desklock_personal_passwd,
               strlen(cfdata->desklock_personal_passwd)))
               return 1;
          }
     }
   if (e_config->desklock_auth_method == E_DESKLOCK_AUTH_METHOD_PIN)
     {
        if ((!e_config->desklock_pin) &&
            cfdata->pin_str && cfdata->pin_str[0])
          {
             if (e_config->desklock_pin !=
               e_auth_hash_djb2(cfdata->pin_str,
               strlen(cfdata->pin_str)))
               return 1;
          }
     }

   if (e_config->desklock_autolock_screensaver != cfdata->screensaver_lock)
     return 1;
   if (!EINA_DBL_EQ(e_config->desklock_post_screensaver_time, cfdata->post_screensaver_time))
     return 1;

   if (cfdata->bg_method_prev != (int)cfdata->bg_method) return 1;
   ll = cfdata->bgs;
   EINA_LIST_FOREACH(e_config->desklock_backgrounds, l, cbg)
     {
        E_Config_Desklock_Background *cbg2 = eina_list_data_get(ll);
        if (!cbg2) return 1;
        if (cbg->file != cbg2->file) return 1;
        if (cbg->hide_logo != cbg2->hide_logo) return 1;
        ll = eina_list_next(ll);
     }

   if (cfdata->login_zone < 0)
     {
        if (e_config->desklock_login_box_zone != cfdata->login_zone)
          return 1;
     }
   else
     {
        if (e_config->desklock_login_box_zone != cfdata->zone)
          return 1;
     }

   return 0;
}

static void
_cb_method_change(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   Evas_Object *bg;
   const char *theme = NULL;
   int x;
   E_Config_Desklock_Background *cbg;

   if (!(cfdata = data)) return;
   EINA_LIST_FOREACH(cfdata->gui.bgs, l, bg)
     e_widget_disabled_set(bg,
       (cfdata->bg_method < E_DESKLOCK_BACKGROUND_METHOD_CUSTOM));

   switch (cfdata->bg_method)
     {
      case E_DESKLOCK_BACKGROUND_METHOD_THEME_DESKLOCK:
        EINA_LIST_FREE(cfdata->bgs, cbg)
          {
             eina_stringshare_del(cbg->file);
             free(cbg);
          }
        for (x = 0; x < cfdata->zone_count; x++)
          cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(NULL, "theme_desklock_background"));
        theme = e_theme_edje_file_get("base/theme/desklock",
                                      "e/desklock/background");
        if (theme)
          EINA_LIST_FOREACH(cfdata->gui.bgs, l, bg)
            e_widget_preview_edje_set(bg, theme, "e/desklock/background");
        break;

      case E_DESKLOCK_BACKGROUND_METHOD_THEME:
        theme = e_theme_edje_file_get("base/theme/backgrounds",
                                      "e/desktop/background");
        if (theme)
          {
             EINA_LIST_FOREACH(cfdata->gui.bgs, l, bg)
               e_widget_preview_edje_set(bg, theme, "e/desktop/background");
             EINA_LIST_FREE(cfdata->bgs, cbg)
               {
                  eina_stringshare_del(cbg->file);
                  free(cbg);
               }
             for (x = 0; x < cfdata->zone_count; x++)
               cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(NULL, "theme_background"));
          }
        break;

      case E_DESKLOCK_BACKGROUND_METHOD_WALLPAPER:
        if (e_config->desktop_backgrounds)
          {
             E_Config_Desktop_Background *cdb;
             if (eina_str_has_extension(e_config->desktop_default_background, "edj"))
               {
                  EINA_LIST_FOREACH(cfdata->gui.bgs, l, bg)
                    e_widget_preview_edje_set(bg, e_config->desktop_default_background, "e/desktop/background");
                  EINA_LIST_FREE(cfdata->bgs, cbg)
                    {
                       eina_stringshare_del(cbg->file);
                       free(cbg);
                    }
               }
             /* attempt to set wallpaper from desktop 0,0 on each zone as a desklock bg */
             EINA_LIST_FOREACH(e_config->desktop_backgrounds, l, cdb)
               if ((!cdb->desk_x) && (!cdb->desk_y))
                 {
                    cbg = eina_list_nth(cfdata->bgs, cdb->zone);
                    if (cbg)
                      eina_stringshare_refplace(&cbg->file, cdb->file);
                    else
                      cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(NULL, cdb->file));
                    bg = eina_list_nth(cfdata->gui.bgs, cdb->zone);
                    if (bg)
                      e_widget_preview_edje_set(bg, cdb->file, "e/desktop/background");
                 }
          }
        else if (e_config->desktop_default_background && eina_str_has_extension(e_config->desktop_default_background, "edj"))
          {
             EINA_LIST_FOREACH(cfdata->gui.bgs, l, bg)
               e_widget_preview_edje_set(bg, e_config->desktop_default_background, "e/desktop/background");
             EINA_LIST_FREE(cfdata->bgs, cbg)
               {
                  eina_stringshare_del(cbg->file);
                  free(cbg);
               }
          }
        EINA_LIST_FREE(cfdata->bgs, cbg)
          {
             eina_stringshare_del(cbg->file);
             free(cbg);
          }
        for (x = 0; x < cfdata->zone_count; x++)
          cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(NULL, "user_background"));
        break;

      case E_DESKLOCK_BACKGROUND_METHOD_CUSTOM:
        {
             Eina_List *ll;

             EINA_LIST_FREE(cfdata->bgs, cbg)
               {
                  eina_stringshare_del(cbg->file);
                  free(cbg);
               }
             ll = cfdata->gui.bgs;
             EINA_LIST_FOREACH(e_config->desklock_backgrounds, l, cbg)
               {
                  if (!ll) break;
                  evas_object_data_set(ll->data, "hide_logo", (void*)(long)cbg->hide_logo);
                  e_widget_preview_edje_set(ll->data, cbg->file, "e/desktop/background");
                  cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(cbg, NULL));
                  ll = ll->next;
               }
             if (!cfdata->bgs)
               {
                  for (x = 0; x < cfdata->zone_count; x++)
                    cfdata->bgs = eina_list_append(cfdata->bgs, desklock_bg_dup(NULL, "theme_desklock_background"));
               }
        }
        break;

      default:
        break;
     }
}

static void
_login_method_change(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   e_widget_disabled_set(cfdata->passwd_entry,
     (cfdata->desklock_auth_method != E_DESKLOCK_AUTH_METHOD_PERSONAL));
   if (!e_widget_disabled_get(cfdata->passwd_entry))
     {
        e_widget_entry_select_all(cfdata->passwd_entry);
        e_widget_focus_set(cfdata->passwd_entry, 1);
     }
   e_widget_disabled_set(cfdata->pin_entry,
     (cfdata->desklock_auth_method != E_DESKLOCK_AUTH_METHOD_PIN));
   if (!e_widget_disabled_get(cfdata->pin_entry))
     {
        e_widget_entry_select_all(cfdata->pin_entry);
        e_widget_focus_set(cfdata->pin_entry, 1);
     }
}

static void
_cb_login_change(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;
   e_widget_disabled_set(cfdata->gui.loginbox_slider, (cfdata->login_zone < 0));
}

static void
_cb_bg_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (e_widget_disabled_get(obj)) return;
   if (!(cfdata = data)) return;
   if (cfdata->bg_fsel)
     elm_win_raise(cfdata->bg_fsel->dia->win);
   else
     cfdata->bg_fsel = e_int_config_desklock_fsel(cfdata->cfd, obj);
}

