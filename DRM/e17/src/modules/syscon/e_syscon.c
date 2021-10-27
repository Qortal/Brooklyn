#include "e.h"
#include "e_mod_main.h"

/* local subsystem functions */
static Eina_Bool _cb_key_down(EINA_UNUSED void *data, Ecore_Event_Key *ev);
static void      _cb_signal_close(void *data, Evas_Object *obj, const char *emission, const char *source);
static void      _cb_signal_syscon(void *data, Evas_Object *obj, const char *emission, const char *source);
static void      _cb_signal_action(void *data, Evas_Object *obj, const char *emission, const char *source);
static void      _cb_signal_action_extra(void *data, Evas_Object *obj, const char *emission, const char *source);
static Eina_Bool _cb_timeout_defaction(void *data);

/* local subsystem globals */
static Evas_Object *popup = NULL;
static const char *do_defact = NULL;
static Eina_List *handlers = NULL;
static Evas_Object *o_bg = NULL;
static Evas_Object *o_flow_main = NULL;
static Evas_Object *o_flow_secondary = NULL;
static Evas_Object *o_flow_extra = NULL;
static Evas_Object *o_selected_flow = NULL;
static Evas_Object *o_selected = NULL;
static int inevas = 0;
static Ecore_Timer *deftimer = NULL;
static double show_time = 0.0;
static int act_count = 0;

static void
_cb_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{
   e_syscon_hide();
}

/* externally accessible functions */
int
e_syscon_init(void)
{
   return 1;
}

int
e_syscon_shutdown(void)
{
   e_syscon_hide();
   return 1;
}

int
e_syscon_show(E_Zone *zone, const char *defact)
{
   Evas_Object *o, *o2;
   Evas_Coord mw, mh;
   int x, y, w, h, zx, zy, zw, zh;
   int iw, ih;
   Eina_List *l;
   double t;
   Evas *evas;

   t = ecore_loop_time_get();
   if (popup)
     {
        if ((t - show_time) > 0.5)
          {
             for (l = e_config->syscon.actions; l; l = l->next)
               {
                  E_Config_Syscon_Action *sca;
                  E_Action *a;

                  if (!(sca = l->data)) continue;
                  if (!sca->action) continue;
                  a = e_action_find(sca->action);
                  if (!a) continue;
                  if (sca->is_main == 2)
                    {
                       a->func.go(NULL, sca->params);
                       e_syscon_hide();
                       break;
                    }
               }
          }
        return 0;
     }

   if (e_desklock_state_get()) return 0;
   if (!e_comp_grab_input(1, 1)) return 0;
   evas = e_comp->evas;
   evas_event_freeze(evas);

   o = edje_object_add(evas);
   o_bg = o;
   e_theme_edje_object_set(o, "base/theme/syscon",
                           "e/widgets/syscon/main");
   edje_object_part_text_set(o, "e.text.label", _("Cancel"));
   edje_object_signal_callback_add(o, "e,action,close", "",
                                   _cb_signal_close, NULL);
   edje_object_signal_callback_add(o, "e,action,syscon", "*",
                                   _cb_signal_syscon, NULL);

   popup = e_comp_object_util_add(o_bg, E_COMP_OBJECT_TYPE_POPUP);
   if (!popup)
     {
        evas_object_del(o_bg);
        evas_event_thaw(evas);
        e_comp_ungrab_input(1, 1);
        return 0;
     }
   evas_object_data_set(popup, "zone", zone);
   evas_object_layer_set(popup, E_LAYER_POPUP);
   e_comp_object_util_autoclose(popup, _cb_del, _cb_key_down, NULL);
   act_count = 0;
   show_time = t;

   // main (default):
   //  halt | suspend | desk_lock
   // secondary (default):
   //  reboot | hibernate | logout
   // extra (example for illume):
   //  home | close | kill

   o = e_flowlayout_add(evas);
   if (!popup)
     {
        evas_object_del(o_bg);
        evas_object_del(o);
        evas_event_thaw(evas);
        e_comp_ungrab_input(1, 1);
        return 0;
     }
   e_comp_object_util_del_list_append(popup, o);
   o_flow_main = o;
   e_flowlayout_orientation_set(o, 1);
   e_flowlayout_flowdirection_set(o, 1, 1);
   e_flowlayout_homogenous_set(o, 1);

   o = e_flowlayout_add(evas);
   e_comp_object_util_del_list_append(popup, o);
   o_flow_secondary = o;
   e_flowlayout_orientation_set(o, 1);
   e_flowlayout_flowdirection_set(o, 1, 1);
   e_flowlayout_homogenous_set(o, 1);

   o = e_flowlayout_add(evas);
   e_comp_object_util_del_list_append(popup, o);
   o_flow_extra = o;
   e_flowlayout_orientation_set(o, 1);
   e_flowlayout_flowdirection_set(o, 1, 1);
   e_flowlayout_homogenous_set(o, 1);

   for (l = e_config->syscon.actions; l; l = l->next)
     {
        E_Config_Syscon_Action *sca;
        char buf[1024];
        E_Action *a;
        int disabled;

        if (!(sca = l->data)) continue;
        if (!sca->action) continue;
        a = e_action_find(sca->action);
        if (!a) continue;
        disabled = 0;
        if ((!strcmp(sca->action, "logout")) &&
            (!e_sys_action_possible_get(E_SYS_LOGOUT))) disabled = 1;
        else if ((!strcmp(sca->action, "halt")) &&
                 (!e_sys_action_possible_get(E_SYS_HALT)))
          disabled = 1;
        else if ((!strcmp(sca->action, "halt_now")) &&
                 (!e_sys_action_possible_get(E_SYS_HALT_NOW)))
          disabled = 1;
        else if ((!strcmp(sca->action, "reboot")) &&
                 (!e_sys_action_possible_get(E_SYS_REBOOT)))
          disabled = 1;
        else if ((!strcmp(sca->action, "suspend")) &&
                 (!e_sys_action_possible_get(E_SYS_SUSPEND)))
          disabled = 1;
        else if ((!strcmp(sca->action, "hibernate")) &&
                 (!e_sys_action_possible_get(E_SYS_HIBERNATE)))
          disabled = 1;
        o = edje_object_add(evas);
        edje_object_signal_callback_add(o, "e,action,click", "",
                                        _cb_signal_action, sca);
        if (sca->button)
          {
             snprintf(buf, sizeof(buf), "e/widgets/syscon/item/%s",
                      sca->button);
             e_theme_edje_object_set(o, "base/theme/widgets", buf);
          }
        else
          e_theme_edje_object_set(o, "base/theme/widgets",
                                  "e/widgets/syscon/item/button");
        edje_object_part_text_set(o, "e.text.label",
                                  _(e_action_predef_label_get(sca->action, sca->params)));
        if (sca->icon)
          {
             o2 = e_icon_add(evas);
             e_util_icon_theme_set(o2, sca->icon);
             edje_object_part_swallow(o, "e.swallow.icon", o2);
             evas_object_show(o2);
             if (disabled)
               edje_object_signal_emit(o2, "e,state,disabled", "e");
          }
        if (disabled)
          edje_object_signal_emit(o, "e,state,disabled", "e");
        if (sca->is_main)
          {
             e_flowlayout_pack_end(o_flow_main, o);
             iw = ih = e_config->syscon.main.icon_size * e_scale;
          }
        else
          {
             e_flowlayout_pack_end(o_flow_secondary, o);
             iw = ih = e_config->syscon.secondary.icon_size * e_scale;
          }
        edje_object_message_signal_process(o);
        edje_object_size_min_calc(o, &mw, &mh);
        if (mw > iw) iw = mw;
        if (mh > ih) ih = mh;
        e_flowlayout_pack_options_set(o, 1, 1, 0, 0, 0.5, 0.5,
                                      iw, ih, iw, ih);
        evas_object_show(o);
     }

   for (l = (Eina_List *)e_sys_con_extra_action_list_get(); l; l = l->next)
     {
        E_Sys_Con_Action *sca;
        char buf[1024];

        sca = l->data;
        o = edje_object_add(evas);
        edje_object_signal_callback_add(o, "e,action,click", "", _cb_signal_action_extra, sca);
        if (sca->button_name)
          {
             snprintf(buf, sizeof(buf), "e/widgets/syscon/item/%s",
                      sca->button_name);
             e_theme_edje_object_set(o, "base/theme/widgets", buf);
          }
        else
          e_theme_edje_object_set(o, "base/theme/widgets",
                                  "e/widgets/syscon/item/button");
        edje_object_part_text_set(o, "e.text.label", sca->label);
        if (sca->icon_group)
          {
             o2 = edje_object_add(evas);
             e_util_edje_icon_set(o2, sca->icon_group);
             edje_object_part_swallow(o, "e.swallow.icon", o2);
             evas_object_show(o2);
             if (sca->disabled)
               edje_object_signal_emit(o2, "e,state,disabled", "e");
          }
        if (sca->disabled)
          edje_object_signal_emit(o, "e,state,disabled", "e");
        e_flowlayout_pack_end(o_flow_extra, o);
        iw = ih = e_config->syscon.extra.icon_size * e_scale;
        e_flowlayout_pack_options_set(o, 1, 1, 0, 0, 0.5, 0.5,
                                      iw, ih, iw, ih);
        evas_object_show(o);
     }

   e_flowlayout_fill_set(o_flow_main, 1);
   edje_object_part_swallow(o_bg, "e.swallow.main", o_flow_main);
   e_flowlayout_fill_set(o_flow_secondary, 1);
   edje_object_part_swallow(o_bg, "e.swallow.secondary", o_flow_secondary);
   e_flowlayout_fill_set(o_flow_extra, 1);
   edje_object_part_swallow(o_bg, "e.swallow.extra", o_flow_extra);

   e_zone_useful_geometry_get(zone, &zx, &zy, &zw, &zh);
   evas_object_resize(o_bg, zw, zh);
   edje_object_calc_force(o_bg);

   e_flowlayout_size_min_get(o_flow_main, &mw, &mh);
   evas_object_size_hint_min_set(o_flow_main, mw, mh);
   edje_object_part_swallow(o_bg, "e.swallow.main", o_flow_main);
   e_flowlayout_size_min_get(o_flow_secondary, &mw, &mh);
   evas_object_size_hint_min_set(o_flow_secondary, mw, mh);
   edje_object_part_swallow(o_bg, "e.swallow.secondary", o_flow_secondary);
   e_flowlayout_size_min_get(o_flow_extra, &mw, &mh);
   evas_object_size_hint_min_set(o_flow_extra, mw, mh);
   edje_object_part_swallow(o_bg, "e.swallow.extra", o_flow_extra);

   edje_object_size_min_calc(o_bg, &mw, &mh);

   w = mw;
   if (w > zw) w = zw;
   x = zx + (zw - w) / 2;
   h = mh;
   if (h > zh) h = zh;
   y = zy + (zh - h) / 2;

   evas_object_geometry_set(popup, x, y, w, h);

   if (e_config->syscon.do_input)
     {
        deftimer = ecore_timer_loop_add(e_config->syscon.timeout,
                                   _cb_timeout_defaction, NULL);
        if (defact) do_defact = eina_stringshare_add(defact);
     }

   evas_event_thaw(evas);
   inevas = 0;
   evas_object_show(popup);
   evas_object_focus_set(popup, 1);
   return 1;
}

void
e_syscon_hide(void)
{
   if (!popup) return;

   E_FREE_FUNC(deftimer, ecore_timer_del);
   eina_stringshare_replace(&do_defact, NULL);
   E_FREE_LIST(handlers, ecore_event_handler_del);
   e_comp_ungrab_input(1, 1);
   evas_object_hide(popup);
   E_FREE_FUNC(popup, evas_object_del);
   o_selected_flow = o_selected = o_flow_extra = o_flow_main = o_flow_secondary = NULL;
}

/* local subsystem functions */
static Eina_Bool
_cb_key_down(EINA_UNUSED void *data, Ecore_Event_Key *ev)
{
   if (!strcmp(ev->key, "Escape"))
     e_syscon_hide();
   else if ((!strcmp(ev->key, "Left")) || (!strcmp(ev->key, "Up")))
     {
        if (!o_selected)
          {
             if (e_flowlayout_pack_count_get(o_flow_extra))
               o_selected_flow = o_flow_extra, o_selected = e_flowlayout_pack_object_last(o_flow_extra);
             else if (e_flowlayout_pack_count_get(o_flow_secondary))
               o_selected_flow = o_flow_secondary, o_selected = e_flowlayout_pack_object_last(o_flow_secondary);
             else
               o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_last(o_flow_main);
          }
        else
          {
             edje_object_signal_emit(o_selected, "e,state,unfocused", "e");
             o_selected = e_flowlayout_pack_object_prev(o_selected_flow, o_selected);
             if (!o_selected)
               {
                  if (o_selected_flow == o_flow_extra)
                    {
                       if (e_flowlayout_pack_count_get(o_flow_secondary))
                         o_selected_flow = o_flow_secondary, o_selected = e_flowlayout_pack_object_last(o_flow_secondary);
                       else
                         o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_last(o_flow_main);
                    }
                  else if (o_selected_flow == o_flow_secondary)
                    o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_last(o_flow_main);
                  else
                    {
                       if (e_flowlayout_pack_count_get(o_flow_extra))
                         o_selected_flow = o_flow_extra, o_selected = e_flowlayout_pack_object_last(o_flow_extra);
                       else if (e_flowlayout_pack_count_get(o_flow_secondary))
                         o_selected_flow = o_flow_secondary, o_selected = e_flowlayout_pack_object_last(o_flow_secondary);
                       else
                         o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_last(o_flow_main);
                    }
               }
          }
        edje_object_signal_emit(o_selected, "e,state,focused", "e");
     }
   else if ((!strcmp(ev->key, "Right")) || (!strcmp(ev->key, "Down")) || (!strcmp(ev->key, "Tab")))
     {
        if (!o_selected)
          o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_first(o_flow_main);
        else
          {
             edje_object_signal_emit(o_selected, "e,state,unfocused", "e");
             o_selected = e_flowlayout_pack_object_next(o_selected_flow, o_selected);
             if (!o_selected)
               {
                  if (o_selected_flow == o_flow_extra)
                    o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_first(o_flow_main);
                  else if (o_selected_flow == o_flow_secondary)
                    {
                       if (e_flowlayout_pack_count_get(o_flow_extra))
                         o_selected_flow = o_flow_extra, o_selected = e_flowlayout_pack_object_first(o_flow_extra);
                       else
                         o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_first(o_flow_main);
                    }
                  else
                    {
                       if (e_flowlayout_pack_count_get(o_flow_secondary))
                         o_selected_flow = o_flow_secondary, o_selected = e_flowlayout_pack_object_first(o_flow_secondary);
                       else if (e_flowlayout_pack_count_get(o_flow_extra))
                         o_selected_flow = o_flow_extra, o_selected = e_flowlayout_pack_object_first(o_flow_extra);
                       else
                         o_selected_flow = o_flow_main, o_selected = e_flowlayout_pack_object_first(o_flow_main);
                    }
               }
          }
        edje_object_signal_emit(o_selected, "e,state,focused", "e");
     }
   else if ((!strcmp(ev->key, "KP_Enter")) || (!strcmp(ev->key, "Return")))
     {
        if (!o_selected) return ECORE_CALLBACK_RENEW;
        edje_object_signal_emit(o_selected, "e,state,focused", "e");
        edje_object_signal_emit(o_selected, "e,action,click", "");
        o_selected = o_selected_flow = NULL;
     }
   else
     {
        E_Action *act;
        double t;

        t = ecore_loop_time_get();
        if (t - show_time > 0.5)
          {
             act = e_bindings_key_event_find(E_BINDING_CONTEXT_ANY, ev, NULL);
             if ((act) && (act->name))
               {
                  if (!strcmp(act->name, "syscon"))
                    {
                       if (popup)
                         {
                            e_syscon_show(evas_object_data_get(popup, "zone"), do_defact);
                         }
                    }
                  else
                    {
                       Eina_List *l;

                       for (l = e_config->syscon.actions; l; l = l->next)
                         {
                            E_Config_Syscon_Action *sca;

                            if (!(sca = l->data)) continue;
                            if (!sca->action) continue;
                            if (!strcmp(sca->action, act->name))
                              {
                                 act_count++;
                                 if (act_count > 2)
                                   {
                                      act->func.go(NULL, sca->params);
                                      e_syscon_hide();
                                      break;
                                   }
                              }
                         }
                    }
               }
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_cb_signal_close(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   e_syscon_hide();
}

static void
_do_action_name(const char *action)
{
   Eina_List *l;

   for (l = e_config->syscon.actions; l; l = l->next)
     {
        E_Config_Syscon_Action *sca;
        E_Action *a;

        sca = l->data;
        if (!sca->action) continue;
        if (!strcmp(sca->action, action))
          {
             a = e_action_find(sca->action);
             if (!a) break;
             if (a) a->func.go(NULL, sca->params);
             break;
          }
     }
}

static void
_cb_signal_syscon(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source)
{
   e_syscon_hide();
   _do_action_name(source);
}

static void
_cb_signal_action(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Config_Syscon_Action *sca;
   E_Action *a;

   e_syscon_hide();
   sca = data;
   if (!sca) return;
   a = e_action_find(sca->action);
   if (!a) return;
   a->func.go(NULL, sca->params);
}

static void
_cb_signal_action_extra(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Sys_Con_Action *sca;

   e_syscon_hide();
   sca = data;
   if (!sca) return;
   if (sca->func) sca->func((void *)sca->data);
}

static Eina_Bool
_cb_timeout_defaction(void *data EINA_UNUSED)
{
   const char *defact = NULL;
   deftimer = NULL;
   if (!do_defact) return ECORE_CALLBACK_CANCEL;
   defact = eina_stringshare_add(do_defact);
   e_syscon_hide();
   if (defact)
     {
        _do_action_name(defact);
        eina_stringshare_del(defact);
     }
   return ECORE_CALLBACK_CANCEL;
}

