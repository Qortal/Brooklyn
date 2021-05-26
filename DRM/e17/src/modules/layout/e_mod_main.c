/**
 * @addtogroup Optional_Layouts
 * @{
 *
 * @defgroup Module_Layout Layout Policy Enforcement
 *
 * Enforces window layout policies.
 *
 * @}
 */
#include "e.h"

/***************************************************************************/
/**/
/* actual module specifics */

static E_Module *layout_module = NULL;
static E_Client_Hook *hook = NULL;

static void
_e_module_layout_cb_hook(void *data, E_Client *ec)
{
   /* FIXME: make some modification based on policy */
   printf("Window:\n"
	  "  Title:    [%s][%s]\n"
	  "  Class:    %s::%s\n"
	  "  Geometry: %ix%i+%i+%i\n"
	  "  New:      %i\n"
	  , bd->icccm.title, bd->netwm.name
	  , bd->icccm.name, bd->icccm.class
	  , bd->x, bd->y, bd->w, bd->h
	  , bd->new_client
	  );
   if ((bd->icccm.transient_for != 0) ||
       (bd->netwm.type == ECORE_X_WINDOW_TYPE_DIALOG))
     {
	bd->client.e.state.centered = 1;
     }
   else
     {
	e_client_unmaximize(bd, E_MAXIMIZE_BOTH);
	e_client_resize(bd, 1, 1);
	e_client_center(bd);
	if (bd->bordername) eina_stringshare_del(bd->bordername);
	bd->bordername = eina_stringshare_add("borderless");
	bd->icccm.base_w = 1;
	bd->icccm.base_h = 1;
	bd->icccm.min_w = 1;
	bd->icccm.min_h = 1;
	bd->icccm.max_w = 32767;
	bd->icccm.max_h = 32767;
	bd->icccm.min_aspect = 0.0;
	bd->icccm.max_aspect = 0.0;
     }
   e_client_maximize(bd, E_MAXIMIZE_FILL | E_MAXIMIZE_BOTH);
}

/**/
/***************************************************************************/

/***************************************************************************/
/**/

/**/
/***************************************************************************/

/***************************************************************************/
/**/
/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Layout"
};

E_API void *
e_modapi_init(E_Module *m)
{
   layout_module = m;

   hook = e_client_hook_add(E_CLIENT_HOOK_EVAL_POST_FETCH,
			    _e_module_layout_cb_hook, NULL);
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m)
{
   if (hook)
     {
	e_client_hook_del(hook);
	hook = NULL;
     }
   layout_module = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m)
{
   return 1;
}
