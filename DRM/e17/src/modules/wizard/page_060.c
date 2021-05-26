/* Ask about focus mode */
#include "e_wizard.h"
#include "e_wizard_api.h"

static int focus_mode = 1;
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
   Evas_Object *o, *of, *ob, *rg;

   if (e_config->focus_policy == E_FOCUS_CLICK) focus_mode = 0;

   api->wizard_title_set(_("Window Focus"));

   of = elm_frame_add(e_comp->elm);
   elm_object_text_set(of, _("Focus:"));
   o = elm_box_add(of);
   elm_box_homogeneous_set(o, 1);
   elm_object_content_set(of, o);

   rg = ob = elm_radio_add(o);
   evas_object_show(ob);
   E_ALIGN(ob, 0, 0.5);
   elm_box_pack_end(o, ob);
   elm_object_text_set(ob, _("Whenever a window is clicked"));
   elm_radio_state_value_set(ob, 0);
   elm_radio_value_pointer_set(ob, &focus_mode);

   ob = elm_radio_add(o);
   E_ALIGN(ob, 0, 0.5);
   evas_object_show(ob);
   elm_box_pack_end(o, ob);
   elm_radio_group_add(ob, rg);
   elm_object_text_set(ob, _("Whenever the mouse enters a window"));
   elm_radio_state_value_set(ob, 1);

   api->wizard_page_show(of);
//   pg->data = o;
   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   if (!focus_mode)
     {
        e_config->focus_policy = E_FOCUS_CLICK;
        e_config->focus_setting = E_FOCUS_NEW_WINDOW;
        e_config->pass_click_on = 1;
        e_config->always_click_to_raise = 1;
        e_config->always_click_to_focus = 1;
        e_config->raise_on_revert_focus = 1;
        e_config->focus_last_focused_per_desktop = 1;
        e_config->pointer_slide = 0;
        e_config->disable_all_pointer_warps = 1;
        e_config->winlist_warp_while_selecting = 0;
        e_config->winlist_warp_at_end = 0;
        e_config->winlist_no_warp_on_direction = 1;
     }
   else
     {
        e_config->focus_policy = E_FOCUS_SLOPPY;
        e_config->focus_setting = E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED;
        e_config->pass_click_on = 1;
        e_config->always_click_to_raise = 0;
        e_config->always_click_to_focus = 0;
        e_config->focus_last_focused_per_desktop = 1;
        e_config->pointer_slide = 1;
// auto raise seems to irritate some people so request - disable in wiz
//        e_config->use_auto_raise = 1;
//        e_config->auto_raise_delay = 0.5;
     }
//   evas_object_del(pg->data);
   return 1;
}
/*
E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
