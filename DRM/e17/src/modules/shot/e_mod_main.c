/**
 * @addtogroup Optional_Look
 * @{
 *
 * @defgroup Module_Shot Screenshot
 *
 * Takes screen shots and can submit them to http://enlightenment.org
 *
 * @}
 */
#include "e_mod_main.h"

E_Module *shot_module = NULL;

static E_Action *border_act = NULL, *delay_act = NULL, *act = NULL;
static E_Int_Menu_Augmentation *maug = NULL;
static Ecore_Timer *timer = NULL, *border_timer = NULL;
static Evas_Object *snap = NULL;
static E_Client_Menu_Hook *border_hook = NULL;
static E_Object_Delfn *delfn_client = NULL;
static E_Object_Delfn *delfn_zone = NULL;

static E_Client *shot_ec = NULL;
static E_Zone *shot_zone = NULL;
static char *shot_params;

static void
_cb_client_del(void *data EINA_UNUSED, void *obj EINA_UNUSED)
{
   if (delfn_client)
     {
        e_object_delfn_del(E_OBJECT(shot_ec), delfn_client);
        delfn_client = NULL;
     }
   if (delfn_zone)
     {
        e_object_delfn_del(E_OBJECT(shot_zone), delfn_zone);
        delfn_zone = NULL;
     }
   if (timer)
     {
        ecore_timer_del(timer);
        timer = NULL;
     }
   if (border_timer)
     {
        ecore_timer_del(border_timer);
        border_timer = NULL;
     }
   E_FREE_FUNC(snap, evas_object_del);
   E_FREE(shot_params);
}

static void
_cb_zone_del(void *data, void *obj)
{
   _cb_client_del(data, obj);
}

static void
_shot_post(void *buffer EINA_UNUSED, Evas *e EINA_UNUSED, void *event EINA_UNUSED)
{
   int x, y, w, h;
   evas_object_geometry_get(snap, &x, &y, &w, &h);
   evas_event_callback_del(e_comp->evas, EVAS_CALLBACK_RENDER_POST, _shot_post);
   preview_dialog_show(shot_zone, shot_ec, shot_params,
                       (void *)evas_object_image_data_get(snap, 0),
                       x, y, w, h);
   E_FREE_FUNC(snap, evas_object_del);
   if (delfn_client)
     {
        e_object_delfn_del(E_OBJECT(shot_ec), delfn_client);
        delfn_client = NULL;
     }
   if (delfn_zone)
     {
        e_object_delfn_del(E_OBJECT(shot_zone), delfn_zone);
        delfn_zone = NULL;
     }
   shot_ec = NULL;
   shot_zone = NULL;
   E_FREE(shot_params);
}

static void
_shot_now(E_Zone *zone, E_Client *ec, const char *params)
{
   int x, y, w, h;
   if (preview_have() || share_have() || (snap)) return;
   if ((!zone) && (!ec)) return;
   if (zone)
     {
        w = e_comp->w;
        h = e_comp->h;
        x = y = 0;
     }
   else
     {
        int pad = 0;

        if (params)
          {
             const char *p = strstr(params, "pad ");

             if (p)
               {
                  pad = atoi(p + 4);
                  if (pad < 0) pad = 0;
               }
          }
        x = ec->x - pad;
        y = ec->y - pad;
        w = ec->w + (pad * 2);
        h = ec->h + (pad * 2);
        E_RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, e_comp->w, e_comp->h);
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        if (x >= e_comp->w) x = e_comp->w - 1;
        if (y >= e_comp->h) y = e_comp->h - 1;
     }
   if (eina_streq(ecore_evas_engine_name_get(e_comp->ee), "buffer"))
     {
        preview_dialog_show(zone, ec, params,
                            (void *)ecore_evas_buffer_pixels_get(e_comp->ee),
                            x, y, w, h);
        if (delfn_client)
          {
             e_object_delfn_del(E_OBJECT(ec), delfn_client);
             delfn_client = NULL;
          }
        if (delfn_zone)
          {
             e_object_delfn_del(E_OBJECT(zone), delfn_zone);
             delfn_zone = NULL;
          }
        return;
     }
   shot_ec = ec;
   shot_zone = zone;
   shot_params = eina_strdup(params);
   snap = evas_object_image_filled_add(e_comp->evas);
   evas_object_pass_events_set(snap, 1);
   evas_object_layer_set(snap, EVAS_LAYER_MAX);
   evas_object_image_snapshot_set(snap, 1);
   evas_object_geometry_set(snap, x, y, w, h);
   evas_object_show(snap);
   evas_object_image_data_update_add(snap, 0, 0, w, h);
   evas_object_image_pixels_dirty_set(snap, 1);
   evas_event_callback_add(e_comp->evas, EVAS_CALLBACK_RENDER_POST, _shot_post, snap);
   ecore_evas_manual_render(e_comp->ee);
}

static Eina_Bool
_shot_delay(void *data)
{
   timer = NULL;
   _shot_now(data, NULL, NULL);
   return EINA_FALSE;
}

static Eina_Bool
_shot_delay_border_padded(void *data)
{
   char buf[128];

   border_timer = NULL;
   snprintf(buf, sizeof(buf), "pad %i", (int)(ELM_SCALE_SIZE(64)));
   _shot_now(NULL, data, buf);
   return EINA_FALSE;
}

static void
_shot_border_padded(E_Client *ec)
{
   if (border_timer) ecore_timer_del(border_timer);
   border_timer = ecore_timer_loop_add(1.0, _shot_delay_border_padded, ec);
   delfn_client = e_object_delfn_add(E_OBJECT(ec), _cb_client_del, NULL);
}

static void
_shot(E_Zone *zone)
{
   if (timer) ecore_timer_del(timer);
   timer = ecore_timer_loop_add(1.0, _shot_delay, zone);
   delfn_zone = e_object_delfn_add(E_OBJECT(zone), _cb_zone_del, NULL);
}

static void
_e_mod_menu_border_padded_cb(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   _shot_border_padded(data);
}

static void
_e_mod_menu_cb(void *data EINA_UNUSED, E_Menu *m, E_Menu_Item *mi EINA_UNUSED)
{
   if (m->zone) _shot(m->zone);
}

static void
_e_mod_action_border_cb(E_Object *obj EINA_UNUSED, const char *params)
{
   E_Client *ec;

   ec = e_client_focused_get();
   if (!ec) return;
   if (border_timer)
     {
        ecore_timer_del(border_timer);
        border_timer = NULL;
     }
   _shot_now(NULL, ec, params);
}

typedef struct
{
   E_Zone *zone;
   char *params;
} Delayed_Shot;

static void
_delayed_shot(void *data)
{
   Delayed_Shot *ds = data;

   _shot_now(ds->zone, NULL, ds->params);
   e_object_unref(E_OBJECT(ds->zone));
   free(ds->params);
   free(ds);
}

static Eina_Bool
_delayed_shot_timer(void *data)
{
   timer = NULL;
   _delayed_shot(data);
   return EINA_FALSE;
}

static void
_e_mod_action_delay_cb(E_Object *obj, const char *params)
{
   E_Zone *zone = NULL;
   Delayed_Shot *ds;
   double delay = 0.0;

   if (obj)
     {
        if (obj->type == E_COMP_TYPE) zone = e_zone_current_get();
        else if (obj->type == E_ZONE_TYPE) zone = ((void *)obj);
        else zone = e_zone_current_get();
     }
   if (!zone) zone = e_zone_current_get();
   if (!zone) return;
   E_FREE_FUNC(timer, ecore_timer_del);
   ds = E_NEW(Delayed_Shot, 1);
   e_object_ref(E_OBJECT(zone));
   ds->zone = zone;
   ds->params = params ? strdup(params) : NULL;
   if (params) delay = (double)atoi(params) / 1000.0;
   if (timer) ecore_timer_del(timer);
   timer = ecore_timer_loop_add(delay, _delayed_shot_timer, ds);
   delfn_zone = e_object_delfn_add(E_OBJECT(zone), _cb_zone_del, NULL);
}

static void
_e_mod_action_cb(E_Object *obj, const char *params)
{
   E_Zone *zone = NULL;
   Delayed_Shot *ds;

   if (obj)
     {
        if (obj->type == E_COMP_TYPE) zone = e_zone_current_get();
        else if (obj->type == E_ZONE_TYPE) zone = ((void *)obj);
        else zone = e_zone_current_get();
     }
   if (!zone) zone = e_zone_current_get();
   if (!zone) return;
   E_FREE_FUNC(timer, ecore_timer_del);
   ds = E_NEW(Delayed_Shot, 1);
   e_object_ref(E_OBJECT(zone));
   ds->zone = zone;
   ds->params = params ? strdup(params) : NULL;
   // forced main loop iteration in screenshots causes bugs if the action
   // executes immediately
   ecore_job_add(_delayed_shot, ds);
}

static void
_bd_hook(void *d EINA_UNUSED, E_Client *ec)
{
   E_Menu_Item *mi;
   E_Menu *m;
   Eina_List *l;

   if (!ec->border_menu) return;
   if (ec->iconic || (ec->desk != e_desk_current_get(ec->zone))) return;
   m = ec->border_menu;

   // position menu item just before first separator
   EINA_LIST_FOREACH(m->items, l, mi)
     {
        if (mi->separator) break;
     }
   if ((!mi) || (!mi->separator)) return;
   l = eina_list_prev(l);
   mi = eina_list_data_get(l);
   if (!mi) return;

   mi = e_menu_item_new_relative(m, mi);
   e_menu_item_label_set(mi, _("Take Shot"));
   e_util_menu_item_theme_icon_set(mi, "screenshot");
   e_menu_item_callback_set(mi, _e_mod_menu_border_padded_cb, ec);
}

static void
_e_mod_menu_add(void *data EINA_UNUSED, E_Menu *m)
{
   E_Menu_Item *mi;

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Take Screenshot"));
   e_util_menu_item_theme_icon_set(mi, "screenshot");
   e_menu_item_callback_set(mi, _e_mod_menu_cb, NULL);
}

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Shot"
};

E_API void *
e_modapi_init(E_Module *m)
{
   if (!ecore_con_url_init())
     {
        e_util_dialog_show(_("Shot Error"),
                           _("Cannot initialize network"));
        return NULL;
     }

   shot_module = m;
   act = e_action_add("shot");
   if (act)
     {
        act->func.go = _e_mod_action_cb;
        e_action_predef_name_set(N_("Screen"), N_("Take Screenshot"),
                                 "shot", NULL,
                                 "syntax: [share|save [perfect|high|medium|low|QUALITY current|all|SCREEN-NUM]", 1);
     }
   delay_act = e_action_add("shot_delay");
   if (delay_act)
     {
        delay_act->func.go = _e_mod_action_delay_cb;
        e_action_predef_name_set(N_("Screen"), N_("Take Screenshot with Delay"),
                                 "shot_delay", NULL,
                                 "syntax: delay_ms (e.g. 3000)", 1);
     }
   border_act = e_action_add("border_shot");
   if (border_act)
     {
        border_act->func.go = _e_mod_action_border_cb;
        e_action_predef_name_set(N_("Window : Actions"), N_("Take Shot"),
                                 "border_shot", NULL,
                                 "syntax: [share|save perfect|high|medium|low|QUALITY all|current] [pad N]", 1);
     }
   maug = e_int_menus_menu_augmentation_add_sorted
     ("main/2",  _("Take Screenshot"), _e_mod_menu_add, NULL, NULL, NULL);
   border_hook = e_int_client_menu_hook_add(_bd_hook, NULL);

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   share_abort();
   preview_abort();
   delay_abort();
   if (delfn_client)
     {
        e_object_delfn_del(E_OBJECT(shot_ec), delfn_client);
        delfn_client = NULL;
     }
   if (delfn_zone)
     {
        e_object_delfn_del(E_OBJECT(shot_zone), delfn_zone);
        delfn_zone = NULL;
     }
   if (timer)
     {
        ecore_timer_del(timer);
        timer = NULL;
     }
   if (border_timer)
     {
        ecore_timer_del(border_timer);
        border_timer = NULL;
     }
   E_FREE_FUNC(snap, evas_object_del);
   E_FREE(shot_params);
   if (maug)
     {
        e_int_menus_menu_augmentation_del("main/2", maug);
        maug = NULL;
     }
   if (act)
     {
        e_action_predef_name_del("Screen", "Take Screenshot");
        e_action_del("shot");
        act = NULL;
     }

   shot_module = NULL;
   e_int_client_menu_hook_del(border_hook);
   ecore_con_url_shutdown();
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}
