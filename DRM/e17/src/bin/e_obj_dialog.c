#include "e.h"

/* local subsystem functions */
static void _e_obj_dialog_free(E_Obj_Dialog *od);
static void _e_obj_dialog_cb_delete(E_Obj_Dialog *od, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED);
static void _e_obj_dialog_cb_close(void *data, Evas_Object *obj, const char *emission, const char *source);

/* local subsystem globals */

/* externally accessible functions */

static void
_key_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   Evas_Event_Key_Down *ev = event;

   if (!strcmp(ev->key, "Escape") && data)
     _e_obj_dialog_cb_delete(data, NULL, NULL, NULL);
}

static void
_focus_in_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   edje_object_signal_emit(data, "e,state,focused", "e");
}

static void
_focus_out_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   edje_object_signal_emit(data, "e,state,unfocused", "e");
}

E_API E_Obj_Dialog *
e_obj_dialog_new(char *title, char *class_name, char *class_class)
{
   E_Obj_Dialog *od;
   Evas_Object *o;
   Eina_Bool kg;
   Evas_Modifier_Mask mask;

   od = E_OBJECT_ALLOC(E_Obj_Dialog, E_OBJ_DIALOG_TYPE, _e_obj_dialog_free);
   if (!od) return NULL;
   od->win = elm_win_add(NULL, NULL, ELM_WIN_DIALOG_BASIC);
   if (!od->win)
     {
        free(od);
        return NULL;
     }
   evas_object_event_callback_add(od->win, EVAS_CALLBACK_DEL, (Evas_Object_Event_Cb)_e_obj_dialog_cb_delete, od);
   ecore_evas_name_class_set(e_win_ee_get(od->win), class_name, class_class);
   elm_win_title_set(od->win, title);

   o = edje_object_add(evas_object_evas_get(od->win));
   elm_win_resize_object_add(od->win, o);
   od->bg_object = o;

   evas_object_smart_callback_add(od->win, "focus,in",
                                  _focus_in_cb, od->bg_object);
   evas_object_smart_callback_add(od->win, "focus,out",
                                  _focus_out_cb, od->bg_object);

   elm_win_center(od->win, 1, 1);
   od->cb_delete = NULL;

   mask = 0;
   kg = evas_object_key_grab(o, "Escape", mask, ~mask, 0);
   if (!kg)
     fprintf(stderr, "ERROR: unable to redirect \"Escape\" key events to object %p.\n", o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN,
                                  _key_down_cb, od);

   return od;
}

E_API void
e_obj_dialog_cb_delete_set(E_Obj_Dialog *od, void (*func)(E_Obj_Dialog *od))
{
   od->cb_delete = func;
}

E_API void
e_obj_dialog_icon_set(E_Obj_Dialog *od, char *icon)
{
   E_OBJECT_CHECK(od);
   E_OBJECT_TYPE_CHECK(od, E_OBJ_DIALOG_TYPE);
   e_win_client_icon_set(od->win, icon);
 }

E_API void
e_obj_dialog_show(E_Obj_Dialog *od)
{
   Evas_Coord w, h, mw, mh;
   const char *s;

   E_OBJECT_CHECK(od);
   E_OBJECT_TYPE_CHECK(od, E_OBJ_DIALOG_TYPE);

   edje_object_size_min_get(od->bg_object, &mw, &mh);
   edje_object_size_min_restricted_calc(od->bg_object, &mw, &mh, mw, mh);
   evas_object_resize(od->bg_object, mw, mh);
   evas_object_resize(od->win, mw, mh);
   evas_object_size_hint_min_set(od->win, mw, mh);
   edje_object_size_max_get(od->bg_object, &w, &h);
   if ((w > 0) && (h > 0))
     {
        if (w < mw) w = mw;
        if (h < mh) h = mh;
        evas_object_size_hint_max_set(od->win, w, h);
     }
   s = edje_object_data_get(od->bg_object, "borderless");
   if (s && (!strcmp(s, "1")))
     elm_win_borderless_set(od->win, 1);
   evas_object_show(od->win);
}

E_API void
e_obj_dialog_obj_part_text_set(E_Obj_Dialog *od, const char *part, const char *text)
{
   E_OBJECT_CHECK(od);
   E_OBJECT_TYPE_CHECK(od, E_OBJ_DIALOG_TYPE);
   edje_object_part_text_set(od->bg_object, part, text);
}

E_API void
e_obj_dialog_obj_theme_set(E_Obj_Dialog *od, char *theme_cat, char *theme_obj)
{
   E_OBJECT_CHECK(od);
   E_OBJECT_TYPE_CHECK(od, E_OBJ_DIALOG_TYPE);

   e_theme_edje_object_set(od->bg_object, theme_cat, theme_obj);
   evas_object_move(od->bg_object, 0, 0);
   evas_object_show(od->bg_object);
   edje_object_signal_callback_add(od->bg_object, "e,action,close", "",
                                   _e_obj_dialog_cb_close, od);
}

/* local subsystem functions */
static void
_e_obj_dialog_free(E_Obj_Dialog *od)
{
   if (od->bg_object) evas_object_del(od->bg_object);
   evas_object_del(od->win);
   free(od);
}

static void
_e_obj_dialog_cb_delete(E_Obj_Dialog *od, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   if (od->cb_delete)
     od->cb_delete(od);
   e_object_del(E_OBJECT(od));
}

static void
_e_obj_dialog_cb_close(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Obj_Dialog *od;

   od = data;
   if (od->cb_delete)
     od->cb_delete(od);
   e_util_defer_object_del(E_OBJECT(od));
}
