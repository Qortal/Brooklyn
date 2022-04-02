#include "emix.h"
#include <alsa/asoundlib.h>

#define MAX_VOLUME 100

#define ERR(...)      EINA_LOG_ERR(__VA_ARGS__)
#define DBG(...)      EINA_LOG_DBG(__VA_ARGS__)
#define WRN(...)      EINA_LOG_WARN(__VA_ARGS__)

typedef struct _Context
{
   Emix_Event_Cb cb;
   const void *userdata;
   Eina_List *sinks;
   Eina_List *sources;
   Eina_List *cards;
} Context;

static Context *ctx = NULL;

typedef struct _Alsa_Emix_Sink
{
  Emix_Sink sink;
  const char *hw_name;
  Eina_List *channels;
} Alsa_Emix_Sink;

typedef struct _Alsa_Emix_Source
{
  Emix_Source source;
  const char *hw_name;
  Eina_List *channels;
} Alsa_Emix_Source;
/*
 * TODO problems:
 *
 * - mono stereo problem...
 */

/*
 * util functions
 */

static int
_alsa_mixer_sink_changed_cb(snd_mixer_t *ctl, unsigned int mask EINA_UNUSED,
                            snd_mixer_elem_t *elem EINA_UNUSED)
{
   Alsa_Emix_Sink *sink = snd_mixer_get_callback_private(ctl);

   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SINK_CHANGED_EVENT,
             (Emix_Sink *)sink);
   return 0;
}

static int
_alsa_mixer_source_changed_cb(snd_mixer_t *ctl, unsigned int mask EINA_UNUSED,
                              snd_mixer_elem_t *elem EINA_UNUSED)
{
   Alsa_Emix_Source *source = snd_mixer_get_callback_private(ctl);

   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SOURCE_CHANGED_EVENT,
             (Emix_Source *)source);
   return 0;
}

static void
_alsa_channel_volume_get(snd_mixer_elem_t *channel, int *v, Eina_Bool capture)
{
   long int min = 0, max = 100, vol = 0;
   int range, divide;

   if (capture)
     snd_mixer_selem_get_capture_volume_range(channel, &min, &max);
   else
     snd_mixer_selem_get_playback_volume_range(channel, &min, &max);

   divide = MAX_VOLUME + min;
   if (divide == 0)
     {
        divide = 1;
        min++;
     }

   range = max - min;
   if (range < 1)
     return;

   if (capture)
     snd_mixer_selem_get_capture_volume(channel, 0, &vol);
   else
     snd_mixer_selem_get_playback_volume(channel, 0, &vol);

   *v = ((vol + min) * divide) / range;
}

static void
_alsa_channel_volume_set(snd_mixer_elem_t *channel, int v, Eina_Bool capture)
{
   long int vol, min, max, divide, range;
   snd_mixer_selem_get_playback_volume_range(channel, &min, &max);

   divide = MAX_VOLUME + min;
   range = max - min;
   if (range < 1)
     return;

   vol = ((v * range) / divide) - min;
   if (!capture)
     snd_mixer_selem_set_playback_volume_all(channel, vol);
   else
     snd_mixer_selem_set_capture_volume_all(channel, vol);
}

/*
 * This will append a new device to the cards and call the ecore event for
 * a new device!
 */
static snd_mixer_t *
_alsa_card_create(char *addr)
{
   snd_mixer_t *control;

   if (snd_mixer_open(&control, 0) < 0)
     goto error_open;
   if (snd_mixer_attach(control, addr) < 0)
     goto error_load;
   if (snd_mixer_selem_register(control, NULL, NULL) < 0)
     goto error_load;
   if (snd_mixer_load(control))
     goto error_load;

   return control;

error_load:
   snd_mixer_close(control);
error_open:
   return NULL;
}

static void
_alsa_volume_create(Emix_Volume *volume, Eina_List *channels)
{
   unsigned int i = 0, count = eina_list_count(channels);
   Eina_List *l;
   snd_mixer_elem_t *elem;

   volume->channel_count = count;
   volume->volumes = calloc(count, sizeof(int));

   EINA_LIST_FOREACH(channels, l, elem)
     {
        _alsa_channel_volume_get(elem, &(volume->volumes[i]), EINA_FALSE);
        i++;
     }
}

static void
_alsa_sink_mute_get(Alsa_Emix_Sink *as)
{
   int i = 0;
   snd_mixer_elem_t *elem;

   elem = eina_list_data_get(as->channels);
   snd_mixer_selem_get_playback_switch(elem, 0, &i);
   as->sink.mute = !i;
}

static void
_alsa_sources_mute_get(Alsa_Emix_Source *as)
{
   int i = 0;
   snd_mixer_elem_t *elem;

   elem = eina_list_data_get(as->channels);
   snd_mixer_selem_get_capture_switch(elem, 0, &i);
   as->source.mute = !i;
}

static Alsa_Emix_Sink*
_alsa_device_sink_create(const char *name, const char* hw_name,
                         Eina_List *channels)
{
   Alsa_Emix_Sink *sink;

   if (!(sink = calloc(1, sizeof(Alsa_Emix_Sink))))
     {
        ERR("Allocation Failed");
        return NULL;
     }
   sink->sink.name = eina_stringshare_add(name);
   sink->hw_name = eina_stringshare_add(hw_name);
   sink->channels = channels;
   _alsa_volume_create(&sink->sink.volume, channels);
   _alsa_sink_mute_get(sink);
   if (ctx->cb)
     {
        ctx->cb((void *)ctx->userdata, EMIX_SINK_ADDED_EVENT,
                (Emix_Sink *)sink);

     }
   ctx->sinks = eina_list_append(ctx->sinks, sink);
   return sink;
}

static Alsa_Emix_Source*
_alsa_device_source_create(const char *name, const char* hw_name,
                           Eina_List *channels)
{
   Alsa_Emix_Source *source;

   if (!(source = calloc(1, sizeof(Alsa_Emix_Source))))
     {
        ERR("Allocation Failed");
        return NULL;
     }
   source->source.name = eina_stringshare_add(name);
   source->hw_name = eina_stringshare_add(hw_name);
   source->channels = channels;
   _alsa_volume_create(&source->source.volume, channels);
   _alsa_sources_mute_get(source);
   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SOURCE_ADDED_EVENT,
                  (Emix_Sink *)source);
   ctx->sources = eina_list_append(ctx->sources, source);
   return source;
}

static void
_alsa_device_sink_free(Alsa_Emix_Sink *sink)
{
   eina_stringshare_del(sink->hw_name);
   eina_stringshare_del(sink->sink.name);
   free(sink->sink.volume.volumes);
   free(sink);
}

static void
_alsa_device_source_free(Alsa_Emix_Source *source)
{
   eina_stringshare_del(source->hw_name);
   eina_stringshare_del(source->source.name);
   free(source->source.volume.volumes);
   free(source);
}

static char*
_alsa_cards_name_get(char *name)
{
   snd_ctl_t *control;
   snd_ctl_card_info_t *hw_info;
   char *result = NULL;

   snd_ctl_card_info_alloca(&hw_info);

   if (snd_ctl_open(&control, name, 0) < 0)
     {
        ERR("Failed to open device");
        goto err;
     }

   if (snd_ctl_card_info(control, hw_info) < 0)
     {
        ERR("Failed to get card information");
        goto err_open;
     }

   result = strdup(snd_ctl_card_info_get_name(hw_info));

err_open:
   snd_ctl_close(control);
err:
   return result;
}

static void
_alsa_cards_refresh(void)
{
   int err, card_num = -1;
   Eina_List *tmp_source = NULL, *tmp_sink = NULL;

   while (((err = snd_card_next(&card_num)) == 0) && (card_num >= 0))
     {
        char buf[PATH_MAX];
        char *device_name;
        snd_mixer_t *mixer;
        snd_mixer_elem_t *elem;
        Alsa_Emix_Source *source;
        Alsa_Emix_Sink *sink;

        source = NULL;
        sink = NULL;
        tmp_source = NULL;
        tmp_sink = NULL;

        //generate card addr
        snprintf(buf, sizeof(buf), "hw:%d", card_num);
        //save the addr to see if there are missing devices in the cache list

        mixer = _alsa_card_create(buf);
        ctx->cards = eina_list_append(ctx->cards, mixer);
        //get elements of the device
        elem = snd_mixer_first_elem(mixer);
        for (; elem; elem = snd_mixer_elem_next(elem))
          {
             if (strncmp(snd_mixer_selem_get_name(elem),
                         "Master", sizeof("Master") - 1))
                continue;

             //check if its a source or a sink
             if (snd_mixer_selem_has_capture_volume(elem))
               tmp_source = eina_list_append(tmp_source, elem);
             else if (snd_mixer_selem_has_playback_volume(elem))
               tmp_sink = eina_list_append(tmp_sink, elem);
          }

        device_name = _alsa_cards_name_get(buf);
        //create the sinks / sources
        if (tmp_sink)
          {
             sink = _alsa_device_sink_create(device_name,
                                             buf,
                                             tmp_sink);
             snd_mixer_set_callback(mixer, _alsa_mixer_sink_changed_cb);
             snd_mixer_set_callback_private(mixer, sink);
          }
        if (tmp_source)
          {
             source = _alsa_device_source_create(device_name,
                                                 buf,
                                                 tmp_source);
             snd_mixer_set_callback(mixer, _alsa_mixer_source_changed_cb);
             snd_mixer_set_callback_private(mixer, source);
          }
        if (device_name)
          free(device_name);
     }
}

static Eina_Bool
_alsa_init(Emix_Event_Cb cb, const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, EINA_FALSE);
   if (!ctx)
      ctx = calloc(1, sizeof(Context));

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);

   ctx->cb = cb;
   ctx->userdata = data;
   _alsa_cards_refresh();

   //call the event because the backend is now ready to use
   ctx->cb((void *)ctx->userdata, EMIX_READY_EVENT, NULL);

   return EINA_TRUE;
}

static void
_alsa_shutdown(void)
{
   Alsa_Emix_Sink *sink;
   Alsa_Emix_Source *source;
   snd_mixer_t *mixer;

   EINA_SAFETY_ON_NULL_RETURN(ctx);

   EINA_LIST_FREE(ctx->sinks, sink)
      _alsa_device_sink_free(sink);
   EINA_LIST_FREE(ctx->sources, source)
      _alsa_device_source_free(source);
   EINA_LIST_FREE(ctx->cards, mixer)
      snd_mixer_close(mixer);

   free(ctx);
   ctx = NULL;
}

static const Eina_List*
_alsa_sources_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return ctx->sources;
}

static void
_alsa_sources_mute_set(Emix_Source *source, Eina_Bool mute)
{
   Alsa_Emix_Source *s = (Alsa_Emix_Source*) source;
   Eina_List *node;
   snd_mixer_elem_t *elem;

   EINA_SAFETY_ON_FALSE_RETURN((ctx && source));

   EINA_LIST_FOREACH(s->channels, node, elem)
     {
       if (!snd_mixer_selem_has_capture_switch(elem))
         continue;
       if (snd_mixer_selem_set_capture_switch_all(elem, !mute) < 0)
         ERR("Failed to mute device\n");
     }

   source->mute = mute;
   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SOURCE_CHANGED_EVENT,
                  (Emix_Source *)source);
}

static void
_alsa_sources_volume_set(Emix_Source *source, Emix_Volume *v)
{
   Alsa_Emix_Source *s = (Alsa_Emix_Source*) source;
   unsigned int i;
   snd_mixer_elem_t *elem;

   EINA_SAFETY_ON_FALSE_RETURN((ctx && source && v));

   if (v->channel_count != eina_list_count(s->channels))
     {
        ERR("Volume struct doesn't have the same length than the channels");
        return;
     }

   for (i = 0; i < v->channel_count; i++ )
     {
        elem = eina_list_nth(s->channels, i);
        _alsa_channel_volume_set(elem, v->volumes[i], EINA_FALSE);
        s->source.volume.volumes[i] = v->volumes[i];
     }
   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SOURCE_CHANGED_EVENT,
                  (Emix_Source *)s);
}


static const Eina_List*
_alsa_sinks_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   //FIXME fork or just return?
/*   Eina_List *result = NULL;
   Eina_List *node;
   void *data;
   EINA_LIST_FOREACH(ctx->sinks, node, data)
     {
        result = eina_list_append(result, data);
     }*/
   return ctx->sinks;
}

static Eina_Bool
_alsa_support(void)
{
  return EINA_FALSE;
}

static void
_alsa_sink_mute_set(Emix_Sink *sink, Eina_Bool mute)
{
   Alsa_Emix_Sink *as = (Alsa_Emix_Sink*) sink;
   Eina_List *node;
   snd_mixer_elem_t *elem;

   EINA_SAFETY_ON_FALSE_RETURN((ctx && sink));

   EINA_LIST_FOREACH(as->channels, node, elem)
     {
       if (!snd_mixer_selem_has_playback_switch(elem))
         continue;

       if (snd_mixer_selem_set_playback_switch_all(elem, !mute) < 0)
         ERR("Failed to set mute(%d) device(%p)", mute, elem);
     }

   sink->mute = mute;
   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SINK_CHANGED_EVENT,
                  (Emix_Sink *)sink);
}

static void
_alsa_sink_volume_set(Emix_Sink *sink, Emix_Volume *v)
{
   Alsa_Emix_Sink *s = (Alsa_Emix_Sink *)sink;
   unsigned int i;
   snd_mixer_elem_t *elem;

   EINA_SAFETY_ON_FALSE_RETURN((ctx && sink && v));

   if (v->channel_count != eina_list_count(s->channels))
     {
        ERR("Volume struct doesn't have the same length than the channels");
        return;
     }

   for (i = 0; i < v->channel_count; i++ )
     {
        elem = eina_list_nth(s->channels, i);
        _alsa_channel_volume_set(elem, v->volumes[i], EINA_FALSE);
        s->sink.volume.volumes[i] = v->volumes[i];
     }
   if (ctx->cb)
     ctx->cb((void *)ctx->userdata, EMIX_SINK_CHANGED_EVENT,
                  (Emix_Sink *)s);
}

static int
_max_volume(void)
{
   return MAX_VOLUME;
}

static Emix_Backend
_alsa_backend =
{
   _alsa_init,
   _alsa_shutdown,
   _max_volume,
   _alsa_sinks_get,
   _alsa_support, /*sink default support*/
   NULL, /*get*/
   NULL, /*set*/
   _alsa_sink_mute_set, /*mute_set*/
   _alsa_sink_volume_set, /*volume_set*/
   NULL, /* port set */
   _alsa_support, /*change support*/
   NULL, /*sink input get*/
   NULL,/*sink input mute set*/
   NULL,/*sink input volume set*/
   NULL,/*sink input sink change*/
   _alsa_sources_get,/*source*/
   _alsa_support, /* source default support*/
   NULL, /*get*/
   NULL, /*set*/
   _alsa_sources_mute_set,/* source mute set */
   _alsa_sources_volume_set, /* source volume set */
   NULL, /* advanced options */
   NULL, /* card list */
   NULL, /* card profile set */
   NULL, /* sink monitor set */
   NULL, /* sink input monitor set */
   NULL /* ssource monitor set */
};

E_API Emix_Backend *
emix_backend_alsa_get(void)
{
   return &_alsa_backend;
}

E_API const char *emix_backend_alsa_name = "ALSA";
