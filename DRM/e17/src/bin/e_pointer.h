#ifdef E_TYPEDEFS

typedef struct _E_Pointer E_Pointer;

typedef enum
{
   /* These are compatible with netwm */
   E_POINTER_RESIZE_TL = 0,
   E_POINTER_RESIZE_T = 1,
   E_POINTER_RESIZE_TR = 2,
   E_POINTER_RESIZE_R = 3,
   E_POINTER_RESIZE_BR = 4,
   E_POINTER_RESIZE_B = 5,
   E_POINTER_RESIZE_BL = 6,
   E_POINTER_RESIZE_L = 7,
   E_POINTER_MOVE = 8,
   E_POINTER_RESIZE_NONE = 11
} E_Pointer_Mode;

#else
# ifndef E_POINTER_H
#  define E_POINTER_H

#  define E_POINTER_TYPE 0xE0b01013

struct _E_Pointer
{
   E_Object e_obj_inherit;

   Evas *evas;
   Ecore_Evas *ee;
   Evas *buffer_evas;
   Evas_Object *o_ptr;
   Evas_Object *o_hot;
   Evas_Object *buffer_o_ptr;
   Evas_Object *buffer_o_hot;
   struct
   {
      E_Client *ec;
      int x, y;
   } client;

   Ecore_Window win;

   int *pixels;
   int x, y, w, h;
   const char *type;
   const char *deferred_type;

   struct
     {
        int x, y;
        Eina_Bool update E_BITFIELD;
     } hot;

   Ecore_Timer *idle_tmr;
   Ecore_Poller *idle_poll;

   Eina_List *stack;

   unsigned short grabcount;

   Eina_Bool e_cursor E_BITFIELD;
   Eina_Bool color E_BITFIELD;
   Eina_Bool idle E_BITFIELD;
   Eina_Bool canvas E_BITFIELD;
};

EINTERN int e_pointer_init(void);
EINTERN int e_pointer_shutdown(void);
EINTERN void e_pointers_freeze_set(Eina_Bool set);

E_API E_Pointer *e_pointer_window_new(Ecore_Window win, Eina_Bool filled);
E_API E_Pointer *e_pointer_canvas_new(Ecore_Evas *ee, Eina_Bool filled);

E_API void e_pointers_size_set(int size);
E_API void e_pointer_hide(E_Pointer *ptr);
E_API void e_pointer_show(E_Pointer *ptr);
E_API void e_pointer_type_push(E_Pointer *ptr, void *obj, const char *type);
E_API void e_pointer_type_pop(E_Pointer *ptr, void *obj, const char *type);
E_API void e_pointer_mode_push(void *obj, E_Pointer_Mode mode);
E_API void e_pointer_mode_pop(void *obj, E_Pointer_Mode mode);
E_API void e_pointer_idler_before(void);
E_API void e_pointer_object_set(E_Pointer *ptr, Evas_Object *obj, int x, int y);
E_API void e_pointer_window_add(E_Pointer *ptr, Ecore_Window win);
E_API void e_pointer_grab_set(E_Pointer *ptr, Eina_Bool grab);
E_API void e_pointer_reset(E_Pointer *ptr);

# endif
#endif
