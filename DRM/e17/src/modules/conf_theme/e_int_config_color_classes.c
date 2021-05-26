#include "e.h"

/*
 * This is currently broken and does nothing other than list colorclasses
 * so no point having a broken dialog in E.

static char *
_translate(char *str)
{
   return _(str);
}

static Eina_List *
_list(void)
{
   Eina_List *l, *ret = NULL;
   Eina_Iterator *it;
   Eina_File *f;

   it = edje_file_iterator_new();
   if (!it) return NULL;
   EINA_ITERATOR_FOREACH(it, f)
     {
        l = elm_color_class_util_edje_file_list(f);
        if (l)
          ret = eina_list_merge(ret, l);
     }
   eina_iterator_free(it);
   return ret;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas EINA_UNUSED, E_Config_Dialog_Data *cfdata EINA_UNUSED)
{
   Evas_Object *o;

   e_dialog_resizable_set(cfd->dia, 1);
   o = elm_color_class_editor_add(cfd->dia->win);

   e_util_win_auto_resize_fill(cfd->dia->win);
   elm_win_center(cfd->dia->win, 1, 1);
   evas_object_size_hint_min_set(cfd->dia->win, 400, 280);

   return o;
}

E_Config_Dialog *
e_int_config_color_classes(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "appearance/colors")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->basic.create_widgets = _basic_create_widgets;

   elm_color_class_translate_cb_set(_translate);
   elm_color_class_list_cb_set(_list);

   cfd = e_config_dialog_new(NULL, _("Colors"), "E", "appearance/colors",
                             "preferences-desktop-color", 0, v, NULL);
   return cfd;
}
*/
