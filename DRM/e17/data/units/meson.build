unit_config = configuration_data()
unit_config.set('prefix', dir_prefix)
if get_option('systemd') == true
  configure_file(input        : 'enlightenment.service.in',
                 output       : 'enlightenment.service',
                 install_dir  : systemd_unitdir,
                 configuration: unit_config
                )
endif
