#include "e.h"
#include "e_mod_main.h"

/* local structures */
typedef struct _Instance Instance;
struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *o_base;
   E_Menu          *menu;
};

typedef struct Syscon_Config
{
   Eina_Bool menu;
} Syscon_Config;

/* local function prototypes */
static E_Gadcon_Client        *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void                    _gc_shutdown(E_Gadcon_Client *gcc);
static void                    _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char             *_gc_label(const E_Gadcon_Client_Class *cc EINA_UNUSED);
static Evas_Object            *_gc_icon(const E_Gadcon_Client_Class *cc, Evas *evas);
static const char             *_gc_id_new(const E_Gadcon_Client_Class *cc);
static void                    _cb_shutdown_show(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED);
static void                    _cb_menu_post(void *data, E_Menu *m EINA_UNUSED);
static void                    _cb_menu_sel(void *data, E_Menu *m, E_Menu_Item *mi EINA_UNUSED);
static E_Config_Syscon_Action *_find_action(const char *name);
static void                    _create_menu(Instance *inst);

/* local variables */
static E_Config_DD *conf_edd = NULL;
static Syscon_Config *syscon_config = NULL;
static Eina_List *instances = NULL;
static E_Module *mod = NULL;

static const E_Gadcon_Client_Class _gc_class =
{
   GADCON_CLIENT_CLASS_VERSION, "syscon",
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new,
      NULL, NULL
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};

/* local functions */
static void
_cb_menu_change(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   syscon_config->menu = !syscon_config->menu;
}


static void
_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;
   E_Menu *m;
   E_Menu_Item *mi;
   E_Zone *zone;
   int x, y;

   inst = data;
   ev = event_info;
   if (ev->button != 3) return;
   zone = e_zone_current_get();

   m = e_menu_new();
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Show Menu"));
   e_menu_item_check_set(mi, 1);
   e_menu_item_toggle_set(mi, syscon_config->menu);
   e_menu_item_callback_set(mi, _cb_menu_change, inst);
   m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
   ecore_evas_pointer_xy_get(e_comp->ee, &x, &y);
   e_menu_activate_mouse(m, zone, x, y, 1, 1,
                         E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
   evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                            EVAS_BUTTON_NONE, ev->timestamp, NULL);
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Instance *inst;

   inst = E_NEW(Instance, 1);
   inst->o_base = edje_object_add(gc->evas);
   e_theme_edje_object_set(inst->o_base,
                           "base/theme/modules/syscon",
                           "e/modules/syscon/button");
   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_base);
   inst->gcc->data = inst;

   e_gadcon_client_util_menu_attach(inst->gcc);
   evas_object_event_callback_add(inst->o_base, EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down, inst);

   edje_object_signal_callback_add(inst->o_base, "e,action,shutdown,show", "e",
                                   _cb_shutdown_show, inst);

   instances = eina_list_append(instances, inst);
   return inst->gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   if (!(inst = gcc->data)) return;
   instances = eina_list_remove(instances, inst);
   if (inst->menu)
     {
        e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->menu));
        inst->menu = NULL;
     }
   if (inst->o_base) evas_object_del(inst->o_base);
   E_FREE(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *cc EINA_UNUSED)
{
   return _("System");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *cc EINA_UNUSED, Evas *evas)
{
   Evas_Object *obj;

   obj = edje_object_add(evas);
   e_theme_edje_object_set(obj,
                           "base/theme/modules/syscon",
                           "e/modules/syscon/button");
   return obj;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *cc)
{
   static char buff[128];

   snprintf(buff, sizeof(buff), "%s.%d",
            cc->name, eina_list_count(instances) + 1);
   return buff;
}

static void
_cb_shutdown_show(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Instance *inst;
   E_Zone *zone;
   Evas_Coord x, y, w, h, cx, cy;

   if (!(inst = data)) return;
   zone = e_zone_current_get();
   if (!syscon_config->menu)
     {
        e_syscon_show(zone, NULL);
        return;
     }
   evas_object_geometry_get(inst->o_base, &x, &y, &w, &h);
   e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy, NULL, NULL);
   x += cx;
   y += cy;
   if (!inst->menu) _create_menu(inst);
   if (inst->menu)
     {
        int dir = 0;

        e_menu_post_deactivate_callback_set(inst->menu, _cb_menu_post, inst);
        switch (inst->gcc->gadcon->orient)
          {
           case E_GADCON_ORIENT_TOP:
           case E_GADCON_ORIENT_CORNER_TL:
           case E_GADCON_ORIENT_CORNER_TR:
             dir = E_MENU_POP_DIRECTION_DOWN;
             break;

           case E_GADCON_ORIENT_CORNER_BL:
           case E_GADCON_ORIENT_CORNER_BR:
           case E_GADCON_ORIENT_BOTTOM:
             dir = E_MENU_POP_DIRECTION_UP;
             break;

           case E_GADCON_ORIENT_LEFT:
           case E_GADCON_ORIENT_CORNER_LT:
           case E_GADCON_ORIENT_CORNER_LB:
             dir = E_MENU_POP_DIRECTION_RIGHT;
             break;

           case E_GADCON_ORIENT_RIGHT:
           case E_GADCON_ORIENT_CORNER_RT:
           case E_GADCON_ORIENT_CORNER_RB:
             dir = E_MENU_POP_DIRECTION_LEFT;
             break;

           case E_GADCON_ORIENT_FLOAT:
           case E_GADCON_ORIENT_HORIZ:
           case E_GADCON_ORIENT_VERT:
           default:
             dir = E_MENU_POP_DIRECTION_AUTO;
             break;
          }
        e_gadcon_locked_set(inst->gcc->gadcon, EINA_TRUE);
        e_menu_activate_mouse(inst->menu, zone, x, y, w, h, dir, 0);
     }
}

static void
_cb_menu_post(void *data, E_Menu *m)
{
   Instance *inst = data;
   Eina_Bool fin;

   if (stopping || (!inst->menu)) return;
   fin = m == inst->menu;
   e_object_del(E_OBJECT(m));
   if (!fin) return;
   e_gadcon_locked_set(inst->gcc->gadcon, 0);
   inst->menu = NULL;
}

static void
_cb_menu_sel(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Config_Syscon_Action *sca;
   E_Action *act;

   if (!(sca = data)) return;
   if (!(act = e_action_find(sca->action))) return;
   act->func.go(NULL, sca->params);
}

static void
_create_menu(Instance *inst)
{
   if (!inst) return;
   inst->menu = e_menu_new();
   e_syscon_menu_fill(inst->menu);
}

static E_Config_Syscon_Action *
_find_action(const char *name)
{
   E_Config_Syscon_Action *sca;
   Eina_List *l;

   if (!name) return NULL;
   EINA_LIST_FOREACH(e_config->syscon.actions, l, sca)
     {
        if (!sca->action) continue;
        if (!strcmp(sca->action, name)) return sca;
     }
   return NULL;
}

/* public functions */
void
e_syscon_gadget_init(E_Module *m)
{
   mod = m;

   e_gadcon_provider_register(&_gc_class);
   conf_edd = E_CONFIG_DD_NEW("Syscon_Config", Syscon_Config);
   #undef T
   #undef D
   #define T Syscon_Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, menu, UCHAR);
   syscon_config = e_config_domain_load("module.syscon", conf_edd);
   if (!syscon_config)
     {
        syscon_config = E_NEW(Syscon_Config, 1);
        syscon_config->menu = EINA_FALSE;
     }
   e_config_save_queue();
}

void
e_syscon_gadget_shutdown(void)
{
   e_gadcon_provider_unregister(&_gc_class);
   e_config_domain_save("module.syscon", conf_edd, syscon_config);
   E_FREE(syscon_config);
   E_CONFIG_DD_FREE(conf_edd);
   conf_edd = NULL;
   mod = NULL;
}


void
e_syscon_menu_fill(E_Menu *m)
{
   E_Config_Syscon_Action *sca;
   E_Menu_Item *it;

   if (!m) return;
   if (m->items) return;
   if ((sca = _find_action("desk_lock")))
     {
        it = e_menu_item_new(m);
        e_menu_item_label_set(it, _(e_action_predef_label_get(sca->action,
                                                              sca->params)));
        if (sca->icon)
          e_util_menu_item_theme_icon_set(it, sca->icon);
        e_menu_item_callback_set(it, _cb_menu_sel, sca);
     }

   if ((sca = _find_action("logout")))
     {
        it = e_menu_item_new(m);
        e_menu_item_label_set(it, _(e_action_predef_label_get(sca->action,
                                                              sca->params)));
        if (sca->icon)
          e_util_menu_item_theme_icon_set(it, sca->icon);
        e_menu_item_callback_set(it, _cb_menu_sel, sca);
        if (!e_sys_action_possible_get(E_SYS_LOGOUT))
          e_menu_item_disabled_set(it, EINA_TRUE);
     }

   it = e_menu_item_new(m);
   e_menu_item_separator_set(it, EINA_TRUE);

   if ((sca = _find_action("suspend")))
     {
        it = e_menu_item_new(m);
        e_menu_item_label_set(it, _(e_action_predef_label_get(sca->action,
                                                              sca->params)));
        if (sca->icon)
          e_util_menu_item_theme_icon_set(it, sca->icon);
        e_menu_item_callback_set(it, _cb_menu_sel, sca);
        if (!e_sys_action_possible_get(E_SYS_SUSPEND))
          e_menu_item_disabled_set(it, EINA_TRUE);
     }

   if ((sca = _find_action("hibernate")))
     {
        it = e_menu_item_new(m);
        e_menu_item_label_set(it, _(e_action_predef_label_get(sca->action,
                                                              sca->params)));
        if (sca->icon)
          e_util_menu_item_theme_icon_set(it, sca->icon);
        e_menu_item_callback_set(it, _cb_menu_sel, sca);
        if (!e_sys_action_possible_get(E_SYS_HIBERNATE))
          e_menu_item_disabled_set(it, EINA_TRUE);
     }

   it = e_menu_item_new(m);
   e_menu_item_separator_set(it, EINA_TRUE);

   if ((sca = _find_action("reboot")))
     {
        it = e_menu_item_new(m);
        e_menu_item_label_set(it, _(e_action_predef_label_get(sca->action,
                                                              sca->params)));
        if (sca->icon)
          e_util_menu_item_theme_icon_set(it, sca->icon);
        e_menu_item_callback_set(it, _cb_menu_sel, sca);
        if (!e_sys_action_possible_get(E_SYS_REBOOT))
          e_menu_item_disabled_set(it, EINA_TRUE);
     }

   if ((sca = _find_action("halt")))
     {
        it = e_menu_item_new(m);
        e_menu_item_label_set(it, _(e_action_predef_label_get(sca->action,
                                                              sca->params)));
        if (sca->icon)
          e_util_menu_item_theme_icon_set(it, sca->icon);
        e_menu_item_callback_set(it, _cb_menu_sel, sca);
        if (!e_sys_action_possible_get(E_SYS_HALT))
          e_menu_item_disabled_set(it, EINA_TRUE);
     }
}


E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.syscon", conf_edd, syscon_config);
   return 1;
}
