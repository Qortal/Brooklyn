created_file = [
  'eldbus_media_player2_player.c',
  'eldbus_mpris_media_player2.c',
  'eldbus_media_player2_player.h',
  'eldbus_mpris_media_player2.h',
  'eldbus_utils.h'
]

src = files(
  'e_mod_main.c',
  'ui.c',
  'e_mod_main.h',
  'private.h'
)

src += custom_target('gen-dbus',
                     input  : 'introspect.xml',
                     command: [eldbus_codegen_cmd, '@INPUT@', '-O', '@OUTDIR@'],
                     output : created_file
                    )
