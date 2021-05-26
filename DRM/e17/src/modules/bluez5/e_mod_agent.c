#include "e_mod_main.h"

/////////////////////////////////////////////////////////////////////////////

static void
cb_get_pass(Eldbus_Message *msg, const char *str)
{
   if (!str) return;
   bz_agent_msg_str_add(msg, str);
}

static void
cb_get_pin(Eldbus_Message *msg, const char *str)
{
   unsigned int pin;
   if (!str) return;
   pin = atoi(str);
   bz_agent_msg_u32_add(msg, pin);
}

void
ebluez5_agent_agent_release(void)
{
   // just debugging
   printf("BZ5 Agent Release\n");
}

void
ebluez5_agent_agent_cancel(void)
{
   // just debugging
   printf("BZ5 Agent Cancel\n");
}

void
ebluez5_agent_agent_req_pin(Eldbus_Message *msg)
{
   const char *path;
   Obj *o;

   if (!(path = bz_agent_msg_path(msg))) goto error;
   if (!(o = bz_obj_find(path))) goto error;
   bz_obj_agent_request(o, _("Supply PIN"), cb_get_pass,
                        bz_agent_msg_ok(msg), bz_agent_msg_err(msg));
   return;
error:
   bz_agent_msg_reply(bz_agent_msg_err(msg));
}

void
ebluez5_agent_agent_disp_pin(Eldbus_Message *msg)
{
   const char *path, *s = "";
   Obj *o;
   char buf[1024];

   if (!(path = bz_agent_msg_path_str(msg, &s))) goto error;
   if (!(o = bz_obj_find(path))) goto error;
   snprintf(buf, sizeof(buf), _("Pair? PIN: <hilight>%s</hilight>"), s);
   bz_obj_agent_request(o, buf, NULL,
                        bz_agent_msg_ok(msg), bz_agent_msg_err(msg));
   return;
error:
   bz_agent_msg_reply(bz_agent_msg_err(msg));
}

void
ebluez5_agent_req_pass(Eldbus_Message *msg)
{
   const char *path;
   Obj *o;

   if (!(path = bz_agent_msg_path(msg))) goto error;
   if (!(o = bz_obj_find(path))) goto error;
   bz_obj_agent_request(o, _("Enter PIN"), cb_get_pin,
                        bz_agent_msg_ok(msg), bz_agent_msg_err(msg));
   return;
error:
   bz_agent_msg_reply(bz_agent_msg_err(msg));
}

void
ebluez5_agent_disp_pass(Eldbus_Message *msg)
{
   const char *path;
   unsigned int pin = 0;
   unsigned short entered = 0;
   Obj *o;
   char buf[1024];

   if (!(path = bz_agent_msg_path_u32_u16(msg, &pin, &entered))) goto error;
   if (!(o = bz_obj_find(path))) goto error;
   snprintf(buf, sizeof(buf), _("Pair? PIN: <hilight>%06u</hilight>"), pin);
   bz_obj_agent_request(o, buf, NULL,
                        bz_agent_msg_ok(msg), bz_agent_msg_err(msg));
   return;
error:
   bz_agent_msg_reply(bz_agent_msg_err(msg));
}

void
ebluez5_agent_req_confirm(Eldbus_Message *msg)
{
   const char *path;
   unsigned int pin = 0;
   Obj *o;
   char buf[1024];

   if (!(path = bz_agent_msg_path_u32(msg, &pin))) goto error;
   if (!(o = bz_obj_find(path))) goto error;
   snprintf(buf, sizeof(buf), _("Pair? PIN: <hilight>%06u</hilight>"), pin);
   bz_obj_agent_request(o, buf, NULL,
                        bz_agent_msg_ok(msg), bz_agent_msg_err(msg));
   return;
error:
   bz_agent_msg_reply(bz_agent_msg_err(msg));
}

void
ebluez5_agent_req_auth(Eldbus_Message *msg)
{
   const char *path;
   Obj *o;

   if (!(path = bz_agent_msg_path(msg))) goto error;
   if (!(o = bz_obj_find(path))) goto error;
   bz_obj_agent_request(o, _("Connect?"), NULL,
                        bz_agent_msg_ok(msg), bz_agent_msg_err(msg));
   return;
error:
   bz_agent_msg_reply(bz_agent_msg_err(msg));
}

void
ebluez5_agent_auth_service(Eldbus_Message *msg)
{
   // if "ask for service" on then ask, otherwise always auth
   // always auth:
//   if (!(path = bz_agent_msg_path_str(msg, &s))) goto error;
   Eldbus_Message *ok;
   ok = bz_agent_msg_ok(msg);
   bz_agent_msg_reply(ok);
}
