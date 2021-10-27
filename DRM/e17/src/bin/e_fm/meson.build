deps = [
  dep_eina,
  dep_ecore,
  dep_eet,
  dep_ecore_con,
  dep_ecore_file,
  dep_ecore_ipc,
  dep_efreet
]

udisks_src = []
if config_h.has('HAVE_UDISKS_MOUNT') == true
  udisks_src = [
    'e_fm_main_udisks.c',
    'e_fm_main_udisks.h',
    'e_fm_main_udisks2.c',
    'e_fm_main_udisks2.h'
  ]
  deps += [ dep_eldbus ]
endif

eeze_src = []
if config_h.has('HAVE_EEZE_MOUNT') == true
  eeze_src = [
    '../e_prefix.c',
    'e_fm_main_eeze.c',
    'e_fm_main_eeze.h'
  ]
  deps += [ dep_eeze, dep_eet ]
endif

src = [
  'e_fm_main.c',
  'e_fm_main.h',
  'e_fm_ipc.c',
  'e_fm_ipc.h',
  '../e_fm_shared_codec.c',
  '../e_fm_shared_device.c',
  '../e_user.c',
  '../e_sha1.c',
  udisks_src,
  eeze_src
]

executable('enlightenment_fm', src,
           include_directories: include_directories('../../../', '../..', '.', '..'),
           dependencies       : deps,
           install_dir        : dir_e_utils,
           install            : true
)
