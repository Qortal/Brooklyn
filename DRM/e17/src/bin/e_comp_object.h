#ifdef E_TYPEDEFS
typedef struct E_Comp_Object_Frame E_Comp_Object_Frame;
typedef struct E_Event_Comp_Object E_Event_Comp_Object;
typedef void (*E_Comp_Object_Autoclose_Cb)(void *, Evas_Object *);
typedef Eina_Bool (*E_Comp_Object_Key_Cb)(void *, Ecore_Event_Key *);
typedef Eina_Bool (*E_Comp_Object_Mover_Cb) (void *data, Evas_Object *comp_object, const char *signal);

typedef struct E_Comp_Object_Mover E_Comp_Object_Mover;

typedef enum
{
   E_COMP_OBJECT_TYPE_NONE,
   E_COMP_OBJECT_TYPE_MENU,
   E_COMP_OBJECT_TYPE_POPUP,
   E_COMP_OBJECT_TYPE_LAST,
} E_Comp_Object_Type;

#else
#ifndef E_COMP_OBJECT_H
#define E_COMP_OBJECT_H

#define E_COMP_OBJECT_FRAME_RESHADOW "COMP_RESHADOW"

struct E_Event_Comp_Object
{
   Evas_Object *comp_object;
};

struct E_Comp_Object_Frame
{
   int l, r, t, b;
   Eina_Bool calc E_BITFIELD; // inset has been calculated
};


extern E_API int E_EVENT_COMP_OBJECT_ADD;

E_API void e_comp_object_zoomap_set(Evas_Object *obj, Eina_Bool enabled);
E_API Eina_Bool e_comp_object_mirror_visibility_check(Evas_Object *obj);
E_API Evas_Object *e_comp_object_client_add(E_Client *ec);
E_API Evas_Object *e_comp_object_util_mirror_add(Evas_Object *obj);
E_API Evas_Object *e_comp_object_util_frame_mirror_add(Evas_Object *obj);
E_API void e_comp_object_util_type_set(Evas_Object *obj, E_Comp_Object_Type type);
E_API Evas_Object *e_comp_object_util_add(Evas_Object *obj, E_Comp_Object_Type type);
E_API Evas_Object *e_comp_object_util_get(Evas_Object *obj);
E_API void e_comp_object_frame_xy_adjust(Evas_Object *obj, int x, int y, int *ax, int *ay);
E_API void e_comp_object_frame_xy_unadjust(Evas_Object *obj, int x, int y, int *ax, int *ay);
E_API void e_comp_object_frame_wh_adjust(Evas_Object *obj, int w, int h, int *aw, int *ah);
E_API void e_comp_object_frame_wh_unadjust(Evas_Object *obj, int w, int h, int *aw, int *ah);
E_API void e_comp_object_frame_extends_get(Evas_Object *obj, int *x, int *y, int *w, int *h);
E_API E_Client *e_comp_object_client_get(Evas_Object *obj);
E_API E_Zone *e_comp_object_util_zone_get(Evas_Object *obj);
E_API void e_comp_object_util_del_list_append(Evas_Object *obj, Evas_Object *to_del);
E_API void e_comp_object_util_del_list_remove(Evas_Object *obj, Evas_Object *to_del);
E_API Eina_Bool e_comp_object_util_autoclose_on_escape(void *d EINA_UNUSED, Ecore_Event_Key *ev);
E_API void e_comp_object_util_autoclose(Evas_Object *obj, E_Comp_Object_Autoclose_Cb del_cb, E_Comp_Object_Key_Cb cb, const void *data);
E_API void e_comp_object_util_center(Evas_Object *obj);
E_API void e_comp_object_util_center_on(Evas_Object *obj, Evas_Object *on);
E_API void e_comp_object_util_center_on_zone(Evas_Object *obj, E_Zone *zone);
E_API void e_comp_object_util_center_pos_get(Evas_Object *obj, int *x, int *y);
E_API void e_comp_object_util_fullscreen(Evas_Object *obj);
E_API Eina_Bool e_comp_object_frame_allowed(Evas_Object *obj);
E_API void e_comp_object_frame_geometry_get(Evas_Object *obj, int *l, int *r, int *t, int *b);
E_API void e_comp_object_frame_geometry_set(Evas_Object *obj, int l, int r, int t, int b);
E_API void e_comp_object_frame_icon_geometry_get(Evas_Object *obj, int *x, int *y, int *w, int *h);
E_API Eina_Bool e_comp_object_frame_title_set(Evas_Object *obj, const char *name);
E_API Eina_Bool e_comp_object_frame_exists(Evas_Object *obj);
E_API Eina_Bool e_comp_object_frame_icon_update(Evas_Object *obj);
E_API Eina_Bool e_comp_object_frame_theme_set(Evas_Object *obj, const char *name);
E_API void e_comp_object_signal_emit(Evas_Object *obj, const char *sig, const char *src);
E_API void e_comp_object_signal_callback_add(Evas_Object *obj, const char *sig, const char *src, Edje_Signal_Cb cb, const void *data);
E_API void e_comp_object_signal_callback_del(Evas_Object *obj, const char *sig, const char *src, Edje_Signal_Cb cb);
E_API void e_comp_object_signal_callback_del_full(Evas_Object *obj, const char *sig, const char *src, Edje_Signal_Cb cb, const void *data);
E_API void e_comp_object_input_area_set(Evas_Object *obj, const Eina_Tiler *area);
E_API Eina_Bool e_comp_object_coords_inside_input_area(Evas_Object *obj, int x, int y);
E_API void e_comp_object_damage(Evas_Object *obj, int x, int y, int w, int h);
E_API Eina_Bool e_comp_object_damage_exists(Evas_Object *obj);
E_API void e_comp_object_render_update_add(Evas_Object *obj);
E_API void e_comp_object_render_update_del(Evas_Object *obj);
E_API void e_comp_object_shape_apply(Evas_Object *obj);
E_API void e_comp_object_redirected_set(Evas_Object *obj, Eina_Bool set);
E_API void e_comp_object_native_surface_set(Evas_Object *obj, Eina_Bool set);
E_API void e_comp_object_native_surface_override(Evas_Object *obj, Evas_Native_Surface *ns);
E_API Evas_Object *e_comp_object_agent_add(Evas_Object *obj);
E_API void e_comp_object_blank(Evas_Object *obj, Eina_Bool set);
E_API void e_comp_object_dirty(Evas_Object *obj);
E_API Eina_Bool e_comp_object_render(Evas_Object *obj);
E_API Eina_Bool e_comp_object_effect_allowed_get(Evas_Object *obj);
E_API Eina_Bool e_comp_object_effect_set(Evas_Object *obj, const char *effect);
E_API void e_comp_object_effect_params_set(Evas_Object *obj, int id, int *params, unsigned int count);
E_API void e_comp_object_effect_clip(Evas_Object *obj);
E_API void e_comp_object_effect_unclip(Evas_Object *obj);
E_API Eina_Bool e_comp_object_effect_start(Evas_Object *obj, Edje_Signal_Cb end_cb, const void *end_data);
E_API Eina_Bool e_comp_object_effect_stop(Evas_Object *obj, Edje_Signal_Cb end_cb);
E_API E_Comp_Object_Mover *e_comp_object_effect_mover_add(int pri, const char *sig, E_Comp_Object_Mover_Cb provider, const void *data);
E_API void e_comp_object_effect_mover_del(E_Comp_Object_Mover *prov);
E_API void e_comp_object_frame_volume_update(Evas_Object *obj);
E_API Evas_Object *e_comp_object_frame_icon_get(Evas_Object *obj);
E_API Evas_Object *e_comp_object_frame_volume_get(Evas_Object *obj);

#endif
#endif

