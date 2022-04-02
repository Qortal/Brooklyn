#ifndef E_KBD_INT_H
#define E_KBD_INT_H

#include "e_kbd_buf.h"

typedef enum _E_Kbd_Int_Type
{
   E_KBD_INT_TYPE_UNKNOWN = 0,
   E_KBD_INT_TYPE_ALPHA = (1 << 0),
   E_KBD_INT_TYPE_NUMERIC = (1 << 1),
   E_KBD_INT_TYPE_PIN = (1 << 2),
   E_KBD_INT_TYPE_PHONE_NUMBER = (1 << 3),
   E_KBD_INT_TYPE_HEX = (1 << 4),
   E_KBD_INT_TYPE_TERMINAL = (1 << 5),
   E_KBD_INT_TYPE_PASSWORD = (1 << 6),
   E_KBD_INT_TYPE_IP = (1 << 7),
   E_KBD_INT_TYPE_HOST = (1 << 8),
   E_KBD_INT_TYPE_FILE = (1 << 9),
   E_KBD_INT_TYPE_URL = (1 << 10),
   E_KBD_INT_TYPE_KEYPAD = (1 << 11),
   E_KBD_INT_TYPE_J2ME = (1 << 12)
} E_Kbd_Int_Type;

/* The natural text direction of the keyboard */
typedef enum _E_kbd_Int_Direction
{
   E_KBD_INT_DIRECTION_LTR = (1 << 0),
   E_KBD_INT_DIRECTION_RTL = (1 << 1)
} E_Kbd_Int_Direction;

typedef struct _E_Kbd_Int            E_Kbd_Int;
typedef struct _E_Kbd_Int_Key        E_Kbd_Int_Key;
typedef struct _E_Kbd_Int_Key_State  E_Kbd_Int_Key_State;
typedef struct _E_Kbd_Int_Layout     E_Kbd_Int_Layout;
typedef struct _E_Kbd_Int_Match      E_Kbd_Int_Match;
typedef struct _E_Kbd_Int_Multi_Info E_Kbd_Int_Multi_Info;

struct _E_Kbd_Int
{
   const char          *themedir, *syskbds, *sysdicts;
   E_Zone              *zone;
   Evas_Object         *base_obj, *layout_obj, *event_obj, *box_obj, *boxgrid_obj;
   Eina_List           *layouts;
   Eina_List           *matches;
   int                  x, y, x0, y0;
   double               px, py;
   struct {
      char             *directory;
      const char       *file;
      int               w, h;
      int               fuzz;
      int               direction;
      E_Kbd_Int_Type    type;
      Eina_List        *keys;
      E_Kbd_Int_Key    *pressed;
      Eina_List        *multis;
      int               state;
   } layout;
   struct {
      Evas_Coord        x, y;
      int               lx, ly;
      unsigned char     down E_BITFIELD;
      unsigned char     stroke E_BITFIELD;
      unsigned char     zoom E_BITFIELD;
      unsigned char     twofinger E_BITFIELD;
   } down;

   E_Kbd_Buf           *kbuf;
   Eina_Bool            visible E_BITFIELD;
};

struct _E_Kbd_Int_Key
{
   int x, y, w, h;

   Eina_List *states;
   Evas_Object *obj, *zoom_obj, *icon_obj, *zoom_icon_obj;

   unsigned char pressed E_BITFIELD;
   unsigned char selected E_BITFIELD;

   unsigned char is_shift E_BITFIELD;
   unsigned char is_multi_shift E_BITFIELD;
   unsigned char is_ctrl E_BITFIELD;
   unsigned char is_alt E_BITFIELD;
   unsigned char is_altgr E_BITFIELD;
   unsigned char is_capslock E_BITFIELD;
};

struct _E_Kbd_Int_Key_State
{
   int state;
   const char *label, *icon;
   const char *out;
};

struct _E_Kbd_Int_Layout
{
   const char *path;
   const char *dir;
   const char *icon;
   const char *name;
   int type;
};

struct _E_Kbd_Int_Match
{
   E_Kbd_Int   *ki;
   const char  *str;
   Evas_Object *obj;
};

struct _E_Kbd_Int_Multi_Info
{
   int            device;
   E_Kbd_Int_Key *ky;
};

EAPI E_Kbd_Int *e_kbd_int_new(int zone_num, const char *zone_id,
                              const char *themedir, const char *syskbds,
                              const char *sysdicts);
EAPI void e_kbd_int_free(E_Kbd_Int *ki);
EAPI void e_kbd_int_update(E_Kbd_Int *ki);
EAPI void e_kbd_int_show(E_Kbd_Int *ki);
EAPI void e_kbd_int_hide(E_Kbd_Int *ki);

#endif
