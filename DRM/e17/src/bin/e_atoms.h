#ifdef E_TYPEDEFS
#else
#ifndef E_ATOMS_H
#define E_ATOMS_H

/* an "overall" atom to see that we recognise the window */
extern E_API Ecore_X_Atom E_ATOM_MANAGED;

/* basic window properties */
extern E_API Ecore_X_Atom E_ATOM_ZONE;
extern E_API Ecore_X_Atom E_ATOM_DESK;
extern E_API Ecore_X_Atom E_ATOM_MAPPED;
extern E_API Ecore_X_Atom E_ATOM_SHADE_DIRECTION;
extern E_API Ecore_X_Atom E_ATOM_HIDDEN;
extern E_API Ecore_X_Atom E_ATOM_BORDER_SIZE;
extern E_API Ecore_X_Atom E_ATOM_DESKTOP_FILE;
/* extra e window states */
/* if we add more states, we need to fix
 * * e_hints_window_e_state_get()
 * * e_hints_window_e_state_set()
 * * _e_win_state_update() + e_win
 */
extern E_API Ecore_X_Atom E_ATOM_WINDOW_STATE;
extern E_API Ecore_X_Atom E_ATOM_WINDOW_STATE_CENTERED;

extern E_API Ecore_X_Atom E_ATOM_ZONE_GEOMETRY;
extern E_API Ecore_X_Atom WL_SURFACE_ID;
extern E_API Ecore_X_Atom E_ATOM_E_WAS_HERE;

EINTERN int    e_atoms_init(void);
EINTERN int    e_atoms_shutdown(void);

#endif
#endif
