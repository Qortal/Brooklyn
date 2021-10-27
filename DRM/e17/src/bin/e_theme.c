#include "e.h"

static void       e_theme_handler_set(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path);
static int        e_theme_handler_test(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path);

static E_Fm2_Mime_Handler *theme_hdl = NULL;

/* externally accessible functions */

EINTERN int
e_theme_init(void)
{
   /* Register mime handler */
   theme_hdl = e_fm2_mime_handler_new(_("Set As Theme"), "preferences-desktop-theme",
                                      e_theme_handler_set, NULL,
                                      e_theme_handler_test, NULL);
   if (theme_hdl) e_fm2_mime_handler_glob_add(theme_hdl, "*.edj");
   return 1;
}

EINTERN int
e_theme_shutdown(void)
{
   if (theme_hdl)
     {
        e_fm2_mime_handler_glob_del(theme_hdl, "*.edj");
        e_fm2_mime_handler_free(theme_hdl);
     }
   return 1;
}

E_API Eina_List *
e_theme_collection_items_find(const char *base EINA_UNUSED, const char *collname)
{
   Eina_List *list, *list2 = NULL, *l;
   const char *s, *s2;
   size_t len = strlen(collname);

   list = elm_theme_group_base_list(NULL, collname);
   if (!list) return NULL;
   EINA_LIST_FREE(list, s)
     {
        char *trans, *p, *p2;
        size_t slen;

        slen = strlen(s);
        if (slen <= len) goto done;
        trans = alloca(slen + 1);
        if (!trans) goto done;
        memcpy(trans, s, slen + 1);
        p = trans + len;
        if (*p != '/') goto done;
        p++;
        if (*p)
          {
             p2 = strchr(p, '/');
             if (p2) *p2 = 0;
             EINA_LIST_FOREACH(list2, l, s2)
               {
                  if (!strcmp(s2, p)) break;
               }
             if (!l) list2 = eina_list_append(list2, eina_stringshare_add(p));
          }
done:
        eina_stringshare_del(s);
     }
   return list2;
}

E_API int
e_theme_edje_object_set(Evas_Object *o, const char *category EINA_UNUSED, const char *group)
{
   const char *file;

   file = elm_theme_group_path_find(NULL, group);
   if (!file) return 0;
   edje_object_file_set(o, file, group);
   return 1;
}

E_API const char *
e_theme_edje_file_get(const char *category EINA_UNUSED, const char *group)
{
   const char *file = elm_theme_group_path_find(NULL, group);
   if (!file) return "";
   return file;
}

E_API const char *
e_theme_edje_icon_fallback_file_get(const char *group)
{
   const char *file;

   if ((e_config->icon_theme) && (!strncmp(group, "e/icons", 7))) return "";
   file = elm_theme_group_path_find(NULL, group);
   if (!file) return "";
   return file;
}

E_API int
e_theme_transition_find(const char *transition)
{
   Eina_List *trans = NULL;
   int found = 0;
   const char *str;

   trans = e_theme_collection_items_find(NULL, "e/transitions");
   if (eina_list_search_sorted(trans, EINA_COMPARE_CB(strcmp), transition))
     found = 1;
   EINA_LIST_FREE(trans, str) eina_stringshare_del(str);
   return found;
}

E_API Eina_List *
e_theme_transition_list(void)
{
   return e_theme_collection_items_find(NULL, "e/transitions");
}

E_API int
e_theme_border_find(const char *border)
{
   Eina_List *bds = NULL;
   int found = 0;
   const char *str;

   bds = e_theme_collection_items_find(NULL, "e/widgets/border");
   if (eina_list_search_sorted(bds, EINA_COMPARE_CB(strcmp), border))
     found = 1;
   EINA_LIST_FREE(bds, str) eina_stringshare_del(str);
   return found;
}

E_API Eina_List *
e_theme_border_list(void)
{
   return e_theme_collection_items_find(NULL, "e/widgets/border");
}

E_API int
e_theme_shelf_find(const char *shelf)
{
   Eina_List *shelfs = NULL;
   int found = 0;
   const char *str;

   shelfs = e_theme_collection_items_find(NULL, "e/shelf");
   if (eina_list_search_sorted(shelfs, EINA_COMPARE_CB(strcmp), shelf))
     found = 1;
   EINA_LIST_FREE(shelfs, str) eina_stringshare_del(str);
   return found;
}

E_API Eina_List *
e_theme_shelf_list(void)
{
   return e_theme_collection_items_find(NULL, "e/shelf");
}

E_API int
e_theme_comp_frame_find(const char *comp)
{
   Eina_List *comps = NULL;
   int found = 0;
   const char *str;

   comps = e_theme_collection_items_find(NULL, "e/comp/frame");
   if (eina_list_search_sorted(comps, EINA_COMPARE_CB(strcmp), comp))
     found = 1;
   EINA_LIST_FREE(comps, str) eina_stringshare_del(str);
   return found;
}

E_API Eina_List *
e_theme_comp_frame_list(void)
{
   return e_theme_collection_items_find(NULL, "e/comp/frame");
}

/* local subsystem functions */
static void
e_theme_handler_set(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path)
{
   E_Action *a;
   char buf[PATH_MAX];
   int copy = 1;

   if (!path) return;
   /* if not in system dir or user dir, copy to user dir */
   snprintf(buf, sizeof(buf), "%s", elm_theme_system_dir_get());
   if (!strncmp(buf, path, strlen(buf))) copy = 0;
   if (copy)
     {
        snprintf(buf, sizeof(buf), "%s", elm_theme_user_dir_get());
        if (!strncmp(buf, path, strlen(buf))) copy = 0;
     }
   if (copy)
     {
        const char *file;
        char *name;

        file = ecore_file_file_get(path);
        name = ecore_file_strip_ext(file);

        if (name)
          {
             double tim = ecore_time_unix_get();

             snprintf(buf, sizeof(buf), "%s/%s-%f.edj",
                      elm_theme_user_dir_get(), name, tim);
             if (!ecore_file_exists(buf))
               {
                  ecore_file_cp(path, buf);
                  snprintf(buf, sizeof(buf), "%s-%f", name, tim);
                  elm_theme_set(NULL, buf);
               }
             else
               elm_theme_set(NULL, path);
             elm_config_all_flush();
             elm_config_save();
             free(name);
          }
     }
   else
     {
        const char *file;
        char *name;

        file = ecore_file_file_get(path);
        name = ecore_file_strip_ext(file);
        if (name)
          {
             elm_theme_set(NULL, name);
             elm_config_all_flush();
             elm_config_save();
             free(name);
          }
     }

   e_config_save_queue();
   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
}

static int
e_theme_handler_test(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *path)
{
   if (!path) return 0;
   if (!edje_file_group_exists(path, "e/widgets/border/default/border"))
     return 0;
   return 1;
}
