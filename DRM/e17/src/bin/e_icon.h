#ifdef E_TYPEDEFS
#else
#ifndef E_ICON_H
#define E_ICON_H

EINTERN int e_icon_init(void);
EINTERN int e_icon_shutdown(void);

E_API Evas_Object *e_icon_add              (Evas *evas);
E_API void         e_icon_file_set         (Evas_Object *obj, const char *file);
E_API void         e_icon_file_key_set     (Evas_Object *obj, const char *file, const char *key);
E_API Evas_Object *e_icon_edje_get         (Evas_Object *obj);
E_API void         e_icon_file_edje_set    (Evas_Object *obj, const char *file, const char *part);
E_API void         e_icon_fdo_icon_set     (Evas_Object *obj, const char *icon);
E_API void         e_icon_edje_object_set  (Evas_Object *obj, Evas_Object *edje);
E_API void         e_icon_image_object_set (Evas_Object *obj, Evas_Object *o);
E_API Eina_Bool    e_icon_file_get         (const Evas_Object *obj, const char **file, const char **group);
E_API void         e_icon_smooth_scale_set (Evas_Object *obj, Eina_Bool smooth);
E_API Eina_Bool    e_icon_smooth_scale_get (const Evas_Object *obj);
E_API void         e_icon_alpha_set        (Evas_Object *obj, Eina_Bool smooth);
E_API Eina_Bool    e_icon_alpha_get        (const Evas_Object *obj);
E_API void         e_icon_preload_set      (Evas_Object *obj, Eina_Bool preload);
E_API Eina_Bool    e_icon_preload_get      (const Evas_Object *obj);
E_API void         e_icon_size_get         (const Evas_Object *obj, int *w, int *h);
E_API Eina_Bool    e_icon_fill_inside_get  (const Evas_Object *obj);
E_API void         e_icon_fill_inside_set  (Evas_Object *obj, Eina_Bool fill_inside);
E_API Eina_Bool    e_icon_scale_up_get     (const Evas_Object *obj);
E_API void         e_icon_scale_up_set     (Evas_Object *obj, Eina_Bool scale_up);
E_API void         e_icon_data_set         (Evas_Object *obj, void *data, int w, int h);
E_API void        *e_icon_data_get         (const Evas_Object *obj, int *w, int *h);
E_API void         e_icon_scale_size_set   (Evas_Object *obj, int size);
E_API int          e_icon_scale_size_get   (const Evas_Object *obj);
E_API void         e_icon_selected_set     (const Evas_Object *obj, Eina_Bool selected);
E_API void         e_icon_edje_emit        (const Evas_Object *obj, const char *sig, const char *src);
#endif
#endif
