#include "e.h"

/* local subsystem functions */
static Eina_Bool _e_focus_raise_timer(void *data);

/* local subsystem globals */

/* externally accessible functions */
E_API void
e_focus_event_mouse_in(E_Client *ec)
{

   if ((e_config->focus_policy == E_FOCUS_MOUSE) ||
       (e_config->focus_policy == E_FOCUS_SLOPPY))
     {
        evas_object_focus_set(ec->frame, 1);
     }
   E_FREE_FUNC(ec->raise_timer, ecore_timer_del);
   if (e_config->use_auto_raise)
     {
        if (eina_dbl_exact(e_config->auto_raise_delay, 0.0))
          {
             if (!ec->lock_user_stacking)
               evas_object_raise(ec->frame);
          }
        else
          ec->raise_timer = ecore_timer_loop_add(e_config->auto_raise_delay, _e_focus_raise_timer, ec);
     }
}

E_API void
e_focus_event_mouse_out(E_Client *ec)
{
   if (e_config->focus_policy == E_FOCUS_MOUSE)
     {
        if (!ec->lock_focus_in)
          {
             if (ec->focused)
               evas_object_focus_set(ec->frame, 0);
          }
     }
   E_FREE_FUNC(ec->raise_timer, ecore_timer_del);
}

E_API void
e_focus_event_mouse_down(E_Client *ec)
{
   if (e_client_focus_policy_click(ec) ||
       e_config->always_click_to_focus)
     evas_object_focus_set(ec->frame, 1);
   if (e_config->always_click_to_raise)
     {
        if (!ec->lock_user_stacking)
          evas_object_raise(ec->frame);
     }
}

E_API void
e_focus_event_mouse_up(E_Client *ec EINA_UNUSED)
{
}

E_API void
e_focus_event_focus_in(E_Client *ec EINA_UNUSED)
{
}

E_API void
e_focus_event_focus_out(E_Client *ec EINA_UNUSED)
{
}

/* local subsystem functions */
static Eina_Bool
_e_focus_raise_timer(void *data)
{
   E_Client *ec = data;

   if (!ec->lock_user_stacking) evas_object_raise(ec->frame);
   ec->raise_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

