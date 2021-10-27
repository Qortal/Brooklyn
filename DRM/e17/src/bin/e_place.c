#include "e.h"

E_API void
e_place_zone_region_smart_cleanup(E_Zone *zone)
{
   E_Desk *desk;
   Eina_List *clients = NULL;
   E_Client *ec;

   E_OBJECT_CHECK(zone);
   desk = e_desk_current_get(zone);
   E_CLIENT_FOREACH(ec)
     {
        /* Build a list of windows on this desktop and not iconified. */
        if ((ec->desk == desk) && (!ec->iconic) &&
            (!ec->lock_user_location) && (!e_client_util_ignored_get(ec)))
          {
             int area;
             Eina_List *ll;
             E_Client *ec2;

             /* Ordering windows largest to smallest gives better results */
             area = ec->w * ec->h;
             EINA_LIST_FOREACH(clients, ll, ec2)
               {
                  int testarea;

                  testarea = ec2->w * ec2->h;
                  /* Insert the ec if larger than the current ec */
                  if (area >= testarea)
                    {
                       clients = eina_list_prepend_relative_list(clients, ec, ll);
                       break;
                    }
               }
             /* Looped over all clients without placing, so place at end */
             if (!ll) clients = eina_list_append(clients, ec);
          }
     }

   /* Loop over the clients moving each one using the smart placement */
   EINA_LIST_FREE(clients, ec)
     {
        int new_x = zone->x, new_y = zone->y;
        Eina_List *l = eina_list_append(NULL, ec);

        e_place_zone_region_smart(zone, l, zone->x, zone->y,
                                  ec->w, ec->h, &new_x, &new_y);
        eina_list_free(l);
        evas_object_move(ec->frame, new_x, new_y);
     }
}

static int
_e_place_cb_sort_cmp(const void *v1, const void *v2)
{
   return (*((int *)v1)) - (*((int *)v2));
}

static Eina_Bool
ignore_client_and_break(const E_Client *ec)
{
   if (ec->fullscreen) return EINA_TRUE;
   if (ec->maximized)
     {
        E_Maximize max = ec->maximized & E_MAXIMIZE_DIRECTION;

        if (max == E_MAXIMIZE_FULLSCREEN) return EINA_TRUE;
        if (max == E_MAXIMIZE_BOTH) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
ignore_client(const E_Client *ec, const Eina_List *skiplist)
{
   if (eina_list_data_find(skiplist, ec)) return EINA_TRUE;
   if (e_client_util_ignored_get(ec)) return EINA_TRUE;
   if (!evas_object_visible_get(ec->frame)) return EINA_TRUE;
   if ((!e_client_has_xwindow(ec)) && (ec->netwm.type != E_WINDOW_TYPE_NORMAL)) return EINA_TRUE;

   return EINA_FALSE;
}

static int
_e_place_coverage_client_add(Eina_List *skiplist, int ar, int x, int y, int w, int h)
{
   E_Client *ec;
   int x2, y2, w2, h2;
   int iw, ih;
   int x0, x00, yy0, y00;

   E_CLIENT_REVERSE_FOREACH(ec)
     {
        if (ignore_client(ec, skiplist)) continue;
        if (ignore_client_and_break(ec)) break;
        x2 = ec->x; y2 = ec->y; w2 = ec->w; h2 = ec->h;
        if (E_INTERSECTS(x, y, w, h, x2, y2, w2, h2))
          {
             x0 = x;
             if (x < x2) x0 = x2;
             x00 = (x + w);
             if ((x2 + w2) < (x + w)) x00 = (x2 + w2);
             yy0 = y;
             if (y < y2) yy0 = y2;
             y00 = (y + h);
             if ((y2 + h2) < (y + h)) y00 = (y2 + h2);
             iw = x00 - x0;
             ih = y00 - yy0;
             ar += (iw * ih);
          }
     }
   return ar;
}

static int
_e_place_coverage_zone_obstacles_add_single(E_Zone_Obstacle *obs, int ar, int x, int y, int w, int h)
{
   int x2, y2, w2, h2;
   int x0, x00, yy0, y00;
   int iw, ih;

   x2 = obs->x; y2 = obs->y; w2 = obs->w; h2 = obs->h;
   if (!E_INTERSECTS(x, y, w, h, x2, y2, w2, h2)) return ar;

   /* FIXME: this option implies that windows should be resized when
    * an autohide shelf toggles its visibility, but it is not used correctly
    * and is instead used to determine whether shelves can be overlapped
    */
   if (!e_config->border_fix_on_shelf_toggle) return 0x7fffffff;

   x0 = x;
   if (x < x2) x0 = x2;
   x00 = (x + w);
   if ((x2 + w2) < (x + w)) x00 = (x2 + w2);
   yy0 = y;
   if (y < y2) yy0 = y2;
   y00 = (y + h);
   if ((y2 + h2) < (y + h)) y00 = (y2 + h2);
   iw = x00 - x0;
   ih = y00 - yy0;
   ar += (iw * ih);
   return ar;
}

static int
_e_place_coverage_zone_obstacles_add(E_Desk *desk, int ar, int x, int y, int w, int h)
{
   E_Zone_Obstacle *obs;

   EINA_INLIST_FOREACH(desk->obstacles, obs)
     ar = _e_place_coverage_zone_obstacles_add_single(obs, ar, x, y, w, h);
   EINA_INLIST_FOREACH(desk->zone->obstacles, obs)
     ar = _e_place_coverage_zone_obstacles_add_single(obs, ar, x, y, w, h);
   return ar;
}

static int *
_e_place_array_resize(int *array, int *pos, int *size)
{
   (*pos)++;
   if (*pos > *size)
     {
        *size += 32;
        E_REALLOC(array, int, *size);
     }
   return array;
}

static void
_e_place_desk_region_smart_obstacle_add(char *u_x, char *u_y, int **a_x, int **a_y, int *a_w, int *a_h, int *a_alloc_w, int *a_alloc_h, int zx, int zy, int zw, int zh, int bx, int by, int bw, int bh)
{
   if (bx < zx)
     {
        bw += bx;
        bx = zx;
     }
   if ((bx + bw) > zx + zw) bw = zx + zw - bx;
   if (bx >= zx + zw) return;
   if (by < zy)
     {
        bh += by;
        by = zy;
     }
   if ((by + bh) > zy + zh) bh = zy + zh - by;
   if (by >= zy + zh) return;
   if (!u_x[bx])
     {
        *a_x = _e_place_array_resize(*a_x, a_w, a_alloc_w);
        (*a_x)[*a_w - 1] = bx;
        u_x[bx] = 1;
     }
   if (!u_x[bx + bw])
     {
        *a_x = _e_place_array_resize(*a_x, a_w, a_alloc_w);
        (*a_x)[*a_w - 1] = bx + bw;
        u_x[bx + bw] = 1;
     }
   if (!u_y[by])
     {
        *a_y = _e_place_array_resize(*a_y, a_h, a_alloc_h);
        (*a_y)[*a_h - 1] = by;
        u_y[by] = 1;
     }
   if (!u_y[by + bh])
     {
        *a_y = _e_place_array_resize(*a_y, a_h, a_alloc_h);
        (*a_y)[*a_h - 1] = by + bh;
        u_y[by + bh] = 1;
     }
}

/* determine whether the "overlapping" area for a given geometry
 * is less than the previous smallest overlapping area.
 * if this obstacle geometry has less overlap, set it as the current
 * geometry to use
 */
static int
_e_place_desk_region_smart_area_check(Eina_List *skiplist, int x, int y, int w, int h, E_Desk *desk, int area, int *rx, int *ry)
{
   int ar = 0;

   ar = _e_place_coverage_client_add(skiplist, ar, x, y, w, h);

   if (e_config->window_placement_policy == E_WINDOW_PLACEMENT_SMART)
     ar = _e_place_coverage_zone_obstacles_add(desk, ar, x, y, w, h);

   if (ar < area)
     {
        *rx = x;
        *ry = y;
        if (ar == 0) return ar;
     }
   return ar;
}

/* calculate optimal placement based on "overlapping" area using:
 * - an obstacle's top-left and bottom-right points
 * - list of clients to skip
 * - current desk
 * - current least overlapping area
 * - pointers to current coords to use for placement
 * and then return the new least overlapping area
 */
static int
_e_place_desk_region_smart_area_calc(int x, int y, int xx, int yy, int zx, int zy, int zw, int zh, int w, int h, Eina_List *skiplist, E_Desk *desk, int area, int *rx, int *ry)
{
   /* check top-left corner placement */
   if ((x <= MAX(zx, zx + (zw - w))) && (y <= MAX(zy, zy + (zh - h))))
     {
        int ar = _e_place_desk_region_smart_area_check(skiplist, x, y, w, h, desk, area, rx, ry);
        if (!ar) return ar;
        if (ar < area) area = ar;
     }
   /* check top-right corner placement */
   if ((MAX(zx, xx - w) > zx) && (y <= MAX(zy, zy + (zh - h))))
     {
        int ar = _e_place_desk_region_smart_area_check(skiplist, xx - w, y, w, h, desk, area, rx, ry);
        if (!ar) return ar;
        if (ar < area) area = ar;
     }
   /* check bottom-right corner placement */
   if ((MAX(zx, xx - w) > zx) && (MAX(zy, yy - h) > zy))
     {
        int ar = _e_place_desk_region_smart_area_check(skiplist, xx - w, yy - h, w, h, desk, area, rx, ry);
        if (!ar) return ar;
        if (ar < area) area = ar;
     }
   /* check bottom-left corner placement */
   if ((x <= MAX(zx, zx + (zw - w))) && (MAX(zy, yy - h) > zy))
     {
        int ar = _e_place_desk_region_smart_area_check(skiplist, x, yy - h, w, h, desk, area, rx, ry);
        if (!ar) return ar;
        if (ar < area) area = ar;
     }
   return area;
}

E_API int
e_place_desk_region_smart(E_Desk *desk, Eina_List *skiplist, int x, int y, int w, int h, int *rx, int *ry)
{
   int a_w = 0, a_h = 0, a_alloc_w = 0, a_alloc_h = 0;
   int *a_x = NULL, *a_y = NULL;
   int zx, zy, zw, zh;
   char *u_x = NULL, *u_y = NULL;
   E_Client *ec;

   *rx = x;
   *ry = y;
#if 0
   /* DISABLE placement entirely for speed testing */
   return 1;
#endif

   if ((w <= 0) || (h <= 0))
     {
        printf("EEEK! trying to place 0x0 window!!!!\n");
        return 1;
     }

   /* FIXME: this NEEDS optimizing */
   a_w = 2;
   a_h = 2;
   a_x = E_NEW(int, 2);
   a_y = E_NEW(int, 2);
   a_alloc_w = 2;
   a_alloc_h = 2;

   zx = desk->zone->x;
   zy = desk->zone->y;
   zw = desk->zone->w;
   zh = desk->zone->h;

   u_x = calloc(zx + zw + 1, sizeof(char));
   u_y = calloc(zy + zh + 1, sizeof(char));

   a_x[0] = zx;
   a_x[1] = zx + zw;
   a_y[0] = zy;
   a_y[1] = zy + zh;

   u_x[zx] = 1;
   u_x[zx + zw] = 1;
   u_y[zy] = 1;
   u_y[zy + zh] = 1;

   if (e_config->window_placement_policy == E_WINDOW_PLACEMENT_SMART)
     {
        E_Zone_Obstacle *obs;

        EINA_INLIST_FOREACH(desk->obstacles, obs)
          {
             int bx, by, bw, bh;

             bx = obs->x;
             by = obs->y;
             bw = obs->w;
             bh = obs->h;
             if (E_INTERSECTS(bx, by, bw, bh, zx, zy, zw, zh))
               _e_place_desk_region_smart_obstacle_add(u_x, u_y, &a_x, &a_y,
                 &a_w, &a_h, &a_alloc_w, &a_alloc_h, zx, zy, zw, zh, bx, by, bw, bh);
          }
        EINA_INLIST_FOREACH(desk->zone->obstacles, obs)
          {
             int bx, by, bw, bh;

             bx = obs->x;
             by = obs->y;
             bw = obs->w;
             bh = obs->h;
             if (E_INTERSECTS(bx, by, bw, bh, zx, zy, zw, zh))
               _e_place_desk_region_smart_obstacle_add(u_x, u_y, &a_x, &a_y,
                 &a_w, &a_h, &a_alloc_w, &a_alloc_h, zx, zy, zw, zh, bx, by, bw, bh);
          }
     }

   E_CLIENT_REVERSE_FOREACH(ec)
     {
        int bx, by, bw, bh;

        if (ignore_client(ec, skiplist)) continue;
        if (ignore_client_and_break(ec)) break;

        bx = ec->x;
        by = ec->y;
        bw = ec->w;
        bh = ec->h;

        if (E_INTERSECTS(bx, by, bw, bh, zx, zy, zw, zh))
          _e_place_desk_region_smart_obstacle_add(u_x, u_y, &a_x, &a_y,
            &a_w, &a_h, &a_alloc_w, &a_alloc_h, zx, zy, zw, zh, bx, by, bw, bh);
     }
   qsort(a_x, a_w, sizeof(int), _e_place_cb_sort_cmp);
   qsort(a_y, a_h, sizeof(int), _e_place_cb_sort_cmp);
   free(u_x);
   free(u_y);

   {
      int i, j;
      int area = 0x7fffffff;

      if ((x <= zx + (zw - w)) &&
          (y <= zy + (zh - h)))
        {
           int ar = 0;

           ar = _e_place_coverage_client_add(skiplist, ar,
                                             x, y,
                                             w, h);

           if (e_config->window_placement_policy == E_WINDOW_PLACEMENT_SMART)
             ar = _e_place_coverage_zone_obstacles_add(desk, ar,
                                              x, y,
                                              w, h);

           if (ar < area)
             {
                area = ar;
                *rx = x;
                *ry = y;
                if (ar == 0) goto done;
             }
        }

      /* loop through all the obstacles in the arrays of coords
       * (a_x[i], a_y[j]) is top-left corner of obstacle
       * (a_x[i + 1], a_y[j + 1]) is bottom-right corner of obstacle
       */
      for (j = 0; j < a_h - 1; j++)
        for (i = 0; i < a_w - 1; i++)
          {
             area = _e_place_desk_region_smart_area_calc(a_x[i], a_y[j], a_x[i + 1], a_y[j + 1],
                                                         zx, zy, zw, zh, w, h, skiplist, desk, area, rx, ry);
             if (!area) goto done;
          }
   }
done:
   E_FREE(a_x);
   E_FREE(a_y);

   e_zone_desk_useful_geometry_get(desk->zone, desk, &zx, &zy, &zw, &zh);

   if ((*rx + w) > zx + zw) *rx = zx + zw - w;
   if (*rx < zx) *rx = zx;
   if ((*ry + h) > zy + zh) *ry = zy + zh - h;
   if (*ry < zy) *ry = zy;

//   printf("0 - PLACE %i %i | %ix%i\n", *rx, *ry, w, h);

   return 1;
}

E_API int
e_place_zone_region_smart(E_Zone *zone, Eina_List *skiplist, int x, int y, int w, int h, int *rx, int *ry)
{
   return e_place_desk_region_smart(e_desk_current_get(zone), skiplist,
                                    x, y, w, h, rx, ry);
}

E_API int
e_place_zone_cursor(E_Zone *zone, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h, int it, int *rx, int *ry)
{
   int cursor_x = 0, cursor_y = 0;
   int zone_right, zone_bottom;
   int zx, zy, zw, zh;

   E_OBJECT_CHECK_RETURN(zone, 0);

   ecore_evas_pointer_xy_get(e_comp->ee, &cursor_x, &cursor_y);
   *rx = cursor_x - (w >> 1);
   *ry = cursor_y - (it >> 1);

   e_zone_useful_geometry_get(zone, &zx, &zy, &zw, &zh);

   if (*rx < zone->x)
     *rx = zx;

   if (*ry < zone->y)
     *ry = zy;

   zone_right = zx + zw;
   zone_bottom = zy + zh;

   if ((*rx + w) > zone_right)
     *rx = zone_right - w;

   if ((*ry + h) > zone_bottom)
     *ry = zone_bottom - h;

   return 1;
}

E_API int
e_place_zone_manual(E_Zone *zone, int w, int h, int *rx, int *ry)
{
   int cursor_x = 0, cursor_y = 0;

   E_OBJECT_CHECK_RETURN(zone, 0);

   ecore_evas_pointer_xy_get(e_comp->ee, &cursor_x, &cursor_y);
   if (rx) *rx = cursor_x - (w >> 1);
   if (ry) *ry = cursor_y - (h >> 1);

   return 1;
}

