#include "e_mod_main.h"

Evas_Object *
util_obj_icon_add(Evas_Object *base, Obj *o, int size)
{
   Evas_Object *ic = elm_icon_add(base);
   unsigned int maj, min;
   const char *s ="bluetooth-active";

   // XXX: replace this with a much better database...
   maj = o->klass & BZ_OBJ_CLASS_MAJ_MASK;
   if (maj == BZ_OBJ_CLASS_MAJ_MISC)
     {
        s ="bluetooth-active";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_COMPUTER)
     {
        min = o->klass & BZ_OBJ_CLASS_MIN_COMPUTER_MASK;
        if (min == BZ_OBJ_CLASS_MIN_COMPUTER_DESKTOP)
          s = "computer";
        else if (min == BZ_OBJ_CLASS_MIN_COMPUTER_SERVER)
          s = "computer";
        else if (min == BZ_OBJ_CLASS_MIN_COMPUTER_LAPTOP)
          s = "computer-laptop";
        else if (min == BZ_OBJ_CLASS_MIN_COMPUTER_CLAMSHELL)
          s = "computer-laptop";
        else if (min == BZ_OBJ_CLASS_MIN_COMPUTER_PDA)
          s = "pda";
        else if (min == BZ_OBJ_CLASS_MIN_COMPUTER_WEARABLE)
          s = "pda";
        else if (min == BZ_OBJ_CLASS_MIN_COMPUTER_TABLET)
          s = "pda";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_PHONE)
     {
        min = o->klass & BZ_OBJ_CLASS_MIN_PHONE_MASK;
        if (min == BZ_OBJ_CLASS_MIN_PHONE_CELL)
          s = "phone";
        else if (min == BZ_OBJ_CLASS_MIN_PHONE_CORDLESS)
          s = "phone";
        else if (min == BZ_OBJ_CLASS_MIN_PHONE_SMARTPHONE)
          s = "phone";
        else if (min == BZ_OBJ_CLASS_MIN_PHONE_WIRED)
          s = "modem";
        else if (min == BZ_OBJ_CLASS_MIN_PHONE_ISDN)
          s = "modem";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_LAN)
     {
        s = "network-wired";
        // XXX: handle (top is max availability)
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_7
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_6
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_5
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_4
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_3
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_2
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_1
        // BZ_OBJ_CLASS_MIN_LAN_AVAIL_0
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_AV)
     {
        min = o->klass & BZ_OBJ_CLASS_MIN_AV_MASK;
        if (min == BZ_OBJ_CLASS_MIN_AV_WEARABLE_HEADSET)
          s = "audio-input-microphone";
        else if (min == BZ_OBJ_CLASS_MIN_AV_HANDS_FREE)
          s = "audio-input-microphone";
        else if (min == BZ_OBJ_CLASS_MIN_AV_MIC)
          s = "audio-input-microphone";
        else if (min == BZ_OBJ_CLASS_MIN_AV_SPEAKER)
          s = "audio-volume-high";
        else if (min == BZ_OBJ_CLASS_MIN_AV_HEADPHONES)
          s = "audio-volume-high";
        else if (min == BZ_OBJ_CLASS_MIN_AV_PORTABLE_AUDIO)
          s = "audio-volume-high";
        else if (min == BZ_OBJ_CLASS_MIN_AV_CAR_AUDIO)
          s = "audio-volume-high";
        else if (min == BZ_OBJ_CLASS_MIN_AV_SET_TOP_BOX)
          s = "modem";
        else if (min == BZ_OBJ_CLASS_MIN_AV_HIFI_AUDIO)
          s = "audio-volume-high";
        else if (min == BZ_OBJ_CLASS_MIN_AV_VCR)
          s = "media-tape";
        else if (min == BZ_OBJ_CLASS_MIN_AV_VIDEO_CAMERA)
          s = "camera-photo";
        else if (min == BZ_OBJ_CLASS_MIN_AV_CAMCORDER)
          s = "camera-photo";
        else if (min == BZ_OBJ_CLASS_MIN_AV_VIDEO_MONITOR)
          s = "video-display";
        else if (min == BZ_OBJ_CLASS_MIN_AV_VIDEO_DISPLAY_SPEAKER)
          s = "video-display";
        else if (min == BZ_OBJ_CLASS_MIN_AV_VIDEO_CONFERENCE)
          s = "video-display";
        else if (min == BZ_OBJ_CLASS_MIN_AV_GAMING)
          s = "input-gaming";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_PERIPHERAL)
     {
        s = "input-keyboard";

        // XXX: handle bits + ide below
        if (o->klass & BZ_OBJ_CLASS_MIN_PERIPHERAL_KEYBOARD_BIT)
          s = "input-keyboard";
        else if (o->klass & BZ_OBJ_CLASS_MIN_PERIPHERAL_MOUSE_BIT)
          s = "input-mouse";

        min = o->klass & BZ_OBJ_CLASS_MIN_PERIPHERAL_MASK2;
        if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_JOYSTICK)
          s = "input-gaming";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_GAMEPAD)
          s = "input-gaming";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_REMOTE)
          s = "input-gaming";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_SENSING)
          s = "input-gaming";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_DIGITIZER_TAB)
          s = "input-tablet";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_CARD_READER)
          s = "media-flash";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_PEN)
          s = "input-mouse";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_SCANNER)
          s = "scanner";
        else if (min == BZ_OBJ_CLASS_MIN_PERIPHERAL_WAND)
          s = "input-mouse";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_IMAGING)
     {
        // XXX: handle permutations of bits
        if (o->klass & BZ_OBJ_CLASS_MIN_IMAGING_CAMERA_BIT)
          s = "camera-photo";
        else if (o->klass & BZ_OBJ_CLASS_MIN_IMAGING_SCANNER_BIT)
          s = "scanner";
        else if (o->klass & BZ_OBJ_CLASS_MIN_IMAGING_PRINTER_BIT)
          s = "printer";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_WEARABLE)
     {
        min = o->klass & BZ_OBJ_CLASS_MIN_WEARABLE_MASK;
        if (min == BZ_OBJ_CLASS_MIN_WEARABLE_WATCH)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_WEARABLE_PAGER)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_WEARABLE_JACKET)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_WEARABLE_HELMET)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_WEARABLE_GLASSES)
          s = "cpu";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_TOY)
     {
        min = o->klass & BZ_OBJ_CLASS_MIN_TOY_MASK;
        if (min == BZ_OBJ_CLASS_MIN_TOY_ROBOT)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_TOY_VEHICLE)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_TOY_DOLL)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_TOY_CONTROLLER)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_TOY_GAME)
          s = "cpu";
     }
   else if (maj == BZ_OBJ_CLASS_MAJ_HEALTH)
     {
        min = o->klass & BZ_OBJ_CLASS_MIN_HEALTH_MASK;
        if (min == BZ_OBJ_CLASS_MIN_HEALTH_BLOOD_PRESSURE)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_THERMOMETER)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_SCALES)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_GLUCOSE)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_PULSE_OXIMITER)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_HEART_RATE)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_HEALTH_DATA_DISP)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_STEP)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_BODY_COMPOSITION)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_PEAK_FLOW)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_MEDICATION)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_KNEE_PROSTHESIS)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_ANKLE_PROSTHESIS)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_GENERIC_HEALTH)
          s = "cpu";
        else if (min == BZ_OBJ_CLASS_MIN_HEALTH_PRESONAL_MOBILITY)
          s = "cpu";
     }
   elm_icon_standard_set(ic, s);
   evas_object_size_hint_min_set(ic,
                                 ELM_SCALE_SIZE(size),
                                 ELM_SCALE_SIZE(size));
   return ic;
}

Evas_Object *
util_obj_icon_rssi_add(Evas_Object *base, Obj *o, int size)
{
   Evas_Object *ic = elm_icon_add(base);
   char buf[64];
   if (o->rssi <= -80)
     elm_icon_standard_set(ic, "network-cellular-signal-excellent");
   else if (o->rssi <= -72)
     elm_icon_standard_set(ic, "network-cellular-signal-good");
   else if (o->rssi <= -64)
     elm_icon_standard_set(ic, "network-cellular-signal-ok");
   else if (o->rssi <= -56)
     elm_icon_standard_set(ic, "network-cellular-signal-weak");
   else if (o->rssi <= -48)
     elm_icon_standard_set(ic, "network-cellular-signal-none");
   else
     elm_icon_standard_set(ic, "network-cellular-signal-acquiring");
   snprintf(buf, sizeof(buf), "RSSI: %i", (int)o->rssi);
   elm_object_tooltip_text_set(ic, buf);
   evas_object_size_hint_min_set(ic,
                                 ELM_SCALE_SIZE(size),
                                 ELM_SCALE_SIZE(size));
   return ic;
}

Evas_Object *
util_check_add(Evas_Object *base, const char *text, const char *tip, Eina_Bool state)
{
   Evas_Object *ck = elm_check_add(base);
   evas_object_size_hint_align_set(ck, 0.0, EVAS_HINT_FILL);
   elm_layout_text_set(ck, NULL, text);
   elm_object_tooltip_text_set(ck, tip);
   elm_check_state_set(ck, state);
   return ck;
}

Evas_Object *
util_button_icon_add(Evas_Object *base, const char *icon, const char *tip)
{
   Evas_Object *ic, *bt = elm_button_add(base);
   ic = elm_icon_add(base);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_icon_standard_set(ic, icon);
   elm_object_tooltip_text_set(bt, tip);
   elm_object_part_content_set(bt, NULL, ic);
   evas_object_show(ic);
   return bt;
}

const char *
util_obj_name_get(Obj *o)
{
   if (o->name) return o->name;
   if (o->alias) return o->alias;
   if (o->address) return o->address;
   return _("Unknown");
}
