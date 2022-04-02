/* Profile chooser */
#include "e_wizard.h"
#include "e_wizard_api.h"

static const char *profile = NULL;
static Evas_Object *textblock = NULL;

/*
E_API int
wizard_page_init(E_Wizard_Page *pg EINA_UNUSED, Eina_Bool *need_xdg_desktops EINA_UNUSED, Eina_Bool *need_xdg_icons EINA_UNUSED)
{
   return 1;
}

E_API int
wizard_page_shutdown(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/


static Evas_Object *
_profile_content_get(char *prof, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX + 100], buf2[PATH_MAX];
   Evas_Object *ic;
   Efreet_Desktop *desktop;

   if (!eina_streq(part, "elm.swallow.icon")) return NULL;
   e_prefix_data_snprintf(buf2, sizeof(buf2), "data/config/%s", prof);
   snprintf(buf, sizeof(buf), "%s/profile.desktop", buf2);
   desktop = efreet_desktop_new(buf);
   snprintf(buf, sizeof(buf), "%s/icon.edj", buf2);
   if (!ecore_file_exists(buf))
     {
        if (desktop && desktop->icon)
          {
             if (eina_str_has_extension(desktop->icon, "png"))
               snprintf(buf, sizeof(buf), "%s/%s", buf2, desktop->icon);
             else
               snprintf(buf, sizeof(buf), "%s/%s.png", buf2, desktop->icon);
          }
        else
          e_prefix_data_concat_static(buf, "data/images/enlightenment.png");
     }
   efreet_desktop_free(desktop);

   ic = elm_icon_add(obj);
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 20, 10);
   return ic;
}

static char *
_profile_text_get(char *prof, Evas_Object *obj EINA_UNUSED, const char *part)
{
   char *label;
   char buf[PATH_MAX + 100], buf2[PATH_MAX];
   Efreet_Desktop *desktop;

   if (!eina_streq(part, "elm.text")) return NULL;
   e_prefix_data_snprintf(buf2, sizeof(buf2), "data/config/%s", prof);
   snprintf(buf, sizeof(buf), "%s/profile.desktop", buf2);
   label = prof;
   desktop = efreet_desktop_new(buf);
   if (desktop && desktop->name) label = desktop->name;
   label = strdup(label);
   efreet_desktop_free(desktop);
   return label;
}

static void
_profile_select(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX + 100], buf2[PATH_MAX];
   Efreet_Desktop *desk = NULL;
   profile = data;

   e_prefix_data_snprintf(buf2, sizeof(buf2), "data/config/%s", profile);
   snprintf(buf, sizeof(buf), "%s/profile.desktop", buf2);
   desk = efreet_desktop_new(buf);
   if (desk)
     {
        elm_object_text_set(textblock, desk->comment);
        efreet_desktop_free(desk);
     }
   else
     elm_object_text_set(textblock, _("Unknown"));

   // enable next once you choose a profile
   api->wizard_button_next_enable_set(1);
}

E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   Evas_Object *o, *of, *ob;
   Eina_List *profiles;
   char *prof;
   void *sel_it = NULL;
   static Elm_Genlist_Item_Class itc =
   {
      .item_style = "default",
      .func =
      {
         .content_get = (Elm_Genlist_Item_Content_Get_Cb)_profile_content_get,
         .text_get = (Elm_Genlist_Item_Text_Get_Cb)_profile_text_get,
      },
      .version = ELM_GENLIST_ITEM_CLASS_VERSION
   };

   api->wizard_title_set(_("Profile"));
   of = elm_frame_add(e_comp->elm);
   elm_object_text_set(of, _("Select one"));

   o = elm_box_add(of);
   elm_object_content_set(of, o);

   ob = elm_genlist_add(o);
   evas_object_show(ob);
   elm_box_pack_end(o, ob);
   E_EXPAND(ob);
   E_FILL(ob);
   elm_genlist_homogeneous_set(ob, 1);
   elm_genlist_mode_set(ob, ELM_LIST_COMPRESS);
   elm_scroller_bounce_set(ob, 0, 0);

   profiles = e_config_profile_list();
   EINA_LIST_FREE(profiles, prof)
     {
        char buf2[PATH_MAX];
        void *it;

        if (eina_streq(prof, e_config_profile_get()))
          {
             free(prof);
             continue;
          }
        e_prefix_data_snprintf(buf2, sizeof(buf2), "data/config/%s", prof);
        // if it's not a system profile - don't offer it
        if (!ecore_file_is_dir(buf2))
          {
             free(prof);
             continue;
          }
        it = elm_genlist_item_append(ob, &itc, prof, NULL, 0, _profile_select, prof);
        if (eina_streq(prof, "standard")) sel_it = it;
     }

   ob = elm_label_add(o);
   evas_object_show(ob);
   elm_box_pack_end(o, ob);
   E_WEIGHT(ob, EVAS_HINT_EXPAND, 0);
   E_FILL(ob);
   elm_object_style_set(ob, "marker");
   elm_object_text_set(ob, _("Select a profile"));
   textblock = ob;

   if (sel_it)
     {
        elm_genlist_item_selected_set(sel_it, 1);
        elm_genlist_item_show(sel_it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
     }

   evas_object_show(ob);
   evas_object_show(of);
   E_EXPAND(of);
   E_FILL(of);
   api->wizard_page_show(of);
//   pg->data = o;
   if (!sel_it)
     api->wizard_button_next_enable_set(0);
   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
//   evas_object_del(pg->data);
// actually apply profile
   if (!profile) profile = "standard";
   e_config_profile_set(profile);
   e_config_profile_del(e_config_profile_get());
   e_config_load();
   e_config_save();
   return 1;
}
/*
E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
