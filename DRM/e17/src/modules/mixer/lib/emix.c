#include <Ecore.h>
#include <Eina.h>

#include "config.h"
#include "emix.h"

#ifdef HAVE_PULSE
E_API Emix_Backend      *emix_backend_pulse_get(void);
E_API extern const char *emix_backend_pulse_name;
#endif
#ifdef HAVE_ALSA
E_API Emix_Backend      *emix_backend_alsa_get(void);
E_API extern const char *emix_backend_alsa_name;
#endif

static int _log_domain;

#define CRIT(...)     EINA_LOG_DOM_CRIT(_log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)

struct Callback_Data
{
   Emix_Event_Cb cb;
   const void *data;
};

typedef struct Context
{
   /* Valid backends *.so */
   Eina_Array *backends;
   Eina_List *backends_names;
   Eina_List *callbacks;
   Eina_List *configs;

   Emix_Backend *loaded;
} Context;

typedef struct _Back
{
   Emix_Backend *(*backend_get) (void);
   const char     *backend_name;
} Back;

static int _init_count = 0;
static Context *ctx = NULL;

static void
_events_cb(void *data EINA_UNUSED, enum Emix_Event event, void *event_info)
{
   Eina_List *l;
   struct Callback_Data *callback;

   EINA_LIST_FOREACH(ctx->callbacks, l, callback)
      callback->cb((void *)callback->data, event, event_info);
}

Eina_Bool
emix_init(void)
{
   Back *back;

   if (_init_count > 0)
      goto end;

   if (!eina_init())
     {
        fprintf(stderr, "Could not init eina\n");
        return EINA_FALSE;
     }

   _log_domain = eina_log_domain_register("emix", NULL);
   if (_log_domain < 0)
     {
        EINA_LOG_CRIT("Could not create log domain 'emix'");
        goto err_log;
     }

   if (!ecore_init())
     {
        CRIT("Could not init ecore");
        goto err_ecore;
     }

   ctx = calloc(1, sizeof(Context));
   if (!ctx)
     {
        ERR("Could not create Epulse Context");
        goto err_ecore;
     }
   ctx->backends = eina_array_new(2);
#ifdef HAVE_PULSE
   back = calloc(1, sizeof(Back));
   if (back)
     {
        back->backend_get = emix_backend_pulse_get;
        back->backend_name = emix_backend_pulse_name;
        eina_array_push(ctx->backends, back);
        ctx->backends_names = eina_list_append(ctx->backends_names,
                                               back->backend_name);
     }
#endif
#ifdef HAVE_ALSA
   back = calloc(1, sizeof(Back));
   if (back)
     {
        back->backend_get = emix_backend_alsa_get;
        back->backend_name = emix_backend_alsa_name;
        eina_array_push(ctx->backends, back);
        ctx->backends_names = eina_list_append(ctx->backends_names,
                                               back->backend_name);
     }
#endif

   if (ctx->backends == NULL)
     {
        ERR("Could not find any valid backend");
        goto err;
     }

 end:
   _init_count++;
   return EINA_TRUE;
 err:
   free(ctx);
   ctx = NULL;
 err_ecore:
   eina_log_domain_unregister(_log_domain);
   _log_domain = -1;
 err_log:
   eina_shutdown();
   return EINA_FALSE;
}

void
emix_shutdown()
{
   unsigned int i;
   Eina_Array_Iterator iterator;
   Back *back;

   if (_init_count == 0)
      return;

   _init_count--;
   if (_init_count > 0)
      return;

   if (ctx->loaded && ctx->loaded->ebackend_shutdown)
      ctx->loaded->ebackend_shutdown();

   eina_list_free(ctx->backends_names);
   EINA_ARRAY_ITER_NEXT(ctx->backends, i, back, iterator)
     {
        free(back);
     }
   eina_array_free(ctx->backends);
   free(ctx);
   ctx = NULL;

   ecore_shutdown();
   eina_shutdown();
}

const Eina_List*
emix_backends_available(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return ctx->backends_names;
}

const Eina_List *
emix_configs_available(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return ctx->configs;
}

int
emix_max_volume_get(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_max_volume_get),
                                   0);
  return ctx->loaded->ebackend_max_volume_get();
}

Eina_Bool
emix_backend_set(const char *backend)
{
   Eina_List *l;
   const char *name;
   unsigned int i = 0;
   Back *back;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && backend), EINA_FALSE);
   if (ctx->loaded && ctx->loaded->ebackend_shutdown)
     {
        ctx->loaded->ebackend_shutdown();
        ctx->loaded = NULL;
     }

   EINA_LIST_FOREACH(ctx->backends_names, l, name)
     {
        if (!strcmp(name, backend)) break;
        i++;
     }

   if (i == eina_list_count(ctx->backends_names))
     {
        CRIT("Requested backend not found (%s)", backend);
        return EINA_FALSE;
     }

   back = eina_array_data_get(ctx->backends, i);
   ctx->loaded = back->backend_get();

   if (!ctx->loaded || !ctx->loaded->ebackend_init)
      return EINA_FALSE;

   return ctx->loaded->ebackend_init(_events_cb, NULL);
}

const Eina_List*
emix_sinks_get(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_sinks_get),
                                    NULL);
   return ctx->loaded->ebackend_sinks_get();
}

Eina_Bool
emix_sink_default_support(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_sink_default_support),
                                   EINA_FALSE);
   return ctx->loaded->ebackend_sink_default_support();
}

const Emix_Sink*
emix_sink_default_get(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_sink_default_get),
                                   NULL);
   return ctx->loaded->ebackend_sink_default_get();
}

void
emix_sink_default_set(Emix_Sink *sink)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_default_set &&
                                sink));

   ctx->loaded->ebackend_sink_default_set(sink);
}

Eina_Bool
emix_sink_port_set(Emix_Sink *sink, Emix_Port *port)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_sink_port_set &&
                                    sink && port), EINA_FALSE);

   return ctx->loaded->ebackend_sink_port_set(sink, port);
}

void
emix_sink_mute_set(Emix_Sink *sink, Eina_Bool mute)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_mute_set &&
                                sink));

   ctx->loaded->ebackend_sink_mute_set(sink, mute);
}

void
emix_sink_volume_set(Emix_Sink *sink, Emix_Volume *volume)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_volume_set &&
                                sink));

   ctx->loaded->ebackend_sink_volume_set(sink, volume);
}

Eina_Bool
emix_sink_change_support(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_sink_change_support),
                                    EINA_FALSE);
   return ctx->loaded->ebackend_sink_change_support();
}

const Eina_List*
emix_sink_inputs_get(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_sink_inputs_get),
                                    NULL);
   return ctx->loaded->ebackend_sink_inputs_get();
}

void
emix_sink_input_mute_set(Emix_Sink_Input *input, Eina_Bool mute)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_input_mute_set &&
                                input));

   ctx->loaded->ebackend_sink_input_mute_set(input, mute);
}

void
emix_sink_input_volume_set(Emix_Sink_Input *input, Emix_Volume *volume)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_input_volume_set &&
                                input));

   ctx->loaded->ebackend_sink_input_volume_set(input, volume);
}

void
emix_sink_input_sink_change(Emix_Sink_Input *input, Emix_Sink *sink)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_input_sink_change &&
                                input && sink));

   ctx->loaded->ebackend_sink_input_sink_change(input, sink);
}

const Eina_List*
emix_sources_get(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_sources_get), NULL);

   return ctx->loaded->ebackend_sources_get();
}

Eina_Bool
emix_source_default_support(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_source_default_support),
                                   EINA_FALSE);
   return ctx->loaded->ebackend_source_default_support();
}

const Emix_Source*
emix_source_default_get(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_source_default_get),
                                   NULL);
   return ctx->loaded->ebackend_source_default_get();
}

void
emix_source_default_set(Emix_Source *source)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_source_default_set &&
                                source));

   ctx->loaded->ebackend_source_default_set(source);
}

void
emix_source_mute_set(Emix_Source *source, Eina_Bool mute)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_source_mute_set &&
                                source));

   ctx->loaded->ebackend_source_mute_set(source, mute);
}

void
emix_source_volume_set(Emix_Source *source, Emix_Volume *volume)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_source_volume_set &&
                                source));

   ctx->loaded->ebackend_source_volume_set(source, volume);
}

Evas_Object *
emix_advanced_options_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded && parent &&
                             ctx->loaded->ebackend_advanced_options_add), NULL);

   return ctx->loaded->ebackend_advanced_options_add(parent);
}

Eina_Bool
emix_event_callback_add(Emix_Event_Cb cb, const void *data)
{
   struct Callback_Data *callback;
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && cb), EINA_FALSE);

   callback = calloc(1, sizeof(*callback));
   callback->cb = cb;
   callback->data = data;

   ctx->callbacks = eina_list_append(ctx->callbacks, callback);
   return EINA_TRUE;
}

Eina_Bool
emix_event_callback_del(Emix_Event_Cb cb, const void *data)
{
   struct Callback_Data *callback;
   Eina_List *l;
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && cb), EINA_FALSE);

   EINA_LIST_FOREACH(ctx->callbacks, l, callback)
     {
        if ((callback->cb == cb) && (callback->data == data))
          {
             ctx->callbacks = eina_list_remove_list(ctx->callbacks, l);
             free(callback);
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

const Eina_List*
emix_cards_get(void)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_cards_get), NULL);

   return ctx->loaded->ebackend_cards_get();
}

Eina_Bool
emix_card_profile_set(Emix_Card *card, Emix_Profile *profile)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL((ctx && ctx->loaded &&
                                    ctx->loaded->ebackend_card_profile_set &&
                                    card && profile), EINA_FALSE);

   return ctx->loaded->ebackend_card_profile_set(card, profile);
}

void
emix_sink_monitor(Emix_Sink *sink, Eina_Bool monitor)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_mute_set &&
                                sink));
   if (!ctx->loaded->ebackend_sink_monitor_set) return;
   ctx->loaded->ebackend_sink_monitor_set(sink, monitor);
}

void
emix_sink_input_monitor(Emix_Sink_Input *input, Eina_Bool monitor)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_mute_set &&
                                input));

   if (!ctx->loaded->ebackend_sink_input_monitor_set) return;
   ctx->loaded->ebackend_sink_input_monitor_set(input, monitor);
}

void
emix_source_monitor(Emix_Source *source, Eina_Bool monitor)
{
   EINA_SAFETY_ON_FALSE_RETURN((ctx && ctx->loaded &&
                                ctx->loaded->ebackend_sink_mute_set &&
                                source));

   if (!ctx->loaded->ebackend_source_monitor_set) return;
   ctx->loaded->ebackend_source_monitor_set(source, monitor);
}


