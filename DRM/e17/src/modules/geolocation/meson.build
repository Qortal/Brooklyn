created_file = [
  'eldbus_geo_clue2_client.c',
  'eldbus_geo_clue2_client.h',
  'eldbus_geo_clue2_location.c',
  'eldbus_geo_clue2_location.h',
  'eldbus_geo_clue2_manager.c',
  'eldbus_geo_clue2_manager.h',
  'eldbus_utils.h'
]

src = files(
  'e_mod_main.c'
)

src += custom_target('geolocation_gen',
                     input  : 'org.freedesktop.GeoClue2.xml',
                     command: [eldbus_codegen_cmd, '@INPUT@', '-O', '@OUTDIR@'],
                     output : created_file
                    )
