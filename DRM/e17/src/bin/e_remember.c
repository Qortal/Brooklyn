#include "e.h"

#define REMEMBER_HIERARCHY 1
#define REMEMBER_SIMPLE    0

E_API int E_EVENT_REMEMBER_UPDATE = -1;
E_API E_Config_DD *e_remember_edd = NULL; //created in e_config.c

typedef struct _E_Remember_List E_Remember_List;

struct _E_Remember_List
{
   Eina_List *list;
};

/* local subsystem functions */
static void        _e_remember_free(E_Remember *rem);
static void        _e_remember_update(E_Client *ec, E_Remember *rem);
static E_Remember *_e_remember_find(E_Client *ec, int check_usable, Eina_Bool sr);
static void        _e_remember_cb_hook_pre_post_fetch(void *data, E_Client *ec);
static void        _e_remember_cb_hook_eval_post_new_client(void *data, E_Client *ec);
static void        _e_remember_init_edd(void);
static Eina_Bool   _e_remember_restore_cb(void *data, int type, void *event);

/* local subsystem globals */
static Eina_List *hooks = NULL;
static E_Config_DD *e_remember_list_edd = NULL;
static E_Remember_List *remembers = NULL;
static Eina_List *handlers = NULL;
static Ecore_Idler *remember_idler = NULL;
static Eina_List *remember_idler_list = NULL;

/* static Eina_List *e_remember_restart_list = NULL; */

/* externally accessible functions */
EINTERN int
e_remember_init(E_Startup_Mode mode)
{
   Eina_List *ll, *l = NULL;
   E_Remember *rem;
   E_Client_Hook *h;

   if (mode == E_STARTUP_START)
     {
        EINA_LIST_FOREACH_SAFE(e_config->remembers, l, ll, rem)
          {
             if ((rem->apply & E_REMEMBER_APPLY_RUN) && (rem->prop.command))
               {
                  if (!e_util_exe_safe_run(rem->prop.command, NULL))
                    {
                       e_util_dialog_show(_("Run Error"),
                                          _("Enlightenment was unable to fork a child process:<ps/>"
                                            "<ps/>"
                                            "%s<ps/>"),
                                          rem->prop.command);
                    }
               }
             if (rem->apply & E_REMEMBER_APPLY_UUID)
               {
                  e_config->remembers = eina_list_remove_list(e_config->remembers, l);
                  _e_remember_free(rem);
               }
          }
     }
   E_EVENT_REMEMBER_UPDATE = ecore_event_type_new();

   h = e_client_hook_add(E_CLIENT_HOOK_EVAL_PRE_POST_FETCH,
                         _e_remember_cb_hook_pre_post_fetch, NULL);
   if (h) hooks = eina_list_append(hooks, h);
   h = e_client_hook_add(E_CLIENT_HOOK_EVAL_POST_NEW_CLIENT,
                         _e_remember_cb_hook_eval_post_new_client, NULL);
   if (h) hooks = eina_list_append(hooks, h);
   h = e_client_hook_add(E_CLIENT_HOOK_UNIGNORE,
                         _e_remember_cb_hook_pre_post_fetch, NULL);
   h = e_client_hook_add(E_CLIENT_HOOK_UNIGNORE,
                         _e_remember_cb_hook_eval_post_new_client, NULL);
   if (h) hooks = eina_list_append(hooks, h);

   _e_remember_init_edd();
   remembers = e_config_domain_load("e_remember_restart", e_remember_list_edd);

   if (remembers)
     {
        handlers = eina_list_append
            (handlers, ecore_event_handler_add
              (E_EVENT_MODULE_INIT_END, _e_remember_restore_cb, NULL));
     }

   return 1;
}

EINTERN int
e_remember_shutdown(void)
{
   E_FREE_LIST(hooks, e_client_hook_del);

   E_CONFIG_DD_FREE(e_remember_edd);
   E_CONFIG_DD_FREE(e_remember_list_edd);

   E_FREE_LIST(handlers, ecore_event_handler_del);
   if (remember_idler) ecore_idler_del(remember_idler);
   remember_idler = NULL;
   remember_idler_list = eina_list_free(remember_idler_list);

   return 1;
}

E_API void
e_remember_internal_save(void)
{
   const Eina_List *l;
   E_Client *ec;
   E_Remember *rem;

   //printf("internal save %d\n", restart);
   if (!remembers)
     remembers = E_NEW(E_Remember_List, 1);
   else
     {
        EINA_LIST_FREE(remembers->list, rem)
          _e_remember_free(rem);
        remember_idler_list = eina_list_free(remember_idler_list);
     }

   EINA_LIST_FOREACH(e_comp->clients, l, ec)
     {
        if ((!ec->internal) || e_client_util_ignored_get(ec)) continue;

        rem = E_NEW(E_Remember, 1);
        if (!rem) break;

        e_remember_default_match_set(rem, ec);
        rem->apply = (E_REMEMBER_APPLY_POS | E_REMEMBER_APPLY_SIZE |
                      E_REMEMBER_APPLY_BORDER | E_REMEMBER_APPLY_LAYER |
                      E_REMEMBER_APPLY_SHADE | E_REMEMBER_APPLY_ZONE |
                      E_REMEMBER_APPLY_DESKTOP | E_REMEMBER_APPLY_LOCKS |
                      E_REMEMBER_APPLY_SKIP_WINLIST |
                      E_REMEMBER_APPLY_SKIP_PAGER |
                      E_REMEMBER_APPLY_SKIP_TASKBAR |
                      E_REMEMBER_APPLY_OFFER_RESISTANCE |
                      E_REMEMBER_APPLY_OPACITY |
                      E_REMEMBER_APPLY_VOLUME);
        _e_remember_update(ec, rem);

        remembers->list = eina_list_append(remembers->list, rem);
     }

   e_config_domain_save("e_remember_restart", e_remember_list_edd, remembers);
}

static Eina_Bool
_e_remember_restore_idler_cb(void *d EINA_UNUSED)
{
   E_Remember *rem;
   E_Action *act_fm = NULL, *act;
   Eina_Bool done = EINA_FALSE;

   EINA_LIST_FREE(remember_idler_list, rem)
     {
        if (!rem->class) continue;
        if (rem->no_reopen) continue;
        if (done) break;

        if (!strncmp(rem->class, "e_fwin::", 8))
          {
             if (!act_fm)
               act_fm = e_action_find("fileman");
             if (!act_fm) continue;
             /* at least '/' */
             if (!rem->class[9]) continue;

             act_fm->func.go(NULL, rem->class + 8);
          }
        else if (!strncmp(rem->class, "_config::", 9))
          {
             char *param = NULL;
             char path[1024];
             const char *p;

             p = rem->class + 9;
             if ((param = strstr(p, "::")))
               {
                  snprintf(path, (param - p) + sizeof(char), "%s", p);
                  param = param + 2;
               }
             else
               snprintf(path, sizeof(path), "%s", p);

             if (e_configure_registry_exists(path))
               {
                  e_configure_registry_call(path, NULL, param);
               }
          }
        else if (!strcmp(rem->class, "_configure"))
          {
             /* TODO this is just for settings panel. it could also
                use e_configure_registry_item_add */
             /* ..or make a general path for window that are started
                by actions */

             act = e_action_find("configuration");
             if (act)
               act->func.go(NULL, NULL);
          }
        done = EINA_TRUE;
     }
   if (!done) remember_idler = NULL;

   return done;
}

static Eina_Bool
_e_remember_restore_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   handlers = eina_list_free(handlers);
   if (!remembers->list) return ECORE_CALLBACK_PASS_ON;
   remember_idler_list = eina_list_clone(remembers->list);
   remember_idler = ecore_idler_add(_e_remember_restore_idler_cb, NULL);
   return ECORE_CALLBACK_PASS_ON;
}

E_API E_Remember *
e_remember_new(void)
{
   E_Remember *rem;

   rem = E_NEW(E_Remember, 1);
   if (!rem) return NULL;
   e_config->remembers = eina_list_prepend(e_config->remembers, rem);
   return rem;
}

E_API int
e_remember_usable_get(E_Remember *rem)
{
   if ((rem->apply_first_only) && (rem->used_count > 0)) return 0;
   return 1;
}

E_API void
e_remember_use(E_Remember *rem)
{
   rem->used_count++;
   if (rem->version < E_REMEMBER_VERSION)
     {
        /* upgrade remembers as they get used */
        switch (rem->version)
          {
           case 0:
             rem->prop.opacity = 255; //don't let people wreck themselves with old configs
           //fall through
           default: break;
          }
        rem->version = E_REMEMBER_VERSION;
        e_config_save_queue();
     }
}

E_API void
e_remember_unuse(E_Remember *rem)
{
   if (rem->used_count) rem->used_count--;
}

E_API void
e_remember_del(E_Remember *rem)
{
   E_Client *ec;

   E_CLIENT_FOREACH(ec)
     {
        if ((ec->remember != rem) && (ec->sr_remember != rem)) continue;

        if (ec->remember == rem)
          ec->remember = NULL;
        else
          ec->sr_remember = NULL;
        e_remember_unuse(rem);
     }

   _e_remember_free(rem);
}

E_API void
e_remember_apply(E_Remember *rem, E_Client *ec)
{
   int temporary = 0;
   if (ec->internal && remembers && ec->icccm.class && ec->icccm.class[0])
     {
        Eina_List *l;
        EINA_LIST_FOREACH(remembers->list, l, rem)
          {
             if (rem->class && !strcmp(rem->class, ec->icccm.class))
               break;
          }
        if (rem)
          {
             temporary = 1;
             remembers->list = eina_list_remove(remembers->list, rem);
             remember_idler_list = eina_list_remove(remember_idler_list, rem);
             if (!remembers->list)
               e_config_domain_save("e_remember_restart",
                                    e_remember_list_edd, remembers);
          }
        else rem = ec->remember;
     }

   if (!rem)
     return;
   rem->applying = 1;
   if (rem->apply & E_REMEMBER_APPLY_ZONE)
     {
        E_Zone *zone;

        zone = e_comp_zone_number_get(rem->prop.zone);
        if (zone)
          e_client_zone_set(ec, zone);
     }
   if (rem->apply & E_REMEMBER_APPLY_DESKTOP)
     {
        E_Desk *desk;

        desk = e_desk_at_xy_get(ec->zone, rem->prop.desk_x, rem->prop.desk_y);
        if (desk)
          {
             if (ec->desk != desk)
               ec->hidden = 0;
             e_client_desk_set(ec, desk);
             if (e_config->desk_auto_switch)
               e_desk_show(desk);
          }
     }
   if (rem->apply & E_REMEMBER_APPLY_BORDER)
     {
        eina_stringshare_replace(&ec->bordername, NULL);
        ec->bordername = eina_stringshare_ref(rem->prop.border);
        ec->border.changed = 1;
        EC_CHANGED(ec);
     }
   if (rem->apply & E_REMEMBER_APPLY_SIZE)
     {
        int w, h;
        int uzx = 0, uzy = 0, uzw = 0, uzh = 0;

        e_zone_useful_geometry_get(ec->zone, &uzx, &uzy, &uzw, &uzh);

        w = ec->client.w;
        h = ec->client.h;
        if (rem->prop.pos_w) ec->client.w = rem->prop.pos_w;
        if (rem->prop.pos_h) ec->client.h = rem->prop.pos_h;
        if (uzw != rem->prop.res_x)
          {
             if (ec->client.w > (uzw - rem->prop.frame_w))
               ec->client.w = uzw - rem->prop.frame_w;
          }
        if (uzh != rem->prop.res_y)
          {
             if (ec->client.h > (uzh - rem->prop.frame_h))
               ec->client.h = uzh - rem->prop.frame_h;
          }
        if (ec->icccm.min_w > ec->client.w)
          ec->client.w = ec->icccm.min_w;
        if ((ec->icccm.max_w > 0) && (ec->icccm.max_w < ec->client.w))
          ec->client.w = ec->icccm.max_w;
        if (ec->icccm.min_h > ec->client.h)
          ec->client.h = ec->icccm.min_h;
        if ((ec->icccm.max_h > 0) && (ec->icccm.max_h < ec->client.h))
          ec->client.h = ec->icccm.max_h;
        e_comp_object_frame_wh_adjust(ec->frame, ec->client.w, ec->client.h,
                                      &ec->w, &ec->h);
        if (rem->prop.maximize)
          {
             ec->saved.x = rem->prop.pos_x;
             ec->saved.y = rem->prop.pos_y;
             ec->saved.w = ec->client.w;
             ec->saved.h = ec->client.h;
             ec->maximized = rem->prop.maximize | e_config->maximize_policy;
             ec->changes.need_maximize = 1;
          }
        if ((w != ec->client.w) || (h != ec->client.h))
          {
             ec->changes.size = 1;
             ec->changes.shape = 1;
             EC_CHANGED(ec);
          }
     }
   if ((rem->apply & E_REMEMBER_APPLY_POS) && (!ec->re_manage))
     {
        int uzx = 0, uzy = 0, uzw = 0, uzh = 0;

        e_zone_useful_geometry_get(ec->zone, &uzx, &uzy, &uzw, &uzh);
        ec->x = rem->prop.pos_x;
        ec->y = rem->prop.pos_y;
        if (uzw != rem->prop.res_x)
          {
             int px;

             px = uzw - (ec->w + rem->prop.frame_w);
             if (px < 1) px = 0;
             else px = (3 * ec->x) / px;
             if (px < 1)
               {
                  if (uzw >= (rem->prop.res_x / 3))
                    ec->x = rem->prop.pos_x;
                  else
                    ec->x = ((rem->prop.pos_x - 0) * uzw) /
                      (rem->prop.res_x / 3);
               }
             else if (px == 1)
               {
                  if (uzw >= (rem->prop.res_x / 3))
                    ec->x = (uzw / 2) +
                      (px - (rem->prop.res_x / 2)) -
                      (ec->w / 2);
                  else
                    ec->x = (uzw / 2) +
                      (((px - (rem->prop.res_x / 2)) * uzw) /
                       (rem->prop.res_x / 3)) -
                      (ec->w / 2);
               }
             else // >= 2
               {
                  if (uzw >= (rem->prop.res_x / 3))
                    ec->x = uzw +
                      rem->prop.pos_x - rem->prop.res_x +
                      (rem->prop.w - ec->client.w);
                  else
                    ec->x = uzw +
                      (((rem->prop.pos_x - rem->prop.res_x) * uzw) /
                       (rem->prop.res_x / 3)) +
                      (rem->prop.w - ec->client.w);
               }
             if ((rem->prop.pos_x >= 0) && (ec->x < 0))
               ec->x = 0;
             else if (((rem->prop.pos_x + rem->prop.w) < rem->prop.res_x) &&
                      ((ec->x + ec->client.w + rem->prop.frame_w) > (uzw - (uzx - ec->zone->x))))
               ec->x = (uzw - (uzx - ec->zone->x)) - ec->client.w - rem->prop.frame_w;
          }
        if (uzh != rem->prop.res_y)
          {
             int py;

             py = uzh - (ec->h + rem->prop.frame_h);
             if (py < 1) py = 0;
             else py = (3 * ec->y) / py;
             if (py < 1)
               {
                  if (uzh >= (rem->prop.res_y / 3))
                    ec->y = rem->prop.pos_y;
                  else
                    ec->y = ((rem->prop.pos_y - 0) * uzh) /
                      (rem->prop.res_y / 3);
               }
             else if (py == 1)
               {
                  if (uzh >= (rem->prop.res_y / 3))
                    ec->y = (uzh / 2) +
                      (py - (rem->prop.res_y / 2)) -
                      (ec->h / 2);
                  else
                    ec->y = (uzh / 2) +
                      (((py - (rem->prop.res_y / 2)) * uzh) /
                       (rem->prop.res_y / 3)) -
                      (ec->h / 2);
               }
             else // >= 2
               {
                  if (uzh >= (rem->prop.res_y / 3))
                    ec->y = uzh +
                      rem->prop.pos_y - rem->prop.res_y +
                      (rem->prop.h - ec->client.h);
                  else
                    ec->y = uzh +
                      (((rem->prop.pos_y - rem->prop.res_y) * uzh) /
                       (rem->prop.res_y / 3)) +
                      (rem->prop.h - ec->client.h);
               }
             if ((rem->prop.pos_y >= 0) && (ec->y < 0))
               ec->y = 0;
             else if (((rem->prop.pos_y + rem->prop.h) < rem->prop.res_y) &&
                      ((ec->y + ec->client.h + rem->prop.frame_h) > (uzh - (uzy - ec->zone->y))))
               ec->y = (uzh - (uzy - ec->zone->y)) - ec->client.h - rem->prop.frame_h;
          }
        //		  if (ec->zone->w != rem->prop.res_x)
        //		    ec->x = (rem->prop.pos_x * ec->zone->w) / rem->prop.res_x;
        //		  if (ec->zone->h != rem->prop.res_y)
        //		    ec->y = (rem->prop.pos_y * ec->zone->h) / rem->prop.res_y;
        if (
          /* upper left */
          (!E_INSIDE(ec->x, ec->y, 0, 0, ec->zone->w, ec->zone->h)) &&
          /* upper right */
          (!E_INSIDE(ec->x + ec->w, ec->y, 0, 0, ec->zone->w, ec->zone->h)) &&
          /* lower left */
          (!E_INSIDE(ec->x, ec->y + ec->h, 0, 0, ec->zone->w, ec->zone->h)) &&
          /* lower right */
          (!E_INSIDE(ec->x + ec->w, ec->y + ec->h, 0, 0, ec->zone->w, ec->zone->h))
          )
          {
             e_comp_object_util_center_pos_get(ec->frame, &ec->x, &ec->y);
             rem->prop.pos_x = ec->x;
             rem->prop.pos_y = ec->y;
          }
        ec->x += uzx;
        ec->y += uzy;
        ec->placed = 1;
        ec->changes.pos = 1;
        EC_CHANGED(ec);
     }
   if (rem->apply & E_REMEMBER_APPLY_LAYER)
     {
        evas_object_layer_set(ec->frame, rem->prop.layer);
     }
   if (rem->apply & E_REMEMBER_APPLY_FULLSCREEN)
     {
        if (rem->prop.fullscreen)
          e_client_fullscreen(ec, e_config->fullscreen_policy);
     }
   if (rem->apply & E_REMEMBER_APPLY_STICKY)
     {
        if (rem->prop.sticky) e_client_stick(ec);
     }
   if (rem->apply & E_REMEMBER_APPLY_SHADE)
     {
        if (rem->prop.shaded >= 100)
          e_client_shade(ec, rem->prop.shaded - 100);
        else if (rem->prop.shaded >= 50)
          e_client_unshade(ec, rem->prop.shaded - 50);
     }
   if (rem->apply & E_REMEMBER_APPLY_LOCKS)
     {
        ec->lock_user_location = rem->prop.lock_user_location;
        ec->lock_client_location = rem->prop.lock_client_location;
        ec->lock_user_size = rem->prop.lock_user_size;
        ec->lock_client_size = rem->prop.lock_client_size;
        ec->lock_user_stacking = rem->prop.lock_user_stacking;
        ec->lock_client_stacking = rem->prop.lock_client_stacking;
        ec->lock_user_iconify = rem->prop.lock_user_iconify;
        ec->lock_client_iconify = rem->prop.lock_client_iconify;
        ec->lock_user_desk = rem->prop.lock_user_desk;
        ec->lock_client_desk = rem->prop.lock_client_desk;
        ec->lock_user_sticky = rem->prop.lock_user_sticky;
        ec->lock_client_sticky = rem->prop.lock_client_sticky;
        ec->lock_user_shade = rem->prop.lock_user_shade;
        ec->lock_client_shade = rem->prop.lock_client_shade;
        ec->lock_user_maximize = rem->prop.lock_user_maximize;
        ec->lock_client_maximize = rem->prop.lock_client_maximize;
        ec->lock_user_fullscreen = rem->prop.lock_user_fullscreen;
        ec->lock_client_fullscreen = rem->prop.lock_client_fullscreen;
        ec->lock_border = rem->prop.lock_border;
        ec->lock_close = rem->prop.lock_close;
        ec->lock_focus_in = rem->prop.lock_focus_in;
        ec->lock_focus_out = rem->prop.lock_focus_out;
        ec->lock_life = rem->prop.lock_life;
     }
   if (rem->apply & E_REMEMBER_APPLY_SKIP_WINLIST)
     ec->user_skip_winlist = rem->prop.skip_winlist;
   if (rem->apply & E_REMEMBER_APPLY_SKIP_PAGER)
     ec->netwm.state.skip_pager = rem->prop.skip_pager;
   if (rem->apply & E_REMEMBER_APPLY_SKIP_TASKBAR)
     ec->netwm.state.skip_taskbar = rem->prop.skip_taskbar;
   if (rem->apply & E_REMEMBER_APPLY_ICON_PREF)
     ec->icon_preference = rem->prop.icon_preference;
   if (rem->apply & E_REMEMBER_APPLY_OFFER_RESISTANCE)
     ec->offer_resistance = rem->prop.offer_resistance;
   if (rem->apply & E_REMEMBER_SET_FOCUS_ON_START)
     ec->want_focus = 1;
   if (rem->apply & E_REMEMBER_APPLY_OPACITY)
     ec->netwm.opacity = rem->prop.opacity;
   if (rem->apply & E_REMEMBER_APPLY_VOLUME)
     {
        if (!ec->volume_control_enabled)
          {
             ec->volume_control_enabled = EINA_TRUE;
             ec->volume = rem->prop.volume;
             ec->volume_min = rem->prop.volume_min;
             ec->volume_max = rem->prop.volume_max;
             ec->mute = rem->prop.mute;
          }
        else
          {
             e_client_volume_set(ec, rem->prop.volume);
             e_client_volume_mute_set(ec, rem->prop.mute);
          }
     }

   rem->applying = 0;
   if (temporary)
     _e_remember_free(rem);
}

E_API E_Remember *
e_remember_find_usable(E_Client *ec)
{
   E_Remember *rem;

   rem = _e_remember_find(ec, 1, 0);
   return rem;
}

E_API E_Remember *
e_remember_find(E_Client *ec)
{
   E_Remember *rem;

   rem = _e_remember_find(ec, 0, 0);
   return rem;
}

E_API E_Remember *
e_remember_sr_find(E_Client *ec)
{
   return _e_remember_find(ec, 1, 1);
}

E_API void
e_remember_match_update(E_Remember *rem)
{
   int max_count = 0;

   if (rem->match & E_REMEMBER_MATCH_NAME) max_count += 2;
   if (rem->match & E_REMEMBER_MATCH_CLASS) max_count += 2;
   if (rem->match & E_REMEMBER_MATCH_TITLE) max_count += 2;
   if (rem->match & E_REMEMBER_MATCH_ROLE) max_count += 2;
   if (rem->match & E_REMEMBER_MATCH_TYPE) max_count += 2;
   if (rem->match & E_REMEMBER_MATCH_TRANSIENT) max_count += 2;
   if (rem->apply_first_only) max_count++;

   if (max_count != rem->max_score)
     {
        /* The number of matches for this remember has changed so we
         * need to remove from list and insert back into the appropriate
         * location. */
        Eina_List *l = NULL;
        E_Remember *r;

        rem->max_score = max_count;
        e_config->remembers = eina_list_remove(e_config->remembers, rem);

        EINA_LIST_FOREACH(e_config->remembers, l, r)
          {
             if (r->max_score <= rem->max_score) break;
          }

        if (l)
          e_config->remembers = eina_list_prepend_relative_list(e_config->remembers, rem, l);
        else
          e_config->remembers = eina_list_append(e_config->remembers, rem);
     }
}

E_API int
e_remember_default_match_set(E_Remember *rem, E_Client *ec)
{
   const char *title, *clasz, *name, *role;
   int match;

   eina_stringshare_replace(&rem->name, NULL);
   eina_stringshare_replace(&rem->class, NULL);
   eina_stringshare_replace(&rem->title, NULL);
   eina_stringshare_replace(&rem->role, NULL);

   name = ec->icccm.name;
   if (!name || name[0] == 0) name = NULL;
   clasz = ec->icccm.class;
   if (!clasz || clasz[0] == 0) clasz = NULL;
   role = ec->icccm.window_role;
   if (!role || role[0] == 0) role = NULL;

   match = E_REMEMBER_MATCH_TRANSIENT;
   if (ec->icccm.transient_for != 0)
     rem->transient = 1;
   else
     rem->transient = 0;

   if (name && clasz)
     {
        match |= E_REMEMBER_MATCH_NAME | E_REMEMBER_MATCH_CLASS;
        rem->name = eina_stringshare_ref(name);
        rem->class = eina_stringshare_ref(clasz);
     }
   else if ((title = e_client_util_name_get(ec)) && title[0])
     {
        match |= E_REMEMBER_MATCH_TITLE;
        rem->title = eina_stringshare_ref(title);
     }
   if (role)
     {
        match |= E_REMEMBER_MATCH_ROLE;
        rem->role = eina_stringshare_ref(role);
     }
   if (ec->netwm.type != E_WINDOW_TYPE_UNKNOWN)
     {
        match |= E_REMEMBER_MATCH_TYPE;
        rem->type = ec->netwm.type;
     }

   rem->match = match;

   return match;
}

E_API void
e_remember_update(E_Client *ec)
{
   if (((!ec->remember) || ec->remember->keep_settings) && (!ec->sr_remember)) return;
   if (ec->remember) _e_remember_update(ec, ec->remember);
   if (ec->sr_remember) _e_remember_update(ec, ec->sr_remember);
   e_config_save_queue();
}

static void
_e_remember_event_free(void *d EINA_UNUSED, void *event)
{
   E_Event_Remember_Update *ev = event;
   UNREFD(ev->ec, 10);
   e_object_unref(E_OBJECT(ev->ec));
   free(ev);
}

static void
_e_remember_update(E_Client *ec, E_Remember *rem)
{
   if (rem->applying) return;
   if (rem->apply & E_REMEMBER_APPLY_POS ||
       rem->apply & E_REMEMBER_APPLY_SIZE)
     {
        int uzx = 0, uzy = 0, uxw = 0, uzh = 0;

        e_zone_useful_geometry_get(ec->zone, &uzx, &uzy, &uxw, &uzh);
        if (ec->fullscreen || ec->maximized)
          {
             rem->prop.pos_x = ec->saved.x - uzx;
             rem->prop.pos_y = ec->saved.y - uzy;
             rem->prop.pos_w = ec->saved.w;
             rem->prop.pos_h = ec->saved.h;
             rem->prop.frame_w = ec->w - ec->client.w;
             rem->prop.frame_h = ec->h - ec->client.h;
          }
        else
          {
             rem->prop.pos_x = ec->x - uzx;
             rem->prop.pos_y = ec->y - uzy;
             rem->prop.res_x = uxw;
             rem->prop.res_y = uzh;
             rem->prop.pos_w = ec->client.w;
             rem->prop.pos_h = ec->client.h;
             rem->prop.w = ec->client.w;
             rem->prop.h = ec->client.h;
             rem->prop.frame_w = ec->w - ec->client.w;
             rem->prop.frame_h = ec->h - ec->client.h;
          }
        rem->prop.maximize = ec->maximized & E_MAXIMIZE_DIRECTION;
     }
   if (rem->apply & E_REMEMBER_APPLY_LAYER)
     {
        if (ec->fullscreen)
          rem->prop.layer = ec->saved.layer;
        else
          rem->prop.layer = ec->layer;
     }
   if (rem->apply & E_REMEMBER_APPLY_LOCKS)
     {
        rem->prop.lock_user_location = ec->lock_user_location;
        rem->prop.lock_client_location = ec->lock_client_location;
        rem->prop.lock_user_size = ec->lock_user_size;
        rem->prop.lock_client_size = ec->lock_client_size;
        rem->prop.lock_user_stacking = ec->lock_user_stacking;
        rem->prop.lock_client_stacking = ec->lock_client_stacking;
        rem->prop.lock_user_iconify = ec->lock_user_iconify;
        rem->prop.lock_client_iconify = ec->lock_client_iconify;
        rem->prop.lock_user_desk = ec->lock_user_desk;
        rem->prop.lock_client_desk = ec->lock_client_desk;
        rem->prop.lock_user_sticky = ec->lock_user_sticky;
        rem->prop.lock_client_sticky = ec->lock_client_sticky;
        rem->prop.lock_user_shade = ec->lock_user_shade;
        rem->prop.lock_client_shade = ec->lock_client_shade;
        rem->prop.lock_user_maximize = ec->lock_user_maximize;
        rem->prop.lock_client_maximize = ec->lock_client_maximize;
        rem->prop.lock_user_fullscreen = ec->lock_user_fullscreen;
        rem->prop.lock_client_fullscreen = ec->lock_client_fullscreen;
        rem->prop.lock_border = ec->lock_border;
        rem->prop.lock_close = ec->lock_close;
        rem->prop.lock_focus_in = ec->lock_focus_in;
        rem->prop.lock_focus_out = ec->lock_focus_out;
        rem->prop.lock_life = ec->lock_life;
     }
   if (rem->apply & E_REMEMBER_APPLY_SHADE)
     {
        if (ec->shaded)
          rem->prop.shaded = (100 + ec->shade_dir);
        else
          rem->prop.shaded = (50 + ec->shade_dir);
     }
   if (rem->apply & E_REMEMBER_APPLY_ZONE)
     {
        rem->prop.zone = ec->zone->num;
     }
   if (rem->apply & E_REMEMBER_APPLY_SKIP_WINLIST)
     rem->prop.skip_winlist = ec->user_skip_winlist;
   if (rem->apply & E_REMEMBER_APPLY_STICKY)
     rem->prop.sticky = ec->sticky;
   if (rem->apply & E_REMEMBER_APPLY_SKIP_PAGER)
     rem->prop.skip_pager = ec->netwm.state.skip_pager;
   if (rem->apply & E_REMEMBER_APPLY_SKIP_TASKBAR)
     rem->prop.skip_taskbar = ec->netwm.state.skip_taskbar;
   if (rem->apply & E_REMEMBER_APPLY_ICON_PREF)
     rem->prop.icon_preference = ec->icon_preference;
   if (rem->apply & E_REMEMBER_APPLY_DESKTOP)
     e_desk_xy_get(ec->desk, &rem->prop.desk_x, &rem->prop.desk_y);
   if (rem->apply & E_REMEMBER_APPLY_FULLSCREEN)
     rem->prop.fullscreen = ec->fullscreen;
   if (rem->apply & E_REMEMBER_APPLY_OFFER_RESISTANCE)
     rem->prop.offer_resistance = ec->offer_resistance;
   if (rem->apply & E_REMEMBER_APPLY_OPACITY)
     rem->prop.opacity = ec->netwm.opacity;
   if (rem->apply & E_REMEMBER_APPLY_BORDER)
     {
        if (ec->borderless)
          eina_stringshare_replace(&rem->prop.border, "borderless");
        else
          eina_stringshare_replace(&rem->prop.border, ec->bordername);
     }
   if (rem->apply & E_REMEMBER_APPLY_UUID)
     {
        eina_stringshare_refplace(&rem->uuid, ec->uuid);
        rem->pid = ec->netwm.pid;
        rem->apply_first_only = 1;
     }
   if (rem->apply & E_REMEMBER_APPLY_VOLUME)
     {
        rem->prop.volume = ec->volume;
        rem->prop.volume_min = ec->volume_min;
        rem->prop.volume_max = ec->volume_max;
        rem->prop.mute = ec->mute;
     }

   rem->no_reopen = ec->internal_no_reopen;
   {
      E_Event_Remember_Update *ev;

      ev = malloc(sizeof(E_Event_Remember_Update));
      if (!ev) return;
      ev->ec = ec;
      REFD(ec, 10);
      e_object_ref(E_OBJECT(ec));
      ecore_event_add(E_EVENT_REMEMBER_UPDATE, ev, _e_remember_event_free, NULL);
   }
}

/* local subsystem functions */
static E_Remember *
_e_remember_find(E_Client *ec, int check_usable, Eina_Bool sr)
{
   Eina_List *l = NULL;
   E_Remember *rem;

#if REMEMBER_SIMPLE
   EINA_LIST_FOREACH(e_config->remembers, l, rem)
     {
        int required_matches;
        int matches;
        const char *title = "";

        matches = 0;
        required_matches = 0;
        if (rem->match & E_REMEMBER_MATCH_NAME) required_matches++;
        if (rem->match & E_REMEMBER_MATCH_CLASS) required_matches++;
        if (rem->match & E_REMEMBER_MATCH_TITLE) required_matches++;
        if (rem->match & E_REMEMBER_MATCH_ROLE) required_matches++;
        if (rem->match & E_REMEMBER_MATCH_TYPE) required_matches++;
        if (rem->match & E_REMEMBER_MATCH_TRANSIENT) required_matches++;

        if (ec->netwm.name) title = ec->netwm.name;
        else title = ec->icccm.title;

        if ((rem->match & E_REMEMBER_MATCH_NAME) &&
            ((!e_util_strcmp(rem->name, ec->icccm.name)) ||
             (e_util_both_str_empty(rem->name, ec->icccm.name))))
          matches++;
        if ((rem->match & E_REMEMBER_MATCH_CLASS) &&
            ((!e_util_strcmp(rem->class, ec->icccm.class)) ||
             (e_util_both_str_empty(rem->class, ec->icccm.class))))
          matches++;
        if ((rem->match & E_REMEMBER_MATCH_TITLE) &&
            ((!e_util_strcmp(rem->title, title)) ||
             (e_util_both_str_empty(rem->title, title))))
          matches++;
        if ((rem->match & E_REMEMBER_MATCH_ROLE) &&
            ((!e_util_strcmp(rem->role, ec->icccm.window_role)) ||
             (e_util_both_str_empty(rem->role, ec->icccm.window_role))))
          matches++;
        if ((rem->match & E_REMEMBER_MATCH_TYPE) &&
            (rem->type == ec->netwm.type))
          matches++;
        if ((rem->match & E_REMEMBER_MATCH_TRANSIENT) &&
            (((rem->transient) && (ec->icccm.transient_for != 0)) ||
             ((!rem->transient) && (ec->icccm.transient_for == 0))))
          matches++;
        if (matches >= required_matches)
          return rem;
     }
   return NULL;
#endif
#if REMEMBER_HIERARCHY
   /* This search method finds the best possible match available and is
    * based on the fact that the list is sorted, with those remembers
    * with the most possible matches at the start of the list. This
    * means, as soon as a valid match is found, it is a match
    * within the set of best possible matches. */
   EINA_LIST_FOREACH(e_config->remembers, l, rem)
     {
        const char *title = "";

        if ((check_usable) && (!e_remember_usable_get(rem)))
          continue;

        if (sr)
          {
             if (!eina_streq(rem->uuid, ec->uuid)) continue;
             if (rem->uuid)
               {
                  if (rem->pid != ec->netwm.pid) continue;
                  return rem;
               }
          }
        else if (rem->apply & E_REMEMBER_APPLY_UUID) continue;

        if (ec->netwm.name) title = ec->netwm.name;
        else title = ec->icccm.title;

        /* For each type of match, check whether the match is
         * required, and if it is, check whether there's a match. If
         * it fails, then go to the next remember */
        if (rem->match & E_REMEMBER_MATCH_NAME &&
            !e_util_glob_match(ec->icccm.name, rem->name))
          continue;
        if (rem->match & E_REMEMBER_MATCH_CLASS &&
            !e_util_glob_match(ec->icccm.class, rem->class))
          continue;
        if (rem->match & E_REMEMBER_MATCH_TITLE &&
            !e_util_glob_match(title, rem->title))
          continue;
        if (rem->match & E_REMEMBER_MATCH_ROLE &&
            e_util_strcmp(rem->role, ec->icccm.window_role) &&
            !e_util_both_str_empty(rem->role, ec->icccm.window_role))
          continue;
        if (rem->match & E_REMEMBER_MATCH_TYPE &&
            rem->type != (int)ec->netwm.type)
          continue;
        if (rem->match & E_REMEMBER_MATCH_TRANSIENT &&
            !(rem->transient && ec->icccm.transient_for != 0) &&
            !(!rem->transient) && (ec->icccm.transient_for == 0))
          continue;

        return rem;
     }

   return NULL;
#endif
}

static void
_e_remember_free(E_Remember *rem)
{
   e_config->remembers = eina_list_remove(e_config->remembers, rem);
   if (rem->name) eina_stringshare_del(rem->name);
   if (rem->class) eina_stringshare_del(rem->class);
   if (rem->title) eina_stringshare_del(rem->title);
   if (rem->role) eina_stringshare_del(rem->role);
   if (rem->prop.border) eina_stringshare_del(rem->prop.border);
   if (rem->prop.command) eina_stringshare_del(rem->prop.command);
   if (rem->prop.desktop_file) eina_stringshare_del(rem->prop.desktop_file);
   eina_stringshare_del(rem->uuid);
   free(rem);
}

static void
_e_remember_cb_hook_eval_post_new_client(void *data EINA_UNUSED, E_Client *ec)
{
   // remember only when window was modified
   // if (!ec->new_client) return;
   if (e_client_util_ignored_get(ec)) return;
   if ((ec->internal) && (!ec->remember) &&
       (e_config->remember_internal_windows) &&
       (!ec->internal_no_remember) &&
       (ec->icccm.class && ec->icccm.class[0]))
     {
        E_Remember *rem;

        if (!strncmp(ec->icccm.class, "e_fwin", 6))
          {
             if (!e_config->remember_internal_fm_windows) return;
          }
        else
          {
             if (!e_config->remember_internal_windows)
               return;
          }

        rem = e_remember_new();
        if (!rem) return;

        e_remember_default_match_set(rem, ec);

        rem->apply = E_REMEMBER_APPLY_POS | E_REMEMBER_APPLY_SIZE | E_REMEMBER_APPLY_BORDER;

        e_remember_use(rem);
        ec->remember = rem;
        e_remember_update(ec);
     }
}

static void
_e_remember_cb_hook_pre_post_fetch(void *data EINA_UNUSED, E_Client *ec)
{
   E_Remember *rem = NULL;

   if ((!ec->new_client) || ec->internal_no_remember || e_client_util_ignored_get(ec)) return;

   if (!ec->remember)
     {
        rem = e_remember_find_usable(ec);
        if (rem)
          {
             ec->remember = rem;
             e_remember_use(rem);
          }
     }

   e_remember_apply(rem, ec);
}

static void
_e_remember_init_edd(void)
{
   e_remember_list_edd = E_CONFIG_DD_NEW("E_Remember_List", E_Remember_List);
#undef T
#undef D
#define T E_Remember_List
#define D e_remember_list_edd
   E_CONFIG_LIST(D, T, list, e_remember_edd);
#undef T
#undef D
}

