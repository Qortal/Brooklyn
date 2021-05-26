#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_FSEL_H
#define E_WIDGET_FSEL_H

E_API Evas_Object *e_widget_fsel_add(Evas *evas, const char *dev, const char *path, char *selected, char *filter,
				    void (*sel_func) (void *data, Evas_Object *obj), void *sel_data,
				    void (*chg_func) (void *data, Evas_Object *obj), void *chg_data, int preview);
E_API void e_widget_fsel_path_get(Evas_Object *obj, const char **dev, const char **path);
E_API const char *e_widget_fsel_selection_path_get(Evas_Object *obj);
E_API void e_widget_fsel_window_set(Evas_Object *obj, Evas_Object *win);
E_API Eina_Bool e_widget_fsel_typebuf_visible_get(Evas_Object *obj);
#endif
#endif
