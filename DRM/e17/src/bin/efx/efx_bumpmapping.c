#include "e_efx_private.h"
#include <math.h>

#define DATA8 unsigned char
#define A_VAL(p) (((DATA8 *)(p))[3])
#define R_VAL(p) (((DATA8 *)(p))[2])
#define G_VAL(p) (((DATA8 *)(p))[1])
#define B_VAL(p) (((DATA8 *)(p))[0])

typedef struct E_Efx_Bumpmap_Data
{
   E_EFX *e;
   int x;
   int y;
   int z;
   int depth;
   int red;
   int green;
   int blue;
   int ambient;
   unsigned int *img_data;
} E_Efx_Bumpmap_Data;

static void
_bumpmap(E_Efx_Bumpmap_Data *ebd)
{
   Evas_Object  *o;
   int w;
   int h;
   int i, j;

   int x;
   int y;
   int z;
   int depth;
   int red;
   int green;
   int blue;
   int ambient;

   int z_2, lightx, lighty;
   int mx;
   int my;
   unsigned int *d1;
   unsigned int *src;
   unsigned int *mp;
   unsigned int *mpy;
   unsigned int *mpp;

   x = ebd->x;
   y = ebd->y;
   z = ebd->z;

   red = ebd->red / 0x100;
   green = ebd->green / 0x100;
   blue = ebd->blue / 0x100;
   ambient = ebd->ambient / 0x100;
   depth = ebd->depth / 0x100;
   depth /= (255 * (255 + 255 + 255));
   z_2 = z * z;

   o = ebd->e->obj;
   evas_object_image_size_get(o, &w, &h);
   if ((!w) || (!h)) return;

   d1 = malloc(w * h * sizeof(int));
   memcpy(d1, ebd->img_data, w * h * sizeof(int));
   src = d1;

   mpp = ebd->img_data;

   my = h;
   lighty = -y;
   for (j = h; --j >= 0;)
     {
       mp = mpp;
       mpp += w;
       if (--my <= 0)
         {
           mpp -= w * h;
           my = h;
         }
       mpy = mpp;
       mx = w;
       lightx = -x;
       i = w - 1;
       do
         {
            double v;
            int r, g, b, gr, x1, y_1;

            gr = A_VAL(mp) * (R_VAL(mp) + G_VAL(mp) + B_VAL(mp));
            y_1 = depth * (A_VAL(mpy) * (R_VAL(mpy) +
                                                G_VAL(mpy) +
                                                B_VAL(mpy)) - gr);
            mp++;
            mpy++;
            if (--mx <= 0)
              {
                mp -= w;
                mpy -= w;
                mx = w;
              }
            x1 = depth * (A_VAL(mp) * (R_VAL(mp) +
                                           G_VAL(mp) + B_VAL(mp)) - gr);
            v = x1 * lightx + y_1 * lighty + z;
            v /= sqrt(((x1 * x1) + (y_1 * y_1) + 1) * ((lightx * lightx) + (lighty * lighty) + z_2));
            v += ambient;
            r = v * R_VAL(src) * red;
            g = v * G_VAL(src) * green;
            b = v * B_VAL(src) * blue;
            if (r < 0)
              r = 0;
            else if (r > 255)
              r = 255;
            if (g < 0)
              g = 0;
            else if (g > 255)
              g = 255;
            if (b < 0)
              b = 0;
            else if (b > 255)
              b = 255;
            R_VAL(src) = r;
            G_VAL(src) = g;
            B_VAL(src) = b;

            lightx++;
            src++;
         } while (--i >= 0);
       lighty++;
     }

   evas_object_image_data_set(o, d1);
   evas_object_image_data_update_add(o, 0, 0, w, h);
}

EAPI Eina_Bool
e_efx_bumpmap(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   E_EFX *e;
   E_Efx_Bumpmap_Data *ebd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   e = evas_object_data_get(obj, "e_efx-data");
   if (!e)
     e = e_efx_new(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, EINA_FALSE);

   if (!e->bumpmap_data)
     e->bumpmap_data = calloc(1, sizeof(E_Efx_Bumpmap_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(e->bumpmap_data, EINA_FALSE);
   ebd = e->bumpmap_data;
   ebd->e = e;
   ebd->x = x;
   ebd->y = y;
   ebd->z = 30;
   ebd->depth = 0x200;
   ebd->red = 0x200;
   ebd->green = 0x200;
   ebd->blue = 0x200;
   ebd->ambient = 0;
   if (!ebd->img_data)
     {
       unsigned int *m;
       int w;
       int h;

       evas_object_image_size_get(obj, &w, &h);
       m = (unsigned int *)evas_object_image_data_get(obj, 1);
       ebd->img_data = (unsigned int *)malloc(w * h * sizeof(unsigned int));
       if (!ebd->img_data)
         {
           free(ebd);
           return EINA_FALSE;
         }
       printf("memcpy\n");
       memcpy(ebd->img_data, m, (w * h * sizeof(unsigned int)));
     }

   _bumpmap(ebd);

   return EINA_TRUE;
   (void)e_efx_speed_str;
}
