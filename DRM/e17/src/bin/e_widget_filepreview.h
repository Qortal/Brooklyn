#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_FILEPREVIEW_H
#define E_WIDGET_FILEPREVIEW_H

E_API Evas_Object *e_widget_filepreview_add(Evas *evas, int w, int h, int horiz);
E_API void e_widget_filepreview_path_set(Evas_Object *obj, const char *path, const char *mime);
E_API void e_widget_filepreview_filemode_force(Evas_Object *obj);
E_API void e_widget_filepreview_clamp_video_set(Evas_Object *obj, Eina_Bool clamp);

#endif
#endif
