#include "e_mod_main.h"

static void        *_create_data    (E_Config_Dialog *cfd);
static void         _fill_data      (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _free_data      (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create   (E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_check    (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _basic_apply    (E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void         _cb_icon_sel    (void *data, void *data2);
static Evas_Object *_get_icon       (void *data);
static void         _cb_type        (void *data, Evas_Object *obj, void *event_info);
static void         _cb_fsel_sel    (void *data, Evas_Object *obj);
static void         _cb_fsel_ok     (void *data, E_Dialog *dia);
static void         _cb_fsel_cancel (void *data, E_Dialog *dia);
static void         _cb_file_change (void *data);

typedef enum _Icon_Type Icon_Type;
enum _Icon_Type
{
     THUMB,
     THEME,
     EDJ,
     IMG,
     DEFAULT
};

struct _E_Config_Dialog_Data
{
   char *mime;
   char *icon;
   int type;

   char *file;
   struct
     {
	Evas_Object *icon;
	Evas_Object *icon_wid, *fsel_wid;
	E_Dialog *fsel;
     } gui;
   E_Config_Mime_Icon *data;
   void *data2;
};

#define IFDUP(src, dst) if (src) dst = strdup(src); else dst = NULL;
#define IFFREE(src) if (src) free(src); src = NULL;

E_Config_Dialog *
e_int_config_mime_edit(E_Config_Mime_Icon *data, void *data2)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   E_Config_Dialog_Data *cfdata;

   if (e_config_dialog_find("E", "fileman/mime_edit_dialog")) return NULL;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->data = data;
   cfdata->data2 = data2;

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.check_changed = _basic_check;
   v->basic.apply_cfdata = _basic_apply;

   cfd = e_config_dialog_new(NULL, _("File Icon"), "E",
			     "fileman/mime_edit_dialog",
			     "preferences-file-icons", 0, v, cfdata);
   return cfd;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   _fill_data(cfd, cfd->data);
   return cfd->data;
}

static void
_fill_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_Config_Mime_Icon *mi;

   mi = (E_Config_Mime_Icon *)cfdata->data;

   IFDUP(mi->mime, cfdata->mime);
   IFDUP(mi->icon, cfdata->icon);
   IFDUP(mi->icon, cfdata->file);

   if (!cfdata->icon)
     cfdata->type = DEFAULT;
   else
     {
	if (!strcmp(cfdata->icon, "THUMB"))
	  cfdata->type = THUMB;
	else if (!strncmp(cfdata->icon, "e/icons/fileman/mime", 20))
	  cfdata->type = THEME;
	else
	  {
	     char *p;

	     p = strrchr(cfdata->icon, '.');
	     if ((p) && (!strcmp(p, ".edj")))
	       cfdata->type = EDJ;
	     else
	       cfdata->type = IMG;
	  }
     }
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   if (cfdata->gui.fsel)
     e_object_del(E_OBJECT(cfdata->gui.fsel));

   IFFREE(cfdata->file);
   IFFREE(cfdata->mime);
   IFFREE(cfdata->icon);
   e_int_config_mime_edit_done(cfdata->data2);
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of;
   Evas_Object *ob, *oi, *icon;
   E_Radio_Group *rg;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_frametable_add(evas, _("Basic Info"), 0);
   ob = e_widget_label_add(evas, _("Mime:"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 0, 1);
   ob = e_widget_entry_add(cfd->dia->win, &(cfdata->mime), NULL, NULL, NULL);
   e_widget_entry_readonly_set(ob, 1);
   e_widget_size_min_set(ob, 100, 1);
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, _("Icon"), 0);
   rg = e_widget_radio_group_new(&cfdata->type);
   ob = e_widget_radio_add(evas, _("Use Generated Thumbnail"), 0, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 0, 3, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Use Theme Icon"), 1, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 1, 3, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Use Edje File"), 2, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 2, 3, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Use Image"), 3, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 3, 3, 1, 1, 1, 1, 1);
   ob = e_widget_radio_add(evas, _("Use Default"), 4, rg);
   evas_object_smart_callback_add(ob, "changed", _cb_type, cfdata);
   e_widget_frametable_object_append(of, ob, 0, 4, 3, 1, 1, 1, 1, 1);

   oi = e_widget_button_add(evas, "", NULL, _cb_icon_sel, cfdata, cfd);
   cfdata->gui.icon_wid = oi;
   if (cfdata->icon)
     {
	icon = _get_icon(cfdata);
	if (icon)
	  e_widget_button_icon_set(oi, icon);
     }
   e_widget_size_min_set(oi, 48, 48);
   e_widget_frametable_object_append(of, oi, 1, 5, 1, 1, 1, 1, 1, 1);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   switch (cfdata->type)
     {
      case EDJ:
      case IMG:
	e_widget_disabled_set(cfdata->gui.icon_wid, 0);
	break;
      default:
	e_widget_disabled_set(cfdata->gui.icon_wid, 1);
	break;
     }

   return o;
}

static int
_basic_check(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_Config_Mime_Icon *mi;

   mi = (E_Config_Mime_Icon *) cfdata->data;

   return ((cfdata->type == DEFAULT) && (mi->icon)) ||
	  ((cfdata->type == THUMB) &&
	   ((!mi->icon) || strcmp(mi->icon, "THUMB"))) ||
	  ((cfdata->type == THEME) &&
	   ((!mi->icon) || strncmp(mi->icon, "e/icons/fileman/mime", 20))) ||
	  (((cfdata->type == EDJ) || (cfdata->type == IMG)) &&
	   ((!mi->icon) || !strcmp(mi->icon, "THUMB") ||
	    !strncmp(mi->icon, "e/icons/fileman/mime", 20) ||
	    strcmp(mi->icon, cfdata->icon)));
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   E_Config_Mime_Icon *mi;
   char buf[4096];
   int found = 0;

   for (l = e_config->mime_icons; l; l = l->next)
     {
	mi = l->data;
	if (!mi) continue;
	if (strcmp(mi->mime, cfdata->mime)) continue;
	found = 1;
	break;
     }

   if (found)
     {
	if (mi->icon)
	  eina_stringshare_del(mi->icon);
     }
   else
     {
	if (cfdata->type == DEFAULT) return 1;
	mi = E_NEW(E_Config_Mime_Icon, 1);
	mi->mime = eina_stringshare_add(cfdata->mime);
     }

   switch (cfdata->type)
     {
      case THUMB:
	mi->icon = eina_stringshare_add("THUMB");
	break;
      case THEME:
	snprintf(buf, sizeof(buf), "e/icons/fileman/mime/%s", cfdata->mime);
	mi->icon = eina_stringshare_add(buf);
	break;
      case EDJ:
      case IMG:
	mi->icon = eina_stringshare_add(cfdata->file);
	break;
      case DEFAULT:
	if (found)
	  {
	     e_config->mime_icons = eina_list_remove(e_config->mime_icons, mi);
	     if (mi->mime) eina_stringshare_del(mi->mime);
	     if (mi->icon) eina_stringshare_del(mi->icon);
	     free(mi);
	  }
	break;
     }

   if (!found)
     e_config->mime_icons = eina_list_append(e_config->mime_icons, mi);

   e_config_save_queue();
   e_fm_mime_icon_cache_flush();
   e_fm2_all_icons_update();
   return 1;
}

static void
_dia_del(void *data)
{
   E_Dialog *dia = data;
   E_Config_Dialog_Data *cfdata;

   cfdata = dia->data;
   if (!cfdata) return;
   cfdata->gui.fsel = NULL;
}

static void
_cb_icon_sel(void *data, void *data2)
{
   E_Config_Dialog_Data *cfdata;
   E_Config_Dialog *cfd;
   E_Dialog *dia;
   Evas_Object *o;
   Evas_Coord w, h;

   cfdata = data;
   if (!cfdata) return;
   if (cfdata->gui.fsel) return;

   cfd = data2;
   if (!cfd) return;

   dia = e_dialog_new(NULL, "E", "_mime_icon_select_dialog");
   if (!dia) return;
   if (cfdata->type == EDJ)
     e_dialog_title_set(dia, _("Select an Edje file"));
   else if (cfdata->type == IMG)
     e_dialog_title_set(dia, _("Select an image"));

   e_dialog_resizable_set(dia, 1);

   dia->data = cfdata;
   e_object_del_attach_func_set(E_OBJECT(dia), _dia_del);
   o = e_widget_fsel_add(evas_object_evas_get(dia->win), "~/", "/", NULL, NULL,
			 _cb_fsel_sel, cfdata, NULL, cfdata, 1);

   cfdata->gui.fsel_wid = o;
   evas_object_show(o);
   e_widget_size_min_get(o, &w, &h);
   e_dialog_content_set(dia, o, w, h);

   e_dialog_button_add(dia, _("OK"), NULL, _cb_fsel_ok, cfdata);
   e_dialog_button_add(dia, _("Cancel"), NULL, _cb_fsel_cancel, cfdata);
   elm_win_center(dia->win, 1, 1);
   e_dialog_show(dia);
   e_dialog_border_icon_set(dia, "enlightenment/file_icons");

   cfdata->gui.fsel = dia;
}

static Evas_Object *
_get_icon(void *data)
{
   Evas *evas;
   E_Config_Dialog_Data *cfdata;
   Evas_Object *icon = NULL;
   char buf[4096];

   cfdata = data;
   if (!cfdata) return NULL;

   e_widget_disabled_set(cfdata->gui.icon, 1);
   if (cfdata->gui.icon)
     evas_object_del(cfdata->gui.icon);
   cfdata->gui.icon = NULL;

   if (cfdata->type == DEFAULT) return NULL;

   evas = evas_object_evas_get(cfdata->gui.icon_wid);
   switch (cfdata->type)
     {
      case THUMB:
	icon = edje_object_add(evas);
	e_theme_edje_object_set(icon, "base/theme/fileman", "e/icons/fileman/file");
	break;
      case THEME:
	icon = edje_object_add(evas);
	snprintf(buf, sizeof(buf), "e/icons/fileman/mime/%s", cfdata->mime);
	if (!e_theme_edje_object_set(icon, "base/theme/fileman", buf))
	  e_theme_edje_object_set(icon, "base/theme/fileman", "e/icons/fileman/file");
	break;
      case EDJ:
	icon = edje_object_add(evas);
	edje_object_file_set(icon, cfdata->file, "icon");
	e_widget_disabled_set(cfdata->gui.icon_wid, 0);
	break;
      case IMG:
	icon = e_widget_image_add_from_file(evas, cfdata->file, 48, 48);
	e_widget_disabled_set(cfdata->gui.icon_wid, 0);
	break;
      default:
	break;
     }

   cfdata->gui.icon = icon;
   return icon;
}

static void
_cb_type(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   if (!cfdata) return;
   switch (cfdata->type)
     {
      case EDJ:
      case IMG:
	e_widget_disabled_set(cfdata->gui.icon_wid, 0);
	break;
      default:
	e_widget_disabled_set(cfdata->gui.icon_wid, 1);
	break;
     }
}

static void
_cb_fsel_sel(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   if (!cfdata) return;
}

static void
_cb_fsel_ok(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;
   const char *file;

   cfdata = data;
   if (!cfdata) return;

   file = e_widget_fsel_selection_path_get(cfdata->gui.fsel_wid);
   IFFREE(cfdata->file);
   IFDUP(file, cfdata->file);
   _cb_fsel_cancel(data, dia);
   if (cfdata->file)
     _cb_file_change(cfdata);
}

static void
_cb_fsel_cancel(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   e_object_del(E_OBJECT(dia));
   cfdata->gui.fsel = NULL;
}

static void
_cb_file_change(void *data)
{
   E_Config_Dialog_Data *cfdata;
   Evas_Object *icon;

   cfdata = data;
   if (!cfdata) return;
   if (!cfdata->file) return;
   switch (cfdata->type)
     {
      case EDJ:
	if (!strstr(cfdata->file, ".edj")) return;
	break;
      default:
	break;
     }

   IFFREE(cfdata->icon);
   IFDUP(cfdata->file, cfdata->icon);

   if (cfdata->icon)
     {
	icon = _get_icon(cfdata);
	if (icon)
	  e_widget_button_icon_set(cfdata->gui.icon_wid, icon);
     }
}
