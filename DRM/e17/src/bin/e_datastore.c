#include "e.h"

/* local subsystem functions */

/* local subsystem globals */
static Eina_Hash *store = NULL;

/* externally accessible functions */
E_API void
e_datastore_set(char *key, void *data)
{
   if (!store) store = eina_hash_string_superfast_new(NULL);
   eina_hash_del(store, key, NULL);
   eina_hash_add(store, key, data);
}

E_API void *
e_datastore_get(char *key)
{
   return eina_hash_find(store, key);
}

E_API void
e_datastore_del(char *key)
{
   eina_hash_del(store, key, NULL);
   if (eina_hash_population(store)) return;
   eina_hash_free(store);
   store = NULL;
}

/* local subsystem functions */
