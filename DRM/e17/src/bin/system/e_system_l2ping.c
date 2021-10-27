#include "e_system.h"

#ifdef HAVE_BLUETOOTH
# include <bluetooth/bluetooth.h>
# include <bluetooth/l2cap.h>
# include <sys/socket.h>
# include <sys/select.h>
#endif

#define MAX_SZ 500

typedef struct
{
   char *dev;
   int timeout, result;
} Action;

static void
_l2ping_free(Action *a)
{
   free(a->dev);
   free(a);
}

#ifdef HAVE_BLUETOOTH
static void
_l2ping_l2addr_init(struct sockaddr_l2 *ad)
{
   memset(ad, 0, sizeof(*ad));
   ad->l2_family = AF_BLUETOOTH;
}

# define SETUP_FDSET(rfds, wfds, exfds) \
   FD_ZERO(&rfds); \
   FD_ZERO(&wfds); \
   FD_ZERO(&exfds);
# define SETUP_TIMEOUT(tv, a) \
   tv.tv_sec = a->timeout / 1000; \
   tv.tv_usec = (a->timeout % 1000) * 1000;
#endif

static void
_cb_l2ping(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
#ifdef HAVE_BLUETOOTH
   Action *a = data;
   char buf1[L2CAP_CMD_HDR_SIZE + MAX_SZ], buf2[L2CAP_CMD_HDR_SIZE + MAX_SZ];
   bdaddr_t ba;
   l2cap_cmd_hdr *cmd;
   struct sockaddr_l2 ad;
   double start;
   int fd, err, size, i;
   fd_set rfds, wfds, exfds;
   struct timeval tv;

   fd = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
   if (fd < 0)
     {
        ERR("l2ping: Can't create socket\n");
        return;
     }

   // bind to local address
   _l2ping_l2addr_init(&ad);
   bacpy(&ba, BDADDR_ANY);
   bacpy(&ad.l2_bdaddr, &ba);
   if (bind(fd, (struct sockaddr *)&ad, sizeof(ad)) < 0)
     {
        ERR("l2ping: Can't bind socket\n");
        goto err;
     }

   // connect to remote device
   _l2ping_l2addr_init(&ad);
   str2ba(a->dev, &ad.l2_bdaddr);
   if (connect(fd, (struct sockaddr *)&ad, sizeof(ad)) < 0)
     {
        ERR("l2ping: Can't connect socket to [%s]\n", a->dev);
        goto err;
     }

   SETUP_FDSET(rfds, wfds, exfds);
   FD_SET(fd, &wfds);
   SETUP_TIMEOUT(tv, a);
   start = ecore_time_get();
   err = select(fd + 1, &rfds, &wfds, &exfds, &tv);
   if (err == 0)
     {
        ERR("l2ping: Connect timeout [%s]\n", a->dev);
        goto err;
     }

   // adjust timeout by how long we waited to connect
   a->timeout -= (ecore_time_get() - start) * 1000;
   if (a->timeout < 1) a->timeout = 1;

   size = 44; // use std 44 byte ping size, but no more than MAX_SZ
   cmd = (l2cap_cmd_hdr *)buf1;
   cmd->ident = 200;
   cmd->len = htobs(size);
   cmd->code = L2CAP_ECHO_REQ;
   // init buffer with some content
   // ABCDEF....WXYZABCEF... up to "size" chars
   for (i = 0; i < size; i++) buf1[L2CAP_CMD_HDR_SIZE + i] = 'A' + (i % 26);

   // get our time just before a send
   start = ecore_time_get();
   // send the ping
   if (send(fd, buf1, L2CAP_CMD_HDR_SIZE + size, 0) <= 0)
     {
        ERR("l2ping: Send to [%s] failed\n", a->dev);
        goto err;
     }

   // wait for the reply to this ping
   for (;;)
     {
        SETUP_FDSET(rfds, wfds, exfds);
        FD_SET(fd, &rfds);
        SETUP_TIMEOUT(tv, a);
        err = select(fd + 1, &rfds, &wfds, &exfds, &tv);
        if (err == 0)
          {
             ERR("l2ping: Select timeout [%s]\n", a->dev);
             goto err;
          }
        else if (err < 0)
          {
             ERR("l2ping: Select for [%s] failed\n", a->dev);
             goto err;
          }

        err = recv(fd, buf2, L2CAP_CMD_HDR_SIZE + size, 0);
        if (err == 0)
          {
             ERR("l2ping: Disconnect %s\n", a->dev);
             goto err;
          }
        else if (err < 0)
          {
             ERR("l2ping: Recv [%s] failed\n", a->dev);
             goto err;
          }

        cmd = (l2cap_cmd_hdr *)buf2;
        cmd->len = btohs(cmd->len);
        // we only want the 200 ident response packets
        if (cmd->ident != 200) continue;
        if (cmd->code == L2CAP_COMMAND_REJ)
          {
             ERR("l2ping: [%s] doesn't do echo\n", a->dev);
             goto err;
          }
        if (cmd->len != size)
          {
             ERR("l2ping: Size %i echo for [%s] does not match %i\n",
                 (int)cmd->len, a->dev, (int)size);
             goto err;
          }
        if (memcmp(buf1 + L2CAP_CMD_HDR_SIZE, buf2 + L2CAP_CMD_HDR_SIZE,
                   size) != 0)
          {
             ERR("l2ping: Echo response from [%s] does not match sent data\n",
                 a->dev);
             goto err;
          }
        break;
     }
   // time it took to send and get our response
   a->result = (ecore_time_get() - start) * 1000.0;
err:
   close(fd);
#else
   ERR("l2ping: Bluetooth support not compiled in\n");
#endif
}

static void
_cb_l2ping_end(void *data, Ecore_Thread *th EINA_UNUSED)
{
   Action *a = data;
   e_system_inout_command_send("l2ping-ping", "%s %i", a->dev, a->result);
   _l2ping_free(a);
}

static void
_cb_l2ping_cancel(void *data, Ecore_Thread *th EINA_UNUSED)
{
   Action *a = data;
   e_system_inout_command_send("l2ping-ping", "%s %i", a->dev, -1);
   _l2ping_free(a);
}

static void
_cb_l2ping_ping(void *data EINA_UNUSED, const char *params)
{
   // ADDR TIMEOUT_MS
   int timeout = 1;
   char dev[1024];
   Action *a;

   if (! params) return;
   if (sscanf(params, "%1023s %i", dev, &timeout) != 2) return;
   if (timeout < 1) timeout = 1;
   else if (timeout > (1000 * 600)) timeout = 1000 * 600;
   a = calloc(1, sizeof(Action));
   if (!a) return;
   a->dev = strdup(dev);
   if (!a->dev) goto err;
   a->timeout = timeout;
   a->result = -1;
   if (ecore_thread_feedback_run(_cb_l2ping, NULL,
                                 _cb_l2ping_end, _cb_l2ping_cancel,
                                 a, EINA_TRUE))
     return;
err:
   _l2ping_free(a);
}

void
e_system_l2ping_init(void)
{
   e_system_inout_command_register("l2ping-ping", _cb_l2ping_ping, NULL);
}

void
e_system_l2ping_shutdown(void)
{
   // only shutdown things we really have to - no need to free mem etc.
}
