#ifdef E_TYPEDEFS

#else
#ifndef E_UTILS_H
#define E_UTILS_H

#define e_util_dialog_show(title, args...) \
do { \
   char __tmpbuf[PATH_MAX + 256]; \
   \
   snprintf(__tmpbuf, sizeof(__tmpbuf), ##args); \
   e_util_dialog_internal(title, __tmpbuf); \
} while (0)

E_API void         e_util_wakeup(void);
E_API void         e_util_env_set(const char *var, const char *val);
E_API int          e_util_glob_match(const char *str, const char *glob);
E_API int          e_util_glob_case_match(const char *str, const char *glob);
E_API int          e_util_strcasecmp(const char *s1, const char *s2);
E_API int          e_util_strcmp(const char *s1, const char *s2);
E_API int          e_util_both_str_empty(const char *s1, const char *s2);
E_API int          e_util_immortal_check(void);
E_API int          e_util_edje_icon_check(const char *name);
E_API int          e_util_edje_icon_set(Evas_Object *obj, const char *name);
E_API int          e_util_icon_theme_set(Evas_Object *obj, const char *icon);
E_API unsigned int e_util_icon_size_normalize(unsigned int desired);
E_API int          e_util_menu_item_theme_icon_set(E_Menu_Item *mi, const char *icon);
E_API const char *e_util_mime_icon_get(const char *mime, unsigned int size);
E_API E_Client    *e_util_desk_client_above(E_Client *ec);
E_API E_Client    *e_util_desk_client_below(E_Client *ec);
E_API int          e_util_edje_collection_exists(const char *file, const char *coll);
E_API E_Dialog     *e_util_dialog_internal(const char *title, const char *txt);
E_API const char  *e_util_filename_escape(const char *filename);
E_API char        *e_util_shell_env_path_eval(const char *path);
E_API char        *e_util_size_string_get(off_t size);
E_API char        *e_util_file_time_get(time_t ftime);
E_API Evas_Object *e_util_icon_add(const char *path, Evas *evas);
E_API Evas_Object *e_util_desktop_icon_add(Efreet_Desktop *desktop, unsigned int size, Evas *evas);
E_API Evas_Object *e_util_icon_theme_icon_add(const char *icon_name, unsigned int size, Evas *evas);
E_API void         e_util_desktop_menu_item_icon_add(Efreet_Desktop *desktop, unsigned int size, E_Menu_Item *mi);
E_API int          e_util_dir_check(const char *dir);
E_API void         e_util_defer_object_del(E_Object *obj);
E_API const char  *e_util_winid_str_get(Ecore_X_Window win);
E_API void         e_util_win_auto_resize_fill(Evas_Object *win);
/* check if loaded config version matches the current version, show a
   dialog warning if loaded version is older or newer than current */
E_API Eina_Bool    e_util_module_config_check(const char *module_name, int loaded, int current);

E_API E_Config_Binding_Key *e_util_binding_match(const Eina_List *bindlist, Ecore_Event_Key *ev, unsigned int *num, const E_Config_Binding_Key *skip);
E_API Eina_Bool e_util_fullscreen_current_any(void);
E_API Eina_Bool e_util_fullscreen_any(void);
E_API const char *e_util_time_str_get(long int seconds);
E_API void e_util_size_debug(Evas_Object *obj);
E_API void e_util_size_debug_set(Evas_Object *obj, Eina_Bool enable);
E_API Efreet_Desktop *e_util_terminal_desktop_get(void);
E_API void e_util_gadcon_orient_icon_set(E_Gadcon_Orient orient, Evas_Object *obj);
E_API void e_util_gadcon_orient_menu_item_icon_set(E_Gadcon_Orient orient, E_Menu_Item *mi);

E_API char *e_util_string_append_char(char *str, size_t *size, size_t *len, char c);
E_API char *e_util_string_append_quoted(char *str, size_t *size, size_t *len, const char *src);

E_API void e_util_evas_objects_above_print(Evas_Object *o);
E_API void e_util_evas_objects_above_print_smart(Evas_Object *o);

E_API void e_util_string_list_free(Eina_List *l);

E_API void e_util_memclear(void *s, size_t n);

E_API Ecore_Exe *e_util_open(const char *exe, void *data);

E_API Ecore_Exe *e_util_exe_safe_run(const char *cmd, void *data);

typedef enum
{
   E_UTIL_ACTION_NONE,
   E_UTIL_ACTION_DONE,
   E_UTIL_ACTION_ABORT,
   E_UTIL_ACTION_DO
} E_Util_Action;

E_API E_Util_Action e_util_key_geometry_action_get(const char *key, int *x, int *y, int dx, int dy);

static inline Eina_Bool
isedje(const Evas_Object *obj)
{
   return obj && !e_util_strcmp(evas_object_type_get(obj), "edje");
}

static inline Eina_Bool
dblequal(double a, double b)
{
   return fabs(a - b) < DBL_EPSILON;
}

static inline Eina_Bool
is_dragged(int dx, int dy)
{
   Evas_Coord resist = elm_config_finger_size_get() / 2;

   if (resist < 8) resist = 8;
   if (((dx * dx) + (dy * dy)) > (resist * resist)) return EINA_TRUE;
   return EINA_FALSE;
}

#endif
#endif
