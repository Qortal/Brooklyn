/* Language chooser */
#include "e_wizard.h"
#include "e_wizard_api.h"

typedef struct _E_Intl_Pair E_Intl_Pair;

static int _basic_lang_list_sort(const void *data1, const void *data2);

struct _E_Intl_Pair
{
   const char *locale_key;
   const char *locale_icon;
   const char *locale_translation;
};

const E_Intl_Pair basic_language_predefined_pairs[] =
{
   {"ar_AE.UTF-8", "ara_flag.png", "العربية"},
   {"bg_BG.UTF-8", "bg_flag.png", "Български"},
   {"ca_ES.UTF-8", "cat_flag.png", "Català"},
   {"cs_CZ.UTF-8", "cz_flag.png", "Čeština"},
   {"da_DK.UTF-8", "dk_flag.png", "Dansk"},
   {"de_DE.UTF-8", "de_flag.png", "Deutsch"},
   {"en_US.UTF-8", "us_flag.png", "English"},
   {"en_GB.UTF-8", "gb_flag.png", "British English"},
   {"el_GR.UTF-8", "gr_flag.png", "Ελληνικά"},
   {"eo_US.UTF-8", "epo_flag.png", "Esperanto"},
   {"eo.UTF-8", "epo_flag.png", "Esperanto"},
   {"es_AR.UTF-8", "ar_flag.png", "Español"},
   {"et_ET.UTF-8", "ee_flag.png", "Eesti keel"},
   {"fi_FI.UTF-8", "fi_flag.png", "Suomi"},
   {"fo_FO.UTF-8", "fo_flag.png", "Føroyskt"},
   {"fr_CH.UTF-8", "ch_flag.png", "Français (Suisse)"},
   {"fr_FR.UTF-8", "fr_flag.png", "Français"},
   {"gl_ES.UTF-8", "es_flag.png", "Galego"},
   {"he_HE.UTF-8", "il_flag.png", "עברית"},
   {"hr_HR.UTF-8", "hr_flag.png", "Hrvatski"},
   {"hu_HU.UTF-8", "hu_flag.png", "Magyar"},
   {"it_IT.UTF-8", "it_flag.png", "Italiano"},
   {"ja_JP.UTF-8", "jp_flag.png", "日本語"},
   {"km_KM.UTF-8", "kh_flag.png", "ភាសាខ្មែរ"},
   {"ko_KR.UTF-8", "kr_flag.png", "한국어"},
   {"ku.UTF-8", "ku_flag.png", "یدروك"},
   {"lt_LT.UTF-8", "lt_flag.png", "Lietuvių kalba"},
   {"ms_MY.UTF-8", "my_flag.png", "Bahasa Melayu"},
   {"nb_NO.UTF-8", "no_flag.png", "Norsk Bokmål"},
   {"nl_NL.UTF-8", "nl_flag.png", "Nederlands"},
   {"pl_PL.UTF-8", "pl_flag.png", "Polski"},
   {"pt_BR.UTF-8", "br_flag.png", "Português"},
   {"ro_RO.UTF-8", "ro_flag.png", "Română"},
   {"ru_RU.UTF-8", "ru_flag.png", "Русский"},
   {"sk_SK.UTF-8", "sk_flag.png", "Slovenčina"},
   {"sl_SI.UTF-8", "si_flag.png", "Slovenščina"},
   {"sv_SE.UTF-8", "se_flag.png", "Svenska"},
   {"tr_TR.UTF-8", "tr_flag.png", "Türkçe"},
   {"uk_UK.UTF-8", "ua_flag.png", "Українська мова"},
   {"vi_VN UTF-8", "vn_flag.png", "Tiếng Việt"},
   {"zh_CN.UTF-8", "cn_flag.png", "中文 (简体)"},
   {"zh_TW.UTF-8", "tw_flag.png", "中文 (繁體)"},
   { NULL, NULL, NULL }
};

static const char *lang = NULL;
static Eina_List *blang_list = NULL;

static int
_basic_lang_list_sort(const void *data1, const void *data2)
{
   E_Intl_Pair *ln1, *ln2;
   const char *trans1;
   const char *trans2;

   if (!data1) return 1;
   if (!data2) return -1;

   ln1 = (E_Intl_Pair *)data1;
   ln2 = (E_Intl_Pair *)data2;

   if (!ln1->locale_translation) return 1;
   trans1 = ln1->locale_translation;

   if (!ln2->locale_translation) return -1;
   trans2 = ln2->locale_translation;

   return strcmp(trans1, trans2);
}

E_API int
wizard_page_init(E_Wizard_Page *pg EINA_UNUSED, Eina_Bool *need_xdg_desktops EINA_UNUSED, Eina_Bool *need_xdg_icons EINA_UNUSED)
{
   FILE *output;

#ifdef __OpenBSD__
   output = popen("ls /usr/share/locale", "r");
#else
   output = popen("locale -a", "r");
#endif
   if (output)
     {
        char line[32];

        while (fscanf(output, "%[^\n]\n", line) == 1)
          {
             E_Locale_Parts *locale_parts;

             locale_parts = e_intl_locale_parts_get(line);
             if (locale_parts)
               {
                  char *basic_language = NULL;

                  if (locale_parts->mask & E_INTL_LOC_REGION)
                    basic_language = e_intl_locale_parts_combine(locale_parts,
                      E_INTL_LOC_LANG | E_INTL_LOC_REGION);
                  else if (locale_parts->lang)
                    basic_language = strdup(locale_parts->lang);
                  if (basic_language)
                    {
                       int i = 0;

                       while (basic_language_predefined_pairs[i].locale_key)
                         {
                            /* if basic language is supported by E and System*/
                            if (!strncmp
                                  (basic_language_predefined_pairs[i].locale_key,
                                  basic_language, strlen(basic_language)))
                              {
                                 if (!eina_list_data_find
                                       (blang_list,
                                       &basic_language_predefined_pairs[i]))
                                   blang_list = eina_list_append
                                       (blang_list,
                                       &basic_language_predefined_pairs[i]);
                                 break;
                              }
                            i++;
                         }
                    }
                  E_FREE(basic_language);
                  e_intl_locale_parts_free(locale_parts);
               }
          }
        /* Sort basic languages */
        blang_list = eina_list_sort(blang_list, eina_list_count(blang_list), _basic_lang_list_sort);
        pclose(output);
     }
   return 1;
}

E_API int
wizard_page_shutdown(E_Wizard_Page *pg EINA_UNUSED)
{
   eina_list_free(blang_list);
   return 1;
}

static Evas_Object *
_lang_content_get(E_Intl_Pair *pair, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic;

   if (!eina_streq(part, "elm.swallow.icon")) return NULL;
   if (pair && (!pair->locale_icon)) return NULL;
   if (pair)
     e_prefix_data_snprintf(buf, sizeof(buf), "data/flags/%s", pair->locale_icon);
   else
     e_prefix_data_snprintf(buf, sizeof(buf), "data/flags/%s", "lang-system.png");

   ic = elm_icon_add(obj);
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 20, 10);
   return ic;
}

static char *
_lang_text_get(E_Intl_Pair *pair, Evas_Object *obj EINA_UNUSED, const char *part)
{
   char buf[4096];

   if (!eina_streq(part, "elm.text")) return NULL;
   if (!pair)
     return strdup(_("System Default"));

   if ((!e_intl_language_get()) || (!eina_streq(pair->locale_key, e_intl_language_get())))
     return strdup(_(pair->locale_translation));

   snprintf(buf, sizeof(buf), "System Default [%s]", pair->locale_translation);
   return strdup(buf);
}

static void
_lang_select(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Intl_Pair *pair = data;

   lang = pair ? pair->locale_key : NULL;
}

E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   Evas_Object *of, *ob;
   Eina_List *l;
   E_Intl_Pair *pair;
   Eina_Bool found = EINA_FALSE;
   static Elm_Genlist_Item_Class itc =
   {
      .item_style = "default",
      .func =
      {
         .content_get = (Elm_Genlist_Item_Content_Get_Cb)_lang_content_get,
         .text_get = (Elm_Genlist_Item_Text_Get_Cb)_lang_text_get,
      },
      .version = ELM_GENLIST_ITEM_CLASS_VERSION
   };

   api->wizard_title_set(_("Language"));
   of = elm_frame_add(e_comp->elm);
   elm_object_text_set(of, _("Select one"));
   ob = elm_genlist_add(of);
   elm_genlist_homogeneous_set(ob, 1);
   elm_genlist_mode_set(ob, ELM_LIST_COMPRESS);
   elm_scroller_bounce_set(ob, 0, 0);
   elm_object_content_set(of, ob);
   EINA_LIST_FOREACH(blang_list, l, pair)
     {
        if (e_intl_language_get() && eina_streq(pair->locale_key, e_intl_language_get()))
          found = 1;
        elm_genlist_item_append(ob, &itc, pair, NULL, 0, _lang_select, pair);
     }
   if (found)
     elm_genlist_item_prepend(ob, &itc, NULL, NULL, 0, _lang_select, NULL);
   if ((!found) && (elm_genlist_items_count(ob) == 2)) // default and one other
     elm_genlist_item_selected_set(elm_genlist_last_item_get(ob), 1);
   else
     elm_genlist_item_selected_set(elm_genlist_first_item_get(ob), 1);

   evas_object_show(ob);
   evas_object_show(of);
   E_EXPAND(of);
   E_FILL(of);
   elm_genlist_item_show(elm_genlist_selected_item_get(ob), ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
   api->wizard_page_show(of);
//   pg->data = o;
   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
//   evas_object_del(pg->data);
/* special - language inits its stuff the moment it goes away */
   eina_stringshare_replace(&e_config->language, lang);
   /* TODO:
    * This should be on lang select,
    * so if next page needs xdg we can't press next */
   if (lang)
     api->wizard_xdg_desktops_reset();
   e_intl_language_set(e_config->language);
   api->wizard_labels_update();
   return 1;
}

E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   // do this again as we want it to apply to the new profile
   eina_stringshare_replace(&e_config->language, lang);
   e_intl_language_set(e_config->language);
   api->wizard_labels_update();
   return 1;
}

