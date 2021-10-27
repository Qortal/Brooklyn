#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_CHECK_H
#define E_WIDGET_CHECK_H

E_API Evas_Object *e_widget_check_add(Evas *evas, const char *label, int *val);
E_API void	  e_widget_check_checked_set(Evas_Object *check, int checked);
E_API int	  e_widget_check_checked_get(Evas_Object *check);
E_API void e_widget_check_valptr_set(Evas_Object *check, int *val);
E_API Evas_Object *e_widget_check_icon_add(Evas *evas, const char *label, const char *icon, int icon_w, int icon_h, int *val);

E_API void e_widget_check_widget_disable_on_checked_add(Evas_Object *check, Evas_Object *obj);
E_API void e_widget_check_widget_disable_on_unchecked_add(Evas_Object *check, Evas_Object *obj);
#endif
#endif
