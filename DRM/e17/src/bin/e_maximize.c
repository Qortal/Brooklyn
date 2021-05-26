#include "e.h"

typedef struct _E_Maximize_Rect E_Maximize_Rect;

struct _E_Maximize_Rect
{
   int x1, yy1, x2, y2;
};

#define OBSTACLE(_x1, _y1, _x2, _y2)                              \
  {                                                               \
     r = E_NEW(E_Maximize_Rect, 1);                               \
     r->x1 = (_x1); r->yy1 = (_y1); r->x2 = (_x2); r->y2 = (_y2); \
     rects = eina_list_append(rects, r);                          \
  }

static void _e_maximize_client_rects_fill(const E_Client *ec, Eina_List *list, int *x1, int *yy1, int *x2, int *y2, E_Maximize dir);
static void _e_maximize_client_rects_fill_both(const E_Client *ec, Eina_List *rects, int *x1, int *yy1, int *x2, int *y2);
static void _e_maximize_client_rects_fill_horiz(const E_Client *ec, Eina_List *rects, int *x1, int *x2, int *bx, int *by, int *bw, int *bh);
static void _e_maximize_client_rects_fill_vert(const E_Client *ec, Eina_List *rects, int *yy1, int *y2, int *bx, int *by, int *bw, int *bh);

E_API void
e_maximize_client_shelf_fit(const E_Client *ec, int *x1, int *yy1, int *x2, int *y2, E_Maximize dir)
{
   e_maximize_client_shelf_fill(ec, x1, yy1, x2, y2, dir);
}

E_API void
e_maximize_client_dock_fit(const E_Client *ec, int *x1, int *yy1, int *x2, int *y2)
{
   E_Client *ec2;
   int cx1, cx2, cy1, cy2;

   cx1 = ec->zone->x;
   if (x1) cx1 = *x1;

   cy1 = ec->zone->y;
   if (yy1) cy1 = *yy1;

   cx2 = ec->zone->x + ec->zone->w;
   if (x2) cx2 = *x2;

   cy2 = ec->zone->y + ec->zone->h;
   if (y2) cy2 = *y2;

   E_CLIENT_FOREACH(ec2)
     {
        enum
        {
           NONE,
           TOP,
           RIGHT,
           BOTTOM,
           LEFT
        } edge = NONE;

        if ((ec2->zone != ec->zone) || (ec2 == ec) ||
            (ec2->netwm.type != E_WINDOW_TYPE_DOCK))
          continue;

        if (((ec2->x == ec2->zone->x) || ((ec2->x + ec2->w) == (ec2->zone->x + ec2->zone->w))) &&
            ((ec2->y == ec2->zone->y) || ((ec2->x + ec2->h) == (ec2->zone->x + ec2->zone->h))))
          {
             /* corner */
             if (ec2->w > ec2->h)
               {
                  if (ec2->y == ec2->zone->y)
                    edge = TOP;
                  else if ((ec2->y + ec2->h) == (ec2->zone->y + ec2->zone->h))
                    edge = BOTTOM;
               }
             else
               {
                  if ((ec2->x + ec2->w) == (ec2->zone->x + ec2->zone->w))
                    edge = RIGHT;
                  else if (ec2->x == ec2->zone->x)
                    edge = LEFT;
               }
          }
        else
          {
             if (ec2->y == ec2->zone->y)
               edge = TOP;
             else if ((ec2->y + ec2->h) == (ec2->zone->y + ec2->zone->h))
               edge = BOTTOM;
             else if (ec2->x == ec2->zone->x)
               edge = LEFT;
             else if ((ec2->x + ec2->w) == (ec2->zone->x + ec2->zone->w))
               edge = RIGHT;
          }

        switch (edge)
          {
           case TOP:
             if ((ec2->y + ec2->h) > cy1)
               cy1 = (ec2->y + ec2->h);
             break;

           case RIGHT:
             if (ec2->x < cx2)
               cx2 = ec2->x;
             break;

           case BOTTOM:
             if (ec2->y < cy2)
               cy2 = ec2->y;
             break;

           case LEFT:
             if ((ec2->x + ec2->w) > cx1)
               cx1 = (ec2->x + ec2->w);
             break;

           case NONE:
             printf("Crazy people. Dock isn't at the edge.\n");
             break;
          }
     }

   if (x1) *x1 = cx1;
   if (yy1) *yy1 = cy1;
   if (x2) *x2 = cx2;
   if (y2) *y2 = cy2;
}

E_API void
e_maximize_client_shelf_fill(const E_Client *ec, int *x1, int *yy1, int *x2, int *y2, E_Maximize dir)
{
   Eina_List *l, *rects = NULL;
   E_Shelf *es;
   E_Maximize_Rect *r;

   l = e_shelf_list_all();
   EINA_LIST_FREE(l, es)
     {
        if (es->cfg->overlap) continue;
        if (!e_shelf_desk_visible(es, ec->desk)) continue;
        OBSTACLE(es->x + es->zone->x, es->y + es->zone->y,
                 es->x + es->zone->x + es->w, es->y + es->zone->y + es->h);
     }
   if (rects)
     {
        _e_maximize_client_rects_fill(ec, rects, x1, yy1, x2, y2, dir);
        E_FREE_LIST(rects, free);
     }
}

E_API void
e_maximize_client_client_fill(const E_Client *ec, int *x1, int *yy1, int *x2, int *y2, E_Maximize dir)
{
   Eina_List *rects = NULL;
   E_Maximize_Rect *r;
   E_Client *ec2;

   E_CLIENT_FOREACH(ec2)
     {
        if ((ec2->zone != ec->zone) || (ec == ec2) || (ec2->desk != ec->desk && !ec2->sticky) || (ec2->iconic))
          continue;
        OBSTACLE(ec2->x, ec2->y, ec2->x + ec2->w, ec2->y + ec2->h);
     }
   if (rects)
     {
        _e_maximize_client_rects_fill(ec, rects, x1, yy1, x2, y2, dir);
        E_FREE_LIST(rects, free);
     }
}

static void
_e_maximize_client_rects_fill(const E_Client *ec, Eina_List *rects, int *x1, int *yy1, int *x2, int *y2, E_Maximize dir)
{
   if ((dir & E_MAXIMIZE_DIRECTION) == E_MAXIMIZE_BOTH)
     {
        _e_maximize_client_rects_fill_both(ec, rects, x1, yy1, x2, y2);
     }
   else
     {
        int bx, by, bw, bh;

        bx = E_CLAMP(ec->x, ec->zone->x, ec->zone->x + ec->zone->w);
        by = E_CLAMP(ec->y, ec->zone->y, ec->zone->y + ec->zone->h);
        bw = E_CLAMP(ec->w, 0, ec->zone->w);
        bh = E_CLAMP(ec->h, 0, ec->zone->h);

        if ((dir & E_MAXIMIZE_DIRECTION) == E_MAXIMIZE_HORIZONTAL)
          _e_maximize_client_rects_fill_horiz(ec, rects, x1, x2, &bx, &by, &bw, &bh);
        else if (((dir & E_MAXIMIZE_DIRECTION) == E_MAXIMIZE_VERTICAL) ||
                 ((dir & E_MAXIMIZE_LEFT) == E_MAXIMIZE_LEFT) ||
                 ((dir & E_MAXIMIZE_RIGHT) == E_MAXIMIZE_RIGHT))
          _e_maximize_client_rects_fill_vert(ec, rects, yy1, y2, &bx, &by, &bw, &bh);
     }
}

static void
_e_maximize_client_rects_fill_both(const E_Client *ec, Eina_List *rects, int *x1, int *yy1, int *x2, int *y2)
{
   int hx1, hy1, hx2, hy2;
   int vx1, vy1, vx2, vy2;
   int bx, by, bw, bh;

   hx1 = vx1 = ec->zone->x;
   if (x1) hx1 = vx1 = *x1;

   hy1 = vy1 = ec->zone->y;
   if (yy1) hy1 = vy1 = *yy1;

   hx2 = vx2 = ec->zone->x + ec->zone->w;
   if (x2) hx2 = vx2 = *x2;

   hy2 = vy2 = ec->zone->y + ec->zone->h;
   if (y2) hy2 = vy2 = *y2;

   /* Init working values, try maximizing horizontally first */
   bx = ec->saved.x ?: ec->x;
   by = ec->saved.y ?: ec->y;
   bw = ec->saved.w ?: ec->w;
   bh = ec->saved.h ?: ec->h;
   _e_maximize_client_rects_fill_horiz(ec, rects, &hx1, &hx2, &bx, &by, &bw, &bh);
   _e_maximize_client_rects_fill_vert(ec, rects, &hy1, &hy2, &bx, &by, &bw, &bh);

   /* Reset working values, try maximizing vertically first */
   bx = ec->saved.x ?: ec->x;
   by = ec->saved.y ?: ec->y;
   bw = ec->saved.w ?: ec->w;
   bh = ec->saved.h ?: ec->h;
   _e_maximize_client_rects_fill_vert(ec, rects, &vy1, &vy2, &bx, &by, &bw, &bh);
   _e_maximize_client_rects_fill_horiz(ec, rects, &vx1, &vx2, &bx, &by, &bw, &bh);

   /* Use the result set that gives the largest volume */
   if (((hx2 - hx1) * (hy2 - hy1)) > ((vx2 - vx1) * (vy2 - vy1)))
     {
        if (x1) *x1 = hx1;
        if (yy1) *yy1 = hy1;
        if (x2) *x2 = hx2;
        if (y2) *y2 = hy2;
     }
   else
     {
        if (x1) *x1 = vx1;
        if (yy1) *yy1 = vy1;
        if (x2) *x2 = vx2;
        if (y2) *y2 = vy2;
     }
}

static void
_e_maximize_client_rects_fill_horiz(const E_Client *ec, Eina_List *rects, int *x1, int *x2, int *bx, int *by, int *bw, int *bh)
{
   Eina_List *l;
   E_Maximize_Rect *rect;
   int cx1, cx2;

   cx1 = ec->zone->x;
   if (x1) cx1 = *x1;

   cx2 = ec->zone->x + ec->zone->w;
   if (x2) cx2 = *x2;

   /* Expand left */
   EINA_LIST_FOREACH(rects, l, rect)
     {
        if ((rect->x2 > cx1) && (rect->x2 <= *bx) &&
            E_INTERSECTS(0, rect->yy1, ec->zone->w, (rect->y2 - rect->yy1), 0, *by, ec->zone->w, *bh))
          {
             cx1 = rect->x2;
          }
     }
   *bw += (*bx - cx1);
   *bx = cx1;

   /* Expand right */
   EINA_LIST_FOREACH(rects, l, rect)
     {
        if ((rect->x1 < cx2) && (rect->x1 >= (*bx + *bw)) &&
            E_INTERSECTS(0, rect->yy1, ec->zone->w, (rect->y2 - rect->yy1), 0, *by, ec->zone->w, *bh))
          {
             cx2 = rect->x1;
          }
     }
   *bw = (cx2 - cx1);

   if (x1) *x1 = cx1;
   if (x2) *x2 = cx2;
}

static void
_e_maximize_client_rects_fill_vert(const E_Client *ec, Eina_List *rects, int *yy1, int *y2, int *bx, int *by, int *bw, int *bh)
{
   Eina_List *l;
   E_Maximize_Rect *rect;
   int cy1, cy2;

   cy1 = ec->zone->y;
   if (yy1) cy1 = *yy1;

   cy2 = ec->zone->y + ec->zone->h;
   if (y2) cy2 = *y2;

   /* Expand up */
   EINA_LIST_FOREACH(rects, l, rect)
     {
        if ((rect->y2 > cy1) && ((rect->yy1 <= *by) || (rect->y2 <= *by)) &&
            E_INTERSECTS(rect->x1, 0, (rect->x2 - rect->x1), ec->zone->h, *bx, 0, *bw, ec->zone->h))
          {
             cy1 = rect->y2;
          }
     }
   *bh += (*by - cy1);
   *by = cy1;

   /* Expand down */
   EINA_LIST_FOREACH(rects, l, rect)
     {
        if ((rect->yy1 < cy2) && (rect->yy1 >= (*by + *bh)) &&
            E_INTERSECTS(rect->x1, 0, (rect->x2 - rect->x1), ec->zone->h, *bx, 0, *bw, ec->zone->h))
          {
             cy2 = rect->yy1;
          }
     }
   *bh = (cy2 - cy1);

   if (yy1) *yy1 = cy1;
   if (y2) *y2 = cy2;
}

