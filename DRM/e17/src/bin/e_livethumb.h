#ifdef E_TYPEDEFS
#else
#ifndef E_LIVETHUMB_H
#define E_LIVETHUMB_H

E_API Evas_Object *e_livethumb_add                   (Evas *e);
E_API Evas        *e_livethumb_evas_get              (Evas_Object *obj);
E_API void         e_livethumb_vsize_set             (Evas_Object *obj, Evas_Coord w, Evas_Coord h);
E_API void         e_livethumb_vsize_get             (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
E_API void         e_livethumb_thumb_set             (Evas_Object *obj, Evas_Object *thumb);
E_API Evas_Object *e_livethumb_thumb_get             (Evas_Object *obj);

#endif
#endif
