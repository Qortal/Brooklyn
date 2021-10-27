src = files(
  'e_mod_main.c',
  'e_wizard.c',
  'e_wizard.h',
  'e_wizard_api.h',
  'e_wizard_priv.h'
 )

if get_option(m) == true
  pages = [
    'page_000',
    'page_010',
    'page_011',
    'page_020',
    'page_030',
    'page_040',
    'page_050',
    'page_060',
    'page_070',
    'page_080',
    'page_090',
    'page_100',
    'page_110',
    'page_115',
    'page_120',
    'page_130',
    'page_150',
    'page_160',
    'page_170',
    'page_180',
    'page_200'
  ]
  foreach page: pages
    shared_module(page, '@0@.c'.format(page),
                  include_directories: include_directories(module_includes),
                  name_prefix        : '',
                  dependencies       : module_deps,
                  install_dir        : _dir_bin,
                  install            : true,
                  link_args          : '-Wl,--unresolved-symbols=ignore-all'
                 )
  endforeach

  data = [
    'data/def-ibar.txt'
  ]
  install_data(['data/desktop/home.desktop',
                'data/desktop/root.desktop',
                'data/desktop/tmp.desktop'],
                install_dir: join_paths(_dir, 'desktop'))

  no_icon = true
endif

