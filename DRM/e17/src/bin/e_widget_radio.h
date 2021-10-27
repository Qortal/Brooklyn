#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_RADIO_H
#define E_WIDGET_RADIO_H

typedef struct _E_Radio_Group E_Radio_Group;

E_API E_Radio_Group *e_widget_radio_group_new(int *val);
E_API Evas_Object *e_widget_radio_add(Evas *evas, const char *label, int valnum, E_Radio_Group *group);
E_API Evas_Object *e_widget_radio_icon_add(Evas *evas, const char *label, const char *icon, int icon_w, int icon_h, int valnum, E_Radio_Group *group);
E_API void e_widget_radio_toggle_set(Evas_Object *obj, int toggle);

#endif
#endif
