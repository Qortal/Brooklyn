executable('enlightenment_paledit',
           [ 'main.c',
             'main.h',
             'colsel.c',
             'palcols.c',
             'palimg.c',
             'palsel.c'
           ],
           include_directories: include_directories('.'),
           dependencies       : [ dep_elementary, deps ],
           install_dir        : dir_bin,
           install            : true
          )

install_data('enlightenment_paledit.desktop',
             install_dir: join_paths(dir_data, 'applications')
            )
install_data('enlightenment_paledit.png',
             install_dir: join_paths(dir_data, 'icons/hicolor/128x128/apps')
            )
