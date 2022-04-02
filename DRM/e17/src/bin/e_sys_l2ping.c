#include "config.h"

#include <Ecore.h>

#ifdef HAVE_BLUETOOTH
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <sys/select.h>
#include <fcntl.h>
#endif

#define MAX_SZ 500

double
e_sys_l2ping(const char *bluetooth_mac, int timeout_ms)
{
#ifdef HAVE_BLUETOOTH
   char send_buf[L2CAP_CMD_HDR_SIZE + MAX_SZ];
   char recv_buf[L2CAP_CMD_HDR_SIZE + MAX_SZ];
   char tmp[18];
   bdaddr_t bdaddr;
   l2cap_cmd_hdr *send_cmd;
   l2cap_cmd_hdr *recv_cmd;
   struct sockaddr_l2 addr;
   socklen_t optlen;
   double start;
   int fd, err, size, i;
   fd_set rfds, wfds, exfds;
   struct timeval tv;

   // Create socket
   fd = socket(PF_BLUETOOTH, SOCK_RAW | SOCK_NONBLOCK, BTPROTO_L2CAP);
   if (fd < 0)
     {
        perror("Can't create socket");
        return -1;
     }

   // Bind to local address
   memset(&addr, 0, sizeof(addr));
   addr.l2_family = AF_BLUETOOTH;
   bacpy(&bdaddr, BDADDR_ANY);
   bacpy(&addr.l2_bdaddr, &bdaddr);
   if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
     {
        perror("Can't bind socket");
        close(fd);
        return -1;
     }

   if (fcntl(fd, F_SETFL, O_NONBLOCK) != 0)
     {
        perror("Can't set socket to non-blocking... continue");
     }

   // Connect to remote device
   memset(&addr, 0, sizeof(addr));
   addr.l2_family = AF_BLUETOOTH;
   str2ba(bluetooth_mac, &addr.l2_bdaddr);
   if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
     {
        perror("Can't bind connect socket");
        close(fd);
        return -1;
     }

   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   FD_ZERO(&exfds);
   FD_SET(fd, &wfds);
   tv.tv_sec = timeout_ms / 1000;
   tv.tv_usec = (timeout_ms % 1000) * 1000;
   start = ecore_time_get();
   err = select(fd + 1, &rfds, &wfds, &exfds, &tv);
   if (err == 0)
     {
        fprintf(stderr, "Connect timeout %s\n", bluetooth_mac);
        return -1;
     }
   // adjust timeout by how long we waited to connect
   timeout_ms -= (ecore_time_get() - start) * 1000;
   if (timeout_ms < 1) timeout_ms = 1;

   // Get local address
   memset(&addr, 0, sizeof(addr));
   optlen = sizeof(addr);
   if (getsockname(fd, (struct sockaddr *)&addr, &optlen) < 0)
     {
        perror("Can't get local address");
        return -1;
     }

   ba2str(&addr.l2_bdaddr, tmp);

   size = 44; // use std 44 byte ping size, but no more than MAX_SZ

   send_cmd = (l2cap_cmd_hdr *)send_buf;
   send_cmd->ident = 200;
   send_cmd->len = htobs(size);
   send_cmd->code = L2CAP_ECHO_REQ;
   // init buffer with some content
   for (i = 0; i < size; i++)
     {
        // ABCDEF....WXYZABCEF... up to "size" chars
        send_buf[L2CAP_CMD_HDR_SIZE + i] = 'A' + (i % 26);
     }
   // get our time just before a send
   start = ecore_time_get();
   // send the ping
   if (send(fd, send_buf, L2CAP_CMD_HDR_SIZE + size, 0) <= 0)
     {
        perror("Send failed");
        return -1;
     }

   do
     {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&exfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        err = select(fd + 1, &rfds, &wfds, &exfds, &tv);
        if (err < 0)
          {
             perror("Select failed");
             return -1;
          }
        else if (err == 0)
          {
             fprintf(stderr, "Select timeout %s\n", bluetooth_mac);
             return -1;
          }
        err = recv(fd, recv_buf, L2CAP_CMD_HDR_SIZE + size, 0);
        if (err == 0)
          {
             fprintf(stderr, "Disconnect %s\n", bluetooth_mac);
             return -1;
          }
        else if (err < 0)
          {
             perror("Recv failed");
             return -1;
          }

        recv_cmd = (l2cap_cmd_hdr *)recv_buf;
        recv_cmd->len = btohs(recv_cmd->len);
        // we only want the 200 ident response packets
        if (recv_cmd->ident != 200) continue;
        if (recv_cmd->code == L2CAP_COMMAND_REJ)
          {
             fprintf(stderr, "Peer %s doesn't do echo\n", bluetooth_mac);
             return -1;
          }
        if (recv_cmd->len != size)
          {
             fprintf(stderr, "Size %i echo for %s does not match %i\n",
                     recv_cmd->len, bluetooth_mac, size);
             return -1;
          }
        if (memcmp(send_buf + L2CAP_CMD_HDR_SIZE,
                   recv_buf + L2CAP_CMD_HDR_SIZE, size) != 0)
          {
             fprintf(stderr, "Echo response for %s data does not match sent data\n", bluetooth_mac);
             return -1;
          }
        fprintf(stderr, "Device %s responded\n", bluetooth_mac);
        break;
     }
   while (1);

   close(fd);
   // time it took to send and get our response
   return ecore_time_get() - start;
#else
   (void) bluetooth_mac;
   fprintf(stderr, "e_sys_l2ping nop mac=%s timeout=%ims\n", bluetooth_mac, timeout_ms);
   return -1;
#endif
}
