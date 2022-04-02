#include "e.h"

/* Atoms */
#ifndef HAVE_WAYLAND_ONLY
E_API Ecore_X_Atom E_ATOM_MANAGED = 0;
E_API Ecore_X_Atom E_ATOM_ZONE = 0;
E_API Ecore_X_Atom E_ATOM_DESK = 0;
E_API Ecore_X_Atom E_ATOM_MAPPED = 0;
E_API Ecore_X_Atom E_ATOM_SHADE_DIRECTION = 0;
E_API Ecore_X_Atom E_ATOM_HIDDEN = 0;
E_API Ecore_X_Atom E_ATOM_BORDER_SIZE = 0;
E_API Ecore_X_Atom E_ATOM_WINDOW_STATE = 0;
E_API Ecore_X_Atom E_ATOM_WINDOW_STATE_CENTERED = 0;
E_API Ecore_X_Atom E_ATOM_DESKTOP_FILE = 0;
E_API Ecore_X_Atom E_ATOM_ZONE_GEOMETRY = 0;
E_API Ecore_X_Atom WL_SURFACE_ID = 0;
E_API Ecore_X_Atom E_ATOM_E_WAS_HERE = 0;
#endif

/* externally accessible functions */
EINTERN int
e_atoms_init(void)
{
#ifndef HAVE_WAYLAND_ONLY
   const char *atom_names[] = {
      "__E_WINDOW_MANAGED",
      "__E_WINDOW_ZONE",
      "__E_WINDOW_DESK",
      "__E_WINDOW_MAPPED",
      "__E_WINDOW_SHADE_DIRECTION",
      "__E_WINDOW_HIDDEN",
      "__E_WINDOW_BORDER_SIZE",
      "__E_ATOM_WINDOW_STATE",
      "__E_ATOM_WINDOW_STATE_CENTERED",
      "__E_ATOM_DESKTOP_FILE",
      "E_ZONE_GEOMETRY",
      "WL_SURFACE_ID",
      "__E_ATOM_E_WAS_HERE",
   };
   Ecore_X_Atom atoms[EINA_C_ARRAY_LENGTH(atom_names)];

   ecore_x_atoms_get(atom_names, EINA_C_ARRAY_LENGTH(atom_names), atoms);
   E_ATOM_MANAGED = atoms[0];
   E_ATOM_ZONE = atoms[1];
   E_ATOM_DESK = atoms[2];
   E_ATOM_MAPPED = atoms[3];
   E_ATOM_SHADE_DIRECTION = atoms[4];
   E_ATOM_HIDDEN = atoms[5];
   E_ATOM_BORDER_SIZE = atoms[6];
   E_ATOM_WINDOW_STATE = atoms[7];
   E_ATOM_WINDOW_STATE_CENTERED = atoms[8];
   E_ATOM_DESKTOP_FILE = atoms[9];
   E_ATOM_ZONE_GEOMETRY = atoms[10];
   WL_SURFACE_ID = atoms[11];
   E_ATOM_E_WAS_HERE = atoms[12];
#endif
   return 1;
}

EINTERN int
e_atoms_shutdown(void)
{
   /* Nothing really to do here yet, just present for consistency right now */
   return 1;
}

