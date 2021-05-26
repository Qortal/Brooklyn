#ifdef E_TYPEDEFS

typedef struct _E_Comp_X_Client_Data E_Comp_X_Client_Data;

#else
# ifndef E_COMP_X_H
#  define E_COMP_X_H
#  include <Ecore_X.h>
#  include "e_atoms.h"
#  include "e_hints.h"

struct _E_Comp_X_Client_Data
{
   Ecore_X_Window lock_win;

   Ecore_X_Damage       damage;  // damage region
   Ecore_X_Visual       vis;  // window visual
   Ecore_X_Colormap     cmap; // colormap of window
   int pw, ph; //XPRESENT!
   Eina_Rectangle shape; //SHAPE

#if 0 //NOT USED
   Ecore_X_Pixmap       cache_pixmap;  // the cached pixmap (1/nth the dimensions)
   int                  cache_w, cache_h;  // cached pixmap size
#endif

   Ecore_X_Image       *xim;  // x image - software fallback
   Ecore_X_Sync_Alarm   alarm;
   Ecore_X_Sync_Counter sync_counter;  // netwm sync counter

   Ecore_X_Window_Attributes initial_attributes;

   unsigned int move_counter; //reduce X calls when moving a window
   unsigned int internal_props_set; //don't need to refetch our own internal props

   Ecore_Timer *first_draw_delay; //configurable placebo
   Eina_Bool first_damage E_BITFIELD; //ignore first damage on non-re_manage clients

   unsigned int parent_activate_count; //number of times a win has activated itself when parent was focused

   struct
   {
      struct
      {
         struct
         {
            unsigned char conformant E_BITFIELD;
         } fetch;
         unsigned char conformant E_BITFIELD;
      } conformant;
      struct
      {
         struct
         {
            unsigned char state E_BITFIELD;
            struct
            {
               unsigned int major E_BITFIELD;
               unsigned int minor E_BITFIELD;
            } priority;
            unsigned char quickpanel E_BITFIELD;
            unsigned char zone E_BITFIELD;
         } fetch;
         Ecore_X_Illume_Quickpanel_State state;
         struct
         {
            unsigned int major E_BITFIELD;
            unsigned int minor E_BITFIELD;
         } priority;
         unsigned char                   quickpanel E_BITFIELD;
         int                             zone;
      } quickpanel;
      struct
      {
         struct
         {
            unsigned char drag E_BITFIELD;
            unsigned char locked E_BITFIELD;
         } fetch;
         unsigned char drag E_BITFIELD;
         unsigned char locked E_BITFIELD;
      } drag;
      struct
      {
         struct
         {
            unsigned char state E_BITFIELD;
         } fetch;
         Ecore_X_Illume_Window_State state;
      } win_state;
   } illume;
   Ecore_X_Stack_Type stack;
#ifdef HAVE_WAYLAND
   uint32_t surface_id;
#endif

   Eina_Bool moving E_BITFIELD;
   Eina_Bool first_map E_BITFIELD;
   Eina_Bool change_icon E_BITFIELD;
   Eina_Bool need_reparent E_BITFIELD;
   Eina_Bool reparented E_BITFIELD;
   Eina_Bool deleted E_BITFIELD;
   Eina_Bool button_grabbed E_BITFIELD;
   Eina_Bool fetch_exe E_BITFIELD;
   Eina_Bool set_win_type E_BITFIELD;
   Eina_Bool frame_update E_BITFIELD;
   Eina_Bool evas_init E_BITFIELD;
   Eina_Bool unredirected_single E_BITFIELD;
   Eina_Bool fetch_gtk_frame_extents E_BITFIELD;
   Eina_Bool iconic E_BITFIELD;
};

E_API Eina_Bool e_comp_x_init(void);
E_API void e_comp_x_shutdown(void);

E_API void e_alert_composite_win(Ecore_X_Window root, Ecore_X_Window win);
EINTERN void e_comp_x_nocomp_end(void);
EINTERN void e_comp_x_xwayland_client_setup(E_Client *ec, E_Client *wc);

E_API E_Pixmap *e_comp_x_client_pixmap_get(const E_Client *ec);

EINTERN Eina_Bool _e_comp_x_screensaver_on();
EINTERN Eina_Bool _e_comp_x_screensaver_off();

# endif
#endif
