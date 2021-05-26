#include "e_system.h"

typedef struct
{
   void (*func) (void *data, const char *params);
   void *data;
} Handler;

static Eina_Hash *_cmd_handlers = NULL;
static Ecore_Fd_Handler *_fdh_in = NULL;

//// note: commands are of the form:
// [ 24 bytes] command string including nul byte terminator and 0 padding
// [  4 bytes] payload size (max size 2gb)
// [  N bytes] payload data (0 or more)...

typedef struct
{
   char cmd[24];
   int  size;
} Message_Head;

static int fd_null = -1, fd_supress = -1;

static void
_stdout_off(void)
{
   fflush(stdout);
   fd_supress = dup(1);
   if (fd_null == -1) fd_null = open("/dev/null", O_WRONLY);
   if (fd_null != -1) dup2(fd_null, 1);
}

static void
_stdout_on(void)
{
   fflush(stdout);
   dup2(fd_supress, 1);
   close(fd_supress);
   close(fd_null);
   fd_supress = -1;
   fd_null = -1;
}

static Eina_Bool
_cb_stdio_in_read(void *data EINA_UNUSED, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Handler *h;
   Message_Head head;
   ssize_t ret;

   errno = 0;
   ret = read(0, &head, sizeof(head));
   if (ret < 1)
     {
        int e = errno;
        if ((e == EIO) || (e == EBADF) || (e == EPIPE) ||
            (e == EINVAL) || (e == ENOSPC) ||
            (!((e == EAGAIN) || (e == EINTR))))
          {
             ecore_main_loop_quit();
             goto done;
          }
        goto done;
     }
   else
     {
        char *buf;

        if (head.size < 0)
          {
             ERR("Invalid message payload size (less than 0)\n");
             abort();
          }
        if (head.size > (1024 * 1024))
          {
             ERR("Invalid message payload size (more than 1M)\n");
             abort();
          }
        buf = NULL;
        if (head.size > 0)
          {
             buf = malloc(head.size);
             if (!buf)
               {
                  ERR("Out of memory for message of size %i bytes\n", head.size);
                  abort();
               }
             ret = read(0, buf, head.size);
             if (ret != head.size)
               {
                  ERR("Cannot read full message payload of %i bytes\n", head.size);
                  abort();
               }
          }
        h = eina_hash_find(_cmd_handlers, head.cmd);
        if (h)
          {
             if ((!buf) || ((buf) && (buf[head.size - 1] == '\0')))
             h->func(h->data, buf);
          }
        free(buf);
     }
done:
   return EINA_TRUE;
}

void
e_system_inout_init(void)
{
   _stdout_off();
   _cmd_handlers = eina_hash_string_superfast_new(free);
   _fdh_in = ecore_main_fd_handler_add(0, ECORE_FD_READ,
                                       _cb_stdio_in_read, NULL,
                                       NULL, NULL);
}

void
e_system_inout_shutdown(void)
{
   ecore_main_fd_handler_del(_fdh_in);
   _fdh_in = NULL;
   eina_hash_free(_cmd_handlers);
   _cmd_handlers = NULL;
   _stdout_on();
}

void
e_system_inout_command_register(const char *cmd, void (*func) (void *data, const char *params), void *data)
{
   Handler *h = malloc(sizeof(Handler));
   size_t len = strlen(cmd);
   if (!h)
     {
        ERR("Out of memory registering command handlers\n");
        abort();
     }
   if (len > 23)
     {
        ERR("Trying to register command of length %i (max 23)\n", (int)len);
        abort();
     }
   h->func = func;
   h->data = data;
   eina_hash_add(_cmd_handlers, cmd, h);
}

void EINA_PRINTF(2, 3)
e_system_inout_command_send(const char *cmd, const char *fmt, ...)
{
   char *buf = NULL, stack_buf[4096];
   Message_Head head;
   size_t len = strlen(cmd);
   int printed = 0;
   ssize_t ret;
   va_list ap;

   if (len > 23)
     {
        ERR("Trying to send command of length %i (max 23)\n", (int)len);
        abort();
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
   ret = write(fd_supress, &head, sizeof(head));
   if (ret != sizeof(head))
     {
        ERR("Write of command failed at %lli\n", (long long)ret);
        abort();
     }
   if ((buf) && (head.size > 0))
     {
        ret = write(fd_supress, buf, head.size);
        if (ret != (ssize_t)head.size)
          {
             ERR("Write of command buffer failed at %lli/%llu\n", (long long)ret, (unsigned long long)head.size);
             abort();
          }
     }
end:
   if (buf != stack_buf) free(buf);
}
