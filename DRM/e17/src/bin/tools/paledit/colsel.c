#include "main.h"

static void
_spinner_to_rect(Evas_Object *win)
{
   Elm_Object_Item *it;
   int r, g, b, a;

   elm_colorselector_color_get(evas_object_data_get(win, "pal_colorsel"), &r, &g, &b, &a);
   it = elm_genlist_selected_item_get(evas_object_data_get(win, "pal_class_list"));
   if (it)
     {
        Elm_Palette_Color *col = elm_object_item_data_get(it);
        Elm_Palette *pal = evas_object_data_get(win, "pal");
        col->r = r;
        col->g = g;
        col->b = b;
        col->a = a;
        elm_genlist_item_update(it);
        pal_save(win);
        palimg_update(evas_object_data_get(win, "pal_image"), pal);
     }
   evas_color_argb_premul(a, &r, &g, &b);
   evas_object_color_set(evas_object_data_get(win, "pal_color_rect"), r, g, b, a);
}

static void
_spinner_to_colorsel_mirror(Evas_Object *win)
{
   int r, g, b, a;

   r = elm_spinner_value_get(evas_object_data_get(win, "pal_spin_int_r"));
   g = elm_spinner_value_get(evas_object_data_get(win, "pal_spin_int_g"));
   b = elm_spinner_value_get(evas_object_data_get(win, "pal_spin_int_b"));
   a = elm_spinner_value_get(evas_object_data_get(win, "pal_spin_int_a"));
   elm_colorselector_color_set(evas_object_data_get(win, "pal_colorsel"), r, g, b, a);
}

static void
_spinner_to_entry_mirror(Evas_Object *win)
{
   int r, g, b, a;
   char buf[256];

   elm_colorselector_color_get(evas_object_data_get(win, "pal_colorsel"), &r, &g, &b, &a);
   snprintf(buf, sizeof(buf),
            "#<+ backing=on backing_color=#0004>"
            "<+ color=#f54>%02x</>"
            "<+ color=#5f4>%02x</>"
            "<+ color=#45f>%02x</>"
            "<+ color=#888>%02x</>"
            "</>"
            , r, g, b, a);
   elm_object_text_set(evas_object_data_get(win, "pal_color_entry"), buf);
}

static void
_spinner_to_spinner_mirror(Evas_Object *win)
{
   int r, g, b, a;
   int pr, pg, pb, pa;

   elm_colorselector_color_get(evas_object_data_get(win, "pal_colorsel"), &r, &g, &b, &a);
   pr = (r * 100) / 255;
   pg = (g * 100) / 255;
   pb = (b * 100) / 255;
   pa = (a * 100) / 255;
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_r"), r);
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_g"), g);
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_b"), b);
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_a"), a);
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_r"), pr);
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_g"), pg);
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_b"), pb);
   elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_a"), pa);
}

static void
_cb_spinner_perc_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   double val;
   int intval;

   val = elm_spinner_value_get(obj);
   intval = (val * 255.0) / 100.0;
   if (obj == evas_object_data_get(win, "pal_spin_perc_r"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_r"), intval);
   else if (obj == evas_object_data_get(win, "pal_spin_perc_g"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_g"), intval);
   else if (obj == evas_object_data_get(win, "pal_spin_perc_b"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_b"), intval);
   else if (obj == evas_object_data_get(win, "pal_spin_perc_a"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_int_a"), intval);
   _spinner_to_colorsel_mirror(win);
   _spinner_to_entry_mirror(win);
   _spinner_to_rect(win);
}

static void
_cb_spinner_int_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   double val;
   int intval;

   val = elm_spinner_value_get(obj);
   intval = (val * 100.0) / 255.0;
   if (obj == evas_object_data_get(win, "pal_spin_int_r"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_r"), intval);
   else if (obj == evas_object_data_get(win, "pal_spin_int_g"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_g"), intval);
   else if (obj == evas_object_data_get(win, "pal_spin_int_b"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_b"), intval);
   else if (obj == evas_object_data_get(win, "pal_spin_int_a"))
     elm_spinner_value_set(evas_object_data_get(win, "pal_spin_perc_a"), intval);
   _spinner_to_colorsel_mirror(win);
   _spinner_to_entry_mirror(win);
   _spinner_to_rect(win);
}

static void
_cb_colorsel_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;

   _spinner_to_spinner_mirror(win);
   _spinner_to_entry_mirror(win);
   _spinner_to_rect(win);
}

static void
_colsel_entry_handle(Evas_Object *win, Evas_Object *entry, Eina_Bool activated)
{
   const char *markup = elm_object_text_get(entry);

   if (markup)
     {
        char *plain = elm_entry_markup_to_utf8(markup);
        if (plain)
          {
             char *p = plain;
             int r, g, b, a;

             if (p[0] == '#') p++;
             if (strlen(p) == 8)
               {
                  if (sscanf(p, "%02x%02x%02x%02x", &r, &g, &b, &a) == 4)
                    {
                       elm_colorselector_color_set(evas_object_data_get(win, "pal_colorsel"), r, g, b, a);
                       goto ok;
                    }
               }
             else if (activated)
               {
                  if (strlen(p) == 6)
                    {
                       if (sscanf(p, "%02x%02x%02x", &r, &g, &b) == 3)
                         {
                            a = 255;
                            elm_colorselector_color_set(evas_object_data_get(win, "pal_colorsel"), r, g, b, a);
                            goto ok;
                         }
                    }
                  else if (strlen(p) == 4)
                    {
                       if (sscanf(p, "%01x%01x%01x%01x", &r, &g, &b, &a) == 4)
                         {
                            r = (r << 4) | r;
                            g = (g << 4) | g;
                            b = (b << 4) | b;
                            a = (a << 4) | a;
                            elm_colorselector_color_set(evas_object_data_get(win, "pal_colorsel"), r, g, b, a);
                            goto ok;
                         }
                    }
                  else if (strlen(p) == 3)
                    {
                       if (sscanf(p, "%01x%01x%01x", &r, &g, &b) == 3)
                         {
                            r = (r << 4) | r;
                            g = (g << 4) | g;
                            b = (b << 4) | b;
                            a = 255;
                            elm_colorselector_color_set(evas_object_data_get(win, "pal_colorsel"), r, g, b, a);
                            goto ok;
                         }
                    }
               }
             free(plain);
          }
     }
   return;
ok:
   colsel_update(win);
}

static void
_cb_color_entry_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;

   _colsel_entry_handle(win, obj, EINA_FALSE);
}

static void
_cb_color_entry_activated(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;

   _colsel_entry_handle(win, obj, EINA_TRUE);
}

static void
_colsel_row(Evas_Object *win, Evas_Object *tb,
            int row,
            int r, int g, int b, int a,
            const char *label, const char *name_percent, const char *name_int)
{
   Evas_Object *o, *sp;

   o = elm_label_add(win);
   elm_object_text_set(o, label);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, 0.0, EVAS_HINT_FILL);
   elm_table_pack(tb, o, 0, row, 1, 1);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(o, r, g, b, a);
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(15), ELM_SCALE_SIZE(15));
   elm_table_pack(tb, o, 1, row, 1, 1);
   evas_object_show(o);

   // invisible spinner for just spacing the 2 cells with real spinners for
   // holding max possible vbalue and then some to avoid resizing due to font
   // changes mwith some digits causing sizing to go up
   sp = o = elm_spinner_add(win);
   elm_spinner_editable_set(o, EINA_TRUE);
   elm_spinner_label_format_set(o, "%1.0f%%");
   elm_spinner_step_set(o, 1);
   elm_spinner_min_max_set(o, 0, 99999);
   elm_spinner_value_set(o, 99999);
   elm_table_pack(tb, o, 2, row, 1, 1);

   sp = o = elm_spinner_add(win);
   elm_spinner_editable_set(o, EINA_TRUE);
   elm_spinner_label_format_set(o, "%1.0f");
   elm_spinner_step_set(o, 1);
   elm_spinner_min_max_set(o, 0, 99999);
   elm_spinner_value_set(o, 99999);
   elm_table_pack(tb, o, 3, row, 1, 1);

   sp = o = elm_spinner_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_spinner_editable_set(o, EINA_TRUE);
   elm_spinner_label_format_set(o, "%1.0f%%");
   elm_spinner_step_set(o, 1);
   elm_spinner_min_max_set(sp, 0, 100);
   evas_object_smart_callback_add(o, "changed", _cb_spinner_perc_changed, win);
   elm_table_pack(tb, o, 2, row, 1, 1);
   evas_object_show(o);
   evas_object_data_set(win, name_percent, o);

   sp = o = elm_spinner_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_spinner_editable_set(o, EINA_TRUE);
   elm_spinner_label_format_set(o, "%1.0f");
   elm_spinner_step_set(o, 1);
   elm_spinner_min_max_set(sp, 0, 255);
   evas_object_smart_callback_add(o, "changed", _cb_spinner_int_changed, win);
   elm_table_pack(tb, o, 3, row, 1, 1);
   evas_object_show(o);
   evas_object_data_set(win, name_int, o);
}

void
colsel_update(Evas_Object *win)
{
   _spinner_to_spinner_mirror(win);
   _spinner_to_entry_mirror(win);
   _spinner_to_rect(win);
}

Evas_Object *
colsel_add(Evas_Object *win)
{
   Evas_Object *o, *bxr, *tb2, *fr2, *fr;

   bxr = o = elm_box_add(win);
   elm_box_padding_set(o, 0, ELM_SCALE_SIZE(10));
   elm_box_align_set(o, 0.5, 0.0);
   evas_object_size_hint_weight_set(o, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   fr = o = elm_frame_add(win);
   elm_object_style_set(o, "pad_small");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxr, o);
   evas_object_show(o);

   fr2 = o = elm_frame_add(win);
   elm_object_style_set(o, "outline");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr, o);
   evas_object_show(o);

   tb2 = o = elm_table_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(fr2, o);
   evas_object_show(o);

   o = elm_bg_add(win);
   elm_object_style_set(o, "checks_small");
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(80), ELM_SCALE_SIZE(60));
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb2, o, 0, 0, 1, 1);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(60), ELM_SCALE_SIZE(40));
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_color_set(o, 128, 64, 0, 128);
   elm_table_pack(tb2, o, 0, 0, 1, 1);
   evas_object_data_set(win, "pal_color_rect", o);
   evas_object_show(o);

   o = elm_colorselector_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "changed", _cb_colorsel_changed, win);
   elm_box_pack_end(bxr, o);
   evas_object_show(o);
   evas_object_data_set(win, "pal_colorsel", o);

   o = elm_entry_add(win);
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "changed,user", _cb_color_entry_changed, win);
   evas_object_smart_callback_add(o, "activated", _cb_color_entry_activated, win);
   elm_box_pack_end(bxr, o);
   evas_object_show(o);
   evas_object_data_set(win, "pal_color_entry", o);

   tb2 = o = elm_table_add(win);
   elm_table_padding_set(o, ELM_SCALE_SIZE(10), ELM_SCALE_SIZE(10));
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxr, o);
   evas_object_show(o);

   _colsel_row(win, tb2, 0, 255,   0,   0, 255, "Red",   "pal_spin_perc_r", "pal_spin_int_r");
   _colsel_row(win, tb2, 1,   0, 255,   0, 255, "Green", "pal_spin_perc_g", "pal_spin_int_g");
   _colsel_row(win, tb2, 2,   0,   0, 255, 255, "Blue",  "pal_spin_perc_b", "pal_spin_int_b");
   _colsel_row(win, tb2, 3,  64,  64,  64, 128, "Alpha", "pal_spin_perc_a", "pal_spin_int_a");

   return bxr;
}
