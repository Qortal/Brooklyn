#include "e_mod_main.h"
#include "e_fm_device.h"

static void _e_mod_menu_populate(void *d, E_Menu *m EINA_UNUSED, E_Menu_Item *mi);

static E_Menu *
_e_mod_menu_top_get(E_Menu *m)
{
   while (m->parent_item)
     {
        if (m->parent_item->menu->header.title)
          break;
        m = m->parent_item->menu;
     }
   return m;
}

static void
_e_mod_menu_gtk_cb(void           *data,
                   E_Menu         *m,
                   E_Menu_Item *mi EINA_UNUSED)
{
   Evas_Object *fm;

   m = _e_mod_menu_top_get(m);
   fm = e_object_data_get(E_OBJECT(m));
   if (fm && ((fileman_config->view.open_dirs_in_place && evas_object_data_get(fm, "page_is_window")) ||
       (fileman_config->view.desktop_navigation && evas_object_data_get(fm, "page_is_zone"))))
     e_fm2_path_set(fm, NULL, data);
   else if (m->zone) e_fwin_new(NULL, data);
}

static void
_e_mod_menu_virtual_cb(void           *data,
                       E_Menu         *m,
                       E_Menu_Item *mi)
{
   Evas_Object *fm;
   Eina_Stringshare *path = e_object_data_get(E_OBJECT(mi));

   m = _e_mod_menu_top_get(m);
   fm = e_object_data_get(E_OBJECT(m));
   if (fm && ((fileman_config->view.open_dirs_in_place && evas_object_data_get(fm, "page_is_window")) ||
       (fileman_config->view.desktop_navigation && evas_object_data_get(fm, "page_is_zone"))))
     e_fm2_path_set(fm, data, path ?: "/");
   else if (m->zone) e_fwin_new(data, path ?: "/");
}

static void
_e_mod_menu_volume_cb(void           *data,
                      E_Menu         *m,
                      E_Menu_Item *mi EINA_UNUSED)
{
   E_Volume *vol = data;
   Evas_Object *fm;

   m = _e_mod_menu_top_get(m);
   fm = e_object_data_get(E_OBJECT(m));
   if (vol->mounted)
     {
       if (fm && ((fileman_config->view.open_dirs_in_place && evas_object_data_get(fm, "page_is_window")) ||
           (fileman_config->view.desktop_navigation && evas_object_data_get(fm, "page_is_zone"))))
         e_fm2_path_set(fm, NULL, vol->mount_point);
        else if (m->zone)
          e_fwin_new(NULL, vol->mount_point);
     }
   else
     {
        char buf[PATH_MAX + sizeof("removable:")];

        snprintf(buf, sizeof(buf), "removable:%s", vol->udi);
        if (fm && ((fileman_config->view.open_dirs_in_place && evas_object_data_get(fm, "page_is_window")) ||
            (fileman_config->view.desktop_navigation && evas_object_data_get(fm, "page_is_zone"))))
          e_fm2_path_set(fm, buf, "/");
        else if (m->zone)
          e_fwin_new(buf, "/");
     }
}

static void
_e_mod_menu_populate_cb(void      *data,
                       E_Menu      *m,
                       E_Menu_Item *mi)
{
   const char *path;
   Evas_Object *fm;

   if (!m->zone) return;
   m = _e_mod_menu_top_get(m);

   fm = e_object_data_get(E_OBJECT(m));
   path = e_object_data_get(E_OBJECT(mi));
   if (fm && ((fileman_config->view.open_dirs_in_place && evas_object_data_get(fm, "page_is_window")) ||
       (fileman_config->view.desktop_navigation && evas_object_data_get(fm, "page_is_zone"))))
     e_fm2_path_set(fm, data, path ?: "/");
   else if (m->zone)
     e_fwin_new(data, path ?: "/");
}

static void
_e_mod_menu_cleanup_cb(void *obj)
{
   eina_stringshare_del(e_object_data_get(E_OBJECT(obj)));
}

typedef struct
{
   E_Menu *subm;
   int count;
} Populate_Data;

static Eina_Bool
_e_mod_menu_populate_filter(void *data, Eio_File *handler, const Eina_File_Direct_Info *info)
{
   Populate_Data *pd = data;
   struct stat st;

   if (!handler) return EINA_FALSE;

   if (eio_file_check(handler)) return EINA_FALSE;
   // don't show links to prevent infinte submenus (recursive links)
   if (lstat(info->path, &st)) return EINA_FALSE;
   // don't show dotfiles
   if (info->path[info->name_start] == '.') return EINA_FALSE;
   // limit to only showing up to 100 items
   pd->count++;
   if (pd->count > 100) return EINA_FALSE;
   // if it's a dir or a .desktop file... then include
   return ((info->type == EINA_FILE_DIR) ||
           eina_str_has_extension(info->path + info->name_start, "desktop"));
}

static void
_e_mod_menu_populate_item(void *data, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   Populate_Data *pd = data;
   E_Menu *m = pd->subm;
   E_Menu_Item *mi;
   const char *dev, *path;
   Efreet_Desktop *ed = NULL;

   if (handler && eio_file_check(handler)) return;

   mi = m->parent_item;
   dev = e_object_data_get(E_OBJECT(m));
   path = mi ? e_object_data_get(E_OBJECT(mi)) : "/";
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, info->path + info->name_start);
   if (fileman_config->view.menu_shows_files)
     {
        if (info->type != EINA_FILE_DIR)
          {
             const char *mime = NULL;
             char group[1024];

             if (eina_str_has_extension(mi->label, "desktop"))
               {
                  ed = efreet_desktop_get(info->path);
                  if (ed)
                    {
                       e_util_menu_item_theme_icon_set(mi, ed->icon);
                       efreet_desktop_free(ed);
                       return;
                    }
               }
             mime = efreet_mime_type_get(mi->label);
             if (!mime) return;
             if (!strncmp(mime, "image/", 6))
               {
                  e_menu_item_icon_file_set(mi, info->path);
                  return;
               }
             snprintf(group, sizeof(group), "fileman/mime/%s", mime);
             if (e_util_menu_item_theme_icon_set(mi, group))
               return;
             e_util_menu_item_theme_icon_set(mi, "fileman/mime/unknown");
             return;
          }
     }

   if (eina_str_has_extension(info->path + info->name_start, "desktop"))
     ed = efreet_desktop_get(info->path);
   if (ed)
     {
        const char *type;
        Efreet_Uri *uri;

        if (ed->type == EFREET_DESKTOP_TYPE_APPLICATION)
          {
             e_object_del(E_OBJECT(mi));
             return;
          }
        e_util_menu_item_theme_icon_set(mi, ed->icon);
        if (ed->name)
          e_menu_item_label_set(mi, ed->name);
        uri = efreet_uri_decode(ed->url);
        if ((!uri) || (!uri->path)) goto end;
        if (ed->type == EFREET_DESKTOP_TYPE_LINK)
          {
             type = efreet_desktop_x_field_get(ed, "X-Enlightenment-Type");
             if (e_util_strcmp(type, "Removable"))
               {
                  const char *p = uri->path;
                  char *esc = NULL;

                  dev = eina_stringshare_add("/");
                  if (p[0] == '$')
                    esc = e_util_shell_env_path_eval(p);
                  e_object_data_set(E_OBJECT(mi), eina_stringshare_add(esc ?: p));
                  free(esc);
               }
             else
               {
                  E_Volume *vol;

                  vol = e_fm2_device_volume_find(ed->url);
                  if (vol)
                    {
                       dev = eina_stringshare_printf("removable:%s", ed->url);
                       e_menu_item_callback_set(mi, _e_mod_menu_volume_cb, vol);
                    }
                  //FIXME: else
               }
          }
        else
          {
             eina_stringshare_ref(dev);
             e_object_data_set(E_OBJECT(mi), eina_stringshare_add(uri->path));
          }
        efreet_uri_free(uri);
        efreet_desktop_free(ed);
     }
   else
     {
        e_util_menu_item_theme_icon_set(mi, "folder");
        eina_stringshare_ref(dev);
        e_object_data_set(E_OBJECT(mi), eina_stringshare_printf("%s/%s", path ?: "", info->path + info->name_start));
     }
end:
   e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, dev);
   //fprintf(stderr, "PATH SET: %s\n", e_object_data_get(E_OBJECT(mi)));
   e_object_free_attach_func_set(E_OBJECT(mi), _e_mod_menu_cleanup_cb);
   e_menu_item_callback_set(mi, _e_mod_menu_populate_cb, dev);
}

static int
_e_mod_menu_populate_sort(E_Menu_Item *a, E_Menu_Item *b)
{
   return strcasecmp(a->label, b->label);
}

static void
_e_mod_menu_populate_done(void *data, Eio_File *handler EINA_UNUSED)
{
   Populate_Data *pd = data;
   E_Menu *m = pd->subm;

   free(pd);
   if (!e_object_unref(E_OBJECT(m))) return;
   if (!m->items)
     {
        E_Menu_Item *mi;
        Eina_Stringshare *dev, *path;

        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("No listable items"));
        dev = e_object_data_get(E_OBJECT(m));
        path = e_object_data_get(E_OBJECT(m->parent_item));
        e_object_data_set(E_OBJECT(mi), eina_stringshare_ref(path));
        if (dev && (dev[0] == '/'))
          e_menu_item_callback_set(mi, _e_mod_menu_populate_cb, dev);
        else
          e_menu_item_callback_set(mi, _e_mod_menu_virtual_cb, dev);
     }
   else
     m->items = eina_list_sort(m->items, 0, (Eina_Compare_Cb)_e_mod_menu_populate_sort);
   e_menu_thaw(m);
}

static void
_e_mod_menu_populate_err(void *data, Eio_File *handler, int error EINA_UNUSED)
{
   (void) data; (void) handler;
}

static void
_e_mod_menu_populate(void *d, E_Menu *m EINA_UNUSED, E_Menu_Item *mi)
{
   E_Menu *subm;
   const char *dev, *path, *rp;
   Eio_File *ls;
   Populate_Data *pd;

   subm = mi->submenu;
   if (subm && subm->items) return;
   dev = d;
   path = mi ? e_object_data_get(E_OBJECT(mi)) : NULL;
   rp = e_fm2_real_path_map(dev, path ?: "/");
   if (!rp) return; //probably a device
   if (!subm)
     {
        subm = e_menu_new();
        e_object_data_set(E_OBJECT(subm), eina_stringshare_add(dev));
        e_object_free_attach_func_set(E_OBJECT(subm), _e_mod_menu_cleanup_cb);
        e_menu_item_submenu_set(mi, subm);
        e_menu_freeze(subm);
     }
   pd = calloc(1, sizeof(Populate_Data));
   if (pd)
     {
        pd->subm = subm;
        ls = eio_file_stat_ls(rp,
                              _e_mod_menu_populate_filter,
                              _e_mod_menu_populate_item,
                              _e_mod_menu_populate_done,
                              _e_mod_menu_populate_err, pd);
        if (!ls)
          {
             free(pd);
             return;
          }
        e_object_ref(E_OBJECT(subm));
     }
   eina_stringshare_del(rp);
}


static void
_e_mod_fileman_parse_gtk_bookmarks(E_Menu   *m,
                                   Eina_Bool need_separator)
{
   char line[4096];
   char buf[PATH_MAX];
   E_Menu *subm = NULL;
   E_Menu_Item *mi;
   Efreet_Uri *uri;
   char *alias;
   FILE *fp;

   snprintf(buf, sizeof(buf), "%s/gtk-3.0/bookmarks", efreet_config_home_get());
   fp = fopen(buf, "r");
   if (!fp)
     {
        snprintf(buf, sizeof(buf), "%s/.gtk-bookmarks", e_user_homedir_get());
        fp = fopen(buf, "r");
    }
   if (fp)
     {
        while (fgets(line, sizeof(line), fp))
          {
             alias = NULL;
             line[strlen(line) - 1] = '\0';
             alias = strchr(line, ' ');
             if (alias)
               {
                  line[alias - line] = '\0';
                  alias++;
               }
             uri = efreet_uri_decode(line);
             if ((!uri) || (!uri->path) || (!ecore_file_exists(uri->path)))
               {
                  if (uri) efreet_uri_free(uri);
                  continue;
               }
             if (!subm)
               {
                  if (need_separator)
                    {
                       mi = e_menu_item_new(m);
                       e_menu_item_separator_set(mi, 1);
                       need_separator = 0;
                    }
                  mi = e_menu_item_new(m);
                  e_menu_item_label_set(mi, _("GTK Bookmarks"));
                  e_util_menu_item_theme_icon_set(mi, "bookmarks");
                  subm = e_menu_new();
                  e_menu_item_submenu_set(mi, subm);
               }

             mi = e_menu_item_new(subm);
             e_object_data_set(E_OBJECT(mi), uri->path);
             e_menu_item_label_set(mi, alias ? alias :
                                   ecore_file_file_get(uri->path));
             e_util_menu_item_theme_icon_set(mi, "folder");
             e_menu_item_callback_set(mi, _e_mod_menu_gtk_cb,
                                      (void *)eina_stringshare_add(uri->path));
             e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, eina_stringshare_add("/"));
             efreet_uri_free(uri);
          }
        fclose(fp);
     }
}

static void
_e_mod_menu_free(void *data)
{
   Eina_List *l;
   E_Menu_Item *mi;
   E_Menu *m = data;

   EINA_LIST_FOREACH(m->items, l, mi)
     if (mi->submenu)
       {
          //INF("SUBMENU %p REF: %d", mi->submenu, e_object_ref_get(E_OBJECT(mi->submenu)) - 1);
          _e_mod_menu_free(mi->submenu);
          e_object_unref(E_OBJECT(mi->submenu));
       }
}

/* menu item add hook */
static void
_e_mod_menu_generate(void *data, E_Menu *m)
{
   E_Volume *vol;
   E_Menu_Item *mi;
   const char *path = data;
   const char *s;
   const Eina_List *l;
   Eina_Bool need_separator;
   Eina_Bool volumes_visible = 0;

   if (eina_list_count(m->items) > 4) return; /* parent, clone,, copy, and separator */
   e_object_free_attach_func_set(E_OBJECT(m), _e_mod_menu_free);

   if (path)
     {
        /* FIXME: need to loop here for idiots who link desktops to other desktops */
        Efreet_Desktop *ed = NULL;

        mi = e_menu_item_new_relative(m, NULL);
        if (eina_str_has_extension(path, "desktop"))
          ed = efreet_desktop_get(path);
        if (ed)
          {
             const char *type;

             e_util_menu_item_theme_icon_set(mi, ed->icon);
             if (ed->type == EFREET_DESKTOP_TYPE_LINK)
               {
                  const char *uri;

                  type = efreet_desktop_x_field_get(ed, "X-Enlightenment-Type");
                  uri = ed->url;
                  if (!strncmp(ed->url, "file://", 7))
                    uri += 6; // need first slash
                  if (e_util_strcmp(type, "Removable"))
                    e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, eina_stringshare_add(uri));
                  else
                    {
                       vol = e_fm2_device_volume_find(uri);
                       if (vol)
                         {
                            e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, eina_stringshare_printf("removable:%s", uri));
                            e_menu_item_callback_set(mi, _e_mod_menu_volume_cb, vol);
                         }
                       //FIXME: else
                    }
               }
             else
               e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, eina_stringshare_ref(path));
             efreet_desktop_free(ed);
          }
        else
          {
             e_util_menu_item_theme_icon_set(mi, "folder");
             e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, eina_stringshare_ref(path));
          }
        e_menu_item_label_set(mi, _("Current Directory"));
     }
   eina_stringshare_del(path);

   /* Home */
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Home"));
   e_util_menu_item_theme_icon_set(mi, "user-home");
   s = eina_stringshare_add("~/");
   e_menu_item_callback_set(mi, _e_mod_menu_virtual_cb, s);
   e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, s);

   /* Desktop */
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Desktop"));
   e_util_menu_item_theme_icon_set(mi, "user-desktop");
   s = eina_stringshare_add("desktop");
   e_menu_item_callback_set(mi, _e_mod_menu_virtual_cb, s);
   e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, s);

   /* Favorites */
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Favorites"));
   e_util_menu_item_theme_icon_set(mi, "user-bookmarks");
   s = eina_stringshare_add("favorites");
   e_menu_item_callback_set(mi, _e_mod_menu_virtual_cb, s);
   e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, s);

   /* Trash */
   //~ mi = e_menu_item_new(em);
   //~ e_menu_item_label_set(mi, D_("Trash"));
   //~ e_util_menu_item_theme_icon_set(mi, "user-trash");
   //~ e_menu_item_callback_set(mi, _places_run_fm, "trash:///");

   /* Root */
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Root"));
   e_util_menu_item_theme_icon_set(mi, "computer");
   s = eina_stringshare_add("/");
   e_menu_item_callback_set(mi, _e_mod_menu_virtual_cb, s);
   e_menu_item_submenu_pre_callback_set(mi, _e_mod_menu_populate, s);
   need_separator = 1;

   /* Volumes */
   EINA_LIST_FOREACH(e_fm2_device_volume_list_get(), l, vol)
     {
        if (vol->mount_point && !strcmp(vol->mount_point, "/")) continue;

        if (need_separator)
          {
             mi = e_menu_item_new(m);
             e_menu_item_separator_set(mi, 1);
             need_separator = 0;
          }

        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, vol->label);
        e_util_menu_item_theme_icon_set(mi, vol->icon);
        e_menu_item_callback_set(mi, _e_mod_menu_volume_cb, vol);
        volumes_visible = 1;
     }

   _e_mod_fileman_parse_gtk_bookmarks(m, need_separator || volumes_visible > 0);

   e_menu_pre_activate_callback_set(m, NULL, NULL);
}

static void
_e_mod_menu_navigate_cb(void *d EINA_UNUSED, E_Menu *m, E_Menu_Item *mi EINA_UNUSED)
{
   if (m->zone)
     e_fwin_new("~/", "/");
}

/* returns submenu so we can add Go to Parent */
E_Menu *
e_mod_menu_add(E_Menu *m, const char *path)
{
#ifdef ENABLE_FILES
   E_Menu_Item *mi;
   E_Menu *sub;

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Navigate..."));
   e_util_menu_item_theme_icon_set(mi, "system-file-manager");
   e_menu_item_callback_set(mi, _e_mod_menu_navigate_cb, NULL);
   sub = e_menu_new();
   e_menu_item_submenu_set(mi, sub);
   e_object_unref(E_OBJECT(sub)); //allow deletion whenever main menu deletes
   e_menu_pre_activate_callback_set(sub, _e_mod_menu_generate, (void*)eina_stringshare_add(path));
   return sub;
#else
   (void)m;
   return NULL;
#endif
}
