#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Il_Kbd_Config Il_Kbd_Config;

struct _Il_Kbd_Config
{
   const char *dict;
   const char *zone_id;
   int         zone_num;
   double      size;
   int         fill_mode;
   double      px, py;

   // Not User Configurable. Placeholders
   E_Action   *act_kbd_show;
   E_Action   *act_kbd_hide;
   E_Action   *act_kbd_toggle;
   const char *mod_dir;
   int         slide_dim;
   int         layout;
};

extern EAPI Il_Kbd_Config *il_kbd_cfg;

#endif
