#ifdef E_TYPEDEFS

typedef struct _E_Gadcon_Popup E_Gadcon_Popup;

#else
#ifndef E_GADCON_POPUP_H
#define E_GADCON_POPUP_H

#define E_GADCON_POPUP_TYPE 0xE0b0104e

struct _E_Gadcon_Popup
{
   E_Object             e_obj_inherit;

   Evas_Object         *comp_object;
   E_Gadcon_Client     *gcc;
   Evas_Object         *o_bg;
   Evas_Object        *content;
   Ecore_Event_Handler *autoclose_handlers[2];
   Ecore_Idler         *show_idler;
   Ecore_Animator      *show_animator;
   Evas_Coord           w, h;

   Eina_Bool            pinned E_BITFIELD;
   Eina_Bool            gadcon_lock E_BITFIELD;
   Eina_Bool            gadcon_was_locked E_BITFIELD;
   Eina_Bool            visible E_BITFIELD;
};

E_API E_Gadcon_Popup *e_gadcon_popup_new(E_Gadcon_Client *gcc, Eina_Bool noshadow);
E_API void e_gadcon_popup_content_set(E_Gadcon_Popup *pop, Evas_Object *o);
E_API void e_gadcon_popup_show(E_Gadcon_Popup *pop);
E_API void e_gadcon_popup_hide(E_Gadcon_Popup *pop);
E_API void e_gadcon_popup_toggle_pinned(E_Gadcon_Popup *pop);
E_API void e_gadcon_popup_lock_set(E_Gadcon_Popup *pop, Eina_Bool setting);

#endif
#endif
