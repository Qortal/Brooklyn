src = files(
  'e_mod_main.c',
  'e_kbd_buf.c',
  'e_kbd_cfg.c',
  'e_kbd_dict.c',
  'e_kbd_int.c',
  'e_kbd_send.c',
  'e_mod_main.h',
  'e_kbd_buf.h',
  'e_kbd_cfg.h',
  'e_kbd_dict.h',
  'e_kbd_int.h',
  'e_kbd_send.h'
 )

if get_option(m) == true
  if get_option('wl') == true
    foreach p: [
      '@0@/unstable/input-method/input-method-unstable-v1.xml'.format(dir_wayland_protocols),
      '@0@/unstable/text-input/text-input-unstable-v1.xml'.format(dir_wayland_protocols),
    ]
      src += gen_scanner_server.process(p)
      src += gen_scanner_client.process(p)
      src += gen_scanner_impl.process(p)
    endforeach
  endif

  cmd = [ edje_cmd,
          '-id', join_paths(meson.source_root(), 'src' , 'modules', m,
                            'images'),
          '@INPUT@', '@OUTPUT@'
        ]
  edc = 'theme'
  custom_target(edc + '.edj',
                input        : edc + '.edc',
                output       : edc + '.edj',
                command      : cmd,
                install_dir  : _dir,
                install_mode : 'rw-r--r--',
                install      : true
               )
  out = join_paths(_dir, edc + '.edj')

  install_data(['dicts/English_US_Small.dic',
                'dicts/English_US.dic'],
               install_dir  : join_paths(_dir, 'dicts'),
               install_mode : 'rw-r--r--'
               )
  install_data(['keyboards/Default.kbd',
                'keyboards/Numbers.kbd',
                'keyboards/Terminal.kbd',
                'keyboards/alpha.png',
                'keyboards/backspace.png',
                'keyboards/down.png',
                'keyboards/enter.png',
                'keyboards/left.png',
                'keyboards/numeric.png',
                'keyboards/qwerty.png',
                'keyboards/right.png',
                'keyboards/shift.png',
                'keyboards/spanner.png',
                'keyboards/tab.png',
                'keyboards/up.png'],
               install_dir  : join_paths(_dir, 'keyboards'),
               install_mode : 'rw-r--r--')
endif
