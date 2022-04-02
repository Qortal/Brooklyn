#include <Eina.h>
#include <Eet.h>
#include "e_macros.h"
#define E_TYPEDEFS
#include "e_config_data.h"
#undef E_TYPEDEFS
#include "e_config_data.h"

static Eina_Hash *config_hash = NULL;

E_API void
e_config_descriptor_free(E_Config_DD *edd)
{
   eina_hash_del_by_key(config_hash, eet_data_descriptor_name_get((Eet_Data_Descriptor*)edd));
   eet_data_descriptor_free((Eet_Data_Descriptor*)edd);
}

E_API E_Config_DD *
e_config_descriptor_new(const char *name, int size)
{
   Eet_Data_Descriptor_Class eddc;
   E_Config_DD *edd;

   if (!eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), name, size))
     return NULL;

   /* FIXME: We can directly map string inside an Eet_File and reuse it.
      But this need a break in all user of config every where in E.
    */

   edd = (E_Config_DD *)eet_data_descriptor_stream_new(&eddc);

   if (!config_hash) config_hash = eina_hash_string_superfast_new(NULL);
   eina_hash_set(config_hash, name, edd);
   return edd;
}

E_API E_Config_DD *
e_config_descriptor_find(const char *name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   return eina_hash_find(config_hash, name);
}
