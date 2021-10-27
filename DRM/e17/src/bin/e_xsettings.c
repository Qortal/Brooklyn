/* TODO
   check http://www.pvv.org/~mariusbu/proposal.html
   for advances in cross toolkit settings */

#include <e.h>
//#include <X11/Xlib.h>
//#include <X11/Xmd.h>            /* For CARD16 */

// define here to avoid needing x includes directly.
#define C16                 unsigned short
#define C32                 unsigned int

#define RETRY_TIMEOUT       2.0

#define SETTING_TYPE_INT    0
#define SETTING_TYPE_STRING 1
#define SETTING_TYPE_COLOR  2

#define OFFSET_ADD(n) ((n + 4 - 1) & (~(4 - 1)))

typedef struct _Settings_Manager Settings_Manager;
typedef struct _Setting         Setting;

struct _Settings_Manager
{
   Ecore_X_Window  selection;
   Ecore_Timer    *timer_retry;
   unsigned long   serial;
   Ecore_X_Atom    _atom_xsettings_screen;
   Eina_Bool       enabled E_BITFIELD;
};

struct _Setting
{
   unsigned short type;
   const char    *name;
   struct {
      const char *value;
   } s;
   struct {
      int value;
   } i;
   struct {
      unsigned short red, green, blue, alpha;
   } c;
   unsigned long length;
   unsigned long last_change;
};

static void _e_xsettings_apply(Settings_Manager *sm);

static Ecore_X_Atom _atom_manager = 0;
static Ecore_X_Atom _atom_xsettings = 0;
static Ecore_X_Atom _atom_gtk_iconthemes = 0;
static Ecore_X_Atom _atom_gtk_rcfiles = 0;
static Settings_Manager *manager = NULL;
static Eina_List *settings = NULL;
static Eina_Bool running = EINA_FALSE;
static Eio_File *eio_op = NULL;
static Eina_Bool setting = EINA_FALSE;
static Eina_Bool reset = EINA_FALSE;
static const char _setting_icon_theme_name[] = "Net/IconThemeName";
static const char _setting_theme_name[] = "Net/ThemeName";
static const char _setting_font_name[] = "Gtk/FontName";
static const char _setting_xft_dpi[] = "Xft/DPI";
static const char _setting_xft_antialias[] = "Xft/Antialias";
static const char _setting_xft_autohint[] = "Xft/Autohint";
static const char _setting_xft_hinting[] = "Xft/Hinting";
static const char _setting_xft_hintstyle[] = "Xft/HintStyle";
static const char _setting_xft_rgba[] = "Xft/RGBA";
static const char *_setting_theme = NULL;

static void _e_xsettings_done_cb(void *data, Eio_File *handler, const Eina_Stat *stat);

static Ecore_X_Atom
_e_xsettings_atom_screen_get(int screen_num)
{
   char buf[32];

   snprintf(buf, sizeof(buf), "_XSETTINGS_S%d", screen_num);
   return ecore_x_atom_get(buf);
}

static Eina_Bool
_e_xsettings_selection_owner_set(void)
{
   Ecore_X_Atom atom;
   Ecore_X_Window cur_selection;
   Eina_Bool ret;

   atom = _e_xsettings_atom_screen_get(0);
   ecore_x_selection_owner_set(e_comp->cm_selection, atom,
                               ecore_x_current_time_get());
   ecore_x_sync();
   cur_selection = ecore_x_selection_owner_get(atom);

   ret = (cur_selection == e_comp->cm_selection);
   if (!ret)
     ERR("XSETTINGS: tried to set selection to %#x, but got %#x",
         (unsigned int)e_comp->cm_selection, cur_selection);

   return ret;
}

static void
_e_xsettings_deactivate(Settings_Manager *sm)
{
   Ecore_X_Atom atom;

   atom = _e_xsettings_atom_screen_get(0);
   ecore_x_selection_owner_set(0, atom, ecore_x_current_time_get());
   ecore_x_sync();
   sm->enabled = 0;
}

static Eina_Bool
_e_xsettings_activate(Settings_Manager *sm)
{
   Ecore_X_Atom atom;
   Ecore_X_Window old_win;

   if (sm->enabled) return 1;

   atom = _e_xsettings_atom_screen_get(0);
   old_win = ecore_x_selection_owner_get(atom);
   if (old_win != 0) return 0;

   if (!_e_xsettings_selection_owner_set())
     return 0;

   ecore_x_client_message32_send(e_comp->root, _atom_manager,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 ecore_x_current_time_get(), atom,
                                 e_comp->cm_selection, 0, 0);

   if (settings) _e_xsettings_apply(sm);
   sm->enabled = 1;

   return 1;
}

static Eina_Bool
_e_xsettings_activate_retry(void *data)
{
   Settings_Manager *sm = data;
   Eina_Bool ret;

   INF("XSETTINGS: reactivate...");
   ret = _e_xsettings_activate(sm);
   if (ret)
     INF("XSETTINGS: activate success!");
   else
     ERR("XSETTINGS: activate failure! retrying in %0.1f seconds", RETRY_TIMEOUT);

   if (!ret)
     return ECORE_CALLBACK_RENEW;

   sm->timer_retry = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_e_xsettings_retry(Settings_Manager *sm)
{
   if (sm->timer_retry) return;
   sm->timer_retry = ecore_timer_loop_add
     (RETRY_TIMEOUT, _e_xsettings_activate_retry, sm);
}

static void
_e_xsettings_string_set(const char *name, const char *value)
{
   Setting *s;
   Eina_List *l;

   if (!name) return;
   if (name == _setting_theme_name)
     e_config->xsettings.net_theme_name_detected = value;
   name = eina_stringshare_add(name);

   EINA_LIST_FOREACH(settings, l, s)
     {
        if (s->type != SETTING_TYPE_STRING) continue;
        if (s->name == name) break;
     }
   if (!value)
     {
        if (!s) return;
        DBG("remove %s\n", name);
        eina_stringshare_del(name);
        eina_stringshare_del(s->name);
        eina_stringshare_del(s->s.value);
        settings = eina_list_remove(settings, s);
        E_FREE(s);
        return;
     }
   if (s)
     {
        DBG("update %s %s\n", name, value);
        eina_stringshare_del(name);
        eina_stringshare_replace(&s->s.value, value);
     }
   else
     {
        DBG("add %s %s\n", name, value);
        s = E_NEW(Setting, 1);
        s->type = SETTING_TYPE_STRING;
        s->name = name;
        s->s.value = eina_stringshare_add(value);
        settings = eina_list_append(settings, s);
     }

   /* type + pad + name-len + last-change-serial + str_len */
   s->length = 12;
   s->length += OFFSET_ADD(strlen(name));
   s->length += OFFSET_ADD(strlen(value));
   s->last_change = ecore_x_current_time_get();
}

static void
_e_xsettings_int_set(const char *name, int value, Eina_Bool set)
{
   Setting *s;
   Eina_List *l;

   if (!name) return;
   name = eina_stringshare_add(name);

   EINA_LIST_FOREACH(settings, l, s)
     {
        if (s->type != SETTING_TYPE_INT) continue;
        if (s->name == name) break;
     }
   if (!set)
     {
        if (!s) return;
        DBG("remove %s\n", name);
        eina_stringshare_del(name);
        eina_stringshare_del(s->name);
        settings = eina_list_remove(settings, s);
        E_FREE(s);
        return;
     }
   if (s)
     {
        DBG("update %s %d\n", name, value);
        eina_stringshare_del(name);
        s->i.value = value;
     }
   else
     {
        DBG("add %s %d\n", name, value);
        s = E_NEW(Setting, 1);
        s->type = SETTING_TYPE_INT;
        s->name = name;
        s->i.value = value;
        settings = eina_list_append(settings, s);
     }

   // type + pad + name-len + last-change-serial + value
   s->length = 12;
   s->length += OFFSET_ADD(strlen(name));
}

static unsigned char *
_e_xsettings_copy(unsigned char *buffer, Setting *s)
{
   size_t str_len;
   size_t len;
   C16 tmp16;
   C32 tmp32;

   buffer[0] = s->type;
   buffer[1] = 0;
   buffer += 2;

   str_len = strlen(s->name);
   tmp16 = str_len;
   memcpy(buffer, &tmp16, sizeof(C16));
   buffer += 2;

   memcpy(buffer, s->name, str_len);
   buffer += str_len;

   len = OFFSET_ADD(str_len) - str_len;
   memset(buffer, 0, len);
   buffer += len;

   tmp32 = s->last_change;
   memcpy(buffer, &tmp32, sizeof(C32));
   buffer += 4;

   switch (s->type)
     {
      case SETTING_TYPE_INT:
        tmp32 = s->i.value;
        memcpy(buffer, &tmp32, sizeof(C32));
        buffer += 4;
        break;

      case SETTING_TYPE_STRING:
        str_len = strlen(s->s.value);
        tmp32 = str_len;
        memcpy(buffer, &tmp32, sizeof(C32));
        buffer += 4;

        memcpy(buffer, s->s.value, str_len);
        buffer += str_len;

        len = OFFSET_ADD(str_len) - str_len;
        memset(buffer, 0, len);
        buffer += len;
        break;

      case SETTING_TYPE_COLOR:
        tmp16 = s->c.red;
        memcpy(buffer, &tmp16, sizeof(C16));
        buffer += 2;
        tmp16 = s->c.green;
        memcpy(buffer, &tmp16, sizeof(C16));
        buffer += 2;
        tmp16 = s->c.blue;
        memcpy(buffer, &tmp16, sizeof(C16));
        buffer += 2;
        tmp16 = s->c.alpha;
        memcpy(buffer, &tmp16, sizeof(C16));
        buffer += 2;
        break;
     }

   return buffer;
}

static void
_e_xsettings_apply(Settings_Manager *sm)
{
   unsigned char *data;
   unsigned char *pos;
   size_t len = 12;
   Setting *s;
   Eina_List *l;
   C32 tmp32;

   EINA_LIST_FOREACH(settings, l, s)
     len += s->length;

   pos = data = calloc(1, len);
   if (!data) return;

#if (defined __BYTE_ORDER && __BYTE_ORDER == __LITTLE_ENDIAN) || (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
   *pos = 0; //LSBFirst
#else
   *pos = 1; //MSBFirst
#endif

   pos += 4;
   tmp32 = sm->serial++;
   memcpy(pos, &tmp32, sizeof(C32));
   pos += 4;
   tmp32 = eina_list_count(settings);
   memcpy(pos, &tmp32, sizeof(C32));
   pos += 4;

   EINA_LIST_FOREACH(settings, l, s)
     pos = _e_xsettings_copy(pos, s);

   ecore_x_window_prop_property_set(e_comp->cm_selection,
                                    _atom_xsettings,
                                    _atom_xsettings,
                                    8, data, len);
   free(data);
}

static void
_e_xsettings_update(void)
{
   if (e_comp->cm_selection) _e_xsettings_apply(manager);
}

static void
_e_xsettings_gtk_icon_update(void)
{
   const Eina_List *l;
   E_Client *ec;

   EINA_LIST_FOREACH(e_comp->clients, l, ec)
     if (ec->icccm.state)
       ecore_x_client_message8_send(e_client_util_win_get(ec),
                                    _atom_gtk_iconthemes, NULL, 0);
}

static void
_e_xsettings_gtk_rcfiles_update(void)
{
   const Eina_List *l;
   E_Client *ec;

   EINA_LIST_FOREACH(e_comp->clients, l, ec)
     if (ec->icccm.state)
       ecore_x_client_message8_send(e_client_util_win_get(ec),
                                    _atom_gtk_rcfiles, NULL, 0);
}

static void
_e_xsettings_icon_theme_set(void)
{
   if (e_config->xsettings.match_e17_icon_theme)
     {
        _e_xsettings_string_set(_setting_icon_theme_name,
                                e_config->icon_theme);
        return;
     }

   if (e_config->xsettings.net_icon_theme_name)
     {
        _e_xsettings_string_set(_setting_icon_theme_name,
                                e_config->xsettings.net_icon_theme_name);
        return;
     }

   _e_xsettings_string_set(_setting_icon_theme_name, NULL);
}

static void
_e_xsettings_error_cb(void *data, Eio_File *handler EINA_UNUSED, int error EINA_UNUSED)
{
   Eina_List *l = data;
   if (reset || setting)
     {
        char buf[PATH_MAX];
        if (reset || (!l)) l = efreet_data_dirs_get();
        else if (l)
          l = l->next;
        reset = EINA_FALSE;
        if (l)
          {
             snprintf(buf, sizeof(buf), "%s/themes/%s",
                      (char *)eina_list_data_get(l), _setting_theme);
             eio_op = eio_file_direct_stat(buf, _e_xsettings_done_cb,
                                           _e_xsettings_error_cb, l);
             return;
          }
     }
   eio_op = NULL;
   setting = EINA_FALSE;
   _setting_theme = NULL;

   if (e_config->xsettings.net_theme_name)
     _e_xsettings_string_set(_setting_theme_name,
                             e_config->xsettings.net_theme_name);
   else
     _e_xsettings_string_set(_setting_theme_name, NULL);
   _e_xsettings_update();
}

static void
_e_xsettings_done_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const Eina_Stat *estat EINA_UNUSED)
{
   if (reset)
     {
        /* should not happen */
        _e_xsettings_error_cb(NULL, NULL, 0);
        return;
     }
   _e_xsettings_string_set(_setting_theme_name, _setting_theme);
   _setting_theme = NULL;
   eio_op = NULL;
   setting = EINA_FALSE;
   _e_xsettings_update();
}

static void
_e_xsettings_theme_set(void)
{
   if (e_config->xsettings.match_e17_theme)
     {
        const char *file;

        file = e_theme_edje_file_get(NULL, "e/desktop/background");
        if (file)
          {
             if ((_setting_theme = edje_file_data_get(file, "gtk-theme")))
               {
                  char buf[PATH_MAX];

                  e_user_homedir_snprintf(buf, sizeof(buf),
                                          ".themes/%s", _setting_theme);
                  eio_op = eio_file_direct_stat(buf, _e_xsettings_done_cb,
                                                _e_xsettings_error_cb, NULL);
                  setting = EINA_TRUE;
                  return;
               }
          }
     }

   if (e_config->xsettings.net_theme_name)
     {
        _e_xsettings_string_set(_setting_theme_name,
                                e_config->xsettings.net_theme_name);
        return;
     }

   _e_xsettings_string_set(_setting_theme_name, NULL);
}

static int
_dbl_to_str(char *buf, size_t buf_size, double val, int decimals)
{
   int ival;
   size_t l = 0;

   if (val < 0)
     {
        if (buf_size < 2) return -1;
        val = -val;
        buf[0] = '-';
        buf[1] = 0;
        l++;
     }
   ival = val;
   snprintf(buf + l, buf_size - l, "%i", ival);
   if (decimals <= 0) return 0;
   l = strlen(buf);
   if (l >= (buf_size - 2)) return -1;
   buf[l] = '.';
   buf[l + 1] = 0;
   l++;

   while (decimals > 0)
     {
        if (l >= (buf_size - 1)) return -1;
        val *= 10.0;
        ival = val;
        val = val - ival;
        buf[l] = '0' + (ival % 10);
        buf[l + 1] = 0;
        l++;
        if (l >= (buf_size - 1)) return -1;
        decimals--;
     }
   return 0;
}

static void
_e_xsettings_font_set(void)
{
   E_Font_Default *efd;
   E_Font_Properties *efp;

   efd = e_font_default_get("application");

   if (efd && efd->font)
     {
        efp = e_font_fontconfig_name_parse(efd->font);
        if (efp->name)
          {
             Eina_Strbuf *buf;
             Eina_List *l;
             int size = efd->size;
             char size_buf[32];
             const char *p;

             /* TODO better way to convert evas font sizes? */
             if      (!size)      size  =   10; // default if not set
             else if (size < 0)   size /=  -10; // in 1/10th units

             // limit sizes
             if      (size < 5)   size  =    5; // don't allow too small
             else if (size > 100) size  =  100; // don't allow silly sizes

             _dbl_to_str(size_buf, sizeof(size_buf), (double)size, 0);

             buf = eina_strbuf_new();
             eina_strbuf_append(buf, efp->name);
             eina_strbuf_append_char(buf, ' ');
             EINA_LIST_FOREACH(efp->styles, l, p)
               {
                  eina_strbuf_append(buf, p);
                  eina_strbuf_append_char(buf, ' ');
               }
             eina_strbuf_append(buf, size_buf);
             _e_xsettings_string_set(_setting_font_name,
                                     eina_strbuf_string_get(buf));
             eina_strbuf_free(buf);
             e_font_properties_free(efp);
             return;
          }

        e_font_properties_free(efp);
     }
   _e_xsettings_string_set(_setting_font_name, NULL);
}

static void
_e_xsettings_dpi_set(void)
{
   if (e_config->scale.set_xapp_dpi)
     _e_xsettings_int_set(_setting_xft_dpi,
                          (double)e_config->scale.xapp_base_dpi * e_scale * 1024.0,
                          EINA_TRUE);
   _e_xsettings_int_set(_setting_xft_antialias,
                        1,
                        EINA_TRUE);
   _e_xsettings_int_set(_setting_xft_hinting,
                        1,
                        EINA_TRUE);
   _e_xsettings_int_set(_setting_xft_autohint,
                        0,
                        EINA_TRUE);
   _e_xsettings_string_set(_setting_xft_hintstyle,
                           "hintfull");
   _e_xsettings_string_set(_setting_xft_rgba,
                           "none");
}

static void
_e_xsettings_cursor_path_set(void)
{
   struct stat st;
   char buf[PATH_MAX], env[PATH_MAX + PATH_MAX + 100], *path;

   e_user_homedir_concat_static(buf, ".icons");

   if (stat(buf, &st)) return;
   path = getenv("XCURSOR_PATH");
   if (path)
     {
        if (strstr(path, buf)) return;
        snprintf(env, sizeof(env), "%s:%s", buf, path);
        path = env;
     }
   else
     {
        snprintf(env, sizeof(env), "%s:%s", buf, "/usr/share/icons");
        path = env;
     }
   e_env_set("XCURSOR_PATH", path);
}

static void
_e_xsettings_start(void)
{
   if (running) return;

   _e_xsettings_dpi_set();
   _e_xsettings_theme_set();
   _e_xsettings_icon_theme_set();
   _e_xsettings_font_set();
   _e_xsettings_cursor_path_set();

   manager = E_NEW(Settings_Manager, 1);

   if (!_e_xsettings_activate(manager))
     _e_xsettings_retry(manager);

   running = EINA_TRUE;
}

static void
_e_xsettings_stop(void)
{
   Setting *s;

   if (!running) return;

   if (manager->timer_retry)
     ecore_timer_del(manager->timer_retry);

   if ((!stopping) && (!x_fatal))
     _e_xsettings_deactivate(manager);

   E_FREE(manager);

   EINA_LIST_FREE(settings, s)
     {
        if (s->name) eina_stringshare_del(s->name);
        if (s->s.value) eina_stringshare_del(s->s.value);
        E_FREE(s);
     }

   running = EINA_FALSE;
}

EINTERN int
e_xsettings_init(void)
{
   _atom_manager = ecore_x_atom_get("MANAGER");
   _atom_xsettings = ecore_x_atom_get("_XSETTINGS_SETTINGS");
   _atom_gtk_iconthemes = ecore_x_atom_get("_GTK_LOAD_ICONTHEMES");
   _atom_gtk_rcfiles = ecore_x_atom_get("_GTK_READ_RCFILES");

   if (e_config->xsettings.enabled)
     {
        _e_xsettings_start();
        if (!getenv("E_RESTART"))
          _e_xsettings_gtk_rcfiles_update();
     }

   return 1;
}

EINTERN int
e_xsettings_shutdown(void)
{
   _e_xsettings_stop();
   if (eio_op) eio_file_cancel(eio_op);
   eio_op = NULL;
   setting = EINA_FALSE;

   return 1;
}

static void
_update_sequence(void)
{
   _e_xsettings_dpi_set();
   _e_xsettings_theme_set();
   _e_xsettings_icon_theme_set();
   _e_xsettings_font_set();
   _e_xsettings_update();
   _e_xsettings_gtk_icon_update();
}

E_API void
e_xsettings_config_update(void)
{
   if (!_atom_manager) return;
   setting = EINA_FALSE;
   if (eio_op) eio_file_cancel(eio_op);
   if (!e_config->xsettings.enabled)
     {
        _e_xsettings_stop();
        return;
     }

   if (!running)
     {
        _e_xsettings_start();
     }
   else
     {
        _update_sequence();
        reset = EINA_TRUE;
     }
}

