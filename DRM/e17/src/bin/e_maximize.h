#ifdef E_TYPEDEFS
#else
#ifndef E_MAXIMIZE_H
#define E_MAXIMIZE_H

E_API void e_maximize_client_shelf_fit(const E_Client *ec, int *x1, int *y1, int *x2, int *y2, E_Maximize dir);
E_API void e_maximize_client_dock_fit(const E_Client *ec, int *x1, int *y1, int *x2, int *y2);
E_API void e_maximize_client_shelf_fill(const E_Client *ec, int *x1, int *y1, int *x2, int *y2, E_Maximize dir);
E_API void e_maximize_client_client_fill(const E_Client *ec, int *x1, int *y1, int *x2, int *y2, E_Maximize dir);

#endif
#endif
