/* Splash Screen */

#include "e_wizard.h"
#include "e_wizard_api.h"

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
   Evas_Object *o;

   api->wizard_title_set(_("Enlightenment"));
   o = edje_object_add(pg->evas);
   e_theme_edje_object_set(o, "base/theme/wizard", "e/wizard/firstpage");
   api->wizard_page_show(o);
   return 1;
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
/*
E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
