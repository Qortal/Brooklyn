#ifdef E_TYPEDEFS

typedef enum _E_Drag_Type
{
   E_DRAG_NONE,
   E_DRAG_INTERNAL,
   E_DRAG_XDND
} E_Drag_Type;

typedef struct _E_Drag            E_Drag;
typedef struct _E_Drop_Handler    E_Drop_Handler;
typedef struct _E_Event_Dnd_Enter E_Event_Dnd_Enter;
typedef struct _E_Event_Dnd_Move  E_Event_Dnd_Move;
typedef struct _E_Event_Dnd_Leave E_Event_Dnd_Leave;
typedef struct _E_Event_Dnd_Drop  E_Event_Dnd_Drop;
typedef struct E_Dnd_X_Moz_Url    E_Dnd_X_Moz_Url;

#else
#ifndef E_DND_H
#define E_DND_H

#define E_DRAG_TYPE 0xE0b0100f

struct _E_Drag
{
   E_Object           e_obj_inherit;

   void              *data;
   int                data_size;

   E_Drag_Type        type;

   struct
   {
      void *(*convert)(E_Drag * drag, const char *type);
      void  (*finished)(E_Drag *drag, int dropped);
      void  (*key_down)(E_Drag *drag, Ecore_Event_Key *e);
      void  (*key_up)(E_Drag *drag, Ecore_Event_Key *e);
   } cb;

   Evas              *evas;
   Evas_Object       *comp_object;
   Evas_Object       *object;
   Ecore_Timer       *timeout;

   int                x, y, w, h;
   int                dx, dy;
   unsigned int       button_mask;

   E_Layer            layer;
   unsigned char      visible E_BITFIELD;
   Eina_Bool          ended E_BITFIELD;

   unsigned int       num_types;
   const char        *types[];
};

struct _E_Drop_Handler
{
   struct
   {
      void  (*enter)(void *data, const char *type, void *event);
      void  (*move)(void *data, const char *type, void *event);
      void  (*leave)(void *data, const char *type, void *event);
      void  (*drop)(void *data, const char *type, void *event);
      Eina_Bool (*xds)(void *data, const char *type);
      void *data;
   } cb;

   E_Object     *obj;
   Evas_Object *win;
   Evas_Object *base;
   int           x, y, w, h;

   const char   *active_type;
   Eina_Bool active E_BITFIELD;
   Eina_Bool entered E_BITFIELD;
   Eina_Bool hidden E_BITFIELD;
   unsigned int  num_types;
   Eina_Stringshare *types[];
};

struct _E_Event_Dnd_Enter
{
   void        *data;
   int          x, y;
   unsigned int action;
};

struct _E_Event_Dnd_Move
{
   int          x, y;
   unsigned int action;
};

struct _E_Event_Dnd_Leave
{
   int x, y;
};

struct _E_Event_Dnd_Drop
{
   void *data;
   int   x, y;
};

struct E_Dnd_X_Moz_Url
{
   Eina_Inarray *links;
   Eina_Inarray *link_names;
};

EINTERN int          e_dnd_init(void);
EINTERN int          e_dnd_shutdown(void);

E_API int             e_dnd_active(void);

E_API E_Drag         *e_drag_current_get(void);
/* x and y are the top left coords of the object that is to be dragged */
E_API E_Drag         *e_drag_new(int x, int y,
                                const char **types, unsigned int num_types,
                                void *data, int size,
                                void *(*convert_cb)(E_Drag * drag, const char *type),
                                void (*finished_cb)(E_Drag *drag, int dropped));
E_API Evas           *e_drag_evas_get(const E_Drag *drag);
E_API void            e_drag_object_set(E_Drag *drag, Evas_Object *object);
E_API void            e_drag_move(E_Drag *drag, int x, int y);
E_API void            e_drag_resize(E_Drag *drag, int w, int h);
E_API void            e_drag_key_down_cb_set(E_Drag *drag, void (*func)(E_Drag *drag, Ecore_Event_Key *e));
E_API void            e_drag_key_up_cb_set(E_Drag *drag, void (*func)(E_Drag *drag, Ecore_Event_Key *e));

/* x and y are the coords where the mouse is when dragging starts */
E_API int             e_drag_start(E_Drag *drag, int x, int y);
E_API int             e_drag_xdnd_start(E_Drag *drag, int x, int y);

E_API void e_drop_xds_update(Eina_Bool enable, const char *value);
E_API void e_drop_handler_xds_set(E_Drop_Handler *handler, Eina_Bool (*cb)(void *data, const char *type));
E_API E_Drop_Handler *e_drop_handler_add(E_Object *obj, Evas_Object *win,
                                        void *data,
                                        void (*enter_cb)(void *data, const char *type, void *event),
                                        void (*move_cb)(void *data, const char *type, void *event),
                                        void (*leave_cb)(void *data, const char *type, void *event),
                                        void (*drop_cb)(void *data, const char *type, void *event),
                                        const char **types, unsigned int num_types,
                                        int x, int y, int w, int h);
E_API void         e_drop_handler_geometry_set(E_Drop_Handler *handler, int x, int y, int w, int h);
E_API int          e_drop_inside(const E_Drop_Handler *handler, int x, int y);
E_API void         e_drop_handler_del(E_Drop_Handler *handler);
E_API int          e_drop_xdnd_register_set(Ecore_Window win, int reg);
E_API void         e_drop_handler_responsive_set(E_Drop_Handler *handler);
E_API int          e_drop_handler_responsive_get(const E_Drop_Handler *handler);
E_API void         e_drop_handler_action_set(unsigned int action);
E_API unsigned int e_drop_handler_action_get(void);
E_API Eina_List *e_dnd_util_text_uri_list_convert(char *data, int size);


static inline void
e_drag_show(E_Drag *drag)
{
   drag->visible = 1;
}

static inline void
e_drag_hide(E_Drag *drag)
{
   drag->visible = 0;
}

#endif
#endif
