#ifdef E_TYPEDEFS

typedef struct _E_Configure_Cat E_Configure_Cat;
typedef struct _E_Configure_It E_Configure_It;

#else
#ifndef E_CONFIGURE_H
#define E_CONFIGURE_H

struct _E_Configure_Cat
{
   const char *cat;
   int         pri;
   const char *label;
   const char *icon_file;
   const char *icon;
   Eina_List  *items;
};

struct _E_Configure_It
{
   const char        *item;
   int                pri;
   const char        *label;
   const char        *icon_file;
   const char        *icon;
   const char        *params;
   E_Config_Dialog *(*func) (Evas_Object *parent, const char *params);
   void             (*generic_func) (Evas_Object *parent, const char *params);
   Efreet_Desktop    *desktop;
};

E_API void e_configure_registry_item_add(const char *path, int pri, const char *label, const char *icon_file, const char *icon, E_Config_Dialog *(*func) (Evas_Object *parent, const char *params));
E_API void e_configure_registry_item_params_add(const char *path, int pri, const char *label, const char *icon_file, const char *icon, E_Config_Dialog *(*func) (Evas_Object *parent, const char *params), const char *params);
E_API void e_configure_registry_generic_item_add(const char *path, int pri, const char *label, const char *icon_file, const char *icon, void (*generic_func) (Evas_Object *parent, const char *params));
E_API void e_configure_registry_item_del(const char *path);
E_API void e_configure_registry_category_add(const char *path, int pri, const char *label, const char *icon_file, const char *icon);
E_API void e_configure_registry_category_del(const char *path);
E_API void e_configure_registry_call(const char *path, Evas_Object *parent, const char *params);
E_API int  e_configure_registry_exists(const char *path);
E_API void e_configure_registry_custom_desktop_exec_callback_set(void (*func) (const void *data, const char *params, Efreet_Desktop *desktop), const void *data);
EINTERN void e_configure_init(void);

extern E_API Eina_List *e_configure_registry;

#endif
#endif
