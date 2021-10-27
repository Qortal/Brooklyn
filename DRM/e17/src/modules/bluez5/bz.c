#include "e_mod_main.h"

Eldbus_Connection *bz_conn = NULL;

static Ecore_Timer *owner_gain_timer = NULL;

/////////////////////////////////////////////////////////////////////////////

static Eina_Bool
cb_name_owner_new(void *data EINA_UNUSED)
{
   owner_gain_timer = NULL;
   bz_obj_init();
   bz_agent_init();
   return EINA_FALSE;
}

static void
cb_name_owner_changed(void *data EINA_UNUSED,
                      const char *bus EINA_UNUSED,
                      const char *from EINA_UNUSED,
                      const char *to)
{
   static Eina_Bool first = EINA_TRUE;
   if (to[0])
     {
        if (owner_gain_timer) ecore_timer_del(owner_gain_timer);
        // on first start try and re-init quickly because we get a name
        // owner change even if all is good when we register to listen for it,
        // so start fast
        if (first)
          owner_gain_timer = ecore_timer_add(0.1, cb_name_owner_new, NULL);
        // but if we gegt a name owner change later it's probably because
        // bluez was restarted or crashed. a new bz daemon will (or should)
        // come up. so re-init more slowly here giving the daemon some time
        // to come up before pestering it.
        else
          owner_gain_timer = ecore_timer_add(1.0, cb_name_owner_new, NULL);
        first = EINA_FALSE;
     }
   else
     {
        if (owner_gain_timer) ecore_timer_del(owner_gain_timer);
        owner_gain_timer = NULL;
        ebluze5_popup_clear();
        bz_agent_shutdown();
        bz_obj_shutdown();
     }
}

static void
cb_obj_add(Obj *o)
{
   if (o->type == BZ_OBJ_ADAPTER)
     {
        o->fn_change = ebluez5_popup_adapter_change;
        o->fn_del = ebluez5_popup_adapter_del;
        ebluez5_popup_adapter_add(o);
        return;
     }
   if (o->type == BZ_OBJ_DEVICE)
     {
        o->fn_change = ebluez5_popup_device_change;
        o->fn_del = ebluez5_popup_device_del;
        ebluez5_popup_device_add(o);
        return;
     }
}

void
bz_init(void)
{
   bz_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   bz_obj_add_func_set(cb_obj_add);
   bz_agent_release_func_set(ebluez5_agent_agent_release);
   bz_agent_cancel_func_set(ebluez5_agent_agent_cancel);
   bz_agent_req_pin_func_set(ebluez5_agent_agent_req_pin);
   bz_agent_disp_pin_func_set(ebluez5_agent_agent_disp_pin);
   bz_agent_req_pass_func_set(ebluez5_agent_req_pass);
   bz_agent_disp_pass_func_set(ebluez5_agent_disp_pass);
   bz_agent_req_confirm_func_set(ebluez5_agent_req_confirm);
   bz_agent_req_auth_func_set(ebluez5_agent_req_auth);
   bz_agent_auth_service_func_set(ebluez5_agent_auth_service);
   eldbus_name_owner_changed_callback_add(bz_conn, "org.bluez",
                                          cb_name_owner_changed,
                                          NULL, EINA_TRUE);
}

void
bz_shutdown(void)
{
   eldbus_name_owner_changed_callback_del(bz_conn, "org.bluez",
                                          cb_name_owner_changed, NULL);
   bz_agent_release_func_set(NULL);
   bz_agent_cancel_func_set(NULL);
   bz_agent_req_pin_func_set(NULL);
   bz_agent_disp_pin_func_set(NULL);
   bz_agent_req_pass_func_set(NULL);
   bz_agent_disp_pass_func_set(NULL);
   bz_agent_req_confirm_func_set(NULL);
   bz_agent_req_auth_func_set(NULL);
   bz_agent_auth_service_func_set(NULL);
   bz_obj_add_func_set(NULL);
   ebluze5_popup_clear();
   bz_agent_shutdown();
   bz_obj_shutdown();
   eldbus_connection_unref(bz_conn);
   bz_conn = NULL;
}
