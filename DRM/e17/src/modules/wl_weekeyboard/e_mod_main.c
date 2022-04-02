#include "e.h"

#include "wkb-log.h"

#include "input-method-unstable-v1-client-protocol.h"
#include "text-input-unstable-v1-client-protocol.h"

struct weekeyboard
{
   E_Module *module;
   Ecore_Evas *ee;
   Ecore_Wl2_Window *win;
   Evas_Object *edje_obj;
   const char *ee_engine;
   char **ignore_keys;

   struct wl_surface *surface;
   struct zwp_input_panel_v1 *ip;
   struct zwp_input_method_v1 *im;
   struct wl_output *output;
   struct zwp_input_method_context_v1 *im_ctx;

   char *surrounding_text;
   char *preedit_str;
   char *language;
   char *theme;

   uint32_t text_direction;
   uint32_t preedit_style;
   uint32_t content_hint;
   uint32_t content_purpose;
   uint32_t serial;
   uint32_t surrounding_cursor;

   Eina_Bool context_changed;
};

static char *
_wkb_insert_text(const char *text, uint32_t offset, const char *insert)
{
   size_t insert_len, text_len;
   char *new_text;

   if ((!text) || (!insert)) return NULL;

   text_len = strlen(text);
   insert_len = strlen(insert);
   new_text = malloc(text_len +insert_len + 1);
   if (!new_text)
     {
        ERR("out of memory");
        return NULL;
     }

   if (offset > text_len) offset = text_len;

   memcpy(new_text, text, offset);
   new_text[offset] = '\0';
   memcpy(new_text, insert, insert_len);
   strcpy(new_text + insert_len, text + offset);

   return new_text;
}

static void
_wkb_commit_preedit_str(struct weekeyboard *wkb)
{
   char *surrounding_text;

   if ((!wkb->preedit_str) || (strlen(wkb->preedit_str) == 0))
     return;

   zwp_input_method_context_v1_cursor_position(wkb->im_ctx, 0, 0);
   zwp_input_method_context_v1_commit_string(wkb->im_ctx, wkb->serial,
                                         wkb->preedit_str);

   if (wkb->surrounding_text)
     {
        surrounding_text =
          _wkb_insert_text(wkb->surrounding_text, wkb->surrounding_cursor,
                           wkb->preedit_str);
        free(wkb->surrounding_text);
        wkb->surrounding_text = surrounding_text;
        wkb->surrounding_cursor += strlen(wkb->preedit_str);
     }
   else
     {
        wkb->surrounding_text = strdup(wkb->preedit_str);
        wkb->surrounding_cursor = strlen(wkb->preedit_str);
     }

   free(wkb->preedit_str);
   wkb->preedit_str = strdup("");
}

static void
_wkb_send_preedit_str(struct weekeyboard *wkb, int cursor)
{
   unsigned int index = strlen(wkb->preedit_str);

   if (wkb->preedit_style)
     zwp_input_method_context_v1_preedit_styling(wkb->im_ctx, 0,
                                             strlen(wkb->preedit_str),
                                             wkb->preedit_style);

   if (cursor > 0)
     index = cursor;

   zwp_input_method_context_v1_preedit_cursor(wkb->im_ctx, index);
   zwp_input_method_context_v1_preedit_string(wkb->im_ctx, wkb->serial,
                                          wkb->preedit_str, wkb->preedit_str);
}

static void
_wkb_update_preedit_str(struct weekeyboard *wkb, const char *key)
{
   char *tmp;

   if (!wkb->preedit_str)
     wkb->preedit_str = strdup("");

   tmp = _wkb_insert_text(wkb->preedit_str, strlen(wkb->preedit_str), key);
   free(wkb->preedit_str);
   wkb->preedit_str = tmp;

   if (eina_streq(key, " "))
     _wkb_commit_preedit_str(wkb);
   else
     _wkb_send_preedit_str(wkb, -1);
}

static Eina_Bool
_wkb_ignore_key(struct weekeyboard *wkb, const char *key)
{
   int i;

   if (!wkb->ignore_keys)
     return EINA_FALSE;

   for (i = 0; wkb->ignore_keys[i] != NULL; i++)
     if (eina_streq(key, wkb->ignore_keys[i]))
       return EINA_TRUE;

   return EINA_FALSE;
}

static void
_cb_wkb_on_key_down(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source)
{
   struct weekeyboard *wkb = data;
   char *src;
   const char *key;

   EINA_SAFETY_ON_FALSE_RETURN(wkb);
   EINA_SAFETY_ON_FALSE_RETURN(source);

   src = strdup(source);
   if (!src)
     {
        ERR("out of memory");
        return;
     }

   key = strtok(src, ":");
   key = strtok(NULL, ":");
   if (key == NULL)
     key = ":";

   if (_wkb_ignore_key(wkb, key))
     {
        DBG("Ignoring key: '%s'", key);
        goto end;
     }
   else if (eina_streq(key, "backspace"))
     {
        if (strlen(wkb->preedit_str) == 0)
          {
             zwp_input_method_context_v1_delete_surrounding_text(wkb->im_ctx, -1, 1);
             zwp_input_method_context_v1_commit_string(wkb->im_ctx, wkb->serial, "");
          }
        else
          {
             wkb->preedit_str[strlen(wkb->preedit_str) -1] = '\0';
             _wkb_send_preedit_str(wkb, -1);
          }

        goto end;
     }
   else if (eina_streq(key, "enter"))
     {
        _wkb_commit_preedit_str(wkb);
        zwp_input_method_context_v1_keysym(wkb->im_ctx, wkb->serial, 0,
                                       XKB_KEY_Return,
                                       WL_KEYBOARD_KEY_STATE_PRESSED, 0);
        goto end;
     }
   else if (eina_streq(key, "space"))
     {
        key = " ";
     }

   DBG("Key pressed: '%s'", key);

   _wkb_update_preedit_str(wkb, key);

end:
   free(src);
}

static Eina_Bool
_wkb_ui_setup(struct weekeyboard *wkb)
{
   char path[PATH_MAX];
   int w = 1080, h;
   char *ignore_keys;
   const char *theme;

   /* First run */
   if (!wkb->edje_obj)
     {
        Evas *evas;

        ecore_evas_alpha_set(wkb->ee, EINA_TRUE);
        ecore_evas_title_set(wkb->ee, "Weekeyboard");

        evas = ecore_evas_get(wkb->ee);
        wkb->edje_obj = edje_object_add(evas);
        edje_object_signal_callback_add(wkb->edje_obj, "key_down", "*",
                                        _cb_wkb_on_key_down, wkb);
     }

   // hard coded
   theme = "default";

   if ((wkb->theme) && (eina_streq(theme, wkb->theme)))
     return EINA_TRUE;

   free(wkb->theme);
   wkb->theme = strdup(theme);

   if (eina_streq(wkb->theme, "default"))
     {
        ecore_wl2_display_screen_size_get(e_comp_wl->ewd, &w, &h);
        DBG("Screen size: w=%d, h=%d", w, h);
        if (w >= 1080)
          w = 1080;
        else if (w >= 720)
          w = 720;
        else
          w = 600;

        DBG("Using default_%d theme", w);
     }

   snprintf(path, PATH_MAX, "%s/%s_%d.edj",
            e_module_dir_get(wkb->module), wkb->theme, w);
   INF("Loading edje file: '%s'", path);

   if (!edje_object_file_set(wkb->edje_obj, path, "main"))
     {
        int err = edje_object_load_error_get(wkb->edje_obj);
        ERR("Unable to load the edje file: '%s'", edje_load_error_str(err));
        return EINA_FALSE;
     }

   edje_object_size_min_get(wkb->edje_obj, &w, &h);
   DBG("edje_object_size_min_get - w: %d h: %d", w, h);
   if ((w == 0) || (h == 0))
     {
        edje_object_size_min_restricted_calc(wkb->edje_obj, &w, &h, w, h);
        DBG("edje_object_size_min_restricted_calc - w: %d h: %d", w, h);
        if ((w == 0) || (h == 0))
          {
             edje_object_parts_extends_calc(wkb->edje_obj, NULL, NULL, &w, &h);
             DBG("edje_object_parts_extends_calc - w: %d h: %d", w, h);
          }
     }

   ecore_evas_move_resize(wkb->ee, 0, 0, w, h);
   evas_object_move(wkb->edje_obj, 0, 0);
   evas_object_resize(wkb->edje_obj, w, h);
   evas_object_size_hint_min_set(wkb->edje_obj, w, h);
   evas_object_size_hint_max_set(wkb->edje_obj, w, h);

   if (wkb->win)
     {
        int rx, ry, rw, rh;

        edje_object_part_geometry_get(wkb->edje_obj, "background",
                                      &rx, &ry, &rw, &rh);
        ecore_wl2_window_input_region_set(wkb->win, rx, ry, rw, rh);
     }

   ignore_keys = edje_file_data_get(path, "ignore-keys");
   if (!ignore_keys)
     {
        ERR("Special keys file not found in: '%s'", path);
        goto end;
     }

   DBG("Got ignore keys: '%s'", ignore_keys);
   wkb->ignore_keys = eina_str_split(ignore_keys, "\n", 0);
   free(ignore_keys);

end:
   ecore_evas_show(wkb->ee);
   return EINA_TRUE;
}

static void
_wkb_im_ctx_surrounding_text(void *data, struct zwp_input_method_context_v1 *im_ctx, const char *text, uint32_t cursor, uint32_t anchor)
{
   struct weekeyboard *wkb = data;

   EINA_SAFETY_ON_NULL_RETURN(text);

   DBG("im_context = %p text = '%s' cursor = %d anchor = %d",
       im_ctx, text, cursor, anchor);

   free(wkb->surrounding_text);

   wkb->surrounding_text = strdup(text);
   if (!wkb->surrounding_text)
     {
        ERR("out of memory");
        return;
     }

   wkb->surrounding_cursor = cursor;
}

static void
_wkb_im_ctx_reset(void *data, struct zwp_input_method_context_v1 *im_ctx)
{
   struct weekeyboard *wkb = data;

   DBG("im_context = %p", im_ctx);

   if (strlen(wkb->preedit_str))
     {
        free(wkb->preedit_str);
        wkb->preedit_str = strdup("");
     }
}

static void
_wkb_im_ctx_content_type(void *data, struct zwp_input_method_context_v1 *im_ctx, uint32_t hint, uint32_t purpose)
{
   struct weekeyboard *wkb = data;

   DBG("im_context = %p hint = %d purpose = %d", im_ctx, hint, purpose);

   if (!wkb->context_changed)
     return;

   switch (purpose)
     {
      case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_DIGITS:
      case ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_NUMBER:
           {
              if (wkb->edje_obj)
                 edje_object_signal_emit(wkb->edje_obj, "show,numeric", "");
              break;
           }
      default:
           {
              if (wkb->edje_obj)
                 edje_object_signal_emit(wkb->edje_obj, "show,alphanumeric", "");
              break;
           }
     }

   wkb->content_hint = hint;
   wkb->content_purpose = purpose;
   wkb->context_changed = EINA_FALSE;
}

static void
_wkb_im_ctx_invoke_action(void *data, struct zwp_input_method_context_v1 *im_ctx, uint32_t button, uint32_t index)
{
   struct weekeyboard *wkb = data;

   DBG("im_context = %p button = %d, index = %d", im_ctx, button, index);

   if (button != BTN_LEFT)
     return;

   _wkb_send_preedit_str(wkb, index);
}

static void
_wkb_im_ctx_commit_state(void *data, struct zwp_input_method_context_v1 *im_ctx, uint32_t serial)
{
   struct weekeyboard *wkb = data;

   DBG("im_context = %p serial = %d", im_ctx, serial);

   if (wkb->surrounding_text)
     INF("Surrounding text updated: %s", wkb->surrounding_text);

   wkb->serial = serial;

   zwp_input_method_context_v1_language(im_ctx, wkb->serial, "en");
   zwp_input_method_context_v1_text_direction(im_ctx, wkb->serial,
                                          ZWP_TEXT_INPUT_V1_TEXT_DIRECTION_LTR);
}

static void
_wkb_im_ctx_preferred_language(void *data, struct zwp_input_method_context_v1 *im_ctx, const char *language)
{
   struct weekeyboard *wkb = data;

   DBG("im_context = %p language = %s", im_ctx, language ? language : "null");

   if ((language) && (wkb->language) && (eina_streq(language, wkb->language)))
     return;

   E_FREE_FUNC(wkb->language, free);

   if (language)
     {
        wkb->language = strdup(language);
        INF("Language changed, new: '%s'", language);
     }
}

static const struct zwp_input_method_context_v1_listener wkb_im_context_listener = {
   _wkb_im_ctx_surrounding_text,
   _wkb_im_ctx_reset,
   _wkb_im_ctx_content_type,
   _wkb_im_ctx_invoke_action,
   _wkb_im_ctx_commit_state,
   _wkb_im_ctx_preferred_language,
};

static void
_wkb_im_activate(void *data, struct zwp_input_method_v1 *input_method EINA_UNUSED, struct zwp_input_method_context_v1 *im_ctx)
{
   struct weekeyboard *wkb = data;

   DBG("Activate");

   // check if the UI is valid and draw it if not
   _wkb_ui_setup(wkb);

   E_FREE_FUNC(wkb->im_ctx, zwp_input_method_context_v1_destroy);

   free(wkb->preedit_str);
   wkb->preedit_str = strdup("");
   wkb->content_hint = ZWP_TEXT_INPUT_V1_CONTENT_HINT_NONE;
   wkb->content_purpose = ZWP_TEXT_INPUT_V1_CONTENT_PURPOSE_NORMAL;

   free(wkb->language);
   wkb->language = NULL;

   free(wkb->surrounding_text);
   wkb->surrounding_text = NULL;

   wkb->serial = 0;

   wkb->im_ctx = im_ctx;
   zwp_input_method_context_v1_add_listener(im_ctx, &wkb_im_context_listener, wkb);

   /* hard coded */
   zwp_input_method_context_v1_language(im_ctx, wkb->serial, "en");
   zwp_input_method_context_v1_text_direction(im_ctx, wkb->serial,
                                          ZWP_TEXT_INPUT_V1_TEXT_DIRECTION_LTR);

   wkb->context_changed = EINA_TRUE;
   evas_object_show(wkb->edje_obj);
}

static void
_wkb_im_deactivate(void *data, struct zwp_input_method_v1 *input_method EINA_UNUSED, struct zwp_input_method_context_v1 *im_ctx EINA_UNUSED)
{
   struct weekeyboard *wkb = data;

   DBG("Deactivate");

   E_FREE_FUNC(wkb->im_ctx, zwp_input_method_context_v1_destroy);

   if (wkb->edje_obj)
     evas_object_hide(wkb->edje_obj);
}

static const struct zwp_input_method_v1_listener wkb_im_listener = {
   _wkb_im_activate,
   _wkb_im_deactivate
};

static Eina_Bool
_wkb_setup(struct weekeyboard *wkb)
{
   Eina_Iterator *itr;
   Ecore_Wl2_Global *global;
   struct wl_registry *registry;
   struct zwp_input_panel_surface_v1 *ips;
   void *data;

   registry = e_comp_wl->wl.registry ?: ecore_wl2_display_registry_get(e_comp_wl->ewd);
   itr = ecore_wl2_display_globals_get(e_comp_wl->ewd);
   EINA_ITERATOR_FOREACH(itr, data)
     {
        global = (Ecore_Wl2_Global *)data;

        DBG("interface: <%s>", global->interface);
        if (eina_streq(global->interface, "zwp_input_panel_v1"))
          {
             wkb->ip =
               wl_registry_bind(registry, global->id,
                                &zwp_input_panel_v1_interface, 1);
             DBG("binding zwp_input_panel_v1");
          }
        else if (eina_streq(global->interface, "zwp_input_method_v1"))
          {
             wkb->im =
               wl_registry_bind(registry, global->id,
                                &zwp_input_method_v1_interface, 1);
             DBG("binding zwp_input_method_v1, id = %d", global->id);
          }
        else if (eina_streq(global->interface, "wl_output"))
          {
             wkb->output =
               wl_registry_bind(registry, global->id, &wl_output_interface, 1);
             DBG("binding wl_output");
          }
     }
   eina_iterator_free(itr);

   if ((!wkb->ip) || (!wkb->im) || (!wkb->output))
     return EINA_FALSE;

   /* invalidate the UI so it is drawn when invoked */
   wkb->theme = NULL;

   /* Set input panel surface */
   DBG("Setting up input panel");

   wkb->win = ecore_evas_wayland2_window_get(wkb->ee);
   ecore_wl2_window_type_set(wkb->win, ECORE_WL2_WINDOW_TYPE_NONE);

   wkb->surface = ecore_wl2_window_surface_get(wkb->win);
   ips = zwp_input_panel_v1_get_input_panel_surface(wkb->ip, wkb->surface);
   zwp_input_panel_surface_v1_set_toplevel(ips, wkb->output,
                                       ZWP_INPUT_PANEL_SURFACE_V1_POSITION_CENTER_BOTTOM);

   /* Input method listener */
   DBG("Adding zwp_input_method_v1 listener");
   zwp_input_method_v1_add_listener(wkb->im, &wkb_im_listener, wkb);

   wkb->edje_obj = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_wkb_check_evas_engine(struct weekeyboard *wkb)
{
   const char *env = getenv("ECORE_EVAS_ENGINE");

   if (!env)
     {
        if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_WAYLAND_SHM))
          env = "wayland_shm";
        else if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_WAYLAND_EGL))
          env = "wayland_egl";
        else
          {
             ERR("Ecore_Evas must be compiled with support for Wayland engines");
             return EINA_FALSE;
          }
     }
   else if ((!eina_streq(env, "wayland_shm")) &&
            (!eina_streq(env, "wayland_egl")))
     {
        ERR("ECORE_EVAS_ENGINE must be set to either 'wayland_shm' or 'wayland_egl'");
        return EINA_FALSE;
     }

   wkb->ee_engine = env;

   return EINA_TRUE;
}

static void
_wkb_free(struct weekeyboard *wkb)
{
   E_FREE_FUNC(wkb->im_ctx, zwp_input_method_context_v1_destroy);
   E_FREE_FUNC(wkb->edje_obj, evas_object_del);

   if (wkb->ignore_keys)
     {
        free(*wkb->ignore_keys);
        free(wkb->ignore_keys);
     }

   free(wkb->preedit_str);
   free(wkb->surrounding_text);
   free(wkb->theme);
   free(wkb);
}

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Wl_Weekeyboard" };

E_API void *
e_modapi_init(E_Module *m)
{
   struct weekeyboard *wkb;

   EINA_LOG_DBG("LOAD 'weekeyboard' module\n");

   wkb = E_NEW(struct weekeyboard, 1);
   if (!wkb)
     {
        EINA_LOG_ERR("out of memory");
        goto err;
     }

   if (!wkb_log_init("weekeyboard"))
     {
        EINA_LOG_ERR("failed to init log");
        goto log_err;
     }

   if (!_wkb_check_evas_engine(wkb))
     {
        ERR("Unable to find available ee engine");
        goto engine_err;
     }

   DBG("Selected engine: '%s'", wkb->ee_engine);
   wkb->ee = ecore_evas_new(wkb->ee_engine, 0, 0, 1, 1, "frame=0");
   if (!wkb->ee)
     {
        ERR("Unable to create Ecore_Evas object");
        goto engine_err;
     }

   if (!_wkb_setup(wkb))
     {
        ERR("Unable to setup weekeyboard");
        goto setup_err;
     }

   wkb->module = m;

   m->data = wkb;

   return m;

setup_err:
   ecore_evas_free(wkb->ee);

engine_err:
   wkb_log_shutdown();

log_err:
   free(wkb);

err:
   return NULL;
}

E_API int
e_modapi_shutdown(E_Module *m)
{
   struct weekeyboard *wkb = m->data;

   wkb_log_shutdown();

   _wkb_free(wkb);

   m->data = NULL;

   return 1;
}
