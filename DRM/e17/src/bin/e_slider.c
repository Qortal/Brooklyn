#define _XOPEN_SOURCE  600
#include "e.h"

#define SMART_NAME     "e_slider"
#define API_ENTRY      E_Smart_Data * sd; sd = evas_object_smart_data_get(obj); if ((!obj) || (!sd) || (evas_object_type_get(obj) && strcmp(evas_object_type_get(obj), SMART_NAME)))
#define INTERNAL_ENTRY E_Smart_Data * sd; sd = evas_object_smart_data_get(obj); if (!sd) return;
typedef struct _E_Smart_Data           E_Smart_Data;

typedef struct _E_Slider_Special_Value E_Slider_Special_Value;

struct _E_Smart_Data
{
   Evas_Coord   x, y, w, h;

   Evas_Object *smart_obj;
   Evas_Object *edje_obj;
   Evas_Object *event;
   double       val, val_min, val_max, val_range, step_size;
   int          reversed, step_count, horizontal;
   int          direction;
   int          changing;
   const char  *format;
   Evas_Coord   minw, minh;
   Ecore_Timer *set_timer;
   Eina_List   *special_values;
   Eina_Bool    disabled E_BITFIELD;
};

struct _E_Slider_Special_Value
{
   double      value;
   double      error;
   const char *label;
};

/* local subsystem functions */
static Eina_Bool _e_smart_set_timer(void *data);
static void      _e_smart_value_update(E_Smart_Data *sd);
static void      _e_smart_value_update_now(E_Smart_Data *sd);
static void      _e_smart_value_fetch(E_Smart_Data *sd);
static void      _e_smart_value_limit(E_Smart_Data *sd);
static void      _e_smart_format_update(E_Smart_Data *sd);
static void      _e_smart_signal_cb_drag(void *data, Evas_Object *obj, const char *emission, const char *source);
static void      _e_smart_signal_cb_drag_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void      _e_smart_signal_cb_drag_stop(void *data, Evas_Object *obj, const char *emission, const char *source);
static void      _e_smart_event_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void      _e_smart_event_mouse_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info);
static void      _e_smart_reconfigure(E_Smart_Data *sd);
static void      _e_smart_add(Evas_Object *obj);
static void      _e_smart_del(Evas_Object *obj);
static void      _e_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void      _e_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void      _e_smart_show(Evas_Object *obj);
static void      _e_smart_hide(Evas_Object *obj);
static void      _e_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void      _e_smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void      _e_smart_clip_unset(Evas_Object *obj);
static void      _e_smart_init(void);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
E_API Evas_Object *
e_slider_add(Evas *evas)
{
   _e_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

E_API void
e_slider_orientation_set(Evas_Object *obj, int horizontal)
{
   API_ENTRY return;
   if (sd->horizontal == horizontal) return;
   sd->horizontal = horizontal;
   if (sd->horizontal)
     e_theme_edje_object_set(sd->edje_obj, "base/theme/widgets",
                             "e/widgets/slider_horizontal");
   else
     e_theme_edje_object_set(sd->edje_obj, "base/theme/widgets",
                             "e/widgets/slider_vertical");
   edje_object_size_min_calc(sd->edje_obj, &(sd->minw), &(sd->minh));
   _e_smart_value_update(sd);
}

E_API int
e_slider_orientation_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->horizontal;
}

E_API void
e_slider_value_set(Evas_Object *obj, double val)
{
   API_ENTRY return;
   sd->val = val;
   _e_smart_value_update_now(sd);
}

E_API double
e_slider_value_get(Evas_Object *obj)
{
   API_ENTRY return 0.0;
   return sd->val;
}

E_API void
e_slider_value_range_set(Evas_Object *obj, double min, double max)
{
   API_ENTRY return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val_max < sd->val_min) sd->val_min = sd->val_max;
   sd->val_range = sd->val_max - sd->val_min;
   sd->direction = 0;
   _e_smart_value_limit(sd);
   _e_smart_value_update_now(sd);
}

E_API void
e_slider_value_range_get(Evas_Object *obj, double *min, double *max)
{
   API_ENTRY return;
   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

E_API void
e_slider_value_step_size_set(Evas_Object *obj, double step_size)
{
   double step;

   API_ENTRY return;
   if (step_size < 0.0) step_size = 0.0;
   sd->step_size = step_size;
   step = 0.0;
   if (sd->val_range > 0.0)
     step = step_size / sd->val_range;
   edje_object_part_drag_step_set
     (sd->edje_obj, "e.dragable.slider", step, step);
   sd->direction = 0;
   _e_smart_value_limit(sd);
   _e_smart_value_update_now(sd);
}

E_API double
e_slider_value_step_size_get(Evas_Object *obj)
{
   API_ENTRY return 0.0;
   return sd->step_size;
}

E_API void
e_slider_value_step_count_set(Evas_Object *obj, int step_count)
{
   API_ENTRY return;
   sd->step_count = step_count;
   sd->direction = 0;
   _e_smart_value_limit(sd);
   _e_smart_value_update_now(sd);
}

E_API int
e_slider_value_step_count_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->step_count;
}

E_API void
e_slider_value_format_display_set(Evas_Object *obj, const char *format)
{
   int changed = 0;

   API_ENTRY return;
   if (((sd->format) && (!format)) || ((!sd->format) && (format))) changed = 1;
   if (sd->format) eina_stringshare_del(sd->format);
   sd->format = NULL;
   if (format) sd->format = eina_stringshare_add(format);
   if (changed)
     {
        if (sd->format)
          edje_object_signal_emit(sd->edje_obj, "e,action,show,label", "e");
        else
          edje_object_signal_emit(sd->edje_obj, "e,action,hide,label", "e");
     }
   _e_smart_format_update(sd);
   edje_object_message_signal_process(sd->edje_obj);
   edje_object_size_min_calc(sd->edje_obj, &(sd->minw), &(sd->minh));
}

E_API const char *
e_slider_value_format_display_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->format;
}

E_API void
e_slider_direction_set(Evas_Object *obj, int reversed)
{
   API_ENTRY return;
   sd->reversed = reversed;
   _e_smart_value_update_now(sd);
}

E_API void
e_slider_disabled_set(Evas_Object *obj, Eina_Bool disable)
{
   API_ENTRY return;

   if (sd->disabled == disable) return;
   sd->disabled = disable;

   if (disable)
     edje_object_signal_emit(sd->edje_obj, "e,state,disabled", "e");
   else
     edje_object_signal_emit(sd->edje_obj, "e,state,enabled", "e");
}

E_API int
e_slider_direction_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->reversed;
}

E_API void
e_slider_size_min_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   API_ENTRY return;
   if (minw) *minw = sd->minw;
   if (minh) *minh = sd->minh;
}

E_API Evas_Object *
e_slider_edje_object_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->edje_obj;
}

E_API void
e_slider_special_value_add(Evas_Object *obj, double value, double error, const char *label)
{
   E_Slider_Special_Value *sv;
   API_ENTRY return;

   sv = malloc(sizeof(E_Slider_Special_Value));
   if (!sv) return;
   sv->value = value;
   sv->error = error;
   sv->label = eina_stringshare_add(label);

   sd->special_values = eina_list_append(sd->special_values, sv);
   _e_smart_format_update(sd);
}

/* local subsystem functions */
static Eina_Bool
_e_smart_set_timer(void *data)
{
   E_Smart_Data *sd = data;
   double pos = 0.0;

   if (sd->val_range > 0.0) pos = (sd->val - sd->val_min) / sd->val_range;
   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0)
     pos = 1.0;
   if (sd->reversed) pos = 1.0 - pos;
   sd->changing++;
   if (sd->set_timer) ecore_timer_del(sd->set_timer);
   sd->set_timer = NULL;
   edje_object_part_drag_value_set(sd->edje_obj, "e.dragable.slider", pos, pos);
   edje_object_message_signal_process(sd->edje_obj);
   sd->changing--;
   return ECORE_CALLBACK_CANCEL;
}

static void
_e_smart_value_update(E_Smart_Data *sd)
{
   if (sd->set_timer) ecore_timer_del(sd->set_timer);
   sd->set_timer = ecore_timer_loop_add(0.05, _e_smart_set_timer, sd);
}

static void
_e_smart_value_update_now(E_Smart_Data *sd)
{
   _e_smart_set_timer(sd);
}

static void
_e_smart_value_fetch(E_Smart_Data *sd)
{
   double posx = 0.0, posy = 0.0, pos = 0.0;

   edje_object_part_drag_value_get
     (sd->edje_obj, "e.dragable.slider", &posx, &posy);
   if (sd->horizontal) pos = posx;
   else pos = posy;
   sd->val = (pos * sd->val_range) + sd->val_min;
}

static void
_e_smart_value_limit(E_Smart_Data *sd)
{
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;
   if (sd->val_range > 0.0)
     {
        if (sd->step_count > 0)
          {
             double p, s;

             s = sd->val_range / sd->step_count;
             p = sd->val / s;
             if (sd->direction == 1)
               p = ceil(p);
             else if (sd->direction == -1)
               p = floor(p);
             else
               p = round(p);
             sd->val = p * s;
          }
        else if (sd->step_size > 0.0)
          {
             double p;

             p = sd->val / sd->step_size;
             if (sd->direction == 1)
               p = ceil(p);
             else if (sd->direction == -1)
               p = floor(p);
             else
               p = round(p);
             sd->val = p * sd->step_size;
          }
     }
   sd->direction = 0;
}

static void
_e_smart_format_update(E_Smart_Data *sd)
{
   const E_Slider_Special_Value *sv;
   const Eina_List *l;

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     if (fabs(sd->val - sv->value) <= sv->error)
       {
          edje_object_part_text_set(sd->edje_obj, "e.text.label", sv->label);
          return;
       }

   if (sd->format)
     {
        char buf[256];
DISABLE_WARNING(format-nonliteral, format-nonliteral, format-nonliteral)
        snprintf(buf, sizeof(buf), sd->format, sd->val);
ENABLE_WARNING(format-nonliteral, format-nonliteral, format-nonliteral)
        edje_object_part_text_set(sd->edje_obj, "e.text.label", buf);
     }
}

static void
_e_smart_signal_cb_drag(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Smart_Data *sd = data;
   double pval = sd->val;

   _e_smart_value_fetch(sd);
   _e_smart_value_limit(sd);
   _e_smart_format_update(sd);
   if (sd->changing) return;
   if (!EINA_DBL_EQ(sd->val, pval))
     evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

static void
_e_smart_signal_cb_drag_start(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Smart_Data *sd = data;
   double pval = sd->val;

   _e_smart_value_fetch(sd);
   _e_smart_value_limit(sd);
   _e_smart_format_update(sd);
   if (sd->changing) return;
   if (!EINA_DBL_EQ(sd->val, pval))
     evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

static void
_e_smart_signal_cb_drag_stop(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Smart_Data *sd = data;
   double pval = sd->val;

   _e_smart_value_fetch(sd);
   _e_smart_value_limit(sd);
   _e_smart_format_update(sd);
   _e_smart_value_update(sd);
   if (sd->changing) return;
   if (!EINA_DBL_EQ(sd->val, pval))
     evas_object_smart_callback_call(sd->smart_obj, "changed", NULL);
}

static void
_e_smart_signal_cb_wheel_up(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Smart_Data *sd = data;

   if (sd->disabled) return;
   edje_object_part_drag_step(sd->edje_obj, "e.dragable.slider", -0.05, -0.05);
   sd->direction = -1;
}

static void
_e_smart_signal_cb_wheel_down(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Smart_Data *sd = data;

   if (sd->disabled) return;
   edje_object_part_drag_step(sd->edje_obj, "e.dragable.slider", 0.05, 0.05);
   sd->direction = 1;
}

static void
_e_smart_event_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   E_Smart_Data *sd = data;

   if (sd->disabled) return;
   if ((!strcmp(ev->key, "Down")) ||
       (!strcmp(ev->key, "KP_Down")) ||
       (!strcmp(ev->key, "Left")) ||
       (!strcmp(ev->key, "KP_Left")))
     {
        if (sd->step_count > 0)
          {
             double s = sd->val_range / sd->step_count;
             edje_object_part_drag_step(sd->edje_obj, "e.dragable.slider", -s, -s);
          }
        else
          edje_object_part_drag_step(sd->edje_obj, "e.dragable.slider", -1, -1);
        sd->direction = -1;
     }
   else if ((!strcmp(ev->key, "Up")) ||
            (!strcmp(ev->key, "KP_Up")) ||
            (!strcmp(ev->key, "Right")) ||
            (!strcmp(ev->key, "KP_Right")))
     {
        if (sd->step_count > 0)
          {
             double s = sd->val_range / sd->step_count;
             edje_object_part_drag_step(sd->edje_obj, "e.dragable.slider", s, s);
          }
        else
          edje_object_part_drag_step(sd->edje_obj, "e.dragable.slider", 1, 1);
        sd->direction = 1;
     }
   else if ((!strcmp(ev->key, "Home")) ||
            (!strcmp(ev->key, "KP_Home")))
     {
        edje_object_part_drag_value_set(sd->edje_obj, "e.dragable.slider", 0., 0.);
        sd->direction = 0;
     }
   else if ((!strcmp(ev->key, "End")) ||
            (!strcmp(ev->key, "KP_End")))
     {
        edje_object_part_drag_value_set(sd->edje_obj, "e.dragable.slider", 1., 1.);
        sd->direction = 0;
     }
}

static void
_e_smart_event_mouse_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   E_Smart_Data *sd = data;
   double pos;
   static int in_md = 0;

   if (sd->disabled) return;
   if (in_md > 0) return;
   in_md++;
   evas_object_geometry_get(sd->event, &x, &y, &w, &h);
   if (sd->horizontal)
     {
        pos = ((double)ev->output.x - (double)x) / (double)w;
        if (pos > 1.0) pos = 1.0;
        if (pos < 0.0) pos = 0.0;
        if (sd->reversed)
          pos = 1.0 - pos;
     }
   else
     {
        pos = (((double)ev->output.y - (double)y) / (double)h);
        if (pos > 1.0) pos = 1.0;
        if (pos < 0.0) pos = 0.0;
        if (!sd->reversed)
          pos = 1.0 - pos;
     }

   edje_object_part_drag_value_set(sd->edje_obj, "e.dragable.slider", pos, pos);
   edje_object_message_signal_process(sd->edje_obj); /* really needed or go in infinite loop */
   evas_event_feed_mouse_cancel(e, 0, NULL);
   evas_event_feed_mouse_down(e, 1, EVAS_BUTTON_NONE, ev->timestamp, NULL);
   in_md--;
}

static void
_e_smart_reconfigure(E_Smart_Data *sd)
{
   evas_object_move(sd->edje_obj, sd->x, sd->y);
   evas_object_resize(sd->edje_obj, sd->w, sd->h);
}

static void
_e_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd;

   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd) return;
   evas_object_smart_data_set(obj, sd);

   sd->smart_obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;

   sd->val = 0.0;
   sd->val_min = 0.0;
   sd->val_max = 1.0;
   sd->val_range = 1.0;
   sd->step_size = 0.01;
   sd->reversed = 0;
   sd->step_count = 0;
   sd->horizontal = 0;
   sd->format = NULL;
   sd->direction = 0;

   sd->edje_obj = edje_object_add(evas_object_evas_get(obj));
   e_theme_edje_object_set(sd->edje_obj, "base/theme/widgets",
                           "e/widgets/slider_vertical");
   edje_object_size_min_calc(sd->edje_obj, &(sd->minw), &(sd->minh));
   evas_object_smart_member_add(sd->edje_obj, obj);

   sd->event = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(sd->event, 0, 0, 0, 0);
   evas_object_pass_events_set(sd->event, EINA_TRUE);
   edje_object_part_swallow(sd->edje_obj, "e.swallow.bar", sd->event);
   evas_object_smart_member_add(sd->event, sd->edje_obj);

   edje_object_signal_callback_add(sd->edje_obj, "drag", "*", _e_smart_signal_cb_drag, sd);
   edje_object_signal_callback_add(sd->edje_obj, "drag,start", "*", _e_smart_signal_cb_drag_start, sd);
   edje_object_signal_callback_add(sd->edje_obj, "drag,stop", "*", _e_smart_signal_cb_drag_stop, sd);
   edje_object_signal_callback_add(sd->edje_obj, "drag,step", "*", _e_smart_signal_cb_drag_stop, sd);
   edje_object_signal_callback_add(sd->edje_obj, "drag,set", "*", _e_smart_signal_cb_drag_stop, sd);
   edje_object_signal_callback_add(sd->edje_obj, "mouse,wheel,0,1", "*", _e_smart_signal_cb_wheel_up, sd);
   edje_object_signal_callback_add(sd->edje_obj, "mouse,wheel,0,-1", "*", _e_smart_signal_cb_wheel_down, sd);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _e_smart_event_key_down, sd);
   evas_object_event_callback_add(sd->event, EVAS_CALLBACK_MOUSE_DOWN, _e_smart_event_mouse_down, sd);
}

static void
_e_smart_del(Evas_Object *obj)
{
   E_Slider_Special_Value *sv;

   INTERNAL_ENTRY;
   evas_object_del(sd->event);
   evas_object_del(sd->edje_obj);
   if (sd->format) eina_stringshare_del(sd->format);
   if (sd->set_timer) ecore_timer_del(sd->set_timer);
   EINA_LIST_FREE(sd->special_values, sv)
     {
        eina_stringshare_del(sv->label);
        free(sv);
     }
   free(sd);
}

static void
_e_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   if ((sd->x == x) && (sd->y == y)) return;
   sd->x = x;
   sd->y = y;
   _e_smart_reconfigure(sd);
}

static void
_e_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;
   _e_smart_reconfigure(sd);
}

static void
_e_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_show(sd->edje_obj);
}

static void
_e_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_hide(sd->edje_obj);
}

static void
_e_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   evas_object_color_set(sd->edje_obj, r, g, b, a);
}

static void
_e_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   INTERNAL_ENTRY;
   evas_object_clip_set(sd->edje_obj, clip);
}

static void
_e_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_clip_unset(sd->edje_obj);
}

/* never need to touch this */

static void
_e_smart_init(void)
{
   if (_e_smart) return;
   {
      static Evas_Smart_Class sc = EVAS_SMART_CLASS_INIT_NAME_VERSION(SMART_NAME);
      if (!sc.add)
        {
           sc.add = _e_smart_add;
           sc.del = _e_smart_del;
           sc.move = _e_smart_move;
           sc.resize = _e_smart_resize;
           sc.show = _e_smart_show;
           sc.hide = _e_smart_hide;
           sc.color_set = _e_smart_color_set;
           sc.clip_set = _e_smart_clip_set;
           sc.clip_unset = _e_smart_clip_unset;
        }
      _e_smart = evas_smart_class_new(&sc);
   }
}

