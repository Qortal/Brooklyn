#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_TEXTBLOCK_H
#define E_WIDGET_TEXTBLOCK_H

E_API Evas_Object *e_widget_textblock_add(Evas *evas);
E_API void e_widget_textblock_markup_set(Evas_Object *obj, const char *text);
E_API void e_widget_textblock_plain_set(Evas_Object *obj, const char *text);

#endif
#endif
