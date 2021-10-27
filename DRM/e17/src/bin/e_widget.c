#include "e.h"

#define SMART_NAME     "e_widget"
#define API_ENTRY      E_Smart_Data * sd; sd = evas_object_smart_data_get(obj); if ((!obj) || (!sd) || (evas_object_type_get(obj) && strcmp(evas_object_type_get(obj), SMART_NAME)))
#define INTERNAL_ENTRY E_Smart_Data * sd; sd = evas_object_smart_data_get(obj); if (!sd) return;
typedef struct _E_Smart_Data E_Smart_Data;

struct _E_Smart_Data
{
   Evas_Object  *parent_obj;
   Evas_Coord    x, y, w, h, minw, minh;
   Eina_List    *subobjs;
   Evas_Object  *resize_obj;
   void          (*del_func)(Evas_Object *obj);
   void          (*focus_func)(Evas_Object *obj);
   void          (*activate_func)(Evas_Object *obj);
   void          (*disable_func)(Evas_Object *obj);
   void          (*on_focus_func)(void *data, Evas_Object *obj);
   void         *on_focus_data;
   void          (*on_change_func)(void *data, Evas_Object *obj);
   void         *on_change_data;
   void          (*on_disable_func)(void *data, Evas_Object *obj);
   void         *on_disable_data;
   void         *data;
   unsigned char can_focus E_BITFIELD;
   unsigned char child_can_focus E_BITFIELD;
   unsigned char focused E_BITFIELD;
   unsigned char disabled E_BITFIELD;
};

/* local subsystem functions */
static void _e_smart_reconfigure(E_Smart_Data *sd);
static void _e_smart_add(Evas_Object *obj);
static void _e_smart_del(Evas_Object *obj);
static void _e_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _e_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _e_smart_show(Evas_Object *obj);
static void _e_smart_hide(Evas_Object *obj);
static void _e_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _e_smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void _e_smart_clip_unset(Evas_Object *obj);
static void _e_smart_init(void);

static void
_e_widget_hint(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w, h;

   evas_object_size_hint_min_get(obj, &w, &h);
   e_widget_size_min_set(data, w, h);
}

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
E_API Evas_Object *
e_widget_add(Evas *evas)
{
   _e_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

E_API void
e_widget_del_hook_set(Evas_Object *obj, void (*func)(Evas_Object *obj))
{
   API_ENTRY return;
   sd->del_func = func;
}

E_API void
e_widget_focus_hook_set(Evas_Object *obj, void (*func)(Evas_Object *obj))
{
   API_ENTRY return;
   sd->focus_func = func;
}

E_API void
e_widget_activate_hook_set(Evas_Object *obj, void (*func)(Evas_Object *obj))
{
   API_ENTRY return;
   sd->activate_func = func;
}

E_API void
e_widget_disable_hook_set(Evas_Object *obj, void (*func)(Evas_Object *obj))
{
   API_ENTRY return;
   sd->disable_func = func;
}

E_API void
e_widget_on_focus_hook_set(Evas_Object *obj, void (*func)(void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_focus_func = func;
   sd->on_focus_data = data;
}

E_API void
e_widget_on_change_hook_set(Evas_Object *obj, void (*func)(void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_change_func = func;
   sd->on_change_data = data;
}

E_API void
e_widget_on_disable_hook_set(Evas_Object *obj, void (*func)(void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_disable_func = func;
   sd->on_disable_data = data;
}

E_API void
e_widget_data_set(Evas_Object *obj, void *data)
{
   API_ENTRY return;
   sd->data = data;
}

E_API void *
e_widget_data_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->data;
}

E_API void
e_widget_size_min_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   evas_object_size_hint_min_set(obj, minw, minh);
}

E_API void
e_widget_size_min_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   evas_object_size_hint_min_get(obj, minw, minh);
}

static void
_sub_obj_del(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj,
             void *event_info EINA_UNUSED)
{
   E_Smart_Data *sd = data;

   sd->subobjs = eina_list_remove(sd->subobjs, obj);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _sub_obj_del);
}

E_API void
e_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;

   if (eina_list_data_find(sd->subobjs, sobj)) return;

   sd->subobjs = eina_list_append(sd->subobjs, sobj);
   evas_object_event_callback_add(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
   if (!sd->child_can_focus)
     {
        if (e_widget_can_focus_get(sobj)) sd->child_can_focus = 1;
     }
   if (!evas_object_smart_parent_get(sobj))
     evas_object_smart_member_add(sobj, obj);
   if (strcmp(evas_object_type_get(sobj), SMART_NAME)) return;

   sd = evas_object_smart_data_get(sobj);
   if (sd)
     {
        if (sd->parent_obj) e_widget_sub_object_del(sd->parent_obj, sobj);
        sd->parent_obj = obj;
     }
   evas_object_event_callback_del_full(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
}

E_API void
e_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   if (evas_object_smart_parent_get(sobj) == obj)
     evas_object_smart_member_del(sobj);
   evas_object_event_callback_del(sobj, EVAS_CALLBACK_DEL, _sub_obj_del);
   sd->subobjs = eina_list_remove(sd->subobjs, sobj);
   if (!sd->child_can_focus)
     {
        if (e_widget_can_focus_get(sobj)) sd->child_can_focus = 0;
     }
}

E_API void
e_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj)
{
   int w, h;
   API_ENTRY return;
   if (sd->resize_obj) evas_object_smart_member_del(sd->resize_obj);
   sd->resize_obj = sobj;
   evas_object_event_callback_add(sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _e_widget_hint, obj);
   evas_object_smart_member_add(sobj, obj);
   _e_smart_reconfigure(sd);
   evas_object_size_hint_min_get(obj, &w, &h);
   if ((w > 0) || (h > 0)) return; //already set
   evas_object_size_hint_min_get(sobj, &w, &h);
   evas_object_size_hint_min_set(obj, w, h);
}

E_API void
e_widget_can_focus_set(Evas_Object *obj, int can_focus)
{
   API_ENTRY return;
   sd->can_focus = can_focus;
}

E_API int
e_widget_can_focus_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   if (sd->disabled) return 0;
   if (sd->can_focus) return 1;
   if (sd->child_can_focus) return 1;
   return 0;
}

E_API int
e_widget_focus_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->focused;
}

E_API Evas_Object *
e_widget_focused_object_get(Evas_Object *obj)
{
   Eina_List *l = NULL;
   Evas_Object *sobj = NULL;

   API_ENTRY return NULL;
   if (!sd->focused) return NULL;
   EINA_LIST_FOREACH(sd->subobjs, l, sobj)
     {
        sobj = e_widget_focused_object_get(sobj);
        if (sobj) return sobj;
     }
   return obj;
}

E_API int
e_widget_focus_jump(Evas_Object *obj, int forward)
{
   API_ENTRY return 0;
   if (!e_widget_can_focus_get(obj)) return 0;

   /* if it has a focus func its an end-point widget like a button */
   if (sd->focus_func)
     {
        if (!sd->focused) sd->focused = 1;
        else sd->focused = 0;
        sd->focus_func(obj);
        if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
        return sd->focused;
     }
   /* its some container */
   else
     {
        Eina_List *l = NULL;
        Evas_Object *sobj = NULL;
        int focus_next = 0;

        if ((!sd->disabled) && (!sd->focused))
          {
             e_widget_focus_set(obj, forward);
             sd->focused = 1;
             if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
             return 1;
          }
        else
          {
             if (forward)
               {
                  EINA_LIST_FOREACH(sd->subobjs, l, sobj)
                    {
                       if (!e_widget_can_focus_get(sobj)) continue;
                       if (focus_next)
                         {
                            /* the previous focused item was unfocused - so focus
                             * the next one (that can be focused) */
                            if (e_widget_focus_jump(sobj, forward))
                              return 1;
                            break;
                         }
                       else
                         {
                            if (e_widget_focus_get(sobj))
                              {
                                 /* jump to the next focused item or focus this item */
                                 if (e_widget_focus_jump(sobj, forward))
                                   return 1;
                                 /* it returned 0 - it got to the last item and is past it */
                                 focus_next = 1;
                              }
                         }
                    }
               }
             else
               {
                  EINA_LIST_REVERSE_FOREACH(sd->subobjs, l, sobj)
                    {
                       if (e_widget_can_focus_get(sobj))
                         {
                            if ((focus_next) &&
                                (!e_widget_disabled_get(sobj)))
                              {
                                 /* the previous focused item was unfocused - so focus
                                  * the next one (that can be focused) */
                                 if (e_widget_focus_jump(sobj, forward))
                                   return 1;
                                 else break;
                              }
                            else
                              {
                                 if (e_widget_focus_get(sobj))
                                   {
                                      /* jump to the next focused item or focus this item */
                                      if (e_widget_focus_jump(sobj, forward))
                                        return 1;
                                      /* it returned 0 - it got to the last item and is past it */
                                      focus_next = 1;
                                   }
                              }
                         }
                    }
               }
          }
     }
   /* no next item can be focused */
   sd->focused = 0;
   return 0;
}

E_API void
e_widget_focus_set(Evas_Object *obj, int first)
{
   API_ENTRY return;
   if (!sd->focused)
     {
        sd->focused = 1;
        if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
     }
   if (sd->focus_func)
     {
        sd->focus_func(obj);
        return;
     }
   else
     {
        Eina_List *l = NULL;
        Evas_Object *sobj;

        if (first)
          {
             EINA_LIST_FOREACH(sd->subobjs, l, sobj)
               {
                  if ((e_widget_can_focus_get(sobj)) &&
                      (!e_widget_disabled_get(sobj)))
                    {
                       e_widget_focus_set(sobj, first);
                       break;
                    }
               }
          }
        else
          {
             EINA_LIST_REVERSE_FOREACH(sd->subobjs, l, sobj)
               {
                  if ((e_widget_can_focus_get(sobj)) &&
                      (!e_widget_disabled_get(sobj)))
                    {
                       e_widget_focus_set(sobj, first);
                       break;
                    }
               }
          }
     }
}

E_API Evas_Object *
e_widget_parent_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->parent_obj;
}

E_API void
e_widget_focused_object_clear(Evas_Object *obj)
{
   Eina_List *l = NULL;
   Evas_Object *sobj = NULL;

   API_ENTRY return;
   if (!sd->focused) return;
   sd->focused = 0;
   EINA_LIST_FOREACH(sd->subobjs, l, sobj)
     {
        if (e_widget_focus_get(sobj))
          {
             e_widget_focused_object_clear(sobj);
             break;
          }
     }
   if (sd->focus_func) sd->focus_func(obj);
}

E_API void
e_widget_focus_steal(Evas_Object *obj)
{
   Evas_Object *parent = NULL, *o = NULL;

   API_ENTRY return;
   if ((sd->focused) || (sd->disabled) || (!sd->can_focus)) return;
   parent = obj;
   for (;; )
     {
        o = e_widget_parent_get(parent);
        if (!o) break;
        parent = o;
     }
   e_widget_focused_object_clear(parent);
   parent = obj;
   for (;; )
     {
        sd = evas_object_smart_data_get(parent);
        sd->focused = 1;
        if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, parent);
        o = e_widget_parent_get(parent);
        if (!o) break;
        parent = o;
     }
   sd = evas_object_smart_data_get(obj);
   if (sd->focus_func) sd->focus_func(obj);
}

E_API void
e_widget_activate(Evas_Object *obj)
{
   API_ENTRY return;
   e_widget_change(obj);
   if (sd->activate_func) sd->activate_func(obj);
}

E_API void
e_widget_change(Evas_Object *obj)
{
   API_ENTRY return;
   if (sd->parent_obj) e_widget_change(sd->parent_obj);
   if (sd->on_change_func) sd->on_change_func(sd->on_change_data, obj);
}

E_API void
e_widget_disabled_set(Evas_Object *obj, int disabled)
{
   API_ENTRY return;
   if (sd->disabled == !!disabled) return;
   sd->disabled = !!disabled;
   if (sd->focused && sd->disabled)
     {
        Evas_Object *o = NULL, *parent = NULL;

        parent = obj;
        for (;; )
          {
             o = e_widget_parent_get(parent);
             if (!o) break;
             parent = o;
          }
        e_widget_focus_jump(parent, 1);
        if (sd->focused)
          {
             sd->focused = 0;
             if (sd->focus_func) sd->focus_func(obj);
          }
     }
   if (sd->disable_func) sd->disable_func(obj);
   if (sd->on_disable_func) sd->on_disable_func(sd->on_disable_data, obj);
}

E_API int
e_widget_disabled_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->disabled;
}

E_API E_Pointer *
e_widget_pointer_get(Evas_Object *obj)
{
   Evas_Object *win = NULL;

   API_ENTRY return NULL;
   win = e_win_evas_object_win_get(obj);
   if (win) return e_win_pointer_get(win);
   return NULL;
}

E_API void
e_widget_size_min_resize(Evas_Object *obj)
{
   Evas_Coord minw, minh;
   evas_object_size_hint_min_get(obj, &minw, &minh);
   evas_object_resize(obj, minw, minh);
}

/* local subsystem functions */
static void
_e_smart_reconfigure(E_Smart_Data *sd)
{
   if (sd->resize_obj)
     {
        evas_object_move(sd->resize_obj, sd->x, sd->y);
        evas_object_resize(sd->resize_obj, sd->w, sd->h);
     }
}

static void
_e_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd = NULL;

   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd) return;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->can_focus = 1;
   evas_object_smart_data_set(obj, sd);
}

static void
_e_smart_del(Evas_Object *obj)
{
   INTERNAL_ENTRY;

   if (sd->del_func) sd->del_func(obj);
   while (sd->subobjs)
     {
        Evas_Object *sobj = sd->subobjs->data;
        /* DO NOT MACRO THIS!
         * list gets reshuffled during object delete callback chain
         * and breaks the world.
         * BORKER CERTIFICATION: GOLD
         * -discomfitor, 7/4/2012
         */
        evas_object_event_callback_del(sobj, EVAS_CALLBACK_DEL, _sub_obj_del);
        sd->subobjs = eina_list_remove_list(sd->subobjs, sd->subobjs);
        evas_object_del(sobj);
     }
   free(sd);
}

static void
_e_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   sd->x = x;
   sd->y = y;
   _e_smart_reconfigure(sd);
}

static void
_e_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   sd->w = w;
   sd->h = h;
   _e_smart_reconfigure(sd);
}

static void
_e_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   if (sd->resize_obj)
     evas_object_show(sd->resize_obj);
}

static void
_e_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   if (sd->resize_obj)
     evas_object_hide(sd->resize_obj);
}

static void
_e_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   if (sd->resize_obj)
     evas_object_color_set(sd->resize_obj, r, g, b, a);
}

static void
_e_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   INTERNAL_ENTRY;
   if (sd->resize_obj)
     evas_object_clip_set(sd->resize_obj, clip);
}

static void
_e_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   if (sd->resize_obj)
     evas_object_clip_unset(sd->resize_obj);
}

/* never need to touch this */

static void
_e_smart_init(void)
{
   if (_e_smart) return;
   {
      static const Evas_Smart_Class sc =
      {
         SMART_NAME,
         EVAS_SMART_CLASS_VERSION,
         _e_smart_add,
         _e_smart_del,
         _e_smart_move,
         _e_smart_resize,
         _e_smart_show,
         _e_smart_hide,
         _e_smart_color_set,
         _e_smart_clip_set,
         _e_smart_clip_unset,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL
      };
      _e_smart = evas_smart_class_new(&sc);
   }
}

