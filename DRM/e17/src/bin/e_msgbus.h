#ifdef E_TYPEDEFS

typedef struct _E_Msgbus_Data E_Msgbus_Data;

#else
#ifndef E_MSGBUS_H
#define E_MSGBUS_H

/* This is the dbus subsystem, but eldbus namespace is taken by eldbus */

struct _E_Msgbus_Data
{
   Eldbus_Connection *conn;
   Eldbus_Service_Interface *iface;
};

EINTERN int e_msgbus_init(void);
EINTERN int e_msgbus_shutdown(void);
E_API Eldbus_Service_Interface *e_msgbus_interface_attach(const Eldbus_Service_Interface_Desc *desc);

#endif
#endif
