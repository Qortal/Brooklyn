#include "e_efx_private.h"

Eina_Bool
e_efx_rotate_center_init(E_EFX *e, const Evas_Point *center)
{
   if (center)
     {
        if (!e->map_data.rotate_center) e->map_data.rotate_center = malloc(sizeof(Evas_Point));
        EINA_SAFETY_ON_NULL_RETURN_VAL(e->map_data.rotate_center, EINA_FALSE);
        e->map_data.rotate_center->x = center->x, e->map_data.rotate_center->y = center->y;
     }
   else
     {
        free(e->map_data.rotate_center);
        e->map_data.rotate_center = NULL;
     }
   return EINA_TRUE;
}

Eina_Bool
e_efx_zoom_center_init(E_EFX *e, const Evas_Point *center)
{
   if (center)
     {
        if (!e->map_data.zoom_center) e->map_data.zoom_center = malloc(sizeof(Evas_Point));
        EINA_SAFETY_ON_NULL_RETURN_VAL(e->map_data.zoom_center, EINA_FALSE);
        e->map_data.zoom_center->x = center->x, e->map_data.zoom_center->y = center->y;
     }
   else
     {
        free(e->map_data.zoom_center);
        e->map_data.zoom_center = NULL;
     }
   return EINA_TRUE;
}

Eina_Bool
e_efx_move_center_init(E_EFX *e, const Evas_Point *center)
{
   if (center)
     {
        if (!e->map_data.move_center) e->map_data.move_center = malloc(sizeof(Evas_Point));
        EINA_SAFETY_ON_NULL_RETURN_VAL(e->map_data.move_center, EINA_FALSE);
        e->map_data.move_center->x = center->x, e->map_data.move_center->y = center->y;
     }
   else
     {
        free(e->map_data.move_center);
        e->map_data.move_center = NULL;
     }
   return EINA_TRUE;
}

Evas_Map *
e_efx_map_new(Evas_Object *obj)
{
   Evas_Map *map;

   map = evas_map_new(4);
   evas_map_smooth_set(map, EINA_FALSE);
   evas_map_util_points_populate_from_object(map, obj);
   return map;
   (void)e_efx_speed_str;
}

void
e_efx_map_set(Evas_Object *obj, Evas_Map *map)
{
   evas_object_map_set(obj, map);
   evas_object_map_enable_set(obj, !!map);
   if (map) evas_map_free(map);
}

void
e_efx_rotate_helper(E_EFX *e, Evas_Object *obj, Evas_Map *map, double degrees)
{

   if (e->map_data.rotate_center)
     evas_map_util_rotate(map, degrees, e->map_data.rotate_center->x, e->map_data.rotate_center->y);
   else
     {
        Evas_Coord x, y, w, h;
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_rotate(map, degrees, x + (w / 2), y + (h / 2));
     }
   //DBG("rotation: %g", degrees);
//   _size_debug(e->obj);
}

void
e_efx_maps_apply(E_EFX *e, Evas_Object *obj, Evas_Map *map, Eina_Bool rotate, Eina_Bool spin, Eina_Bool zoom)
{
   Eina_Bool new = EINA_FALSE;
   if ((!e->owner) && (!e->rotate_data) && (!e->spin_data) && (!e->zoom_data)) return;
   if (!map)
     {
        map = e_efx_map_new(obj);
        new = EINA_TRUE;
     }
   if (rotate && (e->rotate_data || (e->owner && e->owner->rotate_data))) _e_efx_rotate_calc(e->rotate_data, e->owner ? e->owner->rotate_data : NULL, obj, map);
   if (spin && (e->spin_data || (e->owner && e->owner->spin_data))) _e_efx_spin_calc(e->spin_data, e->owner ? e->owner->spin_data : NULL, obj, map);
   if (zoom && (e->zoom_data || (e->owner && e->owner->zoom_data))) _e_efx_zoom_calc(e->zoom_data, e->owner ? e->owner->zoom_data : NULL, obj, map);
   if (new) e_efx_map_set(obj, map);
//   DBG("%p: %s %s %s", obj, rotate ? "rotate" : "", spin ? "spin" : "", zoom ? "zoom" : "");
}

void
e_efx_clip_setup(Evas_Object *obj, Evas_Object *clip)
{
   Evas_Coord x, y, w, h;
   if ((!obj) || (!clip)) return;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   evas_object_move(clip, x - w, y - h);
   evas_object_resize(clip, 3 * w, 3 * h);
}
