#include "e.h"
#include "emix.h"

#include <pulse/pulseaudio.h>

#undef ERR
#undef DBG
#undef WRN
#define ERR(...)      EINA_LOG_ERR(__VA_ARGS__)
#define DBG(...)      EINA_LOG_DBG(__VA_ARGS__)
#define WRN(...)      EINA_LOG_WARN(__VA_ARGS__)

#define PA_VOLUME_TO_INT(_vol) \
   (((_vol * EMIX_VOLUME_BARRIER) / \
    (double)PA_VOLUME_NORM) + 0.5)
#define INT_TO_PA_VOLUME(_vol) \
   (((PA_VOLUME_NORM * _vol) / \
    (double)EMIX_VOLUME_BARRIER) + 0.5)

typedef struct _Context
{
   pa_mainloop_api api;
   pa_context *context;
   pa_context_state_t state;
   Emix_Event_Cb cb;
   const void *userdata;
   Ecore_Timer *connect;

   Eina_List *sinks, *sources, *inputs, *cards;
   Eina_Bool connected;
} Context;

typedef struct _Sink
{
   Emix_Sink base;
   int idx, monitor_idx;
   const char *pulse_name;
   const char *monitor_source_name;
   int mon_count;
   pa_stream *mon_stream;
   Eina_Bool running : 1;
} Sink;

typedef struct _Sink_Input
{
   Emix_Sink_Input base;
   int idx, sink_idx;
   int mon_count;
   pa_stream *mon_stream;
   Eina_Bool running : 1;
} Sink_Input;

typedef struct _Source
{
   Emix_Source base;
   int idx;
   const char *pulse_name;
   int mon_count;
   pa_stream *mon_stream;
} Source;

typedef struct _Profile
{
   Emix_Profile base;
   uint32_t priority;
} Profile;

typedef struct _Card
{
   Emix_Card base;
   int idx;
} Card;

static Context *ctx = NULL;
extern pa_mainloop_api functable;

static void _sink_monitor_begin(Sink *sink);
static void _sink_monitor_end(Sink *sink);
static void _sink_input_monitor_begin(Sink_Input *i);
static void _sink_input_monitor_end(Sink_Input *i);
static void _source_monitor_begin(Source *s);
static void _source_monitor_end(Source *s);

static pa_cvolume
_emix_volume_convert(const Emix_Volume *volume)
{
   pa_cvolume vol;
   unsigned int i;

   vol.channels = volume->channel_count;
   for (i = 0; i < volume->channel_count; i++)
     vol.values[i] = INT_TO_PA_VOLUME(volume->volumes[i]);
   return vol;
}

static void
_pa_cvolume_convert(const pa_cvolume *volume, Emix_Volume *vol)
{
   int i;

   if (vol->volumes) free(vol->volumes);
   vol->volumes = calloc(volume->channels, sizeof(int));
   if (!vol->volumes)
     {
        WRN("Could not allocate memory for volume");
        vol->channel_count = 0;
        return;
     }

   vol->channel_count = volume->channels;
   for (i = 0; i < volume->channels; i++)
     vol->volumes[i] = PA_VOLUME_TO_INT(volume->values[i]);
}

static void
_sink_del(Sink *sink)
{
   unsigned int i;
   Emix_Port *port;

   EINA_SAFETY_ON_NULL_RETURN(sink);
   EINA_LIST_FREE(sink->base.ports, port)
     {
        eina_stringshare_del(port->name);
        eina_stringshare_del(port->description);
        free(port);
     }

   free(sink->base.volume.volumes);
   for(i = 0; i < sink->base.volume.channel_count; ++i)
     eina_stringshare_del(sink->base.volume.channel_names[i]);
   free(sink->base.volume.channel_names);
   eina_stringshare_del(sink->base.name);
   eina_stringshare_del(sink->pulse_name);
   eina_stringshare_del(sink->monitor_source_name);
   if (sink->mon_stream) pa_stream_disconnect(sink->mon_stream);
   free(sink);
}

static void
_sink_input_del(Sink_Input *input)
{
   unsigned int i;
   EINA_SAFETY_ON_NULL_RETURN(input);

   free(input->base.volume.volumes);
   for(i = 0; i < input->base.volume.channel_count; ++i)
     eina_stringshare_del(input->base.volume.channel_names[i]);
   free(input->base.volume.channel_names);
   eina_stringshare_del(input->base.name);
   eina_stringshare_del(input->base.icon);
   if (input->mon_stream) pa_stream_disconnect(input->mon_stream);
   free(input);
}

static void
_source_del(Source *source)
{
   unsigned int i;
   EINA_SAFETY_ON_NULL_RETURN(source);

   free(source->base.volume.volumes);
   for(i = 0; i < source->base.volume.channel_count; ++i)
     eina_stringshare_del(source->base.volume.channel_names[i]);
   free(source->base.volume.channel_names);
   eina_stringshare_del(source->base.name);
   eina_stringshare_del(source->pulse_name);
   free(source);
}

static void
_card_del(Card *card)
{
   Profile *profile;
   EINA_SAFETY_ON_NULL_RETURN(card);

   EINA_LIST_FREE(card->base.profiles, profile)
     {
        eina_stringshare_del(profile->base.name);
        eina_stringshare_del(profile->base.description);
        free(profile);
     }
   eina_stringshare_del(card->base.name);
   free(card);
}

static void
_sink_state_running_set(Sink *sink, Eina_Bool running)
{
   if (running)
     {
        if ((!sink->running) && (sink->mon_count > 0))
          {
             sink->running = running;
             _sink_monitor_begin(sink);
             return;
          }
     }
   else
     {
        if ((sink->running) && (sink->mon_count > 0))
          {
             sink->running = running;
             _sink_monitor_end(sink);
             return;
          }
     }
   sink->running = running;
}

static void
_sink_cb(pa_context *c EINA_UNUSED, const pa_sink_info *info, int eol,
         void *userdata EINA_UNUSED)
{
   Sink *sink;
   Emix_Port *port;
   uint32_t i;

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Sink callback failure");
        return;
     }

   if (eol > 0)
      return;

   DBG("sink index: %d\nsink name: %s", info->index,
       info->name);

   sink = calloc(1, sizeof(Sink));
   sink->idx = info->index;
   sink->monitor_idx = info->monitor_source;
   sink->pulse_name = eina_stringshare_add(info->name);
   sink->base.name = eina_stringshare_add(info->description);
   _pa_cvolume_convert(&info->volume, &sink->base.volume);
   sink->base.volume.channel_names = calloc(sink->base.volume.channel_count, sizeof(Emix_Channel));
   for (i = 0; i < sink->base.volume.channel_count; ++i)
     sink->base.volume.channel_names[i] = eina_stringshare_add(pa_channel_position_to_pretty_string(info->channel_map.map[i]));
   sink->base.mute = !!info->mute;
   sink->monitor_source_name = eina_stringshare_add(info->monitor_source_name);

   for (i = 0; i < info->n_ports; i++)
     {
        port = calloc(1, sizeof(Emix_Port));
        if (!port)
          {
             WRN("Could not allocate memory for Sink's port");
             continue;
          }

        port->available = !!info->ports[i]->available;
        port->name = eina_stringshare_add(info->ports[i]->name);
        port->description =  eina_stringshare_add(info->ports[i]->description);
        sink->base.ports = eina_list_append(sink->base.ports, port);
        if (info->ports[i]->name == info->active_port->name)
           port->active = EINA_TRUE;
     }
   if (info->state == PA_SINK_RUNNING) _sink_state_running_set(sink, EINA_TRUE);
   else _sink_state_running_set(sink, EINA_FALSE);
   ctx->sinks = eina_list_append(ctx->sinks, sink);

   if (ctx->cb)
      ctx->cb((void *)ctx->userdata, EMIX_SINK_ADDED_EVENT,
              (Emix_Sink *)sink);
}

static void
_sink_changed_cb(pa_context *c EINA_UNUSED, const pa_sink_info *info, int eol,
                 void *userdata EINA_UNUSED)
{
   Sink *sink = NULL, *s;
   Emix_Port *port;
   uint32_t i;
   Eina_List *l;

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Sink callback failure");
        return;
     }

   if (eol > 0)
      return;

   DBG("sink index: %d\nsink name: %s", info->index,
       info->name);

   EINA_LIST_FOREACH(ctx->sinks, l, s)
     {
        if (s->idx == (int)info->index)
          {
             sink = s;
             break;
          }
     }

   EINA_SAFETY_ON_NULL_RETURN(sink);

   eina_stringshare_replace(&sink->base.name, info->description);

   if (sink->base.volume.channel_count != info->volume.channels)
     {
        for (i = 0; i < sink->base.volume.channel_count; ++i)
          eina_stringshare_del(sink->base.volume.channel_names[i]);
        free(sink->base.volume.channel_names);
        sink->base.volume.channel_names = calloc(info->volume.channels, sizeof(Emix_Channel));
     }
   _pa_cvolume_convert(&info->volume, &sink->base.volume);
   for (i = 0; i < sink->base.volume.channel_count; ++i)
     eina_stringshare_replace(&sink->base.volume.channel_names[i],
                              pa_channel_position_to_pretty_string(info->channel_map.map[i]));
   sink->base.mute = !!info->mute;

   if (sink->base.ports)
     {
        EINA_LIST_FREE(sink->base.ports, port)
          {
             eina_stringshare_del(port->name);
             eina_stringshare_del(port->description);
             free(port);
          }
     }
   for (i = 0; i < info->n_ports; i++)
     {
        port = calloc(1, sizeof(Emix_Port));
        if (!port)
          {
             WRN("Could not allocate memory for Sink's port");
             continue;
          }

        port->available = !!info->ports[i]->available;
        port->name = eina_stringshare_add(info->ports[i]->name);
        port->description =  eina_stringshare_add(info->ports[i]->description);
        sink->base.ports = eina_list_append(sink->base.ports, port);
        if (info->ports[i]->name == info->active_port->name)
           port->active = EINA_TRUE;
     }

   sink->monitor_idx = info->monitor_source;

   if (info->state == PA_SINK_RUNNING) _sink_state_running_set(sink, EINA_TRUE);
   else _sink_state_running_set(sink, EINA_FALSE);

   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SINK_CHANGED_EVENT,
                  (Emix_Sink *)sink);
}

static void
_sink_remove_cb(int index, void *data EINA_UNUSED)
{
   Sink *sink;
   Eina_List *l;
   DBG("Removing sink: %d", index);

   EINA_LIST_FOREACH(ctx->sinks, l, sink)
     {
        if (sink->idx == index)
          {
             ctx->sinks = eina_list_remove_list(ctx->sinks, l);
             if (ctx->cb)
               ctx->cb((void *)ctx->userdata, EMIX_SINK_REMOVED_EVENT,
                            (Emix_Sink *)sink);
             _sink_del(sink);
             break;
          }
     }
}

static const char *
_icon_from_properties(pa_proplist *l)
{
   const char *t;

   if ((t = pa_proplist_gets(l, PA_PROP_MEDIA_ICON_NAME))) return t;
   if ((t = pa_proplist_gets(l, PA_PROP_WINDOW_ICON_NAME))) return t;
   if ((t = pa_proplist_gets(l, PA_PROP_APPLICATION_ICON_NAME))) return t;
   if ((t = pa_proplist_gets(l, PA_PROP_APPLICATION_ICON))) return t;
   if ((t = pa_proplist_gets(l, PA_PROP_APPLICATION_ID))) return t;
   if ((t = pa_proplist_gets(l, PA_PROP_APPLICATION_NAME))) return t;

   if ((t = pa_proplist_gets(l, PA_PROP_MEDIA_ROLE)))
     {
        if ((!strcmp(t, "video")) || (!strcmp(t, "phone"))) return t;
        if (!strcmp(t, "music")) return "multimedia-player";
        if (!strcmp(t, "game")) return "applications-games";
        if (!strcmp(t, "event")) return "dialog-information";
     }
   return NULL;
}

static void
_sink_input_state_running_set(Sink_Input *input, Eina_Bool running)
{
   if (running)
     {
        if ((!input->running) && (input->mon_count > 0))
          {
             input->running = running;
             _sink_input_monitor_begin(input);
             return;
          }
     }
   else
     {
        if ((input->running) && (input->mon_count > 0))
          {
             input->running = running;
             _sink_input_monitor_end(input);
             return;
          }
     }
   input->running = running;
}

static void
_sink_input_cb(pa_context *c EINA_UNUSED, const pa_sink_input_info *info,
               int eol, void *userdata EINA_UNUSED)
{
   Sink_Input *input;
   Eina_List *l;
   Sink *s;
   const char *t;
   unsigned int i;
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
          return;

        ERR("Sink input callback failure");
        return;
     }

   if (eol > 0)
      return;

   input = calloc(1, sizeof(Sink_Input));
   EINA_SAFETY_ON_NULL_RETURN(input);

   DBG("sink input index: %d\nsink input name: %s", info->index,
       info->name);

   input->idx = info->index;
   input->sink_idx = info->sink;

   Eina_Strbuf *input_name;

   input_name = eina_strbuf_new();
   const char *application = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
   if (application)
     {
        eina_strbuf_append(input_name, application);
        eina_strbuf_append(input_name, ":");
        eina_strbuf_append(input_name, info->name);
     }
   else if (info->name)
     {
        eina_strbuf_append(input_name, info->name);
     }
   input->base.name = eina_stringshare_add(eina_strbuf_string_get(input_name));
   eina_strbuf_free(input_name);
   _pa_cvolume_convert(&info->volume, &input->base.volume);
   input->base.volume.channel_names = calloc(input->base.volume.channel_count, sizeof(Emix_Channel));
   for (i = 0; i < input->base.volume.channel_count; ++i)
     input->base.volume.channel_names[i] = eina_stringshare_add(pa_channel_position_to_pretty_string(info->channel_map.map[i]));
   input->base.mute = !!info->mute;
   EINA_LIST_FOREACH(ctx->sinks, l, s)
     {
        if (s->idx == (int)info->sink)
          input->base.sink = (Emix_Sink *)s;
     }
   input->base.icon = eina_stringshare_add(_icon_from_properties(info->proplist));
   ctx->inputs = eina_list_append(ctx->inputs, input);

   if ((t = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_PROCESS_ID)))
     {
        input->base.pid = atoi(t);
     }
   if (!info->corked) _sink_input_state_running_set(input, EINA_TRUE);
   else _sink_input_state_running_set(input, EINA_FALSE);

   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SINK_INPUT_ADDED_EVENT,
             (Emix_Sink_Input *)input);
}

static void
_sink_input_changed_cb(pa_context *c EINA_UNUSED,
                       const pa_sink_input_info *info, int eol,
                       void *userdata EINA_UNUSED)
{
   Sink_Input *input = NULL, *si;
   Sink *s = NULL;
   Eina_List *l;
   const char *t;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(ctx);
   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Sink input changed callback failure");
        return;
     }

   if (eol > 0)
      return;

   EINA_LIST_FOREACH(ctx->inputs, l, si)
     {
        if (si->idx == (int)info->index)
          {
            input = si;
            break;
          }
     }

   DBG("sink input changed index: %d\n", info->index);

   if (!input)
     {
        input = calloc(1, sizeof(Sink_Input));
        EINA_SAFETY_ON_NULL_RETURN(input);
        ctx->inputs = eina_list_append(ctx->inputs, input);
     }
   input->idx = info->index;
   input->sink_idx = info->sink;
   if (input->base.volume.channel_count != info->volume.channels)
     {
        for (i = 0; i < input->base.volume.channel_count; ++i)
          eina_stringshare_del(input->base.volume.channel_names[i]);
        free(input->base.volume.channel_names);
        input->base.volume.channel_names = calloc(info->volume.channels, sizeof(Emix_Channel));
     }
   _pa_cvolume_convert(&info->volume, &input->base.volume);
   for (i = 0; i < input->base.volume.channel_count; ++i)
     eina_stringshare_replace(&input->base.volume.channel_names[i],
                              pa_channel_position_to_pretty_string(info->channel_map.map[i]));

   input->base.mute = !!info->mute;

   EINA_LIST_FOREACH(ctx->sinks, l, s)
     {
        if (s->idx == (int)info->sink)
          input->base.sink = (Emix_Sink *)s;
     }
   if ((t = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_PROCESS_ID)))
     {
        input->base.pid = atoi(t);
     }

   if (!info->corked) _sink_input_state_running_set(input, EINA_TRUE);
   else _sink_input_state_running_set(input, EINA_FALSE);

   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SINK_INPUT_CHANGED_EVENT,
             (Emix_Sink_Input *)input);

   if (input->mon_count > 0)
     {
        _sink_input_monitor_end(input);
        if (input->running) _sink_input_monitor_begin(input);
     }
}

static void
_sink_input_remove_cb(int index, void *data EINA_UNUSED)
{
   Sink_Input *input;
   Eina_List *l;
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   DBG("Removing sink input: %d", index);

   EINA_LIST_FOREACH(ctx->inputs, l, input)
     {
        if (input->idx == index)
          {
             ctx->inputs = eina_list_remove_list(ctx->inputs, l);
             if (ctx->cb)
               ctx->cb((void *)ctx->userdata,
                       EMIX_SINK_INPUT_REMOVED_EVENT,
                       (Emix_Sink_Input *)input);
             _sink_input_del(input);

             break;
          }
     }
}

static void
_source_cb(pa_context *c EINA_UNUSED, const pa_source_info *info,
           int eol, void *userdata EINA_UNUSED)
{
   Source *source;
   unsigned int i;
   size_t len;
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Source callback failure");
        return;
     }

   if (eol > 0)
      return;

   len = strlen(info->name);
   if (len > 8)
     {
        const char *s = info->name + len - 8;
        if (!strcmp(s, ".monitor")) return;
     }

   source = calloc(1, sizeof(Source));
   EINA_SAFETY_ON_NULL_RETURN(source);

   source->idx = info->index;
   source->pulse_name = eina_stringshare_add(info->name);
   source->base.name = eina_stringshare_add(info->description);
   _pa_cvolume_convert(&info->volume, &source->base.volume);
   source->base.volume.channel_names = calloc(source->base.volume.channel_count, sizeof(Emix_Channel));
   for (i = 0; i < source->base.volume.channel_count; ++i)
     source->base.volume.channel_names[i] = eina_stringshare_add(pa_channel_position_to_pretty_string(info->channel_map.map[i]));
   source->base.mute = !!info->mute;

   ctx->sources = eina_list_append(ctx->sources, source);
   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SOURCE_ADDED_EVENT,
             (Emix_Source *)source);
}

static void
_source_changed_cb(pa_context *c EINA_UNUSED,
                       const pa_source_info *info, int eol,
                       void *userdata EINA_UNUSED)
{
   Source *source = NULL, *s;
   Eina_List *l;
   unsigned int i;
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Source changed callback failure");
        return;
     }

   if (eol > 0)
      return;

   EINA_LIST_FOREACH(ctx->sources, l, s)
     {
        if (s->idx == (int)info->index)
          {
             source = s;
             break;
          }
     }

   DBG("source changed index: %d\n", info->index);

   if (!source)
     {
        source = calloc(1, sizeof(Source));
        EINA_SAFETY_ON_NULL_RETURN(source);
        ctx->sources = eina_list_append(ctx->sources, source);
     }
   source->idx = info->index;
   if (source->base.volume.channel_count != info->volume.channels)
     {
        for (i = 0; i < source->base.volume.channel_count; ++i)
          eina_stringshare_del(source->base.volume.channel_names[i]);
        free(source->base.volume.channel_names);
        source->base.volume.channel_names = calloc(info->volume.channels, sizeof(Emix_Channel));
     }
   _pa_cvolume_convert(&info->volume, &source->base.volume);
   for (i = 0; i < source->base.volume.channel_count; ++i)
     eina_stringshare_replace(&source->base.volume.channel_names[i],
                              pa_channel_position_to_pretty_string(info->channel_map.map[i]));
   source->base.mute = !!info->mute;

   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SOURCE_CHANGED_EVENT,
                  (Emix_Source *)source);
}

static void
_source_remove_cb(int index, void *data EINA_UNUSED)
{
   Source *source;
   Eina_List *l;
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   DBG("Removing source: %d", index);

   EINA_LIST_FOREACH(ctx->sources, l, source)
     {
        if (source->idx == index)
          {
             ctx->sources = eina_list_remove_list(ctx->sources, l);
             if (ctx->cb)
               ctx->cb((void *)ctx->userdata, EMIX_SOURCE_REMOVED_EVENT,
                       (Emix_Source *)source);

             _source_del(source);
             break;
          }
     }
}

static void
_sink_default_cb(pa_context *c EINA_UNUSED, const pa_sink_info *info, int eol,
                 void *userdata EINA_UNUSED)
{
   Sink *sink;
   Eina_List *l;

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Sink callback failure");
        return;
     }

   if (eol > 0)
      return;

   DBG("sink index: %d\nsink name: %s", info->index,
       info->name);

   EINA_LIST_FOREACH(ctx->sinks, l, sink)
     {
        Eina_Bool prev_default = sink->base.default_sink;
        sink->base.default_sink = (uint32_t)sink->idx == info->index;
        if (ctx->cb && (prev_default != sink->base.default_sink))
          ctx->cb((void *)ctx->userdata, EMIX_SINK_CHANGED_EVENT,
                  (Emix_Sink *)sink);
     }

   if (ctx->cb)
      ctx->cb((void *)ctx->userdata, EMIX_READY_EVENT, NULL);
}

static void
_source_default_cb(pa_context *c EINA_UNUSED, const pa_source_info *info, int eol,
                 void *userdata EINA_UNUSED)
{
   Source *source;
   Eina_List *l;

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Source callback failure");
        return;
     }

   if (eol > 0)
      return;

   DBG("source index: %d\nsource name: %s", info->index,
       info->name);

   EINA_LIST_FOREACH(ctx->sources, l, source)
     {
        Eina_Bool prev_default = source->base.default_source;
        source->base.default_source = (uint32_t)source->idx == info->index;
        if (ctx->cb && prev_default != source->base.default_source)
          ctx->cb((void *)ctx->userdata, EMIX_SOURCE_CHANGED_EVENT,
                  (Emix_Source *)source);
     }

   if (ctx->cb)
      ctx->cb((void *)ctx->userdata, EMIX_READY_EVENT, NULL);
}

static void
_server_info_cb(pa_context *c, const pa_server_info *info,
           void *userdata)
{
   pa_operation *o;

// this seems to trap old errors then forget to update defaults as below
//   if (pa_context_errno(c) != PA_OK)
//     {
//        WRN("Could not get pa server info, error: %d", pa_context_errno(c));
//        return;
//     }

   EINA_SAFETY_ON_NULL_RETURN(info);

   if (!(o = pa_context_get_sink_info_by_name(c, info->default_sink_name,
                                              _sink_default_cb, userdata)))
     {
        ERR("pa_context_get_sink_info_by_name() failed");
        return;
     }
   pa_operation_unref(o);

   if (!(o = pa_context_get_source_info_by_name(c, info->default_source_name,
                                              _source_default_cb, userdata)))
     {
        ERR("pa_context_get_source_info_by_name() failed");
        return;
     }
   pa_operation_unref(o);
}

static int
_profile_sort_cb(const Profile *p1, const Profile *p2)
{
   if (p1->priority < p2->priority) return 1;
   if (p1->priority > p2->priority) return -1;
   return 0;
}

static void
_card_cb(pa_context *c, const pa_card_info *info, int eol, void *userdata EINA_UNUSED)
{
     Card *card;
     const char *description;
     uint32_t i, j;
     Profile *profile;

     if (eol < 0) {
          if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

          ERR("Card callback failure: %d", pa_context_errno(c));
          return;
     }
   if (eol > 0)
      return;

   card = calloc(1, sizeof(Card));
   EINA_SAFETY_ON_NULL_RETURN(card);

   card->idx = info->index;

   description = pa_proplist_gets(info->proplist, PA_PROP_DEVICE_DESCRIPTION);
   card->base.name = description
      ? eina_stringshare_add(description)
      : eina_stringshare_add(info->name);

   for (i = 0; i < info->n_ports; ++i)
     {
        for (j = 0; j < info->ports[i]->n_profiles; ++j)
          {
             profile = calloc(1, sizeof(Profile));
             profile->base.name = eina_stringshare_add(info->ports[i]->profiles[j]->name);
             profile->base.description = eina_stringshare_add(info->ports[i]->profiles[j]->description);
             profile->priority = info->ports[i]->profiles[j]->priority;
             if ((info->ports[i]->available == PA_PORT_AVAILABLE_NO)
               && ((strcmp("analog-output-speaker", profile->base.name))
                   || (strcmp("analog-input-microphone-internal", profile->base.name))))
                  profile->base.plugged = EINA_FALSE;
             else
               profile->base.plugged = EINA_TRUE;

             if (info->active_profile)
               {
                  if (info->ports[i]->profiles[j]->name == info->active_profile->name)
                    profile->base.active = EINA_TRUE;
               }
             card->base.profiles =
                eina_list_sorted_insert(card->base.profiles,
                                        (Eina_Compare_Cb)_profile_sort_cb,
                                        profile);
          }
     }
   ctx->cards = eina_list_append(ctx->cards, card);
   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_CARD_ADDED_EVENT,
             (Emix_Card *)card);
}

static void
_card_changed_cb(pa_context *c EINA_UNUSED, const pa_card_info *info, int eol,
                 void *userdata EINA_UNUSED)
{
   Card *card = NULL, *ca;
   Eina_List *l;
   const char *description;
   uint32_t i, j;
   Profile *profile;

   if (eol < 0)
     {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
           return;

        ERR("Card callback failure");
        return;
     }

   if (eol > 0)
      return;

   DBG("card index: %d\n", info->index);

   EINA_LIST_FOREACH(ctx->cards, l, ca)
     {
        if (ca->idx == (int)info->index)
          {
             card = ca;
             break;
          }
     }

   EINA_SAFETY_ON_NULL_RETURN(card);

   description = pa_proplist_gets(info->proplist, PA_PROP_DEVICE_DESCRIPTION);
   eina_stringshare_replace(&card->base.name,
                            (description
                             ? eina_stringshare_add(description)
                             : eina_stringshare_add(info->name)));

   EINA_LIST_FREE(card->base.profiles, profile)
     {
        eina_stringshare_del(profile->base.name);
        eina_stringshare_del(profile->base.description);
        free(profile);
     }
   for (i = 0; i < info->n_ports; ++i)
     {
        for (j = 0; j < info->ports[i]->n_profiles; ++j)
          {
             profile = calloc(1, sizeof(Profile));
             profile->base.name = eina_stringshare_add(info->ports[i]->profiles[j]->name);
             profile->base.description = eina_stringshare_add(info->ports[i]->profiles[j]->description);
             profile->priority = info->ports[i]->profiles[j]->priority;
             if ((info->ports[i]->available == PA_PORT_AVAILABLE_NO)
               && ((strcmp("analog-output-speaker", profile->base.name))
                   || (strcmp("analog-input-microphone-internal", profile->base.name))))
                  profile->base.plugged = EINA_FALSE;
             else
               profile->base.plugged = EINA_TRUE;

             if (info->active_profile)
               {
                  if (info->ports[i]->profiles[j]->name == info->active_profile->name)
                    profile->base.active = EINA_TRUE;
               }
             card->base.profiles =
                eina_list_sorted_insert(card->base.profiles,
                                        (Eina_Compare_Cb)_profile_sort_cb,
                                        profile);
          }
     }

   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_CARD_CHANGED_EVENT,
                  (Emix_Card *)card);
}

static void
_card_remove_cb(int index, void *data EINA_UNUSED)
{
   Card *card;
   Eina_List *l;
   EINA_SAFETY_ON_NULL_RETURN(ctx);

   DBG("Removing card: %d", index);

   EINA_LIST_FOREACH(ctx->cards, l, card)
     {
        if (card->idx == index)
          {
             ctx->cards = eina_list_remove_list(ctx->cards, l);
             if (ctx->cb)
               ctx->cb((void *)ctx->userdata,
                       EMIX_CARD_REMOVED_EVENT,
                       (Emix_Card *)card);
             _card_del(card);

             break;
          }
     }
}


static void
_subscribe_cb(pa_context *c, pa_subscription_event_type_t t,
              uint32_t index, void *data)
{
   pa_operation *o;

   switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
    case PA_SUBSCRIPTION_EVENT_SINK:
       if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
           PA_SUBSCRIPTION_EVENT_REMOVE)
          _sink_remove_cb(index, data);
       else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
                PA_SUBSCRIPTION_EVENT_NEW)
         {
            if (!(o = pa_context_get_sink_info_by_index(c, index,
                                                        _sink_cb, data)))
              {
                 ERR("pa_context_get_sink_info_by_index() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       else
         {
            if (!(o = pa_context_get_sink_info_by_index(c, index,
                                                        _sink_changed_cb,
                                                        data)))
              {
                 ERR("pa_context_get_sink_info_by_index() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       break;

    case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
       if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
           PA_SUBSCRIPTION_EVENT_REMOVE)
          _sink_input_remove_cb(index, data);
       else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
                PA_SUBSCRIPTION_EVENT_NEW)
         {
            if (!(o = pa_context_get_sink_input_info(c, index,
                                                     _sink_input_cb, data)))
              {
                 ERR("pa_context_get_sink_input_info() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       else
         {
            if (!(o = pa_context_get_sink_input_info(c, index,
                                                     _sink_input_changed_cb,
                                                     data)))
              {
                 ERR("pa_context_get_sink_input_info() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       break;

    case PA_SUBSCRIPTION_EVENT_SOURCE:
       if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
           PA_SUBSCRIPTION_EVENT_REMOVE)
          _source_remove_cb(index, data);
       else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
                PA_SUBSCRIPTION_EVENT_NEW)
         {
            if (!(o = pa_context_get_source_info_by_index(c, index,
                                                          _source_cb, data)))
              {
                 ERR("pa_context_get_source_info() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       else
         {
            if (!(o = pa_context_get_source_info_by_index(c, index,
                                                          _source_changed_cb,
                                                          data)))
              {
                 ERR("pa_context_get_source_info() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       break;
    case PA_SUBSCRIPTION_EVENT_SERVER:
       if (!(o = pa_context_get_server_info(c, _server_info_cb,
                                            data)))
         {
            ERR("pa_context_get_server_info() failed");
            return;
         }
       pa_operation_unref(o);
       break;

    case PA_SUBSCRIPTION_EVENT_CARD:
       if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
           PA_SUBSCRIPTION_EVENT_REMOVE)
           _card_remove_cb(index, data);
       else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) ==
                PA_SUBSCRIPTION_EVENT_NEW)
         {
            if (!(o = pa_context_get_card_info_by_index(c, index, _card_cb,
                                                        data)))
              {
                 ERR("pa_context_get_card_info() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       else
         {
            if (!(o = pa_context_get_card_info_by_index(c, index, _card_changed_cb,
                                                        data)))
              {
                 ERR("pa_context_get_card_info() failed");
                 return;
              }
            pa_operation_unref(o);
         }
       break;

    default:
       WRN("Event not handled");
       break;
   }
}

static Eina_Bool _pulse_connect(void *data);
static void _disconnect_cb(void);

static void
_pulse_pa_state_cb(pa_context *context, void *data)
{
   pa_operation *o;

   switch (pa_context_get_state(context))
     {
      case PA_CONTEXT_UNCONNECTED:
      case PA_CONTEXT_CONNECTING:
      case PA_CONTEXT_AUTHORIZING:
      case PA_CONTEXT_SETTING_NAME:
         break;

      case PA_CONTEXT_READY:
        ctx->connect = NULL;
        ctx->connected = EINA_TRUE;
        pa_context_set_subscribe_callback(context, _subscribe_cb, ctx);
        if (!(o = pa_context_subscribe(context, (pa_subscription_mask_t)
                                       (PA_SUBSCRIPTION_MASK_SINK|
                                        PA_SUBSCRIPTION_MASK_SOURCE|
                                        PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                        PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                        PA_SUBSCRIPTION_MASK_CLIENT|
                                        PA_SUBSCRIPTION_MASK_SERVER|
                                        PA_SUBSCRIPTION_MASK_CARD),
                                       NULL, NULL)))
          {
             ERR("pa_context_subscribe() failed");
             return;
          }
        pa_operation_unref(o);

        if (!(o = pa_context_get_sink_info_list(context, _sink_cb, ctx)))
          {
             ERR("pa_context_get_sink_info_list() failed");
             return;
          }
        pa_operation_unref(o);

        if (!(o = pa_context_get_sink_input_info_list(context,
                                                      _sink_input_cb,
                                                      ctx)))
          {
             ERR("pa_context_get_sink_input_info_list() failed");
             return;
          }
        pa_operation_unref(o);

        if (!(o = pa_context_get_source_info_list(context, _source_cb,
                                                  ctx)))
          {
             ERR("pa_context_get_source_info_list() failed");
             return;
          }
        pa_operation_unref(o);

        if (!(o = pa_context_get_server_info(context, _server_info_cb,
                                             ctx)))
          {
             ERR("pa_context_get_server_info() failed");
             return;
          }
        pa_operation_unref(o);

        if (!(o = pa_context_get_card_info_list(context, _card_cb,
                                             ctx)))
          {
             ERR("pa_context_get_server_info() failed");
             return;
          }
        pa_operation_unref(o);



        break;

      case PA_CONTEXT_FAILED:
         WRN("PA_CONTEXT_FAILED");
         if (!ctx->connect)
           ctx->connect = ecore_timer_loop_add(1, _pulse_connect, data);
         goto err;

      case PA_CONTEXT_TERMINATED:
         ERR("PA_CONTEXT_TERMINATE:");
         EINA_FALLTHROUGH;
        /* no break */

      default:
         if (ctx->connect)
           {
              ecore_timer_del(ctx->connect);
              ctx->connect = NULL;
           }
         goto err;
     }
   return;

err:
   if (ctx->connected)
     {
        _disconnect_cb();
        ctx->connected = EINA_FALSE;
     }
   pa_context_unref(ctx->context);
   ctx->context = NULL;
}

static Eina_Bool
_pulse_connect(void *data)
{
   pa_proplist *proplist;
   Context *c = data;

   proplist = pa_proplist_new();
   pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "Efl Volume Control");
   pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID,
                    "org.enlightenment.volumecontrol");
   pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
#if !defined(EMIXER_BUILD) && defined(HAVE_WAYLAND) && !defined(HAVE_WAYLAND_ONLY)
   char *display = NULL;

   if (e_comp->comp_type != E_PIXMAP_TYPE_X)
     {
        display = getenv("DISPLAY");
        if (display) display = strdup(display);
        e_env_unset("DISPLAY");
     }
#endif
   c->context = pa_context_new_with_proplist(&(c->api), NULL, proplist);
   if (c->context)
     {
        pa_context_set_state_callback(c->context, _pulse_pa_state_cb, c);
        if (pa_context_connect(c->context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0)
          ERR("Could not connect to pulse");
     }
#if !defined(EMIXER_BUILD) && defined(HAVE_WAYLAND) && !defined(HAVE_WAYLAND_ONLY)
   if (e_comp->comp_type != E_PIXMAP_TYPE_X)
     {
        if (display)
          {
             e_env_set("DISPLAY", display);
             free(display);
          }
     }
#endif

   pa_proplist_free(proplist);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool pulse_started;

static void
_shutdown(void)
{
   if (!ctx)
      return;
   if (pulse_started)
     {
        ecore_exe_run("pulseaudio -k", NULL);
        pulse_started = EINA_FALSE;
     }

   if (ctx->connect)
     {
        ecore_timer_del(ctx->connect);
        ctx->connect = NULL;
     }
   if (ctx->context)
      pa_context_unref(ctx->context);
   if (ctx->connected)
      _disconnect_cb();
   free(ctx);
   ctx = NULL;
}

static Eina_Bool
_init(Emix_Event_Cb cb, const void *data)
{
   if (ctx)
     return EINA_TRUE;

   ctx = calloc(1, sizeof(Context));
   if (!ctx)
     {
        ERR("Could not create Epulse Context");
        return EINA_FALSE;
     }

   ctx->api = functable;
   ctx->api.userdata = ctx;

   if (_pulse_connect(ctx) == EINA_TRUE) // true == failed and try again
     {
        if (!pulse_started)
          ecore_exe_run("pulseaudio --start", NULL);
        pulse_started = 1;
     }

   ctx->cb = cb;
   ctx->userdata = data;

   return EINA_TRUE;
 }

static void
_disconnect_cb()
{
   Source *source;
   Sink *sink;
   Sink_Input *input;
   Card *card;

   if (ctx->cb)
      ctx->cb((void *)ctx->userdata, EMIX_DISCONNECTED_EVENT, NULL);

   EINA_LIST_FREE(ctx->sources, source)
      _source_del(source);
   EINA_LIST_FREE(ctx->sinks, sink)
      _sink_del(sink);
   EINA_LIST_FREE(ctx->inputs, input)
      _sink_input_del(input);
   EINA_LIST_FREE(ctx->cards, card)
      _card_del(card);
}

static void
_source_volume_set(Emix_Source *source, Emix_Volume *volume)
{
   pa_operation* o;
   pa_cvolume vol = _emix_volume_convert(volume);
   Source *s = (Source *)source;
   EINA_SAFETY_ON_FALSE_RETURN(ctx && ctx->context && source != NULL);

   if (!(o = pa_context_set_source_volume_by_index(ctx->context,
                                                   s->idx, &vol,
                                                   NULL, NULL)))
      ERR("pa_context_set_source_volume_by_index() failed");
}

static void
_source_mute_set(Emix_Source *source, Eina_Bool mute)
{
   pa_operation* o;
   Source *s = (Source *)source;
   EINA_SAFETY_ON_FALSE_RETURN(ctx && ctx->context && source != NULL);

   if (!(o = pa_context_set_source_mute_by_index(ctx->context,
                                                 s->idx, mute, NULL, NULL)))
      ERR("pa_context_set_source_mute() failed");
}

static const Eina_List *
_sinks_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return ctx->sinks;
}

static const Eina_List *
_sources_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return ctx->sources;
}

static const Eina_List *
_sink_inputs_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return ctx->inputs;
}

static void
_sink_volume_set(Emix_Sink *sink, Emix_Volume *volume)
{
   pa_operation* o;
   Sink *s = (Sink *)sink;
   pa_cvolume vol = _emix_volume_convert(volume);
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->context && sink != NULL));

   if (!(o = pa_context_set_sink_volume_by_index(ctx->context,
                                                 s->idx, &vol, NULL, NULL)))
      ERR("pa_context_set_sink_volume_by_index() failed");
}

static void
_sink_mute_set(Emix_Sink *sink, Eina_Bool mute)
{
   pa_operation* o;
   Sink *s = (Sink *)sink;
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->context && sink != NULL));

   if (!(o = pa_context_set_sink_mute_by_index(ctx->context,
                                               s->idx, mute, NULL, NULL)))
      ERR("pa_context_set_sink_mute() failed");
}

static void
_sink_input_volume_set(Emix_Sink_Input *input, Emix_Volume *volume)
{
   pa_operation* o;
   Sink_Input *sink_input = (Sink_Input *)input;
   pa_cvolume vol = _emix_volume_convert(volume);
   EINA_SAFETY_ON_FALSE_RETURN(ctx && ctx->context && input != NULL);

   if (!(o = pa_context_set_sink_input_volume(ctx->context,
                                              sink_input->idx, &vol,
                                              NULL, NULL)))
      ERR("pa_context_set_sink_input_volume_by_index() failed");
}

static void
_sink_input_mute_set(Emix_Sink_Input *input, Eina_Bool mute)
{
   pa_operation* o;
   Sink_Input *sink_input = (Sink_Input *)input;
   EINA_SAFETY_ON_FALSE_RETURN(ctx && ctx->context && input != NULL);

   if (!(o = pa_context_set_sink_input_mute(ctx->context,
                                            sink_input->idx, mute,
                                            NULL, NULL)))
      ERR("pa_context_set_sink_input_mute() failed");
}

static void
_sink_input_move(Emix_Sink_Input *input, Emix_Sink *sink)
{
   pa_operation* o;
   Sink *s = (Sink *)sink;
   Sink_Input *i = (Sink_Input *)input;
   EINA_SAFETY_ON_FALSE_RETURN(ctx && ctx->context && input != NULL
                               && sink != NULL);

   if (!(o = pa_context_move_sink_input_by_index(ctx->context,
                                                 i->idx, s->idx, NULL,
                                                 NULL)))
      ERR("pa_context_move_sink_input_by_index() failed");
}

static Eina_Bool
_sink_port_set(Emix_Sink *sink, const Emix_Port *port)
{
   pa_operation* o;
   Sink *s = (Sink *)sink;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ctx && ctx->context &&
                                   sink != NULL && port != NULL, EINA_FALSE);

   if (!(o = pa_context_set_sink_port_by_index(ctx->context,
                                               s->idx, port->name, NULL,
                                               NULL)))
     {
        ERR("pa_context_set_source_port_by_index() failed");
        return EINA_FALSE;
     }
   pa_operation_unref(o);

   return EINA_TRUE;
}

static Eina_Bool
_sink_default_support(void)
{
   return EINA_TRUE;
}

static const Emix_Sink *
_sink_default_get(void)
{
   Sink *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   EINA_LIST_FOREACH(ctx->sinks, l, s)
      if (s->base.default_sink)
        return (Emix_Sink *)s;

   return NULL;
}

static void
_sink_default_set(Emix_Sink *sink)
{
  Sink *s = (Sink *)sink;
  pa_operation* o;

  DBG("Set default sink: %s", sink->name);
  if (!(o = pa_context_set_default_sink(ctx->context, s->pulse_name, NULL, NULL))) {
      ERR("pa_context_set_default_sink() failed");
      return;
  }
  pa_operation_unref(o);
}

static Eina_Bool
_source_default_support(void)
{
   return EINA_TRUE;
}

static const Emix_Source *
_source_default_get(void)
{
   Source *s;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   EINA_LIST_FOREACH(ctx->sources, l, s)
      if (s->base.default_source)
        return (Emix_Source *)s;

   return NULL;
}

static void
_source_default_set(Emix_Source *source)
{
  Source *s = (Source *)source;
  pa_operation* o;

  DBG("Set default sink: %s", source->name);
  if (!(o = pa_context_set_default_source(ctx->context, s->pulse_name, NULL, NULL))) {
      ERR("pa_context_set_default_source() failed");
      return;
  }
  pa_operation_unref(o);
}

static Eina_Bool
_sink_change_support(void)
{
   return EINA_TRUE;
}

static int
_max_volume(void)
{
   return 150;
}

static const Eina_List *
_cards_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return ctx->cards;
}

static Eina_Bool
_card_profile_set(Emix_Card *card, const Emix_Profile *profile)
{
   pa_operation* o;
   Card *c = (Card *)card;
   Profile *p = (Profile *)profile;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ctx && ctx->context &&
                                   (c != NULL) && (p != NULL), EINA_FALSE);

   if (!(o = pa_context_set_card_profile_by_index(ctx->context,
                                                  c->idx, p->base.name, NULL,
                                                  NULL)))
     {
        ERR("pa_context_set_card_profile_by_index() failed");
        return EINA_FALSE;
     }
   pa_operation_unref(o);

   return EINA_TRUE;
}

static void
_sink_mon_read(pa_stream *stream, size_t bytes EINA_UNUSED, void *data)
{
   Sink *s = data;
   size_t rbytes;
   const void *buf = NULL;

   if (pa_stream_peek(stream, &buf, &rbytes) == 0)
     {
        if ((!buf) && (rbytes))
          {
             pa_stream_drop(stream);
             return;
          }
        s->base.mon_num = (unsigned int)((rbytes / sizeof(float)) / 2);
        s->base.mon_buf = buf;
        if (ctx->cb)
          ctx->cb((void *)ctx->userdata, EMIX_SINK_MONITOR_EVENT, s);
        pa_stream_drop(stream);
     }
}

static void
_sink_monitor_begin(Sink *s)
{
   pa_sample_spec samp;
   pa_buffer_attr attr;

   if (pa_context_get_server_protocol_version(ctx->context) < 13) return;

   pa_sample_spec_init(&samp);
   samp.format = PA_SAMPLE_FLOAT32;
   samp.rate = 44100;
   samp.channels = 2;
   memset(&attr, 0, sizeof(attr));
   attr.fragsize = sizeof(float) * 4096;
   attr.maxlength = (uint32_t) -1;

   s->mon_stream = pa_stream_new(ctx->context, "__e_mon", &samp, NULL);
   pa_stream_set_read_callback(s->mon_stream, _sink_mon_read, s);
   pa_context_set_source_mute_by_name(ctx->context, s->monitor_source_name, 0, NULL, NULL);
   pa_stream_connect_record(s->mon_stream, s->monitor_source_name,
                            &attr,
                            PA_STREAM_NOFLAGS
                            | PA_STREAM_DONT_MOVE
                            | PA_STREAM_ADJUST_LATENCY
                            | PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND
                           );
}

static void
_sink_monitor_end(Sink *s)
{
   if (s->mon_stream)
     {
        if (s->mon_stream) pa_stream_disconnect(s->mon_stream);
        s->mon_stream = NULL;
     }
}

static void
_sink_monitor_set(Emix_Sink *sink, Eina_Bool monitor)
{
   Sink *s = (Sink *)sink;
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   if (monitor) s->mon_count++;
   else s->mon_count--;
   if (s->mon_count < 0) s->mon_count = 0;
   if (s->mon_count == 1)
     {
        if (s->running) _sink_monitor_begin(s);
     }
   else if (s->mon_count == 0) _sink_monitor_end(s);
}

static void
_sink_input_mon_read(pa_stream *stream, size_t bytes EINA_UNUSED, void *data)
{
   Sink_Input *i = data;
   size_t rbytes;
   const void *buf = NULL;

   if (pa_stream_peek(stream, &buf, &rbytes) == 0)
     {
        if ((!buf) && (rbytes))
          {
             pa_stream_drop(stream);
             return;
          }
        i->base.mon_num = (unsigned int)((rbytes / sizeof(float)) / 2);
        i->base.mon_buf = buf;
        if (ctx->cb)
          ctx->cb((void *)ctx->userdata, EMIX_SINK_INPUT_MONITOR_EVENT, i);
        pa_stream_drop(stream);
     }
}

static void
_sink_input_monitor_begin(Sink_Input *i)
{
   pa_sample_spec samp;
   pa_buffer_attr attr;
   char buf[16];
   Eina_List *l;
   Sink *s;
   unsigned int mon_idx = 0;

   if (pa_context_get_server_protocol_version(ctx->context) < 13) return;

   pa_sample_spec_init(&samp);
   samp.format = PA_SAMPLE_FLOAT32;
   samp.rate = 44100;
   samp.channels = 2;
   memset(&attr, 0, sizeof(attr));
   attr.fragsize = sizeof(float) * 4096;
   attr.maxlength = (uint32_t) -1;

   i->mon_stream = pa_stream_new(ctx->context, "__e_mon", &samp, NULL);
   pa_stream_set_monitor_stream(i->mon_stream, i->idx);
   pa_stream_set_read_callback(i->mon_stream, _sink_input_mon_read, i);
   EINA_LIST_FOREACH(ctx->sinks, l, s)
     {
        if (i->sink_idx == s->idx)
          {
             mon_idx = s->monitor_idx;
             break;
          }
     }
   if (!l) return;
   snprintf(buf, sizeof(buf), "%i", mon_idx);
   pa_context_set_source_mute_by_name(ctx->context, buf, 0, NULL, NULL);
   pa_stream_connect_record(i->mon_stream, buf,
                            &attr,
                            PA_STREAM_NOFLAGS
                            | PA_STREAM_DONT_MOVE
                            | PA_STREAM_ADJUST_LATENCY
                            | PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND
                           );
}

static void
_sink_input_monitor_end(Sink_Input *i)
{
   if (i->mon_stream)
     {
        if (i->mon_stream) pa_stream_disconnect(i->mon_stream);
        i->mon_stream = NULL;
     }
}

static void
_sink_input_monitor_set(Emix_Sink_Input *input, Eina_Bool monitor)
{
   Sink_Input *i = (Sink_Input *)input;
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   if (monitor) i->mon_count++;
   else i->mon_count--;
   if (i->mon_count < 0) i->mon_count = 0;
   if (i->mon_count == 1)
     {
        if (i->running) _sink_input_monitor_begin(i);
     }
   else if (i->mon_count == 0) _sink_input_monitor_end(i);
}

static void
_source_mon_read(pa_stream *stream, size_t bytes EINA_UNUSED, void *data)
{
   Source *s = data;
   size_t rbytes;
   const void *buf = NULL;

   if (pa_stream_peek(stream, &buf, &rbytes) == 0)
     {
        if ((!buf) && (rbytes))
          {
             pa_stream_drop(stream);
             return;
          }
        s->base.mon_num = (unsigned int)((rbytes / sizeof(float)) / 2);
        s->base.mon_buf = buf;
        if (ctx->cb)
          ctx->cb((void *)ctx->userdata, EMIX_SOURCE_MONITOR_EVENT, s);
        pa_stream_drop(stream);
     }
}

static void
_source_monitor_begin(Source *s)
{
   pa_sample_spec samp;
   pa_buffer_attr attr;
   char buf[16];

   if (pa_context_get_server_protocol_version(ctx->context) < 13) return;

   pa_sample_spec_init(&samp);
   samp.format = PA_SAMPLE_FLOAT32;
   samp.rate = 44100;
   samp.channels = 2;
   memset(&attr, 0, sizeof(attr));
   attr.fragsize = sizeof(float) * 4096;
   attr.maxlength = (uint32_t) -1;

   s->mon_stream = pa_stream_new(ctx->context, "__e_mon", &samp, NULL);
   pa_stream_set_read_callback(s->mon_stream, _source_mon_read, s);
   snprintf(buf, sizeof(buf), "%i", s->idx);
   pa_stream_connect_record(s->mon_stream, buf,
                            &attr,
                            PA_STREAM_NOFLAGS
                            | PA_STREAM_DONT_MOVE
                            | PA_STREAM_ADJUST_LATENCY
                           );
}

static void
_source_monitor_end(Source *s)
{
   if (s->mon_stream)
     {
        if (s->mon_stream) pa_stream_disconnect(s->mon_stream);
        s->mon_stream = NULL;
     }
}

static void
_source_monitor_set(Emix_Source *source, Eina_Bool monitor)
{
   Source *s = (Source *)source;
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   if (monitor) s->mon_count++;
   else s->mon_count--;
   if (s->mon_count < 0) s->mon_count = 0;
   if (s->mon_count == 1) _source_monitor_begin(s);
   else if (s->mon_count == 0) _source_monitor_end(s);
}

static Emix_Backend
_pulseaudio_backend =
{
   _init,
   _shutdown,
   _max_volume,
   _sinks_get,
   _sink_default_support,
   _sink_default_get,
   _sink_default_set,
   _sink_mute_set,
   _sink_volume_set,
   _sink_port_set,
   _sink_change_support,
   _sink_inputs_get,
   _sink_input_mute_set,
   _sink_input_volume_set,
   _sink_input_move,
   _sources_get,
   _source_default_support,
   _source_default_get,
   _source_default_set,
   _source_mute_set,
   _source_volume_set,
   NULL,
   _cards_get,
   _card_profile_set,
   _sink_monitor_set,
   _sink_input_monitor_set,
   _source_monitor_set
};

E_API Emix_Backend *
emix_backend_pulse_get(void)
{
   return &_pulseaudio_backend;
}

E_API const char *emix_backend_pulse_name = "PULSEAUDIO";
