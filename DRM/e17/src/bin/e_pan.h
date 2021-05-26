#ifdef E_TYPEDEFS
#else
#ifndef E_PAN_H
#define E_PAN_H

E_API Evas_Object *e_pan_add            (Evas *evas);
E_API void         e_pan_child_set      (Evas_Object *obj, Evas_Object *child);
E_API Evas_Object *e_pan_child_get      (Evas_Object *obj);
E_API void         e_pan_set            (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
E_API void         e_pan_get            (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
E_API void         e_pan_max_get        (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
E_API void         e_pan_child_size_get (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

#endif
#endif
