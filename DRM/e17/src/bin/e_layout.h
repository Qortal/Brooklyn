#ifdef E_TYPEDEFS
#else
#ifndef E_LAYOUT_H
#define E_LAYOUT_H

E_API Evas_Object *e_layout_add               (Evas *evas);
E_API int          e_layout_freeze            (Evas_Object *obj);
E_API int          e_layout_thaw              (Evas_Object *obj);
E_API void         e_layout_virtual_size_set  (Evas_Object *obj, Evas_Coord w, Evas_Coord h);
E_API void         e_layout_virtual_size_get  (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

E_API void e_layout_coord_canvas_to_virtual (Evas_Object *obj, Evas_Coord cx, Evas_Coord cy, Evas_Coord *vx, Evas_Coord *vy);
E_API void e_layout_coord_virtual_to_canvas (Evas_Object *obj, Evas_Coord vx, Evas_Coord vy, Evas_Coord *cx, Evas_Coord *cy);

E_API void         e_layout_pack              (Evas_Object *obj, Evas_Object *child);
E_API void         e_layout_child_move        (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
E_API void         e_layout_child_resize      (Evas_Object *obj, Evas_Coord w, Evas_Coord h);
E_API void         e_layout_child_raise       (Evas_Object *obj);
E_API void         e_layout_child_lower       (Evas_Object *obj);
E_API void         e_layout_child_raise_above (Evas_Object *obj, Evas_Object *above);
E_API void         e_layout_child_lower_below (Evas_Object *obj, Evas_Object *below);
E_API void         e_layout_child_geometry_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
E_API void         e_layout_unpack            (Evas_Object *obj);

E_API Eina_List *e_layout_children_get(Evas_Object *obj);
E_API Evas_Object *e_layout_top_child_at_xy_get(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Eina_Bool vis, const Eina_List *ignore);

E_API Evas_Object *e_layout_child_below_get(Evas_Object *obj);
E_API Evas_Object *e_layout_child_above_get(Evas_Object *obj);
E_API Evas_Object *e_layout_top_child_get(Evas_Object *obj);
#endif
#endif
