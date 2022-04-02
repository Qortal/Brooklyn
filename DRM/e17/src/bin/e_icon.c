#include "e.h"

typedef struct _E_Smart_Data E_Smart_Data;
typedef struct _Cache_Item   Cache_Item;
typedef struct _Cache        Cache;

struct _E_Smart_Data
{
   Evas_Coord     x, y, w, h;
   Evas_Object   *obj;
   Evas_Object   *eventarea;
   const char    *fdo;
   const char    *url;
   const char    *tmp_file;
   Ecore_Con_Url       *con_url;
   Ecore_Event_Handler *url_hand;
   Ecore_Timer   *guessing_animation;
   Ecore_Timer   *timer, *fdo_reload_timer;
   double         last_resize;
   int            size;
   int            frame, frame_count;
   int            tmp_fd;
   unsigned char  fill_inside E_BITFIELD;
   unsigned char  scale_up E_BITFIELD;
   unsigned char  preload E_BITFIELD;
   unsigned char  loading E_BITFIELD;
   unsigned char  animated E_BITFIELD;
   unsigned char  invalid E_BITFIELD;
   unsigned char  edje E_BITFIELD;
};

struct _Cache_Item
{
   unsigned int timestamp;

   Evas_Object *icon, *obj;
   const char  *id;
   Eina_List   *objs;
};

struct _Cache
{
   Eina_Hash   *hash;

   char        *file;
   Eet_File    *ef;
   Ecore_Timer *timer;
   Eina_List   *load_queue;
};

/* local subsystem functions */
static void      _e_icon_smart_reconfigure(E_Smart_Data *sd);
static void      _e_icon_smart_init(void);
static void      _e_icon_smart_add(Evas_Object *obj);
static void      _e_icon_smart_del(Evas_Object *obj);
static void      _e_icon_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void      _e_icon_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void      _e_icon_smart_show(Evas_Object *obj);
static void      _e_icon_smart_hide(Evas_Object *obj);
static void      _e_icon_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void      _e_icon_smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void      _e_icon_smart_clip_unset(Evas_Object *obj);
static void      _e_icon_obj_prepare(Evas_Object *obj, E_Smart_Data *sd);
static void      _e_icon_preloaded(void *data, Evas *e, Evas_Object *obj, void *event_info);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

EINTERN int
e_icon_init(void)
{
   return 1;
}

EINTERN int
e_icon_shutdown(void)
{
   return 1;
}

/* externally accessible functions */
E_API Evas_Object *
e_icon_add(Evas *evas)
{
   _e_icon_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

static void
_e_icon_obj_prepare(Evas_Object *obj, E_Smart_Data *sd)
{
   if (!sd->obj) return;

   if (!sd->edje)
     {
        Evas_Object *pclip;

        pclip = evas_object_clip_get(sd->obj);
        evas_object_del(sd->obj);
        sd->obj = evas_object_image_add(evas_object_evas_get(obj));
        if (!sd->animated)
          evas_object_image_scale_hint_set(sd->obj,
                                           EVAS_IMAGE_SCALE_HINT_STATIC);
        evas_object_smart_member_add(sd->obj, obj);
        evas_object_stack_below(sd->obj, sd->eventarea);
        evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_IMAGE_PRELOADED,
                                       _e_icon_preloaded, obj);
        evas_object_clip_set(sd->obj, pclip);
     }
}

static Eina_Bool
_frame_anim(void *data)
{
   E_Smart_Data *sd = data;
   double t;
   int fr;

   sd->frame++;
   fr = (sd->frame % (sd->frame_count)) + 1;
   evas_object_image_animated_frame_set(sd->obj, fr);
   t = evas_object_image_animated_frame_duration_get(sd->obj, fr, 0);
   sd->timer = ecore_timer_loop_add(t, _frame_anim, sd);
   return EINA_FALSE;
}

static int
_handle_anim(E_Smart_Data *sd)
{
   double t;

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   sd->frame = 0;
   sd->frame_count = 0;
   if (!evas_object_image_animated_get(sd->obj)) return 0;
   sd->frame_count = evas_object_image_animated_frame_count_get(sd->obj);
   if (sd->frame_count < 2) return 0;
   if (!sd->invalid) evas_object_show(sd->obj);
   t = evas_object_image_animated_frame_duration_get(sd->obj, sd->frame, 0);
   sd->timer = ecore_timer_loop_add(t, _frame_anim, sd);
   return 1;
}

static Eina_Bool
_url_complete_cb(void *data, int type EINA_UNUSED, void *event_info)
{
   Evas_Object *obj = data;
   E_Smart_Data *sd = evas_object_smart_data_get(obj);
   Ecore_Con_Event_Url_Complete *ev = event_info;

   if (!sd) return EINA_TRUE;
   if (ev->url_con != sd->con_url) return EINA_TRUE;
   e_icon_file_set(obj, sd->tmp_file);
   return EINA_FALSE;
}

E_API void
e_icon_file_set(Evas_Object *obj, const char *file)
{
   E_Smart_Data *sd;
   int len;

   if (!file) return;
   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR();
   if (!(sd = evas_object_smart_data_get(obj))) return;

   len = strlen(file);
   if ((len > 4) && (!strcasecmp(file + len - 4, ".edj")))
     {
        e_icon_file_edje_set(obj, file, "icon");
        return;
     }

   if ((sd->url) && (!strcmp(sd->url, file))) return;
   /* smart code here */
   _e_icon_obj_prepare(obj, sd);
   /* FIXME: 64x64 - unhappy about this. use icon size */
   sd->loading = 0;
   if (sd->fdo)
     {
        eina_stringshare_del(sd->fdo);
        sd->fdo = NULL;
     }
   if (sd->url)
     {
        eina_stringshare_del(sd->url);
        sd->url = NULL;
     }
   if (sd->con_url)
     {
        ecore_con_url_free(sd->con_url);
        sd->con_url = NULL;
     }
   if (sd->tmp_file)
     {
        if (!!strcmp(file, sd->tmp_file))
          {
             // if we set a file that is not the tmp file - delete tmp file
             ecore_file_unlink(sd->tmp_file);
             eina_stringshare_del(sd->tmp_file);
             sd->tmp_file = NULL;
          }
     }
   if (sd->tmp_fd >= 0)
     {
        close(sd->tmp_fd);
        sd->tmp_fd = -1;
     }
   if (sd->url_hand)
     {
        ecore_event_handler_del(sd->url_hand);
        sd->url_hand = NULL;
     }

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->guessing_animation) ecore_timer_del(sd->guessing_animation);
   sd->guessing_animation = NULL;
   sd->frame = 0;
   sd->frame_count = 0;
   sd->invalid = 0;
   sd->edje = EINA_FALSE;

   if (sd->size != 0)
     evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   if (sd->preload) evas_object_hide(sd->obj);

   if (sd->preload)
     evas_object_image_load_head_skip_set(sd->obj, EINA_TRUE);
   if ((!strncasecmp(file, "http://", 7)) ||
       (!strncasecmp(file, "https://", 8)))
     {
        char buf[4096];

        snprintf(buf, sizeof(buf), "/tmp/.e-icon-tmp-XXXXXX");
        sd->tmp_fd = mkstemp(buf);
        if (sd->tmp_fd >= 0)
          {
             sd->url = eina_stringshare_add(file);
             sd->tmp_file = eina_stringshare_add(buf);
             sd->con_url = ecore_con_url_new(file);
             ecore_con_url_fd_set(sd->con_url, sd->tmp_fd);
             if (!ecore_con_url_get(sd->con_url))
               {
                  ecore_file_unlink(buf);
                  close(sd->tmp_fd);
                  ecore_con_url_free(sd->con_url);
                  sd->tmp_fd = -1;
                  sd->con_url = NULL;
               }
             else
               {
                  sd->url_hand = ecore_event_handler_add
                    (ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, obj);
               }
          }
        return;
     }
   else
     evas_object_image_file_set(sd->obj, file, NULL);
//   if (evas_object_image_load_error_get(sd->obj) != EVAS_LOAD_ERROR_NONE)
//     return EINA_FALSE;

   if (!_handle_anim(sd))
     {
        if (sd->preload)
          {
             sd->loading = 1;
             evas_object_image_preload(sd->obj, EINA_FALSE);
          }
        else if (evas_object_visible_get(obj))
          {
             evas_object_show(sd->obj);
          }
     }
   _e_icon_smart_reconfigure(sd);
}

E_API void
e_icon_file_key_set(Evas_Object *obj, const char *file, const char *key)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR();
   if (!(sd = evas_object_smart_data_get(obj))) return;

   /* smart code here */
   sd->loading = 0;
   if (sd->fdo)
     {
        eina_stringshare_del(sd->fdo);
        sd->fdo = NULL;
     }
   if (sd->url)
     {
        eina_stringshare_del(sd->url);
        sd->url = NULL;
     }
   if (sd->con_url)
     {
        ecore_con_url_free(sd->con_url);
        sd->con_url = NULL;
     }
   if (sd->tmp_file)
     {
        ecore_file_unlink(sd->tmp_file);
        eina_stringshare_del(sd->tmp_file);
        sd->tmp_file = NULL;
     }
   if (sd->tmp_fd >= 0)
     {
        close(sd->tmp_fd);
        sd->tmp_fd = -1;
     }
   if (sd->url_hand)
     {
        ecore_event_handler_del(sd->url_hand);
        sd->url_hand = NULL;
     }

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->guessing_animation) ecore_timer_del(sd->guessing_animation);
   sd->guessing_animation = NULL;
   sd->frame = 0;
   sd->frame_count = 0;
   sd->invalid = 0;
   sd->edje = EINA_FALSE;

   _e_icon_obj_prepare(obj, sd);
   if (sd->size != 0)
     evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   if (sd->preload) evas_object_hide(sd->obj);
   if (sd->preload)
     evas_object_image_load_head_skip_set(sd->obj, EINA_TRUE);
   evas_object_image_file_set(sd->obj, file, key);
//   if (evas_object_image_load_error_get(sd->obj) != EVAS_LOAD_ERROR_NONE)
//     return EINA_FALSE;
   if (!_handle_anim(sd))
     {
        if (sd->preload)
          {
             sd->loading = 1;
             evas_object_image_preload(sd->obj, EINA_FALSE);
          }
        else if (evas_object_visible_get(obj))
          evas_object_show(sd->obj);
     }
   _e_icon_smart_reconfigure(sd);
}

E_API void
e_icon_edje_object_set(Evas_Object *obj, Evas_Object *edje)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR();
   if (!(sd = evas_object_smart_data_get(obj))) return;

   /* smart code here */
   if (sd->obj) evas_object_del(sd->obj);
   sd->loading = 0;
   if (sd->fdo)
     {
        eina_stringshare_del(sd->fdo);
        sd->fdo = NULL;
     }
   if (sd->url)
     {
        eina_stringshare_del(sd->url);
        sd->url = NULL;
     }
   if (sd->con_url)
     {
        ecore_con_url_free(sd->con_url);
        sd->con_url = NULL;
     }
   if (sd->tmp_file)
     {
        ecore_file_unlink(sd->tmp_file);
        eina_stringshare_del(sd->tmp_file);
        sd->tmp_file = NULL;
     }
   if (sd->tmp_fd >= 0)
     {
        close(sd->tmp_fd);
        sd->tmp_fd = -1;
     }
   if (sd->url_hand)
     {
        ecore_event_handler_del(sd->url_hand);
        sd->url_hand = NULL;
     }

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->guessing_animation) ecore_timer_del(sd->guessing_animation);
   sd->guessing_animation = NULL;
   sd->frame = 0;
   sd->frame_count = 0;
   sd->invalid = 0;
   sd->edje = EINA_TRUE;
   sd->obj = edje;

   if (evas_object_visible_get(obj)) evas_object_show(sd->obj);
   evas_object_smart_member_add(sd->obj, obj);
   _e_icon_smart_reconfigure(sd);
}

E_API Evas_Object *
e_icon_edje_get(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(NULL);
   if (!(sd = evas_object_smart_data_get(obj)))
     return NULL;
   return sd->edje ? sd->obj : NULL;
}

E_API void
e_icon_file_edje_set(Evas_Object *obj, const char *file, const char *part)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR();
   if (!(sd = evas_object_smart_data_get(obj))) return;

   /* smart code here */
   if (sd->obj) evas_object_del(sd->obj);
   sd->loading = 0;
   if (sd->fdo)
     {
        eina_stringshare_del(sd->fdo);
        sd->fdo = NULL;
     }
   if (sd->url)
     {
        eina_stringshare_del(sd->url);
        sd->url = NULL;
     }
   if (sd->con_url)
     {
        ecore_con_url_free(sd->con_url);
        sd->con_url = NULL;
     }
   if (sd->tmp_file)
     {
        ecore_file_unlink(sd->tmp_file);
        eina_stringshare_del(sd->tmp_file);
        sd->tmp_file = NULL;
     }
   if (sd->tmp_fd >= 0)
     {
        close(sd->tmp_fd);
        sd->tmp_fd = -1;
     }
   if (sd->url_hand)
     {
        ecore_event_handler_del(sd->url_hand);
        sd->url_hand = NULL;
     }

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->guessing_animation) ecore_timer_del(sd->guessing_animation);
   sd->guessing_animation = NULL;
   sd->frame = 0;
   sd->frame_count = 0;
   sd->invalid = 0;
   sd->edje = EINA_TRUE;

   sd->obj = edje_object_add(evas_object_evas_get(obj));
   edje_object_file_set(sd->obj, file, part);
//   if (edje_object_load_error_get(sd->obj) != EDJE_LOAD_ERROR_NONE)
//     return EINA_FALSE;
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_stack_below(sd->obj, sd->eventarea);
   if (evas_object_visible_get(obj)) evas_object_show(sd->obj);
   _e_icon_smart_reconfigure(sd);
}

E_API void
e_icon_fdo_icon_set(Evas_Object *obj, const char *icon)
{
   E_Smart_Data *sd;
   const char *path;
   int len;

   if (!icon) return;
   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR();
   if (icon[0] == '/')
     {
        e_icon_file_set(obj, icon);
        return;
     }

   if (!(sd = evas_object_smart_data_get(obj))) return;

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->guessing_animation) ecore_timer_del(sd->guessing_animation);
   sd->guessing_animation = NULL;
   sd->frame = 0;
   sd->frame_count = 0;
   sd->invalid = 0;
   sd->edje = EINA_FALSE;

   eina_stringshare_replace(&sd->fdo, icon);
   if (!sd->fdo) return;

   path = efreet_icon_path_find(e_config->icon_theme, sd->fdo, sd->size);
   if (!path)
     {
        if (e_util_strcmp(e_config->icon_theme, "hicolor"))
          path = efreet_icon_path_find("hicolor", sd->fdo, sd->size);
        if (!path) return;
     }

   len = strlen(icon);
   if ((len > 4) && (!strcasecmp(icon + len - 4, ".edj")))
     {
        e_icon_file_edje_set(obj, path, "icon");
        return;
     }

   /* smart code here */
   _e_icon_obj_prepare(obj, sd);
   sd->loading = 0;
   if (sd->size != 0)
     evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   if (sd->preload) evas_object_hide(sd->obj);
   if (sd->preload)
     evas_object_image_load_head_skip_set(sd->obj, EINA_TRUE);
   evas_object_image_file_set(sd->obj, path, NULL);
//   if (evas_object_image_load_error_get(sd->obj) != EVAS_LOAD_ERROR_NONE)
//     return EINA_FALSE;
   if (sd->preload)
     {
        sd->loading = 1;
        evas_object_image_preload(sd->obj, 0);
     }
   else if (evas_object_visible_get(obj))
     evas_object_show(sd->obj);
   _e_icon_smart_reconfigure(sd);
   return;
}

E_API void
e_icon_image_object_set(Evas_Object *obj, Evas_Object *o)
{
   E_Smart_Data *sd;
   const char *str;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   str = evas_object_type_get(o);
   if ((!str) || strcmp(str, "image"))
     CRI(EINA_COLOR_RED "******************\ntrying to set an image object of type '%s'! this is not what you want!\n******************\n"EINA_COLOR_RESET, str);

   if (sd->timer) ecore_timer_del(sd->timer);
   sd->timer = NULL;
   if (sd->guessing_animation) ecore_timer_del(sd->guessing_animation);
   sd->guessing_animation = NULL;
   sd->frame = 0;
   sd->frame_count = 0;
   sd->invalid = 0;
   sd->edje = EINA_FALSE;

   /* smart code here */
   if (sd->obj) evas_object_del(sd->obj);
   sd->loading = 0;
   sd->obj = o;
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_stack_below(sd->obj, sd->eventarea);
   if (evas_object_visible_get(obj)) evas_object_show(sd->obj);
   _handle_anim(sd);
   _e_icon_smart_reconfigure(sd);
}

E_API Eina_Bool
e_icon_file_get(const Evas_Object *obj, const char **file, const char **group)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(EINA_FALSE);
   if ((!file) && (!group)) return EINA_FALSE;
   if (file) *file = NULL;
   if (group) *group = NULL;
   if (!(sd = evas_object_smart_data_get(obj))) return EINA_FALSE;
   if (sd->edje)
     {
        edje_object_file_get(sd->obj, file, group);
        return file || group;
     }
   evas_object_image_file_get(sd->obj, file, group);
   return file || group;
}

E_API void
e_icon_smooth_scale_set(Evas_Object *obj, Eina_Bool smooth)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   if (sd->edje) return;
   evas_object_image_smooth_scale_set(sd->obj, smooth);
}

E_API Eina_Bool
e_icon_smooth_scale_get(const Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   if (!(sd = evas_object_smart_data_get(obj))) return EINA_FALSE;
   if (sd->edje)
     return EINA_FALSE;
   return evas_object_image_smooth_scale_get(sd->obj);
}

E_API void
e_icon_alpha_set(Evas_Object *obj, Eina_Bool alpha)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   if (sd->edje) return;
   evas_object_image_alpha_set(sd->obj, alpha);
}

E_API Eina_Bool
e_icon_alpha_get(const Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   if (!(sd = evas_object_smart_data_get(obj))) return EINA_FALSE;
   if (sd->edje) return EINA_FALSE;
   return evas_object_image_alpha_get(sd->obj);
}

E_API void
e_icon_preload_set(Evas_Object *obj, Eina_Bool preload)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   sd->preload = preload;
}

E_API Eina_Bool
e_icon_preload_get(const Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   if (!(sd = evas_object_smart_data_get(obj))) return EINA_FALSE;
   return sd->preload;
}

E_API void
e_icon_size_get(const Evas_Object *obj, int *w, int *h)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj)))
     {
        if (w) *w = 0;
        if (h) *h = 0;
        return;
     }
   if (sd->edje)
     edje_object_size_min_calc(sd->obj, w, h);
   else
     evas_object_image_size_get(sd->obj, w, h);
}

E_API Eina_Bool
e_icon_fill_inside_get(const Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   if (!(sd = evas_object_smart_data_get(obj))) return EINA_FALSE;
   return sd->fill_inside;
}

E_API void
e_icon_fill_inside_set(Evas_Object *obj, Eina_Bool fill_inside)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   fill_inside = !!fill_inside;
   if (sd->fill_inside == fill_inside) return;
   sd->fill_inside = fill_inside;
   _e_icon_smart_reconfigure(sd);
}

E_API Eina_Bool
e_icon_scale_up_get(const Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   if (!(sd = evas_object_smart_data_get(obj))) return EINA_FALSE;
   return sd->scale_up;
}

E_API void
e_icon_scale_up_set(Evas_Object *obj, Eina_Bool scale_up)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   scale_up = !!scale_up;
   if (sd->scale_up == scale_up) return;
   sd->scale_up = scale_up;
   _e_icon_smart_reconfigure(sd);
}

E_API void
e_icon_data_set(Evas_Object *obj, void *data, int w, int h)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   if (sd->edje) return;
   evas_object_image_size_set(sd->obj, w, h);
   evas_object_image_data_copy_set(sd->obj, data);
}

E_API void *
e_icon_data_get(const Evas_Object *obj, int *w, int *h)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(NULL);
   if (!(sd = evas_object_smart_data_get(obj))) return NULL;
   if (sd->edje) return NULL;
   evas_object_image_size_get(sd->obj, w, h);
   return evas_object_image_data_get(sd->obj, 0);
}

E_API void
e_icon_scale_size_set(Evas_Object *obj, int size)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   sd->size = size;
   if (sd->edje) return;
   evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
}

E_API int
e_icon_scale_size_get(const Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   if (!(sd = evas_object_smart_data_get(obj))) return 0;
   return sd->size;
}

E_API void
e_icon_selected_set(const Evas_Object *obj, Eina_Bool selected)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   if (!sd->edje) return;
   if (selected)
     edje_object_signal_emit(sd->obj, "e,state,selected", "e");
   else
     edje_object_signal_emit(sd->obj, "e,state,unselected", "e");
}

E_API void
e_icon_edje_emit(const Evas_Object *obj, const char *sig, const char *src)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR();
   if (!(sd = evas_object_smart_data_get(obj))) return;
   edje_object_signal_emit(sd->obj, sig, src);
}

/* local subsystem globals */
static void
_e_icon_smart_reconfigure(E_Smart_Data *sd)
{
   int iw, ih;
   Evas_Coord x, y, w, h;

   if (!sd->obj) return;
   w = sd->w;
   h = sd->h;
   x = sd->x;
   y = sd->y;
   if (!sd->loading)
     {
        if (sd->edje)
          {
             evas_object_move(sd->obj, x, y);
             evas_object_resize(sd->obj, w, h);
          }
        else
          {
             iw = 0;
             ih = 0;
             evas_object_image_size_get(sd->obj, &iw, &ih);
             if (iw < 1) iw = 1;
             if (ih < 1) ih = 1;

             if (sd->fill_inside)
               {
                  w = sd->w;
                  h = ((double)ih * w) / (double)iw;
                  if (h > sd->h)
                    {
                       h = sd->h;
                       w = ((double)iw * h) / (double)ih;
                    }
               }
             else
               {
                  w = sd->w;
                  h = ((double)ih * w) / (double)iw;
                  if (h < sd->h)
                    {
                       h = sd->h;
                       w = ((double)iw * h) / (double)ih;
                    }
               }
             if (!sd->scale_up)
               {
                  if ((w > iw) || (h > ih))
                    {
                       w = iw;
                       h = ih;
                    }
               }
             x = sd->x + ((sd->w - w) / 2);
             y = sd->y + ((sd->h - h) / 2);
             evas_object_move(sd->obj, x, y);
             evas_object_image_fill_set(sd->obj, 0, 0, w, h);
             evas_object_resize(sd->obj, w, h);
          }
     }
   evas_object_move(sd->eventarea, x, y);
   evas_object_resize(sd->eventarea, w, h);
}

static void
_e_icon_smart_init(void)
{
   if (_e_smart) return;
   {
      static Evas_Smart_Class sc = EVAS_SMART_CLASS_INIT_NAME_VERSION("e_icon");
      if (!sc.add)
        {
           sc.add = _e_icon_smart_add;
           sc.del = _e_icon_smart_del;
           sc.move = _e_icon_smart_move;
           sc.resize = _e_icon_smart_resize;
           sc.show = _e_icon_smart_show;
           sc.hide = _e_icon_smart_hide;
           sc.color_set = _e_icon_smart_color_set;
           sc.clip_set = _e_icon_smart_clip_set;
           sc.clip_unset = _e_icon_smart_clip_unset;
        }
      _e_smart = evas_smart_class_new(&sc);
   }
}

static void
_e_icon_preloaded(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Smart_Data *sd;
   int iw, ih;

   if (!(sd = evas_object_smart_data_get(data))) return;

   evas_object_smart_callback_call(data, "preloaded", NULL);
   evas_object_image_size_get(sd->obj, &iw, &ih);
   if ((iw > 0) && (ih > 0))
     {
        sd->invalid = 0;
        evas_object_show(sd->obj);
     }
   else
     sd->invalid = 1;
   sd->loading = 0;
   _e_icon_smart_reconfigure(sd);
}

static void
_e_icon_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (!(sd = calloc(1, sizeof(E_Smart_Data)))) return;

   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_IMAGE_PRELOADED,
                                  _e_icon_preloaded, obj);

   sd->eventarea = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(sd->eventarea, 0, 0, 0, 0);
   evas_object_smart_member_add(sd->eventarea, obj);

   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->fill_inside = 1;
   sd->scale_up = 1;
   sd->size = 64;
   sd->preload = 0;
   sd->tmp_fd = -1;
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_smart_data_set(obj, sd);
}

static void
_e_icon_smart_del(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_del(sd->obj);
   evas_object_del(sd->eventarea);
   if (sd->fdo) eina_stringshare_del(sd->fdo);
   if (sd->fdo_reload_timer) ecore_timer_del(sd->fdo_reload_timer);
   if (sd->timer) ecore_timer_del(sd->timer);
   if (sd->guessing_animation) ecore_timer_del(sd->guessing_animation);
   if (sd->url) eina_stringshare_del(sd->url);
   if (sd->con_url) ecore_con_url_free(sd->con_url);
   if (sd->tmp_file)
     {
        ecore_file_unlink(sd->tmp_file);
        eina_stringshare_del(sd->tmp_file);
     }
   if (sd->tmp_fd >= 0) close(sd->tmp_fd);
   if (sd->url_hand) ecore_event_handler_del(sd->url_hand);
   evas_object_smart_data_set(obj, NULL);
   memset(sd, 0, sizeof(*sd));
   free(sd);
}

static void
_e_icon_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   if ((sd->x == x) && (sd->y == y)) return;
   sd->x = x;
   sd->y = y;
   _e_icon_smart_reconfigure(sd);
}

static Eina_Bool
_e_icon_guess_anim(void *data)
{
   E_Smart_Data *sd = data;
   double t = ecore_loop_time_get();

   if (t - sd->last_resize < 0.2)
     {
        evas_object_image_scale_hint_set(sd->obj,
                                         EVAS_IMAGE_SCALE_HINT_DYNAMIC);
        sd->animated = EINA_TRUE;
     }
   else
     {
        evas_object_image_scale_hint_set(sd->obj,
                                         EVAS_IMAGE_SCALE_HINT_STATIC);
     }

   sd->guessing_animation = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_e_icon_fdo_reload(void *data)
{
   E_Smart_Data *sd = data;
   const char *path;

   sd->fdo_reload_timer = NULL;
   sd->size = MAX(sd->w, sd->h);
   path = efreet_icon_path_find(e_config->icon_theme, sd->fdo, sd->size);
   if (!path)
     {
        if (e_util_strcmp(e_config->icon_theme, "hicolor"))
          path = efreet_icon_path_find("hicolor", sd->fdo, sd->size);
        if (!path) return EINA_FALSE;
     }


   /* smart code here */
   evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   evas_object_image_file_set(sd->obj, path, NULL);
   if (sd->preload)
     {
        sd->loading = 1;
        evas_object_image_preload(sd->obj, 0);
     }
   return EINA_FALSE;
}

static void
_e_icon_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;
   if (sd->fdo)
     {
        if (sd->fdo_reload_timer) ecore_timer_del(sd->fdo_reload_timer);
        sd->fdo_reload_timer = ecore_timer_loop_add(0.1, _e_icon_fdo_reload, sd);
     }

   if ((!sd->edje) && ((sd->loading && sd->preload) ||
                       (!sd->loading && !sd->preload))
       && !sd->animated)
     {
        evas_object_image_scale_hint_set(sd->obj,
                                         EVAS_IMAGE_SCALE_HINT_DYNAMIC);
        if (!sd->guessing_animation)
          sd->guessing_animation = ecore_timer_loop_add(0.3,
                                                   _e_icon_guess_anim,
                                                   sd);
     }

   sd->last_resize = ecore_loop_time_get();
   _e_icon_smart_reconfigure(sd);
}

static void
_e_icon_smart_show(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   if (!((sd->preload) && (sd->loading)))
     {
        if (!sd->invalid) evas_object_show(sd->obj);
     }

   evas_object_show(sd->eventarea);
}

static void
_e_icon_smart_hide(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_hide(sd->obj);
   evas_object_hide(sd->eventarea);
}

static void
_e_icon_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_color_set(sd->obj, r, g, b, a);
}

static void
_e_icon_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_clip_set(sd->obj, clip);
   evas_object_clip_set(sd->eventarea, clip);
}

static void
_e_icon_smart_clip_unset(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (!(sd = evas_object_smart_data_get(obj))) return;
   evas_object_clip_unset(sd->obj);
   evas_object_clip_unset(sd->eventarea);
}
