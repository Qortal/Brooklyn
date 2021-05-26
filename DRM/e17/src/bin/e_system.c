#include "e.h"

typedef struct
{
   char cmd[24];
   int  size;
} Message_Head;

typedef struct
{
   void (*func) (void *data, const char *params);
   void *data;
   Eina_Bool delete_me : 1;
   Eina_Bool just_added : 1;
} Handler;

static Ecore_Exe *_system_exe = NULL;
static Ecore_Event_Handler *_handler_del = NULL;
static Ecore_Event_Handler *_handler_data = NULL;
static Eina_Binbuf *_msg_buf = NULL;
static Eina_Hash *_handlers = NULL;
static int _handlers_busy = 0;
static Ecore_Timer *_error_dialog_timer = NULL;
static double _last_spawn = 0.0;
static int _respawn_count = 0;

static Eina_Bool
_cb_dialog_timer(void *data)
{
   int exit_code = (int)(long)data;

   _error_dialog_timer = NULL;

        if (exit_code ==  0) // can't even do ecore_exe_run...
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because ecore_exe_run() failed."));
   else if (exit_code == 31) // can't get passwd entry for user
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because your user has no passwd file entry."));
   else if (exit_code == 32) // username is blank in passwd entry
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because your username is blank in the passwd file."));
   else if (exit_code == 33) // can't alloc mem for username
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because it could not allocate memory."));
   else if (exit_code == 34) // can't get group entry for user's group
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because it can't find your user group entry."));
   else if (exit_code == 35) // group name is blank
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because your user group entry is blank."));
   else if (exit_code == 36) // can't setuid to 0
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "since it can't become root. Missing setuid bit?"));
   else if (exit_code == 37) // can't setgid to 0
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "since it can't become group root. Missing setuid bit?"));
   else if (exit_code == 38) // can't get passwd entry for uid 0
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because the passwd file entry for root isn't found."));
   else if (exit_code == 39) // root has no homedir
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because the root home directory is blank."));
   else if (exit_code == 40) // root homedir is not a full path
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because the root home directory is not a full path."));
   else if (exit_code == 41) // root homedir does not resolve to a path
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because the root home directory can't be found."));
   else if (exit_code == 42) // can't change HOME to root homedir
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because it can't change the HOME environment."));
   else if (exit_code == 43) // can't change dir to root homedir
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because it cannot change working directory to root's home."));
   else if (exit_code == 46)    // user denied to all subsystems
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service<br>"
                          "because your user is denied access to all services.<br>"
                          "Please see /etc/enlightenment/system.conf."));
   else
     e_util_dialog_show(_("Error in Enlightenment System Service"),
                        _("Enlightenment cannot successfully start<br>"
                          "the enlightenment_system service for<br>"
                          "some unknown reason."));
   return EINA_FALSE;
}

static void
_system_spawn_error(int exit_code)
{
   if (_error_dialog_timer) ecore_timer_del(_error_dialog_timer);
   _error_dialog_timer = ecore_timer_add(5.0, _cb_dialog_timer,
                                         (void *)(long)exit_code);
}

static void
_system_spawn(void)
{
   char buf[PATH_MAX];
   double t = ecore_time_get();

   if ((t - _last_spawn) < 10.0) _respawn_count++;
   else _respawn_count = 0;
   if (_respawn_count > 5) return;
   snprintf(buf, sizeof(buf),
            "%s/enlightenment/utils/enlightenment_system", e_prefix_lib_get());
   _system_exe = ecore_exe_pipe_run
     (buf, ECORE_EXE_NOT_LEADER | ECORE_EXE_TERM_WITH_PARENT |
      ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE, NULL);
   if (!_system_exe) _system_spawn_error(0);
}

static Eina_Bool
_system_message_read(void)
{
   Message_Head *head;
   const void *bdata = eina_binbuf_string_get(_msg_buf);
   const char *data = bdata;
   size_t len = eina_binbuf_length_get(_msg_buf);
   Eina_Binbuf *buf2;

   if (!data) return EINA_FALSE;
   if (len < sizeof(Message_Head)) return EINA_FALSE;
   head = (Message_Head *)bdata;
   head->cmd[23] = 0;
   if (len < (sizeof(Message_Head) + head->size)) return EINA_FALSE;
   if (_handlers)
     {
        Eina_List *list, *l, *ll, *plist;
        Handler *h;
        int del_count = 0;

        _handlers_busy++;
        list = plist = eina_hash_find(_handlers, head->cmd);
        EINA_LIST_FOREACH(list, l, h)
          {
             if ((!h->delete_me) && (!h->just_added))
               {
                  if (head->size == 0) h->func(h->data, NULL);
                  else
                    {
                       if ((data + sizeof(Message_Head))[head->size - 1] == 0)
                         h->func(h->data,
                                 (const char *)(data + sizeof(Message_Head)));
                    }
               }
          }
        _handlers_busy--;
        if (_handlers_busy == 0)
          {
             EINA_LIST_FOREACH_SAFE(list, l, ll, h)
               {
                  h->just_added = EINA_FALSE;
                  if (h->delete_me)
                    {
                       list = eina_list_remove_list(list, l);
                       del_count++;
                       free(h);
                    }
               }
          }
        if (del_count > 0)
          {
             eina_hash_del(_handlers, head->cmd, plist);
             if (list)
               eina_hash_add(_handlers, head->cmd, list);
          }
     }
   buf2 = eina_binbuf_new();
   if (buf2)
     {
        eina_binbuf_append_length
          (buf2,
           (const unsigned char *)data + sizeof(Message_Head) + head->size,
           len - (sizeof(Message_Head) + head->size));
        eina_binbuf_free(_msg_buf);
        _msg_buf = buf2;
     }
   return EINA_TRUE;
}

static Eina_Bool
_cb_exe_del(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev = event;

   if (ev->exe != _system_exe) return ECORE_CALLBACK_PASS_ON;
   if ((ev->exit_code == 31) || // can't get passwd entry for user
       (ev->exit_code == 32) || // username is blank in passwd entry
       (ev->exit_code == 33) || // can't alloc mem for username
       (ev->exit_code == 34) || // can't get group entry for user's group
       (ev->exit_code == 35) || // group name is blank
       (ev->exit_code == 36) || // can't setuid to 0
       (ev->exit_code == 37) || // can't setgid to 0
       (ev->exit_code == 38) || // can't get passwd entry for uid 0
       (ev->exit_code == 39) || // root has no homedir
       (ev->exit_code == 40) || // root homedir is not a full path
       (ev->exit_code == 41) || // root homedir does not resolve to a path
       (ev->exit_code == 42) || // can't change HOME to root homedir
       (ev->exit_code == 43) || // can't change dir to root homedir
       (ev->exit_code == 46)    // user denied to all subsystems
      )
     {
        // didn't run because it literally refused....
        _system_spawn_error(ev->exit_code);
     }
   else
     {
        // it died for some other reason - restart it - maybe crashed?
        if (_msg_buf) eina_binbuf_free(_msg_buf);
        _msg_buf = eina_binbuf_new();
        _system_spawn();
     }
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_cb_exe_data(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Data *ev = event;

   if (ev->exe != _system_exe) return ECORE_CALLBACK_PASS_ON;
   // this is dumb, but it will work as this is avery low bandwidth
   // i/o channel to/from the child exe stdin/out
   if (_msg_buf)
     {
        eina_binbuf_append_length(_msg_buf, ev->data, ev->size);
        while (_msg_buf && _system_message_read());
     }
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_handler_list_free(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Eina_List *list = data;
   Handler *h;

   EINA_LIST_FREE(list, h) free(h);
   return EINA_TRUE;
}

/* externally accessible functions */
EINTERN int
e_system_init(void)
{
   // XXX:
   //
   // if exe_data - parse/get data
   //   ... per message - call registered cb's for that msg
   _handler_del = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _cb_exe_del, NULL);
   _handler_data = ecore_event_handler_add(ECORE_EXE_EVENT_DATA, _cb_exe_data, NULL);
   _msg_buf = eina_binbuf_new();
   _handlers = eina_hash_string_superfast_new(NULL);
   _system_spawn();
   return 1;
}

EINTERN int
e_system_shutdown(void)
{
   if (_msg_buf) eina_binbuf_free(_msg_buf);
   if (_handler_del) ecore_event_handler_del(_handler_del);
   if (_handler_data) ecore_event_handler_del(_handler_data);
   if (_system_exe)
     {
        ecore_exe_interrupt(_system_exe);
        ecore_exe_quit(_system_exe);
        ecore_exe_terminate(_system_exe);
        ecore_exe_kill(_system_exe);
        ecore_exe_free(_system_exe);
     }
   if (_handlers)
     {
        eina_hash_foreach(_handlers, _handler_list_free, NULL);
        eina_hash_free(_handlers);
     }
   if (_error_dialog_timer) ecore_timer_del(_error_dialog_timer);
   _msg_buf = NULL;
   _handler_del = NULL;
   _handler_data = NULL;
   _system_exe = NULL;
   _handlers = NULL;
   _error_dialog_timer = NULL;
   return 1;
}

E_API void EINA_PRINTF(2, 3)
e_system_send(const char *cmd, const char *fmt, ...)
{
   char *buf = NULL, stack_buf[4096];
   Message_Head head;
   size_t len = strlen(cmd);
   int printed = 0;
   va_list ap;

   if (!_system_exe)
     {
        ERR("Trying to send system command to non-existent process");
        return;
     }
   if (len > 23)
     {
        ERR("Trying to send command of length %i (max 23)", (int)len);
        return;
     }
   if (fmt)
     {
        buf = stack_buf;
        va_start(ap, fmt);
        printed = vsnprintf(stack_buf, sizeof(stack_buf), fmt, ap);
        va_end(ap);
        if ((size_t)printed >= (sizeof(stack_buf) - 1))
          {
             buf = malloc(printed + 1);
             if (!buf) goto end;
             va_start(ap, fmt);
             printed = vsnprintf(buf, printed + 1, fmt, ap);
             va_end(ap);
          }
     }

   memset(head.cmd, 0, sizeof(head.cmd));
   memcpy(head.cmd, cmd, len);
   if (printed > 0) head.size = printed + 1;
   else head.size = 0;
   ecore_exe_send(_system_exe, &head, sizeof(head));
   if ((buf) && (head.size > 0))
     {
        ecore_exe_send(_system_exe, buf, head.size);
     }
end:
   if (buf != stack_buf) free(buf);
}

E_API void
e_system_handler_add(const char *cmd, void (*func) (void *data, const char *params), void *data)
{
   Eina_List *list;
   Handler *h;

   if (!_handlers)
     {
        ERR("Trying to add system handler to NULL handler hash");
        return;
     }
   h = calloc(1, sizeof(Handler));
   if (!h) return;
   h->func = func;
   h->data = data;
   if (_handlers_busy > 0) h->just_added = EINA_TRUE;
   list = eina_hash_find(_handlers, cmd);
   eina_hash_del(_handlers, cmd, list);
   list = eina_list_append(list, h);
   eina_hash_add(_handlers, cmd, list);
}

E_API void
e_system_handler_del(const char *cmd, void (*func) (void *data, const char *params), void *data)
{
   Eina_List *list, *l;
   Handler *h;

  if (!_handlers)
     {
        ERR("Trying to del system handler to NULL handler hash");
        return;
     }
   list = eina_hash_find(_handlers, cmd);
   if (list)
     {
        eina_hash_del(_handlers, cmd, list);
        EINA_LIST_FOREACH(list, l, h)
          {
             if ((h->func == func) && (h->data == data))
               {
                  if (_handlers_busy > 0)
                    h->delete_me = EINA_TRUE;
                  else
                    {
                       list = eina_list_remove_list(list, l);
                       free(h);
                    }
                  break;
               }
          }
        if (list)
          eina_hash_add(_handlers, cmd, list);
     }
}

