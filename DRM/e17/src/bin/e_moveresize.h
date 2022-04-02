#ifdef E_TYPEDEFS

#else
#ifndef E_MOVERESIZE_H
#define E_MOVERESIZE_H

EINTERN int e_moveresize_init(void);
EINTERN int e_moveresize_shutdown(void);
E_API void e_moveresize_replace(Eina_Bool enable);
E_API void e_moveresize_client_extents(const E_Client *ec, int *w, int *h);
#endif
#endif
