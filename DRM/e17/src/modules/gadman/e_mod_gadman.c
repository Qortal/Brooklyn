#include "e_mod_gadman.h"

/* local protos */
static void             _attach_menu(void *data, E_Gadcon_Client *gcc, E_Menu *menu);
static void             _save_widget_position(E_Gadcon_Client *gcc);
static void             _apply_widget_position(E_Gadcon_Client *gcc);
static E_Gadcon_Client *_gadman_gadget_add(const E_Gadcon_Client_Class *cc, Gadman_Layer_Type layer, E_Config_Gadcon_Client *src_cf);
static Eina_Bool _gadman_module_init_end_cb(void *d EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED);
static Evas_Object     *_create_mover(E_Gadcon *gc);
static Evas_Object     *_get_mover(E_Gadcon_Client *gcc);
static E_Gadcon        *_gadman_gadcon_new(const char *name, Gadman_Layer_Type layer, E_Zone *zone, E_Gadcon_Location *loc);
static void _gadman_overlay_create(void);
static void             on_top(void *data, Evas_Object *o, const char *em, const char *src);
static void             on_right(void *data, Evas_Object *o, const char *em, const char *src);
static void             on_down(void *data, Evas_Object *o, const char *em, const char *src);
static void             on_left(void *data, Evas_Object *o, const char *em, const char *src);
static void             on_move(void *data, Evas_Object *o, const char *em, const char *src);

static void             on_frame_click(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void             on_bg_click(void *data, Evas_Object *o, const char *em, const char *src);
static void             on_hide_stop(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED,
                                     const char *em EINA_UNUSED, const char *src EINA_UNUSED);

static void             on_menu_style_plain(void *data, E_Menu *m, E_Menu_Item *mi);
static void             on_menu_style_inset(void *data, E_Menu *m, E_Menu_Item *mi);
static void             on_menu_style_float(void *data, E_Menu *m, E_Menu_Item *mi);
static void             on_menu_style_horiz(void *data, E_Menu *m, E_Menu_Item *mi);
static void             on_menu_style_vert(void *data, E_Menu *m, E_Menu_Item *mi);
static void             on_menu_delete(void *data, E_Menu *m, E_Menu_Item *mi);
static void             on_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi);
static void             on_menu_add(void *data, E_Menu *m, E_Menu_Item *mi);

static Eina_Bool       _gadman_module_cb(void *d EINA_UNUSED, int type EINA_UNUSED, E_Event_Module_Update *ev);
static int              _e_gadman_client_add(void *data EINA_UNUSED, E_Gadcon_Client *, const E_Gadcon_Client_Class *cc);
static void             _e_gadman_client_remove(void *data EINA_UNUSED, E_Gadcon_Client *gcc);

static void             _e_gadman_handlers_add(void);
static void             _e_gadman_handler_del(void);
static Eina_Bool        _e_gadman_cb_zone_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static E_Gadcon_Client *gadman_gadget_place(E_Gadcon_Client *gcc, const E_Gadcon_Client_Class *cc, E_Config_Gadcon_Client *cf, Gadman_Layer_Type layer, E_Zone *zone);

static E_Gadcon        *gadman_gadcon_get(const E_Zone *zone, Gadman_Layer_Type layer);
static Eina_Bool gadman_locked;
Manager *Man = NULL;
static Eina_List *_gadman_hdls = NULL;
static Eina_Hash *_gadman_gadgets = NULL;
static Ecore_Job *gadman_reset_job = NULL;

/* for locking geometry during our own move/resize */
static Eina_Bool mover_lock = EINA_FALSE;

/* Implementation */
void
gadman_reset(void *d EINA_UNUSED)
{
   E_Gadcon *gc;
   unsigned int layer;
   const Eina_List *l;
   E_Zone *zone;

   E_FREE_FUNC(gadman_reset_job, ecore_job_del);
   if (gadman_locked) return;
   evas_event_freeze(e_comp->evas);
   for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
     {
        E_FREE_LIST(Man->gadcons[layer], e_object_del);
        Man->gadgets[layer] = eina_list_free(Man->gadgets[layer]);
        E_FREE_FUNC(Man->movers[layer], evas_object_del);
     }
   evas_object_hide(Man->overlay);
   E_FREE_FUNC(Man->overlay, evas_object_del);
   E_FREE_FUNC(Man->full_bg, evas_object_del);
   if (_gadman_gadgets)
     {
        eina_hash_free_cb_set(_gadman_gadgets, (void *)eina_list_free);
        eina_hash_free(_gadman_gadgets);
     }
   /* iterating through zones - and making gadmans on each */
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        const char *layer_name[] = {"gadman", "gadman_top"};

        for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
          {
             gc = _gadman_gadcon_new(layer_name[layer], layer, zone, Man->location[layer]);
             Man->gadcons[layer] = eina_list_append(Man->gadcons[layer], gc);
          }
     }
   _gadman_overlay_create();

   _gadman_gadgets = eina_hash_string_superfast_new(NULL);
   gadman_update_bg();
   Man->visible = !Man->visible;
   {
      int prev = Man->conf->anim_bg;
      Man->conf->anim_bg = 0;
      gadman_gadgets_toggle();
      Man->conf->anim_bg = prev;
   }
   edje_object_message_signal_process(Man->full_bg);
   evas_event_thaw(e_comp->evas);
}

void
gadman_init(E_Module *m)
{
   E_Gadcon_Location *location;

   /* Create Manager */
   Man = calloc(1, sizeof(Manager));
   if (!Man) return;

   Man->module = m;
   gadman_locked = e_module_loading_get();
   Man->width = e_comp->w;
   Man->height = e_comp->h;

   /* create and register "desktop" location */
   location = Man->location[GADMAN_LAYER_BG] = e_gadcon_location_new(_("Desktop"), E_GADCON_SITE_DESKTOP,
                                    _e_gadman_client_add, (intptr_t*)(long)GADMAN_LAYER_BG,
                                    _e_gadman_client_remove, NULL);
   e_gadcon_location_set_icon_name(location, "preferences-desktop");
   e_gadcon_location_register(location);

   /* create and register "desktop hover" location */
   location = Man->location[GADMAN_LAYER_TOP] = e_gadcon_location_new(_("Desktop Overlay"), E_GADCON_SITE_DESKTOP,
                                    _e_gadman_client_add, (intptr_t*)(long)GADMAN_LAYER_TOP,
                                    _e_gadman_client_remove, NULL);
   e_gadcon_location_set_icon_name(location, "preferences-desktop");
   e_gadcon_location_register(location);

   _e_gadman_handlers_add();
   if (!gadman_locked) gadman_reset_job = ecore_job_add(gadman_reset, NULL);
}

void
gadman_shutdown(void)
{
   unsigned int layer;

   E_FREE_FUNC(gadman_reset_job, ecore_job_del);
   _e_gadman_handler_del();

   gadman_gadget_edit_end(NULL, NULL, NULL, NULL);

   for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
     {
        e_gadcon_location_unregister(Man->location[layer]);
        E_FREE_LIST(Man->gadcons[layer], e_object_del);
        evas_object_del(Man->movers[layer]);
        Man->gadgets[layer] = eina_list_free(Man->gadgets[layer]);
        e_gadcon_location_free(Man->location[layer]);
     }

   eina_stringshare_del(Man->icon_name);

   /* free manager */
   evas_object_hide(Man->overlay);
   E_FREE_FUNC(Man->overlay, evas_object_del);
   if (_gadman_gadgets)
     {
        eina_hash_free_cb_set(_gadman_gadgets, (void *)eina_list_free);
        eina_hash_free(_gadman_gadgets);
     }
   _gadman_gadgets = NULL;
   free(Man);
}

void
gadman_populate_class(void *data, E_Gadcon *gc, const E_Gadcon_Client_Class *cc)
{
   Gadman_Layer_Type layer = (Gadman_Layer_Type)(long)data;
   const Eina_List *l, *ll;
   E_Config_Gadcon_Client *cf_gcc;
   E_Gadcon_Client *gcc = NULL;

   EINA_LIST_FOREACH(gc->cf->clients, l, cf_gcc)
     {
        if ((!strcmp(cf_gcc->name, cc->name)) && (gc->cf->zone == gc->zone->num))
          {
             gcc = e_gadcon_client_find(gc, cf_gcc);
             ll = eina_hash_find(_gadman_gadgets, cc->name);
             if ((!gcc) || (ll && (!eina_list_data_find(ll, cf_gcc))))
               gadman_gadget_place(gcc, cc, cf_gcc, layer, gc->zone);
          }
     }
   gc->populated_classes = eina_list_append(gc->populated_classes, cc);
}

static E_Gadcon *
gadman_gadcon_get(const E_Zone *zone, Gadman_Layer_Type layer)
{
   const Eina_List *l;
   E_Gadcon *gc;

   EINA_LIST_FOREACH(Man->gadcons[layer], l, gc)
     if (gc->zone == zone) return gc;
   return NULL;
}

static void
gadman_gadcon_place_job(E_Gadcon_Client *gcc)
{
   _apply_widget_position(gcc);
   if (gcc == gcc->gadcon->drag_gcc)
     gadman_gadget_edit_start(gcc);
   if (Man->visible || (!eina_list_data_find(Man->gadcons[GADMAN_LAYER_TOP], gcc->gadcon)))
     e_gadcon_client_show(gcc);
   else
     e_gadcon_client_hide(gcc);
}

static void
_gadman_gadget_free(void *data EINA_UNUSED, void *obj)
{
   E_Gadcon_Client *gcc = obj;
   Eina_List *l;
   int layer;
   Eina_Bool edit;

   layer = gcc->gadcon->id - ID_GADMAN_LAYER_BASE;
//   edje_object_part_unswallow(gcc->o_frame, gcc->o_base);
   if (gcc->cf)
     {
        Man->gadgets[layer] = eina_list_remove(Man->gadgets[layer], gcc->cf);
        l = eina_hash_find(_gadman_gadgets, gcc->name);
        if (l)
          {
             eina_hash_set(_gadman_gadgets, gcc->name, eina_list_remove(l, gcc->cf));
          }
        gcc->cf = NULL;
     }
   edit = (gcc == gcc->gadcon->drag_gcc) || (Man->drag_gcc[gcc->gadcon->id - ID_GADMAN_LAYER_BASE] == gcc);
   if (!edit) return;
   gcc->gadcon->editing = 1; // may get unset from a dialog, force here to prevent crash
   gadman_gadget_edit_end(NULL, NULL, NULL, NULL);
}

static void
_gadman_gadget_size_hints_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Gadcon_Client *gcc = data;
   int w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   /* size hints change for either min or aspect */
   if (gcc->min.w || gcc->min.h)
     {
        w = MAX(w, gcc->min.w);
        h = MAX(h, gcc->min.h);
     }
   if (gcc->aspect.w && gcc->aspect.h)
     {
        w = MAX(w, gcc->aspect.w);
        h = MAX(h, gcc->aspect.h);
     }
   evas_object_resize(gcc->o_frame, w, h);
   _save_widget_position(gcc);
}

static E_Gadcon_Client *
gadman_gadget_place(E_Gadcon_Client *gcc, const E_Gadcon_Client_Class *cc, E_Config_Gadcon_Client *cf, Gadman_Layer_Type layer, E_Zone *zone)
{
   E_Gadcon *gc;
   Eina_List *l;

   if (!cf->name) return NULL;

   gc = gadman_gadcon_get(zone, layer);
   if (!cc)
     {
        EINA_LIST_FOREACH(gc->populated_classes, l, cc)
          {
             if (!strcmp(cc->name, cf->name))
               break;
             else
               cc = NULL;
          }
     }

   /* Find provider */
   if (!cc)
     {
        e_gadcon_client_queue(gc, cf);
        e_gadcon_custom_populate_request(gc);
        return NULL;
     }

   /* init Gadcon_Client */
   if (!gcc)
     {
        gcc = cc->func.init(gc, cf->name, cf->id, cc->default_style);
        if (!gcc) return NULL;
        e_object_delfn_add(E_OBJECT(gcc), _gadman_gadget_free, NULL);
        gcc->cf = cf;
        gcc->client_class = cc;

        /* Call the client orientation function */
        if (cc->func.orient)
          cc->func.orient(gcc, gcc->cf->orient);
     }

   Man->gadgets[layer] = eina_list_append(Man->gadgets[layer], cf);

   //printf("Place Gadget %s (style: %s id: %s) (gadcon: %s)\n", gcc->name, cf->style, cf->id, gc->name);

   /* create frame */
   gcc->o_frame = edje_object_add(gc->evas);
   e_theme_edje_object_set(gcc->o_frame, "base/theme/gadman", "e/gadman/frame");

   if ((cf->style) && (!strcmp(cf->style, E_GADCON_CLIENT_STYLE_INSET)))
     edje_object_signal_emit(gcc->o_frame, "e,state,visibility,inset", "e");
   else
     edje_object_signal_emit(gcc->o_frame, "e,state,visibility,plain", "e");

   /* swallow the client inside the frame */
   edje_object_part_swallow(gcc->o_frame, "e.swallow.content", gcc->o_base);
   evas_object_event_callback_add(gcc->o_frame, EVAS_CALLBACK_MOUSE_DOWN,
                                  on_frame_click, gcc);
   evas_object_event_callback_add(gcc->o_frame, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _gadman_gadget_size_hints_cb, gcc);

   gcc->hidden = 1;
   if (gcc->gadcon->id == ID_GADMAN_LAYER_TOP)
     {
        edje_object_signal_emit(gcc->o_frame, "e,state,visibility,hide", "e");
        evas_object_layer_set(gcc->o_frame, E_LAYER_POPUP);
     }
   else
     evas_object_layer_set(gcc->o_frame, E_LAYER_DESKTOP);
   if (cc->name)
     {
        l = eina_hash_find(_gadman_gadgets, cc->name);
        eina_hash_set(_gadman_gadgets, cc->name, eina_list_append(l, gcc->cf));
     }
   ecore_job_add((Ecore_Cb)gadman_gadcon_place_job, gcc);

   return gcc;
}

E_Gadcon_Client *
gadman_gadget_add(const E_Gadcon_Client_Class *cc, E_Gadcon_Client *gcc, Gadman_Layer_Type layer)
{
   return _gadman_gadget_add(cc, layer, gcc->cf);
}

static E_Gadcon_Client *
_gadman_gadget_add(const E_Gadcon_Client_Class *cc, Gadman_Layer_Type layer, E_Config_Gadcon_Client *src_cf)
{
   E_Config_Gadcon_Client *cf = NULL;
   E_Gadcon_Client *gcc = NULL;
   E_Gadcon *gc;
   int w, h;

   gc = gadman_gadcon_get(e_zone_current_get(),
                          layer);

   /* Create Config_Gadcon_Client */
   cf = e_gadcon_client_config_new(gc, cc->name);
   if (cf)
     {
        if (!src_cf)
          {
             cf->style = eina_stringshare_add(cc->default_style);
             cf->geom.pos_x = DEFAULT_POS_X;
             cf->geom.pos_y = DEFAULT_POS_Y;
             cf->geom.size_w = DEFAULT_SIZE_W;
             cf->geom.size_h = DEFAULT_SIZE_H;
          }
        else
          {
             cf->style = eina_stringshare_add(src_cf->style);
             cf->geom.pos_x = src_cf->geom.pos_x;
             cf->geom.pos_y = src_cf->geom.pos_y;
             cf->geom.size_w = src_cf->geom.size_w;
             cf->geom.size_h = src_cf->geom.size_h;
          }
     }

   /* Place the new gadget */
   if (cf)
     gcc = gadman_gadget_place(NULL, cc, cf, layer, gc->zone);
   if (!gcc) return NULL;

   /* Respect Aspect */
   evas_object_geometry_get(gcc->o_frame, NULL, NULL, &w, &h);
   if (gcc->aspect.w && gcc->aspect.h)
     {
        if (gcc->aspect.w > gcc->aspect.h)
          w = ((float)h / (float)gcc->aspect.h) * (gcc->aspect.w);
        else
          h = ((float)w / (float)gcc->aspect.w) * (gcc->aspect.h);
        if (h < gcc->min.h) h = gcc->min.h;
        if (w < gcc->min.w) w = gcc->min.w;
        evas_object_resize(gcc->o_frame, w, h);
     }

   return gcc;
}

static void
gadman_edit(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _apply_widget_position(data);
}

static void
_gadman_gadget_edit_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w, h;
   if (mover_lock) return;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(_get_mover(data), w, h);
}

static void
_gadman_gadget_edit_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int x, y;
   if (mover_lock) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(_get_mover(data), x, y);
}

void
gadman_gadget_edit_start(E_Gadcon_Client *gcc)
{
   E_Gadcon *gc;
   Evas_Object *mover;
   Eina_List *l;
   int x, y, w, h;

   if (Man->drag_gcc[gcc->gadcon->id - ID_GADMAN_LAYER_BASE] == gcc) return;
   else if (Man->drag_gcc[gcc->gadcon->id - ID_GADMAN_LAYER_BASE])
     e_object_unref(E_OBJECT(Man->drag_gcc[gcc->gadcon->id - ID_GADMAN_LAYER_BASE]));

   if (gcc->gadcon->id - ID_GADMAN_LAYER_BASE == 1)
     {
        /* don't set edit if overlay isn't visible */
        if (!Man->visible) return;
     }
   EINA_LIST_FOREACH(Man->gadcons[gcc->gadcon->id - ID_GADMAN_LAYER_BASE], l, gc)
     gc->editing = 1;
   gc = gcc->gadcon;

   e_object_ref(E_OBJECT(gcc));
   //INF("START: %u", e_object_ref_get((void*)gcc));

   /* Move/resize the correct mover */
   mover = _get_mover(gcc);
   if (!mover) return;
   evas_object_geometry_get(gcc->o_frame, &x, &y, &w, &h);
   evas_object_event_callback_add(gcc->o_frame, EVAS_CALLBACK_RESIZE, _gadman_gadget_edit_resize_cb, gcc);
   evas_object_event_callback_add(gcc->o_frame, EVAS_CALLBACK_MOVE, _gadman_gadget_edit_move_cb, gcc);
   Man->drag_gcc[gcc->gadcon->id - ID_GADMAN_LAYER_BASE] = gcc;

   evas_object_move(mover, x, y);
   evas_object_resize(mover, w, h);
   evas_object_raise(mover);
   if (Man->visible || (!eina_list_data_find(Man->gadcons[GADMAN_LAYER_TOP], gc)))
     evas_object_show(mover);
   evas_object_event_callback_del(mover, EVAS_CALLBACK_HIDE, gadman_edit);
   evas_object_event_callback_add(mover, EVAS_CALLBACK_HIDE, gadman_edit, gcc);
}

void
gadman_gadget_edit_end(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   unsigned int layer;
   E_Gadcon_Client *drag_gcc = NULL;

   for (layer = GADMAN_LAYER_COUNT - 1; layer < GADMAN_LAYER_COUNT; layer--)
     {
        const Eina_List *l;
        E_Gadcon *gc;

        gc = eina_list_data_get(Man->gadcons[layer]);
        if (!gc) continue;
        if (!gc->editing) continue;

        evas_object_event_callback_del(Man->movers[layer], EVAS_CALLBACK_HIDE, gadman_edit);
        evas_object_hide(Man->movers[layer]);

        EINA_LIST_FOREACH(Man->gadcons[layer], l, gc)
          gc->editing = 0;
        drag_gcc = Man->drag_gcc[layer];
        if (drag_gcc)
          {
             evas_object_event_callback_del_full(drag_gcc->o_frame, EVAS_CALLBACK_RESIZE, _gadman_gadget_edit_resize_cb, drag_gcc);
             evas_object_event_callback_del_full(drag_gcc->o_frame, EVAS_CALLBACK_MOVE, _gadman_gadget_edit_move_cb, drag_gcc);
          }
        Man->drag_gcc[layer] = NULL;
        break;
     }
   if (!drag_gcc) return;
   drag_gcc->gadcon->drag_gcc = NULL;
   _save_widget_position(drag_gcc);
   if (!e_object_is_del(E_OBJECT(drag_gcc)))
     e_object_unref(E_OBJECT(drag_gcc));
   //INF("END: %d:%u", e_object_is_del(E_OBJECT(drag_gcc)), e_object_ref_get((void*)drag_gcc));
}

void
gadman_gadgets_show(void)
{
   Eina_List *l, *ll;
   E_Gadcon *gc;
   E_Gadcon_Client *gcc;
   Eina_Bool editing = EINA_FALSE;

   Man->visible = 1;
   evas_object_show(Man->overlay);

   if (Man->conf->bg_type == BG_STD)
     {
        if (Man->conf->anim_bg)
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,show", "e");
        else
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,show,now", "e");
     }
   else
     {
        if (Man->conf->anim_bg)
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,show,custom", "e");
        else
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,show,custom,now", "e");
     }

   /* Showing top gadgets */
   EINA_LIST_FOREACH(Man->gadcons[GADMAN_LAYER_TOP], l, gc)
     {
        EINA_LIST_FOREACH(gc->clients, ll, gcc)
          {
             if (Man->conf->anim_gad)
               edje_object_signal_emit(gcc->o_frame,
                                       "e,state,visibility,show", "e");
             else
               edje_object_signal_emit(gcc->o_frame,
                                       "e,state,visibility,show,now", "e");
             e_gadcon_client_show(gcc);
          }
        gc->drop_handler->hidden = 0;
        editing = gc->editing;
     }
   if (editing && gcc) evas_object_show(_get_mover(gcc));
}

void
gadman_gadgets_hide(void)
{
   Eina_List *l, *ll;
   E_Gadcon *gc;
   E_Gadcon_Client *gcc;
   Eina_Bool editing = EINA_FALSE;

   Man->visible = 0;

   if (Man->conf->bg_type == BG_STD)
     {
        if (Man->conf->anim_bg)
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,hide", "e");
        else
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,hide,now", "e");
     }
   else
     {
        if (Man->conf->anim_bg)
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,hide,custom", "e");
        else
          edje_object_signal_emit(Man->full_bg,
                                  "e,state,visibility,hide,custom,now", "e");
     }

   /* Hiding top gadgets */
   EINA_LIST_FOREACH(Man->gadcons[GADMAN_LAYER_TOP], l, gc)
     {
        gc->drop_handler->hidden = 1;
        editing = gc->editing;
        EINA_LIST_FOREACH(gc->clients, ll, gcc)
          {
             if (Man->conf->anim_gad)
               edje_object_signal_emit(gcc->o_frame,
                                       "e,state,visibility,hide", "e");
             else
               edje_object_signal_emit(gcc->o_frame,
                                       "e,state,visibility,hide,now", "e");
             e_gadcon_client_hide(gcc);
          }
     }
   if (editing)
     gadman_gadget_edit_end(NULL, NULL, NULL, NULL);
}

void
gadman_gadgets_toggle(void)
{
   if (Man->visible)
     gadman_gadgets_hide();
   else
     gadman_gadgets_show();
}

void
gadman_update_bg(void)
{
   Evas_Object *obj;

   if (!Man->gadcons[GADMAN_LAYER_TOP]) return;
   if (!Man->conf) return;
   obj = edje_object_part_swallow_get(Man->full_bg, "e.swallow.bg");
   if (obj)
     {
        edje_object_part_unswallow(Man->full_bg, obj);
        evas_object_del(obj);
     }

   switch (Man->conf->bg_type)
     {
      case BG_STD:
      case BG_TRANS:
        break;

      case BG_COLOR:
        {
           double r, g, b;

           r = (double)Man->conf->color_r * (200. / 255.);
           g = (double)Man->conf->color_g * (200. / 255.);
           b = (double)Man->conf->color_b * (200. / 255.);
           obj = evas_object_rectangle_add(e_comp->evas);
           evas_object_color_set(obj, lround(r), lround(g), lround(b), 200);
           edje_object_part_swallow(Man->full_bg, "e.swallow.bg", obj);
        }
        break;

      case BG_CUSTOM:
        if (eina_str_has_extension(Man->conf->custom_bg, ".edj"))
          {
             //THIS IS FOR E backgrounds
             obj = edje_object_add(e_comp->evas);
             edje_object_file_set(obj, Man->conf->custom_bg,
                                  "e/desktop/background");
          }
        else
          {
             //THIS IS FOR A NORMAL IMAGE
             obj = evas_object_image_add(e_comp->evas);
             evas_object_image_file_set(obj, Man->conf->custom_bg, NULL);
             evas_object_image_fill_set(obj, 0, 0, e_comp->w, e_comp->h);
          }
        edje_object_part_swallow(Man->full_bg, "e.swallow.bg", obj);
        break;

      default:
        break;
     }
}

static void
_gadman_gadcon_free(E_Gadcon *gc)
{
   e_gadcon_unpopulate(gc);
   e_gadcon_custom_del(gc);

   /* free gadcons */
   e_config->gadcons = eina_list_remove(e_config->gadcons, gc);
   eina_stringshare_del(gc->name);

   if (gc->config_dialog) e_object_del(E_OBJECT(gc->config_dialog));
   eina_list_free(gc->populated_classes);
   if (gc->drop_handler) e_drop_handler_del(gc->drop_handler);
   free(gc);
}

static void
_gadman_gadcon_dnd_enter_cb(E_Gadcon *gc, E_Gadcon_Client *gcc)
{

   /* only use this for dragging gadcons around the desktop */
   if ((!eina_list_data_find(Man->gadcons[GADMAN_LAYER_BG], gc)) &&
       (!eina_list_data_find(Man->gadcons[GADMAN_LAYER_TOP], gc)))
     return;
   if (gc != gcc->gadcon) return;
   //INF("ENTER: %u", e_object_ref_get((void*)gcc));
   gadman_gadget_edit_start(gcc);
}

static void
_gadman_gadcon_dnd_leave_cb(E_Gadcon *gc, E_Gadcon_Client *gcc)
{
   unsigned int layer;
   E_Gadcon_Client *drag_gcc = NULL;

   /* only use this for dragging gadcons around the desktop */
   if ((!eina_list_data_find(Man->gadcons[GADMAN_LAYER_BG], gc)) &&
       (!eina_list_data_find(Man->gadcons[GADMAN_LAYER_TOP], gc)))
     return;
   if (gc != gcc->gadcon) return;
   //INF("LEAVE: %u", e_object_ref_get((void*)gcc));
   Man->drag_gcc[gcc->gadcon->id - ID_GADMAN_LAYER_BASE] = NULL;
   for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
     {
        const Eina_List *l;

        evas_object_event_callback_del(Man->movers[layer], EVAS_CALLBACK_HIDE, gadman_edit);
        evas_object_hide(Man->movers[layer]);

        EINA_LIST_FOREACH(Man->gadcons[layer], l, gc)
          {
             gc->editing = 0;
             drag_gcc = gc->drag_gcc;
          }
     }
   /* this is slightly different from edit_end because we don't save the position or unset the drag gcc */
   if (!drag_gcc) return;
   e_object_unref(E_OBJECT(drag_gcc));
}

static void
_gadman_gadcon_dnd_move_cb(E_Gadcon *gc, E_Gadcon_Client *gcc)
{
   Evas_Object *mover;
   E_Zone *zone;
   int x, y, mx, my;
   int ox, oy, ow, oh;

   if (gc != gcc->gadcon) return;
   /* only use this for dragging gadcons around the desktop */
   if ((!eina_list_data_find(Man->gadcons[GADMAN_LAYER_BG], gc)) &&
       (!eina_list_data_find(Man->gadcons[GADMAN_LAYER_TOP], gc)))
     return;

   mover = _get_mover(gcc);
   evas_object_geometry_get(gcc->o_frame, &x, &y, NULL, NULL);
   evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);

   /* don't go out of the screen */
   x = MAX(x, gcc->dx), y = MAX(y, gcc->dy);

   /* adjust in case one screen is larger than another */
   zone = e_gadcon_zone_get(gc);
   mx = MIN(Man->width, zone->x + zone->w), my = MIN(Man->height, zone->y + zone->h);
   x = MIN(x, mx - ow + gcc->dx), y = MIN(y, my - oh + gcc->dy);

   evas_object_move(gcc->o_frame, x - gcc->dx, y - gcc->dy);
   evas_object_move(mover, x - gcc->dx, y - gcc->dy);
   evas_object_raise(gcc->o_frame);
   evas_object_raise(mover);
   _save_widget_position(gcc);
}

static void
_gadman_gadcon_dnd_drop_cb(E_Gadcon *gc, E_Gadcon_Client *gcc)
{
   E_Config_Gadcon_Client *cf;
   E_Zone *dst_zone = NULL;
   E_Gadcon *dst_gadcon;
   Evas_Object *mover;
   int gx, gy;

   if (gc != gcc->gadcon) return;
   /* only use this for dragging gadcons around the desktop */
   if ((!eina_list_data_find(Man->gadcons[GADMAN_LAYER_BG], gc)) &&
       (!eina_list_data_find(Man->gadcons[GADMAN_LAYER_TOP], gc)))
     return;

   gcc->moving = 0;
   gcc->dx = gcc->dy = 0;

   /* checking if zone was changed for dragged gadget */
   mover = _get_mover(gcc);
   evas_object_geometry_get(mover, &gx, &gy, NULL, NULL);
   dst_zone = e_comp_zone_xy_get(gx, gy);
   if (dst_zone && (gcc->gadcon->zone != dst_zone))
     {
        unsigned int layer = gcc->gadcon->id - ID_GADMAN_LAYER_BASE;
        cf = gcc->cf;
        gcc->gadcon->cf->clients = eina_list_remove(gcc->gadcon->cf->clients, cf);
        dst_gadcon = gadman_gadcon_get(dst_zone, layer);
        if (dst_gadcon)
          {
             dst_gadcon->cf->clients = eina_list_append(dst_gadcon->cf->clients, cf);
          }
     }
   _save_widget_position(gcc);
   e_config_save_queue();
}

static void
_gadman_overlay_create(void)
{
   Eina_List *l;
   E_Gadcon *gc;

   /* create full background object */
   Man->full_bg = edje_object_add(e_comp->evas);
   evas_object_geometry_set(Man->full_bg, 0, 0, e_comp->w, e_comp->h);
   e_theme_edje_object_set(Man->full_bg, "base/theme/gadman",
                           "e/gadman/full_bg");
   edje_object_signal_callback_add(Man->full_bg, "mouse,down,*",
                                   "grabber", on_bg_click, NULL);
   edje_object_signal_callback_add(Man->full_bg, "e,action,hide,stop",
                                   "", on_hide_stop, NULL);

   Man->overlay = e_comp_object_util_add(Man->full_bg, E_COMP_OBJECT_TYPE_NONE);
   evas_object_layer_set(Man->overlay, E_LAYER_CLIENT_FULLSCREEN);

   /* create placeholder rect to maintain our dnd stacking layer */
   EINA_LIST_FOREACH(Man->gadcons[GADMAN_LAYER_TOP], l, gc)
     {
        gc->drop_handler->base = Man->overlay;
        gc->drop_handler->hidden = 1;
     }
}

static E_Gadcon *
_gadman_gadcon_new(const char *name, Gadman_Layer_Type layer, E_Zone *zone, E_Gadcon_Location *loc)
{
   const Eina_List *l;
   E_Gadcon *gc;
   E_Config_Gadcon *cg;

   /* Create Gadcon */
   gc = E_OBJECT_ALLOC(E_Gadcon, E_GADCON_TYPE, _gadman_gadcon_free);
   if (!gc) return NULL;

   gc->name = eina_stringshare_add(name);
   gc->layout_policy = E_GADCON_LAYOUT_POLICY_PANEL;
   gc->orient = E_GADCON_ORIENT_FLOAT;
   gc->location = loc;

   gc->evas = e_comp->evas;
   e_gadcon_ecore_evas_set(gc, e_comp->ee);
   e_gadcon_xdnd_window_set(gc, e_comp->ee_win);
   e_gadcon_dnd_window_set(gc, e_comp->ee_win);

   e_gadcon_drop_handler_add(gc, _gadman_gadcon_dnd_enter_cb, _gadman_gadcon_dnd_leave_cb,
                             _gadman_gadcon_dnd_move_cb, _gadman_gadcon_dnd_drop_cb,
                             zone->x, zone->y, zone->w, zone->h);
   e_gadcon_zone_set(gc, zone);
   e_gadcon_util_menu_attach_func_set(gc, _attach_menu, NULL);
   e_gadcon_populate_callback_set(gc, gadman_populate_class, (void *)layer);

   gc->id = ID_GADMAN_LAYER_BASE + layer;
   gc->edje.o_parent = NULL;
   gc->edje.swallow_name = NULL;
   gc->shelf = NULL;
   gc->toolbar = NULL;
   gc->editing = 0;
   gc->o_container = NULL;
   gc->frame_request.func = NULL;
   gc->resize_request.func = NULL;
   gc->min_size_request.func = NULL;

   /* Search for existing gadcon config */
   gc->cf = NULL;
   EINA_LIST_FOREACH(e_config->gadcons, l, cg)
     {
        if ((!strcmp(cg->name, name)) && (cg->zone == zone->num))
          {
             gc->cf = cg;
             break;
          }
     }

   /* ... or create a new one */
   if (!gc->cf)
     {
        gc->cf = E_NEW(E_Config_Gadcon, 1);
        gc->cf->name = eina_stringshare_add(name);
        gc->cf->id = gc->id;
        gc->cf->zone = zone->num;
        gc->cf->clients = NULL;
        e_config->gadcons = eina_list_append(e_config->gadcons, gc->cf);
        e_config_save_queue();
     }

   e_gadcon_custom_new(gc);
   e_gadcon_custom_populate_request(gc);

   if (!Man->movers[layer])
     Man->movers[layer] = _create_mover(gc);

   return gc;
}

static void
_mover_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   unsigned int layer;

   for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
     {
        if (Man->movers[layer] != obj) continue;
        Man->movers[layer] = NULL;
        return;
     }
}

static Evas_Object *
_create_mover(E_Gadcon *gc)
{
   Evas_Object *mover;

   /* create mover object */
   mover = edje_object_add(gc->evas);
   if (gc->id == ID_GADMAN_LAYER_BG)
     {
        evas_object_layer_set(mover, E_LAYER_DESKTOP);
        evas_object_event_callback_add(mover, EVAS_CALLBACK_DEL, _mover_del, NULL);
     }
   else
     evas_object_layer_set(mover, E_LAYER_MENU);
   e_theme_edje_object_set(mover, "base/theme/gadman", "e/gadman/control");

   edje_object_signal_callback_add(mover, "e,action,move,start", "",
                                   on_move, (void *)DRAG_START);
   edje_object_signal_callback_add(mover, "mouse,down,3", "*",
                                   gadman_gadget_edit_end, NULL);

   edje_object_signal_callback_add(mover, "e,action,resize,left,start", "",
                                   on_left, (void *)DRAG_START);
   edje_object_signal_callback_add(mover, "e,action,resize,left,stop", "",
                                   on_left, (void *)DRAG_STOP);
   edje_object_signal_callback_add(mover, "e,action,resize,left,go", "",
                                   on_left, (void *)DRAG_MOVE);
   edje_object_signal_callback_add(mover, "e,action,resize,down,start", "",
                                   on_down, (void *)DRAG_START);
   edje_object_signal_callback_add(mover, "e,action,resize,down,stop", "",
                                   on_down, (void *)DRAG_STOP);
   edje_object_signal_callback_add(mover, "e,action,resize,down,go", "",
                                   on_down, (void *)DRAG_MOVE);
   edje_object_signal_callback_add(mover, "e,action,resize,right,start", "",
                                   on_right, (void *)DRAG_START);
   edje_object_signal_callback_add(mover, "e,action,resize,right,stop", "",
                                   on_right, (void *)DRAG_STOP);
   edje_object_signal_callback_add(mover, "e,action,resize,right,go", "",
                                   on_right, (void *)DRAG_MOVE);
   edje_object_signal_callback_add(mover, "e,action,resize,up,start", "",
                                   on_top, (void *)DRAG_START);
   edje_object_signal_callback_add(mover, "e,action,resize,up,stop", "",
                                   on_top, (void *)DRAG_STOP);
   edje_object_signal_callback_add(mover, "e,action,resize,up,go", "",
                                   on_top, (void *)DRAG_MOVE);

   return mover;
}

static Evas_Object *
_get_mover(E_Gadcon_Client *gcc)
{
   return Man->movers[gcc->gadcon->id - ID_GADMAN_LAYER_BASE];
}

static void
_save_widget_position(E_Gadcon_Client *gcc)
{
   int x, y, w, h;

   evas_object_geometry_get(gcc->o_frame, &x, &y, &w, &h);
   x -= gcc->gadcon->zone->x, y -= gcc->gadcon->zone->y;
   gcc->config.pos_x = (double)x / (double)gcc->gadcon->zone->w;
   gcc->config.pos_y = (double)y / (double)gcc->gadcon->zone->h;
   gcc->config.size_w = (double)w / (double)gcc->gadcon->zone->w;
   gcc->config.size_h = (double)h / (double)gcc->gadcon->zone->h;
   if (gcc->cf)
     {
        gcc->cf->geom.pos_x = gcc->config.pos_x;
        gcc->cf->geom.pos_y = gcc->config.pos_y;
        gcc->cf->geom.size_w = gcc->config.size_w;
        gcc->cf->geom.size_h = gcc->config.size_h;
        e_config_save_queue();
     }
}

static void
_apply_widget_position(E_Gadcon_Client *gcc)
{
   int x, y, w, h;
   E_Zone *zone;
   Evas_Object *mover;

   /* Obtain zone from parent gadcon */
   zone = gcc->gadcon->zone;

   x = gcc->cf->geom.pos_x * zone->w + zone->x;
   y = gcc->cf->geom.pos_y * zone->h + zone->y;
   w = gcc->cf->geom.size_w * zone->w;
   h = gcc->cf->geom.size_h * zone->h;

   /* something broke the config's geom, make it visible so it can be
    * resized/deleted
    */
   if ((gcc->cf->geom.pos_x < 0) || (gcc->cf->geom.pos_y < 0) ||
       (!EINA_DBL_NONZERO(gcc->cf->geom.size_w)) || (!EINA_DBL_NONZERO(gcc->cf->geom.size_h)))
     {
        gcc->cf->style = eina_stringshare_add(gcc->client_class->default_style ?: E_GADCON_CLIENT_STYLE_INSET);
        gcc->style = eina_stringshare_ref(gcc->cf->style);
        gcc->cf->geom.pos_x = DEFAULT_POS_X;
        gcc->cf->geom.pos_y = DEFAULT_POS_Y;
        gcc->cf->geom.size_w = DEFAULT_SIZE_W;
        gcc->cf->geom.size_h = DEFAULT_SIZE_H;
        if (!strcmp(gcc->style, E_GADCON_CLIENT_STYLE_INSET))
          edje_object_signal_emit(gcc->o_frame, "e,state,visibility,inset", "e");
        else
          edje_object_signal_emit(gcc->o_frame, "e,state,visibility,plain", "e");
        _apply_widget_position(gcc);
        gadman_gadget_edit_start(gcc);
        return;
     }

   /* Respect min sizes */
   if (h < gcc->min.h) h = gcc->min.h;
   if (w < gcc->min.w) w = gcc->min.w;
   if (h < 1) h = 100;
   if (w < 1) w = 100;

   /* Respect zone marigin */
   if (x < zone->x) x = zone->x;
   if (y < zone->y) y = zone->y;
   if (x > (zone->x + zone->w)) x = zone->x;
   if (y > (zone->y + zone->h)) y = zone->y;

   if ((y + h) > (zone->y + zone->h + MIN_VISIBLE_MARIGIN))
     h = ((zone->y + zone->h + MIN_VISIBLE_MARIGIN) - y);
   if ((x + w) > (zone->x + zone->w + MIN_VISIBLE_MARIGIN))
     w = ((zone->x + zone->w + MIN_VISIBLE_MARIGIN) - x);

   evas_object_move(gcc->o_frame, x, y);
   evas_object_resize(gcc->o_frame, w, h);

   if (Man->drag_gcc[gcc->gadcon->id - ID_GADMAN_LAYER_BASE] != gcc) return;
   /* Move/resize the correct mover */
   mover = _get_mover(gcc);
   if (!mover) return;
   evas_object_move(mover, x, y);
   evas_object_resize(mover, w, h);
}

static void
_attach_menu(void *data EINA_UNUSED, E_Gadcon_Client *gcc, E_Menu *menu)
{
   E_Menu *mn;
   E_Menu_Item *mi;

   //printf("Attach menu (gcc: %x id: %s) [%s]\n", gcc, gcc->cf->id, gcc->cf->style);
   if (!gcc) return;

   if (e_menu_item_nth(menu, 0))
     {
        mi = e_menu_item_new(menu);
        e_menu_item_separator_set(mi, 1);
     }

   /* Move / resize*/
   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Begin move/resize"));
   e_util_menu_item_theme_icon_set(mi, "transform-move");
   e_menu_item_callback_set(mi, on_menu_edit, gcc);

   /* plain / inset */
   if (gcc->cf)
     {
        if (!gcc->cf->style)
          gcc->cf->style = eina_stringshare_add(E_GADCON_CLIENT_STYLE_INSET);

        mn = e_menu_new();
        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, _("Plain"));
        e_menu_item_radio_set(mi, 1);
        e_menu_item_radio_group_set(mi, 1);
        if (!strcmp(gcc->cf->style, E_GADCON_CLIENT_STYLE_PLAIN))
          e_menu_item_toggle_set(mi, 1);
        e_menu_item_callback_set(mi, on_menu_style_plain, gcc);

        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, _("Inset"));
        e_menu_item_radio_set(mi, 1);
        e_menu_item_radio_group_set(mi, 1);
        if (!strcmp(gcc->cf->style, E_GADCON_CLIENT_STYLE_INSET))
          e_menu_item_toggle_set(mi, 1);
        e_menu_item_callback_set(mi, on_menu_style_inset, gcc);

        mi = e_menu_item_new(mn);
        e_menu_item_separator_set(mi, 1);

        /* orient */
        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, _("Free"));
        e_menu_item_radio_set(mi, 1);
        e_menu_item_radio_group_set(mi, 2);
        if (gcc->cf->orient == E_GADCON_ORIENT_FLOAT)
          e_menu_item_toggle_set(mi, 1);
        if (!mi->toggle)
          e_menu_item_callback_set(mi, on_menu_style_float, gcc);

        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, _("Horizontal"));
        e_menu_item_radio_set(mi, 1);
        e_menu_item_radio_group_set(mi, 2);
        if (gcc->cf->orient == E_GADCON_ORIENT_HORIZ)
          e_menu_item_toggle_set(mi, 1);
        if (!mi->toggle)
          e_menu_item_callback_set(mi, on_menu_style_horiz, gcc);

        mi = e_menu_item_new(mn);
        e_menu_item_label_set(mi, _("Vertical"));
        e_menu_item_radio_set(mi, 1);
        e_menu_item_radio_group_set(mi, 2);
        if (gcc->cf->orient == E_GADCON_ORIENT_VERT)
          e_menu_item_toggle_set(mi, 1);
        if (!mi->toggle)
          e_menu_item_callback_set(mi, on_menu_style_vert, gcc);

        mi = e_menu_item_new(menu);
        e_menu_item_label_set(mi, _("Appearance"));
        e_util_menu_item_theme_icon_set(mi, "preferences-look");
        e_menu_item_submenu_set(mi, mn);
        e_object_unref(E_OBJECT(mn));

        mi = e_menu_item_new(menu);
        e_menu_item_separator_set(mi, 1);

        e_gadcon_client_add_location_menu(gcc, menu);
     }

   /* Remove this gadgets */
   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Remove"));
   e_util_menu_item_theme_icon_set(mi, "list-remove");
   e_menu_item_callback_set(mi, on_menu_delete, gcc);

   /* Add other gadgets */
   mi = e_menu_item_new(menu);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Add other gadgets"));
   e_util_menu_item_theme_icon_set(mi, "list-add");
   e_menu_item_callback_set(mi, on_menu_add, gcc);
}

static void
on_menu_style_plain(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Gadcon_Client *gcc = data;

   eina_stringshare_replace(&gcc->style, E_GADCON_CLIENT_STYLE_PLAIN);
   eina_stringshare_replace(&gcc->cf->style, E_GADCON_CLIENT_STYLE_PLAIN);

   edje_object_signal_emit(gcc->o_frame, "e,state,visibility,plain", "e");

   e_config_save_queue();
}

static void
on_menu_style_inset(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Gadcon_Client *gcc = data;

   eina_stringshare_replace(&gcc->style, E_GADCON_CLIENT_STYLE_INSET);
   eina_stringshare_replace(&gcc->cf->style, E_GADCON_CLIENT_STYLE_INSET);

   edje_object_signal_emit(gcc->o_frame, "e,state,visibility,inset", "e");

   e_config_save_queue();
}

static void
_menu_style_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient)
{
   int ow, oh, w, h;
   Eina_Bool same;

   same = (((orient == E_GADCON_ORIENT_LEFT) && (gcc->cf->orient == E_GADCON_ORIENT_FLOAT)) ||
       ((orient == E_GADCON_ORIENT_FLOAT) && (gcc->cf->orient == E_GADCON_ORIENT_LEFT)));
   gcc->cf->orient = orient;
   evas_object_geometry_get(gcc->o_frame, NULL, NULL, &ow, &oh);

   if (gcc->client_class->func.orient)
     gcc->client_class->func.orient(gcc, orient);

   if (same)
     {
        w = ow, h = oh;
     }
   else
     {
        /* just flip aspect */
        w = oh;
        h = ow;
     }

   gcc->max.w = w;
   gcc->max.h = h;
   evas_object_resize(gcc->o_frame, w, h);
   _save_widget_position(gcc);
}

static void
on_menu_style_float(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   _menu_style_orient(data, E_GADCON_ORIENT_FLOAT);
}

static void
on_menu_style_horiz(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   _menu_style_orient(data, E_GADCON_ORIENT_HORIZ);
}

static void
on_menu_style_vert(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   _menu_style_orient(data, E_GADCON_ORIENT_VERT);
}

static void
on_menu_edit(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   gadman_gadget_edit_start(data);
}

static void
on_menu_add(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   if (Man->visible)
     gadman_gadgets_hide();
   e_configure_registry_call("extensions/gadman", NULL, NULL);
}

static void
on_menu_delete(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   E_Gadcon_Client *gcc = data;
   e_gadcon_client_config_del(gcc->gadcon->cf, gcc->cf);
   e_object_del(data);
   e_config_save_queue();
}

static void
on_frame_click(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   E_Gadcon_Client *gcc;

   ev = event_info;

   gcc = data;

   if (ev->button == 5)
     {
        E_Menu *m;
        int cx, cy, cw, ch;

        m = e_menu_new();
        m = e_gadcon_client_util_menu_items_append(gcc, m, 0);
        gcc->menu = m;
        e_gadcon_canvas_zone_geometry_get(gcc->gadcon, &cx, &cy, &cw, &ch);
        e_menu_activate_mouse(m,
                              e_zone_current_get(),
                              cx + ev->output.x, cy + ev->output.y, 1, 1,
                              E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
        evas_event_feed_mouse_up(gcc->gadcon->evas, ev->button,
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

static void
on_top(void *data, Evas_Object *o EINA_UNUSED, const char *em EINA_UNUSED, const char *src EINA_UNUSED)
{
   static int ox, oy, ow, oh; //Object coord
   int mx, my; //Mouse coord
   int action = (int)(long)data;
   Evas_Object *mover;
   E_Gadcon_Client *drag_gcc;
   int layer = Man->visible;

   drag_gcc = Man->drag_gcc[layer];
   if (!drag_gcc) return;

   mover = _get_mover(drag_gcc);
   if (!mover) return;
   mover_lock = EINA_TRUE;

   if (action == DRAG_START)
     {
        drag_gcc->resizing = 1;
        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        drag_gcc->dy = my - oy;
     }
   else if (action == DRAG_STOP)
     {
        drag_gcc->resizing = 0;
        drag_gcc->dy = 0;
        _save_widget_position(drag_gcc);
     }
   else if ((action == DRAG_MOVE) && drag_gcc->resizing)
     {
        int h;

        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);

        h = oy + oh + drag_gcc->dy - my;

        if (h < drag_gcc->min.h)
          {
             my -= drag_gcc->min.h - h;
             h = drag_gcc->min.h;
          }
        /* don't go out of the screen */
        if (my < drag_gcc->dy)
          {
             h += my - drag_gcc->dy;
             my = drag_gcc->dy;
          }

        if (drag_gcc->aspect.w && drag_gcc->aspect.h)
          {
             ow = h * drag_gcc->aspect.w / drag_gcc->aspect.h;
          }

        evas_object_resize(mover, ow, h);
        evas_object_move(mover, ox, my - drag_gcc->dy);

        drag_gcc->max.w = ow, drag_gcc->max.h = h;
        evas_object_resize(drag_gcc->o_frame, ow, h);
        evas_object_move(drag_gcc->o_frame, ox, my - drag_gcc->dy);
        _save_widget_position(drag_gcc);
     }
   mover_lock = EINA_FALSE;
}

static void
on_right(void *data, Evas_Object *o EINA_UNUSED, const char *em EINA_UNUSED, const char *src EINA_UNUSED)
{
   Evas_Object *mover;
   static int ox, oy, ow, oh; //Object coord
   int mx, my; //Mouse coord
   int action;
   E_Gadcon_Client *drag_gcc;
   int layer = Man->visible;

   drag_gcc = Man->drag_gcc[layer];
   if (!drag_gcc) return;

   mover = _get_mover(drag_gcc);
   if (!mover) return;
   mover_lock = EINA_TRUE;

   action = (int)(long)data;
   if (action == DRAG_START)
     {
        drag_gcc->resizing = 1;
        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        drag_gcc->dx = mx - ow;
     }
   else if (action == DRAG_STOP)
     {
        drag_gcc->resizing = 0;
        drag_gcc->dx = 0;
        _save_widget_position(drag_gcc);
     }
   else if ((action == DRAG_MOVE) && drag_gcc->resizing)
     {
        int w;

        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);
        w = mx - drag_gcc->dx;

        if (w < drag_gcc->min.w) w = drag_gcc->min.w;
        /* don't go out of the screen */
        if (w > (Man->width - ox)) w = Man->width - ox;

        if (drag_gcc->aspect.w && drag_gcc->aspect.h)
          {
             oh = w * drag_gcc->aspect.h / drag_gcc->aspect.w;
          }
        drag_gcc->max.w = w, drag_gcc->max.h = oh;
        evas_object_resize(mover, w, oh);
        evas_object_resize(drag_gcc->o_frame, w, oh);
        _save_widget_position(drag_gcc);
     }
   mover_lock = EINA_FALSE;
}

static void
on_down(void *data, Evas_Object *o EINA_UNUSED, const char *em EINA_UNUSED, const char *src EINA_UNUSED)
{
   Evas_Object *mover;
   static int ox, oy, ow, oh; //Object coord
   int mx, my; //Mouse coord
   int action = (int)(long)data;
   E_Gadcon_Client *drag_gcc;
   int layer = Man->visible;

   drag_gcc = Man->drag_gcc[layer];
   if (!drag_gcc) return;

   mover = _get_mover(drag_gcc);
   if (!mover) return;
   mover_lock = EINA_TRUE;

   if (action == DRAG_START)
     {
        drag_gcc->resizing = 1;
        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        drag_gcc->dy = my - oh;
     }
   else if (action == DRAG_STOP)
     {
        drag_gcc->resizing = 0;
        drag_gcc->dy = 0;
        _save_widget_position(drag_gcc);
     }
   else if ((action == DRAG_MOVE) && drag_gcc->resizing)
     {
        int h;

        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);
        h = my - drag_gcc->dy;

        if (h < drag_gcc->min.h) h = drag_gcc->min.h;
        /* don't go out of the screen */
        if (h > (Man->height - oy)) h = Man->height - oy;
        if (drag_gcc->aspect.w && drag_gcc->aspect.h)
          {
             ow = h * drag_gcc->aspect.w / drag_gcc->aspect.h;
          }
        evas_object_resize(mover, ow, h);
        drag_gcc->max.w = ow, drag_gcc->max.h = h;
        evas_object_resize(drag_gcc->o_frame, ow, h);
        _save_widget_position(drag_gcc);
     }
   mover_lock = EINA_FALSE;
}

static void
on_left(void *data, Evas_Object *o EINA_UNUSED, const char *em EINA_UNUSED, const char *src EINA_UNUSED)
{
   Evas_Object *mover;
   static int ox, oy, ow, oh; //Object coord
   int mx, my; //Mouse coord
   int action = (int)(long)data;
   E_Gadcon_Client *drag_gcc;
   int layer = Man->visible;

   drag_gcc = Man->drag_gcc[layer];
   if (!drag_gcc) return;

   mover = _get_mover(drag_gcc);
   if (!mover) return;
   mover_lock = EINA_TRUE;
   if (action == DRAG_START)
     {
        drag_gcc->resizing = 1;
        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        drag_gcc->dx = mx - ox;
     }
   else if (action == DRAG_STOP)
     {
        drag_gcc->resizing = 0;
        drag_gcc->dx = 0;
        _save_widget_position(drag_gcc);
     }
   else if ((action == DRAG_MOVE) && drag_gcc->resizing)
     {
        int w;

        evas_pointer_output_xy_get(drag_gcc->gadcon->evas, &mx, &my);

        w = ox + ow + drag_gcc->dx - mx;

        if (w < drag_gcc->min.w)
          {
             mx -= drag_gcc->min.w - w;
             w = drag_gcc->min.w;
          }
        /* don't go out of the screen */
        if (mx < drag_gcc->dx)
          {
             w += mx - drag_gcc->dx;
             mx = drag_gcc->dx;
          }
        if (drag_gcc->aspect.w && drag_gcc->aspect.h)
          {
             oh = w * drag_gcc->aspect.h / drag_gcc->aspect.w;
          }
        evas_object_resize(mover, w, oh);
        evas_object_move(mover, mx - drag_gcc->dx, oy);

        drag_gcc->max.w = w, drag_gcc->max.h = oh;
        evas_object_resize(drag_gcc->o_frame, w, oh);
        evas_object_move(drag_gcc->o_frame, mx - drag_gcc->dx, oy);
        _save_widget_position(drag_gcc);
     }
   mover_lock = EINA_FALSE;
}

static void
on_move(void *data, Evas_Object *o EINA_UNUSED, const char *em EINA_UNUSED, const char *src EINA_UNUSED)
{
   Evas_Object *mover;
   static int ox, oy;  //Starting object position
   static int ow, oh;  //Starting object size
   int mx, my; //Mouse coord
   int action = (int)(long)data;
   E_Gadcon *gc;
   E_Gadcon_Client *drag_gcc;
   E_Drag *drag;
   const char *drag_types[] = { "enlightenment/gadcon_client" };

   /* DRAG_START */
   if (action != DRAG_START) return;
   drag_gcc = Man->drag_gcc[Man->visible];
   if (!drag_gcc) return;
   gc = drag_gcc->gadcon;
   mover = _get_mover(drag_gcc);

   drag_gcc->moving = 1;
   gc->cf->clients = eina_list_remove(gc->cf->clients, drag_gcc->cf);
   e_gadcon_client_drag_set(drag_gcc);
   e_object_ref(E_OBJECT(drag_gcc));
   evas_pointer_output_xy_get(gc->evas, &mx, &my);
   evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);

   drag_gcc->dx = mx - ox;
   drag_gcc->dy = my - oy;

   drag_gcc->drag.drag = drag = e_drag_new(mx, my,
                     drag_types, 1, drag_gcc, -1, NULL,
                     e_gadcon_drag_finished_cb);
   if (!drag) return;
   drag->button_mask = evas_pointer_button_down_mask_get(e_comp->evas);

   o = drag_gcc->client_class->func.icon((E_Gadcon_Client_Class *)drag_gcc->client_class,
                                    e_drag_evas_get(drag));
   if (!o)
     {
        /* FIXME: fallback icon for drag */
        o = evas_object_rectangle_add(e_drag_evas_get(drag));
        evas_object_color_set(o, 255, 255, 255, 100);
     }

   e_drag_object_set(drag, o);
   e_drag_resize(drag, ow, oh);
   e_drag_start(drag, mx, my);
   e_drag_hide(drag);
}

static void
on_bg_click(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, const char *em EINA_UNUSED, const char *src EINA_UNUSED)
{
   gadman_gadgets_hide();
}

static void
on_hide_stop(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, const char *em EINA_UNUSED, const char *src EINA_UNUSED)
{
   evas_object_hide(Man->overlay);
}

static int
_e_gadman_client_add(void *data, E_Gadcon_Client *gcc, const E_Gadcon_Client_Class *cc)
{
   return !!gadman_gadget_add(cc, gcc, (intptr_t)data);
}

static void
_e_gadman_client_remove(void *data EINA_UNUSED, E_Gadcon_Client *gcc)
{
   if (gcc->cf)
     gcc->gadcon->cf->clients = eina_list_remove(gcc->gadcon->cf->clients, gcc->cf);
   e_object_del(E_OBJECT(gcc));
}

static void
_e_gadman_handlers_add(void)
{
   E_LIST_HANDLER_APPEND(_gadman_hdls, E_EVENT_ZONE_ADD, _e_gadman_cb_zone_change, NULL);
   E_LIST_HANDLER_APPEND(_gadman_hdls, E_EVENT_ZONE_MOVE_RESIZE, _e_gadman_cb_zone_change, NULL);
   E_LIST_HANDLER_APPEND(_gadman_hdls, E_EVENT_ZONE_USEFUL_GEOMETRY_CHANGED, _e_gadman_cb_zone_change, NULL);
   E_LIST_HANDLER_APPEND(_gadman_hdls, E_EVENT_ZONE_DEL, _e_gadman_cb_zone_change, NULL);
   E_LIST_HANDLER_APPEND(_gadman_hdls, E_EVENT_MODULE_UPDATE, _gadman_module_cb, NULL);
   E_LIST_HANDLER_APPEND(_gadman_hdls, E_EVENT_MODULE_INIT_END, _gadman_module_init_end_cb, NULL);
}

static void
_e_gadman_handler_del(void)
{
   E_FREE_LIST(_gadman_hdls, ecore_event_handler_del);
}

static Eina_Bool
_gadman_module_init_end_cb(void *d EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   gadman_locked = EINA_FALSE;
   gadman_reset(NULL);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_gadman_module_cb(void *d EINA_UNUSED, int type EINA_UNUSED, E_Event_Module_Update *ev)
{
   if (!ev->enabled)
     {
        Eina_List *l;
        E_Config_Gadcon_Client *cf_gcc;
        E_Gadcon_Client *gcc;
        l = eina_hash_set(_gadman_gadgets, ev->name, NULL);
        if (!l) return ECORE_CALLBACK_RENEW;
        EINA_LIST_FREE(l, cf_gcc)
          {
             gcc = e_gadcon_client_find(NULL, cf_gcc);
             if (!gcc) continue;
             gcc->cf = NULL;
             e_object_del(E_OBJECT(gcc));
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_gadman_cb_zone_change(void *data EINA_UNUSED, int type, void *event)
{
   E_Gadcon *gc;
   Eina_List *l, *ll;
   E_Event_Zone_Move_Resize *ev = event;
   const char *layer_name[] = {"gadman", "gadman_top"};
   int layer;
   E_Gadcon_Client *gcc;

   if (!Man) return ECORE_CALLBACK_RENEW;
   if (gadman_locked) return ECORE_CALLBACK_RENEW;
   if ((type == E_EVENT_ZONE_MOVE_RESIZE) || (type == E_EVENT_ZONE_USEFUL_GEOMETRY_CHANGED))
     {
        /* probably zone dirty being set */
        EINA_LIST_FOREACH(Man->gadcons[GADMAN_LAYER_BG], l, gc)
          {
             if (e_gadcon_zone_get(gc) != ev->zone) continue;
             EINA_LIST_FOREACH(gc->clients, ll, gcc)
               _apply_widget_position(gcc);
             return ECORE_CALLBACK_RENEW;
          }
     }
   if (type == E_EVENT_ZONE_DEL)
     {
        for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
          {
             EINA_LIST_FOREACH(Man->gadcons[layer], l, gc)
               {
                  if (e_gadcon_zone_get(gc) != ev->zone) continue;
                  e_object_del(E_OBJECT(gc));
                  Man->gadcons[layer] = eina_list_remove_list(Man->gadcons[layer], l);
                  E_FREE_FUNC(Man->movers[layer], evas_object_del);
                  break;
               }
          }
        evas_object_hide(Man->overlay);
        E_FREE_FUNC(Man->overlay, evas_object_del);
        _gadman_overlay_create();
        return ECORE_CALLBACK_RENEW;
     }

   for (layer = 0; layer < GADMAN_LAYER_COUNT; layer++)
     {
        Eina_Bool found = EINA_FALSE;
        EINA_LIST_FOREACH(Man->gadcons[GADMAN_LAYER_BG], l, gc)
          if (e_gadcon_zone_get(gc) == ev->zone)
            {
               found = EINA_TRUE;
               break;
            }
        if (found) continue;
        gc = _gadman_gadcon_new(layer_name[layer], layer, ev->zone, Man->location[layer]);
        Man->gadcons[layer] = eina_list_append(Man->gadcons[layer], gc);
        gc->drop_handler->base = Man->overlay;
        gc->drop_handler->hidden = 1;
     }
   return ECORE_CALLBACK_PASS_ON;
}
