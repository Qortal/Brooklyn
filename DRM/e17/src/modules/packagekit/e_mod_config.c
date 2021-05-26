#include "e_mod_config.h"
#include "e_mod_packagekit.h"

static void
_config_label_add(Evas_Object *tb, const char *txt, int row)
{
   Evas_Object *o;

   o = elm_label_add(tb);
   E_ALIGN(o, 0, 0.5);
   elm_object_text_set(o, txt);
   evas_object_show(o);
   elm_table_pack(tb, o, 0, row, 1, 1);
}

static void
_config_refresh_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   E_PackageKit_Module_Context *ctxt = data;
   Elm_Object_Item *item = event_info;
   int val = (int)(intptr_t)elm_object_item_data_get(item);

   ctxt->config->update_interval = val;
   e_config_save_queue();
}

static void
_config_list_type_changed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   E_PackageKit_Module_Context *ctxt = data;
   Elm_Object_Item *item = event_info;
   int val = (int)(intptr_t)elm_object_item_data_get(item);

   ctxt->config->show_description = val;
   e_config_save_queue();
}

static void
_config_manager_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_PackageKit_Module_Context *ctxt = data;
   const char *text = elm_object_text_get(obj);

   if (ctxt->config->manager_command)
      eina_stringshare_replace(&ctxt->config->manager_command, text);
   else
      ctxt->config->manager_command = eina_stringshare_add(text);
   e_config_save_queue();
}

Evas_Object *
packagekit_config_show(E_PackageKit_Module_Context *ctxt)
{
   Evas_Object *popup, *table, *o;
   int row = 0;

   // Create the popup (this should be handled by the gadget system)
   popup = elm_popup_add(e_comp->elm);
   E_EXPAND(popup);
   evas_object_layer_set(popup, E_LAYER_POPUP);
   elm_popup_allow_events_set(popup, EINA_TRUE);
   elm_popup_scrollable_set(popup, EINA_TRUE);

   // Create the content
   table = elm_table_add(popup);
   elm_table_padding_set(table, 10, 0);
   E_EXPAND(table);
   evas_object_show(table);
   elm_object_content_set(popup, table);

   // auto refresh frequency
   _config_label_add(table, _("Refresh Packages"), row);
   o = elm_hoversel_add(table);
   E_FILL(o); E_EXPAND(o);
   elm_table_pack(table, o, 1, row, 1, 1);
   elm_hoversel_item_add(o, _("Never"), NULL, ELM_ICON_NONE, NULL,
                         (uintptr_t*)(unsigned long) (0) );
   elm_hoversel_item_add(o, _("Hourly"), NULL, ELM_ICON_NONE, NULL,
                         (uintptr_t*)(unsigned long) (60) );
   elm_hoversel_item_add(o, _("Daily"), NULL, ELM_ICON_NONE, NULL,
                         (uintptr_t*)(unsigned long) (60 * 24));
   elm_hoversel_item_add(o, _("Weekly"), NULL, ELM_ICON_NONE, NULL,
                         (uintptr_t*)(unsigned long) (60 * 24 * 7));
   switch (ctxt->config->update_interval)
   {
      case 0:           elm_object_text_set(o, _("Never"));  break;
      case 60:          elm_object_text_set(o, _("Hourly")); break;
      case 60 * 24:     elm_object_text_set(o, _("Daily"));  break;
      case 60 * 24 * 7: elm_object_text_set(o, _("Weekly")); break;
      default: break;
   }
   elm_hoversel_hover_parent_set(o, popup);
   elm_hoversel_auto_update_set(o, EINA_TRUE);
   evas_object_show(o);
   evas_object_smart_callback_add(o, "selected", _config_refresh_changed, ctxt);
   row++;

   // list type (show description)
   _config_label_add(table, _("Package list"), row);
   o = elm_hoversel_add(table);
   E_FILL(o); E_EXPAND(o);
   elm_table_pack(table, o, 1, row, 1, 1);
   elm_hoversel_item_add(o, _("Compact (package name)"), NULL,
                         ELM_ICON_NONE, NULL, (uintptr_t*)(unsigned long) 0 );
   elm_hoversel_item_add(o, _("Extended (name and description)"), NULL,
                         ELM_ICON_NONE, NULL, (uintptr_t*)(unsigned long) 1 );
   switch (ctxt->config->show_description)
   {
      case 0: elm_object_text_set(o, _("Compact (package name)"));  break;
      case 1: elm_object_text_set(o, _("Extended (name and description)")); break;
      default: break;
   }
   elm_hoversel_hover_parent_set(o, popup);
   elm_hoversel_auto_update_set(o, EINA_TRUE);
   evas_object_show(o);
   evas_object_smart_callback_add(o, "selected", _config_list_type_changed, ctxt);
   row++;

   // package manager entry
   _config_label_add(table, _("Package Manager"), row);
   o = elm_entry_add(table);
   elm_table_pack(table, o, 1, row, 1, 1);
   elm_entry_scrollable_set(o, EINA_TRUE);
   elm_entry_single_line_set(o, EINA_TRUE);
   elm_object_text_set(o, ctxt->config->manager_command);
   E_FILL(o); E_EXPAND(o);
   evas_object_show(o);
   evas_object_smart_callback_add(o, "changed,user", _config_manager_changed, ctxt);
   row++;

   // Show the popup (really! this should be handled by the gadget system)
   E_Zone *zone = e_zone_current_get();
   popup = e_comp_object_util_add(popup, E_COMP_OBJECT_TYPE_NONE);
   evas_object_layer_set(popup, E_LAYER_POPUP);
   evas_object_move(popup, zone->x, zone->y);
   evas_object_resize(popup, zone->w / 4, zone->h / 3);
   e_comp_object_util_center(popup);
   evas_object_show(popup);
   e_comp_object_util_autoclose(popup, NULL, e_comp_object_util_autoclose_on_escape, NULL);
   //evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _config_close, NULL);

   return popup;
}
