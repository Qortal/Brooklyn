#include "e_system.h"

typedef struct
{
   char *dev, *mountpoint, *cmd, *cmd2, *cmd1;
   Ecore_Exe *exe;
   Ecore_Event_Handler *handler;
   Eina_Bool eject : 1;
   Eina_Bool mount : 1;
   Eina_Bool umount : 1;
   Eina_Bool in_progress : 1;
} Action;

static Eina_List *_pending_actions = NULL;

static Eina_Bool _store_action_do(Action *a);

static Eina_Bool
_store_device_verify(const char *dev)
{
   const char *s;
   struct stat st;

   // not even /dev/something? no way.
   if (!(!strncmp(dev, "/dev/", 5))) return EINA_FALSE;
   // if it contains /.. - even tho it could be file/..name not file/../name
   // it still looks suspicious enough so - no
   if (strstr(dev, "/..")) return EINA_FALSE;
   // any chars that cold be used in any evil ways - no way. device names
   // should not have these...
   for (s = dev; *s; s++)
     {
        if ((*s <= '*') || (*s == '`') || (*s == ';') || (*s == '<') ||
            (*s == '>') || (*s == '?') || (*s >= '{') ||
            ((*s >= '[') && (*s <= '^')))
          return EINA_FALSE;
     }
   // must exist as a file - if not - nope
   if (stat(dev, &st) != 0) return EINA_FALSE;
   // XXX: must check it's a block device, that is removable etc. etc.
   // XXX: for now we have no code for this so always fail until we do
   return EINA_FALSE;
//   return EINA_TRUE;
}

static Eina_Bool
_store_uuid_verify(const char *dev)
{
   const char *s;

   if (!(!strncmp(dev, "UUID=", 5))) return EINA_FALSE;
   for (s = dev + 5; *s; s++)
     {
        if ((!isalnum(*s)) && (*s != '-')) return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_mkdir(const char *path)
{
   mode_t um;
   int ret, e;

   um = umask(0);
   errno = 0;
   ret = mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
   e = errno;
   umask(um);
   if (ret != 0)
     {
        if (e != EEXIST)
          {
             ERR("Can't create mount dir [%s]\n", path);
             return EINA_FALSE;
          }
        else
          {
             struct stat st;

             if (stat(path, &st) == 0)
               {
                  if (!S_ISDIR(st.st_mode))
                    {
                       ERR("Path is not a dir [%s]\n", path);
                       return EINA_FALSE;
                    }
                  if (st.st_uid != 0) return EINA_FALSE;
                  if (st.st_gid != 0) return EINA_FALSE;
               }
             else return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_store_is_triplet(const char *mnt, char *dirs[3])
{  // check that mnt is "/media/xxx/yyy" and nothing more or less. No special
   // chars or escapes like \ or * oro ; or { or [ etc.
   // no ../ or ./ or empty path elements. then break up the 3 elements
   // into dirs[] is we return true
   const char *s, *s2;

   dirs[0] = NULL;
   dirs[1] = NULL;
   dirs[2] = NULL;

   // phase one sanity check
   if (!mnt) goto err;
   if (mnt[0] != '/') goto err;
   for (s = mnt; *s; s++)
     {
        if ((*s == '\\') ||
            (*s == '(') || (*s == '$') || (*s <= '*') || (*s == '`') ||
            (*s == ';') || (*s == '<') || (*s == '>') || (*s == '?') ||
            (*s >= '{') ||
            ((*s >= '[') && (*s <= '^')))
          goto err;
     }
   if ((strstr(mnt, "/..")) || (strstr(mnt, "/./")) || (strstr(mnt, "//")))
     goto err;

   // phase 2 - break up into 3 dir components
   s = mnt + 1;
   if (*s == '\0') goto err;
   s2 = strchr(s, '/');
   if (!s2) goto err;
   dirs[0] = malloc(s2 - s + 1);
   if (!dirs[0]) goto err;
   memcpy(dirs[0], s, s2 - s);
   dirs[0][s2 - s] = 0;
   s = s2 + 1;

   if (*s == '\0') goto err;
   s2 = strchr(s, '/');
   if (!s2) goto err;
   dirs[1] = malloc(s2 - s + 1);
   if (!dirs[1]) goto err;
   memcpy(dirs[1], s, s2 - s);
   dirs[1][s2 - s] = 0;
   s = s2 + 1;

   if (*s == '\0') goto err;
   s2 = strchr(s, '/');
   if (s2) goto err; // different - if there is another / - even trailing
   for (s2 = s; *s2 != '\0'; s2++); // s2 - walk to nul byte end
   dirs[1] = malloc(s2 - s + 1);
   if (!dirs[1]) goto err;
   memcpy(dirs[1], s, s2 - s);
   dirs[1][s2 - s] = 0;
   s = s2 + 1;

   return EINA_TRUE;
err:
   free(dirs[0]);
   free(dirs[1]);
   free(dirs[2]);
   dirs[0] = NULL;
   dirs[1] = NULL;
   dirs[2] = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_store_mount_verify(const char *mnt)
{
   char *dirs[3] = { NULL, NULL, NULL };
   char *tmp = NULL;

   // XXX: we should use /run/media - possibly make this adapt
   if (!mnt) return EINA_FALSE;
   if (!(!strncmp(mnt, "/media/", 7))) goto err;
   if (!_store_is_triplet(mnt, dirs)) goto err;
   tmp = malloc(strlen(mnt) + 1);
   if (!tmp)
     {
        free(dirs[0]);
        free(dirs[1]);
        free(dirs[2]);
        return EINA_FALSE;
     }
   // 2nd path must be username
   if (!!strcmp(dirs[1], user_name)) goto err;

   tmp[0] = 0;
   strcat(tmp, "/");
   strcat(tmp, dirs[0]);
   if (!_mkdir(tmp)) goto err;

   strcat(tmp, "/");
   strcat(tmp, dirs[1]);
   if (!_mkdir(tmp)) goto err;

   strcat(tmp, "/");
   strcat(tmp, dirs[2]);
   if (!_mkdir(tmp)) goto err;

   free(tmp);
   free(dirs[0]);
   free(dirs[1]);
   free(dirs[2]);
   return EINA_TRUE;
err:
   ERR("Malformed mount point or create error [%s]\n", mnt);
   free(tmp);
   free(dirs[0]);
   free(dirs[1]);
   free(dirs[2]);
   return EINA_FALSE;
}

static Eina_Bool
_store_umount_verify(const char *mnt)
{
   char *dirs[3] = { NULL, NULL, NULL };
   char *tmp = NULL;
   struct stat st;

   // XXX: we should use /run/media - possibly make this adapt
   if (!mnt) return EINA_FALSE;
   if (!(!strncmp(mnt, "/media/", 7))) goto err;
   if (!_store_is_triplet(mnt, dirs)) goto err;
   tmp = malloc(strlen(mnt) + 1);
   if (!tmp)
     {
        free(dirs[0]);
        free(dirs[1]);
        free(dirs[2]);
        return EINA_FALSE;
     }
   // 2nd path must be username
   if (!!strcmp(dirs[1], user_name)) goto err;
   if (stat(mnt, &st) != 0) goto err;
   if (!S_ISDIR(st.st_mode)) goto err;
   if (st.st_uid != 0) goto err;
   if (st.st_gid != 0) goto err;

   free(tmp);
   free(dirs[0]);
   free(dirs[1]);
   free(dirs[2]);
   return EINA_TRUE;
err:
   ERR("Malformed umount point [%s]\n", mnt);
   free(tmp);
   free(dirs[0]);
   free(dirs[1]);
   free(dirs[2]);
   return EINA_FALSE;
}

static void
_store_action_free(Action *a)
{
   free(a->dev);
   free(a->mountpoint);
   free(a->cmd);
   free(a->cmd2);
   free(a->cmd1);
   if (a->handler) ecore_event_handler_del(a->handler);
   if (a->exe) ecore_exe_free(a->exe);
   free(a);
}

static void
_store_pending_action_next(void)
{
   while (_pending_actions)
     {
        Action *a = _pending_actions->data;
        if (_store_action_do(a)) break;
        else _pending_actions = eina_list_remove(_pending_actions, a);
     }
}

static Eina_Bool
_cb_store_eject_exe_del(void *data, int ev_type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev = event;
   Action *a = data;

   if (ev->exe == a->exe)
     {
        a->exe = NULL;
        if (a->eject)
          e_system_inout_command_send("store-eject", "%i %s",
                                      ev->exit_code, a->dev);
        else if ((a->mount) &&
                 ((ev->exit_code == 0) ||
                  (!((a->cmd) || (a->cmd1)  || (a->cmd2)))))
          e_system_inout_command_send("store-mount", "%i %s %s",
                                      ev->exit_code, a->dev, a->mountpoint);
        else if (a->umount)
          {
             if (ev->exit_code == 0)
               {
                  rmdir(a->mountpoint);
               }
             e_system_inout_command_send("store-umount", "%i %s",
                                         ev->exit_code, a->mountpoint);
          }
        if (((a->cmd) || (a->cmd1) || (a->cmd2)) && (ev->exit_code != 0))
          {
             if (!_store_action_do(a))
               {
                  _pending_actions = eina_list_remove(_pending_actions, a);
                  _store_pending_action_next();
               }
          }
        else
          {
             _pending_actions = eina_list_remove(_pending_actions, a);
             _store_action_free(a);
             _store_pending_action_next();
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_store_action_do(Action *a)
{
   a->handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                        _cb_store_eject_exe_del, a);
   if (!a->handler) _store_action_free(a);
   else
     {
        if (a->cmd2)
          {
             a->exe = e_system_run(a->cmd2);
             free(a->cmd2);
             a->cmd2 = NULL;
          }
        else if (a->cmd1)
          {
             a->exe = e_system_run(a->cmd1);
             free(a->cmd1);
             a->cmd1 = NULL;
          }
        else
          {
             a->exe = e_system_run(a->cmd);
             free(a->cmd);
             a->cmd = NULL;
          }
        a->in_progress = EINA_TRUE;
        if (!a->exe) _store_action_free(a);
        else return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_store_action_queue(Action *a)
{
   if (_pending_actions)
     _pending_actions = eina_list_append(_pending_actions, a);
   else if (_store_action_do(a))
     _pending_actions = eina_list_append(_pending_actions, a);
}

static void
_cb_store_eject(void *data EINA_UNUSED, const char *params)
{
   // params:
   // /dev/xxx
   char cmd0[512];
   char cmd[4096 + 512 + 128];

   if (!_store_device_verify(params))
     {
        ERR("Invalid device [%s]\n", params);
        return;
     }
#if defined (__FreeBSD__) || defined (__OpenBSD__)
   // on bsd cdcontrol is the shnizzle for this
   if (ecore_file_app_installed("cdcontrol"))
     snprintf(cmd0, sizeof(cmd0), "cdcontrol eject");
   // otherwise regular old eject will do
   else
#endif
     snprintf(cmd0, sizeof(cmd0), "eject");
   if (snprintf(cmd, sizeof(cmd), "%s %s", cmd0, params) < (int)(sizeof(cmd) - 1))
     {
        Action *a = calloc(1, sizeof(Action));
        if (a)
          {
             a->eject = EINA_TRUE;
             a->dev = strdup(params);
             a->cmd = strdup(cmd);
             if ((!a->dev) || (!a->cmd)) _store_action_free(a);
             else _store_action_queue(a);
          }
     }
}

static void
_cb_store_mount(void *data EINA_UNUSED, const char *params)
{
   // params:
   // /dev/sdc1 - /media/user/dir
   // /dev/disk/by-uuid/d9c53a62-7fc2-4cc3-9616-4e41e065da4c - /media/user/dir
   // /dev/sdb1 x /media/user/dir
   // ...
   // DEV ARG MNT
   // ARG is - OR one or more of the below chars (-, xs, rx, ...):
   // x = noexec
   // r = ro
   // s = sync
   // d = dirsync
   // l = lazytime
   // a = noatime
   // A = relatime
   // D = diratime
   char dev[1024], arg[128], mnt[4096], opt2[512], opt1[512], opt0[256], *p;
   char cmd2[(4096 * 2) + 1024 + 500];
   char cmd1[(4096 * 2) + 1024 + 500];
   char cmd0[(4096 * 2) + 1024 + 500];

   if (sscanf(params, "%1023s %127s %4095s", dev, arg, mnt) == 3)
     {
        char *mnt2;

        if ((!_store_device_verify(dev)) && (!_store_uuid_verify(dev)))
          {
             ERR("Invalid device [%s]\n", dev);
             return;
          }
        if (!_store_mount_verify(mnt))
          {
             ERR("Invalid mount [%s]\n", mnt);
             return;
          }
        mnt2 = ecore_file_escape_name(mnt);
        if (mnt2)
          {
             Eina_Bool o_noexec = EINA_FALSE;
             Eina_Bool o_ro = EINA_FALSE;
             Eina_Bool o_sync = EINA_FALSE;
             Eina_Bool o_dirsync = EINA_FALSE;
             Eina_Bool o_lazytime = EINA_FALSE;
             Eina_Bool o_noatime = EINA_FALSE;
             Eina_Bool o_relatime = EINA_FALSE;
             Eina_Bool o_diratime = EINA_FALSE;

             for (p = arg; *p; p++)
               {
                  if      ((*p) == 'x') o_noexec = EINA_TRUE;
                  else if ((*p) == 'r') o_ro = EINA_TRUE;
                  else if ((*p) == 's') o_sync = EINA_TRUE;
                  else if ((*p) == 'd') o_dirsync = EINA_TRUE;
                  else if ((*p) == 'l') o_lazytime = EINA_TRUE;
                  else if ((*p) == 'a') o_noatime = EINA_TRUE;
                  else if ((*p) == 'A') o_relatime = EINA_TRUE;
                  else if ((*p) == 'D') o_diratime = EINA_TRUE;
               }
             snprintf(opt0, sizeof(opt0),
                      "nosuid,nodev%s%s%s%s%s%s%s%s",
                      o_noexec ? ",noexec" : "",
                      o_ro ? ",ro" : "",
                      o_sync ? ",sync" : "",
                      o_dirsync ? ",dirsync" : "",
                      o_lazytime ? ",lazytime" : "",
                      o_noatime ? ",noatime" : "",
                      o_relatime ? ",relatime" : "",
                      o_diratime ? ",diratime" : "");
             snprintf(opt2, sizeof(opt2), "%s,iocharset=utf8,uid=%i", opt0, uid);
             snprintf(opt1, sizeof(opt1), "%s,iocharset=utf8", opt0);
             // opt2, opt1, opt0
             if ((snprintf(cmd2, sizeof(cmd2), "mount -o %s %s %s",
                           opt2, dev, mnt2) < (int)(sizeof(cmd2) - 1)) &&
                 (snprintf(cmd1, sizeof(cmd1), "mount -o %s %s %s",
                           opt1, dev, mnt2) < (int)(sizeof(cmd1) - 1)) &&
                 (snprintf(cmd0, sizeof(cmd0), "mount -o %s %s %s",
                           opt0, dev, mnt2) < (int)(sizeof(cmd0) - 1)))
               {
                  Action *a = calloc(1, sizeof(Action));
                  if (a)
                    {
                       a->mount = EINA_TRUE;
                       a->dev = strdup(dev);
                       a->mountpoint = strdup(mnt);
                       a->cmd = strdup(cmd0);
                       a->cmd2 = strdup(cmd2);
                       a->cmd1 = strdup(cmd1);
                       if ((!a->dev) || (!a->mountpoint) ||
                           (!a->cmd) || (!a->cmd2) || (!a->cmd1))
                         _store_action_free(a);
                       else _store_action_queue(a);
                    }
               }
          }
        free(mnt2);
     }
}

static void
_cb_store_umount(void *data EINA_UNUSED, const char *params)
{
   // params:
   // /media/user/xxx
   char cmd[4096];

   if (!_store_umount_verify(params))
     {
        ERR("Invalid mount [%s]\n", params);
        return;
     }
   if (snprintf(cmd, sizeof(cmd), "umount %s", params) < (int)(sizeof(cmd) - 1))
     {
        Action *a = calloc(1, sizeof(Action));
        if (a)
          {
             a->umount = EINA_TRUE;
             a->mountpoint = strdup(params);
             a->cmd = strdup(cmd);
             if ((!a->mountpoint) || (!a->cmd)) _store_action_free(a);
             else _store_action_queue(a);
          }
     }
}

void
e_system_storage_init(void)
{
   e_system_inout_command_register("store-eject",  _cb_store_eject, NULL);
   e_system_inout_command_register("store-mount",  _cb_store_mount, NULL);
   e_system_inout_command_register("store-umount", _cb_store_umount, NULL);
}

void
e_system_storage_shutdown(void)
{
   // only shutdown things we really have to - no need to free mem etc.
}
