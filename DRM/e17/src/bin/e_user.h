#ifdef E_TYPEDEFS
#else
#ifndef E_USER_H
#define E_USER_H

E_API const char *e_user_homedir_get(void);
E_API size_t      e_user_homedir_concat_len(char *dst, size_t size, const char *path, size_t path_len);
E_API size_t      e_user_homedir_concat(char *dst, size_t size, const char *path);
E_API size_t      e_user_homedir_snprintf(char *dst, size_t size, const char *fmt, ...) EINA_PRINTF(3, 4);

#define e_user_homedir_concat_static(dst, path) e_user_homedir_concat_len(dst, sizeof(dst), path, (sizeof(path) > 0) ? sizeof(path) - 1 : 0)

E_API const char *e_user_dir_get(void);
E_API size_t      e_user_dir_concat_len(char *dst, size_t size, const char *path, size_t path_len);
E_API size_t      e_user_dir_concat(char *dst, size_t size, const char *path);
E_API size_t      e_user_dir_snprintf(char *dst, size_t size, const char *fmt, ...) EINA_PRINTF(3, 4);

#define e_user_dir_concat_static(dst, path) e_user_dir_concat_len(dst, sizeof(dst), path,  (sizeof(path) > 0) ? sizeof(path) - 1 : 0)

E_API const char *e_user_desktop_dir_get(void);
E_API const char *e_user_icon_dir_get(void);

#endif
#endif
