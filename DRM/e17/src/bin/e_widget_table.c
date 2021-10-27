#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object *o_table;
};

static void _e_wid_del_hook(Evas_Object *obj);

/* local subsystem functions */

/* externally accessible functions */
E_API Evas_Object *
e_widget_table_add(Evas_Object *parent, int homogenous)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;

   obj = e_widget_add(evas_object_evas_get(parent));

   e_widget_del_hook_set(obj, _e_wid_del_hook);
   wd = calloc(1, sizeof(E_Widget_Data));
   e_widget_data_set(obj, wd);

   o = elm_table_add(parent);
   wd->o_table = o;
   elm_table_homogeneous_set(o, homogenous);
   evas_object_show(o);
   e_widget_sub_object_add(obj, o);
   e_widget_resize_object_set(obj, o);

   return obj;
}

E_API void
e_widget_table_object_append(Evas_Object *obj, Evas_Object *sobj, int col, int row, int colspan, int rowspan, int fill_w, int fill_h, int expand_w, int expand_h)
{
   e_widget_table_object_align_append(obj, sobj,
                                      col, row, colspan, rowspan,
                                      fill_w, fill_h, expand_w, expand_h,
                                      0.5, 0.5);
}

E_API void
e_widget_table_object_align_append(Evas_Object *obj, Evas_Object *sobj, int col, int row, int colspan, int rowspan, int fill_w, int fill_h, int expand_w, int expand_h, double ax, double ay)
{
   E_Widget_Data *wd = e_widget_data_get(obj);
   if (!wd) return;

   if (fill_w) ax = -1;
   if (fill_h) ay = -1;
   E_ALIGN(sobj, ax, ay);
   E_WEIGHT(sobj, expand_w, expand_h);
   elm_table_pack(wd->o_table, sobj, col, row, colspan, rowspan);
   e_widget_sub_object_add(obj, sobj);
   evas_object_show(sobj);
}

E_API void
e_widget_table_object_repack(Evas_Object *obj EINA_UNUSED, Evas_Object *sobj, int col, int row, int colspan, int rowspan, int fill_w, int fill_h, int expand_w, int expand_h)
{
   if (fill_w || fill_h)
     E_ALIGN(sobj, fill_w ? -1 : 0.5, fill_h ? -1 : 0.5);
   E_WEIGHT(sobj, expand_w, expand_h);
   elm_table_pack_set(sobj, col, row, colspan, rowspan);
}

E_API void
e_widget_table_unpack(Evas_Object *obj, Evas_Object *sobj)
{
   E_Widget_Data *wd = e_widget_data_get(obj);
   if (!wd) return;
   e_widget_sub_object_del(obj, sobj);
   elm_table_unpack(wd->o_table, sobj);
}

static void
_e_wid_del_hook(Evas_Object *obj)
{
   E_Widget_Data *wd = e_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

