#include "e.h"

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _apply_cfdata(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static void         _ilist_fill(E_Config_Dialog_Data *cfdata);
static void         _ilist_cb_selected(void *data);
static void         _cb_add(void *data, void *data2);
static void         _cb_delete(void *data, void *data2);
static void         _cb_scratch(void *data, void *data2);
static void         _cb_reset(void *data, void *data2);
static void         _cb_dialog_yes(void *data);
static void         _cb_dialog_destroy(void *data);

static void         _new_profile_cb_ok(void *data, char *text);
static void         _new_profile_cb_dia_del(void *obj);

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;
   Evas_Object     *o_list;
   Evas_Object     *o_delete;
   Evas_Object     *o_scratch;
   Evas_Object     *o_reset;
   Evas_Object     *o_text;
   Evas_Object     *o_textlabel;
   const char      *sel_profile;

   E_Entry_Dialog *dia_new_profile;
};

typedef struct _Del_Profile_Confirm_Data Del_Profile_Confirm_Data;
struct _Del_Profile_Confirm_Data
{
   E_Config_Dialog_Data *cfdata;
};

E_Config_Dialog *
e_int_config_profiles(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "settings/profiles")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _apply_cfdata;
   v->basic.create_widgets = _create_widgets;

   cfd = e_config_dialog_new(NULL, _("Profile Selector"),
                             "E", "settings/profiles",
                             "preferences-profiles", 0, v, NULL);
   e_config_dialog_changed_auto_set(cfd, 0);
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->cfd = cfd;
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_FREE(cfdata);
}

static int
_apply_cfdata(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   const char *cur_profile;
   E_Action *a;

   cur_profile = e_config_profile_get();
   if (strcmp(cur_profile, cfdata->sel_profile) == 0)
     return 1;

   e_config_save_flush();
   e_config_profile_set(cfdata->sel_profile);
   elm_config_profile_set(cfdata->sel_profile);
   e_config_profile_save();
   e_config_save_block_set(1);

   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
   return 1;
}

static Evas_Object *
_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ot, *ob;
   Evas_Coord mw, mh;
   char buf[PATH_MAX];

   e_dialog_resizable_set(cfd->dia, 1);
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Available Profiles"), 0);
   cfdata->o_list = e_widget_ilist_add(evas, 24, 24, &(cfdata->sel_profile));
   e_widget_size_min_set(cfdata->o_list, 140 * e_scale, 80 * e_scale);
   e_widget_framelist_object_append(of, cfdata->o_list);

   cfdata->o_textlabel = e_widget_label_add(evas, "");
   e_widget_size_min_get(cfdata->o_textlabel, &mw, &mh);
   e_widget_framelist_object_append_full(of, cfdata->o_textlabel,
                                         1, 1, /* fill */
                                         1, 0, /* expand */
                                         0.5, 0.5, /* align */
                                         mw, mh, /* min */
                                         99999, 99999 /* max */
                                         );

   ob = e_widget_textblock_add(evas);
   e_widget_textblock_markup_set(ob, _("Select a profile"));
   cfdata->o_text = ob;
   e_widget_framelist_object_append_full(of, cfdata->o_text,
                                         1, 1, /* fill */
                                         1, 0, /* expand */
                                         0.5, 0.5, /* align */
                                         140 * e_scale, 60 * e_scale, /* min */
                                         99999, 99999 /* max */
                                         );

   ot = e_widget_table_add(e_win_evas_win_get(evas), 0);
   ob = e_widget_button_add(evas, _("Add"), "list-add", _cb_add, cfdata, NULL);
   e_widget_table_object_append(ot, ob, 0, 0, 1, 1, 1, 1, 0, 0);
   cfdata->o_delete = e_widget_button_add(evas, _("Delete"), "list-remove",
                                          _cb_delete, cfdata, NULL);
   e_widget_table_object_append(ot, cfdata->o_delete, 1, 0, 1, 1, 1, 1, 0, 0);
   cfdata->o_scratch = e_widget_button_add(evas, _("Scratch"), "system-restart",
                                           _cb_scratch, cfdata, NULL);
   e_widget_table_object_align_append(ot, cfdata->o_scratch, 2, 0, 1, 1, 0, 1, 1, 1, 0.5, 0.5);
   cfdata->o_reset = e_widget_button_add(evas, _("Reset"), "system-restart",
                                         _cb_reset, cfdata, NULL);
   e_widget_table_object_align_append(ot, cfdata->o_reset, 3, 0, 1, 1, 0, 1, 1, 1, 1.0, 0.5);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   // if there is a system version of the profile - allow reset
   e_prefix_data_snprintf(buf, sizeof(buf), "data/config/%s/",
                          e_config_profile_get());
   if (ecore_file_is_dir(buf))
     e_widget_disabled_set(cfdata->o_reset, 0);
   else
     e_widget_disabled_set(cfdata->o_reset, 1);

   e_widget_disabled_set(cfdata->o_delete, 1);

   e_widget_list_object_append(o, ot, 1, 0, 0.0);

   _ilist_fill(cfdata);

   return o;
}

static void
_ilist_fill(E_Config_Dialog_Data *cfdata)
{
   Evas *evas;
   Eina_List *l, *profiles;
   int selected = -1, i;

   if (!cfdata) return;
   if (!cfdata->o_list) return;

   evas = evas_object_evas_get(cfdata->o_list);
   evas_event_freeze(evas);
   edje_freeze();
   e_widget_ilist_freeze(cfdata->o_list);

   e_widget_ilist_clear(cfdata->o_list);
   e_widget_ilist_go(cfdata->o_list);

   profiles = e_config_profile_list();
   for (i = 0, l = profiles; l; l = l->next, i++)
     {
        Efreet_Desktop *desk = NULL;
        Evas_Object *ic;
        char buf[PATH_MAX], *prof, *pdir;
        const char *label;

        prof = l->data;
        if (e_config_profile_get())
          {
             if (!strcmp(prof, e_config_profile_get())) selected = i;
          }
        pdir = e_config_profile_dir_get(prof);
        snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
        desk = efreet_desktop_new(buf);
        if (!desk)
          {
             e_prefix_data_snprintf(buf, sizeof(buf), "data/config/%s/", prof);
             free(pdir);
             pdir = strdup(buf);
             if (pdir)
               {
                  snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
                  desk = efreet_desktop_new(buf);
               }
          }
        label = prof;
        if ((desk) && (desk->name)) label = desk->name;
        buf[0] = 0;
        if (pdir)
          snprintf(buf, sizeof(buf), "%s/icon.edj", pdir);
        if ((desk) && (desk->icon) && (pdir))
          {
             if (eina_str_has_extension(desk->icon, "png"))
               snprintf(buf, sizeof(buf), "%s/%s", pdir, desk->icon);
             else
               snprintf(buf, sizeof(buf), "%s/%s.png", pdir, desk->icon);
          }
        else
          e_prefix_data_concat_static(buf, "data/images/enlightenment.png");
        ic = e_util_icon_add(buf, evas);
        e_widget_ilist_append(cfdata->o_list, ic, label, _ilist_cb_selected, cfdata, prof);
        free(prof);
        free(pdir);
        if (desk) efreet_desktop_free(desk);
     }
   if (profiles) eina_list_free(profiles);
   if (selected >= 0)
     e_widget_ilist_selected_set(cfdata->o_list, selected);
   e_widget_size_min_set(cfdata->o_list, 155, 250);
   e_widget_ilist_go(cfdata->o_list);

   e_widget_ilist_thaw(cfdata->o_list);
   edje_thaw();
   evas_event_thaw(evas);
}

static void
_ilist_cb_selected(void *data)
{
   E_Config_Dialog_Data *cfdata;
   const char *cur_profile;
   unsigned char v;
   Efreet_Desktop *desk = NULL;
   char *pdir, buf[PATH_MAX];
   const char *name;

   cfdata = data;
   if (!cfdata) return;

   cur_profile = e_config_profile_get();

   v = (strcmp(cur_profile, cfdata->sel_profile) == 0);
   e_widget_disabled_set(cfdata->o_delete, v);
   e_config_dialog_changed_set(cfdata->cfd, !v);

   pdir = e_config_profile_dir_get(cfdata->sel_profile);
   snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
   desk = efreet_desktop_new(buf);
   if (desk)
     {
        // if there is a system version of the profile - allow reset
        e_prefix_data_snprintf(buf, sizeof(buf), "data/config/%s/", e_config_profile_get());
        if (ecore_file_is_dir(buf))
          e_widget_disabled_set(cfdata->o_reset, 0);
        else
          e_widget_disabled_set(cfdata->o_reset, 1);
     }
   else
     {
        e_prefix_data_snprintf(buf, sizeof(buf), "data/config/%s/", cfdata->sel_profile);
        if (ecore_file_is_dir(buf))
          e_widget_disabled_set(cfdata->o_reset, 0);
        else
          e_widget_disabled_set(cfdata->o_reset, 1);

        free(pdir);
        pdir = strdup(buf);
        if (pdir)
          {
             snprintf(buf, sizeof(buf), "%s/profile.desktop", pdir);
             desk = efreet_desktop_new(buf);
          }
     }

   if ((desk) && (desk->name))
     name = desk->name;
   else
     name = cur_profile;
   snprintf(buf, sizeof(buf), _("Selected profile: %s"), name);
   e_widget_label_text_set(cfdata->o_textlabel, buf);

   if (desk)
     e_widget_textblock_markup_set(cfdata->o_text, desk->comment);
   else
     e_widget_textblock_markup_set(cfdata->o_text, _("Unknown"));
   if (desk) efreet_desktop_free(desk);
   free(pdir);
}

static void
_cb_add(void *data, void *data2 EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   if (!cfdata) return;

   if (cfdata->dia_new_profile)
     elm_win_raise(cfdata->dia_new_profile->dia->win);
   else
     {
        cfdata->dia_new_profile =
          e_entry_dialog_show(NULL, _("Add New Profile"), NULL,
                              _("Name:"), NULL, NULL, NULL,
                              _new_profile_cb_ok,
                              NULL, cfdata);
        e_object_data_set(E_OBJECT(cfdata->dia_new_profile), cfdata);
        e_object_del_attach_func_set(E_OBJECT(cfdata->dia_new_profile), _new_profile_cb_dia_del);
     }
}

static void
_cb_delete(void *data, void *data2 EINA_UNUSED)
{
   Del_Profile_Confirm_Data *d;
   char buf[4096];

   if (!data) return;

   d = E_NEW(Del_Profile_Confirm_Data, 1);
   d->cfdata = data;

   snprintf(buf, sizeof(buf),
            _("You want to delete the \"%s\" profile.<ps/><ps/>"
              "Are you sure?"), d->cfdata->sel_profile);
   e_confirm_dialog_show(_("Are you sure you want to delete this profile?"),
                         "dialog-warning", buf, _("Delete"), _("Keep"),
                         _cb_dialog_yes, NULL, d, NULL,
                         _cb_dialog_destroy, d);
}

static void
_cb_scratch(void *data EINA_UNUSED, void *data2 EINA_UNUSED)
{
   E_Action *a;
   char *pdir;

   e_config_save_flush();
   e_config_save_block_set(1);

   pdir = e_config_profile_dir_get(e_config_profile_get());
   if (pdir)
     {
        ecore_file_recursive_rm(pdir);
        free(pdir);
     }
   e_config_profile_set("default");
   e_config_profile_save();
   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
}

static void
_cb_reset(void *data EINA_UNUSED, void *data2 EINA_UNUSED)
{
   E_Action *a;
   char *pdir;

   e_config_save_flush();
   e_config_save_block_set(1);

   pdir = e_config_profile_dir_get(e_config_profile_get());
   if (pdir)
     {
        ecore_file_recursive_rm(pdir);
        free(pdir);
     }
   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
}

static void
_cb_dialog_yes(void *data)
{
   Del_Profile_Confirm_Data *d;
   Efreet_Desktop *desk = NULL;
   char *pdir, buff[PATH_MAX];
   const char *label;

   d = data;
   if (!data) return;

   e_config_profile_del(d->cfdata->sel_profile);

   pdir = e_config_profile_dir_get(d->cfdata->sel_profile);
   snprintf(buff, sizeof(buff), "%s/profile.desktop", pdir);
   desk = efreet_desktop_new(buff);
   if (!desk)
     {
        e_prefix_data_snprintf(buff, sizeof(buff),
                               "data/config/%s/", d->cfdata->sel_profile);
        free(pdir);
        pdir = strdup(buff);
        if (pdir)
          {
             snprintf(buff, sizeof(buff), "%s/profile.desktop", pdir);
             desk = efreet_desktop_new(buff);
          }
     }

   label = d->cfdata->sel_profile;
   if ((desk) && (desk->name)) label = desk->name;

   free(pdir);
   if (desk) efreet_desktop_free(desk);

   e_action_predef_name_del("Profile: Switch", label);

   e_config_save_queue();
   _ilist_fill(d->cfdata);
}

static void
_cb_dialog_destroy(void *data)
{
   Del_Profile_Confirm_Data *d;

   d = data;
   if (!data) return;

   E_FREE(d);
}

static void
_new_profile_cb_ok(void *data, char *text)
{
   E_Config_Dialog_Data *cfdata;
   char cur_profile[1024];

   cfdata = data;
   if (!cfdata) return;

   snprintf(cur_profile, sizeof(cur_profile), "%s", e_config_profile_get());

   if (text && text[0])
     {
        e_config_profile_add(text);
        e_config_profile_set(text);
        e_config_save();
        e_config_profile_set(cur_profile);

        if (!elm_config_profile_exists(cur_profile))
          elm_config_profile_save(cur_profile);
     }

   _ilist_fill(cfdata);
}

static void
_new_profile_cb_dia_del(void *data)
{
   E_Config_Dialog_Data *cfdata = e_object_data_get(data);

   cfdata->dia_new_profile = NULL;
}

