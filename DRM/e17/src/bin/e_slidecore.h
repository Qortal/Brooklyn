#ifdef E_TYPEDEFS
#else
#ifndef E_SLIDECORE_H
#define E_SLIDECORE_H

E_API Evas_Object *e_slidecore_add              (Evas *evas);
E_API void         e_slidecore_item_distance_set(Evas_Object *obj, Evas_Coord dist);
E_API void         e_slidecore_item_add         (Evas_Object *obj, const char *label, const char *icon, void (*func) (void *data), void *data);
E_API void         e_slidecore_jump             (Evas_Object *obj, int num);
E_API void         e_slidecore_slide_time_set   (Evas_Object *obj, double t);

#endif
#endif
