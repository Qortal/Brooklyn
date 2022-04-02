deps_custom = declare_dependency(link_args: [] )
if host_os == 'openbsd' or host_os == 'freebsd' or host_os == 'dragonfly'
  deps_custom = declare_dependency(link_args : [ '-lkvm' ])
endif

module_deps += deps_custom

src = files(
  'e_mod_main.c',
  'process.c',
  'process.h',
 )
