#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_TOOLBAR_H
#define E_WIDGET_TOOLBAR_H

E_API Evas_Object *e_widget_toolbar_add(Evas *evas, int icon_w, int icon_h);
E_API void e_widget_toolbar_item_append(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data1, void *data2), const void *data1, const void *data2);
E_API void e_widget_toolbar_item_remove(Evas_Object *obj, int num);
E_API void e_widget_toolbar_item_select(Evas_Object *obj, int num);
E_API void e_widget_toolbar_item_label_set(Evas_Object *obj, int num, const char *label);
E_API void e_widget_toolbar_scrollable_set(Evas_Object *obj, Eina_Bool scrollable);
E_API void e_widget_toolbar_focus_steal_set(Evas_Object *obj, Eina_Bool steal);
E_API void e_widget_toolbar_clear(Evas_Object *obj);
E_API int e_widget_toolbar_item_selected_get(Evas_Object *obj);
E_API const Eina_List *e_widget_toolbar_items_get(Evas_Object *obj);
E_API const char *e_widget_toolbar_item_label_get(void *item);
E_API unsigned int e_widget_toolbar_items_count(Evas_Object *obj);
#endif
#endif
