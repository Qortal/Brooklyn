/* Setup if user wants Tasks? */
#include "e_wizard.h"
#include "e_wizard_api.h"

static Eina_Bool do_tasks = 0;
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
E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   Evas_Object *o, *of, *ob;

   api->wizard_title_set(_("Taskbar"));

   of = elm_frame_add(e_comp->elm);
   elm_object_text_set(of, _("Information"));

   o = elm_box_add(of);
   elm_object_content_set(of, o);

   ob = elm_label_add(o);
   elm_object_style_set(ob, "default/left");
   E_ALIGN(ob, 0, 0.5);
   evas_object_show(ob);
   elm_box_pack_end(o, ob);
   elm_object_text_set(ob,
     _("A taskbar can be added to<ps/>"
       "show open windows and applications.<ps/>"
       "The IBar launcher already can do this,<ps/>"
       "so this is in addition to that."
       )
     );

   ob = elm_check_add(o);
   E_ALIGN(ob, 0, 0.5);
   evas_object_show(ob);
   elm_box_pack_end(o, ob);
   elm_object_text_set(ob, _("Enable Taskbar"));
   elm_check_state_pointer_set(ob, &do_tasks);

   evas_object_show(of);

   api->wizard_page_show(of);
   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}
/*
E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   E_Config_Module *em;
   Eina_List *l;

   if (do_tasks) return 1;

   EINA_LIST_FOREACH(e_config->modules, l, em)
     {
        if (!em->name) continue;
        if (strcmp(em->name, "tasks")) continue;
        e_config->modules = eina_list_remove_list(e_config->modules, l);
        eina_stringshare_del(em->name);
        free(em);
        break;
     }

   return 1;
}

