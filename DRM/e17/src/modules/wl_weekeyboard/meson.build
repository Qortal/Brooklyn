if get_option('wl') != true
  disable = true
else
  src = files(
    'e_mod_main.c',
    'wkb-log.c',
    'wkb-log.h'
  )

  foreach p: [
    '@0@/unstable/input-method/input-method-unstable-v1.xml'.format(dir_wayland_protocols),
    '@0@/unstable/text-input/text-input-unstable-v1.xml'.format(dir_wayland_protocols),
  ]
    src += gen_scanner_server.process(p)
    src += gen_scanner_client.process(p)
    src += gen_scanner_impl.process(p)
  endforeach

  data = [
    'themes/default/default_600.edj',
    'themes/default/default_720.edj',
    'themes/default/default_1080.edj'
  ]

  no_icon = true
endif
