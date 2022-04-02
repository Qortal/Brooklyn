#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_PREVIEW_H
#define E_WIDGET_PREVIEW_H

E_API Evas_Object     *e_widget_preview_add(Evas *evas, int minw, int minh);
E_API Evas            *e_widget_preview_evas_get(Evas_Object *obj);
E_API void             e_widget_preview_extern_object_set(Evas_Object *obj, Evas_Object *eobj);
E_API void             e_widget_preview_file_get(Evas_Object *obj, const char **file, const char **group);
E_API int              e_widget_preview_file_set(Evas_Object *obj, const char *file, const char *key);
E_API int	      e_widget_preview_thumb_set(Evas_Object *obj, const char *file, const char *key, int w, int h);
E_API int              e_widget_preview_edje_set(Evas_Object *obj, const char *file, const char *group);
E_API void             e_widget_preview_vsize_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
E_API void             e_widget_preview_size_set(Evas_Object *obj, int minw, int minh);
#endif
#endif
