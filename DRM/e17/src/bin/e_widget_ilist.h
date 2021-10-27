#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_ILIST_H
#define E_WIDGET_ILIST_H

E_API Evas_Object *e_widget_ilist_add(Evas *evas, int icon_w, int icon_h, const char **value);
E_API void         e_widget_ilist_freeze(Evas_Object *obj);
E_API void         e_widget_ilist_thaw(Evas_Object *obj);
E_API void         e_widget_ilist_append(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data), void *data, const char *val);
E_API void         e_widget_ilist_append_relative(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data), void *data, const char *val, int relative);
E_API void         e_widget_ilist_prepend(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data), void *data, const char *val);
E_API void         e_widget_ilist_prepend_relative(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data), void *data, const char *val, int relative);

E_API void         e_widget_ilist_append_full(Evas_Object *obj, Evas_Object *icon, Evas_Object *end, const char *label, void (*func) (void *data), void *data, const char *val);
E_API void         e_widget_ilist_append_relative_full(Evas_Object *obj, Evas_Object *icon, Evas_Object *end, const char *label, void (*func) (void *data), void *data, const char *val, int relative);
E_API void         e_widget_ilist_prepend_full(Evas_Object *obj, Evas_Object *icon, Evas_Object *end, const char *label, void (*func) (void *data), void *data, const char *val);
E_API void         e_widget_ilist_prepend_relative_full(Evas_Object *obj, Evas_Object *icon, Evas_Object *end, const char *label, void (*func) (void *data), void *data, const char *val, int relative);

E_API void         e_widget_ilist_header_append_relative(Evas_Object *obj, Evas_Object *icon, const char *label, int relative);
E_API void         e_widget_ilist_header_prepend_relative(Evas_Object *obj, Evas_Object *icon, const char *label, int relative);
E_API void         e_widget_ilist_header_prepend(Evas_Object *obj, Evas_Object *icon, const char *label);
E_API void         e_widget_ilist_header_append(Evas_Object *obj, Evas_Object *icon, const char *label);
E_API void         e_widget_ilist_selector_set(Evas_Object *obj, int selector);
E_API void         e_widget_ilist_go(Evas_Object *obj);
E_API void         e_widget_ilist_clear(Evas_Object *obj);
E_API int          e_widget_ilist_count(Evas_Object *obj);
E_API Eina_List   const *e_widget_ilist_items_get(Evas_Object *obj);

E_API Eina_Bool    e_widget_ilist_nth_is_header(Evas_Object *obj, int n);
E_API void         e_widget_ilist_nth_label_set(Evas_Object *obj, int n, const char *label);
E_API const char  *e_widget_ilist_nth_label_get(Evas_Object *obj, int n);
E_API void         e_widget_ilist_nth_icon_set(Evas_Object *obj, int n, Evas_Object *icon);
E_API Evas_Object *e_widget_ilist_nth_icon_get(Evas_Object *obj, int n);
E_API void         e_widget_ilist_nth_end_set(Evas_Object *obj, int n, Evas_Object *end);
E_API Evas_Object *e_widget_ilist_nth_end_get(Evas_Object *obj, int n);
E_API void        *e_widget_ilist_nth_data_get(Evas_Object *obj, int n);
E_API const char  *e_widget_ilist_nth_value_get(Evas_Object *obj, int n);

E_API Eina_Bool    e_widget_ilist_item_is_header(const E_Ilist_Item *it);
E_API const char  *e_widget_ilist_item_label_get(const E_Ilist_Item *it);
E_API Evas_Object *e_widget_ilist_item_icon_get(const E_Ilist_Item *it);
E_API Evas_Object *e_widget_ilist_item_end_get(const E_Ilist_Item *it);
E_API void        *e_widget_ilist_item_data_get(const E_Ilist_Item *it);
E_API const char  *e_widget_ilist_item_value_get(const E_Ilist_Item *it);

E_API void         e_widget_ilist_nth_show(Evas_Object *obj, int n, int top);
E_API void         e_widget_ilist_selected_set(Evas_Object *obj, int n);
E_API const Eina_List *e_widget_ilist_selected_items_get(Evas_Object *obj);
E_API int          e_widget_ilist_selected_get(Evas_Object *obj);
E_API const char  *e_widget_ilist_selected_label_get(Evas_Object *obj);
E_API Evas_Object *e_widget_ilist_selected_icon_get(Evas_Object *obj);
E_API Evas_Object *e_widget_ilist_selected_end_get(Evas_Object *obj);
E_API void        *e_widget_ilist_selected_data_get(Evas_Object *obj);
E_API int          e_widget_ilist_selected_count_get(Evas_Object *obj);
E_API const char  *e_widget_ilist_selected_value_get(Evas_Object *obj);

E_API void         e_widget_ilist_unselect(Evas_Object *obj);
E_API void         e_widget_ilist_remove_num(Evas_Object *obj, int n);
E_API void         e_widget_ilist_multi_select_set(Evas_Object *obj, Eina_Bool multi);
E_API Eina_Bool    e_widget_ilist_multi_select_get(Evas_Object *obj);
E_API void         e_widget_ilist_multi_select(Evas_Object *obj, int n);
E_API void         e_widget_ilist_range_select(Evas_Object *obj, int n);
E_API void         e_widget_ilist_preferred_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
E_API Eina_Bool   e_widget_ilist_custom_edje_file_set(Evas_Object *obj, const char *file, const char *group);
#endif
#endif
