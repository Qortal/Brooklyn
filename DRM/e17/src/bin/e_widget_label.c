#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object *text;
};

/* local subsystem functions */
static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_disable_hook(Evas_Object *obj);

/* externally accessible functions */
/**
 * Creates a new label widget
 *
 * @param evas pointer
 * @param text to assign to the widget
 * @return the label widget
 */
E_API Evas_Object *
e_widget_label_add(Evas *evas, const char *label)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;

   obj = e_widget_add(evas);
   e_widget_del_hook_set(obj, _e_wid_del_hook);
   e_widget_disable_hook_set(obj, _e_wid_disable_hook);
   wd = calloc(1, sizeof(E_Widget_Data));
   e_widget_data_set(obj, wd);

   o = elm_label_add(e_win_evas_win_get(evas));
   wd->text = o;
   evas_object_show(o);
   e_widget_can_focus_set(obj, 0);
   e_widget_sub_object_add(obj, o);
   e_widget_resize_object_set(obj, o);
   elm_object_text_set(o, label);

   return obj;
}

/**
 * Changes the text for the label
 *
 * @param the label widget
 * @param text to assign to the widget
 */
E_API void
e_widget_label_text_set(Evas_Object *obj, const char *text)
{
   E_Widget_Data *wd = e_widget_data_get(obj);
   if (!wd) return;
   elm_object_text_set(wd->text, text);
}

static void
_e_wid_del_hook(Evas_Object *obj)
{
   E_Widget_Data *wd = e_widget_data_get(obj);
   free(wd);
   if (!wd) return;
}

static void
_e_wid_disable_hook(Evas_Object *obj)
{
   E_Widget_Data *wd = e_widget_data_get(obj);
   if (!wd) return;
   elm_object_disabled_set(wd->text, e_widget_disabled_get(obj));
}

