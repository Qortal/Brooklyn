#ifndef E_TYPEDEFS

#ifndef _E_NOTIFICATION_H
#define _E_NOTIFICATION_H

#define E_NOTIFICATION_TYPE 0x12342166

typedef enum _E_Notification_Notify_Urgency
{
  E_NOTIFICATION_NOTIFY_URGENCY_LOW,
  E_NOTIFICATION_NOTIFY_URGENCY_NORMAL,
  E_NOTIFICATION_NOTIFY_URGENCY_CRITICAL
} E_Notification_Notify_Urgency;

typedef enum _E_Notification_Notify_Closed_Reason
{
  E_NOTIFICATION_NOTIFY_CLOSED_REASON_EXPIRED, /** The notification expired. */
  E_NOTIFICATION_NOTIFY_CLOSED_REASON_DISMISSED, /** The notification was dismissed by the user. */
  E_NOTIFICATION_NOTIFY_CLOSED_REASON_REQUESTED, /** The notification was closed by a call to CloseNotification method. */
  E_NOTIFICATION_NOTIFY_CLOSED_REASON_UNDEFINED /** Undefined/reserved reasons. */
} E_Notification_Notify_Closed_Reason;

typedef struct _E_Notification_Notify
{
   E_Object e_obj_inherit;
   unsigned int id;
   const char *app_name;
   unsigned replaces_id;
   const char *summary;
   const char *body;
   int timeout;
   E_Notification_Notify_Urgency urgency;
   struct
   {
      const char *icon;
      const char *icon_path;
      struct
      {
         int width;
         int height;
         int rowstride;
         Eina_Bool has_alpha;
         int bits_per_sample;
         int channels;
         unsigned char *data;
         int data_size;
      } raw;
   } icon;
} E_Notification_Notify;

typedef unsigned int (*E_Notification_Notify_Cb)(void *data, E_Notification_Notify *n);
typedef void (*E_Notification_Close_Cb)(void *data, unsigned int id);

typedef struct _E_Notification_Server_Info
{
   const char *name;
   const char *vendor;
   const char *version;
   const char *spec_version;
   const char *capabilities[];
} E_Notification_Server_Info;

/**
 * Register a notification server
 *
 * It's only possible to have one server registered at a time. If this function
 * is called twice it will return EINA_FALSE on the second time.
 *
 * @return EINA_TRUE if server was registered, EINA_FALSE otherwise.
 */
E_API Eina_Bool e_notification_server_register(const E_Notification_Server_Info *server_info, E_Notification_Notify_Cb notify_cb, E_Notification_Close_Cb close_cb, const void *data);

/**
 * Unregister the sole notification server
 */
E_API void e_notification_server_unregister(void);

E_API void e_notification_notify_close(E_Notification_Notify *notify, E_Notification_Notify_Closed_Reason reason);
E_API Evas_Object *e_notification_notify_raw_image_get(E_Notification_Notify *notify, Evas *evas);

//client
typedef void (*E_Notification_Client_Send_Cb)(void *data, unsigned int id);
E_API Eina_Bool e_notification_client_send(E_Notification_Notify *notify, E_Notification_Client_Send_Cb cb, const void *data);
E_API Eina_Bool e_notification_util_send(const char *summary, const char *body);
#endif

#endif
