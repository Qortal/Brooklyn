#include "e_mod_gadman.h"

/* local protos */
static void _gadman_maug_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void _gadman_maug_add(void *data, E_Menu *m);
static void _gadman_action_cb(E_Object *obj, const char *params);
static void _gadman_desktop_menu(void *d EINA_UNUSED, E_Menu *m, void *icon);

/* public module routines. all modules must have these */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Gadman"
};

E_API void *
e_modapi_init(E_Module *m)
{
   char buf[4096];

   /* Set up a new configuration panel */
   snprintf(buf, sizeof(buf), "%s/e-module-gadman.edj", m->dir);
   e_configure_registry_category_add("extensions", 90, _("Extensions"), NULL,
                                     "preferences-extensions");
   e_configure_registry_item_add("extensions/gadman", 150, _("Gadgets"), NULL,
                                 "gadgets", _config_gadman_module);

   /* Set this module to be loaded after all other modules, or we don't see
      modules loaded after this */

   gadman_init(m);

   //Settings values
   Man->conf_edd = E_CONFIG_DD_NEW("Gadman_Config", Config);
#undef T
#undef D
#define T Config
#define D Man->conf_edd
   E_CONFIG_VAL(D, T, bg_type, INT);
   E_CONFIG_VAL(D, T, color_r, INT);
   E_CONFIG_VAL(D, T, color_g, INT);
   E_CONFIG_VAL(D, T, color_b, INT);
   E_CONFIG_VAL(D, T, color_a, INT);
   E_CONFIG_VAL(D, T, anim_bg, INT);
   E_CONFIG_VAL(D, T, anim_gad, INT);
   E_CONFIG_VAL(D, T, custom_bg, STR);

   Man->conf = e_config_domain_load("module.gadman", Man->conf_edd);
   if (!Man->conf)
     {
        Man->conf = E_NEW(Config, 1);
        Man->conf->bg_type = 0;
        Man->conf->color_r = 255;
        Man->conf->color_g = 255;
        Man->conf->color_b = 255;
        Man->conf->color_a = 255;
        Man->conf->anim_bg = 1;
        Man->conf->anim_gad = 1;
        Man->conf->custom_bg = NULL;
     }
   E_CONFIG_LIMIT(Man->conf->bg_type, 0, 5);
   E_CONFIG_LIMIT(Man->conf->color_r, 0, 255);
   E_CONFIG_LIMIT(Man->conf->color_g, 0, 255);
   E_CONFIG_LIMIT(Man->conf->color_b, 0, 255);
   E_CONFIG_LIMIT(Man->conf->color_a, 0, 255);
   E_CONFIG_LIMIT(Man->conf->anim_bg, 0, 1);
   E_CONFIG_LIMIT(Man->conf->anim_gad, 0, 1);

   /* Menu augmentation */
   Man->icon_name = eina_stringshare_add(buf);
   Man->maug = NULL;
   Man->mcat = e_menu_category_callback_add("desktop", _gadman_desktop_menu, NULL, (void*)Man->icon_name);
   Man->maug =
     e_int_menus_menu_augmentation_add_sorted
       ("config/1", _("Gadgets"), _gadman_maug_add, (void *)Man->icon_name,
       NULL, NULL);
   /* Create toggle action */
   Man->action = e_action_add("gadman_toggle");
   if (Man->action)
     {
        Man->action->func.go = _gadman_action_cb;
        e_action_predef_name_set(N_("Gadgets"), N_("Show/hide gadgets"),
                                 "gadman_toggle", NULL, NULL, 0);
     }

   gadman_update_bg();

   return Man;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   if (Man->maug)
     e_int_menus_menu_augmentation_del("config/1", Man->maug);

   e_configure_registry_item_del("extensions/gadman");
   e_configure_registry_category_del("extensions");
   if (Man->mcat) e_menu_category_callback_del(Man->mcat);

   if (Man->config_dialog)
     {
        e_object_del(E_OBJECT(Man->config_dialog));
        Man->config_dialog = NULL;
     }
   if (Man->action)
     {
        e_action_predef_name_del("Gadgets", "Show/hide gadgets");
        e_action_del("gadman_toggle");
        Man->action = NULL;
     }
   if (Man->add)
     ecore_event_handler_del(Man->add);
   Man->waiting = eina_list_free(Man->waiting);
   E_CONFIG_DD_FREE(Man->conf_edd);
   if (Man->conf)
     {
        eina_stringshare_del(Man->conf->custom_bg);
        E_FREE(Man->conf);
     }
   gadman_shutdown();

   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.gadman", Man->conf_edd, Man->conf);
   return 1;
}

static void
_cb_config_del(void *data)
{
   int layer;
   Eina_List *l;
   E_Gadcon *gc;
   Eina_Bool del = EINA_TRUE;

   for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
     EINA_LIST_FOREACH(Man->gadcons[layer], l, gc)
       if (gc->config_dialog)
         {
            del = EINA_FALSE;
            break;
         }
   Man->waiting = eina_list_remove(Man->waiting, data);
   if (del && Man->add) ecore_event_handler_del(Man->add);
}

static void
_gadman_desktop_menu_cb(void *data EINA_UNUSED, E_Menu *m, E_Menu_Item *mi EINA_UNUSED)
{
   Eina_List *l;
   E_Gadcon *gc;

   EINA_LIST_FOREACH(Man->gadcons[0], l, gc)
     {
        if (gc->zone == m->zone)
          {
             e_int_gadcon_config_hook(gc, _("Desktop Gadgets"), E_GADCON_SITE_DESKTOP);
             if (!Man->add)
               Man->add = ecore_event_handler_add(E_EVENT_GADCON_CLIENT_ADD, (Ecore_Event_Handler_Cb)gadman_gadget_add_handler, NULL);
             Man->waiting = eina_list_append(Man->waiting, gc);
             e_object_del_attach_func_set(E_OBJECT(gc->config_dialog), _cb_config_del);
             break;
          }
     }
}

static E_Menu_Item *
_gadman_menu_insert_get(E_Menu *menu)
{
   E_Menu_Item *mi, *prev_mi = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(menu->items, l, mi)
     {
        if (!strcmp(mi->label, _("Change Wallpaper")))
          return prev_mi;
        prev_mi = mi;
     }

   return NULL;
}

static void
_gadman_desktop_menu(void *d EINA_UNUSED, E_Menu *m, void *icon EINA_UNUSED)
{
   E_Menu_Item *mi, *pos;

   pos = _gadman_menu_insert_get(m);
   /* don't add twice */
   if (pos->cb.func == _gadman_desktop_menu_cb) return;

   mi = e_menu_item_new_relative(m, pos);
   e_menu_item_label_set(mi, _("Change Gadgets"));
   e_util_menu_item_theme_icon_set(mi, "preferences-desktop-shelf");
   e_menu_item_callback_set(mi, _gadman_desktop_menu_cb, NULL);
}

static void
_gadman_maug_cb(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   e_configure_registry_call("extensions/gadman", NULL, NULL);
}

static void
_gadman_maug_add(void *data, E_Menu *m)
{
   E_Menu_Item *mi;

   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Gadgets"));
   e_menu_item_icon_edje_set(mi, (char *)data, "icon");
   e_menu_item_callback_set(mi, _gadman_maug_cb, NULL);
}

static void
_gadman_action_cb(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   gadman_gadgets_toggle();
}

Eina_Bool
gadman_gadget_add_handler(void *d EINA_UNUSED, int type EINA_UNUSED, E_Event_Gadcon_Client_Add *ev)
{
   Eina_List *l;
   if (!Man->waiting) return ECORE_CALLBACK_RENEW;
   l = eina_list_data_find_list(Man->waiting, ev->gcc->gadcon);
   if (!l) return ECORE_CALLBACK_RENEW;
   if (EINA_DBL_NONZERO(ev->gcc->cf->geom.pos_x) || EINA_DBL_NONZERO(ev->gcc->cf->geom.pos_y) ||
       EINA_DBL_NONZERO(ev->gcc->cf->geom.size_w) || EINA_DBL_NONZERO(ev->gcc->cf->geom.size_h))
     return ECORE_CALLBACK_RENEW;
   if ((ev->gcc->gadcon->new_gcc) && (ev->gcc->gadcon->new_gcc == ev->gcc))
     return ECORE_CALLBACK_RENEW;
   ev->gcc->cf->style = eina_stringshare_add(ev->gcc->client_class->default_style ?: E_GADCON_CLIENT_STYLE_INSET);
   ev->gcc->style = eina_stringshare_ref(ev->gcc->cf->style);
   ev->gcc->cf->geom.pos_x = DEFAULT_POS_X;
   ev->gcc->cf->geom.pos_y = DEFAULT_POS_Y;
   ev->gcc->cf->geom.size_w = DEFAULT_SIZE_W;
   ev->gcc->cf->geom.size_h = DEFAULT_SIZE_H;
   if (!strcmp(ev->gcc->style, E_GADCON_CLIENT_STYLE_INSET))
     edje_object_signal_emit(ev->gcc->o_frame, "e,state,visibility,inset", "e");
   else
     edje_object_signal_emit(ev->gcc->o_frame, "e,state,visibility,plain", "e");
   gadman_gadget_edit_start(ev->gcc);

   return ECORE_CALLBACK_RENEW;
}
