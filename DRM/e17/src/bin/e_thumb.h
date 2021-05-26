#ifdef E_TYPEDEFS

#else
#ifndef E_THUMB_H
#define E_THUMB_H


EINTERN int                   e_thumb_init(void);
EINTERN int                   e_thumb_shutdown(void);

E_API Evas_Object          *e_thumb_icon_add(Evas *evas);
E_API void                  e_thumb_icon_file_set(Evas_Object *obj, const char *file, const char *key);
E_API void                  e_thumb_icon_size_set(Evas_Object *obj, int w, int h);
E_API void                  e_thumb_icon_begin(Evas_Object *obj);
E_API void                  e_thumb_icon_end(Evas_Object *obj);
E_API void                  e_thumb_icon_rethumb(Evas_Object *obj);
E_API void                  e_thumb_desk_pan_set(Evas_Object *obj, int x, int y, int x_count, int y_count);
E_API void                  e_thumb_signal_add(Evas_Object *obj, const char *sig, const char *src);
E_API const char           *e_thumb_sort_id_get(Evas_Object *obj);

E_API void                  e_thumb_client_data(Ecore_Ipc_Event_Client_Data *e);
E_API void                  e_thumb_client_del(Ecore_Ipc_Event_Client_Del *e);

#endif
#endif
