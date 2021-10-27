#include "private.h"

// XXX: 

#define MUSIC_CONTROL_DOMAIN "module.music_control"

static E_Module *music_control_mod = NULL;
static Eina_Bool was_playing_before_lock = EINA_FALSE;
static const char _e_music_control_Name[] = N_("Music controller");
static Ecore_Event_Handler *music_control_powersave_event_handler = NULL;
static Eina_Bool have_player = EINA_FALSE;

const Player music_player_players[] =
{
    // Label/Name           dbus iface name                         command to execute
   {"Audacious",            "org.mpris.MediaPlayer2.audacious",        "audacious"},
   {"BMP",                  "org.mpris.MediaPlayer2.bmp",              "bmp"},
   {"Banshee",              "org.mpris.MediaPlayer2.banshee",          "banshee"},
   {"Clementine",           "org.mpris.MediaPlayer2.clementine",       "clementine"},
   {"DeaDBeeF",             "org.mpris.MediaPlayer2.deadbeef",         "deadbeef"},
   {"Emotion Media Center", "org.mpris.MediaPlayer2.epymc",            "epymc"},
   {"MPD",                  "org.mpris.MediaPlayer2.mpd",              "mpd"},
   {"Pithos",               "org.mpris.MediaPlayer2.io.github.Pithos", "pithos"},
   {"Quod Libet",           "org.mpris.MediaPlayer2.quodlibet",        "quodlibet"},
   {"Rage",                 "org.mpris.MediaPlayer2.rage",             "rage"},
   {"Rhythmbox",            "org.gnome.Rhythmbox3",                    "rhythmbox"},
   {"Spotify",              "org.mpris.MediaPlayer2.spotify",          "spotify"},
   {"Spotifyd",             "org.mpris.MediaPlayer2.spotifyd",         "spotifyd"},
   {"Squeezebox",           "org.mpris.MediaPlayer2.squeezebox",       "slimpris2"},
   {"Tomahawk",             "org.mpris.MediaPlayer2.tomahawk",         "tomahawk"},
   {"VLC",                  "org.mpris.MediaPlayer2.vlc",              "vlc"},
   {"XMMS2",                "org.mpris.MediaPlayer2.xmms2",            "xmms2"},
   {"gmusicbrowser",        "org.mpris.MediaPlayer2.gmusicbrowser",    "gmusicbrowser"},
   {"Chromium",             "org.mpris.MediaPlayer2.chromium.*",       "chromium"},
   {"Firefox",              "org.mpris.MediaPlayer2.firefox.*",        "firefox"},
   {"Automatic",            "org.mpris.MediaPlayer2.*",                ""},
#define PLAYER_COUNT 21
   {NULL, NULL, NULL}
};

Eina_Bool
_desklock_cb(void *data, int type EINA_UNUSED, void *ev)
{
   E_Music_Control_Module_Context *ctxt = data;
   E_Event_Desklock *event = ev;

   // Lock with music on. Pause it
   if (event->on && ctxt->playing)
     {
        media_player2_player_play_pause_call(ctxt->mpris2_player);
        was_playing_before_lock = EINA_TRUE;
        return ECORE_CALLBACK_PASS_ON;
     }
   // Lock without music. Keep music off as state
   if (event->on && (!ctxt->playing))
     {
        was_playing_before_lock = EINA_FALSE;
        return ECORE_CALLBACK_PASS_ON;
     }
   // Unlock with music pause and playing before lock. Turn it back on
   if ((!event->on) && (!ctxt->playing) && was_playing_before_lock)
     media_player2_player_play_pause_call(ctxt->mpris2_player);
   return ECORE_CALLBACK_PASS_ON;
}

static void
_music_control(E_Object *obj EINA_UNUSED, const char *params)
{
   E_Music_Control_Module_Context *ctxt = music_control_mod->data;
   EINA_SAFETY_ON_NULL_RETURN(music_control_mod->data);
   EINA_SAFETY_ON_NULL_RETURN(params);

   if (!strcmp(params, "play")) //legacy compatibility - this should probebly be called playpause
     media_player2_player_play_pause_call(ctxt->mpris2_player);
   else if (!strcmp(params, "next"))
     media_player2_player_next_call(ctxt->mpris2_player);
   else if (!strcmp(params, "previous"))
     media_player2_player_previous_call(ctxt->mpris2_player);
   else if (!strcmp(params, "pause"))
     media_player2_player_pause_call(ctxt->mpris2_player);
   else if (!strcmp(params, "play_music")) //the name play was already taken by play pause, see above
     media_player2_player_play_call(ctxt->mpris2_player);
}

#define ACTION_NEXT "next_music"
#define ACTION_NEXT_NAME "Next Music"
#define ACTION_PLAY_PAUSE "playpause_music"
#define ACTION_PLAY_PAUSE_NAME "Play/Pause Music"
#define ACTION_PAUSE "pause_music"
#define ACTION_PAUSE_NAME "Pause Music"
#define ACTION_PLAY "play_music"
#define ACTION_PLAY_NAME "Play Music"
#define ACTION_PREVIOUS "previous_music"
#define ACTION_PREVIOUS_NAME "Previous Music"

static void
_actions_register(E_Music_Control_Module_Context *ctxt)
{
   E_Action *action;

   if (ctxt->actions_set) return;
   action = e_action_add(ACTION_NEXT);
   action->func.go = _music_control;
   e_action_predef_name_set(_e_music_control_Name, ACTION_NEXT_NAME,
                            ACTION_NEXT, "next", NULL, 0);
   action = e_action_add(ACTION_PLAY_PAUSE);
   action->func.go = _music_control;
   e_action_predef_name_set(_e_music_control_Name, ACTION_PLAY_PAUSE_NAME,
                            ACTION_PLAY_PAUSE, "play", NULL, 0);
   action = e_action_add(ACTION_PAUSE);
   action->func.go = _music_control;
   e_action_predef_name_set(_e_music_control_Name, ACTION_PAUSE_NAME,
                            ACTION_PAUSE, "pause", NULL, 0);
   action = e_action_add(ACTION_PLAY);
   action->func.go = _music_control;
   e_action_predef_name_set(_e_music_control_Name, ACTION_PLAY_NAME,
                            ACTION_PLAY, "play_music", NULL, 0);
   action = e_action_add(ACTION_PREVIOUS);
   action->func.go = _music_control;
   e_action_predef_name_set(_e_music_control_Name, ACTION_PREVIOUS_NAME,
                            ACTION_PREVIOUS, "previous", NULL, 0);
   ctxt->actions_set = EINA_TRUE;
}

static void
_actions_unregister(E_Music_Control_Module_Context *ctxt)
{
   if (!ctxt->actions_set) return;
   e_action_predef_name_del(ACTION_NEXT_NAME, ACTION_NEXT);
   e_action_del(ACTION_NEXT);
   e_action_predef_name_del(ACTION_PLAY_PAUSE_NAME, ACTION_PLAY_PAUSE);
   e_action_del(ACTION_PLAY_PAUSE);
   e_action_predef_name_del(ACTION_PREVIOUS_NAME, ACTION_PREVIOUS);
   e_action_del(ACTION_PREVIOUS);
   ctxt->actions_set = EINA_FALSE;
}

/* Gadcon Api Functions */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   E_Music_Control_Instance *inst;
   E_Music_Control_Module_Context *ctxt;

   EINA_SAFETY_ON_NULL_RETURN_VAL(music_control_mod, NULL);
   ctxt = music_control_mod->data;

   inst = calloc(1, sizeof(E_Music_Control_Instance));
   inst->ctxt = ctxt;
   inst->gadget = edje_object_add(gc->evas);
   e_theme_edje_object_set(inst->gadget, "base/theme/modules/music-control",
                           "e/modules/music-control/main");
   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->gadget);
   inst->gcc->data = inst;

   evas_object_event_callback_add(inst->gadget, EVAS_CALLBACK_MOUSE_DOWN,
                                  music_control_mouse_down_cb, inst);
   ctxt->instances = eina_list_append(ctxt->instances, inst);
   _actions_register(ctxt);
   return inst->gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   E_Music_Control_Instance *inst;
   E_Music_Control_Module_Context *ctxt;

   EINA_SAFETY_ON_NULL_RETURN(music_control_mod);
   ctxt = music_control_mod->data;
   inst = gcc->data;
   evas_object_del(inst->gadget);
   if (inst->popup) music_control_popup_del(inst);
   ctxt->instances = eina_list_remove(ctxt->instances, inst);
   if (!ctxt->instances) _actions_unregister(ctxt);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _(_e_music_control_Name);
}

static char tmpbuf[1024]; /* general purpose buffer, just use immediately */

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;
   EINA_SAFETY_ON_NULL_RETURN_VAL(music_control_mod, NULL);
   snprintf(tmpbuf, sizeof(tmpbuf), "%s/e-module-music-control.edj",
            e_module_dir_get(music_control_mod));
   o = edje_object_add(evas);
   edje_object_file_set(o, tmpbuf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   E_Music_Control_Module_Context *ctxt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(music_control_mod, NULL);
   ctxt = music_control_mod->data;
   snprintf(tmpbuf, sizeof(tmpbuf), "music-control.%d",
            eina_list_count(ctxt->instances));
   return tmpbuf;
}

static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, "music-control", {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

E_API E_Module_Api e_modapi = { E_MODULE_API_VERSION, _e_music_control_Name };

static void
parse_metadata(E_Music_Control_Module_Context *ctxt, Eina_Value *array)
{
   unsigned i;

   E_FREE_FUNC(ctxt->meta_title, eina_stringshare_del);
   E_FREE_FUNC(ctxt->meta_album, eina_stringshare_del);
   E_FREE_FUNC(ctxt->meta_artist, eina_stringshare_del);
   E_FREE_FUNC(ctxt->meta_cover, eina_stringshare_del);
   // DBG("Metadata: %s", eina_value_to_string(array));
   for (i = 0; i < eina_value_array_count(array); i++)
     {
        const char *key = NULL, *str_val;
        long long llval;
        char *str_markup;
        Eina_Value st, subst;
        Efreet_Uri *uri;

        eina_value_setup(&st, EINA_VALUE_TYPE_UINT64);
        eina_value_setup(&subst, EINA_VALUE_TYPE_UINT64);
        if (eina_value_array_value_get(array, i, &st) &&
            eina_value_struct_get(&st, "arg0", &key))
          {
             if (!strcmp(key, "xesam:title"))
               {
                  if (eina_value_struct_value_get(&st, "arg1", &subst) &&
                      eina_value_struct_get(&subst, "arg0", &str_val))
                    {
                       if ((str_markup = evas_textblock_text_utf8_to_markup(NULL, str_val)))
                         {
                            ctxt->meta_title = eina_stringshare_add(str_markup);
                            free(str_markup);
                         }
                    }
                  eina_value_flush(&subst);
               }
             else if (!strcmp(key, "xesam:album"))
               {
                  if (eina_value_struct_value_get(&st, "arg1", &subst) &&
                      eina_value_struct_get(&subst, "arg0", &str_val))
                    {
                       if ((str_markup = evas_textblock_text_utf8_to_markup(NULL, str_val)))
                         {
                            ctxt->meta_album = eina_stringshare_add(str_markup);
                            free(str_markup);
                         }
                    }
                  eina_value_flush(&subst);
               }
             else if (!strcmp(key, "xesam:artist"))
               {
                  Eina_Value arr;

                  eina_value_setup(&arr, EINA_VALUE_TYPE_UINT64);
                  if (eina_value_struct_value_get(&st, "arg1", &subst) &&
                      eina_value_struct_value_get(&subst, "arg0", &arr) &&
                      eina_value_array_get(&arr, 0, &str_val))
                    {
                       if ((str_markup = evas_textblock_text_utf8_to_markup(NULL, str_val)))
                         {
                            ctxt->meta_artist = eina_stringshare_add(str_markup);
                            free(str_markup);
                         }
                    }
                  eina_value_flush(&arr);
                  eina_value_flush(&subst);
               }
             else if (!strcmp(key, "mpris:artUrl"))
               {
                  if (eina_value_struct_value_get(&st, "arg1", &subst) &&
                      eina_value_struct_get(&subst, "arg0", &str_val))
                    {
                       uri = efreet_uri_decode(str_val);
                       if (uri && !strncmp(uri->protocol, "file", 4))
                         ctxt->meta_cover = eina_stringshare_add(uri->path);
                       else
                         {
                            if (!strncmp(str_val, "https://open.spotify.com/image/", 31))
                              {
                                 // fix spotify brokenness:
                                 //   https://open.spotify.com/image/ab67616d00001e02216a271725a34eba54dd0b55
                                 // becomes
                                 //   https://i.scdn.co/image/ab67616d00001e02216a271725a34eba54dd0b55
                                 char buf[1024];
                                 snprintf(buf, sizeof(buf), "https://i.scdn.co/image/%s", str_val + 31);
                                 ctxt->meta_cover = eina_stringshare_add(buf);
                              }
                            else
                              ctxt->meta_cover = eina_stringshare_add(str_val);
                         }
                       E_FREE_FUNC(uri, efreet_uri_free);
                    }
                  eina_value_flush(&subst);
               }
             else if (!strcmp(key, "mpris:length"))
               {
                  llval = -1;
                  if (eina_value_struct_value_get(&st, "arg1", &subst) &&
                      eina_value_struct_get(&subst, "arg0", &llval))
                    {
                       if (llval < 0) llval = 0;
                       ctxt->meta_length = (double)llval / 1000000.0;
                    }
                  eina_value_flush(&subst);
               }
             // FIXME: to handle in future:
             // xesam:url - s
          }
        eina_value_flush(&st);
     }
}

static void
cb_playback_status_get(void *data, Eldbus_Pending *p EINA_UNUSED,
                       const char *propname EINA_UNUSED,
                       Eldbus_Proxy *proxy EINA_UNUSED,
                       Eldbus_Error_Info *error_info, const char *value)
{
   E_Music_Control_Module_Context *ctxt = data;

   if (error_info)
     {
        printf("MUSIC-CONTROL: %s %s", error_info->error, error_info->message);
        return;
     }

   if (!strcmp(value, "Playing")) ctxt->playing = EINA_TRUE;
   else ctxt->playing = EINA_FALSE;
   music_control_state_update_all(ctxt);
}

static void
cb_metadata_get(void *data, Eldbus_Pending *p EINA_UNUSED,
                const char *propname EINA_UNUSED,
                Eldbus_Proxy *proxy EINA_UNUSED,
                Eldbus_Error_Info *error_info EINA_UNUSED, Eina_Value *value)
{
   E_Music_Control_Module_Context *ctxt = data;
   parse_metadata(ctxt, value);
   music_control_metadata_update_all(ctxt);
}

static void
prop_changed(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event_info)
{
   Eldbus_Proxy_Event_Property_Changed *event = event_info;
   E_Music_Control_Module_Context *ctxt = data;

   if (!strcmp(event->name, "PlaybackStatus"))
     {
        const Eina_Value *value = event->value;
        const char *status;

        eina_value_get(value, &status);
        if (!strcmp(status, "Playing")) ctxt->playing = EINA_TRUE;
        else ctxt->playing = EINA_FALSE;
        music_control_state_update_all(ctxt);
     }
   else if (!strcmp(event->name, "LoopStatus"))
     {
        const Eina_Value *value = event->value;
        const char *status;

        eina_value_get(value, &status);
        if (!strcmp(status, "None")) ctxt->loop = EINA_FALSE;
        else if (!strcmp(status, "Track")) ctxt->loop = EINA_TRUE;
        if (!strcmp(status, "Playlist")) ctxt->loop = EINA_TRUE;
        music_control_state_update_all(ctxt);
     }
   else if (!strcmp(event->name, "Metadata"))
     {
        parse_metadata(ctxt, (Eina_Value*)event->value);
        music_control_metadata_update_all(ctxt);
     }
   else if (!strcmp(event->name, "Position"))
     {
        const Eina_Value *value = event->value;
        long long llval = 0;

        eina_value_get(value, &llval);
        if (llval < 0) llval = 0;
        ctxt->position = (double)llval / 1000000.0;
        music_control_pos_update(ctxt);
     }
}

static void _bus_list(E_Music_Control_Module_Context *ctxt);

static Eina_Bool
_poll_dbus_cb(void *data)
{
   E_Music_Control_Module_Context *ctxt = data;
   ctxt->poll_timer = NULL;
   _bus_list(ctxt);
   return EINA_FALSE;
}

static void
_timer_redo(E_Music_Control_Module_Context *ctxt)
{
   E_Powersave_Mode pm;
   double t = 5.0;

   if (ctxt->poll_timer) ecore_timer_del(ctxt->poll_timer);
   ctxt->poll_timer = NULL;
   pm = e_powersave_mode_get();
   if (e_powersave_mode_screen_get() > pm) pm = e_powersave_mode_screen_get();
   if      (pm >= E_POWERSAVE_MODE_FREEZE)  t = 3600.0;
   else if (pm >= E_POWERSAVE_MODE_EXTREME) t =  600.0;
   else if (pm >= E_POWERSAVE_MODE_HIGH)    t =   10.0;
   else                                     t =    5.0;
   ctxt->poll_timer = ecore_timer_add(t, _poll_dbus_cb, ctxt);
}

static void
_bus_list_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   E_Music_Control_Module_Context *ctxt = data;
   Eldbus_Message_Iter *array = NULL;
   const char *bus;
   char *bus_name = NULL;
   int i, chosen = -1, auto_chosen = -1;

   ctxt->bus_list_pend = NULL;
   if (!eldbus_message_arguments_get(msg, "as", &array)) return;
   while (eldbus_message_iter_get_and_next(array, 's', &bus))
     {
        if ((!bus) || (bus[0] == ':')) continue;
        if ((ctxt->config) &&
            (ctxt->config->player_selected >= 0) &&
            (ctxt->config->player_selected < PLAYER_COUNT) &&
            (e_util_glob_match(bus, music_player_players[ctxt->config->player_selected].dbus_name)))
          {
             if (bus_name) free(bus_name);
             bus_name = strdup(bus);
             chosen = ctxt->config->player_selected;
          }
        else
          {
             for (i = 0; music_player_players[i].name; i++)
               {
                  if (e_util_glob_match(bus, music_player_players[i].dbus_name))
                    {
                       if (bus_name) free(bus_name);
                       bus_name = strdup(bus);
                       auto_chosen = i;
                       break;
                    }
               }
          }
     }
   if (chosen < 0) chosen = auto_chosen;
   if (chosen < 0)
     {
        eina_stringshare_del(ctxt->dbus_name);
        ctxt->dbus_name = NULL;
        _timer_redo(ctxt);
        return;
     }
   if ((!ctxt->dbus_name) ||
       (!bus_name) ||
       (!!strcmp(ctxt->dbus_name, bus_name)))
     {
        eina_stringshare_del(ctxt->dbus_name);
        ctxt->dbus_name = NULL;
        if (bus_name)
          {
             music_control_dbus_init(ctxt, bus_name);
             free(bus_name);
             if (ctxt->poll_timer) ecore_timer_del(ctxt->poll_timer);
             ctxt->poll_timer = NULL;
          }
        else _timer_redo(ctxt);
     }
}

static void
_bus_list(E_Music_Control_Module_Context *ctxt)
{
   if (ctxt->bus_list_pend) eldbus_pending_cancel(ctxt->bus_list_pend);
   ctxt->bus_list_pend = eldbus_names_list(ctxt->conn, _bus_list_cb, ctxt);
}

static void
cb_name_owner_changed(void *data,
                      const char *bus EINA_UNUSED,
                      const char *from EINA_UNUSED,
                      const char *to)
{
   E_Music_Control_Module_Context *ctxt = data;

   have_player = EINA_FALSE;
   if (to[0])
     {
        media_player2_player_playback_status_propget
          (ctxt->mpris2_player, cb_playback_status_get, ctxt);
        media_player2_player_metadata_propget
          (ctxt->mpris2_player, cb_metadata_get, ctxt);
        have_player = EINA_TRUE;
     }
   else
     _bus_list(ctxt);
}

void
music_control_launch(E_Music_Control_Instance *inst)
{
   E_Music_Control_Module_Context *ctxt;

   if (!music_control_mod) return;
   ctxt = music_control_mod->data;
   if (have_player) return;
   if (ctxt->config->player_selected < 0)
     {
        Efreet_Desktop *desktop;
        int i;

        for (i = 0; i < PLAYER_COUNT; i++)
          {
             desktop = efreet_util_desktop_exec_find
               (music_player_players[i].command);
             if (desktop)
               {
                  E_Zone *zone = e_gadcon_zone_get(inst->gcc->gadcon);
                  e_exec(zone, desktop, NULL/* command */,
                         NULL/* file list */, "module/music-control");
                  ctxt->config->player_selected = i;
                  music_control_dbus_init
                    (ctxt, music_player_players[i].dbus_name);
                  break;
               }
          }
     }
   else if (ctxt->config->player_selected < PLAYER_COUNT)
     {
        E_Zone *zone = e_gadcon_zone_get(inst->gcc->gadcon);
        e_exec(zone, NULL/* efreet desktop*/,
               music_player_players[ctxt->config->player_selected].command,
               NULL/* file list */, "module/music-control");
     }
}

static Eina_Bool
_powersave_change_cb(void *data, int type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Music_Control_Module_Context *ctxt = data;

   _timer_redo(ctxt);
   return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool
music_control_dbus_init(E_Music_Control_Module_Context *ctxt, const char *bus)
{
   if (!ctxt->conn)
     ctxt->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctxt->conn, EINA_FALSE);

   if (ctxt->mpris2_player) mpris_media_player2_proxy_unref(ctxt->mpris2_player);
   if (ctxt->mrpis2) media_player2_player_proxy_unref(ctxt->mrpis2);

   ctxt->mrpis2 = mpris_media_player2_proxy_get(ctxt->conn, bus, NULL);
   ctxt->mpris2_player = media_player2_player_proxy_get(ctxt->conn, bus, NULL);
   eldbus_proxy_event_callback_add(ctxt->mpris2_player,
                                   ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   prop_changed, ctxt);
   if (ctxt->dbus_name)
     {
        eldbus_name_owner_changed_callback_del
          (ctxt->conn, ctxt->dbus_name, cb_name_owner_changed, ctxt);
        eina_stringshare_del(ctxt->dbus_name);
     }
   ctxt->dbus_name = eina_stringshare_add(bus);
   eldbus_name_owner_changed_callback_add(ctxt->conn, bus,
                                          cb_name_owner_changed,
                                          ctxt, EINA_TRUE);
   return EINA_TRUE;
}

E_API void *
e_modapi_init(E_Module *m)
{
   E_Music_Control_Module_Context *ctxt;

   ctxt = calloc(1, sizeof(E_Music_Control_Module_Context));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctxt, NULL);
   music_control_mod = m;

   ctxt->conf_edd = E_CONFIG_DD_NEW("music_control_config", Music_Control_Config);
#undef T
#undef D
#define T Music_Control_Config
#define D ctxt->conf_edd
   E_CONFIG_VAL(D, T, player_selected, INT);
   E_CONFIG_VAL(D, T, pause_on_desklock, INT);
   ctxt->config = e_config_domain_load(MUSIC_CONTROL_DOMAIN, ctxt->conf_edd);
   if (!ctxt->config)
     {
        ctxt->config = calloc(1, sizeof(Music_Control_Config));
        ctxt->config->player_selected = -1;
     }

   if (ctxt->config->player_selected >= PLAYER_COUNT)
     ctxt->config->player_selected = PLAYER_COUNT - 1;
   if (!ctxt->conn)
     ctxt->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   _bus_list(ctxt);
   music_control_mod = m;

   e_gadcon_provider_register(&_gc_class);

   if (ctxt->config->pause_on_desklock)
     desklock_handler = ecore_event_handler_add(E_EVENT_DESKLOCK,
                                                _desklock_cb, ctxt);
   music_control_powersave_event_handler =
     ecore_event_handler_add(E_EVENT_POWERSAVE_UPDATE,
                             _powersave_change_cb, ctxt);
   return ctxt;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   E_Music_Control_Module_Context *ctxt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(music_control_mod, 0);
   ctxt = music_control_mod->data;

   ecore_event_handler_del(music_control_powersave_event_handler);
   music_control_powersave_event_handler = NULL;

   E_FREE_FUNC(ctxt->meta_title, eina_stringshare_del);
   E_FREE_FUNC(ctxt->meta_album, eina_stringshare_del);
   E_FREE_FUNC(ctxt->meta_artist, eina_stringshare_del);
   E_FREE_FUNC(ctxt->meta_cover, eina_stringshare_del);
   E_FREE_FUNC(ctxt->meta_cover_prev, eina_stringshare_del);

   if (ctxt->poll_timer) ecore_timer_del(ctxt->poll_timer);
   ctxt->poll_timer = NULL;

   free(ctxt->config);
   E_CONFIG_DD_FREE(ctxt->conf_edd);

   E_FREE_FUNC(desklock_handler, ecore_event_handler_del);

   if (ctxt->bus_list_pend)
     {
        eldbus_pending_cancel(ctxt->bus_list_pend);
        ctxt->bus_list_pend = NULL;
     }
   eldbus_name_owner_changed_callback_del
     (ctxt->conn, ctxt->dbus_name, cb_name_owner_changed, ctxt);
   eina_stringshare_del(ctxt->dbus_name);
   media_player2_player_proxy_unref(ctxt->mpris2_player);
   mpris_media_player2_proxy_unref(ctxt->mrpis2);
   eldbus_connection_unref(ctxt->conn);

   e_gadcon_provider_unregister(&_gc_class);

   if (eina_list_count(ctxt->instances))
     printf("MUSIC-CONTROL: Live instances.");

   free(ctxt);
   music_control_mod = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m)
{
   E_Music_Control_Module_Context *ctxt = m->data;
   e_config_domain_save(MUSIC_CONTROL_DOMAIN, ctxt->conf_edd, ctxt->config);
   return 1;
}
