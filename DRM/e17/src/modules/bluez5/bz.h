#ifndef BZ_H
# define BZ_H
# include <Elementary.h>

typedef enum {
   BZ_OBJ_UNKNOWN,
   BZ_OBJ_BLUEZ,
   BZ_OBJ_ADAPTER,
   BZ_OBJ_DEVICE
} Obj_Type;

typedef struct _Obj Obj;

struct _Obj {
   //// internal object data
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy, *proxy_bat;
   Eldbus_Proxy *prop_proxy, *prop_proxy_bat;
   Eldbus_Signal_Handler *prop_sig, *prop_sig_bat;
   unsigned int ref;
   Eina_Bool in_table : 1;
   Eina_Bool add_called : 1;
   Eina_Bool ping_ok : 1;
   Eina_Bool ping_busy : 1;
   Eina_Bool ping_block : 1;
   Ecore_Timer *ping_timer;
   Ecore_Timer *power_retry_timer;
   //// public data to read
   const char *path;
   Obj_Type type;
   //// data to be set by users of the obj
   void *data; // custom data
   void (*fn_change) (Obj *o);
   void (*fn_del) (Obj *o);
   //// obj properties
   Eina_Array *uuids;
   const char *address;
   const char *address_type;
   const char *name;
   const char *icon;
   const char *alias;
   const char *adapter;
   const char *modalias;
   unsigned int klass;
   unsigned short appearance;
   unsigned short txpower;
   short rssi;
   signed char bat_percent;
   Eina_Bool paired : 1;
   Eina_Bool connected : 1;
   Eina_Bool trusted : 1;
   Eina_Bool blocked : 1;
   Eina_Bool legacy_pairing : 1;
   Eina_Bool services_resolved : 1;
   //// adapter specific properties
   unsigned int discoverable_timeout;
   unsigned int pairable_timeout;
   Eina_Bool discoverable : 1;
   Eina_Bool discovering : 1;
   Eina_Bool pairable : 1;
   Eina_Bool powered : 1;
   //// agent data for when devices ask to pair etc.
   const char *agent_request;
   Eldbus_Message *agent_msg_ok;
   Eldbus_Message *agent_msg_err;
   void (*agent_entry_fn) (Eldbus_Message *msg, const char *str);
   Eina_Bool agent_alert : 1;
};

#define BZ_OBJ_CLASS_SERVICE_LIMITED_DISCOVERABLE  (1 << 13)
#define BZ_OBJ_CLASS_SERVICE_POSITIONING_BIT       (1 << 16)
#define BZ_OBJ_CLASS_SERVICE_NETWORKING_BIT        (1 << 17)
#define BZ_OBJ_CLASS_SERVICE_RENDERING_BIT         (1 << 18)
#define BZ_OBJ_CLASS_SERVICE_CAPTURING_BIT         (1 << 19)
#define BZ_OBJ_CLASS_SERVICE_OBJECT_TRANSFER_BIT   (1 << 20)
#define BZ_OBJ_CLASS_SERVICE_AUDIO_BIT             (1 << 21)
#define BZ_OBJ_CLASS_SERVICE_TELEPHONY_BIT         (1 << 22)
#define BZ_OBJ_CLASS_SERVICE_INFORMATION_BIT       (1 << 23)

#define BZ_OBJ_CLASS_MAJ_MASK                      (0x1f << 8)
#define BZ_OBJ_CLASS_MAJ_MISC                      ( 0 << 8)
#define BZ_OBJ_CLASS_MAJ_COMPUTER                  ( 1 << 8)
#define BZ_OBJ_CLASS_MAJ_PHONE                     ( 2 << 8)
#define BZ_OBJ_CLASS_MAJ_LAN                       ( 3 << 8)
#define BZ_OBJ_CLASS_MAJ_AV                        ( 4 << 8)
#define BZ_OBJ_CLASS_MAJ_PERIPHERAL                ( 5 << 8)
#define BZ_OBJ_CLASS_MAJ_IMAGING                   ( 6 << 8)
#define BZ_OBJ_CLASS_MAJ_WEARABLE                  ( 7 << 8)
#define BZ_OBJ_CLASS_MAJ_TOY                       ( 8 << 8)
#define BZ_OBJ_CLASS_MAJ_HEALTH                    ( 9 << 8)

#define BZ_OBJ_CLASS_MIN_COMPUTER_MASK             (0x3f << 2)
#define BZ_OBJ_CLASS_MIN_COMPUTER_DESKTOP          ( 1 << 2)
#define BZ_OBJ_CLASS_MIN_COMPUTER_SERVER           ( 2 << 2)
#define BZ_OBJ_CLASS_MIN_COMPUTER_LAPTOP           ( 3 << 2)
#define BZ_OBJ_CLASS_MIN_COMPUTER_CLAMSHELL        ( 4 << 2)
#define BZ_OBJ_CLASS_MIN_COMPUTER_PDA              ( 5 << 2)
#define BZ_OBJ_CLASS_MIN_COMPUTER_WEARABLE         ( 6 << 2)
#define BZ_OBJ_CLASS_MIN_COMPUTER_TABLET           ( 7 << 2)

#define BZ_OBJ_CLASS_MIN_PHONE_MASK                (0x3f << 2)
#define BZ_OBJ_CLASS_MIN_PHONE_CELL                ( 1 << 2)
#define BZ_OBJ_CLASS_MIN_PHONE_CORDLESS            ( 2 << 2)
#define BZ_OBJ_CLASS_MIN_PHONE_SMARTPHONE          ( 3 << 2)
#define BZ_OBJ_CLASS_MIN_PHONE_WIRED               ( 4 << 2)
#define BZ_OBJ_CLASS_MIN_PHONE_ISDN                ( 5 << 2)

#define BZ_OBJ_CLASS_MIN_LAN_MASK                  (0x07 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_7               ( 0 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_6               ( 1 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_5               ( 2 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_4               ( 3 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_3               ( 4 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_2               ( 5 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_1               ( 6 << 5)
#define BZ_OBJ_CLASS_MIN_LAN_AVAIL_0               ( 7 << 5)

#define BZ_OBJ_CLASS_MIN_AV_MASK                   (0x3f << 2)
#define BZ_OBJ_CLASS_MIN_AV_WEARABLE_HEADSET       ( 1 << 2)
#define BZ_OBJ_CLASS_MIN_AV_HANDS_FREE             ( 2 << 2)
#define BZ_OBJ_CLASS_MIN_AV_MIC                    ( 4 << 2)
#define BZ_OBJ_CLASS_MIN_AV_SPEAKER                ( 5 << 2)
#define BZ_OBJ_CLASS_MIN_AV_HEADPHONES             ( 6 << 2)
#define BZ_OBJ_CLASS_MIN_AV_PORTABLE_AUDIO         ( 7 << 2)
#define BZ_OBJ_CLASS_MIN_AV_CAR_AUDIO              ( 8 << 2)
#define BZ_OBJ_CLASS_MIN_AV_SET_TOP_BOX            ( 9 << 2)
#define BZ_OBJ_CLASS_MIN_AV_HIFI_AUDIO             (10 << 2)
#define BZ_OBJ_CLASS_MIN_AV_VCR                    (11 << 2)
#define BZ_OBJ_CLASS_MIN_AV_VIDEO_CAMERA           (12 << 2)
#define BZ_OBJ_CLASS_MIN_AV_CAMCORDER              (13 << 2)
#define BZ_OBJ_CLASS_MIN_AV_VIDEO_MONITOR          (14 << 2)
#define BZ_OBJ_CLASS_MIN_AV_VIDEO_DISPLAY_SPEAKER  (15 << 2)
#define BZ_OBJ_CLASS_MIN_AV_VIDEO_CONFERENCE       (16 << 2)
#define BZ_OBJ_CLASS_MIN_AV_GAMING                 (18 << 2)

#define BZ_OBJ_CLASS_MIN_PERIPHERAL_KEYBOARD_BIT   ( 1 << 6)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_MOUSE_BIT      ( 1 << 7)

#define BZ_OBJ_CLASS_MIN_PERIPHERAL_MASK2          (0x0f << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_JOYSTICK       ( 1 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_GAMEPAD        ( 2 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_REMOTE         ( 3 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_SENSING        ( 4 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_DIGITIZER_TAB  ( 5 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_CARD_READER    ( 6 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_PEN            ( 7 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_SCANNER        ( 8 << 2)
#define BZ_OBJ_CLASS_MIN_PERIPHERAL_WAND           ( 9 << 2)

#define BZ_OBJ_CLASS_MIN_IMAGING_DISPLAY_BIT       ( 1 << 4)
#define BZ_OBJ_CLASS_MIN_IMAGING_CAMERA_BIT        ( 1 << 5)
#define BZ_OBJ_CLASS_MIN_IMAGING_SCANNER_BIT       ( 1 << 6)
#define BZ_OBJ_CLASS_MIN_IMAGING_PRINTER_BIT       ( 1 << 7)

#define BZ_OBJ_CLASS_MIN_WEARABLE_MASK             (0x3f << 2)
#define BZ_OBJ_CLASS_MIN_WEARABLE_WATCH            ( 1 << 2)
#define BZ_OBJ_CLASS_MIN_WEARABLE_PAGER            ( 2 << 2)
#define BZ_OBJ_CLASS_MIN_WEARABLE_JACKET           ( 3 << 2)
#define BZ_OBJ_CLASS_MIN_WEARABLE_HELMET           ( 4 << 2)
#define BZ_OBJ_CLASS_MIN_WEARABLE_GLASSES          ( 5 << 2)

#define BZ_OBJ_CLASS_MIN_TOY_MASK                  (0x3f << 2)
#define BZ_OBJ_CLASS_MIN_TOY_ROBOT                 ( 1 << 2)
#define BZ_OBJ_CLASS_MIN_TOY_VEHICLE               ( 2 << 2)
#define BZ_OBJ_CLASS_MIN_TOY_DOLL                  ( 3 << 2)
#define BZ_OBJ_CLASS_MIN_TOY_CONTROLLER            ( 4 << 2)
#define BZ_OBJ_CLASS_MIN_TOY_GAME                  ( 5 << 2)

#define BZ_OBJ_CLASS_MIN_HEALTH_MASK               (0x3f << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_BLOOD_PRESSURE     ( 1 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_THERMOMETER        ( 2 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_SCALES             ( 3 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_GLUCOSE            ( 4 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_PULSE_OXIMITER     ( 5 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_HEART_RATE         ( 6 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_HEALTH_DATA_DISP   ( 7 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_STEP               ( 8 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_BODY_COMPOSITION   ( 9 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_PEAK_FLOW          (10 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_MEDICATION         (11 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_KNEE_PROSTHESIS    (12 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_ANKLE_PROSTHESIS   (13 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_GENERIC_HEALTH     (14 << 2)
#define BZ_OBJ_CLASS_MIN_HEALTH_PRESONAL_MOBILITY  (15 << 2)


extern Eldbus_Connection *bz_conn;

void bz_init(void);
void bz_shutdown(void);

Obj  *bz_obj_add(const char *path);
Obj  *bz_obj_find(const char *path);
void  bz_obj_power_on(Obj *o);
void  bz_obj_power_off(Obj *o);
void  bz_obj_discoverable(Obj *o);
void  bz_obj_undiscoverable(Obj *o);
void  bz_obj_pairable(Obj *o);
void  bz_obj_unpairable(Obj *o);
void  bz_obj_trust(Obj *o);
void  bz_obj_distrust(Obj *o);
void  bz_obj_pair(Obj *o);
void  bz_obj_pair_cancel(Obj *o);
void  bz_obj_connect(Obj *o);
void  bz_obj_disconnect(Obj *o);
void  bz_obj_ping_begin(Obj *o);
void  bz_obj_ping_end(Obj *o);
void  bz_obj_remove(Obj *o);
//void  bz_obj_profile_connect(Obj *o, const char *uuid);
//void  bz_obj_profile_disconnect(Obj *o, const char *uuid);
void  bz_obj_ref(Obj *o);
void  bz_obj_unref(Obj *o);
void  bz_obj_discover_start(Obj *o);
void  bz_obj_discover_stop(Obj *o);
void  bz_obj_agent_request(Obj *o, const char *req, void (*fn) (Eldbus_Message *msg, const char *str), Eldbus_Message *ok_msg, Eldbus_Message *err_msg);

void  bz_obj_init(void);
void  bz_obj_shutdown(void);
void  bz_obj_add_func_set(void (*fn) (Obj *o));

void  bz_agent_msg_reply(Eldbus_Message *msg);
void  bz_agent_msg_drop(Eldbus_Message *msg);
Eldbus_Message *bz_agent_msg_err(Eldbus_Message *msg);
Eldbus_Message *bz_agent_msg_ok(Eldbus_Message *msg);
const char *bz_agent_msg_path(Eldbus_Message *msg);
const char *bz_agent_msg_path_str(Eldbus_Message *msg, const char **str);
const char *bz_agent_msg_path_u32(Eldbus_Message *msg, unsigned int *u32);
const char *bz_agent_msg_path_u32_u16(Eldbus_Message *msg, unsigned int *u32, unsigned short *u16);
void  bz_agent_msg_str_add(Eldbus_Message *msg, const char *str);
void  bz_agent_msg_u32_add(Eldbus_Message *msg, unsigned int u32);

void  bz_agent_init(void);
void  bz_agent_shutdown(void);
void  bz_agent_release_func_set(void (*fn) (void));
void  bz_agent_cancel_func_set(void (*fn) (void));
void  bz_agent_req_pin_func_set(void (*fn) (Eldbus_Message *msg));
void  bz_agent_disp_pin_func_set(void (*fn) (Eldbus_Message *msg));
void  bz_agent_req_pass_func_set(void (*fn) (Eldbus_Message *msg));
void  bz_agent_disp_pass_func_set(void (*fn) (Eldbus_Message *msg));
void  bz_agent_req_confirm_func_set(void (*fn) (Eldbus_Message *msg));
void  bz_agent_req_auth_func_set(void (*fn) (Eldbus_Message *msg));
void  bz_agent_auth_service_func_set(void (*fn) (Eldbus_Message *msg));
#endif
