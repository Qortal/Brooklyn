#ifndef MUSIC_CONTROL_PRIVATE_H
#define MUSIC_CONTROL_PRIVATE_H

#include "e_mod_main.h"
#include "eldbus_media_player2_player.h"
#include "eldbus_mpris_media_player2.h"

static Ecore_Event_Handler *desklock_handler = NULL;

typedef struct _Music_Control_Config
{
   int player_selected;
   int pause_on_desklock;
} Music_Control_Config;

typedef struct _E_Music_Control_Module_Context
{
   Eina_List *instances;
   Eldbus_Connection *conn;
   Eina_Bool playing E_BITFIELD;
   Eina_Stringshare *meta_artist;
   Eina_Stringshare *meta_album;
   Eina_Stringshare *meta_title;
   Eina_Stringshare *meta_cover;
   Eina_Stringshare *meta_cover_prev;
   double            meta_length;
   double            position;
   Eina_Bool         loop;
   Eldbus_Proxy *mrpis2;
   Eldbus_Proxy *mpris2_player;
   E_Config_DD *conf_edd;
   Music_Control_Config *config;
   Eldbus_Pending *bus_list_pend;
   Ecore_Timer *poll_timer;
   const char *dbus_name;
   Eina_Bool actions_set E_BITFIELD;
} E_Music_Control_Module_Context;

typedef struct _E_Music_Control_Instance
{
   E_Music_Control_Module_Context *ctxt;
   E_Gadcon_Client *gcc;
   Evas_Object *gadget;
   E_Gadcon_Popup *popup;
   Evas_Object *content_popup;
} E_Music_Control_Instance;

void music_control_launch(E_Music_Control_Instance *inst);
void music_control_mouse_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event);
const char *music_control_edj_path_get(void);
void music_control_popup_del(E_Music_Control_Instance *inst);
void music_control_state_update_all(E_Music_Control_Module_Context *ctxt);
void music_control_pos_update(E_Music_Control_Module_Context *ctxt);
void music_control_metadata_update_all(E_Music_Control_Module_Context *ctxt);
Eina_Bool music_control_dbus_init(E_Music_Control_Module_Context *ctxt, const char *bus);

Eina_Bool _desklock_cb(void *data, int type EINA_UNUSED, void *ev);

typedef struct _Player {
   const char *name;
   const char *dbus_name;
   const char *command;
} Player;

#endif
