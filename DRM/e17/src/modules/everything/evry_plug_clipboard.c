#include "e_mod_main.h"

#ifndef HAVE_WAYLAND_ONLY
static Evry_Action *act;
static Ecore_Window clipboard_win = 0;

static int
_action(Evry_Action *action)
{
   const Evry_Item *it;

   if (e_comp->comp_type != E_PIXMAP_TYPE_X) return 0;
   it = action->it1.item;
   ecore_x_selection_primary_set(clipboard_win, it->label, strlen(it->label));
   ecore_x_selection_clipboard_set(clipboard_win, it->label, strlen(it->label));
   return 1;
}

static int
_check_item(Evry_Action *action EINA_UNUSED, const Evry_Item *it)
{
   return it && it->label && (strlen(it->label) > 0);
}
#endif
Eina_Bool
evry_plug_clipboard_init(void)
{
#ifndef HAVE_WAYLAND_ONLY
   Ecore_Window win;
#endif

   if (!evry_api_version_check(EVRY_API_VERSION))
     return EINA_FALSE;

   if (e_comp->comp_type != E_PIXMAP_TYPE_X)
     return EINA_FALSE;
#ifndef HAVE_WAYLAND_ONLY
   win = ecore_x_window_input_new(0, 0, 0, 1, 1);
   if (!win) return EINA_FALSE;
   ecore_x_icccm_name_class_set(win, "evry", "clipboard");
   e_comp_ignore_win_add(E_PIXMAP_TYPE_X, win);

//FIXME: Icon name doesn't follow FDO Spec
   act = EVRY_ACTION_NEW(N_("Copy to Clipboard"),
                         EVRY_TYPE_TEXT, 0,
                         "everything-clipboard",
                         _action, _check_item);
   act->remember_context = EINA_TRUE;
   evry_action_register(act, 10);

   clipboard_win = win;

   return EINA_TRUE;
#endif

   return EINA_TRUE;
}

void
evry_plug_clipboard_shutdown(void)
{
   if (e_comp->comp_type != E_PIXMAP_TYPE_X) return;
#ifndef HAVE_WAYLAND_ONLY
   ecore_x_window_free(clipboard_win);
   evry_action_free(act);
#endif
}

