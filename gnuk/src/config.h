#undef DEBUG
#ifdef DEBUG
#define ENABLE_VIRTUAL_COM_PORT 1
#endif
#undef DFU_SUPPORT
#undef PINPAD_SUPPORT

#undef CERTDO_SUPPORT
#undef HID_CARD_CHANGE_SUPPORT
#define SERIALNO_STR_LEN 11
#undef LIFE_CYCLE_MANAGEMENT_SUPPORT
