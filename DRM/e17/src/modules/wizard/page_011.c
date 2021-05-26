/* Language chooser */
#include "e_wizard.h"
#include "e_wizard_api.h"

typedef struct _Layout Layout;

struct _Layout
{
   const char *name;
   const char *label;
};

static const char *rules_file = NULL;
static const char *layout = NULL;
static Eina_List *layouts = NULL;

static void
find_rules(void)
{
   int i = 0;
   const char *lstfiles[] = {
#ifdef XKB_BASE
      XKB_BASE "/rules/xorg.lst",
      XKB_BASE "/rules/xfree86.lst",
#endif
#if defined __NetBSD__
      "/usr/X11R7/lib/X11/xkb/rules/xorg.lst",
#elif defined __OpenBSD__
      "/usr/X11R6/share/X11/xkb/rules/base.lst",
#endif
      "/usr/share/X11/xkb/rules/xorg.lst",
      "/usr/share/X11/xkb/rules/xfree86.lst",
      "/usr/local/share/X11/xkb/rules/xorg.lst",
      "/usr/local/share/X11/xkb/rules/xfree86.lst",
      "/usr/X11R6/lib/X11/xkb/rules/xorg.lst",
      "/usr/X11R6/lib/X11/xkb/rules/xfree86.lst",
      "/usr/local/X11R6/lib/X11/xkb/rules/xorg.lst",
      "/usr/local/X11R6/lib/X11/xkb/rules/xfree86.lst",
      NULL
   };

   for (; lstfiles[i]; i++)
     {
        FILE *f = fopen(lstfiles[i], "r");
        if (f)
          {
             fclose(f);
             rules_file = lstfiles[i];
             break;
          }
     }
}

static int
_layout_sort_cb(const void *data1, const void *data2)
{
   const Layout *l1 = data1;
   const Layout *l2 = data2;
   return e_util_strcasecmp(l1->label ?: l1->name, l2->label ?: l2->name);
}

int
parse_rules(void)
{
   char buf[4096];
   FILE *f = fopen(rules_file, "r");
   if (!f) return 0;

   for (;; )
     {
        if (!fgets(buf, sizeof(buf), f)) goto err;
        if (!strncmp(buf, "! layout", 8))
          {
             for (;; )
               {
                  Layout *lay;
                  char name[4096], label[4096];

                  if (!fgets(buf, sizeof(buf), f)) goto err;
                  if (sscanf(buf, "%s %[^\n]", name, label) != 2) break;
                  lay = calloc(1, sizeof(Layout));
                  lay->name = eina_stringshare_add(name);
                  lay->label = eina_stringshare_add(label);
                  layouts = eina_list_append(layouts, lay);
               }
             break;
          }
     }
err:
   fclose(f);
   layouts = eina_list_sort(layouts, 0, _layout_sort_cb);
   return 1;
}

static void
implement_layout(void)
{
   Eina_List *l;
   E_Config_XKB_Layout *nl;
   Eina_Bool found = EINA_FALSE;

   if (!layout) return;

   EINA_LIST_FOREACH(e_config->xkb.used_layouts, l, nl)
     {
        if ((nl->name) && (!strcmp(layout, nl->name)))
          {
             found = EINA_TRUE;
             break;
          }
     }
   if (!found)
     {
        nl = E_NEW(E_Config_XKB_Layout, 1);
        nl->name = eina_stringshare_ref(layout);
        nl->variant = eina_stringshare_add("basic");
        nl->model = eina_stringshare_add("default");
        e_config->xkb.used_layouts = eina_list_prepend(e_config->xkb.used_layouts, nl);
        e_xkb_reconfig();
     }
   e_xkb_layout_set(nl);
}

E_API int
wizard_page_init(E_Wizard_Page *pg EINA_UNUSED, Eina_Bool *need_xdg_desktops EINA_UNUSED, Eina_Bool *need_xdg_icons EINA_UNUSED)
{
   // parse kbd rules here
   find_rules();
   parse_rules();
   return 1;
}
/*
E_API int
wizard_page_shutdown(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/


static Evas_Object *
_layout_content_get(Layout *lay, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic;

   if (!eina_streq(part, "elm.swallow.icon")) return NULL;
   e_xkb_flag_file_get(buf, sizeof(buf), lay->name);
   ic = elm_icon_add(obj);
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 20, 10);
   return ic;
}

static char *
_layout_text_get(Layout *lay, Evas_Object *obj EINA_UNUSED, const char *part)
{
   if (!eina_streq(part, "elm.text")) return NULL;
   return strdup(_(lay->label));
}

static void
_layout_select(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Layout *lay = data;
   layout = lay->name;
}

E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   Evas_Object *of, *ob;
   Eina_List *l;
   Layout *lay;
   void *sel_it = NULL;
   static Elm_Genlist_Item_Class itc =
   {
      .item_style = "default",
      .func =
      {
         .content_get = (Elm_Genlist_Item_Content_Get_Cb)_layout_content_get,
         .text_get = (Elm_Genlist_Item_Text_Get_Cb)_layout_text_get,
      },
      .version = ELM_GENLIST_ITEM_CLASS_VERSION
   };

   api->wizard_title_set(_("Keyboard"));
   of = elm_frame_add(e_comp->elm);
   elm_object_text_set(of, _("Select one"));
   ob = elm_genlist_add(of);
   elm_genlist_homogeneous_set(ob, 1);
   elm_genlist_mode_set(ob, ELM_LIST_COMPRESS);
   elm_scroller_bounce_set(ob, 0, 0);
   elm_object_content_set(of, ob);

   EINA_LIST_FOREACH(layouts, l, lay)
     {
        void *it;

        it = elm_genlist_item_append(ob, &itc, lay, NULL, 0, _layout_select, lay);
        if (eina_streq(lay->name, "us"))
          sel_it = it;
     }

   evas_object_show(ob);
   evas_object_show(of);
   E_EXPAND(of);
   E_FILL(of);
   if (sel_it)
     {
        elm_genlist_item_selected_set(sel_it, 1);
        elm_genlist_item_show(sel_it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
     }
   api->wizard_page_show(of);
   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   /* special - key layout inits its stuff the moment it goes away */
   implement_layout();
   return 1;
}

E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   // do this again as we want it to apply to the new profile
   implement_layout();
   return 1;
}

