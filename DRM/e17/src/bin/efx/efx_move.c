#include "e_efx_private.h"

typedef struct E_Efx_Move_Data
{
   E_EFX *e;
   Ecore_Animator *anim;
   E_Efx_Effect_Speed speed;
   Evas_Point start;
   Evas_Point change;
   Evas_Point current;
   int degrees;
   E_Efx_End_Cb cb;
   void *data;
} E_Efx_Move_Data;

static void
_obj_del(E_Efx_Move_Data *emd, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (emd->anim) ecore_animator_del(emd->anim);
   emd->e->move_data = NULL;
   if ((!emd->e->owner) && (!emd->e->followers)) e_efx_free(emd->e);
   free(emd);
}

static void
_move(E_EFX *e, int x, int y)
{
   e->x += x, e->y += y;
   evas_object_move(e->obj, e->x, e->y);
   //DBG("%p to (%d,%d)", e->obj, e->x, e->y);
}

static Eina_Bool
_move_circle_cb(E_Efx_Move_Data *emd, double pos)
{
   double pct, degrees;
   Eina_List *l;
   E_EFX *e;
   double x, y, r, rad;
   Evas_Coord xx, yy, ox, oy, w, h;


   pct = ecore_animator_pos_map(pos, (Ecore_Pos_Map)emd->speed, 0, 0);
   degrees = pct * emd->degrees;
   ox = emd->e->x, oy = emd->e->y;
   if (emd->e->resize_data)
     w = emd->e->w, h = emd->e->h;
   else
     evas_object_geometry_get(emd->e->obj, NULL, NULL, &w, &h);
   r = (degrees * M_PI) / 180.0;
   rad = sqrt((emd->current.x + w/2.0 - emd->e->map_data.move_center->x) * (emd->current.x + w/2.0 - emd->e->map_data.move_center->x) +
              (emd->current.y + h/2.0 - emd->e->map_data.move_center->y) * (emd->current.y + h/2.0 - emd->e->map_data.move_center->y));
   x = emd->e->map_data.move_center->x + rad * cos(r);
   y = emd->e->map_data.move_center->y + rad * sin(r);
   x -= (double)w / 2.;
   y -= (double)h / 2.;
   xx = lround(x);
   yy = lround(y);
   //DBG("move: %g || %g,%g", degrees, x, y);
   emd->e->x = xx, emd->e->y = yy;
   evas_object_move(emd->e->obj, xx, yy);
   e_efx_maps_apply(emd->e, emd->e->obj, NULL, E_EFX_MAPS_APPLY_ALL);
   EINA_LIST_FOREACH(emd->e->followers, l, e)
     {
        _move(e, xx - ox, yy - oy);
        e_efx_maps_apply(e, e->obj, NULL, E_EFX_MAPS_APPLY_ALL);
     }

   if (pos < 1.0) return EINA_TRUE;

   E_EFX_QUEUE_CHECK(emd);
   return EINA_TRUE;
}

static Eina_Bool
_move_cb(E_Efx_Move_Data *emd, double pos)
{
   int x, y;
   double pct;
   Eina_List *l;
   E_EFX *e;

   pct = ecore_animator_pos_map(pos, (Ecore_Pos_Map)emd->speed, 0, 0);
   x = lround(pct * (double)emd->change.x) - emd->current.x;
   y = lround(pct * (double)emd->change.y) - emd->current.y;
   _e_efx_resize_adjust(emd->e, &x, &y);
   _move(emd->e, x, y);
   e_efx_maps_apply(emd->e, emd->e->obj, NULL, E_EFX_MAPS_APPLY_ALL);
   EINA_LIST_FOREACH(emd->e->followers, l, e)
     {
        _move(e, x, y);
        e_efx_maps_apply(e, e->obj, NULL, E_EFX_MAPS_APPLY_ALL);
     }

   emd->current.x += x;
   emd->current.y += y;
   if (pos < 1.0) return EINA_TRUE;

   emd->anim = NULL;
   E_EFX_QUEUE_CHECK(emd);
   return EINA_TRUE;
}

static void
_move_stop(Evas_Object *obj, Eina_Bool reset)
{
   E_EFX *e;
   E_Efx_Move_Data *emd;

   e = evas_object_data_get(obj, "e_efx-data");
   if ((!e) || (!e->move_data)) return;
   emd = e->move_data;
   if (reset)
     {
        evas_object_move(obj, emd->start.x, emd->start.y);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, emd);
        if (e_efx_queue_complete(emd->e, emd))
          e_efx_queue_process(emd->e);
        _obj_del(emd, NULL, NULL, NULL);
        INF("reset moved object %p", obj);
     }
   else
     {
        INF("stopped moved object %p", obj);
        if (emd->anim) ecore_animator_del(emd->anim);
        emd->anim = NULL;
        if (e_efx_queue_complete(emd->e, emd))
          e_efx_queue_process(emd->e);
     }
}

EAPI Eina_Bool
e_efx_move(Evas_Object *obj, E_Efx_Effect_Speed speed, const Evas_Point *end_point, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Move_Data *emd;
   Evas_Coord x, y;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   if (!end_point) return EINA_FALSE;
   if (total_time < 0.0) return EINA_FALSE;
   if (speed > E_EFX_EFFECT_SPEED_SINUSOIDAL) return EINA_FALSE;

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   INF("move: %p - (%d,%d) -> (%d,%d) over %gs: %s", obj, x, y, end_point->x, end_point->y, total_time, e_efx_speed_str[speed]);
   if (eina_dbl_exact(total_time, 0))
     {
        evas_object_move(obj, end_point->x, end_point->y);
        return EINA_TRUE;
     }
   if (!e->move_data)
     {
        e->move_data = calloc(1, sizeof(E_Efx_Move_Data));
        EINA_SAFETY_ON_NULL_RETURN_VAL(e->move_data, EINA_FALSE);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, e->move_data);
     }
   emd = e->move_data;
   emd->e = e;
   emd->speed = speed;
   e->x = x, e->y = y;
   emd->change.x = end_point->x - x;
   emd->change.y = end_point->y - y;
   emd->current.x = emd->current.y = 0;
   emd->cb = cb;
   emd->data = (void*)data;
   if (emd->anim) ecore_animator_del(emd->anim);
   emd->anim = ecore_animator_timeline_add(total_time, (Ecore_Timeline_Cb)_move_cb, emd);
   return EINA_TRUE;
}


EAPI Eina_Bool
e_efx_move_circle(Evas_Object *obj, E_Efx_Effect_Speed speed, const Evas_Point *center, int degrees, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Move_Data *emd;
   Evas_Coord x, y;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   if (!degrees) return EINA_FALSE;
   if (!center) return EINA_FALSE;
   if (total_time < 0.0) return EINA_FALSE;
   if (speed > E_EFX_EFFECT_SPEED_SINUSOIDAL) return EINA_FALSE;

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);
   if (!e_efx_move_center_init(e, center)) return EINA_FALSE;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   INF("move: %p - (%d,%d) %d over %gs: %s", obj, x, y, degrees, total_time, e_efx_speed_str[speed]);
   if (eina_dbl_exact(total_time, 0))
     {
     //   evas_object_move(obj, end_point->x, end_point->y);
        return EINA_TRUE;
     }
   if (!e->move_data)
     {
        e->move_data = calloc(1, sizeof(E_Efx_Move_Data));
        EINA_SAFETY_ON_NULL_RETURN_VAL(e->move_data, EINA_FALSE);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, e->move_data);
     }
   emd = e->move_data;
   emd->e = e;
   emd->speed = speed;
   e->x = emd->start.x = emd->current.x = x;
   e->y = emd->start.y = emd->current.y = y;
   emd->degrees = degrees;
   emd->cb = cb;
   emd->data = (void*)data;
   if (emd->anim) ecore_animator_del(emd->anim);
   emd->anim = ecore_animator_timeline_add(total_time, (Ecore_Timeline_Cb)_move_circle_cb, emd);
   return EINA_TRUE;
}

EAPI void
e_efx_move_reset(Evas_Object *obj)
{
   _move_stop(obj, EINA_TRUE);
}

EAPI void
e_efx_move_stop(Evas_Object *obj)
{
   _move_stop(obj, EINA_FALSE);
}
