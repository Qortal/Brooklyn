#include "e.h"
#include "e_mod_main.h"

typedef struct _Import Import;

struct _Import
{
   E_Config_Dialog      *parent;
   E_Config_Dialog_Data *cfdata;

   Evas_Object          *bg_obj;
   Evas_Object          *box_obj;
   Evas_Object          *content_obj;
   Evas_Object          *event_obj;
   Evas_Object          *fsel_obj;

   Evas_Object          *ok_obj;
   Evas_Object          *cancel_obj;

   Evas_Object          *win;
};

struct _E_Config_Dialog_Data
{
   char *file;
};

static void _imc_import_cb_delete(void *data EINA_UNUSED, Evas *e, Evas_Object *obj, void *event_info);
static void _imc_import_cb_wid_focus(void *data, Evas_Object *obj);
static void _imc_import_cb_selected(void *data, Evas_Object *obj);
static void _imc_import_cb_changed(void *data, Evas_Object *obj);
static void _imc_import_cb_ok(void *data, void *data2);
static void _imc_import_cb_close(void *data, void *data2);
static void _imc_import_cb_key_down(void *data, Evas *e, Evas_Object *obj,
                                    void *event);

Evas_Object *
e_int_config_imc_import(E_Config_Dialog *parent)
{
   Evas *evas;
   Evas_Object *win;
   Evas_Object *o, *ofm;
   Import *import;
   E_Config_Dialog_Data *cfdata;
   Evas_Modifier_Mask mask;
   Evas_Coord w, h;
   Eina_Bool kg;

   import = E_NEW(Import, 1);
   if (!import) return NULL;

   win = elm_win_add(parent->parent, "E", ELM_WIN_DIALOG_BASIC);
   if (!win)
     {
        E_FREE(import);
        return NULL;
     }

   evas = evas_object_evas_get(win);

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   import->cfdata = cfdata;
   import->win = win;
   import->parent = parent;

   elm_win_title_set(win, _("Select an Input Method Settings..."));
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _imc_import_cb_delete, NULL);
   ecore_evas_name_class_set(ecore_evas_ecore_evas_get(evas_object_evas_get(win)), "E", "_imc_import_dialog");

   o = elm_layout_add(win);
   E_EXPAND(o);
   E_FILL(o);
   import->bg_obj = o;
   e_theme_edje_object_set(o, "base/theme/dialog", "e/widgets/dialog/main");
   elm_win_resize_object_add(win, o);
   evas_object_show(o);

   o = e_widget_list_add(evas, 1, 1);
   e_widget_on_focus_hook_set(o, _imc_import_cb_wid_focus, import);
   import->box_obj = o;
   elm_object_part_content_set(import->bg_obj, "e.swallow.buttons", o);

   o = evas_object_rectangle_add(evas);
   import->event_obj = o;
   mask = 0;
   kg = evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Tab\" key events to object %p.\n", o);
   mask = evas_key_modifier_mask_get(evas, "Shift");
   kg = evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Tab\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "Return", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Return\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "KP_Enter", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"KP_Enter\" key events to object %p.\n", o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN,
                                  _imc_import_cb_key_down, import);

   o = e_widget_list_add(evas, 0, 0);
   import->content_obj = o;

   ofm = e_widget_fsel_add(evas, e_user_homedir_get(), "/",
                           NULL, NULL,
                           _imc_import_cb_selected, import,
                           _imc_import_cb_changed, import, 1);
   import->fsel_obj = ofm;
   e_widget_list_object_append(o, ofm, 1, 1, 0.5);

   e_widget_size_min_get(o, &w, &h);
   evas_object_size_hint_min_set(o, w, h);
   elm_object_part_content_set(import->bg_obj, "e.swallow.content", o);
   evas_object_show(o);

   import->ok_obj = e_widget_button_add(evas, _("OK"), NULL,
                                        _imc_import_cb_ok, win, cfdata);
   e_widget_list_object_append(import->box_obj, import->ok_obj, 1, 0, 0.5);

   import->cancel_obj = e_widget_button_add(evas, _("Cancel"), NULL,
                                            _imc_import_cb_close,
                                            win, cfdata);
   e_widget_list_object_append(import->box_obj, import->cancel_obj, 1, 0, 0.5);

   e_widget_disabled_set(import->ok_obj, 1);
   elm_win_center(win, 1, 1);

   o = import->box_obj;
   e_widget_size_min_get(o, &w, &h);
   evas_object_size_hint_min_set(o, w, h);
   elm_object_part_content_set(import->bg_obj, "e.swallow.buttons", o);

   evas_object_show(win);
   e_win_client_icon_set(win, "preferences-imc");

   evas_object_data_set(win, "imc_win", import);

   return win;
}

static void
_imc_import_cb_delete(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Import *import;

   import = evas_object_data_get(obj, "imc_win");
   if (!import) return;

   evas_object_del(import->win);
   e_int_config_imc_import_done(import->parent);

   E_FREE(import->cfdata->file);
   E_FREE(import->cfdata);
   E_FREE(import);

   return;
}

static void
_imc_import_cb_wid_focus(void *data, Evas_Object *obj)
{
   Import *import;

   import = data;
   if (obj == import->content_obj)
     e_widget_focused_object_clear(import->box_obj);
   else if (import->content_obj)
     e_widget_focused_object_clear(import->content_obj);
}

static void
_imc_import_cb_selected(void *data, Evas_Object *obj EINA_UNUSED)
{
   Import *import;

   import = data;
   _imc_import_cb_ok(import->win, NULL);
}

static void
_imc_import_cb_changed(void *data, Evas_Object *obj EINA_UNUSED)
{
   Import *import;
   const char *path;
   const char *file;

   if (!(import = data)) return;
   if (!import->fsel_obj) return;

   path = e_widget_fsel_selection_path_get(import->fsel_obj);
   E_FREE(import->cfdata->file);
   if (path)
     import->cfdata->file = strdup(path);

   if (import->cfdata->file)
     {
        char *strip;

        file = ecore_file_file_get(import->cfdata->file);
        strip = ecore_file_strip_ext(file);
        if (!strip)
          {
             E_FREE(import->cfdata->file);
             e_widget_disabled_set(import->ok_obj, 1);
             return;
          }
        free(strip);
        if (!e_util_glob_case_match(file, "*.imc"))
          {
             E_FREE(import->cfdata->file);
             e_widget_disabled_set(import->ok_obj, 1);
             return;
          }
        e_widget_disabled_set(import->ok_obj, 0);
     }
   else
     e_widget_disabled_set(import->ok_obj, 1);
}

static void
_imc_import_cb_ok(void *data, void *data2 EINA_UNUSED)
{
   Import *import;
   Evas_Object *win;
   const char *path;
   const char *file;

   win = data;
   if (!(import = evas_object_data_get(win, "imc_win"))) return;

   path = e_widget_fsel_selection_path_get(import->fsel_obj);
   E_FREE(import->cfdata->file);
   if (path)
     import->cfdata->file = strdup(path);

   if (import->cfdata->file)
     {
        Eet_File *ef;
        E_Input_Method_Config *imc;
        char *strip;

        file = ecore_file_file_get(import->cfdata->file);

        if (!(strip = ecore_file_strip_ext(file))) return;
        free(strip);

        if (!e_util_glob_case_match(file, "*.imc"))
          return;

        imc = NULL;
        ef = eet_open(import->cfdata->file, EET_FILE_MODE_READ);
        if (ef)
          {
             imc = e_intl_input_method_config_read(ef);
             eet_close(ef);
          }

        if (!imc)
          {
             e_util_dialog_show(_("Input Method Config Import Error"),
                                _("Enlightenment was unable to import "
                                  "the configuration.<ps/><ps/>Are "
                                  "you sure this is really a valid "
                                  "configuration?"));
          }
        else
          {
             char buf[PATH_MAX];

             e_intl_input_method_config_free(imc);
             snprintf(buf, sizeof(buf), "%s/%s",
                      e_intl_imc_personal_path_get(), file);

             if (!ecore_file_cp(import->cfdata->file, buf))
               {
                  e_util_dialog_show(_("Input Method Config Import Error"),
                                     _("Enlightenment was unable to import "
                                       "the configuration<ps/>due to a copy "
                                       "error."));
               }
             else
               e_int_config_imc_update(import->parent, buf);
          }
     }

   evas_object_del(import->win);
}

static void
_imc_import_cb_close(void *data, void *data2 EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_imc_import_cb_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Evas_Event_Key_Down *ev;
   Import *import;

   ev = event;
   import = data;
   if ((!e_widget_fsel_typebuf_visible_get(import->fsel_obj)) && (!strcmp(ev->key, "Tab")))
     {
        if (evas_key_modifier_is_set(evas_key_modifier_get(evas_object_evas_get(import->win)), "Shift"))
          {
             if (e_widget_focus_get(import->box_obj))
               {
                  if (!e_widget_focus_jump(import->box_obj, 0))
                    {
                       e_widget_focus_set(import->content_obj, 0);
                       if (!e_widget_focus_get(import->content_obj))
                         e_widget_focus_set(import->box_obj, 0);
                    }
               }
             else
               {
                  if (!e_widget_focus_jump(import->content_obj, 0))
                    e_widget_focus_set(import->box_obj, 0);
               }
          }
        else
          {
             if (e_widget_focus_get(import->box_obj))
               {
                  if (!e_widget_focus_jump(import->box_obj, 1))
                    {
                       e_widget_focus_set(import->content_obj, 1);
                       if (!e_widget_focus_get(import->content_obj))
                         e_widget_focus_set(import->box_obj, 1);
                    }
               }
             else
               {
                  if (!e_widget_focus_jump(import->content_obj, 1))
                    e_widget_focus_set(import->box_obj, 1);
               }
          }
     }
   else if (((!strcmp(ev->key, "Return")) ||
             (!strcmp(ev->key, "KP_Enter")) ||
             (!strcmp(ev->key, "space"))))
     {
        Evas_Object *o = NULL;

        if ((import->content_obj) && (e_widget_focus_get(import->content_obj)))
          o = e_widget_focused_object_get(import->content_obj);
        else
          o = e_widget_focused_object_get(import->box_obj);
        if (o) e_widget_activate(o);
     }
}

