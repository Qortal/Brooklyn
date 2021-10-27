#include "e_mod_main.h"
#include <sys/mman.h>

#define PASSWD_LEN                256

typedef enum
{
   LOKKER_STATE_DEFAULT,
   LOKKER_STATE_CHECKING,
   LOKKER_STATE_INVALID,
   LOKKER_STATE_LAST,
} Lokker_State;

typedef struct Lokker_Popup
{
   E_Zone *zone;
   Evas_Object *comp_object;
   Evas_Object *bg_object;
   Evas_Object *login_box;
   Eina_Bool show_anim E_BITFIELD;
   Eina_Bool hide_anim E_BITFIELD;
} Lokker_Popup;

typedef struct Lokker_Data
{
   Eina_List           *elock_wnd_list;
   Eina_List           *handlers;
   Ecore_Event_Handler *move_handler;
   char                 passwd[PASSWD_LEN];
   int                  state;
   Eina_Bool            selected E_BITFIELD;
} Lokker_Data;

static pid_t _auth_child_pid = -1;
static Ecore_Event_Handler *_auth_child_exit_handler = NULL;
static E_Zone *last_active_zone = NULL;

static Lokker_Data *edd = NULL;

static int _lokker_check_auth(void);

static Eina_Bool
lokker_is_pin(void)
{
   return e_config->desklock_auth_method == E_DESKLOCK_AUTH_METHOD_PIN;
}

static void
_lokker_state_set(int state)
{
   Eina_List *l;
   Lokker_Popup *lp;
   const char *signal_desklock, *text;
   if (!edd) return;

   edd->state = state;
   if (state == LOKKER_STATE_CHECKING)
     {
        signal_desklock = "e,state,checking";
        text = _("Authenticating...");
     }
   else if (state == LOKKER_STATE_INVALID)
     {
        signal_desklock = "e,state,invalid";
        text = _("The password you entered is invalid. Try again.");
     }
   else
     return;

   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     {
        edje_object_signal_emit(lp->login_box, signal_desklock, "e");
        edje_object_signal_emit(lp->bg_object, signal_desklock, "e");
        edje_object_part_text_set(lp->login_box, "e.text.title", text);
     }
}

static void
_text_passwd_update(void)
{
   int len, i;
   char passwd_hidden[PASSWD_LEN] = "", *pp;
   Lokker_Popup *lp;
   Eina_List *l;

   if (!edd) return;

   len = eina_unicode_utf8_get_len(edd->passwd);
   for (i = 0, pp = passwd_hidden; i < len; i++, pp++)
     *pp = '*';
   *pp = 0;

   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     if (lp->login_box)
       edje_object_part_text_set(lp->login_box, "e.text.password",
                                 passwd_hidden);
}

static void
_lokker_null(void)
{
   e_util_memclear(edd->passwd, PASSWD_LEN);

   _text_passwd_update();
}


static void
_lokker_select(void)
{
   Lokker_Popup *lp;
   Eina_List *l;
   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     if (lp->login_box)
       edje_object_signal_emit(lp->login_box, "e,state,selected", "e");
   edd->selected = EINA_TRUE;
}

static void
_lokker_unselect(void)
{
   Lokker_Popup *lp;
   Eina_List *l;
   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     if (lp->login_box)
       edje_object_signal_emit(lp->login_box, "e,state,unselected", "e");
   edd->selected = EINA_FALSE;
}

static void
_lokker_backspace(void)
{
   int len, val, pos;

   if (!edd) return;

   len = strlen(edd->passwd);
   if (len > 0)
     {
        pos = evas_string_char_prev_get(edd->passwd, len, &val);
        if ((pos < len) && (pos >= 0))
          {
             edd->passwd[pos] = 0;
             _text_passwd_update();
          }
     }
}

static void
_lokker_delete(void)
{
   _lokker_backspace();
}

static Eina_Bool
_pin_mouse_button_down(Lokker_Popup *lp EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Mouse_Button *ev)
{
   if (ev->buttons != 1) return ECORE_CALLBACK_DONE;
   evas_event_feed_mouse_move(e_comp->evas,
     e_comp_canvas_x_root_adjust(ev->root.x),
     e_comp_canvas_y_root_adjust(ev->root.y),
     0, NULL);
   evas_event_feed_mouse_down(e_comp->evas, 1, 0, 0, NULL);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_pin_mouse_button_up(Lokker_Popup *lp EINA_UNUSED, int t EINA_UNUSED, Ecore_Event_Mouse_Button *ev)
{
   if (ev->buttons != 1) return ECORE_CALLBACK_DONE;
   evas_event_feed_mouse_up(e_comp->evas, 1, 0, 0, NULL);
   return ECORE_CALLBACK_RENEW;
}

static void
_pin_click(void *data EINA_UNUSED, Evas_Object *obj, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   const char *name;
   int num;

   name = edje_object_part_text_get(obj, "e.text.label");
   if (!name) //wtf
     return;
   if (!e_util_strcmp(name, "Login"))
     {
        _lokker_check_auth();
        return;
     }
   if (!e_util_strcmp(name, "Delete"))
     {
        _lokker_backspace();
        return;
     }
   num = strtol(name, NULL, 10);
   if (num < 0) return;
   if (num > 9) return;
   if (edd->selected)
     {
        _lokker_null();
        _lokker_unselect();
     }
   if ((strlen(edd->passwd) < (PASSWD_LEN - strlen(name))))
     {
        strcat(edd->passwd, name);
        _text_passwd_update();
     }
}

static void
_pin_box_add(Lokker_Popup *lp)
{
   Evas *evas;
   Evas_Object *table, *o, *o2;
   int x, a = 0, b = 0;

   evas = evas_object_evas_get(lp->bg_object);
   lp->login_box = edje_object_add(evas);
   evas_object_name_set(lp->login_box, "desklock->login_box");
   e_theme_edje_object_set(lp->login_box,
                           "base/theme/desklock",
                           "e/desklock/pin_box");
   edje_object_part_text_set(lp->login_box, "e.text.title",
                             _("Please enter your PIN"));
   table = elm_table_add(e_win_evas_win_get(evas));
   e_comp_object_util_del_list_append(lp->login_box, table);
   elm_table_homogeneous_set(table, 1);
   for (x = 1; x < 11; x++)
     {
        char buf[8];

        o = edje_object_add(evas);
        e_comp_object_util_del_list_append(lp->login_box, o);
        e_theme_edje_object_set(o, "base/theme/desklock", "e/desklock/pin_button");
        snprintf(buf, sizeof(buf), "%d", x % 10);
        edje_object_part_text_set(o, "e.text.label", buf);
        evas_object_show(o);
        edje_object_signal_callback_add(o, "e,action,click", "*", _pin_click, lp);
        if (x == 10) a = 1;
        evas_object_size_hint_min_set(o, 48 * e_scale, 48 * e_scale);
        evas_object_size_hint_max_set(o, 48 * e_scale, 48 * e_scale);
        E_FILL(o);
        elm_table_pack(table, o, a, b, 1, 1);
        if (++a >= 3)
          {
             a = 0;
             b++;
          }
     }

   /* delete */
   o = edje_object_add(evas);
   e_comp_object_util_del_list_append(lp->login_box, o);
   e_theme_edje_object_set(o, "base/theme/desklock", "e/desklock/pin_button");
   edje_object_part_text_set(o, "e.text.label", "Delete");
   o2 = e_icon_add(evas);
   e_comp_object_util_del_list_append(lp->login_box, o2);
   e_util_icon_theme_set(o2, "list-remove");
   edje_object_part_swallow(o, "e.swallow.icon", o2);
   evas_object_show(o2);
   evas_object_show(o);
   edje_object_signal_callback_add(o, "e,action,click", "*", _pin_click, lp);
   evas_object_size_hint_min_set(o, 48 * e_scale, 48 * e_scale);
   evas_object_size_hint_max_set(o, 48 * e_scale, 48 * e_scale);
   E_FILL(o);
   elm_table_pack(table, o, 0, 3, 1, 1);

   /* login */
   o = edje_object_add(evas);
   e_comp_object_util_del_list_append(lp->login_box, o);
   e_theme_edje_object_set(o, "base/theme/desklock", "e/desklock/pin_button");
   edje_object_part_text_set(o, "e.text.label", "Login");
   o2 = e_icon_add(evas);
   e_comp_object_util_del_list_append(lp->login_box, o2);
   e_util_icon_theme_set(o2, "preferences-applications-screen-unlock");
   edje_object_part_swallow(o, "e.swallow.icon", o2);
   evas_object_show(o2);
   evas_object_show(o);
   edje_object_signal_callback_add(o, "e,action,click", "*", _pin_click, lp);
   evas_object_size_hint_min_set(o, 48 * e_scale, 48 * e_scale);
   evas_object_size_hint_max_set(o, 48 * e_scale, 48 * e_scale);
   E_FILL(o);
   elm_table_pack(table, o, 2, 3, 1, 1);

   evas_object_show(table);
   edje_object_part_swallow(lp->login_box, "e.swallow.buttons", table);
}

static void
_text_login_box_add(Lokker_Popup *lp)
{
   int mw, mh;
   E_Zone *zone, *current_zone;
   int total_zone_num;
   Evas *evas;

   zone = lp->zone;
   last_active_zone = current_zone = e_zone_current_get();
   total_zone_num = eina_list_count(e_comp->zones);
   if (total_zone_num > 1)
     {
        if ((e_config->desklock_login_box_zone == -2) && (zone != current_zone))
          return;
        if ((e_config->desklock_login_box_zone > -1) && (e_config->desklock_login_box_zone != (int)eina_list_count(edd->elock_wnd_list)))
          return;
     }

   evas = evas_object_evas_get(lp->bg_object);
   lp->login_box = edje_object_add(evas);
   evas_object_name_set(lp->login_box, "desklock->login_box");
   e_theme_edje_object_set(lp->login_box,
                           "base/theme/desklock",
                           "e/desklock/login_box");
   edje_object_part_text_set(lp->login_box, "e.text.title",
                             _("Please enter your unlock password"));
   if (evas_key_lock_is_set(evas_key_lock_get(evas), "Caps_Lock"))
        edje_object_part_text_set(lp->login_box, "e.text.hint",
                             _("Caps Lock is On"));
   else
        edje_object_part_text_set(lp->login_box, "e.text.hint",
                             "");
   edje_object_size_min_calc(lp->login_box, &mw, &mh);
   if (edje_object_part_exists(lp->bg_object, "e.swallow.login_box"))
     {
        evas_object_size_hint_min_set(lp->login_box, mw, mh);
        edje_object_part_swallow(lp->bg_object, "e.swallow.login_box", lp->login_box);
     }
   else
     {
        evas_object_resize(lp->login_box, mw, mh);
        e_comp_object_util_center_on(lp->login_box, lp->comp_object);
        evas_object_show(lp->login_box);
        evas_object_layer_set(lp->login_box, E_LAYER_DESKLOCK);
        evas_object_stack_above(lp->login_box, lp->comp_object);
     }
}

static void
_lokker_cb_show_done(void *data, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   Lokker_Popup *lp = data;

   e_desklock_zone_block_set(lp->zone, 1);
}

static void
_lokker_popup_add(E_Zone *zone)
{
   E_Zone *current_zone;
   Evas_Object *o = NULL;
   int total_zone_num;
   Lokker_Popup *lp;
   E_Config_Desklock_Background *cbg;
   Eina_Stringshare *bg;
   Evas *evas;
   int nocreate = 0;

   lp = E_NEW(Lokker_Popup, 1);
   cbg = eina_list_nth(e_config->desklock_backgrounds, zone->num);
   bg = cbg ? cbg->file : NULL;

   lp->zone = zone;
   evas = e_comp->evas;
   evas_event_freeze(evas);

   if ((!bg) || (!strcmp(bg, "theme_desklock_background")))
     {
        o = edje_object_add(evas);
        evas_object_data_set(o, "is_edje", o);
        e_theme_edje_object_set(o, "base/theme/desklock",
                                "e/desklock/background");
     }
   else if (!strcmp(bg, "theme_background"))
     {
        o = edje_object_add(evas);
        evas_object_data_set(o, "is_edje", o);
        e_theme_edje_object_set(o, "base/theme/backgrounds",
                                "e/desktop/background");
     }
   else
     {
        Eina_Stringshare *f;

        if (!strcmp(bg, "user_background"))
          f = e_desklock_user_wallpaper_get(zone);
        else
          f = bg;

        if (eina_str_has_extension(f, ".edj"))
          {
             o = edje_object_add(evas);
             evas_object_data_set(o, "is_edje", o);
             if (e_util_edje_collection_exists(f, "e/desklock/background"))
               {
                  edje_object_file_set(o, f, "e/desklock/background");
               }
             else
               {
                  if (!edje_object_file_set(o, f, "e/desktop/background"))
                    {
                       edje_object_file_set
                         (o, e_theme_edje_file_get("base/theme/desklock",
                                                   "e/desklock/background"),
                          "e/desklock/background");
                    }
               }
          }
        else if ((eina_str_has_extension(f, ".gif")) ||
                 (eina_str_has_extension(f, ".png")) ||
                 (eina_str_has_extension(f, ".jpg")) ||
                 (eina_str_has_extension(f, ".jpeg")) ||
                 (eina_str_has_extension(f, ".bmp")))
             {
                o = e_icon_add(evas);
                e_icon_file_key_set(o, f, NULL);
                e_icon_scale_size_set(o, 0);
                e_icon_fill_inside_set(o, 0);
             }
        else
          {
             o = e_video_add(evas, f, EINA_FALSE);
          }
     }
   lp->bg_object = o;
   evas_object_name_set(lp->bg_object, "desklock->bg_object");

   evas_object_move(lp->bg_object, zone->x, zone->y);
   evas_object_resize(lp->bg_object, zone->w, zone->h);
   evas_object_show(lp->bg_object);
     {
        const char *s = edje_object_data_get(lp->bg_object, "show_signal");
        lp->show_anim = s && (atoi(s) == 1);
        e_desklock_zone_block_set(zone, !lp->show_anim);
        if ((lp->show_anim) &&
            (evas_object_data_get(lp->bg_object, "is_edje")))
          edje_object_signal_callback_add(lp->bg_object,
                                          "e,action,show,done", "e",
                                          _lokker_cb_show_done, lp);
        s = edje_object_data_get(lp->bg_object, "hide_signal");
        lp->hide_anim = s && (atoi(s) == 1);
     }
   lp->comp_object = e_comp_object_util_add(lp->bg_object, 0);
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), "desklock.%d", zone->id);
        evas_object_name_set(lp->comp_object, buf);
     }
   evas_object_layer_set(lp->comp_object, E_LAYER_DESKLOCK);
   evas_object_clip_set(lp->comp_object, lp->zone->bg_clip_object);

   last_active_zone = current_zone = e_zone_current_get();
   total_zone_num = eina_list_count(e_comp->zones);
   if (total_zone_num > 1)
     {
        if ((e_config->desklock_login_box_zone == -2) && (zone != current_zone))
          nocreate = 1;
        else if ((e_config->desklock_login_box_zone > -1) && (e_config->desklock_login_box_zone != (int)eina_list_count(edd->elock_wnd_list)))
          nocreate = 1;
     }

   if (!nocreate)
     {
        switch (e_config->desklock_auth_method)
          {
           case E_DESKLOCK_AUTH_METHOD_SYSTEM:
           case E_DESKLOCK_AUTH_METHOD_PERSONAL:
             _text_login_box_add(lp);
             break;
           case E_DESKLOCK_AUTH_METHOD_PIN:
             _pin_box_add(lp);
             edje_object_part_swallow(lp->bg_object, "e.swallow.login_box", lp->login_box);

             E_LIST_HANDLER_APPEND(edd->handlers, ECORE_EVENT_MOUSE_BUTTON_DOWN, _pin_mouse_button_down, lp);
             E_LIST_HANDLER_APPEND(edd->handlers, ECORE_EVENT_MOUSE_BUTTON_UP, _pin_mouse_button_up, lp);
             break;
           default: break;
          }
        if (cbg)
          {
             const char *sig[] =
             {
                "e,state,logo,visible",
                "e,state,logo,hidden",
             };
             if (lp->bg_object)
               edje_object_signal_emit(lp->bg_object, sig[cbg->hide_logo], "e");
             if (lp->login_box)
               edje_object_signal_emit(lp->login_box, sig[cbg->hide_logo], "e");
          }
     }

   evas_event_thaw(evas);

   edd->elock_wnd_list = eina_list_append(edd->elock_wnd_list, lp);
}

static void
_lokker_cb_hide_done(void *data, Evas_Object *obj, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   Evas_Object *comp_object = evas_object_data_get(obj, "comp_object");
   edje_object_signal_callback_del(obj, "e,action,hide,done", "e",
                                   _lokker_cb_hide_done);
   evas_object_del(data);
   evas_object_del(obj);
   evas_object_hide(comp_object);
   evas_object_del(comp_object);
}

static void
_lokker_popup_free(Lokker_Popup *lp)
{
   if (!lp) return;

   e_desklock_zone_block_set(lp->zone, !lp->hide_anim);
   if (lp->hide_anim)
     {
        evas_object_data_set(lp->bg_object, "comp_object", lp->comp_object);
        evas_object_data_set(lp->bg_object, "login_box", lp->login_box);
        edje_object_signal_callback_add(lp->bg_object,
                                        "e,action,hide,done", "e",
                                        _lokker_cb_hide_done,
                                        lp->login_box);
        edje_object_signal_emit(lp->bg_object, "e,action,hide", "e");
        edje_object_signal_emit(lp->login_box, "e,action,hide", "e");
        lp->bg_object = NULL;
        lp->login_box = NULL;
     }
   else
     {
        evas_object_del(lp->bg_object);
        evas_object_del(lp->login_box);
        evas_object_hide(lp->comp_object);
        evas_object_del(lp->comp_object);
        lp->comp_object = NULL;
        lp->bg_object = NULL;
        lp->login_box = NULL;
     }
   free(lp);
}

static Eina_List *
_lokker_popup_find(E_Zone *zone)
{
   Eina_List *l;
   Lokker_Popup *lp;

   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     if (lp->zone == zone) return l;
   return NULL;
}

static Eina_Bool
_lokker_cb_mouse_move(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Lokker_Popup *lp;
   E_Zone *current_zone;
   Eina_List *l;

   current_zone = e_zone_current_get();

   if (current_zone == last_active_zone)
     return ECORE_CALLBACK_PASS_ON;

   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     {
        if (!lp) continue;

        if (lp->zone != current_zone)
          {
             if (lp->login_box) evas_object_hide(lp->login_box);
             continue;
          }
        if (lp->login_box)
          evas_object_show(lp->login_box);
        else
          _text_login_box_add(lp);
     }
   _text_passwd_update();
   last_active_zone = current_zone;
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_lokker_cb_exit(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev = event;

   if (ev->pid != _auth_child_pid) return ECORE_CALLBACK_PASS_ON;

   _auth_child_pid = -1;
   /* ok */
   if (ev->exited && (ev->exit_code == 0))
     {
        /* security - null out passwd string once we are done with it */
        _lokker_null();
        e_desklock_hide();
     }
   /* error */
   else if ((!ev->exited) || (ev->exit_code < 128))
     {
        /* security - null out passwd string once we are done with it */
        _lokker_null();
        e_desklock_hide();
        e_util_dialog_show(_("Authentication System Error"),
                           _("Authentication via PAM had errors setting up the<ps/>"
                             "authentication session. The error code was <hilight>%i</hilight>.<ps/>"
                             "This is bad and should not be happening. Please report this bug.")
                           , ev->exited ? ev->exit_code : ev->exit_signal);
     }
   /* failed auth */
   else
     {
        _lokker_state_set(LOKKER_STATE_INVALID);
        /* security - null out passwd string once we are done with it */
        _lokker_null();
     }
   E_FREE_FUNC(_auth_child_exit_handler, ecore_event_handler_del);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_lokker_cb_zone_add(void *data EINA_UNUSED,
                        int type EINA_UNUSED,
                        void *event)
{
   E_Event_Zone_Add *ev = event;
   if (!edd) return ECORE_CALLBACK_PASS_ON;
   if ((!edd->move_handler) && (e_config->desklock_login_box_zone == -2))
     edd->move_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _lokker_cb_mouse_move, NULL);
   if (!_lokker_popup_find(ev->zone)) _lokker_popup_add(ev->zone);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_lokker_cb_zone_del(void *data EINA_UNUSED,
                        int type EINA_UNUSED,
                        void *event)
{
   E_Event_Zone_Del *ev = event;
   Eina_List *l;
   if (!edd) return ECORE_CALLBACK_PASS_ON;
   if ((eina_list_count(e_comp->zones) == 1) && (e_config->desklock_login_box_zone == -2))
     edd->move_handler = ecore_event_handler_del(edd->move_handler);

   l = _lokker_popup_find(ev->zone);
   if (l)
     {
        _lokker_popup_free(l->data);
        edd->elock_wnd_list = eina_list_remove_list(edd->elock_wnd_list, l);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_lokker_cb_zone_move_resize(void *data EINA_UNUSED,
                                int type EINA_UNUSED,
                                void *event)
{
   Lokker_Popup *lp;
   Eina_List *l;
   E_Event_Zone_Move_Resize *ev = event;

   if (!edd) return ECORE_CALLBACK_PASS_ON;
   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     if (lp->zone == ev->zone)
       {
          evas_object_resize(lp->bg_object, ev->zone->w, ev->zone->h);
          e_comp_object_util_center_on(lp->login_box, lp->comp_object);
          break;
       }
   return ECORE_CALLBACK_PASS_ON;
}

static int
_desklock_auth(char *passwd)
{
   _lokker_state_set(LOKKER_STATE_CHECKING);
   _auth_child_pid = e_auth_begin(passwd);
   if (_auth_child_pid > 0)
     /* parent */
     _auth_child_exit_handler =
       ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _lokker_cb_exit, NULL);
   else
     _lokker_state_set(LOKKER_STATE_INVALID);
   return (_auth_child_pid > 0);
}

static void
_lokker_caps_hint_update(const char *msg)
{
   Eina_List *l;
   Lokker_Popup *lp;
   EINA_LIST_FOREACH(edd->elock_wnd_list, l, lp)
     {
        edje_object_part_text_set(lp->login_box, "e.text.hint", msg);
     }
}

static int
_lokker_check_auth(void)
{
   if (!edd) return 0;
   if (e_desklock_is_system())
     {
        int ret;

        ret = _desklock_auth(edd->passwd);
        // passwd off in child proc now - null out from parent
        _lokker_null();
        return ret;
     }
   else if (e_desklock_is_personal())
     {
        if ((e_config->desklock_passwd) && edd->passwd[0] &&
            (e_config->desklock_passwd == e_auth_hash_djb2(edd->passwd, strlen(edd->passwd))))
          {
             /* password ok */
             /* security - null out passwd string once we are done with it */
             _lokker_null();
             e_desklock_hide();
             return 1;
          }
     }
   else if (lokker_is_pin())
     {
        if (edd->passwd[0])
          {
             if (e_auth_hash_djb2(edd->passwd, strlen(edd->passwd)) ==
                 e_config->desklock_pin)
               {
                  _lokker_null();
                  e_desklock_hide();
                  return 1;
               }
          }
     }

   /* password is definitely wrong */
   _lokker_state_set(LOKKER_STATE_INVALID);
   _lokker_null();
   return 0;
}

EINTERN Eina_Bool
lokker_key_up(Ecore_Event_Key *ev)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_X) return ECORE_CALLBACK_DONE;
   if (!strcmp(ev->key, "Caps_Lock"))
     {
        if ((ev->modifiers & ECORE_EVENT_LOCK_CAPS) == ECORE_EVENT_LOCK_CAPS)
          _lokker_caps_hint_update(_("Caps Lock is On"));
        else
          _lokker_caps_hint_update("");
        return ECORE_CALLBACK_DONE;
     }
   return ECORE_CALLBACK_DONE;
}

EINTERN Eina_Bool
lokker_key_down(Ecore_Event_Key *ev)
{
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        if (!strcmp(ev->key, "Caps_Lock"))
          {
             if(ev->modifiers & ECORE_EVENT_LOCK_CAPS)
               _lokker_caps_hint_update("");
             else
               _lokker_caps_hint_update(_("Caps Lock is On"));
             return ECORE_CALLBACK_DONE;
          }
     }
   if (edd->state == LOKKER_STATE_CHECKING) return ECORE_CALLBACK_DONE;

   if (!strcmp(ev->key, "Escape"))
     {
        if (edd->selected)
          {
             _lokker_unselect();
             return ECORE_CALLBACK_DONE;
          }
     }
   else if (!strcmp(ev->key, "KP_Enter"))
     _lokker_check_auth();
   else if (!strcmp(ev->key, "Return"))
     _lokker_check_auth();
   else if (!strcmp(ev->key, "BackSpace"))
     {
        if (edd->selected)
          {
             _lokker_null();
             _lokker_unselect();
             return ECORE_CALLBACK_DONE;
          }
        _lokker_backspace();
     }
   else if (!strcmp(ev->key, "Delete"))
     {
        if (edd->selected)
          {
             _lokker_null();
             _lokker_unselect();
             return ECORE_CALLBACK_DONE;
          }
        _lokker_delete();
     }
   else if ((!strcmp(ev->key, "u") &&
             (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)))
     _lokker_null();
   else if ((!strcmp(ev->key, "a") &&
             (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)))
     _lokker_select();
   else
     {
        /* here we have to grab a password */
        if (ev->compose && (ev->string[0] >= 0x20) && (ev->string[0] != 0x7f))
          {
             if (lokker_is_pin())
               {
                  /* block non-digits */
                  const char *c;

                  for (c = ev->compose; c[0]; c++)
                    {
                       if (!isdigit(c[0])) return ECORE_CALLBACK_DONE;
                    }
               }
             if (edd->selected)
               {
                  _lokker_null();
                  _lokker_unselect();
               }
             if ((strlen(edd->passwd) < (PASSWD_LEN - strlen(ev->compose))))
               {
                  strcat(edd->passwd, ev->compose);
                  _text_passwd_update();
               }
          }
     }

   return ECORE_CALLBACK_DONE;
}

EINTERN Eina_Bool
lokker_lock(void)
{
   int total_zone_num = 0;

   if (edd) return EINA_TRUE;

   if (lokker_is_pin())
     {
        if (!e_config->desklock_pin)
          {
             e_configure_registry_call("screen/screen_lock", NULL, NULL);
             return EINA_FALSE;
          }
     }
   edd = E_NEW(Lokker_Data, 1);
   if (!edd) return EINA_FALSE;
#ifdef HAVE_MLOCK
   mlock(edd, sizeof(Lokker_Data));
#endif

   e_pointer_type_push(e_comp->pointer, e_comp->pointer, "default");
   E_LIST_FOREACH(e_comp->zones, _lokker_popup_add);
   total_zone_num = eina_list_count(e_comp->zones);

   /* handlers */
   E_LIST_HANDLER_APPEND(edd->handlers, E_EVENT_ZONE_ADD, _lokker_cb_zone_add, NULL);
   E_LIST_HANDLER_APPEND(edd->handlers, E_EVENT_ZONE_DEL, _lokker_cb_zone_del, NULL);
   E_LIST_HANDLER_APPEND(edd->handlers, E_EVENT_ZONE_MOVE_RESIZE, _lokker_cb_zone_move_resize, NULL);

   if ((total_zone_num > 1) && (e_config->desklock_login_box_zone == -2))
     edd->move_handler = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _lokker_cb_mouse_move, NULL);

   _text_passwd_update();
   return EINA_TRUE;
}

EINTERN void
lokker_unlock(void)
{
   E_FREE_LIST(edd->elock_wnd_list, _lokker_popup_free);
   e_pointer_type_pop(e_comp->pointer, e_comp->pointer, "default");
   e_pointer_type_pop(e_comp->pointer, NULL, NULL);
   E_FREE_LIST(edd->handlers, ecore_event_handler_del);
   if (edd->move_handler) ecore_event_handler_del(edd->move_handler);

#ifdef HAVE_MLOCK
   munlock(edd, sizeof(Lokker_Data));
#endif
   E_FREE(edd);
}
