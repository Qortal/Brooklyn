/* Menu setup */
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
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/etc/xdg/menus/e-applications.menu",
            e_prefix_get());
   e_config->default_system_menu = eina_stringshare_add(buf);
   return 0; /* 1 == show ui, and wait for user, 0 == just continue */
}
/*
E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}

E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
