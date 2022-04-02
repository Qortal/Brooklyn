#ifdef E_TYPEDEFS

typedef struct _E_Int_Menu_Augmentation E_Int_Menu_Augmentation;
typedef struct _E_Int_Menu_Applications E_Int_Menu_Applications;

#else
#ifndef E_INT_MENUS_H
#define E_INT_MENUS_H

typedef enum
{
   E_CLIENTLIST_GROUP_NONE,
   E_CLIENTLIST_GROUP_DESK,
   E_CLIENTLIST_GROUP_CLASS
} E_Clientlist_Group_Type;

typedef enum
{
   E_CLIENTLIST_GROUP_SEP_NONE,
   E_CLIENTLIST_GROUP_SEP_BAR,
   E_CLIENTLIST_GROUP_SEP_MENU
} E_Clientlist_Group_Sep_Type;

typedef enum
{
   E_CLIENTLIST_SORT_NONE,
   E_CLIENTLIST_SORT_ALPHA,
   E_CLIENTLIST_SORT_ZORDER,
   E_CLIENTLIST_SORT_MOST_RECENT
} E_Clientlist_Group_Sort_Type;

typedef enum
{
   E_CLIENTLIST_GROUPICONS_OWNER,
   E_CLIENTLIST_GROUPICONS_CURRENT,
   E_CLIENTLIST_GROUPICONS_SEP,
} E_Clientlist_Groupicons_Type;


#define E_CLIENTLIST_MAX_CAPTION_LEN 256

struct _E_Int_Menu_Applications
{
   const char *orig_path;
   const char *try_exec;
   const char *exec;
   long long load_time;
   int exec_valid;
};

struct _E_Int_Menu_Augmentation
{
   const char *sort_key;
   struct
     {
        void (*func) (void *data, E_Menu *m);
        void *data;
     } add, del;
};

E_API E_Menu *e_int_menus_main_new(void);
E_API E_Menu *e_int_menus_desktops_new(void);
E_API E_Menu *e_int_menus_clients_new(void);
E_API E_Menu *e_int_menus_apps_new(const char *dir);
E_API E_Menu *e_int_menus_favorite_apps_new(void);
E_API E_Menu *e_int_menus_all_apps_new(void);
E_API E_Menu *e_int_menus_config_new(void);
E_API E_Menu *e_int_menus_lost_clients_new(void);
E_API E_Menu *e_int_menus_shelves_new(void);

E_API E_Int_Menu_Augmentation *e_int_menus_menu_augmentation_add(const char *menu,
								void (*func_add) (void *data, E_Menu *m),
								void *data_add,
								void (*func_del) (void *data, E_Menu *m),
								void *data_del);
E_API E_Int_Menu_Augmentation *e_int_menus_menu_augmentation_add_sorted(const char *menu,
								       const char *sort_key,
								       void (*func_add) (void *data, E_Menu *m),
								       void *data_add,
								       void (*func_del) (void *data, E_Menu *m),
								       void *data_del);
E_API void                     e_int_menus_menu_augmentation_del(const char *menu,
								E_Int_Menu_Augmentation *maug);

E_API void                     e_int_menus_menu_augmentation_point_disabled_set(const char *menu,
                                       Eina_Bool disabled);
E_API void e_int_menus_cache_clear(void);
EINTERN void e_int_menus_init(void);
EINTERN void e_int_menus_shutdown(void);
#endif
#endif
