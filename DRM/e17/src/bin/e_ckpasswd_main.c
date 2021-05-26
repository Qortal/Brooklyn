#include "e_util_suid.h"

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus.h>

static uid_t uid = -1; // uid of person running me
static gid_t gid = -1; // gid of person running me
static char *user_name = NULL;
static char *group_name = NULL;

#if defined(__OpenBSD__)

static int
_check_auth(uid_t id, const char *guess)
{
   struct passwd *pwent;

   pwent = getpwuid_shadow(id);
   if (!pwent) return -1;
   if (!pwent->pw_passwd) return -1;

   return crypt_checkpass(guess, pwent->pw_passwd);
}





#elif defined(__FreeBSD__)
#include <security/pam_constants.h>

static int
_check_auth(uid_t id, const char *pw)
{
   struct passwd *pwent = getpwuid(id);

   if (!pwent) return -1;
   if (!pwent->pw_passwd) return -1;

   if (!strcmp(crypt(pw, pwent->pw_passwd), pwent->pw_passwd)) return 0;
   return -1;
}





#elif defined(HAVE_PAM)
# include <security/pam_appl.h>

typedef struct
{
   const char *user;
   const char *pw;
} Authinfo;

static int
_conv_cb(int num, const struct pam_message **msg, struct pam_response **resp, void *data)
{
   Authinfo *ai = data;
   int replies;
   struct pam_response *reply = NULL;

   reply = malloc(sizeof(struct pam_response) * num);
   if (!reply) return PAM_CONV_ERR;

   for (replies = 0; replies < num; replies++)
     {
        switch (msg[replies]->msg_style)
          {
           case PAM_PROMPT_ECHO_ON:
             reply[replies].resp_retcode = PAM_SUCCESS;
             reply[replies].resp = strdup(ai->user);
             break;
           case PAM_PROMPT_ECHO_OFF:
             reply[replies].resp_retcode = PAM_SUCCESS;
             reply[replies].resp = strdup(ai->pw);
             break;
           case PAM_ERROR_MSG:
           case PAM_TEXT_INFO:
             reply[replies].resp_retcode = PAM_SUCCESS;
             reply[replies].resp = NULL;
             break;
           default:
             free(reply);
             return PAM_CONV_ERR;
          }
     }
   *resp = reply;
   return PAM_SUCCESS;
}

static int
_check_auth(uid_t id, const char *pw)
{
   Authinfo ai;
   struct passwd *pwent;
   const char *user;
   const char *prof;
   const char *host;
   struct stat st;
   pam_handle_t *handle;
   int pamerr;
   struct pam_conv conv;

   pwent = getpwuid(id);
   if (!pwent) return -1;
   user = pwent->pw_name;
   if (!user) return -1;

   host = "localhost";

   prof = "login";
   if      (!stat("/etc/pam.d/enlightenment", &st)) prof = "enlightenment";
   else if (!stat("/etc/pam.d/xscreensaver", &st))  prof = "xscreensaver";
   else if (!stat("/etc/pam.d/kscreensaver", &st))  prof = "kscreensaver";
   else if (!stat("/etc/pam.d/system-auth", &st))   prof = "system-auth";
   else if (!stat("/etc/pam.d/system", &st))        prof = "system";
   else if (!stat("/etc/pam.d/xdm", &st))           prof = "xdm";
   else if (!stat("/etc/pam.d/gdm", &st))           prof = "gdm";
   else if (!stat("/etc/pam.d/kdm", &st))           prof = "kdm";

   ai.user = user;
   ai.pw = pw;

   conv.conv = _conv_cb;
   conv.appdata_ptr = &ai;
   if (pam_start(prof, user, &conv, &handle) != PAM_SUCCESS) return -1;
   if (pam_set_item(handle, PAM_USER, user) != PAM_SUCCESS) return -1;
   if (pam_set_item(handle, PAM_RHOST, host) != PAM_SUCCESS) return -1;

   pamerr = pam_authenticate(handle, 0);
   pam_end(handle, pamerr);

   if (pamerr != PAM_SUCCESS) return -1;

   return 0;
}





#else

static int
_check_auth(uid_t id, const char *pw)
{
   return -1;
}





#endif

static int polkit_auth_ok = -1;

static void
polkit_agent_response(void *data EINA_UNUSED, const Eldbus_Message *msg,
                      Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;

   ecore_main_loop_quit();
   if (eldbus_message_error_get(msg, &name, &text))
     {
        fprintf(stderr, "AUTH: Could not respond to auth.\n %s:\n %s\n", name, text);
        return;
     }
   polkit_auth_ok = 0;
   fprintf(stderr, "AUTH: OK\n");
}

int
polkit_auth(const char *cookie, unsigned int auth_uid)
{
   Eldbus_Connection *c;
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   Eldbus_Message *m;
   Eldbus_Message_Iter *iter, *subj, *array, *dict, *vari;

   eina_init();
   ecore_init();
   eldbus_init();
   c = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
#define BARF(str) do { \
   fprintf(stderr, "AUTH: POLKIT: %s\n", str); \
   return -1; \
} while (0)
   if (!c) BARF("Cannot get session dbus");
   obj = eldbus_object_get(c, "org.freedesktop.PolicyKit1",
                           "/org/freedesktop/PolicyKit1/Authority");
   if (!obj) BARF("Cannot get obj: org.freedesktop.PolicyKit1 /org/freedesktop/PolicyKit1/Authority");
   proxy = eldbus_proxy_get(obj, "org.freedesktop.PolicyKit1.Authority");
   if (!proxy) BARF("Cannot proxy: org.freedesktop.PolicyKit1.Authority");
   m = eldbus_proxy_method_call_new(proxy, "AuthenticationAgentResponse2");
   if (!m) BARF("Cannot get method call: AuthenticationAgentResponse2");
   iter = eldbus_message_iter_get(m);
   if (!iter) BARF("Cannot set iter on proxy");
   if (!eldbus_message_iter_arguments_append(iter, "us", uid, cookie))
     BARF("Cannot append 'us' args");
   if (!eldbus_message_iter_arguments_append(iter, "(sa{sv})", &subj))
     BARF("Cannot append '(sa{sv})' args");
   if (!eldbus_message_iter_basic_append(subj, 's', "unix-user"))
     BARF("Cannot append 's' arg for unix-user");
   if (!eldbus_message_iter_arguments_append(subj, "a{sv}", &array))
     BARF("Cannot append 'a{sv}' args");
   if (!eldbus_message_iter_arguments_append(array, "{sv}", &dict))
     BARF("Cannot append '{sv}' args");
   if (!eldbus_message_iter_basic_append(dict, 's', "uid"))
     BARF("Cannot append 's' arg for uid");
   vari = eldbus_message_iter_container_new(dict, 'v', "u");
   if (!vari)
     BARF("Cannot create new iter container");
   if (!eldbus_message_iter_basic_append(vari, 'u', auth_uid))
     BARF("Cannot append 'u' arg for auth_uid");
   eldbus_message_iter_container_close(dict, vari);
   eldbus_message_iter_container_close(array, dict);
   eldbus_message_iter_container_close(subj, array);
   eldbus_message_iter_container_close(iter, subj);
   eldbus_proxy_send(proxy, m, polkit_agent_response, NULL, -1);

   ecore_main_loop_begin();

   eldbus_connection_unref(c);
   eldbus_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return polkit_auth_ok;
}

int
main(int argc, char **argv)
{
   ssize_t rd;
   char pw[4096], *p;
   int polkit_mode = 0;
   char polkit_cookie[4096];
   unsigned int polkit_uid = 0;

   if (argc < 2)
     {
        fprintf(stderr, "This is an internal tool for Enlightenment\n");
        fprintf(stderr, "Options: pw | pk\n");
        goto err;
     }
   if      (!strcmp(argv[1], "pw")) polkit_mode = 0;
   else if (!strcmp(argv[1], "pk")) polkit_mode = 1;

   // read passwd from stdin
   if (polkit_mode == 0)
     {
        rd = read(0, pw, 3);
        if (rd != 3)
          {
             fprintf(stderr, "AUTH: Error. Can't read passwd preable\n");
             goto err;
          }
        if (!((pw[0] == 'p') && (pw[1] == 'w') && (pw[2] == ' ')))
          {
             fprintf(stderr, "AUTH: Error. Preable is wrong\n");
             goto err;
          }
        rd = read(0, pw, sizeof(pw) - 1);
        if (rd < 0)
          {
             fprintf(stderr, "AUTH: Error. Can't read passwd on stdin\n");
             goto err;
          }
        pw[rd] = 0;
        for (p = pw; *p; p++)
          {
             if ((*p == '\r') || (*p == '\n'))
               {
                  *p = 0;
                  break;
               }
          }
     }
   else if (polkit_mode == 1)
     {
        unsigned int pos = 0;

        // read "cookie-string uid-string password...[\r|\n|EOF]"
        for (;;) // cookie
          {
             rd = read(0, pw + pos, 1);
             if (rd < 0)
               {
                  fprintf(stderr, "AUTH: POLKIT: Error. Can't read polkit cookie on stdin\n");
                  goto err;
               }
             if (pw[pos] == ' ')
               {
                  memcpy(polkit_cookie, pw, pos);
                  polkit_cookie[pos] = 0;
                  fprintf(stderr, "AUTH: POLKIT: [%s]\n", polkit_cookie);
                  pos = 0;
                  break;
               }
             else
               {
                  pos++;
                  if (pos > 4000)
                    {
                       fprintf(stderr, "AUTH: POLKIT: Error. Polkit cookie too long\n");
                       return -10;
                    }
               }
          }
        for (;;) // uid
          {
             rd = read(0, pw + pos, 1);
             if (rd < 0)
               {
                  fprintf(stderr, "AUTH: Error. Can't read polkit uid on stdin\n");
                  goto err;
               }
             if (pw[pos] == ' ')
               {
                  pw[pos] = 0;
                  polkit_uid = atoi(pw);
                  fprintf(stderr, "AUTH: UID: [%u]\n", polkit_uid);
                  break;
               }
             else
               {
                  pos++;
                  if (pos > 4000)
                    {
                       fprintf(stderr, "AUTH: Error. Polkit uid too long\n");
                       return -11;
                    }
               }
          }
        // password
        fprintf(stderr, "AUTH: readpass...\n");
        rd = read(0, pw, sizeof(pw) - 1);
        if (rd < 0)
          {
             fprintf(stderr, "AUTH: Error. Can't read passwd on stdin\n");
             goto err;
          }
        pw[rd] = 0;
        for (p = pw; *p; p++)
          {
             if ((*p == '\r') || (*p == '\n'))
               {
                  *p = 0;
                  break;
               }
          }
     }

   // ok to fail - auth will just possibly fail then
   e_setuid_setup(&uid, &gid, &user_name, &group_name);

   if (_check_auth(polkit_mode ? polkit_uid : uid, pw) == 0)
     {
        fprintf(stderr, "AUTH: Password OK\n");
        if (polkit_mode == 1)
          {
             if (polkit_auth(polkit_cookie, polkit_uid) == 0)
               {
                  fprintf(stderr, "AUTH: Polkit AuthenticationAgentResponse2 success\n");
                  return 0;
               }
             fprintf(stderr, "AUTH: Polkit AuthenticationAgentResponse2 failure\n");
             return -2;
          }
        return 0;
     }
err:
   fprintf(stderr, "AUTH: Password auth fail\n");
   return -1;
}
