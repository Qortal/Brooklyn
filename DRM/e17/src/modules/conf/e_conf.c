#include "e.h"

typedef struct _E_Configure          E_Configure;
typedef struct _E_Configure_CB       E_Configure_CB;
typedef struct _E_Configure_Category E_Configure_Category;
typedef struct _E_Configure_Item     E_Configure_Item;

#define E_CONFIGURE_TYPE 0xE0b01014

struct _E_Configure
{
   E_Object             e_obj_inherit;

   Evas_Object         *win;
   Evas                *evas;
   Evas_Object         *edje;

   Evas_Object         *o_list;
   Evas_Object         *cat_list;
   Evas_Object         *item_list;
   Evas_Object         *close;

   Eina_List           *cats;
   Ecore_Event_Handler *mod_hdl;
};

struct _E_Configure_CB
{
   E_Configure *eco;
   const char  *path;
};

struct _E_Configure_Category
{
   E_Configure *eco;
   const char  *label;
   Eina_List   *items;
};

struct _E_Configure_Item
{
   E_Configure_CB *cb;
   const char     *label;
   const char     *icon_file;
   const char     *icon;
};

static void                  _e_configure_free(E_Configure *eco);
static void                  _e_configure_cb_del_req(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void                  _e_configure_cb_close(void *data, void *data2);
static E_Configure_Category *_e_configure_category_add(E_Configure *eco, const char *label, const char *icon_file, const char *icon);
static void                  _e_configure_category_cb(void *data, void *data2);
static void                  _e_configure_item_add(E_Configure_Category *cat, const char *label, const char *icon_file, const char *icon, const char *path);
static void                  _e_configure_item_cb(void *data);
static void                  _e_configure_focus_cb(void *data, Evas_Object *obj);
static void                  _e_configure_keydown_cb(void *data, Evas *e, Evas_Object *obj, void *event);
static void                  _e_configure_fill_cat_list(void *data, const char *sel);
static Eina_Bool             _e_configure_module_update_cb(void *data, int type, void *event);

static E_Configure *_e_configure = NULL;

void
e_configure_show(const char *params)
{
   E_Configure *eco;
   Evas_Object *o;
   Evas_Modifier_Mask mask;
   Eina_Bool kg;
   E_Client *ec;

   if (_e_configure)
     {
        const Eina_List *l;
        void *it;
        int x = 0;

        eco = _e_configure;
        ec = e_win_client_get(eco->win);
        ec->hidden = 0;
        e_client_desk_set(ec, e_desk_current_get(ec->zone));
        evas_object_show(eco->win);
        elm_win_raise(eco->win);
        e_client_unshade(ec, ec->shade_dir);
        if ((e_config->focus_setting == E_FOCUS_NEW_DIALOG) ||
            (e_config->focus_setting == E_FOCUS_NEW_WINDOW) ||
            (e_config->focus_setting == E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED))
          e_client_focus_set_with_pointer(ec);
        EINA_LIST_FOREACH(e_widget_toolbar_items_get(eco->cat_list), l, it)
          {
             if (e_widget_toolbar_item_label_get(it) == params)
               {
                  e_widget_toolbar_item_select(eco->cat_list, x);
                  break;
               }
             x++;
          }
        return;
     }

   eco = E_OBJECT_ALLOC(E_Configure, E_CONFIGURE_TYPE, _e_configure_free);
   if (!eco) return;
   eco->win = elm_win_add(NULL, NULL, ELM_WIN_BASIC);
   if (!eco->win)
     {
        free(eco);
        return;
     }
   evas_object_data_set(eco->win, "e_conf_win", eco);
   eco->evas = evas_object_evas_get(eco->win);

   /* Event Handler for Module Updates */
   eco->mod_hdl = ecore_event_handler_add(E_EVENT_MODULE_UPDATE,
                                          _e_configure_module_update_cb, eco);

   elm_win_title_set(eco->win, _("Settings"));
   ecore_evas_name_class_set(e_win_ee_get(eco->win), "E", "_configure");
   evas_object_event_callback_add(eco->win, EVAS_CALLBACK_DEL, _e_configure_cb_del_req, eco);
   elm_win_center(eco->win, 1, 1);

   eco->edje = elm_layout_add(e_win_evas_win_get(eco->evas));
   E_EXPAND(eco->edje);
   E_FILL(eco->edje);
   elm_win_resize_object_add(eco->win, eco->edje);
   e_theme_edje_object_set(eco->edje, "base/theme/configure",
                           "e/widgets/configure/main");
   elm_object_part_text_set(eco->edje, "e.text.title", _("Settings"));

   eco->o_list = e_widget_list_add(eco->evas, 0, 0);
   elm_object_part_content_set(eco->edje, "e.swallow.content", eco->o_list);

   /* Event Obj for keydown */
   o = evas_object_rectangle_add(eco->evas);
   mask = 0;
   kg = evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Tab\" key events to object %p.\n", o);
   mask = evas_key_modifier_mask_get(evas_object_evas_get(eco->win), "Shift");
   kg = evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Tab\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "Return", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Return\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "KP_Enter", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"KP_Enter\" key events to object %p.\n", o);
   mask = 0;
   kg = evas_object_key_grab(o, "Escape", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Escape\" key events to object %p.\n", o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN,
                                  _e_configure_keydown_cb, eco);

   _e_configure_fill_cat_list(eco, params);

   /* Close Button */
   eco->close = e_widget_button_add(eco->evas, _("Close"), NULL,
                                    _e_configure_cb_close, eco, NULL);
   e_widget_on_focus_hook_set(eco->close, _e_configure_focus_cb, eco);
   elm_object_part_content_set(eco->edje, "e.swallow.button", eco->close);
   e_util_win_auto_resize_fill(eco->win);

   evas_object_show(eco->edje);
   evas_object_show(eco->win);
   e_win_client_icon_set(eco->win, "preferences-system");

   /* Preselect "Appearance" */
   e_widget_focus_set(eco->cat_list, 1);
   if (!e_widget_toolbar_item_selected_get(eco->cat_list))
     e_widget_toolbar_item_select(eco->cat_list, 0);

   _e_configure = eco;
}

void
e_configure_del(void)
{
   if (_e_configure) e_object_del(E_OBJECT(_e_configure));
   _e_configure = NULL;
}

static void
_e_configure_free(E_Configure *eco)
{
   if (_e_configure->mod_hdl)
     ecore_event_handler_del(_e_configure->mod_hdl);
   _e_configure->mod_hdl = NULL;
   _e_configure = NULL;
   while (eco->cats)
     {
        E_Configure_Category *cat;

        if (!(cat = eco->cats->data)) return;
        if (cat->label) eina_stringshare_del(cat->label);

        while (cat->items)
          {
             E_Configure_Item *ci;

             if (!(ci = cat->items->data)) continue;
             if (ci->label) eina_stringshare_del(ci->label);
             if (ci->icon_file) eina_stringshare_del(ci->icon_file);
             if (ci->icon) eina_stringshare_del(ci->icon);
             if (ci->cb)
               {
                  if (ci->cb->path) eina_stringshare_del(ci->cb->path);
                  free(ci->cb);
               }
             cat->items = eina_list_remove_list(cat->items, cat->items);
             E_FREE(ci);
          }
        eco->cats = eina_list_remove_list(eco->cats, eco->cats);
        E_FREE(cat);
     }
//   evas_object_del(eco->close);
//   evas_object_del(eco->cat_list);
//   evas_object_del(eco->item_list);
//   evas_object_del(eco->o_list);
//   evas_object_del(eco->edje);
   evas_object_del(eco->win);
   free(eco);
}

static void
_e_configure_cb_del_req(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   e_object_del(E_OBJECT(data));
}

static void
_e_configure_cb_close(void *data, void *data2 EINA_UNUSED)
{
   E_Configure *eco;

   if (!(eco = data)) return;
   e_util_defer_object_del(E_OBJECT(eco));
}

static E_Configure_Category *
_e_configure_category_add(E_Configure *eco, const char *label, const char *icon_file, const char *icon)
{
   Evas_Object *o = NULL;
   E_Configure_Category *cat;

   if (!eco) return NULL;
   if (!label) return NULL;

   cat = E_NEW(E_Configure_Category, 1);
   cat->eco = eco;
   cat->label = eina_stringshare_add(label);
   if (icon)
     {
        o = e_icon_add(eco->evas);
        if (icon_file)
          e_icon_file_edje_set(o, icon_file, icon);
        else if (!e_util_icon_theme_set(o, icon))
          {
             evas_object_del(o);
             o = e_util_icon_add(icon, eco->evas);
          }
     }
   eco->cats = eina_list_append(eco->cats, cat);

   e_widget_toolbar_item_append(eco->cat_list, o, label,
                                _e_configure_category_cb, cat, NULL);
   return cat;
}

static void
_e_configure_category_cb(void *data, void *data2 EINA_UNUSED)
{
   E_Configure_Category *cat;
   E_Configure *eco;
   Eina_List *l;
   Evas_Coord w, h;

   if (!(cat = data)) return;
   eco = cat->eco;

   evas_event_freeze(evas_object_evas_get(eco->item_list));
   edje_freeze();
   e_widget_ilist_freeze(eco->item_list);
   e_widget_ilist_clear(eco->item_list);
   /***--- fill ---***/
   for (l = cat->items; l; l = l->next)
     {
        E_Configure_Item *ci;
        Evas_Object *o = NULL;

        if (!(ci = l->data)) continue;
        if (ci->icon)
          {
             o = e_icon_add(eco->evas);
             if (ci->icon_file)
               e_icon_file_edje_set(o, ci->icon_file, ci->icon);
             else if (!e_util_icon_theme_set(o, ci->icon))
               {
                  evas_object_del(o);
                  o = e_util_icon_add(ci->icon, eco->evas);
               }
          }
        e_widget_ilist_append(eco->item_list, o, ci->label,
                              _e_configure_item_cb, ci, NULL);
     }
   e_widget_ilist_go(eco->item_list);
   e_widget_size_min_get(eco->item_list, &w, &h);
   e_widget_size_min_set(eco->item_list, w, h);
   e_widget_ilist_thaw(eco->item_list);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(eco->item_list));
}

static void
_e_configure_item_add(E_Configure_Category *cat, const char *label, const char *icon_file, const char *icon, const char *path)
{
   E_Configure_Item *ci;
   E_Configure_CB *cb;

   if ((!cat) || (!label)) return;

   ci = E_NEW(E_Configure_Item, 1);
   cb = E_NEW(E_Configure_CB, 1);
   cb->eco = cat->eco;
   cb->path = eina_stringshare_add(path);
   ci->cb = cb;
   ci->label = eina_stringshare_add(label);
   if (icon_file) ci->icon_file = eina_stringshare_add(icon_file);
   if (icon) ci->icon = eina_stringshare_add(icon);
   if ((!ci->icon_file) && (!ci->icon))
     ci->icon = eina_stringshare_add("unknown");
   cat->items = eina_list_append(cat->items, ci);
}

static void
_e_configure_item_cb(void *data)
{
   E_Configure_Item *ci;
   E_Configure_CB *cb;

   if (!(ci = data)) return;
   cb = ci->cb;
   if (cb->path) e_configure_registry_call(cb->path, NULL, NULL);
}

static void
_e_configure_focus_cb(void *data, Evas_Object *obj)
{
   E_Configure *eco = data;

   if (obj == eco->close)
     {
        e_widget_focused_object_clear(eco->item_list);
        e_widget_focused_object_clear(eco->cat_list);
     }
   else if (obj == eco->item_list)
     {
        e_widget_focused_object_clear(eco->cat_list);
        e_widget_focused_object_clear(eco->close);
     }
   else if (obj == eco->cat_list)
     {
        e_widget_focused_object_clear(eco->item_list);
        e_widget_focused_object_clear(eco->close);
     }
}

static void
_e_configure_keydown_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Evas_Event_Key_Down *ev;
   E_Configure *eco = data;

   ev = event;

   if (!strcmp(ev->key, "Tab"))
     {
        if (evas_key_modifier_is_set(evas_key_modifier_get(evas_object_evas_get(eco->win)), "Shift"))
          {
             if (e_widget_focus_get(eco->close))
               e_widget_focus_set(eco->item_list, 0);
             else if (e_widget_focus_get(eco->item_list))
               e_widget_focus_set(eco->cat_list, 1);
             else if (e_widget_focus_get(eco->cat_list))
               e_widget_focus_set(eco->close, 0);
          }
        else
          {
             if (e_widget_focus_get(eco->close))
               e_widget_focus_set(eco->cat_list, 1);
             else if (e_widget_focus_get(eco->item_list))
               e_widget_focus_set(eco->close, 0);
             else if (e_widget_focus_get(eco->cat_list))
               e_widget_focus_set(eco->item_list, 0);
          }
     }
   else if (((!strcmp(ev->key, "Return")) ||
             (!strcmp(ev->key, "KP_Enter")) ||
             (!strcmp(ev->key, "space"))))
     {
        Evas_Object *o = NULL;

        if (e_widget_focus_get(eco->cat_list))
          o = eco->cat_list;
        else if (e_widget_focus_get(eco->item_list))
          o = eco->item_list;
        else if (e_widget_focus_get(eco->close))
          o = eco->close;

        if (o)
          {
             if (!(o = e_widget_focused_object_get(o))) return;
             e_widget_activate(o);
          }
     }
   else if (!strcmp(ev->key, "Escape"))
     e_widget_activate(eco->close);
}

static void
_e_configure_fill_cat_list(void *data, const char *sel)
{
   E_Configure *eco;
   Evas_Coord mw, mh;
   E_Configure_Category *cat;
   Eina_List *l, *ll;
   E_Configure_Cat *ecat;
   int num = -1;

   if (!(eco = data)) return;

   if (eco->cat_list) evas_object_del(eco->cat_list);
   if (eco->item_list) evas_object_del(eco->item_list);
   eco->cat_list = NULL;
   eco->item_list = NULL;

   /* Category List */
   eco->cat_list = e_widget_toolbar_add(eco->evas, 40 * e_scale, 40 * e_scale);
   e_widget_toolbar_scrollable_set(eco->cat_list, 1);
   /***--- fill ---***/
   EINA_LIST_FOREACH(e_configure_registry, l, ecat)
     {
        if ((ecat->pri >= 0) && (ecat->items))
          {
             E_Configure_It *eci;
             cat = _e_configure_category_add(eco, _(ecat->label),
                                             ecat->icon_file, ecat->icon);
             EINA_LIST_FOREACH(ecat->items, ll, eci)
               {
                  char buf[1024];

                  if (eci->pri >= 0)
                    {
                       snprintf(buf, sizeof(buf), "%s/%s", ecat->cat,
                                eci->item);
                       _e_configure_item_add(cat, _(eci->label),
                                             eci->icon_file, eci->icon, buf);
                    }
               }
             if (sel && (ecat->cat == sel))
               num = e_widget_toolbar_items_count(eco->cat_list) - 1;
          }
     }
   e_widget_on_focus_hook_set(eco->cat_list, _e_configure_focus_cb, eco);
   e_widget_list_object_append(eco->o_list, eco->cat_list, 1, 0, 0.5);

   /* Item List */
   eco->item_list = e_widget_ilist_add(eco->evas, 40 * e_scale, 40 * e_scale, NULL);
   e_widget_ilist_selector_set(eco->item_list, 1);
   e_widget_ilist_go(eco->item_list);
   e_widget_on_focus_hook_set(eco->item_list, _e_configure_focus_cb, eco);
   e_widget_size_min_get(eco->item_list, &mw, &mh);
   if (mw < (200 * e_scale)) mw = 200 * e_scale;
   if (mh < (120 * e_scale)) mh = 120 * e_scale;
   e_widget_size_min_set(eco->item_list, mw, mh);
   e_widget_list_object_append(eco->o_list, eco->item_list, 1, 1, 0.5);
   if (num != -1) e_widget_toolbar_item_select(eco->cat_list, num);
}

static Eina_Bool
_e_configure_module_update_cb(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   E_Configure *eco;
   int sel = 0;

   if (!(eco = data)) return ECORE_CALLBACK_PASS_ON;
   if (!eco->cat_list) return ECORE_CALLBACK_PASS_ON;
   sel = e_widget_toolbar_item_selected_get(eco->cat_list);
   _e_configure_fill_cat_list(eco, NULL);
   e_widget_toolbar_item_select(eco->cat_list, sel);
   return ECORE_CALLBACK_PASS_ON;
}

