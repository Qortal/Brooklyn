#ifndef _MAIN_H
#define _MAIN_H

#include <Elementary.h>

void         palimg_update(Evas_Object *img, Elm_Palette *pal);
Evas_Object *palimg_add(Evas_Object *win);

Evas_Object *palsel_add(Evas_Object *win);

void         palcols_fill(Evas_Object *win);
Evas_Object *palcols_add(Evas_Object *win);

void         colsel_update(Evas_Object *win);
Evas_Object *colsel_add(Evas_Object *win);

void         pal_load(Evas_Object *win);
void         pal_save(Evas_Object *win);

#endif
