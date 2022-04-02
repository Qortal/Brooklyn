dir = 'tiling'
src = [
  'e',
  'e_bindings',
  'module.battery',
  'module.cpufreq',
  'module.fileman',
  'module.ibar',
  'module.ibox',
  'module.pager',
  'module.temperature',
  'module.tiling',
  'module.conf'
]

##### boilerplate config build + install of icons/dirs
i = 0
foreach cd: src
  custom_target(' '.join(['config_dist', dir, cd]),
		input        : cd + '.src',
		output       : cd + '.cfg',
		command      : [eet_cmd, '-e', '@OUTPUT@', 'config', '@INPUT@', '1'],
		install      : true,
		install_dir  : join_paths(dir_config, dir),
                install_mode : 'rw-r--r--'
               )
  i += 1
endforeach
install_data([ 'enlightenment-' + dir + '.png',
               'profile.desktop'
             ],
             install_dir  : join_paths(dir_config, dir),
             install_mode : 'rw-r--r--'
            )
