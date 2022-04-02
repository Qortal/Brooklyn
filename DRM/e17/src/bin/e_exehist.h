#ifdef E_TYPEDEFS

#else
#ifndef E_EXEHIST_H
#define E_EXEHIST_H

typedef enum _E_Exehist_Sort
{
   E_EXEHIST_SORT_BY_DATE,
   E_EXEHIST_SORT_BY_EXE,
   E_EXEHIST_SORT_BY_POPULARITY
} E_Exehist_Sort;

EINTERN int e_exehist_init(void);
EINTERN int e_exehist_shutdown(void);

E_API void e_exehist_startup_id_set(int id);
E_API int e_exehist_startup_id_get(void);
E_API void e_exehist_add(const char *launch_method, const char *exe);
E_API void e_exehist_del(const char *exe);
E_API void e_exehist_clear(void);
E_API int e_exehist_popularity_get(const char *exe);
E_API double e_exehist_newest_run_get(const char *exe);
E_API Eina_List *e_exehist_list_get(void);
E_API Eina_List *e_exehist_sorted_list_get(E_Exehist_Sort sort_type, int max);
E_API void e_exehist_mime_desktop_add(const char *mime, Efreet_Desktop *desktop);
E_API Efreet_Desktop *e_exehist_mime_desktop_get(const char *mime);

extern E_API int E_EVENT_EXEHIST_UPDATE;

#endif
#endif
