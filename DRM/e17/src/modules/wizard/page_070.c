/* Setup of default icon theme */
#include "e_wizard.h"
#include "e_wizard_api.h"

E_API int
wizard_page_init(E_Wizard_Page *pg EINA_UNUSED, Eina_Bool *need_xdg_desktops EINA_UNUSED, Eina_Bool *need_xdg_icons)
{
   *need_xdg_icons = EINA_TRUE;
   return 1;
}
/*
E_API int
wizard_page_shutdown(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
E_API int
wizard_page_show(E_Wizard_Page *pg EINA_UNUSED)
{
   Eina_List *l, *themes = efreet_icon_theme_list_get();
   Efreet_Icon_Theme *th;
   int i;
   const char *selected = NULL;
   const char *search[] =
   {
      "gnome",
      "Humanity",
      "Humanity-Dark",
      "ubuntu-mono-light",
      "ubuntu-mono-dark",
      "ubuntu-mono-light",
      "unity-icon-theme",
      "Oxygen",            /* KDE 4 */
      "Adwaita",           /* Gnome 3 */
      "Breeze",            /* KDE 5 */
      "HighContrast",
      NULL
   };

   if (!themes) return 0;
   for (i = 0; search[i]; i++)
     {
        EINA_LIST_FOREACH(themes, l, th)
          {
             if (!strcasecmp(search[i], th->name.internal))
               {
                  selected = search[i];
                  goto done;
               }
          }
     }
done:
   if (selected)
     {
        if (e_config->icon_theme) eina_stringshare_del(e_config->icon_theme);
        e_config->icon_theme = eina_stringshare_add(selected);
     }
   eina_list_free(themes);
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
