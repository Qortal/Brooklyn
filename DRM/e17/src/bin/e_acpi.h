#ifdef E_TYPEDEFS

/* enum for various event types */
typedef enum _E_Acpi_Type
{
   E_ACPI_TYPE_UNKNOWN = 0,

   E_ACPI_TYPE_AC_ADAPTER, // 1
   E_ACPI_TYPE_BATTERY, // 2
   E_ACPI_TYPE_BUTTON, // 3
   E_ACPI_TYPE_FAN, // 4
   E_ACPI_TYPE_LID, // 5
   E_ACPI_TYPE_POWER, // 6
   E_ACPI_TYPE_PROCESSOR, // 7
   E_ACPI_TYPE_SLEEP, // 8
   E_ACPI_TYPE_THERMAL, // 9
   E_ACPI_TYPE_VIDEO, // 10
   E_ACPI_TYPE_WIFI, // 11
   E_ACPI_TYPE_HIBERNATE, // 12
   E_ACPI_TYPE_ZOOM_OUT, // 13
   E_ACPI_TYPE_ZOOM_IN, // 14
   E_ACPI_TYPE_BRIGHTNESS_DOWN, // 15
   E_ACPI_TYPE_BRIGHTNESS_UP, // 16
   E_ACPI_TYPE_ASSIST, // 17
   E_ACPI_TYPE_S1, // 18
   E_ACPI_TYPE_VAIO, // 19
   E_ACPI_TYPE_MUTE, // 20
   E_ACPI_TYPE_VOLUME, // 21
   E_ACPI_TYPE_BRIGHTNESS, // 22
   E_ACPI_TYPE_VOLUME_DOWN, // 23
   E_ACPI_TYPE_VOLUME_UP, // 24
   E_ACPI_TYPE_TABLET_ON, // 25
   E_ACPI_TYPE_TABLET_OFF, // 26
   E_ACPI_TYPE_ZOOM, // 27
   E_ACPI_TYPE_SCREENLOCK, // 28
   E_ACPI_TYPE_BATTERY_BUTTON, // 29
   E_ACPI_TYPE_TABLET, // 30
   E_ACPI_TYPE_CD_NEXT, // 31
   E_ACPI_TYPE_CD_PREV, // 32
   E_ACPI_TYPE_CD_PLAY, // 33
   E_ACPI_TYPE_CD_STOP, // 34
} E_Acpi_Type;

/* enum for acpi signals */
typedef enum _E_Acpi_Device_Signal
{
   E_ACPI_DEVICE_SIGNAL_UNKNOWN, // 0
   E_ACPI_DEVICE_SIGNAL_NOTIFY = 80,
   E_ACPI_DEVICE_SIGNAL_CHANGED = 82, // device added or removed
   E_ACPI_DEVICE_SIGNAL_AWAKE = 83,
   E_ACPI_DEVICE_SIGNAL_EJECT = 84
} E_Acpi_Device_Signal;

/* enum for lid status */
typedef enum _E_Acpi_Lid_Status
{
   E_ACPI_LID_UNKNOWN, // 0
   E_ACPI_LID_CLOSED, // 1
   E_ACPI_LID_OPEN // 2
} E_Acpi_Lid_Status;

/* struct used to pass to event handlers */
typedef struct _E_Event_Acpi E_Event_Acpi;

#else
# ifndef E_ACPI_H
#  define E_ACPI_H

struct _E_Event_Acpi
{
   const char *device, *bus_id;
   int         type, signal, status;
};

EINTERN int e_acpi_init(void);
EINTERN int e_acpi_shutdown(void);

EINTERN E_Acpi_Lid_Status e_acpi_lid_status_get(void);
E_API Eina_Bool e_acpi_lid_is_closed(void);
E_API void e_acpi_events_freeze(void);
E_API void e_acpi_events_thaw(void);

extern E_API int E_EVENT_ACPI;

# endif
#endif
