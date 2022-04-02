src = files(
  'e_mod_main.c',
  'evry.c',
  'evry_config.c',
  'evry_gadget.c',
  'evry_history.c',
  'evry_plug_actions.c',
  'evry_plug_aggregator.c',
  'evry_plug_apps.c',
  'evry_plug_calc.c',
  'evry_plug_clipboard.c',
  'evry_plug_collection.c',
  'evry_plug_files.c',
  'evry_plugin.c',
  'evry_plug_settings.c',
  'evry_plug_text.c',
  'evry_plug_windows.c',
  'evry_util.c',
  'evry_view.c',
  'evry_view_help.c',
  'evry_view_tabs.c',
  'md5.c',
  'e_mod_main.h',
  'evry_api.h',
  'evry_types.h',
  'md5.h'
 )

data = [ 'e-module-everything-start.edj' ]

if get_option(m) == true
  install_headers([ 'evry_api.h', 'evry_types.h' ],
                  subdir: 'enlightenment'
                 )
  pkgconfig.generate(name: 'e17-everything',
                     description: 'Everything module for Enlightenment',
                     filebase   : 'everything',
                     subdirs    : 'enlightenment',
                     requires   : 'enlightenment',
                     version    : e_version_rev,
                     install_dir: dir_pkgconfig,
                     variables  : [
                       'exec_prefix=${prefix}',
                       'plugindir=${libdir}/enlightenment/modules/everything/plugins'
                     ]
                    )
endif

