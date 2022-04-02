#ifdef E_TYPEDEFS
#else
#ifndef E_SCROLLFRAME_H
#define E_SCROLLFRAME_H

typedef enum _E_Scrollframe_Policy
{
   E_SCROLLFRAME_POLICY_OFF,
     E_SCROLLFRAME_POLICY_ON,
     E_SCROLLFRAME_POLICY_AUTO
}
E_Scrollframe_Policy;

E_API Evas_Object *e_scrollframe_add             (Evas *evas);
E_API void e_scrollframe_child_set               (Evas_Object *obj, Evas_Object *child);
E_API void e_scrollframe_extern_pan_set          (Evas_Object *obj, Evas_Object *pan, void (*pan_set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y), void (*pan_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y), void (*pan_child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y));
E_API int  e_scrollframe_custom_theme_set        (Evas_Object *obj, const char *custom_category, const char *custom_group);
E_API int  e_scrollframe_custom_edje_file_set    (Evas_Object *obj, const char *file, const char *group);
E_API void e_scrollframe_child_pos_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
E_API void e_scrollframe_child_pos_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
E_API void e_scrollframe_child_region_show       (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
E_API void e_scrollframe_child_viewport_size_get (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
E_API void e_scrollframe_step_size_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
E_API void e_scrollframe_step_size_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
E_API void e_scrollframe_page_size_set           (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
E_API void e_scrollframe_page_size_get           (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
E_API void e_scrollframe_policy_set              (Evas_Object *obj, E_Scrollframe_Policy hbar, E_Scrollframe_Policy vbar);
E_API void e_scrollframe_policy_get              (Evas_Object *obj, E_Scrollframe_Policy *hbar, E_Scrollframe_Policy *vbar);
E_API Evas_Object *e_scrollframe_edje_object_get (Evas_Object *obj);
E_API void e_scrollframe_single_dir_set          (Evas_Object *obj, Eina_Bool single_dir);
E_API Eina_Bool e_scrollframe_single_dir_get     (Evas_Object *obj);
E_API void e_scrollframe_thumbscroll_force       (Evas_Object *obj, Eina_Bool forced);
E_API void e_scrollframe_key_navigation_set      (Evas_Object *obj, Eina_Bool enabled);

#endif
#endif
