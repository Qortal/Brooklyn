#include "e.h"
#include "e_mod_main.h"
#include "e_kbd_int.h"
#include "e_kbd_send.h"

/* local function prototypes */
static void _il_kbd_stop(void);
static void _il_kbd_start(void);
static Eina_Bool _il_ki_delay_cb(void *data EINA_UNUSED);

/* local variables */
static E_Kbd_Int *ki = NULL;
static Ecore_Timer *ki_delay_timer = NULL;
static E_Config_DD *cd = NULL;

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Virtual Keyboard" };

Il_Kbd_Config *il_kbd_cfg = NULL;

static void
_cb_act_vkbd_show(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (ki) e_kbd_int_show(ki);
}

static void
_cb_act_vkbd_hide(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (ki) e_kbd_int_hide(ki);
}

static void
_cb_act_vkbd_toggle(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (ki)
     {
        if (ki->visible) e_kbd_int_hide(ki);
        else e_kbd_int_show(ki);
     }
}

E_API void *
e_modapi_init(E_Module *m)
{
   cd = E_CONFIG_DD_NEW("Il_Kbd_Config", Il_Kbd_Config);
   E_CONFIG_VAL(cd, Il_Kbd_Config, dict, STR);
   E_CONFIG_VAL(cd, Il_Kbd_Config, zone_id, STR);
   E_CONFIG_VAL(cd, Il_Kbd_Config, zone_num, INT);
   E_CONFIG_VAL(cd, Il_Kbd_Config, size, DOUBLE);
   E_CONFIG_VAL(cd, Il_Kbd_Config, fill_mode, INT);
   E_CONFIG_VAL(cd, Il_Kbd_Config, px, DOUBLE);
   E_CONFIG_VAL(cd, Il_Kbd_Config, py, DOUBLE);

   il_kbd_cfg = e_config_domain_load("module.vkbd", cd);
   if (!il_kbd_cfg)
     {
        il_kbd_cfg = E_NEW(Il_Kbd_Config, 1);
        il_kbd_cfg->dict = eina_stringshare_add("English_US.dic");
        il_kbd_cfg->zone_num = 0;
        il_kbd_cfg->size = 4.0;
        il_kbd_cfg->fill_mode = 0;
     }

   il_kbd_cfg->mod_dir = eina_stringshare_add(m->dir);
   il_kbd_cfg->slide_dim = 15;
   il_kbd_cfg->act_kbd_show = e_action_add("vkbd_show");
   if (il_kbd_cfg->act_kbd_show)
     {
        il_kbd_cfg->act_kbd_show->func.go = _cb_act_vkbd_show;
        e_action_predef_name_set("Virtual Keyboard", _("Show"),
                                 "vkbd_show", NULL, NULL, 0);
     }
   il_kbd_cfg->act_kbd_hide = e_action_add("vkbd_hide");
   if (il_kbd_cfg->act_kbd_hide)
     {
        il_kbd_cfg->act_kbd_hide->func.go = _cb_act_vkbd_hide;
        e_action_predef_name_set("Virtual Keyboard", _("Hide"),
                                 "vkbd_hide", NULL, NULL, 0);
     }
   il_kbd_cfg->act_kbd_toggle = e_action_add("vkbd_toggle");
   if (il_kbd_cfg->act_kbd_toggle)
     {
        il_kbd_cfg->act_kbd_toggle->func.go = _cb_act_vkbd_toggle;
        e_action_predef_name_set("Virtual Keyboard", _("Toggle"),
                                 "vkbd_toggle", NULL, NULL, 0);
     }

   ki_delay_timer = ecore_timer_add(1.0, _il_ki_delay_cb, NULL);
   e_kbd_send_init();
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_kbd_send_shutdown();
   e_config_domain_save("module.vkbd", cd, il_kbd_cfg);
   if (ki_delay_timer)
     {
        ecore_timer_del(ki_delay_timer);
        ki_delay_timer = NULL;
     }
   _il_kbd_stop();
   eina_stringshare_del(il_kbd_cfg->dict);
   eina_stringshare_del(il_kbd_cfg->mod_dir);

   if (il_kbd_cfg->act_kbd_show)
     {
        e_action_predef_name_del("Virtual Keyboard", _("Show"));
        e_action_del("vkbd_show");
     }
   if (il_kbd_cfg->act_kbd_hide)
     {
        e_action_predef_name_del("Virtual Keyboard", _("Hide"));
        e_action_del("vkbd_hide");
     }
   if (il_kbd_cfg->act_kbd_toggle)
     {
        e_action_predef_name_del("Virtual Keyboard", _("Toggle"));
        e_action_del("vkbg_toggle");
     }
   E_FREE(il_kbd_cfg);
   E_CONFIG_DD_FREE(cd);
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return e_config_domain_save("module.vkbd", cd, il_kbd_cfg);
}

static void
_il_kbd_stop(void)
{
   if (ki) e_kbd_int_free(ki);
   ki = NULL;
}

static void
_il_kbd_start(void)
{
   ki = e_kbd_int_new(il_kbd_cfg->zone_num,
                      il_kbd_cfg->zone_id,
                      il_kbd_cfg->mod_dir,
                      il_kbd_cfg->mod_dir,
                      il_kbd_cfg->mod_dir);
   ki->px = il_kbd_cfg->px;
   ki->py = il_kbd_cfg->py;
   e_kbd_int_update(ki);
   e_kbd_int_show(ki);
}

static Eina_Bool
_il_ki_delay_cb(void *data EINA_UNUSED)
{
   ki_delay_timer = NULL;
   _il_kbd_start();
   return EINA_FALSE;
}
