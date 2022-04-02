#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_BUTTON_H
#define E_WIDGET_BUTTON_H

E_API Evas_Object *e_widget_button_add(Evas *evas, const char *label, const char *icon, void (*func) (void *data, void *data2), void *data, void *data2);
E_API void e_widget_button_label_set(Evas_Object *obj, const char *label);
E_API void e_widget_button_icon_set(Evas_Object *obj, Evas_Object *icon);

#endif
#endif
