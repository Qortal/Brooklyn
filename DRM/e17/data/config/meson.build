dir_config = join_paths(dir_data, 'enlightenment/data/config')

eet_cmd = get_option('eet')
if eet_cmd == ''
  eet_cmd = join_paths(dep_eet.get_pkgconfig_variable('prefix'),
                        'bin', 'eet')
endif

custom_target('profile.src',
              input        : 'profile.src',
              command      : [eet_cmd, '-i', '@OUTPUT@', 'config', '@INPUT@', '1'],
              output       : ['profile.cfg'],
              install      : true,
              install_dir  : dir_config,
              install_mode : 'rw-r--r--'
             )

subdir('default')
subdir('standard')
subdir('tiling')
