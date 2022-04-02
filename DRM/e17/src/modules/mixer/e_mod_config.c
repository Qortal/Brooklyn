#include "e.h"
#include "e_mod_config.h"
#include "e_mod_main.h"
#include "emix.h"

typedef struct _Emix_Config
{
   const char *backend;
   int notify;
   int mute;

   int save;
   const char *save_sink;

   Eina_List *sinks;
   Eina_List *sources;

   emix_config_backend_changed cb;
   const void *userdata;
} Emix_Config;

typedef struct _Emix_Config_Sink
{
   const char *name;
   Eina_List *ports;
   int mute;
   int volume;
   int default_sink;
} Emix_Config_Sink;

typedef struct _Emix_Config_Source
{
   const char *name;
   int mute;
   int volume;
   int default_source;
} Emix_Config_Source;

typedef struct _Emix_Config_Port
{
   const char *name;
   int active;
} Emix_Config_Port;


struct _E_Config_Dialog_Data
{
   Emix_Config config;
   Evas_Object *list;
   Evas_Object *sources;
};

static E_Config_DD *cd, *c_portd, *c_sinkd, *c_sourced;
static Emix_Config *_config = NULL;

static void
_emix_config_dd_new(void)
{
   c_portd = E_CONFIG_DD_NEW("Emix_Config_Port", Emix_Config_Port);
   E_CONFIG_VAL(c_portd, Emix_Config_Port, name, STR);
   E_CONFIG_VAL(c_portd, Emix_Config_Port, active, INT);

   c_sinkd = E_CONFIG_DD_NEW("Emix_Config_Sink", Emix_Config_Sink);
   E_CONFIG_VAL(c_sinkd, Emix_Config_Sink, name, STR);
   E_CONFIG_LIST(c_sinkd, Emix_Config_Sink, ports, c_portd);
   E_CONFIG_VAL(c_sinkd, Emix_Config_Sink, mute, INT);
   E_CONFIG_VAL(c_sinkd, Emix_Config_Sink, volume, INT);
   E_CONFIG_VAL(c_sinkd, Emix_Config_Sink, default_sink, INT);

   c_sourced = E_CONFIG_DD_NEW("Emix_Config_Source", Emix_Config_Source);
   E_CONFIG_VAL(c_sourced, Emix_Config_Source, name, STR);
   E_CONFIG_VAL(c_sourced, Emix_Config_Source, mute, INT);
   E_CONFIG_VAL(c_sourced, Emix_Config_Source, volume, INT);
   E_CONFIG_VAL(c_sourced, Emix_Config_Source, default_source, INT);

   cd = E_CONFIG_DD_NEW("Emix_Config", Emix_Config);

   E_CONFIG_VAL(cd, Emix_Config, backend, STR);
   E_CONFIG_VAL(cd, Emix_Config, notify, INT);
   E_CONFIG_VAL(cd, Emix_Config, mute, INT);

   E_CONFIG_VAL(cd, Emix_Config, save, INT);
   E_CONFIG_VAL(cd, Emix_Config, save_sink, STR);

   E_CONFIG_LIST(cd, Emix_Config, sinks, c_sinkd);
   E_CONFIG_LIST(cd, Emix_Config, sources, c_sourced);
}

const char *
emix_config_backend_get(void)
{
   return _config->backend;
}

void
emix_config_backend_set(const char *backend)
{
   eina_stringshare_replace(&_config->backend, backend);
   e_config_domain_save("module.emix", cd, _config);
}

Eina_Bool
emix_config_notify_get(void)
{
   return _config->notify;
}

Eina_Bool
emix_config_desklock_mute_get(void)
{
   return _config->mute;
}

static void
_config_set(Emix_Config *config)
{
   if ((config->backend) && (_config->backend != config->backend))
     eina_stringshare_replace(&_config->backend, config->backend);

   _config->notify = config->notify;
   _config->mute = config->mute;

   if (config->save == 0) _config->save = -1;
   else if (config->save == 1) _config->save = 1;

   DBG("SAVING CONFIG %s %d %d", _config->backend, config->notify,
       config->mute);
   e_config_domain_save("module.emix", cd, config);
}

void
emix_config_init(emix_config_backend_changed cb, const void *userdata)
{
   const Eina_List *backends, *l;
   const char *s;

   EINA_SAFETY_ON_FALSE_RETURN(emix_init());
   _emix_config_dd_new();
   _config = e_config_domain_load("module.emix", cd);
   if (!_config)
     {
        _config = E_NEW(Emix_Config, 1);
        backends = emix_backends_available();
        // prefer pulseaudio as a packend if it exists as this is generally
        // more useful, and a superset of ALSA. so if pulse is there, alsa
        // is too - so choosing alsa if pulse is available is wrong (as a
        // default) and leads to brokenness. in the case pulse is not
        // around, alsa will then work
        EINA_LIST_FOREACH(backends, l, s)
          {
             if (!strcmp(s, "PULSEAUDIO"))
               {
                  _config->backend = eina_stringshare_add(s);
                  break;
               }
          }
        if (!_config->backend)
          {
             if (backends)
               _config->backend = eina_stringshare_add(backends->data);
          }
     }

   if (_config->save == 0) _config->save = 1;

   _config->cb = cb;
   _config->userdata = userdata;
   DBG("Config loaded, backend to use: %s", _config->backend);
}

void
emix_config_shutdown(void)
{
   Emix_Config_Sink *sink;
   Emix_Config_Port *port;
   Emix_Config_Source *source;

   E_CONFIG_DD_FREE(cd);
   E_CONFIG_DD_FREE(c_sourced);
   E_CONFIG_DD_FREE(c_sinkd);
   E_CONFIG_DD_FREE(c_portd);
   if (_config->backend) eina_stringshare_del(_config->backend);
   if (_config->save_sink) eina_stringshare_del(_config->save_sink);
   EINA_LIST_FREE(_config->sinks, sink)
     {
        if (sink->name) eina_stringshare_del(sink->name);
        EINA_LIST_FREE(sink->ports, port)
          {
             if (port->name) eina_stringshare_del(port->name);
             free(port);
          }
        free(sink);
     }
   EINA_LIST_FREE(_config->sources, source)
     {
        if (source->name) eina_stringshare_del(source->name);
        free(source);
     }
   free(_config);
   emix_shutdown();
}

void
emix_config_save(void)
{
   if ((_config) && (cd))
     e_config_domain_save("module.emix", cd, _config);
}

Eina_Bool
emix_config_save_get(void)
{
   if (_config->save == 1) return EINA_TRUE;
   return EINA_FALSE;
}

void
emix_config_save_state_get(void)
{
   Emix_Config_Sink *sink;
   Emix_Config_Port *port;
   Emix_Config_Source *source;

   Eina_List *sinks, *sources, *l, *ll;
   Emix_Sink *emsink;
   Emix_Source *emsource;
   Emix_Port *emport;

   EINA_LIST_FREE(_config->sinks, sink)
     {
        if (sink->name) eina_stringshare_del(sink->name);
        EINA_LIST_FREE(sink->ports, port)
          {
             if (port->name) eina_stringshare_del(port->name);
             free(port);
          }
        free(sink);
     }
   EINA_LIST_FREE(_config->sources, source)
     {
        if (source->name) eina_stringshare_del(source->name);
        free(source);
     }

   sinks = (Eina_List *)emix_sinks_get();
   EINA_LIST_FOREACH(sinks, l, emsink)
     {
        if (!emsink->name) continue;
        sink = calloc(1, sizeof(Emix_Config_Sink));
        if (sink)
          {
             sink->name = eina_stringshare_add(emsink->name);
             EINA_LIST_FOREACH(emsink->ports, ll, emport)
               {
                  if (!emport->name) continue;
                  port = calloc(1, sizeof(Emix_Config_Port));
                  if (port)
                    {
                       port->name = eina_stringshare_add(emport->name);
                       port->active = emport->active;
                       sink->ports = eina_list_append(sink->ports, port);
                    }
               }
             if (emsink->volume.channel_count > 0)
               sink->volume = emsink->volume.volumes[0];
             sink->mute = emsink->mute;
             sink->default_sink = emsink->default_sink;
             _config->sinks = eina_list_append(_config->sinks, sink);
          }
     }
   sources = (Eina_List *)emix_sources_get();
   EINA_LIST_FOREACH(sources, ll, emsource)
     {
        if (!emsource->name) continue;
        source = calloc(1, sizeof(Emix_Config_Source));
        if (source)
          {
             source->name = eina_stringshare_add(emsource->name);
             if (emsource->volume.channel_count > 0)
               source->volume = emsource->volume.volumes[0];
             source->mute = emsource->mute;
             source->default_source = emsource->default_source;
             _config->sources = eina_list_append(_config->sources, source);
          }
     }
}

static Emix_Sink *
_find_sink(const char *name)
{
   Eina_List *sinks, *l;
   Emix_Sink *emsink;

   sinks = (Eina_List *)emix_sinks_get();
   EINA_LIST_FOREACH(sinks, l, emsink)
     {
        if (!emsink->name) continue;
        if (!strcmp(emsink->name, name)) return emsink;
     }
   return NULL;
}

static Emix_Port *
_find_port(Emix_Sink *sink, const char *name)
{
   Eina_List *l;
   Emix_Port *emport;

   EINA_LIST_FOREACH(sink->ports, l, emport)
     {
        if (!emport->name) continue;
        if (!strcmp(emport->name, name)) return emport;
     }
   return NULL;
}

static Emix_Source *
_find_source(const char *name)
{
   Eina_List *sources, *l;
   Emix_Source *emsource;

   sources = (Eina_List *)emix_sources_get();
   EINA_LIST_FOREACH(sources, l, emsource)
     {
        if (!emsource->name) continue;
        if (!strcmp(emsource->name, name)) return emsource;
     }
   return NULL;
}

void
emix_config_save_state_restore(void)
{
   Emix_Config_Sink *sink;
   Emix_Config_Port *port;
   Emix_Config_Source *source;
   Emix_Sink *emsink;
   Emix_Source *emsource;
   Emix_Port *emport;
   Eina_List *l, *ll;
   Emix_Volume v;
   unsigned int i;

   EINA_LIST_FOREACH(_config->sinks, l, sink)
     {
        if (!sink->name) continue;
        emsink = _find_sink(sink->name);
        if (!emsink) continue;
        v.volumes = calloc(emsink->volume.channel_count, sizeof(int));
        if (v.volumes)
          {
             v.channel_count = emsink->volume.channel_count;
             for (i = 0; i < v.channel_count; i++)
               v.volumes[i] = sink->volume;
             emix_sink_volume_set(emsink, &v);
             free(v.volumes);
          }
        emix_sink_mute_set(emsink, sink->mute);
        if (sink->default_sink)
          emix_sink_default_set(emsink);
        EINA_LIST_FOREACH(sink->ports, ll, port)
          {
             if (!port->name) continue;
             if (port->active)
               {
                  emport = _find_port(emsink, port->name);
                  if (emport) emix_sink_port_set(emsink, emport);
                  break;
               }
          }
     }
   EINA_LIST_FOREACH(_config->sources, l, source)
     {
        if (!source->name) continue;
        emsource = _find_source(source->name);
        if (!emsource) continue;
        v.volumes = calloc(emsource->volume.channel_count, sizeof(int));
        if (v.volumes)
          {
             v.channel_count = emsource->volume.channel_count;
             for (i = 0; i < v.channel_count; i++)
               v.volumes[i] = source->volume;
             emix_source_volume_set(emsource, &v);
             free(v.volumes);
          }
        emix_source_mute_set(emsource, source->mute);
        if (source->default_source)
          emix_source_default_set(emsource);
     }
}

const char *
emix_config_save_sink_get(void)
{
   return _config->save_sink;
}

void
emix_config_save_sink_set(const char *sink)
{
   eina_stringshare_replace(&(_config->save_sink), sink);
   if (_config->save == 1) e_config_save_queue();
}

static void*
_create_data(E_Config_Dialog *cfg EINA_UNUSED)
{
   E_Config_Dialog_Data *d;

   d = E_NEW(E_Config_Dialog_Data, 1);
   d->config.backend = eina_stringshare_add(_config->backend);
   d->config.notify = _config->notify;
   d->config.mute = _config->mute;

   if (_config->save == -1) d->config.save = 0;
   else d->config.save = 1;

   return d;
}

static void
_free_data(E_Config_Dialog *c EINA_UNUSED, E_Config_Dialog_Data *cf)
{
   eina_stringshare_del(cf->config.backend);
   free(cf);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas,
                      E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *l;
   const Eina_List *node;
   char *name;
   int i = 0;

   o = e_widget_list_add(evas, 0, 0);

   l = e_widget_check_add(evas, _("Notify on volume change"), &cfdata->config.notify);
   e_widget_list_object_append(o, l, 0, 0, 0);

   l = e_widget_check_add(evas, _("Mute on lock"), &cfdata->config.mute);
   e_widget_list_object_append(o, l, 0, 0, 0);

   l = e_widget_check_add(evas, _("Remember"), &cfdata->config.save);
   e_widget_list_object_append(o, l, 0, 0, 0);

   l = e_widget_label_add(evas, _("Backend to use:"));
   e_widget_list_object_append(o, l, 0, 0, 0);

   cfdata->list = l = e_widget_ilist_add(evas, 0, 0, NULL);
   e_widget_ilist_multi_select_set(l, EINA_FALSE);
   e_widget_size_min_set(l, 100, 100);
   EINA_LIST_FOREACH(emix_backends_available(), node, name)
     {
        e_widget_ilist_append(l, NULL, name, NULL, NULL, NULL);
        i ++;
        if (_config->backend && !strcmp(_config->backend, name))
          e_widget_ilist_selected_set(l, i);
     }
   e_widget_ilist_go(l);
   e_widget_ilist_thaw(l);
   e_widget_list_object_append(o, l, 1, 1, 0);

   return o;
}

static int
_basic_apply_data(E_Config_Dialog *cfd EINA_UNUSED,
                  E_Config_Dialog_Data *cfdata)
{
   char *new_backend = eina_list_nth(
                            emix_backends_available(),
                            e_widget_ilist_selected_get(cfdata->list));

   eina_stringshare_replace(&cfdata->config.backend, new_backend);

   _config_set(&cfdata->config);
   if (_config->cb)
      _config->cb(new_backend, (void *)_config->userdata);
   return 1;
}

E_Config_Dialog*
emix_config_popup_new(Evas_Object *comp, const char *p EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "windows/emix"))
      return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;

   cfd = e_config_dialog_new(comp,
                             _("Emix Configuration"),
                             "E", "windows/emix",
                             NULL,
                             0, v, NULL);
   return cfd;
}
