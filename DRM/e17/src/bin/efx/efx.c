#include "e_efx_private.h"

int _e_efx_log_dom = -1;
static int _e_efx_init_count = 0;

static uint64_t _e_efx_obj_count = 0;

E_EFX *
e_efx_new(Evas_Object *obj)
{
   E_EFX *e;
   e = calloc(1, sizeof(E_EFX));
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);
   e->obj = obj;
   evas_object_data_set(obj, "e_efx-data", e);
   _e_efx_obj_count++;
   return e;
}

void
e_efx_free(E_EFX *e)
{
   E_EFX *ef;
   if (e->zoom_data || e->resize_data || e->rotate_data || e->spin_data || e->move_data || e->bumpmap_data || e->pan_data || e->fade_data || e->queue) return;
   DBG("freeing e_efx for %p", e->obj);
   EINA_LIST_FREE(e->followers, ef)
     e_efx_free(ef);
   evas_object_data_del(e->obj, "e_efx-data");
   e_efx_map_set(e->obj, NULL);
   free(e->map_data.rotate_center);
   free(e->map_data.zoom_center);
   if (!(--_e_efx_obj_count))
     {
        if (!_e_efx_init_count)
          e_efx_shutdown();
     }
   free(e);
}

EAPI int
e_efx_init(void)
{
   if (++_e_efx_init_count > 1) return _e_efx_init_count;

   if (eina_init() < 1) goto err;

   _e_efx_log_dom = eina_log_domain_register("e_efx", EINA_COLOR_GREEN);
   if (_e_efx_log_dom < 0) goto lgerr;
   return _e_efx_init_count;
lgerr:
   eina_shutdown();
err:
   return --_e_efx_init_count;
   (void)e_efx_speed_str;
}

EAPI void
e_efx_shutdown(void)
{
   if (_e_efx_init_count && (--_e_efx_init_count != 0)) return;
   if (_e_efx_obj_count) return;
   eina_log_domain_unregister(_e_efx_log_dom);
   _e_efx_log_dom = -1;
   eina_shutdown();
}
