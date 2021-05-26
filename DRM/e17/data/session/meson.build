desktop_config = configuration_data()
desktop_config.set('prefix', dir_prefix)
desktop_config.set('VERSION', e_version_rev)
e_desktop = configure_file(input        : 'enlightenment.desktop.in',
               output       : 'enlightenment.desktop',
               configuration: desktop_config
              )

if config_h.has('HAVE_WAYLAND') == true
	install_data(e_desktop,
		install_dir  : join_paths(dir_data, 'wayland-sessions'))
endif

if config_h.has('HAVE_WAYLAND_ONLY') == false
	install_data(e_desktop,
		install_dir  : join_paths(dir_data, 'xsessions'))
endif
