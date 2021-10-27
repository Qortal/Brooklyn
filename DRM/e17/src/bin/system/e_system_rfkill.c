#include "e_system.h"

typedef enum
{
   RFKILL_BLOCK,
   RFKILL_UNBLOCK,
   RFKILL_LIST,
} RFKill_Action;

typedef struct
{
   char *dev;
   Ecore_Event_Handler *handle_del, *handle_data;
   Ecore_Exe *exe;
   int id;
   RFKill_Action act;
   Eina_Strbuf *list_ret;
   Eina_Bool doit : 1;
} Action;

static Eina_Bool
_rfkill_dev_verify(const char *dev)
{
   const char *s;

   for (s = dev; *s; s++)
     {
        if (!(((*s >= 'a') && (*s <= 'z')) ||
              ((*s >= 'A') && (*s <= 'Z')) ||
              ((*s >= '0') && (*s <= '9')) ||
              (*s == '-') || (*s == '+') || (*s == ',') || (*s == '.') ||
              (*s == '/') || (*s == ':') || (*s == '=') || (*s == '@') ||
              (*s == '_')))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_rfkill_action_free(Action *a)
{
   // a->exe can stay - exe's are auto-freed by ecore on exit
//   if (a->exe) ecore_exe_free(a->exe);
   if (a->handle_del) ecore_event_handler_del(a->handle_del);
   if (a->handle_data) ecore_event_handler_del(a->handle_data);
   if (a->list_ret) eina_strbuf_free(a->list_ret);
   free(a->dev);
   free(a);
}

static Eina_Bool
_cb_rfkill_exe_del(void *data, int ev_type EINA_UNUSED, void *event)
{
   Action *a = data;
   Ecore_Exe_Event_Del *ev = event;
   if (ev->exe != a->exe) return EINA_TRUE;
   if (a->id >= 0)
     {
        if (a->doit)
          {
             if (a->act == RFKILL_BLOCK)
               e_system_inout_command_send("rfkill-block", "%i %s",
                                           ev->exit_code, a->dev);
             else if (a->act == RFKILL_UNBLOCK)
               e_system_inout_command_send("rfkill-unblock", "%i %s",
                                           ev->exit_code, a->dev);
             _rfkill_action_free(a);
          }
        else
          {
             char cmd[1024];

             // a->exe can stay - exe's are auto-freed by ecore on exit
             // ecore_exe_free(a->exe);
             a->exe = NULL;
             if ((a->act == RFKILL_BLOCK) || (a->act == RFKILL_UNBLOCK))
               {
                  if (snprintf(cmd, sizeof(cmd), "rfkill %s %i",
                               (a->act == RFKILL_BLOCK) ? "block" : "unblock",
                               a->id) < (int)(sizeof(cmd) - 1))
                    {
                       a->doit = EINA_TRUE;
                       a->exe = ecore_exe_pipe_run(cmd,
                                                   ECORE_EXE_NOT_LEADER |
                                                   ECORE_EXE_TERM_WITH_PARENT |
                                                   ECORE_EXE_PIPE_READ |
                                                   ECORE_EXE_PIPE_WRITE |
                                                   ECORE_EXE_PIPE_READ_LINE_BUFFERED, NULL);
                       if (!a->exe) _rfkill_action_free(a);
                    }
               }
             else if (a->act == RFKILL_LIST)
               {
                  e_system_inout_command_send("rfkill-list", "-");
                  _rfkill_action_free(a);
               }
             else _rfkill_action_free(a);
          }
     }
   else
     {
        if (a->act == RFKILL_BLOCK)
          e_system_inout_command_send("rfkill-block", "%i %s",
                                      123, a->dev);
        else if (a->act == RFKILL_UNBLOCK)
          e_system_inout_command_send("rfkill-unblock", "%i %s",
                                      123, a->dev);
        else if (a->act == RFKILL_LIST)
          {
             if (a->list_ret)
               {
                  const char *s = eina_strbuf_string_get(a->list_ret);
                  if (s)
                    e_system_inout_command_send("rfkill-list", "%s", s);
                  else
                    e_system_inout_command_send("rfkill-list", "-");
               }
             else
               e_system_inout_command_send("rfkill-list", "-");
             _rfkill_action_free(a);
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_cb_rfkill_exe_data(void *data, int ev_type EINA_UNUSED, void *event)
{
   Action *a = data;
   Ecore_Exe_Event_Data *ev = event;
   int i;

   if (ev->exe != a->exe) return EINA_TRUE;
   if (a->id >= 0) return EINA_TRUE;
   for (i = 0; ev->lines[i].line; i++)
     {
        int id;
        char dev[1024], type[1024];

        id = -1;
        if (sscanf(ev->lines[i].line, "%i: %1023[^:]: %1023[^\n]", &id, dev, type) == 3)
          {
             if ((a->act == RFKILL_BLOCK) || (a->act == RFKILL_UNBLOCK))
               {
                  if (!strcmp(a->dev, dev))
                    {
                       a->id = id;
                       // wait for exit to spawn rfkill again...
                       break;
                    }
               }
             else
               {
                  if (!a->list_ret) a->list_ret = eina_strbuf_new();
                  if (a->list_ret)
                    eina_strbuf_append_printf(a->list_ret, "%s\t%s\n",
                                              dev, type);
               }
          }
     }
   return EINA_TRUE;
}

static void
_rfkill_do(RFKill_Action act, const char *dev)
{
   Action *a = calloc(1, sizeof(Action));
   if (!a) return;
   a->dev = strdup(dev);
   if (!a->dev) goto err;
   a->id = -1;
   a->act = act;
   a->handle_del = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                           _cb_rfkill_exe_del, a);
   a->handle_data = ecore_event_handler_add(ECORE_EXE_EVENT_DATA,
                                            _cb_rfkill_exe_data, a);
   if ((!a->handle_del) || (!a->handle_data)) goto err;
   // stage 1 - run to get list of devices and id's to know what id to use
   a->exe = ecore_exe_pipe_run("rfkill list",
                               ECORE_EXE_NOT_LEADER |
                               ECORE_EXE_TERM_WITH_PARENT |
                               ECORE_EXE_PIPE_READ |
                               ECORE_EXE_PIPE_WRITE |
                               ECORE_EXE_PIPE_READ_LINE_BUFFERED, NULL);
   if (!a->exe) goto err;
   return;
err:
   ERR("Can't run rfkill\n");
   _rfkill_action_free(a);
}

static void
_cb_rfkill_block(void *data EINA_UNUSED, const char *params)
{
   if (!_rfkill_dev_verify(params)) return;
   _rfkill_do(RFKILL_BLOCK, params);
}

static void
_cb_rfkill_unblock(void *data EINA_UNUSED, const char *params)
{
   if (!_rfkill_dev_verify(params)) return;
   _rfkill_do(RFKILL_UNBLOCK, params);
}

static void
_cb_rfkill_list(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   _rfkill_do(RFKILL_LIST, "");
}

void
e_system_rfkill_init(void)
{
   e_system_inout_command_register("rfkill-block",   _cb_rfkill_block, NULL);
   e_system_inout_command_register("rfkill-unblock", _cb_rfkill_unblock, NULL);
   e_system_inout_command_register("rfkill-list",    _cb_rfkill_list, NULL);
}

void
e_system_rfkill_shutdown(void)
{
   // only shutdown things we really have to - no need to free mem etc.
}
