#include "e.h"

#define MODE_NOTHING        0
#define MODE_GEOMETRY       1
#define MODE_LOCKS          2
#define MODE_GEOMETRY_LOCKS 3
#define MODE_ALL            4

static void         _ec_cb_dialog_del(void *obj);
static void         _ec_cb_dialog_close(void *data, E_Dialog *dia);
static Evas_Object *_ec_icccm_create(E_Dialog *dia, void *data);
static Evas_Object *_ec_netwm_create(E_Dialog *dia, void *data);
static void         _ec_go(void *data, void *data2);
static void         _create_data(E_Dialog *cfd, E_Client *ec);
static void         _free_data(E_Dialog *cfd, E_Config_Dialog_Data *cfdata);

struct _E_Config_Dialog_Data
{
   E_Client *client;
   /*- BASIC -*/
   struct
   {
      char *title;
      char *name;
      char *class;
      char *icon_name;
      char *machine;
      char *role;

      char *min;
      char *max;
      char *base;
      char *step;

      char *aspect;
      char *initial_state;
      char *state;
      char *window_id;
      char *window_group;
      char *transient_for;
      char *client_leader;
      char *gravity;
      char *command;

      int   take_focus;
      int   accepts_focus;
      int   urgent;
      int   delete_request;
      int   request_pos;
   } icccm;

   struct
   {
      char *name;
      char *icon_name;
      int   modal;
      int   sticky;
      int   shaded;
      int   skip_taskbar;
      int   skip_pager;
      int   hidden;
      int   fullscreen;
      char *stacking;
   } netwm;
};

E_API void
e_int_client_prop(E_Client *ec)
{
   E_Dialog *dia;

   if (ec->border_prop_dialog) return;

   dia = e_dialog_new(NULL, "E", "_window_props");
   e_object_del_attach_func_set(E_OBJECT(dia), _ec_cb_dialog_del);

   _create_data(dia, ec);

   _ec_go(dia, (void *)0);

   e_dialog_button_add(dia, _("Close"), NULL, _ec_cb_dialog_close, dia);
   elm_win_center(dia->win, 1, 1);
   e_dialog_show(dia);
   e_dialog_border_icon_set(dia, "preferences-system-windows");
   evas_object_layer_set(e_win_client_get(dia->win)->frame, ec->layer);
}

static void
_create_data(E_Dialog *cfd, E_Client *ec)
{
   E_Config_Dialog_Data *cfdata;
   char buf[4096];

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->client = ec;
   ec->border_prop_dialog = cfd;

#define IFDUP(prop, dest)   \
  if (cfdata->client->prop) \
    cfdata->dest = strdup(cfdata->client->prop)

   IFDUP(icccm.title, icccm.title);
   IFDUP(icccm.name, icccm.name);
   IFDUP(icccm.class, icccm.class);
   IFDUP(icccm.icon_name, icccm.icon_name);
   IFDUP(icccm.machine, icccm.machine);
   IFDUP(icccm.window_role, icccm.role);

   if (cfdata->client->icccm.min_w >= 0)
     {
        snprintf(buf, sizeof(buf), _("%i×%i"),
                 cfdata->client->icccm.min_w,
                 cfdata->client->icccm.min_h);
        cfdata->icccm.min = strdup(buf);
     }
   if (cfdata->client->icccm.max_w >= 0)
     {
        snprintf(buf, sizeof(buf), _("%i×%i"),
                 cfdata->client->icccm.max_w,
                 cfdata->client->icccm.max_h);
        cfdata->icccm.max = strdup(buf);
     }
   if (cfdata->client->icccm.base_w >= 0)
     {
        snprintf(buf, sizeof(buf), _("%i×%i"),
                 cfdata->client->icccm.base_w,
                 cfdata->client->icccm.base_h);
        cfdata->icccm.base = strdup(buf);
     }
   if (cfdata->client->icccm.step_w >= 0)
     {
        snprintf(buf, sizeof(buf), _("%i,%i"),
                 cfdata->client->icccm.step_w,
                 cfdata->client->icccm.step_h);
        cfdata->icccm.step = strdup(buf);
     }
   if ((cfdata->client->icccm.min_aspect > 0.0) &&
       (cfdata->client->icccm.max_aspect > 0.0))
     {
        if (EINA_DBL_EQ(cfdata->client->icccm.min_aspect, cfdata->client->icccm.max_aspect))
          snprintf(buf, sizeof(buf), _("%1.3f"),
                   cfdata->client->icccm.min_aspect);
        else
          snprintf(buf, sizeof(buf), _("%1.3f–%1.3f"),
                   cfdata->client->icccm.min_aspect,
                   cfdata->client->icccm.max_aspect);
        cfdata->icccm.aspect = strdup(buf);
     }
#ifndef HAVE_WAYLAND_ONLY
   if (cfdata->client->icccm.initial_state != ECORE_X_WINDOW_STATE_HINT_NONE)
     {
        switch (cfdata->client->icccm.initial_state)
          {
           case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
             snprintf(buf, sizeof(buf), _("Withdrawn"));
             break;

           case ECORE_X_WINDOW_STATE_HINT_NORMAL:
             snprintf(buf, sizeof(buf), _("Normal"));
             break;

           case ECORE_X_WINDOW_STATE_HINT_ICONIC:
             snprintf(buf, sizeof(buf), _("Iconic"));
             break;

           default:
             buf[0] = 0;
             break;
          }
        cfdata->icccm.initial_state = strdup(buf);
     }
   if (cfdata->client->icccm.state != ECORE_X_WINDOW_STATE_HINT_NONE)
     {
        switch (cfdata->client->icccm.state)
          {
           case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
             snprintf(buf, sizeof(buf), _("Withdrawn"));
             break;

           case ECORE_X_WINDOW_STATE_HINT_NORMAL:
             snprintf(buf, sizeof(buf), _("Normal"));
             break;

           case ECORE_X_WINDOW_STATE_HINT_ICONIC:
             snprintf(buf, sizeof(buf), _("Iconic"));
             break;

           default:
             buf[0] = 0;
             break;
          }
        cfdata->icccm.state = strdup(buf);
     }
#endif
   snprintf(buf, sizeof(buf), "0x%08x",
            (unsigned int)e_client_util_win_get(cfdata->client));
   cfdata->icccm.window_id = strdup(buf);
   if (cfdata->client->icccm.window_group != 0)
     {
        snprintf(buf, sizeof(buf), "0x%08x",
                 (unsigned int)cfdata->client->icccm.window_group);
        cfdata->icccm.window_group = strdup(buf);
     }
   if (cfdata->client->icccm.transient_for != 0)
     {
        snprintf(buf, sizeof(buf), "0x%08x",
                 (unsigned int)cfdata->client->icccm.transient_for);
        cfdata->icccm.transient_for = strdup(buf);
     }
   if (cfdata->client->icccm.client_leader != 0)
     {
        snprintf(buf, sizeof(buf), "0x%08x",
                 (unsigned int)cfdata->client->icccm.client_leader);
        cfdata->icccm.client_leader = strdup(buf);
     }
#ifndef HAVE_WAYLAND_ONLY
   switch (cfdata->client->icccm.gravity)
     {
      case ECORE_X_GRAVITY_FORGET:
        snprintf(buf, sizeof(buf), _("Forget/Unmap"));
        break;

      case ECORE_X_GRAVITY_NW:
        snprintf(buf, sizeof(buf), _("Northwest"));
        break;

      case ECORE_X_GRAVITY_N:
        snprintf(buf, sizeof(buf), _("North"));
        break;

      case ECORE_X_GRAVITY_NE:
        snprintf(buf, sizeof(buf), _("Northeast"));
        break;

      case ECORE_X_GRAVITY_W:
        snprintf(buf, sizeof(buf), _("West"));
        break;

      case ECORE_X_GRAVITY_CENTER:
        snprintf(buf, sizeof(buf), _("Center"));
        break;

      case ECORE_X_GRAVITY_E:
        snprintf(buf, sizeof(buf), _("East"));
        break;

      case ECORE_X_GRAVITY_SW:
        snprintf(buf, sizeof(buf), _("Southwest"));
        break;

      case ECORE_X_GRAVITY_S:
        snprintf(buf, sizeof(buf), _("South"));
        break;

      case ECORE_X_GRAVITY_SE:
        snprintf(buf, sizeof(buf), _("Southeast"));
        break;

      case ECORE_X_GRAVITY_STATIC:
        snprintf(buf, sizeof(buf), _("Static"));
        break;

      default:
        buf[0] = 0;
        break;
     }
   cfdata->icccm.gravity = strdup(buf);
#endif
   if (cfdata->client->icccm.command.argv)
     {
        int i;

        buf[0] = 0;
        for (i = 0; i < cfdata->client->icccm.command.argc; i++)
          {
             if ((sizeof(buf) - strlen(buf)) <
                 (strlen(cfdata->client->icccm.command.argv[i]) - 2))
               break;
             strcat(buf, cfdata->client->icccm.command.argv[i]);
             strcat(buf, " ");
          }
        cfdata->icccm.command = strdup(buf);
     }

   cfdata->icccm.take_focus = cfdata->client->icccm.take_focus;
   cfdata->icccm.accepts_focus = cfdata->client->icccm.accepts_focus;
   cfdata->icccm.urgent = cfdata->client->icccm.urgent;
   cfdata->icccm.delete_request = cfdata->client->icccm.delete_request;
   cfdata->icccm.request_pos = cfdata->client->icccm.request_pos;

   IFDUP(netwm.name, netwm.name);
   IFDUP(netwm.icon_name, netwm.icon_name);
   cfdata->netwm.modal = cfdata->client->netwm.state.modal;
   cfdata->netwm.sticky = cfdata->client->netwm.state.sticky;
   cfdata->netwm.shaded = cfdata->client->netwm.state.shaded;
   cfdata->netwm.skip_taskbar = cfdata->client->netwm.state.skip_taskbar;
   cfdata->netwm.skip_pager = cfdata->client->netwm.state.skip_pager;
   cfdata->netwm.hidden = cfdata->client->netwm.state.hidden;
   cfdata->netwm.fullscreen = cfdata->client->netwm.state.fullscreen;
   switch (cfdata->client->netwm.state.stacking)
     {
      case 0:
        cfdata->netwm.stacking = strdup(_("None"));
        break;

      case 1:
        cfdata->netwm.stacking = strdup(_("Above"));
        break;

      case 2:
        cfdata->netwm.stacking = strdup(_("Below"));
        break;
     }

   cfd->data = cfdata;
}

static void
_free_data(E_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   if (cfdata->client)
     cfdata->client->border_prop_dialog = NULL;

   /* Free the cfdata */
#define IFREE(x) E_FREE(cfdata->x)
   IFREE(icccm.title);
   IFREE(icccm.name);
   IFREE(icccm.class);
   IFREE(icccm.icon_name);
   IFREE(icccm.machine);
   IFREE(icccm.role);
   IFREE(icccm.min);
   IFREE(icccm.max);
   IFREE(icccm.base);
   IFREE(icccm.step);
   IFREE(icccm.aspect);
   IFREE(icccm.initial_state);
   IFREE(icccm.state);
   IFREE(icccm.window_id);
   IFREE(icccm.window_group);
   IFREE(icccm.transient_for);
   IFREE(icccm.client_leader);
   IFREE(icccm.gravity);
   IFREE(icccm.command);

   IFREE(netwm.name);
   IFREE(netwm.icon_name);
   IFREE(netwm.stacking);

   free(cfdata);
   cfd->data = NULL;
}

static void
_ec_cb_dialog_del(void *obj)
{
   E_Dialog *dia;

   dia = obj;
   if (dia->data)
     _free_data(dia, dia->data);
}

static void
_ec_cb_dialog_close(void *data EINA_UNUSED, E_Dialog *dia)
{
   if (dia->data)
     _free_data(dia, dia->data);
   e_object_del(E_OBJECT(dia));
}

static void
_ec_go(void *data, void *data2)
{
   E_Dialog *dia;
   Evas_Object *c, *o, *ob;
   Evas_Coord w, h;

   dia = data;
   if (!dia) return;

   if (dia->content_object)
     evas_object_del(dia->content_object);

   c = e_widget_list_add(evas_object_evas_get(dia->win), 0, 0);

   if (!data2)
     {
        o = _ec_icccm_create(dia, NULL);
        e_dialog_title_set(dia, _("ICCCM Properties"));
        e_widget_list_object_append(c, o, 1, 1, 0.0);
        ob = e_widget_button_add(evas_object_evas_get(dia->win), _("NetWM"), "go-next",
                                 _ec_go, dia, (void *)1);
     }
   else
     {
        o = _ec_netwm_create(dia, NULL);
        e_dialog_title_set(dia, _("NetWM Properties"));
        e_widget_list_object_append(c, o, 1, 1, 0.0);
        ob = e_widget_button_add(evas_object_evas_get(dia->win), _("ICCCM"), "go-next",
                                 _ec_go, dia, (void *)0);
     }

   e_widget_list_object_append(c, ob, 0, 0, 1.0);

   e_widget_size_min_get(c, &w, &h);
   e_dialog_content_set(dia, c, w, h);
   e_dialog_show(dia);
}

#define STR_ENTRY(label, x, y, val)                                    \
  {                                                                    \
     Evas_Coord mw, mh;                                                \
     ob = e_widget_label_add(evas, label);                             \
     if (!cfdata->val) e_widget_disabled_set(ob, 1);                   \
     e_widget_table_object_append(o, ob, x, y, 1, 1, 1, 1, 0, 1);      \
     ob = e_widget_entry_add(dia->win, & (cfdata->val), NULL, NULL, NULL); \
     e_widget_entry_readonly_set(ob, 1);                               \
     e_widget_size_min_get(ob, &mw, &mh);                              \
     e_widget_size_min_set(ob, 160, mh);                               \
     e_widget_table_object_append(o, ob, x + 1, y, 1, 1, 1, 0, 1, 0);  \
  }
#define CHK_ENTRY(label, x, y, val)                                   \
  {                                                                   \
     ob = e_widget_label_add(evas, label);                            \
     e_widget_table_object_append(o, ob, x, y, 1, 1, 1, 1, 0, 1);     \
     ob = e_widget_check_add(evas, "", & (cfdata->val));              \
     e_widget_disabled_set(ob, 1);                                    \
     e_widget_table_object_append(o, ob, x + 1, y, 1, 1, 1, 0, 1, 0); \
  }

static Evas_Object *
_ec_icccm_create(E_Dialog *dia, void *data EINA_UNUSED)
{
   Evas *evas;
   Evas_Object *o, *ob, *otb;
   E_Config_Dialog_Data *cfdata;

   if (!dia) return NULL;
   cfdata = dia->data;

   if (dia->content_object)
     evas_object_del(dia->content_object);

   evas = evas_object_evas_get(dia->win);
   otb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);

   o = e_widget_table_add(e_win_evas_win_get(evas), 0);
   STR_ENTRY(_("Title"), 0, 0, icccm.title);
   STR_ENTRY(_("Name"), 0, 1, icccm.name);
   STR_ENTRY(_("Class"), 0, 2, icccm.class);
   STR_ENTRY(_("Icon Name"), 0, 3, icccm.icon_name);
   STR_ENTRY(_("Machine"), 0, 4, icccm.machine);
   STR_ENTRY(_("Role"), 0, 5, icccm.role);
   e_widget_toolbook_page_append(otb, NULL, _("General"), o, 1, 1, 1, 1, 0.5, 0.0);

   o = e_widget_table_add(e_win_evas_win_get(evas), 0);
   STR_ENTRY(_("Minimum Size"), 0, 6, icccm.min);
   STR_ENTRY(_("Maximum Size"), 0, 7, icccm.max);
   STR_ENTRY(_("Base Size"), 0, 8, icccm.base);
   STR_ENTRY(_("Resize Steps"), 0, 9, icccm.step);
   e_widget_toolbook_page_append(otb, NULL, _("Sizing"), o, 1, 1, 1, 1, 0.5, 0.0);

   o = e_widget_table_add(e_win_evas_win_get(evas), 0);
   STR_ENTRY(_("Aspect Ratio"), 2, 0, icccm.aspect);
   STR_ENTRY(_("Initial State"), 2, 1, icccm.initial_state);
   STR_ENTRY(_("State"), 2, 2, icccm.state);
   STR_ENTRY(_("Window ID"), 2, 3, icccm.window_id);
   STR_ENTRY(_("Window Group"), 2, 4, icccm.window_group);
   STR_ENTRY(_("Transient For"), 2, 5, icccm.transient_for);
   STR_ENTRY(_("Client Leader"), 2, 6, icccm.client_leader);
   STR_ENTRY(_("Gravity"), 2, 7, icccm.gravity);
   STR_ENTRY(_("Command"), 2, 8, icccm.command);
   e_widget_toolbook_page_append(otb, NULL, _("States"), o, 1, 1, 1, 1, 0.5, 0.0);

   o = e_widget_table_add(e_win_evas_win_get(evas), 0);
   CHK_ENTRY(_("Take Focus"), 0, 11, icccm.take_focus);
   CHK_ENTRY(_("Accepts Focus"), 0, 12, icccm.accepts_focus);
   CHK_ENTRY(_("Urgent"), 0, 13, icccm.urgent);
   CHK_ENTRY(_("Request Delete"), 2, 11, icccm.delete_request);
   CHK_ENTRY(_("Request Position"), 2, 12, icccm.request_pos);
   e_widget_toolbook_page_append(otb, NULL, _("Settings"), o, 1, 1, 1, 1, 0.5, 0.0);
   e_widget_toolbook_page_show(otb, 0);

   return otb;
}

static Evas_Object *
_ec_netwm_create(E_Dialog *dia, void *data EINA_UNUSED)
{
   Evas *evas;
   Evas_Object *o, *ob, *otb;
   E_Config_Dialog_Data *cfdata;

   if (!dia) return NULL;
   cfdata = dia->data;

   if (dia->content_object)
     evas_object_del(dia->content_object);

   evas = evas_object_evas_get(dia->win);
   otb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);
   o = e_widget_table_add(e_win_evas_win_get(evas), 0);
   STR_ENTRY(_("Name"), 0, 1, netwm.name);
   STR_ENTRY(_("Icon Name"), 0, 2, netwm.icon_name);
   STR_ENTRY(_("Stacking"), 0, 3, netwm.stacking);
   e_widget_toolbook_page_append(otb, NULL, _("General"), o, 1, 1, 1, 1, 0.5, 0.0);

   o = e_widget_table_add(e_win_evas_win_get(evas), 0);
   CHK_ENTRY(_("Modal"), 0, 4, netwm.modal);
   CHK_ENTRY(_("Sticky"), 0, 5, netwm.sticky);
   CHK_ENTRY(_("Shaded"), 0, 6, netwm.shaded);
   CHK_ENTRY(_("Skip Taskbar"), 0, 7, netwm.skip_taskbar);
   CHK_ENTRY(_("Skip Pager"), 0, 8, netwm.skip_pager);
   CHK_ENTRY(_("Hidden"), 0, 9, netwm.hidden);
   CHK_ENTRY(_("Fullscreen"), 0, 10, netwm.fullscreen);
   e_widget_toolbook_page_append(otb, NULL, _("Settings"), o, 1, 1, 1, 1, 0.5, 0.0);
   e_widget_toolbook_page_show(otb, 0);

   return otb;
}

