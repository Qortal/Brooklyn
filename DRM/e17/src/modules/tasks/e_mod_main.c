#include "e.h"
#include "e_mod_main.h"

/***************************************************************************/
/**/
/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void             _gc_shutdown(E_Gadcon_Client *gcc);
static void             _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char      *_gc_label(const E_Gadcon_Client_Class *client_class);
static Evas_Object     *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);
static const char      *_gc_id_new(const E_Gadcon_Client_Class *client_class);

/* and actually define the gadcon class that this module provides (just 1) */
static E_Gadcon_Client_Class _gadcon_class = {
   GADCON_CLIENT_CLASS_VERSION,
   "tasks",
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, NULL
   },
   E_GADCON_CLIENT_STYLE_PLAIN
};
/**/
/***************************************************************************/
/***************************************************************************/
/**/
/* actual module specifics */

typedef struct _Tasks      Tasks;
typedef struct _Tasks_Item Tasks_Item;

struct _Tasks
{
   E_Gadcon_Client *gcc; // The gadcon client
   E_Comp_Object_Mover *iconify_provider;
   Evas_Object     *o_items; // Table of items
   Eina_List       *items; // List of items
   Eina_List       *clients; // List of clients
   E_Zone          *zone; // Current Zone
   Config_Item     *config; // Configuration
   int              horizontal;
};

struct _Tasks_Item
{
   Tasks       *tasks; // Parent tasks
   E_Client    *client; // The client this item points to
   Evas_Object *o_item; // The edje theme object
   Evas_Object *o_icon; // The icon
   Evas_Object *o_preview; // The preview
   Ecore_Timer *timer; // The preview timer
   Eina_Bool skip_taskbar E_BITFIELD;
   Eina_Bool focused E_BITFIELD;
   Eina_Bool urgent E_BITFIELD;
   Eina_Bool iconified E_BITFIELD;
   Eina_Bool iconifying E_BITFIELD;
   Eina_Bool delete_me E_BITFIELD;
};

static Tasks       *_tasks_new(Evas *e, E_Zone *zone, const char *id);
static void         _tasks_free(Tasks *tasks);
static void         _tasks_refill(Tasks *tasks);
static void         _tasks_refill_all();
static void         _tasks_refill_border(E_Client *ec);

static Tasks_Item  *_tasks_item_find(Tasks *tasks, E_Client *ec);
static Tasks_Item  *_tasks_item_new(Tasks *tasks, E_Client *ec);
static Tasks_Item  *_tasks_all_item_find(E_Client *ec);

static int          _tasks_item_check_add(Tasks *tasks, E_Client *ec);
static void         _tasks_item_add(Tasks *tasks, E_Client *ec);
static void         _tasks_item_remove(Tasks_Item *item);
static void         _tasks_item_refill(Tasks_Item *item);
static void         _tasks_item_fill(Tasks_Item *item);
static void         _tasks_item_free(Tasks_Item *item);
static void         _tasks_item_signal_emit(Tasks_Item *item, char *sig, char *src);
static void         _tasks_item_preview_add(Tasks_Item *item);
static void         _tasks_item_preview_del(Tasks_Item *item);

static Config_Item *_tasks_config_item_get(const char *id);

static void         _tasks_cb_menu_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void         _tasks_cb_item_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _tasks_cb_item_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _tasks_cb_item_mouse_wheel(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void         _tasks_cb_item_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void         _tasks_cb_item_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void         _tasks_cb_item_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);

static Eina_Bool    _tasks_cb_event_client_add(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_event_client_remove(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_event_client_iconify(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_event_client_uniconify(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_event_client_prop_change(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_event_client_zone_set(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_event_client_desk_set(void *data, int type, E_Event_Client *ev);
static Eina_Bool    _tasks_cb_window_focus_in(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_window_focus_out(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_event_desk_show(void *data, int type, void *event);
static Eina_Bool    _tasks_cb_timer_del(void *data);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;

static Ecore_Timer *task_refill_timer;

Config *tasks_config = NULL;

/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Tasks"
};

E_API void *
e_modapi_init(E_Module *m)
{
   conf_item_edd = E_CONFIG_DD_NEW("Tasks_Config_Item", Config_Item);

#undef T
#undef D
#define T Config_Item
#define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, show_all_desktops, INT);
   E_CONFIG_VAL(D, T, show_all_screens, INT);
   E_CONFIG_VAL(D, T, minw, INT);
   E_CONFIG_VAL(D, T, minh, INT);
   E_CONFIG_VAL(D, T, preview_size, INT);
   E_CONFIG_VAL(D, T, icon_only, UCHAR);
   E_CONFIG_VAL(D, T, text_only, UCHAR);
   E_CONFIG_VAL(D, T, preview, UCHAR);

   conf_edd = E_CONFIG_DD_NEW("Tasks_Config", Config);

#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);

   tasks_config = e_config_domain_load("module.tasks", conf_edd);
   if (!tasks_config)
     {
        Config_Item *config;

        tasks_config = E_NEW(Config, 1);

        config = E_NEW(Config_Item, 1);
        config->id = eina_stringshare_add("0");
        config->show_all_desktops = 0;
        config->show_all_screens = 0;
        config->minw = 100;
        config->minh = 32;
        config->preview = 0;
        config->preview_size = 32;
        tasks_config->items = eina_list_append(tasks_config->items, config);
     }

   tasks_config->module = m;

   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_ADD, _tasks_cb_event_client_add, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_REMOVE, _tasks_cb_event_client_remove, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_ICONIFY, _tasks_cb_event_client_iconify, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_UNICONIFY, _tasks_cb_event_client_uniconify, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_PROPERTY, _tasks_cb_event_client_prop_change, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_DESK_SET, _tasks_cb_event_client_desk_set, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_ZONE_SET, _tasks_cb_event_client_zone_set, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_FOCUS_IN, _tasks_cb_window_focus_in, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_CLIENT_FOCUS_OUT, _tasks_cb_window_focus_out, NULL);
   E_LIST_HANDLER_APPEND(tasks_config->handlers, E_EVENT_DESK_SHOW, _tasks_cb_event_desk_show, NULL);

   e_gadcon_provider_register(&_gadcon_class);
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   Ecore_Event_Handler *eh;
   Tasks *tasks;
   Config_Item *cfg;

   e_gadcon_provider_unregister(&_gadcon_class);

   EINA_LIST_FREE(tasks_config->tasks, tasks)
     {
        _tasks_free(tasks);
     }

   if (tasks_config->config_dialog)
     e_object_del(E_OBJECT(tasks_config->config_dialog));

   EINA_LIST_FREE(tasks_config->items, cfg)
     {
        eina_stringshare_del(cfg->id);
        free(cfg);
     }

   EINA_LIST_FREE(tasks_config->handlers, eh)
     {
        ecore_event_handler_del(eh);
     }

   free(tasks_config);
   tasks_config = NULL;
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.tasks", conf_edd, tasks_config);
   return 1;
}

/**************************************************************/

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Tasks *tasks;
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   /* Evas_Coord x, y, w, h; */
   /* int cx, cy, cw, ch; */

   tasks = _tasks_new(gc->evas, gc->zone, id);

   o = tasks->o_items;
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = tasks;
   tasks->gcc = gcc;

   /* e_gadcon_canvas_zone_geometry_get(gcc->gadcon, &cx, &cy, &cw, &ch); */
   /* evas_object_geometry_get(o, &x, &y, &w, &h); */

   tasks_config->tasks = eina_list_append(tasks_config->tasks, tasks);

   e_gadcon_client_autoscroll_toggle_disabled_set(gcc, 1);
   // Fill on initial config
   _tasks_config_updated(tasks->config);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Tasks *tasks;

   tasks = (Tasks *)gcc->data;
   tasks_config->tasks = eina_list_remove(tasks_config->tasks, tasks);
   _tasks_free(tasks);
}

/* TODO */
static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   Tasks *tasks;

   tasks = (Tasks *)gcc->data;

   switch (orient)
     {
      case E_GADCON_ORIENT_FLOAT:
      case E_GADCON_ORIENT_HORIZ:
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
        if (!tasks->horizontal)
          {
             tasks->horizontal = 1;
             elm_box_horizontal_set(tasks->o_items, tasks->horizontal);
             _tasks_refill(tasks);
          }
        break;

      case E_GADCON_ORIENT_VERT:
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_RIGHT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_LB:
      case E_GADCON_ORIENT_CORNER_RB:
        if (tasks->horizontal)
          {
             tasks->horizontal = 0;
             elm_box_horizontal_set(tasks->o_items, tasks->horizontal);
             _tasks_refill(tasks);
          }
        break;

      default:
        break;
     }
   elm_box_align_set(tasks->o_items, 0.5, 0.5);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Tasks");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;
   char buf[4096];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-tasks.edj",
            e_module_dir_get(tasks_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   Config_Item *config;

   config = _tasks_config_item_get(NULL);
   return config->id;
}

/***************************************************************************/
static void
_tasks_cb_iconify_end_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
   Tasks_Item *item;
   E_Client *ec = data;

   if (!ec) return;

   evas_object_layer_set(ec->frame, ec->layer);
   ec->layer_block = 0;
   if (ec->iconic) evas_object_hide(ec->frame);

   item = _tasks_all_item_find(ec);
   if (!item) return;

   item->iconifying = 0;
   if (item->delete_me) free(item);
}

static Eina_Bool
_tasks_cb_iconify_provider(void *data, Evas_Object *obj, const char *signal)
{
   Tasks *tasks = data;
   Tasks_Item *item;
   Evas_Coord ox, oy, ow, oh;
   Eina_List *l;
   E_Client *ec;

   ec = e_comp_object_client_get(obj);
   if (ec->zone != tasks->gcc->gadcon->zone) return EINA_FALSE;
   EINA_LIST_FOREACH(tasks->items, l, item)
     {
        E_Client *ec2;

        ec2 = e_client_stack_bottom_get(item->client);
        for (; ec2; ec2 = ec2->stack.next)
          {
             if ((ec2 == ec) && (!item->iconifying))
               {
                  evas_object_geometry_get(item->o_item, &ox, &oy, &ow, &oh);
                  ec->layer_block = 1;
                  evas_object_layer_set(ec->frame, E_LAYER_CLIENT_PRIO);
                  e_comp_object_effect_set(ec->frame, "iconify/tasks");
                  e_comp_object_effect_params_set(ec->frame, 1, (int[]){ec->x, ec->y, ec->w, ec->h, ox, oy, ow, oh}, 8);
                  e_comp_object_effect_params_set(ec->frame, 0, (int[]){!!strcmp(signal, "e,action,iconify")}, 1);
                  item->iconifying = 1;
                  e_comp_object_effect_start(ec->frame, _tasks_cb_iconify_end_cb, ec);
                  return EINA_TRUE;
               }
          }
     }
   return EINA_FALSE;
}

static Tasks *
_tasks_new(Evas *e, E_Zone *zone, const char *id)
{
   Tasks *tasks;
   Eina_List *l;
   E_Client *ec;

   tasks = E_NEW(Tasks, 1);
   tasks->config = _tasks_config_item_get(id);
   tasks->o_items = elm_box_add(e_win_evas_win_get(e));
   E_EXPAND(tasks->o_items);
   E_FILL(tasks->o_items);
   tasks->horizontal = 1;
   EINA_LIST_FOREACH(e_comp->clients, l, ec)
     {
        if ((!e_client_util_ignored_get(ec)) && (!e_object_is_del(E_OBJECT(ec))) &&
            (!e_client_util_is_popup(ec)))
          tasks->clients = eina_list_append(tasks->clients, ec);
     }

   elm_box_homogeneous_set(tasks->o_items, 1);
   elm_box_horizontal_set(tasks->o_items, tasks->horizontal);
   elm_box_align_set(tasks->o_items, 0.5, 0.5);
   tasks->zone = zone;
   tasks->iconify_provider = e_comp_object_effect_mover_add(90, "e,action,*iconify", _tasks_cb_iconify_provider, tasks);
   return tasks;
}

static void
_tasks_free(Tasks *tasks)
{
   Tasks_Item *item;
   e_comp_object_effect_mover_del(tasks->iconify_provider);
   EINA_LIST_FREE(tasks->items, item)
     _tasks_item_free(item);
   eina_list_free(tasks->clients);
   evas_object_del(tasks->o_items);
   free(tasks);
}

static void
_tasks_refill(Tasks *tasks)
{
   Eina_List *l;
   E_Client *ec;
   Tasks_Item *item;
   Evas_Coord w, h, tw, th;

   while (tasks->items)
     {
        item = tasks->items->data;
        _tasks_item_remove(item);
     }
   EINA_LIST_FOREACH(tasks->clients, l, ec)
     {
        _tasks_item_check_add(tasks, ec);
     }
   if (tasks->items)
     {
        item = tasks->items->data;
        evas_object_geometry_get(tasks->o_items, NULL, NULL, &tw, &th);
//        edje_object_size_min_calc(item->o_item, &w, &h);
        if (tasks->horizontal)
          edje_object_size_min_restricted_calc(item->o_item, &w, &h, 0, th);
        else
          edje_object_size_min_restricted_calc(item->o_item, &w, &h, tw, 0);
        if (tasks->horizontal)
          {
             if (w < tasks->config->minw) w = tasks->config->minw;
             if (tasks->config->icon_only) w = h;
          }
        else
          {
             if (h < tasks->config->minh) h = tasks->config->minh;
             if (tasks->config->icon_only) h = w;
          }
        if (!tasks->gcc->resizable)
          {
             if (tasks->horizontal)
               e_gadcon_client_min_size_set(tasks->gcc,
                                            w * eina_list_count(tasks->items),
                                            h);
             else
               e_gadcon_client_min_size_set(tasks->gcc,
                                            w,
                                            h * eina_list_count(tasks->items));
          }
        if (tasks->horizontal)
          {
             EINA_LIST_FOREACH(tasks->items, l, item)
               {
                  evas_object_size_hint_min_set(item->o_item, w, th);
               }
          }
        else
          {
             EINA_LIST_FOREACH(tasks->items, l, item)
               {
                  evas_object_size_hint_min_set(item->o_item, tw, h);
               }
          }
     }
   else
     e_gadcon_client_min_size_set(tasks->gcc, 0, 0);
}

static Eina_Bool
_refill_timer(void *d EINA_UNUSED)
{
   if (e_drag_current_get()) return EINA_TRUE;

   _tasks_refill_all();
   task_refill_timer = NULL;
   return EINA_FALSE;
}

static void
_tasks_refill_all(void)
{
   const Eina_List *l;
   Tasks *tasks;

   if (e_drag_current_get())
     {
        if (task_refill_timer)
          ecore_timer_loop_reset(task_refill_timer);
        else
          task_refill_timer = ecore_timer_loop_add(0.5, _refill_timer, NULL);
        return;
     }

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        _tasks_refill(tasks);
     }
}

static void
_tasks_refill_border(E_Client *ec)
{
   const Eina_List *l;
   Tasks *tasks;
   Eina_Bool found = EINA_FALSE;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        const Eina_List *m;
        Tasks_Item *item;
        EINA_LIST_FOREACH(tasks->items, m, item)
          {
             E_Client *ec2;

             ec2 = e_client_stack_bottom_get(item->client);
             for (; ec2; ec2 = ec2->stack.next)
               {
                  if (ec2 == ec)
                    {
                       _tasks_item_refill(item);
                       found = EINA_TRUE;
                       goto done;
                    }
               }
          }
     }
done:
   if (!found) _tasks_refill_all();
}

static Tasks_Item *
_tasks_item_find(Tasks *tasks, E_Client *ec)
{
   const Eina_List *l;
   Tasks_Item *item;

   EINA_LIST_FOREACH(tasks->items, l, item)
     {
        E_Client *ec2;

        ec2 = e_client_stack_bottom_get(item->client);
        for (; ec2; ec2 = ec2->stack.next)
          {
             if (ec2 == ec) return item;
          }
     }
   return NULL;
}

static Tasks_Item *
_tasks_all_item_find(E_Client *ec)
{
   const Eina_List *l;
   Tasks *tasks;
   Tasks_Item *item = NULL;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        item = _tasks_item_find(tasks, ec);
        if (item) break;
     }
   return item;
}

static Tasks_Item *
_tasks_item_new(Tasks *tasks, E_Client *ec)
{
   Tasks_Item *item;

   item = E_NEW(Tasks_Item, 1);
   e_object_ref(E_OBJECT(ec));
   item->tasks = tasks;
   item->client = ec;
   item->skip_taskbar = ec->netwm.state.skip_taskbar;
   item->o_item = edje_object_add(evas_object_evas_get(tasks->o_items));
   if (tasks->horizontal)
     e_theme_edje_object_set(item->o_item,
                             "base/theme/modules/tasks",
                             "e/modules/tasks/item");
   else
     {
        if (!e_theme_edje_object_set(item->o_item,
                                     "base/theme/modules/tasks",
                                     "e/modules/tasks/item_vert"))
          e_theme_edje_object_set(item->o_item,
                                  "base/theme/modules/tasks",
                                  "e/modules/tasks/item");
     }
   if (tasks->config->text_only)
     {
        edje_object_signal_emit(item->o_item, "e,state,text_only", "e");
        edje_object_message_signal_process(item->o_item);
     }
   else if (tasks->config->icon_only)
     {
        edje_object_signal_emit(item->o_item, "e,state,icon_only", "e");
        edje_object_message_signal_process(item->o_item);
     }
   evas_object_event_callback_add(item->o_item, EVAS_CALLBACK_MOUSE_DOWN,
                                  _tasks_cb_item_mouse_down, item);
   evas_object_event_callback_add(item->o_item, EVAS_CALLBACK_MOUSE_UP,
                                  _tasks_cb_item_mouse_up, item);
   evas_object_event_callback_add(item->o_item, EVAS_CALLBACK_MOUSE_WHEEL,
                                  _tasks_cb_item_mouse_wheel, item);
   evas_object_event_callback_add(item->o_item, EVAS_CALLBACK_MOUSE_IN,
                                  _tasks_cb_item_mouse_in, item);
   evas_object_event_callback_add(item->o_item, EVAS_CALLBACK_MOUSE_OUT,
                                  _tasks_cb_item_mouse_out, item);
   evas_object_event_callback_add(item->o_item, EVAS_CALLBACK_DEL,
                                  _tasks_cb_item_del, item);
   evas_object_show(item->o_item);

   _tasks_item_fill(item);
   return item;
}

static int
_tasks_item_check_add(Tasks *tasks, E_Client *ec)
{
   if (ec->user_skip_winlist) return 1;
   if (ec->netwm.state.skip_taskbar) return 1;
   if (ec->stack.prev) return 1;
   if (_tasks_item_find(tasks, ec)) return 1;
   if (!tasks->config) return 1;

   if (!(tasks->config->show_all_desktops))
     {
        if ((ec->desk != e_desk_current_get(ec->zone)) &&
            (!ec->sticky))
          return 1;
     }

   if (!(tasks->config->show_all_screens))
     {
        if (ec->zone != tasks->zone) return 1;
     }

   _tasks_item_add(tasks, ec);
   return 0;
}

static void
_tasks_item_add(Tasks *tasks, E_Client *ec)
{
   Tasks_Item *item;

   item = _tasks_item_new(tasks, ec);
   E_EXPAND(item->o_item);
   E_FILL(item->o_item);
   elm_box_pack_end(tasks->o_items, item->o_item);
   tasks->items = eina_list_append(tasks->items, item);
}

static void
_tasks_item_remove(Tasks_Item *item)
{
   item->tasks->items = eina_list_remove(item->tasks->items, item);
   elm_box_unpack(item->tasks->o_items, item->o_item);
   _tasks_item_free(item);
}

static void
_tasks_item_preview_del(Tasks_Item *item)
{
   _tasks_cb_timer_del(item);
}

static void
_tasks_item_free(Tasks_Item *item)
{
   if (item->o_icon)
     {
        evas_object_del(item->o_icon);
        item->o_icon = NULL;
     }
   if (e_object_is_del(E_OBJECT(item->client)))
     item->tasks->clients = eina_list_remove(item->tasks->clients, item->client);
   e_object_unref(E_OBJECT(item->client));
   _tasks_item_preview_del(item);
   if (item->o_item)
     {
        evas_object_del(item->o_item);
        item->o_item = NULL;
     }
   item->client = NULL;
   item->tasks = NULL;
   if (item->iconifying)
     item->delete_me = EINA_TRUE;
   else
     free(item);
}

static void
_tasks_item_refill(Tasks_Item *item)
{
   if (item->client->netwm.state.skip_taskbar != item->skip_taskbar)
     {
        _tasks_refill(item->tasks);
        return;
     }
   if (item->o_icon)
     {
        evas_object_del(item->o_icon);
        item->o_icon = NULL;
     }
   _tasks_item_fill(item);
}

static void
_tasks_focused_eval(Tasks_Item *item)
{
   if (!item) return;

   if (e_client_stack_focused_get(item->client) != item->focused)
     {
        item->focused = e_client_stack_focused_get(item->client);
        if (item->focused)
          _tasks_item_signal_emit(item, "e,state,focused", "e");
        else
          _tasks_item_signal_emit(item, "e,state,unfocused", "e");
     }
}

static void
_tasks_iconified_eval(Tasks_Item *item)
{
   if (!item) return;

   if (e_client_stack_iconified_get(item->client) != item->iconified)
     {
        item->iconified = e_client_stack_iconified_get(item->client);
        if (item->iconified)
          _tasks_item_signal_emit(item, "e,state,iconified", "e");
        else
          _tasks_item_signal_emit(item, "e,state,uniconified", "e");
     }
}

static void
_tasks_urgent_eval(Tasks_Item *item)
{
   if (!item) return;

   if (e_client_stack_urgent_get(item->client) != item->urgent)
     {
        item->urgent = e_client_stack_urgent_get(item->client);
        if (item->urgent)
          _tasks_item_signal_emit(item, "e,state,urgent", "e");
        else
          _tasks_item_signal_emit(item, "e,state,not_urgent", "e");
     }
}

static void
_tasks_item_fill(Tasks_Item *item)
{
   const char *label;
   E_Client *ec;

   ec = item->client;

   if (!item->tasks->config->text_only)
     {
        item->o_icon = e_client_icon_add(ec, evas_object_evas_get(item->tasks->o_items));
        edje_object_part_swallow(item->o_item, "e.swallow.icon", item->o_icon);
        evas_object_pass_events_set(item->o_icon, 1);
        evas_object_show(item->o_icon);
     }

   ec = e_client_stack_active_adjust(ec);

   if (item->tasks->config->icon_only) label = "";
   else label = e_client_util_name_get(ec);
   edje_object_part_text_set(item->o_item, "e.text.label", label);

   _tasks_iconified_eval(item);
   _tasks_focused_eval(item);
   _tasks_urgent_eval(item);
}

static void
_tasks_item_signal_emit(Tasks_Item *item, char *sig, char *src)
{
   if (item->o_item) edje_object_signal_emit(item->o_item, sig, src);
   if (item->o_icon && e_icon_edje_get(item->o_icon))
     e_icon_edje_emit(item->o_icon, sig, src);
}

static Config_Item *
_tasks_config_item_get(const char *id)
{
   Eina_List *l;
   Config_Item *config;
   char buf[128];

   if (!id)
     {
        int num = 0;

        /* Create id */
        if (tasks_config->items)
          {
             const char *p;

             config = eina_list_last(tasks_config->items)->data;
             p = strrchr(config->id, '.');
             if (p) num = atoi(p + 1) + 1;
          }
        snprintf(buf, sizeof(buf), "%s.%d", _gadcon_class.name, num);
        id = buf;
     }
   else
     {
        EINA_LIST_FOREACH(tasks_config->items, l, config)
          {
             if (!config->id) continue;
             if (!strcmp(config->id, id)) return config;
          }
     }

   config = E_NEW(Config_Item, 1);
   config->id = eina_stringshare_add(id);
   config->show_all_desktops = 0;
   config->show_all_screens = 0;
   config->minw = 100;
   config->minh = 32;

   tasks_config->items = eina_list_append(tasks_config->items, config);

   return config;
}

void
_tasks_config_updated(Config_Item *config)
{
   const Eina_List *l;
   Tasks *tasks;

   if (!tasks_config) return;
   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        if (tasks->config == config) _tasks_refill(tasks);
     }
}

static void
_tasks_cb_menu_configure(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   Tasks *tasks;

   tasks = (Tasks *)data;
   _config_tasks_module(tasks->config);
}

static void
_tasks_cb_item_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Tasks_Item *item = data;

   _tasks_item_preview_del(item);
}

static void
_tasks_cb_item_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Tasks_Item *item;

   item = (Tasks_Item *)data;
   ev = event_info;

   _tasks_item_preview_del(item);

   if (ev->button == 3)
     {
        E_Menu *m;
        E_Menu_Item *mi;
        int cx, cy, cw, ch;

        e_gadcon_canvas_zone_geometry_get(item->tasks->gcc->gadcon, &cx, &cy, &cw, &ch);

        e_int_client_menu_create(item->client);

        mi = e_menu_item_new(item->client->border_menu);
        e_menu_item_separator_set(mi, 1);

        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, _("Settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_menu_item_callback_set(mi, _tasks_cb_menu_configure, item->tasks);

        m = e_gadcon_client_util_menu_items_append(item->tasks->gcc, m, 0);

        mi = e_menu_item_new(item->client->border_menu);
        e_menu_item_label_set(mi, _("Tasks"));
        e_menu_item_submenu_set(mi, m);
        e_object_unref(E_OBJECT(m));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_gadcon_client_menu_set(item->tasks->gcc, item->client->border_menu);

        e_menu_activate_mouse(item->client->border_menu,
                              e_zone_current_get(),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(item->tasks->gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
        /* gadcon menu post cb won't remove this so we'll do it now to prevent problems later */
        item->client->border_menu = NULL;
     }
}

static Eina_Bool
_tasks_cb_timer_del(void *data)
{
   Tasks_Item *item = data;

   if (item->o_preview)
     evas_object_del(item->o_preview);
   if (item->timer)
     ecore_timer_del(item->timer);
   item->timer = item->o_preview = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_tasks_item_preview_add(Tasks_Item *item)
{
   E_Client *ec;
   Evas_Object *o, *ot, *or, *img;
   Evas_Coord ox, oy, ow, oh, size;
   double n;

   if (item->o_preview) _tasks_cb_timer_del(item);

   evas_object_geometry_get(item->o_item, &ox, &oy, &ow, &oh);

   item->o_preview = o = elm_ctxpopup_add(e_comp->elm);
   evas_object_pass_events_set(o, EINA_TRUE);
   elm_object_focus_allow_set(o, EINA_FALSE);
   elm_object_tree_focus_allow_set(o, EINA_FALSE);
   elm_object_style_set(o, "noblock");
   evas_object_layer_set(o, E_LAYER_POPUP);

   ot = elm_table_add(o);
   evas_object_size_hint_align_set(ot, 0, 0);
   elm_object_content_set(o, ot);
   evas_object_show(ot);

   ec = item->client;
   img = e_comp_object_util_mirror_add(ec->frame);
   evas_object_size_hint_aspect_set(img, EVAS_ASPECT_CONTROL_BOTH, ec->client.w, ec->client.h);
   evas_object_show(img);

   or = evas_object_rectangle_add(evas_object_evas_get(o));

   size = item->tasks->config->preview_size;
   if (ec->client.w > ec->client.h)
     {
        n = size * (1.0 / ec->client.w);
        evas_object_size_hint_min_set(img, size, n * ec->client.h);
        evas_object_size_hint_max_set(img, size, n * ec->client.h);
        evas_object_size_hint_min_set(or,  size + 1, (n * ec->client.h) + 1);
     }
   else
     {
        n = size * (1.0 / ec->client.h);
        evas_object_size_hint_min_set(img, n * ec->client.w, size);
        evas_object_size_hint_max_set(img, n * ec->client.w, size);
        evas_object_size_hint_min_set(or, (n * ec->client.w) + 1, size + 1);
     }

   elm_table_pack(ot, or, 0, 0, 1, 1);
   elm_table_pack(ot, img, 0, 0, 1, 1);

   switch (item->tasks->gcc->gadcon->orient)
     {
        case E_GADCON_ORIENT_BOTTOM:
        case E_GADCON_ORIENT_CORNER_BR:
        case E_GADCON_ORIENT_CORNER_BL:
          evas_object_move(o, ox + (ow / 2), oy);
          elm_ctxpopup_direction_priority_set(o, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_DOWN,
                                              ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_RIGHT);
          break;
        case E_GADCON_ORIENT_TOP:
        case E_GADCON_ORIENT_CORNER_TL:
        case E_GADCON_ORIENT_CORNER_TR:
          evas_object_move(o, ox + (ow / 2), oy + oh);
          elm_ctxpopup_direction_priority_set(o, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_UP,
                                              ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_RIGHT);
          break;
        case E_GADCON_ORIENT_LEFT:
        case E_GADCON_ORIENT_CORNER_LB:
        case E_GADCON_ORIENT_CORNER_LT:
          evas_object_move(o, ox + ow, oy + (oh / 2));
          elm_ctxpopup_direction_priority_set(o, ELM_CTXPOPUP_DIRECTION_RIGHT, ELM_CTXPOPUP_DIRECTION_DOWN,
                                              ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_UP);
          break;
        case E_GADCON_ORIENT_RIGHT:
        case E_GADCON_ORIENT_CORNER_RB:
        case E_GADCON_ORIENT_CORNER_RT:
          evas_object_move(o, ox, oy + (oh / 2));
          elm_ctxpopup_direction_priority_set(o, ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_DOWN,
                                              ELM_CTXPOPUP_DIRECTION_RIGHT, ELM_CTXPOPUP_DIRECTION_UP);
          break;
        default:
          evas_object_move(o, ox + (ow / 2), oy);
          elm_ctxpopup_direction_priority_set(o, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_DOWN,
                                              ELM_CTXPOPUP_DIRECTION_LEFT, ELM_CTXPOPUP_DIRECTION_RIGHT);
          break;
     }
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
}

static void
_tasks_cb_item_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Tasks_Item *item = data;
   Eina_Bool show;

   if (!item->tasks->config->preview) return;

   show = !item->iconifying;
   if ((item->tasks->gcc->gadcon->shelf) &&
       (item->tasks->gcc->gadcon->shelf->hide_animator))
     show = 0;

   if (show) _tasks_item_preview_add(item);
}

static void
_tasks_cb_item_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Tasks_Item *item = data;

   if (item->o_preview)
     {
        elm_ctxpopup_dismiss(item->o_preview);
        item->timer = ecore_timer_add(1.0, _tasks_cb_timer_del, item);
     }
}

static void
_tasks_cb_item_mouse_wheel(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Tasks_Item *item;

   ev = event_info;
   item = data;

   if (ev->z < 0)
     {
        if (item->client->iconic)
          e_client_uniconify(item->client);
        else
          evas_object_raise(item->client->frame);
        evas_object_focus_set(item->client->frame, 1);
     }
   else if (ev->z > 0)
     e_client_iconify(item->client);
}

static void
_tasks_cb_item_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Tasks_Item *item;

   ev = event_info;
   item = data;

   _tasks_item_preview_del(item);

   if (ev->button == 1)
     {
        if (!item->client->sticky && item->tasks->config->show_all_desktops)
          e_desk_show(item->client->desk);
        if (evas_key_modifier_is_set(ev->modifiers, "Alt"))
          {
             if (item->client->iconic)
               e_client_uniconify(item->client);
             else
               e_client_iconify(item->client);
          }
        else if (evas_key_modifier_is_set(ev->modifiers, "Control"))
          {
             if (item->client->maximized)
               e_client_unmaximize(item->client, e_config->maximize_policy);
             else
               e_client_maximize(item->client, e_config->maximize_policy);
          }
        else if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
          {
             if (item->client->shaded)
               e_client_unshade(item->client, item->client->shade_dir);
             else
               e_client_shade(item->client, item->client->shade_dir);
          }
        else if (evas_key_modifier_is_set(ev->modifiers, "Super"))
          {
             e_client_act_close_begin(item->client);
          }
        else if (!item->iconifying)
          {
             if (item->client->iconic)
               {
                  e_client_uniconify(item->client);
                  evas_object_focus_set(item->client->frame, 1);
               }
             else
               {
                  if (item->client->focused)
                    {
                       e_client_iconify(item->client);
                    }
                  else
                    {
                       evas_object_raise(item->client->frame);
                       evas_object_focus_set(item->client->frame, 1);
                    }
               }
          }
     }
   else if (ev->button == 2)
     {
        if (!item->client->sticky && item->tasks->config->show_all_desktops)
          e_desk_show(item->client->desk);
        evas_object_raise(item->client->frame);
        evas_object_focus_set(item->client->frame, 1);
        if (item->client->maximized)
          e_client_unmaximize(item->client, e_config->maximize_policy);
        else
          e_client_maximize(item->client, e_config->maximize_policy);
     }
}

/************ BORDER CALLBACKS *********************/

static Eina_Bool
_tasks_cb_event_client_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   Tasks *tasks;
   Eina_List *l;

   if (e_client_util_ignored_get(ev->ec) || e_object_is_del(E_OBJECT(ev->ec))) return ECORE_CALLBACK_RENEW;
   if (e_client_util_is_popup(ev->ec)) return ECORE_CALLBACK_RENEW;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        if ((!tasks->clients) || (!eina_list_data_find(tasks->clients, ev->ec)))
          tasks->clients = eina_list_append(tasks->clients, ev->ec);
     }
   _tasks_refill_all();
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_event_client_remove(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   Tasks *tasks;
   Eina_List *l;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        tasks->clients = eina_list_remove(tasks->clients, ev->ec);
     }
   _tasks_refill_all();
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_event_client_iconify(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   Eina_List *l;
   Tasks *tasks;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        _tasks_iconified_eval(_tasks_item_find(tasks, ev->ec));
     }
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_event_client_uniconify(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   Eina_List *l;
   Tasks *tasks;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        _tasks_iconified_eval(_tasks_item_find(tasks, ev->ec));
     }
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_window_focus_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   Eina_List *l;
   Tasks *tasks;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        _tasks_focused_eval(_tasks_item_find(tasks, ev->ec));
     }
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_window_focus_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client *ev = event;
   Eina_List *l;
   Tasks *tasks;

   EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
     {
        _tasks_focused_eval(_tasks_item_find(tasks, ev->ec));
     }
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_event_client_prop_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Client_Property *ev = event;
   Eina_List *l;
   Tasks *tasks;

   if ((ev->property & E_CLIENT_PROPERTY_URGENCY) == E_CLIENT_PROPERTY_URGENCY)
     {
        EINA_LIST_FOREACH(tasks_config->tasks, l, tasks)
          _tasks_urgent_eval(_tasks_item_find(tasks, ev->ec));
     }
   else if (((ev->property & E_CLIENT_PROPERTY_TITLE) == E_CLIENT_PROPERTY_TITLE) ||
            ((ev->property & E_CLIENT_PROPERTY_ICON) == E_CLIENT_PROPERTY_ICON))
     _tasks_refill_border(ev->ec);
   else if (ev->property & E_CLIENT_PROPERTY_MISC)
     _tasks_refill_all();
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_event_client_zone_set(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   _tasks_refill_all();
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_event_client_desk_set(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Client *ev)
{
   if (!ev->ec->sticky)
     _tasks_refill_all();
   return EINA_TRUE;
}

static Eina_Bool
_tasks_cb_event_desk_show(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   _tasks_refill_all();
   return EINA_TRUE;
}

/***************************************************************************/
/**/

/**/
/***************************************************************************/
