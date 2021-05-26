#include "e_efx_private.h"

typedef struct E_Efx_Fade_Data
{
   E_EFX *e;
   E_Efx_Effect_Speed speed;
   Ecore_Animator *anim;
   Evas_Object *clip;
   E_Efx_Color start;
   E_Efx_Color color;
   unsigned char alpha[2];
   E_Efx_End_Cb cb;
   void *data;
} E_Efx_Fade_Data;


static void
_clip_setup(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_EFX *e;
   E_Efx_Fade_Data *efd;

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return;
   efd = e->fade_data;
   e_efx_clip_setup(obj, efd->clip);
}

static void
_obj_del(E_Efx_Fade_Data *efd, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (efd->anim) ecore_animator_del(efd->anim);
   evas_object_event_callback_del_full(efd->e->obj, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)_clip_setup, efd);
   evas_object_event_callback_del_full(efd->e->obj, EVAS_CALLBACK_MOVE, (Evas_Object_Event_Cb)_clip_setup, efd);
   if (efd->clip)
     {
        Evas_Object *clip;

        clip = evas_object_clip_get(efd->clip);
        evas_object_clip_unset(efd->e->obj);
        evas_object_del(efd->clip);
        efd->clip = NULL;
        if (clip && (!obj)) //obj is only passed during actual del
          evas_object_clip_set(efd->e->obj, clip);
     }
   efd->e->fade_data = NULL;
   if ((!efd->e->owner) && (!efd->e->followers)) e_efx_free(efd->e);
   free(efd);
}

static Eina_Bool
_fade_cb(E_Efx_Fade_Data *efd, double pos)
{
   double factor;
   unsigned char r, g, b, a;

   if (pos < 1.0)
     {
        r = efd->start.r;
        g = efd->start.g;
        b = efd->start.b;
        a = efd->alpha[0];
        factor = ecore_animator_pos_map(pos, (Ecore_Pos_Map)efd->speed, 0, 0);
        if (efd->color.r != efd->start.r)
          r -= lround(factor * ((int)efd->start.r - (int)efd->color.r));
        if (efd->color.g != efd->start.g)
          g -= lround(factor * ((int)efd->start.g - (int)efd->color.g));
        if (efd->color.b != efd->start.b)
          b -= lround(factor * ((int)efd->start.b - (int)efd->color.b));
        if (efd->alpha[0] != efd->alpha[1])
          a -= lround(factor * ((int)efd->alpha[0] - (int)efd->alpha[1]));
        evas_object_color_set(efd->clip, MIN(r, a), MIN(g, a), MIN(b, a), a);
//        _color_debug(efd->clip);
        return EINA_TRUE;
     }
   else
     /* lround will usually be off by 1 at the end, so we manually set this here */
     evas_object_color_set(efd->clip, MIN(efd->color.r, efd->alpha[1]), MIN(efd->color.g, efd->alpha[1]), MIN(efd->color.b, efd->alpha[1]), efd->alpha[1]);

   efd->anim = NULL;
   E_EFX_QUEUE_CHECK(efd);
   return EINA_TRUE;
}

static void
_fade_stop(Evas_Object *obj, Eina_Bool reset)
{
   E_EFX *e;
   E_Efx_Fade_Data *efd;

   e = evas_object_data_get(obj, "e_efx-data");
   if ((!e) || (!e->fade_data)) return;
   efd = e->fade_data;
   if (reset)
     {
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, efd);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)_clip_setup, efd);
        evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOVE, (Evas_Object_Event_Cb)_clip_setup, efd);
        if (e_efx_queue_complete(efd->e, efd))
          e_efx_queue_process(efd->e);
        _obj_del(efd, NULL, NULL, NULL);
        INF("reset faded object %p", obj);
     }
   else
     {
        INF("stopped faded object %p", obj);
        if (efd->anim) ecore_animator_del(efd->anim);
        efd->anim = NULL;
        if (e_efx_queue_complete(efd->e, efd))
          e_efx_queue_process(efd->e);
     }
}

void
e_efx_fade_reclip(void *fade_data)
{
   E_Efx_Fade_Data *efd = fade_data;
   Evas_Object *clip;

   clip = evas_object_clip_get(efd->e->obj);
   if (clip == efd->clip) return;
   evas_object_clip_set(efd->e->obj, efd->clip);
   if (clip)
     evas_object_clip_set(efd->clip, clip);
   e_efx_clip_setup(efd->e->obj, efd->clip);
}

EAPI Eina_Bool
e_efx_fade(Evas_Object *obj, E_Efx_Effect_Speed speed, E_Efx_Color *ec, unsigned char alpha, double total_time, E_Efx_End_Cb cb, const void *data)
{
   E_EFX *e;
   E_Efx_Fade_Data *efd;
   int r, g, b, a;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);
   efd = e->fade_data;
   if (!efd)
     {
        e->fade_data = efd = calloc(1, sizeof(E_Efx_Fade_Data));
        EINA_SAFETY_ON_NULL_RETURN_VAL(efd, EINA_FALSE);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, e->fade_data);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, (Evas_Object_Event_Cb)_clip_setup, e->fade_data);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, (Evas_Object_Event_Cb)_clip_setup, e->fade_data);
        efd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
     }

   efd->e = e;
   e_efx_fade_reclip(efd);
   evas_object_show(efd->clip);
   efd->alpha[1] = alpha;
   efd->cb = cb;
   efd->data = (void*)data;
   evas_object_color_get(efd->clip, &r, &g, &b, &a);
   efd->start.r = r;
   efd->start.g = g;
   efd->start.b = b;
   efd->alpha[0] = a;
   if (ec)
     {
        efd->color.r = ec->r;
        efd->color.g = ec->g;
        efd->color.b = ec->b;
     }
   else efd->color = (E_Efx_Color){255, 255, 255};
   INF("fade: %p || %d/%d/%d/%d => %d/%d/%d/%d %s over %gs", obj, efd->start.r, efd->start.g, efd->start.b, efd->alpha[0], efd->color.r, efd->color.g, efd->color.b, efd->alpha[1], e_efx_speed_str[speed], total_time);
   if (efd->anim) ecore_animator_del(efd->anim);
   efd->anim = NULL;
   if (!eina_dbl_exact(total_time, 0))
     efd->anim = ecore_animator_timeline_add(total_time, (Ecore_Timeline_Cb)_fade_cb, efd);
   else
     _fade_cb(efd, 1.0);

   return EINA_TRUE;
}

EAPI void
e_efx_fade_reset(Evas_Object *obj)
{
   _fade_stop(obj, EINA_TRUE);
}

EAPI void
e_efx_fade_stop(Evas_Object *obj)
{
   _fade_stop(obj, EINA_FALSE);
}
