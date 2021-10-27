#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_SCROLLFRAME_H
#define E_WIDGET_SCROLLFRAME_H

E_API Evas_Object *e_widget_scrollframe_pan_add(Evas *evas, Evas_Object *pan, void (*pan_set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y), void (*pan_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y));
E_API Evas_Object *e_widget_scrollframe_simple_add(Evas *evas, Evas_Object *child);
E_API void e_widget_scrollframe_child_pos_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
E_API void e_widget_scrollframe_child_pos_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
E_API void e_widget_scrollframe_child_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
E_API void e_widget_scrollframe_focus_object_set(Evas_Object *obj, Evas_Object *fobj);
E_API Evas_Object *e_widget_scrollframe_object_get(Evas_Object *obj);

#endif
#endif
