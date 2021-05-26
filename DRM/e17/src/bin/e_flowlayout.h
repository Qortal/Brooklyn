#ifdef E_TYPEDEFS
#else
#ifndef E_FLOWLAYOUT_H
#define E_FLOWLAYOUT_H

E_API Evas_Object *e_flowlayout_add               (Evas *evas);
E_API int          e_flowlayout_freeze            (Evas_Object *obj);
E_API int          e_flowlayout_thaw              (Evas_Object *obj);
E_API void         e_flowlayout_orientation_set   (Evas_Object *obj, int horizontal);
E_API int          e_flowlayout_orientation_get   (Evas_Object *obj);
E_API void         e_flowlayout_flowdirection_set (Evas_Object *obj, int right, int bottom);
E_API void         e_flowlayout_flowdirection_get (Evas_Object *obj, int *right, int *bottom);
E_API void         e_flowlayout_homogenous_set    (Evas_Object *obj, int homogenous);
E_API int          e_flowlayout_fill_get          (Evas_Object *obj);
E_API void         e_flowlayout_fill_set          (Evas_Object *obj, int fill);
E_API int          e_flowlayout_pack_start        (Evas_Object *obj, Evas_Object *child);
E_API int          e_flowlayout_pack_end          (Evas_Object *obj, Evas_Object *child);
E_API int          e_flowlayout_pack_before       (Evas_Object *obj, Evas_Object *child, Evas_Object *before);
E_API int          e_flowlayout_pack_after        (Evas_Object *obj, Evas_Object *child, Evas_Object *after);
E_API int          e_flowlayout_pack_count_get    (Evas_Object *obj);
E_API Evas_Object *e_flowlayout_pack_object_nth   (Evas_Object *obj, int n);
E_API Evas_Object *e_flowlayout_pack_object_first (Evas_Object *obj);
E_API Evas_Object *e_flowlayout_pack_object_last  (Evas_Object *obj);
E_API Evas_Object *e_flowlayout_pack_object_prev(Evas_Object *obj, Evas_Object *child);
E_API Evas_Object *e_flowlayout_pack_object_next(Evas_Object *obj, Evas_Object *child);
E_API void         e_flowlayout_pack_options_set  (Evas_Object *obj, int fill_w, int fill_h, int expand_w, int expand_h, double align_x, double align_y, Evas_Coord min_w, Evas_Coord min_h, Evas_Coord max_w, Evas_Coord max_h);
E_API void         e_flowlayout_unpack            (Evas_Object *obj);
E_API void         e_flowlayout_size_min_get      (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);
E_API void         e_flowlayout_size_max_get      (Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);
E_API void         e_flowlayout_align_get         (Evas_Object *obj, double *ax, double *ay);
E_API void         e_flowlayout_align_set         (Evas_Object *obj, double ax, double ay);
/* This function only works if homogenous is set */
E_API int          e_flowlayout_max_children      (Evas_Object *obj);

#endif
#endif
