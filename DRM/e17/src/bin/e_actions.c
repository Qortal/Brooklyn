#include "e.h"

#define INITS
#define ACT_GO(name)                                      \
  {                                                       \
     act = e_action_add(#name);                           \
     if (act) act->func.go = _e_actions_act_##name##_go;  \
  }
#define ACT_FN_GO(act, use) \
  static void _e_actions_act_##act##_go(E_Object * obj EINA_UNUSED, const char *params use)

#define ACT_GO_MOUSE(name)                                            \
  {                                                                   \
     act = e_action_add(#name);                                       \
     if (act) act->func.go_mouse = _e_actions_act_##name##_go_mouse;  \
  }
#define ACT_FN_GO_MOUSE(act, use) \
  static Eina_Bool _e_actions_act_##act##_go_mouse(E_Object * obj EINA_UNUSED, const char *params use, E_Binding_Event_Mouse_Button * ev EINA_UNUSED)

#define ACT_GO_WHEEL(name)                                            \
  {                                                                   \
     act = e_action_add(#name);                                       \
     if (act) act->func.go_wheel = _e_actions_act_##name##_go_wheel;  \
  }
#define ACT_FN_GO_WHEEL(act, use) \
  static Eina_Bool _e_actions_act_##act##_go_wheel(E_Object * obj EINA_UNUSED, const char *params use, E_Binding_Event_Wheel * ev EINA_UNUSED)

#define ACT_GO_EDGE(name)                                           \
  {                                                                 \
     act = e_action_add(#name);                                     \
     if (act) act->func.go_edge = _e_actions_act_##name##_go_edge;  \
  }
#define ACT_FN_GO_EDGE(act, use) \
  static void _e_actions_act_##act##_go_edge(E_Object * obj EINA_UNUSED, const char *params use, E_Event_Zone_Edge * ev EINA_UNUSED)

#define ACT_GO_SIGNAL(name)                                             \
  {                                                                     \
     act = e_action_add(#name);                                         \
     if (act) act->func.go_signal = _e_actions_act_##name##_go_signal;  \
  }
#define ACT_FN_GO_SIGNAL(act, use) \
  static void _e_actions_act_##act##_go_signal(E_Object * obj EINA_UNUSED, const char *params use, const char *sig, const char *src)

#define ACT_GO_KEY(name)                                          \
  {                                                               \
     act = e_action_add(#name);                                   \
     if (act) act->func.go_key = _e_actions_act_##name##_go_key;  \
  }
#define ACT_FN_GO_KEY(act, use1, use2) \
  static void _e_actions_act_##act##_go_key(E_Object * obj EINA_UNUSED, const char *params use1, Ecore_Event_Key * ev use2)

#define ACT_END(name)                                       \
  {                                                         \
     act = e_action_add(#name);                             \
     if (act) act->func.end = _e_actions_act_##name##_end;  \
  }
#define ACT_FN_END(act, use) \
  static void _e_actions_act_##act##_end(E_Object * obj EINA_UNUSED, const char *params use)

#define ACT_END_MOUSE(name)                                             \
  {                                                                     \
     act = e_action_add(#name);                                         \
     if (act) act->func.end_mouse = _e_actions_act_##name##_end_mouse;  \
  }
#define ACT_FN_END_MOUSE(act, use) \
  static Eina_Bool _e_actions_act_##act##_end_mouse(E_Object * obj EINA_UNUSED, const char *params use, E_Binding_Event_Mouse_Button * ev EINA_UNUSED)

#define ACT_END_KEY(name)                                           \
  {                                                                 \
     act = e_action_add(#name);                                     \
     if (act) act->func.end_key = _e_actions_act_##name##_end_key;  \
  }
#define ACT_FN_END_KEY(act, use) \
  static void _e_actions_act_##act##_end_key(E_Object * obj EINA_UNUSED, const char *params use, Ecore_Event_Key * ev EINA_UNUSED)

#define ACT_GO_ACPI(name)                                           \
  {                                                                 \
     act = e_action_add(#name);                                     \
     if (act) act->func.go_acpi = _e_actions_act_##name##_go_acpi;  \
  }
#define ACT_FN_GO_ACPI(act, use) \
  static void _e_actions_act_##act##_go_acpi(E_Object * obj EINA_UNUSED, const char *params use, E_Event_Acpi * ev EINA_UNUSED)

/* local functions forward declarations (window_jump_to needs the definition of exec) */
ACT_FN_GO(exec, );

/* local subsystem functions */
static void       _e_action_free(E_Action *act);
static E_Maximize _e_actions_maximize_parse(const char *maximize);
static int        _action_groups_sort_cb(const void *d1, const void *d2);

/* to save writing this in N places - the sections are defined here */
/***************************************************************************/
ACT_FN_GO(window_move, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE) return;
   if (!((E_Client *)(void *)obj)->lock_user_location)
     e_client_act_move_begin((E_Client *)(void *)obj, NULL);
}

ACT_FN_GO_MOUSE(window_move, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return EINA_FALSE;
   if (obj->type != E_CLIENT_TYPE) return EINA_FALSE;
   e_client_act_move_begin((E_Client *)(void *)obj, ev);
   return EINA_TRUE;
}

ACT_FN_GO_SIGNAL(window_move, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE) return;
   if (!((E_Client *)(void *)obj)->lock_user_location)
     {
        if ((params) && (!strcmp(params, "end")))
          e_client_signal_move_end((E_Client *)(void *)obj, sig, src);
        else
          {
             if (!((E_Client *)(void *)obj)->moving)
               e_client_signal_move_begin((E_Client *)(void *)obj, sig, src);
          }
     }
}

ACT_FN_END(window_move, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE) return;
   e_client_act_move_end((E_Client *)(void *)obj, NULL);
}

ACT_FN_END_MOUSE(window_move, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return EINA_FALSE;
   if (obj->type != E_CLIENT_TYPE) return EINA_FALSE;
   e_client_act_move_end((E_Client *)(void *)obj, ev);
   return EINA_TRUE;
}

ACT_FN_GO_KEY(window_move, EINA_UNUSED, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_location)
     e_client_act_move_keyboard((E_Client *)(void *)obj);
}

/***************************************************************************/
ACT_FN_GO(window_resize, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_size)
     e_client_act_resize_begin((E_Client *)(void *)obj, NULL);
}

ACT_FN_GO_MOUSE(window_resize, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return EINA_FALSE;
   if (obj->type != E_CLIENT_TYPE) return EINA_FALSE;
   if (!((E_Client *)(void *)obj)->lock_user_size)
     e_client_act_resize_begin((E_Client *)(void *)obj, ev);
   return EINA_TRUE;
}

ACT_FN_GO_SIGNAL(window_resize, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE) return;
   if (!((E_Client *)(void *)obj)->lock_user_size)
     {
        if ((params) && (!strcmp(params, "end")))
          e_client_signal_resize_end((E_Client *)(void *)obj, params, sig, src);
        else
          {
             if (!params) params = "";
             if (e_client_util_resizing_get((E_Client *)(void *)obj))
               e_client_signal_resize_end((E_Client *)(void *)obj, params, sig, src);
             else
               e_client_signal_resize_begin((E_Client *)(void *)obj, params, sig, src);
          }
     }
}

ACT_FN_END(window_resize, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE) return;
   e_client_act_resize_end((E_Client *)(void *)obj, NULL);
}

ACT_FN_END_MOUSE(window_resize, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return EINA_FALSE;
   if (obj->type != E_CLIENT_TYPE) return EINA_FALSE;
   e_client_act_resize_end((E_Client *)(void *)obj, ev);
   return EINA_TRUE;
}

ACT_FN_GO_KEY(window_resize, EINA_UNUSED, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE)
     {
        obj = E_OBJECT(e_client_focused_get());
        if (!obj) return;
     }
   if (!((E_Client *)(void *)obj)->lock_user_size)
     e_client_act_resize_keyboard((E_Client *)(void *)obj);
}

/***************************************************************************/
ACT_FN_GO(window_menu, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   e_client_act_menu_begin((E_Client *)(void *)obj, NULL, 0);
}

ACT_FN_GO_MOUSE(window_menu, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return EINA_FALSE;
   if (obj->type != E_CLIENT_TYPE)
     {
        obj = E_OBJECT(e_client_focused_get());
        if (!obj) return EINA_FALSE;
     }
   e_client_act_menu_begin((E_Client *)(void *)obj, ev, 0);
   return EINA_TRUE;
}

ACT_FN_GO_KEY(window_menu, EINA_UNUSED, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE)
     {
        obj = E_OBJECT(e_client_focused_get());
        if (!obj) return;
     }
   e_client_act_menu_begin((E_Client *)(void *)obj, NULL, 1);
}

/***************************************************************************/
ACT_FN_GO(window_raise, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_stacking)
     evas_object_raise(((E_Client *)(void *)obj)->frame);
}

/***************************************************************************/
ACT_FN_GO(window_lower, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_stacking)
     evas_object_lower(((E_Client *)(void *)obj)->frame);
}

/***************************************************************************/
ACT_FN_GO(window_close, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_close)
     e_client_act_close_begin((E_Client *)(void *)obj);
}

/***************************************************************************/
static E_Dialog *kill_dialog = NULL;

static void
_e_actions_cb_kill_dialog_ok(void *data, E_Dialog *dia)
{
   E_Object *obj;

   obj = data;
   if (dia)
     {
        e_object_del(E_OBJECT(kill_dialog));
        kill_dialog = NULL;
     }
   if ((!((E_Client *)(void *)obj)->lock_close) && (!((E_Client *)(void *)obj)->internal))
     e_client_act_kill_begin((E_Client *)(void *)obj);
}

static void
_e_actions_cb_kill_dialog_cancel(void *data EINA_UNUSED, E_Dialog *dia EINA_UNUSED)
{
   e_object_del(E_OBJECT(kill_dialog));
   kill_dialog = NULL;
}

static void
_e_actions_cb_kill_dialog_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _e_actions_cb_kill_dialog_cancel(NULL, data);
}

ACT_FN_GO(window_kill, EINA_UNUSED)
{
   E_Client *ec;
   char dialog_text[1024];

   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   ec = (E_Client *)(void *)obj;
   if ((ec->lock_close) || (ec->internal)) return;

   if (kill_dialog) e_object_del(E_OBJECT(kill_dialog));

   if (e_config->cnfmdlg_disabled)
     {
        _e_actions_cb_kill_dialog_ok(obj, NULL);
        return;
     }

   snprintf(dialog_text, sizeof(dialog_text),
            _("You are about to kill %s.<ps/><ps/>"
              "Please keep in mind that all data from this window<ps/>"
              "which has not yet been saved will be lost!<ps/><ps/>"
              "Are you sure you want to kill this window?"),
            ec->icccm.name);

   kill_dialog = e_dialog_new(NULL,
                              "E", "_kill_dialog");
   if (!kill_dialog) return;
   evas_object_event_callback_add(kill_dialog->win, EVAS_CALLBACK_DEL,
                             _e_actions_cb_kill_dialog_delete, kill_dialog);
   e_dialog_title_set(kill_dialog,
                      _("Are you sure you want to kill this window?"));
   e_dialog_text_set(kill_dialog, _(dialog_text));
   e_dialog_icon_set(kill_dialog, "application-exit", 64);
   e_dialog_button_add(kill_dialog, _("Kill"), NULL,
                       _e_actions_cb_kill_dialog_ok, obj);
   e_dialog_button_add(kill_dialog, _("No"), NULL,
                       _e_actions_cb_kill_dialog_cancel, NULL);
   e_dialog_button_focus_num(kill_dialog, 1);
   elm_win_center(kill_dialog->win, 1, 1);
   e_dialog_show(kill_dialog);
}

/***************************************************************************/
ACT_FN_GO(window_sticky_toggle, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_sticky)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (ec->sticky) e_client_unstick(ec);
        else e_client_stick(ec);
     }
}

/***************************************************************************/
ACT_FN_GO(window_sticky, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_sticky)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (params)
          {
             if (atoi(params) == 1)
               e_client_stick(ec);
             else if (atoi(params) == 0)
               e_client_unstick(ec);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_iconic_toggle, EINA_UNUSED)
{
   E_Client *ec;

   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   ec = (E_Client *)(void *)obj;

   if ((!ec->lock_user_iconify) && (!ec->fullscreen) &&
       ((ec->netwm.type == E_WINDOW_TYPE_NORMAL) ||
        (ec->netwm.type == E_WINDOW_TYPE_UNKNOWN)))
     {
        if (ec->iconic) e_client_uniconify(ec);
        else e_client_iconify(ec);
     }
}

/***************************************************************************/
ACT_FN_GO(window_iconic, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_iconify)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (params)
          {
             if (atoi(params) == 1)
               e_client_iconify(ec);
             else if (atoi(params) == 0)
               e_client_uniconify(ec);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_fullscreen_toggle, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_fullscreen)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (ec->fullscreen)
          e_client_unfullscreen(ec);
        else if (!params || *params == '\0')
          e_client_fullscreen(ec, e_config->fullscreen_policy);
        else if (!strcmp(params, "resize"))
          e_client_fullscreen(ec, E_FULLSCREEN_RESIZE);
        else if (!strcmp(params, "zoom"))
          e_client_fullscreen(ec, E_FULLSCREEN_ZOOM);
     }
}

/***************************************************************************/
ACT_FN_GO(window_fullscreen, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_fullscreen)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (params)
          {
             int v;
             char buf[32];

             buf[0] = 0;
             if (sscanf(params, "%i %20s", &v, buf) == 2)
               {
                  if (v == 1)
                    {
                       if (*buf == '\0')
                         e_client_fullscreen(ec, e_config->fullscreen_policy);
                       else if (!strcmp(buf, "resize"))
                         e_client_fullscreen(ec, E_FULLSCREEN_RESIZE);
                       else if (!strcmp(buf, "zoom"))
                         e_client_fullscreen(ec, E_FULLSCREEN_ZOOM);
                    }
                  else if (v == 0)
                    e_client_unfullscreen(ec);
               }
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_maximized_toggle, )
{
   E_Client *ec;
   Eina_Bool resize = EINA_FALSE;

   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   ec = (E_Client *)(void *)obj;

   /* internal dialog which is resizable */
   if (ec->internal && (ec->netwm.type == E_WINDOW_TYPE_DIALOG))
     resize = (ec->icccm.max_w != ec->icccm.min_w);
   if ((!ec->lock_user_maximize) && (!ec->fullscreen) &&
       (resize || (ec->netwm.type == E_WINDOW_TYPE_NORMAL) ||
        (ec->netwm.type == E_WINDOW_TYPE_UNKNOWN)))
     {
        if ((ec->maximized & E_MAXIMIZE_TYPE) != E_MAXIMIZE_NONE)
          {
             if (!params)
               e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
             else
               {
                  E_Maximize max;

                  max = _e_actions_maximize_parse(params);
                  max &= E_MAXIMIZE_DIRECTION;
                  if (max == E_MAXIMIZE_VERTICAL)
                    {
                       if (ec->maximized & E_MAXIMIZE_VERTICAL)
                         e_client_unmaximize(ec, E_MAXIMIZE_VERTICAL);
                       else
                         goto maximize;
                    }
                  else if (max == E_MAXIMIZE_HORIZONTAL)
                    {
                       if (ec->maximized & E_MAXIMIZE_HORIZONTAL)
                         e_client_unmaximize(ec, E_MAXIMIZE_HORIZONTAL);
                       else
                         goto maximize;
                    }
                  else if (max == E_MAXIMIZE_LEFT)
                    {
                       if (ec->maximized & E_MAXIMIZE_LEFT)
                         e_client_unmaximize(ec, E_MAXIMIZE_LEFT);
                       else
                         goto maximize;
                    }
                  else if (max == E_MAXIMIZE_RIGHT)
                    {
                       if (ec->maximized & E_MAXIMIZE_RIGHT)
                         e_client_unmaximize(ec, E_MAXIMIZE_RIGHT);
                       else
                         goto maximize;
                    }
                  else
                    e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
               }
          }
        else
          {
maximize:
             e_client_maximize(ec, _e_actions_maximize_parse(params));
          }
     }
}
/***************************************************************************/
ACT_FN_GO(window_maximized, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_maximize)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (params)
          {
             E_Maximize max;
             int v, ret;
             char s1[32], s2[32];

             max = (e_config->maximize_policy & E_MAXIMIZE_DIRECTION);
             ret = sscanf(params, "%i %20s %20s", &v, s1, s2);
             if (ret == 3)
               {
                  if (!strcmp(s2, "horizontal"))
                    max = E_MAXIMIZE_HORIZONTAL;
                  else if (!strcmp(s2, "vertical"))
                    max = E_MAXIMIZE_VERTICAL;
                  else if (!strcmp(s2, "left"))
                    max = E_MAXIMIZE_LEFT;
                  else if (!strcmp(s2, "right"))
                    max = E_MAXIMIZE_RIGHT;
                  else
                    max = E_MAXIMIZE_BOTH;
               }
             if (ret > 1)
               {
                  if (v == 1)
                    {
                       if (!strcmp(s1, "fullscreen"))
                         e_client_maximize(ec, E_MAXIMIZE_FULLSCREEN | max);
                       else if (!strcmp(s1, "smart"))
                         e_client_maximize(ec, E_MAXIMIZE_SMART | max);
                       else if (!strcmp(s1, "expand"))
                         e_client_maximize(ec, E_MAXIMIZE_EXPAND | max);
                       else if (!strcmp(s1, "fill"))
                         e_client_maximize(ec, E_MAXIMIZE_FILL | max);
                       else
                         e_client_maximize(ec, (e_config->maximize_policy & E_MAXIMIZE_TYPE) | max);
                    }
                  else if (v == 0)
                    e_client_unmaximize(ec, max);
               }
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_shaded_toggle, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_shade)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (ec->shaded)
          {
             if (!params)
               e_client_unshade(ec, E_DIRECTION_UP);
             else
               {
                  if (!strcmp(params, "up"))
                    e_client_unshade(ec, E_DIRECTION_UP);
                  else if (!strcmp(params, "down"))
                    e_client_unshade(ec, E_DIRECTION_DOWN);
                  else if (!strcmp(params, "left"))
                    e_client_unshade(ec, E_DIRECTION_LEFT);
                  else if (!strcmp(params, "right"))
                    e_client_unshade(ec, E_DIRECTION_RIGHT);
                  else
                    e_client_unshade(ec, E_DIRECTION_UP);
               }
          }
        else
          {
             if (!params)
               e_client_shade(ec, E_DIRECTION_UP);
             else
               {
                  if (!strcmp(params, "up"))
                    e_client_shade(ec, E_DIRECTION_UP);
                  else if (!strcmp(params, "down"))
                    e_client_shade(ec, E_DIRECTION_DOWN);
                  else if (!strcmp(params, "left"))
                    e_client_shade(ec, E_DIRECTION_LEFT);
                  else if (!strcmp(params, "right"))
                    e_client_shade(ec, E_DIRECTION_RIGHT);
                  else
                    e_client_shade(ec, E_DIRECTION_UP);
               }
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_shaded, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_user_shade)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (!e_comp_object_frame_allowed(ec->frame)) return;
        if (params)
          {
             int v;
             char buf[32];

             if (sscanf(params, "%i %20s", &v, buf) == 2)
               {
                  if (v == 1)
                    {
                       if (!strcmp(buf, "up"))
                         e_client_shade(ec, E_DIRECTION_UP);
                       else if (!strcmp(buf, "down"))
                         e_client_shade(ec, E_DIRECTION_DOWN);
                       else if (!strcmp(buf, "left"))
                         e_client_shade(ec, E_DIRECTION_LEFT);
                       else if (!strcmp(buf, "right"))
                         e_client_shade(ec, E_DIRECTION_RIGHT);
                    }
                  else if (v == 0)
                    {
                       if (!strcmp(buf, "up"))
                         e_client_unshade(ec, E_DIRECTION_UP);
                       else if (!strcmp(buf, "down"))
                         e_client_unshade(ec, E_DIRECTION_DOWN);
                       else if (!strcmp(buf, "left"))
                         e_client_unshade(ec, E_DIRECTION_LEFT);
                       else if (!strcmp(buf, "right"))
                         e_client_unshade(ec, E_DIRECTION_RIGHT);
                    }
               }
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_borderless_toggle, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_border)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (!e_comp_object_frame_allowed(ec->frame)) return;
        ec->borderless = !ec->borderless;

        ec->border.changed = 1;
        EC_CHANGED(ec);
     }
}

/***************************************************************************/
ACT_FN_GO(window_border_set, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_border)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (!e_comp_object_frame_allowed(ec->frame)) return;
        if (ec && params)
          {
             eina_stringshare_replace(&ec->bordername, params);
             ec->border.changed = 1;
             EC_CHANGED(ec);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_border_cycle, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_border)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if (!e_comp_object_frame_allowed(ec->frame)) return;
        if (ec && params)
          {
             const char *space;

             while (*params == ' ')
               params++;

             if (ec->bordername)
               {
                  const char *bdname = params;

                  while ((space = strchr(bdname, ' ')))
                    {
                       if (strncmp(ec->bordername, bdname, space - bdname) == 0)
                         {
                            bdname = space + 1;
                            while (*bdname == ' ')
                              bdname++;
                            space = strchr(bdname, ' ');
                            if (space)
                              eina_stringshare_replace_length(
                                &ec->bordername,
                                bdname, space - bdname);
                            else
                              eina_stringshare_replace(&ec->bordername, bdname);
                            ec->border.changed = 1;
                            EC_CHANGED(ec);
                            return;
                         }
                       bdname = space + 1;
                       while (*bdname == ' ')
                         bdname++;
                    }
               }

             space = strchr(params, ' ');
             if (space)
               eina_stringshare_replace_length(&ec->bordername,
                                               params, space - params);
             else
               eina_stringshare_replace(&ec->bordername, params);
             ec->border.changed = 1;
             EC_CHANGED(ec);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_pinned_toggle, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (!((E_Client *)(void *)obj)->lock_border)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if ((ec->netwm.state.stacking == E_STACKING_BELOW) &&
            (ec->user_skip_winlist) && (ec->borderless))
          e_client_pinned_set(ec, 0);
        else
          e_client_pinned_set(ec, 1);
     }
}

/***************************************************************************/
ACT_FN_GO(window_move_by, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (obj->type != E_CLIENT_TYPE)
     {
        obj = E_OBJECT(e_client_focused_get());
        if (!obj) return;
     }
   if (params)
     {
        int dx, dy;

        if (sscanf(params, "%i %i", &dx, &dy) == 2)
          {
             E_Client *ec;

             ec = (E_Client *)(void *)obj;
             evas_object_move(ec->frame, ec->x + dx, ec->y + dy);

             if (!e_client_focus_policy_click(ec))
               e_client_pointer_warp_to_center_now(ec);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_move_to, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   if (params)
     {
        E_Client *ec;
        int x, y, zx, zy, zw, zh;
        char cx, cy;

        ec = (E_Client *)(void *)obj;
        e_zone_useful_geometry_get(ec->zone, &zx, &zy, &zw, &zh);

        if (sscanf(params, "%c%i %c%i", &cx, &x, &cy, &y) == 4)
          {
             x += zx;
             y += zy;
          }
        else if (sscanf(params, "* %c%i", &cy, &y) == 2)
          {
             /* Updated y, keep x. */
             y += zy;
             x = ec->x;
             cx = 0;
          }
        else if (sscanf(params, "%c%i *", &cx, &x) == 2)
          {
             /* Updated x, keep y. */
             x += zx;
             y = ec->y;
             cy = 0;
          }
        else return;

        if (cx == '-') x = zw - ec->w - x + 2 * zx;  /* compensate x with zx */
        if (cy == '-') y = zh - ec->h - y + 2 * zy;  /* compensate y with zy */

        if ((x != ec->x) || (y != ec->y))
          {
             evas_object_move(ec->frame, x, y);

             if (!e_client_focus_policy_click(ec))
               e_client_pointer_warp_to_center_now(ec);
          }
     }
}


/***************************************************************************/
ACT_FN_GO(window_quick_tile_to_quadrant, )
{
   E_Client *ec;
   int x, y, zx, zy, zw, zh;

   if ((!obj) || (obj->type != E_CLIENT_TYPE))
      obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;

   ec = (E_Client *)(void *)obj;
   if((ec->maximized & E_MAXIMIZE_TYPE) != E_MAXIMIZE_NONE)
      e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
   e_zone_useful_geometry_get(ec->zone, &zx, &zy, &zw, &zh);

   if (params)
     {
        if (strcmp(params, "upper_left") == 0)
          {
             x = zx;
             y = zy;
          }
        else if (strcmp(params, "upper_right") == 0)
          {
             x = zx + (zw / 2);
             y = zy;
          }
        else if (strcmp(params, "lower_left") == 0)
          {
             x = zx;
             y = zy + (zh / 2);
          }
        else if (strcmp(params, "lower_right") == 0)
          {
             x = zx + (zw / 2);
             y = zy + (zh / 2);
          }
        else
          return;

        evas_object_geometry_set(ec->frame, x, y, zw / 2, zh / 2);
     }
}


/***************************************************************************/
ACT_FN_GO(window_move_to_center, EINA_UNUSED)
{
   E_Client *ec;

   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;

   ec = (E_Client *)(void *)obj;
   e_comp_object_util_center(ec->frame);

   if (!e_client_focus_policy_click(ec))
     e_client_pointer_warp_to_center_now(ec);
}

/***************************************************************************/
ACT_FN_GO(window_resize_by, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;

   if (params)
     {
        int dw, dh;

        if (sscanf(params, "%i %i", &dw, &dh) == 2)
          {
             E_Client *ec;

             ec = (E_Client *)(void *)obj;

             dw += ec->w;
             dh += ec->h;
             e_client_resize_limit(ec, &dw, &dh);
             evas_object_resize(ec->frame, dw, dh);

             if (!e_client_focus_policy_click(ec))
               e_client_pointer_warp_to_center_now(ec);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_push, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;

   if (params)
     {
        E_Client *ec, *cur;
        E_Desk *desk_current;
        int hdir, vdir;
        int x, y, zx, zy, zw, zh;

        if (strcmp(params, "left") == 0)
          {
             hdir = -1;
             vdir = 0;
          }
        else if (strcmp(params, "right") == 0)
          {
             hdir = +1;
             vdir = 0;
          }
        else if (strcmp(params, "up") == 0)
          {
             hdir = 0;
             vdir = -1;
          }
        else if (strcmp(params, "down") == 0)
          {
             hdir = 0;
             vdir = +1;
          }
        else if (strcmp(params, "up-left") == 0)
          {
             hdir = -1;
             vdir = -1;
          }
        else if (strcmp(params, "up-right") == 0)
          {
             hdir = +1;
             vdir = -1;
          }
        else if (strcmp(params, "down-left") == 0)
          {
             hdir = -1;
             vdir = +1;
          }
        else if (strcmp(params, "down-right") == 0)
          {
             hdir = +1;
             vdir = +1;
          }
        else
          return;

        ec = (E_Client *)(void *)obj;
        e_zone_useful_geometry_get(ec->zone, &zx, &zy, &zw, &zh);

        if (hdir < 0) x = zx;
        else if (hdir > 0)
          x = zx + zw - ec->w;
        else x = ec->x;

        if (vdir < 0) y = zy;
        else if (vdir > 0)
          y = zy + zh - ec->h;
        else y = ec->y;

        desk_current = e_desk_current_get(ec->zone);

        E_CLIENT_FOREACH(cur)
          {
             if (((cur->desk == desk_current) || (cur->sticky)) && (ec != cur) && (!cur->iconic))
               {
                  if ((hdir < 0)
                      && (cur->x + cur->w < ec->x)
                      && (E_SPANS_COMMON(ec->y, ec->h, cur->y, cur->h)))
                    x = MAX(x, cur->x + cur->w);
                  else if ((hdir > 0)
                           && (cur->x > ec->x + ec->w)
                           && (E_SPANS_COMMON(ec->y, ec->h, cur->y, cur->h)))
                    x = MIN(x, zx + cur->x - ec->w);

                  if ((vdir < 0)
                      && (cur->y + cur->h < ec->y)
                      && (E_SPANS_COMMON(ec->x, ec->w, cur->x, cur->w)))
                    y = MAX(y, cur->y + cur->h);
                  else if ((vdir > 0)
                           && (cur->y > ec->y + ec->h)
                           && (E_SPANS_COMMON(ec->x, ec->w, cur->x, cur->w)))
                    y = MIN(y, zy + cur->y - ec->h);
               }
          }

        if ((x != ec->x) || (y != ec->y))
          {
             evas_object_move(ec->frame, x, y);
             if (!e_client_focus_policy_click(ec))
               e_client_pointer_warp_to_center_now(ec);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_grow, )
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;

   if (params)
     {
        E_Client *ec = (E_Client *)((void *)obj), *cur;
        E_Desk *desk_current;
        int hdir = 0, vdir = 0;
        int x1, y1, x2, y2, w, h;
        int zx, zy, zw, zh;

        if (!strcmp(params, "left"))
          hdir = -1;
        else if (!strcmp(params, "right"))
          hdir = 1;
        else if (!strcmp(params, "up"))
          vdir = -1;
        else if (!strcmp(params, "down"))
          vdir = 1;
        else if (!strcmp(params, "up-left"))
          {
             hdir = -1;
             vdir = -1;
          }
        else if (!strcmp(params, "up-right"))
          {
             hdir =  1;
             vdir = -1;
          }
        else if (!strcmp(params, "down-left"))
          {
             hdir = -1;
             vdir =  1;
          }
        else if (!strcmp(params, "down-right"))
          {
             hdir =  1;
             vdir =  1;
          }
        else
          return;

        e_zone_useful_geometry_get(ec->zone, &zx, &zy, &zw, &zh);

        x1 = ec->x;
        y1 = ec->y;
        x2 = ec->x + ec->w;
        y2 = ec->y + ec->h;

        if (hdir < 0)
          x1 = zx;
        else if (hdir > 0)
          x2 = zx + zw;

        if (vdir < 0)
          y1 = zy;
        else if (vdir > 0)
          y2 = zy + zh;

        w = x2 - x1;
        h = y2 - y1;

        e_client_resize_limit(ec, &w, &h);

        desk_current = e_desk_current_get(ec->zone);

        E_CLIENT_FOREACH(cur)
          {
             if (((cur->desk == desk_current) || (cur->sticky)) && (ec != cur) && (!cur->iconic))
               {
                  if ((hdir < 0)
                      && ((cur->x + cur->w) < ec->x)
                      && (E_SPANS_COMMON(ec->y, ec->h, cur->y, cur->h)))
                    {
                       int nw = ec->w + (ec->x - cur->x - cur->w);
                       int nh = h;
                       e_client_resize_limit(ec, &nw, &nh);
                       if ((nw > ec->w) && (nw < w))
                         {
                            x1 = cur->x + cur->w;
                            w = nw;
                            h = nh;
                         }
                    }
                  else if ((hdir > 0)
                           && (cur->x > (ec->x + ec->w))
                           && (E_SPANS_COMMON(ec->y, ec->h, cur->y, cur->h)))
                    {
                       int nw = cur->x - x1;
                       int nh = h;
                       e_client_resize_limit(ec, &nw, &nh);
                       if ((nw > ec->w) && (nw < w))
                         {
                            w = nw;
                            h = nh;
                         }
                    }

                  if ((vdir < 0)
                      && ((cur->y + cur->h) < ec->y)
                      && (E_SPANS_COMMON(ec->x, ec->w, cur->x, cur->w)))
                    {
                       int nw = w;
                       int nh = y2 - (cur->y + cur->h);
                       e_client_resize_limit(ec, &nw, &nh);
                       if ((nh > ec->h) && (nh < h))
                         {
                            y1 = cur->y + cur->h;
                            w = nw;
                            h = nh;
                         }
                    }
                  else if ((vdir > 0)
                           && (cur->y > (ec->y + ec->h))
                           && (E_SPANS_COMMON(ec->x, ec->w, cur->x, cur->w)))
                    {
                       int nw = w;
                       int nh = cur->y - y1;
                       e_client_resize_limit(ec, &nw, &nh);
                       if ((nh > ec->h) && (nh < h))
                         {
                            w = nw;
                            h = nh;
                         }
                    }
               }
          }

        if ((x1 != ec->x) || (y1 != ec->y) || (w != ec->w) || (h != ec->h))
          {
             evas_object_move(ec->frame, x1, y1);
             evas_object_resize(ec->frame, w, h);
             if (!e_client_focus_policy_click(ec))
               e_client_pointer_warp_to_center_now(ec);
          }
     }
}

/*
 * These actions jump to a window with the given name. It uses the last focused
 * window it finds (going through e_client_focus_stack_get), so the name should
 * be unique, if you want to jump to a specific window.
 *
 */
static int
window_jump_to(const char *params)
{
   Eina_List *l, *ll;
   E_Zone *current_zone;
   E_Client *ec;

   if (!params) return 0;

   /* Go through the list of all windows (that's what the focus stack is used for) */
   l = e_client_focus_stack_get();
   /* If we can start at the second window, we start there in order to cycle.
    * If not, there is only one window, so let's use it */
   if (eina_list_next(l))
     l = l->next;
   EINA_LIST_FOREACH(l, ll, ec)
     {
        if (!ec->icccm.name) continue;
        if (strcasecmp(ec->icccm.name, params)) continue;
        /* Jump to the screen the window is on if it isn't on the current screen but
         * only if we don't have to warp the pointer anyway */
        current_zone = e_zone_current_get();

        /* Change the virtual desktop if the window isn't on the current virtual desktop */
        e_desk_show(ec->desk);

        /* A minimized window won't be focusable for key input, un-minimize it */
        if (!ec->lock_user_iconify)
          e_client_uniconify(ec);


        evas_object_raise(ec->frame);
        if (ec->zone != current_zone)
          {
             if (!e_client_focus_policy_click(ec))
               e_client_pointer_warp_to_center_now(ec);
          }
        evas_object_focus_set(ec->frame, 1);
        return 1;
     }
   return 0;
}

ACT_FN_GO(window_jump_to, )
{
   window_jump_to(params);
}

ACT_FN_GO(window_jump_to_or_start, )
{
   char *window_name, *start_name;
   if (!params) return;

   window_name = strdup(params);
   start_name = window_name;
   start_name = strchr(window_name, ' ');
   if (start_name)
     {
        start_name[0] = 0;
        start_name++;
     }
   else
     {
        /* The user just specified one parameter, just call window_jump_to */
        window_jump_to(window_name);
        free(window_name);
        return;
     }

   /* If we cannot jump to the specified window... */
   if (!window_jump_to(window_name))
     {
        /* ...start application by calling "exec" action */
        _e_actions_act_exec_go(obj, start_name);
     }
   free(window_name);
}

/***************************************************************************/
ACT_FN_GO(window_drag_icon, EINA_UNUSED)
{
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj) return;
   {
      E_Client *ec;

      ec = (E_Client *)(void *)obj;
      ec->drag.start = 1;
      ec->drag.x = -1;
      ec->drag.y = -1;
   }
}

/***************************************************************************/
ACT_FN_GO(window_desk_move_by, )
{
   E_Client *ec;
   int x, y;

   if (!params) return;
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj)
     obj = E_OBJECT(e_client_action_get());
   if (!obj) return;

   ec = (E_Client *)(void *)obj;
   if ((!ec->zone) || (!ec->desk)) return;
   if (sscanf(params, "%d %d", &x, &y) == 2)
     {
        E_Desk *desk;
        int dx, dy;
        int to_x = 0, to_y = 0;

        e_desk_xy_get(ec->desk, &dx, &dy);

        to_x = dx + x;
        to_y = dy + y;
        while (!(desk = e_desk_at_xy_get(ec->zone, to_x, to_y)))
          {
             /* here we are out of our desktop range */
             while (to_x >= ec->zone->desk_x_count)
               {
                  to_x -= ec->zone->desk_x_count;
                  to_y++;
               }
             while (to_x < 0)
               {
                  to_x += ec->zone->desk_x_count;
                  to_y--;
               }

             while (to_y >= ec->zone->desk_y_count)
               to_y -= ec->zone->desk_y_count;
             while (to_y < 0)
               to_y += ec->zone->desk_y_count;
          }

        if (desk)
          {
             /* send the border to the required desktop. */
             e_client_desk_set(ec, desk);
             /* switch desktop. Quite useful from the interface point of view. */
             e_zone_desk_flip_by(ec->zone, to_x - dx, to_y - dy);
             if (!ec->lock_user_stacking)
               evas_object_raise(ec->frame);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(window_zone_move_by, )
{
   E_Client *ec;
   E_Zone *zone;
   E_Maximize max;
   int move;
   char *end;
   E_Fullscreen fs;
   Eina_Bool fullscreen;

   if (!params) return;
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj)
     obj = E_OBJECT(e_client_action_get());
   if (!obj) return;

   ec = (E_Client *)(void *)obj;
   /* bad */
   if (!ec->zone) return;
   /* only one screen */
   if (eina_list_count(e_comp->zones) < 2) return;
   errno = 0;
   move = strtol(params, &end, 10);
   if ((!end) || end[0] || errno) return;
   move += ec->zone->num;
   if (move < 0)
     move = eina_list_count(e_comp->zones) - 1;
   else if ((unsigned int)move >= eina_list_count(e_comp->zones))
     move = 0;
   zone = e_comp_zone_number_get(move);
   if (!zone) return;
   if (ec->zone == zone) return;
   max = ec->maximized;
   fs = ec->fullscreen_policy;
   fullscreen = ec->fullscreen;
   if (ec->maximized) e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
   if (fullscreen) e_client_unfullscreen(ec);
   e_client_zone_set(ec, zone);
   if (max) e_client_maximize(ec, max);
   if (fullscreen) e_client_fullscreen(ec, fs);
   e_client_focus_set_with_pointer(ec);
}

/***************************************************************************/
ACT_FN_GO(window_desk_move_to, )
{
   E_Client *ec;
   int x, y;

   if (!params) return;
   if ((!obj) || (obj->type != E_CLIENT_TYPE))
     obj = E_OBJECT(e_client_focused_get());
   if (!obj)
     obj = E_OBJECT(e_client_action_get());
   if (!obj) return;

   ec = (E_Client *)(void *)obj;
   if ((!ec->zone) || (!ec->desk)) return;
   if (sscanf(params, "%d %d", &x, &y) == 2)
     {
        E_Desk *desk;
        E_Desk *old_desk = ec->desk;
        Eina_Bool was_focused = e_client_stack_focused_get(ec);

        desk = e_desk_at_xy_get(ec->zone, x, y);
        if ((desk) && (desk != old_desk))
          {
             e_client_desk_set(ec, desk);
             if (was_focused)
               e_desk_last_focused_focus(old_desk);
          }
     }
}

/***************************************************************************/
static E_Zone *
_e_actions_zone_get(E_Object *obj)
{
   if (obj)
     {
        if (obj->type == (int)E_COMP_TYPE)
          return e_zone_current_get();
        else if (obj->type == (int)E_ZONE_TYPE)
          return (void *)obj;
        else if (obj->type == (int)E_CLIENT_TYPE)
          return ((E_Client *)(void *)obj)->zone;
        else if (obj->type == (int)E_SHELF_TYPE)
          return ((E_Shelf *)(void *)obj)->zone;
        else if (e_obj_is_win(obj))
          return e_win_client_get((void*)obj)->zone;
     }
   return e_zone_current_get();
}

ACT_FN_GO(desk_flip_by, )
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             int dx = 0, dy = 0;

             if (sscanf(params, "%i %i", &dx, &dy) == 2)
               e_zone_desk_flip_by(zone, dx, dy);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_flip_to, )
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             int dx = 0, dy = 0;

             if (sscanf(params, "%i %i", &dx, &dy) == 2)
               e_zone_desk_flip_to(zone, dx, dy);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_flip_prev, EINA_UNUSED)
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (!zone) return;
   e_zone_desk_flip_to(zone, zone->desk_x_prev, zone->desk_y_prev);
}

/***************************************************************************/
#define ACT_FLIP_LEFT(zone)       ((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1)) || ((zone)->desk_x_current > 0))
#define ACT_FLIP_RIGHT(zone)      ((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1)) || (((zone)->desk_x_current + 1) < (zone)->desk_x_count))
#define ACT_FLIP_UP(zone)         ((e_config->desk_flip_wrap && ((zone)->desk_y_count > 1)) || ((zone)->desk_y_current > 0))
#define ACT_FLIP_DOWN(zone)       ((e_config->desk_flip_wrap && ((zone)->desk_y_count > 1)) || (((zone)->desk_y_current + 1) < (zone)->desk_y_count))
#define ACT_FLIP_UP_LEFT(zone)    ((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1) && ((zone)->desk_y_count > 1)) || (((zone)->desk_x_current > 0) && ((zone)->desk_y_current > 0)))
#define ACT_FLIP_UP_RIGHT(zone)   ((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1) && ((zone)->desk_y_count > 1)) || ((((zone)->desk_x_current + 1) < (zone)->desk_x_count) && ((zone)->desk_y_current > 0)))
#define ACT_FLIP_DOWN_RIGHT(zone) ((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1) && ((zone)->desk_y_count > 1)) || ((((zone)->desk_x_current + 1) < (zone)->desk_x_count) && (((zone)->desk_y_current + 1) < (zone)->desk_y_count)))
#define ACT_FLIP_DOWN_LEFT(zone)  ((e_config->desk_flip_wrap && ((zone)->desk_x_count > 1) && ((zone)->desk_y_count > 1)) || (((zone)->desk_x_current > 0) && (((zone)->desk_y_current + 1) < (zone)->desk_y_count)))

ACT_FN_GO_EDGE(desk_flip_in_direction, )
{
   E_Zone *zone;
   E_Desk *current = NULL;
   E_Event_Pointer_Warp *wev;
   int x, y, offset = 25;

   if (!ev) return;  // with flip on _e_zone_cb_edge_timer we don't have ev!!!
   if (e_client_action_get() && e_client_util_resizing_get(e_client_action_get())) return;
   zone = _e_actions_zone_get(obj);
   if (!zone) return;
   wev = E_NEW(E_Event_Pointer_Warp, 1);
   if (!wev) return;
   ecore_evas_pointer_xy_get(e_comp->ee, &x, &y);
   wev->prev.x = x;
   wev->prev.y = y;
   if (params)
     {
        if (sscanf(params, "%i", &offset) != 1)
          offset = 25;
     }
   switch (ev->edge)
     {
      case E_ZONE_EDGE_LEFT:
        if (ACT_FLIP_LEFT(zone))
          {
             e_zone_desk_flip_by(zone, -1, 0);
             ecore_evas_pointer_warp(e_comp->ee, zone->x + zone->w - offset, y);
             wev->curr.y = y;
             wev->curr.x = zone->w - offset;
          }
        break;

      case E_ZONE_EDGE_RIGHT:
        if (ACT_FLIP_RIGHT(zone))
          {
             e_zone_desk_flip_by(zone, 1, 0);
             ecore_evas_pointer_warp(e_comp->ee, zone->x + offset, y);
             wev->curr.y = y;
             wev->curr.x = offset;
          }
        break;

      case E_ZONE_EDGE_TOP:
        if (ACT_FLIP_UP(zone))
          {
             e_zone_desk_flip_by(zone, 0, -1);
             ecore_evas_pointer_warp(e_comp->ee, x, zone->y + zone->h - offset);
             wev->curr.x = x;
             wev->curr.y = zone->h - offset;
          }
        break;

      case E_ZONE_EDGE_BOTTOM:
        if (ACT_FLIP_DOWN(zone))
          {
             e_zone_desk_flip_by(zone, 0, 1);
             ecore_evas_pointer_warp(e_comp->ee, x, zone->y + offset);
             wev->curr.x = x;
             wev->curr.y = offset;
          }
        break;

      case E_ZONE_EDGE_TOP_LEFT:
        if (ACT_FLIP_UP_LEFT(zone))
          {
             e_zone_desk_flip_by(zone, -1, -1);
             ecore_evas_pointer_warp(e_comp->ee, zone->x + zone->w - offset, zone->y + zone->h - offset);
             wev->curr.x = zone->w - offset;
             wev->curr.y = zone->h - offset;
          }
        break;

      case E_ZONE_EDGE_TOP_RIGHT:
        if (ACT_FLIP_UP_RIGHT(zone))
          {
             e_zone_desk_flip_by(zone, 1, -1);
             ecore_evas_pointer_warp(e_comp->ee, zone->x + offset, zone->y + zone->h - offset);
             wev->curr.x = offset;
             wev->curr.y = zone->h - offset;
          }
        break;

      case E_ZONE_EDGE_BOTTOM_LEFT:
        if (ACT_FLIP_DOWN_LEFT(zone))
          {
             e_zone_desk_flip_by(zone, -1, 1);
             ecore_evas_pointer_warp(e_comp->ee, zone->x + zone->w - offset, zone->y + offset);
             wev->curr.y = offset;
             wev->curr.x = zone->w - offset;
          }
        break;

      case E_ZONE_EDGE_BOTTOM_RIGHT:
        if (ACT_FLIP_DOWN_RIGHT(zone))
          {
             e_zone_desk_flip_by(zone, 1, 1);
             ecore_evas_pointer_warp(e_comp->ee, zone->x + offset, zone->y + offset);
             wev->curr.y = offset;
             wev->curr.x = offset;
          }
        break;

      default:
        break;
     }

   current = e_desk_current_get(zone);
   if (current)
     ecore_event_add(E_EVENT_POINTER_WARP, wev, NULL, NULL);
   else
     free(wev);
}

/***************************************************************************/
ACT_FN_GO(desk_linear_flip_by, )
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             int dx = 0;

             if (sscanf(params, "%i", &dx) == 1)
               e_zone_desk_linear_flip_by(zone, dx);
          }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_linear_flip_to, )
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             int dx = 0;

             if (sscanf(params, "%i", &dx) == 1)
               e_zone_desk_linear_flip_to(zone, dx);
          }
     }
}

#define DESK_ACTION_ALL(zone, act) \
   do { \
     E_Zone *zone; \
     const Eina_List *lz; \
     \
     EINA_LIST_FOREACH(e_comp->zones, lz, zone) { \
        { \
           zone->desk_flip_sync = 1; \
           act; \
        } \
     } \
   } while (0)

/***************************************************************************/
ACT_FN_GO(desk_linear_flip_to_screen, )
{
   if (params)
     {
        int dx = 0, ds = 0;

        if (sscanf(params, "%i %i", &dx, &ds) == 2)
          {
             int current_screen = 0;

             DESK_ACTION_ALL(zone,
                             if (current_screen == ds)
                               {
                                  e_zone_desk_linear_flip_to(zone, dx);
                                  break;
                               }
                             else current_screen++;
                            );
          }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_flip_by_all, )
{
   if (params)
     {
        int dx = 0, dy = 0;

        if (sscanf(params, "%i %i", &dx, &dy) == 2)
          {
             DESK_ACTION_ALL(zone, e_zone_desk_flip_by(zone, dx, dy));
          }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_flip_to_all, )
{
   if (params)
     {
        int dx = 0, dy = 0;

        if (sscanf(params, "%i %i", &dx, &dy) == 2)
          {
             DESK_ACTION_ALL(zone, e_zone_desk_flip_to(zone, dx, dy));
          }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_linear_flip_by_all, )
{
   if (params)
     {
        int dx = 0;

        if (sscanf(params, "%i", &dx) == 1)
          {
             DESK_ACTION_ALL(zone, e_zone_desk_linear_flip_by(zone, dx));
          }
     }
}

/***************************************************************************/
ACT_FN_GO(desk_linear_flip_to_all, )
{
   if (params)
     {
        int dx = 0;

        if (sscanf(params, "%i", &dx) == 1)
          {
             DESK_ACTION_ALL(zone, e_zone_desk_linear_flip_to(zone, dx));
          }
     }
}

/***************************************************************************/
ACT_FN_GO(screen_send_to, )
{
   E_Zone *zone, *zone2;
   int scr;

   zone = _e_actions_zone_get(obj);
   if (!zone) zone = e_zone_current_get();
   if (!zone) return;
   if (!params) return;

   errno = 0;
   scr = strtol(params, NULL, 10);
   if (errno) return;

   if (scr != -1)
     scr = scr % eina_list_count(e_comp->zones);
   if (scr < 0) scr += eina_list_count(e_comp->zones);
   zone2 = e_comp_zone_number_get(scr);
   if ((zone2) && (zone != zone2))
     {
        ecore_evas_pointer_warp(e_comp->ee,
                                zone2->x + (zone2->w / 2),
                                zone2->y + (zone2->h / 2));
        e_desk_last_focused_focus(e_desk_current_get(zone2));
     }
}

ACT_FN_GO(screen_send_by, )
{
   E_Zone *zone, *zone2;
   int scr = 0;

   zone = _e_actions_zone_get(obj);
   if (!zone) zone = e_zone_current_get();
   if (!zone) return;
   if (!params) return;

   errno = 0;
   scr = strtol(params, NULL, 10);
   if (errno) return;
   scr += zone->num;
   if (scr != -1)
     scr = scr % eina_list_count(e_comp->zones);
   if (scr < 0) scr += eina_list_count(e_comp->zones);
   zone2 = e_comp_zone_number_get(scr);
   if ((zone2) && (zone != zone2))
     {
        ecore_evas_pointer_warp(e_comp->ee,
                             zone2->x + (zone2->w / 2),
                             zone2->y + (zone2->h / 2));
        e_desk_last_focused_focus(e_desk_current_get(zone2));
     }
}

#define ZONE_DESK_ACTION(con_num, zone_num, zone, act)                    \
  E_Zone * zone;                                                          \
  if ((con_num < 0) || (zone_num < 0)) {                                  \
       Eina_List *l;                                           \
       if ((con_num >= 0) && (zone_num < 0)) /* con=1 zone=all */ {       \
            EINA_LIST_FOREACH(e_comp->zones, l, zone) {                     \
                 act;                                                     \
              } }                                                         \
       else if ((con_num < 0) && (zone_num >= 0)) /* c=all zone=1 */ {  \
              zone = e_comp_zone_number_get(zone_num);  \
              if (zone)                                           \
                act;                                              \
                    }                                                  \
       else if ((con_num < 0) && (zone_num < 0)) /* c=all zone=all */ { \
              EINA_LIST_FOREACH(e_comp->zones, lll, zone) {         \
                   act;                                           \
                } } }                                         \
  else {                                                                  \
       zone = e_comp_zone_number_get(zone_num);        \
       if (zone) act;                                                     \
    }

/***************************************************************************/
#if 0
ACT_FN_GO(zone_desk_flip_by, )
{
   if (params)
     {
        int con_num = 0, zone_num = 0;
        int dx = 0, dy = 0;

        if (sscanf(params, "%i %i %i %i", &con_num, &zone_num, &dx, &dy) == 4)
          ZONE_DESK_ACTION(con_num, zone_num, zone,
                           e_zone_desk_flip_by(zone, dx, dy));
     }
}
#endif

/***************************************************************************/
#if 0
ACT_FN_GO(zone_desk_flip_to, )
{
   if (params)
     {
        int con_num = 0, zone_num = 0;
        int dx = 0, dy = 0;

        if (sscanf(params, "%i %i %i %i", &con_num, &zone_num, &dx, &dy) == 4)
          ZONE_DESK_ACTION(con_num, zone_num, zone,
                           e_zone_desk_flip_to(zone, dx, dy));
     }
}
#endif

/***************************************************************************/
#if 0
ACT_FN_GO(zone_desk_linear_flip_by, )
{
   if (params)
     {
        int con_num = 0, zone_num = 0;
        int dx = 0;

        if (sscanf(params, "%i %i %i", &con_num, &zone_num, &dx) == 3)
          ZONE_DESK_ACTION(con_num, zone_num, zone,
                           e_zone_desk_linear_flip_by(zone, dx));
     }
}
#endif

/***************************************************************************/
#if 0
ACT_FN_GO(zone_desk_linear_flip_to, )
{
   if (params)
     {
        int con_num = 0, zone_num = 0;
        int dx = 0;

        if (sscanf(params, "%i %i %i", &con_num, &zone_num, &dx) == 3)
          ZONE_DESK_ACTION(con_num, zone_num, zone,
                           e_zone_desk_linear_flip_to(zone, dx));
     }
}
#endif

/***************************************************************************/
static void
_e_actions_cb_menu_end(void *data EINA_UNUSED, E_Menu *m)
{
   e_object_del(E_OBJECT(m));
}

static E_Menu *
_e_actions_menu_find(const char *name)
{
   if (!strcmp(name, "main"))
     return e_int_menus_main_new();
   else if (!strcmp(name, "favorites"))
     return e_int_menus_favorite_apps_new();
   else if (!strcmp(name, "all"))
     return e_int_menus_all_apps_new();
   else if (!strcmp(name, "clients"))
     return e_int_menus_clients_new();
   else if (!strcmp(name, "lost_clients"))
     return e_int_menus_lost_clients_new();
   else if (!strcmp(name, "configuration"))
     return e_int_menus_config_new();
   return NULL;
}

ACT_FN_GO(menu_show, )
{
   E_Zone *zone;

   /* menu is active - abort */
   if (e_comp_util_kbd_grabbed() || e_comp_util_mouse_grabbed()) return;
   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             E_Menu *m = NULL;

             m = _e_actions_menu_find(params);
             if (m)
               {
                  int x, y;

                  /* FIXME: this is a bit of a hack... setting m->c - bad hack */
                  m->zone = zone;
                  ecore_evas_pointer_xy_get(e_comp->ee, &x, &y);
                  e_menu_post_deactivate_callback_set(m, _e_actions_cb_menu_end, NULL);
                  e_menu_activate_mouse(m, zone, x, y, 1, 1,
                                        E_MENU_POP_DIRECTION_AUTO, 0);
               }
          }
     }
}

ACT_FN_GO_MOUSE(menu_show, )
{
   E_Zone *zone;
   E_Menu *m = NULL;
   int x, y;

   /* menu is active - abort */
   if (e_comp_util_kbd_grabbed() || e_comp_util_mouse_grabbed()) return EINA_TRUE;
   zone = _e_actions_zone_get(obj);
   if (!zone) return EINA_TRUE;
   if (!params) return EINA_TRUE;
   m = _e_actions_menu_find(params);
   if (!m) return EINA_TRUE;
   /* FIXME: this is a bit of a hack... setting m->zone - bad hack */
   m->zone = zone;
   x = ev->canvas.x;
   y = ev->canvas.y;
   e_menu_post_deactivate_callback_set(m, _e_actions_cb_menu_end, NULL);
   e_menu_activate_mouse(m, zone, x, y, 1, 1,
                         E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
   return EINA_TRUE;
}

ACT_FN_GO_KEY(menu_show, , EINA_UNUSED)
{
   E_Zone *zone;

   /* menu is active - abort */
   if (e_comp_util_kbd_grabbed() || e_comp_util_mouse_grabbed()) return;
   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             E_Menu *m = NULL;

             m = _e_actions_menu_find(params);
             if (m)
               {
                  int x, y;

                  /* FIXME: this is a bit of a hack... setting m->con - bad hack */
                  m->zone = zone;
                  ecore_evas_pointer_xy_get(e_comp->ee, &x, &y);
                  e_menu_post_deactivate_callback_set(m, _e_actions_cb_menu_end, NULL);
                  e_menu_activate_key(m, zone, x, y, 1, 1,
                                      E_MENU_POP_DIRECTION_DOWN);
               }
          }
     }
}

ACT_FN_GO(menu_show_object,)
{
   E_Zone *zone;

   /* menu is active - abort */
   if (e_comp_util_kbd_grabbed() || e_comp_util_mouse_grabbed()) return;
   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             E_Menu *m = NULL;

             m = _e_actions_menu_find(params);
             if (m)
               {
                  int x, y, w, h;
                  Eina_List *l, *ll;
                  Evas_Object *o;

                  ecore_evas_pointer_xy_get(e_comp->ee, &x, &y);
                  l = evas_objects_at_xy_get(e_comp->evas, x, y, 0, 0);
                  EINA_LIST_REVERSE_FOREACH(l, ll, o)
                    {
                       if (evas_object_repeat_events_get(o) && ll->prev) continue;
                       evas_object_geometry_get(o, &x, &y, &w, &h);
                       m->zone = e_comp_object_util_zone_get(o);
                       e_menu_post_deactivate_callback_set(m, _e_actions_cb_menu_end, NULL);
                       e_menu_activate_mouse(m, zone, x, y, w, h,
                                             E_MENU_POP_DIRECTION_AUTO, 0);
                       break;
                    }
                  eina_list_free(l);
               }
          }
     }
}

/***************************************************************************/
ACT_FN_GO(exec, )
{
   E_Zone *zone;
   static double lock;
   double test;

   /* prevent exec actions from occurring too frequently */
   test = ecore_loop_time_get();
   if (test - lock < 0.05)
     {
        lock = test;
        return;
     }
   lock = ecore_loop_time_get();
   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          e_exec(zone, NULL, params, NULL, "action/exec");
     }
}

/***************************************************************************/
ACT_FN_GO(app, )
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (zone)
     {
        if (params)
          {
             Efreet_Desktop *desktop = NULL;
             char *p, *p2;
             size_t plen;

             plen = strlen(params);
             p2 = memcpy(alloca(plen + 1), params, plen + 1);
             p = strchr(p2, ':');
             if (p)
               {
                  *p++ = 0;
                  if (*p == ' ')
                    {
                       E_Dialog *dia;
                       char dialog_text[1024];

                       dia = e_dialog_new(NULL, "E", "_e_action_act_app_go_syntax_error");
                       if (!dia) return;

                       snprintf(dialog_text, sizeof(dialog_text),
                                "%s<ps/><ps/>"
                                "Check syntax. You should not put a whitespace right after colon in action params.<ps/>"
                                "syntax: [file:file.desktop|name:App Name|generic:Generic Name|exe:exename]<ps/><ps/>"
                                "exe:terminology (O)<ps/>"
                                "exe: terminology (X)", params);

                       e_dialog_title_set(dia, _("Action Params Syntax Error"));
                       e_dialog_text_set(dia, _(dialog_text));
                       e_dialog_icon_set(dia, "dialog-error", 64);
                       e_dialog_button_add(dia, _("Close"), NULL, NULL, NULL);
                       e_dialog_button_focus_num(dia, 0);
                       elm_win_center(dia->win, 1, 1);
                       e_dialog_show(dia);

                       return;
                    }
                  if (!strcmp(p2, "file"))
                    desktop = efreet_util_desktop_file_id_find(p);
                  else if (!strcmp(p2, "name"))
                    desktop = efreet_util_desktop_name_find(p);
                  else if (!strcmp(p2, "generic"))
                    desktop = efreet_util_desktop_generic_name_find(p);
                  else if (!strcmp(p2, "exe"))
                    desktop = efreet_util_desktop_exec_find(p);
                  if (desktop)
                    {
                       e_exec(zone, desktop, NULL, NULL, "action/app");
                       efreet_desktop_free(desktop);
                    }
               }
          }
     }
}

/***************************************************************************/
ACT_FN_GO(app_new_instance, EINA_UNUSED)
{
   E_Client *ec;
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (!zone)
     zone = e_zone_current_get();

   if (!obj) obj = E_OBJECT(e_client_focused_get());
   if ((!obj) || (!zone)) return;
   if (obj->type != E_CLIENT_TYPE)
     {
        obj = E_OBJECT(e_client_focused_get());
        if (!obj) return;
     }
   ec = (E_Client *)(void *)obj;

   if (ec->desktop)
     e_exec(zone, ec->desktop, NULL, NULL, "action/app");
}

/***************************************************************************/
ACT_FN_GO(desk_deskshow_toggle, EINA_UNUSED)
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (!zone) zone = e_zone_current_get();
   if (zone) e_desk_deskshow(zone);
}

ACT_FN_GO(cleanup_windows, EINA_UNUSED)
{
   E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (!zone) zone = e_zone_current_get();
   if (zone) e_place_zone_region_smart_cleanup(zone);
}

/***************************************************************************/
static E_Dialog *exit_dialog = NULL;

static void
_e_actions_cb_exit_dialog_ok(void *data EINA_UNUSED, E_Dialog *dia)
{
   if (dia)
     {
        e_object_del(E_OBJECT(exit_dialog));
        exit_dialog = NULL;
     }
   e_sys_action_do(E_SYS_EXIT, NULL);
}

static void
_e_actions_cb_exit_dialog_cancel(void *data EINA_UNUSED, E_Dialog *dia EINA_UNUSED)
{
   e_object_del(E_OBJECT(exit_dialog));
   exit_dialog = NULL;
}

static void
_e_actions_cb_exit_dialog_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _e_actions_cb_exit_dialog_cancel(NULL, data);
}

ACT_FN_GO(exit, )
{
   if ((params) && (!strcmp(params, "now")))
     {
        e_sys_action_do(E_SYS_EXIT, NULL);
        return;
     }
   if (exit_dialog) e_object_del(E_OBJECT(exit_dialog));

   if (e_config->cnfmdlg_disabled)
     {
        _e_actions_cb_exit_dialog_ok(NULL, NULL);
        return;
     }

   exit_dialog = e_dialog_new(NULL, "E", "_exit_dialog");
   if (!exit_dialog) return;
   evas_object_event_callback_add(exit_dialog->win, EVAS_CALLBACK_DEL, _e_actions_cb_exit_dialog_delete, exit_dialog);
   e_dialog_title_set(exit_dialog, _("Exit"));
   e_dialog_text_set(exit_dialog, _("Are you sure you want to exit Enlightenment?"));
   e_dialog_icon_set(exit_dialog, "application-exit", 64);
   e_dialog_button_add(exit_dialog, _("Yes"), NULL,
                       _e_actions_cb_exit_dialog_ok, NULL);
   e_dialog_button_add(exit_dialog, _("No"), NULL,
                       _e_actions_cb_exit_dialog_cancel, NULL);
   e_dialog_button_focus_num(exit_dialog, 1);
   elm_win_center(exit_dialog->win, 1, 1);
   e_dialog_show(exit_dialog);
}

/***************************************************************************/
ACT_FN_GO(restart, EINA_UNUSED)
{
   e_sys_action_do(E_SYS_RESTART, NULL);
}

/***************************************************************************/
ACT_FN_GO(exit_now, EINA_UNUSED)
{
   e_sys_action_do(E_SYS_EXIT_NOW, NULL);
}

/***************************************************************************/
ACT_FN_GO(halt_now, EINA_UNUSED)
{
   e_sys_action_do(E_SYS_HALT_NOW, NULL);
}

/***************************************************************************/
static E_Dialog *logout_dialog = NULL;

static void
_e_actions_cb_logout_dialog_ok(void *data EINA_UNUSED, E_Dialog *dia)
{
   if (dia)
     {
        e_object_del(E_OBJECT(logout_dialog));
        logout_dialog = NULL;
     }
   e_sys_action_do(E_SYS_LOGOUT, NULL);
}

static void
_e_actions_cb_logout_dialog_cancel(void *data EINA_UNUSED, E_Dialog *dia EINA_UNUSED)
{
   e_object_del(E_OBJECT(logout_dialog));
   logout_dialog = NULL;
}

static void
_e_actions_cb_logout_dialog_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _e_actions_cb_logout_dialog_cancel(NULL, data);
}

ACT_FN_GO(logout, )
{
   if ((params) && (!strcmp(params, "now")))
     {
        e_sys_action_do(E_SYS_LOGOUT, NULL);
        return;
     }
   if (logout_dialog) e_object_del(E_OBJECT(logout_dialog));

   if (e_config->cnfmdlg_disabled)
     {
        _e_actions_cb_logout_dialog_ok(NULL, NULL);
        return;
     }

   logout_dialog = e_dialog_new(NULL, "E", "_logout_dialog");
   if (!logout_dialog) return;
   evas_object_event_callback_add(logout_dialog->win, EVAS_CALLBACK_DEL, _e_actions_cb_logout_dialog_delete, logout_dialog);
   e_dialog_title_set(logout_dialog, _("Logout"));
   e_dialog_text_set(logout_dialog, _("Are you sure you want to logout?"));
   e_dialog_icon_set(logout_dialog, "system-log-out", 64);
   e_dialog_button_add(logout_dialog, _("Yes"), NULL,
                       _e_actions_cb_logout_dialog_ok, NULL);
   e_dialog_button_add(logout_dialog, _("No"), NULL,
                       _e_actions_cb_logout_dialog_cancel, NULL);
   e_dialog_button_focus_num(logout_dialog, 1);
   elm_win_center(logout_dialog->win, 1, 1);
   e_dialog_show(logout_dialog);
}

/***************************************************************************/
static E_Dialog *halt_dialog = NULL;

static void
_e_actions_cb_halt_dialog_ok(void *data EINA_UNUSED, E_Dialog *dia)
{
   if (dia)
     {
        e_object_del(E_OBJECT(halt_dialog));
        halt_dialog = NULL;
     }
   e_sys_action_do(E_SYS_HALT, NULL);
}

static void
_e_actions_cb_halt_dialog_cancel(void *data EINA_UNUSED, E_Dialog *dia EINA_UNUSED)
{
   e_object_del(E_OBJECT(halt_dialog));
   halt_dialog = NULL;
}

static void
_e_actions_cb_halt_dialog_delete(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (!halt_dialog) return;
   e_object_del(E_OBJECT(halt_dialog));
   halt_dialog = NULL;
}

ACT_FN_GO(halt, )
{
   if ((params) && (!strcmp(params, "now")))
     {
        e_sys_action_do(E_SYS_HALT, NULL);
        return;
     }
   if (halt_dialog) e_object_del(E_OBJECT(halt_dialog));

   if (e_config->cnfmdlg_disabled)
     {
        _e_actions_cb_halt_dialog_ok(NULL, NULL);
        return;
     }

   halt_dialog = e_dialog_new(NULL, "E", "_halt_dialog");
   if (!halt_dialog) return;
   evas_object_event_callback_add(halt_dialog->win, EVAS_CALLBACK_DEL, _e_actions_cb_halt_dialog_delete, halt_dialog);
   e_dialog_title_set(halt_dialog, _("Power off"));
   e_dialog_text_set(halt_dialog,
                     _("Are you sure you want to power off your computer?"));
   e_dialog_icon_set(halt_dialog, "system-shutdown", 64);
   e_dialog_button_add(halt_dialog, _("Yes"), NULL,
                       _e_actions_cb_halt_dialog_ok, NULL);
   e_dialog_button_add(halt_dialog, _("No"), NULL,
                       _e_actions_cb_halt_dialog_cancel, NULL);
   e_dialog_button_focus_num(halt_dialog, 1);
   elm_win_center(halt_dialog->win, 1, 1);
   e_dialog_show(halt_dialog);
}

/***************************************************************************/
static E_Dialog *reboot_dialog = NULL;

static void
_e_actions_cb_reboot_dialog_ok(void *data EINA_UNUSED, E_Dialog *dia)
{
   if (dia)
     {
        e_object_del(E_OBJECT(reboot_dialog));
        reboot_dialog = NULL;
     }
   e_sys_action_do(E_SYS_REBOOT, NULL);
}

static void
_e_actions_cb_reboot_dialog_cancel(void *data EINA_UNUSED, E_Dialog *dia EINA_UNUSED)
{
   e_object_del(E_OBJECT(reboot_dialog));
   reboot_dialog = NULL;
}

static void
_e_actions_cb_reboot_dialog_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _e_actions_cb_reboot_dialog_cancel(NULL, data);
}

ACT_FN_GO(reboot, )
{
   if ((params) && (!strcmp(params, "now")))
     {
        e_sys_action_do(E_SYS_REBOOT, NULL);
        return;
     }
   if (reboot_dialog) e_object_del(E_OBJECT(reboot_dialog));

   if (e_config->cnfmdlg_disabled)
     {
        _e_actions_cb_reboot_dialog_ok(NULL, NULL);
        return;
     }

   reboot_dialog = e_dialog_new(NULL, "E", "_reboot_dialog");
   if (!reboot_dialog) return;
   evas_object_event_callback_add(reboot_dialog->win, EVAS_CALLBACK_DEL, _e_actions_cb_reboot_dialog_delete, reboot_dialog);
   e_dialog_title_set(reboot_dialog, _("Reboot"));
   e_dialog_text_set(reboot_dialog, _("Are you sure you want to reboot your computer?"));
   e_dialog_icon_set(reboot_dialog, "system-restart", 64);
   e_dialog_button_add(reboot_dialog, _("Yes"), NULL,
                       _e_actions_cb_reboot_dialog_ok, NULL);
   e_dialog_button_add(reboot_dialog, _("No"), NULL,
                       _e_actions_cb_reboot_dialog_cancel, NULL);
   e_dialog_button_focus_num(reboot_dialog, 1);
   elm_win_center(reboot_dialog->win, 1, 1);
   e_dialog_show(reboot_dialog);
}

/***************************************************************************/
static E_Dialog *suspend_dialog = NULL;

static void
_e_actions_cb_suspend_dialog_ok(void *data EINA_UNUSED, E_Dialog *dia)
{
   if (dia)
     {
        e_object_del(E_OBJECT(suspend_dialog));
        suspend_dialog = NULL;
     }
   e_sys_action_do(E_SYS_SUSPEND, NULL);
}

static void
_e_actions_cb_suspend_dialog_cancel(void *data EINA_UNUSED, E_Dialog *dia EINA_UNUSED)
{
   e_object_del(E_OBJECT(suspend_dialog));
   suspend_dialog = NULL;
}

static void
_e_actions_cb_suspend_dialog_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _e_actions_cb_suspend_dialog_cancel(NULL, data);
}

ACT_FN_GO(suspend_now, EINA_UNUSED)
{
   e_sys_action_do(E_SYS_SUSPEND, NULL);
}

ACT_FN_GO(suspend, )
{
   if ((params) && (!strcmp(params, "now")))
     {
        e_sys_action_do(E_SYS_SUSPEND, NULL);
        return;
     }
   if (suspend_dialog) e_object_del(E_OBJECT(suspend_dialog));

   if (e_config->cnfmdlg_disabled)
     {
        _e_actions_cb_suspend_dialog_ok(NULL, NULL);
        return;
     }

   suspend_dialog = e_dialog_new(NULL, "E", "_suspend_dialog");
   if (!suspend_dialog) return;
   evas_object_event_callback_add(suspend_dialog->win, EVAS_CALLBACK_DEL, _e_actions_cb_suspend_dialog_delete, suspend_dialog);
   e_dialog_title_set(suspend_dialog, _("Suspend"));
   e_dialog_text_set(suspend_dialog, _("Are you sure you want to suspend your computer?"));
   e_dialog_icon_set(suspend_dialog, "system-suspend", 64);
   e_dialog_button_add(suspend_dialog, _("Yes"), NULL,
                       _e_actions_cb_suspend_dialog_ok, NULL);
   e_dialog_button_add(suspend_dialog, _("No"), NULL,
                       _e_actions_cb_suspend_dialog_cancel, NULL);
   e_dialog_button_focus_num(suspend_dialog, 1);
   elm_win_center(suspend_dialog->win, 1, 1);
   e_dialog_show(suspend_dialog);
}

static Eina_Bool
_have_lid_and_external_screens_on(void)
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
   if ((lids > 0) && (ext_screens > 0)) return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_should_suspend_if_plugged_in(void)
{
   if ((e_config->screensaver_suspend_on_ac) ||
       (e_powersave_mode_get() > E_POWERSAVE_MODE_LOW))
     return EINA_TRUE;
   return EINA_FALSE;
}

ACT_FN_GO(suspend_smart, EINA_UNUSED)
{
   if (!_have_lid_and_external_screens_on())
     {
        if (_should_suspend_if_plugged_in())
          e_sys_action_do(E_SYS_SUSPEND, NULL);
        else
          e_powersave_defer_suspend();
     }
}

/***************************************************************************/
static E_Dialog *hibernate_dialog = NULL;

static void
_e_actions_cb_hibernate_dialog_ok(void *data EINA_UNUSED, E_Dialog *dia)
{
   if (dia)
     {
        e_object_del(E_OBJECT(hibernate_dialog));
        hibernate_dialog = NULL;
     }
   e_sys_action_do(E_SYS_HIBERNATE, NULL);
}

static void
_e_actions_cb_hibernate_dialog_cancel(void *data EINA_UNUSED, E_Dialog *dia EINA_UNUSED)
{
   e_object_del(E_OBJECT(hibernate_dialog));
   hibernate_dialog = NULL;
}

static void
_e_actions_cb_hibernate_dialog_delete(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _e_actions_cb_hibernate_dialog_cancel(NULL, data);
}

ACT_FN_GO(hibernate_now, EINA_UNUSED)
{
   e_sys_action_do(E_SYS_HIBERNATE, NULL);
}

ACT_FN_GO(hibernate, )
{
   if ((params) && (!strcmp(params, "now")))
     {
        e_sys_action_do(E_SYS_HIBERNATE, NULL);
        return;
     }
   if (hibernate_dialog) e_object_del(E_OBJECT(hibernate_dialog));

   if (e_config->cnfmdlg_disabled)
     {
        _e_actions_cb_hibernate_dialog_ok(NULL, NULL);
        return;
     }

   hibernate_dialog = e_dialog_new(NULL, "E", "_hibernate_dialog");
   if (!hibernate_dialog) return;
   evas_object_event_callback_add(hibernate_dialog->win, EVAS_CALLBACK_DEL, _e_actions_cb_hibernate_dialog_delete, hibernate_dialog);
   e_dialog_title_set(hibernate_dialog, _("Hibernate"));
   e_dialog_text_set(hibernate_dialog, _("Are you sure you want to hibernate your computer?"));
   e_dialog_icon_set(hibernate_dialog, "system-suspend-hibernate", 64);
   e_dialog_button_add(hibernate_dialog, _("Yes"), NULL,
                       _e_actions_cb_hibernate_dialog_ok, NULL);
   e_dialog_button_add(hibernate_dialog, _("No"), NULL,
                       _e_actions_cb_hibernate_dialog_cancel, NULL);
   e_dialog_button_focus_num(hibernate_dialog, 1);
   elm_win_center(hibernate_dialog->win, 1, 1);
   e_dialog_show(hibernate_dialog);
}

ACT_FN_GO(hibernate_smart, EINA_UNUSED)
{
   if (!_have_lid_and_external_screens_on())
     {
        if (_should_suspend_if_plugged_in())
          e_sys_action_do(E_SYS_HIBERNATE, NULL);
        else
          e_powersave_defer_hibernate();
     }
}

/***************************************************************************/
ACT_FN_GO(pointer_resize_push, )
{
   if (!obj) return;
   if (obj->type == E_CLIENT_TYPE)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if ((ec->lock_user_size) || (ec->shaded) || (ec->shading) ||
            (ec->fullscreen) || ((ec->maximized) && (!e_config->allow_manip)))
          return;
        e_pointer_type_push(e_comp->pointer, ec, params);
     }
}

/***************************************************************************/
ACT_FN_GO(pointer_resize_pop, )
{
   if (!obj) return;
   if (obj->type == E_CLIENT_TYPE)
     {
        E_Client *ec;

        ec = (E_Client *)(void *)obj;
        if ((ec->lock_user_size) || (ec->shaded) || (ec->shading) ||
            (ec->fullscreen) || ((ec->maximized) && (!e_config->allow_manip)))
          return;
        e_pointer_type_pop(e_comp->pointer, ec, params);
     }
}

/***************************************************************************/
ACT_FN_GO(desk_lock, EINA_UNUSED)
{
/*  E_Zone *zone;

   zone = _e_actions_zone_get(obj);
   if (zone)*/
   e_desklock_show_manual(EINA_FALSE);
}

/***************************************************************************/
ACT_FN_GO(shelf_show, )
{
   Eina_List *l;
   E_Shelf *es;

   if (params)
     {
        for (; *params != '\0'; params++)
          if (!isspace(*params))
            break;
        if (*params == '\0')
          params = NULL;
     }

   EINA_LIST_FOREACH(e_shelf_list(), l, es)
     {
        if ((!params) || (params && (eina_fnmatch(params, es->name, 0))))
          {
             e_shelf_toggle(es, 1);
             e_shelf_toggle(es, 0);
          }
     }
}
/***************************************************************************/
#define ACT_SHELF_SHOW(params, es)                                  \
  if ((!params) || (params && (eina_fnmatch(params, es->name, 0)))) \
    {                                                               \
       e_shelf_toggle(es, 1);                                       \
       e_shelf_toggle(es, 0);                                       \
    }

ACT_FN_GO_EDGE(shelf_show, )
{
   Eina_List *l;
   E_Shelf *es;

   if (params)
     {
        for (; *params != '\0'; params++)
          {
             if (!isspace(*params))
               break;
          }
        if (*params == '\0')
          params = NULL;
     }

   EINA_LIST_FOREACH(e_shelf_list(), l, es)
     {
        switch (ev->edge)
          {
           case E_ZONE_EDGE_LEFT:
             if ((es->gadcon->orient == E_GADCON_ORIENT_LEFT ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_LT ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_LB) &&
                 (ev->y >= es->y) && (ev->y <= (es->y + es->h)))
               ACT_SHELF_SHOW(params, es);
             break;

           case E_ZONE_EDGE_RIGHT:
             if ((es->gadcon->orient == E_GADCON_ORIENT_RIGHT ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_RT ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_RB) &&
                 (ev->y >= es->y) && (ev->y <= (es->y + es->h)))
               ACT_SHELF_SHOW(params, es);
             break;

           case E_ZONE_EDGE_TOP:
             if ((es->gadcon->orient == E_GADCON_ORIENT_TOP ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_TL ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_TR) &&
                 (ev->x >= es->x) && (ev->x <= (es->x + es->w)))
               ACT_SHELF_SHOW(params, es);
             break;

           case E_ZONE_EDGE_BOTTOM:
             if ((es->gadcon->orient == E_GADCON_ORIENT_BOTTOM ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_BL ||
                  es->gadcon->orient == E_GADCON_ORIENT_CORNER_BR) &&
                 (ev->x >= es->x) && (ev->x <= (es->x + es->w)))
               ACT_SHELF_SHOW(params, es);
             break;

           default:
             break;
          }
     }
}
#undef ACT_SHELF_SHOW

/***************************************************************************/
typedef struct _Delayed_Action Delayed_Action;

struct _Delayed_Action
{
   int          mouse, button;
   const char  *key;
   E_Object    *obj;
   Ecore_Timer *timer;
   struct
   {
      const char *action, *params;
   } def, delayed;
};

static Eina_List *_delayed_actions = NULL;

static void
_delayed_action_free(Delayed_Action *da)
{
   if (da->obj) e_object_unref(da->obj);
   if (da->key) eina_stringshare_del(da->key);
   if (da->timer) ecore_timer_del(da->timer);
   if (da->def.action) eina_stringshare_del(da->def.action);
   if (da->def.params) eina_stringshare_del(da->def.params);
   if (da->delayed.action) eina_stringshare_del(da->delayed.action);
   if (da->delayed.params) eina_stringshare_del(da->delayed.params);
   free(da);
}

static Eina_Bool
_delayed_action_cb_timer(void *data)
{
   Delayed_Action *da;
   E_Action *act;

   da = data;
   da->timer = NULL;
   if (da->delayed.action)
     {
        act = e_action_find(da->delayed.action);
        if (act)
          {
             if (act->func.go) act->func.go(da->obj, da->delayed.params);
          }
     }
   _delayed_actions = eina_list_remove(_delayed_actions, da);
   _delayed_action_free(da);
   return ECORE_CALLBACK_CANCEL;
}

static void
_delayed_action_do(Delayed_Action *da)
{
   E_Action *act;

   if (da->def.action)
     {
        act = e_action_find(da->def.action);
        if (act)
          {
             if (act->func.go) act->func.go(da->obj, da->def.params);
          }
     }
}

static void
_delayed_action_list_parse_action(const char *str, double *delay, const char **action, const char **params)
{
   char fbuf[16];
   char buf[1024];
   const char *p;

   buf[0] = 0;
   if (sscanf(str, "%10s %1000s", fbuf, buf) != 2) return;
   *action = eina_stringshare_add(buf);
   *delay = atof(fbuf);
   p = strchr(str, ' ');
   if (p)
     {
        p++;
        p = strchr(p, ' ');
        if (p)
          {
             p++;
             *params = eina_stringshare_add(p);
          }
     }
}

static void
_delayed_action_list_parse(Delayed_Action *da, const char *params)
{
   double delay = 2.0;
   const char *p, *a1start = NULL, *a1stop = NULL;
   const char *a2start = NULL, *a2stop = NULL;

   // FORMAT: "[0.0 default_action param1 param2] [x.x action2 param1 param2]"
   p = params;
   while (*p)
     {
        if ((*p == '[') && ((p == params) || ((p > params) && (p[-1] != '\\')))) {a1start = p + 1; break; }
        p++;
     }
   while (*p)
     {
        if ((*p == ']') && ((p == params) || ((p > params) && (p[-1] != '\\')))) {a1stop = p; break; }
        p++;
     }
   while (*p)
     {
        if ((*p == '[') && ((p == params) || ((p > params) && (p[-1] != '\\')))) {a2start = p + 1; break; }
        p++;
     }
   while (*p)
     {
        if ((*p == ']') && ((p == params) || ((p > params) && (p[-1] != '\\')))) {a2stop = p; break; }
        p++;
     }
   if ((a1start) && (a2start) && (a1stop) && (a2stop))
     {
        char *a1, *a2;

        a1 = alloca(a1stop - a1start + 1);
        eina_strlcpy(a1, a1start, a1stop - a1start + 1);
        _delayed_action_list_parse_action(a1, &delay, &da->def.action, &da->def.params);

        a2 = alloca(a1stop - a1start + 1);
        eina_strlcpy(a2, a2start, a2stop - a2start + 1);
        _delayed_action_list_parse_action(a2, &delay, &da->delayed.action, &da->delayed.params);
     }
   da->timer = ecore_timer_loop_add(delay, _delayed_action_cb_timer, da);
}

static void
_delayed_action_key_add(E_Object *obj, const char *params, Ecore_Event_Key *ev)
{
   Delayed_Action *da;

   da = E_NEW(Delayed_Action, 1);
   if (!da) return;
   if (obj)
     {
        da->obj = obj;
        e_object_ref(da->obj);
     }
   da->mouse = 0;
   da->key = eina_stringshare_add(ev->key);
   if (params) _delayed_action_list_parse(da, params);
   _delayed_actions = eina_list_append(_delayed_actions, da);
}

static void
_delayed_action_key_del(E_Object *obj, const char *params EINA_UNUSED, Ecore_Event_Key *ev)
{
   Eina_List *l;
   Delayed_Action *da;

   EINA_LIST_FOREACH(_delayed_actions, l, da)
     {
        if ((da->obj == obj) && (!da->mouse) &&
            (!strcmp(da->key, ev->key)))
          {
             _delayed_action_do(da);
             _delayed_action_free(da);
             _delayed_actions = eina_list_remove_list(_delayed_actions, l);
             return;
          }
     }
}

static void
_delayed_action_mouse_add(E_Object *obj, const char *params, E_Binding_Event_Mouse_Button *ev)
{
   Delayed_Action *da;

   da = E_NEW(Delayed_Action, 1);
   if (!da) return;
   if (obj)
     {
        da->obj = obj;
        e_object_ref(da->obj);
     }
   da->mouse = 1;
   da->button = ev->button;
   if (params) _delayed_action_list_parse(da, params);
   _delayed_actions = eina_list_append(_delayed_actions, da);
}

static void
_delayed_action_mouse_del(E_Object *obj, const char *params EINA_UNUSED, E_Binding_Event_Mouse_Button *ev)
{
   Eina_List *l;
   Delayed_Action *da;

   EINA_LIST_FOREACH(_delayed_actions, l, da)
     {
        if ((da->obj == obj) && (da->mouse) &&
            ((int)ev->button == da->button))
          {
             _delayed_action_do(da);
             _delayed_action_free(da);
             _delayed_actions = eina_list_remove_list(_delayed_actions, l);
             return;
          }
     }
}

// obj , params  , ev
ACT_FN_GO_KEY(delayed_action, , )
{
   _delayed_action_key_add(obj, params, ev);
}

ACT_FN_GO_MOUSE(delayed_action, )
{
   _delayed_action_mouse_add(obj, params, ev);
   return EINA_TRUE;
}

ACT_FN_END_KEY(delayed_action, )
{
   _delayed_action_key_del(obj, params, ev);
}

ACT_FN_END_MOUSE(delayed_action, )
{
   _delayed_action_mouse_del(obj, params, ev);
   return EINA_TRUE;
}

ACT_FN_GO(dim_screen, EINA_UNUSED)
{
   E_Zone *zone = _e_actions_zone_get(obj);
   e_backlight_mode_set(zone, E_BACKLIGHT_MODE_DIM);
}

ACT_FN_GO(undim_screen, EINA_UNUSED)
{
   E_Zone *zone = _e_actions_zone_get(obj);
   e_backlight_mode_set(zone, E_BACKLIGHT_MODE_NORMAL);
}

ACT_FN_GO(backlight_set, )
{
   E_Zone *zone = _e_actions_zone_get(obj);
   int v;
   if (params)
     v = atoi(params);
   else
     {
        v = e_backlight_level_get(zone) * 100.0;
        if (v == 0) v = 100;
        else v = 0;
     }
   e_backlight_mode_set(zone, E_BACKLIGHT_MODE_NORMAL);
   e_backlight_level_set(zone, ((double)v / 100.0), -1.0);
   e_config->backlight.normal = e_backlight_level_get(zone);
   e_config_save_queue();
}

ACT_FN_GO(backlight_adjust, )
{
   E_Zone *zone = _e_actions_zone_get(obj);
   int v;
   if (!params) return;
   v = atoi(params);
   e_backlight_mode_set(zone, E_BACKLIGHT_MODE_NORMAL);
   e_backlight_level_set(zone, e_backlight_level_get(zone) + ((double)v / 100.0), -1.0);
   e_config->backlight.normal = e_backlight_level_get(zone);
   e_config_save_queue();
}

ACT_FN_GO(blanking, EINA_UNUSED)
{
   if (params && (!strcmp(params, "on")))
     e_config->screensaver_enable = 1;
   else if (params && (!strcmp(params, "off")))
     e_config->screensaver_enable = 0;
   else if (params && (!strcmp(params, "toggle")))
     {
        if (e_config->screensaver_enable) e_config->screensaver_enable = 0;
        else e_config->screensaver_enable = 1;
     }
   e_screensaver_update();
   e_dpms_update();
   e_config_save_queue();
}

ACT_FN_GO(kbd_layout, )
{
   unsigned int x;

   if (!params) return;
   errno = 0;
   x = strtoul(params, NULL, 10);
   if (errno || (x > eina_list_count(e_config->xkb.used_layouts) - 1)) return;
   e_xkb_layout_set(eina_list_nth(e_config->xkb.used_layouts, x));
}

ACT_FN_GO(kbd_layout_next, EINA_UNUSED)
{
   e_xkb_layout_next();
}

ACT_FN_GO(kbd_layout_prev, EINA_UNUSED)
{
   e_xkb_layout_prev();
}

#ifdef HAVE_WAYLAND
ACT_FN_GO_MOUSE(mouse_to_key, )
{
   const char *p, *nextp, *key = NULL;
   const char *mods[] =
   {
      "shift",
      "ctrl",
      "alt",
      "win",
      "altgr",
      NULL
   };
   int modifiers = 0, mod = 0;

   if ((!params) || (!params[0]) || (params[0] == '+')) return EINA_TRUE;
   for (p = params; p; p = nextp)
     {
        const char **m;

        nextp = strchr(p + 1, '+');
        if (!nextp) break;
        for (m = mods; *m; m++)
          {
             if (strncmp(p, *m, nextp - p)) continue;
             modifiers |= 1 << (m - mods);
             break;
          }
        key = nextp;
     }
   if (key)
     key++;
   else
     key = params;
   if (!key[0]) return EINA_TRUE;

   mod |= (ECORE_EVENT_MODIFIER_SHIFT * !!(modifiers & E_BINDING_MODIFIER_SHIFT));
   mod |= (ECORE_EVENT_MODIFIER_CTRL * !!(modifiers & E_BINDING_MODIFIER_CTRL));
   mod |= (ECORE_EVENT_MODIFIER_ALT * !!(modifiers & E_BINDING_MODIFIER_ALT));
   mod |= (ECORE_EVENT_MODIFIER_WIN * !!(modifiers & E_BINDING_MODIFIER_WIN));
   mod |= (ECORE_EVENT_MODIFIER_ALTGR * !!(modifiers & E_BINDING_MODIFIER_ALTGR));
   e_comp_wl_input_keyboard_event_generate(key, mod, 0);
   e_comp_wl_input_keyboard_event_generate(key, mod, 1);
   return EINA_TRUE;
}
#endif

ACT_FN_GO(module_enable, )
{
   E_Module *m;

   if (!params) return;
   m = e_module_find(params);
   if (!m)
     {
        m = e_module_new(params);
        if (!m) return;
     }
   e_module_enable(m);
}

ACT_FN_GO(module_disable, )
{
   E_Module *m;

   if (!params) return;
   m = e_module_find(params);
   if (!m) return;
   e_module_disable(m);
}

ACT_FN_GO(module_toggle, )
{
   E_Module *m;

   fprintf(stderr, "toggle\n");
   if (!params) return;
   fprintf(stderr, "'%s'\n", params);
   m = e_module_find(params);
   fprintf(stderr, "m = %p\n", m);
   if (!m)
     {
        m = e_module_new(params);
        if (!m) return;
     }
   fprintf(stderr, "currently %i\n", e_module_enabled_get(m));
   if (e_module_enabled_get(m)) e_module_disable(m);
   else e_module_enable(m);
}

ACT_FN_GO(screen_redo, EINA_UNUSED)
{
   printf("REDOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
#ifndef HAVE_WAYLAND_ONLY
   e_randr2_screeninfo_update();
   e_randr2_config_apply();
#endif
}

/***************************************************************************/
static Eina_Bool
_skip_win(E_Client *ec, E_Zone *zone, E_Desk *desk)
{
   if ((!ec->icccm.accepts_focus) && (!ec->icccm.take_focus)) return EINA_TRUE;
   if (ec->netwm.state.skip_taskbar) return EINA_TRUE;
   if (ec->user_skip_winlist) return EINA_TRUE;
   if (ec->iconic) return EINA_TRUE;
   if (ec->zone != zone) return EINA_TRUE;
   if (!((ec->sticky) || (ec->desk == desk))) return EINA_TRUE;
   return EINA_FALSE;
}

static int
_point_line_dist(int x, int y, int lx1, int ly1, int lx2, int ly2)
{
   int xx, yy, dx, dy;
   int a = x - lx1;
   int b = y - ly1;
   int c = lx2 - lx1;
   int d = ly2 - ly1;
   int dot = (a * c) + (b * d);
   int len_sq = (c * c) + (d * d);
   double dist, param = -1.0;

   // if line is 0 length
   if (len_sq) param = (double)dot / len_sq;

   if (param < 0)
     {
        xx = lx1;
        yy = ly1;
     }
   else if (param > 1)
     {
        xx = lx2;
        yy = ly2;
     }
   else
     {
        xx = lx1 + lround(param * c);
        yy = ly1 + lround(param * d);
     }

   dx = x - xx;
   dy = y - yy;
   dist = sqrt((dx * dx) + (dy * dy));
   return lround(dist);
}

ACT_FN_GO(window_focus, EINA_UNUSED)
{
   E_Zone *zone = e_zone_current_get();
   E_Desk *desk = e_desk_current_get(zone);
   E_Client *ec, *ec_orig,
     *ec_prev = NULL, *ec_last = NULL, *ec_first = NULL, *ec_next = NULL;
   Eina_List *l;
   int distance = INT_MAX, cx, cy, dir = -1, found = 0;

   if (!params) return;
   ec_orig = e_client_focused_get();
   if (!ec_orig)
     {
        // XXX: just pick any window to focus
        EINA_LIST_FOREACH(e_client_focus_stack_get(), l, ec)
          {
             if (_skip_win(ec, zone, desk)) continue;
             e_client_focus_set_with_pointer(ec);
             return;
          }
        return;
     }

   if      (!strcmp(params, "next"))  dir = -1;
   else if (!strcmp(params, "prev"))  dir = -2;
   else if (!strcmp(params, "up"))    dir =  0;
   else if (!strcmp(params, "down"))  dir =  1;
   else if (!strcmp(params, "left"))  dir =  2;
   else if (!strcmp(params, "right")) dir =  3;
   else
     {
        e_util_dialog_show(_("Error: window_focus action"),
                           _("Invalid parameter: %s"), params);
        return;
     }
   if (dir < 0)
     {
        EINA_LIST_FOREACH(e_client_focus_stack_get(), l, ec)
          {
             if (_skip_win(ec, zone, desk)) continue;

             if (ec == ec_orig)              found = 1;
             else if (!found)                ec_prev = ec;
             else if ((found) && (!ec_next)) ec_next = ec;

             if (!ec_first)                  ec_first = ec;
             ec_last = ec;
          }
        if (dir == -1) /* next */
          {
             if (ec_next) e_client_focus_set_with_pointer(ec_next);
             else if (ec_first) e_client_focus_set_with_pointer(ec_first);
          }
        else if (dir == -2)
          {
             if (ec_prev) e_client_focus_set_with_pointer(ec_prev);
             else if (ec_last) e_client_focus_set_with_pointer(ec_last);
          }
        return;
     }

   cx = ec_orig->x + (ec_orig->w / 2);
   cy = ec_orig->y + (ec_orig->h / 2);

   EINA_LIST_FOREACH(e_client_focus_stack_get(), l, ec)
     {
        int a = 0, d = 0;

        if (ec == ec_orig) continue;
        if (_skip_win(ec, zone, desk)) continue;

        switch (dir)
          {
           case 0: /* up */
             d = _point_line_dist(cx, cy,
                                  ec->x,         ec->y + ec->h,
                                  ec->x + ec->w, ec->y + ec->h);
             if (d >= distance) continue;
             d = _point_line_dist(cx, cy,
                                  ec->x,         ec->y + (ec->h / 2),
                                  ec->x + ec->w, ec->y + (ec->h / 2));
             if (d >= distance) continue;
             if (cy <= (ec->y + (ec->h / 2))) continue;
             a = abs(cx - (ec->x + (ec->w / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
           case 1: /* down */
             d = _point_line_dist(cx, cy,
                                  ec->x,         ec->y,
                                  ec->x + ec->w, ec->y);
             if (d >= distance) continue;
             d = _point_line_dist(cx, cy,
                                  ec->x,         ec->y + (ec->h / 2),
                                  ec->x + ec->w, ec->y + (ec->h / 2));
             if (d >= distance) continue;
             if (cy >= (ec->y + (ec->h / 2))) continue;
             a = abs(cx - (ec->x + (ec->w / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
           case 2: /* left */
             d = _point_line_dist(cx, cy,
                                  ec->x + ec->w, ec->y,
                                  ec->x + ec->w, ec->y + ec->h);
             if (d >= distance) continue;
             d = _point_line_dist(cx, cy,
                                  ec->x + (ec->w / 2), ec->y,
                                  ec->x + (ec->w / 2), ec->y + ec->h);
             if (d >= distance) continue;
             if (cx <= (ec->x + (ec->w / 2))) continue;
             a = abs(cy - (ec->y + (ec->h / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
           case 3: /* right */
             d = _point_line_dist(cx, cy,
                                  ec->x, ec->y,
                                  ec->x, ec->y + ec->h);
             if (d >= distance) continue;
             d = _point_line_dist(cx, cy,
                                  ec->x + (ec->w / 2), ec->y,
                                  ec->x + (ec->w / 2), ec->y + ec->h);
             if (d >= distance) continue;
             if (cx >= (ec->x + (ec->w / 2))) continue;
             a = abs(cy - (ec->y + (ec->h / 2)));
             d += (a * a) / d;
             if (d >= distance) continue;
             break;
          }
        ec_next = ec;
        distance = d;
     }

   if (ec_next) e_client_focus_set_with_pointer(ec_next);
}

/* local subsystem globals */
static Eina_Hash *actions = NULL;
static Eina_List *action_list = NULL;
static Eina_List *action_names = NULL;
static Eina_List *action_groups = NULL;

static void
_e_actions_post_init(void *d EINA_UNUSED)
{
#ifdef HAVE_WAYLAND
   E_Action *act;

   /* wayland-specific actions */
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     {
        /* mouse -> key */
        /* "key" here is the platform-specific key name;
         * /usr/share/X11/xkb/keycodes/evdev is probably what your system is using
         */
        ACT_GO_MOUSE(mouse_to_key);
        e_action_predef_name_set(N_("Mouse Remapping"),
                                 N_("Mouse to key"), "mouse_to_key",
                                 NULL, "[AD02] [ctrl+shift+alt+win+altgr+AD02]", 1);
     }
#endif
}

ACT_FN_GO_KEY(profile_switch, , EINA_UNUSED)
{
   E_Action *a;
   const char *cur_profile;

   if (!params) return;

   cur_profile = e_config_profile_get();

   if (strcmp(cur_profile, params) == 0) return;

   e_config_save_flush();
   e_config_profile_set(params);
   elm_config_profile_set(params);
   e_config_profile_save();
   e_config_save_block_set(1);

   a = e_action_find("restart");
   if ((a) && (a->func.go)) a->func.go(NULL, NULL);
}

static void
_e_actions_profiles_create(void)
{
   E_Action *act;
   Eina_List *profiles, *l;
   int i = 0;

   ACT_GO_KEY(profile_switch);

   profiles = e_config_profile_list();
   for (i = 0, l = profiles; l; l = l->next, i++)
     {
        Efreet_Desktop *desk = NULL;
        char *prof, *pdir, buff[PATH_MAX];
        const char *label;

        prof = l->data;

        pdir = e_config_profile_dir_get(prof);
        snprintf(buff, sizeof(buff), "%s/profile.desktop", pdir);
        desk = efreet_desktop_new(buff);
        if (!desk)
          {
             e_prefix_data_snprintf(buff, sizeof(buff),
                                    "data/config/%s/", prof);
             free(pdir);
             pdir = strdup(buff);
             if (pdir)
               {
                  snprintf(buff, sizeof(buff), "%s/profile.desktop", pdir);
                  desk = efreet_desktop_new(buff);
               }
          }

        label = prof;
        if ((desk) && (desk->name)) label = desk->name;
        e_action_predef_name_set(N_("Profile: Switch"), label,
                                 "profile_switch", prof, NULL, 0);

        free(prof);
        free(pdir);
        if (desk) efreet_desktop_free(desk);
     }
   if (profiles) eina_list_free(profiles);
}

/* externally accessible functions */

EINTERN int
e_actions_init(void)
{
   E_Action *act;

   actions = eina_hash_string_superfast_new(NULL);
   ACT_GO(window_move);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move"),
                            "window_move", NULL, NULL, 0);

   ACT_GO_MOUSE(window_move);
   ACT_GO_SIGNAL(window_move);
   ACT_END(window_move);
   ACT_END_MOUSE(window_move);
   ACT_GO_KEY(window_move);

   /* window_resize */
   ACT_GO(window_resize);
   e_action_predef_name_set(N_("Window : Actions"), N_("Resize"),
                            "window_resize", NULL, NULL, 0);

   ACT_GO_MOUSE(window_resize);
   ACT_GO_SIGNAL(window_resize);
   ACT_END(window_resize);
   ACT_END_MOUSE(window_resize);
   ACT_GO_KEY(window_resize);

   /* window_menu */
   ACT_GO(window_menu);
   e_action_predef_name_set(N_("Menu"), N_("Window Menu"),
                            "window_menu", NULL, NULL, 0);

   ACT_GO_MOUSE(window_menu);
   ACT_GO_KEY(window_menu);

   /* window_raise */
   ACT_GO(window_raise);
   e_action_predef_name_set(N_("Window : Actions"), N_("Raise"),
                            "window_raise", NULL, NULL, 0);

   /* window_lower */
   ACT_GO(window_lower);
   e_action_predef_name_set(N_("Window : Actions"), N_("Lower"),
                            "window_lower", NULL, NULL, 0);

   /* window_close */
   ACT_GO(window_close);
   e_action_predef_name_set(N_("Window : Actions"), N_("Close"),
                            "window_close", NULL, NULL, 0);

   /* window_kill */
   ACT_GO(window_kill);
   e_action_predef_name_set(N_("Window : Actions"), N_("Kill"),
                            "window_kill", NULL, NULL, 0);

   /* window_sticky_toggle */
   ACT_GO(window_sticky_toggle);
   e_action_predef_name_set(N_("Window : State"), N_("Sticky Mode Toggle"),
                            "window_sticky_toggle", NULL, NULL, 0);

   ACT_GO(window_sticky);
   e_action_predef_name_set(N_("Window : State"), N_("Sticky Mode Enable"),
                            "window_sticky", NULL, NULL, 0);

   /* window_iconic_toggle */
   ACT_GO(window_iconic_toggle);
   e_action_predef_name_set(N_("Window : State"), N_("Iconic Mode Toggle"),
                            "window_iconic_toggle", NULL, NULL, 0);

   ACT_GO(window_iconic);
   e_action_predef_name_set(N_("Window : State"), N_("Iconic Mode Enable"),
                            "window_iconic", NULL, NULL, 0);

   /* window_fullscreen_toggle */
   ACT_GO(window_fullscreen_toggle);
   e_action_predef_name_set(N_("Window : State"), N_("Fullscreen Mode Toggle"),
                            "window_fullscreen_toggle", NULL, NULL, 0);

   ACT_GO(window_fullscreen);
   e_action_predef_name_set(N_("Window : State"), N_("Fullscreen Mode Enable"),
                            "window_fullscreen", NULL, NULL, 0);

   /* window_maximized_toggle */
   ACT_GO(window_maximized_toggle);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize"),
                            "window_maximized_toggle", NULL, NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Vertically"),
                            "window_maximized_toggle", "default vertical",
                            NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Horizontally"),
                            "window_maximized_toggle", "default horizontal",
                            NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Left"),
                            "window_maximized_toggle", "default left",
                            NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Right"),
                            "window_maximized_toggle", "default right",
                            NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Fullscreen"),
                            "window_maximized_toggle", "fullscreen", NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Mode \"Smart\""),
                            "window_maximized_toggle", "smart", NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Mode \"Expand\""),
                            "window_maximized_toggle", "expand", NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Maximize Mode \"Fill\""),
                            "window_maximized_toggle", "fill", NULL, 0);

   ACT_GO(window_maximized);

   /* window_shaded_toggle */
   ACT_GO(window_shaded_toggle);
   e_action_predef_name_set(N_("Window : State"), N_("Shade Up Mode Toggle"),
                            "window_shaded_toggle", "up", NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Shade Down Mode Toggle"),
                            "window_shaded_toggle", "down", NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Shade Left Mode Toggle"),
                            "window_shaded_toggle", "left", NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Shade Right Mode Toggle"),
                            "window_shaded_toggle", "right", NULL, 0);
   e_action_predef_name_set(N_("Window : State"), N_("Shade Mode Toggle"),
                            "window_shaded_toggle", NULL, NULL, 0);

   ACT_GO(window_shaded);
   e_action_predef_name_set(N_("Window : State"), N_("Set Shaded State"),
                            "window_shaded", NULL, _("syntax: \"(0|1) (up|down|left|right)\""), 1);

   /* window_borderless_toggle */
   ACT_GO(window_borderless_toggle);
   e_action_predef_name_set(N_("Window : State"), N_("Toggle Borderless State"),
                            "window_borderless_toggle", NULL, NULL, 0);

   /* window_border_set */
   ACT_GO(window_border_set);
   e_action_predef_name_set(N_("Window : State"), N_("Set Border"),
                            "window_border_set", NULL,
                            "syntax: BorderName, example: pixel", 1);

   /* window_border_cycle */
   ACT_GO(window_border_cycle);
   e_action_predef_name_set(N_("Window : State"), N_("Cycle between Borders"),
                            "window_border_cycle", NULL,
                            "syntax: BorderNames, example: default pixel", 1);

   /* window_pinned_toggle */
   ACT_GO(window_pinned_toggle);
   e_action_predef_name_set(N_("Window : State"), N_("Toggle Pinned State"),
                            "window_pinned_toggle", NULL, NULL, 0);

   /* desk_flip_by */
   ACT_GO(desk_flip_by);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Left"),
                            "desk_flip_by", "-1 0", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Right"),
                            "desk_flip_by", "1 0", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Up"),
                            "desk_flip_by", "0 -1", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Down"),
                            "desk_flip_by", "0 1", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop By..."),
                            "desk_flip_by", NULL,
                            "syntax: X-offset Y-offset, example: -1 0", 1);

   /* desk_flip_prev */
   ACT_GO(desk_flip_prev);
   e_action_predef_name_set(N_("Desktop"), N_("Flip To Previous Desktop"),
                            "desk_flip_prev", NULL, NULL, 0);

   /* desk_deskshow_toggle */
   ACT_GO(desk_deskshow_toggle);
   e_action_predef_name_set(N_("Desktop"), N_("Show The Desktop"),
                            "desk_deskshow_toggle", NULL, NULL, 0);

   /* shelf_show */
   ACT_GO(shelf_show);
   ACT_GO_EDGE(shelf_show);
   e_action_predef_name_set(N_("Desktop"), N_("Show The Shelf"), "shelf_show",
                            NULL, "shelf name glob: Shelf-* ", 1);

   /* desk_linear_flip_to */
   ACT_GO(desk_flip_to);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop To..."),
                            "desk_flip_to", NULL,
                            "syntax: X Y, example: 1 2", 1);
   /* desk_linear_flip_to_screen */
   ACT_GO(desk_linear_flip_to_screen);
   e_action_predef_name_set(N_("Desktop"), N_("Switch Desktop To... On Screen..."),
                            "desk_linear_flip_to_screen", NULL,
                            "syntax: D S, example: 0 1", 1);

   /* desk_linear_flip_by */
   ACT_GO(desk_linear_flip_by);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Linearly..."),
                            "desk_linear_flip_by",
                            NULL, "syntax: N-offset, example: -2", 1);

   /* desk_linear_flip_to */
   ACT_GO(desk_linear_flip_to);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 0"),
                            "desk_linear_flip_to", "0", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 1"),
                            "desk_linear_flip_to", "1", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 2"),
                            "desk_linear_flip_to", "2", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 3"),
                            "desk_linear_flip_to", "3", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 4"),
                            "desk_linear_flip_to", "4", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 5"),
                            "desk_linear_flip_to", "5", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 6"),
                            "desk_linear_flip_to", "6", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 7"),
                            "desk_linear_flip_to", "7", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 8"),
                            "desk_linear_flip_to", "8", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 9"),
                            "desk_linear_flip_to", "9", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 10"),
                            "desk_linear_flip_to", "10", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 11"),
                            "desk_linear_flip_to", "11", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop..."),
                            "desk_linear_flip_to", NULL,
                            "syntax: N, example: 1", 1);

   /* desk_flip_by_all */
   ACT_GO(desk_flip_by_all);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Left (All Screens)"),
                            "desk_flip_by_all", "-1 0", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Right (All Screens)"),
                            "desk_flip_by_all", "1 0", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Up (All Screens)"),
                            "desk_flip_by_all", "0 -1", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Down (All Screens)"),
                            "desk_flip_by_all", "0 1", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop By... (All Screens)"),
                            "desk_flip_by_all", NULL,
                            "syntax: X-offset Y-offset, example: -1 0", 1);

   /* desk_flip_to_all */
   ACT_GO(desk_flip_to_all);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop To... (All Screens)"),
                            "desk_flip_to_all", NULL,
                            "syntax: X Y, example: 1 2", 1);

   /* desk_linear_flip_by_all */
   ACT_GO(desk_linear_flip_by_all);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop Linearly... (All Screens)"),
                            "desk_linear_flip_by_all",
                            NULL, "syntax: N-offset, example: -2", 1);

   /* desk_flip_in_direction */
   ACT_GO_EDGE(desk_flip_in_direction);
   e_action_predef_name_set(N_("Desktop"), N_("Flip Desktop In Direction..."),
                            "desk_flip_in_direction", NULL, "syntax: N-pixel-offset, example: 25", 1);

   /* desk_linear_flip_to_all */
   ACT_GO(desk_linear_flip_to_all);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 0 (All Screens)"),
                            "desk_linear_flip_to_all", "0", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 1 (All Screens)"),
                            "desk_linear_flip_to_all", "1", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 2 (All Screens)"),
                            "desk_linear_flip_to_all", "2", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 3 (All Screens)"),
                            "desk_linear_flip_to_all", "3", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 4 (All Screens)"),
                            "desk_linear_flip_to_all", "4", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 5 (All Screens)"),
                            "desk_linear_flip_to_all", "5", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 6 (All Screens)"),
                            "desk_linear_flip_to_all", "6", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 7 (All Screens)"),
                            "desk_linear_flip_to_all", "7", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 8 (All Screens)"),
                            "desk_linear_flip_to_all", "8", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 9 (All Screens)"),
                            "desk_linear_flip_to_all", "9", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 10 (All Screens)"),
                            "desk_linear_flip_to_all", "10", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop 11 (All Screens)"),
                            "desk_linear_flip_to_all", "11", NULL, 0);
   e_action_predef_name_set(N_("Desktop"), N_("Switch To Desktop... (All Screens)"),
                            "desk_linear_flip_to_all", NULL,
                            "syntax: N, example: 1", 1);

   /* window_jump_to */
   ACT_GO(window_jump_to);
   e_action_predef_name_set(N_("Window : List"), N_("Jump to window..."),
                            "window_jump_to", NULL, "syntax: icccm window name, example: urxvt-mutt", 1);

   ACT_GO(window_jump_to_or_start);
   e_action_predef_name_set(N_("Window : List"), N_("Jump to window... or start..."),
                            "window_jump_to_or_start", NULL, "syntax: icccm_window_name application", 1);

   /* screen_send_to */
   ACT_GO(screen_send_to);
   e_action_predef_name_set(N_("Screen"), N_("Send Mouse To Screen 0"),
                            "screen_send_to", "0", NULL, 0);
   e_action_predef_name_set(N_("Screen"), N_("Send Mouse To Screen 1"),
                            "screen_send_to", "1", NULL, 0);
   e_action_predef_name_set(N_("Screen"), N_("Send Mouse To Screen..."),
                            "screen_send_to", NULL,
                            "syntax: N, example: 0", 1);

   /* screen_send_by */
   ACT_GO(screen_send_by);
   e_action_predef_name_set(N_("Screen"), N_("Send Mouse Forward 1 Screen"),
                            "screen_send_by", "1", NULL, 0);
   e_action_predef_name_set(N_("Screen"), N_("Send Mouse Back 1 Screen"),
                            "screen_send_by", "-1", NULL, 0);
   e_action_predef_name_set(N_("Screen"), N_("Send Mouse Forward/Back Screens..."),
                            "screen_send_by", NULL,
                            "syntax: N-offset, example: -2", 1);

   ACT_GO(dim_screen);
   e_action_predef_name_set(N_("Screen"), N_("Dim"), "dim_screen",
                            NULL, NULL, 0);
   ACT_GO(undim_screen);
   e_action_predef_name_set(N_("Screen"), N_("Undim"), "undim_screen",
                            NULL, NULL, 0);
   ACT_GO(backlight_set);
   e_action_predef_name_set(N_("Screen"), N_("Backlight Set"), "backlight_set",
                            NULL, "syntax: brightness(0 - 100), example: 50", 1);
   e_action_predef_name_set(N_("Screen"), N_("Backlight Min"), "backlight_set",
                            "0", NULL, 0);
   e_action_predef_name_set(N_("Screen"), N_("Backlight Mid"), "backlight_set",
                            "50", NULL, 0);
   e_action_predef_name_set(N_("Screen"), N_("Backlight Max"), "backlight_set",
                            "100", NULL, 0);
   ACT_GO(backlight_adjust);
   e_action_predef_name_set(N_("Screen"), N_("Backlight Adjust"), "backlight_adjust",
                            NULL, "syntax: brightness(-100 - 100), example: -20", 1);
   e_action_predef_name_set(N_("Screen"), N_("Backlight Up"), "backlight_adjust",
                            "10", NULL, 0);
   e_action_predef_name_set(N_("Screen"), N_("Backlight Down"), "backlight_adjust",
                            "-10", NULL, 0);

   /* blanking */
   ACT_GO(blanking);
   e_action_predef_name_set(N_("Screen"), N_("Blanking Off"), "blanking",
                            "off", "syntax: toggle/off/on, example: off", 0);
   e_action_predef_name_set(N_("Screen"), N_("Blanking On"), "blanking",
                            "on", "syntax: toggle/off/on, example: on", 0);
   e_action_predef_name_set(N_("Screen"), N_("Blanking Toggle"), "blanking",
                            "toggle", "syntax: toggle/off/on, example: toggle", 0);
   /* screen setup */
   ACT_GO(screen_redo);
   e_action_predef_name_set(N_("Screen"),
                            N_("Update and re-apply screen setup"),
                            "screen_redo", NULL, NULL, 0);

   /* window_move_to_center */
   ACT_GO(window_move_to_center);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move To Center"),
                            "window_move_to_center", NULL, NULL, 0);

   /* window_quick_tile_upper_left */
   ACT_GO(window_quick_tile_to_quadrant);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move/resize to upper-left quadrant"),
                            "window_quick_tile_to_quadrant", "upper_left", NULL, 0);

   /* window_quick_tile_upper_right */
   ACT_GO(window_quick_tile_to_quadrant);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move/resize to upper-right quadrant"),
                            "window_quick_tile_to_quadrant", "upper_right", NULL, 0);

   /* window_quick_tile_lower_left */
   ACT_GO(window_quick_tile_to_quadrant);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move/resize to lower-left quadrant"),
                            "window_quick_tile_to_quadrant", "lower_left", NULL, 0);

   /* window_quick_tile_lower_right */
   ACT_GO(window_quick_tile_to_quadrant);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move/resize to lower-right quadrant"),
                            "window_quick_tile_to_quadrant", "lower_right", NULL, 0);

   /* window_move_to */
   ACT_GO(window_move_to);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move To Coordinates..."),
                            "window_move_to", NULL,
                            "syntax: [+,-]X [+,-]Y or * [+,-]Y or [+,-]X *, example: -1 +1", 1);
   /* window_move_by */
   ACT_GO(window_move_by);
   e_action_predef_name_set(N_("Window : Actions"), N_("Move By Coordinate Offset..."),
                            "window_move_by", NULL,
                            "syntax: X-offset Y-offset, example: -1 0", 1);

   /* window_resize_by */
   ACT_GO(window_resize_by);
   e_action_predef_name_set(N_("Window : Actions"), N_("Resize By..."),
                            "window_resize_by", NULL,
                            "syntax: W H, example: 100 150", 1);

   /* window_push */
   ACT_GO(window_push);
   e_action_predef_name_set(N_("Window : Actions"), N_("Push in Direction..."),
                            "window_push", NULL,
                            "syntax: direction, example: up, down, left, right, up-left, up-right, down-left, down-right", 1);

   /* window_grow */
   ACT_GO(window_grow);
   e_action_predef_name_set(N_("Window : Actions"), N_("Grow in Direction..."),
                            "window_grow", NULL,
                            "syntax: direction, example: up, down, left, right, up-left, up-right, down-left, down-right", 1);

   /* window_drag_icon */
   ACT_GO(window_drag_icon);
   e_action_predef_name_set(N_("Window : Actions"), N_("Drag Icon..."),
                            "window_drag_icon", NULL, NULL, 0);

   /* window_desk_move_by */
   ACT_GO(window_desk_move_by);
   e_action_predef_name_set(N_("Window : Moving"), N_("To Next Desktop"),
                            "window_desk_move_by", "1 0", NULL, 0);
   e_action_predef_name_set(N_("Window : Moving"), N_("To Previous Desktop"),
                            "window_desk_move_by", "-1 0", NULL, 0);
   e_action_predef_name_set(N_("Window : Moving"), N_("By Desktop #..."),
                            "window_desk_move_by", NULL,
                            "syntax: X-offset Y-offset, example: -2 2", 1);

   /* window_desk_move_to */
   ACT_GO(window_desk_move_to);
   e_action_predef_name_set(N_("Window : Moving"), N_("To Desktop..."),
                            "window_desk_move_to", NULL,
                            "syntax: X Y, example: 0 1", 1);

   /* window_zone_move_by */
   ACT_GO(window_zone_move_by);
   e_action_predef_name_set(N_("Window : Moving"), N_("To Next Screen"),
                            "window_zone_move_by", "1", NULL, 0);
   e_action_predef_name_set(N_("Window : Moving"), N_("To Previous Screen"),
                            "window_zone_move_by", "-1", NULL, 0);

   /* Move window focus somewhere */
   ACT_GO(window_focus);
   e_action_predef_name_set(N_("Window : Focus"),
                            N_("Focus next window"),
                            "window_focus", "next", NULL, 0);
   e_action_predef_name_set(N_("Window : Focus"),
                            N_("Focus previous window"),
                            "window_focus", "prev", NULL, 0);
   e_action_predef_name_set(N_("Window : Focus"),
                            N_("Focus window above"),
                            "window_focus", "up", NULL, 0);
   e_action_predef_name_set(N_("Window : Focus"),
                            N_("Focus window below"),
                            "window_focus", "down", NULL, 0);
   e_action_predef_name_set(N_("Window : Focus"),
                            N_("Focus window left"),
                            "window_focus", "left", NULL, 0);
   e_action_predef_name_set(N_("Window : Focus"),
                            N_("Focus window right"),
                            "window_focus", "right", NULL, 0);

   /* menu_show */
   ACT_GO(menu_show);
   e_action_predef_name_set(N_("Menu"), N_("Show Main Menu"),
                            "menu_show", "main", NULL, 0);
   e_action_predef_name_set(N_("Menu"), N_("Show Favorites Menu"), "menu_show",
                            "favorites", NULL, 0);
   e_action_predef_name_set(N_("Menu"), N_("Show All Applications Menu"),
                            "menu_show", "all", NULL, 0);
   e_action_predef_name_set(N_("Menu"), N_("Show Clients Menu"), "menu_show",
                            "clients", NULL, 0);
   e_action_predef_name_set(N_("Menu"), N_("Show Menu..."), "menu_show", NULL,
                            "syntax: MenuName, example: MyMenu", 1);
   ACT_GO_MOUSE(menu_show);
   ACT_GO_KEY(menu_show);

   /* internal: for showing a menu from an object */
   ACT_GO(menu_show_object);

   /* exec */
   ACT_GO(exec);
   e_action_predef_name_set(N_("Launch"), N_("Command"), "exec", NULL,
                            "syntax: CommandName, example: /usr/bin/xmms", 1);

   /* app */
   ACT_GO(app);
   e_action_predef_name_set(N_("Launch"), N_("Application"), "app", NULL,
                            "syntax: [file:file.desktop|name:App Name|generic:Generic Name|exe:exename], example: file:terminology.desktop | file:/path/to/terminology.desktop | name:Terminology | generic:Terminal Emulator | exe:xterm", 1);

   /* new instance of focused app */
   ACT_GO(app_new_instance);
   e_action_predef_name_set(N_("Launch"), N_("New Instance of Focused App"), "app_new_instance", NULL,
                            NULL, 0);

   ACT_GO(restart);
   e_action_predef_name_set(N_("Enlightenment"), N_("Restart"), "restart",
                            NULL, NULL, 0);

   ACT_GO(exit);
   e_action_predef_name_set(N_("Enlightenment"), N_("Exit"), "exit",
                            NULL, NULL, 0);

   ACT_GO(exit_now);
   e_action_predef_name_set(N_("Enlightenment"), N_("Exit Now"),
                            "exit_now", NULL, NULL, 0);

   /* modules */
   ACT_GO(module_enable);
   e_action_predef_name_set(N_("Enlightenment : Module"),
                            N_("Enable the named module"),
                            "module_enable", NULL, NULL, 1);
   ACT_GO(module_disable);
   e_action_predef_name_set(N_("Enlightenment : Module"),
                            N_("Disable the named module"),
                            "module_disable", NULL, NULL, 1);
   ACT_GO(module_toggle);
   e_action_predef_name_set(N_("Enlightenment : Module"),
                            N_("Toggle the named module"),
                            "module_toggle", NULL, NULL, 1);

   ACT_GO(logout);
   e_action_predef_name_set(N_("System"), N_("Log Out"), "logout",
                            NULL, NULL, 0);

   ACT_GO(halt_now);
   e_action_predef_name_set(N_("System"), N_("Power Off Now"),
                            "halt_now", NULL, NULL, 0);

   ACT_GO(halt);
   e_action_predef_name_set(N_("System"), N_("Power Off"), "halt",
                            NULL, NULL, 0);

   ACT_GO(reboot);
   e_action_predef_name_set(N_("System"), N_("Reboot"), "reboot",
                            NULL, NULL, 0);

   ACT_GO(suspend_now);
   e_action_predef_name_set(N_("System"), N_("Suspend Now"), "suspend_now",
                            NULL, NULL, 0);

   ACT_GO(suspend);
   e_action_predef_name_set(N_("System"), N_("Suspend"), "suspend",
                            NULL, NULL, 0);

   ACT_GO(suspend_smart);
   e_action_predef_name_set(N_("System"), N_("Suspend Intelligently"), "suspend_smart",
                            NULL, NULL, 0);

   ACT_GO(hibernate);
   e_action_predef_name_set(N_("System"), N_("Hibernate"), "hibernate",
                            NULL, NULL, 0);

   ACT_GO(hibernate_now);
   e_action_predef_name_set(N_("System"), N_("Hibernate Now"), "hibernate_now",
                            NULL, NULL, 0);

   ACT_GO(hibernate_smart);
   e_action_predef_name_set(N_("System"), N_("Hibernate Intelligently"), "hibernate_smart",
                            NULL, NULL, 0);

   ACT_GO(pointer_resize_push);
   ACT_GO(pointer_resize_pop);

   /* desk_lock */
   ACT_GO(desk_lock);
   e_action_predef_name_set(N_("Desktop"), N_("Lock"), "desk_lock",
                            NULL, NULL, 0);

   /* cleanup_windows */
   ACT_GO(cleanup_windows);
   e_action_predef_name_set(N_("Desktop"), N_("Cleanup Windows"),
                            "cleanup_windows", NULL, NULL, 0);

   /* delayed_action */
   ACT_GO_KEY(delayed_action);
   e_action_predef_name_set(N_("Generic : Actions"), N_("Delayed Action"),
                            "delayed_action", NULL, "[0.0 exec xterm] [0.3 exec xev]", 1);
   ACT_GO_MOUSE(delayed_action);
   ACT_END_KEY(delayed_action);
   ACT_END_MOUSE(delayed_action);

   /* xkb */
   ACT_GO(kbd_layout);
   e_action_predef_name_set(N_("Keyboard Layouts"),
                            N_("Use keyboard layout"), "kbd_layout",
                            NULL, "syntax: Keyboard-layout-name, example: us", 1);
   ACT_GO(kbd_layout_next);
   e_action_predef_name_set(N_("Keyboard Layouts"),
                            N_("Next keyboard layout"), "kbd_layout_next",
                            NULL, NULL, 0);
   ACT_GO(kbd_layout_prev);
   e_action_predef_name_set(N_("Keyboard Layouts"),
                            N_("Previous keyboard layout"), "kbd_layout_prev",
                            NULL, NULL, 0);

   /* profiles */
   _e_actions_profiles_create();

   ecore_job_add(_e_actions_post_init, NULL);

   return 1;
}

EINTERN int
e_actions_shutdown(void)
{
   Eina_List *tlist = NULL, *l;
   E_Action *act;

   e_action_predef_name_all_del();

   EINA_LIST_FOREACH(action_list, l, act)
     tlist = eina_list_append(tlist, act);
   EINA_LIST_FREE(tlist, act)
     e_object_del(E_OBJECT(act));

   action_names = eina_list_free(action_names);
   eina_hash_free(actions);
   actions = NULL;

   return 1;
}

E_API Eina_List *
e_action_name_list(void)
{
   return action_names;
}

E_API E_Action *
e_action_add(const char *name)
{
   E_Action *act;

   act = e_action_find(name);
   if (!act)
     {
        act = E_OBJECT_ALLOC(E_Action, E_ACTION_TYPE, _e_action_free);
        if (!act) return NULL;
        act->name = name;
        eina_hash_direct_add(actions, act->name, act);
        action_names = eina_list_append(action_names, name);
        action_list = eina_list_append(action_list, act);
     }
   return act;
}

E_API void
e_action_del(const char *name)
{
   E_Action *act;

   act = eina_hash_find(actions, name);
   if (act) _e_action_free(act);
}

E_API E_Action *
e_action_find(const char *name)
{
   E_Action *act;

   act = eina_hash_find(actions, name);
   return act;
}

E_API const char *
e_action_predef_label_get(const char *action, const char *params)
{
   E_Action_Group *actg = NULL;
   E_Action_Description *actd = NULL;
   Eina_List *l, *l2;

   EINA_LIST_FOREACH(action_groups, l, actg)
     {
        EINA_LIST_FOREACH(actg->acts, l2, actd)
          {
             if (!strcmp(actd->act_cmd, action))
               {
                  if ((params) && (actd->act_params))
                    {
                       if (!strcmp(params, actd->act_params))
                         return actd->act_name;
                    }
                  else return actd->act_name;
               }
          }
     }
   if (params) return e_action_predef_label_get(action, NULL);
   return NULL;
}

E_API void
e_action_predef_name_set(const char *act_grp, const char *act_name, const char *act_cmd, const char *act_params, const char *param_example, int editable)
{
   E_Action_Group *actg = NULL;
   E_Action_Description *actd = NULL;
   Eina_List *l;

   if ((!act_grp) || (!act_name)) return;

   EINA_LIST_FOREACH(action_groups, l, actg)
     {
        if (!strcmp(actg->act_grp, act_grp)) break;
        actg = NULL;
     }

   if (!actg)
     {
        actg = E_NEW(E_Action_Group, 1);
        if (!actg) return;

        actg->act_grp = eina_stringshare_add(act_grp);
        action_groups = eina_list_append(action_groups, actg);
        action_groups =
          eina_list_sort(action_groups, -1, _action_groups_sort_cb);
     }

   EINA_LIST_FOREACH(actg->acts, l, actd)
     {
        if (!strcmp(actd->act_name, act_name)) break;
        actd = NULL;
     }

   if (actd) return;

   actd = E_NEW(E_Action_Description, 1);
   if (!actd) return;

   actd->act_name = eina_stringshare_add(act_name);
   actd->act_cmd = !act_cmd ? NULL : eina_stringshare_add(act_cmd);
   actd->act_params = !act_params ? NULL : eina_stringshare_add(act_params);
   actd->param_example = !param_example ? NULL : eina_stringshare_add(param_example);
   actd->editable = editable;

   actg->acts = eina_list_append(actg->acts, actd);
}

E_API void
e_action_predef_name_del(const char *act_grp, const char *act_name)
{
   E_Action_Group *actg = NULL;
   E_Action_Description *actd = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(action_groups, l, actg)
     {
        if (!strcmp(actg->act_grp, act_grp)) break;
        actg = NULL;
     }

   if (!actg) return;

   EINA_LIST_FOREACH(actg->acts, l, actd)
     {
        if (!strcmp(actd->act_name, act_name))
          {
             actg->acts = eina_list_remove(actg->acts, actd);

             if (actd->act_name) eina_stringshare_del(actd->act_name);
             if (actd->act_cmd) eina_stringshare_del(actd->act_cmd);
             if (actd->act_params) eina_stringshare_del(actd->act_params);
             if (actd->param_example) eina_stringshare_del(actd->param_example);

             E_FREE(actd);

             if (!eina_list_count(actg->acts))
               {
                  action_groups = eina_list_remove(action_groups, actg);
                  if (actg->act_grp) eina_stringshare_del(actg->act_grp);
                  E_FREE(actg);
               }
             break;
          }
     }
}

E_API void
e_action_predef_name_all_del(void)
{
   E_Action_Group *actg = NULL;
   E_Action_Description *actd = NULL;

   EINA_LIST_FREE(action_groups, actg)
     {
        EINA_LIST_FREE(actg->acts, actd)
          {
             if (actd->act_name) eina_stringshare_del(actd->act_name);
             if (actd->act_cmd) eina_stringshare_del(actd->act_cmd);
             if (actd->act_params) eina_stringshare_del(actd->act_params);
             if (actd->param_example) eina_stringshare_del(actd->param_example);

             E_FREE(actd);
          }
        if (actg->act_grp) eina_stringshare_del(actg->act_grp);
        E_FREE(actg);
     }
   action_groups = NULL;
}

E_API Eina_List *
e_action_groups_get(void)
{
   return action_groups;
}

/* local subsystem functions */

static void
_e_action_free(E_Action *act)
{
   eina_hash_del(actions, act->name, act);
   action_names = eina_list_remove(action_names, act->name);
   action_list = eina_list_remove(action_list, act);
   free(act);
}

static E_Maximize
_e_actions_maximize_parse(const char *params)
{
   E_Maximize max = 0;
   int ret;
   char s1[32], s2[32];

   if (!params) return e_config->maximize_policy;
   ret = sscanf(params, "%20s %20s", s1, s2);
   if (ret == 2)
     {
        if (!strcmp(s2, "horizontal"))
          max = E_MAXIMIZE_HORIZONTAL;
        else if (!strcmp(s2, "vertical"))
          max = E_MAXIMIZE_VERTICAL;
        else if (!strcmp(s2, "left"))
          max = E_MAXIMIZE_LEFT;
        else if (!strcmp(s2, "right"))
          max = E_MAXIMIZE_RIGHT;
        else
          max = E_MAXIMIZE_BOTH;
     }
   if (ret >= 1)
     {
        if (!strcmp(s1, "fullscreen"))
          max |= E_MAXIMIZE_FULLSCREEN;
        else if (!strcmp(s1, "smart"))
          max |= E_MAXIMIZE_SMART;
        else if (!strcmp(s1, "expand"))
          max |= E_MAXIMIZE_EXPAND;
        else if (!strcmp(s1, "fill"))
          max |= E_MAXIMIZE_FILL;
        else
          max |= (e_config->maximize_policy & E_MAXIMIZE_TYPE);
     }
   else
     max = e_config->maximize_policy;
   return max;
}

static int
_action_groups_sort_cb(const void *d1, const void *d2)
{
   const E_Action_Group *g1, *g2;

   if (!(g1 = d1)) return 1;
   if (!(g2 = d2)) return -1;
   return strcmp(g1->act_grp, g2->act_grp);
}

