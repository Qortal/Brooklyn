#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object *obj, *o_frame, *img, *o_thumb, *o_extern;
};

/* local subsystem functions */
static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_preview_thumb_gen(void *data, Evas_Object *obj, void *event_info);

/* externally accessible functions */
E_API Evas_Object *
e_widget_preview_add(Evas *evas, int minw, int minh)
{
   Evas_Object *obj;
   E_Widget_Data *wd;
   Evas_Coord mw, mh;

   obj = e_widget_add(evas);
   e_widget_del_hook_set(obj, _e_wid_del_hook);

   wd = E_NEW(E_Widget_Data, 1);
   if (!wd) return NULL;

   wd->obj = obj;

   wd->o_frame = edje_object_add(evas);
   e_theme_edje_object_set(wd->o_frame, "base/theme/widgets",
                           "e/widgets/preview");
   evas_object_show(wd->o_frame);
   e_widget_sub_object_add(obj, wd->o_frame);
   e_widget_resize_object_set(obj, wd->o_frame);

   wd->img = e_livethumb_add(evas);
   e_livethumb_vsize_set(wd->img, minw * 2, minh * 2);
   evas_object_size_hint_min_set(wd->img, minw, minh);
   /* Why do we set max size? This prevents from being nicely resized. */
   /* evas_object_size_hint_max_set(o, minw, minh); */
   evas_object_show(wd->img);
   e_widget_sub_object_add(obj, wd->img);
   edje_object_part_swallow(wd->o_frame, "e.swallow.content", wd->img);

   e_widget_data_set(obj, wd);
   e_widget_can_focus_set(obj, 0);
   edje_object_size_min_calc(wd->o_frame, &mw, &mh);
   e_widget_size_min_set(obj, mw, mh);

   return obj;
}

E_API void
e_widget_preview_size_set(Evas_Object *obj, int minw, int minh)
{
   E_Widget_Data *wd;
   int mw, mh;

   wd = e_widget_data_get(obj);
   e_livethumb_vsize_set(wd->img, minw * 2, minh * 2);
   evas_object_size_hint_min_set(wd->img, minw, minh);
   edje_object_size_min_calc(wd->o_frame, &mw, &mh);
   e_widget_size_min_set(obj, mw, mh);
}

E_API Evas *
e_widget_preview_evas_get(Evas_Object *obj)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   return e_livethumb_evas_get(wd->img);
}

E_API void
e_widget_preview_extern_object_set(Evas_Object *obj, Evas_Object *eobj)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   wd->o_extern = eobj;
   e_livethumb_thumb_set(wd->img, wd->o_extern);

   e_widget_change(obj);
}

E_API int
e_widget_preview_file_set(Evas_Object *obj, const char *file, const char *key)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   if (wd->o_thumb) evas_object_del(wd->o_thumb);
   wd->o_thumb = e_icon_add(e_livethumb_evas_get(wd->img));
   e_icon_fill_inside_set(wd->o_thumb, 0);
   e_icon_file_key_set(wd->o_thumb, file, key);
   evas_object_show(wd->o_thumb);
   e_livethumb_thumb_set(wd->img, wd->o_thumb);

   e_widget_change(obj);

   return 1;
}

E_API int
e_widget_preview_thumb_set(Evas_Object *obj, const char *file, const char *key, int w, int h)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   if (wd->o_thumb) evas_object_del(wd->o_thumb);
   wd->o_thumb = e_thumb_icon_add(e_livethumb_evas_get(wd->img));
   if (eina_str_has_extension(file, "edj"))
     {
        if (key)
          e_thumb_icon_file_set(wd->o_thumb, file, key);
        else
        /* FIXME: There is probably a quicker way of doing this. */
        if (edje_file_group_exists(file, "icon"))
          e_thumb_icon_file_set(wd->o_thumb, file, "icon");
        else if (edje_file_group_exists(file, "e/desktop/background"))
          e_thumb_icon_file_set(wd->o_thumb, file, "e/desktop/background");
        else if (edje_file_group_exists(file, "e/init/splash"))
          e_thumb_icon_file_set(wd->o_thumb, file, "e/init/splash");
     }
   else
     e_thumb_icon_file_set(wd->o_thumb, file, NULL);
   evas_object_smart_callback_add(wd->o_thumb, "e_thumb_gen",
                                  _e_wid_preview_thumb_gen, wd);
   e_thumb_icon_size_set(wd->o_thumb, w, h);
   e_thumb_icon_begin(wd->o_thumb);
   e_livethumb_thumb_set(wd->img, wd->o_thumb);

   e_widget_change(obj);

   return 1;
}

E_API void
e_widget_preview_vsize_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   e_livethumb_vsize_set(wd->img, w, h);
   e_widget_size_min_set(wd->obj, w, h);
   evas_object_smart_callback_call(wd->obj, "preview_update", NULL);
   e_widget_change(wd->obj);
}

static void
_e_wid_preview_thumb_gen(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Widget_Data *wd;

   wd = data;
   evas_object_show(wd->o_thumb);
   evas_object_smart_callback_call(wd->obj, "preview_update", NULL);
   e_widget_change(wd->obj);
}

E_API void
e_widget_preview_file_get(Evas_Object *obj, const char **file, const char **group)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   edje_object_file_get(wd->o_thumb, file, group);
}

E_API int
e_widget_preview_edje_set(Evas_Object *obj, const char *file, const char *group)
{
   E_Widget_Data *wd;
   int ret;

   wd = e_widget_data_get(obj);
   if (wd->o_thumb) evas_object_del(wd->o_thumb);
   wd->o_thumb = edje_object_add(e_livethumb_evas_get(wd->img));
   ret = edje_object_file_set(wd->o_thumb, file, group);
   e_livethumb_thumb_set(wd->img, wd->o_thumb);
   e_widget_change(obj);
   return ret;
}

static void
_e_wid_del_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   E_FREE(wd);
}

