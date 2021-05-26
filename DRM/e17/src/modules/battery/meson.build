src = files(
  'e_mod_main.c',
  'e_mod_config.c',
  'e_mod_sysctl.c',
  'e_mod_udev.c',
  'e_mod_upower.c',
  'e_mod_main.h'
)

if get_option(m) == true
  executable('batget',
             'batget.c',
             include_directories: include_directories(module_includes),
             dependencies       : [ dep_eina, dep_ecore, dep_ecore_con, dep_ecore_file ],
             install_dir        : _dir_bin,
             install            : true
            )
endif
