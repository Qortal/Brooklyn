#include "e.h"

typedef struct _E_Resist_Rect E_Resist_Rect;

struct _E_Resist_Rect
{
   int x, y, w, h;
   int v1;
   int resist_out;
};

static void _e_resist_rects(Eina_List *rects, int px, int py, int pw, int ph, int x, int y, int w, int h, int *rx, int *ry, int *rw, int *rh);

E_API int
e_resist_client_position(Eina_List *skiplist,
                                   int px, int py, int pw, int ph,
                                   int x, int y, int w, int h,
                                   int *rx, int *ry, int *rw, int *rh)
{
   Eina_List *l, *rects = NULL;
   E_Resist_Rect *r;
   E_Client *ec;
   E_Desk *desk;
   E_Zone *zone;

   if (!e_config->use_resist)
     {
        *rx = x;
        *ry = y;
        *rw = w;
        *rh = h;
        return 0;
     }

   /* edges of screen */
#define OBSTACLE(_x, _y, _w, _h, _resist)                         \
  {                                                               \
     r = E_NEW(E_Resist_Rect, 1);                                 \
     r->x = _x; r->y = _y; r->w = _w; r->h = _h; r->v1 = _resist; \
     r->resist_out = 0;                                           \
     rects = eina_list_append(rects, r);                          \
  }
#define HOLDER(_x, _y, _w, _h, _resist)                           \
  {                                                               \
     r = E_NEW(E_Resist_Rect, 1);                                 \
     r->x = _x; r->y = _y; r->w = _w; r->h = _h; r->v1 = _resist; \
     r->resist_out = 1;                                           \
     rects = eina_list_append(rects, r);                          \
  }

   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        HOLDER(zone->x, zone->y, zone->w, zone->h, e_config->desk_resist);
     }
   /* FIXME: need to add resist or complete BLOCKS for things like ibar */
   /* can add code here to add more fake obstacles with custom resist values */
   /* here if need be - ie xinerama middle between screens and panels etc. */
   E_CLIENT_FOREACH(ec)
     {
        if (e_client_util_ignored_get(ec) || (!evas_object_visible_get(ec->frame))) continue;
        if (ec->offer_resistance && (!eina_list_data_find(skiplist, ec)))
          {
             OBSTACLE(ec->x, ec->y, ec->w, ec->h, e_config->window_resist);
          }
     }

   desk = e_desk_current_get(e_zone_current_get());
   {
      E_Zone_Obstacle *obs;

      EINA_INLIST_FOREACH(desk->obstacles, obs)
        OBSTACLE(obs->x, obs->y, obs->w, obs->h, e_config->gadget_resist);
      EINA_INLIST_FOREACH(desk->zone->obstacles, obs)
        OBSTACLE(obs->x, obs->y, obs->w, obs->h, e_config->gadget_resist);
   }
   if (rects)
     {
        _e_resist_rects(rects,
                        px, py, pw, ph,
                        x, y, w, h,
                        rx, ry, rw, rh);

        E_FREE_LIST(rects, E_FREE);
     }
   return 1;
}

E_API int
e_resist_gadman_position(Eina_List *skiplist EINA_UNUSED,
                                   int px, int py, int pw, int ph,
                                   int x, int y, int w, int h,
                                   int *rx, int *ry)
{
   Eina_List *l, *rects = NULL;
   E_Resist_Rect *r;
   E_Shelf *es;

   if (!e_config->use_resist)
     {
        *rx = x;
        *ry = y;
        return 0;
     }

   l = e_shelf_list_all();
   EINA_LIST_FREE(l, es)
     {
        if (e_shelf_desk_visible(es, e_desk_current_get(es->zone)))
          {
             OBSTACLE(es->x + es->zone->x, es->y + es->zone->y, es->w, es->h,
                      e_config->gadget_resist);
          }
     }

   if (rects)
     {
        _e_resist_rects(rects,
                        px, py, pw, ph,
                        x, y, w, h,
                        rx, ry, NULL, NULL);

        E_FREE_LIST(rects, E_FREE);
     }
   return 1;
}

static void
_e_resist_rects(Eina_List *rects,
                int px, int py, int pw, int ph,
                int x, int y, int w, int h,
                int *rx, int *ry, int *rw, int *rh)
{
   int dx, dy, dw, dh, d, pd;
   int resist_x = 0, resist_y = 0;
   int resist_w = 0, resist_h = 0;
   Eina_List *l;
   E_Resist_Rect *r;

   dx = x - px;
   dy = y - py;
   dw = w - pw;
   dh = h - ph;

   EINA_LIST_FOREACH(rects, l, r)
     {
        if (E_SPANS_COMMON(r->y, r->h, y, h))
          {
             if (dx > 0)
               {
                  /* moving right */
                  if (r->resist_out)
                    {
                       /* check right edge of windows against left */
                       d = x + w - (r->x + r->w);
                       pd = px + pw - (r->x + r->w);
                       if ((d > 0) && (pd <= 0) && (d <= r->v1))
                         {
                            if (-resist_x < d)
                              resist_x = -d;
                         }
                    }
                  else
                    {
                       /* check left edge of windows against right */
                       d = r->x - (x + w);
                       pd = r->x - (px + pw);
                       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
                         {
                            if (resist_x > d)
                              resist_x = d;
                         }
                    }
               }
             else if (dx < 0)
               {
                  /* moving left */
                  if (r->resist_out)
                    {
                       /* check left edge of windows against right */
                       d = r->x - x;
                       pd = r->x - px;
                       if ((d > 0) && (pd <= 0) && (d <= r->v1))
                         {
                            if (resist_x < d)
                              {
                                 resist_x = d;
                                 resist_w = -d;
                              }
                         }
                    }
                  else
                    {
                       /* check right edge of windows against left */
                       d = x - (r->x + r->w);
                       pd = px - (r->x + r->w);
                       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
                         {
                            if (-resist_x > d)
                              {
                                 resist_x = -d;
                                 resist_w = d;
                              }
                         }
                    }
               }
             if ((dw > 0) && (dx == 0))
               {
                  /* enlarging window by moving lower corner */
                  if (r->resist_out)
                    {
                       /* check right edge of windows against left */
                       d = x + w - (r->x + r->w);
                       pd = px + pw - (r->x + r->w);
                       if ((d > 0) && (pd <= 0) && (d <= r->v1))
                         {
                            if (-resist_w < d)
                              resist_w = -d;
                         }
                    }
                  else
                    {
                       /* check left edge of windows against right */
                       d = r->x - (x + w);
                       pd = r->x - (px + pw);
                       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
                         {
                            if (resist_w > d)
                              resist_w = d;
                         }
                    }
               }
          }
        if (E_SPANS_COMMON(r->x, r->w, x, w))
          {
             if (dy > 0)
               {
                  /* moving down */
                  if (r->resist_out)
                    {
                       /* check bottom edge of windows against top */
                       d = y + h - (r->y + r->h);
                       pd = py + ph - (r->y + r->h);
                       if ((d > 0) && (pd <= 0) && (d <= r->v1))
                         {
                            if (-resist_y < d)
                              resist_y = -d;
                         }
                    }
                  else
                    {
                       /* check top edge of windows against bottom */
                       d = r->y - (y + h);
                       pd = r->y - (py + ph);
                       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
                         {
                            if (resist_y > d)
                              resist_y = d;
                         }
                    }
               }
             else if (dy < 0)
               {
                  /* moving up */
                  if (r->resist_out)
                    {
                       /* check top edge of windows against bottom */
                       d = r->y - y;
                       pd = r->y - py;
                       if ((d > 0) && (pd <= 0) && (d <= r->v1))
                         {
                            if (resist_y < d)
                              {
                                 resist_y = d;
                                 resist_h = -d;
                              }
                         }
                    }
                  else
                    {
                       /* moving up - check bottom edge of windows against top */
                       d = y - (r->y + r->h);
                       pd = py - (r->y + r->h);
                       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
                         {
                            if (-resist_y > d)
                              {
                                 resist_y = -d;
                                 resist_h = d;
                              }
                         }
                    }
               }
             if ((dh > 0) && (dy == 0))
               {
                  /* enlarging window by moving lower corner */
                  if (r->resist_out)
                    {
                       /* check bottom edge of windows against top */
                       d = y + h - (r->y + r->h);
                       pd = py + ph - (r->y + r->h);
                       if ((d > 0) && (pd <= 0) && (d <= r->v1))
                         {
                            if (-resist_h < d)
                              resist_h = -d;
                         }
                    }
                  else
                    {
                       /* check top edge of windows against bottom */
                       d = r->y - (y + h);
                       pd = r->y - (py + ph);
                       if ((d < 0) && (pd >= 0) && (d >= -r->v1))
                         {
                            if (resist_h > d)
                              resist_h = d;
                         }
                    }
               }
          }
     }
   if (rx)
     {
        if (dx != 0)
          *rx = x + resist_x;
        else
          *rx = x;
     }
   if (ry)
     {
        if (dy != 0)
          *ry = y + resist_y;
        else
          *ry = y;
     }
   if (rw)
     {
        if (dw != 0)
          *rw = w + resist_w;
        else
          *rw = w;
     }
   if (rh)
     {
        if (dh != 0)
          *rh = h + resist_h;
        else
          *rh = h;
     }
}

