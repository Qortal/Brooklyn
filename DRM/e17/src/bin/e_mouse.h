#ifdef E_TYPEDEFS

typedef enum _E_Mouse_Hand
{
   E_MOUSE_HAND_LEFT,
   E_MOUSE_HAND_RIGHT
} E_Mouse_Hand;

#else
#ifndef E_MOUSE_H
#define E_MOUSE_H

E_API int e_mouse_update(void);

#endif
#endif
