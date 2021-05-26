#ifdef E_TYPEDEFS
#else
#ifndef E_SLIDESEL_H
#define E_SLIDESEL_H

E_API Evas_Object *e_slidesel_add               (Evas *evas);
E_API void         e_slidesel_item_distance_set (Evas_Object *obj, Evas_Coord dist);
E_API void         e_slidesel_item_add          (Evas_Object *obj, const char *label, const char *icon,  void (*func) (void *data), void *data);
E_API void         e_slidesel_jump              (Evas_Object *obj, int num);

#endif
#endif
