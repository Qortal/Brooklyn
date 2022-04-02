#ifndef E_AUTH_H
#define E_AUTH_H

E_API int e_auth_begin(char *passwd);
E_API int e_auth_polkit_begin(char *passwd, const char *cookie, unsigned int uid);

static inline int
e_auth_hash_djb2(const char *key, int len)
{
   unsigned int hash_num = 5381;
   const unsigned char *ptr;

   if (!key) return 0;
   for (ptr = (unsigned char *)key; len; ptr++, len--)
     hash_num = ((hash_num << 5) + hash_num) ^ *ptr; /* hash * 33 ^ c */

   return (int)hash_num;
}

#endif
