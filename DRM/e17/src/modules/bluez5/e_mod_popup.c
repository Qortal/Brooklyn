#include "e_mod_main.h"

static Elm_Genlist_Item_Class *adapt_itc = NULL;
static Elm_Genlist_Item_Class *dev_itc = NULL;
static Elm_Genlist_Item_Class *group_itc = NULL;
static Eina_List *lists = NULL;
static Eina_List *adapters = NULL;
static Eina_List *devices = NULL;

static int unlock_count = 0;
static int unlock_do = 0;
static Eina_Bool unlock_block = EINA_FALSE;

static Config_Device *
_devices_conifg_find(const char *address)
{
   Config_Device *dev;
   Eina_List *l;

   if (!ebluez5_config) return NULL;
   if (!address) return NULL;
   EINA_LIST_FOREACH(ebluez5_config->devices, l, dev)
     {
        if ((dev->addr) && (!strcmp(address, dev->addr))) return dev;
     }
   return NULL;
}

static void
_devices_eval(void)
{
   Obj *o;
   Eina_List *l;
   Config_Device *dev;
   int unlock_count_prev = 0;

   unlock_count_prev = unlock_count;
   unlock_count = 0;
   unlock_do = 0;
   printf("=== _devices_eval...\n");
   EINA_LIST_FOREACH(devices, l, o)
     {
        if (o->paired)
          {
             Eina_Bool need_ping = EINA_FALSE;

             dev = _devices_conifg_find(o->address);
             if (dev)
               {
                  printf("=== dev: %s|%s [%s]\n", dev->addr, o->address, o->name);
                  if ((dev->force_connect) && (!o->connected))
                    {
                       printf("=== %s force con, not conn, ping ok=%i\n", o->address, o->ping_ok);
                       if (o->ping_ok)
                         {
                            printf("=== %s force con, not conn, ping ok=%i\n", o->address, o->ping_ok);
                            bz_obj_connect(o);
                         }
                       else need_ping = EINA_TRUE;
                    }
                  if (dev->unlock)
                    {
                       printf("=== unlock...\n");
                       // if a device on our list needs an unlock, then
                       // add to our possible unlock counts needed
                       unlock_count++;
#if 0
                       // if the device is connected then assume it unlocks
                       if (o->connected)
                         {
                            printf("=== don't need ping1\n");
                            unlock_do++;
                         }
                       else
#endif
                         {
                            printf("=== need ping2\n");
                            need_ping = EINA_TRUE;
                            if (o->ping_ok) unlock_do++;
                         }
                    }
               }
             printf("=== %s need_ping=%i conn=%i ping_ok=%i\n", o->address, need_ping, o->connected, o->ping_ok);
             if (need_ping) bz_obj_ping_begin(o);
             else bz_obj_ping_end(o);
          }
     }
   printf("=================== unlock: %i/%i\n", unlock_do, unlock_count);
   if (unlock_count > 0)
     {
        if (unlock_do == 0)
          {
             if (unlock_block)
               {
                  unlock_block = EINA_FALSE;
                  printf("=== DESKLOCK UNBLOCK\n");
                  e_desklock_unblock();
                  printf("=== DESLOCK SHOW\n");
                  e_desklock_show(EINA_FALSE);
               }
          }
        else
          {
             if (!unlock_block)
               {
                  unlock_block = EINA_TRUE;
                  printf("=== DESKLOCK BLOCK\n");
                  e_desklock_block();
               }
          }
     }
   else
     {
        if (unlock_count_prev != unlock_count)
          {
             if (!e_desklock_manual_get())
               {
                  if (e_desklock_state_get())
                    {
                       printf("=== DESKLOCK HIDE\n");
                       e_desklock_hide();
                    }
               }
          }
        if (unlock_block)
          {
             unlock_block = EINA_FALSE;
             printf("=== DESKLOCK UNBLOCK\n");
             e_desklock_unblock();
          }
     }
}

static void
_adapter_add(Evas_Object *gl, Obj *o)
{
   Elm_Object_Item *it = evas_object_data_get(gl, "adapters_item");;

   elm_genlist_item_append(gl, adapt_itc, o, it, ELM_GENLIST_ITEM_NONE,
                           NULL, NULL);
}

static int
_cb_insert_cmp(const void *ai, const void *bi)
{
   Obj *a = elm_object_item_data_get(ai);
   Obj *b = elm_object_item_data_get(bi);
   Eina_Bool apub = EINA_FALSE, bpub = EINA_FALSE;

   if ((!a) || (!a->address)) return -1;
   if ((!b) || (!b->address)) return 1;

   // prefer paired at top
   if ((a->paired) && (!b->paired)) return -1;
   if ((!a->paired) && (b->paired)) return 1;
   // prefer public addresses next after being paired
   if ((a->address_type) && (!strcmp(a->address_type, "public")))
     apub = EINA_TRUE;
   if ((b->address_type) && (!strcmp(b->address_type, "public")))
     bpub = EINA_TRUE;
   if ((apub) && (!bpub)) return -1;
   if ((!apub) && (bpub)) return 1;
   // and sort by address
   return strcmp(a->address, b->address);
}

static void
_device_add(Evas_Object *gl, Obj *o)
{
   Elm_Object_Item *it = evas_object_data_get(gl, "devices_item");;

   elm_genlist_item_sorted_insert(gl, dev_itc, o, it,
                                  ELM_GENLIST_ITEM_NONE,
                                  _cb_insert_cmp, NULL, NULL);
}

static void
_cb_power(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   if (elm_check_state_get(obj))
     {
        if (o->path)
          {
             const char *s = strrchr(o->path, '/');

             if (s) ebluez5_rfkill_unblock(s + 1);
          }
        bz_obj_power_on(o);
     }
   else bz_obj_power_off(o);
}

static void
_cb_scan(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   if (elm_check_state_get(obj)) bz_obj_discover_start(o);
   else bz_obj_discover_stop(o);
}

static void
_cb_visible(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   if (elm_check_state_get(obj)) bz_obj_discoverable(o);
   else bz_obj_undiscoverable(o);
}

static void
_unflip(Obj *o, Evas_Object *obj)
{
   Evas_Object *gl = evas_object_data_get(obj, "genlist");
   Elm_Object_Item *it;

   for (it = elm_genlist_first_item_get(gl); it;
        it = elm_genlist_item_next_get(it))
     {
        if (o == elm_object_item_data_get(it))
          {
             if (elm_genlist_item_flip_get(it))
               elm_genlist_item_flip_set(it, EINA_FALSE);
             break;
          }
     }
}

static void
_cb_pairable(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   if (elm_check_state_get(obj)) bz_obj_pairable(o);
   else bz_obj_unpairable(o);
   _unflip(o, obj);
}

static void
_cb_connect(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   bz_obj_connect(o);
   _unflip(o, obj);
}

static void
_cb_disconnect(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   bz_obj_disconnect(o);
   _unflip(o, obj);
}

static void
_cb_trust(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   bz_obj_trust(o);
   _unflip(o, obj);
}

static void
_cb_distrust(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   bz_obj_distrust(o);
   _unflip(o, obj);
}

static void
_cb_pair(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   bz_obj_pair(o);
   _unflip(o, obj);
}

static void
_cb_unpair(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   ebluez5_device_prop_unlock_set(o-> address, EINA_FALSE);
   bz_obj_remove(o);
   _unflip(o, obj);
}

static void
_cb_unlock_start(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   printf("BZ5: unlock start %s\n", o->address);
   ebluez5_device_prop_unlock_set(o->address, EINA_TRUE);
   ebluez5_popup_device_change(o);
   _unflip(o, obj);
}

static void
_cb_unlock_stop(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   printf("BZ5: unlock stop %s\n", o->address);
   ebluez5_device_prop_unlock_set(o->address, EINA_FALSE);
   ebluez5_popup_device_change(o);
   _unflip(o, obj);
}

static void
_cb_force_connect_start(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   ebluez5_device_prop_force_connect_set(o->address, EINA_TRUE);
   ebluez5_popup_adapter_change(o);
   _unflip(o, obj);
}

static void
_cb_force_connect_stop(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   ebluez5_device_prop_force_connect_set(o->address, EINA_FALSE);
   ebluez5_popup_adapter_change(o);
   _unflip(o, obj);
}

static void
_cb_flip(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   Evas_Object *gl = evas_object_data_get(obj, "genlist");
   Elm_Object_Item *it;

   for (it = elm_genlist_first_item_get(gl); it;
        it = elm_genlist_item_next_get(it))
     {
        if (o == elm_object_item_data_get(it))
          {
             if (elm_genlist_item_flip_get(it))
               elm_genlist_item_flip_set(it, EINA_FALSE);
             else
               elm_genlist_item_flip_set(it, EINA_TRUE);
             break;
          }
     }
}

static void
_agent_done(Obj *o)
{
   Eina_List *l;
   Evas_Object *gl;
   Elm_Object_Item *it;

   if (o->agent_request)
     {
        eina_stringshare_del(o->agent_request);
        o->agent_request = NULL;
     }
   EINA_LIST_FOREACH(lists, l, gl)
     {
        for (it = elm_genlist_first_item_get(gl); it;
             it = elm_genlist_item_next_get(it))
          {
             if (o == elm_object_item_data_get(it))
               {
                  elm_genlist_item_update(it);
                  break;
               }
          }
     }
}

static void
_cb_agent_ok(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Obj *o = data;

   if ((o->agent_entry_fn) && (o->agent_msg_ok))
     {
        Evas_Object *en = evas_object_data_get(obj, "entry");

        if (en)
          {
             const char *s = elm_object_text_get(en);

             if (s) o->agent_entry_fn(o->agent_msg_ok, s);
          }
     }
   if (o->agent_msg_err)
     {
        bz_agent_msg_drop(o->agent_msg_err);
        o->agent_msg_err = NULL;
     }
   if (o->agent_msg_ok)
     {
        bz_agent_msg_reply(o->agent_msg_ok);
        o->agent_msg_ok = NULL;
     }
   _agent_done(o);
}

static void
_cb_agent_cancel(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Obj *o = data;
   if (o->agent_msg_ok)
     {
        bz_agent_msg_drop(o->agent_msg_ok);
        o->agent_msg_ok = NULL;
     }
   if (o->agent_msg_err)
     {
        bz_agent_msg_reply(o->agent_msg_err);
        o->agent_msg_err = NULL;
     }
   _agent_done(o);
}

static char *
_cb_group_text_get(void *data, Evas_Object *obj EINA_UNUSED,
                   const char *part EINA_UNUSED)
{
   if (!data) return strdup(_("Adapters"));
   return strdup(_("Devices"));
}

static Evas_Object *
_cb_group_content_get(void *data EINA_UNUSED, Evas_Object *obj,
                      const char *part EINA_UNUSED)
{
   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *ic = elm_icon_add(obj);
        if (!data)
          elm_icon_standard_set(ic, "computer");
        else
          elm_icon_standard_set(ic, "system-run");
        evas_object_size_hint_min_set(ic,
                                      ELM_SCALE_SIZE(16),
                                      ELM_SCALE_SIZE(16));
        return ic;
     }
   return NULL;
}

static char *
_cb_adapt_text_get(void *data, Evas_Object *obj EINA_UNUSED,
                   const char *part EINA_UNUSED)
{
   Obj *o = data;

   if (!strcmp(part, "elm.text"))
     {
        return strdup(util_obj_name_get(o));
     }
   else if (!strcmp(part, "elm.text.sub"))
     {
        if (o->address) return strdup(o->address);
        return strdup(_("Unknown Address"));
     }
   return NULL;
}

static Evas_Object *
_cb_adapt_content_get(void *data EINA_UNUSED, Evas_Object *obj,
                      const char *part EINA_UNUSED)
{
   Obj *o = data;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        return util_obj_icon_add(obj, o, 48);
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *tab, *ck;

        tab = elm_table_add(obj);
        evas_object_size_hint_weight_set(tab, EVAS_HINT_EXPAND, 0);
        evas_object_size_hint_align_set(tab, EVAS_HINT_FILL, 0.0);

        ck = util_check_add(obj, _("Power"), _("Enable power for this adapter"),
                            o->powered);
        evas_object_smart_callback_add(ck, "changed", _cb_power, o);
        elm_table_pack(tab, ck, 0, 0, 1, 1);
        evas_object_show(ck);

        ck = util_check_add(obj, _("Visible"), _("Make this adapter visible to other devices"),
                            o->discoverable);
        evas_object_smart_callback_add(ck, "changed", _cb_visible, o);
        elm_table_pack(tab, ck, 1, 0, 1, 1);
        evas_object_show(ck);

        ck = util_check_add(obj, _("Scan"), _("Scan for other devices"),
                            o->discovering);
        evas_object_smart_callback_add(ck, "changed", _cb_scan, o);
        elm_table_pack(tab, ck, 0, 1, 1, 1);
        evas_object_show(ck);

        ck = util_check_add(obj, _("Pairable"), _("Allow this adapter to have other devices request to pair with it"),
                            o->pairable);
        evas_object_smart_callback_add(ck, "changed", _cb_pairable, o);
        elm_table_pack(tab, ck, 1, 1, 1, 1);
        evas_object_show(ck);

        return tab;
     }
   return NULL;
}

static char *
_cb_dev_text_get(void *data, Evas_Object *obj EINA_UNUSED,
                 const char *part EINA_UNUSED)
{
   Obj *o = data;
   return strdup(util_obj_name_get(o));
}

static Evas_Object *
_cb_dev_content_get(void *data, Evas_Object *obj,
                    const char *part)
{
   Obj *o = data;
   char buf[512];

   if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *ic, *bx;

        bx = elm_box_add(obj);
        ic = util_obj_icon_add(obj, o, 24);
        snprintf(buf, sizeof(buf),
                 _("Address: %s (%s)<br>"
                   "Services: %s%s%s%s%s%s%s%s%s<br>"
                   "Trusted: %s<br>"
                   "Blocked: %s<br>"
                     )
                 ,
                 (o->address) ? o->address : _("Unknown"),
                 (o->address_type) ? o->address_type : _("Unknown"),
                 (o->klass & BZ_OBJ_CLASS_SERVICE_LIMITED_DISCOVERABLE) ? _("Limited-Discoverable ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_POSITIONING_BIT) ? _("Positioning ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_NETWORKING_BIT) ? _("Networking ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_RENDERING_BIT) ? _("Rendering ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_CAPTURING_BIT) ? _("Capture ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_OBJECT_TRANSFER_BIT) ? _("OBEX ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_AUDIO_BIT) ? _("Audio ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_TELEPHONY_BIT) ? _("Telephony ") : "",
                 (o->klass & BZ_OBJ_CLASS_SERVICE_INFORMATION_BIT) ? _("Information ") : "",
                 (o->trusted) ? _("Yes") : _("No"),
                 (o->blocked) ? _("Yes") : _("No")
                );
        elm_object_tooltip_text_set(ic, buf);
        elm_box_pack_end(bx, ic);
        evas_object_show(ic);
        return bx;
     }
   else if (!strcmp(part, "elm.text.flip"))
     {
        Evas_Object *bx, *bt, *ic;

        bx = elm_box_add(obj);
        elm_box_horizontal_set(bx, EINA_TRUE);
        elm_box_align_set(bx, 1.0, 0.5);
        if (o->paired)
          {
             if (o->paired)
               {
                  Config_Device *dev = ebluez5_device_prop_find(o->address);

                  if ((dev) && (dev->unlock))
                    {
                       bt = util_button_icon_add(obj, "bt-lock-device-off",
                                                 _("Stop this from being an unlock device"));
                       evas_object_data_set(bt, "genlist", obj);
                       evas_object_smart_callback_add(bt, "clicked", _cb_unlock_stop, o);
                    }
                  else
                    {
                       bt = util_button_icon_add(obj, "bt-lock-device-on",
                                                 _("Make this auto unlock when detected (and lock when not)"));
                       evas_object_data_set(bt, "genlist", obj);
                       evas_object_smart_callback_add(bt, "clicked", _cb_unlock_start, o);
                    }
                  elm_box_pack_end(bx, bt);
                  evas_object_show(bt);

                  if ((dev) && (dev->force_connect))
                    {
                       bt = util_button_icon_add(obj, "bt-force-connect-off",
                                                 _("Stop this device from being forcefully connected"));
                       evas_object_data_set(bt, "genlist", obj);
                       evas_object_smart_callback_add(bt, "clicked", _cb_force_connect_stop, o);
                    }
                  else
                    {
                       bt = util_button_icon_add(obj, "bt-force-connect-on",
                                                 _("Force this device to be connected when detected"));
                       evas_object_data_set(bt, "genlist", obj);
                       evas_object_smart_callback_add(bt, "clicked", _cb_force_connect_start, o);
                    }
                  elm_box_pack_end(bx, bt);
                  evas_object_show(bt);
               }
             if (o->connected)
               {
                  bt = util_button_icon_add(obj, "bt-connect-off",
                                            _("Disconnect this device"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_disconnect, o);
               }
             else
               {
                  bt = util_button_icon_add(obj, "bt-connect-on",
                                            _("Connect this device"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_connect, o);
               }
             elm_box_pack_end(bx, bt);
             evas_object_show(bt);

             if (o->trusted)
               {
                  bt = util_button_icon_add(obj, "security-low",
                                            _("Disrust this device"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_distrust, o);
               }
             else
               {
                  bt = util_button_icon_add(obj, "security-high",
                                            _("Trust this device"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_trust, o);
               }
             elm_box_pack_end(bx, bt);
             evas_object_show(bt);
          }
        if (!o->paired)
          {
             if (!o->agent_request)
               {
                  bt = util_button_icon_add(obj, "bt-pair-on",
                                            _("Pair with this device"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_pair, o);
                  elm_box_pack_end(bx, bt);
                  evas_object_show(bt);
               }
          }
        else
          {
             bt = util_button_icon_add(obj, "bt-pair-off",
                                        _("Unpair with this device"));
             evas_object_data_set(bt, "genlist", obj);
             evas_object_smart_callback_add(bt, "clicked", _cb_unpair, o);
             elm_box_pack_end(bx, bt);
             evas_object_show(bt);
          }

        bt = util_button_icon_add(obj, "bt-expand-right",
                                  _("Cancel"));
        evas_object_data_set(bt, "genlist", obj);
        evas_object_smart_callback_add(bt, "clicked", _cb_flip, o);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);

        ic = util_obj_icon_rssi_add(obj, o, 24);
        elm_box_pack_end(bx, ic);
        evas_object_show(ic);
        return bx;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *bx, *ic, *bt, *lb, *tb, *en, *rec;

        bx = elm_box_add(obj);
        elm_box_horizontal_set(bx, EINA_TRUE);

        if (o->agent_request)
          {
             if (o->agent_entry_fn)
               {
                  tb = elm_table_add(obj);

                  rec = evas_object_rectangle_add(evas_object_evas_get(obj));
                  evas_object_size_hint_min_set(rec, ELM_SCALE_SIZE(80), ELM_SCALE_SIZE(1));
                  elm_table_pack(tb, rec, 0, 0, 1, 1);

                  en = elm_entry_add(obj);
                  elm_entry_single_line_set(en, EINA_TRUE);
                  elm_entry_scrollable_set(en, EINA_TRUE);
                  elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
                  elm_object_part_text_set(en, "guide", o->agent_request);
//                  elm_entry_password_set(en, EINA_TRUE);
                  evas_object_smart_callback_add(en, "activated", _cb_agent_ok, o);
                  evas_object_smart_callback_add(en, "aborted", _cb_agent_cancel, o);
                  elm_table_pack(tb, en, 0, 0, 1, 1);
                  evas_object_show(en);

                  elm_box_pack_end(bx, tb);
                  evas_object_show(tb);

                  bt = util_button_icon_add(obj, "bt-pair-ok",
                                            _("Pair with this device"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_data_set(bt, "entry", en);
                  evas_object_smart_callback_add(bt, "clicked", _cb_agent_ok, o);
                  elm_box_pack_end(bx, bt);
                  evas_object_show(bt);

                  bt = util_button_icon_add(obj, "bt-pair-cancel",
                                            _("Reject pairing"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_agent_cancel, o);
                  elm_box_pack_end(bx, bt);
                  evas_object_show(bt);
               }
             else
               {
                  lb = elm_label_add(obj);
                  elm_layout_text_set(lb, NULL, o->agent_request);
                  elm_box_pack_end(bx, lb);
                  evas_object_show(lb);

                  bt = util_button_icon_add(obj, "bt-pair-ok",
                                            _("Pair with this device"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_agent_ok, o);
                  elm_box_pack_end(bx, bt);
                  evas_object_show(bt);

                  bt = util_button_icon_add(obj, "bt-pair-cancel",
                                            _("Reject pairing"));
                  evas_object_data_set(bt, "genlist", obj);
                  evas_object_smart_callback_add(bt, "clicked", _cb_agent_cancel, o);
                  elm_box_pack_end(bx, bt);
                  evas_object_show(bt);
               }
          }

        if (o->bat_percent >= 0)
          {
             Evas_Object *bat;
             Edje_Message_Float msg;
             double level = 0.0;
             const int size = 24;

             bat = edje_object_add(evas_object_evas_get(obj));
             e_theme_edje_object_set(bat, "base/theme/modules/battery",
                                     "e/modules/battery/main");
             snprintf(buf, sizeof(buf), "%i", o->bat_percent);
             edje_object_part_text_set(bat, "e.text.reading", buf);
             level = (double)o->bat_percent / 100.0;
             if (level > 1.0) level = 1.0;
             msg.val = level;
             edje_object_message_send(bat, EDJE_MESSAGE_FLOAT, 1, &msg);
             evas_object_size_hint_min_set(bat,
                                           ELM_SCALE_SIZE(size),
                                           ELM_SCALE_SIZE(size));
             elm_box_pack_end(bx, bat);
             evas_object_show(bat);
          }

        bt = util_button_icon_add(obj, "bt-expand-left",
                                  _("Options for device like connect, pair etc."));
        evas_object_data_set(bt, "genlist", obj);
        evas_object_smart_callback_add(bt, "clicked", _cb_flip, o);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);

        ic = util_obj_icon_rssi_add(obj, o, 24);
        elm_box_pack_end(bx, ic);
        evas_object_show(ic);
        return bx;
     }
   return NULL;
}

static void
_cb_list_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   lists = eina_list_remove(lists, obj);
}

/*
static void
_cb_settings(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst = data;

   ebluez5_popup_hide(inst);
   if (e_configure_registry_exists("extensions/bluez5"))
      e_configure_registry_call("extensions/bluez5", NULL, NULL);
}
*/

Evas_Object *
ebluez5_popup_content_add(Evas_Object *base, Instance *inst)
{
   Evas_Object *o, *box, *tab, *gl;
   Eina_List *l;
   Elm_Object_Item *it;
   Obj *oo;

   o = box = elm_box_add(base);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   tab = o = elm_table_add(base);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   o = evas_object_rectangle_add(evas_object_evas_get(base));
   evas_object_size_hint_min_set(o, ELM_SCALE_SIZE(320), ELM_SCALE_SIZE(240));
   evas_object_size_hint_max_set(o, ELM_SCALE_SIZE(560), ELM_SCALE_SIZE(400));
   elm_table_pack(tab, o, 0, 0, 1, 1);

   o = gl = elm_genlist_add(base);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_genlist_mode_set(o, ELM_LIST_LIMIT);
   elm_genlist_select_mode_set(o, ELM_OBJECT_SELECT_MODE_NONE);

   lists = eina_list_append(lists, gl);
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, _cb_list_del, inst);

   it = elm_genlist_item_append(gl, group_itc, NULL, NULL,
                                ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   evas_object_data_set(gl, "adapters_item", it);
   it = elm_genlist_item_append(gl, group_itc, gl, NULL,
                                ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   evas_object_data_set(gl, "devices_item", it);

   EINA_LIST_FOREACH(adapters, l, oo)
     {
        _adapter_add(gl, oo);
     }
   EINA_LIST_FOREACH(devices, l, oo)
     {
        _device_add(gl, oo);
     }

   elm_table_pack(tab, o, 0, 0, 1, 1);
   evas_object_show(o);

   elm_box_pack_end(box, tab);
   evas_object_show(tab);
/*
   o = elm_separator_add(base);
   elm_separator_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, o);
   evas_object_show(o);

   o = elm_button_add(base);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_text_set(o, NULL, _("Settings"));
   elm_object_tooltip_text_set(o, _("Bring up more detailed Bluetooth settings"));
   evas_object_smart_callback_add(o, "clicked", _cb_settings, inst);
   elm_box_pack_end(box, o);
   evas_object_show(o);
 */
   return box;
}

void
ebluze5_popup_init(void)
{
   adapt_itc = elm_genlist_item_class_new();
   adapt_itc->item_style = "double_label";
   adapt_itc->func.text_get = _cb_adapt_text_get;
   adapt_itc->func.content_get = _cb_adapt_content_get;
   adapt_itc->func.state_get = NULL;
   adapt_itc->func.del = NULL;

   dev_itc = elm_genlist_item_class_new();
   dev_itc->item_style = "default";
   dev_itc->func.text_get = _cb_dev_text_get;
   dev_itc->func.content_get = _cb_dev_content_get;
   dev_itc->func.state_get = NULL;
   dev_itc->func.del = NULL;

   group_itc = elm_genlist_item_class_new();
   group_itc->item_style = "group_index";
   group_itc->func.text_get = _cb_group_text_get;
   group_itc->func.content_get = _cb_group_content_get;
   group_itc->func.state_get = NULL;
   group_itc->func.del = NULL;
}

void
ebluze5_popup_shutdown(void)
{
   if (unlock_block)
     {
        unlock_block = EINA_FALSE;
        e_desklock_unblock();
     }
   ebluze5_popup_clear();
   elm_genlist_item_class_free(group_itc);
   elm_genlist_item_class_free(dev_itc);
   elm_genlist_item_class_free(adapt_itc);
   group_itc = NULL;
   dev_itc = NULL;
   adapt_itc = NULL;
}

void
ebluze5_popup_clear(void)
{
   Eina_List *l;
   Evas_Object *gl;

   adapters = eina_list_free(adapters);
   devices = eina_list_free(devices);
   EINA_LIST_FOREACH(lists, l, gl)
     {
        elm_genlist_clear(gl);
     }
}

static Eina_Bool
_cb_adapter_add_delayed_setup(void *data)
{
   char *path = data;
   Obj *o;

   if (!path) return EINA_FALSE;
   o = bz_obj_find(path);
   if ((o) && (o->address))
     {
        Eina_List *l;
        Config_Adapter *ad;

        EINA_LIST_FOREACH(ebluez5_config->adapters, l, ad)
          {
             if (!ad->addr) continue;
             if (!strcmp(ad->addr, o->address))
               {
                  if (ad->powered)
                    {
                       printf("==== BZ INIT REQ POWER ON %s\n", o->address);
                       if (o->path)
                         {
                            const char *s = strrchr(o->path, '/');

                            if (s) ebluez5_rfkill_unblock(s + 1);
                         }
                       bz_obj_power_on(o);
                    }
                  else
                    {
                       printf("==== BZ INIT REQ POWER OFF %s\n", o->address);
                       bz_obj_power_off(o);
                    }
                  if (ad->pairable) bz_obj_pairable(o);
                  else bz_obj_unpairable(o);
               }
          }
     }
   free(path);
   return EINA_FALSE;
}

void
ebluez5_popup_adapter_add(Obj *o)
{
   Eina_List *l;
   Config_Adapter *ad;
   Evas_Object *gl;

   adapters = eina_list_append(adapters, o);
   EINA_LIST_FOREACH(lists, l, gl)
     {
        _adapter_add(gl, o);
     }
   if ((ebluez5_config) && (o->address))
     {
        EINA_LIST_FOREACH(ebluez5_config->adapters, l, ad)
          {
             if (!ad->addr) continue;
             if (!strcmp(ad->addr, o->address))
               ecore_timer_add(1.0, _cb_adapter_add_delayed_setup,
                               strdup(o->path));
          }
     }
   ebluez5_instances_update();
}

void
ebluez5_popup_adapter_del(Obj *o)
{
   Eina_List *l;
   Evas_Object *gl;
   Elm_Object_Item *it;

   EINA_LIST_FOREACH(lists, l, gl)
     {
        for (it = elm_genlist_first_item_get(gl); it;
             it = elm_genlist_item_next_get(it))
          {
             if (o == elm_object_item_data_get(it))
               {
                  elm_object_item_del(it);
                  break;
               }
          }
     }
   adapters = eina_list_remove(adapters, o);
   ebluez5_instances_update();
}

void
ebluez5_popup_adapter_change(Obj *o)
{
   Eina_List *l;
   Evas_Object *gl;
   Elm_Object_Item *it;

   EINA_LIST_FOREACH(lists, l, gl)
     {
        for (it = elm_genlist_first_item_get(gl); it;
             it = elm_genlist_item_next_get(it))
          {
             if (o == elm_object_item_data_get(it))
               {
                  elm_genlist_item_update(it);
                  if (o->address)
                    ebluez5_conf_adapter_add(o->address, o->powered,
                                             o->pairable);
                  break;
               }
          }
     }
   ebluez5_instances_update();
}

void
ebluez5_popup_device_add(Obj *o)
{
   Eina_List *l;
   Evas_Object *gl;

   devices = eina_list_append(devices, o);
   EINA_LIST_FOREACH(lists, l, gl)
     {
        _device_add(gl, o);
     }
   _devices_eval();
}

void
ebluez5_popup_device_del(Obj *o)
{
   Eina_List *l;
   Evas_Object *gl;
   Elm_Object_Item *it;

   EINA_LIST_FOREACH(lists, l, gl)
     {
        for (it = elm_genlist_first_item_get(gl); it;
             it = elm_genlist_item_next_get(it))
          {
             if (o == elm_object_item_data_get(it))
               {
                  elm_object_item_del(it);
                  break;
               }
          }
     }
   devices = eina_list_remove(devices, o);
   _devices_eval();
}

void
ebluez5_popup_device_change(Obj *o)
{
   Eina_List *l;
   Evas_Object *gl;
   Elm_Object_Item *it;
   Eina_Bool alert = EINA_FALSE;

   if (o->agent_alert)
     {
        alert = EINA_TRUE;
        o->agent_alert = EINA_FALSE;
        if (!lists) ebluez5_popups_show();
     }
   EINA_LIST_FOREACH(lists, l, gl)
     {
        for (it = elm_genlist_first_item_get(gl); it;
             it = elm_genlist_item_next_get(it))
          {
             if (o == elm_object_item_data_get(it))
               {
                  // if this is a change for the obj for an "initial"
                  // request to the user for ok/pin etc. then change it
                  // and do the updates
                  if (alert)
                    {
                       elm_genlist_item_update(it);
                       elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
                    }
                  // otherwise only do changes to items with no agent
                  // requests pending so we dont delete focused widgets
                  // or entry widgets etc.
                  else if (!o->agent_request)
                    elm_genlist_item_update(it);
                  break;
               }
          }
     }
   _devices_eval();
}

const Eina_List *
ebluez5_popup_adapters_get(void)
{
   return (const Eina_List *)adapters;
}
