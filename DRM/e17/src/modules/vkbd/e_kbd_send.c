#include "e.h"
#include "e_kbd_send.h"

#ifdef HAVE_WAYLAND
#include "input-method-unstable-v1-client-protocol.h"
#include "text-input-unstable-v1-client-protocol.h"

/*
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
   char *new_text = malloc(strlen(text) + strlen(insert) + 1);
   uint32_t text_len = 0;

   if (!new_text) return NULL;

   if ((!text) || (!insert))
     {
        free(new_text);
        return NULL;
     }

   text_len = strlen(text);
   if (offset > text_len) offset = text_len;

   new_text = malloc(text_len + strlen(insert) + 1);
   if (!new_text) return NULL;

   strncpy(new_text, text, offset);
   new_text[offset] = '\0';
   strcat(new_text, insert);
   strcat(new_text, text + offset);

   return new_text;
}

static void
_wkb_commit_preedit_str(struct weekeyboard *wkb)
{
   char *surrounding_text;

   if ((!wkb->preedit_str) || (strlen(wkb->preedit_str) == 0)) return;

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
   if (cursor > 0) index = cursor;
   zwp_input_method_context_v1_preedit_cursor(wkb->im_ctx, index);
   zwp_input_method_context_v1_preedit_string(wkb->im_ctx, wkb->serial,
                                              wkb->preedit_str,
                                              wkb->preedit_str);
}

static void
_wkb_update_preedit_str(struct weekeyboard *wkb, const char *key)
{
   char *tmp;

   if (!wkb->preedit_str) wkb->preedit_str = strdup("");

   tmp = _wkb_insert_text(wkb->preedit_str, strlen(wkb->preedit_str), key);
   free(wkb->preedit_str);
   wkb->preedit_str = tmp;

   if (eina_streq(key, " ")) _wkb_commit_preedit_str(wkb);
   else _wkb_send_preedit_str(wkb, -1);
}

static Eina_Bool
_wkb_ignore_key(struct weekeyboard *wkb, const char *key)
{
   int i;

   if (!wkb->ignore_keys) return EINA_FALSE;

   for (i = 0; wkb->ignore_keys[i] != NULL; i++)
     {
        if (eina_streq(key, wkb->ignore_keys[i])) return EINA_TRUE;
     }

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
   if (!src) return NULL;

   key = strtok(src, ":");
   key = strtok(NULL, ":");
   if (key == NULL) key = ":";

   if (_wkb_ignore_key(wkb, key)) goto end;
   else if (eina_streq(key, "backspace"))
     {
        if (strlen(wkb->preedit_str) == 0)
          {
             zwp_input_method_context_v1_delete_surrounding_text(wkb->im_ctx,
                                                                 -1, 1);
             zwp_input_method_context_v1_commit_string(wkb->im_ctx,
                                                       wkb->serial, "");
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
   else if (eina_streq(key, "space")) key = " ";

   _wkb_update_preedit_str(wkb, key);
end:
   free(src);
}

static void
_wkb_im_ctx_surrounding_text(void *data, struct zwp_input_method_context_v1 *im_ctx, const char *text, uint32_t cursor, uint32_t anchor)
{
   struct weekeyboard *wkb = data;

   EINA_SAFETY_ON_NULL_RETURN(text);
   free(wkb->surrounding_text);
   wkb->surrounding_text = strdup(text);
   if (!wkb->surrounding_text) return;
   wkb->surrounding_cursor = cursor;
}

static void
_wkb_im_ctx_reset(void *data, struct zwp_input_method_context_v1 *im_ctx)
{
   struct weekeyboard *wkb = data;

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

   if (!wkb->context_changed) return;

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

   if (button != BTN_LEFT) return;
   _wkb_send_preedit_str(wkb, index);
}

static void
_wkb_im_ctx_commit_state(void *data, struct zwp_input_method_context_v1 *im_ctx, uint32_t serial)
{
   struct weekeyboard *wkb = data;

   wkb->serial = serial;
   zwp_input_method_context_v1_language(im_ctx, wkb->serial, "en");
   zwp_input_method_context_v1_text_direction(im_ctx, wkb->serial,
                                              ZWP_TEXT_INPUT_V1_TEXT_DIRECTION_LTR);
}

static void
_wkb_im_ctx_preferred_language(void *data, struct zwp_input_method_context_v1 *im_ctx, const char *language)
{
   struct weekeyboard *wkb = data;

   if ((language) && (wkb->language) && (eina_streq(language, wkb->language))) return;
     E_FREE_FUNC(wkb->language, free);

   if (language) wkb->language = strdup(language);
}

static const struct zwp_input_method_context_v1_listener
wkb_im_context_listener =
{
   _wkb_im_ctx_surrounding_text,
   _wkb_im_ctx_reset,
   _wkb_im_ctx_content_type,
   _wkb_im_ctx_invoke_action,
   _wkb_im_ctx_commit_state,
   _wkb_im_ctx_preferred_language,
};
*/
#endif

#ifndef HAVE_WAYLAND_ONLY
static const char *
_string_to_keysym(const char *str)
{
   if (e_comp_util_has_x())
     {
        int glyph;

        /* utf8 -> glyph id (unicode - ucs4) */
        glyph = 0;
        evas_string_char_next_get(str, 0, &glyph);
        if (glyph <= 0) return NULL;
        /* glyph id -> keysym */
        if (glyph > 0xff) glyph |= 0x1000000;
        return ecore_x_keysym_string_get(glyph);
     }
   else
     {
        return NULL;
     }
}
#endif

EAPI void
e_kbd_send_string_press(const char *str EINA_UNUSED, Kbd_Mod mod EINA_UNUSED)
{
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        const char *key = NULL;

        key = _string_to_keysym(str);
        if (!key) return;
        e_kbd_send_keysym_press(key, mod);
     }
#endif
}

EAPI void
e_kbd_send_keysym_press(const char *key EINA_UNUSED, Kbd_Mod mod EINA_UNUSED)
{
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        if (mod & KBD_MOD_CTRL) ecore_x_test_fake_key_down("Control_L");
        if (mod & KBD_MOD_ALT)  ecore_x_test_fake_key_down("Alt_L");
        if (mod & KBD_MOD_WIN)  ecore_x_test_fake_key_down("Super_L");
        ecore_x_test_fake_key_press(key);
        if (mod & KBD_MOD_WIN)  ecore_x_test_fake_key_up("Super_L");
        if (mod & KBD_MOD_ALT)  ecore_x_test_fake_key_up("Alt_L");
        if (mod & KBD_MOD_CTRL) ecore_x_test_fake_key_up("Control_L");
     }
   else
     {
/*
        struct wl_resource *res;
        uint32_t serial, *end, *k, keycode, timestamp;
        xkb_keysym_t keysym;
        Eina_List *l;

        timestamp = ecore_loop_time_get() * 1000.0;
        keysym = xkb_keysym_from_name(key, XKB_KEYSYM_NO_FLAGS);
        keycode = xkb_keymap_key_by_name(keymap, key);
        serial = wl_display_next_serial(e_comp_wl->wl.disp);
        EINA_LIST_FOREACH(e_comp_wl->kbd.focused, l, res)
          {
             //keycode = KEYCOADE_ctrl
             //e_comp_wl_input_keyboard_state_update(keycode, EINA_TRUE);
             //if (mod & KBD_MOD_CTRL) ecore_x_test_fake_key_down("Control_L");
             //if (mod & KBD_MOD_ALT)  ecore_x_test_fake_key_down("Alt_L");
             //if (mod & KBD_MOD_WIN)  ecore_x_test_fake_key_down("Super_L");
             wl_keyboard_send_key(res, serial, timestamp,
                                  keycode, WL_KEYBOARD_KEY_STATE_PRESSED);
          }
        serial = wl_display_next_serial(e_comp_wl->wl.disp);
        EINA_LIST_FOREACH(e_comp_wl->kbd.focused, l, res)
          {
             wl_keyboard_send_key(res, serial, timestamp,
                                  keycode, WL_KEYBOARD_KEY_STATE_RELEASED);
             //if (mod & KBD_MOD_WIN)  ecore_x_test_fake_key_up("Super_L");
             //if (mod & KBD_MOD_ALT)  ecore_x_test_fake_key_up("Alt_L");
             //if (mod & KBD_MOD_CTRL) ecore_x_test_fake_key_up("Control_L");
          }
 */
     }
#endif
}

EAPI void
e_kbd_send_init(void)
{
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        return;
     }
#endif
#ifdef HAVE_WAYLAND
   if (!e_comp_util_has_x())
     {
/*
        void *data;
        struct wl_registry *registry =
          e_comp_wl->wl.registry ? :
          ecore_wl2_display_registry_get(e_comp_wl->ewd);

        itr = ecore_wl2_display_globals_get(e_comp_wl->ewd);
        EINA_ITERATOR_FOREACH(itr, data)
          {
             Ecore_Wl2_Global *global = data;

             if (eina_streq(global->interface, "zwp_input_panel_v1"))
               {
                  wkb->ip =
                    wl_registry_bind(registry, global->id,
                                     &zwp_input_panel_v1_interface, 1);
               }
             else if (eina_streq(global->interface, "zwp_input_method_v1"))
               {
                  wkb->im =
                    wl_registry_bind(registry, global->id,
                                     &zwp_input_method_v1_interface, 1);
               }
             else if (eina_streq(global->interface, "wl_output"))
               {
                  wkb->output =
                    wl_registry_bind(registry, global->id,
                                     &wl_output_interface, 1);
               }
          }
        eina_iterator_free(itr);
        if ((!wkb->ip) || (!wkb->im) || (!wkb->output)) return;

        zwp_input_method_v1_add_listener(wkb->im, &wkb_im_listener, wkb);
 */
     }
#endif
}


EAPI void
e_kbd_send_shutdown(void)
{
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        return;
     }
#endif
#ifdef HAVE_WAYLAND
   if (!e_comp_util_has_x())
     {
/*
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
 */
     }
#endif
}
