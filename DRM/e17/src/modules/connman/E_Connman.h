#ifndef E_CONNMAN_H
#define E_CONNMAN_H

#include "e.h"
#include <stdbool.h>
#include <Eldbus.h>

typedef struct _E_Connman_Agent E_Connman_Agent;

enum Connman_State
{
   CONNMAN_STATE_NONE = -1, /* All unknown states */
   CONNMAN_STATE_OFFLINE,
   CONNMAN_STATE_IDLE,
   CONNMAN_STATE_ASSOCIATION,
   CONNMAN_STATE_CONFIGURATION,
   CONNMAN_STATE_READY,
   CONNMAN_STATE_ONLINE,
   CONNMAN_STATE_DISCONNECT,
   CONNMAN_STATE_FAILURE,
};

enum Connman_Service_Type
{
   CONNMAN_SERVICE_TYPE_NONE = -1, /* All non-supported types */
   CONNMAN_SERVICE_TYPE_ETHERNET,
   CONNMAN_SERVICE_TYPE_WIFI,
   CONNMAN_SERVICE_TYPE_BLUETOOTH,
   CONNMAN_SERVICE_TYPE_CELLULAR,
};

struct Connman_Manager
{
   const char *path;
   Eldbus_Proxy *technology_iface;
   Eldbus_Proxy *manager_iface;

   Eina_Inlist *services; /* The prioritized list of services */

   /* Properties */
   enum Connman_State state;
   bool offline_mode;
   bool powered;

   /* Private */
   struct
     {
        Eldbus_Pending *get_services;
        Eldbus_Pending *get_wifi_properties;
        Eldbus_Pending *set_powered;
     } pending;
};

struct Connman_Service
{
   const char *path;
   Eldbus_Proxy *service_iface;
   EINA_INLIST;

   /* Properties */
   char *name;
   Eina_Array *security;
   enum Connman_State state;
   enum Connman_Service_Type type;
   uint8_t strength;

   /* Private */
   struct
     {
        Eldbus_Pending *connect;
        Eldbus_Pending *disconnect;
        Eldbus_Pending *remov;
        void *data;
     } pending;
};

/* Ecore Events */
extern int E_CONNMAN_EVENT_MANAGER_IN;
extern int E_CONNMAN_EVENT_MANAGER_OUT;


/* Daemon monitoring */
unsigned int e_connman_system_init(Eldbus_Connection *eldbus_conn) EINA_ARG_NONNULL(1);
unsigned int e_connman_system_shutdown(void);
void         e_connman_scan(struct Connman_Manager *cm);


/* Requests from UI */

/**
 * Find service using a non-stringshared path
 */
struct Connman_Service *econnman_manager_find_service(struct Connman_Manager *cm, const char *path) EINA_ARG_NONNULL(1, 2);

typedef void (*Econnman_Simple_Cb)(void *data, const char *error);

bool econnman_service_connect(struct Connman_Service *cs, Econnman_Simple_Cb cb, void *data);
bool econnman_service_disconnect(struct Connman_Service *cs, Econnman_Simple_Cb cb, void *data);
bool econnman_service_remove(struct Connman_Service *cs, Econnman_Simple_Cb cb, void *data);

void econnman_powered_set(struct Connman_Manager *cm, Eina_Bool powered);

/* UI calls from econnman */

/*
 * TODO: transform these in proper callbacks or ops that UI calls to register
 * itself
 */

void econnman_mod_manager_update(struct Connman_Manager *cm);
void econnman_mod_manager_inout(struct Connman_Manager *cm);
void econnman_mod_services_changed(struct Connman_Manager *cm);

/* Util */

const char *econnman_state_to_str(enum Connman_State state);
const char *econnman_service_type_to_str(enum Connman_Service_Type type);

/* Log */
extern int _e_connman_log_dom;

#undef DBG
#undef INF
#undef WRN
#undef ERR

#define DBG(...) EINA_LOG_DOM_DBG(_e_connman_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_e_connman_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_e_connman_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_e_connman_log_dom, __VA_ARGS__)

#endif /* E_CONNMAN_H */
