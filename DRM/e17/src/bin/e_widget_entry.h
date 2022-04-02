#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_ENTRY_H
#define E_WIDGET_ENTRY_H

E_API Evas_Object *e_widget_entry_add                 (Evas_Object *parent, char **text_location, void (*func) (void *data, void *data2), void *data, void *data2);
E_API void         e_widget_entry_text_set            (Evas_Object *entry, const char *text);
E_API const char  *e_widget_entry_text_get            (Evas_Object *entry);
E_API void         e_widget_entry_clear               (Evas_Object *entry);
E_API void         e_widget_entry_password_set        (Evas_Object *entry, int password_mode);
E_API void         e_widget_entry_readonly_set        (Evas_Object *entry, int readonly_mode);
E_API void         e_widget_entry_select_all          (Evas_Object *entry);
E_API void         e_widget_entry_select_set          (Evas_Object *entry, int start, int end);
#endif
#endif
