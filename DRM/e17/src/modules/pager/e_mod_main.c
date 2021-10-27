#include "e.h"
#include "e_mod_main.h"

/* gadcon requirements */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void             _gc_shutdown(E_Gadcon_Client *gcc);
static void             _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED);
static const char      *_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED);
static Evas_Object     *_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas);
static const char      *_gc_id_new(const E_Gadcon_Client_Class *client_class EINA_UNUSED);

/* and actually define the gadcon class that this module provides (just 1) */
static const E_Gadcon_Client_Class _gadcon_class =
{
   GADCON_CLIENT_CLASS_VERSION,
   "pager",
   {
      _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL,
      e_gadcon_site_is_not_toolbar
   },
   E_GADCON_CLIENT_STYLE_INSET
};

/* actual module specifics */
typedef struct _Instance    Instance;
typedef struct _Pager       Pager;
typedef struct _Pager_Desk  Pager_Desk;
typedef struct _Pager_Win   Pager_Win;
typedef struct _Pager_Popup Pager_Popup;

struct _Instance
{
   E_Gadcon_Client *gcc;
   Evas_Object     *o_pager; /* table */
   Evas_Object     *o_base;
   Pager           *pager;
};

struct _Pager
{
   Instance       *inst;
   E_Drop_Handler *drop_handler;
   Pager_Popup    *popup;
   Evas_Object    *o_table;
   E_Zone         *zone;
   int             xnum, ynum;
   Eina_List      *desks;
   Pager_Desk     *active_pd;
   struct {
      int          zone_num, desk_x, desk_y;
   } menu;
   unsigned char   dragging E_BITFIELD;
   unsigned char   just_dragged E_BITFIELD;
   Evas_Coord      dnd_x, dnd_y;
   Pager_Desk     *active_drop_pd;
   E_Client       *active_drag_client;
   Ecore_Job      *recalc;
   Eina_Bool       invert E_BITFIELD;
   Eina_Bool       noshelf E_BITFIELD;
};

struct _Pager_Desk
{
   Pager           *pager;
   E_Desk          *desk;
   Eina_List       *wins;
   Evas_Object     *o_desk;
   Evas_Object     *o_layout;
   int              xpos, ypos, urgent;
   int              current E_BITFIELD;
   struct {
      Pager        *from_pager;
      unsigned char in_pager E_BITFIELD;
      unsigned char start E_BITFIELD;
      int           x, y, dx, dy, button;
   } drag;
};

struct _Pager_Win
{
   E_Client         *client;
   Pager_Desk       *desk;
   Evas_Object      *o_window;
   Evas_Object      *o_mirror;
   unsigned char     skip_winlist E_BITFIELD;
   struct {
      Pager         *from_pager;
      unsigned char  start E_BITFIELD;
      unsigned char  in_pager E_BITFIELD;
      unsigned char  desktop  E_BITFIELD;
      int            x, y, dx, dy, button;
   } drag;
};

struct _Pager_Popup
{
   Evas_Object   *popup;
   Evas_Object   *o_bg;
   Pager         *pager;
   Ecore_Timer   *timer;
   unsigned char  urgent E_BITFIELD;
};

static void             _pager_cb_mirror_add(Pager_Desk *pd, Evas_Object *obj, Evas_Object *mirror);

static void             _pager_cb_obj_show(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void             _pager_cb_obj_hide(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void             _pager_cb_obj_moveresize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void             _button_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void             _pager_inst_cb_menu_configure(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED);
static void             _pager_inst_cb_menu_virtual_desktops_dialog(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED);
static void             _pager_inst_cb_menu_virtual_desktop_dialog(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED);
static void             _pager_instance_drop_zone_recalc(Instance *inst);
static Eina_Bool        _pager_cb_event_desk_show(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static Eina_Bool        _pager_cb_event_desk_name_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static Eina_Bool        _pager_cb_event_compositor_resize(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static void             _pager_window_cb_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void             _pager_window_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void             _pager_window_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void             _pager_window_cb_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void            *_pager_window_cb_drag_convert(E_Drag *drag, const char *type);
static void             _pager_window_cb_drag_finished(E_Drag *drag, int dropped);
static void             _pager_drop_cb_enter(void *data, const char *type EINA_UNUSED, void *event_info);
static void             _pager_drop_cb_move(void *data, const char *type EINA_UNUSED, void *event_info);
static void             _pager_drop_cb_leave(void *data, const char *type EINA_UNUSED, void *event_info EINA_UNUSED);
static void             _pager_drop_cb_drop(void *data, const char *type, void *event_info);
static void             _pager_inst_cb_scroll(void *data);
static void             _pager_update_drop_position(Pager *p, Evas_Coord x, Evas_Coord y);
static void             _pager_desk_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void             _pager_desk_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void             _pager_desk_cb_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static void             _pager_desk_cb_drag_finished(E_Drag *drag, int dropped);
static void             _pager_desk_cb_mouse_wheel(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info);
static Eina_Bool        _pager_popup_cb_timeout(void *data);
static Pager           *_pager_new(Evas *evas, E_Zone *zone, E_Gadcon *gc, Instance *inst);
static void             _pager_free(Pager *p);
static void             _pager_fill(Pager *p, E_Gadcon *gc);
static void             _pager_empty(Pager *p);
static Pager_Desk      *_pager_desk_new(Pager *p, E_Desk *desk, int xpos, int ypos, Eina_Bool invert);
static void             _pager_desk_free(Pager_Desk *pd);
static Pager_Desk      *_pager_desk_at_coord(Pager *p, Evas_Coord x, Evas_Coord y);
static void             _pager_desk_select(Pager_Desk *pd);
static Pager_Desk      *_pager_desk_find(Pager *p, E_Desk *desk);
static void             _pager_desk_switch(Pager_Desk *pd1, Pager_Desk *pd2);
static Pager_Win       *_pager_window_new(Pager_Desk *pd, Evas_Object *mirror, E_Client *client);
static void             _pager_window_free(Pager_Win *pw);
static Pager_Popup     *_pager_popup_new(E_Zone *zone, int keyaction, Eina_Bool pass_events);
static void             _pager_popup_free(Pager_Popup *pp);
static Pager_Popup     *_pager_popup_find(E_Zone *zone);

/* functions for pager popup on key actions */
static int              _pager_popup_show(void);
static void             _pager_popup_hide(int switch_desk);
static Eina_Bool        _pager_popup_cb_mouse_wheel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static void             _pager_popup_desk_switch(int x, int y);
static void             _pager_popup_cb_action_show(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED, Ecore_Event_Key *ev EINA_UNUSED);
static void             _pager_popup_cb_action_switch(E_Object *obj EINA_UNUSED, const char *params, Ecore_Event_Key *ev EINA_UNUSED);

/* variables for pager popup on key actions */
static E_Action     *act_popup_show = NULL;
static E_Action     *act_popup_switch = NULL;
static Ecore_Window  input_window = 0;
static Eina_List    *handlers = NULL;
static Pager_Popup  *act_popup = NULL; /* active popup */
static E_Desk       *current_desk = NULL;
static Eina_List    *pagers = NULL;
static double        _pager_start_time = 0.0;
static E_Config_DD  *conf_edd = NULL;

EINTERN E_Module        *module = NULL;
EINTERN E_Config_Dialog *config_dialog = NULL;
EINTERN Eina_List       *instances = NULL;
EINTERN Eina_List       *shandlers = NULL;
EINTERN Config          *pager_config = NULL;

static Pager_Win *
_pager_desk_window_find(Pager_Desk *pd, E_Client *client)
{
   Eina_List *l;
   Pager_Win *pw;

   EINA_LIST_FOREACH(pd->wins, l, pw)
     {
        if (pw->client == client) return pw;
     }
   return NULL;
}

static Pager_Win *
_pager_window_find(Pager *p, E_Client *client)
{
   Eina_List *l;
   Pager_Desk *pd;

   EINA_LIST_FOREACH(p->desks, l, pd)
     {
        Pager_Win *pw = _pager_desk_window_find(pd, client);
        if (pw) return pw;
     }
   return NULL;
}

static void
_emit_orient(Instance *inst, E_Gadcon_Orient orient)
{
   static const char *signals[] =
     {
        "float",
        "horiz",
        "vert",
        "left",
        "right",
        "top",
        "bottom",
        "corner_tl",
        "corner_tr",
        "corner_bl",
        "corner_br",
        "corner_lt",
        "corner_rt",
        "corner_lb",
        "corner_rb"
     };
   char buf[256];

   if (orient > E_GADCON_ORIENT_CORNER_RB) orient = E_GADCON_ORIENT_HORIZ;
   snprintf(buf, sizeof(buf), "e,state,orient,%s", signals[orient]);

   if (inst->o_base)
     {
        edje_object_signal_emit(inst->o_base, buf, "e");
        edje_object_message_signal_process(inst->o_base);
     }
}

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Pager *p;
   Evas_Object *o;
   E_Gadcon_Client *gcc;
   Instance *inst;
   Evas_Coord x, y, w, h;
   const char *drop[] = {
      "enlightenment/pager_win",
      "enlightenment/border",
      "enlightenment/vdesktop"
   };

   inst = E_NEW(Instance, 1);

   p = _pager_new(gc->evas, gc->zone, gc, inst);
   o = edje_object_add(gc->evas);
   if (!e_theme_edje_object_set(o, "base/theme/modules/pager",
                                "e/modules/pager16/base"))
     {
        evas_object_del(o);
        o = p->o_table;
     }
   else
     {
        edje_object_part_swallow(o, "e.swallow.content", p->o_table);
        inst->o_base = o;
     }
   gcc = e_gadcon_client_new(gc, name, id, style, o);
   gcc->data = inst;

   inst->gcc = gcc;
   inst->o_pager = o;

   _emit_orient(inst, gc->orient);

   evas_object_geometry_get(o, &x, &y, &w, &h);
   p->drop_handler =
     e_drop_handler_add(E_OBJECT(inst->gcc), NULL, p,
                        _pager_drop_cb_enter, _pager_drop_cb_move,
                        _pager_drop_cb_leave, _pager_drop_cb_drop,
                        drop, 3, x, y, w, h);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOVE,
                                  _pager_cb_obj_moveresize, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE,
                                  _pager_cb_obj_moveresize, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
                                  _button_cb_mouse_down, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_SHOW,
                                  _pager_cb_obj_show, inst);
   evas_object_event_callback_add(o, EVAS_CALLBACK_HIDE,
                                  _pager_cb_obj_hide, inst);
   instances = eina_list_append(instances, inst);
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst;

   inst = gcc->data;
   if (pager_config)
     instances = eina_list_remove(instances, inst);
   if (inst->o_base)
     {
        evas_object_del(inst->o_base);
        inst->o_base = NULL;
     }
   _pager_free(inst->pager);
   free(inst);
}

static void
_aspect(E_Gadcon_Client *gcc)
{
   Instance *inst;
   int aspect_w, aspect_h;
   double aspect_ratio;

   inst = gcc->data;
   _emit_orient(inst, gcc->gadcon->orient);
   if (inst->pager->noshelf)
     evas_object_geometry_get(inst->pager->o_table, NULL, NULL, &aspect_w, &aspect_h);
   else if (inst->pager->invert)
     {
        aspect_w = inst->pager->ynum * inst->pager->zone->w;
        aspect_h = inst->pager->xnum * inst->pager->zone->h;
     }
   else
     {
        aspect_w = inst->pager->xnum * inst->pager->zone->w;
        aspect_h = inst->pager->ynum * inst->pager->zone->h;
     }

   e_gadcon_client_aspect_set(gcc, aspect_w, aspect_h);
   aspect_ratio = (double)aspect_w / (double)aspect_h;

   if (aspect_ratio > 1.0)
     e_gadcon_client_min_size_set(gcc, 4 * aspect_ratio, 4);
   else
     e_gadcon_client_min_size_set(gcc, 4, 4 * aspect_ratio);
}

static void
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient EINA_UNUSED)
{
   _aspect(gcc);
}

static const char *
_gc_label(const E_Gadcon_Client_Class *client_class EINA_UNUSED)
{
   return _("Pager");
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class EINA_UNUSED, Evas *evas)
{
   Evas_Object *o;
   char buf[PATH_MAX];

   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/e-module-pager.edj",
            e_module_dir_get(module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class)
{
   static char buf[4096];

   snprintf(buf, sizeof(buf), "%s.%d", client_class->name,
            eina_list_count(instances) + 1);
   return buf;
}

static void
_pager_recalc(void *data)
{
   Pager *p = data;
   Pager_Desk *pd;
   Evas_Coord mw = 0, mh = 0;
   int w, h, zw, zh, w2, h2;
   E_Gadcon_Orient orient;

   p->recalc = NULL;
   if (!p->inst || !p->inst->gcc || !p->inst->gcc->gadcon) return;

   zw = p->zone->w; zh = p->zone->h;
   pd = eina_list_data_get(p->desks);
   if (!pd) return;

   orient = p->inst->gcc->gadcon->orient;
   switch (orient)
     {
        case E_GADCON_ORIENT_FLOAT:
        case E_GADCON_ORIENT_VERT:
        case E_GADCON_ORIENT_HORIZ:
          p->noshelf = 1;
          break;
        default:
          break;
     }

   edje_object_size_min_calc(pd->o_desk, &mw, &mh);
   evas_object_geometry_get(pd->o_desk, NULL, NULL, &w, &h);
   w -= mw; h -= mh;
   w2 = w; h2 = (zh * w) / zw;
   if (h2 > h)
     {
        h2 = h; w2 = (zw * h) / zh;
     }
   w = w2; h = h2;
   w += mw; h += mh;

   _emit_orient(p->inst, orient);
   if (p->invert)
     e_gadcon_client_aspect_set(p->inst->gcc, p->ynum * w, p->xnum * h);
   else
     e_gadcon_client_aspect_set(p->inst->gcc, p->xnum * w, p->ynum * h);
   _aspect(p->inst->gcc);
}

static void
_pager_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Pager *p = data;

   if (p->recalc) ecore_job_del(p->recalc);
   p->recalc = ecore_job_add(_pager_recalc, p);
}

static Pager *
_pager_new(Evas *evas, E_Zone *zone, E_Gadcon *gc, Instance *inst)
{
   Pager *p;

   p = E_NEW(Pager, 1);
   p->inst = inst;
   if (inst) inst->pager = p;
   p->o_table = elm_table_add(e_win_evas_win_get(evas));
   evas_object_event_callback_add(p->o_table, EVAS_CALLBACK_RESIZE,
                                  _pager_resize, p);
   elm_table_homogeneous_set(p->o_table, 1);
   p->zone = zone;
   _pager_fill(p, gc);
   pagers = eina_list_append(pagers, p);
   return p;
}

static void
_pager_free(Pager *p)
{
   pagers = eina_list_remove(pagers, p);
   _pager_empty(p);
   if (p->drop_handler) e_drop_handler_del(p->drop_handler);
   p->drop_handler = NULL;
   evas_object_del(p->o_table);
   p->o_table = NULL;
   ecore_job_del(p->recalc);
   p->recalc = NULL;
   free(p);
}

static void
_pager_fill(Pager *p, E_Gadcon *gc)
{
   int x, y;

   if (gc)
     {
        switch (gc->orient)
          {

             case E_GADCON_ORIENT_TOP:
             case E_GADCON_ORIENT_BOTTOM:
             case E_GADCON_ORIENT_CORNER_TL:
             case E_GADCON_ORIENT_CORNER_TR:
             case E_GADCON_ORIENT_CORNER_BL:
             case E_GADCON_ORIENT_CORNER_BR:
             case E_GADCON_ORIENT_HORIZ:
             case E_GADCON_ORIENT_FLOAT:
               p->invert = EINA_FALSE;
               break;
             case E_GADCON_ORIENT_VERT:
             case E_GADCON_ORIENT_LEFT:
             case E_GADCON_ORIENT_RIGHT:
             case E_GADCON_ORIENT_CORNER_LT:
             case E_GADCON_ORIENT_CORNER_RT:
             case E_GADCON_ORIENT_CORNER_LB:
             case E_GADCON_ORIENT_CORNER_RB:
             default:
               p->invert = EINA_TRUE;
          }
     }
   e_zone_desk_count_get(p->zone, &(p->xnum), &(p->ynum));
   if (p->ynum != 1) p->invert = EINA_FALSE;
   for (x = 0; x < p->xnum; x++)
     {
        for (y = 0; y < p->ynum; y++)
          {
             Pager_Desk *pd;
             E_Desk *desk;

             desk = e_desk_at_xy_get(p->zone, x, y);
             if (desk)
               {
                  pd = _pager_desk_new(p, desk, x, y, p->invert);
                  if (pd)
                    {
                       p->desks = eina_list_append(p->desks, pd);
                       if (desk == e_desk_current_get(desk->zone))
                         _pager_desk_select(pd);
                    }
               }
          }
     }
}

static void
_pager_empty(Pager *p)
{
   p->active_pd = NULL;
   E_FREE_LIST(p->desks, _pager_desk_free);
}

static Pager_Desk *
_pager_desk_new(Pager *p, E_Desk *desk, int xpos, int ypos, Eina_Bool invert)
{
   Pager_Desk *pd;
   Evas_Object *o, *evo;
   E_Client *ec;
   Eina_List *l;
   int w, h;
   Evas *e;

   if (!desk) return NULL;
   pd = E_NEW(Pager_Desk, 1);
   if (!pd) return NULL;

   pd->xpos = xpos;
   pd->ypos = ypos;
   pd->urgent = 0;
   pd->desk = desk;
   e_object_ref(E_OBJECT(desk));
   pd->pager = p;

   e = evas_object_evas_get(p->o_table);
   o = edje_object_add(e);
   pd->o_desk = o;
   e_theme_edje_object_set(o, "base/theme/modules/pager",
                           "e/modules/pager16/desk");
   edje_object_part_text_set(o, "e.text.label", desk->name);
   if (pager_config->show_desk_names)
     edje_object_signal_emit(o, "e,name,show", "e");

   edje_object_size_min_calc(o, &w, &h);
   evas_object_size_hint_min_set(o, w, h);
   E_EXPAND(o);
   E_FILL(o);
   if (invert)
     elm_table_pack(p->o_table, o, ypos, xpos, 1, 1);
   else
     elm_table_pack(p->o_table, o, xpos, ypos, 1, 1);

   evo = (Evas_Object *)edje_object_part_object_get(o, "e.eventarea");
   if (!evo) evo = o;

   evas_object_event_callback_add(evo, EVAS_CALLBACK_MOUSE_DOWN,
                                  _pager_desk_cb_mouse_down, pd);
   evas_object_event_callback_add(evo, EVAS_CALLBACK_MOUSE_UP,
                                  _pager_desk_cb_mouse_up, pd);
   evas_object_event_callback_add(evo, EVAS_CALLBACK_MOUSE_MOVE,
                                  _pager_desk_cb_mouse_move, pd);
   evas_object_event_callback_add(evo, EVAS_CALLBACK_MOUSE_WHEEL,
                                  _pager_desk_cb_mouse_wheel, pd);
   evas_object_show(o);

   pd->o_layout = e_deskmirror_add(desk, 1, 0);
   evas_object_smart_callback_add(pd->o_layout, "mirror_add", (Evas_Smart_Cb)_pager_cb_mirror_add, pd);

   l = e_deskmirror_mirror_list(pd->o_layout);
   EINA_LIST_FREE(l, o)
     {
        ec = evas_object_data_get(o, "E_Client");
        if (ec)
          {
             Pager_Win *pw;

             pw = _pager_window_new(pd, o, ec);
             if (pw) pd->wins = eina_list_append(pd->wins, pw);
          }
     }
   edje_object_part_swallow(pd->o_desk, "e.swallow.content", pd->o_layout);
   evas_object_show(pd->o_layout);

   return pd;
}

static void
_pager_desk_free(Pager_Desk *pd)
{
   Pager_Win *w;

   evas_object_del(pd->o_desk);
   evas_object_del(pd->o_layout);
   EINA_LIST_FREE(pd->wins, w)
     _pager_window_free(w);
   e_object_unref(E_OBJECT(pd->desk));
   free(pd);
}

static Pager_Desk *
_pager_desk_at_coord(Pager *p, Evas_Coord x, Evas_Coord y)
{
   Eina_List *l;
   Pager_Desk *pd;

   EINA_LIST_FOREACH(p->desks, l, pd)
     {
        Evas_Coord dx, dy, dw, dh;

        evas_object_geometry_get(pd->o_desk, &dx, &dy, &dw, &dh);
        if (E_INSIDE(x, y, dx, dy, dw, dh)) return pd;
     }
   return NULL;
}

static void
_pager_desk_select(Pager_Desk *pd)
{
   if (pd->current) return;
   if (pd->pager->active_pd)
     {
        pd->pager->active_pd->current = 0;
        edje_object_signal_emit(pd->pager->active_pd->o_desk, "e,state,unselected", "e");
     }
   pd->current = 1;
   evas_object_raise(pd->o_desk);
   edje_object_signal_emit(pd->o_desk, "e,state,selected", "e");
   pd->pager->active_pd = pd;
}

static Pager_Desk *
_pager_desk_find(Pager *p, E_Desk *desk)
{
   Eina_List *l;
   Pager_Desk *pd;

   EINA_LIST_FOREACH(p->desks, l, pd)
     if (pd->desk == desk) return pd;

   return NULL;
}

static void
_pager_desk_switch(Pager_Desk *pd1, Pager_Desk *pd2)
{
   int c;
   E_Zone *zone1, *zone2;
   E_Desk *desk1, *desk2;
   Pager_Win *pw;
   Eina_List *l;

   if ((!pd1) || (!pd2) || (!pd1->desk) || (!pd2->desk)) return;
   if (pd1 == pd2) return;

   desk1 = pd1->desk;
   desk2 = pd2->desk;
   zone1 = pd1->desk->zone;
   zone2 = pd2->desk->zone;

   /* Move opened windows from on desk to the other */
   EINA_LIST_FOREACH(pd1->wins, l, pw)
     {
        if ((!pw) || (!pw->client) || (pw->client->iconic)) continue;
        pw->client->hidden = 0;
        e_client_desk_set(pw->client, desk2);
     }
   EINA_LIST_FOREACH(pd2->wins, l, pw)
     {
        if ((!pw) || (!pw->client) || (pw->client->iconic)) continue;
        pw->client->hidden = 0;
        e_client_desk_set(pw->client, desk1);
     }
   e_deskmirror_update_force(pd1->o_layout);
   e_deskmirror_update_force(pd2->o_layout);

   /* Modify desktop names in the config */
   for (l = e_config->desktop_names, c = 0; l && c < 2; l = l->next)
     {
        E_Config_Desktop_Name *tmp_dn;

        tmp_dn = l->data;
        if (!tmp_dn) continue;
        if ((tmp_dn->desk_x == desk1->x) &&
            (tmp_dn->desk_y == desk1->y) &&
            (tmp_dn->zone == (int)desk1->zone->num))
          {
             tmp_dn->desk_x = desk2->x;
             tmp_dn->desk_y = desk2->y;
             tmp_dn->zone = desk2->zone->num;
             c++;
          }
        else if ((tmp_dn->desk_x == desk2->x) &&
                 (tmp_dn->desk_y == desk2->y) &&
                 (tmp_dn->zone == (int)desk2->zone->num))
          {
             tmp_dn->desk_x = desk1->x;
             tmp_dn->desk_y = desk1->y;
             tmp_dn->zone = desk1->zone->num;
             c++;
          }
     }
   if (c > 0) e_config_save();
   e_desk_name_update();

   /* Modify desktop backgrounds in the config */
   for (l = e_config->desktop_backgrounds, c = 0; l && c < 2; l = l->next)
     {
        E_Config_Desktop_Background *tmp_db;

        tmp_db = l->data;
        if (!tmp_db) continue;
        if ((tmp_db->desk_x == desk1->x) &&
            (tmp_db->desk_y == desk1->y) &&
            (tmp_db->zone == (int)desk1->zone->num))
          {
             tmp_db->desk_x = desk2->x;
             tmp_db->desk_y = desk2->y;
             tmp_db->zone = desk2->zone->num;
             c++;
          }
        else if ((tmp_db->desk_x == desk2->x) &&
                 (tmp_db->desk_y == desk2->y) &&
                 (tmp_db->zone == (int)desk2->zone->num))
          {
             tmp_db->desk_x = desk1->x;
             tmp_db->desk_y = desk1->y;
             tmp_db->zone = desk1->zone->num;
             c++;
          }
     }
   if (c > 0) e_config_save();

   /* If the current desktop has been switched, force to update of the screen */
   if (desk2 == e_desk_current_get(zone2))
     {
        desk2->visible = 0;
        e_desk_show(desk2);
     }
   if (desk1 == e_desk_current_get(zone1))
     {
        desk1->visible = 0;
        e_desk_show(desk1);
     }
}

static Pager_Win *
_pager_window_new(Pager_Desk *pd, Evas_Object *mirror, E_Client *client)
{
   Pager_Win *pw;

   if (!client) return NULL;
   pw = E_NEW(Pager_Win, 1);
   if (!pw) return NULL;

   pw->client = client;
   pw->o_mirror = mirror;

   pw->desk = pd;

   evas_object_event_callback_add(mirror, EVAS_CALLBACK_MOUSE_DOWN,
                                  _pager_window_cb_mouse_down, pw);
   evas_object_event_callback_add(mirror, EVAS_CALLBACK_MOUSE_UP,
                                  _pager_window_cb_mouse_up, pw);
   evas_object_event_callback_add(mirror, EVAS_CALLBACK_MOUSE_MOVE,
                                  _pager_window_cb_mouse_move, pw);
   evas_object_event_callback_add(mirror, EVAS_CALLBACK_DEL,
                                  _pager_window_cb_del, pw);

   if (client->urgent)
     {
        if (!(client->iconic))
          edje_object_signal_emit(pd->o_desk, "e,state,urgent", "e");
     }
   return pw;
}

static void
_pager_window_free(Pager_Win *pw)
{
   if ((pw->drag.from_pager) && (pw->desk->pager->dragging))
     pw->desk->pager->dragging = 0;
   if (pw->o_mirror)
     evas_object_event_callback_del_full(pw->o_mirror, EVAS_CALLBACK_DEL,
                                    _pager_window_cb_del, pw);
   if (pw->o_window) evas_object_del(pw->o_window);
   free(pw);
}

static void
_pager_popup_cb_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Pager_Popup *pp = data;
   E_FREE_FUNC(pp->timer, ecore_timer_del);
   _pager_free(pp->pager);
   free(pp);
}

static void
_popup_autoclose_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{
   if (act_popup) _pager_popup_hide(0);
}

static Eina_Bool
_popup_autoclose_key_cb(void *data EINA_UNUSED, Ecore_Event_Key *ev)
{
   if (!strcmp(ev->key, "Up"))          _pager_popup_desk_switch(0, -1);
   else if (!strcmp(ev->key, "Down"))   _pager_popup_desk_switch(0, 1);
   else if (!strcmp(ev->key, "Left"))   _pager_popup_desk_switch(-1, 0);
   else if (!strcmp(ev->key, "Right"))  _pager_popup_desk_switch(1, 0);
   else if (!strcmp(ev->key, "Escape")) return EINA_FALSE;
   else if ((!strcmp(ev->key, "Return")) ||
            (!strcmp(ev->key, "KP_Enter")) ||
            (!strcmp(ev->key, "space")))
     {
        Pager_Popup *pp = act_popup;

        if (pp)
          {
             E_Desk *desk = e_desk_at_xy_get(pp->pager->zone,
                                             current_desk->x,
                                             current_desk->y);
             if (desk) e_desk_show(desk);
          }
        return EINA_FALSE;
     }
   else
     {
        E_Config_Binding_Key *binding;
        Eina_List *l;

        EINA_LIST_FOREACH(e_bindings->key_bindings, l, binding)
          {
             E_Binding_Modifier mod = 0;

             if ((binding->action) && (strcmp(binding->action, "pager_switch")))
               continue;

             if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
               mod |= E_BINDING_MODIFIER_SHIFT;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL)
               mod |= E_BINDING_MODIFIER_CTRL;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_ALT)
               mod |= E_BINDING_MODIFIER_ALT;
             if (ev->modifiers & ECORE_EVENT_MODIFIER_WIN)
               mod |= E_BINDING_MODIFIER_WIN;

             if (binding->key && (!strcmp(binding->key, ev->key)) &&
                 ((binding->modifiers == mod)))
               {
                  E_Action *act;

                  act = e_action_find(binding->action);

                  if (act)
                    {
                       if (act->func.go_key)
                         act->func.go_key(NULL, binding->params, ev);
                    }
               }
          }
     }
   return EINA_TRUE;
}

static Pager_Popup *
_pager_popup_new(E_Zone *zone, int keyaction, Eina_Bool pass_events)
{
   Pager_Popup *pp;
   Evas_Coord w, h, zx, zy, zw, zh;
   int x, y, height, width;
   E_Desk *desk;
   Pager_Desk *pd;

   pp = E_NEW(Pager_Popup, 1);
   if (!pp) return NULL;

   /* Show popup */
   pp->pager = _pager_new(e_comp->evas, zone, NULL, NULL);

   pp->pager->popup = pp;
   pp->urgent = 0;

   e_zone_desk_count_get(zone, &x, &y);

   if (keyaction) height = pager_config->popup_act_height * e_scale;
   else           height = pager_config->popup_height * e_scale;

   pd = eina_list_data_get(pp->pager->desks);
   if (!pd)
     {
        height *= y;
        width = height * (zone->w * x) / (zone->h * y);
     }
   else
     {
        Evas_Coord mw = 0, mh = 0;

        edje_object_size_min_calc(pd->o_desk, &mw, &mh);
        height -= mh;
        width = (height * zone->w) / zone->h;
        height *= y;
        height += (y * mh);
        width *= x;
        width += (x * mw);
     }

   evas_object_move(pp->pager->o_table, 0, 0);
   evas_object_resize(pp->pager->o_table, width, height);

   pp->o_bg = edje_object_add(e_comp->evas);
   evas_object_name_set(pp->o_bg, "pager_popup");
   e_theme_edje_object_set(pp->o_bg, "base/theme/modules/pager",
                           "e/modules/pager16/popup");
   desk = e_desk_current_get(zone);
   if (desk) edje_object_part_text_set(pp->o_bg, "e.text.label", desk->name);

   evas_object_size_hint_min_set(pp->pager->o_table, width, height);
   edje_object_part_swallow(pp->o_bg, "e.swallow.content", pp->pager->o_table);
   edje_object_size_min_calc(pp->o_bg, &w, &h);

   pp->popup = e_comp_object_util_add(pp->o_bg, E_COMP_OBJECT_TYPE_POPUP);
   evas_object_layer_set(pp->popup, E_LAYER_CLIENT_POPUP);
   evas_object_pass_events_set(pp->popup, pass_events);
   e_zone_useful_geometry_get(zone, &zx, &zy, &zw, &zh);
   evas_object_geometry_set(pp->popup, zx, zy, w, h);
   e_comp_object_util_center(pp->popup);
   evas_object_event_callback_add(pp->popup, EVAS_CALLBACK_DEL, _pager_popup_cb_del, pp);
   evas_object_show(pp->popup);

   if (!pass_events)
     {
        const char *drop[] = {
           "enlightenment/pager_win",
           "enlightenment/border",
           "enlightenment/vdesktop"
        };

        evas_object_geometry_get(pp->pager->o_table, &x, &y, &w, &h);
        pp->pager->drop_handler =
          e_drop_handler_add(E_OBJECT(zone), NULL, pp->pager,
                             _pager_drop_cb_enter, _pager_drop_cb_move,
                             _pager_drop_cb_leave, _pager_drop_cb_drop,
                             drop, 3, x, y, w, h);
        e_comp_object_util_autoclose(pp->popup,
                                     _popup_autoclose_cb,
                                     _popup_autoclose_key_cb, NULL);
     }

   pp->timer = NULL;

   return pp;
}

static void
_pager_popup_free(Pager_Popup *pp)
{
   E_FREE_FUNC(pp->timer, ecore_timer_del);
   evas_object_hide(pp->popup);
   evas_object_del(pp->popup);
}

static Pager_Popup *
_pager_popup_find(E_Zone *zone)
{
   Eina_List *l;
   Pager *p;

   EINA_LIST_FOREACH(pagers, l, p)
     {
        if (!p->inst) continue;
        if ((p->popup) && (p->zone == zone)) return p->popup;
     }
   return NULL;
}

static void
_pager_cb_obj_hide(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst = data;
   Eina_List *l;
   Pager_Desk *pd;

   EINA_LIST_FOREACH(inst->pager->desks, l, pd)
     {
        edje_object_signal_emit(pd->o_desk, "e,state,hidden", "e");
     }
}

static void
_pager_cb_obj_show(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst = data;
   Eina_List *l;
   Pager_Desk *pd;

   EINA_LIST_FOREACH(inst->pager->desks, l, pd)
     {
        edje_object_signal_emit(pd->o_desk, "e,state,visible", "e");
     }
}

static void
_pager_cb_obj_moveresize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Instance *inst;

   inst = data;
   _pager_instance_drop_zone_recalc(inst);
}

static void
_button_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;
   E_Menu *m;
   E_Menu_Item *mi;
   int cx, cy;
   Pager_Desk *pd;

   inst = data;
   ev = event_info;
   if (ev->button != 3) return;
   if (inst->gcc->menu) return;

   m = e_menu_new();
   mi = e_menu_item_new(m);
   e_menu_item_label_set(mi, _("Settings"));
   e_util_menu_item_theme_icon_set(mi, "configure");
   e_menu_item_callback_set(mi, _pager_inst_cb_menu_configure, NULL);

   m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);

   mi = e_menu_item_new_relative(m, NULL);
   e_menu_item_separator_set(mi, 1);

   if (e_configure_registry_exists("screen/virtual_desktops"))
     {
        mi = e_menu_item_new_relative(m, NULL);
        e_menu_item_label_set(mi, _("All desktop settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-desktop");
        e_menu_item_callback_set(mi, _pager_inst_cb_menu_virtual_desktops_dialog, inst);
     }
   if (e_configure_registry_exists("internal/desk"))
     {
        pd = _pager_desk_at_coord(inst->pager, ev->canvas.x, ev->canvas.y);
        inst->pager->menu.zone_num = inst->pager->zone->num;
        inst->pager->menu.desk_x = pd->desk->x;
        inst->pager->menu.desk_y = pd->desk->y;
        mi = e_menu_item_new_relative(m, NULL);
        e_menu_item_label_set(mi, _("This desktop name and wallpaper settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-desktop");
        e_menu_item_callback_set(mi, _pager_inst_cb_menu_virtual_desktop_dialog, inst);
     }

   e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &cx, &cy,
                                     NULL, NULL);
   e_menu_activate_mouse(m, e_zone_current_get(),
                         cx + ev->output.x, cy + ev->output.y, 1, 1,
                         E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
   evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button,
                            EVAS_BUTTON_NONE, ev->timestamp, NULL);
}

static void
_pager_inst_cb_menu_configure(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   if (!pager_config) return;
   if (config_dialog) return;
   /* FIXME: pass zone config item */
   _config_pager_module(NULL);
}

static E_Config_Dialog *
_pager_config_dialog(Evas_Object *parent EINA_UNUSED, const char *params EINA_UNUSED)
{
   if (!pager_config) return NULL;
   if (config_dialog) return NULL;
   /* FIXME: pass zone config item */
   _config_pager_module(NULL);
   return config_dialog;
}

static void
_pager_inst_cb_menu_virtual_desktops_dialog(void *data EINA_UNUSED, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   e_configure_registry_call("screen/virtual_desktops", NULL, NULL);
}

static void
_pager_inst_cb_menu_virtual_desktop_dialog(void *data, E_Menu *m EINA_UNUSED, E_Menu_Item *mi EINA_UNUSED)
{
   Instance *inst = data;
   char buf[256];

   snprintf(buf, sizeof(buf), "%i %i %i", inst->pager->menu.zone_num,
            inst->pager->menu.desk_x, inst->pager->menu.desk_y);
   e_configure_registry_call("internal/desk", NULL, buf);
}

static void
_pager_instance_drop_zone_recalc(Instance *inst)
{
   Evas_Coord x, y, w, h;

   e_gadcon_client_viewport_geometry_get(inst->gcc, &x, &y, &w, &h);
   e_drop_handler_geometry_set(inst->pager->drop_handler, x, y, w, h);
}

EINTERN void
_pager_cb_config_updated(void)
{
   Pager *p;
   Pager_Desk *pd;
   Eina_List *l, *ll;

   if (!pager_config) return;
   EINA_LIST_FOREACH(pagers, l, p)
     {
        EINA_LIST_FOREACH(p->desks, ll, pd)
          {
             if (pd->current)
               edje_object_signal_emit(pd->o_desk, "e,state,selected", "e");
             else
               edje_object_signal_emit(pd->o_desk, "e,state,unselected", "e");
             if (pager_config->show_desk_names)
               edje_object_signal_emit(pd->o_desk, "e,name,show", "e");
             else
               edje_object_signal_emit(pd->o_desk, "e,name,hide", "e");
          }
     }
}

static void
_pager_cb_mirror_add(Pager_Desk *pd, Evas_Object *obj EINA_UNUSED, Evas_Object *mirror)
{
   Pager_Win *pw = _pager_window_new(pd, mirror, evas_object_data_get(mirror, "E_Client"));
   if (pw) pd->wins = eina_list_append(pd->wins, pw);
}

static Eina_Bool
_pager_cb_event_zone_desk_count_set(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Zone_Desk_Count_Set *ev)
{
   Eina_List *l;
   Pager *p;

   EINA_LIST_FOREACH(pagers, l, p)
     {
        if ((ev->zone->desk_x_count == p->xnum) &&
            (ev->zone->desk_y_count == p->ynum)) continue;
        _pager_empty(p);
        _pager_fill(p, p->inst ? p->inst->gcc->gadcon : NULL);
        if (p->inst) _gc_orient(p->inst->gcc, p->inst->gcc->gadcon->orient);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_pager_cb_event_desk_show(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Desk_Show *ev = event;
   Eina_List *l;
   Pager *p;
   Pager_Popup *pp;
   Pager_Desk *pd;

   if (!eina_list_count(pagers)) return ECORE_CALLBACK_PASS_ON;

   EINA_LIST_FOREACH(pagers, l, p)
     {
        if (p->zone != ev->desk->zone) continue;
        pd = _pager_desk_find(p, ev->desk);
        if (pd) _pager_desk_select(pd);

        if (p->popup)
          edje_object_part_text_set(p->popup->o_bg, "e.text.label", ev->desk->name);
     }

   if ((pager_config->popup) && (!act_popup) &&
       ((ecore_time_get() - _pager_start_time) > 0.5)) //. not at start
     {
        if ((pp = _pager_popup_find(ev->desk->zone)))
          evas_object_show(pp->popup);
        else
          pp = _pager_popup_new(ev->desk->zone, 0, EINA_TRUE);
        if (pp->timer)
          ecore_timer_loop_reset(pp->timer);
        else
          pp->timer = ecore_timer_add(pager_config->popup_speed,
                                      _pager_popup_cb_timeout, pp);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_pager_cb_event_desk_name_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   E_Event_Desk_Name_Change *ev = event;
   Eina_List *l;
   Pager *p;

   EINA_LIST_FOREACH(pagers, l, p)
     {
        Pager_Desk *pd;

        if (p->zone != ev->desk->zone) continue;
        pd = _pager_desk_find(p, ev->desk);
        if (pager_config->show_desk_names)
          {
             if (pd)
               edje_object_part_text_set(pd->o_desk, "e.text.label",
                                         ev->desk->name);
          }
        else
          {
             if (pd)
               edje_object_part_text_set(pd->o_desk, "e.text.label", "");
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_pager_cb_event_client_urgent_change(void *data EINA_UNUSED, int type EINA_UNUSED, E_Event_Client_Property *ev)
{
   if (!(ev->property & E_CLIENT_PROPERTY_URGENCY)) return ECORE_CALLBACK_RENEW;

   if (!eina_list_count(pagers)) return ECORE_CALLBACK_RENEW;

   if (pager_config->popup_urgent && (!e_client_util_desk_visible(ev->ec, e_desk_current_get(ev->ec->zone))) &&
                                      (pager_config->popup_urgent_focus ||
                                      (!pager_config->popup_urgent_focus && (!ev->ec->focused) && (!ev->ec->want_focus))))
     {
        Pager_Popup *pp;

        pp = _pager_popup_find(ev->ec->zone);

        if ((!pp) && (ev->ec->urgent || ev->ec->icccm.urgent) && (!ev->ec->iconic))
          {
             pp = _pager_popup_new(ev->ec->zone, 0, EINA_TRUE);
             if (!pp) return ECORE_CALLBACK_RENEW;

             if (!pager_config->popup_urgent_stick)
               pp->timer = ecore_timer_loop_add(pager_config->popup_urgent_speed,
                                           _pager_popup_cb_timeout, pp);
             pp->urgent = 1;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_pager_cb_event_compositor_resize(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eina_List *l;
   Pager *p;

   EINA_LIST_FOREACH(pagers, l, p)
     {
        Eina_List *l2;
        Pager_Desk *pd;

        EINA_LIST_FOREACH(p->desks, l2, pd)
          e_layout_virtual_size_set(pd->o_layout, pd->desk->zone->w,
                                    pd->desk->zone->h);

        if (p->inst) _gc_orient(p->inst->gcc, p->inst->gcc->gadcon->orient);
        /* TODO if (p->popup) */
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
_pager_window_cb_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Pager_Win *pw = data;

   pw->desk->wins = eina_list_remove(pw->desk->wins, pw);
   _pager_window_free(data);
}

static void
_pager_window_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Pager_Win *pw = data;

   pw->drag.button = 0;
}

static void
_pager_window_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Pager_Win *pw;

   ev = event_info;
   pw = data;

   if (!pw) return;
   pw->desk->pager->active_drag_client = NULL;
   if (pw->desk->pager->popup && !act_popup) return;
   if (!pw->desk->pager->popup && ev->button == 3) return;
   if (e_client_util_ignored_get(pw->client) || e_client_util_is_popup(pw->client)) return;
   if (ev->button == (int)pager_config->btn_desk) return;
   if ((ev->button == (int)pager_config->btn_drag) ||
       (ev->button == (int)pager_config->btn_noplace))
     {
        Evas_Coord ox, oy;

        evas_object_geometry_get(pw->o_mirror, &ox, &oy, NULL, NULL);
        pw->drag.in_pager = 1;
        pw->drag.x = ev->canvas.x;
        pw->drag.y = ev->canvas.y;
        pw->drag.dx = ox - ev->canvas.x;
        pw->drag.dy = oy - ev->canvas.y;
        pw->drag.start = 1;
        pw->drag.button = ev->button;
        pw->desk->pager->active_drag_client = pw->client;
     }
}

static void
_pager_window_cb_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Pager_Win *pw;
   E_Drag *drag;
   Evas_Object *o;
   Evas_Coord x, y, w, h;
   const char *drag_types[] = {
      "enlightenment/pager_win",
      "enlightenment/border"
   };

   ev = event_info;
   pw = data;

   if (!pw) return;
   if (pw->client->lock_user_location) return;
   if ((pw->desk->pager->popup) && (!act_popup)) return;
   if (!pw->drag.button) return;

   /* prevent drag for a few pixels */
   if (!pw->drag.start) return;

   if (!is_dragged(pw->drag.x - ev->cur.output.x,
                   pw->drag.y - ev->cur.output.y)) return;

   pw->desk->pager->dragging = 1;
   pw->drag.start = 0;
   e_comp_object_effect_clip(pw->client->frame);
   edje_object_signal_emit(pw->desk->o_desk, "e,action,drag,in", "e");
   pw->desk->pager->active_drop_pd = pw->desk;

   evas_object_geometry_get(pw->o_mirror, &x, &y, &w, &h);
   evas_object_hide(pw->o_mirror);

   drag = e_drag_new(x, y, drag_types, 2, pw->desk->pager, -1,
                     _pager_window_cb_drag_convert,
                     _pager_window_cb_drag_finished);
   drag->button_mask = evas_pointer_button_down_mask_get(e_comp->evas);

   evas_object_raise(pw->client->frame);
   /* this is independent of the original mirror */
   o = e_deskmirror_mirror_copy(pw->o_mirror);
   evas_object_show(o);

   e_drag_object_set(drag, o);
   e_drag_resize(drag, w, h);
   e_drag_show(drag);
   e_drag_start(drag, x - pw->drag.dx, y - pw->drag.dy);
}

static void *
_pager_window_cb_drag_convert(E_Drag *drag, const char *type)
{
   Pager *p = drag->data;

   if (!strcmp(type, "enlightenment/pager_win")) return _pager_window_find(p, p->active_drag_client);
   if (!strcmp(type, "enlightenment/border")) return p->active_drag_client;
   return NULL;
}

static void
_pager_window_cb_drag_finished(E_Drag *drag, int dropped)
{
   Pager_Win *pw;
   Pager *p = drag->data;

   if (!p) return;
   pw = _pager_window_find(p, p->active_drag_client);
   if (!pw) return;
   p->active_drag_client = NULL;
   evas_object_show(pw->o_mirror);
   if (dropped)
     {
        /* be helpful */
        if (pw->client->desk->visible && (!e_client_focused_get()))
          evas_object_focus_set(pw->client->frame, 1);
        e_deskmirror_update_force(pw->desk->o_layout);
     }
   else
     {
        int dx, dy, x, y, zx, zy, zw, zh;
        E_Client *ec = pw->client;

        ec->hidden = !p->active_pd->desk->visible;
        e_client_desk_set(ec, p->active_pd->desk);

        dx = (ec->w / 2);
        dy = (ec->h / 2);

        evas_pointer_canvas_xy_get(evas_object_evas_get(p->o_table), &x, &y);
        e_zone_useful_geometry_get(p->zone, &zx, &zy, &zw, &zh);

        /* offset so that center of window is on mouse, but keep within desk bounds */
        if (dx < x)
          {
             x -= dx;
             if ((ec->w < zw) && (x + ec->w > zx + zw))
               x -= x + ec->w - (zx + zw);
          }
        else x = 0;

        if (dy < y)
          {
             y -= dy;
             if ((ec->h < zh) && (y + ec->h > zy + zh))
               y -= y + ec->h - (zy + zh);
          }
        else y = 0;
        evas_object_move(ec->frame, x, y);

        if (!(ec->lock_user_stacking)) evas_object_raise(ec->frame);
        evas_object_focus_set(ec->frame, 1);
        e_deskmirror_update_force(pw->desk->o_layout);
     }
   if (p->active_drop_pd)
     {
        edje_object_signal_emit(p->active_drop_pd->o_desk, "e,action,drag,out", "e");
        if (!pw->drag.start) p->active_drop_pd->pager->just_dragged = 1;
        p->active_drop_pd = NULL;
     }
   edje_object_signal_emit(pw->desk->o_desk, "e,action,drag,out", "e");
   if (!pw->drag.from_pager)
     {
        if (!pw->drag.start) p->just_dragged = 1;
        pw->drag.in_pager = 0;
        pw->drag.button = pw->drag.start = 0;
        p->dragging = 0;
     }
   if (pw->drag.from_pager) pw->drag.from_pager->dragging = 0;
   pw->drag.from_pager = NULL;
   e_comp_object_effect_unclip(pw->client->frame);
   if (act_popup)
     {
        if (e_comp->comp_type == E_PIXMAP_TYPE_X)
          e_grabinput_get(0, 0, input_window);
        else
          e_comp_grab_input(0, 1);
        if (p->inst) _pager_popup_hide(1);
     }
}

static void
_pager_inst_cb_scroll(void *data)
{
   Pager *p = data;

   _pager_update_drop_position(p, p->dnd_x, p->dnd_y);
}

static void
_pager_update_drop_position(Pager *p, Evas_Coord x, Evas_Coord y)
{
   Pager_Desk *pd;
   Pager_Win *pw = NULL;
   Eina_Bool changed;

   p->dnd_x = x;
   p->dnd_y = y;
   pd = _pager_desk_at_coord(p, x, y);
   changed = (pd != p->active_drop_pd);
   if (changed)
     {
        if (pd)
          edje_object_signal_emit(pd->o_desk, "e,action,drag,in", "e");
        if (p->active_drop_pd)
          edje_object_signal_emit(p->active_drop_pd->o_desk, "e,action,drag,out", "e");
        p->active_drop_pd = pd;
     }
   if (pd) pw = _pager_desk_window_find(pd, p->active_drag_client);
   if (!pw) pw = _pager_window_find(p, p->active_drag_client);

   if (!pw) return;
   if (pd)
     {
        int zx, zy, zw, zh, vx, vy, offx, offy;
        E_Client *ec = pw->client;
        E_Desk *old_desk = ec->desk;
        Eina_Bool was_focused = e_client_stack_focused_get(ec);
        E_Drag *drag = e_drag_current_get();

        pw->drag.in_pager = 1;
        //makes drags look weird
        //e_zone_useful_geometry_get(pd->desk->zone, &zx, &zy, &zw, &zh);
        zx = pd->desk->zone->x, zy = pd->desk->zone->y;
        zw = pd->desk->zone->w, zh = pd->desk->zone->h;
        e_deskmirror_coord_canvas_to_virtual(pd->o_layout,
                                             x, y, &vx, &vy);
        ec->hidden = !pd->desk->visible;
        e_client_desk_set(ec, pd->desk);
        offx = (ec->w / 2);
        offy = (ec->h / 2);
        if (drag)
          {
             if (drag->w > 0) offx = ((drag->dx) * ec->w) / drag->w;
             if (drag->h > 0) offy = ((drag->dy) * ec->h) / drag->h;
          }
        x = E_CLAMP(vx + zx - offx, zx, zx + zw - ec->w);
        y = E_CLAMP(vy + zy - offy, zy, zy + zh - ec->h);
        evas_object_move(ec->frame, x, y);
        if (was_focused)
          e_desk_last_focused_focus(old_desk);
     }
   else
     {
        /* this prevents the desk from switching on drags */
        pw->drag.from_pager = pw->desk->pager;
        pw->drag.from_pager->dragging = 1;
        pw->drag.in_pager = 0;
     }
}

static void
_pager_drop_cb_enter(void *data, const char *type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Pager *p = data;

   /* FIXME this fixes a segv, but the case is not easily
    * reproduceable. this makes no sense either since
    * the same 'pager' is passed to e_drop_handler_add
    * and it works without this almost all the time.
    * so this must be an issue with e_dnd code... i guess */
   if (act_popup) p = act_popup->pager;

   if (p->inst)
     e_gadcon_client_autoscroll_cb_set(p->inst->gcc, _pager_inst_cb_scroll, p);
}

static void
_pager_drop_cb_move(void *data, const char *type EINA_UNUSED, void *event_info)
{
   E_Event_Dnd_Move *ev = event_info;
   Pager *p = data;

   if (act_popup) p = act_popup->pager;

   _pager_update_drop_position(p, ev->x, ev->y);

   if (p->inst)
     e_gadcon_client_autoscroll_update(p->inst->gcc, ev->x, ev->y);
}

static void
_pager_drop_cb_leave(void *data, const char *type EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Pager *p = data;

   if (act_popup) p = act_popup->pager;

   if (p->active_drop_pd)
     edje_object_signal_emit(p->active_drop_pd->o_desk, "e,action,drag,out", "e");
   p->active_drop_pd = NULL;

   if (p->inst) e_gadcon_client_autoscroll_cb_set(p->inst->gcc, NULL, NULL);
}

static void
_pager_drop_cb_drop(void *data, const char *type, void *event_info)
{
   E_Event_Dnd_Drop *ev = event_info;
   Pager_Desk *pd;
   Pager_Desk *pd2 = NULL;
   E_Client *ec = NULL;
   Eina_List *l;
   Pager_Win *pw = NULL;
   Evas_Coord wx, wy, wx2, wy2;
   Evas_Coord nx, ny;
   Pager *p = data;

   if (act_popup) p = act_popup->pager;

   pd = _pager_desk_at_coord(p, ev->x, ev->y);
   if (pd)
     {
        if (!strcmp(type, "enlightenment/pager_win"))
          {
             pw = (Pager_Win *)(ev->data);
             if (pw) ec = pw->client;
          }
        else if (!strcmp(type, "enlightenment/border"))
          {
             ec = ev->data;
             e_deskmirror_coord_virtual_to_canvas(pd->o_layout, ec->x, ec->y,
                                                  &wx, &wy);
             e_deskmirror_coord_virtual_to_canvas(pd->o_layout, ec->x + ec->w,
                                                  ec->y + ec->h, &wx2, &wy2);
          }
        else if (!strcmp(type, "enlightenment/vdesktop"))
          {
             pd2 = ev->data;
             if (!pd2) return;
             _pager_desk_switch(pd, pd2);
          }
        else return;

        if (ec)
          {
             E_Maximize max = ec->maximized;
             E_Fullscreen fs = ec->fullscreen_policy;
             Eina_Bool fullscreen = ec->fullscreen;
             E_Desk *old_desk = ec->desk;
             Eina_Bool was_focused = e_client_stack_focused_get(ec);

             if (ec->iconic) e_client_uniconify(ec);
             if (ec->maximized)
               e_client_unmaximize(ec, E_MAXIMIZE_BOTH);
             if (fullscreen) e_client_unfullscreen(ec);
               ec->hidden = 0;
             e_client_desk_set(ec, pd->desk);
             if (was_focused)
               e_desk_last_focused_focus(old_desk);
             evas_object_raise(ec->frame);

             if ((!max) && (!fullscreen))
               {
                  E_Drag *drag = e_drag_current_get();
                  int zx, zy, zw, zh, mx, my, offx, offy;

                  e_deskmirror_coord_canvas_to_virtual(pd->o_layout,
                                                       ev->x, ev->y,
                                                       &nx, &ny);
                  e_zone_useful_geometry_get(pd->desk->zone,
                                             &zx, &zy, &zw, &zh);
                  offx = (ec->w / 2);
                  offy = (ec->h / 2);
                  if (drag)
                    {
                       if (drag->w > 0) offx = ((drag->dx) * ec->w) / drag->w;
                       if (drag->h > 0) offy = ((drag->dy) * ec->h) / drag->h;
                    }
                  mx = E_CLAMP(nx + zx - offx, zx, zx + zw - ec->w);
                  my = E_CLAMP(ny + zy - offy, zy, zy + zh - ec->h);
                  evas_object_move(ec->frame, mx, my);
               }
             if (max) e_client_maximize(ec, max);
             if (fullscreen) e_client_fullscreen(ec, fs);
             e_deskmirror_update_force(pd->o_layout);
             if (!p->inst)
               {
                  Instance *inst;
                  Eina_List *ll;

                  EINA_LIST_FOREACH(instances, l, inst)
                    {
                       EINA_LIST_FOREACH(inst->pager->desks, ll, pd2)
                         {
                            if (pd2->desk == pd->desk)
                              {
                                 printf("update pager desk %p\n", pd2);
                                 e_deskmirror_update_force(pd2->o_layout);
                              }
                         }
                    }
               }
          }
     }

   EINA_LIST_FOREACH(p->desks, l, pd)
     {
        if (!p->active_drop_pd) break;
        if (pd == p->active_drop_pd)
          {
             edje_object_signal_emit(pd->o_desk, "e,action,drag,out", "e");
             p->active_drop_pd = NULL;
          }
     }

   if (p->inst) e_gadcon_client_autoscroll_cb_set(p->inst->gcc, NULL, NULL);
}

static void
_pager_desk_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Pager_Desk *pd = data;
   Evas_Coord ox, oy;

   if (!pd) return;
   if ((!pd->pager->popup) && (ev->button == 3)) return;
   if (ev->button == (int)pager_config->btn_desk)
     {
        evas_object_geometry_get(pd->o_desk, &ox, &oy, NULL, NULL);
        pd->drag.start = 1;
        pd->drag.in_pager = 1;
        pd->drag.dx = ox - ev->canvas.x;
        pd->drag.dy = oy - ev->canvas.y;
        pd->drag.x = ev->canvas.x;
        pd->drag.y = ev->canvas.y;
        pd->drag.button = ev->button;
     }
   else
     {
        pd->drag.dx = pd->drag.dy = pd->drag.x = pd->drag.y = 0;
     }
   pd->pager->just_dragged = 0;
}

static void
_pager_desk_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Pager_Desk *pd = data;
   Pager *p;

   if (!pd) return;
   p = pd->pager;

   /* FIXME: pd->pager->dragging is 0 when finishing a drag from desk to desk */
   if ((ev->button == 1) && (!pd->pager->dragging) &&
       (!pd->pager->just_dragged))
     {
        current_desk = pd->desk;
        e_desk_show(pd->desk);
        pd->drag.start = 0;
        pd->drag.in_pager = 0;
        p->active_drop_pd = NULL;
     }
   else if (ev->button == (int)pager_config->btn_desk)
     {
        if (pd->pager->dragging) pd->pager->dragging = 0;
        pd->drag.start = 0;
        pd->drag.in_pager = 0;
     }

   if ((p->popup) && (p->popup->urgent)) _pager_popup_free(p->popup);
}

static void
_pager_desk_cb_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Pager_Desk *pd = data;
   E_Drag *drag;
   Evas_Object *o;
   Evas_Coord x, y, w, h;
   const char *drag_types[] = {
      "enlightenment/vdesktop"
   };

   if (!pd) return;
   /* prevent drag for a few pixels */
   if (pd->drag.start)
     {
        if (!is_dragged(pd->drag.x - ev->cur.output.x,
                        pd->drag.y - ev->cur.output.y)) return;

        if (pd->pager) pd->pager->dragging = 1;
        pd->drag.start = 0;
     }

   if (pd->drag.in_pager && pd->pager)
     {
        evas_object_geometry_get(pd->o_desk, &x, &y, &w, &h);
        drag = e_drag_new(x, y, drag_types, 1, pd, -1,
                          NULL, _pager_desk_cb_drag_finished);
        drag->button_mask = evas_pointer_button_down_mask_get(e_comp->evas);

        /* redraw the desktop theme above */
        o = e_comp_object_util_mirror_add(pd->o_layout);
        e_drag_object_set(drag, o);

        e_drag_resize(drag, w, h);
        e_drag_start(drag, x - pd->drag.dx, y - pd->drag.dy);

        pd->drag.from_pager = pd->pager;
        pd->drag.from_pager->dragging = 1;
        pd->drag.in_pager = 0;
     }
}

static void
_pager_desk_cb_drag_finished(E_Drag *drag, int dropped)
{
   Pager_Desk *pd = drag->data;
   Pager_Desk *pd2 = NULL;
   Eina_List *l;
   E_Desk *desk;
   E_Zone *zone;
   Pager *p;

   if (!pd) return;
   if (!dropped)
     {
        /* wasn't dropped on pager, switch with current desktop */
        if (!pd->desk) return;
        zone = e_zone_current_get();
        desk = e_desk_current_get(zone);
        EINA_LIST_FOREACH(pagers, l, p)
          {
             pd2 = _pager_desk_find(p, desk);
             if (pd2) break;
          }
        _pager_desk_switch(pd, pd2);
     }
   if (pd->drag.from_pager)
     {
        pd->drag.from_pager->dragging = 0;
        pd->drag.from_pager->just_dragged = 0;
     }
   if (pd->pager->active_drop_pd)
     {
        edje_object_signal_emit(pd->pager->active_drop_pd->o_desk, "e,action,drag,out", "e");
        pd->pager->active_drop_pd = NULL;
     }
   pd->drag.from_pager = NULL;

   if (act_popup)
     {
        if (e_comp->comp_type == E_PIXMAP_TYPE_X)
          e_grabinput_get(0, 0, input_window);
        else
          e_comp_grab_input(0, 1);
        if (pd->pager->inst) _pager_popup_hide(1);
     }
}

static void
_pager_desk_cb_mouse_wheel(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev = event_info;
   Pager_Desk *pd = data;

   if (pd->pager->popup) return;

   if (pager_config->flip_desk)
     e_zone_desk_linear_flip_by(pd->desk->zone, ev->z);
}

static Eina_Bool
_pager_popup_cb_timeout(void *data)
{
   Pager_Popup *pp =  data;

   pp->timer = NULL;
   _pager_popup_free(pp);

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        if (input_window)
          {
             e_grabinput_release(input_window, input_window);
             ecore_x_window_free(input_window);
             input_window = 0;
          }
     }
#endif
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     {
        e_comp_ungrab_input(1, 1);
        input_window = 0;
     }

   return ECORE_CALLBACK_CANCEL;
}

/************************************************************************/
/* popup-on-keyaction functions */
static int
_pager_popup_show(void)
{
   E_Zone *zone;
   int x, y, w, h;
   Pager_Popup *pp;

   if ((act_popup) || (input_window)) return 0;

   zone = e_zone_current_get();
   pp = _pager_popup_find(zone);
   if (pp) _pager_popup_free(pp);

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     {
        input_window = ecore_x_window_input_new(e_comp->win, 0, 0, 1, 1);
        ecore_x_window_show(input_window);
        if (!e_grabinput_get(0, 0, input_window))
          {
             ecore_x_window_free(input_window);
             input_window = 0;
             return 0;
          }
     }
#endif
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     {
        input_window = e_comp->ee_win;
        e_comp_grab_input(0, 1);
     }

   handlers = eina_list_append
       (handlers, ecore_event_handler_add
         (ECORE_EVENT_MOUSE_WHEEL, _pager_popup_cb_mouse_wheel, NULL));

   act_popup = _pager_popup_new(zone, 1, EINA_FALSE);
   evas_object_geometry_get(act_popup->pager->o_table, &x, &y, &w, &h);
   current_desk = e_desk_current_get(zone);
   return 1;
}

static void
_pager_popup_hide(int switch_desk)
{
   while (handlers)
     {
        ecore_event_handler_del(handlers->data);
        handlers = eina_list_remove_list(handlers, handlers);
     }

   act_popup->timer = ecore_timer_loop_add(0.1, _pager_popup_cb_timeout, act_popup);

   if ((switch_desk) && (current_desk)) e_desk_show(current_desk);

   act_popup = NULL;
}

static void
_pager_popup_desk_switch(int x, int y)
{
   int max_x, max_y, desk_x, desk_y;
   Pager_Desk *pd;
   Pager_Popup *pp = act_popup;

   e_zone_desk_count_get(pp->pager->zone, &max_x, &max_y);

   desk_x = current_desk->x + x;
   desk_y = current_desk->y + y;

   if (desk_x < 0)           desk_x = max_x - 1;
   else if (desk_x >= max_x) desk_x = 0;

   if (desk_y < 0)           desk_y = max_y - 1;
   else if (desk_y >= max_y) desk_y = 0;

   current_desk = e_desk_at_xy_get(pp->pager->zone, desk_x, desk_y);

   pd = _pager_desk_find(pp->pager, current_desk);
   if (pd) _pager_desk_select(pd);

   edje_object_part_text_set(pp->o_bg, "e.text.label", current_desk->name);
}

static void
_pager_popup_cb_action_show(E_Object *obj EINA_UNUSED, const char *params EINA_UNUSED, Ecore_Event_Key *ev EINA_UNUSED)
{
   _pager_popup_show();
}

static void
_pager_popup_cb_action_switch(E_Object *obj EINA_UNUSED, const char *params, Ecore_Event_Key *ev EINA_UNUSED)
{
   int max_x, max_y, desk_x;
   int x = 0, y = 0;

   if (!act_popup) _pager_popup_show();

   e_zone_desk_count_get(act_popup->pager->zone, &max_x, &max_y);
   desk_x = current_desk->x /* + x <=this is always 0 */;

   if      (!strcmp(params, "left"))  x = -1;
   else if (!strcmp(params, "right")) x = 1;
   else if (!strcmp(params, "up"))    y = -1;
   else if (!strcmp(params, "down"))  y = 1;
   else if (!strcmp(params, "next"))
     {
        x = 1;
        if (desk_x == max_x - 1) y = 1;
     }
   else if (!strcmp(params, "prev"))
     {
        x = -1;
        if (desk_x == 0) y = -1;
     }
   _pager_popup_desk_switch(x, y);
}

static Eina_Bool
_pager_popup_cb_mouse_wheel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Wheel *ev = event;
   Pager_Popup *pp = act_popup;
   int max_x;

   e_zone_desk_count_get(pp->pager->zone, &max_x, NULL);

   if      ((current_desk->x + ev->z) >= max_x) _pager_popup_desk_switch(1, 1);
   else if ((current_desk->x + ev->z) < 0)      _pager_popup_desk_switch(-1, -1);
   else                                         _pager_popup_desk_switch(ev->z, 0);

   return ECORE_CALLBACK_PASS_ON;
}

/***************************************************************************/
/* module setup */
E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION, "Pager"
};

E_API void *
e_modapi_init(E_Module *m EINA_UNUSED)
{
   E_Module *p;

   _pager_start_time = ecore_time_get();
   conf_edd = E_CONFIG_DD_NEW("Pager_Config", Config);
#undef T
#undef D
#define T Config
#define D conf_edd
   E_CONFIG_VAL(D, T, popup, UINT);
   E_CONFIG_VAL(D, T, popup_speed, DOUBLE);
   E_CONFIG_VAL(D, T, popup_urgent, UINT);
   E_CONFIG_VAL(D, T, popup_urgent_stick, UINT);
   E_CONFIG_VAL(D, T, popup_urgent_speed, DOUBLE);
   E_CONFIG_VAL(D, T, show_desk_names, UINT);
   E_CONFIG_VAL(D, T, popup_height, INT);
   E_CONFIG_VAL(D, T, popup_act_height, INT);
   E_CONFIG_VAL(D, T, drag_resist, UINT);
   E_CONFIG_VAL(D, T, btn_drag, UCHAR);
   E_CONFIG_VAL(D, T, btn_noplace, UCHAR);
   E_CONFIG_VAL(D, T, btn_desk, UCHAR);
   E_CONFIG_VAL(D, T, flip_desk, UCHAR);
   E_CONFIG_VAL(D, T, plain, UCHAR);
   E_CONFIG_VAL(D, T, permanent_plain, UCHAR);

   pager_config = e_config_domain_load("module.pager", conf_edd);

   if (!pager_config)
     {
        pager_config = E_NEW(Config, 1);
        pager_config->popup = 1;
        pager_config->popup_speed = 1.0;
        pager_config->popup_urgent = 0;
        pager_config->popup_urgent_stick = 0;
        pager_config->popup_urgent_speed = 1.5;
        pager_config->show_desk_names = 0;
        pager_config->popup_height = 60;
        pager_config->popup_act_height = 60;
        pager_config->drag_resist = 3;
        pager_config->btn_drag = 1;
        pager_config->btn_noplace = 2;
        pager_config->btn_desk = 2;
        pager_config->flip_desk = 0;
        pager_config->plain = 0;
        pager_config->permanent_plain = 0;
     }
   E_CONFIG_LIMIT(pager_config->popup, 0, 1);
   E_CONFIG_LIMIT(pager_config->popup_speed, 0.1, 10.0);
   E_CONFIG_LIMIT(pager_config->popup_urgent, 0, 1);
   E_CONFIG_LIMIT(pager_config->popup_urgent_stick, 0, 1);
   E_CONFIG_LIMIT(pager_config->popup_urgent_speed, 0.1, 10.0);
   E_CONFIG_LIMIT(pager_config->show_desk_names, 0, 1);
   E_CONFIG_LIMIT(pager_config->popup_height, 20, 200);
   E_CONFIG_LIMIT(pager_config->popup_act_height, 20, 200);
   E_CONFIG_LIMIT(pager_config->drag_resist, 0, 50);
   E_CONFIG_LIMIT(pager_config->flip_desk, 0, 1);
   E_CONFIG_LIMIT(pager_config->btn_drag, 0, 32);
   E_CONFIG_LIMIT(pager_config->btn_noplace, 0, 32);
   E_CONFIG_LIMIT(pager_config->btn_desk, 0, 32);
   E_CONFIG_LIMIT(pager_config->plain, 0, 1);
   E_CONFIG_LIMIT(pager_config->permanent_plain, 0, 1);

   p = e_module_find("pager_plain");
   if (p && p->enabled)
     {
        e_util_dialog_show(_("Error"), _("Pager module cannot be loaded at the same time as Pager Plain!"));
        return NULL;
     }

   E_LIST_HANDLER_APPEND(shandlers, E_EVENT_ZONE_DESK_COUNT_SET, _pager_cb_event_zone_desk_count_set, NULL);
   E_LIST_HANDLER_APPEND(shandlers, E_EVENT_DESK_SHOW, _pager_cb_event_desk_show, NULL);
   E_LIST_HANDLER_APPEND(shandlers, E_EVENT_DESK_NAME_CHANGE, _pager_cb_event_desk_name_change, NULL);
   E_LIST_HANDLER_APPEND(shandlers, E_EVENT_COMPOSITOR_UPDATE, _pager_cb_event_compositor_resize, NULL);
   E_LIST_HANDLER_APPEND(shandlers, E_EVENT_CLIENT_PROPERTY, _pager_cb_event_client_urgent_change, NULL);

   module = m;

   e_gadcon_provider_register(&_gadcon_class);

   e_configure_registry_item_add("extensions/pager", 40, _("Pager"), NULL,
                                 "preferences-pager", _pager_config_dialog);

   act_popup_show = e_action_add("pager_show");
   if (act_popup_show)
     {
        act_popup_show->func.go_key = _pager_popup_cb_action_show;
        e_action_predef_name_set(N_("Pager"), N_("Show Pager Popup"),
                                 "pager_show", "<none>", NULL, 0);
     }
   act_popup_switch = e_action_add("pager_switch");
   if (act_popup_switch)
     {
        act_popup_switch->func.go_key = _pager_popup_cb_action_switch;
        e_action_predef_name_set(N_("Pager"), N_("Popup Desk Right"),
                                 "pager_switch", "right", NULL, 0);
        e_action_predef_name_set(N_("Pager"), N_("Popup Desk Left"),
                                 "pager_switch", "left", NULL, 0);
        e_action_predef_name_set(N_("Pager"), N_("Popup Desk Up"),
                                 "pager_switch", "up", NULL, 0);
        e_action_predef_name_set(N_("Pager"), N_("Popup Desk Down"),
                                 "pager_switch", "down", NULL, 0);
        e_action_predef_name_set(N_("Pager"), N_("Popup Desk Next"),
                                 "pager_switch", "next", NULL, 0);
        e_action_predef_name_set(N_("Pager"), N_("Popup Desk Previous"),
                                 "pager_switch", "prev", NULL, 0);
     }

   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   e_gadcon_provider_unregister(&_gadcon_class);

   if (config_dialog) e_object_del(E_OBJECT(config_dialog));
   E_FREE_LIST(shandlers, ecore_event_handler_del);

   e_action_del("pager_show");
   e_action_del("pager_switch");

   e_action_predef_name_del("Pager", "Popup Desk Right");
   e_action_predef_name_del("Pager", "Popup Desk Left");
   e_action_predef_name_del("Pager", "Popup Desk Up");
   e_action_predef_name_del("Pager", "Popup Desk Down");
   e_action_predef_name_del("Pager", "Popup Desk Next");
   e_action_predef_name_del("Pager", "Popup Desk Previous");

   module = NULL;

   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.pager", conf_edd, pager_config);
   return 1;
}

