#ifdef E_TYPEDEFS

typedef enum _E_Binding_Context
{
   E_BINDING_CONTEXT_NONE,
   E_BINDING_CONTEXT_UNKNOWN,
   E_BINDING_CONTEXT_WINDOW,
   E_BINDING_CONTEXT_ZONE,
   E_BINDING_CONTEXT_COMPOSITOR,
   E_BINDING_CONTEXT_MANAGER,
   E_BINDING_CONTEXT_MENU,
   E_BINDING_CONTEXT_WINLIST,
   E_BINDING_CONTEXT_POPUP,
   E_BINDING_CONTEXT_ANY,
   E_BINDING_CONTEXT_LAST,
} E_Binding_Context;

/* why do we do this? config stored bindings must be fixed. x's modifier masks
 * may change from time to time, xserver to xserver - so we can't do a
 * simple match without translating to fixed values
 */
typedef enum _E_Binding_Modifier
{
   E_BINDING_MODIFIER_NONE = 0,
   E_BINDING_MODIFIER_SHIFT = (1 << 0),
   E_BINDING_MODIFIER_CTRL = (1 << 1),
   E_BINDING_MODIFIER_ALT = (1 << 2),
   E_BINDING_MODIFIER_WIN = (1 << 3),
   E_BINDING_MODIFIER_ALTGR = (1 << 4)
} E_Binding_Modifier;

typedef struct _E_Binding_Mouse  E_Binding_Mouse;
typedef struct _E_Binding_Key    E_Binding_Key;
typedef struct _E_Binding_Edge   E_Binding_Edge;
typedef struct _E_Binding_Signal E_Binding_Signal;
typedef struct _E_Binding_Wheel  E_Binding_Wheel;
typedef struct _E_Binding_Acpi   E_Binding_Acpi;
typedef struct _E_Binding_Swipe  E_Binding_Swipe;

typedef struct E_Binding_Event_Mouse_Button E_Binding_Event_Mouse_Button;
typedef struct E_Binding_Event_Wheel E_Binding_Event_Wheel;

#else
#ifndef E_BINDINGS_H
#define E_BINDINGS_H

struct E_Binding_Event_Mouse_Button
{
   int               button; /**< Mouse button number that was raised (1 - 32) */

   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   unsigned int modifiers; /**< modifier keys pressed during the event */
   unsigned int timestamp;
   Eina_Bool double_click E_BITFIELD;
   Eina_Bool triple_click E_BITFIELD;
   Eina_Bool hold E_BITFIELD;
   Eina_Bool scroll E_BITFIELD;
};

struct E_Binding_Event_Wheel
{
   int direction;
   int z;

   Evas_Coord_Point  canvas; /**< The X/Y location of the cursor */

   unsigned int modifiers; /**< modifier keys pressed during the event */
   unsigned int timestamp;
};

struct _E_Binding_Mouse
{
   E_Binding_Context ctxt;
   int button;
   E_Binding_Modifier mod;
   unsigned char any_mod E_BITFIELD;
   const char *action;
   const char *params;
};

struct _E_Binding_Key
{
   E_Binding_Context ctxt;
   const char *key;
   E_Binding_Modifier mod;
   unsigned char any_mod E_BITFIELD;
   const char *action;
   const char *params;
};

struct _E_Binding_Edge
{
   E_Binding_Context ctxt;
   E_Zone_Edge edge;
   E_Binding_Modifier mod;
   const char *action;
   const char *params;

   float delay;
   Ecore_Timer *timer;
   Eina_Bool any_mod E_BITFIELD;
   Eina_Bool drag_only E_BITFIELD;
};

struct _E_Binding_Signal
{
   E_Binding_Context ctxt;
   const char *sig;
   const char *src;
   E_Binding_Modifier mod;
   unsigned char any_mod E_BITFIELD;
   const char *action;
   const char *params;
};

struct _E_Binding_Wheel
{
   E_Binding_Context ctxt;
   int direction;
   int z;
   E_Binding_Modifier mod;
   unsigned char any_mod E_BITFIELD;
   const char *action;
   const char *params;
};

struct _E_Binding_Acpi
{
   E_Binding_Context ctxt;
   int type, status;
   const char *action, *params;
};

struct _E_Binding_Swipe
{
   E_Binding_Context ctxt;
   double direction, length, error;
   unsigned int fingers;
   const char *action, *params;
};


EINTERN int         e_bindings_init(void);
EINTERN int         e_bindings_shutdown(void);

E_API void        e_bindings_mouse_reset(void);
E_API void        e_bindings_key_reset(void);
E_API void        e_bindings_wheel_reset(void);
E_API void        e_bindings_edge_reset(void);
E_API void        e_bindings_signal_reset(void);
E_API void        e_bindings_swipe_reset(void);
E_API void        e_bindings_reset(void);

E_API void        e_bindings_mouse_add(E_Binding_Context ctxt, int button, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API void        e_bindings_mouse_del(E_Binding_Context ctxt, int button, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API void        e_bindings_mouse_grab(E_Binding_Context ctxt, Ecore_X_Window win);
E_API void        e_bindings_mouse_ungrab(E_Binding_Context ctxt, Ecore_X_Window win);
E_API E_Action   *e_bindings_mouse_button_find(E_Binding_Context ctxt, E_Binding_Event_Mouse_Button *ev, E_Binding_Mouse **bind_ret);
E_API E_Action   *e_bindings_mouse_down_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Binding_Event_Mouse_Button *ev);
E_API E_Action   *e_bindings_mouse_down_evas_event_handle(E_Binding_Context ctxt, E_Object *obj, Evas_Event_Mouse_Down *ev);
E_API E_Action   *e_bindings_mouse_down_ecore_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_Event_Mouse_Button *ev);
E_API E_Action   *e_bindings_mouse_up_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Binding_Event_Mouse_Button *ev);
E_API E_Action   *e_bindings_mouse_up_evas_event_handle(E_Binding_Context ctxt, E_Object *obj, Evas_Event_Mouse_Up *ev);
E_API E_Action   *e_bindings_mouse_up_ecore_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_Event_Mouse_Button *ev);
E_API char       *e_bindings_mouse_action_modifiers_text_generate(E_Binding_Context ctxt, const char *action, unsigned int *modifiers, unsigned int *button);

E_API void        e_bindings_key_add(E_Binding_Context ctxt, const char *key, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API void        e_bindings_key_del(E_Binding_Context ctxt, const char *key, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API E_Binding_Key *e_bindings_key_get(const char *action);
E_API E_Binding_Key *e_bindings_key_find(const char *key, E_Binding_Modifier mod, int any_mod);
E_API void        e_bindings_key_grab(E_Binding_Context ctxt, Ecore_X_Window win);
E_API void        e_bindings_key_ungrab(E_Binding_Context ctxt, Ecore_X_Window win);
E_API E_Action   *e_bindings_key_down_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_Event_Key *ev);
E_API E_Action   *e_bindings_key_up_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_Event_Key *ev);
E_API E_Action   *e_bindings_key_event_find(E_Binding_Context ctxt, Ecore_Event_Key *ev, E_Binding_Key **bind_ret);

E_API Eina_Bool   e_bindings_key_allowed(const char *key);

E_API void        e_bindings_edge_add(E_Binding_Context ctxt, E_Zone_Edge edge, Eina_Bool drag_only, E_Binding_Modifier mod, int any_mod, const char *action, const char *params, float delay);
E_API Eina_Bool   e_bindings_edge_flippable_get(E_Zone_Edge edge);
E_API Eina_Bool   e_bindings_edge_non_flippable_get(E_Zone_Edge edge);
E_API E_Action *e_bindings_edge_event_find(E_Binding_Context ctxt, E_Event_Zone_Edge *ev, Eina_Bool click, E_Binding_Edge **bind_ret);
E_API E_Binding_Edge *e_bindings_edge_get(const char *action, E_Zone_Edge edge, int click);
E_API void        e_bindings_edge_del(E_Binding_Context ctxt, E_Zone_Edge edge, Eina_Bool drag_only, E_Binding_Modifier mod, int any_mod, const char *action, const char *params, float delay);
E_API E_Action   *e_bindings_edge_in_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Event_Zone_Edge *ev);
E_API E_Action   *e_bindings_edge_out_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Event_Zone_Edge *ev);
E_API E_Action   *e_bindings_edge_down_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Event_Zone_Edge *ev);
E_API E_Action   *e_bindings_edge_up_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Event_Zone_Edge *ev);

E_API void        e_bindings_signal_add(E_Binding_Context ctxt, const char *sig, const char *src, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API void        e_bindings_signal_del(E_Binding_Context ctxt, const char *sig, const char *src, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API E_Action   *e_bindings_signal_find(E_Binding_Context ctxt, const char *sig, const char *src, E_Binding_Signal **bind_ret);
E_API E_Action   *e_bindings_signal_handle(E_Binding_Context ctxt, E_Object *obj, const char *sig, const char *src);

E_API void        e_bindings_wheel_add(E_Binding_Context ctxt, int direction, int z, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API void        e_bindings_wheel_del(E_Binding_Context ctxt, int direction, int z, E_Binding_Modifier mod, int any_mod, const char *action, const char *params);
E_API void        e_bindings_wheel_grab(E_Binding_Context ctxt, Ecore_X_Window win);
E_API void        e_bindings_wheel_ungrab(E_Binding_Context ctxt, Ecore_X_Window win);
E_API E_Action   *e_bindings_wheel_find(E_Binding_Context ctxt, E_Binding_Event_Wheel *ev, E_Binding_Wheel **bind_ret);
E_API E_Action   *e_bindings_wheel_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Binding_Event_Wheel *ev);
E_API E_Action   *e_bindings_wheel_evas_event_handle(E_Binding_Context ctxt, E_Object *obj, Evas_Event_Mouse_Wheel *ev);
E_API E_Action   *e_bindings_wheel_ecore_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_Event_Mouse_Wheel *ev);

E_API void e_bindings_acpi_add(E_Binding_Context ctxt, int type, int status, const char *action, const char *params);
E_API void e_bindings_acpi_del(E_Binding_Context ctxt, int type, int status, const char *action, const char *params);
E_API E_Action *e_bindings_acpi_find(E_Binding_Context ctxt, E_Event_Acpi *ev, E_Binding_Acpi **bind_ret);
E_API E_Action *e_bindings_acpi_event_handle(E_Binding_Context ctxt, E_Object *obj, E_Event_Acpi *ev);

typedef struct {
   const char *name;
   double acceptance; //0 to 1
} E_Binding_Swipe_Candidate;

/**
 * Direction is in radiens, 0 is pointing to the right. Going clockwise. (Only positive range)
 */
E_API Eina_Bool     e_bindings_swipe_available(void);
E_API void          e_bindings_swipe_add(E_Binding_Context ctxt, double direction, double length, unsigned int fingers, double error, const char *action, const char *params);
E_API void          e_bindings_swipe_del(E_Binding_Context ctxt, double direction, double length, unsigned int fingers, double error, const char *action, const char *params);
E_API E_Action*     e_bindings_swipe_handle(E_Binding_Context ctxt, E_Object *obj, double direction, double length, unsigned int fingers);
E_API Eina_Inarray/*<E_Bindings_Swipe_Candidate>*/* e_bindings_swipe_find_candidates(E_Binding_Context ctxt, double direction, double lenght, unsigned int fingers);

E_API int e_bindings_evas_modifiers_convert(Evas_Modifier *modifiers);
E_API int e_bindings_modifiers_to_ecore_convert(E_Binding_Modifier modifiers);
E_API void e_bindings_evas_event_mouse_button_convert(const Evas_Event_Mouse_Down *ev, E_Binding_Event_Mouse_Button *event);
E_API void e_bindings_evas_event_mouse_wheel_convert(const Evas_Event_Mouse_Wheel *ev, E_Binding_Event_Wheel *event);
E_API void e_bindings_ecore_event_mouse_button_convert(const Ecore_Event_Mouse_Button *ev, E_Binding_Event_Mouse_Button *event);
E_API void e_bindings_ecore_event_mouse_wheel_convert(const Ecore_Event_Mouse_Wheel *ev, E_Binding_Event_Wheel *event);

E_API int e_bindings_context_match(E_Binding_Context bctxt, E_Binding_Context ctxt);
E_API E_Binding_Modifier e_bindings_modifiers_from_ecore(unsigned int modifiers);

E_API void e_bindings_disabled_set(Eina_Bool disabled);
#endif
#endif
