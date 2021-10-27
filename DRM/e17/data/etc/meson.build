MOUNT     = '/bin/mount'
UMOUNT    = '/bin/umount'
EJECT     = '/usr/bin/eject'

if host_machine.system().contains('bsd') == true
  MOUNT     = '/sbin/mount'
  UMOUNT    = '/sbin/umount'
  EJECT     = '/usr/sbin/cdcontrol eject'
endif

sysactions = configuration_data()
sysactions.set('MOUNT'    , MOUNT)
sysactions.set('UMOUNT'   , UMOUNT)
sysactions.set('EJECT'    , EJECT)

if get_option('install-sysactions')
  configure_file(input        : 'sysactions.conf.in',
                 output       : 'sysactions.conf',
                 install_dir  : join_paths(dir_sysconf, 'enlightenment'),
                 configuration: sysactions
                )
endif

if get_option('install-enlightenment-menu')
  install_data('e-applications.menu',
               install_dir: join_paths(dir_sysconf, 'xdg/menus')
              )
endif

if get_option('install-system')
  install_data('system.conf',
               install_dir: join_paths(dir_sysconf, 'enlightenment')
              )
endif
