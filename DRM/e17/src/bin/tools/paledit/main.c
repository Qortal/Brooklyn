#include "main.h"

void
pal_save(Evas_Object *win)
{
   char *palname = evas_object_data_get(win, "pal_selected_palette");
   if (!palname) return;
   Elm_Palette *pal = evas_object_data_get(win, "pal");
   elm_config_palette_save(pal, palname);
   elm_config_palette_set(palname);
   elm_config_all_flush();
   elm_config_reload();
}

void
pal_load(Evas_Object *win)
{
   char *palname = evas_object_data_get(win, "pal_selected_palette");
   if (!palname) return;
   Elm_Palette *pal = elm_config_palette_load(palname);
   if (!pal) return;
   evas_object_data_set(win, "pal", pal);
   palcols_fill(win);
   palimg_update(evas_object_data_get(win, "pal_image"), pal);
   elm_object_text_set(evas_object_data_get(win, "pal_name"), palname);
   pal_save(win);
}

static void
_cb_close_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *o, *win, *bx, *bxh, *fr, *tb;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("e_paledit", "Palette Editor");
   elm_win_autodel_set(win, EINA_TRUE);

   fr = o = elm_frame_add(win);
   elm_object_style_set(o, "pad_medium");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);

   bx = o = elm_box_add(win);
   elm_box_padding_set(o, 0, ELM_SCALE_SIZE(10));
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, o);
   evas_object_show(o);

   bxh = o = elm_box_add(win);
   elm_box_padding_set(o, ELM_SCALE_SIZE(10), 0);
   elm_box_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = palcols_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxh, o);
   evas_object_show(o);

   o = elm_separator_add(win);
   evas_object_size_hint_weight_set(o, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, 0.5, EVAS_HINT_FILL);
   elm_box_pack_end(bxh, o);
   evas_object_show(o);

   o = colsel_add(win);
   evas_object_size_hint_weight_set(o, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxh, o);
   evas_object_show(o);

   tb = o = elm_table_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, 1.0, EVAS_HINT_FILL);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(120), 0);
   elm_table_pack(tb, o, 0, 0, 1, 1);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_close_click, win);
   elm_object_text_set(o, "Close");
   elm_table_pack(tb, o, 0, 0, 1, 1);
   evas_object_show(o);

   colsel_update(win);

   if (argc > 1)
     {
        evas_object_data_set(win, "pal_selected_palette", strdup(argv[1]));
     }
   else
     {
        const char *palconf = elm_config_palette_get();
        if (!palconf) palconf = "default";
        evas_object_data_set(win, "pal_selected_palette", strdup(palconf));
     }
   pal_load(win);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
