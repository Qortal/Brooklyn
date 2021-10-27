#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object         *o_button;
   Evas_Object         *o_icon;
   void                 (*func)(void *data, void *data2);
   void                *data;
   void                *data2;
};

static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_focus_hook(Evas_Object *obj);
static void _e_wid_activate_hook(Evas_Object *obj);
static void _e_wid_disable_hook(Evas_Object *obj);
static void _click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void _e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info);
/* local subsystem functions */

/* externally accessible functions */
E_API Evas_Object *
e_widget_button_add(Evas *evas, const char *label, const char *icon, void (*func)(void *data, void *data2), void *data, void *data2)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;

   obj = e_widget_add(evas);

   e_widget_del_hook_set(obj, _e_wid_del_hook);
   e_widget_focus_hook_set(obj, _e_wid_focus_hook);
   e_widget_activate_hook_set(obj, _e_wid_activate_hook);
   e_widget_disable_hook_set(obj, _e_wid_disable_hook);
   wd = calloc(1, sizeof(E_Widget_Data));
   wd->func = func;
   wd->data = data;
   wd->data2 = data2;
   e_widget_data_set(obj, wd);

   o = elm_button_add(e_win_evas_win_get(evas));
   evas_object_smart_callback_add(o, "clicked", _click, obj);
   wd->o_button = o;
   if ((label) && (label[0] != 0))
     {
        elm_object_text_set(o, label);
     }

   e_widget_sub_object_add(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
                                  _e_wid_focus_steal, obj);
   e_widget_resize_object_set(obj, o);
   evas_object_show(o);

   if (icon)
     {
        o = e_icon_add(evas);
        wd->o_icon = o;
        e_util_icon_theme_set(o, icon);
        elm_object_content_set(wd->o_button, o);
        e_widget_sub_object_add(obj, o);
        evas_object_show(o);
     }

   return obj;
}

E_API void
e_widget_button_label_set(Evas_Object *obj, const char *label)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   elm_object_text_set(wd->o_button, label);
}

E_API void
e_widget_button_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   if (wd->o_icon)
     {
        e_widget_sub_object_del(obj, wd->o_icon);
        evas_object_hide(wd->o_icon);
        elm_object_content_unset(wd->o_button);
        evas_object_del(wd->o_icon);
        wd->o_icon = NULL;
     }
   if (icon)
     {
        wd->o_icon = icon;
        elm_object_content_set(wd->o_button, icon);
        evas_object_show(icon);
        e_widget_sub_object_add(obj, icon);
     }
}

static void
_e_wid_del_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   free(wd);
}

static void
_e_wid_focus_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   elm_object_focus_set(wd->o_button, e_widget_focus_get(obj));
}

static void
_e_wid_activate_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   if (wd->func) wd->func(wd->data, wd->data2);
}

static void
_e_wid_disable_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   elm_object_disabled_set(wd->o_button, e_widget_disabled_get(obj));
}

static void
_click(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *wid;

   wid = data;
   if ((!wid) || (e_widget_disabled_get(wid))) return;
   e_widget_focus_steal(wid);
   e_widget_change(wid);
   _e_wid_activate_hook(wid);
}

static void
_e_wid_focus_steal(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_widget_focus_steal(data);
}
