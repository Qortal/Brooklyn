
/**
 * Move window to coordinates that do not account client decorations yet.
 *
 * This call will consider given position does not account client
 * decoration, so these values (e_comp_object_frame) will be
 * accounted automatically. This is specially useful when it is a new
 * client and has not be evaluated yet, in this case
 * the frame will be zeroed and no information is known. It
 * will mark pending requests so client will be accounted on
 * evalutation phase.
 *
 * @parm x horizontal position to place window.
 * @parm y vertical position to place window.
 *
 * @see e_client_move()
 */
static inline void
e_client_util_move_without_frame(E_Client *ec, int x, int y)
{
   if (!ec) return;
   e_comp_object_frame_xy_adjust(ec->frame, x, y, &x, &y);
   evas_object_move(ec->frame, x, y);
}

/**
 * Resize window to values that do not account client decorations yet.
 *
 * This call will consider given size and does not for account client
 * decoration, so these values (e_comp_object_frame) will be
 * accounted for automatically. This is specially useful when it is a new
 * client and has not been evaluated yet, in this case
 * e_comp_object_frame will be zeroed and no information is known. It
 * will mark pending requests so the client will be accounted for on
 * evalutation phase.
 *
 * @parm w horizontal window size.
 * @parm h vertical window size.
 *
 * @see e_client_resize()
 */
static inline void
e_client_util_resize_without_frame(E_Client *ec, int w, int h)
{
   if (!ec) return;
   e_comp_object_frame_wh_adjust(ec->frame, w, h, &w, &h);
   evas_object_resize(ec->frame, w, h);
}

/**
 * Move and resize window to values that do not account for client decorations yet.
 *
 * This call will consider given values already accounts client
 * decorations, so it will not be considered later. This will just
 * work properly with clients that have being evaluated and client
 * decorations are known (e_comp_object_frame).
 *
 * @parm x horizontal position to place window.
 * @parm y vertical position to place window.
 * @parm w horizontal window size.
 * @parm h vertical window size.
 *
 * @see e_client_move_resize()
 */
static inline void
e_client_util_move_resize_without_frame(E_Client *ec, int x, int y, int w, int h)
{
   e_client_util_move_without_frame(ec, x, y);
   e_client_util_resize_without_frame(ec, w, h);
}

static inline Eina_Bool
e_client_util_ignored_get(const E_Client *ec)
{
   if (!ec) return EINA_TRUE;
   return ec->override || ec->input_only || ec->ignored;
}

static inline Eina_Bool
e_client_util_is_popup(const E_Client *ec)
{
   if (!ec) return EINA_FALSE;
   switch (ec->netwm.type)
     {
      case E_WINDOW_TYPE_MENU:
      case E_WINDOW_TYPE_SPLASH:
      case E_WINDOW_TYPE_DROPDOWN_MENU:
      case E_WINDOW_TYPE_POPUP_MENU:
      case E_WINDOW_TYPE_TOOLTIP:
      case E_WINDOW_TYPE_NOTIFICATION:
      case E_WINDOW_TYPE_COMBO:
      case E_WINDOW_TYPE_DND:
        return EINA_TRUE;
      default: break;
     }
   return EINA_FALSE;
}

static inline Eina_Bool
e_client_util_desk_visible(const E_Client *ec, const E_Desk *desk)
{
   if (!ec) return EINA_FALSE;
   return !ec->desk || ec->sticky || (ec->desk == desk);
}

static inline Ecore_Window
e_client_util_pwin_get(const E_Client *ec)
{
   if (!ec->pixmap) return 0;
#if defined(HAVE_WAYLAND) && !defined(HAVE_WAYLAND_ONLY)
   return e_pixmap_parent_window_get(e_comp_x_client_pixmap_get(ec));
#else
   return e_pixmap_parent_window_get(ec->pixmap);
#endif
}

static inline Ecore_Window
e_client_util_win_get(const E_Client *ec)
{
   if (!ec->pixmap) return 0;
#if defined(HAVE_WAYLAND) && !defined(HAVE_WAYLAND_ONLY)
   return e_pixmap_window_get(e_comp_x_client_pixmap_get(ec));
#else
   return e_pixmap_window_get(ec->pixmap);
#endif
}

static inline Eina_Bool
e_client_util_resizing_get(const E_Client *ec)
{
   if (!ec) return EINA_FALSE;
   return (ec->resize_mode != E_POINTER_RESIZE_NONE);
}

static inline Eina_Bool
e_client_util_borderless(const E_Client *ec)
{
   if (!ec) return EINA_FALSE;
   return (ec->borderless || ec->mwm.borderless || (!ec->border.name) || (!strcmp(ec->border.name, "borderless")));
}

static inline Eina_Bool
e_client_util_shadow_state_get(const E_Client *ec)
{
   Eina_Bool on;
   if (ec->shaped) return EINA_FALSE;
   if (ec->argb)
     {
        return (!ec->borderless) && (ec->bordername || (ec->border.name && strcmp(ec->border.name, "borderless")));
     }
   on = !ec->e.state.video;
   if (on)
     on = !ec->fullscreen;
   return on;
}

static inline Eina_Stringshare *
e_client_util_name_get(const E_Client *ec)
{
   if (!ec) return NULL;
   if (ec->netwm.name)
     return ec->netwm.name;
   else if (ec->icccm.title)
     return ec->icccm.title;
   return NULL;
}

static inline E_Client *
e_client_util_top_parent_get(const E_Client *ec)
{
   E_Client *parent = ec->parent;

   while (parent->parent) parent = parent->parent;
   return parent;
}
