# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

#ifdef __linux__
# include <features.h>
#endif
#include <unistd.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_File.h>
#include <Efreet.h>
#include <Eldbus.h>

# ifdef E_API
#  undef E_API
# endif
# ifdef WIN32
#  ifdef BUILDING_DLL
#   define E_API __declspec(dllexport)
#  else
#   define E_API __declspec(dllimport)
#  endif
# else
#  ifdef __GNUC__
#   if __GNUC__ >= 4
/* BROKEN in gcc 4 on amd64 */
#    if 0
#     pragma GCC visibility push(hidden)
#    endif
#    define E_API __attribute__ ((visibility("default")))
#   else
#    define E_API
#   endif
#  else
#   define E_API
#  endif
# endif

static Eldbus_Connection *conn = NULL;
static int retval = EXIT_SUCCESS;
static int pending = 0;

static void
fm_open_reply(void *data EINA_UNUSED, const Eldbus_Message *msg,
              Eldbus_Pending *dbus_pending EINA_UNUSED)
{
   const char *name, *txt;

   if (eldbus_message_error_get(msg, &name, &txt))
     {
        retval = EXIT_FAILURE;
        EINA_LOG_ERR("%s: %s", name, txt);
        fprintf(stderr, "Did you unload the EFM module? Try loading it.\n");
     }
   pending--;
   if (!pending) ecore_main_loop_quit();
}

static Eina_Bool
fm_error_quit_last(void *data EINA_UNUSED)
{
   if (!pending) ecore_main_loop_quit();
   return EINA_FALSE;
}

static void
fm_open(const char *path)
{
   Eldbus_Message *msg;
   const char *method;
   char *p;

   if (path[0] == '/')
     p = strdup(path);
   else if (strstr(path, "://"))
     {
        Efreet_Uri *uri = efreet_uri_decode(path);
        if ((!uri) || (!uri->protocol) || (strcmp(uri->protocol, "file") != 0))
          {
             EINA_LOG_ERR("Invalid URI '%s'", path);
             ecore_idler_add(fm_error_quit_last, NULL);
             if (uri)
               efreet_uri_free(uri);
             return;
          }

        p = strdup(uri->path);
        efreet_uri_free(uri);
     }
   else
     {
        char buf[PATH_MAX];
        if (!getcwd(buf, sizeof(buf)))
          {
             EINA_LOG_ERR("Could not get current working directory: %s", strerror(errno));
             ecore_idler_add(fm_error_quit_last, NULL);
             return;
          }
        if (strcmp(path, ".") == 0)
          p = strdup(buf);
        else
          {
             char tmp[PATH_MAX + PATH_MAX + 2];
             snprintf(tmp, sizeof(tmp), "%s/%s", buf, path);
             p = strdup(tmp);
          }
     }

   EINA_LOG_DBG("'%s' -> '%s'", path, p);
   if ((!p) || (p[0] == '\0'))
     {
        EINA_LOG_ERR("Could not get path '%s'", path);
        ecore_idler_add(fm_error_quit_last, NULL);
        free(p);
        return;
     }

   if (ecore_file_is_dir(p))
     method = "OpenDirectory";
   else
     method = "OpenFile";

   msg = eldbus_message_method_call_new("org.enlightenment.FileManager",
                                       "/org/enlightenment/FileManager",
                                       "org.enlightenment.FileManager",
                                       method);
   if (!msg)
     {
        EINA_LOG_ERR("Could not create DBus Message");
        ecore_idler_add(fm_error_quit_last, NULL);
        free(p);
        return;
     }
   eldbus_message_arguments_append(msg, "s", p);
   free(p);

   if (!eldbus_connection_send(conn, msg, fm_open_reply, NULL, -1))
     {
        EINA_LOG_ERR("Could not send DBus Message");
        eldbus_message_unref(msg);
        ecore_idler_add(fm_error_quit_last, NULL);
     }
   else
     pending++;
}

static const Ecore_Getopt options = {
   "enlightenment_filemanager",
   "%prog [options] [file-or-folder1] ... [file-or-folderN]",
   PACKAGE_VERSION,
   "(C) 2012 Gustavo Sverzut Barbieri and others",
   "BSD 2-Clause",
   "Opens the Enlightenment File Manager at a given folders.",
   EINA_FALSE,
   {
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('C', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

E_API int
main(int argc, char *argv[])
{
   Eina_Bool quit_option = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_NONE
   };
   int args;

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        EINA_LOG_ERR("Could not parse command line options.");
        return EXIT_FAILURE;
     }

   if (quit_option) return EXIT_SUCCESS;

   ecore_init();
   ecore_file_init();
   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   if (!conn)
     {
        EINA_LOG_ERR("Could not DBus SESSION bus.");
        retval = EXIT_FAILURE;
        goto end;
     }

   retval = EXIT_SUCCESS;

   if (args == argc) fm_open(".");
   else
     {
        for (; args < argc; args++)
          fm_open(argv[args]);
     }

   ecore_main_loop_begin();
   eldbus_connection_unref(conn);
end:
   eldbus_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   return retval;
}

