#include "e_mod_main.h"

/* Config function protos */
static Config *_notification_cfg_new(void);
static void    _notification_cfg_free(Config *cfg);

/* Global variables */
E_Module *notification_mod = NULL;
Config *notification_cfg = NULL;

static E_Config_DD *conf_edd = NULL;

static unsigned int
_notification_notify(E_Notification_Notify *n)
{
   unsigned int new_id;

   if (e_desklock_state_get()) return 0;

   notification_cfg->next_id++;
   new_id = notification_cfg->next_id;
   notification_popup_notify(n, new_id);

   return new_id;
}

/* Module Api Functions */
E_API E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Notification"};

static const E_Notification_Server_Info server_info = {
   .name = "e17",
   .vendor = "enlightenment.org",
   .version = "0.17",
   .spec_version = "1.2",
   .capabilities = { "body", "body-markup", NULL }
};

/* Callbacks */
static unsigned int
_notification_cb_notify(void *data EINA_UNUSED, E_Notification_Notify *n)
{
   return _notification_notify(n);
}

static void
_notification_cb_close(void *data EINA_UNUSED, unsigned int id)
{
   notification_popup_close(id);
}

E_API void *
e_modapi_init(E_Module *m)
{
   /* register config panel entry */
   e_configure_registry_category_add("extensions", 90, _("Extensions"), NULL,
                                     "preferences-extensions");
   e_configure_registry_item_add("extensions/notification", 30,
                                 _("Notification"), NULL,
                                 "preferences-notification",
                                 e_int_config_notification_module);


   conf_edd = E_CONFIG_DD_NEW("Notification_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, show_low, INT);
   E_CONFIG_VAL(D, T, show_normal, INT);
   E_CONFIG_VAL(D, T, show_critical, INT);
   E_CONFIG_VAL(D, T, corner, INT);
   E_CONFIG_VAL(D, T, timeout, FLOAT);
   E_CONFIG_VAL(D, T, force_timeout, INT);
   E_CONFIG_VAL(D, T, ignore_replacement, INT);
   E_CONFIG_VAL(D, T, dual_screen, INT);

   notification_cfg = e_config_domain_load("module.notification", conf_edd);
   if (notification_cfg &&
       !(e_util_module_config_check(_("Notification Module"),
                                    notification_cfg->version,
                                    MOD_CONFIG_FILE_VERSION)))
     {
        _notification_cfg_free(notification_cfg);
        notification_cfg = NULL;
     }

   if (!notification_cfg)
     notification_cfg = _notification_cfg_new();
   /* upgrades */
   if (notification_cfg->version - (MOD_CONFIG_FILE_EPOCH * 1000000) < 1)
     {
        if (notification_cfg->dual_screen) notification_cfg->dual_screen = POPUP_DISPLAY_POLICY_MULTI;
     }
   notification_cfg->version = MOD_CONFIG_FILE_VERSION;

   /* set up the notification daemon */
   if (!e_notification_server_register(&server_info, _notification_cb_notify,
                                       _notification_cb_close, NULL))
     {
        e_util_dialog_show(_("Error during notification server initialization"),
                           _("Ensure there's no other module acting as a server "
                             "and that D-Bus is correctly installed and "
                             "running"));
        return NULL;
     }

   notification_mod = m;

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   if (notification_cfg->cfd) e_object_del(E_OBJECT(notification_cfg->cfd));
   e_configure_registry_item_del("extensions/notification");
   e_configure_registry_category_del("extensions");

   notification_popup_shutdown();
   e_notification_server_unregister();

   _notification_cfg_free(notification_cfg);
   E_CONFIG_DD_FREE(conf_edd);
   notification_mod = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return e_config_domain_save("module.notification", conf_edd, notification_cfg);
}

static Config *
_notification_cfg_new(void)
{
   Config *cfg;

   cfg = E_NEW(Config, 1);
   cfg->cfd = NULL;
   cfg->version = MOD_CONFIG_FILE_VERSION;
   cfg->show_low = 0;
   cfg->show_normal = 1;
   cfg->show_critical = 1;
   cfg->timeout = 5.0;
   cfg->force_timeout = 0;
   cfg->ignore_replacement = 0;
   cfg->dual_screen = POPUP_DISPLAY_POLICY_FIRST;
   cfg->corner = CORNER_TR;

   return cfg;
}

static void
_notification_cfg_free(Config *cfg)
{
   free(cfg);
}
