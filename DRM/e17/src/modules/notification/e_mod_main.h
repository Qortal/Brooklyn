#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include "e.h"

/* Increment for Major Changes */
#define MOD_CONFIG_FILE_EPOCH      1
/* Increment for Minor Changes (ie: user doesn't need a new config) */
#define MOD_CONFIG_FILE_GENERATION 1
#define MOD_CONFIG_FILE_VERSION    ((MOD_CONFIG_FILE_EPOCH * 1000000) + MOD_CONFIG_FILE_GENERATION)

typedef enum   _Popup_Corner Popup_Corner;
typedef struct _Config Config;
typedef struct _Popup_Data Popup_Data;

enum _Popup_Corner
 {
   CORNER_TL,
   CORNER_TR,
   CORNER_BL,
   CORNER_BR
 };

typedef enum
{
   POPUP_DISPLAY_POLICY_FIRST,
   POPUP_DISPLAY_POLICY_CURRENT,
   POPUP_DISPLAY_POLICY_ALL,
   POPUP_DISPLAY_POLICY_MULTI
} Popup_Display_Policy;

struct _Config
{
  E_Config_Dialog *cfd;

  int version;
  int show_low;
  int show_normal;
  int show_critical;
  int force_timeout;
  int ignore_replacement;
  Popup_Display_Policy dual_screen;
  float timeout;
  Popup_Corner corner;

  Eina_List  *popups;
  unsigned int         next_id;
};

struct _Popup_Data
{
  unsigned id;
  E_Notification_Notify *notif;
  Evas_Object *win;
  Eina_List *mirrors;
  Evas *e;
  Evas_Object *theme;
  const char  *app_name;
  Evas_Object *app_icon;
  Ecore_Timer *timer;
  Eina_Bool pending E_BITFIELD;
};


void notification_popup_notify(E_Notification_Notify *n, unsigned int id);
void notification_popup_shutdown(void);
void notification_popup_close(unsigned int id);

E_API extern E_Module_Api e_modapi;
E_API void  *e_modapi_init(E_Module *m);
E_API int    e_modapi_shutdown(E_Module *m);
E_API int    e_modapi_save(E_Module *m);

void _gc_orient    (E_Gadcon_Client *gcc, E_Gadcon_Orient orient);

E_Config_Dialog *e_int_config_notification_module(Evas_Object *parent, const char *params);

extern E_Module *notification_mod;
extern Config   *notification_cfg;

/**
 * @addtogroup Optional_Monitors
 * @{
 *
 * @defgroup Module_Notification Notification (Notify-OSD)
 *
 * Presents notifications on screen as an unobtrusive popup. It
 * implements the Notify-OSD and FreeDesktop.org standards.
 *
 * @see http://www.galago-project.org/specs/notification/0.9/
 * @see https://wiki.ubuntu.com/NotifyOSD
 * @}
 */

#endif
