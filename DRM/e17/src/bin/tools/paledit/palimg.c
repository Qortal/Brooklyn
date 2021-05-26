#include "main.h"

void
palimg_update(Evas_Object *img, Elm_Palette *pal)
{
   unsigned char *pix, *p, *lin;
   int w, h, stride;
   Eina_List *l;
   Elm_Palette_Color *col;
   int x = 0, pixn = 0;

   if (!pal) return;
   evas_object_image_size_get(img, &w, &h);
   stride = evas_object_image_stride_get(img);
   pix = evas_object_image_data_get(img, EINA_TRUE);
   lin = p = pix;
   while (pixn < 256)
     {
#define PIX_WRITE(r, g, b, a) \
   *((int *)(void *)p) = (a << 24) | (r << 16) | (g << 8) | (b); \
   p += sizeof(int); \
   pixn++; \
   x++; \
   if (x == w) \
   { \
      lin += stride; \
      p = lin; \
      x = 0; \
   } \
   if (pixn >= 256) break
        if (!pal->colors)
          {
             PIX_WRITE(0, 0, 0, 0);
          }
        else
          {
             EINA_LIST_FOREACH(pal->colors, l, col)
               {
                  int r, g, b, a;

                  r = col->r;
                  g = col->g;
                  b = col->b;
                  a = col->a;
                  evas_color_argb_premul(a, &r, &g, &b);
                  PIX_WRITE(r, g, b, a);
               }
          }
     }
   evas_object_image_data_set(img, pix);
   evas_object_image_data_update_add(img, 0, 0, w, h);
}

Evas_Object *
palimg_add(Evas_Object *win)
{
   const int zoom = 2;
   Evas_Object *o = evas_object_image_filled_add(evas_object_evas_get(win));
   evas_object_image_smooth_scale_set(o, EINA_FALSE);
   evas_object_image_alpha_set(o, EINA_TRUE);
   evas_object_image_size_set(o, 16, 16);
   evas_object_size_hint_min_set(o,
                                 ELM_SCALE_SIZE(zoom * 16),
                                 ELM_SCALE_SIZE(zoom * 16));
   return o;
}
