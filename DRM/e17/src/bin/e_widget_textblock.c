#include "e.h"

typedef struct _E_Widget_Data E_Widget_Data;
struct _E_Widget_Data
{
   Evas_Object *o_widget, *o_entry;
};

static void _e_wid_del_hook(Evas_Object *obj);
static void _e_wid_focus_hook(Evas_Object *obj);
static void _e_wid_focus_steal(void *data, Evas *e, Evas_Object *obj, void *event_info);

/* externally accessible functions */
E_API Evas_Object *
e_widget_textblock_add(Evas *evas)
{
   Evas_Object *obj, *o;
   E_Widget_Data *wd;

   obj = e_widget_add(evas);

   e_widget_del_hook_set(obj, _e_wid_del_hook);
   e_widget_focus_hook_set(obj, _e_wid_focus_hook);
   wd = calloc(1, sizeof(E_Widget_Data));
   e_widget_data_set(obj, wd);

   wd->o_entry = o = elm_entry_add(e_win_evas_win_get(evas));
   elm_entry_scrollable_set(o, 1);
   elm_entry_editable_set(o, 0);
   evas_object_show(o);
   e_widget_sub_object_add(obj, o);
   e_widget_resize_object_set(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_wid_focus_steal, obj);

   evas_object_resize(obj, 32, 32);
   e_widget_size_min_set(obj, 32, 32);
   return obj;
}

E_API void
e_widget_textblock_markup_set(Evas_Object *obj, const char *text)
{
   E_Widget_Data *wd;

   wd = e_widget_data_get(obj);
   elm_entry_entry_set(wd->o_entry, text);
}

E_API void
e_widget_textblock_plain_set(Evas_Object *obj, const char *text)
{
   char *markup;
   char *d;
   const char *p;
   int mlen;

   if (!text)
     {
        e_widget_textblock_markup_set(obj, NULL);
        return;
     }
   mlen = strlen(text);
   /* need to look for these and replace with a new string (on the right)
    * '\n' -> "<ps/>"
    * '\t' -> "        "
    * '<'  -> "&lt;"
    * '>'  -> "&gt;"
    * '&'  -> "&amp;"
    */
   for (p = text; *p != 0; p++)
     {
        if (*p == '\n') mlen += 4 - 1;
        else if (*p == '\t')
          mlen += 8 - 1;
        else if (*p == '<')
          mlen += 4 - 1;
        else if (*p == '>')
          mlen += 4 - 1;
        else if (*p == '&')
          mlen += 5 - 1;
     }
   markup = alloca(mlen + 1);
   if (!markup) return;
   markup[0] = 0;
   for (d = markup, p = text; *p != 0; p++, d++)
     {
        if (*p == '\n')
          {
             strcpy(d, "<ps/>");
             d += 4 - 1;
          }
        else if (*p == '\t')
          {
             strcpy(d, "        ");
             d += 8 - 1;
          }
        else if (*p == '<')
          {
             strcpy(d, "&lt;");
             d += 4 - 1;
          }
        else if (*p == '>')
          {
             strcpy(d, "&gt;");
             d += 4 - 1;
          }
        else if (*p == '&')
          {
             strcpy(d, "&amp;");
             d += 5 - 1;
          }
        else *d = *p;
     }
   *d = 0;
   elm_entry_entry_set(obj, markup);
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
   elm_object_focus_set(wd->o_entry, e_widget_focus_get(obj));
}

static void
_e_wid_focus_steal(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_widget_focus_steal(data);
}

