if get_option('wl') != true
  disable = true
else
  src = files(
    'e_mod_main.c'
  )

  foreach p: [
    '@0@/unstable/input-method/input-method-unstable-v1.xml'.format(dir_wayland_protocols),
    '@0@/unstable/text-input/text-input-unstable-v1.xml'.format(dir_wayland_protocols)
  ]
    src += gen_scanner_server.process(p)
    src += gen_scanner_impl.process(p)
  endforeach

  no_icon = true
endif
