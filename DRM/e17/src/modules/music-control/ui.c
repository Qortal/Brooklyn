#include "private.h"

extern Player music_player_players[];

static void
_pos_update(E_Music_Control_Instance *inst)
{
   Edje_Message_Float_Set *msg;

   if (!inst->popup) return;
   msg = alloca(sizeof(Edje_Message_Float_Set) + (2 * sizeof(double)));
   msg->count = 2;
   msg->val[0] = inst->ctxt->position;
   msg->val[1] = inst->ctxt->meta_length;
   if (msg->val[0] < 0.0) msg->val[0] = 0.0;
   if (msg->val[0] > msg->val[1]) msg->val[0] = msg->val[1];
   edje_object_message_send(inst->content_popup, EDJE_MESSAGE_FLOAT_SET, 1, msg);
}

static void
_play_state_update(E_Music_Control_Instance *inst, Eina_Bool without_delay)
{
   if (!inst->popup) return;
   if (inst->ctxt->playing)
     {
        edje_object_signal_emit(inst->content_popup, "btn,state,image,pause", "play");
        return;
     }

   if (without_delay)
     edje_object_signal_emit(inst->content_popup, "btn,state,image,play,no_delay", "play");
   else
     edje_object_signal_emit(inst->content_popup, "btn,state,image,play", "play");
   if (inst->ctxt->loop)
     edje_object_signal_emit(inst->content_popup, "loop,state,on", "play");
   else
     edje_object_signal_emit(inst->content_popup, "loop,state,off", "play");
   _pos_update(inst);
}


void
music_control_state_update_all(E_Music_Control_Module_Context *ctxt)
{
   E_Music_Control_Instance *inst;
   Eina_List *list;

   EINA_LIST_FOREACH(ctxt->instances, list, inst)
     {
        _play_state_update(inst, EINA_FALSE);
     }
}

void
music_control_pos_update(E_Music_Control_Module_Context *ctxt)
{
   E_Music_Control_Instance *inst;
   Eina_List *list;

   EINA_LIST_FOREACH(ctxt->instances, list, inst)
     {
        _pos_update(inst);
     }
}

static void
_metadata_update(E_Music_Control_Instance *inst)
{
   Eina_Strbuf *str;
   Evas_Object *img;

   if (!inst->popup) return;

   str = eina_strbuf_new();

   if (inst->ctxt->meta_title)
     eina_strbuf_append_printf(str, "<title>%s</><ps/>", inst->ctxt->meta_title);
   if (inst->ctxt->meta_artist)
     eina_strbuf_append_printf(str, "<tag>by</> %s<ps/>", inst->ctxt->meta_artist);
   if (inst->ctxt->meta_album)
     eina_strbuf_append_printf(str, "<tag>from</> %s<ps/>", inst->ctxt->meta_album);
   edje_object_part_text_set(inst->content_popup, "metadata", eina_strbuf_string_get(str));
   eina_strbuf_free(str);

   img = edje_object_part_swallow_get(inst->content_popup, "cover_swallow");
   if (img)
     {
        if (inst->ctxt->meta_cover_prev != inst->ctxt->meta_cover)
          {
             e_comp_object_util_del_list_remove(inst->popup->comp_object, img);
             evas_object_del(img);
             img = NULL;
             if (inst->ctxt->meta_cover_prev)
               {
                  eina_stringshare_del(inst->ctxt->meta_cover_prev);
                  inst->ctxt->meta_cover_prev = NULL;
               }
          }
     }
   if ((!img) && (inst->ctxt->meta_cover))
     {
        if (inst->ctxt->meta_cover)
          inst->ctxt->meta_cover_prev = eina_stringshare_add(inst->ctxt->meta_cover);

        img = e_icon_add(evas_object_evas_get(inst->content_popup));
        e_icon_preload_set(img, EINA_TRUE);
        e_icon_scale_size_set(img, 512);
        e_icon_scale_up_set(img, EINA_TRUE);
        e_icon_fill_inside_set(img, EINA_FALSE);
        e_icon_file_set(img, inst->ctxt->meta_cover);
        edje_object_part_swallow(inst->content_popup, "cover_swallow", img);
        e_comp_object_util_del_list_append(inst->popup->comp_object, img);
     }
}

void
music_control_metadata_update_all(E_Music_Control_Module_Context *ctxt)
{
   E_Music_Control_Instance *inst;
   Eina_List *list;

   EINA_LIST_FOREACH(ctxt->instances, list, inst)
     _metadata_update(inst);
}

static void
_btn_clicked(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source)
{
   E_Music_Control_Instance *inst = data;
   if (!strcmp(source, "play"))
     media_player2_player_play_pause_call(inst->ctxt->mpris2_player);
   else if (!strcmp(source, "next"))
     media_player2_player_next_call(inst->ctxt->mpris2_player);
   else if (!strcmp(source, "previous"))
    media_player2_player_previous_call(inst->ctxt->mpris2_player);
}

static void
_label_clicked(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Music_Control_Instance *inst = data;
   music_control_popup_del(inst);
   music_control_launch(inst);
   mpris_media_player2_raise_call(inst->ctxt->mrpis2);
}

static void
_player_name_update(E_Music_Control_Instance *inst)
{
   Edje_Message_String msg;
   msg.str = (char *)music_player_players[inst->ctxt->config->player_selected].name;
   edje_object_message_send(inst->content_popup, EDJE_MESSAGE_STRING, 0, &msg);
}

static void
_popup_del_cb(void *obj)
{
   music_control_popup_del(e_object_data_get(obj));
}

static void
_popup_autoclose_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   music_control_popup_del((E_Music_Control_Instance *)data);
}

static void
_popup_new(E_Music_Control_Instance *inst)
{
   Evas_Object *o;
   inst->popup = e_gadcon_popup_new(inst->gcc, 0);

   o = edje_object_add(e_comp->evas);
   e_theme_edje_object_set(o, "base/theme/modules/music-control",
                           "e/modules/music-control/popup");
   edje_object_signal_callback_add(o, "btn,clicked", "*", _btn_clicked, inst);
   edje_object_signal_callback_add(o, "label,clicked", "player_name", _label_clicked, inst);

   e_gadcon_popup_content_set(inst->popup, o);
   inst->content_popup = o;

   _player_name_update(inst);
   _play_state_update(inst, EINA_TRUE);
   _metadata_update(inst);
   _pos_update(inst);
   e_comp_object_util_autoclose(inst->popup->comp_object,
                                _popup_autoclose_cb, NULL, inst);
   e_gadcon_popup_show(inst->popup);
   e_object_data_set(E_OBJECT(inst->popup), inst);
   E_OBJECT_DEL_SET(inst->popup, _popup_del_cb);
}

void
music_control_popup_del(E_Music_Control_Instance *inst)
{
   E_FREE_FUNC(inst->popup, e_object_del);
}

struct _E_Config_Dialog_Data
{
   int index;
   int pause_on_desklock;
};

static Evas_Object *
_cfg_widgets_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob, *oc;
   E_Radio_Group *rg;
   int i;
   E_Music_Control_Instance *inst = cfd->data;
   int player_selected = inst->ctxt->config->player_selected;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, _("Music Player"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   rg = e_widget_radio_group_new(&(cfdata->index));
   for (i = 0; music_player_players[i].dbus_name; i++)
     {
        ob = e_widget_radio_add(evas, music_player_players[i].name, i, rg);
        e_widget_framelist_object_append(of, ob);
        if (i == player_selected)
          e_widget_radio_toggle_set(ob, EINA_TRUE);
     }
   ob = e_widget_label_add(evas, _("* Your player must be configured to export the DBus interface MPRIS2."));
   e_widget_framelist_object_append(of, ob);

   oc = e_widget_check_add(evas, _("Pause music when screen is locked"), &(cfdata->pause_on_desklock));
   e_widget_framelist_object_append(of, oc);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static void *
_cfg_data_create(E_Config_Dialog *cfd)
{
   E_Music_Control_Instance *inst = cfd->data;
   E_Config_Dialog_Data *cfdata = calloc(1, sizeof(E_Config_Dialog_Data));
   cfdata->index = inst->ctxt->config->player_selected;
   cfdata->pause_on_desklock = inst->ctxt->config->pause_on_desklock;

   return cfdata;
}

static void
_cfg_data_free(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   free(cfdata);
}

static int
_cfg_check_changed(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Music_Control_Instance *inst = cfd->data;

   return ((inst->ctxt->config->pause_on_desklock != cfdata->pause_on_desklock) ||
            (inst->ctxt->config->player_selected != cfdata->index));
}

static int
_cfg_data_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   E_Music_Control_Instance *inst = cfd->data;

   if ((inst->ctxt->config->player_selected == cfdata->index) &&
       (inst->ctxt->config->pause_on_desklock == cfdata->pause_on_desklock))
     return 1;

   inst->ctxt->config->player_selected = cfdata->index;
   inst->ctxt->config->pause_on_desklock = cfdata->pause_on_desklock;

   if (inst->ctxt->config->pause_on_desklock)
      desklock_handler = ecore_event_handler_add(E_EVENT_DESKLOCK, _desklock_cb, inst->ctxt);
   else
     E_FREE_FUNC(desklock_handler, ecore_event_handler_del);

   inst->ctxt->playing = EINA_FALSE;
   music_control_dbus_init(inst->ctxt,
          music_player_players[inst->ctxt->config->player_selected].dbus_name);
   return 1;
}

static void
_cb_menu_cfg(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Config_Dialog_View *v;

   v = calloc(1, sizeof(E_Config_Dialog_View));
   v->create_cfdata = _cfg_data_create;
   v->free_cfdata = _cfg_data_free;
   v->basic.create_widgets = _cfg_widgets_create;
   v->basic.apply_cfdata = _cfg_data_apply;
   v->basic.check_changed = _cfg_check_changed;

   e_config_dialog_new(NULL, _("Music control Settings"), "E",
                       "_e_mod_music_config_dialog",
                       NULL, 0, v, data);
}

void
music_control_mouse_down_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   E_Music_Control_Instance *inst = data;
   Evas_Event_Mouse_Down *ev = event;

   if (ev->button == 1)
     {
        music_control_launch(inst);
        if (!inst->popup)
          _popup_new(inst);
        else
          music_control_popup_del(inst);
     }
   else if (ev->button == 3)
     {
        E_Menu *m;
        E_Menu_Item *mi;
        E_Zone *zone = e_zone_current_get();
        int x, y;

        if (inst->popup)
          music_control_popup_del(inst);

        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("Settings"));
        e_util_menu_item_theme_icon_set(mi, "configure");
        e_menu_item_callback_set(mi, _cb_menu_cfg, inst);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse(m, zone, (x + ev->output.x),(y + ev->output.y),
                              1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}
