src = files(
  'e_mod_main.c',
  'e_mod_main.h',
  'e_mod_preview.c',
  'e_mod_edit.c',
  'e_mod_save.c',
  'e_mod_share.c',
  'e_mod_delay.c'
)

if get_option(m) == true
  executable('upload',
             'upload.c',
             include_directories: include_directories(module_includes),
             dependencies       : [ dep_elementary ],
             install_dir        : _dir_bin,
             install            : true
            )
  subdir('data')
endif
