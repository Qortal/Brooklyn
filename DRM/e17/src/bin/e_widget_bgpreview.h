#ifndef E_WIDGET_DESK_PREVIEW_H
#define E_WIDGET_DESK_PREVIEW_H

E_API Evas_Object *e_widget_bgpreview_add(Evas *evas, int nx, int ny);
E_API void e_widget_bgpreview_num_desks_set(Evas_Object *obj, int nx, int ny);
E_API Evas_Object *e_widget_bgpreview_desk_add(Evas *e, E_Zone *zone, int x, int y);
E_API void e_widget_bgpreview_desk_configurable_set(Evas_Object *obj, Eina_Bool enable);

#endif
