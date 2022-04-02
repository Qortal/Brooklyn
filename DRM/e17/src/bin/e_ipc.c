#include "e.h"

EINTERN char *e_ipc_socket = NULL;

#ifdef USE_IPC
/* local subsystem functions */
static Eina_Bool _e_ipc_cb_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static Eina_Bool _e_ipc_cb_client_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

/* local subsystem globals */
static Ecore_Ipc_Server *_e_ipc_server = NULL;
static Eina_Stringshare *_e_ipc_dir = NULL;
#endif

/* externally accessible functions */
EINTERN int
e_ipc_init(void)
{
   char buf[PATH_MAX], buf2[128], buf3[PATH_MAX + 20];
   char *tmp, *user, *base;
   int pid, trynum = 0, id1 = 0;
   struct stat st;

   tmp = getenv("TMPDIR");
   if (!tmp) tmp = "/tmp";
   base = tmp;

   tmp = getenv("XDG_RUNTIME_DIR");
   if (tmp)
     {
        if (stat(tmp, &st) == 0)
          {
             if ((st.st_uid == getuid()) &&
                 ((st.st_mode & (S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO)) ==
                  (S_IRWXU | S_IFDIR)))
               base = tmp;
             else
               ERR("XDG_RUNTIME_DIR of '%s' failed permissions check", tmp);
          }
        else
          ERR("XDG_RUNTIME_DIR of '%s' cannot be accessed", tmp);
     }

   tmp = getenv("SD_USER_SOCKETS_DIR");
   if (tmp)
     {
        if (stat(tmp, &st) == 0)
          {
             if ((st.st_uid == getuid()) &&
                 ((st.st_mode & (S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO)) ==
                  (S_IRWXU | S_IFDIR)))
               base = tmp;
             else
               ERR("SD_USER_SOCKETS_DIR of '%s' failed permissions check", tmp);
          }
        else
          ERR("SD_USER_SOCKETS_DIR of '%s' cannot be accessed", tmp);
     }

   user = getenv("USER");
   if (!user)
     {
        int uidint;

        user = "__unknown__";
        uidint = getuid();
        if (uidint >= 0)
          {
             snprintf(buf2, sizeof(buf2), "%i", uidint);
             user = buf2;
          }
     }

   e_util_env_set("E_IPC_SOCKET", "");

   pid = (int)getpid();
   for (trynum = 0; trynum <= 4096; trynum++)
     {
        snprintf(buf, sizeof(buf), "%s/e-%s@%x",
                 base, user, id1);
        if (!mkdir(buf, S_IRWXU))
          {
#ifdef USE_IPC
             _e_ipc_dir = eina_stringshare_add(buf);
             snprintf(buf3, sizeof(buf3), "%s/%i",
                      buf, pid);
             _e_ipc_server = ecore_ipc_server_add
                (ECORE_IPC_LOCAL_SYSTEM, buf3, 0, NULL);
             if (_e_ipc_server)
#endif
               {
                  e_ipc_socket = strdup(ecore_file_file_get(buf));
                  break;
               }
          }
        id1 = rand();
     }
#ifdef USE_IPC
   if (!_e_ipc_server)
     {
        if (_e_ipc_dir)
          {
             ecore_file_recursive_rm(_e_ipc_dir);
             eina_stringshare_del(_e_ipc_dir);
             _e_ipc_dir = NULL;
          }
        ERR("Gave up after 4096 sockets in '%s'. All failed", base);
        return 0;
     }

   INF("E_IPC_SOCKET=%s", buf3);
   e_util_env_set("E_IPC_SOCKET", buf3);
   ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL,
                           _e_ipc_cb_client_del, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA,
                           _e_ipc_cb_client_data, NULL);

   e_ipc_codec_init();
#endif
   return 1;
}

EINTERN int
e_ipc_shutdown(void)
{
#ifdef USE_IPC
   e_ipc_codec_shutdown();
   if (_e_ipc_server)
     {
        ecore_ipc_server_del(_e_ipc_server);
        _e_ipc_server = NULL;
     }
   if (_e_ipc_dir)
     {
        ecore_file_recursive_rm(_e_ipc_dir);
        eina_stringshare_del(_e_ipc_dir);
        _e_ipc_dir = NULL;
     }
#endif
   E_FREE(e_ipc_socket);
   return 1;
}

#ifdef USE_IPC
/* local subsystem globals */
static Eina_Bool
_e_ipc_cb_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Del *e;

   e = event;
   if (ecore_ipc_client_server_get(e->client) != _e_ipc_server)
     return ECORE_CALLBACK_PASS_ON;
   /* delete client sruct */
   e_thumb_client_del(e);
   e_fm2_client_del(e);
   ecore_ipc_client_del(e->client);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_ipc_cb_client_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Data *e;

   e = event;
   if (ecore_ipc_client_server_get(e->client) != _e_ipc_server)
     return ECORE_CALLBACK_PASS_ON;
   switch (e->major)
     {
      case E_IPC_DOMAIN_SETUP:
      case E_IPC_DOMAIN_REQUEST:
      case E_IPC_DOMAIN_REPLY:
      case E_IPC_DOMAIN_EVENT:
        break;

      case E_IPC_DOMAIN_THUMB:
        e_thumb_client_data(e);
        break;

      case E_IPC_DOMAIN_FM:
        e_fm2_client_data(e);
        break;

      case E_IPC_DOMAIN_ALERT:
      {
         switch (e->minor)
           {
            case E_ALERT_OP_RESTART:
              exit(111);
              break;

            case E_ALERT_OP_EXIT:
              exit(101);
              break;
           }
      }
      break;

      default:
        break;
     }
   return ECORE_CALLBACK_PASS_ON;
}

#endif
