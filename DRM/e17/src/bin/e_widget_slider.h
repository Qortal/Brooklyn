#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_SLIDER_H
#define E_WIDGET_SLIDER_H

E_API Evas_Object *e_widget_slider_add(Evas *evas, int horiz, int rev, const char *fmt, double min, double max, double step, int count, double *dval, int *ival, Evas_Coord size);
E_API int e_widget_slider_value_double_set(Evas_Object *slider, double dval);
E_API int e_widget_slider_value_int_set(Evas_Object *slider, int ival);
E_API int e_widget_slider_value_double_get(Evas_Object *slider, double *dval);
E_API int e_widget_slider_value_int_get(Evas_Object *slider, int *ival);
E_API void e_widget_slider_value_format_display_set(Evas_Object *slider, const char *format);
E_API void e_widget_slider_value_range_set(Evas_Object *slider, double min, double max);
E_API void e_widget_slider_value_step_size_set(Evas_Object *slider, double step_size);

E_API void e_widget_slider_special_value_add(Evas_Object *obj, double value, double error, const char *label);

#endif
#endif
