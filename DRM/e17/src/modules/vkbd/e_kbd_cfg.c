#include "e.h"
#include "e_mod_main.h"
#include "e_kbd_cfg.h"
#include "e_kbd_buf.h"

static E_Kbd_Int *cfg_kbd = NULL;
static Eina_List *dicts = NULL;
static Evas_Object *win = NULL;

static void
_cb_close(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_kbd_cfg_hide(cfg_kbd);
}

static void
_cb_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char *s;

   EINA_LIST_FREE(dicts, s) eina_stringshare_del(s);
   win = NULL;
   cfg_kbd = NULL;
}

static void
_cb_size(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   double v = elm_slider_value_get(obj);
   il_kbd_cfg->size = v;
   e_kbd_int_update(cfg_kbd);
   e_config_save_queue();
}

static void
_cb_fill_mode(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   il_kbd_cfg->fill_mode = (int)(uintptr_t)data;
   if      (il_kbd_cfg->fill_mode == 0) elm_object_text_set(obj, _("Shrink"));
   else if (il_kbd_cfg->fill_mode == 1) elm_object_text_set(obj, _("Stretch"));
   else if (il_kbd_cfg->fill_mode == 2) elm_object_text_set(obj, _("Fill"));
   else if (il_kbd_cfg->fill_mode == 3) elm_object_text_set(obj, _("Float"));
   e_kbd_int_update(cfg_kbd);
   e_config_save_queue();
}

static void
_cb_dict(void *data, Evas_Object *li EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char *s = data;
   eina_stringshare_replace(&il_kbd_cfg->dict, s);
   e_kbd_buf_dict_set(cfg_kbd->kbuf, il_kbd_cfg->dict);
   e_kbd_buf_clear(cfg_kbd->kbuf);
   e_config_save_queue();
}

EAPI void
e_kbd_cfg_show(E_Kbd_Int *ki)
{
   Evas_Object *o, *box, *fr, *li, *tb;
   Elm_Object_Item *it;
   Eina_List *files;
   char buf[PATH_MAX], *p, *f;
   const char *p2;

   if (cfg_kbd) return;
   cfg_kbd = ki;
   win = e_elm_win_util_dialog_add(NULL, "vkbd_config",
                                   _("Virtual Keyboard Settings"));
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _cb_del, NULL);
   elm_win_autodel_set(win, EINA_TRUE);

   o = fr = elm_frame_add(win);
   E_EXPAND(o);
   E_FILL(o);
   elm_object_style_set(o, "pad_large");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);

   box = o = elm_box_add(win);
   E_EXPAND(o);
   E_FILL(o);
   elm_object_content_set(fr, o);
   evas_object_show(o);

   o = fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_text_set(o, NULL, _("Layout Mode"));
   elm_box_pack_end(box, o);
   evas_object_show(o);

   o = elm_hoversel_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_hoversel_auto_update_set(o, EINA_TRUE);
   elm_hoversel_hover_parent_set(o, win);
   if      (il_kbd_cfg->fill_mode == 0) elm_object_text_set(o, _("Shrink"));
   else if (il_kbd_cfg->fill_mode == 1) elm_object_text_set(o, _("Stretch"));
   else if (il_kbd_cfg->fill_mode == 2) elm_object_text_set(o, _("Fill"));
   else if (il_kbd_cfg->fill_mode == 3) elm_object_text_set(o, _("Float"));
   elm_hoversel_item_add(o, _("Shrink"),  NULL, ELM_ICON_NONE, _cb_fill_mode, (void *)(uintptr_t)0);
   elm_hoversel_item_add(o, _("Stretch"), NULL, ELM_ICON_NONE, _cb_fill_mode, (void *)(uintptr_t)1);
   elm_hoversel_item_add(o, _("Fill"),    NULL, ELM_ICON_NONE, _cb_fill_mode, (void *)(uintptr_t)2);
   elm_hoversel_item_add(o, _("Float"),   NULL, ELM_ICON_NONE, _cb_fill_mode, (void *)(uintptr_t)3);
   elm_object_content_set(fr, o);
   evas_object_show(o);

   o = fr = elm_frame_add(win);
   E_EXPAND(o);
   E_FILL(o);
   elm_layout_text_set(o, NULL, _("Dictionary"));
   elm_box_pack_end(box, o);
   evas_object_show(o);

   o = tb = elm_table_add(win);
   E_EXPAND(o);
   E_FILL(o);
   elm_object_content_set(fr, o);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   E_EXPAND(o);
   E_FILL(o);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_size_hint_min_set(o,
                                 elm_config_scale_get() * 80,
                                 elm_config_scale_get() * 80);
   elm_table_pack(tb, o, 0, 0, 1, 1);

   o = li = elm_list_add(win);
   E_EXPAND(o);
   E_FILL(o);
   snprintf(buf, sizeof(buf), "%s/dicts", ki->syskbds);
   files = ecore_file_ls(buf);
   EINA_LIST_FREE(files, f)
     {
        strncpy(buf, f, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = 0;
        p = strrchr(buf, '.');
        if ((p) && (!strcasecmp(p, ".dic")))
          {
             p2 = eina_stringshare_add(buf);
             dicts = eina_list_append(dicts, p2);
             *p = 0;
             it = elm_list_item_append(li, buf, NULL, NULL, _cb_dict, p2);
             if (!strcasecmp(p2, il_kbd_cfg->dict))
               elm_list_item_selected_set(it, EINA_TRUE);
          }
        free(f);
     }

   elm_list_go(o);
   elm_table_pack(tb, o, 0, 0, 1, 1);
   evas_object_show(o);

   o = fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_text_set(o, NULL, _("Sizing"));
   elm_box_pack_end(box, o);
   evas_object_show(o);

   o = elm_slider_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_slider_unit_format_set(o, "%1.1f");
   elm_slider_step_set(o, 0.2);
   elm_slider_span_size_set(o, 240);
   elm_slider_min_max_set(o, 1.0, 9.0);
   elm_slider_value_set(o, il_kbd_cfg->size);
   evas_object_smart_callback_add(o, "changed", _cb_size, NULL);
   elm_object_content_set(fr, o);
   evas_object_show(o);

   o = elm_separator_add(win);
   elm_separator_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, o);
   evas_object_show(o);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, _("Close"));
   evas_object_smart_callback_add(o, "clicked", _cb_close, NULL);
   elm_box_pack_end(box, o);
   evas_object_show(o);

   evas_object_show(win);
   evas_smart_objects_calculate(evas_object_evas_get(win));
   elm_win_center(win, 1, 1);
}

EAPI void
e_kbd_cfg_hide(E_Kbd_Int *ki)
{
   if (!ki) return;
   if (cfg_kbd != ki) return;
   if (win) evas_object_del(win);
   cfg_kbd = NULL;
}
