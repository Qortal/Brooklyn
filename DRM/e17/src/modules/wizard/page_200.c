/* Delete previous copy of config profile and save new one */
#include "e_wizard.h"
#include "e_wizard_api.h"

#if 0
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

E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   return 0; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
#endif
E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   // save the config now everyone has modified it
   e_config_save();
   // diusable restart env so we actually start a whole new session properly
   e_util_env_set("E_RESTART", NULL);
   // restart e
   e_sys_action_do(E_SYS_RESTART, NULL);
   return 1;
}

