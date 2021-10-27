#include "config.h"
#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   const char *src_profile;
   const char *derive_profile;
   const char *derive_options;

   if (argc < 4)
     {
        printf("This is an internal tool for enlightenment\n"
               "  enlightenment_elm_cfgtool set SRCPROFILE DERIVEDPROFILE DERIVEOPTIONS\n"
               "  enlightenment_elm_cfgtool del SRCPROFILE DERIVEDPROFILE\n"
               "\n"
               "e.g.\n"
               "  enlightenment_elm_cfgtool set standard .scale-0150-standard 'scale-mul 150'\n"
               "\n");
        return 0;
     }
   if (!strcmp(argv[1], "set"))
     {
        src_profile    = argv[2];
        derive_profile = argv[3];
        derive_options = argv[4];
        elm_config_profile_set(src_profile);
        elm_config_profile_derived_del(derive_profile);
        elm_config_profile_derived_add(derive_profile, derive_options);
     }
   else if (!strcmp(argv[1], "del"))
     {
        src_profile    = argv[2];
        derive_profile = argv[3];
        elm_config_profile_set(src_profile);
        elm_config_profile_derived_del(derive_profile);
     }
   else
     {
        printf("Unknown command '%s'\n", argv[1]);
     }
   return 0;
}
ELM_MAIN()
