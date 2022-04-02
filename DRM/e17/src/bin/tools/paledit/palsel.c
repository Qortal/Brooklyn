#include "main.h"

static void
_name_dismiss(Evas_Object *win)
{
   evas_object_del(evas_object_data_get(win, "name_entry"));
   evas_object_del(evas_object_data_get(win, "name_ok"));
   evas_object_del(evas_object_data_get(win, "name_cancel"));
}

static void
_cb_name_ok_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   const char *markup = elm_object_text_get(evas_object_data_get(data, "name_entry"));

   if (markup)
     {
        char *plain = elm_entry_markup_to_utf8(markup);
        if (plain)
          {
             const char *op = evas_object_data_get(data, "name_op");

             if (op)
               {
                  if (!strcmp(op, "new"))
                    {
                       Elm_Palette *pal = elm_config_palette_load(plain);
                       if (pal)
                         {
                            free(evas_object_data_get(data, "pal_selected_palette"));
                            evas_object_data_set(data, "pal_selected_palette",
                                                 strdup(plain));
                            elm_config_palette_save(pal, plain);
                            elm_config_palette_free(pal);
                         }
                    }
                  else if (!strcmp(op, "copy"))
                    {
                       char *palname = evas_object_data_get(data, "pal_selected_palette");
                       if (palname)
                         {
                            Elm_Palette *pal = elm_config_palette_load(palname);
                            if (pal)
                              {
                                 free(evas_object_data_get(data, "pal_selected_palette"));
                                 evas_object_data_set(data, "pal_selected_palette",
                                                      strdup(plain));
                                 elm_config_palette_save(pal, plain);
                                 elm_config_palette_free(pal);
                              }
                         }
                    }
                  pal_load(data);
                  elm_popup_dismiss(evas_object_data_get(data, "pal_popup"));
               }
             free(plain);
          }
     }
   _name_dismiss(data);
}

static void
_cb_name_activate(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _cb_name_ok_click(data, NULL, NULL);
}

static void
_cb_name_cancel_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _name_dismiss(data);
}

static void
_palsel_name_request(Evas_Object *win, const char *name, const char *op)
{
   Evas_Object *o, *tb;

   tb = evas_object_data_get(win, "pal_popup_table");
   evas_object_data_set(win, "name_op", op);

   o = elm_entry_add(win);
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_entry_scrollable_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, name);
   evas_object_smart_callback_add(o, "activated", _cb_name_activate, win);
   elm_table_pack(tb, o, 0, 1, 2, 1);
   evas_object_show(o);
   evas_object_data_set(win, "name_entry", o);
   elm_entry_select_all(o);
   elm_object_focus_set(o, EINA_TRUE);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "OK");
   evas_object_smart_callback_add(o, "clicked", _cb_name_ok_click, win);
   elm_table_pack(tb, o, 0, 2, 1, 1);
   evas_object_show(o);
   evas_object_data_set(win, "name_ok", o);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "Cancel");
   evas_object_smart_callback_add(o, "clicked", _cb_name_cancel_click, win);
   elm_table_pack(tb, o, 1, 2, 1, 1);
   evas_object_show(o);
   evas_object_data_set(win, "name_cancel", o);
}

static void
_cb_select_new_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _palsel_name_request(data, "new", "new");
}

static void
_cb_select_del_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char *palname = evas_object_data_get(data, "pal_selected_palette");
   if (palname)
     {
        Elm_Object_Item *it_sel, *it = NULL;

        it_sel = elm_genlist_selected_item_get(evas_object_data_get(data, "pal_popup_list"));
        if (it_sel)
          {
             it = elm_genlist_item_next_get(it_sel);
             if (!it) it = elm_genlist_item_prev_get(it_sel);
          }
        elm_object_item_del(it_sel);
        elm_config_palette_delete(palname);
        evas_object_data_del(data, "pal_selected_palette");
        free(palname);
        if (it)
          {
             elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
             elm_genlist_item_selected_set(it, EINA_TRUE);
          }
     }
}

static void
_cb_select_copy_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _palsel_name_request(data, "copy", "copy");
}

static void
_cb_select_select_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   pal_load(data);
   elm_popup_dismiss(evas_object_data_get(data, "pal_popup"));
}

static void
_cb_select_cancel_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_popup_dismiss(evas_object_data_get(data, "pal_popup"));
}

static void
_cb_select_dismissed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(evas_object_data_get(data, "pal_popup"));
   evas_object_data_del(data, "pal_popup");
}

static char *
_cb_palette_gl_label_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup(data);
}

static Evas_Object *
_cb_palette_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   Elm_Palette *pal;
   Evas_Object *o;

   if (!!strcmp(part, "elm.swallow.icon")) return NULL;
   pal = elm_config_palette_load(data);
   if (!pal) return NULL;
   o = palimg_add(obj);
   palimg_update(o, pal);
   elm_config_palette_free(pal);
   return o;
}

static void
_cb_palette_gl_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   free(data);
}

static void
_cb_palette_gl_sel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Object *win = data;
   Elm_Object_Item *it = event_info;

   free(evas_object_data_get(win, "pal_selected_palette"));
   evas_object_data_set(win, "pal_selected_palette",
                        strdup(elm_object_item_data_get(it)));
}

static void
_palette_list_fill(Evas_Object *win, Evas_Object *list)
{
   Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
   Eina_List *palettes, *l;
   const char *pal, *palconf;
   Elm_Object_Item *sel_it = NULL;

   itc->item_style = "default";
   itc->func.text_get = _cb_palette_gl_label_get;
   itc->func.content_get = _cb_palette_gl_content_get;
   itc->func.del = _cb_palette_gl_del;

   palconf = elm_config_palette_get();
   palettes = elm_config_palette_list();
   EINA_LIST_FOREACH(palettes, l, pal)
     {
        char *dat = strdup(pal);
        Elm_Object_Item *it;

        it = elm_genlist_item_append(list, itc, dat,
                                     NULL, ELM_GENLIST_ITEM_NONE,
                                     _cb_palette_gl_sel, win);
        if ((palconf) && (!strcmp(pal, palconf)))
          {
             sel_it = it;
          }
     }
   elm_config_palette_list_free(palettes);
   elm_genlist_item_class_free(itc);
   if (sel_it)
     {
        elm_genlist_item_show(sel_it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
        elm_genlist_item_selected_set(sel_it, EINA_TRUE);
     }
}

Evas_Object *
palsel_add(Evas_Object *win)
{
   Evas_Object *o, *pop, *tb, *li, *bx;

   pop = o = elm_popup_add(win);
   elm_popup_scrollable_set(o, EINA_FALSE);
   elm_object_part_text_set(o, "title,text", "Select a palette");
   evas_object_smart_callback_add(o, "dismissed", _cb_select_dismissed, win);
   evas_object_data_set(win, "pal_popup", o);

   tb = o = elm_table_add(win);
   elm_table_padding_set(o, ELM_SCALE_SIZE(10), ELM_SCALE_SIZE(10));
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(pop, o);
   evas_object_show(o);
   evas_object_data_set(win, "pal_popup_table", o);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(220), ELM_SCALE_SIZE(300));
   elm_table_pack(tb, o, 0, 0, 2, 1);

   li = o = elm_genlist_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "activated", _cb_select_select_click, win);
   elm_table_pack(tb, o, 0, 0, 2, 1);
   evas_object_show(o);
   evas_object_data_set(win, "pal_popup_list", o);

   _palette_list_fill(win, li);

   bx = o = elm_box_add(win);
   elm_box_homogeneous_set(o, EINA_TRUE);
   elm_box_padding_set(o, ELM_SCALE_SIZE(5), 0);
   elm_box_horizontal_set(o, EINA_TRUE);
   elm_object_part_content_set(pop, "button1", o);
   evas_object_show(o);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "New");
   evas_object_smart_callback_add(o, "clicked", _cb_select_new_click, win);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "Del");
   evas_object_smart_callback_add(o, "clicked", _cb_select_del_click, win);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "Copy");
   evas_object_smart_callback_add(o, "clicked", _cb_select_copy_click, win);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, o);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "Select");
   evas_object_smart_callback_add(o, "clicked", _cb_select_select_click, win);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_button_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(o, "Cancel");
   evas_object_smart_callback_add(o, "clicked", _cb_select_cancel_click, win);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   return pop;
}
