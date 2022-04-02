#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   Evas_Object *icon, *text;
   int show_all_desktops;
   int show_all_screens;
   int minw, minh;
   int icon_only;
   int text_only;
   int preview;
   int preview_size;
};

/* Protos */
static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

void
_config_tasks_module(Config_Item *ci)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;

   cfd = e_config_dialog_new(NULL, _("Tasks Configuration"), "Tasks",
                             "_e_modules_tasks_config_dialog", NULL, 0, v, ci);
   if (tasks_config->config_dialog)
     e_object_del(E_OBJECT(tasks_config->config_dialog));
   tasks_config->config_dialog = cfd;
}

static void
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata)
{
   cfdata->show_all_desktops = ci->show_all_desktops;
   cfdata->show_all_screens = ci->show_all_screens;
   cfdata->minw = ci->minw;
   cfdata->minh = ci->minh;
   cfdata->icon_only = ci->icon_only;
   cfdata->text_only = ci->text_only;
   cfdata->preview = ci->preview;
   cfdata->preview_size = ci->preview_size;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Config_Item *ci;

   ci = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ci, cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   if (!tasks_config) return;
   tasks_config->config_dialog = NULL;
   free(cfdata);
}

static void
_toggle_check(void *data, Evas_Object *obj EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;
   e_widget_disabled_set(cfdata->text, cfdata->icon_only);
   e_widget_disabled_set(cfdata->icon, cfdata->text_only);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob, *ow;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Display"), 0);
   ob = e_widget_check_add(evas, _("Show windows from all desktops"),
                           &(cfdata->show_all_desktops));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Show windows from all screens"),
                           &(cfdata->show_all_screens));
   e_widget_framelist_object_append(of, ob);
   cfdata->icon = ob = e_widget_check_add(evas, _("Show icon only"),
                           &(cfdata->icon_only));
   e_widget_framelist_object_append(of, ob);
   e_widget_on_change_hook_set(ob, _toggle_check, cfdata);
   cfdata->text = ob = e_widget_check_add(evas, _("Show text only"),
                           &(cfdata->text_only));
   e_widget_framelist_object_append(of, ob);
   e_widget_on_change_hook_set(ob, _toggle_check, cfdata);
   ow = e_widget_label_add(evas, _("Item width"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_slider_add(evas, 1, 0, _("%1.0f px"), 20, 420, 1, 0,
                            NULL, &(cfdata->minw), 100);
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_label_add(evas, _("Item height"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_slider_add(evas, 1, 0, _("%1.0f px"), 20, 420, 1, 0,
                            NULL, &(cfdata->minh), 100);
   e_widget_framelist_object_append(of, ow);
   _toggle_check(cfdata, NULL);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   e_widget_framelist_object_append(of, ob);
   of = e_widget_framelist_add(evas, _("Preview"), 0);
   ob = e_widget_check_add(evas, _("Show"),
                           &(cfdata->preview));
   e_widget_framelist_object_append(of, ob);
   ow = e_widget_label_add(evas, _("Preview Size"));
   e_widget_framelist_object_append(of, ow);
   ow = e_widget_slider_add(evas, 1, 0, _("%1.0f px"), 20, 420, 1, 0,
                            NULL, &(cfdata->preview_size), 100);
   e_widget_framelist_object_append(of, ow);
   _toggle_check(cfdata, NULL);

   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   Config_Item *ci;

   ci = cfd->data;
   ci->show_all_desktops = cfdata->show_all_desktops;
   ci->show_all_screens = cfdata->show_all_screens;
   ci->minw = cfdata->minw;
   ci->minh = cfdata->minh;
   ci->icon_only = cfdata->icon_only;
   ci->text_only = cfdata->text_only;
   ci->preview = cfdata->preview;
   ci->preview_size = cfdata->preview_size;
   e_config_save_queue();
   _tasks_config_updated(ci);
   return 1;
}

