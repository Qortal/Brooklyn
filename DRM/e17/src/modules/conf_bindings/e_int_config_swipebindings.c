#include "e.h"

#define TEXT_NO_PARAMS           _("<None>")

struct _E_Config_Dialog_Data
{
   Evas *evas;
   struct
   {
      Eina_List *swipe;
   } binding;
   struct
   {
      const char *binding, *action;
      char       *params;
      const char *cur;
      int         button;
      int         cur_act;
      const char *swipe;
      const char *source;

      E_Dialog   *dia;
      double degree;
      double error;
      double length;
      unsigned int fingers;
   } locals;
   struct
   {
      Evas_Object *o_add, *o_del, *o_del_all;
      Evas_Object *o_binding_list, *o_action_list;
      Evas_Object *o_params, *o_selector;
   } gui;

   const char      *params;

   int              fullscreen_flip;
   int              multiscreen_flip;

   E_Config_Dialog *cfd;
};

static E_Config_Binding_Swipe *
_swipe_binding_copy(E_Config_Binding_Swipe *bi)
{
   E_Config_Binding_Swipe *bi2;
   if (!bi) return NULL;

   bi2 = E_NEW(E_Config_Binding_Swipe, 1);
   bi2->context = bi->context;
   bi2->direction = bi->direction;
   bi2->length = bi->length;
   bi2->fingers = bi->fingers;
   bi2->error = bi->error;
   bi2->action = bi->action;
   bi2->params = bi->params;
   return bi2;
}

static void
_swipe_binding_free(E_Config_Binding_Swipe *bi)
{
   if (!bi) return;
   eina_stringshare_del(bi->action);
   eina_stringshare_del(bi->params);
   free(bi);
}


static void
_auto_apply_changes(E_Config_Dialog_Data *cfdata)
{
   int n, g, a, ok;
   E_Config_Binding_Swipe *bi;
   E_Action_Group *actg;
   E_Action_Description *actd;

   if ((!cfdata->locals.cur) || (!cfdata->locals.cur[0]) ||
       (!cfdata->locals.action) || (!cfdata->locals.action[0])) return;

   if (sscanf(cfdata->locals.cur, "s%d", &n) != 1)
     return;
   if (sscanf(cfdata->locals.action, "%d %d", &g, &a) != 2)
     return;

   bi = eina_list_nth(cfdata->binding.swipe, n);
   if (!bi) return;

   actg = eina_list_nth(e_action_groups_get(), g);
   if (!actg) return;
   actd = eina_list_nth(actg->acts, a);
   if (!actd) return;

   eina_stringshare_del(bi->action);
   bi->action = NULL;

   if (actd->act_cmd) bi->action = eina_stringshare_add(actd->act_cmd);

   eina_stringshare_del(bi->params);
   bi->params = NULL;

   if (actd->act_params)
     bi->params = eina_stringshare_add(actd->act_params);
   else
     {
        ok = 1;
        if (cfdata->locals.params)
          {
             if (!strcmp(cfdata->locals.params, TEXT_NO_PARAMS))
               ok = 0;

             if ((actd->param_example) && (!strcmp(cfdata->locals.params, actd->param_example)))
               ok = 0;
          }
        else
          ok = 0;

        if (ok)
          bi->params = eina_stringshare_add(cfdata->locals.params);
     }
}

static void
_fill_data(E_Config_Dialog_Data *cfdata)
{
   E_Config_Binding_Swipe *bi, *bi2;
   Eina_List *l;

   cfdata->locals.params = strdup("");
   cfdata->locals.action = eina_stringshare_add("");
   cfdata->locals.binding = eina_stringshare_add("");
   cfdata->locals.swipe = eina_stringshare_add("");
   cfdata->locals.source = eina_stringshare_add("");
   cfdata->locals.cur = NULL;
   cfdata->locals.dia = NULL;
   cfdata->binding.swipe = NULL;

   EINA_LIST_FOREACH(e_bindings->swipe_bindings, l, bi)
     {
        if (!bi) continue;
        bi2 = _swipe_binding_copy(bi);
        cfdata->binding.swipe = eina_list_append(cfdata->binding.swipe, bi2);
     }
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->cfd = cfd;
   _fill_data(cfdata);

   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_FREE_LIST(cfdata->binding.swipe, _swipe_binding_free);

   eina_stringshare_del(cfdata->locals.cur);
   eina_stringshare_del(cfdata->params);
   eina_stringshare_del(cfdata->locals.binding);
   eina_stringshare_del(cfdata->locals.action);
   eina_stringshare_del(cfdata->locals.swipe);
   eina_stringshare_del(cfdata->locals.source);

   if (cfdata->locals.dia) e_object_del(E_OBJECT(cfdata->locals.dia));

   free(cfdata->locals.params);
   E_FREE(cfdata);
}


static void
_update_action_params(E_Config_Dialog_Data *cfdata)
{
   int g, a, b;
   E_Action_Group *actg;
   E_Action_Description *actd;
   E_Config_Binding_Swipe *bi;
   const char *action, *params;

#define SIGNAL_EXAMPLE_PARAMS                                         \
  if ((!actd->param_example) || (!actd->param_example[0]))          \
    e_widget_entry_text_set(cfdata->gui.o_params, TEXT_NO_PARAMS);  \
  else                                                              \
    e_widget_entry_text_set(cfdata->gui.o_params, actd->param_example)

   if ((!cfdata->locals.action) || (!cfdata->locals.action[0]))
     {
        e_widget_disabled_set(cfdata->gui.o_params, 1);
        e_widget_entry_clear(cfdata->gui.o_params);
        return;
     }
   if (sscanf(cfdata->locals.action, "%d %d", &g, &a) != 2)
     return;

   actg = eina_list_nth(e_action_groups_get(), g);
   if (!actg) return;
   actd = eina_list_nth(actg->acts, a);
   if (!actd) return;

   if (actd->act_params)
     {
        e_widget_disabled_set(cfdata->gui.o_params, 1);
        e_widget_entry_text_set(cfdata->gui.o_params, actd->act_params);
        return;
     }

   if ((!cfdata->locals.cur) || (!cfdata->locals.cur[0]))
     {
        e_widget_disabled_set(cfdata->gui.o_params, 1);
        SIGNAL_EXAMPLE_PARAMS;
        return;
     }

   if (!actd->editable)
     e_widget_disabled_set(cfdata->gui.o_params, 1);
   else
     e_widget_disabled_set(cfdata->gui.o_params, 0);

   if (cfdata->locals.cur[0] == 's')
     {
        if (sscanf(cfdata->locals.cur, "s%d", &b) != 1)
          {
             e_widget_disabled_set(cfdata->gui.o_params, 1);
             SIGNAL_EXAMPLE_PARAMS;
             return;
          }

        bi = eina_list_nth(cfdata->binding.swipe, b);
        if (!bi)
          {
             e_widget_disabled_set(cfdata->gui.o_params, 1);
             SIGNAL_EXAMPLE_PARAMS;
             return;
          }
        action = bi->action;
        params = bi->params;
     }
   else
     {
        e_widget_disabled_set(cfdata->gui.o_params, 1);
        SIGNAL_EXAMPLE_PARAMS;
        return;
     }

   if (action)
     {
        if (!strcmp(action, actd->act_cmd))
          {
             if ((!params) || (!params[0]))
               SIGNAL_EXAMPLE_PARAMS;
             else
               e_widget_entry_text_set(cfdata->gui.o_params, params);
          }
        else
          SIGNAL_EXAMPLE_PARAMS;
     }
   else
     SIGNAL_EXAMPLE_PARAMS;
}

static void
_action_change_cb(void *data)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;
   _update_action_params(cfdata);
}

static int
_swipe_binding_sort_cb(E_Config_Binding_Swipe *a, E_Config_Binding_Swipe *b)
{
   int finger_diff = a->fingers - b->fingers;
   if (!finger_diff)
     {
        return a->direction - b->direction;
     }
   return finger_diff*-1;
}

static void
_update_buttons(E_Config_Dialog_Data *cfdata)
{
   if (e_widget_ilist_count(cfdata->gui.o_binding_list))
     e_widget_disabled_set(cfdata->gui.o_del_all, 0);
   else
     e_widget_disabled_set(cfdata->gui.o_del_all, 1);

   if (!cfdata->locals.cur)
     {
        e_widget_disabled_set(cfdata->gui.o_del, 1);
        return;
     }
   e_widget_disabled_set(cfdata->gui.o_del, 0);
}


static void
_find_swipe_binding_action(const char *action, const char *params, int *g, int *a, int *n)
{
   Eina_List *l, *l2;
   int gg = -1, aa = -1, nn = -1, found;
   E_Action_Group *actg;
   E_Action_Description *actd;

   if (g) *g = -1;
   if (a) *a = -1;
   if (n) *n = -1;

   found = 0;
   for (l = e_action_groups_get(), gg = 0, nn = 0; l; l = l->next, gg++)
     {
        actg = l->data;

        for (l2 = actg->acts, aa = 0; l2; l2 = l2->next, aa++)
          {
             actd = l2->data;
             if (!strcmp((!action ? "" : action), (!actd->act_cmd ? "" : actd->act_cmd)))
               {
                  if (!params || !params[0])
                    {
                       if ((!actd->act_params) || (!actd->act_params[0]))
                         {
                            if (g) *g = gg;
                            if (a) *a = aa;
                            if (n) *n = nn;
                            return;
                         }
                       else
                         continue;
                    }
                  else
                    {
                       if ((!actd->act_params) || (!actd->act_params[0]))
                         {
                            if (g) *g = gg;
                            if (a) *a = aa;
                            if (n) *n = nn;
                            found = 1;
                         }
                       else
                         {
                            if (!strcmp(params, actd->act_params))
                              {
                                 if (g) *g = gg;
                                 if (a) *a = aa;
                                 if (n) *n = nn;
                                 return;
                              }
                         }
                    }
               }
             nn++;
          }
        if (found) break;
     }

   if (!found)
     {
        if (g) *g = -1;
        if (a) *a = -1;
        if (n) *n = -1;
     }
}

static void
_update_action_list(E_Config_Dialog_Data *cfdata)
{
   E_Config_Binding_Swipe *bi;
   int j = -1, i, n;
   const char *action, *params;

   if (!cfdata->locals.cur) return;

   if (cfdata->locals.cur[0] == 's')
     {
        if (sscanf(cfdata->locals.cur, "s%d", &n) != 1)
          return;

        bi = eina_list_nth(cfdata->binding.swipe, n);
        if (!bi)
          {
             e_widget_ilist_unselect(cfdata->gui.o_action_list);
             e_widget_entry_clear(cfdata->gui.o_params);
             e_widget_disabled_set(cfdata->gui.o_params, 1);
             return;
          }
        action = bi->action;
        params = bi->params;
     }
   else
     return;

   _find_swipe_binding_action(action, params, NULL, NULL, &j);

   if (j >= 0)
     {
        for (i = 0; i < e_widget_ilist_count(cfdata->gui.o_action_list); i++)
          {
             if (i > j) break;
             if (e_widget_ilist_nth_is_header(cfdata->gui.o_action_list, i)) j++;
          }
     }

   if (j >= 0)
     {
        if (j == e_widget_ilist_selected_get(cfdata->gui.o_action_list))
          _update_action_params(cfdata);
        else
          e_widget_ilist_selected_set(cfdata->gui.o_action_list, j);
     }
   else
     {
        e_widget_ilist_unselect(cfdata->gui.o_action_list);
        eina_stringshare_del(cfdata->locals.action);
        cfdata->locals.action = eina_stringshare_add("");
        e_widget_entry_clear(cfdata->gui.o_params);
     }
}

static void
_binding_change_cb(void *data)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;

   _auto_apply_changes(cfdata);
   if (cfdata->locals.cur) eina_stringshare_del(cfdata->locals.cur);
   cfdata->locals.cur = NULL;

   if ((!cfdata->locals.binding) || (!cfdata->locals.binding[0])) return;

   cfdata->locals.cur = eina_stringshare_ref(cfdata->locals.binding);

   _update_buttons(cfdata);
   _update_action_list(cfdata);
}


static int
_basic_apply_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   E_Config_Binding_Swipe *bi, *bi2;

   _auto_apply_changes(cfdata);
   E_FREE_LIST(e_bindings->swipe_bindings, _swipe_binding_free);
   EINA_LIST_FOREACH(cfdata->binding.swipe, l, bi2)
     {
        bi = _swipe_binding_copy(bi2);
        e_bindings->swipe_bindings = eina_list_append(e_bindings->swipe_bindings, bi);
     }
   e_bindings_swipe_reset();

   e_config_save_queue();

   return 1;
}

static Evas_Object*
create_visualisation(Evas *e, double direction, double error)
{
   Evas_Vg_Container *vg = NULL;
   Evas_Vg_Shape *container, *shape, *viewport = NULL;

   double center_x = 15, center_y = 15;

   vg = evas_object_vg_add(e);
   evas_object_vg_viewbox_set(vg, EINA_RECT(0, 0, 50, 50));

   container = evas_vg_container_add(vg);

   viewport = evas_vg_shape_add(container);
   evas_vg_shape_append_rect(viewport, 0, 0, 51, 51, 0, 0);
   evas_vg_shape_stroke_cap_set(viewport, EVAS_VG_CAP_SQUARE);
   evas_vg_shape_stroke_color_set(viewport, 0, 0, 0, 0);
   evas_vg_shape_stroke_width_set(viewport, 1);

   shape = evas_vg_shape_add(container);
   evas_vg_shape_append_rect(shape, 1, 1, 29, 29, 0, 0);
   evas_vg_shape_stroke_cap_set(shape, EVAS_VG_CAP_SQUARE);
   evas_vg_shape_stroke_color_set(shape, 100, 100, 100, 255);
   evas_vg_shape_stroke_width_set(shape, 1);

   shape = evas_vg_shape_add(container);
   evas_vg_shape_append_line_to(shape, center_x, center_y);
   evas_vg_shape_append_line_to(shape, center_x + cos(direction - error)*11, center_y + sin(direction - error)*11);
   evas_vg_shape_append_arc_to(shape, center_x + cos(direction + error)*11, center_y + sin(direction + error)*11, 11, 11, 0, EINA_FALSE, EINA_TRUE);
   evas_vg_shape_append_line_to(shape, center_x, center_y);

   evas_vg_shape_stroke_cap_set(shape, EVAS_VG_CAP_ROUND);
   evas_vg_shape_stroke_color_set(shape, 255, 0, 0, 255);
   evas_vg_shape_stroke_width_set(shape, 2);

   evas_object_vg_root_node_set(vg, container);

   evas_object_show(vg);

   return vg;
}

static void
_update_swipe_binding_list(E_Config_Dialog_Data *cfdata)
{
   int i = 0;
   char b2[64], b3[64];
   Eina_List *l;
   E_Config_Binding_Swipe *bi;
   unsigned int previous_fingers = 0;

   evas_event_freeze(evas_object_evas_get(cfdata->gui.o_binding_list));
   edje_freeze();
   e_widget_ilist_freeze(cfdata->gui.o_binding_list);

   e_widget_ilist_clear(cfdata->gui.o_binding_list);
   e_widget_ilist_go(cfdata->gui.o_binding_list);

   if (cfdata->binding.swipe)
     cfdata->binding.swipe = eina_list_sort(cfdata->binding.swipe, 0, (Eina_Compare_Cb)_swipe_binding_sort_cb);

   EINA_LIST_FOREACH(cfdata->binding.swipe, l, bi)
     {
        Evas_Object *vis;

        vis = create_visualisation(evas_object_evas_get(cfdata->gui.o_binding_list), bi->direction, bi->error);
        if (bi->fingers != previous_fingers)
          {
             snprintf(b3, sizeof(b3), "%d Fingers", bi->fingers);
             previous_fingers = bi->fingers;
             e_widget_ilist_header_append(cfdata->gui.o_binding_list, NULL, b3);
          }
        snprintf(b2, sizeof(b2), "s%d", i);
        snprintf(b3, sizeof(b3), "Length: %.2f Error: %.2f", bi->length, bi->error);

        e_widget_ilist_append(cfdata->gui.o_binding_list, vis, b3, _binding_change_cb, cfdata, b2);
        i++;
     }
   e_widget_ilist_go(cfdata->gui.o_binding_list);

   e_widget_ilist_thaw(cfdata->gui.o_binding_list);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(cfdata->gui.o_binding_list));

   if (eina_list_count(cfdata->binding.swipe))
     e_widget_disabled_set(cfdata->gui.o_del_all, 0);
   else
     e_widget_disabled_set(cfdata->gui.o_del_all, 1);
}

static void
_fill_actions_list(E_Config_Dialog_Data *cfdata)
{
   char buf[1024];
   Eina_List *l, *l2;
   E_Action_Group *actg;
   E_Action_Description *actd;
   int g, a;

   evas_event_freeze(evas_object_evas_get(cfdata->gui.o_action_list));
   edje_freeze();
   e_widget_ilist_freeze(cfdata->gui.o_action_list);

   e_widget_ilist_clear(cfdata->gui.o_action_list);
   for (l = e_action_groups_get(), g = 0; l; l = l->next, g++)
     {
        actg = l->data;

        if (!actg->acts) continue;

        e_widget_ilist_header_append(cfdata->gui.o_action_list, NULL, _(actg->act_grp));

        for (l2 = actg->acts, a = 0; l2; l2 = l2->next, a++)
          {
             actd = l2->data;

             snprintf(buf, sizeof(buf), "%d %d", g, a);
             e_widget_ilist_append(cfdata->gui.o_action_list, NULL, _(actd->act_name),
                                   _action_change_cb, cfdata, buf);
          }
     }
   e_widget_ilist_go(cfdata->gui.o_action_list);
   e_widget_ilist_thaw(cfdata->gui.o_action_list);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(cfdata->gui.o_action_list));
}

static void
_flush_binding_swipe(E_Config_Dialog_Data *cfdata)
{
   E_Config_Binding_Swipe *bi;

   bi = E_NEW(E_Config_Binding_Swipe, 1);
   bi->context = E_BINDING_CONTEXT_NONE;
   bi->length = cfdata->locals.length;
   bi->direction = cfdata->locals.degree;
   bi->error = cfdata->locals.error;
   bi->fingers = cfdata->locals.fingers;

   cfdata->binding.swipe = eina_list_append(cfdata->binding.swipe, bi);
   _update_swipe_binding_list(cfdata);
}

static void
_swipe_add_cb_ok(void *data, E_Dialog *dia)
{
   E_Config_Dialog_Data *cfdata = data;

   _flush_binding_swipe(cfdata);

   e_object_del(E_OBJECT(dia));
}

static void
_swipe_add_cb_cancel(void *data EINA_UNUSED, E_Dialog *dia)
{
   e_object_del(E_OBJECT(dia));
}

static void
_swipe_add_del(void *data)
{
   E_Dialog *dia = data;
   E_Config_Dialog_Data *cfdata;

   if (!dia->data) return;
   cfdata = dia->data;
   cfdata->locals.dia = NULL;
}

static void
_double_getter(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double *container = data;
   double tmp = elm_spinner_value_get(obj);
   *container = tmp;
}

static void
_int_getter(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   unsigned int *container = data;
   double tmp = elm_spinner_value_get(obj);
   *container = tmp;
}

static void
_swipe_add_show(E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *obg, *ol, *entry;
   Evas *evas;
   int w, h;

   if (cfdata->locals.dia) return;

   cfdata->locals.dia = e_dialog_new(cfdata->cfd->dia->win, "E", "_swipebind_new_dialog");
   e_dialog_resizable_set(cfdata->locals.dia, 1);
   e_dialog_title_set(cfdata->locals.dia, _("Add Swipe Binding"));
   e_dialog_icon_set(cfdata->locals.dia, "enlightenment/swipes", 48);
   e_dialog_button_add(cfdata->locals.dia, _("OK"), NULL, _swipe_add_cb_ok, cfdata);
   e_dialog_button_add(cfdata->locals.dia, _("Cancel"), NULL, _swipe_add_cb_cancel, cfdata);
   e_object_del_attach_func_set(E_OBJECT(cfdata->locals.dia), _swipe_add_del);
   cfdata->locals.dia->data = cfdata;
   elm_win_center(cfdata->locals.dia->win, 1, 1);

   evas = evas_object_evas_get(cfdata->locals.dia->win);
   obg = e_widget_list_add(evas, 1, 0);

   ol = e_widget_framelist_add(evas, _("Direction:"), 0);
   entry = o = elm_spinner_add(cfdata->locals.dia->win);
   evas_object_smart_callback_add(o, "changed", _double_getter, &cfdata->locals.degree);
   elm_spinner_min_max_set(o, 0, 2*M_PI);
   elm_spinner_label_format_set(o, "%f");
   elm_spinner_step_set(o, 0.1);
   elm_spinner_editable_set(o, EINA_TRUE);
   e_widget_framelist_object_append(ol, o);
   e_widget_list_object_append(obg, ol, 1, 0, 0.5);

   ol = e_widget_framelist_add(evas, _("Error:"), 0);
   entry = o = elm_spinner_add(cfdata->locals.dia->win);
   evas_object_smart_callback_add(o, "changed", _double_getter, &cfdata->locals.error);
   elm_spinner_min_max_set(o, 0, 2*M_PI);
   elm_spinner_label_format_set(o, "%f");
   elm_spinner_step_set(o, 0.1);
   elm_spinner_editable_set(o, EINA_TRUE);
   e_widget_framelist_object_append(ol, o);
   e_widget_list_object_append(obg, ol, 1, 0, 0.5);

   ol = e_widget_framelist_add(evas, _("Length:"), 0);
   entry = o = elm_spinner_add(cfdata->locals.dia->win);
   evas_object_smart_callback_add(o, "changed", _double_getter, &cfdata->locals.length);
   elm_spinner_min_max_set(o, 0, 200);
   elm_spinner_step_set(o, 5);
   elm_spinner_editable_set(o, EINA_TRUE);
   e_widget_framelist_object_append(ol, o);
   e_widget_list_object_append(obg, ol, 1, 0, 0.5);

   cfdata->locals.fingers = 3;
   ol = e_widget_framelist_add(evas, _("Fingers:"), 0);
   entry = o = elm_spinner_add(cfdata->locals.dia->win);
   evas_object_smart_callback_add(o, "changed", _int_getter, &cfdata->locals.fingers);
   elm_spinner_min_max_set(o, 3, 10);
   elm_spinner_value_set(o, 3.0);
   elm_spinner_editable_set(o, EINA_TRUE);
   e_widget_framelist_object_append(ol, o);
   e_widget_list_object_append(obg, ol, 1, 0, 0.5);

   e_widget_size_min_get(obg, &w, &h);
   e_dialog_content_set(cfdata->locals.dia, obg, MAX(w, 200), MAX(h, 100));

   e_dialog_show(cfdata->locals.dia);
   e_widget_focus_set(entry, 1);
}

static void
_add_swipe_binding_cb(void *data, void *data2 EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   _auto_apply_changes(cfdata);
   _swipe_add_show(cfdata);
}

static void
_update_swipe_cb(void *data, Eina_Bool end, double direction, double length, double error, unsigned int fingers)
{
   E_Config_Dialog_Data *cfdata = data;

   if (end)
     {
        e_object_del(E_OBJECT(cfdata->locals.dia));
        cfdata->locals.dia = NULL;
        cfdata->locals.degree = direction;
        cfdata->locals.length = length;
        cfdata->locals.error = error;
        cfdata->locals.fingers = fingers;
        _flush_binding_swipe(cfdata);
        e_bindings_swipe_live_update_hook_set(NULL, NULL);
     }
   else
     {
        char text_buf[1000];
        Evas_Object *vis;

        vis = create_visualisation(evas_object_evas_get(cfdata->locals.dia->win), direction, error);
        evas_object_size_hint_align_set(vis, 0.0, 0.5);
        snprintf(text_buf, sizeof(text_buf), _("Using %d Fingers<br> <b>Direction</b> %f <b>Length</b> %f <b>Error</b> %f"), fingers, direction, length, error);
        e_dialog_text_set(cfdata->locals.dia, text_buf);
        elm_object_part_content_set(cfdata->locals.dia->bg_object, "e.swallow.icon", vis);
        evas_object_size_hint_min_set(vis, 30 * e_scale, 30 * e_scale);
        elm_layout_signal_emit(cfdata->locals.dia->bg_object, "e,state,icon", "e");
        elm_layout_signal_emit(cfdata->locals.dia->bg_object, "e,icon,enabled", "e");
     }
}

static void
_add_swipe_binding_by_sample_cb(void *data, void *data2 EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   cfdata->locals.dia = e_dialog_new(cfdata->cfd->dia->win, "E", "_swipe_recognition");
   e_dialog_icon_set(cfdata->locals.dia, "preferences-desktop-swipe-bindings", 48);
   e_dialog_title_set(cfdata->locals.dia, _("Swipe recognition"));
   e_dialog_text_set(cfdata->locals.dia, _("Do your swipe gesture.<br><br>Press <hilight>Escape</hilight> to abort"));
   elm_win_center(cfdata->locals.dia->win, 1, 1);
   elm_win_borderless_set(cfdata->locals.dia->win, 1);
   e_dialog_resizable_set(cfdata->locals.dia, 0);
   e_dialog_show(cfdata->locals.dia);
   evas_object_layer_set(e_win_client_get(cfdata->locals.dia->win)->frame, E_LAYER_CLIENT_PRIO);

   e_bindings_swipe_live_update_hook_set(_update_swipe_cb, data);
}

static void
_delete_swipe_binding_cb(void *data, void *data2 EINA_UNUSED)
{
   Eina_List *l = NULL;
   int sel, n;
   E_Config_Dialog_Data *cfdata;

   cfdata = data;

   sel = e_widget_ilist_selected_get(cfdata->gui.o_binding_list);
   if (cfdata->locals.binding[0] == 's')
     {
        if (sscanf(cfdata->locals.binding, "s%d", &n) != 1)
          return;

        l = eina_list_nth_list(cfdata->binding.swipe, n);

        if (l)
          {
             _swipe_binding_free(eina_list_data_get(l));
             cfdata->binding.swipe = eina_list_remove_list(cfdata->binding.swipe, l);
          }
     }

   _update_swipe_binding_list(cfdata);

   if (sel >= e_widget_ilist_count(cfdata->gui.o_binding_list))
     sel = e_widget_ilist_count(cfdata->gui.o_binding_list) - 1;

   eina_stringshare_del(cfdata->locals.cur);
   cfdata->locals.cur = NULL;

   e_widget_ilist_selected_set(cfdata->gui.o_binding_list, sel);
   if (sel < 0)
     {
        e_widget_ilist_unselect(cfdata->gui.o_action_list);
        e_widget_entry_clear(cfdata->gui.o_params);
        e_widget_disabled_set(cfdata->gui.o_params, 1);
        _update_buttons(cfdata);
     }
}


static void
_delete_all_swipe_binding_cb(void *data, void *data2 EINA_UNUSED)
{
   E_Config_Binding_Swipe *bi;
   E_Config_Dialog_Data *cfdata;

   cfdata = data;

   EINA_LIST_FREE(cfdata->binding.swipe, bi)
     _swipe_binding_free(bi);

   eina_stringshare_del(cfdata->locals.cur);
   cfdata->locals.cur = NULL;

   e_widget_ilist_clear(cfdata->gui.o_binding_list);
   e_widget_ilist_go(cfdata->gui.o_binding_list);
   e_widget_ilist_unselect(cfdata->gui.o_action_list);
   e_widget_entry_clear(cfdata->gui.o_params);
   e_widget_disabled_set(cfdata->gui.o_params, 1);

   _update_buttons(cfdata);
}


static void
_restore_swipe_binding_defaults_cb(void *data, void *data2 EINA_UNUSED)
{
   E_Config_Bindings *ecb;
   Eina_Stringshare *prof;
   E_Config_Dialog_Data *cfdata = data;

   ecb = e_config_domain_system_load("e_bindings", e_config_descriptor_find("E_Config_Bindings"));
   if (!ecb)
     {
        const char *type;

        prof = eina_stringshare_ref(e_config_profile_get());
        switch (e_config->config_type)
          {
           case E_CONFIG_PROFILE_TYPE_DESKTOP:
             type = "standard";
             break;
           case E_CONFIG_PROFILE_TYPE_MOBILE:
             type = "mobile";
             break;
           //case E_CONFIG_PROFILE_TYPE_TABLET: FIXME - not used
           default:
             type = "default";
             break;
          }
        e_config_profile_set(type);
        ecb = e_config_domain_system_load("e_bindings", e_config_descriptor_find("E_Config_Bindings"));
        e_config_profile_set(prof);
        eina_stringshare_del(prof);
     }
   if (!ecb) return;
   E_FREE_LIST(cfdata->binding.swipe, e_config_binding_swipe_free);
   cfdata->binding.swipe = ecb->swipe_bindings, ecb->swipe_bindings = NULL;
   e_config_bindings_free(ecb);

   eina_stringshare_del(cfdata->locals.cur);
   cfdata->locals.cur = NULL;

   _update_swipe_binding_list(cfdata);
   _update_buttons(cfdata);

   e_widget_ilist_unselect(cfdata->gui.o_action_list);
   e_widget_entry_clear(cfdata->gui.o_params);
   e_widget_disabled_set(cfdata->gui.o_params, 1);
}

static void
_close_help_dialog(void *data EINA_UNUSED, E_Dialog *dia)
{
   e_object_del(E_OBJECT(dia));
}

static void
_help_swipe_bindings_cb(void *data EINA_UNUSED, void *data2 EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   E_Dialog *help = e_dialog_new(cfdata->cfd->dia->win, "E", "_swipe_recognition");
   e_dialog_title_set(help, _("Swipe Bindings Help"));
   e_dialog_text_set(help, _("Enlightenment is using libinput to detect swipe gesture. In case there are problems:<br> 1. Test gestures while executing \"libinput debug-events\" in terminal. The console output will tell the precision of your hardware.<br>2. Watch for error in console, some libinput devices are returning wrong results. <br>3. If your session is running inside Xorg, ensure that your user is part of the libinput group.<br>"));
   e_dialog_button_add(help, _("Close"), NULL, _close_help_dialog, help);
   e_dialog_show(help);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *ol, *ot, *of, *ob;

   cfdata->evas = evas;
   o = e_widget_list_add(evas, 0, 0);
   ol = e_widget_list_add(evas, 0, 1);

   of = e_widget_frametable_add(evas, _("Swipe Bindings"), 0);
   ob = e_widget_ilist_add(evas, 32, 32, &(cfdata->locals.binding));
   cfdata->gui.o_binding_list = ob;
   e_widget_size_min_set(ob, 200, 160);
   e_widget_frametable_object_append(of, ob, 0, 0, 2, 1, 1, 1, 1, 1);

   ob = e_widget_button_add(evas, _("Add"), "list-add", _add_swipe_binding_by_sample_cb, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 0, 1, 0);

   ob = e_widget_button_add(evas, _("Add by props"), "list-add", _add_swipe_binding_cb, cfdata, NULL);
   cfdata->gui.o_add = ob;
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 0, 1, 0);

   ob = e_widget_button_add(evas, _("Delete"), "list-remove", _delete_swipe_binding_cb, cfdata, NULL);
   cfdata->gui.o_del = ob;
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 0, 1, 0);
   ob = e_widget_button_add(evas, _("Delete All"), "edit-clear", _delete_all_swipe_binding_cb, cfdata, NULL);
   cfdata->gui.o_del_all = ob;
   e_widget_disabled_set(ob, 1);
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 0, 1, 0);
   ob = e_widget_button_add(evas, _("Restore Default Bindings"), "enlightenment", _restore_swipe_binding_defaults_cb, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 0, 1, 0);
   ob = e_widget_button_add(evas, _("Help"), "help", _help_swipe_bindings_cb, cfdata, NULL);
   e_widget_frametable_object_append(of, ob, 1, 3, 1, 1, 1, 0, 1, 0);
   e_widget_list_object_append(ol, of, 1, 1, 0.5);

   ot = e_widget_table_add(e_win_evas_win_get(evas), 0);
   of = e_widget_framelist_add(evas, _("Action"), 0);
   ob = e_widget_ilist_add(evas, 24, 24, &(cfdata->locals.action));
   cfdata->gui.o_action_list = ob;
   e_widget_size_min_set(ob, 200, 240);
   e_widget_framelist_object_append(of, ob);
   e_widget_table_object_append(ot, of, 0, 0, 1, 1, 1, 1, 1, 1);

   of = e_widget_framelist_add(evas, _("Action Params"), 0);
   ob = e_widget_entry_add(cfd->dia->win, &(cfdata->locals.params), NULL, NULL, NULL);
   cfdata->gui.o_params = ob;
   e_widget_disabled_set(ob, 1);
   e_widget_framelist_object_append(of, ob);
   e_widget_table_object_append(ot, of, 0, 3, 1, 1, 1, 1, 1, 0);
   e_widget_list_object_append(ol, ot, 1, 1, 0.5);

   e_widget_list_object_append(o, ol, 1, 1, 0.5);

   _update_swipe_binding_list(cfdata);
   _fill_actions_list(cfdata);

   e_dialog_resizable_set(cfd->dia, 1);
   return o;
}

static Eina_Bool
_user_part_of_input(void)
{
   uid_t user = getuid();
   struct passwd *user_pw = getpwuid(user);
   gid_t *gids = NULL;
   int number_of_groups = 0;
   struct group *input_group = getgrnam("input");

   EINA_SAFETY_ON_NULL_RETURN_VAL(user_pw, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input_group, EINA_FALSE);

   if (getgrouplist(user_pw->pw_name, getgid(), NULL, &number_of_groups) != -1)
     {
        ERR("Failed to enumerate groups of user");
        return EINA_FALSE;
     }
   number_of_groups ++;
   gids = alloca((number_of_groups) * sizeof(gid_t));
   if (getgrouplist(user_pw->pw_name, getgid(), gids, &number_of_groups) == -1)
     {
        ERR("Failed to get groups of user");
        return EINA_FALSE;
     }

   for (int i = 0; i < number_of_groups; ++i)
     {
        if (gids[i] == input_group->gr_gid)
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

E_Config_Dialog *
e_int_config_swipebindings(Evas_Object *parent EINA_UNUSED, const char *params)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "keyboard_and_mouse/swipe_bindings")) return NULL;
   v = E_NEW(E_Config_Dialog_View, 1);

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->override_auto_apply = 1;

   if (e_comp->comp_type == E_PIXMAP_TYPE_X && !_user_part_of_input())
     {
        e_module_dialog_show(NULL, "Gesture Recognition", "Your user is not part of the input group, libinput cannot be used.");
     }

   if (e_bindings_gesture_capable_devices_get() == 0)
     {
        e_module_dialog_show(NULL, "Gesture Recognition", "No devices detected that are capable of gesture recognition.");
     }

   cfd = e_config_dialog_new(NULL, _("Swipe Bindings Settings"), "E",
                             "keyboard_and_mouse/swipe_bindings",
                             "enlightenment/swipes", 0, v, NULL);
   if ((params) && (params[0]))
     {
        cfd->cfdata->params = eina_stringshare_add(params);
     }

   return cfd;
}
