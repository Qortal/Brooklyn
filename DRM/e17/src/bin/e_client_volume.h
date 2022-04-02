#ifndef E_CLIENT_VOLUME_H_
#define E_CLIENT_VOLUME_H_

typedef struct _E_Client_Volume_Sink E_Client_Volume_Sink;
typedef struct _E_Event_Client_Volume_Sink E_Event_Client_Volume_Sink;

typedef void (*E_Client_Volume_Sink_Get)(int *volume, Eina_Bool *mute, void *data);
typedef void (*E_Client_Volume_Sink_Set)(int volume, Eina_Bool mute, void *data);
typedef int (*E_Client_Volume_Sink_Min_Get)(void *data);
typedef int (*E_Client_Volume_Sink_Max_Get)(void *data);
typedef const char *(*E_Client_Volume_Sink_Name_Get)(void *data);

E_API extern int E_EVENT_CLIENT_VOLUME;
E_API extern int E_EVENT_CLIENT_MUTE;
E_API extern int E_EVENT_CLIENT_UNMUTE;
E_API extern int E_EVENT_CLIENT_VOLUME_SINK_ADD;
E_API extern int E_EVENT_CLIENT_VOLUME_SINK_DEL;
E_API extern int E_EVENT_CLIENT_VOLUME_SINK_CHANGED;

struct _E_Client_Volume_Sink
{
   E_Client_Volume_Sink_Get func_get;
   E_Client_Volume_Sink_Set func_set;
   E_Client_Volume_Sink_Min_Get func_min_get;
   E_Client_Volume_Sink_Max_Get func_max_get;
   E_Client_Volume_Sink_Name_Get func_name_get;
   void *data;
   Eina_List *clients;
};

struct _E_Event_Client_Volume_Sink
{
   E_Client *ec;
   E_Client_Volume_Sink *sink;
};


EINTERN int        e_client_volume_init(void);
EINTERN void       e_client_volume_shutdown(void);

E_API void         e_client_volume_set(E_Client *ec, int volume);
E_API void         e_client_volume_mute_set(E_Client *ec, Eina_Bool mute);

E_API Evas_Object *e_client_volume_object_add(E_Client *ec, Evas *evas);

E_API E_Client_Volume_Sink *e_client_volume_sink_new(E_Client_Volume_Sink_Get func_get, E_Client_Volume_Sink_Set func_set, E_Client_Volume_Sink_Min_Get func_min_get, E_Client_Volume_Sink_Max_Get func_max_get, E_Client_Volume_Sink_Name_Get func_name_get, void *data);
E_API void         e_client_volume_sink_del(E_Client_Volume_Sink *mixer);
E_API void         e_client_volume_sink_set(E_Client_Volume_Sink *mixer, int volume, Eina_Bool mute);
E_API void         e_client_volume_sink_get(const E_Client_Volume_Sink *mixer, int *volume, Eina_Bool *mute);
E_API int          e_client_volume_sink_min_get(const E_Client_Volume_Sink *mixer);
E_API int          e_client_volume_sink_max_get(const E_Client_Volume_Sink *mixer);
E_API const char * e_client_volume_sink_name_get(const E_Client_Volume_Sink *mixer);
E_API void         e_client_volume_sink_append(E_Client *ec, E_Client_Volume_Sink *mixer);
E_API void         e_client_volume_sink_remove(E_Client *ec, E_Client_Volume_Sink *mixer);
E_API void         e_client_volume_sink_update(E_Client_Volume_Sink *mixer);

E_API void         e_client_volume_display_set(E_Client *ec, int volume, Eina_Bool mute);
E_API void         e_client_volume_object_emit(E_Client *ec, const char *sig, const char *src);

#endif
