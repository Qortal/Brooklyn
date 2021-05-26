src = [
  'e_system_main.c',
  'e_system_inout.c',
  'e_system_backlight.c',
  'e_system_ddc.c',
  'e_system_storage.c',
  'e_system_power.c',
  'e_system_rfkill.c',
  'e_system_l2ping.c',
  'e_system_cpufreq.c',
  'e_system.h',
]
executable('enlightenment_system', src,
           include_directories: include_directories('..', '../../..'),
           dependencies       : [ dep_eina, dep_ecore, dep_ecore_file,
                                  dep_m, dep_dl,
                                  dep_eet, dep_eeze,
                                  dep_bluez ],
           c_args             : suid_cflags,
           link_args          : suid_ldflags,
           install_dir        : dir_e_utils,
           install            : true
           )
suid_exes += join_paths(dir_e_utils, 'enlightenment_system')
