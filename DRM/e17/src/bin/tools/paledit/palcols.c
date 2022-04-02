#include "main.h"

static void _pal_color_add(Evas_Object *win);
static void _pal_color_del(Evas_Object *win);

static char *
_cb_class_gl_label_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   Elm_Palette_Color *col = data;

   if (col->name) return strdup(col->name);
   else return strdup("");
}

static Evas_Object *
_cb_class_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   Elm_Palette_Color *col = data;
   Evas_Object *o, *tb;
   int r, g, b, a;

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;

   tb = o = elm_table_add(obj);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   o = elm_bg_add(obj);
   elm_object_style_set(o, "checks_small");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, o, 0, 0, 1, 1);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas_object_evas_get(obj));
   r = col->r;
   g = col->g;
   b = col->b;
   a = col->a;
   evas_color_argb_premul(a, &r, &g, &b);
   evas_object_color_set(o, r, g, b, a);
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(30), ELM_SCALE_SIZE(15));
   elm_table_pack(tb, o, 0, 0, 1, 1);
   evas_object_show(o);

   return tb;
}

static void
_cb_class_gl_sel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Object *win = data;
   Elm_Object_Item *it = event_info;
   Elm_Palette_Color *col = elm_object_item_data_get(it);

   elm_colorselector_color_set(evas_object_data_get(win, "pal_colorsel"),
                               col->r, col->g, col->b, col->a);
   colsel_update(win);
   elm_object_text_set(evas_object_data_get(win, "pal_class_entry"), col->name);
}

static int
_cb_class_insert_cmp(const void *pa, const void *pb)
{
   Elm_Palette_Color *col1 = elm_object_item_data_get(pa);
   Elm_Palette_Color *col2 = elm_object_item_data_get(pb);
   const char *n1, *n2;

   if (!col1) return -1;
   if (!col2) return 1;
   n1 = col1->name;
   if (!n1) n1 = "";
   n2 = col2->name;
   if (!n2) n2 = "";
   return strcmp(n1, n2);
}

static void
_cb_select_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *o = palsel_add(win);
   evas_object_show(o);
}

static void
_cb_add_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _pal_color_add(data);
}

static void
_cb_del_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _pal_color_del(data);
}

static void
_pal_color_add(Evas_Object *win)
{
   const char *markup = elm_object_text_get(evas_object_data_get(win, "pal_class_entry"));

   if (markup)
     {
        char *plain = elm_entry_markup_to_utf8(markup);
        if (plain)
          {
             Elm_Genlist_Item_Class *itc = evas_object_data_get(win, "pal_class_itc");

             if (itc)
               {
                  Elm_Palette *pal = evas_object_data_get(win, "pal");
                  Evas_Object *list = evas_object_data_get(win, "pal_class_list");
                  Eina_List *l;
                  Elm_Palette_Color *col;
                  int r, g, b, a;

                  EINA_LIST_FOREACH(pal->colors, l, col)
                    {
                       if (!col->name) continue;
                       if (!strcmp(col->name, plain)) goto exists;
                    }
                  elm_colorselector_color_get(evas_object_data_get(win, "pal_colorsel"), &r, &g, &b, &a);
                  elm_config_palette_color_set(pal, plain, r, g, b, a);
                  EINA_LIST_FOREACH(pal->colors, l, col)
                    {
                       if (!col->name) continue;
                       if (!strcmp(col->name, plain))
                         {
                            Elm_Object_Item *it;

                            it = elm_genlist_item_sorted_insert(list, itc, col,
                                                                NULL, ELM_GENLIST_ITEM_NONE,
                                                                _cb_class_insert_cmp,
                                                                _cb_class_gl_sel, win);
                            elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
                            elm_genlist_item_selected_set(it, EINA_TRUE);
                            break;
                         }
                    }
                  palimg_update(evas_object_data_get(win, "pal_image"), pal);
               }
exists:
             free(plain);
          }
     }
   pal_save(win);
}

static void
_pal_color_del(Evas_Object *win)
{
   Elm_Object_Item *it, *it_sel;

   it = elm_genlist_selected_item_get(evas_object_data_get(win, "pal_class_list"));
   if (it)
     {
        Elm_Palette *pal = evas_object_data_get(win, "pal");
        Elm_Palette_Color *col = elm_object_item_data_get(it);
        if (col->name) elm_config_palette_color_unset(pal, col->name);
        it_sel = elm_genlist_item_next_get(it);
        if (!it_sel) it_sel = elm_genlist_item_prev_get(it);
        elm_object_item_del(it);
        if (it_sel)
          {
             elm_genlist_item_show(it_sel, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
             elm_genlist_item_selected_set(it_sel, EINA_TRUE);
          }
        palimg_update(evas_object_data_get(win, "pal_image"), pal);
     }
   pal_save(win);
}

void
palcols_fill(Evas_Object *win)
{
   Elm_Object_Item *sel_it = NULL;
   Elm_Palette *pal = evas_object_data_get(win, "pal");
   Elm_Palette_Color *col;
   Evas_Object *list = evas_object_data_get(win, "pal_class_list");
   Eina_List *l;
   Elm_Genlist_Item_Class *itc = evas_object_data_get(win, "pal_class_itc");
   Elm_Object_Item *it;

   elm_genlist_clear(list);

   if (!itc)
     {
        itc = elm_genlist_item_class_new();
        evas_object_data_set(win, "pal_class_itc", itc);

        itc->item_style = "default";
        itc->func.text_get = _cb_class_gl_label_get;
        itc->func.content_get = _cb_class_gl_content_get;
     }

   EINA_LIST_FOREACH(pal->colors, l, col)
     {
        it = elm_genlist_item_sorted_insert(list, itc, col,
                                            NULL, ELM_GENLIST_ITEM_NONE,
                                            _cb_class_insert_cmp,
                                            _cb_class_gl_sel, win);
        if (!sel_it) sel_it = it;
     }
   if (sel_it)
     {
        elm_genlist_item_show(sel_it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
        elm_genlist_item_selected_set(sel_it, EINA_TRUE);
     }
}

Evas_Object *
palcols_add(Evas_Object *win)
{
   Evas_Object *o, *bxl, *btn, *bxh2;

   bxl = o = elm_box_add(win);
   elm_box_padding_set(o, 0, ELM_SCALE_SIZE(10));
   elm_box_align_set(o, 0.5, 0.0);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(o);

   btn = o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_select_click, win);
   elm_object_text_set(o, "Select palette");
   elm_box_pack_end(bxl, o);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(240), 0);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bxl, o);

   bxh2 = o = elm_box_add(win);
   elm_box_padding_set(o, ELM_SCALE_SIZE(10), 0);
   elm_box_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxl, o);
   evas_object_show(o);

   o = elm_entry_add(win);
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   elm_entry_editable_set(o, EINA_FALSE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxh2, o);
   evas_object_show(o);
   evas_object_data_set(win, "pal_name", o);

   o = palimg_add(win);
   elm_box_pack_end(bxh2, o);
   evas_object_show(o);
   evas_object_data_set(win, "pal_image", o);

   o = elm_genlist_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxl, o);
   evas_object_show(o);
   evas_object_data_set(win, "pal_class_list", o);

   bxh2 = o = elm_box_add(win);
   elm_box_padding_set(o, ELM_SCALE_SIZE(10), 0);
   elm_box_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxl, o);
   evas_object_show(o);

   o = elm_entry_add(win);
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxh2, o);
   evas_object_show(o);
   evas_object_data_set(win, "pal_class_entry", o);

   btn = o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_add_click, win);
   elm_box_pack_end(bxh2, o);
   evas_object_show(o);

   o = elm_icon_add(win);
   elm_icon_standard_set(o, "add");
   elm_object_content_set(btn, o);
   evas_object_show(o);

   btn = o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_del_click, win);
   elm_box_pack_end(bxh2, o);
   evas_object_show(o);

   o = elm_icon_add(win);
   elm_icon_standard_set(o, "sub");
   elm_object_content_set(btn, o);
   evas_object_show(o);

   return bxl;
}
