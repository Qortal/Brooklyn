#include "e.h"

/* local subsystem functions */
static void      _e_order_free(E_Order *eo);
static void      _e_order_cb_monitor(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path);
static void      _e_order_read(E_Order *eo);
static void      _e_order_save(E_Order *eo);
static void      _e_order_remove_if_exists(E_Order *eo, Efreet_Desktop *desktop);
static Eina_Bool _e_order_cb_efreet_cache_update(void *data, int ev_type, void *ev);

static Eina_List *orders = NULL;
static Eina_List *handlers = NULL;

/* externally accessible functions */
EINTERN int
e_order_init(void)
{
   char *menu_file = NULL;

   E_LIST_HANDLER_APPEND(handlers, EFREET_EVENT_DESKTOP_CACHE_UPDATE,
                         _e_order_cb_efreet_cache_update, NULL);
   E_LIST_HANDLER_APPEND(handlers, EFREET_EVENT_DESKTOP_CACHE_BUILD,
                         _e_order_cb_efreet_cache_update, NULL);
   if (e_config->default_system_menu)
     menu_file = strdup(e_config->default_system_menu);
   if (!menu_file)
     menu_file = strdup("/etc/xdg/menus/applications.menu");
   if (menu_file)
     {
        if (!ecore_file_exists(menu_file))
          {
             char buf[PATH_MAX];

             E_FREE(menu_file);
             snprintf(buf, sizeof(buf), "/etc/xdg/menus/e-applications.menu");
             if (ecore_file_exists(buf)) menu_file = strdup(buf);
             else
               {
                  snprintf(buf, sizeof(buf),
                           "%s/etc/xdg/menus/e-applications.menu",
                           e_prefix_get());
                  if (ecore_file_exists(buf)) menu_file = strdup(buf);
               }
          }
     }
   efreet_menu_file_set(menu_file);
   free(menu_file);
   return 1;
}

EINTERN int
e_order_shutdown(void)
{
   orders = eina_list_free(orders);

   E_FREE_LIST(handlers, ecore_event_handler_del);
   return 1;
}

E_API E_Order *
e_order_new(const char *path)
{
   E_Order *eo;

   if (!path)
     {
        ERR("null order");
        return NULL;
     }

   eo = E_OBJECT_ALLOC(E_Order, E_ORDER_TYPE, _e_order_free);
   if (!eo) return NULL;

   if (path[0] != '/')
     {
        ERR("not full");
     }
   if (path) eo->path = eina_stringshare_add(path);
   _e_order_read(eo);
   eo->monitor = ecore_file_monitor_add(path, _e_order_cb_monitor, eo);

   orders = eina_list_append(orders, eo);

   return eo;
}

E_API E_Order *
e_order_clone(const E_Order *eo)
{
   E_Order *eoc;
   Eina_List *l;
   Efreet_Desktop *desktop;

   E_OBJECT_CHECK_RETURN(eo, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(eo, E_ORDER_TYPE, NULL);

   eoc = E_OBJECT_ALLOC(E_Order, E_ORDER_TYPE, _e_order_free);
   if (!eoc) return NULL;

   eoc->path = eina_stringshare_ref(eo->path);
   EINA_LIST_FOREACH(eo->desktops, l, desktop)
     {
        efreet_desktop_ref(desktop);
        eoc->desktops = eina_list_append(eoc->desktops, desktop);
     }
   eoc->monitor = ecore_file_monitor_add(eoc->path, _e_order_cb_monitor, eoc);

   orders = eina_list_append(orders, eoc);
   return eoc;
}

E_API void
e_order_update_callback_set(E_Order *eo, void (*cb)(void *data, E_Order *eo), void *data)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   eo->cb.update = cb;
   eo->cb.data = data;
}

E_API void
e_order_remove(E_Order *eo, Efreet_Desktop *desktop)
{
   Eina_List *tmp;

   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   tmp = eina_list_data_find_list(eo->desktops, desktop);
   if (!tmp) return;
   efreet_desktop_free(eina_list_data_get(tmp));
   eo->desktops = eina_list_remove_list(eo->desktops, tmp);
   _e_order_save(eo);
}

E_API void
e_order_append(E_Order *eo, Efreet_Desktop *desktop)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   _e_order_remove_if_exists(eo, desktop);
   efreet_desktop_ref(desktop);
   eo->desktops = eina_list_append(eo->desktops, desktop);
   _e_order_save(eo);
}

E_API void
e_order_prepend_relative(E_Order *eo, Efreet_Desktop *desktop, Efreet_Desktop *before)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   _e_order_remove_if_exists(eo, desktop);
   efreet_desktop_ref(desktop);
   eo->desktops = eina_list_prepend_relative(eo->desktops, desktop, before);
   _e_order_save(eo);
}

E_API void
e_order_files_append(E_Order *eo, Eina_List *files)
{
   Eina_List *l;
   const char *file;

   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   EINA_LIST_FOREACH(files, l, file)
     {
        Efreet_Desktop *desktop;

        if (!strncmp(file, "file://", 7))
          file += 7;
        desktop = efreet_desktop_get(file);
        if (!desktop) continue;
        _e_order_remove_if_exists(eo, desktop);
        eo->desktops = eina_list_append(eo->desktops, desktop);
     }
   _e_order_save(eo);
}

E_API void
e_order_files_prepend_relative(E_Order *eo, Eina_List *files, Efreet_Desktop *before)
{
   Eina_List *l;
   const char *file;

   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   EINA_LIST_FOREACH(files, l, file)
     {
        Efreet_Desktop *desktop;

        if (!strncmp(file, "file://", 7))
          file += 7;
        desktop = efreet_desktop_get(file);
        if (!desktop) continue;
        _e_order_remove_if_exists(eo, desktop);
        eo->desktops = eina_list_prepend_relative(eo->desktops, desktop, before);
     }
   _e_order_save(eo);
}

E_API void
e_order_clear(E_Order *eo)
{
   E_OBJECT_CHECK(eo);
   E_OBJECT_TYPE_CHECK(eo, E_ORDER_TYPE);

   E_FREE_LIST(eo->desktops, efreet_desktop_free);
   _e_order_save(eo);
}

/* local subsystem functions */
static void
_e_order_free(E_Order *eo)
{
   if (eo->delay) ecore_timer_del(eo->delay);
   E_FREE_LIST(eo->desktops, efreet_desktop_free);
   if (eo->path) eina_stringshare_del(eo->path);
   if (eo->monitor) ecore_file_monitor_del(eo->monitor);
   orders = eina_list_remove(orders, eo);
   free(eo);
}

static void
_e_order_remove_if_exists(E_Order *eo, Efreet_Desktop *desktop)
{
   Efreet_Desktop *desk;
   Eina_List *node, *nnode;

   EINA_LIST_FOREACH_SAFE(eo->desktops, node, nnode, desk)
     {
        if (!strcmp(desk->orig_path, desktop->orig_path))
          {
             efreet_desktop_free(desktop);
             eo->desktops = eina_list_remove(eo->desktops, desktop);
             return;
          }
     }
}

static Eina_Bool
_e_order_cb_monitor_delay(void *data)
{
   E_Order *eo = data;

   /* It doesn't really matter what the change is, just re-read the file */
   _e_order_read(eo);
   if (eo->cb.update) eo->cb.update(eo->cb.data, eo);
   eo->delay = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_e_order_cb_monitor(void *data, Ecore_File_Monitor *em EINA_UNUSED, Ecore_File_Event event EINA_UNUSED, const char *path EINA_UNUSED)
{
   E_Order *eo = data;

   if (eo->delay) ecore_timer_del(eo->delay);
   eo->delay = ecore_timer_loop_add(0.2, _e_order_cb_monitor_delay, eo);
}

static void
_e_order_read(E_Order *eo)
{
   char *dir, *s, *s2, buf[4096], buf2[8192];
   Efreet_Desktop *desktop;
   Eina_List *files, *l;
   size_t len;
   FILE *f;

   E_FREE_LIST(eo->desktops, efreet_desktop_free);
   if (!eo->path) return;

   // eo->path = /path/to/dir/.order ...
   dir = ecore_file_dir_get(eo->path);
   if (!dir) return;

   files = ecore_file_ls(dir);

   f = fopen(eo->path, "rb");
   if (!f) goto err;

   while (fgets(buf, sizeof(buf), f))
     {
        len = strlen(buf);
        if (len == 0) continue;
        if (buf[len - 1] == '\n')
          {
             buf[len - 1] = 0;
             len--;
          }
        if (len == 0) continue;
        // if full path - use that first
        desktop = NULL;
        if (buf[0] == '/')
          desktop = efreet_desktop_get(buf);
        else
          {
             // /path/to/dir/filename.desktop (buf)
             snprintf(buf2, sizeof(buf2), "%s/%s", dir, buf);
             // remove file if its in the dir already
             EINA_LIST_FOREACH(files, l, s)
               {
                  if ((!strcmp(s, buf)) || (!strcmp(s, buf2)))
                    {
                       files = eina_list_remove_list(files, l);
                       free(s);
                       break;
                    }
               }
             // desktop file in the order dir first
             if (!desktop)
               desktop = efreet_desktop_get(buf2);
             // ignore any path elements and look up just filename
             if (!desktop)
               desktop = efreet_desktop_get(ecore_file_file_get(buf));
             // look uop by id
             if (!desktop)
               desktop = efreet_util_desktop_file_id_find(ecore_file_file_get(buf));
          }
        // if we found it - append.
        if (desktop)
          eo->desktops = eina_list_append(eo->desktops, desktop);
     }
   fclose(f);
err:
   EINA_LIST_FOREACH(files, l, s)
     {
        if (s[0] == '.') continue;
        s2 = strchr(s, '.');
        if (!s2) continue;
        if (!(!strcasecmp(s2, ".desktop"))) continue;
        snprintf(buf2, sizeof(buf2), "%s/%s", dir, s);
        desktop = efreet_desktop_get(buf2);
        if (desktop)
          eo->desktops = eina_list_append(eo->desktops, desktop);
     }
   free(dir);
   EINA_LIST_FREE(files, s) free(s);
}

static void
_e_order_save(E_Order *eo)
{
   FILE *f;
   Eina_List *l;
   Efreet_Desktop *desktop;

   if (!eo->path) return;
   f = fopen(eo->path, "wb");
   if (!f) return;

   EINA_LIST_FOREACH(eo->desktops, l, desktop)
     {
        const char *id;

        id = efreet_util_path_to_file_id(desktop->orig_path);
        if (id)
          fprintf(f, "%s\n", id);
        else
          fprintf(f, "%s\n", desktop->orig_path);
     }

   fclose(f);
}

static Eina_Bool
_e_order_cb_efreet_cache_update(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Eina_List *l;
   E_Order *eo;

   /* reread all .order files */
   EINA_LIST_FOREACH(orders, l, eo)
     {
        _e_order_read(eo);
        if (eo->cb.update) eo->cb.update(eo->cb.data, eo);
     }
   return ECORE_CALLBACK_PASS_ON;
}

