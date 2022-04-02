#include "e.h"

E_API int E_EVENT_CLIENT_VOLUME = -1;
E_API int E_EVENT_CLIENT_MUTE = -1;
E_API int E_EVENT_CLIENT_UNMUTE = -1;
E_API int E_EVENT_CLIENT_VOLUME_SINK_ADD = -1;
E_API int E_EVENT_CLIENT_VOLUME_SINK_DEL = -1;
E_API int E_EVENT_CLIENT_VOLUME_SINK_CHANGED = -1;

static void _e_client_volume_event_simple_free(void *d, E_Event_Client *ev);
static void _e_client_volume_event_simple(E_Client *ec, int type);
static void _e_client_volume_object_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_client_volume_object_volume_changed(void *data, Evas_Object *obj, void *event_info);
static void _e_client_volume_object_volume_drag_stop(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _e_client_volume_object_changed(void *data, int type, void *event);
static void _e_client_volume_object_del_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);

static void
_e_client_volume_event_simple_free(void *d EINA_UNUSED, E_Event_Client *ev)
{
   UNREFD(ev->ec, 3);
   e_object_unref(E_OBJECT(ev->ec));
   free(ev);
}

static void
_e_client_volume_event_simple(E_Client *ec, int type)
{
   E_Event_Client *ev;

   ev = E_NEW(E_Event_Client, 1);
   ev->ec = ec;
   REFD(ec, 3);
   e_object_ref(E_OBJECT(ec));
   ecore_event_add(type, ev,
                   (Ecore_End_Cb)_e_client_volume_event_simple_free, NULL);
}

static void
_e_client_volume_sink_event_simple_free(void *d EINA_UNUSED, E_Event_Client_Volume_Sink *ev)
{
   UNREFD(ev->ec, 3);
   e_object_unref(E_OBJECT(ev->ec));
   free(ev);
}

static void
_e_client_volume_sink_event_simple(E_Client *ec, E_Client_Volume_Sink *sink, int type)
{
   E_Event_Client_Volume_Sink *ev;

   ev = E_NEW(E_Event_Client_Volume_Sink, 1);
   ev->ec = ec;
   ev->sink = sink;
   REFD(ec, 3);
   e_object_ref(E_OBJECT(ec));
   ecore_event_add(type, ev,
                   (Ecore_End_Cb)_e_client_volume_sink_event_simple_free, NULL);
}

static void
_e_client_volume_object_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Client *ec;

   ec = data;

   e_client_volume_mute_set(ec, !(ec->mute));
}

static void
_e_client_volume_object_volume_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Client *ec;

   ec = data;

   e_client_volume_set(ec, elm_slider_value_get(obj));
}

static void
_e_client_volume_object_volume_drag_stop(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Client *ec;

   ec = data;

   e_client_volume_set(ec, elm_slider_value_get(obj));
}

static Eina_Bool
_e_client_volume_object_changed(void *data, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev;
   Evas_Object *o;
   Evas_Object *bx;
   Edje_Message_Int_Set *msg;

   ev = event;
   bx = data;

   o = edje_object_part_swallow_get(bx, "e.swallow.volume_icon");
   if (o)
     {
        msg = alloca(sizeof(Edje_Message_Int_Set) + (2 * sizeof(int)));
        msg->count = 3;
        msg->val[0] = ev->ec->mute;
        msg->val[1] = ev->ec->volume;
        msg->val[2] = msg->val[1];
        edje_object_message_send(o, EDJE_MESSAGE_INT_SET, 0, msg);
        edje_object_signal_emit(o,  "e,action,volume,change", "e");
     }
   o = edje_object_part_swallow_get(bx, "e.swallow.volume");
   if (o)
     {
        elm_slider_value_set(o, ev->ec->volume);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_e_client_volume_object_del_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eina_List *handlers;
   Evas_Object *o;

   handlers = data;

   o = edje_object_part_swallow_get(obj, "e.swallow.volume_icon");
   if (o) evas_object_del(o);
   o = edje_object_part_swallow_get(obj, "e.swallow.volume");
   if (o) evas_object_del(o);

   E_FREE_LIST(handlers, ecore_event_handler_del);
}



EINTERN int
e_client_volume_init(void)
{
   E_EVENT_CLIENT_VOLUME = ecore_event_type_new();
   E_EVENT_CLIENT_MUTE = ecore_event_type_new();
   E_EVENT_CLIENT_UNMUTE = ecore_event_type_new();
   E_EVENT_CLIENT_VOLUME_SINK_ADD = ecore_event_type_new();
   E_EVENT_CLIENT_VOLUME_SINK_DEL = ecore_event_type_new();
   E_EVENT_CLIENT_VOLUME_SINK_CHANGED = ecore_event_type_new();

   return EINA_TRUE;
}

EINTERN void
e_client_volume_shutdown(void)
{
}

E_API E_Client_Volume_Sink *
e_client_volume_sink_new(E_Client_Volume_Sink_Get func_get, E_Client_Volume_Sink_Set func_set, E_Client_Volume_Sink_Min_Get func_min_get, E_Client_Volume_Sink_Max_Get func_max_get, E_Client_Volume_Sink_Name_Get func_name_get, void *data)
{
   E_Client_Volume_Sink *sink;

   sink = E_NEW(E_Client_Volume_Sink, 1);
   sink->func_set = func_set;
   sink->func_get = func_get;
   sink->func_min_get = func_min_get;
   sink->func_max_get = func_max_get;
   sink->func_name_get = func_name_get;
   sink->data = data;

   return sink;
}

E_API void
e_client_volume_sink_del(E_Client_Volume_Sink *sink)
{
   E_Client *ec;

   EINA_LIST_FREE(sink->clients, ec)
     {
        ec->sinks = eina_list_remove(ec->sinks, sink);
        e_comp_object_frame_volume_update(ec->frame);
        _e_client_volume_sink_event_simple(ec, sink,
                                           E_EVENT_CLIENT_VOLUME_SINK_DEL);
        e_object_unref(E_OBJECT(ec));
     }
   free(sink);
}

E_API void
e_client_volume_sink_set(E_Client_Volume_Sink *sink, int volume, Eina_Bool mute)
{
   EINA_SAFETY_ON_NULL_RETURN(sink);
   if (sink->func_set)
     sink->func_set(volume, mute, sink->data);
}

E_API void
e_client_volume_sink_get(const E_Client_Volume_Sink *sink, int *volume, Eina_Bool *mute)
{
   EINA_SAFETY_ON_NULL_RETURN(sink);
   if (sink->func_get)
     sink->func_get(volume, mute, sink->data);
}

E_API int
e_client_volume_sink_min_get(const E_Client_Volume_Sink *sink)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sink, 0);
   if (sink->func_min_get)
     return sink->func_min_get(sink->data);

   return 0;
}

E_API int
e_client_volume_sink_max_get(const E_Client_Volume_Sink *sink)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sink, 0);
   if (sink->func_max_get)
     return sink->func_max_get(sink->data);
   return 0;
}

E_API const char *
e_client_volume_sink_name_get(const E_Client_Volume_Sink *sink)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sink, 0);
   if (sink->func_name_get)
     return sink->func_name_get(sink->data);
   return NULL;
}

static void
_e_client_volume_update(E_Client *ec)
{
   E_Client_Volume_Sink *sink;
   Eina_List *l;
   int volume_min, volume_max, volume, count;
   Eina_Bool mute;

   mute = EINA_TRUE;
   volume = 0;
   volume_min = 9999999;
   volume_max = 0;
   count = 0;
   EINA_LIST_FOREACH(ec->sinks, l, sink)
     {
        int volume_min2, volume_max2, volume2;
        Eina_Bool mute2;

        volume_min2 = e_client_volume_sink_min_get(sink);
        volume_max2 = e_client_volume_sink_max_get(sink);
        if (volume_min2 < volume_min) volume_min = volume_min2;
        if (volume_max2 > volume_max) volume_max = volume_max2;
        e_client_volume_sink_get(sink, &volume2, &mute2);
        if (!mute2)
          {
             mute = EINA_FALSE;
             volume += volume2;
             count++;
          }
     }
   if (ec->sinks)
     {
        ec->volume_min = volume_min;
        ec->volume_max = volume_max;
        if (count == 0)
          {
             if (ec->volume < volume_min)
               ec->volume = volume_min;
             if (ec->volume > volume_max)
               ec->volume = volume_max;
          }
        else
          ec->volume = volume / count;
        ec->mute = mute;
        ec->volume_control_enabled = EINA_TRUE;
     }
   else
     {
        ec->volume_min = 0;
        ec->volume_max = 0;
        ec->volume = 0;
        ec->mute = EINA_FALSE;
        ec->volume_control_enabled = EINA_FALSE;
     }
   if (ec->volume_control_enabled)
     {
        e_comp_object_frame_volume_update(ec->frame);
        e_client_volume_display_set(ec, ec->volume, ec->mute);
     }
}

E_API void
e_client_volume_sink_append(E_Client *ec, E_Client_Volume_Sink *sink)
{
   int volume;
   Eina_Bool mute;

   EINA_SAFETY_ON_NULL_RETURN(ec);

   ec->sinks = eina_list_append(ec->sinks, sink);
   sink->clients = eina_list_append(sink->clients, ec);
   e_object_ref(E_OBJECT(ec));
   if (!ec->volume_control_enabled)
     {
        ec->volume_min = e_client_volume_sink_min_get(sink);
        ec->volume_max = e_client_volume_sink_max_get(sink);
        e_client_volume_sink_get(sink, &volume, &mute);
        ec->volume = volume;
        ec->mute = !!mute;
        ec->volume_control_enabled = EINA_TRUE;
     }
   _e_client_volume_update(ec);
   _e_client_volume_sink_event_simple(ec, sink,
                                      E_EVENT_CLIENT_VOLUME_SINK_ADD);
}

E_API void
e_client_volume_sink_remove(E_Client *ec, E_Client_Volume_Sink *sink)
{
   EINA_SAFETY_ON_NULL_RETURN(ec);
   ec->sinks = eina_list_remove(ec->sinks, sink);
   sink->clients = eina_list_remove(sink->clients, ec);
   _e_client_volume_update(ec);
   _e_client_volume_sink_event_simple(ec, sink,
                                      E_EVENT_CLIENT_VOLUME_SINK_DEL);
   e_object_unref(E_OBJECT(ec));
}

E_API void
e_client_volume_sink_update(E_Client_Volume_Sink *sink)
{
   Eina_List *l;
   E_Client *ec;

   EINA_SAFETY_ON_NULL_RETURN(sink);

   EINA_LIST_FOREACH(sink->clients, l, ec)
     {
        _e_client_volume_update(ec);
        _e_client_volume_sink_event_simple(ec, sink,
                                           E_EVENT_CLIENT_VOLUME_SINK_CHANGED);
     }
}

E_API void
e_client_volume_display_set(E_Client *ec, int volume, Eina_Bool mute)
{
   ec->volume = volume;
   ec->mute = !!mute;
   if (mute)
     _e_client_volume_event_simple(ec, E_EVENT_CLIENT_MUTE);
   else
     _e_client_volume_event_simple(ec, E_EVENT_CLIENT_UNMUTE);
   _e_client_volume_event_simple(ec, E_EVENT_CLIENT_VOLUME);
}

E_API void
e_client_volume_set(E_Client *ec, int volume)
{
   Eina_List *l;
   E_Client_Volume_Sink *sink;

   EINA_SAFETY_ON_NULL_RETURN(ec);
   if (volume < ec->volume_min) volume = ec->volume_min;
   if (volume > ec->volume_max) volume = ec->volume_max;
   if (volume == ec->volume) return;
   ec->volume = volume;

   EINA_LIST_FOREACH(ec->sinks, l, sink)
     {
        e_client_volume_sink_set(sink, ec->volume, ec->mute);
     }

   _e_client_volume_update(ec);
   _e_client_volume_event_simple(ec, E_EVENT_CLIENT_VOLUME);
}

E_API void
e_client_volume_mute_set(E_Client *ec, Eina_Bool mute)
{
   Eina_List *l;
   E_Client_Volume_Sink *sink;

   EINA_SAFETY_ON_NULL_RETURN(ec);
   if (!!mute == ec->mute) return;
   ec->mute = !!mute;

   EINA_LIST_FOREACH(ec->sinks, l, sink)
     {
        if (sink->func_set)
          sink->func_set(ec->volume, ec->mute, sink->data);
     }
   if (mute)
     _e_client_volume_event_simple(ec, E_EVENT_CLIENT_MUTE);
   else
     _e_client_volume_event_simple(ec, E_EVENT_CLIENT_UNMUTE);
}

E_API Evas_Object *
e_client_volume_object_add(E_Client *ec, Evas *evas)
{
   Evas_Object *bx, *o;
   Edje_Message_Int_Set *msg;
   Eina_List *handlers = NULL;

   bx = edje_object_add(evas);
   e_theme_edje_object_set(bx,
                           "base/theme/border/volume",
                           "e/widgets/border/volume");

   if (edje_object_part_exists(bx, "e.swallow.volume_icon"))
     {
        o = edje_object_add(evas);
        e_theme_edje_object_set(o,
                                "base/theme/modules/mixer",
                                "e/modules/mixer/main");

        evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
                                       _e_client_volume_object_mouse_down_cb,
                                       ec);
        edje_object_part_swallow(bx, "e.swallow.volume_icon", o);
        msg = alloca(sizeof(Edje_Message_Int_Set) + (2 * sizeof(int)));
        msg->count = 3;
        msg->val[0] = ec->mute;
        msg->val[1] = ec->volume;
        msg->val[2] = msg->val[1];
        edje_object_message_send(o, EDJE_MESSAGE_INT_SET, 0, msg);
        edje_object_signal_emit(o,  "e,action,volume,change", "e");
        evas_object_show(o);
     }
   if (edje_object_part_exists(bx, "e.swallow.volume"))
     {
        o = elm_slider_add(e_comp->elm);
        elm_slider_min_max_set(o, ec->volume_min,
                               ec->volume_max);
        elm_slider_span_size_set(o, ec->volume_max
                                 * elm_config_scale_get());
        elm_slider_unit_format_set(o, "%.0f");
        elm_slider_indicator_format_set(o, "%.0f");
        evas_object_smart_callback_add(o, "changed",
                                       _e_client_volume_object_volume_changed,
                                       ec);
        evas_object_smart_callback_add(o, "slider,drag,stop",
                                       _e_client_volume_object_volume_drag_stop,
                                       ec);
        elm_slider_value_set(o, ec->volume);
        edje_object_part_swallow(bx, "e.swallow.volume", o);
        evas_object_show(o);
     }

   E_LIST_HANDLER_APPEND(handlers, E_EVENT_CLIENT_VOLUME,
                         _e_client_volume_object_changed, bx);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_CLIENT_MUTE,
                         _e_client_volume_object_changed, bx);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_CLIENT_UNMUTE,
                         _e_client_volume_object_changed, bx);

   evas_object_event_callback_add(bx, EVAS_CALLBACK_DEL,
                            _e_client_volume_object_del_cb, handlers);
   return bx;
}

E_API void
e_client_volume_object_emit(E_Client *ec, const char *sig, const char *src)
{
   Evas_Object *o;

   EINA_SAFETY_ON_NULL_RETURN(ec);
   o = e_comp_object_frame_volume_get(ec->frame);
   if (o)
     {
        edje_object_signal_emit(o, sig, src);
        o = edje_object_part_swallow_get(o, "e.swallow.volume_icon");
        if (o) edje_object_signal_emit(o,  sig, src);
     }
}
