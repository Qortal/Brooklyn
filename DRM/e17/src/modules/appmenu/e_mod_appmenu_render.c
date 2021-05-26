#include "e_mod_appmenu_private.h"

static int menu_count = 0;
static E_Menu *menu_pending = NULL;
static Ecore_Timer *menu_timer = NULL;
static int menu_dir = 0;
static Evas_Coord menu_x = 0, menu_y = 0, menu_w = 0, menu_h = 0;

void
appmenu_cancel(void)
{
   if (menu_pending)
     {
        e_object_del(E_OBJECT(menu_pending));
        menu_pending = NULL;
     }
   if (menu_timer)
     {
        ecore_timer_del(menu_timer);
        menu_timer = NULL;
     }
}

int
appmenu_menu_count_get(void)
{
   return menu_count;
}

static void
item_activate(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED)
{
   menu_count++;
}

static void
item_deactivate(void *data EINA_UNUSED, E_Menu *m)
{
   if (menu_pending == m) menu_pending = NULL;
   menu_count--;
}

static void
menu_deactive(E_Menu *m)
{
   Eina_List *iter;
   E_Menu_Item *mi;
   E_DBusMenu_Item *item;

   item = e_object_data_get(E_OBJECT(m));
   if (item)
     {
        e_object_data_set(E_OBJECT(m), NULL);
        e_dbusmenu_item_unref(item);
     }
   EINA_LIST_FOREACH(m->items, iter, mi)
     {
        item = e_object_data_get(E_OBJECT(mi));
        if (item)
          {
             e_object_data_set(E_OBJECT(m), NULL);
             e_dbusmenu_item_unref(item);
          }
        if (mi->submenu)
          {
             menu_deactive(mi->submenu);
             e_menu_deactivate(mi->submenu);
          }
     }
   e_object_del(E_OBJECT(m));
}

static void
menu_post_deactivate(void *data, E_Menu *m)
{
   E_Gadcon *gadcon = data;

   item_deactivate(data, m);
   e_gadcon_locked_set(gadcon, 0);
   menu_deactive(m);
}

static void
sub_item_clicked_cb(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_DBusMenu_Item *item = data;
   e_dbusmenu_event_send(item, E_DBUSMENU_ITEM_EVENT_CLICKED);
}

static E_Menu *
item_submenu_new(E_DBusMenu_Item *item, E_Menu_Item *mi)
{
   E_Menu *m;
   E_DBusMenu_Item *child;

   m = e_menu_new();
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);
   e_dbusmenu_item_ref(item);
   e_object_data_set(E_OBJECT(m), item);
   if (mi) e_menu_item_submenu_set(mi, m);

   EINA_INLIST_FOREACH(item->sub_items, child)
     {
        E_Menu_Item *submi;
        if (!child->visible) continue;
        submi = e_menu_item_new(m);
        e_dbusmenu_item_ref(child);
        e_object_data_set(E_OBJECT(submi), child);
        if (child->type == E_DBUSMENU_ITEM_TYPE_SEPARATOR)
          {
             e_menu_item_separator_set(submi, 1);
             continue;
          }
        e_menu_item_label_set(submi, child->label);
        e_menu_item_callback_set(submi, sub_item_clicked_cb, child);
        if (!child->enabled) e_menu_item_disabled_set(submi, 1);
        if (child->toggle_type)
          {
             if (child->toggle_type == E_DBUSMENU_ITEM_TOGGLE_TYPE_CHECKMARK)
               e_menu_item_check_set(submi, 1);
             else if (child->toggle_type == E_DBUSMENU_ITEM_TOGGLE_TYPE_RADIO)
               e_menu_item_radio_set(submi, 1);
             e_menu_item_toggle_set(submi, child->toggle_state);
          }
        if (eina_inlist_count(child->sub_items))
          item_submenu_new(child, submi);
        e_util_menu_item_theme_icon_set(submi, child->icon_name);
     }
   e_menu_pre_activate_callback_set(m, item_activate, item);
   e_menu_post_deactivate_callback_set(m, item_deactivate, item);
   return m;
}

static Eina_Bool
item_menu_delay(void *data EINA_UNUSED)
{
   Evas_Coord x, y, w, h;
   E_Zone *zone;

   x = menu_x;
   y = menu_y;
   w = menu_w;
   h = menu_h;
   zone = e_comp_zone_xy_get(x + (w / 2), y + (h / 2));
   e_menu_activate_mouse(menu_pending, zone, x, y, w, h, menu_dir, 0);
   menu_pending = NULL;
   menu_timer = NULL;
   return EINA_FALSE;
}

static void
item_menu_open(E_DBusMenu_Item *item, E_Gadcon *gadcon, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   E_Menu *m = item_submenu_new(item, NULL);
   int dir = E_MENU_POP_DIRECTION_AUTO;

   EINA_SAFETY_ON_NULL_RETURN(m);
   e_gadcon_locked_set(gadcon, 1);
   e_menu_post_deactivate_callback_set(m, menu_post_deactivate, gadcon);

   if      ((gadcon->orient == E_GADCON_ORIENT_TOP) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_TL) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_TR))
     dir = E_MENU_POP_DIRECTION_DOWN;
   else if ((gadcon->orient == E_GADCON_ORIENT_BOTTOM) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_BL) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_BR))
     dir = E_MENU_POP_DIRECTION_UP;
   else if ((gadcon->orient == E_GADCON_ORIENT_LEFT) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_LT) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_LB))
     dir = E_MENU_POP_DIRECTION_RIGHT;
   else if ((gadcon->orient == E_GADCON_ORIENT_RIGHT) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_RT) ||
            (gadcon->orient == E_GADCON_ORIENT_CORNER_RB))
     dir = E_MENU_POP_DIRECTION_LEFT;

   if (menu_pending) e_object_del(E_OBJECT(menu_pending));
   menu_pending = m;
   menu_dir = dir;
   menu_x = x;
   menu_y = y;
   menu_w = w;
   menu_h = h;
   if (menu_timer) ecore_timer_del(menu_timer);
   menu_timer = ecore_timer_loop_add(0.33, item_menu_delay, NULL);
}

static void
clicked_toolbar_item(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   E_DBusMenu_Item *item = data;
   E_Gadcon *gadcon = evas_object_data_get(obj, "gadcon");
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   item_menu_open(item, gadcon, x, y, w, h);
}

void
appmenu_menu_of_instance_render(E_AppMenu_Instance *inst, E_AppMenu_Window *window)
{
   E_DBusMenu_Item *child;
   Evas_Coord w, h, sum_w = 0, sum_h = 0, padding = 0;
   Eina_List *l;
   Evas_Object *item;

   l = evas_object_box_children_get(inst->box);
   EINA_LIST_FREE(l, item)
     evas_object_del(item);
   e_gadcon_client_min_size_set(inst->gcc, 0, 0);

   if (!window || !window->root_item)
     return;

   EINA_INLIST_FOREACH(window->root_item->sub_items, child)
     {
        if (!child->label)
          continue;

        item = edje_object_add(inst->evas);
        e_theme_edje_object_set(item, "base/themes", "e/modules/appmenu/item");
        edje_object_part_text_set(item, "text", child->label);
        edje_object_size_min_calc(item, &w, &h);

        if (!padding)
          {
             const char *padding_txt;
             if (inst->orientation_horizontal)
               padding_txt = edje_object_data_get(item, "padding_horizontal");
             else
               padding_txt = edje_object_data_get(item, "padding_vertical");
             padding = atoi(padding_txt);
          }

        if (inst->orientation_horizontal)
          {
             h = inst->gcc->gadcon->shelf->h;
             w = w + padding;
             sum_w = sum_w + w;
          }
        else
          {
             w = inst->gcc->gadcon->shelf->w;
             h = h + padding;
             sum_h = sum_h + h;
          }
        evas_object_resize(item, w, h);
        evas_object_size_hint_min_set(item, w, h);
        evas_object_box_append(inst->box, item);
        evas_object_show(item);
        evas_object_data_set(item, "gadcon", inst->gcc->gadcon);
        evas_object_event_callback_add(item, EVAS_CALLBACK_MOUSE_DOWN,
                                       clicked_toolbar_item, child);
     }
   e_gadcon_client_min_size_set(inst->gcc, sum_w, sum_h);
}

void
appmenu_menu_render(E_AppMenu_Context *ctxt, E_AppMenu_Window *window)
{
   Eina_List *list;
   E_AppMenu_Instance *inst;

   appmenu_cancel();

   ctxt->window = window;

   EINA_LIST_FOREACH(ctxt->instances, list, inst)
     appmenu_menu_of_instance_render(inst, window);
}
