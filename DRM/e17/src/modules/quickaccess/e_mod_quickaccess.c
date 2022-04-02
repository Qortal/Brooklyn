#include "e_mod_main.h"

EINTERN int _e_qa_log_dom = -1;
static E_Action *_e_qa_toggle = NULL;
static E_Action *_e_qa_add = NULL;
static E_Action *_e_qa_del = NULL;
static const char _e_qa_name[] = N_("Quickaccess");
static const char _lbl_toggle[] = N_("Toggle Visibility");
static const char _lbl_add[] = N_("Add Quickaccess For Current Window");
static const char _lbl_del[] = N_("Remove Quickaccess From Current Window");
const char *_act_toggle = NULL;
static const char _act_add[] = "qa_add";
static const char _act_del[] = "qa_del";
static E_Grab_Dialog *eg = NULL;
static Eina_List *_e_qa_border_hooks = NULL;
static Eina_List *_e_qa_event_handlers = NULL;

static E_Client_Menu_Hook *border_hook = NULL;

static Eina_Bool qa_running = EINA_FALSE;

static void _e_qa_bd_menu_add(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_qa_bd_menu_del(void *data, E_Menu *m, E_Menu_Item *mi);
static void _e_qa_entry_transient_convert(E_Quick_Access_Entry *entry);

static void e_qa_help(void);
static Eina_Bool _e_qa_help_timer_cb(void *data);
static Eina_Bool _e_qa_help_timer2_cb(void *data);
static void _e_qa_help_activate_hook(E_Quick_Access_Entry *entry);
/**
 * in priority order:
 *
 * @todo config (see e_mod_config.c)
 *
 * @todo custom border based on E_Quick_Access_Entry_Mode/E_Gadcon_Orient
 *
 * @todo show/hide effects:
 *        - fullscreen
 *        - centered
 *        - slide from top, bottom, left or right
 *
 * @todo match more than one, doing tabs (my idea is to do another
 *       tabbing module first, experiment with that, maybe use/reuse
 *       it here)
 */

static void
_e_qa_entry_dia_hide(void *data)
{
   E_Quick_Access_Entry *entry;
   entry = e_object_data_get(data);
   if (entry) entry->dia = NULL;
}

/* note: id must be stringshared! */
static E_Quick_Access_Entry *
_e_qa_entry_find(const char *id)
{
   E_Quick_Access_Entry *entry;
   const Eina_List *n;
   EINA_LIST_FOREACH(qa_config->transient_entries, n, entry)
     if (entry->id == id)
       return entry;
   EINA_LIST_FOREACH(qa_config->entries, n, entry)
     if (entry->id == id)
       return entry;
   return NULL;
}

static E_Quick_Access_Entry *
_e_qa_entry_find_exe(const Ecore_Exe *exe)
{
   E_Quick_Access_Entry *entry;
   const Eina_List *n;
   EINA_LIST_FOREACH(qa_config->transient_entries, n, entry)
     if (entry->exe == exe)
       return entry;
   EINA_LIST_FOREACH(qa_config->entries, n, entry)
     if (entry->exe == exe)
       return entry;
   return NULL;
}

static E_Quick_Access_Entry *
_e_qa_entry_find_border(const E_Client *ec)
{
   E_Quick_Access_Entry *entry;
   const Eina_List *n;
   EINA_LIST_FOREACH(qa_config->transient_entries, n, entry)
     if ((ec->pixmap && (entry->win == e_client_util_win_get(ec))) || (entry->client == ec))
       return entry;
   EINA_LIST_FOREACH(qa_config->entries, n, entry)
     if (entry->client == ec)
       return entry;
   return NULL;
}

static E_Quick_Access_Entry *
_e_qa_entry_find_match_stringshared(const char *name, const char *class, Eina_Bool nontrans)
{
   E_Quick_Access_Entry *entry;
   const Eina_List *n;

   if (!nontrans)
     {
        EINA_LIST_FOREACH(qa_config->transient_entries, n, entry)
          {
             if (entry->win) continue;
             if (entry->class != class) continue;
             /* no entry name matches all */
             if ((entry->name) && (entry->name != name)) continue;

             return entry;
          }
     }
   EINA_LIST_FOREACH(qa_config->entries, n, entry)
     {
        if (entry->win) continue;
        if (entry->class != class) continue;
        /* no entry name matches all */
        if ((entry->name) && (entry->name != name)) continue;

        return entry;
     }

   return NULL;
}

static E_Quick_Access_Entry *
_e_qa_entry_find_match(const E_Client *ec, Eina_Bool nontrans)
{
   const char *name, *class;
   E_Quick_Access_Entry *entry;

   name = ec->icccm.name;
   class = ec->icccm.class;
   entry = _e_qa_entry_find_match_stringshared(name, class, nontrans);

   return entry;
}

static Eina_Bool
_e_qa_event_exe_del_cb(void *data, int type EINA_UNUSED, Ecore_Exe_Event_Del *ev)
{
   E_Quick_Access_Entry *entry;

   if (!data) return ECORE_CALLBACK_RENEW;

   entry = _e_qa_entry_find_exe(ev->exe);
   if (!entry) return ECORE_CALLBACK_RENEW;
   entry->exe = NULL;  /* not waiting/running anymore */
   if (entry->exe_handler) ecore_event_handler_del(entry->exe_handler);
   entry->exe_handler = NULL;

   return ECORE_CALLBACK_RENEW;
}

static void
_e_qa_entry_border_props_restore(E_Quick_Access_Entry *entry EINA_UNUSED, E_Client *ec)
{
#undef SET
#define SET(X) \
   ec->X = 0

   SET(lock_user_iconify);
   SET(lock_client_iconify);
   SET(lock_user_sticky);
   SET(lock_client_sticky);
   SET(user_skip_winlist);
#undef SET
   e_client_unstick(ec);

   ec->netwm.state.skip_taskbar = 0;
   ec->netwm.state.skip_pager = 0;
   ec->changed = 1;
}

static void
_e_qa_border_activate(E_Quick_Access_Entry *entry)
{
   entry->config.hidden = 0;
   if (!entry->client) return;
   if (entry->client->iconic)
     {
        if (!entry->client->lock_user_iconify)
          e_client_uniconify(entry->client);
     }
   if (entry->client->shaded)
     {
        if (!entry->client->lock_user_shade)
          e_client_unshade(entry->client, entry->client->shade_dir);
     }
   else if (entry->client->desk && entry->config.jump)
     {
        if (!entry->client->sticky) e_desk_show(entry->client->desk);
     }
   if (!entry->client->lock_user_stacking)
     evas_object_raise(entry->client->frame);
   entry->client->hidden = 0;
   e_client_comp_hidden_set(entry->client, 0);
   evas_object_show(entry->client->frame);
   if (!entry->client->lock_focus_out)
     e_client_focus_set_with_pointer(entry->client);
}

static void
_e_qa_border_deactivate(E_Quick_Access_Entry *entry)
{
   Eina_Bool focused;
   if (entry->config.jump) return;
   entry->config.hidden = 1;
   if (!entry->client) return;
   entry->client->hidden = 1;
   focused = entry->client->focused;
   e_client_comp_hidden_set(entry->client, 1);
   evas_object_hide(entry->client->frame);
   if (focused && e_config->focus_revert_on_hide_or_close)
     e_desk_last_focused_focus(e_desk_current_get(entry->client->zone));
}

static void
_e_qa_entry_border_props_apply(E_Quick_Access_Entry *entry)
{
   if (!entry->client) return;

   if (entry->config.autohide && (!entry->client->focused))
     _e_qa_border_deactivate(entry);
#define SET(X) \
   entry->client->X = 1
   if (entry->config.jump)
     {
        entry->client->netwm.state.skip_taskbar = 0;
        entry->client->netwm.state.skip_pager = 0;
     }
   else
     {
        if (qa_config->skip_taskbar)
          SET(netwm.state.skip_taskbar);
        if (qa_config->skip_pager)
          SET(netwm.state.skip_pager);
        e_client_stick(entry->client);
     }
   //ec->e.state.centered = 1;
   SET(lock_user_iconify);
   SET(lock_client_iconify);
   SET(lock_user_sticky);
   SET(lock_client_sticky);
   if (qa_config->skip_window_list)
     SET(user_skip_winlist);
   EC_CHANGED(entry->client);
#undef SET
}

static void
_e_qa_entry_border_associate(E_Quick_Access_Entry *entry, E_Client *ec)
{
   if (entry->exe) entry->exe = NULL;  /* not waiting anymore */

   entry->client = ec;
   /* FIXME: doesn't work, causes window to flicker on associate
   if (entry->config.hidden)
     _e_qa_border_deactivate(entry);
   */
   _e_qa_entry_border_props_apply(entry);
}

static void
_e_qa_entry_relaunch_setup_continue(void *data, E_Dialog *dia)
{
   E_Quick_Access_Entry *entry = data;
   char buf[8192];
   int i;

   if (dia) e_object_del(E_OBJECT(dia));
   entry->dia = NULL;
   if (!entry->client->icccm.command.argv)
     {
        e_util_dialog_show(_("Quickaccess Error"), _("Could not determine command for starting this application!"));
        /* FIXME: e_entry_dialog? */
        return;
     }
   entry->config.relaunch = 1;
   buf[0] = 0;
   for (i = 0; i < entry->client->icccm.command.argc; i++)
     {
        if ((sizeof(buf) - strlen(buf)) <
            (strlen(entry->client->icccm.command.argv[i]) - 2))
          break;
        strcat(buf, entry->client->icccm.command.argv[i]);
        strcat(buf, " ");
     }
   entry->cmd = eina_stringshare_add(buf);
   if (entry->transient)
     _e_qa_entry_transient_convert(entry);
}

static void
_e_qa_entry_relaunch_setup_cancel(void *data, E_Dialog *dia)
{
   E_Quick_Access_Entry *entry = data;

   e_object_del(E_OBJECT(dia));
   entry->config.relaunch = 0;
}

static void
_e_qa_entry_relaunch_setup_help(void *data, E_Dialog *dia)
{
   E_Quick_Access_Entry *entry = data;
   char buf[8192];

   e_object_del(E_OBJECT(dia));
   entry->dia = NULL;
   entry->dia = dia = e_dialog_new(NULL, "E", "_quickaccess_cmd_help_dialog");

   snprintf(buf, sizeof(buf), _("The relaunch option is meant to be used<ps/>"
                               "with terminal applications to create a persistent<ps/>"
                               "terminal which reopens when closed, generally seen<ps/>"
                               "in quake-style drop-down terminals.<ps/>"
                               "Either the selected application is not a terminal<ps/>"
                               "or the cmdline flag for changing the terminal's window<ps/>"
                               "name is not known. Feel free to submit a bug report if this<ps/>"
                               "is a terminal which can change its window name.<ps/>"
                               "Alternatively, you can add a data.item to<ps/>"
                               "%s/e-module-quickaccess.edj<ps/>"
                               "Like so:<ps/>"
                               "data.item: \"%s\" \"--OPT\";"),
                               e_module_dir_get(qa_mod->module),
                               entry->class);

   e_dialog_title_set(dia, _("Quickaccess Help"));
   e_dialog_icon_set(dia, "enlightenment", 64);
   e_dialog_text_set(dia, buf);
   e_dialog_button_add(dia, _("Cancel"), NULL, _e_qa_entry_relaunch_setup_cancel, entry);
   elm_win_center(dia->win, 1, 1);
   e_dialog_show(dia);
   e_object_data_set(E_OBJECT(dia), entry);
   e_object_del_attach_func_set(E_OBJECT(dia), _e_qa_entry_dia_hide);
}

static void
_e_qa_entry_relaunch_setup(E_Quick_Access_Entry *entry)
{
   char *opt;
   const char *name;
   int i;
   char buf[4096];
   Eina_List *l;
   E_Quick_Access_Entry *e;

   if (entry->dia)
     {
        elm_win_raise(entry->dia->win);
        return;
     }
   if ((!entry->class) || (!entry->name))
     {
        e_util_dialog_show(_("Quickaccess Error"), _("Cannot set relaunch for window without name and class!"));
        entry->config.relaunch = 0;
        return;
     }
   if (!strcmp(entry->name, "E"))
     {
        /* can't set relaunch for internal E dialogs; safety #2 */
        e_util_dialog_show(_("Quickaccess Error"), _("Cannot set relaunch for internal E dialog!"));
        entry->config.relaunch = 0;
        return;
     }
   opt = e_qa_db_class_lookup(entry->class);
   if ((!opt) || (!opt[0]))
     {
        E_Dialog *dia;

        free(opt);
        if (qa_config->dont_bug_me)
          {
             _e_qa_entry_relaunch_setup_continue(entry, NULL);
             return;
          }
        entry->dia = dia = e_dialog_new(NULL, "E", "_quickaccess_cmd_dialog");

        snprintf(buf, sizeof(buf),
                 _("The selected window created with name:<ps/>%s<ps/>"
                   "and class:<ps/>%s<ps/>"
                   "could not be found in the Quickaccess app database<ps/>"
                   "This means that either the app is unknown to us<ps/>"
                   "or it is not intended for use with this option.<ps/>"
                   "Please choose an action to take:"),
                 entry->name, entry->class);

        e_dialog_title_set(dia, _("Quickaccess Error"));
        e_dialog_icon_set(dia, "enlightenment", 64);
        e_dialog_text_set(dia, buf);
        e_dialog_button_add(dia, _("Continue"), NULL, _e_qa_entry_relaunch_setup_continue, entry);
        e_dialog_button_add(dia, _("More Help"), NULL, _e_qa_entry_relaunch_setup_help, entry);
        e_dialog_button_add(dia, _("Cancel"), NULL, _e_qa_entry_relaunch_setup_cancel, entry);
        elm_win_center(dia->win, 1, 1);
        e_dialog_show(dia);
        e_object_data_set(E_OBJECT(dia), entry);
        e_object_del_attach_func_set(E_OBJECT(dia), _e_qa_entry_dia_hide);
        entry->config.relaunch = 0;
        return;
     }
   if (!entry->client->icccm.command.argv)
     {
        free(opt);
        e_util_dialog_show(_("Quickaccess Error"), _("Could not determine command for starting this application!"));
        /* FIXME: e_entry_dialog? */
        return;
     }

   buf[0] = 0;
   for (i = 0; i < entry->client->icccm.command.argc; i++)
     {
        if ((sizeof(buf) - strlen(buf)) <
            (strlen(entry->client->icccm.command.argv[i]) - 2))
          break;
        strcat(buf, entry->client->icccm.command.argv[i]);
        strcat(buf, " ");
     }
   name = entry->name;
   entry->name = eina_stringshare_printf("e-%s-%u", entry->name, entry->client->netwm.pid);
   while (i)
     {
        i = 0;
        EINA_LIST_FOREACH(qa_config->entries, l, e)
          {
             if (e == entry) continue;
             if (e->class != entry->class) continue;
             if ((e->name == entry->name) || (e->id == entry->name))
               {
                  eina_stringshare_del(entry->name);
                  entry->name = eina_stringshare_printf("e-%s-%u%d", entry->name, entry->client->netwm.pid, i);
                  i++;
                  break;
               }
          }
     }
   eina_stringshare_del(name);
   entry->cmd = eina_stringshare_printf("%s %s \"%s\"", buf, opt, entry->name);
   entry->config.relaunch = 1;
   if (entry->transient)
     _e_qa_entry_transient_convert(entry);
   free(opt);
}

static void
_e_qa_border_new(E_Quick_Access_Entry *entry)
{
   E_Exec_Instance *ei;

   if ((!entry->cmd) || (!entry->config.relaunch)) return;
   if (entry->exe)
     {
        INF("already waiting '%s' to start for '%s' (name=%s, class=%s), "
            "run request ignored.", entry->cmd, entry->id, entry->name, entry->class);
        return;
     }

   INF("start quick access '%s' (name=%s, class=%s), "
       "run command '%s'", entry->id, entry->name, entry->class, entry->cmd);

   ei = e_exec(NULL, NULL, entry->cmd, NULL, NULL);
   if ((!ei) || (!ei->exe))
     {
        ERR("could not execute '%s'", entry->cmd);
        return;
     }

   entry->exe = ei->exe;
   entry->exe_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, (Ecore_Event_Handler_Cb)_e_qa_event_exe_del_cb, entry);
}

static void
_e_qa_del_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   _e_qa_bd_menu_del(_e_qa_entry_find_border(e_client_focused_get()), NULL, NULL);
}

static void
_e_qa_add_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   _e_qa_bd_menu_del(e_client_focused_get(), NULL, NULL);
}

static void
_e_qa_toggle_cb(E_Object *obj EINA_UNUSED, const char *params)
{
   E_Quick_Access_Entry *entry;

   if (!params)
     {
        ERR("%s got params == NULL", _act_toggle);
        return;
     }
   /* params is stringshared according to e_bindings.c */
   DBG("%s %s (stringshared=%p)", _act_toggle, params, params);

   entry = _e_qa_entry_find(params);
   if (!entry)
     {
        e_util_dialog_show(_("Quickaccess Error"), _("The requested Quickaccess entry does not exist!"));
        return;
     }

   if (entry->client)
     {
        if (entry->help_watch)
          _e_qa_help_activate_hook(entry);
        if ((!entry->config.jump) && evas_object_visible_get(entry->client->frame) && ((entry->client->icccm.accepts_focus && entry->client->focused) || entry->config.hide_when_behind))
          {
             _e_qa_border_deactivate(entry);
             return;
          }

        DBG("activate border for identifier '%s' (name=%s, class=%s).",
            entry->id, entry->name, entry->class);
        _e_qa_border_activate(entry);
     }
   else
     {
        DBG("no border known for identifier '%s' (name=%s, class=%s).",
            entry->id, entry->name, entry->class);
        _e_qa_border_new(entry);
     }
}

static Eina_Bool
_e_qa_client_is_valid(const E_Client *ec)
{
   if (e_client_util_ignored_get(ec)) return EINA_FALSE;
   if (ec->internal) return EINA_FALSE;
   if ((!ec->icccm.class) || (!ec->icccm.class[0])) return EINA_FALSE;
   if ((!ec->icccm.name) || (!ec->icccm.name[0])) return EINA_FALSE;
   return EINA_TRUE;
}

static void
_e_qa_border_eval_pre_post_fetch_cb(void *data EINA_UNUSED, E_Client *ec)
{
   E_Quick_Access_Entry *entry;

   if ((!ec->new_client) || (!_e_qa_client_is_valid(ec))) return;

   entry = _e_qa_entry_find_match(ec, 0);
   if (!entry) return;
   DBG("border=%p matches entry %s", ec, entry->id);
   _e_qa_entry_border_associate(entry, ec);
}

static Eina_Bool
_e_qa_event_border_focus_out_cb(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Client *ev)
{
   E_Quick_Access_Entry *entry;

   entry = _e_qa_entry_find_border(ev->ec);
   if (entry && entry->config.autohide) _e_qa_border_deactivate(entry);
   return ECORE_CALLBACK_RENEW;
}

static void
_e_qa_begin(void)
{
   Eina_List *l, *ll;
   E_Quick_Access_Entry *entry;
   unsigned int count;
   E_Client *ec;
   /* assume that by now, e has successfully placed all windows */
   count = eina_list_count(qa_config->transient_entries);
   EINA_LIST_FOREACH_SAFE(qa_config->transient_entries, l, ll, entry)
     {
        if (entry->client) continue;
        entry->client = e_pixmap_find_client(E_PIXMAP_TYPE_X, entry->win);
        if (entry->client)
          {
             DBG("qa window for %u:transient:%s still exists; restoring", entry->win, entry->id);
             _e_qa_entry_border_associate(entry, entry->client);
             continue;
          }
        DBG("qa window for %u:transient:%s no longer exists; deleting", entry->win, entry->id);
        e_qa_entry_free(entry);
     }
   if (count != eina_list_count(qa_config->transient_entries)) e_bindings_reset();
   qa_running = EINA_TRUE;
   count = 0;
   EINA_LIST_FOREACH(qa_config->entries, l, entry)
     {
        if (entry->config.relaunch && (!entry->client))
          {
             DBG("qa window for relaunch entry %s not present, starting", entry->id);
             _e_qa_border_new(entry);
          }
        if (entry->client) continue;
        count++;
     }
   if (count)
     {
        /* some non-transient entries exist without assigned borders
         * try assigning from existing borders
         */
        EINA_LIST_FOREACH(e_comp->clients, l, ec)
          {
             if (e_client_util_ignored_get(ec)) continue;
             entry = _e_qa_entry_find_match(ec, 1);
             if ((!entry) || entry->client) continue;
             DBG("border=%p matches entry %s", ec, entry->id);
             _e_qa_entry_border_associate(entry, ec);
             count--;
             if (!count) break;
          }
     }
}

static Eina_Bool
_e_qa_event_border_remove_cb(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Client *ev)
{
   E_Quick_Access_Entry *entry;

   entry = _e_qa_entry_find_border(ev->ec);
   if (!entry) return ECORE_CALLBACK_RENEW;
   if (entry->transient)
     {
        DBG("closed transient qa border: deleting keybind and entry");
        e_qa_entry_free(entry);
        return ECORE_CALLBACK_RENEW;
     }
   else if ((!stopping) && entry->config.relaunch) _e_qa_border_new(entry);
   entry->client = NULL;

   return ECORE_CALLBACK_RENEW;
}

static void
_e_qa_entry_transient_convert(E_Quick_Access_Entry *entry)
{
   e_qa_config_entry_transient_convert(entry);
   if (entry->transient)
     {
        entry->transient = EINA_FALSE;
        entry->win = 0;
        eina_list_move(&qa_config->entries, &qa_config->transient_entries, entry);
        return;
     }
   entry->transient = EINA_TRUE;
   entry->win = e_client_util_win_get(entry->client);
   eina_list_move(&qa_config->transient_entries, &qa_config->entries, entry);
   eina_stringshare_replace(&entry->cmd, NULL);
   entry->config.relaunch = 0;
}

static E_Quick_Access_Entry *
_e_qa_entry_transient_new(E_Client *ec)
{
   E_Quick_Access_Entry *entry;
   char buf[8192];

   snprintf(buf, sizeof(buf), "%s:%u:%s", ec->icccm.name ?: "", (unsigned int)e_client_util_win_get(ec), ec->icccm.class ?: "");

   entry = e_qa_entry_new(buf, EINA_TRUE);
   entry->win = e_client_util_win_get(ec);
   entry->name = eina_stringshare_ref(ec->icccm.name);
   entry->class = eina_stringshare_ref(ec->icccm.class);
   _e_qa_entry_border_associate(entry, ec);
   qa_config->transient_entries = eina_list_append(qa_config->transient_entries, entry);
   e_config_save_queue();
   return entry;
}

static Eina_Bool
_grab_key_down_cb(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Key *ev = event;
   E_Client *ec = data;
   E_Config_Binding_Key *bi;
   E_Quick_Access_Entry *entry;
   unsigned int mod = E_BINDING_MODIFIER_NONE;

   if (!strcmp(ev->key, "Control_L") || !strcmp(ev->key, "Control_R") ||
       !strcmp(ev->key, "Shift_L") || !strcmp(ev->key, "Shift_R") ||
       !strcmp(ev->key, "Alt_L") || !strcmp(ev->key, "Alt_R") ||
       !strcmp(ev->key, "Super_L") || !strcmp(ev->key, "Super_R"))
     return ECORE_CALLBACK_RENEW;

   if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
     mod |= E_BINDING_MODIFIER_SHIFT;
   if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)
     mod |= E_BINDING_MODIFIER_CTRL;
   if (ev->modifiers & ECORE_EVENT_MODIFIER_ALT)
     mod |= E_BINDING_MODIFIER_ALT;
   if (ev->modifiers & ECORE_EVENT_MODIFIER_WIN)
     mod |= E_BINDING_MODIFIER_WIN;

   if (e_util_binding_match(NULL, ev, NULL, NULL))
     {
        e_util_dialog_show(_("Keybind Error"), _("The keybinding you have entered is already in use!"));
        e_object_del(E_OBJECT(eg));
        return ECORE_CALLBACK_RENEW;
     }
   entry = _e_qa_entry_transient_new(ec);

   bi = E_NEW(E_Config_Binding_Key, 1);

   bi->context = E_BINDING_CONTEXT_ANY;
   bi->modifiers = mod;
   bi->key = eina_stringshare_add(ev->key);
   bi->action = eina_stringshare_ref(_act_toggle);
   bi->params = eina_stringshare_ref(entry->id);

   e_comp_canvas_keys_ungrab();
   e_bindings->key_bindings = eina_list_append(e_bindings->key_bindings, bi);
   e_bindings_key_add(bi->context, bi->key, bi->modifiers, bi->any_mod, bi->action, bi->params);
   e_comp_canvas_keys_grab();
   e_config_save_queue();
   e_object_del(E_OBJECT(eg));
   return ECORE_CALLBACK_RENEW;
}

static void
_grab_wnd_hide(void *data EINA_UNUSED)
{
   eg = NULL;
}

static void
_e_qa_bd_menu_transient(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Quick_Access_Entry *entry = data;
   _e_qa_entry_transient_convert(entry);
}

static void
_e_qa_bd_menu_relaunch(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Quick_Access_Entry *entry = data;

   entry->config.relaunch = !entry->config.relaunch;
   if (!entry->config.relaunch) return;
   _e_qa_entry_relaunch_setup(entry);
   if (!entry->config.relaunch) return;
   /* a relaunchable entry cannot be transient */
   if (entry->transient) _e_qa_entry_transient_convert(entry);
}

static void
_e_qa_bd_menu_jump(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Quick_Access_Entry *entry = data;

   entry->config.jump = !entry->config.jump;
   if (entry->config.jump)
     {
        entry->config.autohide = entry->config.hide_when_behind = 0;
        _e_qa_entry_border_props_restore(entry, entry->client);
     }
   else
     _e_qa_entry_border_props_apply(entry);
}

static void
_e_qa_bd_menu_hideraise(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Quick_Access_Entry *entry = data;

   entry->config.hide_when_behind = !entry->config.hide_when_behind;
}

static void
_e_qa_bd_menu_autohide(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Quick_Access_Entry *entry = data;

   entry->config.autohide = !entry->config.autohide;
   _e_qa_entry_border_props_apply(entry);
}

static void
_e_qa_bd_menu_help(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   e_qa_help();
}

static void
_e_qa_bd_menu_del(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Quick_Access_Entry *entry = data;

   if (!entry) return;

   e_qa_entry_free(entry);
}

static void
_e_qa_bd_menu_config(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   e_configure_registry_call("launcher/quickaccess", NULL, NULL);
}

static void
_e_qa_bd_menu_add(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Client *ec = data;
   if (!ec) return;
   if (eg) return;
   eg = e_grab_dialog_show(NULL, EINA_FALSE, _grab_key_down_cb, NULL, NULL, ec);
   e_object_data_set(E_OBJECT(eg), ec);
   e_object_del_attach_func_set(E_OBJECT(eg), _grab_wnd_hide);
}

static void
_e_qa_bd_menu_free(void *data EINA_UNUSED)
{
   qa_mod->menu = NULL;
}

static void
_e_qa_bd_menu_pre(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi)
{
   E_Quick_Access_Entry *entry = data;
   E_Menu *subm;

   qa_mod->menu = subm = e_menu_new();
   e_menu_title_set(subm, entry->class);
   e_object_data_set(E_OBJECT(subm), entry);
   e_menu_item_submenu_set(mi, subm);
   e_object_unref(E_OBJECT(subm));
   e_object_free_attach_func_set(E_OBJECT(subm), _e_qa_bd_menu_free);

   if (!entry->config.jump)
     {
        mi = e_menu_item_new(subm);
        e_menu_item_check_set(mi, 1);
        e_menu_item_toggle_set(mi, entry->config.autohide);
        e_menu_item_label_set(mi, _("Autohide"));
        e_menu_item_callback_set(mi, _e_qa_bd_menu_autohide, entry);

        mi = e_menu_item_new(subm);
        e_menu_item_check_set(mi, 1);
        e_menu_item_toggle_set(mi, entry->config.hide_when_behind);
        e_menu_item_label_set(mi, _("Hide Instead Of Raise"));
        e_menu_item_callback_set(mi, _e_qa_bd_menu_hideraise, entry);
     }

   mi = e_menu_item_new(subm);
   e_menu_item_check_set(mi, 1);
   e_menu_item_toggle_set(mi, entry->config.jump);
   e_menu_item_label_set(mi, _("Jump Mode"));
   e_menu_item_callback_set(mi, _e_qa_bd_menu_jump, entry);

   /* can't set relaunch for internal E dialogs; safety #1 */
   if (entry->name && strcmp(entry->name, "E"))
     {
        mi = e_menu_item_new(subm);
        e_menu_item_check_set(mi, 1);
        e_menu_item_toggle_set(mi, entry->config.relaunch);
        e_menu_item_label_set(mi, _("Relaunch When Closed"));
        e_menu_item_callback_set(mi, _e_qa_bd_menu_relaunch, entry);
     }

   mi = e_menu_item_new(subm);
   e_menu_item_check_set(mi, 1);
   e_menu_item_toggle_set(mi, entry->transient);
   e_menu_item_label_set(mi, _("Transient"));
   e_menu_item_callback_set(mi, _e_qa_bd_menu_transient, entry);

   mi = e_menu_item_new(subm);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(subm);
   e_menu_item_label_set(mi, _("Remove Quickaccess"));
   e_menu_item_callback_set(mi, _e_qa_bd_menu_del, entry);

   mi = e_menu_item_new(subm);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(subm);
   e_menu_item_label_set(mi, _("Quickaccess Help"));
   e_menu_item_callback_set(mi, _e_qa_bd_menu_help, NULL);
}

static void
_e_qa_bd_menu_hook(void *d EINA_UNUSED, E_Client *ec)
{
   E_Menu_Item *mi;
   E_Menu *m;
   E_Quick_Access_Entry *entry;
   char buf[PATH_MAX];

   if (!ec->border_menu) return;
   m = ec->border_menu;

   /* position menu item just before first separator */
   mi = m->items->next->data;
   mi = e_menu_item_new_relative(m, mi);
   entry = _e_qa_entry_find_border(ec);
   if (entry)
     {
        e_menu_item_label_set(mi, _("Quickaccess..."));
        e_menu_item_submenu_pre_callback_set(mi, _e_qa_bd_menu_pre, entry);
        e_menu_item_callback_set(mi, _e_qa_bd_menu_config, NULL);
     }
   else
     {
        e_menu_item_label_set(mi, _("Add Quickaccess"));
        e_menu_item_callback_set(mi, _e_qa_bd_menu_add, ec);
     }
   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
   e_menu_item_icon_edje_set(mi, buf, "icon");
}

static void
_e_qa_entry_config_apply(E_Quick_Access_Entry *entry)
{
#define SET(X) entry->config.X = qa_config->X
        SET(autohide);
        SET(hide_when_behind);
#undef SET
}

static Eina_Bool
_e_qa_help_timeout(void *data EINA_UNUSED)
{
   if (qa_mod->help_dia) e_object_del(qa_mod->help_dia);
   if (qa_mod->demo_dia)
     {
        E_Quick_Access_Entry *entry;
        entry = _e_qa_entry_find_border(e_win_client_get(qa_mod->demo_dia->win));
        e_qa_entry_free(entry);
        e_object_del(E_OBJECT(qa_mod->demo_dia));
     }
   if (qa_mod->help_timer) ecore_timer_del(qa_mod->help_timer);
   if (qa_mod->help_timeout) ecore_timer_del(qa_mod->help_timeout);
   qa_mod->demo_state = 0;
   qa_mod->help_timeout = qa_mod->help_timer = NULL;
   return EINA_FALSE;
}

static void
_e_qa_dia_end_del(void *data EINA_UNUSED)
{
   qa_mod->help_dia = NULL;
   _e_qa_help_timeout(NULL);
   qa_config->first_run = EINA_TRUE;
}

static void
_e_qa_dia_del(void *data EINA_UNUSED)
{
   qa_mod->help_dia = NULL;
   if (qa_mod->help_timeout)
     ecore_timer_loop_reset(qa_mod->help_timeout);
   else
     qa_mod->help_timeout = ecore_timer_loop_add(20.0, _e_qa_help_timeout, NULL);
}

static void
_e_qa_demo_dia_del(void *data EINA_UNUSED)
{
   qa_mod->demo_dia = NULL;
   _e_qa_help_timeout(NULL);
}

static void
_e_qa_help_cancel(void *data EINA_UNUSED)
{
   qa_config->first_run = EINA_TRUE;
   _e_qa_help_timeout(NULL);
}

static void
e_qa_help(void)
{
   char buf[PATH_MAX];

   if (qa_mod->help_dia) return;
   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
   qa_mod->help_dia = (E_Object*)e_util_dialog_internal(_("Quickaccess Help"),
                                                        _("The options found in the Quickaccess menu are as follows:<ps/>"
                                                          "<hilight>Autohide</hilight> - hide the window whenever it loses focus<ps/>"
                                                          "<hilight>Hide Instead of Raise</hilight> - Hide window when activated without focus<ps/>"
                                                          "<hilight>Jump Mode</hilight> - Switch to window's desk and raise instead of showing/hiding<ps/>"
                                                          "<hilight>Relaunch When Closed</hilight> - Run the entry's command again when its window exits<ps/>"
                                                          "<hilight>Transient</hilight> - Remember only this instance of the window (not permanent)"));
   if (qa_mod->help_timeout) ecore_timer_freeze(qa_mod->help_timeout);
   e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_end_del);
}

static void
_e_qa_help6(void *data EINA_UNUSED)
{
   if (qa_mod->help_dia)
     {
        ecore_job_add(_e_qa_help6, NULL);
        return;
     }
   e_qa_help();
}

static void
_e_qa_help5(void *data EINA_UNUSED)
{
   char buf[PATH_MAX];

   if (_e_qa_entry_find_border(e_win_client_get(qa_mod->demo_dia->win)))
     {
        qa_mod->help_timer = ecore_timer_loop_add(1, _e_qa_help_timer_cb, NULL);
        return;
     }

   if (qa_mod->help_dia)
     {
        ecore_job_add(_e_qa_help5, NULL);
        return;
     }
   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
   qa_mod->help_dia = (E_Object*)e_confirm_dialog_show(_("Quickaccess Help"), buf,
                                                        _("You deleted it on your own, you rascal!<ps/>"
                                                          "Way to go!"),
                                                        _("Continue"), _("Stop"),
                                                        _e_qa_help6, _e_qa_help_cancel, NULL, NULL, NULL, NULL);
   e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);
}

static void
_e_qa_help_activate_hook(E_Quick_Access_Entry *entry)
{
   char buf[PATH_MAX];

   switch (qa_mod->demo_state++)
     {
      case 0:
        {
           char *txt;

           if (entry->config.hidden)
             txt =  _("Great! Activate the Quickaccess entry again to show it!");
           else
             txt = _("Great! Activate the Quickaccess entry again to hide it!");
           snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
           if (qa_mod->help_dia)
             {
                e_dialog_text_set((E_Dialog*)qa_mod->help_dia, txt);
                break;
             }
           qa_mod->help_dia = (E_Object*)e_util_dialog_internal(_("Quickaccess Help"), txt);
           e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);
           break;
        }
      case 1:
        e_object_del(qa_mod->help_dia);
        ecore_job_add((Ecore_Cb)_e_qa_help_activate_hook, entry);
        break;
      default:
        snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
        if (entry->config.hidden)
          _e_qa_border_activate(_e_qa_entry_find_border(e_win_client_get(qa_mod->demo_dia->win)));
        qa_mod->help_dia = (E_Object*)e_confirm_dialog_show(_("Quickaccess Help"), buf,
                                                             _("Well done.<ps/>"
                                                               "Now to delete the entry we just made..."),
                                                             _("Continue"), _("Stop"),
                                                             _e_qa_help5, _e_qa_help_cancel, NULL, NULL, NULL, NULL);
        e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);
        qa_mod->demo_state = 0;
     }
}

static void
_e_qa_help4(void *data EINA_UNUSED)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
   qa_mod->help_dia = (E_Object*)e_util_dialog_internal(_("Quickaccess Help"),
                                                         _("The demo dialog has been bound to the keys you pressed.<ps/>"
                                                           "Try pressing the same keys!"));
   e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);
}

static void
_e_qa_help_qa_added_cb(void *data EINA_UNUSED)
{
   E_Quick_Access_Entry *entry;

   ecore_timer_thaw(qa_mod->help_timeout);
   if ((!qa_mod->demo_dia) || (!_e_qa_entry_find_border(e_win_client_get(qa_mod->demo_dia->win))))
     {
        _e_qa_help_timeout(NULL);
        return;
     }
   entry = eina_list_last_data_get(qa_config->transient_entries);
   entry->help_watch = EINA_TRUE;
   ecore_job_add(_e_qa_help4, NULL);
   e_object_del(qa_mod->help_dia);
}

static void
_e_qa_help_bd_menu_del(void *data EINA_UNUSED)
{
   if (qa_mod->help_timer) ecore_timer_del(qa_mod->help_timer);
   qa_mod->demo_state = 0;
   qa_mod->help_timer = NULL;
   if (eg)
     {
        e_object_free_attach_func_set(E_OBJECT(eg), _e_qa_help_qa_added_cb);
        return;
     }
   _e_qa_help_timeout(NULL);
}

static void
_e_qa_help_bd_menu2_del(void *data EINA_UNUSED)
{
   if (qa_mod->help_timer) ecore_timer_del(qa_mod->help_timer);
   qa_mod->demo_state = 0;
   qa_mod->help_timer = NULL;
   if (!qa_config->transient_entries) return;
   _e_qa_help_timeout(NULL);
}

static Eina_Bool
_e_qa_help_timer_helper(void)
{
   E_Client *ec;
   E_Menu_Item *mi;
   Eina_List *items;

   ec = e_win_client_get(qa_mod->demo_dia->win);
   ecore_timer_interval_set(qa_mod->help_timer, 0.2);
   mi = e_menu_item_active_get();
   if (qa_mod->menu)
     {
        if (mi && ((mi->cb.func == _e_qa_bd_menu_del)))
          {
             e_menu_active_item_activate();
             qa_mod->demo_state = 0;
             qa_mod->help_timer = NULL;
             return EINA_FALSE;
          }
        if (mi && (qa_mod->demo_state != 1) && (!mi->menu->parent_item) && (mi->submenu_pre_cb.func == _e_qa_bd_menu_pre))
          {
             qa_mod->demo_state = 0;
             qa_mod->help_timer = NULL;
             if (mi->menu != qa_mod->menu)
               qa_mod->help_timer = ecore_timer_loop_add(0.2, _e_qa_help_timer2_cb, NULL);
             return EINA_FALSE;
          }
        items = qa_mod->menu->items;
     }
   else
     {
        if (mi && (mi->cb.func == _e_qa_bd_menu_add))
          {
             e_menu_active_item_activate();
             qa_mod->demo_state = 0;
             qa_mod->help_timer = NULL;
             return EINA_FALSE;
          }
        items = ec->border_menu->items;
     }
   do
     {
        mi = eina_list_nth(items, qa_mod->demo_state - 1);
        if (mi)
          {
             if (mi->separator)
               qa_mod->demo_state++;
             else
               e_menu_item_active_set(mi, 1);
          }
        else
          /* someone's messing with the menu. joke's on them, we can dance all day */
          qa_mod->demo_state = 0;
     } while (mi && mi->separator);
   return EINA_TRUE;
}

static Eina_Bool
_e_qa_help_timer2_cb(void *data EINA_UNUSED)
{
   E_Client *ec;

   if ((!qa_mod->demo_dia) || (!qa_mod->demo_dia->win) || (!e_win_client_get(qa_mod->demo_dia->win)))
     /* FIXME */
     return EINA_TRUE;

   ec = e_win_client_get(qa_mod->demo_dia->win);
   switch (qa_mod->demo_state)
     {
      case 0:
        e_object_free_attach_func_set(E_OBJECT(ec->border_menu), _e_qa_help_bd_menu2_del);
        break;
      default:
        if (_e_qa_help_timer_helper()) break;
        e_qa_help();
        return EINA_FALSE;
     }
   qa_mod->demo_state++;
   return EINA_TRUE;
}

static Eina_Bool
_e_qa_help_timer_cb(void *data EINA_UNUSED)
{
   E_Client *ec;

   if ((!qa_mod->demo_dia) || (!qa_mod->demo_dia->win) || (!e_win_client_get(qa_mod->demo_dia->win)))
     /* wait longer */
     return EINA_TRUE;

   ec = e_win_client_get(qa_mod->demo_dia->win);
   switch (qa_mod->demo_state)
     {
      case 0:
        e_int_client_menu_show(ec, ec->x + ec->w * .5, ec->y + 5, 0, 0);
        ecore_timer_interval_set(qa_mod->help_timer, 0.8);
        e_object_free_attach_func_set(E_OBJECT(ec->border_menu), _e_qa_help_bd_menu_del);
        break;
      default:
        if (!_e_qa_help_timer_helper()) return EINA_FALSE;
     }
   qa_mod->demo_state++;
   return EINA_TRUE;
}

static void
_e_qa_help3(void *data EINA_UNUSED)
{
   char buf[PATH_MAX];
   E_Dialog *dia;

   if (qa_mod->help_dia)
     {
        ecore_job_add(_e_qa_help3, NULL);
        return;
     }
   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));

   qa_mod->help_dia = (E_Object*)e_util_dialog_internal(_("Quickaccess Help"),
                                                        _("The newly displayed window will activate<ps/>"
                                                          "the Quickaccess binding sequence."));
   e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);

   qa_mod->demo_dia = dia = e_dialog_normal_win_new(NULL, "E", "_qa_demo_dia");
   e_dialog_border_icon_set(dia, buf);
   e_dialog_icon_set(dia, buf, 128);
   e_dialog_title_set(dia, _("Quickaccess Demo"));
   e_dialog_text_set(dia, _("This is a demo dialog used in the Quickaccess tutorial"));
   e_dialog_show(dia);

   qa_mod->help_timer = ecore_timer_loop_add(1, _e_qa_help_timer_cb, NULL);
   ecore_timer_loop_reset(qa_mod->help_timeout);
   ecore_timer_freeze(qa_mod->help_timeout);

   e_object_free_attach_func_set(E_OBJECT(qa_mod->demo_dia), _e_qa_demo_dia_del);
}

static void
_e_qa_help2(void *data EINA_UNUSED)
{
   char buf[PATH_MAX];

   if (qa_mod->help_dia)
     {
        ecore_job_add(_e_qa_help2, NULL);
        return;
     }
   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
   qa_mod->help_dia = (E_Object*)e_confirm_dialog_show(_("Quickaccess Help"), buf,
                                                        _("Quickaccess entries can be created from<ps/>"
                                                          "the border menu of any window.<ps/>"
                                                          "Click Continue to see a demonstration."),
                                                        _("Continue"), _("Stop"),
                                                        _e_qa_help3, _e_qa_help_cancel, NULL, NULL, NULL, NULL);
   e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);
}

static void
_e_qa_help(void *data)
{
   char buf[PATH_MAX];

   if (data && qa_mod->help_dia)
     {
        ecore_job_add(_e_qa_help, (void*)1);
        return;
     }
   if (qa_mod->help_dia) return;
   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
   qa_mod->help_dia = (E_Object*)e_confirm_dialog_show(_("Quickaccess Help"), buf,
                                                        _("Quickaccess is a way of binding user-selected<ps/>"
                                                          "windows and applications to keyboard shortcuts.<ps/>"
                                                          "Once a Quickaccess entry has been created,<ps/>"
                                                          "the associated window can be returned to immediately<ps/>"
                                                          "on demand by pushing the keyboard shortcut."),
                                                        _("Continue"), _("Stop"),
                                                        _e_qa_help2, _e_qa_help_cancel, NULL, NULL, NULL, NULL);
   e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);
}

static void
_e_qa_first_run(void)
{
   char buf[PATH_MAX];

   snprintf(buf, sizeof(buf), "%s/e-module-quickaccess.edj", e_module_dir_get(qa_mod->module));
   qa_mod->help_dia = (E_Object*)e_confirm_dialog_show(_("Quickaccess Help"), buf,
                                                        _("This appears to be your first time using the Quickaccess module.<ps/>"
                                                          "Would you like some usage tips?"),
                                                        _("Yes"), _("No"),
                                                        _e_qa_help, _e_qa_help_cancel, (void*)1, NULL, NULL, NULL);
   e_object_free_attach_func_set(qa_mod->help_dia, _e_qa_dia_del);
}

//////////////////////////////////////////////////////////////////////////////

Eina_Bool
e_qa_init(void)
{
   E_Client_Hook *h;

   _act_toggle = eina_stringshare_add("qa_toggle");
   _e_qa_toggle = e_action_add(_act_toggle);
   _e_qa_add = e_action_add(_act_add);
   _e_qa_del = e_action_add(_act_del);
   if ((!_e_qa_toggle) || (!_e_qa_add) || (!_e_qa_del))
     {
        CRIT("could not register %s E_Action", _act_toggle);
        e_action_del(_act_toggle);
        e_action_del(_act_add);
        e_action_del(_act_del);
        _e_qa_add = _e_qa_del = _e_qa_toggle = NULL;
        eina_stringshare_replace(&_act_toggle, NULL);
        return EINA_FALSE;
     }
#define CB(id, func)                                             \
  h = e_client_hook_add(E_CLIENT_HOOK_##id, _e_qa_border_##func##_cb, NULL); \
  _e_qa_border_hooks = eina_list_append(_e_qa_border_hooks, h)

   CB(EVAL_PRE_POST_FETCH, eval_pre_post_fetch);
#undef CB

   E_LIST_HANDLER_APPEND(_e_qa_event_handlers, E_EVENT_CLIENT_FOCUS_OUT, (Ecore_Event_Handler_Cb)_e_qa_event_border_focus_out_cb, NULL);
   E_LIST_HANDLER_APPEND(_e_qa_event_handlers, E_EVENT_CLIENT_REMOVE, (Ecore_Event_Handler_Cb)_e_qa_event_border_remove_cb, NULL);
   E_LIST_HANDLER_APPEND(_e_qa_event_handlers, ECORE_EXE_EVENT_DEL, (Ecore_Event_Handler_Cb)_e_qa_event_exe_del_cb, NULL);

   _e_qa_toggle->func.go = _e_qa_toggle_cb;
   e_action_predef_name_set(_e_qa_name, _lbl_toggle, _act_toggle, NULL, _("quick access name/identifier"), 1);
   _e_qa_add->func.go = _e_qa_add_cb;
   e_action_predef_name_set(_e_qa_name, _lbl_add, _act_add, NULL, NULL, 0);
   _e_qa_del->func.go = _e_qa_del_cb;
   e_action_predef_name_set(_e_qa_name, _lbl_del, _act_del, NULL, NULL, 0);
   INF("loaded qa module, registered %s action.", _act_toggle);

   border_hook = e_int_client_menu_hook_add(_e_qa_bd_menu_hook, NULL);
   if (!qa_config->first_run) _e_qa_first_run();
   else
     _e_qa_begin();

   return EINA_TRUE;
}

void
e_qa_shutdown(void)
{
   if (_e_qa_toggle)
     {
        e_action_predef_name_del(_e_qa_name, _lbl_toggle);

        e_action_del(_act_toggle);
        _e_qa_toggle = NULL;
     }
   if (_e_qa_add)
     {
        e_action_predef_name_del(_e_qa_name, _lbl_add);

        e_action_del(_act_add);
        _e_qa_add = NULL;
     }
   if (_e_qa_del)
     {
        e_action_predef_name_del(_e_qa_name, _lbl_del);

        e_action_del(_act_del);
        _e_qa_del = NULL;
     }

   E_FREE_LIST(_e_qa_event_handlers, ecore_event_handler_del);
   E_FREE_LIST(_e_qa_border_hooks, e_client_hook_del);
   if (qa_mod->help_timeout) ecore_timer_del(qa_mod->help_timeout);
   _e_qa_help_timeout(NULL);
   e_int_client_menu_hook_del(border_hook);
   border_hook = NULL;
   INF("unloaded quickaccess module, unregistered %s action.", _act_toggle);
   eina_stringshare_del(_act_toggle);
   _act_toggle = NULL;
   qa_running = EINA_FALSE;
}

void
e_qa_entry_free(E_Quick_Access_Entry *entry)
{
   if (!entry) return;
   if (entry->exe_handler) ecore_event_handler_del(entry->exe_handler);
   if (entry->client) _e_qa_entry_border_props_restore(entry, entry->client);
   if (entry->cfg_entry) e_qa_config_entry_free(entry);
   e_qa_entry_bindings_cleanup(entry);
   e_bindings_reset();
   eina_stringshare_del(entry->id);
   eina_stringshare_del(entry->name);
   eina_stringshare_del(entry->class);
   eina_stringshare_del(entry->cmd);
   if (entry->transient)
     qa_config->transient_entries = eina_list_remove(qa_config->transient_entries, entry);
   else
     qa_config->entries = eina_list_remove(qa_config->entries, entry);
   free(entry);
   e_config_save_queue();
}

E_Quick_Access_Entry *
e_qa_entry_new(const char *id, Eina_Bool transient)
{
   E_Quick_Access_Entry *entry;

   entry = E_NEW(E_Quick_Access_Entry, 1);
   entry->id = eina_stringshare_add(id);
   entry->transient = !!transient;
   entry->config.autohide = qa_config->autohide;
   entry->config.hide_when_behind = qa_config->hide_when_behind;
   if (qa_mod->cfd) e_qa_config_entry_add(entry);
   return entry;
}

Eina_Bool
e_qa_entry_rename(E_Quick_Access_Entry *entry, const char *name)
{
   Eina_List *l;
   E_Quick_Access_Entry *e;

   /* ensure we don't get duplicates as a result of rename */
   EINA_LIST_FOREACH(qa_config->entries, l, e)
     if (e->id == name) return EINA_FALSE;
   EINA_LIST_FOREACH(qa_config->transient_entries, l, e)
     if (e->id == name) return EINA_FALSE;
   e_qa_entry_bindings_rename(entry, name);
   eina_stringshare_replace(&entry->id, name);
   e_config_save_queue();
   return EINA_TRUE;
}

void
e_qa_entries_update(void)
{
   E_Quick_Access_Entry *entry;
   Eina_List *l;

   EINA_LIST_FOREACH(qa_config->entries, l, entry)
     {
        _e_qa_entry_config_apply(entry);
        _e_qa_entry_border_props_apply(entry);
     }
   EINA_LIST_FOREACH(qa_config->transient_entries, l, entry)
     {
        _e_qa_entry_config_apply(entry);
        _e_qa_entry_border_props_apply(entry);
     }
}
