#include "e.h"

#define IMPORT_STRETCH          0
#define IMPORT_TILE             1
#define IMPORT_CENTER           2
#define IMPORT_SCALE_ASPECT_IN  3
#define IMPORT_SCALE_ASPECT_OUT 4
#define IMPORT_PAN              5

static void      _import_edj_gen(E_Import_Config_Dialog *import);
static Eina_Bool _import_cb_edje_cc_exit(void *data, int type, void *event);

static void
_import_edj_gen(E_Import_Config_Dialog *import)
{
   const char *file, *fill, *s;
   char buf[PATH_MAX], fbuf[PATH_MAX], cmd[PATH_MAX + PATH_MAX + 40];
   char *fstrip, *infile, *outfile;
   int num, cr, cg, cb;
   size_t len, off;

   file = ecore_file_file_get(import->file);
   fstrip = ecore_file_strip_ext(file);
   if (!fstrip) return;
   len = e_user_dir_snprintf(buf, sizeof(buf), "backgrounds/.tmp.%s.edj", fstrip);
   if (len >= sizeof(buf))
     {
        free(fstrip);
        return;
     }
   off = len - (sizeof(".edj") - 1);
   for (num = 1; ecore_file_exists(buf) && num < 100; num++)
     snprintf(buf + off, sizeof(buf) - off, "-%d.edj", num);
   free(fstrip);
   cr = import->color.r;
   cg = import->color.g;
   cb = import->color.b;

   if (num == 100)
     {
        printf("Couldn't come up with another filename for %s\n", buf);
        return;
     }
   switch (import->method)
     {
      case IMPORT_STRETCH:          fill = "stretch"; break;
      case IMPORT_TILE:             fill = "tile"; break;
      case IMPORT_CENTER:           fill = "center"; break;
      case IMPORT_SCALE_ASPECT_IN:  fill = "scale_in"; break;
      case IMPORT_SCALE_ASPECT_OUT: fill = "scale_out"; break;
      case IMPORT_PAN:              fill = "pan"; break;
      default: return; break;
     }
   s = e_util_filename_escape(import->file);
   if (s) infile = strdup(s);
   else return;
   s = e_util_filename_escape(buf);
   if (s) outfile = strdup(s);
   else
     {
        free(infile);
        return;
     }
   snprintf(fbuf, sizeof(fbuf), "%s/edje_cc", e_prefix_bin_get());
   if (!ecore_file_can_exec(fbuf))
     snprintf(fbuf, sizeof(fbuf), "edje_cc");
   snprintf(cmd, sizeof(cmd),
            "%s/enlightenment/utils/enlightenment_wallpaper_gen "
            "%s %s %s %s %i %i %i %i",
            e_prefix_lib_get(),
            fbuf, fill, infile, outfile, import->quality, cr, cg, cb);
   free(infile);
   free(outfile);
   import->fdest = eina_stringshare_add(buf);
   import->exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                                 _import_cb_edje_cc_exit,
                                                 import);
   import->exe = ecore_exe_run(cmd, import);
}

static Eina_Bool
_import_cb_edje_cc_exit(void *data, EINA_UNUSED int type, void *event)
{
   E_Import_Config_Dialog *import;
   Ecore_Exe_Event_Del *ev;
   int r = 1;

   ev = event;
   import = data;
   if (ecore_exe_data_get(ev->exe) != import) return ECORE_CALLBACK_PASS_ON;

   if (ev->exit_code != 0)
     {
        e_util_dialog_show(_("Picture Import Error"),
                           _("Enlightenment was unable to import the picture<ps/>"
                             "due to conversion errors."));
        r = 0;
     }

   if (r && import->ok)
     {
        char *p, *newfile = strdup(import->fdest);

        if (!newfile)
          {
             e_object_del(E_OBJECT(import));
             return ECORE_CALLBACK_DONE;
          }
        p = strrchr(newfile, '/');
        if (!p)
          {
             e_object_del(E_OBJECT(import));
             free(newfile);
             return ECORE_CALLBACK_DONE;
          }
        // strip out the .tmp. before the name
        for (p = p + 1; ; p++)
          {
             *p = p[5];
             if (*p == 0) break;
          }
        ecore_file_mv(import->fdest, newfile);
        eina_stringshare_replace(&(import->fdest), newfile);
        free(newfile);

        e_object_ref(E_OBJECT(import));
        import->ok((void *)import->fdest, import);
        e_object_del(E_OBJECT(import));
        e_object_unref(E_OBJECT(import));
     }
   else
     e_object_del(E_OBJECT(import));

   return ECORE_CALLBACK_DONE;
}

static void
_import_cb_close(void *data, E_Dialog *dia EINA_UNUSED)
{
   E_Import_Config_Dialog *import = data;

   e_object_ref(data);
   if (import->cancel) import->cancel(import);
   e_object_del(data);
   e_object_unref(data);
}

static void
_import_cb_ok(void *data, E_Dialog *dia EINA_UNUSED)
{
   E_Import_Config_Dialog *import = data;
   const char *file;
   char buf[PATH_MAX];
   int is_bg, is_theme, r;

   r = 0;
   if (!import->file) return;
   file = ecore_file_file_get(import->file);
   if (!eina_str_has_extension(file, "edj"))
     {
        _import_edj_gen(import);
        evas_object_hide(import->dia->win);
        return;
     }
   e_user_dir_snprintf(buf, sizeof(buf), "backgrounds/%s", file);

   is_bg = edje_file_group_exists(import->file, "e/desktop/background");
   is_theme = edje_file_group_exists(import->file,
                                     "e/widgets/border/default/border");

   if ((is_bg) && (!is_theme))
     {
        if (!ecore_file_cp(import->file, buf))
          {
             e_util_dialog_show(_("Import Error"),
                                _("Enlightenment was unable to "
                                  "import the image<ps/>due to a "
                                  "copy error."));
          }
        else
          r = 1;
     }
   else
     {
        e_util_dialog_show(_("Import Error"),
                           _("Enlightenment was unable to "
                             "import the image.<ps/><ps/>"
                             "Are you sure this is a valid "
                             "image?"));
     }

   if (r)
     {
        e_object_ref(E_OBJECT(import));
        if (import->ok) import->ok((void *)buf, import);
        e_object_del(E_OBJECT(import));
        e_object_unref(E_OBJECT(import));
     }
   else
     _import_cb_close(import, NULL);
}

static void
_e_import_config_preview_size_get(int size, int w, int h, int *tw, int *th)
{
   if (size <= 0) return;
   double aspect;
   aspect = (double)w / h;

   if (w > size)
     {
        w = size;
        h = (w / aspect);
     }
   *tw = w;
   *th = h;
}

static void
_e_import_config_dia_del(void *data)
{
   E_Dialog *dia = data;
   E_Import_Config_Dialog *import;

   import = dia->data;
   dia->data = NULL;
   e_object_del(E_OBJECT(import));
}

static void
_e_import_config_dialog_del(void *data)
{
   E_Import_Config_Dialog *import = data;

   if (import->exe_handler) ecore_event_handler_del(import->exe_handler);
   import->exe_handler = NULL;
   eina_stringshare_del(import->fdest);
   import->exe = NULL;
   eina_stringshare_del(import->file);
   e_object_del(E_OBJECT(import->dia));
   free(import);
}

static void
_e_import_config_dialog_win_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Dialog *dia = data;
   E_Import_Config_Dialog *import;

   import = dia->data;
   if (!import) return;
   e_object_ref(E_OBJECT(import));
   if (import->cancel) import->cancel(import);
   e_object_del(E_OBJECT(import));
   e_object_unref(E_OBJECT(import));
}

///////////////////////////////////////////////////////////////////////////////////

E_API E_Import_Config_Dialog *
e_import_config_dialog_show(Evas_Object *parent, const char *path, Ecore_End_Cb ok, Ecore_Cb cancel)
{
   Evas *evas;
   E_Dialog *dia;
   E_Import_Config_Dialog *import;
   Evas_Object *o, *of, *ord, *ot, *ol, *preview, *frame;
   E_Radio_Group *rg;
   int w, h, tw, th;

   if (!path) return NULL;

   import = E_OBJECT_ALLOC(E_Import_Config_Dialog, E_IMPORT_CONFIG_DIALOG_TYPE, _e_import_config_dialog_del);
   if (!import) return NULL;

   dia = e_dialog_new(parent, "E", "_import_config_dialog");
   if (!dia)
     {
        e_object_del(E_OBJECT(import));
        return NULL;
     }
   e_dialog_resizable_set(dia, 1);

   e_dialog_title_set(dia, _("Import Settings..."));
   dia->data = import;
   import->dia = dia;
   import->ok = ok, import->cancel = cancel;
   e_object_del_attach_func_set(E_OBJECT(dia), _e_import_config_dia_del);
   evas_object_event_callback_add(dia->win, EVAS_CALLBACK_DEL, _e_import_config_dialog_win_del, dia);

   import->method = IMPORT_SCALE_ASPECT_OUT;
   import->quality = 90;
   import->file = eina_stringshare_add(path);

   evas = evas_object_evas_get(dia->win);

   o = e_widget_list_add(evas, 0, 0);

   ot = e_widget_list_add(evas, 0, 0);
   frame = e_widget_frametable_add(evas, _("Preview"), 1);

   preview = evas_object_image_add(evas);
   evas_object_image_file_set(preview, path, NULL);
   evas_object_image_size_get(preview, &w, &h);
   evas_object_del(preview);

   _e_import_config_preview_size_get(320, w, h, &tw, &th);

   preview = e_widget_preview_add(evas, tw, th);
   e_widget_preview_thumb_set(preview, path, NULL, tw, th);

   e_widget_frametable_object_append(frame, preview, 0, 0, 1, 1, 0, 0, 1, 1);
   e_widget_list_object_append(ot, frame, 1, 1, 0.5);
   of = e_widget_frametable_add(evas, _("Fill and Stretch Options"), 1);
   rg = e_widget_radio_group_new(&import->method);
   ord = e_widget_radio_icon_add(evas, _("Stretch"),
                                 "enlightenment/wallpaper_stretch",
                                 24, 24, IMPORT_STRETCH, rg);
   e_widget_frametable_object_append(of, ord, 0, 0, 1, 1, 1, 0, 1, 0);
   ord = e_widget_radio_icon_add(evas, _("Center"),
                                 "enlightenment/wallpaper_center",
                                 24, 24, IMPORT_CENTER, rg);
   e_widget_frametable_object_append(of, ord, 1, 0, 1, 1, 1, 0, 1, 0);
   ord = e_widget_radio_icon_add(evas, _("Tile"),
                                 "enlightenment/wallpaper_tile",
                                 24, 24, IMPORT_TILE, rg);
   e_widget_frametable_object_append(of, ord, 2, 0, 1, 1, 1, 0, 1, 0);

   ord = e_widget_radio_icon_add(evas, _("Within"),
                                 "enlightenment/wallpaper_scale_aspect_in",
                                 24, 24, IMPORT_SCALE_ASPECT_IN, rg);
   e_widget_frametable_object_append(of, ord, 0, 1, 1, 1, 1, 0, 1, 0);
   ord = e_widget_radio_icon_add(evas, _("Fill"),
                                 "enlightenment/wallpaper_scale_aspect_out",
                                 24, 24, IMPORT_SCALE_ASPECT_OUT, rg);
   e_widget_frametable_object_append(of, ord, 1, 1, 1, 1, 1, 0, 1, 0);
   ord = e_widget_radio_icon_add(evas, _("Pan"),
                                 "enlightenment/wallpaper_pan",
                                 24, 24, IMPORT_PAN, rg);
   e_widget_frametable_object_append(of, ord, 2, 1, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(ot, of, 1, 1, 0.5);

   ol = e_widget_list_add(evas, 0, 1);

   of = e_widget_frametable_add(evas, _("File Quality"), 0);
   ord = e_widget_slider_add(evas, 1, 0, _("%3.0f%%"), 0.0, 100.0, 1.0, 0,
                             NULL, &(import->quality), 150);
   e_widget_frametable_object_append(of, ord, 0, 0, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(ol, of, 1, 1, 0);

   of = e_widget_framelist_add(evas, _("Fill Color"), 0);
   ord = e_widget_color_well_add(evas, &import->color, 1);
   e_widget_framelist_object_append(of, ord);
   e_widget_list_object_append(ol, of, 1, 0, 1);
   e_widget_list_object_append(ot, ol, 1, 1, 0);

   e_widget_list_object_append(o, ot, 0, 0, 0.5);

   e_widget_size_min_get(o, &w, &h);
   e_dialog_content_set(dia, o, w, h);
   e_dialog_button_add(dia, _("OK"), NULL, _import_cb_ok, import);
   e_dialog_button_add(dia, _("Cancel"), NULL, _import_cb_close, import);
   elm_win_center(dia->win, 1, 1);
   e_dialog_border_icon_set(dia, "folder-image");
   e_dialog_button_focus_num(dia, 0);
   e_dialog_show(dia);

   return import;
}

