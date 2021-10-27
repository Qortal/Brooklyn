#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include "e.h"

typedef struct _Config Config;
typedef struct _Config_Face Config_Face;
typedef struct _Tempthread Tempthread;

typedef enum _Unit
{
   CELSIUS,
   FAHRENHEIT
} Unit;

struct _Tempthread
{
   Config_Face *inst;
   int poll_interval;
   const char *sensor_name;
   E_Powersave_Sleeper *sleeper;
   Eina_Bool initted E_BITFIELD;
};

struct _Config_Face
{
   const char *id;
   /* saved * loaded config values */
   int poll_interval;
   int low, high;
   int sensor_type;
   int temp;
   const char *sensor_name;
   Unit units;
   /* config state */
   E_Gadcon_Client *gcc;
   Evas_Object *o_temp;
   E_Module *module;

   E_Config_Dialog *config_dialog;
   E_Menu *menu;
   Ecore_Thread *th;

   Eina_Bool have_temp E_BITFIELD;
};

struct _Config
{
   /* saved * loaded config values */
   Eina_Hash *faces;
   /* config state */
   E_Module *module;
};

E_API extern E_Module_Api e_modapi;

E_API void *e_modapi_init(E_Module *m);
E_API int e_modapi_shutdown(E_Module *m);
E_API int e_modapi_save(E_Module *m);

void config_temperature_module(Config_Face *inst);
void temperature_face_update_config(Config_Face *inst);

typedef struct
{
   const char *name;
   const char *label;
} Sensor;

int        temperature_tempget_get(Tempthread *tth);
Eina_List *temperature_tempget_sensor_list(void);
void       temperature_tempget_setup(void);
void       temperature_tempget_clear(void);

/**
 * @addtogroup Optional_Gadgets
 * @{
 *
 * @defgroup Module_Temperature Temperature
 *
 * Monitors computer temperature sensors and may do actions given some
 * thresholds.
 *
 * @see Module_CPUFreq
 * @}
 */
#endif
