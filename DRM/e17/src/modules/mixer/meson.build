src = files(
  'e_mod_main.c',
  'e_mod_config.c',
  'e_mod_config.h',
  'e_mod_main.h',
  'backend.c',
  'backend.h'
 )

mixer_lib = files(
  'lib/emix.c',
  'lib/emix.h'
 )

if get_option(m) == true
  dep_alsa         = dependency('alsa', version: '>= 1.0.8', required: false)
  dep_pulse        = dependency('libpulse', required: false)
  dep_pulse_simple = dependency('libpulse-simple', required: false)

  deps += [ dep_alsa, dep_pulse, dep_pulse_simple ]
  inc = [ 'lib' ]

  config_h.set('HAVE_ALSA' , dep_alsa.found())
  config_h.set('HAVE_PULSE', dep_pulse.found() and dep_pulse_simple.found())

  if config_h.get('HAVE_ALSA') == true
    mixer_lib += files(
      'lib/backends/alsa/alsa.c'
    )
  endif
  if config_h.get('HAVE_PULSE') == true
    mixer_lib += files(
      'lib/backends/pulseaudio/pulse_ml.c',
      'lib/backends/pulseaudio/pulse.c'
    )
  endif
  if config_h.has('HAVE_PULSE') == false and config_h.has('HAVE_ALSA') == false
    error('mixer support requested, but no backends found!')
  endif

  src += mixer_lib

  install_data('emixer.desktop',
               install_dir: join_paths(dir_data, 'applications')
              )
  install_data('emixer.png',
               install_dir: join_paths(dir_data, 'icons/hicolor/128x128/apps')
              )

  executable('emixer',
             [ 'emixer.c', mixer_lib ],
             include_directories: include_directories(module_includes, '.', './lib'),
             dependencies       : [ dep_elementary, deps ],
             c_args             : '-DEMIXER_BUILD',
             install_dir        : dir_bin,
             install            : true
            )
  install_data(['sink-icons.txt'],
                install_dir: _dir)

endif
