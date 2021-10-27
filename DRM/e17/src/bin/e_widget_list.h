#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_LIST_H
#define E_WIDGET_LIST_H

E_API Evas_Object *e_widget_list_add(Evas *evas, int homogenous, int horiz);
E_API void e_widget_list_object_append(Evas_Object *obj, Evas_Object *sobj, int fill, int expand, double align);
E_API void e_widget_list_object_prepend(Evas_Object *obj, Evas_Object *sobj, int fill, int expand, double align);
E_API void e_widget_list_object_repack(Evas_Object *obj, Evas_Object *sobj, int fill, int expand, double align);
E_API void e_widget_list_homogeneous_set(Evas_Object *obj, int homogenous);

#endif
#endif
