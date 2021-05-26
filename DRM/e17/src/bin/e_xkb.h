#ifdef E_TYPEDEFS
#else
#ifndef E_XKB_H
#define E_XKB_H

E_API int e_xkb_init(E_Pixmap_Type comp_type);
E_API int e_xkb_shutdown(void);
E_API void e_xkb_reconfig(void);
E_API void e_xkb_update(int);
E_API void e_xkb_layout_next(void);
E_API void e_xkb_layout_prev(void);
E_API E_Config_XKB_Layout *e_xkb_layout_get(void);
E_API void e_xkb_layout_set(const E_Config_XKB_Layout *cl);
E_API const char *e_xkb_layout_name_reduce(const char *name);
E_API void e_xkb_e_icon_flag_setup(Evas_Object *eicon, const char *name);
E_API void e_xkb_flag_file_get(char *buf, size_t bufsize, const char *name);

E_API Eina_Bool e_config_xkb_layout_eq(const E_Config_XKB_Layout *a, const E_Config_XKB_Layout *b);
E_API void e_config_xkb_layout_free(E_Config_XKB_Layout *cl);
E_API E_Config_XKB_Layout *e_config_xkb_layout_dup(const E_Config_XKB_Layout *cl);

extern E_API int E_EVENT_XKB_CHANGED;

#endif
#endif
