static const uint8_t gnuk_string_vendor[] = {
  33*2+2,                       /* bLength */
  STRING_DESCRIPTOR,            /* bDescriptorType */
  /* Manufacturer: "Free Software Initiative of Japan" */
  'F', 0, 'r', 0, 'e', 0, 'e', 0, ' ', 0, 'S', 0, 'o', 0, 'f', 0,
  't', 0, 'w', 0, 'a', 0, 'r', 0, 'e', 0, ' ', 0, 'I', 0, 'n', 0,
  'i', 0, 't', 0, 'i', 0, 'a', 0, 't', 0, 'i', 0, 'v', 0, 'e', 0,
  ' ', 0, 'o', 0, 'f', 0, ' ', 0, 'J', 0, 'a', 0, 'p', 0, 'a', 0,
  'n', 0,
};

static const uint8_t gnuk_string_product[] = {
  10*2+2,                      /* bLength */
  STRING_DESCRIPTOR,            /* bDescriptorType */
  /* Product name: "Gnuk Token" */
  'G', 0, 'n', 0, 'u', 0, 'k', 0, ' ', 0, 'T', 0, 'o', 0, 'k', 0,
  'e', 0, 'n', 0,
};

const uint8_t gnuk_string_serial[] = {
  11*2+2+16,                  /* bLength */
  STRING_DESCRIPTOR,            /* bDescriptorType */
  /* Serial number: "FSIJ-1.2.6-" */
  'F', 0, 'S', 0, 'I', 0, 'J', 0, '-', 0, '1', 0, '.', 0, '2', 0,
  '.', 0, '6', 0, '-', 0,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

#ifdef USB_STRINGS_FOR_GNUK
static const uint8_t gnuk_revision_detail[] = {
  35*2+2,                     /* bLength */
  STRING_DESCRIPTOR,            /* bDescriptorType */
  /* revision detail: "release/1.1.7-199-gf4083b8-modified" */
  'r', 0, 'e', 0, 'l', 0, 'e', 0, 'a', 0, 's', 0, 'e', 0, '/', 0,
  '1', 0, '.', 0, '1', 0, '.', 0, '7', 0, '-', 0, '1', 0, '9', 0,
  '9', 0, '-', 0, 'g', 0, 'f', 0, '4', 0, '0', 0, '8', 0, '3', 0,
  'b', 0, '8', 0, '-', 0, 'm', 0, 'o', 0, 'd', 0, 'i', 0, 'f', 0,
  'i', 0, 'e', 0, 'd', 0,
};

static const uint8_t gnuk_config_options[] = {
  59*2+2,                       /* bLength */
  STRING_DESCRIPTOR,            /* bDescriptorType */
  /* configure options: "FST_01:dfu=no:debug=no:pinpad=no:certdo=no:factory_reset=no" */
  'F', 0, 'S', 0, 'T', 0, '_', 0, '0', 0, '1', 0, ':', 0, 'd', 0,
  'f', 0, 'u', 0, '=', 0, 'n', 0, 'o', 0, ':', 0, 'd', 0, 'e', 0,
  'b', 0, 'u', 0, 'g', 0, '=', 0, 'n', 0, 'o', 0, ':', 0, 'p', 0,
  'i', 0, 'n', 0, 'p', 0, 'a', 0, 'd', 0, '=', 0, 'n', 0, 'o', 0,
  ':', 0, 'c', 0, 'e', 0, 'r', 0, 't', 0, 'd', 0, 'o', 0, '=', 0,
  'n', 0, 'o', 0, ':', 0, 'f', 0, 'a', 0, 'c', 0, 't', 0, 'o', 0,
  'r', 0, 'y', 0, '_', 0, 'r', 0, 'e', 0, 's', 0, 'e', 0, 't', 0,
  '=', 0, 'n', 0, 'o', 0,
};
#endif
