#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object *o_entry, *o_inout;
   char       **text_location;
   void         (*func)(void *data, void *data2);
   char        *text;
   void        *data;
   void        *data2;
};

/* local subsystem functions */
static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_focus_hook(Evas_Object *obj);
static void _e_wid_disable_hook(Evas_Object *obj);
static void _e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_wid_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _e_wid_keydown(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_wid_movresz(void *data, Evas *e, Evas_Object *obj, void *event_info);

/* externally accessible functions */

/**
 * Creates a new entry widget
 *
 * @param evas the evas where to add the new entry widget
 * @param text_location the location where to store the text of the entry.
 * @param func DOCUMENT ME!
 * @param data  DOCUMENT ME!
 * @param data2 DOCUMENT ME!
 * The current value will be used to initialize the entry
 * @return Returns the new entry widget
 */
E_API Evas_Object *
e_widget_entry_add(Evas_Object *parent, char **text_location, void (*func)(void *data, void *data2), void *data, void *data2)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;
   Evas_Coord minw, minh;
   Evas *evas;

   evas = evas_object_evas_get(parent);
   obj = e_widget_add(evas);

   e_widget_del_hook_set(obj, _e_wid_del_hook);
   e_widget_focus_hook_set(obj, _e_wid_focus_hook);
   e_widget_disable_hook_set(obj, _e_wid_disable_hook);

   wd = calloc(1, sizeof(E_Widget_Data));
   e_widget_data_set(obj, wd);
   wd->text_location = text_location;

   o = elm_entry_add(parent);
   elm_object_text_set(o, "test");
   elm_entry_scrollable_set(o, 1);
   elm_entry_single_line_set(o, 1);
   elm_scroller_policy_set(o, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
   wd->o_entry = o;
   evas_object_size_hint_min_get(wd->o_entry, &minw, &minh);
   e_widget_size_min_set(obj, minw, minh);
   elm_object_text_set(o, NULL);

   e_widget_sub_object_add(obj, o);
   e_widget_resize_object_set(obj, o);
   evas_object_show(o);

   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, _e_wid_keydown, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOVE, _e_wid_movresz, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE, _e_wid_movresz, obj);

   o = evas_object_rectangle_add(evas);
   wd->o_inout = o;
   evas_object_repeat_events_set(o, EINA_TRUE);
   evas_object_color_set(o, 0, 0, 0, 0);
   e_widget_sub_object_add(obj, o);
   evas_object_smart_member_add(o, obj);
   evas_object_show(o);

   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_wid_focus_steal, obj);

   o = wd->o_entry;
   if ((text_location) && (*text_location))
     {
        char *tmp = elm_entry_utf8_to_markup(*text_location);

        if (tmp)
          {
             elm_object_text_set(o, tmp);
             free(tmp);
          }
        else elm_object_text_set(wd->o_entry, NULL);
     }

   wd->func = func;
   wd->data = data;
   wd->data2 = data2;
   evas_object_smart_callback_add(o, "changed", _e_wid_changed_cb, obj);

   return obj;
}

/**
 * Sets the text of the entry widget
 *
 * @param entry an entry widget
 * @param text the text to set
 */
E_API void
e_widget_entry_text_set(Evas_Object *entry, const char *text)
{
   E_Widget_Data *wd;
   char *tmp;

   if (!(entry) || (!(wd = e_widget_data_get(entry))))
     return;
   tmp = elm_entry_utf8_to_markup(text);
   if (tmp)
     {
        elm_object_text_set(wd->o_entry, tmp);
        free(tmp);
     }
   else elm_object_text_set(wd->o_entry, NULL);
}

/**
 * Gets the text of the entry widget
 *
 * @param entry an entry widget
 * @return Returns the text of the entry widget
 */
E_API const char *
e_widget_entry_text_get(Evas_Object *entry)
{
   E_Widget_Data *wd;
   char *tmp;

   if (!(entry) || (!(wd = e_widget_data_get(entry))))
     return NULL;
   tmp = elm_entry_markup_to_utf8(elm_object_text_get(wd->o_entry));
   if (tmp)
     {
        if (wd->text) free(wd->text);
        wd->text = tmp;
        return wd->text;
     }
   if (wd->text) free(wd->text);
   wd->text = NULL;
   return NULL;
}

/**
 * Clears the entry widget
 *
 * @param entry an entry widget
 */
E_API void
e_widget_entry_clear(Evas_Object *entry)
{
   E_Widget_Data *wd;

   if (!(entry) || (!(wd = e_widget_data_get(entry))))
     return;
   elm_object_text_set(wd->o_entry, NULL);
}

/**
 * Sets whether or not the entry widget is in password mode. In password mode,
 * the entry displays '*' instead of the characters
 *
 * @param entry an entry widget
 * @param password_mode 1 to turn on password mode, 0 to turn it off
 */
E_API void
e_widget_entry_password_set(Evas_Object *entry, int password_mode)
{
   E_Widget_Data *wd;

   if (!(entry) || (!(wd = e_widget_data_get(entry))))
     return;
   elm_entry_password_set(wd->o_entry, !!password_mode);
}

/**
 * Sets whether or not the entry widget is user-editable. This still
 * allows copying and selecting, just no inserting or deleting of text.
 *
 * @param entry an entry widget
 * @param readonly_mode 1 to enable read-only mode, 0 to turn it off
 */
E_API void
e_widget_entry_readonly_set(Evas_Object *entry, int readonly_mode)
{
   E_Widget_Data *wd;

   if (!(entry) || (!(wd = e_widget_data_get(entry))))
     return;

   elm_entry_editable_set(wd->o_entry, !readonly_mode);
}

/**
 * Selects the content of the entry.
 *
 * @param entry an entry widget
 */
E_API void
e_widget_entry_select_all(Evas_Object *entry)
{
   E_Widget_Data *wd;

   if (!(entry) || (!(wd = e_widget_data_get(entry))))
     return;
   elm_entry_select_all(wd->o_entry);
}

E_API void
e_widget_entry_select_set(Evas_Object *entry, int start, int end)
{
   E_Widget_Data *wd;

   if (!(entry) || (!(wd = e_widget_data_get(entry))))
     return;
   elm_entry_select_region_set(wd->o_entry, start, end);
}

/* Private functions */

static void
_e_wid_del_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   if (!(obj) || (!(wd = e_widget_data_get(obj))))
     return;
   if (wd->text) free(wd->text);
   evas_object_del(wd->o_entry);
   evas_object_del(wd->o_inout);
   wd->o_entry = NULL;
   wd->o_inout = NULL;
   wd->text = NULL;
   free(wd);
}

static void
_e_wid_focus_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   if (!(obj) || (!(wd = e_widget_data_get(obj))))
     return;

   elm_object_focus_set(wd->o_entry, !!e_widget_focus_get(obj));
}

static void
_e_wid_disable_hook(Evas_Object *obj)
{
   E_Widget_Data *wd;

   if (!(obj) || (!(wd = e_widget_data_get(obj))))
     return;

   elm_object_disabled_set(wd->o_entry, !!e_widget_disabled_get(obj));
}

static void
_e_wid_focus_steal(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_widget_focus_steal(data);
}

static void
_e_wid_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *entry;
   E_Widget_Data *wd;
   char *tmp;

   if (!(entry = data) || (!(wd = e_widget_data_get(entry))))
     return;

   tmp = elm_entry_markup_to_utf8(elm_object_text_get(wd->o_entry));
   if (tmp)
     {
        if (wd->text) free(wd->text);
        wd->text = tmp;
     }
   else
     {
        if (wd->text) free(wd->text);
        wd->text = NULL;
     }
   if (wd->text_location)
     {
        free(*wd->text_location);
        *wd->text_location = wd->text ? strdup(wd->text) : NULL;
     }
   e_widget_change(data);

   if (wd->func) wd->func(wd->data, wd->data2);
}

static void
_e_wid_keydown(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   evas_object_smart_callback_call(data, "key_down", event_info);
}

static void
_e_wid_movresz(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Widget_Data *wd;
   Evas_Coord x, y, w, h;

   if (!(data) || (!(wd = e_widget_data_get(data))))
     return;
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   if (wd->o_inout)
     {
        evas_object_move(wd->o_inout, x, y);
        evas_object_resize(wd->o_inout, w, h);
     }
}

