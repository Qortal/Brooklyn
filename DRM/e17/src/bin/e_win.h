#ifndef E_TYPEDEFS
#ifndef E_WIN_H
#define E_WIN_H

EINTERN int    e_win_init               (void);
EINTERN int    e_win_shutdown           (void);


E_API E_Client *e_win_client_get(Evas_Object *obj);
E_API Ecore_Evas *e_win_ee_get(Evas_Object *obj);
E_API E_Pointer *e_win_pointer_get(Evas_Object *obj);
E_API Eina_Bool e_win_centered_get(Evas_Object *obj);

E_API void e_win_client_icon_set(Evas_Object *win, const char *icon);
E_API void e_win_client_icon_key_set(Evas_Object *win, const char *key);

E_API void e_win_placed_set(Evas_Object *win, Eina_Bool placed);
E_API void e_win_no_remember_set(Evas_Object *win, Eina_Bool no_rem);
E_API void e_win_no_reopen_set(Evas_Object *win, Eina_Bool no_reopen);

static inline Evas_Object *
e_win_evas_object_win_get(Evas_Object *obj)
{
   return ecore_evas_data_get(e_win_ee_get(obj), "elm_win");
}

static inline Evas_Object *
e_win_evas_win_get(Evas *e)
{
   return ecore_evas_data_get(ecore_evas_ecore_evas_get(e), "elm_win");
}

static inline Eina_Bool
e_obj_is_win(const void *obj)
{
   const char *type = evas_object_type_get(obj);
   return type && !strcmp(type, "elm_win");
}

E_API Evas_Object *e_elm_win_add(Evas_Object *parent, const char *name, Elm_Win_Type type);
E_API Evas_Object *elm_win_util_standard_add(const char *name, const char *title);
E_API Evas_Object *e_elm_win_util_dialog_add(Evas_Object *parent, const char *name, const char *title);

#define elm_win_add(X, Y, Z) \
   e_elm_win_add((X), (Y), (Z))

#define elm_win_util_dialog_add(X, Y, Z) \
   e_elm_win_util_dialog_add((X), (Y), (Z))

#define elm_win_util_standard_add(X, Y) \
   e_elm_win_util_standard_add((X), (Y))

#endif
#endif
