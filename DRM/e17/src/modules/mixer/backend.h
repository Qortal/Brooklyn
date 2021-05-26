#ifndef MIXER_GADGET_BACKEND_
#define MIXER_GADGET_BACKEND_

typedef void (*Backend_Hook)(void);

EINTERN extern int E_EVENT_MIXER_BACKEND_CHANGED;
EINTERN extern int E_EVENT_MIXER_SINKS_CHANGED;

EINTERN int backend_init(void);
EINTERN int backend_shutdown(void);

EINTERN void backend_emixer_exec(void);

EINTERN void backend_volume_set(unsigned int volume);
EINTERN unsigned int backend_volume_get(void);
EINTERN void backend_volume_decrease(void);
EINTERN void backend_volume_increase(void);
EINTERN void backend_mute_set(Eina_Bool mute);
EINTERN Eina_Bool backend_mute_get(void);

EINTERN void backend_sink_default_set(const Emix_Sink *s);
EINTERN const Emix_Sink *backend_sink_default_get(void);

#endif /* MIXER_GADGET_BACKEND */
