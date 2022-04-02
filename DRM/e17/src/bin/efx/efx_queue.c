#include "e_efx_private.h"

struct E_Efx_Queue_Data
{
   E_EFX *e;
   E_Efx_Queued_Effect effect;
   E_Efx_Effect_Speed speed;
   double time;
   E_Efx_End_Cb cb;
   void *data;

   Eina_List *subeffects;
   void *effect_data;
   Eina_Bool active : 1;
};

static void
_queue_advance(E_Efx_Queue_Data *eqd)
{
   E_Efx_Queue_Data *run;
   Eina_List *l;

   INF("queue_advance: %p", eqd->e->obj);
   switch (eqd->effect.type)
     {
      case E_EFX_EFFECT_TYPE_ROTATE:
        e_efx_rotate(eqd->e->obj, eqd->speed, eqd->effect.effect.rotation.degrees, eqd->effect.effect.rotation.center, eqd->time, eqd->cb, eqd->data);
        eqd->effect_data = eqd->e->rotate_data;
        break;
      case E_EFX_EFFECT_TYPE_ZOOM:
        e_efx_zoom(eqd->e->obj, eqd->speed, eqd->effect.effect.zoom.start, eqd->effect.effect.zoom.end, eqd->effect.effect.zoom.center, eqd->time, eqd->cb, eqd->data);
        eqd->effect_data = eqd->e->zoom_data;
        break;
      case E_EFX_EFFECT_TYPE_MOVE:
        e_efx_move(eqd->e->obj, eqd->speed, &eqd->effect.effect.movement.point, eqd->time, eqd->cb, eqd->data);
        eqd->effect_data = eqd->e->move_data;
        break;
      case E_EFX_EFFECT_TYPE_PAN:
        e_efx_pan(eqd->e->obj, eqd->speed, &eqd->effect.effect.movement.point, eqd->time, eqd->cb, eqd->data);
        eqd->effect_data = eqd->e->pan_data;
        break;
      case E_EFX_EFFECT_TYPE_FADE:
        e_efx_fade(eqd->e->obj, eqd->speed, &eqd->effect.effect.fade.color, eqd->effect.effect.fade.alpha, eqd->time, eqd->cb, eqd->data);
        eqd->effect_data = eqd->e->fade_data;
        break;
      case E_EFX_EFFECT_TYPE_RESIZE:
      default:
        e_efx_resize(eqd->e->obj, eqd->speed, eqd->effect.effect.resize.point, eqd->effect.effect.resize.w, eqd->effect.effect.resize.h, eqd->time, eqd->cb, eqd->data);
        eqd->effect_data = eqd->e->resize_data;
     }
   eqd->active = EINA_TRUE;
   EINA_LIST_FOREACH(eqd->subeffects, l, run)
     _queue_advance(run);
}

void
e_efx_queue_process(E_EFX *e)
{
   E_Efx_Queue_Data *eqd;

   eqd = eina_list_data_get(e->queue);
   if (!eqd) return;
   if (eqd->active) return;

   _queue_advance(eqd);
}

void
eqd_free(E_Efx_Queue_Data *eqd)
{
   E_Efx_Queue_Data *sub;
   if (!eqd) return;
   if (eqd->effect.type == E_EFX_EFFECT_TYPE_ROTATE)
     free(eqd->effect.effect.rotation.center);
   else if (eqd->effect.type == E_EFX_EFFECT_TYPE_ZOOM)
     free(eqd->effect.effect.zoom.center);
   else if (eqd->effect.type == E_EFX_EFFECT_TYPE_RESIZE)
     free(eqd->effect.effect.resize.point);
   EINA_LIST_FREE(eqd->subeffects, sub)
     eqd_free(sub);
   free(eqd);
}

Eina_Bool
e_efx_queue_complete(E_EFX *e, void *effect_data)
{
   E_Efx_Queue_Data *eqd;

   eqd = eina_list_data_get(e->queue);
   if (!eqd)
     {
        e_efx_free(e);
        return EINA_FALSE;
     }
   DBG("%p: %p", e->obj, effect_data);
   if (eqd->effect_data != effect_data) return EINA_FALSE;
   e->queue = eina_list_remove_list(e->queue, e->queue);
   eqd_free(eqd);
   return !!e->queue;
}

E_Efx_Queue_Data *
eqd_new(E_EFX *e, E_Efx_Effect_Speed speed, const E_Efx_Queued_Effect *effect, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_Efx_Queue_Data *eqd;

   eqd = calloc(1, sizeof(E_Efx_Queue_Data));
   memcpy(&eqd->effect, effect, sizeof(E_Efx_Queued_Effect));
   eqd->e = e;
   if (effect->type == E_EFX_EFFECT_TYPE_ROTATE)
     {
        if (effect->effect.rotation.center)
          {
             eqd->effect.effect.rotation.center = malloc(sizeof(Evas_Point));
             if (!eqd->effect.effect.rotation.center) goto error;
             memcpy(eqd->effect.effect.rotation.center, effect->effect.rotation.center, sizeof(Evas_Point));
          }
     }
   else if (effect->type == E_EFX_EFFECT_TYPE_ZOOM)
     {
        if (effect->effect.zoom.center)
          {
             eqd->effect.effect.zoom.center = malloc(sizeof(Evas_Point));
             if (!eqd->effect.effect.zoom.center) goto error;
             memcpy(eqd->effect.effect.zoom.center, effect->effect.zoom.center, sizeof(Evas_Point));
          }
     }
   else if (effect->type == E_EFX_EFFECT_TYPE_RESIZE)
     {
        if (effect->effect.resize.point)
          {
             eqd->effect.effect.resize.point = malloc(sizeof(Evas_Point));
             if (!eqd->effect.effect.resize.point) goto error;
             memcpy(eqd->effect.effect.resize.point, effect->effect.resize.point, sizeof(Evas_Point));
          }
     }
   eqd->speed = speed;
   eqd->time = total_time;
   eqd->cb = cb;
   eqd->data = (void*)data;
   return eqd;
error:
   free(eqd);
   e_efx_free(e);
   return NULL;
}


EAPI void
e_efx_queue_run(Evas_Object *obj)
{
   E_EFX *e;

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return;
   e_efx_queue_process(e);
}

EAPI E_Efx_Queue_Data *
e_efx_queue_append(Evas_Object *obj, E_Efx_Effect_Speed speed, const E_Efx_Queued_Effect *effect, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Queue_Data *eqd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(effect, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(total_time >= 0.0, NULL);
   if (effect->type > E_EFX_EFFECT_TYPE_RESIZE) return NULL;
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   eqd = eqd_new(e, speed, effect, total_time, cb, data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(eqd, NULL);

   e->queue = eina_list_append(e->queue, eqd);
   return eqd;
   (void)e_efx_speed_str;
}

EAPI E_Efx_Queue_Data *
e_efx_queue_prepend(Evas_Object *obj, E_Efx_Effect_Speed speed, const E_Efx_Queued_Effect *effect, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Queue_Data *eqd, *eqd2;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(effect, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(total_time >= 0.0, NULL);
   if (effect->type > E_EFX_EFFECT_TYPE_RESIZE) return NULL;
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, NULL);

   eqd = eqd_new(e, speed, effect, total_time, cb, data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(eqd, NULL);

   if (e->queue)
     {
        eqd2 = eina_list_data_get(e->queue);
        if (eqd2->active)
          e->queue = eina_list_append_relative_list(e->queue, eqd, e->queue);
        else
          e->queue = eina_list_prepend(e->queue, eqd);
     }
   else
     e->queue = eina_list_append(e->queue, eqd);
   return eqd;
   (void)e_efx_speed_str;
}

EAPI void
e_efx_queue_promote(Evas_Object *obj, E_Efx_Queue_Data *eqd)
{
   E_EFX *e;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(eqd);
   e = evas_object_data_get(obj, "e_efx-data");
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_NULL_RETURN(e->queue);
   EINA_SAFETY_ON_TRUE_RETURN(eqd->active);

   if (e->queue->data == eqd) return;

   e->queue = eina_list_remove(e->queue, eqd);
   e->queue = eina_list_append_relative_list(e->queue, eqd, e->queue);
}

EAPI void
e_efx_queue_demote(Evas_Object *obj, E_Efx_Queue_Data *eqd)
{
   E_EFX *e;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(eqd);
   e = evas_object_data_get(obj, "e_efx-data");
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_NULL_RETURN(e->queue);
   EINA_SAFETY_ON_TRUE_RETURN(eqd->active);

   if (eina_list_last(e->queue)->data == eqd) return;

   e->queue = eina_list_demote_list(e->queue, eina_list_data_find_list(e->queue, eqd));
}

EAPI void
e_efx_queue_delete(Evas_Object *obj, E_Efx_Queue_Data *eqd)
{
   E_EFX *e;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(eqd);
   e = evas_object_data_get(obj, "e_efx-data");
   EINA_SAFETY_ON_NULL_RETURN(e);
   EINA_SAFETY_ON_NULL_RETURN(e->queue);
   EINA_SAFETY_ON_TRUE_RETURN(eqd->active);

   e->queue = eina_list_remove(e->queue, eqd);
   eqd_free(eqd);
}

EAPI void
e_efx_queue_clear(Evas_Object *obj)
{
   E_EFX *e;
   E_Efx_Queue_Data *eqd;
   Eina_List *l, *ll;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   e = evas_object_data_get(obj, "e_efx-data");
   EINA_SAFETY_ON_NULL_RETURN(e);
   if (!e->queue) return;

   EINA_LIST_FOREACH_SAFE(e->queue, l, ll, eqd)
     {
        if (eqd->active) continue;
        e->queue = eina_list_remove_list(e->queue, l);
        eqd_free(eqd);
     }
}

EAPI Eina_Bool
e_efx_queue_effect_attach(E_Efx_Queue_Data *eqd, E_Efx_Effect_Speed speed, const E_Efx_Queued_Effect *effect, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Queue_Data *sub;

   EINA_SAFETY_ON_NULL_RETURN_VAL(eqd, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(effect, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(total_time >= 0.0, EINA_FALSE);
   if (effect->type > E_EFX_EFFECT_TYPE_RESIZE) return EINA_FALSE;
   e = eqd->e;

   sub = eqd_new(e, speed, effect, total_time, cb, data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sub, EINA_FALSE);

   eqd->subeffects = eina_list_append(eqd->subeffects, sub);
   return EINA_TRUE;
   (void)e_efx_speed_str;
}
