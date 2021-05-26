#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_CONFIG_LIST_H
#define E_WIDGET_CONFIG_LIST_H


E_API Evas_Object *e_widget_config_list_add(Evas *evas, Evas_Object *(*func_entry_add) (Evas_Object *, char **val, void (*func) (void *data, void *data2), void *data, void *data2), const char *label, int listspan);
E_API int e_widget_config_list_count(Evas_Object *obj);
E_API const char *e_widget_config_list_nth_get(Evas_Object *obj, int n);
E_API void e_widget_config_list_append(Evas_Object *obj, const char *entry);
E_API void e_widget_config_list_object_append(Evas_Object *obj, Evas_Object *sobj, int col, int row, int colspan, int rowspan, int fill_w, int fill_h, int expand_w, int expand_h);
E_API void e_widget_config_list_clear(Evas_Object *obj);

#endif
#endif
