#include "e.h"
#include "e_mod_main.h"
#include "e_mod_parse.h"

struct _E_Config_Dialog_Data
{
   Evas        *evas, *dlg_evas;
   Evas_Object *layout_list, *used_list;
   Evas_Object *dmodel_list, *model_list, *variant_list;
   Evas_Object *btn_add, *btn_del, *btn_up, *btn_down;
   Ecore_Timer *fill_delay;
   Ecore_Timer *dlg_fill_delay;
   Evas_Object *popup;

   Evas_Object *btn_layout;
   Evas_Object *led_list;
   Evas_Object *switch_list;
   Evas_Object *ctrl_list;
   Evas_Object *lv3_list;
   Evas_Object *keypad_list;
   Evas_Object *delkeypad_list;
   Evas_Object *capslock_list;
   Evas_Object *altwin_list;
   Evas_Object *compose_list;
   Evas_Object *currency_list;
   Evas_Object *lv5_list;
   Evas_Object *spacebar_list;
   Evas_Object *japan_list;
   Evas_Object *korean_list;
   Evas_Object *esperanto_list;
   Evas_Object *solaris_list;
   Evas_Object *terminate_list;
   Evas_Object *misc_list;

   Evas_Object *chk_label;

   Eina_List   *cfg_layouts;

   Eina_List   *cfg_led_options;
   Eina_List   *cfg_switch_options;
   Eina_List   *cfg_lv3_options;
   Eina_List   *cfg_ctrl_options;
   Eina_List   *cfg_keypad_options;
   Eina_List   *cfg_delkeypad_options;
   Eina_List   *cfg_capslock_options;
   Eina_List   *cfg_altwin_options;
   Eina_List   *cfg_compose_options;
   Eina_List   *cfg_currency_options;
   Eina_List   *cfg_lv5_options;
   Eina_List   *cfg_spacebar_options;
   Eina_List   *cfg_japan_options;
   Eina_List   *cfg_korean_options;
   Eina_List   *cfg_esperanto_options;
   Eina_List   *cfg_solaris_options;
   Eina_List   *cfg_terminate_options;
   Eina_List   *cfg_misc_options;

   const char  *default_model;

   int          only_label;
   int          dont_touch_my_damn_keyboard;

   E_Dialog *dlg_add_new;
   E_Config_Dialog *cfd;
};

/* Local prototypes */

static void        *_create_data(E_Config_Dialog *cfd);
static void         _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int          _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static int          _check_changed(E_Config_Dialog_Data *cfdata);


static void         _dont_touch_my_damn_keyboard_changed(void *data, Evas_Object *obj, void *event);
static void         _only_label_changed(void *data, Evas_Object *obj, void *event);

static void         _layout_clicked(void *data, Evas_Object *obj, void *event);
static void         _cb_add(void *data, Evas_Object *obj, void *event);
static void         _cb_del(void *data, Evas_Object *obj, void *event);

static void         _cb_up(void *data, Evas_Object *obj, void *event);
static void         _cb_dn(void *data, Evas_Object *obj, void *event);

static void         _cb_led_up(void *data, Evas_Object *obj, void *event);
static void         _cb_ctrl_up(void *data, Evas_Object *obj, void *event);
static void         _cb_compose_up(void *data, Evas_Object *obj, void *event);
static void         _cb_lv3_up(void *data, Evas_Object *obj, void *event);
static void         _cb_switch_up(void *data, Evas_Object *obj, void *event);
static void         _cb_keypad_up(void *data, Evas_Object *obj, void *event);
static void         _cb_delkeypad_up(void *data, Evas_Object *obj, void *event);
static void         _cb_capslock_up(void *data, Evas_Object *obj, void *event);
static void         _cb_altwin_up(void *data, Evas_Object *obj, void *event);
static void         _cb_currency_up(void *data, Evas_Object *obj, void *event);
static void         _cb_lv5_up(void *data, Evas_Object *obj, void *event);
static void         _cb_spacebar_up(void *data, Evas_Object *obj, void *event);
static void         _cb_japan_up(void *data, Evas_Object *obj, void *event);
static void         _cb_korean_up(void *data, Evas_Object *obj, void *event);
static void         _cb_esperanto_up(void *data, Evas_Object *obj, void *event);
static void         _cb_solaris_up(void *data, Evas_Object *obj, void *event);
static void         _cb_terminate_up(void *data, Evas_Object *obj, void *event);
static void         _cb_misc_up(void *data, Evas_Object *obj, void *event);

static void         _popup_cancel_clicked(void *data, Evas_Object *obj, void *event_info);

static void         _dlg_add_cb_ok(void *data, E_Dialog *dlg);
static void         _dlg_add_cb_cancel(void *data EINA_UNUSED, E_Dialog *dlg);

static E_Dialog  *_dlg_add_new(E_Config_Dialog_Data *cfdata);
static void         _dlg_add_cb_del(void *object);

static Eina_Bool    _cb_dlg_fill_delay(void *data);

static void         _cb_layout_select(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void         _cb_used_select(void *data, Evas_Object *obj, void *event);

static Eina_Bool    _cb_fill_delay(void *data);

/* Externals */

E_Config_Dialog *
_xkb_cfg_dialog(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   if (e_config_dialog_find("E", "keyboard_and_mouse/xkbswitch"))
     return NULL;
   if (!(v = E_NEW(E_Config_Dialog_View, 1))) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;
   v->advanced.create_widgets = _advanced_create;
   v->advanced.apply_cfdata = _basic_apply;

   cfd = e_config_dialog_new(NULL, _("Keyboard Settings"), "E",
                             "keyboard_and_mouse/xkbswitch",
                             "preferences-desktop-keyboard",
                             0, v, NULL);
   _xkb.cfd = cfd;
   return cfd;
}

/* Locals */

static Eina_Bool
_fill_data(E_XKB_Option *op, const char *name, int size, Eina_List *check, Eina_List **add)
{
   E_XKB_Option *op2;
   Eina_List *l;

   if (!strncmp(op->name, name, size))
     {
        EINA_LIST_FOREACH(check, l, op2)
          {
             if (op->name == op2->name)
               {
                  *add = eina_list_append(*add, op2);
                  return EINA_TRUE;
               }
          }
     }
   return EINA_FALSE;
}

static void *
_create_data(E_Config_Dialog *cfd)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   E_Config_XKB_Layout *cl, *nl;
   E_XKB_Option *op;

   find_rules();
   parse_rules(); /* XXX: handle in case nothing was found? */

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   cfdata->cfd = cfd;

   cfdata->cfg_layouts = NULL;
   EINA_LIST_FOREACH(e_config->xkb.used_layouts, l, cl)
     {
        nl = E_NEW(E_Config_XKB_Layout, 1);
        nl->name = eina_stringshare_add(cl->name);
        nl->model = eina_stringshare_add(cl->model);
        nl->variant = eina_stringshare_add(cl->variant);

        cfdata->cfg_layouts = eina_list_append(cfdata->cfg_layouts, nl);
     }

   /* Initialize options */
   cfdata->default_model = eina_stringshare_add(e_config->xkb.default_model);

   cfdata->only_label = e_config->xkb.only_label;
   cfdata->dont_touch_my_damn_keyboard = e_config->xkb.dont_touch_my_damn_keyboard;

#undef FILL_DATA
#define FILL_DATA(name, list_name) \
   if (_fill_data(op, name, (sizeof(name) - 1),\
                  opt ## list_name, \
                  &cfdata->cfg_ ## list_name ## _options)) continue

   EINA_LIST_FOREACH(e_config->xkb.used_options, l, op)
     {
        FILL_DATA("grp_led", led);
        FILL_DATA("grp", switch);
        FILL_DATA("lv3", lv3);
        FILL_DATA("ctrl", ctrl);
        FILL_DATA("keypad", keypad);
        FILL_DATA("kpdl", delkeypad);
        FILL_DATA("caps", capslock);
        FILL_DATA("altwin", altwin);
        FILL_DATA("compose", compose);
        FILL_DATA("eurosign", currency);
        FILL_DATA("rupeesign", currency);
        FILL_DATA("lv5", lv5);
        FILL_DATA("nbsp", spacebar);
        FILL_DATA("japan", japan);
        FILL_DATA("korean", korean);
        FILL_DATA("esperanto", esperanto);
        FILL_DATA("solaris", solaris);
        FILL_DATA("terminate", terminate);
     }

   return cfdata;
}

static void
_list_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object **o;

   o = data;

   *o = NULL;
}

static void
_free_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   E_Config_XKB_Layout *cl;

   _xkb.cfd = NULL;

   if (cfdata->compose_list)
     evas_object_event_callback_del(cfdata->compose_list, EVAS_CALLBACK_DEL, _list_del);
   if (cfdata->lv3_list)
     evas_object_event_callback_del(cfdata->lv3_list, EVAS_CALLBACK_DEL, _list_del);
   if (cfdata->switch_list)
     evas_object_event_callback_del(cfdata->switch_list, EVAS_CALLBACK_DEL, _list_del);

   EINA_LIST_FREE(cfdata->cfg_layouts, cl)
     {
        eina_stringshare_del(cl->name);
        eina_stringshare_del(cl->model);
        eina_stringshare_del(cl->variant);
        E_FREE(cl);
     }

   eina_stringshare_del(cfdata->default_model);
   E_FREE(cfdata);
   clear_rules();
}

static int
_check_changed(E_Config_Dialog_Data *cfdata)
{
   Eina_List *l, *l2;
   Eina_List *list_option_found = NULL;
   E_Config_XKB_Layout *cl, *nl;
   E_Config_XKB_Option *od, *op;

   if ((eina_list_count(e_config->xkb.used_layouts) !=
        eina_list_count(cfdata->cfg_layouts)) ||
       (e_config->xkb.default_model != cfdata->default_model) ||
       (e_config->xkb.only_label != cfdata->only_label) ||
       (e_config->xkb.dont_touch_my_damn_keyboard != cfdata->dont_touch_my_damn_keyboard))
     return 1;

   l2 = cfdata->cfg_layouts;
   EINA_LIST_FOREACH(e_config->xkb.used_layouts, l, cl)
     {
        nl = eina_list_data_get(l2);
        if ((cl->name != nl->name) ||
            (cl->model != nl->model) ||
            (cl->variant != nl->variant))
          return 1;
        l2 = eina_list_next(l2);
     }

#undef CHECK_OPTION_AND_ADD
#define CHECK_OPTION_AND_ADD(list) \
   do { \
        EINA_LIST_FOREACH(list, l, od) \
          { \
             if (op->name == od->name) \
               { \
                  list_option_found = eina_list_append(list_option_found, op); \
                  break; \
               } \
          } \
   } \
   while(0); \
   if (l) continue \

   EINA_LIST_FOREACH(e_config->xkb.used_options, l, op)
     {
        CHECK_OPTION_AND_ADD(cfdata->cfg_compose_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_lv3_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_switch_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_led_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_ctrl_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_keypad_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_delkeypad_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_capslock_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_altwin_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_currency_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_lv5_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_spacebar_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_japan_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_korean_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_esperanto_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_solaris_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_terminate_options);
        CHECK_OPTION_AND_ADD(cfdata->cfg_misc_options);
     }
   /* If user have deleted an option */
   if (eina_list_count(e_config->xkb.used_options) > eina_list_count(list_option_found))
     {
        eina_list_free(list_option_found);
        return 1;
     }
   /* If user have added an option */
   eina_list_free(list_option_found);
   if ((eina_list_count(e_config->xkb.used_options))
       < (eina_list_count(cfdata->cfg_compose_options)
          + eina_list_count(cfdata->cfg_lv3_options)
          + eina_list_count(cfdata->cfg_switch_options)
          + eina_list_count(cfdata->cfg_led_options)
          + eina_list_count(cfdata->cfg_ctrl_options)
          + eina_list_count(cfdata->cfg_keypad_options)
          + eina_list_count(cfdata->cfg_delkeypad_options)
          + eina_list_count(cfdata->cfg_capslock_options)
          + eina_list_count(cfdata->cfg_altwin_options)
          + eina_list_count(cfdata->cfg_currency_options)
          + eina_list_count(cfdata->cfg_lv5_options)
          + eina_list_count(cfdata->cfg_spacebar_options)
          + eina_list_count(cfdata->cfg_japan_options)
          + eina_list_count(cfdata->cfg_korean_options)
          + eina_list_count(cfdata->cfg_esperanto_options)
          + eina_list_count(cfdata->cfg_solaris_options)
          + eina_list_count(cfdata->cfg_terminate_options)
          + eina_list_count(cfdata->cfg_misc_options)))
     return 1;

   return 0;
}

static int
_basic_apply(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   Eina_List *l;
   E_Config_XKB_Layout *cl, *nl;
   E_Config_XKB_Option *oc, *op;
   Eina_Bool cur_ok = EINA_FALSE, sel_ok = EINA_FALSE;

   EINA_LIST_FREE(e_config->xkb.used_layouts, cl)
     {
        eina_stringshare_del(cl->name);
        eina_stringshare_del(cl->model);
        eina_stringshare_del(cl->variant);
        E_FREE(cl);
     }

   EINA_LIST_FOREACH(cfdata->cfg_layouts, l, cl)
     {
        nl = E_NEW(E_Config_XKB_Layout, 1);
        nl->name = eina_stringshare_ref(cl->name);
        nl->model = eina_stringshare_ref(cl->model);
        nl->variant = eina_stringshare_ref(cl->variant);

        e_config->xkb.used_layouts =
           eina_list_append(e_config->xkb.used_layouts, nl);
        if (e_config_xkb_layout_eq(e_config->xkb.current_layout, nl))
          cur_ok = EINA_TRUE;
        if (e_config_xkb_layout_eq(e_config->xkb.sel_layout, nl))
          sel_ok = EINA_TRUE;
     }
   if (!cur_ok)
     {
        E_FREE_FUNC(e_config->xkb.current_layout, e_config_xkb_layout_free);
        EINA_LIST_FOREACH(e_config->xkb.used_layouts, l, cl)
           if (e_config->xkb.cur_layout == cl->name)
             {
                e_config->xkb.current_layout = e_config_xkb_layout_dup(cl);
                break;
             }
        if (!e_config->xkb.current_layout)
          eina_stringshare_replace(&e_config->xkb.cur_layout, NULL);
     }
   if (!sel_ok)
     {
        E_FREE_FUNC(e_config->xkb.sel_layout, e_config_xkb_layout_free);
        EINA_LIST_FOREACH(e_config->xkb.used_layouts, l, cl)
           if (e_config->xkb.selected_layout == cl->name)
             {
                e_config->xkb.sel_layout = e_config_xkb_layout_dup(cl);
                break;
             }
        if (!e_config->xkb.sel_layout)
          eina_stringshare_replace(&e_config->xkb.selected_layout, NULL);
     }

   eina_stringshare_replace(&e_config->xkb.default_model, cfdata->default_model);

   /* Save options */
   e_config->xkb.only_label = cfdata->only_label;
   e_config->xkb.dont_touch_my_damn_keyboard = cfdata->dont_touch_my_damn_keyboard;

   EINA_LIST_FREE(e_config->xkb.used_options, oc)
     {
        eina_stringshare_del(oc->name);
        E_FREE(oc);
     }
#undef FILL_CONFIG
#define FILL_CONFIG(list) \
   do { \
        EINA_LIST_FOREACH(list, l, op) \
          { \
             oc = E_NEW(E_Config_XKB_Option, 1); \
             oc->name = eina_stringshare_ref(op->name); \
             e_config->xkb.used_options = eina_list_append(e_config->xkb.used_options, oc); \
          } \
   } while(0)

   FILL_CONFIG(cfdata->cfg_compose_options);
   FILL_CONFIG(cfdata->cfg_lv3_options);
   FILL_CONFIG(cfdata->cfg_switch_options);
   FILL_CONFIG(cfdata->cfg_led_options);
   FILL_CONFIG(cfdata->cfg_ctrl_options);
   FILL_CONFIG(cfdata->cfg_keypad_options);
   FILL_CONFIG(cfdata->cfg_delkeypad_options);
   FILL_CONFIG(cfdata->cfg_capslock_options);
   FILL_CONFIG(cfdata->cfg_altwin_options);
   FILL_CONFIG(cfdata->cfg_currency_options);
   FILL_CONFIG(cfdata->cfg_lv5_options);
   FILL_CONFIG(cfdata->cfg_spacebar_options);
   FILL_CONFIG(cfdata->cfg_japan_options);
   FILL_CONFIG(cfdata->cfg_korean_options);
   FILL_CONFIG(cfdata->cfg_esperanto_options);
   FILL_CONFIG(cfdata->cfg_solaris_options);
   FILL_CONFIG(cfdata->cfg_terminate_options);
   FILL_CONFIG(cfdata->cfg_misc_options);

   e_xkb_reconfig();
   e_config_save_queue();
   return 1;
}

static void
_option_del(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Eina_List **list;
   Eina_List *l;
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   l = evas_object_data_get(obj, "list_option");
   list = evas_object_data_get(obj, "list");
   (*list) = eina_list_remove_list((*list), l);
   evas_object_del(obj);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));
}

static void
_basic_create_fill(E_Config_Dialog_Data *cfdata)
{
   E_XKB_Option *option;
   Eina_List *l;
   Evas_Object *o;
   E_Config_XKB_Layout *cl;
   Elm_Object_Item *it, *sel = NULL;

   elm_object_disabled_set(cfdata->chk_label,
                           cfdata->dont_touch_my_damn_keyboard);
   elm_object_disabled_set(cfdata->used_list,
                           cfdata->dont_touch_my_damn_keyboard);
   if (cfdata->dont_touch_my_damn_keyboard)
     elm_list_select_mode_set(cfdata->used_list, ELM_OBJECT_SELECT_MODE_NONE);
   else
     elm_list_select_mode_set(cfdata->used_list, ELM_OBJECT_SELECT_MODE_DEFAULT);
   elm_object_disabled_set(cfdata->btn_add,
                           cfdata->dont_touch_my_damn_keyboard);
   elm_object_disabled_set(cfdata->btn_del,
                           cfdata->dont_touch_my_damn_keyboard);
   elm_object_disabled_set(cfdata->btn_up,
                           cfdata->dont_touch_my_damn_keyboard);
   elm_object_disabled_set(cfdata->btn_down,
                           cfdata->dont_touch_my_damn_keyboard);
   elm_object_disabled_set(cfdata->btn_layout,
                           cfdata->dont_touch_my_damn_keyboard);
   if (cfdata->default_model)
     elm_object_text_set(cfdata->btn_layout, cfdata->default_model);
   else
     elm_object_text_set(cfdata->btn_layout, "default");

   /* Update the list of used layouts */
   elm_list_clear(cfdata->used_list);
   if (!cfdata->dont_touch_my_damn_keyboard)
     {
        EINA_LIST_FOREACH(cfdata->cfg_layouts, l, cl)
          {
             Evas_Object *ic = elm_icon_add(cfdata->used_list);
             const char *name = cl->name;
             char buf[PATH_MAX];

             e_xkb_flag_file_get(buf, sizeof(buf), name);
             elm_image_file_set(ic, buf, NULL);
             snprintf(buf, sizeof(buf), "%s%s%s%s%s%s",
                      cl->name ? cl->name : _("No Name"),
                      cl->model || cl->variant ? " (" : "",
                      cl->model ? cl->model : "",
                      cl->model && cl->variant ? ", " : "",
                      cl->variant ? cl->variant : "",
                      cl->model || cl->variant ? ")" : ""
                     );
             evas_object_show(ic);
             it = elm_list_item_append(cfdata->used_list, buf, ic,
                                       NULL, NULL, cl);
             if (!l->prev)
               sel = it;
          }
        if (sel)
          elm_list_item_selected_set(sel, EINA_TRUE);
        elm_list_go(cfdata->used_list);
     }


#undef FILL_GUI
#define FILL_GUI(list, box, cb) \
   do { \
        if (!box) break; \
        elm_box_clear(box); \
        if (cfdata->dont_touch_my_damn_keyboard) break; \
        EINA_LIST_FOREACH(list, l, option) \
          { \
             o = elm_button_add(box); \
             elm_object_text_set(o, option->description); \
             evas_object_data_set(o, "list_option", l); \
             evas_object_data_set(o, "list", &list); \
             evas_object_smart_callback_add(o, "clicked", _option_del, cfdata); \
             elm_box_pack_end(box, o); \
             evas_object_show(o); \
          } \
        o = elm_button_add(box); \
        elm_object_text_set(o, "+"); \
        evas_object_smart_callback_add(o, "clicked", cb, cfdata); \
        elm_box_pack_end(box, o); \
        evas_object_show(o); \
   } while (0)

   FILL_GUI(cfdata->cfg_compose_options, cfdata->compose_list, _cb_compose_up);
   FILL_GUI(cfdata->cfg_lv3_options, cfdata->lv3_list, _cb_lv3_up);
   FILL_GUI(cfdata->cfg_switch_options, cfdata->switch_list, _cb_switch_up);
   FILL_GUI(cfdata->cfg_led_options, cfdata->led_list, _cb_led_up);
   FILL_GUI(cfdata->cfg_ctrl_options, cfdata->ctrl_list, _cb_ctrl_up);
   FILL_GUI(cfdata->cfg_keypad_options, cfdata->keypad_list, _cb_keypad_up);
   FILL_GUI(cfdata->cfg_delkeypad_options, cfdata->delkeypad_list,
            _cb_delkeypad_up);
   FILL_GUI(cfdata->cfg_capslock_options, cfdata->capslock_list,
            _cb_capslock_up);
   FILL_GUI(cfdata->cfg_altwin_options, cfdata->altwin_list, _cb_altwin_up);
   FILL_GUI(cfdata->cfg_currency_options, cfdata->currency_list,
            _cb_currency_up);
   FILL_GUI(cfdata->cfg_lv5_options, cfdata->lv5_list, _cb_lv5_up);
   FILL_GUI(cfdata->cfg_spacebar_options, cfdata->spacebar_list,
            _cb_spacebar_up);
   FILL_GUI(cfdata->cfg_japan_options, cfdata->japan_list, _cb_japan_up);
   FILL_GUI(cfdata->cfg_korean_options, cfdata->korean_list, _cb_korean_up);
   FILL_GUI(cfdata->cfg_esperanto_options, cfdata->esperanto_list,
            _cb_esperanto_up);
   FILL_GUI(cfdata->cfg_solaris_options, cfdata->solaris_list, _cb_solaris_up);
   FILL_GUI(cfdata->cfg_terminate_options,
            cfdata->terminate_list, _cb_terminate_up);
   FILL_GUI(cfdata->cfg_misc_options, cfdata->misc_list, _cb_misc_up);
}

static Evas_Object *
_config_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *mainn, *configs, *buttons, *only_label,
               *dont_touch_my_damn_keyboard;
   Evas_Object *listh, *frame;
   Evas_Object *o;

   /* Holds the dialog contents, displays a toolbar on the top */
   e_dialog_resizable_set(cfd->dia, 1);

   /* The main evas */
   cfdata->evas = evas;

   mainn = elm_box_add(cfd->dia->win);
   elm_box_horizontal_set(mainn, EINA_FALSE);
   evas_object_size_hint_weight_set(mainn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   dont_touch_my_damn_keyboard = elm_check_add(mainn);
   evas_object_smart_callback_add(dont_touch_my_damn_keyboard, "changed",
                                  _dont_touch_my_damn_keyboard_changed, cfdata);
   elm_object_text_set(dont_touch_my_damn_keyboard,
                       _("Do not apply any keyboard settings ever"));
   evas_object_show(dont_touch_my_damn_keyboard);
   evas_object_size_hint_align_set(dont_touch_my_damn_keyboard, 0.0, 0.5);
   elm_box_pack_end(mainn, dont_touch_my_damn_keyboard);

   only_label = elm_check_add(mainn);
   evas_object_smart_callback_add(only_label, "changed",
                                  _only_label_changed, cfdata);
   cfdata->chk_label = only_label;
   elm_object_text_set(only_label, _("Label only in gadgets"));
   evas_object_show(only_label);
   evas_object_size_hint_align_set(only_label, 0.0, 0.5);
   elm_box_pack_end(mainn, only_label);

   o = elm_separator_add(mainn);
   elm_separator_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, 0.5);
   evas_object_show(o);
   elm_box_pack_end(mainn, o);

   /* Holds the default layouts */
   listh = elm_box_add(mainn);
   elm_box_horizontal_set(listh, EINA_TRUE);
   elm_box_homogeneous_set(listh, EINA_TRUE);
   evas_object_size_hint_align_set(listh, EVAS_HINT_FILL, 0.5);
   o = elm_label_add(listh);
   elm_object_text_set(o, _("Default keyboard layout"));
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(listh, o);
   evas_object_show(o);
   o = elm_button_add(listh);
   cfdata->btn_layout = o;
   evas_object_smart_callback_add(o, "clicked", _layout_clicked, cfdata);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(o);
   elm_box_pack_end(listh, o);
   evas_object_show(listh);
   elm_box_pack_end(mainn, listh);

   /* Holds the used layouts */
   listh = elm_box_add(mainn);
   elm_box_horizontal_set(listh, EINA_TRUE);
   o = evas_object_rectangle_add(listh);
   evas_object_size_hint_min_set(o, 0, 160);
   elm_box_pack_end(listh, o);
   configs = elm_list_add(listh);
   evas_object_size_hint_align_set(configs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(configs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(listh, configs);
   elm_box_pack_end(mainn, listh);
   evas_object_show(configs);
   evas_object_size_hint_align_set(listh, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(listh, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(listh);
   evas_object_smart_callback_add(configs, "selected", _cb_used_select, cfdata);
   cfdata->used_list = configs;

   /* Holds the buttons */
   buttons = elm_table_add(mainn);
   elm_table_homogeneous_set(buttons, EINA_TRUE);

   o = elm_button_add(buttons);
   cfdata->btn_add = o;
   elm_object_text_set(o, _("Add"));
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_add, cfdata);
   evas_object_show(o);
   elm_table_pack(buttons, o, 0, 0, 1, 1);

   o = elm_button_add(buttons);
   cfdata->btn_del = o;
   elm_object_text_set(o, _("Del"));
   elm_object_disabled_set(o, EINA_TRUE);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_del, cfdata);
   evas_object_show(o);
   elm_table_pack(buttons, o, 0, 1, 1, 1);

   o = elm_button_add(buttons);
   cfdata->btn_up = o;
   elm_object_text_set(o, _("Up"));
   elm_object_disabled_set(o, EINA_TRUE);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_up, cfdata);
   evas_object_show(o);
   elm_table_pack(buttons, o, 1, 0, 1, 1);

   o = elm_button_add(buttons);
   cfdata->btn_down = o;
   elm_object_text_set(o, _("Down"));
   elm_object_disabled_set(o, EINA_TRUE);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(o, "clicked", _cb_dn, cfdata);
   evas_object_show(o);
   elm_table_pack(buttons, o, 1, 1, 1, 1);

   evas_object_size_hint_fill_set(buttons, EVAS_HINT_FILL, 0.5);
   evas_object_show(buttons);
   elm_box_pack_end(mainn, buttons);

   elm_box_pack_end(mainn, buttons);
   evas_object_show(buttons);

   o = elm_separator_add(mainn);
   elm_separator_horizontal_set(o, EINA_TRUE);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, 0.5);
   evas_object_show(o);
   elm_box_pack_end(mainn, o);

#define FRAME_ADD(name, list) \
   do { \
        frame = elm_frame_add(mainn); \
        elm_object_text_set(frame, name); \
        listh = elm_box_add(frame); \
        elm_object_content_set(frame, listh); \
        elm_box_layout_set(listh, evas_object_box_layout_flow_horizontal, \
                           NULL, NULL); \
        evas_object_size_hint_weight_set(listh, EVAS_HINT_EXPAND, 0.0); \
        elm_box_pack_end(mainn, frame); \
        evas_object_size_hint_fill_set(frame, EVAS_HINT_FILL, 0.5); \
        evas_object_show(frame); \
        evas_object_size_hint_weight_set(listh, EVAS_HINT_EXPAND, 0.0); \
        evas_object_size_hint_align_set(listh, EVAS_HINT_FILL, 0.5); \
        elm_box_align_set(listh, 0.0, 0.5); \
        evas_object_event_callback_add(listh, EVAS_CALLBACK_DEL, \
                                       _list_del, &list); \
        list = listh; \
   } while (0)


   FRAME_ADD(_("Compose"), cfdata->compose_list);
   FRAME_ADD(_("Third level"), cfdata->lv3_list);
   FRAME_ADD(_("Switch layout"), cfdata->switch_list);

   return mainn;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *mainn;

   mainn = _config_basic_create(cfd, evas, cfdata);

   /* Clear up any previous timer */
   if (cfdata->fill_delay)
     ecore_timer_del(cfdata->fill_delay);

   /* Trigger the fill */
   cfdata->fill_delay = ecore_timer_loop_add(0.2, _cb_fill_delay, cfdata);

   return mainn;
}


static Evas_Object *
_advanced_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *mainn;
   Evas_Object *frame, *listh;
   Evas_Object *sc;

   sc = elm_scroller_add(cfd->dia->win);

   mainn = _config_basic_create(cfd, evas, cfdata);

   FRAME_ADD(_("Led"), cfdata->led_list);
   FRAME_ADD(_("Control"), cfdata->ctrl_list);
   FRAME_ADD(_("Keypad"), cfdata->keypad_list);
   FRAME_ADD(_("Keypad delete key"), cfdata->delkeypad_list);
   FRAME_ADD(_("Capslock"), cfdata->capslock_list);
   FRAME_ADD(_("Alt win"), cfdata->altwin_list);
   FRAME_ADD(_("Currency"), cfdata->currency_list);
   FRAME_ADD(_("Fifth level"), cfdata->lv5_list);
   FRAME_ADD(_("Spacebar"), cfdata->spacebar_list);
   FRAME_ADD(_("Japan"), cfdata->japan_list);
   FRAME_ADD(_("Korean"), cfdata->korean_list);
   FRAME_ADD(_("Esperanto"), cfdata->esperanto_list);
   FRAME_ADD(_("Solaris"), cfdata->solaris_list);
   FRAME_ADD(_("Terminate X"), cfdata->terminate_list);
   FRAME_ADD(_("Miscellaneous"), cfdata->misc_list);

   elm_object_content_set(sc, mainn);

   /* Clear up any previous timer */
   if (cfdata->fill_delay)
     ecore_timer_del(cfdata->fill_delay);

   /* Trigger the fill */
   cfdata->fill_delay = ecore_timer_loop_add(0.2, _cb_fill_delay, cfdata);

   return sc;
}

static void
_model_item_clicked(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   E_XKB_Model *model;
   E_Config_Dialog_Data *cfdata;

   if (!(model = data)) return;

   cfdata = evas_object_data_get(obj, "cfdata");

   elm_object_text_set(cfdata->btn_layout, model->name);
   eina_stringshare_replace(&cfdata->default_model, model->name);
   evas_object_del(cfdata->popup);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));
}

static void
_layout_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *popup, *fr, *vbx, *bx, *list, *o;
   E_XKB_Model *model;
   E_Config_Dialog_Data *cfdata;
   Elm_Object_Item *it, *sel = NULL;
   Eina_List *l;

   if (!(cfdata = data)) return;

   popup = elm_popup_add(cfdata->cfd->dia->win);
   elm_popup_allow_events_set(popup, EINA_FALSE);

   fr = elm_frame_add(popup);
   elm_object_text_set(fr, _("Default keyboard layout"));
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(popup, fr);
   vbx = elm_box_add(fr);
   elm_box_horizontal_set(vbx, EINA_FALSE);
   elm_object_content_set(fr, vbx);

   bx = elm_box_add(vbx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(vbx, bx);

   o = evas_object_rectangle_add(evas_object_evas_get(fr));
   evas_object_size_hint_min_set(o, 0, 240);
   elm_box_pack_end(bx, o);

   list = elm_list_add(fr);
   elm_list_mode_set(list, ELM_LIST_COMPRESS);
   elm_box_pack_end(bx, list);
   evas_object_data_set(list, "cfdata", cfdata);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   EINA_LIST_FOREACH(models, l, model)
     {
        it = elm_list_item_append(list, model->name, NULL, NULL,
                                  _model_item_clicked, model);
        if ((model->name == cfdata->default_model)
            || ((!cfdata->default_model) && (!strcmp(model->name, "default"))))
          sel = it;
     }
   if (sel)
     elm_list_item_selected_set(sel, EINA_TRUE);
   elm_list_go(list);

   o = elm_button_add(vbx);
   elm_object_text_set(o, _("Cancel"));
   evas_object_smart_callback_add(o, "clicked", _popup_cancel_clicked, popup);
   evas_object_show(o);
   elm_box_pack_end(vbx, o);

   evas_object_show(fr);
   evas_object_show(vbx);
   evas_object_show(bx);
   evas_object_show(list);
   evas_object_show(popup);
   cfdata->popup = popup;
}

static void
_cb_add(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   if (!(cfdata = data)) return;

   if (cfdata->dlg_add_new) elm_win_raise(cfdata->dlg_add_new->win);
   else cfdata->dlg_add_new = _dlg_add_new(cfdata);
}

static void
_cb_del(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   Elm_Object_Item *it;
   E_Config_XKB_Layout *cl;

   if (!(cfdata = data)) return;

   it = elm_list_selected_item_get(cfdata->used_list);
   if (!it) return;
   cl = elm_object_item_data_get(it);
   elm_object_item_del(it);
   if (!cl) return;
   cfdata->cfg_layouts = eina_list_remove(cfdata->cfg_layouts, cl);
   it = elm_list_first_item_get(cfdata->used_list);
   if (it)
     elm_list_item_selected_set(it, EINA_TRUE);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));
}

static void
_popup_cancel_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_popup_item_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eina_List **cfg_opts;
   E_Config_Dialog_Data *cfdata;

   cfg_opts = evas_object_data_get(obj, "cfg_opts");
   cfdata = evas_object_data_get(obj, "cfdata");

   *cfg_opts = eina_list_append((*cfg_opts), data);
   evas_object_del(cfdata->popup);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));

   _basic_create_fill(cfdata);
}

static Evas_Object *
_popup_item_tooltip(void *data, Evas_Object *obj EINA_UNUSED, Evas_Object *tooltip, void *item EINA_UNUSED)
{
   E_XKB_Option *option;
   Evas_Object *o;

   if (!(option = data)) return NULL;
   o = elm_label_add(tooltip);
   elm_object_text_set(o, option->description);
   elm_label_line_wrap_set(o, ELM_WRAP_WORD);

   return o;
}

static void
_popup_add(const char *title, E_Config_Dialog_Data *cfdata, Eina_List *opts, Eina_List **cfg_opts, Evas_Object *list_objects EINA_UNUSED)
{
   Evas_Object *popup, *fr, *vbx, *bx, *list, *o;
   E_XKB_Option *option, *op;
   Eina_List *l, *ll;
   Eina_Bool found;
   Elm_Object_Item *it;

   popup = elm_popup_add(cfdata->cfd->dia->win);
   elm_popup_allow_events_set(popup, EINA_FALSE);

   fr = elm_frame_add(popup);
   elm_object_text_set(fr, title);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(popup, fr);
   vbx = elm_box_add(fr);
   elm_box_horizontal_set(vbx, EINA_FALSE);
   elm_object_content_set(fr, vbx);

   bx = elm_box_add(vbx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_pack_end(vbx, bx);

   o = evas_object_rectangle_add(evas_object_evas_get(fr));
   evas_object_size_hint_min_set(o, 0, 240);
   elm_box_pack_end(bx, o);

   list = elm_list_add(fr);
   elm_list_mode_set(list, ELM_LIST_COMPRESS);
   elm_box_pack_end(bx, list);
   evas_object_data_set(list, "cfg_opts", cfg_opts);
   evas_object_data_set(list, "cfdata", cfdata);

   EINA_LIST_FOREACH(opts, l, option)
     {
        found = EINA_FALSE;
        EINA_LIST_FOREACH(*cfg_opts, ll, op)
          {
             if (op == option)
               found = EINA_TRUE;
          }
        if (!found)
          {
             it = elm_list_item_append(list, option->description, NULL, NULL,
                                       _popup_item_clicked, option);
             elm_object_item_tooltip_content_cb_set(it, _popup_item_tooltip,
                                                    option, NULL);
          }
     }
   o = elm_button_add(vbx);
   elm_object_text_set(o, _("Cancel"));
   evas_object_smart_callback_add(o, "clicked", _popup_cancel_clicked, popup);
   elm_box_pack_end(vbx, o);
   evas_object_size_hint_weight_set(vbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(vbx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bx);
   evas_object_show(o);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fr);
   evas_object_show(popup);
   cfdata->popup = popup;
}

static void
_cb_compose_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Compose"), cfdata, optcompose,
              &cfdata->cfg_compose_options, cfdata->compose_list);
}

static void
_cb_lv3_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Third level"), cfdata, optlv3,
              &cfdata->cfg_lv3_options, cfdata->lv3_list);
}

static void
_cb_switch_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Switch Layout"), cfdata, optswitch,
              &cfdata->cfg_switch_options, cfdata->switch_list);
}

static void
_cb_led_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Led"), cfdata, optled,
              &cfdata->cfg_led_options, cfdata->led_list);
}


static void
_cb_ctrl_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Switch Layout"), cfdata, optswitch,
              &cfdata->cfg_ctrl_options, cfdata->ctrl_list);
}

static void
_cb_keypad_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Keypad"), cfdata, optkeypad,
              &cfdata->cfg_keypad_options, cfdata->keypad_list);
}


static void
_cb_delkeypad_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Keypad delete key"), cfdata, optdelkeypad,
              &cfdata->cfg_delkeypad_options, cfdata->delkeypad_list);
}

static void
_cb_capslock_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Capslock"), cfdata, optcapslock,
              &cfdata->cfg_capslock_options, cfdata->capslock_list);
}

static void
_cb_altwin_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Alternate win key"), cfdata, optaltwin,
              &cfdata->cfg_altwin_options, cfdata->altwin_list);
}

static void
_cb_currency_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Currency"), cfdata, optcurrency,
              &cfdata->cfg_currency_options, cfdata->currency_list);
}

static void
_cb_lv5_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Fifth level"), cfdata, optlv5,
              &cfdata->cfg_lv5_options, cfdata->lv5_list);
}

static void
_cb_spacebar_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Spacebar"), cfdata, optspacebar,
              &cfdata->cfg_spacebar_options, cfdata->spacebar_list);
}

static void
_cb_japan_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Japan"), cfdata, optjapan,
              &cfdata->cfg_japan_options, cfdata->japan_list);
}

static void
_cb_korean_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Korean"), cfdata, optkorean,
              &cfdata->cfg_korean_options, cfdata->korean_list);
}

static void
_cb_esperanto_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Esperanto"), cfdata, optesperanto,
              &cfdata->cfg_esperanto_options, cfdata->esperanto_list);
}

static void
_cb_solaris_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Solaris"), cfdata, optsolaris,
              &cfdata->cfg_solaris_options, cfdata->solaris_list);
}

static void
_cb_terminate_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Terminate X"), cfdata, optterminate,
              &cfdata->cfg_terminate_options, cfdata->terminate_list);
}

static void
_cb_misc_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   _popup_add(_("Miscellaneous"), cfdata, optmisc,
              &cfdata->cfg_misc_options, cfdata->misc_list);
}


static void
_dont_touch_my_damn_keyboard_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   cfdata->dont_touch_my_damn_keyboard = elm_check_state_get(obj);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));

   _basic_create_fill(cfdata);
}

static void
_only_label_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return;

   cfdata->only_label = elm_check_state_get(obj);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));
}

static void
_cb_up(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   Elm_Object_Item *it, *prev;
   E_Config_XKB_Layout *cl;
   Eina_List *l, *ll;
   void *ndata;

   if (!(cfdata = data)) return;

   it = elm_list_selected_item_get(cfdata->used_list);
   if (!it) return;
   prev = elm_list_item_prev(it);
   if ((!prev) || (prev == it)) return;
   ndata = elm_object_item_data_get(it);
   EINA_LIST_FOREACH(cfdata->cfg_layouts, l, cl)
     {
        if (cl == ndata)
          {
             ll = eina_list_prev(l);
             eina_list_data_set(l, eina_list_data_get(ll));
             eina_list_data_set(ll, ndata);
             break;
          }
     }
   if (!l) return;
   prev = elm_list_item_insert_before(cfdata->used_list, prev,
                                      elm_object_item_text_get(it),
                                      NULL, NULL, NULL,
                                      ndata);
   elm_object_item_del(it);
   elm_list_item_selected_set(prev, EINA_TRUE);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));
}

static void
_cb_dn(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   Elm_Object_Item *it, *next;
   E_Config_XKB_Layout *cl;
   Eina_List *l, *ll;
   void *ndata;

   if (!(cfdata = data)) return;

   it = elm_list_selected_item_get(cfdata->used_list);
   if (!it) return;
   next = elm_list_item_next(it);
   if ((!next) || (next == it)) return;
   ndata = elm_object_item_data_get(it);
   EINA_LIST_FOREACH(cfdata->cfg_layouts, l, cl)
     {
        if (cl == ndata)
          {
             ll = eina_list_next(l);
             if (!ll) return;
             eina_list_data_set(l, eina_list_data_get(ll));
             eina_list_data_set(ll, ndata);
             break;
          }
     }
   if (!l) return;
   next = elm_list_item_insert_after(cfdata->used_list, next,
                                     elm_object_item_text_get(it),
                                     NULL, NULL, NULL,
                                     elm_object_item_data_get(it));
   elm_object_item_del(it);
   elm_list_item_selected_set(next, EINA_TRUE);
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));
}

static void
_show_layout(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   evas_object_hide(cfdata->model_list);
   evas_object_hide(cfdata->variant_list);
   evas_object_show(cfdata->layout_list);
}

static void
_show_model(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   evas_object_hide(cfdata->layout_list);
   evas_object_hide(cfdata->variant_list);
   evas_object_show(cfdata->model_list);
}

static void
_show_variant(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata = data;

   evas_object_hide(cfdata->layout_list);
   evas_object_hide(cfdata->model_list);
   evas_object_show(cfdata->variant_list);
}

static E_Dialog *
_dlg_add_new(E_Config_Dialog_Data *cfdata)
{
   E_Dialog *dlg;
   Evas_Object *mainn, *list, *layout, *modelss, *variants, *tb, *box;
   E_Zone *zone = e_zone_current_get();
   Elm_Object_Item *it;

   if (!(dlg = e_dialog_new(e_win_evas_win_get(cfdata->evas), "E", "xkbswitch_config_add_dialog"))) return NULL;
   e_dialog_resizable_set(dlg, 1);
   dlg->data = cfdata;
   e_dialog_title_set(dlg, _("Add New Configuration"));
   e_dialog_border_icon_set(dlg, "preferences-desktop-keyboard");
   e_object_del_attach_func_set(E_OBJECT(dlg), _dlg_add_cb_del);
   elm_win_center(dlg->win, 1, 1);

   mainn = elm_box_add(dlg->win);
   elm_box_horizontal_set(mainn, EINA_FALSE);
   E_EXPAND(mainn);
   e_dialog_content_set(dlg, mainn, 0, 0);
   evas_object_show(mainn);

   box = elm_box_add(mainn);
   elm_box_horizontal_set(box, EINA_TRUE);
   E_EXPAND(box);
   E_FILL(box);
   elm_box_pack_end(mainn, box);
   evas_object_show(box);

   list = elm_list_add(box);
   E_FILL(list);
   E_WEIGHT(list, 0.0, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, list);
   elm_list_select_mode_set(list, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_scroller_content_min_limit(list, 1, 1);
   it = elm_list_item_append(list, _("Layout"), NULL, NULL,
       _show_layout, cfdata);
   elm_list_item_selected_set(it, EINA_TRUE);
   it = elm_list_item_append(list, _("Model"), NULL, NULL,
       _show_model,  cfdata);
   it = elm_list_item_append(list, _("Variant"), NULL, NULL,
       _show_variant, cfdata);
   elm_list_go(list);
   evas_object_show(list);

   tb = elm_table_add(box);
   E_EXPAND(tb);
   E_FILL(tb);
   elm_box_pack_end(box, tb);
   evas_object_show(tb);

   layout = elm_genlist_add(tb);
   E_EXPAND(layout);
   E_FILL(layout);
   elm_genlist_select_mode_set(layout, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_genlist_mode_set(layout, ELM_LIST_COMPRESS);
   elm_table_pack(tb, layout, 0, 0, 1, 1);
   evas_object_show(layout);
   cfdata->layout_list = layout;

   modelss = elm_genlist_add(tb);
   E_EXPAND(modelss);
   E_FILL(modelss);
   elm_table_pack(tb, modelss, 0, 0, 1, 1);
   elm_genlist_select_mode_set(modelss, ELM_OBJECT_SELECT_MODE_ALWAYS);
   evas_object_show(modelss);
   cfdata->model_list = modelss;

   variants = elm_genlist_add(tb);
   E_EXPAND(variants);
   E_FILL(variants);
   elm_table_pack(tb, variants, 0, 0, 1, 1);
   elm_genlist_select_mode_set(variants, ELM_OBJECT_SELECT_MODE_ALWAYS);
   evas_object_show(variants);
   cfdata->variant_list = variants;

   e_dialog_button_add(dlg, _("OK"), NULL, _dlg_add_cb_ok, cfdata);
   e_dialog_button_add(dlg, _("Cancel"), NULL, _dlg_add_cb_cancel, cfdata);

   e_dialog_button_disable_num_set(dlg, 0, 1);
   e_dialog_button_disable_num_set(dlg, 1, 0);

   cfdata->dlg_evas = evas_object_evas_get(dlg->win);
   evas_object_resize(dlg->win, zone->w / 3, zone->h / 3);
   evas_object_resize(mainn, zone->w / 3, zone->h / 3);
   e_dialog_show(dlg);

   if (cfdata->dlg_fill_delay) ecore_timer_del(cfdata->dlg_fill_delay);
   cfdata->dlg_fill_delay = ecore_timer_loop_add(0.2, _cb_dlg_fill_delay, cfdata);

   _show_layout(cfdata, NULL, NULL);

   return dlg;
}

static void
_dlg_add_cb_ok(void *data, E_Dialog *dlg)
{
   E_Config_Dialog_Data *cfdata = data;
   E_XKB_Layout *l;
   E_XKB_Model *m;
   E_XKB_Variant *v;
   E_Config_XKB_Layout *cl;
   char buf[512], icon_buf[PATH_MAX];
   Evas_Object *ic;
   Elm_Object_Item *it;
   /* Configuration information */
   Eina_Stringshare *layout, *model, *variant;

   it = elm_genlist_selected_item_get(cfdata->layout_list);
   l = elm_object_item_data_get(it);
   it = elm_genlist_selected_item_get(cfdata->model_list);
   m = elm_object_item_data_get(it);
   it = elm_genlist_selected_item_get(cfdata->variant_list);
   v = elm_object_item_data_get(it);

   layout = eina_stringshare_add(l->name);
   model = eina_stringshare_add(m->name);
   variant = eina_stringshare_add(v->name);

   /* The new configuration */
   cl = E_NEW(E_Config_XKB_Layout, 1);
   cl->name = eina_stringshare_ref(layout);
   cl->model = eina_stringshare_ref(model);
   cl->variant = eina_stringshare_ref(variant);

   cfdata->cfg_layouts = eina_list_append(cfdata->cfg_layouts, cl);

   /* Update the main list */
   ic = elm_icon_add(cfdata->used_list);
   e_xkb_flag_file_get(icon_buf, sizeof(icon_buf), cl->name);
   elm_image_file_set(ic, icon_buf, NULL);
   snprintf(buf, sizeof(buf), "%s%s%s%s%s%s",
            cl->name ? cl->name : _("No Name"),
            cl->model || cl->variant ? " (" : "",
            cl->model ? cl->model : "",
            cl->model && cl->variant ? ", " : "",
            cl->variant ? cl->variant : "",
            cl->model || cl->variant ? ")" : ""
           );
   elm_list_item_append(cfdata->used_list, buf, ic, NULL, NULL, cl);
   elm_list_go(cfdata->used_list);

   e_object_del(E_OBJECT(dlg));
   e_config_dialog_changed_set(cfdata->cfd, _check_changed(cfdata));
}

static void
_dlg_add_cb_cancel(void *data EINA_UNUSED, E_Dialog *dlg)
{
   e_object_del(E_OBJECT(dlg));
}

static char *
_layout_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   E_XKB_Layout *layout = data;
   char buf[512];

   snprintf(buf, sizeof(buf), "%s%s%s%s",
            layout->description ? layout->description : _("No Description"),
            layout->name ? " (" : "",
            layout->name ? layout->name : "",
            layout->name ? ")" : "");
   return strdup(buf);
}

Evas_Object *_layout_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   E_XKB_Layout *layout = data;
   Evas_Object *ic;
   char tmp[PATH_MAX];

   if (!strcmp(part, "elm.swallow.end"))
     return NULL;

   ic = elm_icon_add(obj);
   e_xkb_flag_file_get(tmp, sizeof(tmp), layout->name);
   elm_image_file_set(ic, tmp, NULL);

   return ic;
}

Eina_Bool _layout_gl_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_cb_dlg_fill_delay(void *data)
{
   E_Config_Dialog_Data *cfdata;
   Eina_List *l;
   E_XKB_Layout *layout;
   const char *lang;
   E_Locale_Parts *lang_part = NULL;
   Elm_Object_Item *it, *sel = NULL;
   Elm_Genlist_Item_Class *itc;

   if (!(cfdata = data)) return ECORE_CALLBACK_RENEW;

   elm_genlist_clear(cfdata->layout_list);

   lang = e_intl_language_get();
   if (lang)
   {
      lang_part = e_intl_locale_parts_get(lang);
   }

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = _layout_gl_text_get;
   itc->func.content_get = _layout_gl_content_get;
   itc->func.state_get = _layout_gl_state_get;
   itc->func.del = NULL;
   EINA_LIST_FOREACH(layouts, l, layout)
     {
        it = elm_genlist_item_append(cfdata->layout_list, itc, layout, NULL, ELM_GENLIST_ITEM_NONE,
             _cb_layout_select, cfdata);
        if (lang_part)
          {
             if (!strncasecmp(lang_part->region, layout->name, 2))
               sel = it;
          }
     }
   elm_genlist_item_class_free(itc);
   if (lang_part) e_intl_locale_parts_free(lang_part);

   if (sel)
     {
        elm_genlist_item_selected_set(sel, EINA_TRUE);
        elm_genlist_item_show(sel, ELM_GENLIST_ITEM_SCROLLTO_TOP);
        _cb_layout_select(cfdata, NULL, NULL);
     }
   cfdata->dlg_fill_delay = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static char *
_model_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   E_XKB_Model *model = data;
   char buf[512];

   snprintf(buf, sizeof(buf), "%s%s%s%s",
            model->description ? model->description : _("No Description"),
            model->name ? " (" : "",
            model->name ? model->name : "",
            model->name ? ")" : "");
   return strdup(buf);
}

static char *
_variant_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   E_XKB_Variant *variant = data;
   char buf[512];

   snprintf(buf, sizeof(buf), "%s%s%s%s",
            variant->name ? variant->name : _("No Name"),
            variant->description ? " (" : "",
            variant->description ? variant->description : "",
            variant->description ? ")" : "");
   return strdup(buf);
}

Eina_Bool _gl_state_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_cb_layout_select(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   E_XKB_Variant *variant;
   E_XKB_Layout *layout;
   E_XKB_Model *model;
   Eina_List *l;
   Elm_Object_Item *n;
   Elm_Genlist_Item_Class *itc;

   if (!(cfdata = data)) return;

   if (!(n = elm_genlist_selected_item_get(cfdata->layout_list)))
     return;
   if (!(layout = elm_object_item_data_get(n)))
     return;

   elm_genlist_clear(cfdata->model_list);
   elm_genlist_clear(cfdata->variant_list);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = _model_gl_text_get;
   itc->func.content_get = NULL;
   itc->func.state_get = _gl_state_get;
   itc->func.del = NULL;
   EINA_LIST_FOREACH(models, l, model)
     {
        elm_genlist_item_append(cfdata->model_list, itc, model, NULL, ELM_GENLIST_ITEM_NONE,
             NULL, NULL);
     }
   elm_genlist_item_class_free(itc);
   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = _variant_gl_text_get;
   itc->func.content_get = NULL;
   itc->func.state_get = _gl_state_get;
   itc->func.del = NULL;
   EINA_LIST_FOREACH(layout->variants, l, variant)
     {
        elm_genlist_item_append(cfdata->variant_list, itc, variant, NULL, ELM_GENLIST_ITEM_NONE,
             NULL, NULL);
     }
   elm_genlist_item_class_free(itc);
   elm_genlist_item_selected_set(elm_genlist_first_item_get(cfdata->model_list), EINA_TRUE);
   elm_genlist_item_selected_set(elm_genlist_first_item_get(cfdata->variant_list), EINA_TRUE);
   e_dialog_button_disable_num_set(cfdata->dlg_add_new, 0, 0);
}

static Eina_Bool
_cb_fill_delay(void *data)
{
   E_Config_Dialog_Data *cfdata;

   if (!(cfdata = data)) return ECORE_CALLBACK_RENEW;

   _basic_create_fill(cfdata);

   cfdata->fill_delay = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_dlg_add_cb_del(void *object)
{
   E_Dialog *dlg = object;
   E_Config_Dialog_Data *cfdata = dlg->data;
   cfdata->dlg_add_new = NULL;
}

static void
_cb_used_select(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Config_Dialog_Data *cfdata;
   Elm_Widget_Item *it, *first, *last;
   E_Config_XKB_Layout *cl;

   if (!(cfdata = data)) return;

   it = elm_list_selected_item_get(cfdata->used_list);
   first = elm_list_first_item_get(cfdata->used_list);
   last = elm_list_last_item_get(cfdata->used_list);

   if (!it) return;
   cl = elm_object_item_data_get(it);
   if (!cl) return;
   elm_object_disabled_set(cfdata->btn_del, EINA_FALSE);
   if (first == last)
     {
        elm_object_disabled_set(cfdata->btn_up, EINA_TRUE);
        elm_object_disabled_set(cfdata->btn_down, EINA_TRUE);
     }
   else
     {
        if (it == first)
          elm_object_disabled_set(cfdata->btn_up, EINA_TRUE);
        else
          elm_object_disabled_set(cfdata->btn_up, EINA_FALSE);
        if (it == last)
          elm_object_disabled_set(cfdata->btn_down, EINA_TRUE);
        else
          elm_object_disabled_set(cfdata->btn_down, EINA_FALSE);
     }
}

