#include "e.h"

E_API E_Path * path_data = NULL;
E_API E_Path * path_images = NULL;
E_API E_Path * path_fonts = NULL;
E_API E_Path * path_icons = NULL;
E_API E_Path * path_modules = NULL;
E_API E_Path * path_backgrounds = NULL;
E_API E_Path * path_messages = NULL;

/* local subsystem functions */
static Eina_Bool    _e_util_cb_delayed_del(void *data);
static Eina_Bool    _e_util_wakeup_cb(void *data);

static Evas_Object *_e_util_icon_add(const char *path, Evas *evas, int size);

static void         _e_util_cb_delayed_cancel(void *data, void *obj);

/* local subsystem globals */
static Ecore_Timer *_e_util_dummy_timer = NULL;

/* externally accessible functions */
E_API void
e_util_wakeup(void)
{
   if (_e_util_dummy_timer) return;
   _e_util_dummy_timer = ecore_timer_loop_add(0.0, _e_util_wakeup_cb, NULL);
}

E_API void
e_util_env_set(const char *var, const char *val)
{
   if (val)
     {
#ifdef HAVE_SETENV
        setenv(var, val, 1);
#else
        char buf[8192];

        snprintf(buf, sizeof(buf), "%s=%s", var, val);
        putenv(strdup(buf));
#endif
     }
   else
     {
#ifdef HAVE_UNSETENV
        unsetenv(var);
#else
        if (getenv(var)) putenv(var);
#endif
     }
}

E_API int
e_util_glob_match(const char *str, const char *pattern)
{
   if ((!str) || (!pattern)) return 0;
   if (pattern[0] == 0)
     {
        if (str[0] == 0) return 1;
        return 0;
     }
   if (str == pattern) return 1;
   if (!strcmp(pattern, "*")) return 1;
   if (eina_fnmatch(pattern, str, 0)) return 1;
   return 0;
}

E_API int
e_util_glob_case_match(const char *str, const char *pattern)
{
   const char *p;
   char *tstr, *tglob, *tp;

   if (str == pattern) return 1;
   if (pattern[0] == 0)
     {
        if (str[0] == 0) return 1;
        return 0;
     }
   if (!strcmp(pattern, "*")) return 1;
   tstr = alloca(strlen(str) + 1);
   for (tp = tstr, p = str; *p != 0; p++, tp++)
     *tp = tolower(*p);
   *tp = 0;
   tglob = alloca(strlen(pattern) + 1);
   for (tp = tglob, p = pattern; *p != 0; p++, tp++)
     *tp = tolower(*p);
   *tp = 0;
   if (eina_fnmatch(tglob, tstr, 0)) return 1;
   return 0;
}

E_API int
e_util_strcmp(const char *s1, const char *s2)
{
   if ((s1) && (s2))
     {
        if (s1 == s2) return 0;
        return strcmp(s1, s2);
     }
   return 0x7fffffff;
}

E_API int
e_util_strcasecmp(const char *s1, const char *s2)
{
   if ((!s1) && (!s2)) return 0;
   if (!s1) return -1;
   if (!s2) return 1;
   return strcasecmp(s1, s2);
}

E_API int
e_util_both_str_empty(const char *s1, const char *s2)
{
   int empty = 0;

   if ((!s1) && (!s2)) return 1;
   if ((!s1) || ((s1) && (s1[0] == 0))) empty++;
   if ((!s2) || ((s2) && (s2[0] == 0))) empty++;
   if (empty == 2) return 1;
   return 0;
}

E_API int
e_util_immortal_check(void)
{
   Eina_List *wins;

   wins = e_clients_immortal_list();
   if (wins)
     {
        e_util_dialog_show(_("Cannot exit - immortal windows."),
                           _("Some windows are left still around with the Lifespan lock enabled. This means<ps/>"
                             "that Enlightenment will not allow itself to exit until these windows have<ps/>"
                             "been closed or have the lifespan lock removed.<ps/>"));
        /* FIXME: should really display a list of these lifespan locked */
        /* windows in a dialog and let the user disable their locks in */
        /* this dialog */
        eina_list_free(wins);
        return 1;
     }
   return 0;
}

E_API int
e_util_edje_icon_check(const char *name)
{
   const char *file;
   char buf[4096];

   if ((!name) || (!name[0])) return 0;
   snprintf(buf, sizeof(buf), "e/icons/%s", name);
   file = e_theme_edje_file_get("base/theme/icons", buf);
   if (file[0]) return 1;
   return 0;
}

/* WARNING This function is deprecated,. must be made static.
 * You should use e_util_icon_theme_set instead
 */
E_API int
e_util_edje_icon_set(Evas_Object *obj, const char *name)
{
   const char *file;
   char buf[4096];

   if ((!name) || (!name[0])) return 0;
   snprintf(buf, sizeof(buf), "e/icons/%s", name);
   file = e_theme_edje_file_get("base/theme/icons", buf);
   if (file[0])
     {
        edje_object_file_set(obj, file, buf);
        return 1;
     }
   return 0;
}

static int
_e_util_icon_theme_set(Evas_Object *obj, const char *icon, Eina_Bool fallback)
{
   const char *file;
   char buf[4096];

   if ((!icon) || (!icon[0])) return 0;
   snprintf(buf, sizeof(buf), "e/icons/%s", icon);

   if (fallback)
     file = e_theme_edje_icon_fallback_file_get(buf);
   else
     file = e_theme_edje_file_get("base/theme/icons", buf);

   if (file[0])
     {
        e_icon_file_edje_set(obj, file, buf);
        return 1;
     }

   return 0;
}

static int
_e_util_icon_fdo_set(Evas_Object *obj, const char *icon)
{
   const char *path = NULL;
   unsigned int size;

   if ((!icon) || (!icon[0])) return 0;
   size = e_icon_scale_size_get(obj);
   if (size < 16) size = 16;
   size = e_util_icon_size_normalize(size * e_scale);

   path = efreet_icon_path_find(e_config->icon_theme, icon, size);
   if (!path) return 0;

   e_icon_file_set(obj, path);
   return 1;
}

/* use e_icon_size_scale_set(obj, size) to set the preferred icon size */
E_API int
e_util_icon_theme_set(Evas_Object *obj, const char *icon)
{
   if (icon && (icon[0] == '/'))
     {
        e_icon_file_set(obj, icon);
        return 1;
     }
   if (e_config->icon_theme_overrides)
     {
        if (_e_util_icon_fdo_set(obj, icon))
          return 1;
        if (_e_util_icon_theme_set(obj, icon, EINA_FALSE))
          return 1;
        return _e_util_icon_theme_set(obj, icon, EINA_TRUE);
     }
   else
     {
        if (_e_util_icon_theme_set(obj, icon, EINA_FALSE))
          return 1;
        if (_e_util_icon_fdo_set(obj, icon))
          return 1;
        return _e_util_icon_theme_set(obj, icon, EINA_TRUE);
     }
}

int
_e_util_menu_item_edje_icon_set(E_Menu_Item *mi, const char *name, Eina_Bool fallback)
{
   const char *file;
   char buf[4096];

   if ((!name) || (!name[0])) return 0;

   if ((!fallback) && (name[0] == '/') && ecore_file_exists(name))
     {
        e_menu_item_icon_edje_set(mi, name, "icon");
        return 1;
     }
   snprintf(buf, sizeof(buf), "e/icons/%s", name);

   if (fallback)
     file = e_theme_edje_icon_fallback_file_get(buf);
   else
     file = e_theme_edje_file_get("base/theme/icons", buf);

   if (file[0])
     {
        e_menu_item_icon_edje_set(mi, file, buf);
        return 1;
     }
   return 0;
}

E_API unsigned int
e_util_icon_size_normalize(unsigned int desired)
{
   const unsigned int *itr, known_sizes[] =
   {
      16, 22, 24, 32, 36, 48, 64, 72, 96, 128, 192, 256, 0
   };

   for (itr = known_sizes; *itr > 0; itr++)
     if (*itr >= desired)
       return *itr;

   return 256; /* largest know size? */
}

static int
_e_util_menu_item_fdo_icon_set(E_Menu_Item *mi, const char *icon)
{
   const char *path = NULL;
   unsigned int size;

   if ((!icon) || (!icon[0])) return 0;
   size = e_util_icon_size_normalize(96 * e_scale);
   path = efreet_icon_path_find(e_config->icon_theme, icon, size);
   if (!path) return 0;
   e_menu_item_icon_file_set(mi, path);
   return 1;
}

E_API int
e_util_menu_item_theme_icon_set(E_Menu_Item *mi, const char *icon)
{
   if (e_config->icon_theme_overrides)
     {
        if (_e_util_menu_item_fdo_icon_set(mi, icon))
          return 1;
        if (_e_util_menu_item_edje_icon_set(mi, icon, EINA_FALSE))
          return 1;
        return _e_util_menu_item_edje_icon_set(mi, icon, EINA_TRUE);
     }
   else
     {
        if (_e_util_menu_item_edje_icon_set(mi, icon, EINA_FALSE))
          return 1;
        if (_e_util_menu_item_fdo_icon_set(mi, icon))
          return 1;
        return _e_util_menu_item_edje_icon_set(mi, icon, EINA_TRUE);
     }
}

E_API const char *
e_util_mime_icon_get(const char *mime, unsigned int size)
{
   char buf[1024];
   const char *file = NULL;

   if (e_config->icon_theme_overrides)
     file = efreet_mime_type_icon_get(mime, e_config->icon_theme, e_util_icon_size_normalize(size));
   if (file) return file;

   if (snprintf(buf, sizeof(buf), "e/icons/fileman/mime/%s", mime) >= (int)sizeof(buf))
     return NULL;

   file = e_theme_edje_file_get("base/theme/icons", buf);
   if (file && file[0]) return file;
   return efreet_mime_type_icon_get(mime, e_config->icon_theme, e_util_icon_size_normalize(size));
}

E_API E_Client *
e_util_desk_client_above(E_Client *ec)
{
   E_Client *ec2;

   E_OBJECT_CHECK_RETURN(ec, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(ec, E_CLIENT_TYPE, NULL);

   for (ec2 = e_client_above_get(ec); ec2; ec2 = e_client_above_get(ec2))
     {
        if ((ec2->desk == ec->desk) || (ec2->sticky)) return ec2;
     }
   return NULL;
}

E_API E_Client *
e_util_desk_client_below(E_Client *ec)
{
   E_Client *ec2;

   E_OBJECT_CHECK_RETURN(ec, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(ec, E_CLIENT_TYPE, NULL);

   for (ec2 = e_client_below_get(ec); ec2; ec2 = e_client_below_get(ec2))
     {
        if ((ec2->desk == ec->desk) || (ec2->sticky)) return ec2;
     }
   return NULL;
}

E_API int
e_util_edje_collection_exists(const char *file, const char *coll)
{
   Eina_List *clist;
   Eina_Stringshare *str;
   Eina_Bool ret = EINA_FALSE;

   clist = edje_file_collection_list(file);
   EINA_LIST_FREE(clist, str)
     {
        if ((!ret) && (!strcmp(coll, str)))
          ret = EINA_TRUE;
        eina_stringshare_del(str);
     }
   return ret;
}

E_API E_Dialog *
e_util_dialog_internal(const char *title, const char *txt)
{
   E_Dialog *dia;

   dia = e_dialog_new(NULL, "E", "_error_dialog");
   if (!dia) return NULL;
   e_dialog_title_set(dia, title);
   e_dialog_text_set(dia, txt);
   e_dialog_icon_set(dia, "dialog-error", 64);
   e_dialog_button_add(dia, _("OK"), NULL, NULL, NULL);
   e_dialog_button_focus_num(dia, 0);
   elm_win_center(dia->win, 1, 1);
   e_dialog_show(dia);
   return dia;
}

E_API const char *
e_util_filename_escape(const char *filename)
{
   const char *p;
   char *q;
   static char buf[PATH_MAX];

   if (!filename) return NULL;
   p = filename;
   q = buf;
   while (*p)
     {
        if ((q - buf) > 4090) return NULL;
        if (
          (*p == ' ') || (*p == '\t') || (*p == '\n') ||
          (*p == '\\') || (*p == '\'') || (*p == '\"') ||
          (*p == ';') || (*p == '!') || (*p == '#') ||
          (*p == '$') || (*p == '%') || (*p == '&') ||
          (*p == '*') || (*p == '(') || (*p == ')') ||
          (*p == '[') || (*p == ']') || (*p == '{') ||
          (*p == '}') || (*p == '|') || (*p == '<') ||
          (*p == '>') || (*p == '?')
          )
          {
             *q = '\\';
             q++;
          }
        *q = *p;
        q++;
        p++;
     }
   *q = 0;
   return buf;
}


E_API char *
e_util_shell_env_path_eval(const char *path)
{
   /* evaluate things like:
    * $HOME/bling -> /home/user/bling
    * $HOME/bin/$HOSTNAME/blah -> /home/user/bin/localhost/blah
    * etc. etc.
    */
   const char *p, *v2, *v1 = NULL;
   char buf[PATH_MAX], *pd, *s, *vp;
   char *v = NULL;
   int esc = 0, invar = 0;

   for (p = path, pd = buf; (pd < (buf + sizeof(buf) - 1)); p++)
     {
        if (invar)
          {
             if (!((isalnum(*p)) || (*p == '_')))
               {
                  v2 = p;
                  invar = 0;
                  if ((v2 - v1) > 1)
                    {
                       s = alloca(v2 - v1);
                       strncpy(s, v1 + 1, v2 - v1 - 1);
                       s[v2 - v1 - 1] = 0;
                       if (strncmp(s, "XDG", 3))
                         v = getenv(s);
                       else
                         {
                            if (!strcmp(s, "XDG_CONFIG_HOME"))
                              v = (char *)efreet_config_home_get();
                            else if (!strcmp(s, "XDG_CACHE_HOME"))
                              v = (char *)efreet_cache_home_get();
                            else if (!strcmp(s, "XDG_DATA_HOME"))
                              v = (char *)efreet_data_home_get();
                            else if (!strcmp(s, "XDG_DESKTOP_DIR"))
                              v = (char *)efreet_desktop_dir_get();
                            else if (!strcmp(s, "XDG_DOWNLOAD_DIR"))
                              v = (char *)efreet_download_dir_get();
                            else if (!strcmp(s, "XDG_TEMPLATES_DIR"))
                              v = (char *)efreet_templates_dir_get();
                            else if (!strcmp(s, "XDG_PUBLICSHARE_DIR"))
                              v = (char *)efreet_public_share_dir_get();
                            else if (!strcmp(s, "XDG_DOCUMENTS_DIR"))
                              v = (char *)efreet_documents_dir_get();
                            else if (!strcmp(s, "XDG_MUSIC_DIR"))
                              v = (char *)efreet_music_dir_get();
                            else if (!strcmp(s, "XDG_PICTURES_DIR"))
                              v = (char *)efreet_pictures_dir_get();
                            else if (!strcmp(s, "XDG_VIDEOS_DIR"))
                              v = (char *)efreet_videos_dir_get();
                            else if (!strcmp(s, "XDG_RUNTIME_DIR"))
                              v = (char *)efreet_runtime_dir_get();
                         }

                       if (v)
                         {
                            vp = v;
                            while ((*vp) && (pd < (buf + sizeof(buf) - 1)))
                              {
                                 *pd = *vp;
                                 vp++;
                                 pd++;
                              }
                         }
                    }
                  if (pd < (buf + sizeof(buf) - 1))
                    {
                       *pd = *p;
                       pd++;
                    }
               }
          }
        else
          {
             if (esc)
               {
                  *pd = *p;
                  pd++;
               }
             else
               {
                  if (*p == '\\') esc = 1;
                  else if (*p == '$')
                    {
                       invar = 1;
                       v1 = p;
                    }
                  else
                    {
                       *pd = *p;
                       pd++;
                    }
               }
          }
        if (*p == 0) break;
     }
   *pd = 0;
   return strdup(buf);
}

E_API char *
e_util_size_string_get(off_t size)
{
   double dsize;
   char buf[256];

   dsize = (double)size;
   if (dsize < 1024.0) snprintf(buf, sizeof(buf), _("%'.0f bytes"), dsize);
   else
     {
        dsize /= 1024.0;
        if (dsize < 1024) snprintf(buf, sizeof(buf), _("%'.0f KiB"), dsize);
        else
          {
             dsize /= 1024.0;
             if (dsize < 1024) snprintf(buf, sizeof(buf), _("%'.1f MiB"), dsize);
             else
               {
                  dsize /= 1024.0;
                  if (dsize < 1024) snprintf(buf, sizeof(buf), _("%'.1f GiB"), dsize);
                  else
                    {
                       dsize /= 1024.0;
                       snprintf(buf, sizeof(buf), _("%'.1f TiB"), dsize);
                    }
               }
          }
     }
   return strdup(buf);
}

E_API char *
e_util_file_time_get(time_t ftime)
{
   time_t diff, ltime;
   char buf[256];
   char *s = NULL;
   long test;
   ltime = time(NULL);
   diff = ltime - ftime;
   buf[0] = 0;
   if (e_config->explicit_date_time)
     {
        struct tm tm;

        if (localtime_r(&ftime, &tm))
          {
             s = eina_strftime("%F %T", &tm);
             if (s) return s;
          }
     }
   else
     {
        if (ftime > ltime)
          snprintf(buf, sizeof(buf), _("In the future"));
        else
          {
             if (diff <= 60)
               snprintf(buf, sizeof(buf), _("In the last minute"));
             else if (diff >= 31526000)
               {
                  test = diff / 31526000;
                  snprintf(buf, sizeof(buf), P_("Last year", "%li Years ago", test), test);
               }
             else if (diff >= 2592000)
               {
                  test = diff / 2592000;
                  snprintf(buf, sizeof(buf), P_("Last month", "%li Months ago", test), test);
               }
             else if (diff >= 604800)
               {
                  test = diff / 604800;
                  snprintf(buf, sizeof(buf), P_("Last week", "%li Weeks ago", test), test);
               }
             else if (diff >= 86400)
               {
                  test = diff / 86400;
                  snprintf(buf, sizeof(buf), P_("Yesterday", "%li Days ago", test), test);
               }
             else if (diff >= 3600)
               {
                  test = diff / 3600;
                  snprintf(buf, sizeof(buf), P_("An hour ago", "%li Hours ago", test), test);
               }
             else if (diff > 60)
               {
                  test = diff / 60;
                  snprintf(buf, sizeof(buf), P_("A minute ago", "%li Minutes ago", test), test);
               }
          }
     }

   if (buf[0])
     s = strdup(buf);
   else
     s = strdup(_("Unknown"));
   return s;
}

E_API Evas_Object *
e_util_icon_add(const char *path, Evas *evas)
{
   return _e_util_icon_add(path, evas, 64);
}

E_API Evas_Object *
e_util_desktop_icon_add(Efreet_Desktop *desktop, unsigned int size, Evas *evas)
{
   if ((!desktop) || (!desktop->icon)) return NULL;
   return e_util_icon_theme_icon_add(desktop->icon, size, evas);
}

E_API Evas_Object *
e_util_icon_theme_icon_add(const char *icon_name, unsigned int size, Evas *evas)
{
   if (!icon_name) return NULL;
   if (icon_name[0] == '/') return _e_util_icon_add(icon_name, evas, size);
   else
     {
        Evas_Object *obj;
        const char *path;

        path = efreet_icon_path_find(e_config->icon_theme, icon_name, size);
        if (path)
          {
             obj = _e_util_icon_add(path, evas, size);
             return obj;
          }
     }
   return NULL;
}

E_API void
e_util_desktop_menu_item_icon_add(Efreet_Desktop *desktop, unsigned int size, E_Menu_Item *mi)
{
   const char *path = NULL;

   if ((!desktop) || (!desktop->icon)) return;

   if (desktop->icon[0] == '/') path = desktop->icon;
   else path = efreet_icon_path_find(e_config->icon_theme, desktop->icon, size);

   if (path)
     {
        const char *ext;

        ext = strrchr(path, '.');
        if (ext)
          {
             if (strcmp(ext, ".edj") == 0)
               e_menu_item_icon_edje_set(mi, path, "icon");
             else
               e_menu_item_icon_file_set(mi, path);
          }
        else
          e_menu_item_icon_file_set(mi, path);
     }
}

E_API int
e_util_dir_check(const char *dir)
{
   if (!ecore_file_exists(dir))
     {
        if (!ecore_file_mkpath(dir))
          {
             e_util_dialog_show(_("Error creating directory"), _("Failed to create directory: %s .<ps/>Check that you have correct permissions set."), dir);
             return 0;
          }
     }
   else
     {
        if (!ecore_file_is_dir(dir))
          {
             e_util_dialog_show(_("Error creating directory"), _("Failed to create directory: %s .<ps/>A file of that name already exists."), dir);
             return 0;
          }
     }
   return 1;
}

static Eina_Array *_delay_del_array = NULL;
static Ecore_Idle_Enterer *_delay_del_idler = NULL;

static Eina_Bool
_e_util_cb_delayed_del(void *data EINA_UNUSED)
{
   while (_delay_del_array)
     {
        Eina_Array *arr = _delay_del_array;
        Eina_Iterator *itr = eina_array_iterator_new(arr);
        void *ptr;

        _delay_del_array = NULL;
        while (eina_iterator_next(itr, &ptr))
          {
             if (ptr) e_object_del(E_OBJECT(ptr));
          }
        eina_array_free(arr);
     }
   _delay_del_idler = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_e_util_cb_delayed_cancel(void *data, void *obj EINA_UNUSED)
{
   unsigned long c = (unsigned long)data;
   if (_delay_del_array) eina_array_data_set(_delay_del_array, c, NULL);
}

E_API void
e_util_defer_object_del(E_Object *obj)
{
   if (stopping)
     e_object_del(obj);
   else
     {
        if (!_delay_del_array)
          {
             _delay_del_array = eina_array_new(8);
             if (!_delay_del_idler)
               _delay_del_idler = ecore_idle_enterer_before_add
                 (_e_util_cb_delayed_del, NULL);
          }
        if (_delay_del_array)
          {
             if (eina_array_push(_delay_del_array, obj))
               {
                  unsigned long c = eina_array_count_get(_delay_del_array);
                  e_object_delfn_add(obj, _e_util_cb_delayed_cancel, (void *)c);
               }
          }
     }
}

E_API const char *
e_util_winid_str_get(Ecore_X_Window win)
{
   const char *vals = "qWeRtYuIoP5-$&<~";
   static char id[9];
   unsigned int val;

   val = (unsigned int)win;
   id[0] = vals[(val >> 28) & 0xf];
   id[1] = vals[(val >> 24) & 0xf];
   id[2] = vals[(val >> 20) & 0xf];
   id[3] = vals[(val >> 16) & 0xf];
   id[4] = vals[(val >> 12) & 0xf];
   id[5] = vals[(val >> 8) & 0xf];
   id[6] = vals[(val >> 4) & 0xf];
   id[7] = vals[(val) & 0xf];
   id[8] = 0;
   return id;
}

static int
_win_auto_size_calc(int max, int min)
{
   const float *itr, scales[] = {0.25, 0.3, 0.5, 0.75, 0.8, 0.9, 0.95, -1};

   for (itr = scales; *itr > 0; itr++)
     {
        int value = *itr * max;
        if (value > min) /* not >=, try a bit larger */
          {
             if (min > 10)
               value = E_CLAMP(value, min, min * 1.5);
             return value;
          }
     }

   return min;
}

E_API void
e_util_win_auto_resize_fill(Evas_Object *win)
{
   E_Zone *zone = NULL;
   E_Client *ec;

   ec = e_win_client_get(win);
   if (ec)
     zone = ec->zone;
   if (!zone)
     zone = e_zone_current_get();

   if (zone)
     {
        int w, h, mw, mh;

        e_zone_useful_geometry_get(zone, NULL, NULL, &w, &h);

        evas_object_size_hint_min_get(win, &mw, &mh);
        w = _win_auto_size_calc(w, mw);
        h = _win_auto_size_calc(h, mh);
        evas_object_resize(win, w, h);
     }
}

/* local subsystem functions */

static Evas_Object *
_e_util_icon_add(const char *path, Evas *evas, int size)
{
   Evas_Object *o = NULL;
   const char *ext;

   if (!path) return NULL;
   if (!ecore_file_exists(path)) return NULL;

   o = e_icon_add(evas);
   e_icon_scale_size_set(o, size);
   e_icon_preload_set(o, 1);
   ext = strrchr(path, '.');
   if (ext)
     {
        if (!strcmp(ext, ".edj"))
          e_icon_file_edje_set(o, path, "icon");
        else
          e_icon_file_set(o, path);
     }
   else
     e_icon_file_set(o, path);
   e_icon_fill_inside_set(o, 1);

   return o;
}

static Eina_Bool
_e_util_wakeup_cb(void *data EINA_UNUSED)
{
   _e_util_dummy_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_e_util_conf_timer_old(void *data)
{
   char *module_name = data;
   char buf[4096];
   char *msg = _("Configuration data needed upgrading. Your old configuration<ps/>"
                 "has been wiped and a new set of defaults initialized. This<ps/>"
                 "will happen regularly during development, so don't report a<ps/>"
                 "bug. This means the module needs new configuration<ps/>"
                 "data by default for usable functionality that your old<ps/>"
                 "configuration lacked. This new set of defaults will fix<ps/>"
                 "that by adding it in. You can re-configure things now to your<ps/>"
                 "liking. Sorry for the hiccup in your configuration.<ps/>");

   snprintf(buf, sizeof(buf), N_("%s Configuration Updated"), module_name);
   e_util_dialog_internal(buf, msg);
   E_FREE(module_name);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_e_util_conf_timer_new(void *data)
{
   char *module_name = data;
   char buf[4096];
   char *msg =
     _("Your module configuration is NEWER "
       "than the module version. This is "
       "very<ps/>strange. This should not happen unless"
       " you downgraded<ps/>the module or "
       "copied the configuration from a place where"
       "<ps/>a newer version of the module "
       "was running. This is bad and<ps/>as a "
       "precaution your configuration has been now "
       "restored to<ps/>defaults. Sorry for the "
       "inconvenience.<ps/>");

   snprintf(buf, sizeof(buf), _("%s Configuration Updated"), module_name);
   e_util_dialog_internal(buf, msg);
   E_FREE(module_name);

   return ECORE_CALLBACK_CANCEL;
}

E_API Eina_Bool
e_util_module_config_check(const char *module_name, int loaded, int current)
{
   if (loaded > current)
     {
        ecore_timer_loop_add(1.0, _e_util_conf_timer_new, strdup(module_name));
        return EINA_FALSE;
     }
   loaded -= loaded % 1000000, current -= current % 1000000;
   if (loaded < current)
     {
        ecore_timer_loop_add(1.0, _e_util_conf_timer_old, strdup(module_name));
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/**
 * Checks whenever the current manager/comp/zone have fullscreen windows.
 */
E_API Eina_Bool
e_util_fullscreen_current_any(void)
{
   E_Zone *zone = e_zone_current_get();
   E_Desk *desk;

   if ((zone) && (zone->fullscreen > 0)) return EINA_TRUE;
   desk = e_desk_current_get(zone);
   if ((desk) && (desk->fullscreen_clients)) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * Checks whenever any manager/comp/zone have fullscreen windows.
 */
E_API Eina_Bool
e_util_fullscreen_any(void)
{
   E_Zone *zone;
   const Eina_List *lz;
   E_Desk *desk;
   int x, y;

   EINA_LIST_FOREACH(e_comp->zones, lz, zone)
     {
        if (zone->fullscreen > 0) return EINA_TRUE;

        for (x = 0; x < zone->desk_x_count; x++)
          for (y = 0; y < zone->desk_y_count; y++)
            {
               desk = e_desk_at_xy_get(zone, x, y);
               if ((desk) && (desk->fullscreen_clients))
                 return EINA_TRUE;
            }
     }
   return EINA_FALSE;
}

E_API const char *
e_util_time_str_get(long int seconds)
{
   static char buf[1024];
   long int test;

   if (seconds < 0)
     snprintf(buf, sizeof(buf), _("Never"));
   else
     {
        if (seconds <= 60)
          snprintf(buf, sizeof(buf), P_("A second", "%li Seconds", seconds), seconds);
        else if (seconds >= 31526000)
          {
             test = seconds / 31526000;
             snprintf(buf, sizeof(buf), P_("One year", "%li Years", test), test);
          }
        else if (seconds >= 2592000)
          {
             test = seconds / 2592000;
             snprintf(buf, sizeof(buf), P_("One month", "%li Months", test), test);
          }
        else if (seconds >= 604800)
          {
             test = seconds / 604800;
             snprintf(buf, sizeof(buf), P_("One week", "%li Weeks", test), test);
          }
        else if (seconds >= 86400)
          {
             test = seconds / 86400;
             snprintf(buf, sizeof(buf), P_("One day", "%li Days", test), test);
          }
        else if (seconds >= 3600)
          {
             test = seconds / 3600;
             snprintf(buf, sizeof(buf), P_("An hour", "%li Hours", test), test);
          }
        else if (seconds > 60)
          {
             test = seconds / 60;
             snprintf(buf, sizeof(buf), P_("A minute", "%li Minutes", test), test);
          }
     }
   return buf;
}

static void
_e_util_size_debug_free(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int x, y, w, h;
   const char *name;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   name = evas_object_name_get(obj);
   fprintf(stderr, "FREE %s %d OBJ[%s%s%p]: (%d,%d) - %dx%d\n", evas_object_visible_get(obj) ? "VIS" : "HID", evas_object_layer_get(obj), name ?: "", name ? "|" : "", obj, x, y, w, h);
}

static void
_e_util_size_debug_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int x, y, w, h;
   const char *name;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   name = evas_object_name_get(obj);
   fprintf(stderr, "DEL %s %d OBJ[%s%s%p]: (%d,%d) - %dx%d\n", evas_object_visible_get(obj) ? "VIS" : "HID", evas_object_layer_get(obj), name ?: "", name ? "|" : "", obj, x, y, w, h);
}

static void
_e_util_size_debug_stack(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int x, y, w, h;
   const char *name;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   name = evas_object_name_get(obj);
   fprintf(stderr, "RESTACK %s %d OBJ[%s%s%p]: (%d,%d) - %dx%d\n", evas_object_visible_get(obj) ? "VIS" : "HID", evas_object_layer_get(obj), name ?: "", name ? "|" : "", obj, x, y, w, h);
}

E_API void
e_util_size_debug(Evas_Object *obj)
{
   int x, y, w, h;
   const char *name;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   name = evas_object_name_get(obj);
   fprintf(stderr, "%s %d OBJ[%s%s%p]: (%d,%d) - %dx%d\n", evas_object_visible_get(obj) ? "VIS" : "HID", evas_object_layer_get(obj), name ?: evas_object_type_get(obj), "|", obj, x, y, w, h);
}

static void
_e_util_size_debug(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   e_util_size_debug(obj);
}

E_API void
e_util_size_debug_set(Evas_Object *obj, Eina_Bool enable)
{
   if (enable)
     {
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE,
                                       _e_util_size_debug, NULL);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                       _e_util_size_debug, NULL);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW,
                                       _e_util_size_debug, NULL);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE,
                                       _e_util_size_debug, NULL);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_RESTACK,
                                       _e_util_size_debug_stack, NULL);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _e_util_size_debug_del, NULL);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE,
                                       _e_util_size_debug_free, NULL);
     }
   else
     {
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOVE,
                                            _e_util_size_debug, NULL);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE,
                                            _e_util_size_debug, NULL);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_SHOW,
                                       _e_util_size_debug, NULL);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_HIDE,
                                       _e_util_size_debug, NULL);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _e_util_size_debug_del, NULL);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_FREE,
                                       _e_util_size_debug_free, NULL);
     }
}

E_API void
e_util_string_list_free(Eina_List *l)
{
   char *s;

   EINA_LIST_FREE(l, s)
     free(s);
}

static Efreet_Desktop *
_e_util_default_terminal_get(const char *defaults_list)
{
   Efreet_Desktop *tdesktop = NULL;
   Efreet_Ini *ini;
   const char *s;

   ini = efreet_ini_new(defaults_list);
   if ((ini) && (ini->data) &&
       (efreet_ini_section_set(ini, "Default Applications")) &&
       (ini->section))
     {
        s = efreet_ini_string_get(ini, "x-scheme-handler/terminal");
        if (s) tdesktop = efreet_util_desktop_file_id_find(s);
     }
   if (ini) efreet_ini_free(ini);
   return tdesktop;
}

E_API Efreet_Desktop *
e_util_terminal_desktop_get(void)
{
   const char *terms[] =
   {
      "terminology.desktop",
      "rxvt.desktop",
      "gnome-terminal.desktop",
      "konsole.desktop",
      NULL
   };
   const char *s;
   char buf[PATH_MAX];
   Efreet_Desktop *tdesktop = NULL, *td;
   Eina_List *l;
   int i;

   s = efreet_data_home_get();
   if (s)
     {
        snprintf(buf, sizeof(buf), "%s/mimeapps.list",
                 efreet_config_home_get());
        tdesktop = _e_util_default_terminal_get(buf);
     }
   if (tdesktop) return tdesktop;
   EINA_LIST_FOREACH(efreet_data_dirs_get(), l, s)
     {
        snprintf(buf, sizeof(buf), "%s/applications/defaults.list", s);
        tdesktop = _e_util_default_terminal_get(buf);
        if (tdesktop) return tdesktop;
     }

   for (i = 0; terms[i]; i++)
     {
        tdesktop = efreet_util_desktop_file_id_find(terms[i]);
        if (tdesktop) return tdesktop;
     }
   if (!tdesktop)
     {
        l = efreet_util_desktop_category_list("TerminalEmulator");
        if (l)
          {
             // just take first one since above list doesn't work.
             tdesktop = l->data;
             EINA_LIST_FREE(l, td)
               {
                  // free/unref the desktops we are not going to use
                  if (td != tdesktop) efreet_desktop_free(td);
               }
          }
     }
   return tdesktop;
}

E_API E_Config_Binding_Key *
e_util_binding_match(const Eina_List *bindlist, Ecore_Event_Key *ev, unsigned int *num, const E_Config_Binding_Key *skip)
{
   E_Config_Binding_Key *bi;
   const Eina_List *l;
   unsigned int mod = E_BINDING_MODIFIER_NONE;

   if (num) *num = 0;

   if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
     mod |= E_BINDING_MODIFIER_SHIFT;
   if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)
     mod |= E_BINDING_MODIFIER_CTRL;
   if (ev->modifiers & ECORE_EVENT_MODIFIER_ALT)
     mod |= E_BINDING_MODIFIER_ALT;
   if (ev->modifiers & ECORE_EVENT_MODIFIER_WIN)
     mod |= E_BINDING_MODIFIER_WIN;
   /* see comment in e_bindings on numlock
      if (ev->modifiers & ECORE_X_LOCK_NUM)
      mod |= ECORE_X_LOCK_NUM;
    */
   EINA_LIST_FOREACH(bindlist ? : e_bindings->key_bindings, l, bi)
     {
        if (bi != skip)
          {
             if ((bi->modifiers == mod) && eina_streq(bi->key, ev->key))
               return bi;
          }
        if (num) (*num)++;
     }
   if (num) *num = 0;
   return NULL;
}

E_API void
e_util_gadcon_orient_icon_set(E_Gadcon_Orient orient, Evas_Object *obj)
{
   switch (orient)
     {
      case E_GADCON_ORIENT_LEFT:
        e_util_icon_theme_set(obj, "preferences-position-left");
        break;

      case E_GADCON_ORIENT_RIGHT:
        e_util_icon_theme_set(obj, "preferences-position-right");
        break;

      case E_GADCON_ORIENT_TOP:
        e_util_icon_theme_set(obj, "preferences-position-top");
        break;

      case E_GADCON_ORIENT_BOTTOM:
        e_util_icon_theme_set(obj, "preferences-position-bottom");
        break;

      case E_GADCON_ORIENT_CORNER_TL:
        e_util_icon_theme_set(obj, "preferences-position-top-left");
        break;

      case E_GADCON_ORIENT_CORNER_TR:
        e_util_icon_theme_set(obj, "preferences-position-top-right");
        break;

      case E_GADCON_ORIENT_CORNER_BL:
        e_util_icon_theme_set(obj, "preferences-position-bottom-left");
        break;

      case E_GADCON_ORIENT_CORNER_BR:
        e_util_icon_theme_set(obj, "preferences-position-bottom-right");
        break;

      case E_GADCON_ORIENT_CORNER_LT:
        e_util_icon_theme_set(obj, "preferences-position-left-top");
        break;

      case E_GADCON_ORIENT_CORNER_RT:
        e_util_icon_theme_set(obj, "preferences-position-right-top");
        break;

      case E_GADCON_ORIENT_CORNER_LB:
        e_util_icon_theme_set(obj, "preferences-position-left-bottom");
        break;

      case E_GADCON_ORIENT_CORNER_RB:
        e_util_icon_theme_set(obj, "preferences-position-right-bottom");
        break;

      default:
        e_util_icon_theme_set(obj, "enlightenment");
        break;
     }
}

E_API void
e_util_gadcon_orient_menu_item_icon_set(E_Gadcon_Orient orient, E_Menu_Item *mi)
{
   switch (orient)
     {
      case E_GADCON_ORIENT_LEFT:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-left");
        break;

      case E_GADCON_ORIENT_RIGHT:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-right");
        break;

      case E_GADCON_ORIENT_TOP:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-top");
        break;

      case E_GADCON_ORIENT_BOTTOM:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-bottom");
        break;

      case E_GADCON_ORIENT_CORNER_TL:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-top-left");
        break;

      case E_GADCON_ORIENT_CORNER_TR:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-top-right");
        break;

      case E_GADCON_ORIENT_CORNER_BL:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-bottom-left");
        break;

      case E_GADCON_ORIENT_CORNER_BR:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-bottom-right");
        break;

      case E_GADCON_ORIENT_CORNER_LT:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-left-top");
        break;

      case E_GADCON_ORIENT_CORNER_RT:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-right-top");
        break;

      case E_GADCON_ORIENT_CORNER_LB:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-left-bottom");
        break;

      case E_GADCON_ORIENT_CORNER_RB:
        e_util_menu_item_theme_icon_set(mi, "preferences-position-right-bottom");
        break;

      default:
        e_util_menu_item_theme_icon_set(mi, "enlightenment");
        break;
     }
}

E_API char *
e_util_string_append_char(char *str, size_t *size, size_t *len, char c)
{
   if (!str)
     {
        str = malloc(4096);
        if (!str) return NULL;
        str[0] = 0;
        *size = 4096;
        *len = 0;
     }

   if (*len >= *size - 1)
     {
        char *str2;

        *size += 1024;
        str2 = realloc(str, *size);
        if (!str2)
          {
             *size = 0;
             free(str);
             return NULL;
          }
        str = str2;
     }

   str[(*len)++] = c;
   str[*len] = 0;

   return str;
}

E_API char *
e_util_string_append_quoted(char *str, size_t *size, size_t *len, const char *src)
{
   str = e_util_string_append_char(str, size, len, '\'');
   if (!str) return NULL;

   while (*src)
     {
        if (*src == '\'')
          {
             str = e_util_string_append_char(str, size, len, '\'');
             if (!str) return NULL;
             str = e_util_string_append_char(str, size, len, '\\');
             if (!str) return NULL;
             str = e_util_string_append_char(str, size, len, '\'');
             if (!str) return NULL;
             str = e_util_string_append_char(str, size, len, '\'');
             if (!str) return NULL;
          }
        else
          {
             str = e_util_string_append_char(str, size, len, *src);
             if (!str) return NULL;
          }

        src++;
     }

   str = e_util_string_append_char(str, size, len, '\'');
   if (!str) return NULL;

   return str;
}

E_API void
e_util_evas_objects_above_print(Evas_Object *o)
{
   Evas_Object *a, *oo;

   EINA_SAFETY_ON_NULL_RETURN(o);
   a = o;
   while ((a = evas_object_above_get(a)))
     {
        const Eina_List *l, *ll;

        l = evas_object_clipees_get(a);
        if (l)
          {
             fprintf(stderr, "[%p] - %s(%s) %s :: CLIPPEES: ", a, evas_object_type_get(a), evas_object_name_get(a), evas_object_visible_get(a) ? "VISIBLE" : "HIDDEN");
             EINA_LIST_FOREACH(l, ll, oo)
               fprintf(stderr, "[%p] - %s(%s) %s", oo, evas_object_type_get(oo), evas_object_name_get(oo), ll->next ? "| " : "");
          }
        else
          fprintf(stderr, "[%p] - %s(%s) %s\n", a, evas_object_type_get(a), evas_object_name_get(a), evas_object_visible_get(a) ? "VISIBLE" : "HIDDEN");
     }
}

E_API void
e_util_evas_objects_above_print_smart(Evas_Object *o)
{
   Evas_Object *a, *oo;

   EINA_SAFETY_ON_NULL_RETURN(o);
   a = o;
   while ((a = evas_object_above_get(a)))
     {
        const Eina_List *l, *ll;

        l = evas_object_clipees_get(a);
        if (l)
          {
             fprintf(stderr, "[%p] - %s(%s) %s :: CLIPPEES: ", a, evas_object_type_get(a), evas_object_name_get(a), evas_object_visible_get(a) ? "VISIBLE" : "HIDDEN");
             EINA_LIST_FOREACH(l, ll, oo)
               fprintf(stderr, "[%p] - %s(%s) %s", oo, evas_object_type_get(oo), evas_object_name_get(oo), ll->next ? "| " : "");
             fprintf(stderr, "\n");
          }
        else if (evas_object_smart_data_get(a))
          {
             fprintf(stderr, "[%p] - %s(%s) %s :: SMART MEMBERS: ", a, evas_object_type_get(a), evas_object_name_get(a), evas_object_visible_get(a) ? "VISIBLE" : "HIDDEN");
             EINA_LIST_FOREACH(evas_object_smart_members_get(a), l, oo)
               fprintf(stderr, "[%p] - %s(%s) %s", oo, evas_object_type_get(oo), evas_object_name_get(oo), l->next ? "| " : "");
             fprintf(stderr, "\n");
          }
        else
          fprintf(stderr, "[%p] - %s(%s) %s\n", a, evas_object_type_get(a), evas_object_name_get(a), evas_object_visible_get(a) ? "VISIBLE" : "HIDDEN");
     }
}

#if   defined(HAVE_MEMSET_S)
#elif defined(HAVE_EXPLICIT_BZERO)
#elif defined(HAVE_EXPLICIT_MEMSET)
#else
void *(* const volatile __memset_ptr)(void *, int, size_t) = memset;
#endif

E_API void
e_util_memclear(void *s, size_t n)
{
   if (n == 0) return;
#if   defined(HAVE_MEMSET_S)
   memset_s(s, n, 0, n);
#elif defined(HAVE_EXPLICIT_BZERO)
   explicit_bzero(s, n);
#elif defined(HAVE_EXPLICIT_MEMSET)
   explicit_memset(s, 0, n);
#else
   __memset_ptr(s, 0, n);
   __asm__ __volatile__("": :"r"(s) : "memory");
#endif
}

E_API Ecore_Exe *
e_util_open(const char *exe, void *data)
{
   char *sb;
   size_t size = 65536, len;
   Ecore_Exe *ret;

   sb = malloc(size);
   snprintf(sb, size, "%s/enlightenment_open ", e_prefix_bin_get());
   len = strlen(sb);
   sb = e_util_string_append_quoted(sb, &size, &len, exe);
   ret = e_util_exe_safe_run(sb, data);
   free(sb);
   return ret;
}

E_API Ecore_Exe *
e_util_exe_safe_run(const char *cmd, void *data)
{
   Ecore_Exe_Flags flags = ECORE_EXE_NONE;

#if (ECORE_VERSION_MAJOR >= 1) && (ECORE_VERSION_MINOR >= 21)
   flags |= ECORE_EXE_ISOLATE_IO;
#else
   flags |= 1024; // isolate_io is bit 10 .... it will be ignored if
                  // efl doesn't do it, so harmless
#endif
   return ecore_exe_pipe_run(cmd, flags, data);
}

static Eina_Bool
str_matches_one(const char *instr, const char **strset)
{
   int i;

   if ((!instr) || (!strset)) return EINA_FALSE;
   for (i = 0; strset[i]; i++)
     {
        if (!strcmp(strset[i], instr)) return EINA_TRUE;
     }
   return EINA_FALSE;
}

E_API E_Util_Action
e_util_key_geometry_action_get(const char *key, int *x, int *y, int dx, int dy)
{
   const char *k_up[]    = { "Up",     "KP_Up",     "k", "K", "w", "W", NULL };
   const char *k_down[]  = { "Down",   "KP_Down",   "j", "J", "s", "S", NULL };
   const char *k_left[]  = { "Left",   "KP_Left",   "h", "H", "a", "A", NULL };
   const char *k_right[] = { "Right",  "KP_Right",  "l", "L", "d", "D", NULL };
   const char *k_done[]  = { "Return", "KP_Enter",  "space",  NULL };
   const char *k_abort[] = { "Escape", "BackSpace", "Delete", "x", "X", NULL };

   if      (str_matches_one(key, k_up))    *y -= dy;
   else if (str_matches_one(key, k_down))  *y += dy;
   else if (str_matches_one(key, k_left))  *x -= dx;
   else if (str_matches_one(key, k_right)) *x += dx;
   else if (str_matches_one(key, k_done))  return E_UTIL_ACTION_DONE;
   else if (str_matches_one(key, k_abort)) return E_UTIL_ACTION_ABORT;
   else return E_UTIL_ACTION_NONE;

   return E_UTIL_ACTION_DO;
}
