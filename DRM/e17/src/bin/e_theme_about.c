#include "e.h"

/* local subsystem functions */
static void
_cb_settings_theme(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   e_configure_registry_call("appearance/theme", NULL, NULL);
}

/* local subsystem globals */

/* externally accessible functions */

E_API E_Theme_About *
e_theme_about_new(void)
{
   E_Obj_Dialog *od;

   od = e_obj_dialog_new(_("About Theme"), "E", "_theme_about");
   if (!od) return NULL;
   e_obj_dialog_obj_theme_set(od, "base/theme", "e/theme/about");
   e_obj_dialog_obj_part_text_set(od, "e.text.label", _("Close"));
   e_obj_dialog_obj_part_text_set(od, "e.text.theme", _("Select Theme"));
   edje_object_signal_callback_add(od->bg_object,
                                   "e,action,settings,theme", "",
                                   _cb_settings_theme, od);
   return (E_Theme_About *)od;
}

E_API void
e_theme_about_show(E_Theme_About *about)
{
   e_obj_dialog_show((E_Obj_Dialog *)about);
   e_obj_dialog_icon_set((E_Obj_Dialog *)about, "preferences-desktop-theme");
}

