#ifndef E_WIDGET_DESKMIRROR_H
#define E_WIDGET_DESKMIRROR_H

E_API Evas_Object *e_deskmirror_add(E_Desk *desk, Eina_Bool pager, Eina_Bool taskbar);
E_API Evas_Object *e_deskmirror_mirror_find(Evas_Object *deskmirror, Evas_Object *comp_object);
E_API Eina_List *e_deskmirror_mirror_list(Evas_Object *deskmirror);
E_API Evas_Object *e_deskmirror_mirror_copy(Evas_Object *obj);
E_API void e_deskmirror_coord_canvas_to_virtual(Evas_Object *obj, Evas_Coord cx, Evas_Coord cy, Evas_Coord *vx, Evas_Coord *vy);
E_API void e_deskmirror_coord_virtual_to_canvas(Evas_Object *obj, Evas_Coord vx, Evas_Coord vy, Evas_Coord *cx, Evas_Coord *cy);
E_API E_Desk *e_deskmirror_desk_get(Evas_Object *obj);
E_API void e_deskmirror_util_wins_print(Evas_Object *obj);
E_API void e_deskmirror_update_force(Evas_Object *obj);
//#define DESKMIRROR_TEST

#endif
