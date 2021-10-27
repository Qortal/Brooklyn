src = files(
  'e_mod_main.c',
  'e_int_config_acpibindings.c',
  'e_int_config_edgebindings.c',
  'e_int_config_keybindings.c',
  'e_int_config_mousebindings.c',
  'e_int_config_signalbindings.c',
  'e_mod_main.h'
)
if get_option('gesture-recognition') == true
   src += files('e_int_config_swipebindings.c')
endif

