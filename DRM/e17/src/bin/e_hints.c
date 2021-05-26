#include "e.h"

#ifndef HAVE_WAYLAND_ONLY
static void e_hints_e16_comms_pretend(Ecore_X_Window root, Ecore_X_Window propwin);
static void e_hints_openoffice_gnome_fake(Ecore_Window root);
//static void e_hints_openoffice_kde_fake(Ecore_Window root);

E_API Ecore_X_Atom ATM__QTOPIA_SOFT_MENU = 0;
E_API Ecore_X_Atom ATM__QTOPIA_SOFT_MENUS = 0;
E_API Ecore_X_Atom ATM_GNOME_SM_PROXY = 0;
E_API Ecore_X_Atom ATM_ENLIGHTENMENT_COMMS = 0;
E_API Ecore_X_Atom ATM_ENLIGHTENMENT_VERSION = 0;
E_API Ecore_X_Atom ATM_ENLIGHTENMENT_SCALE = 0;

E_API Ecore_X_Atom ATM_NETWM_SHOW_WINDOW_MENU = 0;
E_API Ecore_X_Atom ATM_NETWM_PERFORM_BUTTON_ACTION = 0;
E_API Ecore_X_Atom ATM_GTK_FRAME_EXTENTS = 0;

E_API Ecore_X_Atom ATM_STEAM_GAME = 0;
#endif

EINTERN void
e_hints_init(Ecore_Window root, Ecore_Window propwin)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)root;
   (void)propwin;
#else
   const char *atom_names[] = {
      "_QTOPIA_SOFT_MENU",
      "_QTOPIA_SOFT_MENUS",
      "GNOME_SM_PROXY",
      "ENLIGHTENMENT_COMMS",
      "ENLIGHTENMENT_VERSION",
      "ENLIGHTENMENT_SCALE",
      "_NET_WM_SHOW_WINDOW_MENU",
      "_NET_WM_PERFORM_BUTTON_ACTION",
      "_GTK_FRAME_EXTENTS",
      "STEAM_GAME",
   };
   Ecore_X_Atom atoms[EINA_C_ARRAY_LENGTH(atom_names)];
   Ecore_X_Atom supported[46];
   int supported_num;
   Ecore_X_Window win, twin;
   int nwins;
   char *name;
   double ts;

   ecore_x_atoms_get(atom_names, EINA_C_ARRAY_LENGTH(atom_names), atoms);
   ATM__QTOPIA_SOFT_MENU = atoms[0];
   ATM__QTOPIA_SOFT_MENUS = atoms[1];
   ATM_GNOME_SM_PROXY = atoms[2];
   ATM_ENLIGHTENMENT_COMMS = atoms[3];
   ATM_ENLIGHTENMENT_VERSION = atoms[4];
   ATM_ENLIGHTENMENT_SCALE = atoms[5];
   ATM_NETWM_SHOW_WINDOW_MENU = atoms[6];
   ATM_NETWM_PERFORM_BUTTON_ACTION = atoms[7];
   ATM_GTK_FRAME_EXTENTS = atoms[8];
   ATM_STEAM_GAME = atoms[9];

   supported_num = 0;
   /* Set what hints we support */
   /* Root Window Properties (and Related Messages) */
   supported[supported_num++] = ECORE_X_ATOM_NET_CLIENT_LIST;
   supported[supported_num++] = ECORE_X_ATOM_NET_CLIENT_LIST_STACKING;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_DESKTOP_GEOMETRY, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_DESKTOP_VIEWPORT, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_CURRENT_DESKTOP, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_DESKTOP_NAMES, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_ACTIVE_WINDOW;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WORKAREA, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK;
   supported[supported_num++] = ECORE_X_ATOM_NET_VIRTUAL_ROOTS;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_DESKTOP_LAYOUT, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_SHOWING_DESKTOP, 1);*/

   /* Other Root Window Messages */
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_CLOSE_WINDOW, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_MOVERESIZE_WINDOW, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_MOVERESIZE;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_RESTACK_WINDOW, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS;

   /* Application Window Properties */
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_NAME;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_VISIBLE_NAME;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_ICON_NAME;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_DESKTOP, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_MODAL;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_STICKY;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_SHADED;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_HIDDEN;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_ABOVE;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STATE_BELOW;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_MOVE, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_RESIZE, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_SHADE, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_STICK, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP, 1);*/
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ACTION_CLOSE, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STRUT;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_STRUT_PARTIAL;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_ICON;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_PID;
   /*ecore_x_netwm_supported(roots[supported_num], ECORE_X_ATOM_NET_WM_HANDLED_ICONS, 1);*/
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_USER_TIME;
   supported[supported_num++] = ECORE_X_ATOM_NET_FRAME_EXTENTS;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_PING;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;
   supported[supported_num++] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER;
   supported[supported_num++] = ECORE_X_ATOM_E_VIDEO_PARENT;
   supported[supported_num++] = ECORE_X_ATOM_E_VIDEO_POSITION;

   supported[supported_num++] = ATM_NETWM_SHOW_WINDOW_MENU;
   supported[supported_num++] = ATM_NETWM_PERFORM_BUTTON_ACTION;
   supported[supported_num++] = ATM_GTK_FRAME_EXTENTS;




   /* check for previous netwm wm and wait for it to die */
   ts = ecore_time_get();
   nwins = ecore_x_window_prop_window_get(root,
                                          ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK,
                                          &win, 1);
   if ((nwins > 0) && (win != propwin))
     {
        twin = win;
        for (;; )
          {
             Ecore_X_Window selfwin = 0;

             /* check that supporting wm win points to itself to be valid */
             nwins = ecore_x_window_prop_window_get(twin,
                                                    ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK,
                                                    &selfwin, 1);
             if (nwins < 1) break;
             if (selfwin != twin) break;
             /* check the wm is e */
             if (ecore_x_netwm_name_get(twin, &name))
               {
                  if (name)
                    {
                       /* if it is NOT e - don't care here as all this code is dealing with e restarts */
                       if (strcmp(name, "Enlightenment"))
                         {
                            free(name);
                            break;
                         }
                       free(name);
                    }
                  /* no name - not e - don't care */
                  else
                    break;
               }
             else
               /* can't get name - obviously not e */
               break;
             /* have we been spinning too long? 2 sec */
             if ((ecore_time_get() - ts) > 2.0)
               {
                  e_error_message_show(_("A previous instance of Enlightenment is still active\n"
                                         "on this screen. Aborting startup.\n"));
                  exit(101);
               }
             /* get/check agan */
             nwins = ecore_x_window_prop_window_get(root,
                                                    ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK,
                                                    &twin, 1);
             if (nwins < 1) break;
             if (twin != win) break;
          }
     }

/*
* I don't FUCKING believe it. if we PRETEND we are Kwin - java is happy.
* why? it expects a double reparenting wm then. java insists on finding this
* out when it should be irrelevant! stupid code! I can't believe the time we
* just wasted hunting a bug that wasn't and that is due to sheer stupid
* coding (in java's awt layer that swing also uses).
*/
/* Now for more stupidity... Openoffice.org will change its look and feel
* depending on what wm it thinks there is... so if we pretend to be Kwin...
* it tries to use kde preferences, if found.
*/
/* I have disabled this now by pretending to be E16 with e16 comms. this
* means java plays nice and uses our FRAME property.. but we had to do other
* evil stuff as java EXPECTS all this at REPARENT time... i've deferred
* reparenting... i hate java!
*/
/*	     ecore_x_netwm_wm_identify(root, win, "KWin");*/
   ecore_x_netwm_wm_identify(root, propwin, "Enlightenment");
/* this makes openoffice.org read gtk settings so it doesn't look like shit */
   e_hints_openoffice_gnome_fake(root);

   ecore_x_netwm_supported_set(root, supported, supported_num);

   /* fake mwm, this might crash some ol' motif apps, if
      they still exist, but at least it makes borderless
      feature of Eterm and urxvt work... */
   ecore_x_atom_get("_MOTIF_WM_INFO");
   e_hints_e16_comms_pretend(root, propwin);
   ecore_x_sync();
#endif
}

#ifndef HAVE_WAYLAND_ONLY
/*
 * This is here so we don't have to pretend to be Kwin anymore - we pretend
 * to do old e16 style ipc. in fact we just ignore it... but set up the
 * window port anyway
 */
static void
e_hints_e16_comms_pretend(Ecore_X_Window root, Ecore_X_Window propwin)
{
   char buf[256];

   /* to help detect this is NOT e16 */
   snprintf(buf, sizeof(buf), "Enlightenment %s", VERSION);
   ecore_x_window_prop_property_set(propwin, ATM_ENLIGHTENMENT_VERSION, ECORE_X_ATOM_STRING, 8, buf, strlen(buf));
   ecore_x_window_prop_property_set(root, ATM_ENLIGHTENMENT_VERSION, ECORE_X_ATOM_STRING, 8, buf, strlen(buf));

   snprintf(buf, sizeof(buf), "WINID %8x", (int)propwin);
   ecore_x_window_prop_property_set(propwin, ATM_ENLIGHTENMENT_COMMS, ECORE_X_ATOM_STRING, 8, buf, 14);

   ecore_x_window_prop_property_set(root, ATM_ENLIGHTENMENT_COMMS, ECORE_X_ATOM_STRING, 8, buf, 14);
}
#endif

#if 0
THIS FUNCTION DOES NOTHING!!!!
EINTERN void
e_hints_manager_init(E_Manager *man)
{
   /* Set desktop count, desktop names and workarea */
   int i = 0, num = 0;
   unsigned int *areas = NULL;
   Eina_List *cl;
   E_Container *c;
   Ecore_X_Window *vroots = NULL;
   /* FIXME: Desktop names not yet implemented */
/*   char			**names; */

   e_hints_e16_comms_pretend(man);

   num = eina_list_count(man->containers);

   vroots = calloc(num, sizeof(Ecore_X_Window));
   if (!vroots) return;

/*   names = calloc(num, sizeof(char *));*/

   areas = calloc(4 * num, sizeof(unsigned int));
   if (!areas)
     {
        free(vroots);
        return;
     }

   EINA_LIST_FOREACH(man->containers, cl, c)
     {
        areas[4 * i] = c->x;
        areas[4 * i + 1] = c->y;
        areas[4 * i + 2] = c->w;
        areas[4 * i + 3] = c->h;
        vroots[i++] = c->win;
     }

#if 0
   ecore_x_netwm_desk_count_set(man->root, num);
   /* No need for workarea without desktops */
   ecore_x_netwm_desk_workareas_set(man->root, num, areas);
#endif

   free(vroots);
   free(areas);
}
#endif

E_API void
e_hints_client_list_set(void)
{
#ifdef HAVE_WAYLAND_ONLY
#else
   /* Get client count by adding client lists on all containers */
   unsigned int i = 0;
   Ecore_X_Window *clients = NULL;

   if (!e_comp_util_has_x()) return;
   if (e_comp->clients)
     {
        E_Client *ec;
        const Eina_List *ll;

        clients = calloc(e_clients_count(), sizeof(Ecore_X_Window));
        EINA_LIST_FOREACH(e_comp->clients, ll, ec)
          {
             if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_X) continue;
             clients[i++] = e_client_util_win_get(ec);
          }
     }
   ecore_x_netwm_client_list_set(e_comp->root, clients, i);
   free(clients);
#endif
}

/* Client list is already in stacking order, so this function is nearly
 * identical to the previous one */
E_API void
e_hints_client_stacking_set(void)
{
#ifdef HAVE_WAYLAND_ONLY
#else
   unsigned int c, i = 0, non_x = 0;
   Ecore_X_Window *clients = NULL;

//#define CLIENT_STACK_DEBUG
   /* Get client count */
   c = e_clients_count();
   if (c)
     {
        E_Client *ec;
#ifdef CLIENT_STACK_DEBUG
        Eina_List *ll = NULL;
#endif
        clients = calloc(c, sizeof(Ecore_X_Window));
        E_CLIENT_FOREACH(ec)
          {
             if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_X)
               {
                  non_x++;
                  continue;
               }
             clients[i++] = e_client_util_win_get(ec);
#ifdef CLIENT_STACK_DEBUG
             ll = eina_list_append(ll, ec);
#endif
             if (i > c)
               {
                  CRI("Window list size greater than window count.");
                  break;
               }
          }

        if (i < c - non_x)
          {
#ifdef CLIENT_STACK_DEBUG
             Eina_List *lll = eina_list_clone(e_comp->clients);

             EINA_LIST_FREE(ll, ec)
               lll = eina_list_remove(lll, ec);
             EINA_LIST_FREE(lll, ec)
               WRN("Missing %p: %snew client", ec, ec->new_client ? "" : "not ");
#endif
             CRI("Window list size less than window count.");
          }
     }
   /* XXX: it should be "more correct" to be setting the stacking atom as "windows per root"
    * since any apps using it are probably not going to want windows from other screens
    * to be returned in the list
    */
   if (i <= c)
     ecore_x_netwm_client_list_stacking_set(e_comp->root, clients, c);
   free(clients);
#endif
}

E_API void
e_hints_active_window_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   if (!e_comp_util_has_x()) return;
   if (ec && (e_pixmap_type_get(ec->pixmap) == E_PIXMAP_TYPE_X))
     ecore_x_netwm_client_active_set(e_comp->root, e_client_util_win_get(ec));
   else
     ecore_x_netwm_client_active_set(e_comp->root, 0);
#endif
}

EINTERN void
e_hints_window_init(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   E_Remember *rem = NULL;

   if (!e_client_has_xwindow(ec)) return;
   if (ec->remember)
     rem = ec->remember;

   if ((rem) && (rem->apply & E_REMEMBER_APPLY_LAYER))
     {
        ec->layer = rem->prop.layer;
        evas_object_layer_set(ec->frame, ec->layer);
     }
   else
     {
        if (!ec->lock_client_stacking)
          {
             if (ec->netwm.type == E_WINDOW_TYPE_DESKTOP)
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_DESKTOP);
             else if (ec->netwm.state.stacking == E_STACKING_BELOW)
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_BELOW);
             else if (ec->netwm.state.stacking == E_STACKING_ABOVE)
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_ABOVE);
             else if (ec->netwm.type == E_WINDOW_TYPE_DOCK)
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_ABOVE);
             else if (!evas_object_layer_get(ec->frame)) //impossible?
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_NORMAL);
          }
        else
          evas_object_raise(ec->frame);
     }

   if ((ec->parent) && (e_config->transient.layer))
     evas_object_layer_set(ec->frame, ec->parent->layer);

#if 0
   /* Ignore this, E has incompatible desktop setup */
   if (ecore_x_netwm_desktop_get(e_client_util_win_get(ec), &ec->netwm.desktop))
     {
        if (ec->netwm.desktop == 0xffffffff)
          {
             e_client_stick(ec);
          }
        else if (ec->netwm.desktop < (ec->zone->desk_x_count * ec->zone->desk_y_count))
          {
             E_Desk *desk;

             desk = e_desk_at_pos_get(ec->zone, ec->netwm.desktop);
             if (desk)
               e_client_desk_set(ec, desk);
          }
        else
          {
             /* Update netwm desktop with current desktop */
             e_hints_window_desktop_set(ec);
          }
     }
   else
     {
        /* Update netwm desktop with current desktop */
        e_hints_window_desktop_set(ec);
     }
#endif

   if (ec->netwm.state.sticky)
     {
        if (!ec->lock_client_sticky)
          e_client_stick(ec);
        else
          e_hints_window_sticky_set(ec, 0);
     }
   if (ec->netwm.state.shaded)
     {
        if (!ec->lock_client_shade)
          e_client_shade(ec, e_hints_window_shade_direction_get(ec));
        else
          e_hints_window_shaded_set(ec, 0);
     }
   if ((ec->netwm.state.maximized_v) && (ec->netwm.state.maximized_h))
     {
        if (!ec->lock_client_maximize)
          {
             e_hints_window_size_get(ec);
             e_client_maximize(ec, e_config->maximize_policy);
          }
        else
          e_hints_window_maximized_set(ec, 0, 0);
     }
   else if (ec->netwm.state.maximized_h)
     {
        if (!ec->lock_client_maximize)
          {
             e_hints_window_size_get(ec);
             e_client_maximize(ec, (e_config->maximize_policy & E_MAXIMIZE_TYPE) | E_MAXIMIZE_HORIZONTAL);
          }
        else
          e_hints_window_maximized_set(ec, 0, 0);
     }
   else if (ec->netwm.state.maximized_v)
     {
        if (!ec->lock_client_maximize)
          {
             e_hints_window_size_get(ec);
             do
               {
                  if (ec->client.w == (ec->zone->w / 2))
                    {
                       if (!ec->client.x)
                         {
                            e_client_maximize(ec, (e_config->maximize_policy & E_MAXIMIZE_TYPE) | E_MAXIMIZE_LEFT);
                            break;
                         }
                       else if (ec->client.x == ec->zone->w / 2)
                         {
                            e_client_maximize(ec, (e_config->maximize_policy & E_MAXIMIZE_TYPE) | E_MAXIMIZE_RIGHT);
                            break;
                         }
                    }
                  e_client_maximize(ec, (e_config->maximize_policy & E_MAXIMIZE_TYPE) | E_MAXIMIZE_VERTICAL);
               } while (0);
          }
        else
          e_hints_window_maximized_set(ec, 0, 0);
     }
   if (ec->netwm.state.fullscreen)
     {
        if (!ec->lock_client_fullscreen)
          {
             e_hints_window_size_get(ec);
             e_client_fullscreen(ec, e_config->fullscreen_policy);
          }
        else
          e_hints_window_fullscreen_set(ec, 0);
     }
   if ((ec->icccm.state == ECORE_X_WINDOW_STATE_HINT_ICONIC) &&
       (ec->netwm.state.hidden))
     {
        if (!ec->lock_client_iconify)
          e_client_iconify(ec);
        else
          e_hints_window_visible_set(ec);
     }
   else if ((ec->parent) && (e_config->transient.iconify) && (ec->parent->iconic))
     e_client_iconify(ec);
   /* If a window isn't iconic, and is one the current desk,
    * show it! */
   else if (ec->desk == e_desk_current_get(ec->zone))
     {
        /* ...but only if it's supposed to be shown */
        if (ec->re_manage && (ec->icccm.state != ECORE_X_WINDOW_STATE_HINT_WITHDRAWN))
          {
             ec->changes.visible = 1;
             ec->visible = 1;
          }
     }
   /* e hints */
/*
   if (ec->e.state.centered)
     {
        e_client_center(ec);
     }
 */
#endif
}

E_API void
e_hints_window_state_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   Ecore_X_Window_State state[10];
   int num = 0;

   if (!e_client_has_xwindow(ec)) return;
   if (ec->netwm.state.modal)
     state[num++] = ECORE_X_WINDOW_STATE_MODAL;
   if (ec->netwm.state.sticky)
     state[num++] = ECORE_X_WINDOW_STATE_STICKY;
   if (ec->netwm.state.maximized_v)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_VERT;
   if (ec->netwm.state.maximized_h)
     state[num++] = ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ;
   if (ec->netwm.state.shaded)
     state[num++] = ECORE_X_WINDOW_STATE_SHADED;
   if (ec->internal)
     {
        if (ec->netwm.state.skip_taskbar)
          state[num++] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
        if (ec->netwm.state.skip_pager)
          state[num++] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
     }
   if (ec->netwm.state.hidden)
     state[num++] = ECORE_X_WINDOW_STATE_HIDDEN;
   if (ec->netwm.state.fullscreen)
     state[num++] = ECORE_X_WINDOW_STATE_FULLSCREEN;

   switch (ec->netwm.state.stacking)
     {
      case E_STACKING_ABOVE:
        state[num++] = ECORE_X_WINDOW_STATE_ABOVE;
        break;

      case E_STACKING_BELOW:
        state[num++] = ECORE_X_WINDOW_STATE_BELOW;
        break;

      case E_STACKING_NONE:
      default:
        break;
     }
   ecore_x_netwm_window_state_set(e_client_util_win_get(ec), state, num);
   if (!ec->internal)
     {
        num = 0;
        if (ec->netwm.state.skip_taskbar)
          state[num++] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
        if (ec->netwm.state.skip_pager)
          state[num++] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
        if (num)
          ecore_x_netwm_window_state_set(e_client_util_win_get(ec), state, num);
     }
#endif
}

E_API void
e_hints_allowed_action_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   Ecore_X_Action action[10];
   int num = 0;

   if (!e_client_has_xwindow(ec)) return;
   if (ec->netwm.action.move)
     action[num++] = ECORE_X_ACTION_MOVE;
   if (ec->netwm.action.resize)
     action[num++] = ECORE_X_ACTION_RESIZE;
   if (ec->netwm.action.minimize)
     action[num++] = ECORE_X_ACTION_MINIMIZE;
   if (ec->netwm.action.shade)
     action[num++] = ECORE_X_ACTION_SHADE;
   if (ec->netwm.action.stick)
     action[num++] = ECORE_X_ACTION_STICK;
   if (ec->netwm.action.maximized_h)
     action[num++] = ECORE_X_ACTION_MAXIMIZE_HORZ;
   if (ec->netwm.action.maximized_v)
     action[num++] = ECORE_X_ACTION_MAXIMIZE_VERT;
   if (ec->netwm.action.fullscreen)
     action[num++] = ECORE_X_ACTION_FULLSCREEN;
   if (ec->netwm.action.change_desktop)
     action[num++] = ECORE_X_ACTION_CHANGE_DESKTOP;
   if (ec->netwm.action.close)
     action[num++] = ECORE_X_ACTION_CLOSE;

   ecore_x_netwm_allowed_action_set(e_client_util_win_get(ec), action, num);
#endif
}

E_API void
e_hints_window_type_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   if (!e_client_has_xwindow(ec)) return;
   ecore_x_netwm_window_type_set(e_client_util_win_get(ec),
                                 (Ecore_X_Window_Type)ec->netwm.type);
#endif
}

E_API void
e_hints_window_type_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   Ecore_X_Window_Type *types = NULL;
   int num, i, j, k;

   if (!e_client_has_xwindow(ec)) return;
   num = ecore_x_netwm_window_types_get(e_client_util_win_get(ec), &types);
   if (ec->netwm.extra_types)
     {
        free(ec->netwm.extra_types);
        ec->netwm.extra_types = NULL;
        ec->netwm.extra_types_num = 0;
     }
   if (num == 0)
     ec->netwm.type = E_WINDOW_TYPE_UNKNOWN;
   else
     {
        j = 0;
        ec->netwm.type = (E_Window_Type)types[j];
        j++;
        while ((j < num) &&
               (ec->netwm.type == E_WINDOW_TYPE_UNKNOWN))
          {
             ec->netwm.type = (E_Window_Type)types[j];
             j++;
          }
        if (num > j)
          {
             ec->netwm.extra_types =
               malloc((num - j) * sizeof(Ecore_X_Window_Type));
             if (ec->netwm.extra_types)
               {
                  for (i = j, k = 0; i < num; i++, k++)
                    ec->netwm.extra_types[k] = (E_Window_Type)types[i];
                  ec->netwm.extra_types_num = num - j;
               }
          }
        free(types);
        evas_object_pass_events_set(ec->frame, ec->netwm.type == E_WINDOW_TYPE_DND);
     }
   ec->dialog = (ec->netwm.type == E_WINDOW_TYPE_DIALOG);
   if (!ec->dialog)
     ec->tooltip = (ec->netwm.type == E_WINDOW_TYPE_TOOLTIP);
#endif
}

#ifdef E_COMP_X_H
E_API void
e_hints_window_state_update(E_Client *ec, int state, int action)
{
   if (!e_client_has_xwindow(ec)) return;
   switch (state)
     {
      case ECORE_X_WINDOW_STATE_ICONIFIED:
        if (action != ECORE_X_WINDOW_STATE_ACTION_ADD) return;
#ifndef HAVE_WAYLAND_ONLY
        if (ec->icccm.state == ECORE_X_WINDOW_STATE_HINT_ICONIC) return;
#endif
        if (ec->lock_client_iconify) return;
        e_client_iconify(ec);
        break;

      case ECORE_X_WINDOW_STATE_MODAL:
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             if (ec->netwm.state.modal)
               {
                  ec->netwm.state.modal = 0;
                  ec->netwm.update.state = 1;
                  EC_CHANGED(ec);
               }
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             if (!ec->netwm.state.modal)
               {
                  ec->netwm.state.modal = 1;
                  ec->netwm.update.state = 1;
                  EC_CHANGED(ec);
               }
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             ec->netwm.state.modal = !ec->netwm.state.modal;
             ec->netwm.update.state = 1;
             EC_CHANGED(ec);
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_STICKY:
        if (ec->lock_client_sticky) return;
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             e_client_unstick(ec);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             e_client_stick(ec);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             if (ec->sticky)
               e_client_unstick(ec);
             else
               e_client_stick(ec);
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
      case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
      case INT_MAX:
      {
         int max[] =
         {
            [ECORE_X_WINDOW_STATE_MAXIMIZED_VERT] = E_MAXIMIZE_VERTICAL,
            [ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ] = E_MAXIMIZE_HORIZONTAL,
         };
         E_Maximize m;

         if (state > ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ)
           m = E_MAXIMIZE_BOTH;
         else
           m = max[state];
         if (ec->lock_client_maximize) return;
         switch (action)
           {
            case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
              if ((ec->maximized & m) == m)
                e_client_unmaximize(ec, m);
              break;

            case ECORE_X_WINDOW_STATE_ACTION_ADD:
              if ((ec->maximized & m) == m) break;
              ec->changes.need_maximize = 1;
              ec->maximized &= ~E_MAXIMIZE_TYPE;
              ec->maximized |= (e_config->maximize_policy & E_MAXIMIZE_TYPE) | m;
              EC_CHANGED(ec);
              break;

            case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
              if ((ec->maximized & m) == m)
                {
                   e_client_unmaximize(ec, m);
                   break;
                }
              ec->changes.need_maximize = 1;
              ec->maximized &= ~E_MAXIMIZE_TYPE;
              ec->maximized |= (e_config->maximize_policy & E_MAXIMIZE_TYPE) | m;
              EC_CHANGED(ec);
              break;
           }
      }
        break;

      case ECORE_X_WINDOW_STATE_SHADED:
        if (ec->lock_client_shade) return;
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             e_client_unshade(ec, e_hints_window_shade_direction_get(ec));
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             e_client_shade(ec, e_hints_window_shade_direction_get(ec));
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             if (ec->shaded)
               e_client_unshade(ec, e_hints_window_shade_direction_get(ec));
             else
               e_client_shade(ec, e_hints_window_shade_direction_get(ec));
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             if (ec->netwm.state.skip_taskbar)
               {
                  ec->netwm.state.skip_taskbar = 0;
                  ec->netwm.update.state = 1;
                  EC_CHANGED(ec);
                  e_client_prop_misc_changed(ec);
               }
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             if (!ec->netwm.state.skip_taskbar)
               {
                  ec->netwm.state.skip_taskbar = 1;
                  ec->netwm.update.state = 1;
                  EC_CHANGED(ec);
                  e_client_prop_misc_changed(ec);
               }
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             ec->netwm.state.skip_taskbar = !ec->netwm.state.skip_taskbar;
             ec->netwm.update.state = 1;
             EC_CHANGED(ec);
             e_client_prop_misc_changed(ec);
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_SKIP_PAGER:
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             if (ec->netwm.state.skip_pager)
               {
                  ec->netwm.state.skip_pager = 0;
                  ec->netwm.update.state = 1;
                  EC_CHANGED(ec);
                  e_client_prop_misc_changed(ec);
               }
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             if (!ec->netwm.state.skip_pager)
               {
                  ec->netwm.state.skip_pager = 1;
                  ec->netwm.update.state = 1;
                  EC_CHANGED(ec);
                  e_client_prop_misc_changed(ec);
               }
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             ec->netwm.state.skip_pager = !ec->netwm.state.skip_pager;
             ec->netwm.update.state = 1;
             EC_CHANGED(ec);
             e_client_prop_misc_changed(ec);
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_HIDDEN:
        /* XXX: fixme */
        break;

      case ECORE_X_WINDOW_STATE_FULLSCREEN:
        if (ec->lock_client_fullscreen) return;
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             e_client_unfullscreen(ec);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             e_client_fullscreen(ec, e_config->fullscreen_policy);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             if (ec->fullscreen)
               e_client_unfullscreen(ec);
             else
               e_client_fullscreen(ec, e_config->fullscreen_policy);
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_ABOVE:
        if (ec->lock_client_stacking) return;
        /* FIXME: Should this require that BELOW is set to 0 first, or just
         * do it? */
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             evas_object_layer_set(ec->frame, E_LAYER_CLIENT_NORMAL);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             evas_object_layer_set(ec->frame, E_LAYER_CLIENT_ABOVE);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             if (ec->layer == E_LAYER_CLIENT_ABOVE)
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_NORMAL);
             else
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_ABOVE);
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_BELOW:
        if (ec->lock_client_stacking) return;
        /* FIXME: Should this require that ABOVE is set to 0 first, or just
         * do it? */
        switch (action)
          {
           case ECORE_X_WINDOW_STATE_ACTION_REMOVE:
             evas_object_layer_set(ec->frame, E_LAYER_CLIENT_NORMAL);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_ADD:
             evas_object_layer_set(ec->frame, E_LAYER_CLIENT_BELOW);
             break;

           case ECORE_X_WINDOW_STATE_ACTION_TOGGLE:
             if (ec->layer == E_LAYER_CLIENT_BELOW)
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_NORMAL);
             else
               evas_object_layer_set(ec->frame, E_LAYER_CLIENT_BELOW);
             break;
          }
        break;

      case ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION:
        /* FIXME */
        break;

      case ECORE_X_WINDOW_STATE_UNKNOWN:
        /* Ignore */
        break;
     }
}
#endif

E_API void
e_hints_window_state_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   unsigned int i, num, was_here = 0;
   Ecore_X_Window_State *state;

   if (!e_client_has_xwindow(ec)) return;
   ec->netwm.state.modal = 0;
   ec->netwm.state.sticky = 0;
   ec->netwm.state.maximized_v = 0;
   ec->netwm.state.maximized_h = 0;
   ec->netwm.state.shaded = 0;
   ec->netwm.state.skip_taskbar = 0;
   ec->netwm.state.skip_pager = 0;
   ec->netwm.state.hidden = 0;
   ec->netwm.state.fullscreen = 0;
   ec->netwm.state.stacking = 0;

   if (ecore_x_window_prop_card32_get(e_client_util_win_get(ec),
                                      E_ATOM_E_WAS_HERE,
                                      &was_here, 1) != 1)
     {
        unsigned int val = 1;

        was_here = 0;
        ecore_x_window_prop_card32_set(e_client_util_win_get(ec),
                                       E_ATOM_E_WAS_HERE,
                                       &val, 1);
     }
   ecore_x_netwm_window_state_get(e_client_util_win_get(ec), &state, &num);
   if (state)
     {
        for (i = 0; i < num; i++)
          {
             switch (state[i])
               {
                case ECORE_X_WINDOW_STATE_ICONIFIED:
                  /* Ignore */
                  break;

                case ECORE_X_WINDOW_STATE_MODAL:
                  ec->netwm.state.modal = 1;
                  break;

                case ECORE_X_WINDOW_STATE_STICKY:
                  ec->netwm.state.sticky = 1;
                  break;

                case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
                  ec->netwm.state.maximized_v = 1;
                  break;

                case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
                  ec->netwm.state.maximized_h = 1;
                  break;

                case ECORE_X_WINDOW_STATE_SHADED:
                  ec->netwm.state.shaded = 1;
                  break;

                case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
                  ec->netwm.state.skip_taskbar = 1;
                  break;

                case ECORE_X_WINDOW_STATE_SKIP_PAGER:
                  ec->netwm.state.skip_pager = 1;
                  break;

                case ECORE_X_WINDOW_STATE_HIDDEN:
                  if (!was_here)
                    ec->netwm.state.hidden = 1;
                  break;

                case ECORE_X_WINDOW_STATE_FULLSCREEN:
                  ec->netwm.state.fullscreen = 1;
                  break;

                case ECORE_X_WINDOW_STATE_ABOVE:
                  ec->netwm.state.stacking = E_STACKING_ABOVE;
                  break;

                case ECORE_X_WINDOW_STATE_BELOW:
                  ec->netwm.state.stacking = E_STACKING_BELOW;
                  break;

                case ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION:
                  /* FIXME */
                  break;

                case ECORE_X_WINDOW_STATE_UNKNOWN:
                  /* Ignore */
                  break;
               }
          }
        free(state);
     }
#endif
}

E_API void
e_hints_allowed_action_update(E_Client *ec, int action)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
   (void)action;
#else
   if (!e_client_has_xwindow(ec)) return;
   switch (action)
     {
      case ECORE_X_ACTION_MOVE:
        break;

      case ECORE_X_ACTION_RESIZE:
        break;

      case ECORE_X_ACTION_MINIMIZE:
        break;

      case ECORE_X_ACTION_SHADE:
        break;

      case ECORE_X_ACTION_STICK:
        break;

      case ECORE_X_ACTION_MAXIMIZE_HORZ:
        break;

      case ECORE_X_ACTION_MAXIMIZE_VERT:
        break;

      case ECORE_X_ACTION_FULLSCREEN:
        break;

      case ECORE_X_ACTION_CHANGE_DESKTOP:
        break;

      case ECORE_X_ACTION_CLOSE:
        break;

      case ECORE_X_ACTION_ABOVE:
        break;

      case ECORE_X_ACTION_BELOW:
        break;
     }
#endif
}

E_API void
e_hints_allowed_action_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   Ecore_X_Action *action;
   unsigned int i;
   unsigned int num;

   if (!e_client_has_xwindow(ec)) return;
   ec->netwm.action.move = 0;
   ec->netwm.action.resize = 0;
   ec->netwm.action.minimize = 0;
   ec->netwm.action.shade = 0;
   ec->netwm.action.stick = 0;
   ec->netwm.action.maximized_h = 0;
   ec->netwm.action.maximized_v = 0;
   ec->netwm.action.fullscreen = 0;
   ec->netwm.action.change_desktop = 0;
   ec->netwm.action.close = 0;

   ecore_x_netwm_allowed_action_get(e_client_util_win_get(ec), &action, &num);
   if (action)
     {
        for (i = 0; i < num; i++)
          {
             switch (action[i])
               {
                case ECORE_X_ACTION_MOVE:
                  ec->netwm.action.move = 1;
                  break;

                case ECORE_X_ACTION_RESIZE:
                  ec->netwm.action.resize = 1;
                  break;

                case ECORE_X_ACTION_MINIMIZE:
                  ec->netwm.action.minimize = 1;
                  break;

                case ECORE_X_ACTION_SHADE:
                  ec->netwm.action.shade = 1;
                  break;

                case ECORE_X_ACTION_STICK:
                  ec->netwm.action.stick = 1;
                  break;

                case ECORE_X_ACTION_MAXIMIZE_HORZ:
                  ec->netwm.action.maximized_h = 1;
                  break;

                case ECORE_X_ACTION_MAXIMIZE_VERT:
                  ec->netwm.action.maximized_v = 1;
                  break;

                case ECORE_X_ACTION_FULLSCREEN:
                  ec->netwm.action.fullscreen = 1;
                  break;

                case ECORE_X_ACTION_CHANGE_DESKTOP:
                  ec->netwm.action.change_desktop = 1;
                  break;

                case ECORE_X_ACTION_CLOSE:
                  ec->netwm.action.close = 1;
                  break;

                case ECORE_X_ACTION_ABOVE:
                  break;

                case ECORE_X_ACTION_BELOW:
                  break;
               }
          }
        free(action);
     }
#endif
}

#ifndef HAVE_WAYLAND_ONLY
static void
_e_hints_process_wakeup(E_Client *ec)
{
   // check for e vkbd state property - if its there, it's efl, so sending a
   // a fake sigchild to wake things up os just fine
   if (!ec->vkbd.have_property) return;
   if (ec->netwm.pid <= 0) return;
# ifdef SIGCHLD
   kill(ec->netwm.pid, SIGCHLD);
# endif
}
#endif

E_API void
e_hints_window_visible_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   if (!e_client_has_xwindow(ec)) return;
   if ((ec->icccm.state != ECORE_X_WINDOW_STATE_HINT_WITHDRAWN) &&
       (!ec->frozen))
     {
        ec->icccm.state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
        ecore_x_icccm_state_set(e_client_util_win_get(ec), ECORE_X_WINDOW_STATE_HINT_NORMAL);
     }
   else if (ec->frozen)
     {
        if (ec->icccm.state != ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
          ec->icccm.state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
        ecore_x_icccm_state_set(e_client_util_win_get(ec), ECORE_X_WINDOW_STATE_HINT_ICONIC);
     }
   if (ec->netwm.state.hidden)
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.hidden = 0;
        EC_CHANGED(ec);
     }
   _e_hints_process_wakeup(ec);
#endif
}

E_API void
e_hints_window_iconic_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   if (!e_client_has_xwindow(ec)) return;
   if (ec->icccm.state != ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     {
        ec->icccm.state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
        ecore_x_icccm_state_set(e_client_util_win_get(ec), ECORE_X_WINDOW_STATE_HINT_ICONIC);
     }
   else if (ec->frozen)
     {
        if (ec->icccm.state != ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
          ec->icccm.state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
        ecore_x_icccm_state_set(e_client_util_win_get(ec), ECORE_X_WINDOW_STATE_HINT_ICONIC);
     }
   if (!ec->netwm.state.hidden)
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.hidden = 1;
        EC_CHANGED(ec);
     }
   _e_hints_process_wakeup(ec);
#endif
}

E_API void
e_hints_window_hidden_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   if (!e_client_has_xwindow(ec)) return;
   if (ec->netwm.state.hidden)
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.hidden = 0;
        EC_CHANGED(ec);
     }
   ecore_x_window_prop_property_del(e_client_util_win_get(ec), ECORE_X_ATOM_WM_STATE);
   _e_hints_process_wakeup(ec);
#endif
}

E_API void
e_hints_window_shaded_set(E_Client *ec, int on)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
   (void)on;
#else
   if (!e_client_has_xwindow(ec)) return;
   if ((!ec->netwm.state.shaded) && (on))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.shaded = 1;
        if (ec->hacks.iconic_shading)
          e_hints_window_iconic_set(ec);
        EC_CHANGED(ec);
     }
   else if ((ec->netwm.state.shaded) && (!on))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.shaded = 0;
        if (ec->hacks.iconic_shading)
          e_hints_window_visible_set(ec);
        EC_CHANGED(ec);
     }
   _e_hints_process_wakeup(ec);
#endif
}

E_API void
e_hints_window_shade_direction_set(E_Client *ec, E_Direction dir)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
   (void)dir;
#else
   if (!e_client_has_xwindow(ec)) return;
   ecore_x_window_prop_card32_set(e_client_util_win_get(ec), E_ATOM_SHADE_DIRECTION, &dir, 1);
#endif
}

E_API E_Direction
e_hints_window_shade_direction_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   int ret;
   E_Direction dir;

   if (!e_client_has_xwindow(ec)) return E_DIRECTION_UP;
   ret = ecore_x_window_prop_card32_get(e_client_util_win_get(ec),
                                        E_ATOM_SHADE_DIRECTION,
                                        &dir, 1);
   if (ret == 1)
     return dir;
#endif
   return E_DIRECTION_UP;
}

E_API void
e_hints_window_size_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   unsigned int sizes[4];

   if (!e_client_has_xwindow(ec)) return;
   sizes[0] = ec->x;
   sizes[1] = ec->y;
   sizes[2] = ec->w;
   sizes[3] = ec->h;
   ecore_x_window_prop_card32_set(e_client_util_win_get(ec), E_ATOM_BORDER_SIZE, sizes, 4);
#endif
}

E_API void
e_hints_window_size_unset(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   if (!e_client_has_xwindow(ec)) return;
   ecore_x_window_prop_property_del(e_client_util_win_get(ec), E_ATOM_BORDER_SIZE);
#endif
}

E_API int
e_hints_window_size_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   int ret;
   unsigned int sizes[4];

   if (!e_client_has_xwindow(ec)) return 0;
   memset(sizes, 0, sizeof(sizes));
   ret = ecore_x_window_prop_card32_get(e_client_util_win_get(ec), E_ATOM_BORDER_SIZE,
                                        sizes, 4);
   if (ret != 4)
     return 0;

   ec->x = sizes[0];
   ec->y = sizes[1];
   ec->w = sizes[2];
   ec->h = sizes[3];
#endif

   return 1;
}

E_API void
e_hints_window_maximized_set(E_Client *ec, int horizontal, int vertical)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
   (void)horizontal;
   (void)vertical;
#else
   if (!e_client_has_xwindow(ec)) return;
   if ((horizontal) && (!ec->netwm.state.maximized_h))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.maximized_h = 1;
        EC_CHANGED(ec);
     }
   else if ((!horizontal) && (ec->netwm.state.maximized_h))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.maximized_h = 0;
        EC_CHANGED(ec);
     }
   if ((vertical) && (!ec->netwm.state.maximized_v))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.maximized_v = 1;
        EC_CHANGED(ec);
     }
   else if ((!vertical) && (ec->netwm.state.maximized_v))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.maximized_v = 0;
        EC_CHANGED(ec);
     }
#endif
}

E_API void
e_hints_window_fullscreen_set(E_Client *ec,
                              int on)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
   (void)on;
#else
   if (!e_client_has_xwindow(ec)) return;
   if ((!ec->netwm.state.fullscreen) && (on))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.fullscreen = 1;
        EC_CHANGED(ec);
     }
   else if ((ec->netwm.state.fullscreen) && (!on))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.fullscreen = 0;
        EC_CHANGED(ec);
     }
#endif
}

E_API void
e_hints_window_sticky_set(E_Client *ec, int on)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
   (void)on;
#else
   if (!e_client_has_xwindow(ec)) return;
   if ((!ec->netwm.state.sticky) && (on))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.sticky = 1;
        EC_CHANGED(ec);
     }
   else if ((ec->netwm.state.sticky) && (!on))
     {
        ec->netwm.update.state = 1;
        ec->netwm.state.sticky = 0;
        EC_CHANGED(ec);
     }
#endif
}

E_API void
e_hints_window_stacking_set(E_Client *ec, E_Stacking stacking)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
   (void)stacking;
#else
   if (!e_client_has_xwindow(ec)) return;
   if (ec->netwm.state.stacking == stacking) return;
   ec->netwm.update.state = 1;
   ec->netwm.state.stacking = stacking;
   EC_CHANGED(ec);
#endif
}

E_API void
e_hints_window_desktop_set(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   /* This function is only called when really changing desktop,
    * so just set the property and don't care about the roundtrip.
    */
   unsigned int deskpos[2];

   /* if valgrind complains here it is complaining ec->netwm.desktop
    * is an uninitialised variable - but it isn't. it can't be. its part of
    * a calloc()'d struct and thus has to have been set to 0. hell even
    * e_client.c explicitly sets it to 0 on creation of the border object.
    */
   if (!e_client_has_xwindow(ec)) return;
   deskpos[0] = ec->desk->x;
   deskpos[1] = ec->desk->y;
   ecore_x_window_prop_card32_set(e_client_util_win_get(ec), E_ATOM_DESK, deskpos, 2);

#if 0
   ecore_x_netwm_desktop_set(e_client_util_win_get(ec), current);
#endif
   ec->netwm.desktop = (ec->desk->y * ec->zone->desk_x_count) + ec->desk->x;
#endif
}

E_API void
e_hints_window_e_state_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   /* Remember to update the count if we add more states! */
   Ecore_X_Atom state[1];
   int num = 0, i = 0;
   int size = 0;

   if (!e_client_has_xwindow(ec)) return;
   memset(state, 0, sizeof(state));

   /* ugly, but avoids possible future overflow if more states are added */
   size = (sizeof(state) / sizeof(state[0]));

   num =
     ecore_x_window_prop_card32_get(e_client_util_win_get(ec), E_ATOM_WINDOW_STATE,
                                    state, size);
   if (!num) return;

   for (i = 0; (i < num) && (i < size); i++)
     {
        if (state[i] == E_ATOM_WINDOW_STATE_CENTERED)
          ec->e.state.centered = 1;
     }
#endif
}

E_API void
e_hints_window_e_state_set(E_Client *ec EINA_UNUSED)
{
   /* TODO */
}

E_API void
e_hints_window_qtopia_soft_menu_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   unsigned int val;

   if (!e_client_has_xwindow(ec)) return;
   if (ecore_x_window_prop_card32_get(e_client_util_win_get(ec), ATM__QTOPIA_SOFT_MENU, &val, 1))
     ec->qtopia.soft_menu = val;
   else
     ec->qtopia.soft_menu = 0;
#endif
}

E_API void
e_hints_window_qtopia_soft_menus_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   unsigned int val;

   if (!e_client_has_xwindow(ec)) return;
   if (ecore_x_window_prop_card32_get(e_client_util_win_get(ec), ATM__QTOPIA_SOFT_MENUS, &val, 1))
     ec->qtopia.soft_menus = val;
   else
     ec->qtopia.soft_menus = 0;
#endif
}

E_API void
e_hints_window_steam_game_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   unsigned int val;

   if (!e_client_has_xwindow(ec)) return;
   if (ecore_x_window_prop_card32_get(e_client_util_win_get(ec), ATM_STEAM_GAME, &val, 1))
     ec->steam.steam_game_id = val;
   else
     ec->steam.steam_game_id = 0;
#endif
}

E_API void
e_hints_window_virtual_keyboard_state_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   Ecore_X_Atom atom = 0;

   if (!e_client_has_xwindow(ec)) return;
   ec->vkbd.state = ecore_x_e_virtual_keyboard_state_get(e_client_util_win_get(ec));
   if (ecore_x_window_prop_atom_get(e_client_util_win_get(ec),
                                    ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE,
                                    &atom, 1))
     ec->vkbd.have_property = 1;
   else
     ec->vkbd.have_property = 0;
#endif
}

E_API void
e_hints_window_virtual_keyboard_get(E_Client *ec)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)ec;
#else
   if (!e_client_has_xwindow(ec)) return;
   ec->vkbd.vkbd = ecore_x_e_virtual_keyboard_get(e_client_util_win_get(ec));
#endif
}

static void
e_hints_openoffice_gnome_fake(Ecore_Window root)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)root;
#else
   const char *string = "ATM_GNOME_SM_PROXY";

   ecore_x_window_prop_property_set(root, ATM_GNOME_SM_PROXY, ECORE_X_ATOM_STRING,
                                    8, (void *)string, strlen(string));
#endif
}

#if 0
static void
e_hints_openoffice_kde_fake(Ecore_Window root)
{
#ifdef HAVE_WAYLAND_ONLY
   (void)root;
#else
   Ecore_X_Window win2;

   win2 = ecore_x_window_new(root, -20, -20, 1, 1);
   ecore_x_netwm_wm_identify(root, win2, "KWin");
#endif
}
#endif

E_API void
e_hints_scale_update(void)
{
#ifdef HAVE_WAYLAND_ONLY
   Eina_List *l;
   E_Comp_Wl_Output *output;

   EINA_LIST_FOREACH(e_comp_wl->outputs, l, output)
     output->scale = e_scale;

#else
   unsigned int scale = e_scale * 1000;

   if (e_comp->root)
     ecore_x_window_prop_card32_set(e_comp->root, ATM_ENLIGHTENMENT_SCALE, &scale, 1);
#endif
}


#ifdef HAVE_WAYLAND
static Eina_List *aux_hints_supported = NULL;

static E_Comp_Client_Data *
e_pixmap_cdata_get(E_Pixmap *cp)
{
   return e_pixmap_client_get(cp)->comp_data;
}

E_API const Eina_List *
e_hints_aux_hint_supported_add(const char *hint)
{
   Eina_List *l;
   const char *supported;

   EINA_LIST_FOREACH(aux_hints_supported, l, supported)
     {
        if (!strcmp(supported, hint))
          return aux_hints_supported;
     }

   aux_hints_supported = eina_list_append(aux_hints_supported, hint);

   return aux_hints_supported;
}

E_API const Eina_List *
e_hints_aux_hint_supported_del(const char *hint)
{
   Eina_List *l;
   const char *supported;

   EINA_LIST_FOREACH(aux_hints_supported, l, supported)
     {
        if (!strcmp(supported, hint))
          {
             aux_hints_supported = eina_list_remove(aux_hints_supported, hint);
             break;
          }
     }

   return aux_hints_supported;
}

E_API const Eina_List *
e_hints_aux_hint_supported_get(void)
{
   return aux_hints_supported;
}

E_API Eina_Bool
e_hints_aux_hint_add(E_Client *ec, int32_t id, const char *name, const char *val)
{
   if (!ec) return EINA_FALSE;
   return e_hints_aux_hint_add_with_pixmap(ec->pixmap, id, name, val, -1);
}

E_API Eina_Bool
e_hints_aux_hint_add_fd(E_Client *ec, int32_t id, const char *name, int32_t fd)
{
   if (!ec) return EINA_FALSE;
   return e_hints_aux_hint_add_with_pixmap(ec->pixmap, id, name, NULL, fd);
}

E_API Eina_Bool
e_hints_aux_hint_change(E_Client *ec, int32_t id, const char *val)
{
   if (!ec) return EINA_FALSE;
   return e_hints_aux_hint_change_with_pixmap(ec->pixmap, id, val, -1);
}

E_API Eina_Bool
e_hints_aux_hint_change_fd(E_Client *ec, int32_t id, int32_t fd)
{
   if (!ec) return EINA_FALSE;
   return e_hints_aux_hint_change_with_pixmap(ec->pixmap, id, NULL, fd);
}

E_API Eina_Bool
e_hints_aux_hint_del(E_Client *ec, int32_t id)
{
   if (!ec) return EINA_FALSE;
   return e_hints_aux_hint_del_with_pixmap(ec->pixmap, id);
}

E_API const char *
e_hints_aux_hint_value_get(E_Client *ec, const char *name)
{
   if (!ec) return NULL;
   return e_hints_aux_hint_value_get_with_pixmap(ec->pixmap, name);
}

E_API Eina_Bool
e_hints_aux_hint_add_with_pixmap(E_Pixmap *cp, int32_t id, const char *name, const char *val, int32_t fd)
{
   E_Comp_Wl_Client_Data *cdata;
   Eina_Bool found = EINA_FALSE;
   E_Comp_Wl_Aux_Hint *hint;
   Eina_List *l;

   if ((!cp) || (!val) || (!name)) return EINA_FALSE;
   cdata = (E_Comp_Wl_Client_Data*)e_pixmap_cdata_get(cp);
   if (!cdata) return EINA_FALSE;

   EINA_LIST_FOREACH(cdata->aux_hint.hints, l, hint)
     {
        if (hint->id == id)
          {
             if (strcmp(hint->val, val) != 0)
               {
                  eina_stringshare_del(hint->val);
                  hint->val = eina_stringshare_add(val);
                  hint->fd = fd;
                  hint->changed = EINA_TRUE;
                  if (hint->deleted)
                    hint->deleted = EINA_FALSE;
                  cdata->aux_hint.changed = 1;
               }
             found = EINA_TRUE;
             break;
          }
     }

   if (!found)
     {
        hint = E_NEW(E_Comp_Wl_Aux_Hint, 1);
        EINA_SAFETY_ON_NULL_RETURN_VAL(hint, EINA_FALSE);

        memset(hint, 0, sizeof(E_Comp_Wl_Aux_Hint));

        hint->id = id;
        hint->hint = eina_stringshare_add(name);
        hint->val = eina_stringshare_add(val);
        hint->fd = fd;
        hint->changed = EINA_TRUE;
        hint->deleted = EINA_FALSE;
        cdata->aux_hint.hints = eina_list_append(cdata->aux_hint.hints, hint);
        cdata->aux_hint.changed = 1;
     }

   if (!found)
     return EINA_TRUE;

   return EINA_FALSE;
}

E_API Eina_Bool
e_hints_aux_hint_change_with_pixmap(E_Pixmap *cp, int32_t id, const char *val, int32_t fd)
{
   E_Comp_Wl_Client_Data *cdata;
   Eina_List *l;
   E_Comp_Wl_Aux_Hint *hint;
   Eina_Bool found = EINA_FALSE;

   if ((!cp) || (!val)) return EINA_FALSE;
   cdata = (E_Comp_Wl_Client_Data*)e_pixmap_cdata_get(cp);
   if (!cdata) return EINA_FALSE;

   EINA_LIST_FOREACH(cdata->aux_hint.hints, l, hint)
     {
        if (hint->id == id)
          {
             if ((hint->val) && (strcmp(hint->val, val) != 0))
               {
                  eina_stringshare_del(hint->val);
                  hint->val = eina_stringshare_add(val);
                  hint->fd = fd;
                  hint->changed = EINA_TRUE;
                  cdata->aux_hint.changed = 1;
               }

             if (hint->deleted)
               hint->deleted = EINA_FALSE;

             found = EINA_TRUE;
             break;
          }
     }

   if (found)
     return EINA_TRUE;

   return EINA_FALSE;
}

E_API Eina_Bool
e_hints_aux_hint_del_with_pixmap(E_Pixmap *cp, int32_t id)
{
   E_Comp_Wl_Client_Data *cdata;
   Eina_List *l, *ll;
   E_Comp_Wl_Aux_Hint *hint;
   int res = -1;

   if (!cp) return EINA_FALSE;
   cdata = (E_Comp_Wl_Client_Data*)e_pixmap_cdata_get(cp);
   if (!cdata) return EINA_FALSE;

   EINA_LIST_FOREACH_SAFE(cdata->aux_hint.hints, l, ll, hint)
     {
        if (hint->id == id)
          {
             hint->changed = EINA_TRUE;
             hint->deleted = EINA_TRUE;
             cdata->aux_hint.changed = 1;
             res = hint->id;
             break;
          }
     }

   if (res == -1)
     return EINA_FALSE;

   return EINA_TRUE;
}

E_API const char *
e_hints_aux_hint_value_get_with_pixmap(E_Pixmap *cp, const char *name)
{
   E_Comp_Wl_Client_Data *cdata;
   Eina_List *l;
   E_Comp_Wl_Aux_Hint *hint;
   const char *res = NULL;

   if (!cp) return NULL;
   cdata = (E_Comp_Wl_Client_Data*)e_pixmap_cdata_get(cp);
   if (!cdata) return NULL;

   EINA_LIST_REVERSE_FOREACH(cdata->aux_hint.hints, l, hint)
     {
        if ((!hint->deleted) &&
            (!strcmp(hint->hint, name)))
          {
             res =  hint->val;
             break;
          }
     }

   return res;
}
#endif
