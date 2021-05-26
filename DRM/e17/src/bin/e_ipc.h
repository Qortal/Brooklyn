#ifdef E_TYPEDEFS

EINTERN extern char *e_ipc_socket;

#ifdef USE_IPC

typedef enum _E_Ipc_Domain
{
   E_IPC_DOMAIN_NONE,
   E_IPC_DOMAIN_SETUP,
   E_IPC_DOMAIN_REQUEST,
   E_IPC_DOMAIN_REPLY,
   E_IPC_DOMAIN_EVENT,
   E_IPC_DOMAIN_THUMB,
   E_IPC_DOMAIN_FM,
   E_IPC_DOMAIN_ALERT,
   E_IPC_DOMAIN_LAST
} E_Ipc_Domain;

typedef int E_Ipc_Op;
#endif

#else
#ifndef E_IPC_H
#define E_IPC_H

EINTERN int e_ipc_init(void);
EINTERN int e_ipc_shutdown(void);

#endif
#endif
