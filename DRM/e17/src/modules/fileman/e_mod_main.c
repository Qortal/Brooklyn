#include "e_mod_main.h"

/* actual module specifics */
static void _e_mod_action_fileman_cb(E_Object   *obj,
                                     const char *params);
static void _e_mod_action_fileman_show_cb(E_Object   *obj,
                                          const char *params);
static void _e_mod_action_fileman_reset_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED);
static void      _e_mod_menu_add(void   *data, E_Menu *m);
static void      _e_mod_fileman_config_load(void);
static void      _e_mod_fileman_config_free(void);
static Eina_Bool _e_mod_zone_reconf(void *data, int type, void *event);
static Eina_Bool _e_mod_zone_add(void *data, int type, void *event);

static E_Module *conf_module = NULL;
static E_Action *act = NULL;
static E_Action *act2 = NULL;
static E_Action *act3 = NULL;
static E_Int_Menu_Augmentation *maug = NULL;
static Ecore_Event_Handler *zone_reconf_handler = NULL;
static Ecore_Event_Handler *zone_add_handler = NULL;

static E_Config_DD *paths_edd = NULL, *conf_edd = NULL;
Config *fileman_config = NULL;

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Fileman"
};

E_API void *
e_modapi_init(E_Module *m)
{
   const Eina_List *l;
   E_Zone *zone;

   conf_module = m;

   /* Setup Entry in Config Panel */
   e_configure_registry_category_add("fileman", 100, _("Files"),
                                     NULL, "system-file-manager");
   e_configure_registry_item_add("fileman/fileman", 10, _("File Manager"),
                                 NULL, "system-file-manager",
                                 e_int_config_fileman);
   e_configure_registry_item_add("fileman/file_icons", 20, _("File Icons"),
                                 NULL, "preferences-file-icons",
                                 e_int_config_mime);
   /* Setup Config edd */
   _e_mod_fileman_config_load();

   /* add module supplied action */
   act = e_action_add("fileman");
   if (act)
     {
        act->func.go = _e_mod_action_fileman_cb;
        e_action_predef_name_set(N_("Launch"), N_("File Manager"),
                                 "fileman", NULL, "syntax: /path/to/dir or ~/path/to/dir or favorites or desktop, examples: /boot/grub, ~/downloads", 1);
     }
   act2 = e_action_add("fileman_reset");
   if (act2)
     act2->func.go = _e_mod_action_fileman_reset_cb;
   act3 = e_action_add("fileman_show");
   if (act3)
     {
        act3->func.go = _e_mod_action_fileman_show_cb;
        e_action_predef_name_set(N_("Show Dir"), N_("File Manager"),
                                 "fileman_show", NULL, "syntax: /path/to/dir or ~/path/to/dir or favorites or desktop, examples: /boot/grub, ~/downloads", 1);
     }
   maug = e_int_menus_menu_augmentation_add_sorted("main/1", _("Navigate"), _e_mod_menu_add, NULL, NULL, NULL);

   e_fwin_init();

   /* Hook into zones */
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if (e_fwin_zone_find(zone)) continue;
        if (e_config->show_desktop_icons)
          e_fwin_zone_new(zone, e_mod_fileman_path_find(zone));
     }
   zone_reconf_handler = ecore_event_handler_add(E_EVENT_ZONE_MOVE_RESIZE,
                                                 _e_mod_zone_reconf, NULL);
   zone_add_handler = ecore_event_handler_add(E_EVENT_ZONE_ADD,
                                              _e_mod_zone_add, NULL);

   /* FIXME: add system event for new zone creation, and on creation, add an fwin to the zone */

   e_fileman_dbus_init();

   e_fwin_nav_init();

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   const Eina_List *l;
   E_Zone *zone;
   E_Config_Dialog *cfd;

   e_fileman_dbus_shutdown();

   ecore_event_handler_del(zone_add_handler);
   ecore_event_handler_del(zone_reconf_handler);
   zone_add_handler = NULL;
   zone_reconf_handler = NULL;

   /* Unhook zone fm */
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     e_fwin_zone_shutdown(zone);

   e_fwin_nav_shutdown();

   /* remove module-supplied menu additions */
   if (maug)
     {
        e_int_menus_menu_augmentation_del("main/1", maug);
        maug = NULL;
     }
   e_fwin_shutdown();
   /* remove module-supplied action */
   if (act)
     {
        e_action_predef_name_del("Launch", "File Manager");
        e_action_del("fileman");
        act = NULL;
     }
   if (act2)
     {
        e_action_del("fileman_reset");
        act2 = NULL;
     }
   if (act3)
     {
        e_action_del("fileman_show");
        act3 = NULL;
     }
   while ((cfd = e_config_dialog_get("E", "fileman/mime_edit_dialog")))
      e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "fileman/file_icons")))
      e_object_del(E_OBJECT(cfd));
   while ((cfd = e_config_dialog_get("E", "fileman/fileman")))
      e_object_del(E_OBJECT(cfd));

   e_configure_registry_item_del("fileman/file_icons");
   e_configure_registry_item_del("fileman/fileman");
   e_configure_registry_category_del("fileman");

   e_config_domain_save("module.fileman", conf_edd, fileman_config);

   _e_mod_fileman_config_free();
   E_CONFIG_DD_FREE(conf_edd);
   E_CONFIG_DD_FREE(paths_edd);

   //eina_shutdown();

   conf_module = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.fileman", conf_edd, fileman_config);
   return 1;
}

/* action callback */
static void
_e_mod_action_fileman_reset_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   e_fwin_reload_all();
}

static void
fwin(const char *dev, const char *path)
{
   if (!e_fwin_show(dev, path)) e_fwin_new(dev, path);
}

static void
_e_mod_action_fileman_cb(E_Object   *obj EINA_UNUSED,
                         const char *params)
{
   E_Zone *zone = NULL;

   zone = e_zone_current_get();
   if (zone)
     {
        if (params && params[0] == '/')
          e_fwin_new("/", params);
        else if (params && params[0] == '~')
          e_fwin_new("~/", params + 1);
        else if (params && strcmp(params, "(none)")) /* avoid matching paths that no longer exist */
          {
             char *path;
             path = e_util_shell_env_path_eval(params);
             if (path)
               {
                  e_fwin_new(path, "/");
                  free(path);
               }
          }
        else
          e_fwin_new("favorites", "/");
     }
}

static void
_e_mod_action_fileman_show_cb(E_Object   *obj EINA_UNUSED,
                              const char *params)
{
   const char *dev = "/", *path = "/";
   char *p = NULL;
   E_Zone *zone = NULL;

   zone = e_zone_current_get();
   if (zone)
     {
        if (params && params[0] == '/')
          path = params;
        else if (params && params[0] == '~')
          path = params + 1;
        else if (params && strcmp(params, "(none)")) /* avoid matching paths that no longer exist */
          {
             p = e_util_shell_env_path_eval(params);
             if (p)
               {
                  dev = p;
                  path = "/";
               }
          }
        else
          {
             dev = "favorites";
             path = "/";
          }
     }
   fwin(dev, path);
   free(p);
}

void
_e_mod_menu_add(void *data EINA_UNUSED, E_Menu *m)
{
   e_mod_menu_add(m, NULL);
}

/* Abstract fileman config load/create to one function for maintainability */
static void
_e_mod_fileman_config_load(void)
{
#undef T
#undef D
#define T Fileman_Path
#define D paths_edd
   paths_edd = E_CONFIG_DD_NEW("Fileman_Path", Fileman_Path);
   E_CONFIG_VAL(D, T, dev, STR);
   E_CONFIG_VAL(D, T, path, STR);
   E_CONFIG_VAL(D, T, zone, UINT);
   E_CONFIG_VAL(D, T, desktop_mode, INT);
   conf_edd = E_CONFIG_DD_NEW("Fileman_Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, config_version, INT);
   E_CONFIG_VAL(D, T, view.mode, INT);
   E_CONFIG_VAL(D, T, view.open_dirs_in_place, UCHAR);
   E_CONFIG_VAL(D, T, view.selector, UCHAR);
   E_CONFIG_VAL(D, T, view.single_click, UCHAR);
   E_CONFIG_VAL(D, T, view.no_subdir_jump, UCHAR);
   E_CONFIG_VAL(D, T, view.no_subdir_drop, UCHAR);
   E_CONFIG_VAL(D, T, view.always_order, UCHAR);
   E_CONFIG_VAL(D, T, view.link_drop, UCHAR);
   E_CONFIG_VAL(D, T, view.fit_custom_pos, UCHAR);
   E_CONFIG_VAL(D, T, view.show_full_path, UCHAR);
   E_CONFIG_VAL(D, T, view.show_toolbar, UCHAR);
   E_CONFIG_VAL(D, T, view.show_sidebar, UCHAR);
   E_CONFIG_VAL(D, T, view.desktop_navigation, UCHAR);
   E_CONFIG_VAL(D, T, icon.icon.w, INT);
   E_CONFIG_VAL(D, T, icon.icon.h, INT);
   E_CONFIG_VAL(D, T, icon.list.w, INT);
   E_CONFIG_VAL(D, T, icon.list.h, INT);
   E_CONFIG_VAL(D, T, icon.fixed.w, UCHAR);
   E_CONFIG_VAL(D, T, icon.fixed.h, UCHAR);
   E_CONFIG_VAL(D, T, icon.extension.show, UCHAR);
   E_CONFIG_VAL(D, T, icon.max_thumb_size, UINT);
   E_CONFIG_VAL(D, T, list.sort.no_case, UCHAR);
   E_CONFIG_VAL(D, T, list.sort.extension, UCHAR);
   E_CONFIG_VAL(D, T, list.sort.mtime, UCHAR);
   E_CONFIG_VAL(D, T, list.sort.size, UCHAR);
   E_CONFIG_VAL(D, T, list.sort.dirs.first, UCHAR);
   E_CONFIG_VAL(D, T, list.sort.dirs.last, UCHAR);
   E_CONFIG_VAL(D, T, selection.single, UCHAR);
   E_CONFIG_VAL(D, T, selection.windows_modifiers, UCHAR);
   E_CONFIG_VAL(D, T, theme.background, STR);
   E_CONFIG_VAL(D, T, theme.frame, STR);
   E_CONFIG_VAL(D, T, theme.icons, STR);
   E_CONFIG_VAL(D, T, theme.fixed, UCHAR);
   E_CONFIG_VAL(D, T, tooltip.delay, DOUBLE);
   E_CONFIG_VAL(D, T, tooltip.size, DOUBLE);
   E_CONFIG_VAL(D, T, tooltip.enable, UCHAR);
   E_CONFIG_VAL(D, T, tooltip.clamp_size, UCHAR);
   E_CONFIG_VAL(D, T, view.spring_delay, INT);
   E_CONFIG_VAL(D, T, view.toolbar_orient, UINT);
   E_CONFIG_LIST(D, T, paths, paths_edd);

   fileman_config = e_config_domain_load("module.fileman", conf_edd);
   if (fileman_config)
     {
        if (!e_util_module_config_check(_("Fileman"), fileman_config->config_version, MOD_CONFIG_FILE_VERSION))
          _e_mod_fileman_config_free();
     }

   if (!fileman_config)
     {
        fileman_config = E_NEW(Config, 1);
        fileman_config->view.mode = E_FM2_VIEW_MODE_GRID_ICONS;
        e_config->show_desktop_icons = 1;
        fileman_config->icon.icon.w = 48;
        fileman_config->icon.icon.h = 48;
        fileman_config->icon.extension.show = 1;
        fileman_config->list.sort.no_case = 1;
        fileman_config->list.sort.dirs.first = 1;
        fileman_config->view.show_toolbar = 1;
        fileman_config->view.open_dirs_in_place = 1;
        fileman_config->tooltip.delay = 1.0;
        fileman_config->tooltip.size = 30.0;
        fileman_config->view.show_sidebar = 1;
        fileman_config->tooltip.enable = 1;
        fileman_config->view.spring_delay = 1;
        fileman_config->icon.max_thumb_size = 5;
        fileman_config->view.toolbar_orient = E_GADCON_ORIENT_TOP;
     }
    fileman_config->config_version = MOD_CONFIG_FILE_VERSION;
    fileman_config->icon.icon.h = fileman_config->icon.icon.w;

    /* UCHAR's give nasty compile warnings about comparisons so not gonna limit those */
    E_CONFIG_LIMIT(fileman_config->view.mode, E_FM2_VIEW_MODE_ICONS, E_FM2_VIEW_MODE_LIST);
    E_CONFIG_LIMIT(fileman_config->icon.icon.w, 16, 256);
    E_CONFIG_LIMIT(fileman_config->icon.icon.h, 16, 256);
    E_CONFIG_LIMIT(fileman_config->icon.list.w, 16, 256);
    E_CONFIG_LIMIT(fileman_config->icon.list.h, 16, 256);

    E_CONFIG_LIMIT(fileman_config->tooltip.delay, 0.0, 5.0);
    E_CONFIG_LIMIT(fileman_config->tooltip.size, 10.0, 75.0);
    E_CONFIG_LIMIT(fileman_config->view.spring_delay, 1, 10);
    E_CONFIG_LIMIT(fileman_config->icon.max_thumb_size, 0, 1024);

    fileman_config->view.menu_shows_files = 0;

    e_config_save_queue();
}

static void
_e_mod_fileman_path_free(Fileman_Path *path)
{
   if (!path) return;
   eina_stringshare_del(path->dev);
   eina_stringshare_del(path->path);
   free(path);
}

static void
_e_mod_fileman_config_free(void)
{
   eina_stringshare_del(fileman_config->theme.background);
   eina_stringshare_del(fileman_config->theme.frame);
   eina_stringshare_del(fileman_config->theme.icons);
   E_FREE_LIST(fileman_config->paths, _e_mod_fileman_path_free);
   E_FREE(fileman_config);
}

static Eina_Bool
_e_mod_zone_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Zone_Add *ev = event;
   E_Zone *zone = ev->zone;

   if (e_fwin_zone_find(zone)) return ECORE_CALLBACK_PASS_ON;
   if (e_config->show_desktop_icons)
     e_fwin_zone_new(zone, e_mod_fileman_path_find(zone));
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_mod_zone_reconf(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Zone_Move_Resize *ev = event;
   E_Zone *zone = ev->zone;

   if (e_fwin_zone_find(zone))
     {
        e_fwin_zone_shutdown(zone);
        e_fwin_zone_new(zone, e_mod_fileman_path_find(zone));
     }
   else if (e_config->show_desktop_icons)
     e_fwin_zone_new(zone, e_mod_fileman_path_find(zone));
   return ECORE_CALLBACK_PASS_ON;
}

Fileman_Path *
e_mod_fileman_path_find(E_Zone *zone)
{
   Eina_List *l;
   Fileman_Path *path;

   EINA_LIST_FOREACH(fileman_config->paths, l, path)
     if (path->zone == zone->num) break;
   if (l && fileman_config->view.desktop_navigation) return path;
   if (l)
     {
        eina_stringshare_replace(&path->path, NULL);
        eina_stringshare_replace(&path->dev, "desktop");
     }
   else
     {
        path = E_NEW(Fileman_Path, 1);
        path->zone = zone->num;
        path->dev = eina_stringshare_add("desktop");
        fileman_config->paths = eina_list_append(fileman_config->paths, path);
        path->desktop_mode = E_FM2_VIEW_MODE_CUSTOM_ICONS;
     }
   if (zone->num == 0)
     path->path = eina_stringshare_add("/");
   else
     path->path = eina_stringshare_printf("%d", zone->num);
   return path;
}
