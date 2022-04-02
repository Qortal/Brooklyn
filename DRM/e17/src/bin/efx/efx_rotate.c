#include "e_efx_private.h"

typedef struct E_Efx_Rotate_Data
{
   E_EFX *e;
   Ecore_Animator *anim;
   E_Efx_Effect_Speed speed;
   double start_degrees;
   double degrees;
   E_Efx_End_Cb cb;
   void *data;
} E_Efx_Rotate_Data;

static void
_obj_del(E_Efx_Rotate_Data *erd, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (erd->anim) ecore_animator_del(erd->anim);
   erd->e->rotate_data = NULL;
   if ((!erd->e->owner) && (!erd->e->followers)) e_efx_free(erd->e);
   free(erd);
}

static Eina_Bool
_rotate_cb(E_Efx_Rotate_Data *erd, double pos)
{
   double degrees;
   Eina_List *l;
   E_EFX *e;

   degrees = ecore_animator_pos_map(pos, (Ecore_Pos_Map)erd->speed, 0, 0);
   erd->e->map_data.rotation = degrees * erd->degrees + erd->start_degrees;
   //DBG("erd->e->map_data.rotation=%g,erd->degrees=%g,erd->start_degrees=%g", erd->e->map_data.rotation, erd->degrees, erd->start_degrees);
   e_efx_maps_apply(erd->e, erd->e->obj, NULL, E_EFX_MAPS_APPLY_ALL);
   EINA_LIST_FOREACH(erd->e->followers, l, e)
     e_efx_maps_apply(e, e->obj, NULL, E_EFX_MAPS_APPLY_ALL);

   if (pos < 1.0) return EINA_TRUE;

   erd->anim = NULL;
   E_EFX_QUEUE_CHECK(erd);
   return EINA_TRUE;
}

static void
_rotate_stop(Evas_Object *obj, Eina_Bool reset)
{
   E_EFX *e;
   E_Efx_Rotate_Data *erd;

   e = evas_object_data_get(obj, "e_efx-data");
   if ((!e) || (!e->rotate_data)) return;
   erd = e->rotate_data;
   if (reset)
     {
        erd->start_degrees = 0;
        _rotate_cb(erd, 0);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, erd);
        erd->e->map_data.rotation = 0;
        if (e_efx_queue_complete(erd->e, erd))
          e_efx_queue_process(erd->e);
        _obj_del(erd, NULL, NULL, NULL);
        INF("reset rotating object %p", obj);
     }
   else
     {
        if (erd->anim) ecore_animator_del(erd->anim);
        erd->anim = NULL;
        INF("stopped rotating object %p", obj);
        if (e_efx_queue_complete(erd->e, erd))
          e_efx_queue_process(erd->e);
     }
}

void
_e_efx_rotate_calc(void *data, void *owner, Evas_Object *obj, Evas_Map *map)
{
   E_Efx_Rotate_Data *erd = data;
   E_Efx_Rotate_Data *erd2 = owner;
   e_efx_rotate_helper(erd2 ? erd2->e : (erd ? erd->e : NULL), obj, map, (erd ? erd->e->map_data.rotation : 0) + (erd2 ? erd2->e->map_data.rotation : 0));
}

EAPI Eina_Bool
e_efx_rotate(Evas_Object *obj, E_Efx_Effect_Speed speed, double degrees, const Evas_Point *center, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Rotate_Data *erd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   if (eina_dbl_exact(degrees, 0)) return EINA_FALSE;
   if (total_time < 0.0) return EINA_FALSE;
   if (speed > E_EFX_EFFECT_SPEED_SINUSOIDAL) return EINA_FALSE;
   /* can't rotate a spinning object, so we stop it first */
   e = evas_object_data_get(obj, "e_efx-data");
   if (e)
     {
        if (e->spin_data) e_efx_spin_stop(obj);
        if (e->rotate_data)
          {
             erd = e->rotate_data;
             if (erd->anim) e_efx_rotate_stop(obj);
          }
     }
   else
     {
         e = e_efx_new(obj);
         EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);
     }

   if (!e_efx_rotate_center_init(e, center)) return EINA_FALSE;
   INF("rotate: %p - %g degrees over %gs: %s", obj, degrees, total_time, e_efx_speed_str[speed]);
   if (!e->rotate_data)
     {
        e->rotate_data = calloc(1, sizeof(E_Efx_Rotate_Data));
        EINA_SAFETY_ON_NULL_RETURN_VAL(e->rotate_data, EINA_FALSE);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, e->rotate_data);
     }
   erd = e->rotate_data;
   erd->e = e;
   erd->speed = speed;
   erd->degrees = degrees;
   erd->start_degrees = e->map_data.rotation;
   erd->cb = cb;
   erd->data = (void*)data;
   if (eina_dbl_exact(total_time, 0))
     {
        e->map_data.rotation += degrees;
        _rotate_cb(erd, 1.0);
        return EINA_TRUE;
     }
   if (erd->anim) ecore_animator_del(erd->anim);
   erd->anim = ecore_animator_timeline_add(total_time, (Ecore_Timeline_Cb)_rotate_cb, erd);
   return EINA_TRUE;
}

EAPI void
e_efx_rotate_reset(Evas_Object *obj)
{
   _rotate_stop(obj, EINA_TRUE);
}

EAPI void
e_efx_rotate_stop(Evas_Object *obj)
{
   _rotate_stop(obj, EINA_FALSE);
}
