module_files     = []
module_ldflags   = '-module -avoid-version'
module_includes  = [ '../../..', '../../bin', '../../bin/efx' ]
module_includes2 = [ '../..'   , '../bin'   , '../bin/efx'  ]
module_deps      = [ deps_e, dep_dl ]

mods = [
# standard run of the mill modules with cion and desktop
  'ibar',
  'pager',
  'temperature',
  'mixer',
  'notification',
  'everything',
  'ibox',
  'start',
  'winlist',
  'fileman',
  'fileman_opinfo',
  'conf',
  'msgbus',
  'music-control',
  'conf_applications',
  'conf_shelves',
  'conf_window_remembers',
  'conf_window_manipulation',
  'conf_menus',
  'conf_dialogs',
  'conf_performance',
  'conf_paths',
  'conf_interaction',
  'gadman',
  'geolocation',
  'connman',
  'bluez5',
  'syscon',
  'systray',
  'appmenu',
  'quickaccess',
  'shot',
  'backlight',
  'tasks',
  'conf_randr',
  'xkbswitch',
  'tiling',
  'packagekit',
  'vkbd',
  'procstats',
# modules have a custom binary as well
  'battery',
  'cpufreq',
  'clock',
# custyom sub modules and custom data
  'wizard',
# also standard modules, just with only a desktop file using a generic icon
  'conf_theme',
  'conf_intl',
  'conf_display',
  'conf_bindings',
# also standard modules with no icon or desktop file
  'xwayland',
  'lokker',
  'polkit',
  'wl_x11',
  'wl_wl',
  'wl_buffer',
  'wl_drm',
  'wl_text_input',
  'wl_desktop_shell',
  'wl_weekeyboard',
### XXX: disabled for now
#  'wl_fb'
]

foreach m: mods
  disable      = false
  no_icon      = false
  cargs        = ''
  data         = []
  deps         = deps_e
  inc          = []
  _dir         = join_paths(dir_module_e, m)
  _dir_bin     = join_paths(_dir, module_arch)

  opt = '-'.join(m.split('_'))

  if get_option(opt) == true
    subdir(m)
  else
    disable = true
  endif

  if disable == false

    _inc2 = []
    foreach i: inc
      _inc2 +=  join_paths('.', m, i)
    endforeach
    _inc          = include_directories(module_includes2,
                                        join_paths('.', m),
                                        _inc2)
    _conf         = 'USE_MODULE_' + m.underscorify().to_upper()
    module_files += join_paths(_dir_bin, m + '.so')

    if no_icon == true
      _data = []
    else
      _data = [ join_paths(m, 'e-module-' + m + '.edj'),
                join_paths(m, 'module.desktop') ]
    endif

    foreach d: data
      _data += join_paths(m, d)
    endforeach

    if _data.length() > 0
      install_data(_data,
                   install_dir: _dir)
    endif

    config_h.set(_conf, '1')
    if cargs == ''
      shared_module(m, src,
                    include_directories: _inc,
                    name_prefix        : '',
                    dependencies       : [ module_deps, deps ],
                    install_dir        : _dir_bin,
                    install            : true,
                    link_args          : '-Wl,--unresolved-symbols=ignore-all'
                   )
    else
      shared_module(m, src,
                    include_directories: _inc,
                    c_args             : cargs,
                    name_prefix        : '',
                    dependencies       : [ module_deps, deps ],
                    install_dir        : _dir_bin,
                    install            : true,
                    link_args          : '-Wl,--unresolved-symbols=ignore-all'
                   )
    endif
  endif
endforeach
