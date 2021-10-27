#ifdef E_TYPEDEFS

typedef enum {
   E_BG_TRANSITION_NONE,
     E_BG_TRANSITION_START,
     E_BG_TRANSITION_DESK,
     E_BG_TRANSITION_CHANGE
} E_Bg_Transition;

typedef struct _E_Event_Bg_Update E_Event_Bg_Update;

typedef struct _E_Bg_Image_Import_Handle E_Bg_Image_Import_Handle;

#else
#ifndef E_BG_H
#define E_BG_H

extern E_API int E_EVENT_BG_UPDATE;

struct _E_Event_Bg_Update
{
   int zone;
   int desk_x;
   int desk_y;
};

EINTERN int e_bg_init(void);
EINTERN int e_bg_shutdown(void);

E_API const E_Config_Desktop_Background *e_bg_config_get(int zone_num, int desk_x, int desk_y);
E_API Eina_Stringshare *e_bg_file_get(int zone_num, int desk_x, int desk_y);
E_API void e_bg_zone_update(E_Zone *zone, E_Bg_Transition transition);
E_API void e_bg_add(int zone, int desk_x, int desk_y, const char *file);
E_API void e_bg_del(int zone, int desk_x, int desk_y);
E_API void e_bg_default_set(const char *file);
E_API void e_bg_update(void);

E_API E_Bg_Image_Import_Handle *e_bg_image_import_new(const char *image_file, void (*cb)(void *data, const char *edje_file), const void *data);
E_API void                      e_bg_image_import_cancel(E_Bg_Image_Import_Handle *handle);

#endif
#endif
