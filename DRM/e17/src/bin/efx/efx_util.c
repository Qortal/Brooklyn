#include "e_efx_private.h"

static void
_obj_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_EFX *e;

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return;
   if (e->owner)
     e->owner->followers = eina_list_remove(e->owner->followers, e);
   e->owner = NULL;
   e_efx_free(e);
}

EAPI void
e_efx_realize(Evas_Object *obj)
{
   E_EFX *e;
   Evas_Coord x, y, ox, oy, w, h;
   Evas_Point p1, p2;
   double zw, zh;
   Evas_Map *map;

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return;
   if (!e->map_data.rotate_center) return;
   evas_object_geometry_get(obj, &ox, &oy, &w, &h);
   map = (Evas_Map*)evas_object_map_get(obj);
   if (!map) return;
   evas_map_point_coord_get(map, 0, &p1.x, &p1.y, NULL);
   evas_map_point_coord_get(map, 2, &p2.x, &p2.y, NULL);
   x = lround((double)(p1.x + p2.x) / 2.);
   y = lround((double)(p1.y + p2.y) / 2.);
   if (!eina_dbl_exact(e->map_data.zoom, 0))
     zw = e->map_data.zoom * w, zh = e->map_data.zoom * h;
   else
     zw = w, zh = h;
   x = lround(x - (zw / 2.));
   y = lround(y - (zh / 2.));
   evas_object_move(obj, x, y);
   evas_object_resize(obj, lround(zw), lround(zh));
   e->map_data.zoom = 0;
   free(e->map_data.rotate_center);
   e->map_data.rotate_center = NULL;
   map = e_efx_map_new(obj);
   e_efx_maps_apply(e, obj, map, E_EFX_MAPS_APPLY_ALL);
   e_efx_map_set(obj, map);
   INF("realize: %p - (%d,%d)@%dx%d -> (%d,%d)@%dx%d", obj, ox, oy, w, h, x, y, (int)zw, (int)zh);
}

EAPI Eina_Bool
e_efx_follow(Evas_Object *obj, Evas_Object *follower)
{
   E_EFX *e, *ef;
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(follower, EINA_FALSE);

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);
   while (e->owner) e = e->owner;

   ef = evas_object_data_get(follower, "e_efx-data");
   if (ef)
     {
        if (ef->owner)
          {
             if (ef->owner == e) return EINA_TRUE;
             ef->owner->followers = eina_list_remove(ef->owner->followers, ef);
          }
     }
   else
     ef = e_efx_new(follower);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, EINA_FALSE);
   if ((!ef->zoom_data) && (!ef->rotate_data) && (!ef->spin_data) && (!ef->move_data) && (!ef->bumpmap_data) && (!ef->pan_data) && (!ef->fade_data))
     evas_object_event_callback_priority_add(ef->obj, EVAS_CALLBACK_FREE, EVAS_CALLBACK_PRIORITY_BEFORE, (Evas_Object_Event_Cb)_obj_del, ef);

   ef->owner = e;
   e->followers = eina_list_append(e->followers, ef);
   INF("follow: (owner %p: %u) || (follower %p)", obj, eina_list_count(e->followers), follower);
   return EINA_TRUE;
   (void)e_efx_speed_str;
}

EAPI void
e_efx_unfollow(Evas_Object *obj)
{
   E_EFX *e;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return;
   if (!e->owner) return;
   INF("unfollow: (owner %p) || (follower %p)", e->owner->obj, obj);
   e->owner->followers = eina_list_remove(e->owner->followers, e);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_FREE, (Evas_Object_Event_Cb)_obj_del, e);
   e_efx_free(e->owner);
   e->owner = NULL;
   e_efx_free(e);
}

EAPI Eina_List *
e_efx_followers_get(Evas_Object *obj)
{
   E_EFX *e, *f;
   Eina_List *l, *ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return NULL;
   EINA_LIST_FOREACH(e->followers, l, f)
     ret = eina_list_append(ret, f->obj);
   return ret;
}

EAPI Evas_Object *
e_efx_leader_get(Evas_Object *obj)
{
   E_EFX *e;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return NULL;
   return e->owner ? e->owner->obj : NULL;
}

EAPI void
e_efx_reclip(Evas_Object *obj)
{
   E_EFX *e;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   e = evas_object_data_get(obj, "e_efx-data");
   if (!e) return;
   if (e->fade_data) e_efx_fade_reclip(e->fade_data);
}
