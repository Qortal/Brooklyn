src = files(
  'e_mod_main.c',
  'dnd.c'
 )

no_icon = true

if get_option('wl') != true
  disable = true
else
  xwayland = find_program('Xwayland')
  cargs = '-DXWAYLAND_BIN="@0@"'.format(xwayland.path())
endif
