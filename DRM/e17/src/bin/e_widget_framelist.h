#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_FRAMELIST_H
#define E_WIDGET_FRAMELIST_H

E_API Evas_Object *e_widget_framelist_add(Evas *evas, const char *label, int horiz);
E_API void e_widget_framelist_object_append(Evas_Object *obj, Evas_Object *sobj);
E_API void e_widget_framelist_object_append_full(Evas_Object *obj, Evas_Object *sobj, int fill_w, int fill_h, int expand_w, int expand_h, double align_x, double align_y, Evas_Coord min_w, Evas_Coord min_h, Evas_Coord max_w, Evas_Coord max_h);

E_API void e_widget_framelist_content_align_set(Evas_Object *obj, double halign, double valign);
E_API void e_widget_framelist_label_set(Evas_Object *obj, const char *label);

#endif
#endif
