#ifndef E_EFX_PRIVATE_H
#define E_EFX_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>

#include <Evas.h>
#include <Ecore.h>

#include "e_Efx.h"

#define DBG(...)            EINA_LOG_DOM_DBG(_e_efx_log_dom, __VA_ARGS__)
#define INF(...)            EINA_LOG_DOM_INFO(_e_efx_log_dom, __VA_ARGS__)
#define WRN(...)            EINA_LOG_DOM_WARN(_e_efx_log_dom, __VA_ARGS__)
#define ERR(...)            EINA_LOG_DOM_ERR(_e_efx_log_dom, __VA_ARGS__)
#define CRI(...)            EINA_LOG_DOM_CRIT(_e_efx_log_dom, __VA_ARGS__)

#ifdef EAPI
# undef EAPI
#endif /* ifdef EAPI */

#ifdef _WIN32
# ifdef EFL_E_EFX_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else /* ifdef DLL_EXPORT */
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else /* ifdef EFL_BUILD */
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_BUILD */
#else /* ifdef _WIN32 */
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else /* if __GNUC__ >= 4 */
#   define EAPI
#  endif /* if __GNUC__ >= 4 */
# else /* ifdef __GNUC__ */
#  define EAPI
# endif /* ifdef __GNUC__ */
#endif /* ! _WIN32 */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

static const char *e_efx_speed_str[] =
{
   "LINEAR", "ACCELERATE", "DECELERATE", "SINUSOIDAL"
};

extern int _e_efx_log_dom;

typedef struct E_EFX E_EFX;

struct E_EFX
{
   Evas_Object *obj;
   E_EFX *owner;
   void *spin_data;
   void *rotate_data;
   void *zoom_data;
   void *move_data;
   void *bumpmap_data;
   void *pan_data;
   void *fade_data;
   void *resize_data;
   E_Efx_Map_Data map_data;
   Eina_List *followers;
   Eina_List *queue;
   int x, y, w, h;
};

void _e_efx_zoom_calc(void *, void *, Evas_Object *obj, Evas_Map *map);
void _e_efx_rotate_calc(void *, void *, Evas_Object *obj, Evas_Map *map);
void _e_efx_spin_calc(void *, void *, Evas_Object *obj, Evas_Map *map);
void _e_efx_resize_adjust(E_EFX *e, int *x, int *y);

#define E_EFX_MAPS_APPLY_ALL EINA_TRUE, EINA_TRUE, EINA_TRUE
#define E_EFX_MAPS_APPLY_ROTATE EINA_TRUE, EINA_FALSE, EINA_FALSE
#define E_EFX_MAPS_APPLY_SPIN EINA_FALSE, EINA_TRUE, EINA_FALSE
#define E_EFX_MAPS_APPLY_ZOOM EINA_FALSE, EINA_FALSE, EINA_TRUE
#define E_EFX_MAPS_APPLY_ROTATE_SPIN EINA_TRUE, EINA_TRUE, EINA_FALSE
void e_efx_maps_apply(E_EFX *e, Evas_Object *obj, Evas_Map *map, Eina_Bool rotate, Eina_Bool spin, Eina_Bool zoom);

E_EFX *e_efx_new(Evas_Object *obj);
void e_efx_free(E_EFX *e);
Evas_Map *e_efx_map_new(Evas_Object *obj);
void e_efx_map_set(Evas_Object *obj, Evas_Map *map);
Eina_Bool e_efx_rotate_center_init(E_EFX *e, const Evas_Point *center);
Eina_Bool e_efx_zoom_center_init(E_EFX *e, const Evas_Point *center);
Eina_Bool e_efx_move_center_init(E_EFX *e, const Evas_Point *center);
void e_efx_rotate_helper(E_EFX *e, Evas_Object *obj, Evas_Map *map, double degrees);
void e_efx_clip_setup(Evas_Object *obj, Evas_Object *clip);
void e_efx_fade_reclip(void *efd);

#define E_EFX_QUEUE_CHECK(X) do \
   { \
      E_EFX *ee = (X)->e; \
      evas_object_ref(ee->obj); \
      if ((X)->cb) (X)->cb((X)->data, &(X)->e->map_data, (X)->e->obj); \
      if (e_efx_queue_complete((X)->e, (X))) \
        e_efx_queue_process(ee); \
      evas_object_unref(ee->obj); \
   } while (0)
Eina_Bool e_efx_queue_complete(E_EFX *e, void *effect_data);
void e_efx_queue_process(E_EFX *e);

static inline void
_size_debug(Evas_Object *obj)
{
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   DBG("%s %p: x=%d,y=%d,w=%d,h=%d", evas_object_visible_get(obj) ? "vis" : "hid", obj, x, y, w, h);
}

static inline void
_color_debug(Evas_Object *obj)
{
   Evas_Coord r, g, b, a;
   evas_object_color_get(obj, &r, &g, &b, &a);
   DBG("%d/%d/%d/%d", MIN(r, a), MIN(g, a), MIN(b, a), a);
}

#define HIT DBG("HIT")

#endif
