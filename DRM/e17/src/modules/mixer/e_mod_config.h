#ifndef E_MOD_CONFIG_H
#define E_MOD_CONFIG_H

#include <e.h>

typedef void (*emix_config_backend_changed)(const char *backend, void *data);
typedef void (*emix_config_meter_changed)(Eina_Bool enable, void *data);

void emix_config_init(emix_config_backend_changed cb, const void *userdata);
void emix_config_shutdown(void);
void emix_config_save(void);
Eina_Bool emix_config_save_get(void);
void emix_config_save_state_get(void);
void emix_config_save_state_restore(void);
const char *emix_config_save_sink_get(void);
void emix_config_save_sink_set(const char *sink);

const char *emix_config_backend_get(void);
void emix_config_backend_set(const char *backend);
Eina_Bool emix_config_desklock_mute_get(void);
Eina_Bool emix_config_meter_get(void);
Eina_Bool emix_config_notify_get(void);
E_Config_Dialog* emix_config_popup_new(Evas_Object *comp, const char*p);

#endif
