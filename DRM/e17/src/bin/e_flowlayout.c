#include "e.h"

typedef struct _E_Smart_Data      E_Smart_Data;
typedef struct _E_Flowlayout_Item E_Flowlayout_Item;

struct _E_Smart_Data
{
   Evas_Coord    x, y, w, h;
   Evas_Object  *obj;
   Evas_Object  *clip;
   int           frozen;
   unsigned char changed E_BITFIELD;
   unsigned char horizontal E_BITFIELD;
   unsigned char homogenous E_BITFIELD;
   unsigned char fill E_BITFIELD;
   unsigned char flowright E_BITFIELD;
   unsigned char flowbottom E_BITFIELD;
   Eina_List    *items;
   struct
   {
      Evas_Coord w, h;
   } min, max;
   struct
   {
      double x, y;
   } align;
   int           rows;
   int           cols;
};

struct _E_Flowlayout_Item
{
   E_Smart_Data *sd;
   unsigned char fill_w E_BITFIELD;
   unsigned char fill_h E_BITFIELD;
   unsigned char expand_w E_BITFIELD;
   unsigned char expand_h E_BITFIELD;
   struct
   {
      Evas_Coord w, h;
   } min, max;
   struct
   {
      double x, y;
   } align;
   Evas_Object  *obj;
};

/* local subsystem functions */
static E_Flowlayout_Item *_e_flowlayout_smart_adopt(E_Smart_Data *sd, Evas_Object *obj);
static void               _e_flowlayout_smart_disown(Evas_Object *obj);
static void               _e_flowlayout_smart_item_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void               _e_flowlayout_smart_reconfigure(E_Smart_Data *sd);
static void               _e_flowlayout_smart_extents_calcuate(E_Smart_Data *sd);

static void               _e_flowlayout_smart_init(void);
static void               _e_flowlayout_smart_add(Evas_Object *obj);
static void               _e_flowlayout_smart_del(Evas_Object *obj);
static void               _e_flowlayout_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void               _e_flowlayout_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void               _e_flowlayout_smart_show(Evas_Object *obj);
static void               _e_flowlayout_smart_hide(Evas_Object *obj);
static void               _e_flowlayout_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void               _e_flowlayout_smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void               _e_flowlayout_smart_clip_unset(Evas_Object *obj);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
E_API Evas_Object *
e_flowlayout_add(Evas *evas)
{
   _e_flowlayout_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

E_API int
e_flowlayout_freeze(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   sd->frozen++;
   return sd->frozen;
}

E_API int
e_flowlayout_thaw(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   sd->frozen--;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
   return sd->frozen;
}

E_API void
e_flowlayout_orientation_set(Evas_Object *obj, int horizontal)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->horizontal) && (horizontal)) ||
       ((!sd->horizontal) && (!horizontal))) return;
   sd->horizontal = horizontal;
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
}

E_API int
e_flowlayout_orientation_get(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->horizontal;
}

E_API void
e_flowlayout_flowdirection_set(Evas_Object *obj, int right, int bottom)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((((sd->flowbottom) && (bottom)) ||
        ((!sd->flowbottom) && (!bottom))) &&
       (((sd->flowright) && (right)) ||
        ((!sd->flowright) && (!right)))) return;
   sd->flowright = right;
   sd->flowbottom = bottom;
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
}

E_API void
e_flowlayout_flowdirection_get(Evas_Object *obj, int *right, int *bottom)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (right) *right = sd->flowright;
   if (bottom) *bottom = sd->flowright;
}

E_API void
e_flowlayout_homogenous_set(Evas_Object *obj, int homogenous)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->homogenous == homogenous) return;
   sd->homogenous = homogenous;
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
}

E_API int
e_flowlayout_homogenous_get(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->homogenous;
}

E_API void
e_flowlayout_fill_set(Evas_Object *obj, int fill)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->fill == fill) return;
   sd->fill = fill;
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
}

E_API int
e_flowlayout_fill_get(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->fill;
}

E_API int
e_flowlayout_pack_start(Evas_Object *obj, Evas_Object *child)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _e_flowlayout_smart_adopt(sd, child);
   sd->items = eina_list_prepend(sd->items, child);
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
   return 0;
}

E_API int
e_flowlayout_pack_end(Evas_Object *obj, Evas_Object *child)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _e_flowlayout_smart_adopt(sd, child);
   sd->items = eina_list_append(sd->items, child);
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
   return (int)eina_list_count(sd->items) - 1;
}

E_API int
e_flowlayout_pack_before(Evas_Object *obj, Evas_Object *child, Evas_Object *before)
{
   E_Smart_Data *sd;
   int i = 0;
   Eina_List *l;
   Evas_Object *item;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _e_flowlayout_smart_adopt(sd, child);
   sd->items = eina_list_prepend_relative(sd->items, child, before);
   EINA_LIST_FOREACH(sd->items, l, item)
     {
        if (item == child) break;
        i++;
     }
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
   return i;
}

E_API int
e_flowlayout_pack_after(Evas_Object *obj, Evas_Object *child, Evas_Object *after)
{
   E_Smart_Data *sd;
   int i = 0;
   Eina_List *l;
   Evas_Object *item;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _e_flowlayout_smart_adopt(sd, child);
   sd->items = eina_list_append_relative(sd->items, child, after);
   EINA_LIST_FOREACH(sd->items, l, item)
     {
        if (item == child) break;
        i++;
     }
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
   return i;
}

E_API int
e_flowlayout_pack_count_get(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return (int)eina_list_count(sd->items);
}

E_API Evas_Object *
e_flowlayout_pack_object_nth(Evas_Object *obj, int n)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(NULL);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   return eina_list_nth(sd->items, n);
}

E_API Evas_Object *
e_flowlayout_pack_object_first(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(NULL);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   return eina_list_data_get(sd->items);
}

E_API Evas_Object *
e_flowlayout_pack_object_last(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(NULL);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   return eina_list_last_data_get(sd->items);
}

E_API Evas_Object *
e_flowlayout_pack_object_prev(Evas_Object *obj, Evas_Object *child)
{
   E_Smart_Data *sd;
   Evas_Object *o;
   Eina_List *l;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(NULL);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   EINA_LIST_FOREACH(sd->items, l, o)
     if (o == child) return eina_list_data_get(l->prev);
   return NULL;
}

E_API Evas_Object *
e_flowlayout_pack_object_next(Evas_Object *obj, Evas_Object *child)
{
   E_Smart_Data *sd;
   Evas_Object *o;
   Eina_List *l;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(NULL);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   EINA_LIST_FOREACH(sd->items, l, o)
     if (o == child) return eina_list_data_get(l->next);
   return NULL;
}

E_API void
e_flowlayout_pack_options_set(Evas_Object *obj, int fill_w, int fill_h, int expand_w, int expand_h, double align_x, double align_y, Evas_Coord min_w, Evas_Coord min_h, Evas_Coord max_w, Evas_Coord max_h)
{
   E_Flowlayout_Item *bi;

   bi = evas_object_data_get(obj, "e_flowlayout_data");
   if (!bi) return;
   bi->fill_w = fill_w;
   bi->fill_h = fill_h;
   bi->expand_w = expand_w;
   bi->expand_h = expand_h;
   bi->align.x = align_x;
   bi->align.y = align_y;
   bi->min.w = min_w;
   bi->min.h = min_h;
   bi->max.w = max_w;
   bi->max.h = max_h;
   bi->sd->changed = 1;
   if (bi->sd->frozen <= 0) _e_flowlayout_smart_reconfigure(bi->sd);
}

E_API void
e_flowlayout_unpack(Evas_Object *obj)
{
   E_Flowlayout_Item *bi;
   E_Smart_Data *sd;

   bi = evas_object_data_get(obj, "e_flowlayout_data");
   if (!bi) return;
   sd = bi->sd;
   if (!sd) return;
   sd->items = eina_list_remove(sd->items, obj);
   _e_flowlayout_smart_disown(obj);
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
}

E_API void
e_flowlayout_size_min_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->changed) _e_flowlayout_smart_extents_calcuate(sd);
   if (minw) *minw = sd->min.w;
   if (minh) *minh = sd->min.h;
}

E_API void
e_flowlayout_size_max_get(Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->changed) _e_flowlayout_smart_extents_calcuate(sd);
   if (maxw) *maxw = sd->max.w;
   if (maxh) *maxh = sd->max.h;
}

E_API int
e_flowlayout_max_children(Evas_Object *obj)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERR(0);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return -1;
   if (!sd->homogenous) return -1;
   return sd->rows * sd->cols;
}

E_API void
e_flowlayout_align_get(Evas_Object *obj, double *ax, double *ay)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (ax) *ax = sd->align.x;
   if (ay) *ay = sd->align.y;
}

E_API void
e_flowlayout_align_set(Evas_Object *obj, double ax, double ay)
{
   E_Smart_Data *sd;

   if (evas_object_smart_smart_get(obj) != _e_smart) SMARTERRNR();
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (EINA_DBL_EQ(sd->align.x, ax) && EINA_DBL_EQ(sd->align.y, ay)) return;
   sd->align.x = ax;
   sd->align.y = ay;
   sd->changed = 1;
   if (sd->frozen <= 0) _e_flowlayout_smart_reconfigure(sd);
}

/* local subsystem functions */
static E_Flowlayout_Item *
_e_flowlayout_smart_adopt(E_Smart_Data *sd, Evas_Object *obj)
{
   E_Flowlayout_Item *bi;

   bi = calloc(1, sizeof(E_Flowlayout_Item));
   if (!bi) return NULL;
   bi->sd = sd;
   bi->obj = obj;
   /* defaults */
   bi->fill_w = 0;
   bi->fill_h = 0;
   bi->expand_w = 0;
   bi->expand_h = 0;
   bi->align.x = 0.5;
   bi->align.y = 0.5;
   bi->min.w = 0;
   bi->min.h = 0;
   bi->max.w = 0;
   bi->max.h = 0;
   evas_object_clip_set(obj, sd->clip);
   evas_object_smart_member_add(obj, bi->sd->obj);
   evas_object_data_set(obj, "e_flowlayout_data", bi);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _e_flowlayout_smart_item_del_hook, NULL);
   if ((!evas_object_visible_get(sd->clip)) &&
       (evas_object_visible_get(sd->obj)))
     evas_object_show(sd->clip);
   return bi;
}

static void
_e_flowlayout_smart_disown(Evas_Object *obj)
{
   E_Flowlayout_Item *bi;

   bi = evas_object_data_get(obj, "e_flowlayout_data");
   if (!bi) return;
   if (!bi->sd->items)
     {
        if (evas_object_visible_get(bi->sd->clip))
          evas_object_hide(bi->sd->clip);
     }
   evas_object_event_callback_del(obj,
                                  EVAS_CALLBACK_DEL,
                                  _e_flowlayout_smart_item_del_hook);
   evas_object_smart_member_del(obj);
   evas_object_clip_unset(obj);
   evas_object_data_del(obj, "e_flowlayout_data");
   free(bi);
}

static void
_e_flowlayout_smart_item_del_hook(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   e_flowlayout_unpack(obj);
}

static void
_e_flowlayout_smart_reconfigure(E_Smart_Data *sd)
{
   Evas_Coord x, y, w, h, xx, yy, cr, cc;
   Eina_List *l;
   Evas_Object *obj;
   int minw, minh;
   int count, expand;

   if (!sd->changed) return;

   /* local variables */
   x = sd->x;
   y = sd->y;
   w = sd->w;
   h = sd->h;

   /* Calculate extents */
   _e_flowlayout_smart_extents_calcuate(sd);
   minw = sd->min.w;
   minh = sd->min.h;
   count = eina_list_count(sd->items);
   expand = 0;

   /* Too small? move a little */
   if (w < minw)
     {
        x = x + ((w - minw) * (1.0 - sd->align.x));
        w = minw;
     }
   if (h < minh)
     {
        y = y + ((h - minh) * (1.0 - sd->align.y));
        h = minh;
     }

   /* Some odd expanding counter */
   EINA_LIST_FOREACH(sd->items, l, obj)
     {
        E_Flowlayout_Item *bi;

        bi = evas_object_data_get(obj, "e_flowlayout_data");
        if (bi)
          {
             if (sd->horizontal)
               {
                  if (bi->expand_w) expand++;
               }
             else
               {
                  if (bi->expand_h) expand++;
               }
          }
     }

   /* If no expansion, w is minw, h is minh */
   if (expand == 0)
     {
        if (sd->horizontal)
          {
             x += (double)(w - minw) * sd->align.x;
             w = minw;
          }
        else
          {
             y += (double)(h - minh) * sd->align.y;
             h = minh;
          }
     }

   /* Some calculations */
   if (sd->flowright)
     xx = x;
   else
     xx = x + w;
   if (sd->flowbottom)
     yy = y;
   else
     yy = y + h;
   cr = 0;
   cc = 0;

   /* Now for the real deal */
   EINA_LIST_FOREACH(sd->items, l, obj)
     {
        E_Flowlayout_Item *bi;

        bi = evas_object_data_get(obj, "e_flowlayout_data");
        if (bi)
          {
             /* Horiziontal */
             if (sd->horizontal)
               {
                  /* Homogenous */
                  if (sd->homogenous)
                    {
                       Evas_Coord ww, hh, ow, oh;
                       ww = w / sd->cols;
                       hh = h / sd->rows;
                       if (sd->fill)
                         {
                            int num = count - (cr) * (sd->cols);
                            if (num < sd->cols)
                              {
                                 ww = w / num;
                              }
                         }
                       ow = bi->min.w;
                       if (bi->fill_w) ow = ww;
                       if ((bi->max.w >= 0) && (bi->max.w < ow)) ow = bi->max.w;
                       oh = bi->min.h;
                       if (bi->fill_h) oh = hh;
                       if ((bi->max.h >= 0) && (bi->max.h < oh)) oh = bi->max.h;
                       if (sd->flowright)
                         {
                            if (sd->flowbottom)
                              {
                                 evas_object_move(obj,
                                                  xx + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            else
                              {
                                 evas_object_move(obj,
                                                  xx + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy - hh + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            evas_object_resize(obj, ow, oh);
                            xx += ww;
                         }
                       else
                         {
                            if (sd->flowbottom)
                              {
                                 evas_object_move(obj,
                                                  xx - ww + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            else
                              {
                                 evas_object_move(obj,
                                                  xx - ww + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy - hh + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            evas_object_resize(obj, ow, oh);
                            xx -= ww;
                         }
                       cc++;
                       if (cc >= sd->cols)
                         {
                            cc = 0;
                            cr++;
                            if (sd->flowright)
                              xx = x;
                            else
                              xx = x + w;
                            if (sd->flowbottom)
                              yy += hh;
                            else
                              yy -= hh;
                         }
                    }
                  /* TODO Nonhomogenous Horizontal */
                  else
                    {
                    }
               }
             /* Vertical */
             else
               {
                  if (sd->homogenous)
                    {
                       Evas_Coord ww, hh, ow, oh;
                       ww = w / sd->cols;
                       hh = h / sd->rows;
                       if (sd->fill)
                         {
                            int num = count - (cc) * (sd->rows);
                            if (num < sd->rows)
                              {
                                 hh = h / num;
                              }
                         }
                       oh = bi->min.h;
                       if (bi->fill_h) oh = hh;
                       if ((bi->max.h >= 0) && (bi->max.h < oh)) oh = bi->max.h;
                       ow = bi->min.w;
                       if (bi->fill_w) ow = ww;
                       if ((bi->max.w >= 0) && (bi->max.w < ow)) ow = bi->max.w;
                       if (sd->flowbottom)
                         {
                            if (sd->flowright)
                              {
                                 evas_object_move(obj,
                                                  xx + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            else
                              {
                                 evas_object_move(obj,
                                                  xx - ww + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            evas_object_resize(obj, ow, oh);
                            yy += hh;
                         }
                       else
                         {
                            if (sd->flowright)
                              {
                                 evas_object_move(obj,
                                                  xx + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy - hh + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            else
                              {
                                 evas_object_move(obj,
                                                  xx - ww + (Evas_Coord)(((double)(ww - ow)) * bi->align.x),
                                                  yy - hh + (Evas_Coord)(((double)(hh - oh)) * bi->align.y));
                              }
                            evas_object_resize(obj, ow, oh);
                            yy -= hh;
                         }
                       cr++;
                       if (cr >= sd->rows)
                         {
                            cr = 0;
                            cc++;
                            if (sd->flowbottom)
                              yy = y;
                            else
                              yy = y + h;
                            if (sd->flowright)
                              xx += ww;
                            else
                              xx -= ww;
                         }
                    }
                  /* TODO Nonhomogeneous Vertical */
                  else
                    {
                    }
               }
          }
     }
   sd->changed = 0;
}

static void
_e_flowlayout_smart_extents_calcuate(E_Smart_Data *sd)
{
   Eina_List *l;
   Evas_Object *obj;
   int minw, minh, count;

   /* FIXME: need to calc max */
   sd->max.w = -1; /* max < 0 == unlimited */
   sd->max.h = -1;

   minw = 1;
   minh = 1;
   if (sd->homogenous)
     {
        EINA_LIST_FOREACH(sd->items, l, obj)
          {
             E_Flowlayout_Item *bi;

             bi = evas_object_data_get(obj, "e_flowlayout_data");
             if (bi)
               {
                  if (sd->horizontal)
                    {
                       if (minh < bi->min.h) minh = bi->min.h;
                       if (minw < bi->min.w) minw = bi->min.w;
                    }
                  else
                    {
                       if (minw < bi->min.w) minw = bi->min.w;
                       if (minh < bi->min.h) minh = bi->min.h;
                    }
               }
          }
        if (sd->horizontal)
          {
             sd->rows = sd->h / minh;
             if (sd->rows < 1)
               sd->rows = 1;
             count = eina_list_count(sd->items);
             sd->cols = sd->w / minw;
             if (count < sd->cols) sd->cols = count;
             sd->rows = 0;
             if (sd->cols > 0)
               {
                  while (count > 0)
                    {
                       count -= sd->cols;
                       sd->rows++;
                    }
               }
          }
        else
          {
             sd->cols = sd->w / minw;
             if (sd->cols < 1)
               sd->cols = 1;
             count = eina_list_count(sd->items);
             sd->rows = sd->h / minh;
             if (count < sd->rows) sd->rows = count;
             sd->cols = 0;
             if (sd->rows > 0)
               {
                  while (count > 0)
                    {
                       count -= sd->rows;
                       sd->cols++;
                    }
               }
          }

        minw *= sd->cols;
        minh *= sd->rows;
     }
   /* TODO nonhomogenous */
   else
     {
        EINA_LIST_FOREACH(sd->items, l, obj)
          {
             E_Flowlayout_Item *bi;

             bi = evas_object_data_get(obj, "e_flowlayout_data");
             if (bi)
               {
                  if (sd->horizontal)
                    {
                       if (minh < bi->min.h) minh = bi->min.h * sd->rows;
                       minw += bi->min.w;
                    }
                  else
                    {
                       if (minw < bi->min.w) minw = bi->min.w * sd->cols;
                       minh += bi->min.h;
                    }
               }
          }
        if (sd->horizontal)
          {
             sd->rows = sd->h / minh;
             minh *= sd->rows;
             sd->cols = -1;
             minw = minw / sd->cols + 0.2 * minw;
          }
        else
          {
             sd->cols = sd->w / minw;
             minw *= sd->cols;
             sd->rows = -1;
             minh = minh / sd->cols + 0.2 * minh;
          }
     }
   sd->min.w = minw;
   sd->min.h = minh;
   if (sd->rows < 1)
     sd->rows = 1;
   if (sd->cols < 1)
     sd->cols = 1;
}

static void
_e_flowlayout_smart_init(void)
{
   if (_e_smart) return;
   {
      static const Evas_Smart_Class sc =
      {
         "e_flowlayout",
         EVAS_SMART_CLASS_VERSION,
         _e_flowlayout_smart_add,
         _e_flowlayout_smart_del,
         _e_flowlayout_smart_move,
         _e_flowlayout_smart_resize,
         _e_flowlayout_smart_show,
         _e_flowlayout_smart_hide,
         _e_flowlayout_smart_color_set,
         _e_flowlayout_smart_clip_set,
         _e_flowlayout_smart_clip_unset,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL
      };
      _e_smart = evas_smart_class_new(&sc);
   }
}

static void
_e_flowlayout_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd;

   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd) return;
   sd->obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->align.x = 0.5;
   sd->align.y = 0.5;
   sd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(sd->clip, obj);
   evas_object_move(sd->clip, -100004, -100004);
   evas_object_resize(sd->clip, 200008, 200008);
   evas_object_color_set(sd->clip, 255, 255, 255, 255);
   evas_object_smart_data_set(obj, sd);
}

static void
_e_flowlayout_smart_del(Evas_Object *obj)
{
   E_Smart_Data *sd;
   Evas_Object *child;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   EINA_LIST_FREE(sd->items, child)
     {
        evas_object_event_callback_del(child, EVAS_CALLBACK_DEL, _e_flowlayout_smart_item_del_hook);
        free(evas_object_data_get(child, "e_flowlayout_data"));
     }
   free(sd);
}

static void
_e_flowlayout_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((x == sd->x) && (y == sd->y)) return;
   {
      Eina_List *l;
      Evas_Object *item;
      Evas_Coord dx, dy;

      dx = x - sd->x;
      dy = y - sd->y;
      EINA_LIST_FOREACH(sd->items, l, item)
        {
           Evas_Coord ox, oy;

           evas_object_geometry_get(item, &ox, &oy, NULL, NULL);
           evas_object_move(item, ox + dx, oy + dy);
        }
   }
   sd->x = x;
   sd->y = y;
}

static void
_e_flowlayout_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((w == sd->w) && (h == sd->h)) return;
   sd->w = w;
   sd->h = h;
   sd->changed = 1;
   _e_flowlayout_smart_reconfigure(sd);
}

static void
_e_flowlayout_smart_show(Evas_Object *obj)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->items) evas_object_show(sd->clip);
}

static void
_e_flowlayout_smart_hide(Evas_Object *obj)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->clip);
}

static void
_e_flowlayout_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_color_set(sd->clip, r, g, b, a);
}

static void
_e_flowlayout_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->clip, clip);
}

static void
_e_flowlayout_smart_clip_unset(Evas_Object *obj)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->clip);
}

