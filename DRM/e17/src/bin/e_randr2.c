#include "e.h"

#define E_RANDR_CONFIG_VERSION 3

/////////////////////////////////////////////////////////////////////////
static Eina_Bool               _screen_closed(E_Randr2_Screen *s);
static void                    _animated_apply_abort(void);
static void                    _cb_delay_init_save(void *data);
static Eina_Bool               _cb_delay_timer(void *data);
static Eina_Bool               _cb_fade_animator(void *data);
static void                    _animated_apply(void);
static void                    _do_apply(void);
static void                    _info_free(E_Randr2 *r);
static E_Config_Randr2        *_config_load(void);
static void                    _config_free(E_Config_Randr2 *cfg);
static Eina_Bool               _config_save(E_Randr2 *r, E_Config_Randr2 *cfg);
static Eina_Bool               _config_update(E_Randr2 *r, E_Config_Randr2 *cfg, Eina_Bool );
static void                    _config_apply(E_Randr2 *r, E_Config_Randr2 *cfg);
static int                     _config_screen_match_count(E_Randr2 *r, E_Config_Randr2 *cfg);
static char                   *_screens_fingerprint(E_Randr2 *r);
static Eina_Bool               _screens_differ(E_Randr2 *r1, E_Randr2 *r2);
static Eina_Bool               _cb_screen_change_delay(void *data);
static E_Randr2_Screen        *_screen_output_find(const char *out);
static E_Randr2_Screen        *_screen_id_find(const char *id);
static void                    _screen_config_takeover(void);
static void                    _screen_config_do(E_Randr2_Screen *s);
static void                    _screen_config_eval(void);
static void                    _screen_config_maxsize(void);

/////////////////////////////////////////////////////////////////////////
static E_Config_DD   *_e_randr2_cfg_edd = NULL;
static E_Config_DD   *_e_randr2_cfg_screen_edd = NULL;
static Eina_List     *_ev_handlers = NULL;
static Ecore_Timer   *_screen_delay_timer = NULL;
static Eina_Bool      event_screen = EINA_FALSE;
static Eina_Bool      event_ignore = EINA_FALSE;
static Eina_Bool      initted = EINA_FALSE;
static Eina_Bool      blocked = EINA_FALSE;

/////////////////////////////////////////////////////////////////////////
E_API E_Config_Randr2 *e_randr2_cfg = NULL;
E_API E_Randr2        *e_randr2 = NULL;

E_API int              E_EVENT_RANDR_CHANGE = 0;

static void
_cb_delay_init_save(void *data EINA_UNUSED)
{
   e_randr2_config_save();
}

/////////////////////////////////////////////////////////////////////////
EINTERN Eina_Bool
e_randr2_init(void)
{
   int count;

   if (!E_EVENT_RANDR_CHANGE) E_EVENT_RANDR_CHANGE = ecore_event_type_new();
   if (blocked) return EINA_FALSE;
   if ((!e_comp->screen) || (!e_comp->screen->available) || (!e_comp->screen->available())) return EINA_FALSE;
   initted = EINA_TRUE;
   // create data descriptors for config storage
   _e_randr2_cfg_screen_edd =
     E_CONFIG_DD_NEW("E_Config_Randr2_Screen", E_Config_Randr2_Screen);
#undef T
#undef D
#define T E_Config_Randr2_Screen
#define D _e_randr2_cfg_screen_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, rel_to, STR);
   E_CONFIG_VAL(D, T, rel_align, DOUBLE);
   E_CONFIG_VAL(D, T, mode_refresh, DOUBLE);
   E_CONFIG_VAL(D, T, mode_w, UINT);
   E_CONFIG_VAL(D, T, mode_h, INT);
   E_CONFIG_VAL(D, T, rotation, INT);
   E_CONFIG_VAL(D, T, priority, INT);
   E_CONFIG_VAL(D, T, rel_mode, UCHAR);
   E_CONFIG_VAL(D, T, enabled, UCHAR);
   E_CONFIG_VAL(D, T, ignore_disconnect, UCHAR);
   E_CONFIG_VAL(D, T, profile, STR);
   E_CONFIG_VAL(D, T, scale_multiplier, DOUBLE);

   _e_randr2_cfg_edd = E_CONFIG_DD_NEW("E_Config_Randr2", E_Config_Randr2);
#undef T
#undef D
#define T E_Config_Randr2
#define D _e_randr2_cfg_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_LIST(D, T, screens, _e_randr2_cfg_screen_edd);
   E_CONFIG_VAL(D, T, hotplug_response, DOUBLE);
   E_CONFIG_VAL(D, T, restore, UCHAR);
   E_CONFIG_VAL(D, T, ignore_hotplug_events, UCHAR);
   E_CONFIG_VAL(D, T, ignore_acpi_events, UCHAR);
   E_CONFIG_VAL(D, T, default_policy, UINT);

   // set up events from the driver
   if (e_comp->screen->init)
     e_comp->screen->init();
   // get current screen info
   e_randr2 = e_comp->screen->create();
   // from screen info calculate screen max dimensions
   _screen_config_maxsize();
   // load config and apply it
   e_randr2_cfg = _config_load();
   // only apply if restore is set AND at least one configured screen
   // matches one we have

   count = _config_screen_match_count(e_randr2, e_randr2_cfg);
   if (e_randr2_cfg->restore && count)
     {
        if (count != (int)eina_list_count(e_randr2->screens))
          {
             if (_config_update(e_randr2, e_randr2_cfg, 1))
               e_randr2_config_save();
          }
        _do_apply();
     }
   else
     {
        _config_update(e_randr2, e_randr2_cfg, 0);
        ecore_job_add(_cb_delay_init_save, NULL);
     }
   ecore_event_add(E_EVENT_RANDR_CHANGE, NULL, NULL, NULL);
   return EINA_TRUE;
}

EINTERN int
e_randr2_shutdown(void)
{
   if (!initted) return 0;
   initted = EINA_FALSE;
   _animated_apply_abort();
   // nuke any screen config delay handler
   if (_screen_delay_timer) ecore_timer_del(_screen_delay_timer);
   _screen_delay_timer = NULL;
   // stop listening to driver info
   if (e_comp->screen && e_comp->screen->shutdown)
     e_comp->screen->shutdown();
   // clear up all event handlers
   E_FREE_LIST(_ev_handlers, ecore_event_handler_del);
   // free up screen info
   _info_free(e_randr2);
   e_randr2 = NULL;
   _config_free(e_randr2_cfg);
   e_randr2_cfg = NULL;
   // free up data descriptors
   E_CONFIG_DD_FREE(_e_randr2_cfg_edd);
   E_CONFIG_DD_FREE(_e_randr2_cfg_screen_edd)
   return 1;
}

E_API void
e_randr2_stop(void)
{
   blocked = EINA_TRUE;
   if (initted) e_randr2_shutdown();
}

E_API Eina_Bool
e_randr2_config_save(void)
{
   // save our config
   return _config_save(e_randr2, e_randr2_cfg);
}

E_API void
e_randr2_config_apply(void)
{
   _animated_apply();
}

static E_Randr2_Screen *
_randr_screen_find(E_Randr2 *r, E_Randr2_Screen *src)
{
   Eina_List *l;
   E_Randr2_Screen *s;

   EINA_LIST_FOREACH(r->screens, l, s)
     {
        if ((s->info.name) && (src->info.name) &&
            (!strcmp(s->info.name, src->info.name)))
          return s;
     }
   return NULL;
}

static void
_randr_screen_props_copyover(E_Randr2 *rold, E_Randr2 *rnew)
{
   Eina_List *l;
   E_Randr2_Screen *s, *s2;

   // copy s->config.ignore_disconnect over from tr to e_randr2
   EINA_LIST_FOREACH(rnew->screens, l, s)
     {
        s2 = _randr_screen_find(rold, s);
        if (s2) s->config.ignore_disconnect = s2->config.ignore_disconnect;
     }
}

E_API void
e_randr2_screeninfo_update(void)
{
   E_Randr2 *tr = e_randr2;
   // re-fetch/update current screen info
   e_randr2 = e_comp->screen->create();
   if ((tr) && (e_randr2))
     {
        _randr_screen_props_copyover(tr, e_randr2);
        _info_free(tr);
     }
   _screen_config_maxsize();
}

/////////////////////////////////////////////////////////////////////////
static double _start_time = 0.0;
static Ecore_Animator *_fade_animator = NULL;
static Ecore_Timer *_apply_delay = NULL;
Eina_Bool _applying = EINA_FALSE;
static int _target_from = 0;
static int _target_to = 0;
static Evas_Object *_fade_obj = NULL;

static Eina_Bool
_screen_closed(E_Randr2_Screen *s)
{
   printf("RRR: check lid for %s...\n", s->info.name);
   if (!e_acpi_lid_is_closed()) return EINA_FALSE;
   if (s->info.is_lid)
     {
        printf("RRR:   is closed lid\n");
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_animated_apply_abort(void)
{
   if (_apply_delay) ecore_timer_del(_apply_delay);
   if (_fade_animator) ecore_animator_del(_fade_animator);
   _apply_delay = NULL;
   _fade_animator = NULL;
   _applying = EINA_FALSE;
   _fade_obj = NULL;
}

static Eina_Bool
_cb_delay_timer(void *data EINA_UNUSED)
{
   _apply_delay = NULL;
   _target_from = 255;
   _target_to = 0;
   _start_time = ecore_loop_time_get();
   _fade_animator = ecore_animator_add(_cb_fade_animator, NULL);
   return EINA_FALSE;
}

static Eina_Bool
_cb_fade_animator(void *data EINA_UNUSED)
{
   double t = ecore_loop_time_get() - _start_time;
   const double duration = 0.5;
   int v;

   if (t < 0.0) t = 0.0;
   t = t / duration;
   if (t > 1.0) t = 1.0;
   t = ecore_animator_pos_map(t, ECORE_POS_MAP_SINUSOIDAL, 0.0, 0.0);
   v = _target_from + ((_target_to - _target_from) * t);
   if (t >= 1.0) v = _target_to;
   evas_object_color_set(_fade_obj, 0, 0, 0, v);
   if (v == _target_to)
     {
        if (_target_to == 255)
          {
             _apply_delay = ecore_timer_loop_add(1.0, _cb_delay_timer, NULL);
             _do_apply();
          }
        else
          {
             evas_object_del(_fade_obj);
             _fade_obj = NULL;
             _applying = EINA_FALSE;
          }
        _fade_animator = NULL;
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_animated_apply(void)
{
   Evas *e;

   // fade out, config, wait 3 seconds, fade back in
   if (_applying) return;
   _applying = EINA_TRUE;
   _start_time = ecore_loop_time_get();
   e = e_comp->evas;
   _fade_obj = evas_object_rectangle_add(e);
   evas_object_pass_events_set(_fade_obj, EINA_TRUE);
   evas_object_color_set(_fade_obj, 0, 0, 0, 0);
   evas_object_move(_fade_obj, 0, 0);
   evas_object_resize(_fade_obj, 999999, 999999);
   evas_object_layer_set(_fade_obj, EVAS_LAYER_MAX);
   evas_object_show(_fade_obj);
   _target_from = 0;
   _target_to = 255;
   _fade_animator = ecore_animator_add(_cb_fade_animator, NULL);
}

static void
_do_apply(void)
{
   E_Randr2 *tr = e_randr2;
   // take current screen config and apply it to the driver
   printf("RRR: re-get info before applying..\n");
   tr = e_randr2;
   e_randr2 = e_comp->screen->create();
   if ((tr) && (e_randr2))
     {
        _randr_screen_props_copyover(tr, e_randr2);
        _info_free(tr);
     }
   _screen_config_maxsize();
   printf("RRR: apply config...\n");
   _config_apply(e_randr2, e_randr2_cfg);
   printf("RRR: takeover config...\n");
   _screen_config_takeover();
   printf("RRR: eval config...\n");
   _screen_config_eval();
   printf("RRR: really apply config...\n");
   e_comp->screen->apply();
   printf("RRR: done config...\n");
}

static void
_info_free(E_Randr2 *r)
{
   E_Randr2_Screen *s;
   E_Randr2_Mode *m;

   if (!r) return;
   // free up our randr screen data
   EINA_LIST_FREE(r->screens, s)
     {
        free(s->id);
        free(s->info.screen);
        free(s->info.name);
        free(s->info.edid);
        EINA_LIST_FREE(s->info.modes, m) free(m);
        free(s->config.relative.to);
        free(s->config.profile);
        free(s);
     }
   free(r);
}

static void
_config_upgrade(E_Config_Randr2 *cfg)
{
   if (cfg->version < 2)
     cfg->default_policy = E_RANDR2_POLICY_EXTEND;
   if (cfg->version < 3)
     cfg->hotplug_response = 1.0;
   cfg->version = E_RANDR_CONFIG_VERSION;
}

static E_Config_Randr2 *
_config_load(void)
{
   E_Config_Randr2 *cfg;

   // load config and check if version is up to date
   cfg = e_config_domain_load("e_randr2", _e_randr2_cfg_edd);
   if (cfg)
     {
        if (cfg->version < E_RANDR_CONFIG_VERSION)
          _config_upgrade(cfg);
        if (cfg->hotplug_response < 0.2)
          cfg->hotplug_response = 0.2;
        else if (cfg->hotplug_response > 9.0)
          cfg->hotplug_response = 9.0;
        printf("RRR: loaded existing config\n");
        return cfg;
     }

   // need new config
   cfg = calloc(1, sizeof(E_Config_Randr2));
   cfg->version = E_RANDR_CONFIG_VERSION;
   cfg->screens = NULL;
   cfg->restore = 1;
   cfg->ignore_hotplug_events = 0;
   cfg->ignore_acpi_events = 0;
   cfg->default_policy = E_RANDR2_POLICY_EXTEND;
   cfg->hotplug_response = 1.0;
   printf("RRR: fresh config\n");
   return cfg;
}

static void
_config_free(E_Config_Randr2 *cfg)
{
   E_Config_Randr2_Screen *cs;

   if (!cfg) return;
   // free config data
   EINA_LIST_FREE(cfg->screens, cs)
     {
        eina_stringshare_del(cs->id);
        eina_stringshare_del(cs->rel_to);
        eina_stringshare_del(cs->profile);
        free(cs);
     }
   free(cfg);
}

static Eina_Bool
_config_save(E_Randr2 *r, E_Config_Randr2 *cfg)
{
   if ((!r) || (!cfg)) return EINA_FALSE;
   // save config struct to cfg file
   return e_config_domain_save("e_randr2", _e_randr2_cfg_edd, cfg);
}

static Eina_Bool
_config_ask_dialog(void *data)
{
   e_configure_registry_call("screen/screen_setup", NULL, data);
   free(data);
   return EINA_FALSE;
}

static Eina_Bool
_config_update(E_Randr2 *r, E_Config_Randr2 *cfg, Eina_Bool update_only)
{
   Eina_List *l;
   E_Randr2_Screen *s;
   E_Config_Randr2_Screen *cs;
   Eina_Bool ret = EINA_FALSE;

   printf("--------------------------------------------------\n");
   EINA_LIST_FOREACH(r->screens, l, s)
     {
        printf("RRR: out id=%s:  connected=%i enabled=%i configured=%i\n",
               s->id, s->info.connected,
               s->config.enabled, s->config.configured);
        if (!s->id) continue;
        if (!s->info.connected) continue;
        if (_screen_closed(s)) continue;

        cs = e_randr2_config_screen_find(s, cfg);
        if (cs && update_only) continue;
        if (!cs)
          {
             cs = calloc(1, sizeof(E_Config_Randr2_Screen));
             if (cs)
               {
                  cs->id = eina_stringshare_add(s->id);
                  cfg->screens = eina_list_prepend(cfg->screens, cs);
               }
          }
        if (cs)
          {
             if (update_only)
               {
                  switch (cfg->default_policy)
                    {
                     case E_RANDR2_POLICY_EXTEND:
                       if (s->config.relative.mode < E_RANDR2_RELATIVE_TO_LEFT)
                         cs->rel_mode = E_RANDR2_RELATIVE_TO_RIGHT;
                       else
                         cs->rel_mode = s->config.relative.mode;
                       break;
                     case E_RANDR2_POLICY_CLONE:
                       cs->rel_mode = E_RANDR2_RELATIVE_CLONE;
                       break;
                     case E_RANDR2_POLICY_ASK:
                       if (starting)
                         ecore_timer_loop_add(2, _config_ask_dialog, eina_strdup(s->info.name));
                       else
                         ecore_timer_loop_add(0.01, _config_ask_dialog, eina_strdup(s->info.name));
                       EINA_FALLTHROUGH;
                       /* no break */
                     case E_RANDR2_POLICY_NONE:
                       cs->rel_mode = E_RANDR2_RELATIVE_NONE;
                       break;
                    }
               }
             else
               cs->rel_mode = s->config.relative.mode;
             if (update_only &&
               ((cfg->default_policy == E_RANDR2_POLICY_CLONE) ||
                (cfg->default_policy == E_RANDR2_POLICY_EXTEND)))
               {
                  E_Randr2_Mode *m = eina_list_data_get(s->info.modes);

                  cs->enabled = 1;
                  cs->mode_refresh = m->refresh;
                  cs->mode_w = m->w;
                  cs->mode_h = m->h;
                  if (s->config.relative.to)
                    cs->rel_to = eina_stringshare_add(s->config.relative.to);
                  else
                    {
                       /* find right-most screen */
                       E_Zone *zone = eina_list_last_data_get(e_comp->zones);
                       if (zone)
                         eina_stringshare_replace(&cs->rel_to, zone->randr2_id);
                    }
                  cs->rel_align = 0;
               }
             else
               {
                  cs->enabled = s->config.enabled;
                  cs->mode_refresh = s->config.mode.refresh;
                  cs->mode_w = s->config.mode.w;
                  cs->mode_h = s->config.mode.h;
                  if (s->config.relative.to)
                    cs->rel_to = eina_stringshare_add(s->config.relative.to);
                  cs->rel_align = s->config.relative.align;
               }
             cs->rotation = s->config.rotation;
             cs->priority = s->config.priority;
             if (cs->profile)
               {
                  printf("RRR: store config profile '%s'\n", cs->profile);
                  free(s->config.profile);
                  s->config.profile = strdup(cs->profile);
               }
             else
               {
                  free(s->config.profile);
                  s->config.profile = NULL;
               }
             printf("RRR: store scale mul %1.5f\n", cs->scale_multiplier);
             s->config.scale_multiplier = cs->scale_multiplier;
             s->config.ignore_disconnect = cs->ignore_disconnect;
             ret = EINA_TRUE;
          }
     }
   printf("--------------------------------------------------\n");
   return ret;
}

static void
_config_really_apply(E_Randr2_Screen *s, E_Config_Randr2_Screen *cs)
{
   if (cs)
     {
        s->config.enabled = EINA_TRUE;
        s->config.mode.w = cs->mode_w;
        s->config.mode.h = cs->mode_h;
        s->config.mode.refresh = cs->mode_refresh;
        s->config.mode.preferred = EINA_FALSE;
        printf("RRR: really apply rotation=%i\n", cs->rotation);
        s->config.rotation = cs->rotation;
        s->config.priority = cs->priority;
        free(s->config.relative.to);
        if (cs->rel_to) s->config.relative.to = strdup(cs->rel_to);
        else s->config.relative.to = NULL;
        s->config.relative.mode = cs->rel_mode;
        s->config.relative.align = cs->rel_align;
        free(s->config.profile);
        if (cs->profile) s->config.profile = strdup(cs->profile);
        else s->config.profile = NULL;
        s->config.scale_multiplier = cs->scale_multiplier;
        s->config.ignore_disconnect = cs->ignore_disconnect;
        printf("RRR: really apply '%s' ignore discon %i\n", s->info.name, s->config.ignore_disconnect);
     }
   else
     {
        s->config.enabled = EINA_FALSE;
        s->config.geom.x = 0;
        s->config.geom.y = 0;
        s->config.geom.w = 0;
        s->config.geom.h = 0;
        s->config.mode.w = 0;
        s->config.mode.h = 0;
        s->config.mode.refresh = 0.0;
        s->config.mode.preferred = EINA_FALSE;
        s->config.rotation = 0;
        s->config.priority = 0;
        free(s->config.relative.to);
        s->config.relative.to = NULL;
        s->config.relative.mode = E_RANDR2_RELATIVE_NONE;
        s->config.relative.align = 0.0;
        free(s->config.profile);
        s->config.profile = NULL;
        s->config.scale_multiplier = 0.0;
        s->config.ignore_disconnect = EINA_FALSE;
     }
}

static void
_config_apply(E_Randr2 *r, E_Config_Randr2 *cfg)
{
   Eina_List *l;
   E_Randr2_Screen *s;
   E_Config_Randr2_Screen *cs;

   if ((!r) || (!cfg)) return;
   EINA_LIST_FOREACH(r->screens, l, s)
     {
        printf("RRR: apply '%s'...\n", s->info.name);
        cs = NULL;
        if ((!_screen_closed(s)) && (s->info.connected))
          cs = e_randr2_config_screen_find(s, cfg);
        printf("RRR: connected =  %i\n", s->info.connected);
        if ((cs) && (cs->enabled))
          {
             printf("RRR: ... enabled\n");
             printf("RRR: ... priority = %i\n", cs->priority);
             _config_really_apply(s, cs);
          }
        else
          {
             printf("RRR: ... disabled\n");
             if (!s->config.ignore_disconnect)
               _config_really_apply(s, NULL);
             else
               printf("RRR: ... ignore disconnected\n");
          }
        s->config.configured = EINA_TRUE;
     }
}

static int
_config_screen_match_count(E_Randr2 *r, E_Config_Randr2 *cfg)
{
   Eina_List *l, *ll;
   E_Randr2_Screen *s;
   E_Config_Randr2_Screen *cs;
   int count = 0;

   EINA_LIST_FOREACH(cfg->screens, l, cs)
     {
        if (!cs->id) continue;
        EINA_LIST_FOREACH(r->screens, ll, s)
          {
             if ((!s->id) || (!s->info.connected) ||
                 (_screen_closed(s))) continue;
             if (!strcmp(cs->id, s->id)) count++;
          }
     }
   return count;
}

static char *
_screens_fingerprint(E_Randr2 *r)
{
   Eina_List *l;
   E_Randr2_Screen *s;
   Eina_Strbuf *buf;
   char *str;

   buf = eina_strbuf_new();
   if (!buf) return NULL;
   EINA_LIST_FOREACH(r->screens, l, s)
     {
        if (!s->id) eina_strbuf_append(buf, ":NULL:");
        else
          {
             eina_strbuf_append(buf, ":");
             eina_strbuf_append(buf, s->id);
             eina_strbuf_append(buf, ":");
// Don't do this asbecause this forces a screen replug when you open and
// close your laptop lid and if that is the only thing you open or close...
//             if (s->info.lid_closed) eina_strbuf_append(buf, ":LC:");
//             else eina_strbuf_append(buf, ":LO:");
          }
     }
   str = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return str;
}

static Eina_Bool
_screens_differ(E_Randr2 *r1, E_Randr2 *r2)
{
   char *s1, *s2;
   Eina_Bool changed = EINA_FALSE;
   Eina_List *l, *ll;
   E_Randr2_Screen *s, *ss;
   int r1_screen_num = 0, r2_screen_num = 0;

   // check monitor outputs and edids, plugged in things
   s1 = _screens_fingerprint(r1);
   s2 = _screens_fingerprint(r2);
   if ((!s1) && (!s2)) return EINA_FALSE;
   printf("RRR: check fingerprint...\n");
   if ((s1) && (s2) && (strcmp(s1, s2))) changed = EINA_TRUE;
   printf("RRR: ... fingerprint says %i\n", changed);
   free(s1);
   free(s2);
   // check screen config
   printf("RRR: screens lists %i -> %i\n", eina_list_count(r1->screens), eina_list_count(r2->screens));
   printf("RRR: --------\n");
   EINA_LIST_FOREACH(r1->screens, l, s)
     {
        if (!s->id) continue;
        if ((s->info.connected) &&
            (!((s->info.is_lid) && (s->info.lid_closed))))
          r1_screen_num++;
     }
   printf("RRR: --------\n");
   EINA_LIST_FOREACH(r2->screens, l, s)
     {
        if (!s->id) continue;
        if ((s->info.connected) &&
            (!((s->info.is_lid) && (s->info.lid_closed))))
          r2_screen_num++;
     }
   printf("RRR: --------\n");
   printf("RRR: screens %i -> %i\n", r1_screen_num, r2_screen_num);
   printf("RRR: --------\n");
   EINA_LIST_FOREACH(r2->screens, l, s)
     {
        if (s->id)
          printf("RRR: look at r2 screen ID %s\n", s->id);
        else
          printf("RRR: look at r2 screen ID NIL\n");
        if (!s->id) continue;
        EINA_LIST_FOREACH(r1->screens, ll, ss)
          {
             if (ss->id)
               printf("RRR: look at r1 screen ID %s\n", ss->id);
             else
               printf("RRR: look at r1 screen ID NIL\n");
             if ((ss->id) && (!strcmp(s->id, ss->id))) break;
             ss = NULL;
          }
        if (!ss)
          {
             printf("RRR: do change because cannot find screen matching ID %s\n", s->id);
             changed = EINA_TRUE;
          }
        else if ((s->config.geom.x != ss->config.geom.x) ||
                 (s->config.geom.y != ss->config.geom.y) ||
                 (s->config.geom.w != ss->config.geom.w) ||
                 (s->config.geom.h != ss->config.geom.h) ||
                 (s->config.mode.w != ss->config.mode.w) ||
                 (s->config.mode.h != ss->config.mode.h) ||
                 (s->config.enabled != ss->config.enabled) ||
                 (s->config.rotation != ss->config.rotation))
          {
             printf("RRR: do change because geom/mode/rotation don't match\n");
             printf("RRR: geom=%i,%i-%ix%i mode=%ix%i enable=%i rot=%i  !=  geom=%i,%i-%ix%i mode=%ix%i enable=%i rot=%i\n",
                    s->config.geom.x, s->config.geom.y,
                    s->config.geom.w, s->config.geom.h,
                    s->config.mode.w, s->config.mode.h,
                    s->config.enabled, s->config.rotation,
                    ss->config.geom.x, ss->config.geom.y,
                    ss->config.geom.w, ss->config.geom.h,
                    ss->config.mode.w, ss->config.mode.h,
                    ss->config.enabled, ss->config.rotation);
             changed = EINA_TRUE;
          }
        else
          {
             if (r1_screen_num != r2_screen_num)
               {
                  printf("RRR: do change because screen count changed\n");
                  changed = EINA_TRUE;
               }
             else
               {
                  if ((r2_screen_num != 1) &&
                      (s->info.lid_closed != ss->info.lid_closed))
                    {
                       printf("RRR: change because laptop lid open/close and number of screens > 1\n");
                       changed = EINA_TRUE;
                    }
                  else
                    printf("RRR: skip change because of single laptop lid\n");
               }
          }
     }
   printf("RRR: --------\n");
   printf("RRR: changed = %i\n", changed);
   return changed;
}

static Eina_Bool
_cb_deferred_suspend_screen_change(void *data EINA_UNUSED)
{
   Eina_List *l;
   E_Randr2_Screen *s;
   int lids = 0;
   int ext_screens = 0;

   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if (s->info.is_lid) lids++;
        else if ((s->config.enabled) &&
                 (s->config.geom.w > 0) &&
                 (s->config.geom.h > 0))
          ext_screens++;
     }
   printf("RRR: =========================== deferred suspend.... %i %i\n", lids, ext_screens);
   if ((lids > 0) && (ext_screens == 0))
     {
        if ((e_config->screensaver_suspend_on_ac) ||
            (e_powersave_mode_get() > E_POWERSAVE_MODE_LOW))
          {
             printf("RRR: =========================== powermd low / suspend on ac");
             e_sys_action_do(E_SYS_SUSPEND, NULL);
          }
     }
   return EINA_FALSE;
}

static Eina_Bool
_cb_screen_change_delay(void *data EINA_UNUSED)
{
   Eina_Bool change = EINA_FALSE;
   _screen_delay_timer = NULL;
   printf("RRR: ... %i %i\n", event_screen, event_ignore);
   // if we had a screen plug/unplug etc. event and we shouldn't ignore it...
   if ((event_screen) && (!event_ignore))
     {
        Eina_List *l;
        E_Randr2_Screen *s;
        int lid_screens = 0;
        int close_lid_screens = 0;
        int external_screens = 0;
        int prev_external_screens = 0;
        E_Randr2 *rtemp;

        printf("RRR: reconfigure screens due to event...\n");
        rtemp = e_comp->screen->create();
        if (rtemp)
          {
             if (e_randr2) _randr_screen_props_copyover(e_randr2, rtemp);
             if (_screens_differ(e_randr2, rtemp)) change = EINA_TRUE;
             if (e_randr2_cfg->default_policy != E_RANDR2_POLICY_NONE)
               {
                  if (_config_update(rtemp, e_randr2_cfg, 1))
                    {
                       e_randr2_config_save();
                       if (e_randr2_cfg->default_policy != E_RANDR2_POLICY_ASK)
                         change = EINA_TRUE;
                    }
               }
             EINA_LIST_FOREACH(rtemp->screens, l, s)
               {
                  printf("RRR: scr: %s lid=%i conn=%i\n", s->id, s->info.is_lid, s->info.connected);
//                  if (!s->id) continue;
                  if (s->info.is_lid)
                    {
                       printf("RRR: is lid, lid++\n");
                       lid_screens++;
                       if ((s->info.lid_closed) && (!s->config.ignore_disconnect))
                         {
                            printf("RRR: is lid, is closed, closed++\n");
                            close_lid_screens++;
                         }
                    }
                  else
                    {
                       if (s->info.connected)
                         {
                            printf("RRR: is not lid, is connected, ext++\n");
                            external_screens++;
                         }
                       else
                         {
                            printf("RRR: is not lid, is not connected\n");
                         }
                    }
               }
             EINA_LIST_FOREACH(e_randr2->screens, l, s)
               {
                  printf("RRR: prev_scr: %s lid=%i conn=%i\n", s->id, s->info.is_lid, s->info.connected);
//                  if (!s->id) continue;
                  if (!s->info.is_lid)
                    {
                       if (s->info.connected) prev_external_screens++;
                    }
               }
             printf("RRR: lids=%i closed=%i ext=%i prev_ext=%i\n", lid_screens, close_lid_screens, external_screens, prev_external_screens);
             _info_free(rtemp);
          }
        printf("RRR: change = %i\n", change);
        // we plugged or unplugged some monitor - re-apply config so
        // known screens can be configured
        if (change)
          {
             if ((lid_screens > 0) && (close_lid_screens == lid_screens) &&
                 (external_screens == 0) && (external_screens == prev_external_screens))
               {
                  printf("RRR: skip change with all lids closed and no ext\n");
                  change = EINA_FALSE;
                  e_screensaver_now_set(EINA_TRUE);
                  e_screensaver_attrs_set
                    (1, e_config->screensaver_blanking,
                     e_config->screensaver_expose);
                  e_screensaver_update();
                  e_dpms_force_update();
                  e_screensaver_activate();
                  e_screensaver_eval(EINA_TRUE);
                  // force dpms...
               }
             if ((lid_screens > 0) && (close_lid_screens < lid_screens) &&
                 (external_screens == 0) && (external_screens == prev_external_screens))
               {
                  printf("RRR: skip change with lid screens open and no ext\n");
                  change = EINA_FALSE;
                  e_screensaver_now_set(EINA_FALSE);
                  e_screensaver_attrs_set
                    (e_config->screensaver_timeout,
                     e_config->screensaver_blanking,
                     e_config->screensaver_expose);
                  e_screensaver_update();
                  e_dpms_force_update();
                  e_screensaver_deactivate();
               }
             if (change)
               e_randr2_config_apply();
          }
        if (change)
          {
             if ((lid_screens > 0) && (close_lid_screens == lid_screens) &&
                 (external_screens == 0))
               {
                  printf("RRR: have all closed laptop screens and no external\n");
                  if ((e_config->screensaver_suspend_on_ac) ||
                      (e_powersave_mode_get() > E_POWERSAVE_MODE_LOW))
                    {
                       printf("RRR: we should try and suspend now because on ac or suspend on ac is on\n");
                       ecore_timer_add(1.0, _cb_deferred_suspend_screen_change, NULL);
                    }
               }
          }
     }
   // update screen info after the above apply or due to external changes
   e_randr2_screeninfo_update();
   e_comp_canvas_resize(e_randr2->w, e_randr2->h);
   e_randr2_screens_setup(e_comp->w, e_comp->h);
   e_comp_canvas_update();
   // tell the rest of e some screen reconfigure thing happened
   ecore_event_add(E_EVENT_RANDR_CHANGE, NULL, NULL, NULL);
   event_screen = EINA_FALSE;
   event_ignore = EINA_FALSE;
   return EINA_FALSE;
}

static E_Randr2_Screen *
_screen_output_find(const char *out)
{
   E_Randr2_Screen *s;
   Eina_List *l;
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if (!strcmp(s->info.name, out)) return s;
     }
   return NULL;
}

static E_Randr2_Screen *
_screen_id_find(const char *id)
{
   E_Randr2_Screen *s;
   Eina_List *l;
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if (!strcmp(s->id, id)) return s;
     }
   return NULL;
}

static void
_screen_config_takeover(void)
{
   Eina_List *l;
   E_Randr2_Screen *s;
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        s->config.configured = EINA_TRUE;
     }
}

static E_Config_Randr2_Screen *_config_screen_string_find(E_Config_Randr2 *cfg, const char *id);
static E_Randr2_Screen *_screen_fuzzy_fallback_find(E_Config_Randr2 *cfg, const char *id);

static E_Config_Randr2_Screen *
_config_screen_string_find(E_Config_Randr2 *cfg, const char *id)
{
   Eina_List *l;
   E_Config_Randr2_Screen *cs;

   if ((!id) || (!cfg)) return NULL;
   EINA_LIST_FOREACH(cfg->screens, l, cs)
     {
        if (!cs->id) continue;
        if (!strcmp(cs->id, id)) return cs;
     }
   return NULL;
}

static E_Randr2_Screen *
_screen_fuzzy_fallback_find(E_Config_Randr2 *cfg, const char *id)
{
   E_Randr2_Screen *s = NULL;
   char *p, *name;

   // strip out everythng in the string from / on as that is edid
   // and fall back to finding just the output name in the rel
   // to identifier, rather than the specific screen id
   p = strchr(id, '/');
   if (!p) return NULL;
   name = alloca((p - id) + 1);
   strncpy(name, id, p - id);
   name[p - id] = 0;

   s = _screen_id_find(id);
   if (!s) s = _screen_output_find(name);
   if (!s)
     {
        E_Config_Randr2_Screen *cs;

        cs = _config_screen_string_find(cfg, name);
        if ((cs) && (cs->id)) return _screen_fuzzy_fallback_find(cfg, cs->id);
     }
   return s;
}

static E_Config_Randr2_Screen *
_config_screen_clone_resolve(E_Config_Randr2 *cfg, const char *id, int *x, int *y)
{
   E_Config_Randr2_Screen *cs;
   E_Randr2_Screen *s;
   char *p, *name;

   cs = _config_screen_string_find(cfg, id);
   if (!cs) return NULL;

   name = alloca(strlen(cs->id) + 1);
   strcpy(name, cs->id);
   if ((p = strchr(name, '/'))) *p = 0;

   s = _screen_id_find(cs->id);
   if (!s) s = _screen_output_find(name);
   if (!s)
     {
        if ((cs->rel_mode == E_RANDR2_RELATIVE_CLONE) && (cs->rel_to))
          return _config_screen_clone_resolve(cfg, cs->rel_to, x, y);
        return NULL;
     }
   printf("RRR: resolve clone... [%s]\n", cs->id);
   _screen_config_do(s);
   *x = s->config.geom.x;
   *y = s->config.geom.y;
   return cs;
}

static Eina_List *
_screen_clones_find(Eina_List *screens, E_Randr2_Screen *s)
{
   Eina_List *clones = NULL, *l;
   E_Randr2_Screen *s2, *sclone;
   Eina_Bool added;

   // go over all screens and as long as we have found another screen that is
   // cloned from something in the clone set, then keep looking.
   clones = eina_list_append(clones, s);
   added = EINA_TRUE;
   while (added)
     {
        added = EINA_FALSE;
        // og over all screens
        EINA_LIST_FOREACH(screens, l, s2)
          {
             // skip looking at screens we already have in our set
             if (eina_list_data_find(clones, s2)) continue;
             // if this clones another screen... get that as sclone
             if ((s2->config.relative.to) &&
                 (s2->config.relative.mode == E_RANDR2_RELATIVE_CLONE))
               {
                  sclone = _screen_fuzzy_fallback_find(e_randr2_cfg,
                                                       s2->config.relative.to);
                  if (!sclone) continue;
                  // if the screen s2 is relative to is not in our list, add
                  // s2 to our clones list as well
                  if (!eina_list_data_find(clones, sclone))
                    {
                       clones = eina_list_append(clones, sclone);
                       added = EINA_TRUE;
                    }
                  if (!eina_list_data_find(clones, s2))
                    {
                       clones = eina_list_append(clones, s2);
                       added = EINA_TRUE;
                    }
                  if (added)
                    {
                       // break our list walk, and iterate while again
                       break;
                    }
              }
          }
     }
   return clones;
}

static void
_screen_clones_common_sync(Eina_List *clones)
{
   E_Randr2_Screen *s, *sbase = NULL;
   E_Randr2_Mode *m, *m2, *mcommon = NULL;
   Eina_List *modes = NULL, *l, *l2, *l3;
   Eina_Bool common;
   int d, diff = 0x7fffffff;

   // find the base/root/master screen for clones
   printf("RRR: find base/root for list=%p count=%i\n", clones, eina_list_count(clones));
   EINA_LIST_FOREACH(clones, l, s)
     {
        // simple check - if it doesn't clone something else - then it's
        // the master (doesn't handle missing screens)
        printf("RRR: clone=%p mode=%i\n", s, s->config.relative.mode);
        if ((s->config.relative.mode != E_RANDR2_RELATIVE_CLONE) &&
            (s->config.relative.mode != E_RANDR2_RELATIVE_NONE) &&
            (s->config.relative.mode != E_RANDR2_RELATIVE_UNKNOWN))
          {
             printf("RRR: got it\n");
             sbase = s;
             break;
          }
     }
   if (!sbase) return;
   // store all modes that master/base screen has and we'll "weed them out"
   EINA_LIST_FOREACH(sbase->info.modes, l, m)
     {
        modes = eina_list_append(modes, m);
     }
   // ensure it's configured
   printf("RRR: clone common sync... %p %p\n", sbase, s);
   _screen_config_do(sbase);
again:
   // we took all modes in the "master"
   EINA_LIST_FOREACH(modes, l, m)
     {
        // find all other screens in the clone list and...
        EINA_LIST_FOREACH(clones, l2, s)
          {
             if (s == sbase) continue; // skip if its the base/master
             // see if mode "m" is common to other clones
             common = EINA_FALSE;
             EINA_LIST_FOREACH(s->info.modes, l3, m2)
               {
                  /// only check res, not refresh
                  if ((m->w == m2->w) && (m->h == m2->h))
                    {
                       common = EINA_TRUE;
                       break;
                    }
               }
             // m is not common with modes in screen s - so removed it
             if (!common)
               {
                  modes = eina_list_remove_list(modes, l);
                  // the list is no longer save to walk - so let's just
                  // walk it again from scratch
                  goto again;
               }
          }
     }
   // no modes in common :(
   if (!modes) return;
   common = EINA_FALSE;
   EINA_LIST_FOREACH(modes, l, m)
     {
        // one of the common modes matches the base config - we are ok
        if ((m->w == sbase->config.mode.w) && (m->h == sbase->config.mode.h))
          {
             modes = eina_list_free(modes);
             return;
          }
     }
   // find a common mode since current config doesn't match
   EINA_LIST_FOREACH(modes, l, m)
     {
        // calculate a "difference" based on a combo of diff in area pixels
        // actual resolutin pixels (squared) and refresh delta * 10 squared
        d = abs((sbase->config.mode.w * sbase->config.mode.h) - (m->w * m->h));
        d += (sbase->config.mode.w - m->w) * (sbase->config.mode.w - m->w);
        d += (sbase->config.mode.h - m->h) * (sbase->config.mode.h - m->h);
        d += ((sbase->config.mode.refresh - m->refresh) * 10) *
             ((sbase->config.mode.refresh - m->refresh) * 10);
        if ((m->w > sbase->config.mode.w) || (m->h > sbase->config.mode.h))
          continue;
        if (d < diff)
          {
             diff = d;
             mcommon = m;
          }
     }
   modes = eina_list_free(modes);
   // no common mode with least difference found
   if (!mcommon) return;
   // we have a common mode - apply it to the base screen
   s = sbase;
   s->config.mode.w = mcommon->w;
   s->config.mode.h = mcommon->h;
   s->config.mode.refresh = mcommon->refresh;
   printf("RRR: clones common sync=%ix%i rotation=%i\n", s->config.mode.w, s->config.mode.h, s->config.rotation);
   if ((s->config.rotation == 0) || (s->config.rotation == 180))
     {
        s->config.geom.w = s->config.mode.w;
        s->config.geom.h = s->config.mode.h;
     }
   else
     {
        s->config.geom.w = s->config.mode.h;
        s->config.geom.h = s->config.mode.w;
     }
}

static int _config_do_recurse = 0;

static void
_screen_config_do(E_Randr2_Screen *s)
{
   E_Randr2_Screen *s2 = NULL;
   Eina_List *cloneset;

   printf("RRR: screen do '%s'\n", s->info.name);
   if (_config_do_recurse > 5)
     {
        printf("RRR: screen config loop!\n");
        return;
     }
   _config_do_recurse++;
   // find dependent clones and find a common config res
   cloneset = _screen_clones_find(e_randr2->screens, s);
   if (cloneset)
     {
        _screen_clones_common_sync(cloneset);
        eina_list_free(cloneset);
     }
   // if screen has a dependency...
   if ((s->config.relative.mode != E_RANDR2_RELATIVE_UNKNOWN) &&
       (s->config.relative.mode != E_RANDR2_RELATIVE_NONE) &&
       (s->config.relative.to))
     {
        // if this screen is relative TO something (clone or left/right etc.
        // then calculate what it is relative to first
        s2 = _screen_fuzzy_fallback_find(e_randr2_cfg, s->config.relative.to);
        printf("RRR: '%s' is relative to '%s'\n", s->info.name, s2 ? s2->info.name : "NONE");
        if (s2)
          {
             _screen_config_do(s2);
             if (!s2->config.enabled) s2 = NULL;
          }
     }
   s->config.geom.x = 0;
   s->config.geom.y = 0;
   printf("RRR: screen config do %ix%i rotation=%i\n", s->config.mode.w, s->config.mode.h, s->config.rotation);
   if ((s->config.rotation == 0) || (s->config.rotation == 180))
     {
        s->config.geom.w = s->config.mode.w;
        s->config.geom.h = s->config.mode.h;
     }
   else
     {
        s->config.geom.w = s->config.mode.h;
        s->config.geom.h = s->config.mode.w;
     }
   if (s2)
     {
        if (s->config.relative.mode == E_RANDR2_RELATIVE_CLONE)
          {
             printf("RRR: clone relative\n");
             s->config.geom.x = s2->config.geom.x;
             s->config.geom.y = s2->config.geom.y;
             s->config.geom.w = s2->config.geom.w;
             s->config.geom.h = s2->config.geom.h;
             s->config.mode.w = s2->config.mode.w;
             s->config.mode.h = s2->config.mode.h;
             printf("RRR: screen config do rotation=%i\n", s2->config.rotation);
             s->config.rotation = s2->config.rotation;
             s->config.mode.refresh = s2->config.mode.refresh;
          }
        else if (s->config.relative.mode == E_RANDR2_RELATIVE_TO_LEFT)
          {
             printf("RRR: to left relative\n");
             s->config.geom.x = s2->config.geom.x - s->config.geom.w;
             s->config.geom.y = s2->config.geom.y +
             ((s2->config.geom.h - s->config.geom.h) *
              s->config.relative.align);
          }
        else if (s->config.relative.mode == E_RANDR2_RELATIVE_TO_RIGHT)
          {
             printf("RRR: to right relative\n");
             s->config.geom.x = s2->config.geom.x + s2->config.geom.w;
             s->config.geom.y = s2->config.geom.y +
             ((s2->config.geom.h - s->config.geom.h) *
              s->config.relative.align);
          }
        else if (s->config.relative.mode == E_RANDR2_RELATIVE_TO_ABOVE)
          {
             printf("RRR: to above relative\n");
             s->config.geom.x = s2->config.geom.x +
             ((s2->config.geom.w - s->config.geom.w) *
              s->config.relative.align);
             s->config.geom.y = s2->config.geom.y - s->config.geom.h;
          }
        else if (s->config.relative.mode == E_RANDR2_RELATIVE_TO_BELOW)
          {
             printf("RRR: to below relative\n");
             s->config.geom.x = s2->config.geom.x +
             ((s2->config.geom.w - s->config.geom.w) *
              s->config.relative.align);
             s->config.geom.y = s2->config.geom.y + s2->config.geom.h;
          }
     }
   else
     {
        if ((s->config.relative.mode == E_RANDR2_RELATIVE_CLONE) &&
            (s->config.relative.to))
          {
             E_Config_Randr2_Screen *cs;
             int x = 0, y = 0;

             cs = _config_screen_clone_resolve(e_randr2_cfg,
                                               s->config.relative.to, &x, &y);
             printf("RRR: clone relative - config %p\n", cs);
             if (cs)
               {
                  s->config.geom.x = x;
                  s->config.geom.y = y;
                  s->config.mode.w = cs->mode_w;
                  s->config.mode.h = cs->mode_h;
                  printf("RRR: clone cs rotation=%i\n", cs->rotation);
                  s->config.rotation = cs->rotation;
                  s->config.mode.refresh = cs->mode_refresh;
                  if ((cs->rotation == 0) || (cs->rotation == 180))
                    {
                       s->config.geom.w = s->config.mode.w;
                       s->config.geom.h = s->config.mode.h;
                    }
                  else
                    {
                       s->config.geom.w = s->config.mode.h;
                       s->config.geom.h = s->config.mode.w;
                    }
               }
          }
     }
   _config_do_recurse--;
}

static void
_screen_config_eval(void)
{
   Eina_List *l;
   E_Randr2_Screen *s;
   int minx, miny, maxx, maxy;

   printf("RRR:--------------------------------1\n");
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if (s->config.configured)
          {
             printf("RRR: screen config eval this...\n");
             _screen_config_do(s);
          }
     }
   minx = 65535;
   miny = 65535;
   maxx = -65536;
   maxy = -65536;
   printf("RRR:--------------------------------2\n");
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if (!s->config.enabled) continue;
        if (s->config.geom.x < minx) minx = s->config.geom.x;
        if (s->config.geom.y < miny) miny = s->config.geom.y;
        if ((s->config.geom.x + s->config.geom.w) > maxx)
          maxx = s->config.geom.x + s->config.geom.w;
        if ((s->config.geom.y + s->config.geom.h) > maxy)
          maxy = s->config.geom.y + s->config.geom.h;
        printf("RRR: s: '%s' @ %i %i - %ix%i\n",
               s->info.name,
               s->config.geom.x, s->config.geom.y,
               s->config.geom.w, s->config.geom.h);
     }
   printf("RRR:--- %i %i -> %i %i\n", minx, miny, maxx, maxy);
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        s->config.geom.x -= minx;
        s->config.geom.y -= miny;
     }
   e_randr2->w = maxx - minx;
   e_randr2->h = maxy - miny;
}

static void
_screen_config_maxsize(void)
{
   Eina_List *l;
   E_Randr2_Screen *s;
   int maxx, maxy;

   maxx = -65536;
   maxy = -65536;
   printf("RRR:-------------------------------- 2\n");
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if (!s->config.enabled) continue;
        if ((s->config.geom.x + s->config.geom.w) > maxx)
          maxx = s->config.geom.x + s->config.geom.w;
        if ((s->config.geom.y + s->config.geom.h) > maxy)
          maxy = s->config.geom.y + s->config.geom.h;
        printf("RRR: '%s': %i %i %ix%i\n",
               s->info.name,
               s->config.geom.x, s->config.geom.y,
               s->config.geom.w, s->config.geom.h);
     }
   printf("RRR: result max: %ix%i\n", maxx, maxy);
   e_randr2->w = maxx;
   e_randr2->h = maxy;
}

static int
_screen_sort_cb(const void *data1, const void *data2)
{
   const E_Randr2_Screen *s1 = data1, *s2 = data2;
   int dif;

   dif = -(s1->config.priority - s2->config.priority);
   if (dif == 0)
     {
        dif = s1->config.geom.x - s2->config.geom.x;
        if (dif == 0)
          dif = s1->config.geom.y - s2->config.geom.y;
     }
   return dif;
}

E_API void
e_randr2_screen_refresh_queue(Eina_Bool lid_event)
{
   // delay handling of screen shances as they can come in in a series over
   // time and thus we can batch up responding to them by waiting 1.0 sec
   if (_screen_delay_timer)
     ecore_timer_loop_reset(_screen_delay_timer);
   else
     {
        double t = 1.0;

        if ((e_randr2_cfg) && (e_randr2_cfg->hotplug_response > 0.2))
          t = e_randr2_cfg->hotplug_response;
        _screen_delay_timer = ecore_timer_loop_add(t, _cb_screen_change_delay, NULL);
     }
   event_screen |= !!lid_event;
}

E_API E_Config_Randr2_Screen *
e_randr2_config_screen_find(E_Randr2_Screen *s, E_Config_Randr2 *cfg)
{
   Eina_List *l;
   E_Config_Randr2_Screen *cs;

   if ((!s) || (!cfg)) return NULL;
   if (!s->id) return NULL;
   EINA_LIST_FOREACH(cfg->screens, l, cs)
     {
        if (!cs->id) continue;
        if (!strcmp(cs->id, s->id)) return cs;
     }
   return NULL;
}

E_API void
e_randr2_screens_setup(int rw, int rh)
{
   int i;
   E_Screen *screen;
   Eina_List *screens = NULL, *screens_rem;
   Eina_List *all_screens = NULL;
   Eina_List *l, *ll;
   E_Randr2_Screen *s, *s2, *s_chosen;
   Eina_Bool removed;

   if ((!e_randr2) || (!e_randr2->screens)) goto out;
   // put screens in tmp list
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if ((s->config.enabled) &&
            (s->config.geom.w > 0) &&
            (s->config.geom.h > 0))
          {
             screens = eina_list_append(screens, s);
          }
     }
   // remove overlapping screens - if a set of screens overlap, keep the
   // smallest/lowest res
   do
     {
        removed = EINA_FALSE;

        EINA_LIST_FOREACH(screens, l, s)
          {
             screens_rem = NULL;

             EINA_LIST_FOREACH(l->next, ll, s2)
               {
                  if (E_INTERSECTS(s->config.geom.x, s->config.geom.y,
                                   s->config.geom.w, s->config.geom.h,
                                   s2->config.geom.x, s2->config.geom.y,
                                   s2->config.geom.w, s2->config.geom.h))
                    {
                       if (!screens_rem)
                         screens_rem = eina_list_append(screens_rem, s);
                       screens_rem = eina_list_append(screens_rem, s2);
                    }
               }
             // we have intersecting screens - choose the lowest res one
             if (screens_rem)
               {
                  removed = EINA_TRUE;
                  // find the smallest screen (chosen one)
                  s_chosen = NULL;
                  EINA_LIST_FOREACH(screens_rem, ll, s2)
                    {
                       if (!s_chosen) s_chosen = s2;
                       else
                         {
                            if ((s_chosen->config.geom.w *
                                 s_chosen->config.geom.h) >
                                (s2->config.geom.w *
                                 s2->config.geom.h))
                              s_chosen = s2;
                         }
                    }
                  // remove all from screens but the chosen one
                  EINA_LIST_FREE(screens_rem, s2)
                    {
                       if (s2 != s_chosen)
                         screens = eina_list_remove_list(screens, l);
                    }
                  // break our list walk and try again
                  break;
               }
          }
     }
   while (removed);
   // sort screens by priority etc.
   screens = eina_list_sort(screens, 0, _screen_sort_cb);
   i = 0;
   EINA_LIST_FOREACH(screens, l, s)
     {
        screen = E_NEW(E_Screen, 1);
        screen->escreen = screen->screen = i;
        screen->x = s->config.geom.x;
        screen->y = s->config.geom.y;
        screen->w = s->config.geom.w;
        screen->h = s->config.geom.h;
        if (s->id) screen->id = strdup(s->id);

        all_screens = eina_list_append(all_screens, screen);
        printf("xinerama screen %i %i %ix%i\n", screen->x, screen->y, screen->w, screen->h);
        INF("E INIT: XINERAMA SCREEN: [%i][%i], %ix%i+%i+%i",
            i, i, screen->w, screen->h, screen->x, screen->y);
        i++;
     }
   eina_list_free(screens);
   // if we have NO screens at all (above - i will be 0) AND we have no
   // existing screens set up in xinerama - then just say root window size
   // is the entire screen. this should handle the case where you unplug ALL
   // screens from an existing setup (unplug external monitors and/or close
   // laptop lid), in which case as long as at least one screen is configured
   // in xinerama, it will be left-as is until next time we re-eval screen
   // setup and have at least one screen
   printf("xinerama setup............... %i %p\n", i, e_xinerama_screens_all_get());
   if ((i == 0) && (!e_xinerama_screens_all_get()))
     {
out:
        screen = E_NEW(E_Screen, 1);
        screen->escreen = screen->screen = 0;
        screen->x = 0;
        screen->y = 0;
        if ((rw > 0) && (rh > 0))
          screen->w = rw, screen->h = rh;
        else
          ecore_evas_screen_geometry_get(e_comp->ee, NULL, NULL, &screen->w, &screen->h);
        all_screens = eina_list_append(all_screens, screen);
     }
   e_xinerama_screens_set(all_screens);
}

E_API E_Randr2_Screen *
e_randr2_screen_id_find(const char *id)
{
   Eina_List *l;
   E_Randr2_Screen *s;

   if (!id) return NULL;
   EINA_LIST_FOREACH(e_randr2->screens, l, s)
     {
        if (!strcmp(id, s->id)) return s;
     }
   return NULL;
}

E_API double
e_randr2_screen_dpi_get(E_Randr2_Screen *s)
{
   double dpi1, dpi2;

   if ((s->info.size.w <= 0) || (s->info.size.h <= 0)) return 0.0;
   dpi1 = (25.4 * (double)(s->config.mode.w)) / (double)(s->info.size.w);
   dpi2 = (25.4 * (double)(s->config.mode.h)) / (double)(s->info.size.h);
   return (dpi1 + dpi2) / 2.0;
}

static int
_modelist_sort(const void *a, const void *b)
{
   const E_Randr2_Mode *ma = a, *mb = b;

   /* largest resolutions first */
   if ((ma->w * ma->h) > (mb->w * mb->h)) return -1;
   /* highest refresh first */
   if (ma->refresh > mb->refresh) return -1;
   return 1;
}

EAPI void
e_randr2_screen_modes_sort(E_Randr2_Screen *s)
{
   EINA_SAFETY_ON_NULL_RETURN(s);
   s->info.modes = eina_list_sort(s->info.modes, 0, _modelist_sort);
}
