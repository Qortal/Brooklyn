#ifdef E_TYPEDEFS

#else
#ifndef E_DATASTORE_H
#define E_DATASTORE_H

E_API void        e_datastore_set(char *key, void *data);
E_API void       *e_datastore_get(char *key);
E_API void        e_datastore_del(char *key);

#endif
#endif
