#include "e.h"
#include "e_mod_main.h"

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _fill_data(E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   E_Config_Dialog *cfd;

   /* Basic */
   Evas_Object     *o_fm;
   Evas_Object     *o_up_button;
   Evas_Object     *o_preview;
   Evas_Object     *o_personal;
   Evas_Object     *o_system;
   int              fmdir;
   const char      *theme;
   Eio_File        *eio[2];
   Eio_File        *init[2];
   Eina_List       *theme_init; /* list of eio ops to load themes */
   Eina_List       *themes; /* eet file refs to work around load locking */
   Eina_Bool        free E_BITFIELD;

   /* Dialog */
   Evas_Object      *win_import;
   Ecore_Job *theme_check;
};

static void
_e_int_theme_preview_clear(Evas_Object *preview)
{
   Eina_List *objs = evas_object_data_get(preview, "objects");
   Evas_Object *o;

   e_widget_preview_extern_object_set(preview, NULL);
   EINA_LIST_FREE(objs, o) evas_object_del(o);
   evas_object_data_del(preview, "objects");
}

static void
_e_int_theme_edje_file_set(Evas_Object *o, const char *file, const char *group)
{
   if (!edje_object_file_set(o, file, group))
     e_theme_edje_object_set(o, NULL, group);
}

static Eina_Bool
_e_int_theme_preview_set(Evas_Object *preview, const char *file)
{
   Evas *e;
   Evas_Coord w = 320, h = 240, mw = 0, mh = 0;
   Eina_List *objs = NULL;
   Evas_Object *o, *po, *po2, *po3, *win, *parent;

   _e_int_theme_preview_clear(preview);
   e = e_widget_preview_evas_get(preview);
   parent = e_win_evas_win_get(e);
   win = parent;
   if (!win)
     win = elm_win_fake_add(ecore_evas_ecore_evas_get(e));
   evas_object_size_hint_min_get(preview, &w, &h);
   w *= 2; h *= 2;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/desktop/background");
   evas_object_move(o, 0, 0);
   evas_object_resize(o, w, h);
   evas_object_show(o);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/comp/frame/popup");
   evas_object_move(o, (w - (400 * e_scale)) / 2, h - (40 * e_scale));
   evas_object_resize(o, 400 * e_scale, (40 * e_scale));
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,shadow,on", "e");
   edje_object_signal_emit(o, "e,state,visible,on", "e");
   objs = eina_list_append(objs, o);
   po = o;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/shelf/default/base");
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,orientation,bottom", "e");
   edje_object_part_swallow(po, "e.swallow.content", o);
   objs = eina_list_append(objs, o);
   po = o;
   po2 = po;

   o = elm_box_add(win);
   elm_box_horizontal_set(o, 1);
   evas_object_show(o);
   edje_object_part_swallow(po, "e.swallow.content", o);
   objs = eina_list_append(objs, o);
   po = o;

   mh = 42 * e_scale;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/start/main");
   evas_object_show(o);
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/shelf/default/inset");
   evas_object_show(o);
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, 4 * mh, 0);
   objs = eina_list_append(objs, o);
   po2 = o;

   o = elm_box_add(win);
   elm_box_horizontal_set(o, 1);
   evas_object_show(o);
   edje_object_part_swallow(po2, "e.swallow.content", o);
   objs = eina_list_append(objs, o);
   po3 = o;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/pager/desk");
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,selected", "e");
   elm_box_pack_end(po3, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/pager/desk");
   evas_object_show(o);
   elm_box_pack_end(po3, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/pager/desk");
   evas_object_show(o);
   elm_box_pack_end(po3, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/pager/desk");
   evas_object_show(o);
   elm_box_pack_end(po3, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/backlight/main");
   evas_object_show(o);
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/mixer/main");
   evas_object_show(o);
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/battery/main");
   evas_object_show(o);
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/modules/clock/main");
   evas_object_show(o);
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, mh, 0);
   objs = eina_list_append(objs, o);


   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/comp/frame/default");
   evas_object_move(o, w / 2, h / 9);
   evas_object_resize(o, w / 3, h / 3);
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,shadow,on", "e");
   edje_object_signal_emit(o, "e,state,visible,on", "e");
   edje_object_signal_emit(o, "e,state,focus,off", "e");
   objs = eina_list_append(objs, o);
   po = o;
   po2 = po;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/widgets/border/default/border");
   edje_object_part_text_set(o, "e.text.title", "Title");
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,unfocused", "e");
   edje_object_part_swallow(po, "e.swallow.content", o);
   objs = eina_list_append(objs, o);
   po = o;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/theme/about");
   edje_object_size_min_get(o, &mw, &mh);
   if (mw > 0) evas_object_resize(po2, mw, mh);
   edje_object_part_text_set(o, "e.text.label", "Close");
   edje_object_part_text_set(o, "e.text.theme", "Select Theme");
   evas_object_show(o);
   edje_object_part_swallow(po, "e.swallow.client", o);
   objs = eina_list_append(objs, o);




   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/comp/frame/default");
   evas_object_move(o, w / 10, h / 5);
   evas_object_resize(o, w / 2, h / 3);
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,shadow,on", "e");
   edje_object_signal_emit(o, "e,state,visible,on", "e");
   edje_object_signal_emit(o, "e,state,focus,on", "e");
   objs = eina_list_append(objs, o);
   po = o;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/widgets/border/default/border");
   edje_object_part_text_set(o, "e.text.title", "Title");
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,focused", "e");
   edje_object_part_swallow(po, "e.swallow.content", o);
   objs = eina_list_append(objs, o);
   po = o;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/widgets/dialog/main");
   evas_object_show(o);
   edje_object_signal_emit(o, "e,icon,enabled", "e");
   edje_object_part_swallow(po, "e.swallow.client", o);
   objs = eina_list_append(objs, o);
   po = o;
   po2 = po;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/widgets/dialog/text");
   edje_object_part_text_set(o, "e.textblock.message",
                             "<hilight>Welcome to enlightenment.</hilight><ps/>"
                             "<ps/>"
                             "This is a sample set of content for a<ps/>"
                             "theme to test to see what it looks like.");
   evas_object_show(o);
   edje_object_part_swallow(po, "e.swallow.content", o);
   objs = eina_list_append(objs, o);

   o = e_icon_add(e);
   e_util_icon_theme_set(o, "dialog-warning");
   evas_object_show(o);
   evas_object_size_hint_min_set(o, 64 * e_scale, 64 * e_scale);
   edje_object_part_swallow(po, "e.swallow.icon", o);
   objs = eina_list_append(objs, o);

   o = elm_box_add(win);
   elm_box_horizontal_set(o, 1);
   elm_box_homogeneous_set(o, 1);
   evas_object_show(o);
   edje_object_part_swallow(po, "e.swallow.buttons", o);
   objs = eina_list_append(objs, o);
   po = o;

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/widgets/button");
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,text", "e");
   edje_object_part_text_set(o, "e.text.label", "OK");
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, 50, 20);
   objs = eina_list_append(objs, o);

   o = edje_object_add(e);
   _e_int_theme_edje_file_set(o, file, "e/widgets/button");
   evas_object_show(o);
   edje_object_signal_emit(o, "e,state,text", "e");
   edje_object_part_text_set(o, "e.text.label", "Cancel");
   E_FILL(o);
   elm_box_pack_end(po, o);
   evas_object_size_hint_min_set(o, 50, 20);
   objs = eina_list_append(objs, o);

   elm_box_recalculate(po);
   evas_object_size_hint_min_get(po, &mw, &mh);
   evas_object_size_hint_min_set(po, mw, mh);
   edje_object_part_swallow(po2, "e.swallow.buttons", po);

   evas_object_data_set(preview, "objects", objs);

//   e_widget_preview_edje_set(preview, file, "e/desktop/background");
   return EINA_TRUE;
}

E_Config_Dialog *
e_int_config_theme(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "appearance/theme")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->override_auto_apply = 1;
   cfd = e_config_dialog_new(NULL,
                             _("Theme Selector"),
                             "E", "appearance/theme",
                             "preferences-desktop-theme", 0, v, NULL);
   return cfd;
}

void
e_int_config_theme_import_done(E_Config_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = dia->cfdata;
   cfdata->win_import = NULL;
}

void
e_int_config_theme_update(E_Config_Dialog *dia, char *file)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = dia->cfdata;

   cfdata->fmdir = 1;
   e_widget_radio_toggle_set(cfdata->o_personal, 1);

   eina_stringshare_replace(&cfdata->theme, file);

   if (cfdata->o_fm)
     {
        ecore_file_mkpath(elm_theme_user_dir_get());
        e_widget_flist_path_set(cfdata->o_fm, elm_theme_user_dir_get(), "/");
     }

   if (cfdata->o_preview)
     _e_int_theme_preview_set(cfdata->o_preview, cfdata->theme);
   if (cfdata->o_fm) e_widget_change(cfdata->o_fm);
}

static Eina_Bool
_eio_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file)
{
   return eina_str_has_extension(file, ".edj");
}

static void
_cb_button_up(void *data1, void *data2 EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data1;
   if (cfdata->o_fm) e_widget_flist_parent_go(cfdata->o_fm);
}

static void
_cb_files_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   if (!cfdata->o_fm) return;
   if (cfdata->o_up_button)
     e_widget_disabled_set(cfdata->o_up_button,
                           !e_widget_flist_has_parent_get(cfdata->o_fm));
}

static void
_cb_files_selection_change(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *selected;
   E_Fm2_Icon_Info *ici;
   const char *real_path;
   char buf[4096];

   cfdata = data;
   if (!cfdata->o_fm) return;

   if (!(selected = e_widget_flist_selected_list_get(cfdata->o_fm))) return;

   ici = selected->data;
   real_path = e_widget_flist_real_path_get(cfdata->o_fm);

   if (!strcmp(real_path, "/"))
     snprintf(buf, sizeof(buf), "/%s", ici->file);
   else
     snprintf(buf, sizeof(buf), "%s/%s", real_path, ici->file);
   eina_list_free(selected);

   if (ecore_file_is_dir(buf)) return;

   eina_stringshare_del(cfdata->theme);
   cfdata->theme = eina_stringshare_add(buf);
   if (cfdata->o_preview)
     _e_int_theme_preview_set(cfdata->o_preview, buf);
   if (cfdata->o_fm) e_widget_change(cfdata->o_fm);
}

#if 0
/* FIXME unused */
static void
_cb_files_selected(void *data, Evas_Object *obj, void *event_info)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
}

#endif

static void
_cb_files_files_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   const char *p;
   char buf[PATH_MAX];
   size_t len;

   cfdata = data;
   if ((!cfdata->theme) || (!cfdata->o_fm)) return;

   p = e_widget_flist_real_path_get(cfdata->o_fm);
   if (p)
     {
        if (strncmp(p, cfdata->theme, strlen(p))) return;
     }
   if (!p) return;

   snprintf(buf, sizeof(buf), "%s", elm_theme_user_dir_get());
   len = strlen(buf);
   if (!strncmp(cfdata->theme, buf, len))
     p = cfdata->theme + len + 1;
   else
     {
        snprintf(buf, sizeof(buf), "%s", elm_theme_system_dir_get());
        len = strlen(buf);
        if (!strncmp(cfdata->theme, buf, len))
          p = cfdata->theme + len + 1;
        else
          p = cfdata->theme;
     }
   e_widget_flist_select_set(cfdata->o_fm, p, 1);
   e_widget_flist_file_show(cfdata->o_fm, p);
}

static void
_cb_dir(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   char path[PATH_MAX];

   cfdata = data;
   if (cfdata->fmdir == 1)
     snprintf(path, sizeof(path), "%s", elm_theme_system_dir_get());
   else
     {
        snprintf(path, sizeof(path), "%s", elm_theme_user_dir_get());
        ecore_file_mkpath(path);
     }
   e_widget_flist_path_set(cfdata->o_fm, path, "/");
}

static void
_cb_files_files_deleted(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *sel, *all, *n;
   E_Fm2_Icon_Info *ici, *ic;

   cfdata = data;
   if ((!cfdata->theme) || (!cfdata->o_fm)) return;

   if (!(all = e_widget_flist_all_list_get(cfdata->o_fm))) return;
   if (!(sel = e_widget_flist_selected_list_get(cfdata->o_fm))) return;

   ici = sel->data;

   all = eina_list_data_find_list(all, ici);
   n = eina_list_next(all);
   if (!n)
     {
        n = eina_list_prev(all);
        if (!n) return;
     }

   if (!(ic = n->data)) return;

   e_widget_flist_select_set(cfdata->o_fm, ic->file, 1);
   e_widget_flist_file_show(cfdata->o_fm, ic->file);

   eina_list_free(n);

   evas_object_smart_callback_call(cfdata->o_fm, "selection_change", cfdata);
}

static void
_cb_import_online(void *data1 EINA_UNUSED, void *data2 EINA_UNUSED)
{
   Efreet_Desktop *desktop;
   E_Zone *zone;

   desktop = efreet_util_desktop_file_id_find("extra.desktop");
   if (!desktop)
   {
      e_util_dialog_internal
         (_("Missing Application"),
         _("This module wants to execute an external application<ps/> "
            "that does not exist.<ps/>"
            "Please install <b>extra</b> application.<ps/>"
            "https://git.enlightenment.org/apps/extra.git/"));
      return;
   }

   zone = e_zone_current_get();

   e_exec(zone, desktop, NULL, NULL, "extra/app");
   efreet_desktop_free(desktop);
}

static void
_cb_import(void *data1, void *data2 EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data1;
   if (cfdata->win_import)
     elm_win_raise(cfdata->win_import);
   else
     cfdata->win_import = e_int_config_theme_import(cfdata->cfd);
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   const char *theme;
   char path[PATH_MAX];
   size_t len;

   theme = elm_theme_get(NULL);
   if (theme)
     {
        snprintf(path, sizeof(path), "%s.edj", theme);
        cfdata->theme = eina_stringshare_add(path);
     }
   else
     {
        snprintf(path, sizeof(path), "%s/%s", elm_theme_system_dir_get(),
                 "default.edj");
        cfdata->theme = eina_stringshare_add(path);
     }
   if (cfdata->theme[0] != '/')
     {
        snprintf(path, sizeof(path), "%s/%s", elm_theme_user_dir_get(),
                cfdata->theme);
        if (ecore_file_exists(path))
          eina_stringshare_replace(&cfdata->theme, path);
        else
          {
             snprintf(path, sizeof(path), "%s/%s", elm_theme_system_dir_get(),
                      cfdata->theme);
             if (ecore_file_exists(path))
               eina_stringshare_replace(&cfdata->theme, path);
          }
     }
   snprintf(path, sizeof(path), "%s", elm_theme_system_dir_get());
   len = strlen(path);
   if (!strncmp(cfdata->theme, path, len))
     cfdata->fmdir = 1;
}

static void
_open_test_cb(void *data)
{
   E_Config_Dialog_Data *cfdata = data;
   Eina_List *l, *fails = NULL;
   Eet_File *file;
   Eina_Strbuf *buf;

   cfdata->theme_check = NULL;
   EINA_LIST_FOREACH(cfdata->themes, l, file)
     if (!edje_file_group_exists(eet_file_get(file), "e/desktop/background"))
       fails = eina_list_append(fails, file);
   if (!fails) return;
   buf = eina_strbuf_new();
   EINA_LIST_FREE(fails, file)
     eina_strbuf_append_printf(buf, "<b>%s</b><ps/>", eet_file_get(file));
   e_util_dialog_show(_("Theme File Error"),
                      _("The listed files are probably not E themes:<ps/>%s"),
                      eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static void
_open_done_cb(void *data, Eio_File *handler, Eet_File *file)
{
   E_Config_Dialog_Data *cfdata = data;
   cfdata->themes = eina_list_append(cfdata->themes, file);
   cfdata->theme_init = eina_list_remove(cfdata->theme_init, handler);
   if (!cfdata->theme_init)
     cfdata->theme_check = ecore_job_add(_open_test_cb, cfdata);
}

static void
_open_error_cb(void *data, Eio_File *handler, int error EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;
   cfdata->theme_init = eina_list_remove(cfdata->theme_init, handler);
   if (cfdata->free) _free_data(NULL, cfdata);
}

static void
_init_main_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   E_Config_Dialog_Data *cfdata = data;
   cfdata->theme_init = eina_list_append(cfdata->theme_init, eio_eet_open(file, EET_FILE_MODE_READ, _open_done_cb, _open_error_cb, cfdata));
}

static void
_init_done_cb(void *data, Eio_File *handler)
{
   E_Config_Dialog_Data *cfdata = data;
   if (cfdata->init[0] == handler)
     cfdata->init[0] = NULL;
   else
     cfdata->init[1] = NULL;
   if (cfdata->free) _free_data(NULL, cfdata);
}

static void
_init_error_cb(void *data, Eio_File *handler, int error EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;
   if (cfdata->init[0] == handler)
     cfdata->init[0] = NULL;
   else
     cfdata->init[1] = NULL;
   if (cfdata->free) _free_data(NULL, cfdata);
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfd->cfdata = cfdata;
   cfdata->cfd = cfd;
   _fill_data(cfdata);
   /* Grab the "Personal" themes. */
   cfdata->init[0] = eio_file_ls(elm_theme_user_dir_get(), _eio_filter_cb, _init_main_cb, _init_done_cb, _init_error_cb, cfdata);

   /* Grab the "System" themes. */
   cfdata->init[1] = eio_file_ls(elm_theme_system_dir_get(), _eio_filter_cb, _init_main_cb, _init_done_cb, _init_error_cb, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   Eio_File *ls;
   Eet_File *ef;

   E_FREE_FUNC(cfdata->win_import, evas_object_del);
   if (cfdata->eio[0]) eio_file_cancel(cfdata->eio[0]);
   if (cfdata->eio[1]) eio_file_cancel(cfdata->eio[1]);
   EINA_LIST_FOREACH(cfdata->theme_init, l, ls)
     eio_file_cancel(ls);
   EINA_LIST_FREE(cfdata->themes, ef)
     eet_close(ef);
   E_FREE_FUNC(cfdata->theme_check, ecore_job_del);
   if (cfdata->eio[0] || cfdata->eio[1] || cfdata->themes || cfdata->theme_init)
     cfdata->free = EINA_TRUE;
   else
     E_FREE(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ot, *of, *il, *ol, *parent;
   E_Zone *z;
   E_Radio_Group *rg;
   char path[PATH_MAX];

   parent = e_win_evas_win_get(evas);

   z = e_zone_current_get();
   e_dialog_resizable_set(cfd->dia, 1);

   ot = e_widget_table_add(parent, 0);
   ol = e_widget_table_add(parent, 0);
   il = e_widget_table_add(parent, 0);

   rg = e_widget_radio_group_new(&(cfdata->fmdir));
   o = e_widget_radio_add(evas, _("Personal"), 0, rg);
   cfdata->o_personal = o;
   evas_object_smart_callback_add(o, "changed", _cb_dir, cfdata);
   e_widget_table_object_append(il, o, 0, 0, 1, 1, 1, 1, 0, 0);
   o = e_widget_radio_add(evas, _("System"), 1, rg);
   cfdata->o_system = o;
   evas_object_smart_callback_add(o, "changed", _cb_dir, cfdata);
   e_widget_table_object_append(il, o, 1, 0, 1, 1, 1, 1, 0, 0);
      o = e_widget_button_add(evas, _("Go up a Directory"), "go-up",
                           _cb_button_up, cfdata, NULL);
   cfdata->o_up_button = o;
   e_widget_table_object_append(il, o, 2, 0, 1, 1, 1, 1, 0, 0);

   e_widget_table_object_align_append(ol, il, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0);


   if (cfdata->fmdir == 1)
     snprintf(path, sizeof(path), "%s", elm_theme_system_dir_get());
   else
     {
        snprintf(path, sizeof(path), "%s", elm_theme_user_dir_get());
        ecore_file_mkpath(path);
     }

   o = e_widget_flist_add(evas);
   cfdata->o_fm = o;
   {
      E_Fm2_Config *cfg;
      cfg = e_widget_flist_config_get(o);
      cfg->view.no_click_rename = 1;
   }
   evas_object_smart_callback_add(o, "dir_changed",
                                  _cb_files_changed, cfdata);
   evas_object_smart_callback_add(o, "selection_change",
                                  _cb_files_selection_change, cfdata);
   evas_object_smart_callback_add(o, "changed",
                                  _cb_files_files_changed, cfdata);
   evas_object_smart_callback_add(o, "files_deleted",
                                  _cb_files_files_deleted, cfdata);
   e_widget_flist_path_set(o, path, "/");

   e_widget_size_min_set(o, 160, 160);
   e_widget_table_object_append(ol, o, 0, 2, 1, 1, 1, 1, 1, 1);
   e_widget_table_object_append(ot, ol, 0, 0, 1, 1, 1, 1, 0, 1);

   of = e_widget_list_add(evas, 0, 0);

   il = e_widget_table_add(evas, 0);

   o = e_widget_button_add(evas, _(" Import File..."), "preferences-desktop-theme",
                           _cb_import, cfdata, NULL);
   e_widget_table_object_align_append(il, o, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0);

   if (efreet_util_desktop_file_id_find("extra.desktop"))
   {
      o = e_widget_button_add(evas, _(" Import Online..."), "preferences-desktop-theme",
                              _cb_import_online, NULL, NULL);
      e_widget_table_object_align_append(il, o, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0);
   }

   e_widget_list_object_append(of, il, 0, 0, 0.0);

   {
      Evas_Object *oa;
      int mw, mh;

      mw = 500;
      mh = (mw * z->h) / z->w;
      oa = e_widget_aspect_add(evas, mw, mh);
      o = e_widget_preview_add(evas, mw, mh);
      evas_object_size_hint_min_set(o, mw, mh);
      cfdata->o_preview = o;
      if (cfdata->theme)
        _e_int_theme_preview_set(o, cfdata->theme);
      e_widget_aspect_child_set(oa, o);
      e_widget_list_object_append(of, oa, 1, 1, 0);
      evas_object_show(o);
      evas_object_show(oa);
   }
   e_widget_table_object_append(ot, of, 1, 0, 1, 1, 1, 1, 1, 1);

   return ot;
}

static int
_basic_apply_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_Action *a;
   const char *file;
   char *name;

   file = ecore_file_file_get(cfdata->theme);
   name = ecore_file_strip_ext(file);
   if (name)
     {
        const char *theme = elm_theme_get(NULL);

        if (e_util_strcmp(name, theme))
          {
             elm_theme_set(NULL, name);
             elm_config_all_flush();
             elm_config_save();
             free(name);
             name = NULL;
             a = e_action_find("restart");
             if ((a) && (a->func.go)) a->func.go(NULL, NULL);
          }
        free(name);
     }
   return 1; /* Apply was OK */
}
