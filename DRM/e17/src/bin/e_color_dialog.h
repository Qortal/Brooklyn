#ifdef E_TYPEDEFS

typedef struct _E_Color_Dialog E_Color_Dialog;

#else
#ifndef E_COLOR_DIALOG_H
#define E_COLOR_DIALOG_H

#define E_COLOR_DIALOG_TYPE 0xE0b0101c

struct _E_Color_Dialog
{
   E_Object             e_obj_inherit;

   E_Dialog            *dia;

   E_Color             *color, *initial;

   void (*select_func)(E_Color_Dialog *dia, E_Color *color, void *data);
   void  *select_data;
   void (*cancel_func)(E_Color_Dialog *dia, E_Color *color, void *data);
   void  *cancel_data;
   void (*change_func)(E_Color_Dialog *dia, E_Color *color, void *data);
   void  *change_data;
};

E_API E_Color_Dialog  *e_color_dialog_new                (const E_Color *initial_color, Eina_Bool alpha_enabled);
E_API void             e_color_dialog_show               (E_Color_Dialog *dia);
E_API void             e_color_dialog_title_set          (E_Color_Dialog *dia, const char *title);

/* select and change pass the current color back, cancel passes the initial color */
E_API void             e_color_dialog_select_callback_set(E_Color_Dialog *dia, void (*func)(E_Color_Dialog *dia, E_Color *color, void *data), void *data);
E_API void             e_color_dialog_cancel_callback_set(E_Color_Dialog *dia, void (*func)(E_Color_Dialog *dia, E_Color *color, void *data), void *data);
E_API void             e_color_dialog_change_callback_set(E_Color_Dialog *dia, void (*func)(E_Color_Dialog *dia, E_Color *color, void *data), void *data);

#endif
#endif
