/**
 * @addtogroup Optional_Devices
 * @{
 *
 * @defgroup Module_Backlight Backlight
 *
 * Controls backlights such as laptop LCD.
 *
 * @}
 */

#include "e.h"

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);

/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
     "backlight",
     {
        _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL
     },
   E_GADCON_CLIENT_STYLE_PLAIN
};

/* actual module specifics */
typedef struct _Instance Instance;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *o_backlight, *o_table, *o_slider;
   E_Gadcon_Popup  *popup;
   double           val;
   Ecore_Timer     *popup_timer;
};

static Eina_List *backlight_instances = NULL;
static E_Module *backlight_module = NULL;
static E_Action *act = NULL;
static Eina_List *handlers;


static void _backlight_popup_free(Instance *inst);
static void _backlight_level_set(Instance *inst, double value, Eina_Bool set_slider);

static void
_backlight_gadget_update(Instance *inst)
{
   Edje_Message_Float msg;

   msg.val = inst->val;
   if (msg.val < 0.0) msg.val = 0.0;
   else if (msg.val > 1.0) msg.val = 1.0;
   edje_object_message_send(inst->o_backlight, EDJE_MESSAGE_FLOAT, 0, &msg);
}

static void
_backlight_level_set(Instance *inst, double val, Eina_Bool set_slider)
{
   if (val > 1.0) val = 1.0;
   if (val < 0.0) val = 0.0;
   if (set_slider)
     e_widget_slider_value_double_set(inst->o_slider, val);
   inst->val = val;
   e_backlight_mode_set(inst->gcc->gadcon->zone, E_BACKLIGHT_MODE_NORMAL);
   e_backlight_level_set(inst->gcc->gadcon->zone, val, 0.0);
   e_config->backlight.normal = val;
   e_config_save_queue();
}

static Eina_Bool
_backlight_win_key_down_cb(void *data, Ecore_Event_Key *ev)
{
   Instance *inst = data;
   const char *keysym;

   keysym = ev->key;
   if (!strcmp(keysym, "Escape"))
      _backlight_popup_free(inst);
   else if ((!strcmp(keysym, "Up")) ||
            (!strcmp(keysym, "Left")) ||
            (!strcmp(keysym, "KP_Up")) ||
            (!strcmp(keysym, "KP_Left")) ||
            (!strcmp(keysym, "w")) ||
            (!strcmp(keysym, "d")) ||
            (!strcmp(keysym, "bracketright")) ||
            (!strcmp(keysym, "Prior")))
     {
        _backlight_level_set(inst, inst->val + 0.1, EINA_TRUE);
        _backlight_gadget_update(inst);
     }
   else if ((!strcmp(keysym, "Down")) ||
            (!strcmp(keysym, "Right")) ||
            (!strcmp(keysym, "KP_Down")) ||
            (!strcmp(keysym, "KP_Right")) ||
            (!strcmp(keysym, "s")) ||
            (!strcmp(keysym, "a")) ||
            (!strcmp(keysym, "bracketleft")) ||
            (!strcmp(keysym, "Next")))
     {
        _backlight_level_set(inst, inst->val - 0.1, EINA_TRUE);
        _backlight_gadget_update(inst);
     }
   else if ((!strcmp(keysym, "0")) ||
            (!strcmp(keysym, "1")) ||
            (!strcmp(keysym, "2")) ||
            (!strcmp(keysym, "3")) ||
            (!strcmp(keysym, "4")) ||
            (!strcmp(keysym, "5")) ||
            (!strcmp(keysym, "6")) ||
            (!strcmp(keysym, "7")) ||
            (!strcmp(keysym, "8")) ||
            (!strcmp(keysym, "9")))
     {
        _backlight_level_set(inst, (double)atoi(keysym) / 9.0, EINA_TRUE);
        _backlight_gadget_update(inst);
     }
   else
     {
        Eina_List *l;
        E_Config_Binding_Key *binding;
        E_Binding_Modifier mod;

        EINA_LIST_FOREACH(e_bindings->key_bindings, l, binding)
          {
             if (binding->action && strcmp(binding->action, "backlight")) continue;

             mod = 0;

             if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
                mod |= E_BINDING_MODIFIER_SHIFT;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)
                mod |= E_BINDING_MODIFIER_CTRL;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_ALT)
                mod |= E_BINDING_MODIFIER_ALT;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_WIN)
                mod |= E_BINDING_MODIFIER_WIN;

             if (binding->key && (!strcmp(binding->key, ev->key)) &&
                 ((binding->modifiers == mod) || (binding->any_mod)))
               {
                  _backlight_popup_free(inst);
                  break;
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static void
_backlight_settings_cb(void *d1, void *d2 EINA_UNUSED)
{
   Instance *inst = d1;
   e_configure_registry_call("screen/power_management", NULL, NULL);
   _backlight_popup_free(inst);
}

static void
_slider_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst = data;
   _backlight_level_set(inst, inst->val, EINA_FALSE);
}

static void
_backlight_popup_del_cb(void *obj)
{
   _backlight_popup_free(e_object_data_get(obj));
}

static void
_backlight_popup_comp_del_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   _backlight_popup_free(data);
}

static void
_backlight_popup_new(Instance *inst)
{
   Evas *evas;
   Evas_Object *o;

   if (inst->popup) return;

   e_backlight_mode_set(inst->gcc->gadcon->zone, E_BACKLIGHT_MODE_NORMAL);
   inst->val = e_backlight_level_get(inst->gcc->gadcon->zone);
   _backlight_gadget_update(inst);

   inst->popup = e_gadcon_popup_new(inst->gcc, 0);
   evas = e_comp->evas;

   inst->o_table = e_widget_table_add(e_win_evas_win_get(evas), 0);

   o = e_widget_slider_add(evas, 0, 0, NULL, 0.05, 1.0, 0.05, 0, &(inst->val), NULL, 100);
   evas_object_smart_callback_add(o, "changed", _slider_cb, inst);
   inst->o_slider = o;
   e_widget_table_object_align_append(inst->o_table, o,
                                      0, 0, 1, 1, 0, 0, 0, 0, 0.5, 0.5);

   o = e_widget_button_add(evas, NULL, "preferences-system",
                           _backlight_settings_cb, inst, NULL);
   e_widget_table_object_align_append(inst->o_table, o,
                                      0, 1, 1, 1, 0, 0, 0, 0, 0.5, 1.0);

   e_gadcon_popup_content_set(inst->popup, inst->o_table);
   e_gadcon_popup_show(inst->popup);
   e_comp_object_util_autoclose(inst->popup->comp_object, _backlight_popup_comp_del_cb,
     _backlight_win_key_down_cb, inst);
   e_object_data_set(E_OBJECT(inst->popup), inst);
   E_OBJECT_DEL_SET(inst->popup, _backlight_popup_del_cb);
}

static void
_backlight_popup_free(Instance *inst)
{
   E_FREE_FUNC(inst->popup, e_object_del);
}

static void
_backlight_menu_cb_cfg(void *data, E_Menu *menu EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   Instance *inst = data;

   _backlight_popup_free(inst);
   e_configure_registry_call("screen/power_management", NULL, NULL);
}

static void
_backlight_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Instance *inst = data;
   Evas_Event_Mouse_Down *ev = event;

   if (ev->button == 1)
     {
        if (inst->popup) _backlight_popup_free(inst);
        else _backlight_popup_new(inst);
     }
   else if (ev->button == 3)
     {
        E_Zone *zone;
        E_Menu *m;
        E_Menu_Item *mi;
        int x, y;

        zone = e_zone_current_get();

        m = e_menu_new();

        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("Settings"));
        e_util_menu_item_theme_icon_set(mi, "configure");
        e_menu_item_callback_set(mi, _backlight_menu_cb_cfg, inst);

        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse(m, zone, x + ev->output.x, y + ev->output.y,
                              1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
_backlight_cb_mouse_wheel(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Evas_Event_Mouse_Wheel *ev = event;
   Instance *inst = data;

   inst->val = e_backlight_level_get(inst->gcc->gadcon->zone);
   if (ev->z > 0)
     _backlight_level_set(inst, inst->val - 0.1, EINA_FALSE);
   else if (ev->z < 0)
     _backlight_level_set(inst, inst->val + 0.1, EINA_FALSE);
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;

   inst = E_NEW(Instance, 1);

   o = edje_object_add(gc->evas);
   e_theme_edje_object_set(o, "base/theme/modules/backlight",
                           "e/modules/backlight/main");
   evas_object_show(o);

   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;

   inst->gcc = gcc;
   inst->o_backlight = o;

   inst->val = e_backlight_level_get(inst->gcc->gadcon->zone);
   _backlight_gadget_update(inst);

   evas_object_event_callback_add(inst->o_backlight,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _backlight_cb_mouse_down,
                                  inst);
   evas_object_event_callback_add(inst->o_backlight,
                                  EVAS_CALLBACK_MOUSE_WHEEL,
                                  _backlight_cb_mouse_wheel,
                                  inst);

   backlight_instances = eina_list_append(backlight_instances, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   _backlight_popup_free(inst);
   backlight_instances = eina_list_remove(backlight_instances, inst);
   evas_object_del(inst->o_backlight);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   Instance *inst;
   Evas_Coord mw, mh;

   inst = gcc->data;
   mw = 0, mh = 0;
   edje_object_size_min_get(inst->o_backlight, &mw, &mh);
   if ((mw < 1) || (mh < 1))
     edje_object_size_min_calc(inst->o_backlight, &mw, &mh);
   if (mw < 4) mw = 4;
   if (mh < 4) mh = 4;
   e_gadcon_client_aspect_set(gcc, mw, mh);
   e_gadcon_client_min_size_set(gcc, mw, mh);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Backlight");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;
   char buf[4096];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-backlight.edj",
	    e_module_dir_get(backlight_module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   static char buf[4096];

   snprintf(buf, sizeof(buf), "%s.%d", client_class->name,
            eina_list_count(backlight_instances) + 1);
   return buf;
}

static Eina_Bool
_backlight_popup_timer_cb(void *data)
{
   Instance *inst;
   inst = data;

   if (inst->popup)
      _backlight_popup_del_cb(inst->popup);
   inst->popup_timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_backlight_popup_timer_new(Instance *inst)
{
   if (inst->popup)
     {
        if (inst->popup_timer)
          {
             ecore_timer_del(inst->popup_timer);
             e_widget_slider_value_double_set(inst->o_slider, inst->val);
             inst->popup_timer = ecore_timer_loop_add(1.0, _backlight_popup_timer_cb, inst);
          }
     }
   else
     {
        _backlight_popup_new(inst);
        inst->popup_timer = ecore_timer_loop_add(1.0, _backlight_popup_timer_cb, inst);
     }
}

static void
_e_mod_action_cb(E_Object *obj EINA_UNUSED,
                 const char *params)
{
   Eina_List *l;
   Instance *inst;

   EINA_LIST_FOREACH(backlight_instances, l, inst)
     {
        if (!params)
          {
             if (inst->popup) _backlight_popup_free(inst);
             else _backlight_popup_new(inst);
          }
        else
          {
             _backlight_level_set(inst, inst->val + atof(params), EINA_FALSE);
             _backlight_popup_timer_new(inst);
          }
     }
}

static Eina_Bool
_backlight_cb_mod_init_end(void *d EINA_UNUSED, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Eina_List *l;
   Instance *inst;

   EINA_LIST_FOREACH(backlight_instances, l, inst)
     {
        inst->val = e_backlight_level_get(inst->gcc->gadcon->zone);
        _backlight_gadget_update(inst);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_backlight_cb_changed(void *d EINA_UNUSED, int type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Eina_List *l;
   Instance *inst;

   EINA_LIST_FOREACH(backlight_instances, l, inst)
     {
        inst->val = e_backlight_level_get(inst->gcc->gadcon->zone);
        _backlight_gadget_update(inst);
     }
   return ECORE_CALLBACK_RENEW;
}

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
     "Backlight"
};

E_API void *
e_modapi_init(E_Module *m)
{
   backlight_module = m;
   e_gadcon_provider_register(&_gadcon_class);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_BACKLIGHT_CHANGE, _backlight_cb_changed, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_MODULE_INIT_END, _backlight_cb_mod_init_end, NULL);
   act = e_action_add("backlight");
   if (act)
     {
        act->func.go = _e_mod_action_cb;
        e_action_predef_name_set(N_("Screen"), N_("Backlight Controls"), "backlight",
                                 NULL, "syntax: brightness change(-1.0 - 1.0), example: -0.1", 1);
     }
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   if (act)
     {
        e_action_predef_name_del("Screen", "Backlight Controls");
        e_action_del("backlight");
        act = NULL;
     }
   E_FREE_LIST(handlers, ecore_event_handler_del);
   e_gadcon_provider_unregister(&_gadcon_class);
   backlight_module = NULL;
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}
