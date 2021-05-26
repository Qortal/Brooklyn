#include "e_efx_private.h"

typedef enum
{
   NONE,
   TOP_RIGHT,
   BOTTOM_LEFT,
   BOTTOM_RIGHT
} Anchor;

typedef struct E_Efx_Resize_Data
{
   E_EFX *e;
   E_Efx_Effect_Speed speed;
   Ecore_Animator *anim;
   int w, h;
   int start_w, start_h;
   E_Efx_End_Cb cb;
   void *data;
   Anchor anchor_type;
   Evas_Point anchor;
   Eina_Bool moving : 1;
} E_Efx_Resize_Data;

static void
_obj_del(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Efx_Resize_Data *erd = data;

   if (erd->anim) ecore_animator_del(erd->anim);
   erd->e->resize_data = NULL;
   if ((!erd->e->owner) && (!erd->e->followers)) e_efx_free(erd->e);
   free(erd);
}

static void
_resize_anchor(E_Efx_Resize_Data *erd)
{
   int x = 0, y = 0;
   int cx, cy;

   if (!erd->anchor_type) return;

   _e_efx_resize_adjust(erd->e, &x, &y);
   if ((!x) && (!y)) return;

   if (erd->e->move_data)
     cx = erd->e->x, cy = erd->e->y;
   else
     evas_object_geometry_get(erd->e->obj, &cx, &cy, NULL, NULL);
   x += cx, y += cy;
   evas_object_move(erd->e->obj, x, y);
}

static Eina_Bool
_resize_cb(E_Efx_Resize_Data *erd, double pos)
{
   double factor;

   if (pos < 1.0)
     {
        int w, h;

        factor = ecore_animator_pos_map(pos, (Ecore_Pos_Map)erd->speed, 0, 0);
        w = lround(factor * (erd->w - erd->start_w)) + erd->start_w;
        h = lround(factor * (erd->h - erd->start_h)) + erd->start_h;
        //DBG("%p to (%dx%d)", erd->e->obj, w, h);
        erd->e->w = w, erd->e->h = h;
        evas_object_resize(erd->e->obj, w, h);
        _resize_anchor(erd);
        return EINA_TRUE;
     }
   /* lround will usually be off by 1 at the end, so we manually set this here */
   erd->e->w = erd->w, erd->e->h = erd->h;
   evas_object_resize(erd->e->obj, erd->w, erd->h);
   _resize_anchor(erd);

   erd->anim = NULL;
   E_EFX_QUEUE_CHECK(erd);
   return EINA_TRUE;
}

static void
_resize_stop(Evas_Object *obj, Eina_Bool reset)
{
   E_EFX *e;
   E_Efx_Resize_Data *erd;

   e = evas_object_data_get(obj, "e_efx-data");
   if ((!e) || (!e->resize_data)) return;
   erd = e->resize_data;
   if (reset)
     {
        erd->e->w = erd->start_w, erd->e->h = erd->start_h;
        evas_object_resize(obj, erd->start_w, erd->start_h);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, erd);
        if (erd->moving)
          {
             erd->moving = 0;
             e_efx_move_reset(obj);
          }
        else if (e_efx_queue_complete(erd->e, erd))
          e_efx_queue_process(erd->e);
        _obj_del(erd, NULL, NULL, NULL);
        INF("reset resized object %p", obj);
     }
   else
     {
        INF("stopped resized object %p", obj);
        if (erd->anim) ecore_animator_del(erd->anim);
        erd->anim = NULL;
        if (erd->moving)
          {
             erd->moving = 0;
             e_efx_move_stop(obj);
          }
        if (e_efx_queue_complete(erd->e, erd))
          e_efx_queue_process(erd->e);
     }
}

void
_e_efx_resize_adjust(E_EFX *e, int *ax, int *ay)
{
   E_Efx_Resize_Data *erd = e->resize_data;
   int x, y, w, h;

   if ((!erd) || (!erd->anim)) return;
   if (e->move_data)
     x = e->x, y = e->y;
   else
     evas_object_geometry_get(e->obj, &x, &y, NULL, NULL);
   w = e->w, h = e->h;
   switch (erd->anchor_type)
     {
      case TOP_RIGHT:
        *ax = erd->anchor.x - (x + w);
        *ay = erd->anchor.y - y;
        break;
      case BOTTOM_LEFT:
        *ax = erd->anchor.x - x;
        *ay = erd->anchor.y - (y + h);
        break;
      case BOTTOM_RIGHT:
        *ax = erd->anchor.x - (x + w);
        *ay = erd->anchor.y - (y + h);
        break;
      default: break;
     }
}

EAPI Eina_Bool
e_efx_resize(Evas_Object *obj, E_Efx_Effect_Speed speed, const Evas_Point *position, int w, int h, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Resize_Data *erd;
   int x, y;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(w < 0, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(h < 0, EINA_FALSE);
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);
   erd = e->resize_data;
   if (!erd)
     {
        e->resize_data = erd = calloc(1, sizeof(E_Efx_Resize_Data));
        EINA_SAFETY_ON_NULL_RETURN_VAL(erd, EINA_FALSE);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, e->resize_data);
     }

   erd->e = e;
   erd->anchor_type = NONE;
   erd->w = w;
   erd->h = h;
   erd->cb = cb;
   erd->data = (void*)data;
   evas_object_geometry_get(obj, &x, &y, &erd->start_w, &erd->start_h);
   e->w = erd->start_w;
   e->h = erd->start_h;
   if (e->move_data)
     x = e->x, y = e->y;
   INF("resize: %p || %dx%d => %dx%d %s over %gs", obj, erd->start_w, erd->start_h, w, h, e_efx_speed_str[speed], total_time);
   if (position)
     {
        if ((position->x != x) || (position->y != y))
          {
             Evas_Point tr, bl, br;
             Evas_Point atr, abl, abr;

             tr = (Evas_Point){x + erd->start_w, y};
             bl = (Evas_Point){x, y + erd->start_h};
             br = (Evas_Point){x + erd->start_w, y + erd->start_h};
             atr = (Evas_Point){position->x + w, position->y};
             abl = (Evas_Point){position->x, position->y + h};
             abr = (Evas_Point){position->x + w, position->y + h};
             if (!memcmp(&tr, &atr, sizeof(Evas_Point)))
               {
                  erd->anchor_type = TOP_RIGHT;
                  erd->anchor = tr;
               }
             else if (!memcmp(&bl, &abl, sizeof(Evas_Point)))
               {
                  erd->anchor_type = BOTTOM_LEFT;
                  erd->anchor = bl;
               }
             else if (!memcmp(&br, &abr, sizeof(Evas_Point)))
               {
                  erd->anchor_type = BOTTOM_RIGHT;
                  erd->anchor = br;
               }

             if (!e_efx_move(obj, speed, position, total_time, NULL, NULL))
               {
                  evas_object_event_callback_del_full(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, e->resize_data);
                  free(erd);
                  e->resize_data = NULL;
                  e_efx_free(e);
                  return EINA_FALSE;
               }
             else
               erd->moving = 1;
          }
        else
          evas_object_move(obj, position->x, position->y);
     }
   if (!eina_dbl_exact(total_time, 0))
     erd->anim = ecore_animator_timeline_add(total_time, (Ecore_Timeline_Cb)_resize_cb, erd);
   else
     _resize_cb(erd, 1.0);

   return EINA_TRUE;
}

EAPI void
e_efx_resize_reset(Evas_Object *obj)
{
   _resize_stop(obj, EINA_TRUE);
}

EAPI void
e_efx_resize_stop(Evas_Object *obj)
{
   _resize_stop(obj, EINA_FALSE);
}
