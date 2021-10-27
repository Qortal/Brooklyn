#include "e.h"
#include "e_mod_main.h"
#include "e_kbd_buf.h"
#include "e_kbd_int.h"
#include "e_kbd_send.h"
#include "e_kbd_cfg.h"

enum
{
   NORMAL   = 0,
   SHIFT    = (1 << 0),
   CAPSLOCK = (1 << 1),
   CTRL     = (1 << 2),
   ALT      = (1 << 3),
   ALTGR    = (1 << 4)
};

enum
{
   NONE,
   RIGHT,
   DOWN,
   LEFT,
   UP
};

static void _e_kbd_int_layout_next(E_Kbd_Int *ki);
static void _e_kbd_int_matches_update(void *data);


static Evas_Object *
_theme_obj_new(Evas *e, const char *custom_dir, const char *group)
{
   Evas_Object *o = edje_object_add(e);

   if (!e_theme_edje_object_set(o, "base/theme/modules/vkbd", group))
     {
        if (custom_dir)
          {
             char buf[PATH_MAX];

             snprintf(buf, sizeof(buf), "%s/theme.edj", custom_dir);
             edje_object_file_set(o, buf, group);
          }
     }
   return o;
}

static const char *
_e_kbd_int_str_unquote(const char *str)
{
   static char buf[256];
   char *p;

   snprintf(buf, sizeof(buf), "%s", str + 1);
   p = strrchr(buf, '"');
   if (p) *p = 0;
   return buf;
}

static E_Kbd_Int_Key *
_e_kbd_int_at_coord_get(E_Kbd_Int *ki, Evas_Coord x, Evas_Coord y)
{
   Eina_List *l;
   Evas_Coord dist;
   E_Kbd_Int_Key *ky;
   E_Kbd_Int_Key *closest_ky = NULL;

   EINA_LIST_FOREACH(ki->layout.keys, l, ky)
     if ((x >= ky->x) && (y >= ky->y) &&
         (x < (ky->x + ky->w)) && (y < (ky->y + ky->h)))
       return ky;
   dist = 0x7fffffff;
   EINA_LIST_FOREACH(ki->layout.keys, l, ky)
     {
        Evas_Coord dx, dy;

        dx = x - (ky->x + (ky->w / 2));
        dy = y - (ky->y + (ky->h / 2));
        dx = (dx * dx) + (dy * dy);
        if (dx < dist)
          {
             dist = dx;
             closest_ky = ky;
          }
     }
   return closest_ky;
}

static E_Kbd_Int_Key_State *
_e_kbd_int_key_state_get(E_Kbd_Int *ki, E_Kbd_Int_Key *ky)
{
   E_Kbd_Int_Key_State *found = NULL;
   E_Kbd_Int_Key_State *st;
   Eina_List *l;

   EINA_LIST_FOREACH(ky->states, l, st)
     {
        if (st->state & ki->layout.state) return st;
        if (!found && st->state == NORMAL) found = st;
     }
   return found;
}

static void
_e_kbd_int_layout_buf_update(E_Kbd_Int *ki)
{
   E_Kbd_Int_Key *ky;
   Eina_List *l, *l2;

   e_kbd_buf_layout_clear(ki->kbuf);
   e_kbd_buf_layout_size_set(ki->kbuf, ki->layout.w, ki->layout.h);
   e_kbd_buf_layout_fuzz_set(ki->kbuf, ki->layout.fuzz);
   EINA_LIST_FOREACH(ki->layout.keys, l, ky)
     {
        E_Kbd_Int_Key_State *st;
        const char *out, *out_shift, *out_capslock, *out_altgr;

        out = NULL;
        out_shift = NULL;
        out_capslock = NULL;
        out_altgr = NULL;

        EINA_LIST_FOREACH(ky->states, l2, st)
          {
             if (st->state == NORMAL)
               out = st->out;
             else if (st->state == SHIFT)
               out_shift = st->out;
             else if (st->state == CAPSLOCK)
               out_capslock = st->out;
             else if (st->state == ALTGR)
               out_altgr = st->out;
          }
        if (out)
          {
             char *s1 = NULL, *s2 = NULL, *s3 = NULL, *s4 = NULL;

             if ((out) && (out[0] == '"'))
               s1 = strdup(_e_kbd_int_str_unquote(out));
             if ((out_shift) && (out_shift[0] == '"'))
               s2 = strdup(_e_kbd_int_str_unquote(out_shift));
             if ((out_altgr) && (out_altgr[0] == '"'))
               s4 = strdup(_e_kbd_int_str_unquote(out_altgr));
             if ((out_capslock) && (out_capslock[0] == '"'))
               s3 = strdup(_e_kbd_int_str_unquote(out_capslock));
             e_kbd_buf_layout_key_add(ki->kbuf, s1, s2, s3, s4,
                                      ky->x, ky->y, ky->w, ky->h);
             free(s1);
             free(s2);
             free(s3);
             free(s4);
          }
     }
}

static void
_e_kbd_int_layout_state_update(E_Kbd_Int *ki)
{
   E_Kbd_Int_Key *ky;
   Eina_List *l;

   EINA_LIST_FOREACH(ki->layout.keys, l, ky)
     {
        E_Kbd_Int_Key_State *st;
        int selected;

        st = _e_kbd_int_key_state_get(ki, ky);
        if (st)
          {
             if (st->label)
               edje_object_part_text_set(ky->obj, "e.text.label", st->label);
             else
               edje_object_part_text_set(ky->obj, "e.text.label", "");
             if (st->icon)
               {
                  char buf[PATH_MAX];

                  snprintf(buf, sizeof(buf), "%s/%s",
                           ki->layout.directory, st->icon);
                  if (eina_str_has_extension(st->icon, ".edj"))
                    e_icon_file_edje_set(ky->icon_obj, buf, "icon");
                  else
                    e_icon_file_set(ky->icon_obj, buf);
               }
             else
               e_icon_file_set(ky->icon_obj, NULL);
          }
        selected = 0;
        if ((ki->layout.state & SHIFT) && (ky->is_shift)) selected = 1;
        if ((ki->layout.state & CTRL) && (ky->is_ctrl)) selected = 1;
        if ((ki->layout.state & ALT) && (ky->is_alt)) selected = 1;
        if ((ki->layout.state & ALTGR) && (ky->is_altgr)) selected = 1;
        if ((ki->layout.state & CAPSLOCK) && (ky->is_capslock)) selected = 1;
        if ((ki->layout.state & (SHIFT | CAPSLOCK)) && (ky->is_multi_shift))
          selected = 1;
        if (selected)
          {
             if (!ky->selected)
               {
                  edje_object_signal_emit(ky->obj, "e,state,selected", "e");
                  ky->selected = 1;
               }
          }
        if (!selected)
          {
             if (ky->selected)
               {
                  edje_object_signal_emit(ky->obj, "e,state,unselected", "e");
                  ky->selected = 0;
               }
          }
     }
}

static void
_e_kbd_int_string_send(E_Kbd_Int *ki, const char *str)
{
   int pos, newpos, glyph;

   pos = 0;
   e_kbd_buf_word_use(ki->kbuf, str);
   for (;;)
     {
        char buf[16];

        newpos = evas_string_char_next_get(str, pos, &glyph);
        if (glyph <= 0) return;
        strncpy(buf, str + pos, newpos - pos);
        buf[newpos - pos] = 0;
        e_kbd_send_string_press(buf, 0);
        pos = newpos;
     }
}

static void
_e_kbd_int_buf_send(E_Kbd_Int *ki)
{
   const char *str = NULL;
   const Eina_List *matches;

   matches = e_kbd_buf_string_matches_get(ki->kbuf);
   if (matches) str = matches->data;
   else str = e_kbd_buf_actual_string_get(ki->kbuf);
   if (str) _e_kbd_int_string_send(ki, str);
}

static void
_e_kbd_int_cb_match_select(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Kbd_Int_Match *km;

   km = data;
   _e_kbd_int_string_send(km->ki, km->str);
   e_kbd_buf_clear(km->ki->kbuf);
   e_kbd_send_keysym_press("space", 0);
   if (km->ki->layout.state & (SHIFT | CTRL | ALT | ALTGR))
     {
        km->ki->layout.state &= (~(SHIFT | CTRL | ALT | ALTGR));
        _e_kbd_int_layout_state_update(km->ki);
     }
   _e_kbd_int_matches_update(km->ki);
}

static void
_e_kbd_int_matches_add(E_Kbd_Int *ki, const char *str, int num)
{
   E_Kbd_Int_Match *km;
   Evas_Object *o;
   Evas_Coord mw, mh;

   km = E_NEW(E_Kbd_Int_Match, 1);
   if (!km) return;
   o = _theme_obj_new(evas_object_evas_get(ki->base_obj), ki->themedir,
                      "e/modules/kbd/match/word");
   km->ki = ki;
   km->str = eina_stringshare_add(str);
   km->obj = o;
   ki->matches = eina_list_append(ki->matches, km);

   edje_object_part_text_set(o, "e.text.label", str);
   edje_object_size_min_calc(o, &mw, &mh);
   if (mw < 32) mw = 32;
   evas_object_size_hint_weight_set(o, 1.0, 1.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (num & 0x1) elm_box_pack_start(ki->box_obj, o);
   else elm_box_pack_end(ki->box_obj, o);
   E_EXPAND(o);
   if (mw < (10 * e_scale * il_kbd_cfg->size))
     mw = 10 * e_scale * il_kbd_cfg->size;
   if (mh < (10 * e_scale * il_kbd_cfg->size))
     mh = 10 * e_scale * il_kbd_cfg->size;
   evas_object_size_hint_min_set(o, mw, mh);
   if (num == 0)
     edje_object_signal_emit(o, "e,state,selected", "e");
   edje_object_signal_callback_add(o, "e,action,do,select", "e",
                                   _e_kbd_int_cb_match_select, km);
   evas_object_show(o);
}

static void
_e_kbd_int_matches_free(E_Kbd_Int *ki)
{
   E_Kbd_Int_Match *km;

   EINA_LIST_FREE(ki->matches, km)
     {
        if (km->str) eina_stringshare_del(km->str);
        evas_object_del(km->obj);
        free(km);
     }
}

static void
_e_kbd_int_recenter(E_Kbd_Int *ki)
{
   Evas_Coord mw = 0, mh = 0, lmw = 0, lmh = 0, w, h;

   evas_object_size_hint_min_get(ki->layout_obj, &lmw, &lmh);
   edje_object_size_min_calc(ki->base_obj, &mw, &mh);
   if (mw < (40 * e_scale)) mw = (40 * e_scale);
   if (mh < (40 * e_scale)) mh = (40 * e_scale);
   if (il_kbd_cfg->fill_mode == 0)
     {
        if (mw > ki->zone->w)
          {
             w = ki->zone->w - (mw - lmw);
             h = (w * lmh) / lmw;
             mw = w + (mw - lmw);
             mh = h + (mh - lmh);
          }
        ki->x = ki->zone->x + ((ki->zone->w - mw) / 2);
        ki->y = ki->zone->y + ki->zone->h - mh;
     }
   else if (il_kbd_cfg->fill_mode == 1)
     {
        mw = ki->zone->w;
        ki->x = ki->zone->x + ((ki->zone->w - mw) / 2);
        ki->y = ki->zone->y + ki->zone->h - mh;
     }
   else if (il_kbd_cfg->fill_mode == 2)
     {
        w = ki->zone->w - (mw - lmw);
        h = (w * lmh) / lmw;
        mw = w + (mw - lmw);
        mh = h + (mh - lmh);
        ki->x = ki->zone->x + ((ki->zone->w - mw) / 2);
        ki->y = ki->zone->y + ki->zone->h - mh;
     }
   else
     {
        if (ki->down.down)
          {
             if (ki->x < ki->zone->x) ki->x = 0;
             if (ki->y < ki->zone->y) ki->y = 0;
             if ((ki->x + mw) > (ki->zone->x + ki->zone->w))
               ki->x = ki->zone->x + ki->zone->w - mw;
             if ((ki->y + mh) > (ki->zone->y + ki->zone->h))
               ki->y = ki->zone->y + ki->zone->h - mh;
          }
        else
          {
             ki->x = ki->zone->x + (ki->px * (ki->zone->w - mw));
             ki->y = ki->zone->y + (ki->py * (ki->zone->h - mh));
          }
     }
   evas_object_move(ki->base_obj, ki->x, ki->y);
   evas_object_resize(ki->base_obj, mw, mh);
}

static void
_e_kbd_int_matches_update(void *data)
{
   E_Kbd_Int *ki;
   const Eina_List *l, *matches;
   const char *actual;
   Evas_Coord mw, mh;

   if (!(ki = data)) return;
   _e_kbd_int_matches_free(ki);
   matches = e_kbd_buf_string_matches_get(ki->kbuf);
   if (!matches)
     {
        actual = e_kbd_buf_actual_string_get(ki->kbuf);
        if (actual) _e_kbd_int_matches_add(ki, actual, 0);
     }
   else
     {
        int i;

        actual = e_kbd_buf_actual_string_get(ki->kbuf);
        for (i = 0, l = matches; l; l = l->next)
          {
             if ((i == 1) && (actual) &&
                 (!(!strcmp(matches->data, actual))))
               {
                  _e_kbd_int_matches_add(ki, actual, i);
                  i++;
               }
             if ((i > 1) && (actual) && (!strcmp(l->data, actual))) continue;
             _e_kbd_int_matches_add(ki, l->data, i);
             i++;
          }
     }
   evas_smart_objects_calculate(evas_object_evas_get(ki->box_obj));
   evas_object_resize(ki->boxgrid_obj, 0, 0);
   evas_object_size_hint_min_get(ki->box_obj, &mw, &mh);
   if (matches)
     {
        if (mh < (10 * e_scale * il_kbd_cfg->size))
          mh = 10 * e_scale * il_kbd_cfg->size;
     }
   evas_object_size_hint_min_set(ki->boxgrid_obj, 0, mh);
   evas_smart_objects_calculate(evas_object_evas_get(ki->boxgrid_obj));
   edje_object_part_swallow(ki->base_obj, "e.swallow.completion",
                            ki->boxgrid_obj);

   _e_kbd_int_recenter(ki);
}

static void
_e_kbd_int_key_press_handle(E_Kbd_Int *ki, E_Kbd_Int_Key *ky, int x, int y)
{
   E_Kbd_Int_Key_State *st;
   const char *out = NULL;

   if (!ky) return;

   if (ky->is_shift)
     {
        if (ki->layout.state & SHIFT) ki->layout.state &= (~(SHIFT));
        else ki->layout.state |= SHIFT;
        _e_kbd_int_layout_state_update(ki);
        return;
     }
   if (ky->is_multi_shift)
     {
        if (ki->layout.state & SHIFT)
          {
             ki->layout.state &= (~(SHIFT));
             ki->layout.state |= CAPSLOCK;
          }
        else if (ki->layout.state & CAPSLOCK)
          ki->layout.state &= (~(CAPSLOCK));
        else
          ki->layout.state |= SHIFT;
        _e_kbd_int_layout_state_update(ki);
        return;
     }
   if (ky->is_ctrl)
     {
        if (ki->layout.state & CTRL) ki->layout.state &= (~(CTRL));
        else ki->layout.state |= CTRL;
        if (e_kbd_buf_actual_string_get(ki->kbuf)) _e_kbd_int_buf_send(ki);
        e_kbd_buf_clear(ki->kbuf);
        _e_kbd_int_layout_state_update(ki);
        _e_kbd_int_matches_update(ki);
        return;
     }
   if (ky->is_alt)
     {
        if (ki->layout.state & ALT) ki->layout.state &= (~(ALT));
        else ki->layout.state |= ALT;
        if (e_kbd_buf_actual_string_get(ki->kbuf)) _e_kbd_int_buf_send(ki);
        e_kbd_buf_clear(ki->kbuf);
        _e_kbd_int_layout_state_update(ki);
        _e_kbd_int_matches_update(ki);
        return;
     }
   if (ky->is_altgr)
     {
        if (ki->layout.state & ALTGR) ki->layout.state &= (~(ALTGR));
        else ki->layout.state |= ALTGR;
        _e_kbd_int_layout_state_update(ki);
        return;
     }
   if (ky->is_capslock)
     {
        if (ki->layout.state & CAPSLOCK) ki->layout.state &= (~CAPSLOCK);
        else ki->layout.state |= CAPSLOCK;
        _e_kbd_int_layout_state_update(ki);
        return;
     }
   st = _e_kbd_int_key_state_get(ki, ky);
   if (st) out = st->out;
   if (ki->layout.state & (CTRL | ALT))
     {
        if (out)
          {
             Kbd_Mod mods = 0;

             if (ki->layout.state & CTRL) mods |= KBD_MOD_CTRL;
             if (ki->layout.state & ALT) mods |= KBD_MOD_ALT;
             if (out[0] == '"')
               e_kbd_send_string_press(_e_kbd_int_str_unquote(out), mods);
             else
               e_kbd_send_keysym_press(out, mods);
          }
        ki->layout.state &= (~(SHIFT | CTRL | ALT | ALTGR));
        _e_kbd_int_layout_state_update(ki);
        e_kbd_buf_lookup(ki->kbuf, _e_kbd_int_matches_update, ki);
        return;
     }
   if (out)
     {
        if (!strcmp(out, "CONFIG"))
          {
             e_kbd_cfg_show(ki);
          }
        else if (out[0] == '"')
          {
             e_kbd_buf_pressed_point_add(ki->kbuf, x, y,
                                         ki->layout.state & SHIFT,
                                         ki->layout.state & CAPSLOCK);
             e_kbd_buf_lookup(ki->kbuf, _e_kbd_int_matches_update, ki);
          }
        else
          {
             if (e_kbd_buf_actual_string_get(ki->kbuf))
               _e_kbd_int_buf_send(ki);
             e_kbd_buf_clear(ki->kbuf);
             e_kbd_send_keysym_press(out, 0);
             _e_kbd_int_matches_update(ki);
          }
     }
   if (ki->layout.state & (SHIFT | CTRL | ALT | ALTGR))
     {
        if (!ky->is_multi_shift)
          ki->layout.state &= (~(SHIFT | CTRL | ALT | ALTGR));
        _e_kbd_int_layout_state_update(ki);
     }
}

static void
_e_kbd_int_stroke_handle(E_Kbd_Int *ki, int dir)
{
   /* If the keyboard direction is RTL switch dir 3 and 1
    * i.e, make forward backwards and the other way around */
   if (ki->layout.direction == E_KBD_INT_DIRECTION_RTL)
     {
        if (dir == LEFT)       dir = RIGHT;
        else if (dir == RIGHT) dir = LEFT;
     }

   if (dir == UP) // up
     _e_kbd_int_layout_next(ki);
   else if (dir == LEFT) // left
     {
        if (e_kbd_buf_actual_string_get(ki->kbuf))
          {
             e_kbd_buf_backspace(ki->kbuf);
             e_kbd_buf_lookup(ki->kbuf, _e_kbd_int_matches_update, ki);
          }
        else
          e_kbd_send_keysym_press("BackSpace", 0);
     }
   else if (dir == DOWN) // down
     {
        if (e_kbd_buf_actual_string_get(ki->kbuf)) _e_kbd_int_buf_send(ki);
        e_kbd_buf_clear(ki->kbuf);
        e_kbd_send_keysym_press("Return", 0);
        _e_kbd_int_matches_update(ki);
     }
   else if (dir == RIGHT) // right
     {
        if (e_kbd_buf_actual_string_get(ki->kbuf)) _e_kbd_int_buf_send(ki);
        e_kbd_buf_clear(ki->kbuf);
        e_kbd_send_keysym_press("space", 0);
        _e_kbd_int_matches_update(ki);
     }
   if (ki->layout.state)
     {
        ki->layout.state = 0;
        _e_kbd_int_layout_state_update(ki);
     }
}

static void
_e_kbd_int_scale_coords(E_Kbd_Int *ki, Evas_Coord inx, Evas_Coord iny, int *x, int *y)
{
   Evas_Coord xx, yy, ww, hh;

   evas_object_geometry_get(ki->event_obj, &xx, &yy, &ww, &hh);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   inx = inx - xx;
   iny = iny - yy;
   *x = (inx * ki->layout.w) / ww;
   *y = (iny * ki->layout.h) / hh;
}

static void
_e_kbd_int_key_press(E_Kbd_Int *ki, int x, int y, Eina_Bool is_first, int device)
{
   E_Kbd_Int_Key *ky = _e_kbd_int_at_coord_get(ki, x, y);

   if ((ky) && (!ky->pressed))
     {
        if (is_first) ki->layout.pressed = ky;
        else
          {
             E_Kbd_Int_Multi_Info *inf =
               calloc(1, sizeof(E_Kbd_Int_Multi_Info));
             if (inf)
               {
                  inf->device = device;
                  inf->ky = ky;
                  ki->layout.multis = eina_list_append(ki->layout.multis, inf);
               }
          }
        ky->pressed = 1;
        evas_object_raise(ky->obj);
        evas_object_raise(ki->event_obj);
        edje_object_signal_emit(ky->obj, "e,state,pressed", "e");
     }
}

static void
_e_kbd_int_key_release(E_Kbd_Int *ki, E_Kbd_Int_Key *ky)
{
   if ((ky) && (ky->pressed))
     {
        if (ky == ki->layout.pressed) ki->layout.pressed = NULL;
        else
          {
             Eina_List *l;
             E_Kbd_Int_Multi_Info *inf;

             EINA_LIST_FOREACH(ki->layout.multis, l, inf)
               {
                  if (inf->ky == ky)
                    {
                       ki->layout.multis = eina_list_remove_list
                         (ki->layout.multis, l);
                       free(inf);
                       break;
                    }
               }
          }
        ky->pressed = 0;
        edje_object_signal_emit(ky->obj, "e,state,released", "e");
     }
}

static void
_e_kbd_int_cb_multi_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Down *ev = event_info;
   E_Kbd_Int *ki = data;
   int x, y;

   if (ev->device <= 0) return;

   _e_kbd_int_scale_coords(ki, ev->canvas.x, ev->canvas.y, &x, &y);
   _e_kbd_int_key_press(ki, x, y, EINA_FALSE, ev->device);
}
/*
static void
_e_kbd_int_cb_multi_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Move *ev = event_info;
   E_Kbd_Int *ki = data;

   if (ev->device <= 0) return;
   if (!ki) return;
}
*/
static void
_e_kbd_int_cb_multi_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Up *ev = event_info;
   E_Kbd_Int *ki = data;
   Eina_List *l;
   E_Kbd_Int_Multi_Info *inf;
   int x, y;

   if (ev->device <= 0) return;

   _e_kbd_int_scale_coords(ki, ev->canvas.x, ev->canvas.y, &x, &y);

   EINA_LIST_FOREACH(ki->layout.multis, l, inf)
     {
        if (inf->device == ev->device)
          {
             _e_kbd_int_key_press_handle(ki, inf->ky, x, y);
             _e_kbd_int_key_release(ki, inf->ky);
             return;
          }
     }
}

static void
_e_kbd_int_cb_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   E_Kbd_Int *ki = data;
   int x, y;

   if (ev->button != 1) return;
   ki->down.x = ev->canvas.x;
   ki->down.y = ev->canvas.y;
   ki->down.down = 1;
   ki->down.stroke = 0;
   ki->down.twofinger = 0;

   _e_kbd_int_scale_coords(ki, ev->canvas.x, ev->canvas.y, &x, &y);

   ki->down.lx = x;
   ki->down.ly = y;
   ki->x0 = ki->x;
   ki->y0 = ki->y;

   _e_kbd_int_key_press(ki, x, y, EINA_TRUE, 0);
}

static void
_e_kbd_int_cb_mouse_move(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   E_Kbd_Int *ki = data;
   E_Kbd_Int_Key *ky;
   E_Kbd_Int_Multi_Info *inf;
   Evas_Coord dx, dy;

   if (ki->down.stroke)
     {
        if (ki->down.twofinger)
          {
             if (il_kbd_cfg->fill_mode == 3)
               {
                  ki->x = ki->x0 + ev->cur.canvas.x - ki->down.x;
                  ki->y = ki->y0 + ev->cur.canvas.y - ki->down.y;
                  _e_kbd_int_recenter(ki);
               }
          }
        return;
     }

   dx = (ev->cur.canvas.x - ki->down.x) / (e_scale * il_kbd_cfg->size);
   dy = (ev->cur.canvas.y - ki->down.y) / (e_scale * il_kbd_cfg->size);
   if      ((dx > 0) && (dx  > il_kbd_cfg->slide_dim)) ki->down.stroke = 1;
   else if ((dx < 0) && (-dx > il_kbd_cfg->slide_dim)) ki->down.stroke = 1;
   if      ((dy > 0) && (dy  > il_kbd_cfg->slide_dim)) ki->down.stroke = 1;
   else if ((dy < 0) && (-dy > il_kbd_cfg->slide_dim)) ki->down.stroke = 1;
   if (ki->down.stroke)
     {
        if ((ki->down.down) &&
            (// 2 finger drag
             (eina_list_count(ki->layout.multis) == 1) ||
             // or ctrl is held
             (evas_key_modifier_is_set(ev->modifiers, "Control"))))
          ki->down.twofinger = 1;
        ky = ki->layout.pressed;
        if (ky) _e_kbd_int_key_release(ki, ky);
        while (ki->layout.multis)
          {
             inf = ki->layout.multis->data;
             _e_kbd_int_key_release(ki, inf->ky);
          }
     }
}

static void
_e_kbd_int_cb_mouse_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   E_Kbd_Int *ki = data;
   E_Kbd_Int_Key *ky;
   int x, y, dir = 0;

   if (ev->button != 1) return;

   if (il_kbd_cfg->fill_mode == 3)
     {
        Evas_Coord w, h;

        evas_object_geometry_get(ki->base_obj, NULL, NULL, &w, &h);
        w = ki->zone->w - w;
        h = ki->zone->h - h;
        if (w <= 0) ki->px = 0.0;
        else ki->px = (double)(ki->x - ki->zone->x) / (double)(w);
        if (h <= 0) ki->py = 0.0;
        else ki->py = (double)(ki->y - ki->zone->y) / (double)(h);
        il_kbd_cfg->px = ki->px;
        il_kbd_cfg->py = ki->py;
        e_config_save_queue();
     }
   _e_kbd_int_scale_coords(ki, ev->canvas.x, ev->canvas.y, &x, &y);

   if (!ki->down.stroke)
     {
        ky = ki->layout.pressed;
        if (ky) _e_kbd_int_key_press_handle(ki, ky, x, y);
     }
   else
     {
        Evas_Coord dx, dy;

        dx = ev->canvas.x - ki->down.x;
        dy = ev->canvas.y - ki->down.y;
        if (dx > 0)
          {
             if (dy > 0)
               {
                  if (dx > dy) dir = RIGHT;
                  else         dir = DOWN;
               }
             else
               {
                  if (dx > -dy) dir = RIGHT;
                  else          dir = UP;
               }
          }
        else
          {
             if (dy > 0)
               {
                  if (-dx > dy) dir = LEFT;
                  else          dir = DOWN;
               }
             else
               {
                  if (-dx > -dy) dir = LEFT;
                  else           dir = UP;
               }
          }
     }

   ky = ki->layout.pressed;
   if (ky) _e_kbd_int_key_release(ki, ky);

   ki->down.down = 0;
   ki->down.stroke = 0;

   if (dir > 0)
     {
        if ((ki->down.twofinger) &&
            (il_kbd_cfg->fill_mode != 3))
          {
             // handle 2 finger gesture
             if (dir == DOWN) e_kbd_int_hide(ki);
          }
        else if (!ki->down.twofinger)
          _e_kbd_int_stroke_handle(ki, dir);
     }
   if (il_kbd_cfg->fill_mode == 3)
     _e_kbd_int_recenter(ki);

   ki->down.twofinger = 0;
}

static E_Kbd_Int_Layout *
_e_kbd_int_layouts_list_default_get(E_Kbd_Int *ki)
{
   E_Kbd_Int_Layout *kil;
   Eina_List *l;

   EINA_LIST_FOREACH(ki->layouts, l, kil)
     {
        if (kil->type == il_kbd_cfg->layout) return kil;
     }

   EINA_LIST_FOREACH(ki->layouts, l, kil)
     {
        if ((!strcmp(ecore_file_file_get(kil->path), "Default.kbd")))
          return kil;
     }
   return NULL;
}

static void
_e_kbd_int_layout_free(E_Kbd_Int *ki)
{
   E_Kbd_Int_Key *ky;
   E_Kbd_Int_Multi_Info *inf;

   free(ki->layout.directory);
   if (ki->layout.file) eina_stringshare_del(ki->layout.file);
   ki->layout.directory = NULL;
   ki->layout.file = NULL;
   ki->layout.pressed = NULL;
   EINA_LIST_FREE(ki->layout.multis, inf) free(inf);
   EINA_LIST_FREE(ki->layout.keys, ky)
     {
        E_Kbd_Int_Key_State *st;

        EINA_LIST_FREE(ky->states, st)
          {
             if (st->label) eina_stringshare_del(st->label);
             if (st->icon) eina_stringshare_del(st->icon);
             if (st->out) eina_stringshare_del(st->out);
             free(st);
          }
        if (ky->obj) evas_object_del(ky->obj);
        if (ky->icon_obj) evas_object_del(ky->icon_obj);
        free(ky);
     }
   if (ki->event_obj) evas_object_del(ki->event_obj);
   ki->event_obj = NULL;
}

static void
_e_kbd_int_layout_parse(E_Kbd_Int *ki, const char *layout)
{
   FILE *f;
   char buf[4096];
   int isok = 0;
   E_Kbd_Int_Key *ky = NULL;
   E_Kbd_Int_Key_State *st = NULL;

   if (!(f = fopen(layout, "r"))) return;

   ki->layout.directory = ecore_file_dir_get(layout);
   ki->layout.file = eina_stringshare_add(layout);

   /* Make the default direction LTR */
   ki->layout.direction = E_KBD_INT_DIRECTION_LTR;

   while (fgets(buf, sizeof(buf), f))
     {
        int len;
        char str[4096];

        if (!isok)
          {
             if (!strcmp(buf, "##KBDCONF-1.0\n")) isok = 1;
          }
        if (!isok) break;
        if (buf[0] == '#') continue;
        len = strlen(buf);
        if (len > 0)
          {
             if (buf[len - 1] == '\n') buf[len - 1] = 0;
          }
        if (sscanf(buf, "%4000s", str) != 1) continue;
        if (!strcmp(str, "kbd"))
          {
             if (sscanf(buf, "%*s %i %i\n",
                        &(ki->layout.w), &(ki->layout.h)) != 2)
               continue;
          }
        if (!strcmp(str, "fuzz"))
          {
             sscanf(buf, "%*s %i\n", &(ki->layout.fuzz));
             continue;
          }
        if (!strcmp(str, "direction"))
          {
             char direction[4];

             sscanf(buf, "%*s %3s\n", direction);

             /* If rtl mark as rtl, otherwise make it ltr */
             if (!strcmp(direction, "rtl"))
               ki->layout.direction = E_KBD_INT_DIRECTION_RTL;
             else
               ki->layout.direction = E_KBD_INT_DIRECTION_LTR;
             continue;
          }
        if (!strcmp(str, "key"))
          {
             ky = calloc(1, sizeof(E_Kbd_Int_Key));
             if (!ky) continue;
             if (sscanf(buf, "%*s %i %i %i %i\n",
                        &(ky->x), &(ky->y), &(ky->w), &(ky->h)) != 4)
               {
                  E_FREE(ky);
                  continue;
               }
             ki->layout.keys = eina_list_append(ki->layout.keys, ky);
          }
        if (!ky) continue;
        if ((!strcmp(str, "normal")) || (!strcmp(str, "shift")) ||
            (!strcmp(str, "capslock")) || (!strcmp(str, "altgr")))
          {
             char label[4096];

             if (sscanf(buf, "%*s %4000s", label) != 1) continue;
             st = calloc(1, sizeof(E_Kbd_Int_Key_State));
             if (!st) continue;
             ky->states = eina_list_append(ky->states, st);
             if (!strcmp(str, "normal")) st->state = NORMAL;
             if (!strcmp(str, "shift")) st->state = SHIFT;
             if (!strcmp(str, "capslock")) st->state = CAPSLOCK;
             if (!strcmp(str, "altgr")) st->state = ALTGR;
             if (eina_str_has_extension(label, ".png") || eina_str_has_extension(label, ".edj"))
               st->icon = eina_stringshare_add(label);
             else
               st->label = eina_stringshare_add(label);
             if (sscanf(buf, "%*s %*s %4000s", str) != 1) continue;
             st->out = eina_stringshare_add(str);
          }
        if (!strcmp(str, "is_shift")) ky->is_shift = 1;
        if (!strcmp(str, "is_multi_shift")) ky->is_multi_shift = 1;
        if (!strcmp(str, "is_ctrl")) ky->is_ctrl = 1;
        if (!strcmp(str, "is_alt")) ky->is_alt = 1;
        if (!strcmp(str, "is_altgr")) ky->is_altgr = 1;
        if (!strcmp(str, "is_capslock")) ky->is_capslock = 1;
     }
   fclose(f);
}

static void
_e_kbd_int_layout_build(E_Kbd_Int *ki)
{
   E_Kbd_Int_Key *ky;
   Evas_Object *o, *o2;
   Eina_List *l;
   Evas_Coord mw, mh;

   evas_object_grid_size_set(ki->layout_obj, ki->layout.w, ki->layout.h);

   EINA_LIST_FOREACH(ki->layout.keys, l, ky)
     {
        E_Kbd_Int_Key_State *st;
        const char *label, *icon;

        o = _theme_obj_new(e_comp->evas, ki->themedir,
                           "e/modules/kbd/key/default");
        ky->obj = o;
        label = "";
        icon = NULL;
        st = _e_kbd_int_key_state_get(ki, ky);
        if (st)
          {
             label = st->label;
             icon = st->icon;
          }

        edje_object_part_text_set(o, "e.text.label", label);

        if (icon)
          {
             char buf[PATH_MAX];

             o2 = e_icon_add(e_comp->evas);
             e_icon_fill_inside_set(o2, 1);
             e_icon_scale_up_set(o2, 0);
             ky->icon_obj = o2;
             edje_object_part_swallow(o, "e.swallow.content", o2);
             evas_object_show(o2);

             snprintf(buf, sizeof(buf), "%s/%s", ki->layout.directory, icon);
             if (eina_str_has_extension(icon, ".edj")) e_icon_file_edje_set(o2, buf, "icon");
             else e_icon_file_set(o2, buf);
          }
        evas_object_grid_pack(ki->layout_obj, o,
                              ky->x, ky->y, ky->w, ky->h);
        evas_object_show(o);
     }

   o = evas_object_rectangle_add(e_comp->evas);
   ki->event_obj = o;
   evas_object_grid_pack(ki->layout_obj, o,
                         0, 0, ki->layout.w, ki->layout.h);
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN,
                                  _e_kbd_int_cb_mouse_down, ki);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE,
                                  _e_kbd_int_cb_mouse_move, ki);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,
                                  _e_kbd_int_cb_mouse_up, ki);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MULTI_DOWN,
                                  _e_kbd_int_cb_multi_down, ki);
//   evas_object_event_callback_add(o, EVAS_CALLBACK_MULTI_MOVE,
//                                  _e_kbd_int_cb_multi_move, ki);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MULTI_UP,
                                  _e_kbd_int_cb_multi_up, ki);
   evas_object_show(o);

   mw = ki->layout.w * e_scale * il_kbd_cfg->size;
   mh = ki->layout.h * e_scale * il_kbd_cfg->size;
   if (mw > ki->zone->w)
     {
        mh = (mw * mh) / ki->zone->w;
        mw = ki->zone->w;
     }
   evas_object_size_hint_min_set(ki->layout_obj, mw, mh);
   edje_object_part_swallow(ki->base_obj, "e.swallow.content", ki->layout_obj);
}

static void
_e_kbd_int_layouts_free(E_Kbd_Int *ki)
{
   E_Kbd_Int_Layout *kil;

   EINA_LIST_FREE(ki->layouts, kil)
     {
        eina_stringshare_del(kil->path);
        eina_stringshare_del(kil->dir);
        eina_stringshare_del(kil->icon);
        eina_stringshare_del(kil->name);
        free(kil);
     }
}

static void
_e_kbd_int_layouts_list_update(E_Kbd_Int *ki)
{
   Eina_List *files;
   Eina_List *l;
   char buf[PATH_MAX + 100], *file, *path;
   const char *fl;
   Eina_List *kbs = NULL, *layouts = NULL;
   int ok;
   size_t len;

   len = e_user_dir_concat_static(buf, "keyboards");
   if (len + 2 >= sizeof(buf)) return;

   files = ecore_file_ls(buf);

   buf[len] = '/';
   len++;

   EINA_LIST_FREE(files, file)
     {
        if (eina_str_has_extension(file, ".kbd"))
          {
             if (eina_strlcpy(buf + len, file, sizeof(buf) - len) >=
                 sizeof(buf) - len)
               continue;
             kbs = eina_list_append(kbs, eina_stringshare_add(buf));
          }
        free(file);
     }

   len = snprintf(buf, sizeof(buf), "%s/keyboards", ki->syskbds);
   if (len + 2 >= sizeof(buf)) return;

   files = ecore_file_ls(buf);

   buf[len] = '/';
   len++;

   EINA_LIST_FREE(files, file)
     {
        if (eina_str_has_extension(file, ".kbd"))
          {
             ok = 1;
             EINA_LIST_FOREACH(kbs, l, fl)
               {
                  if (!strcmp(file, fl))
                    {
                       ok = 0;
                       break;
                    }
               }
             if (ok)
               {
                  if (eina_strlcpy(buf + len, file, sizeof(buf) - len) >=
                      sizeof(buf) - len)
                    continue;
                  kbs = eina_list_append(kbs, eina_stringshare_add(buf));
               }
          }
        free(file);
     }
   /* Previous loop could break before destroying all items. */
   EINA_LIST_FREE(files, file) free(file);

   EINA_LIST_FREE(kbs, path)
     {
        E_Kbd_Int_Layout *kil;

        kil = E_NEW(E_Kbd_Int_Layout, 1);
        if (kil)
          {
             char *s;
             FILE *f;

             kil->path = path;
             s = strdup(ecore_file_file_get(kil->path));
             if (s)
               {
                  char *p = strrchr(s, '.');
                  if (p) *p = 0;
                  kil->name = eina_stringshare_add(s);
                  free(s);
               }
             s = ecore_file_dir_get(kil->path);
             if (s)
               {
                  kil->dir = eina_stringshare_add(s);
                  free(s);
               }
             f = fopen(kil->path, "r");
             if (f)
               {
                  int isok = 0;

                  while (fgets(buf, sizeof(buf), f))
                    {
                       char str[PATH_MAX];

                       if (!isok)
                         {
                            if (!strcmp(buf, "##KBDCONF-1.0\n")) isok = 1;
                         }
                       if (!isok) break;
                       if (buf[0] == '#') continue;
                       len = strlen(buf);
                       if (len > 0)
                         {
                            if (buf[len - 1] == '\n') buf[len - 1] = 0;
                         }
                       if (sscanf(buf, "%4000s", str) != 1) continue;
                       if (!strcmp(str, "type"))
                         {
                            sscanf(buf, "%*s %4000s\n", str);
                            if (!strcmp(str, "ALPHA"))
                              kil->type = E_KBD_INT_TYPE_ALPHA;
                            else if (!strcmp(str, "NUMERIC"))
                              kil->type = E_KBD_INT_TYPE_NUMERIC;
                            else if (!strcmp(str, "PIN"))
                              kil->type = E_KBD_INT_TYPE_PIN;
                            else if (!strcmp(str, "PHONE_NUMBER"))
                              kil->type = E_KBD_INT_TYPE_PHONE_NUMBER;
                            else if (!strcmp(str, "HEX"))
                              kil->type = E_KBD_INT_TYPE_HEX;
                            else if (!strcmp(str, "TERMINAL"))
                              kil->type = E_KBD_INT_TYPE_TERMINAL;
                            else if (!strcmp(str, "PASSWORD"))
                              kil->type = E_KBD_INT_TYPE_PASSWORD;
                            else if (!strcmp(str, "IP"))
                              kil->type = E_KBD_INT_TYPE_IP;
                            else if (!strcmp(str, "HOST"))
                              kil->type = E_KBD_INT_TYPE_HOST;
                            else if (!strcmp(str, "FILE"))
                              kil->type = E_KBD_INT_TYPE_FILE;
                            else if (!strcmp(str, "URL"))
                              kil->type = E_KBD_INT_TYPE_URL;
                            else if (!strcmp(str, "KEYPAD"))
                              kil->type = E_KBD_INT_TYPE_KEYPAD;
                            else if (!strcmp(str, "J2ME"))
                              kil->type = E_KBD_INT_TYPE_J2ME;
                            continue;
                         }
                       if (!strcmp(str, "icon"))
                         {
                            sscanf(buf, "%*s %4000s\n", str);
                            snprintf(buf, sizeof(buf), "%s/%s", kil->dir, str);
                            kil->icon = eina_stringshare_add(buf);
                            continue;
                         }
                    }
                  fclose(f);
               }
             layouts = eina_list_append(layouts, kil);
          }
     }
   _e_kbd_int_layouts_free(ki);
   ki->layouts = layouts;
}

static void
_e_kbd_int_layout_select(E_Kbd_Int *ki, E_Kbd_Int_Layout *kil)
{
   _e_kbd_int_layout_free(ki);
   _e_kbd_int_layout_parse(ki, kil->path);
   _e_kbd_int_layout_build(ki);
   _e_kbd_int_layout_buf_update(ki);
   _e_kbd_int_layout_state_update(ki);
   _e_kbd_int_recenter(ki);
}

static void
_e_kbd_int_layout_next(E_Kbd_Int *ki)
{
   Eina_List *l, *ln = NULL;
   E_Kbd_Int_Layout *kil;

   EINA_LIST_FOREACH(ki->layouts, l, kil)
     {
        if (!strcmp(kil->path, ki->layout.file))
          {
             ln = l->next;
             break;
          }
     }
   if (!ln) ln = ki->layouts;
   if (!ln) return;
   kil = ln->data;
   _e_kbd_int_layout_select(ki, kil);
}
/*
static void
_e_kbd_int_cb_layouts(void *data, Evas_Object *obj EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   E_Kbd_Int *ki;

   ki = data;
}
*/
EAPI E_Kbd_Int *
e_kbd_int_new(int zone_num, const char *zone_id, const char *themedir, const char *syskbds, const char *sysdicts)
{
   E_Kbd_Int *ki;
   Evas_Object *o;
   E_Kbd_Int_Layout *kil;
   E_Zone *zone = NULL;

   ki = E_NEW(E_Kbd_Int, 1);
   if (!ki) return NULL;
   if (themedir) ki->themedir = eina_stringshare_add(themedir);
   if (syskbds) ki->syskbds = eina_stringshare_add(syskbds);
   if (sysdicts) ki->sysdicts = eina_stringshare_add(sysdicts);
   if (zone_id) zone = e_zone_for_id_get(zone_id);
   if (!zone) zone = e_comp_zone_id_get(zone_num);
   if (!zone) zone = e_zone_current_get();
   ki->zone = zone;

   ki->base_obj = _theme_obj_new(e_comp->evas, ki->themedir,
                                 "e/modules/kbd/base/default");
//   edje_object_signal_callback_add(ki->base_obj, "e,action,do,layouts", "",
//                                   _e_kbd_int_cb_layouts, ki);

   o = evas_object_grid_add(e_comp->evas);
   edje_object_part_swallow(ki->base_obj, "e.swallow.content", o);
   ki->layout_obj = o;

   o = elm_grid_add(e_comp->elm);
   elm_grid_size_set(o, 10, 10);
   edje_object_part_swallow(ki->base_obj, "e.swallow.completion", o);
   ki->boxgrid_obj = o;

   o = elm_box_add(e_comp->elm);
   elm_box_horizontal_set(o, EINA_TRUE);
   elm_box_align_set(o, 0.5, 0.5);
   elm_box_homogeneous_set(o, EINA_FALSE);
   elm_grid_pack(ki->boxgrid_obj, o, 0, 0, 10, 10);
   evas_object_show(o);
   ki->box_obj = o;

   if (il_kbd_cfg->dict)
     ki->kbuf = e_kbd_buf_new(ki->sysdicts, il_kbd_cfg->dict);
   else
     ki->kbuf = e_kbd_buf_new(ki->sysdicts, "English_US.dic");

   _e_kbd_int_layouts_list_update(ki);

   kil = _e_kbd_int_layouts_list_default_get(ki);
   if ((!kil) && (ki->layouts)) kil = ki->layouts->data;

   if (kil) _e_kbd_int_layout_select(ki, kil);

   _e_kbd_int_recenter(ki);

   evas_object_layer_set(ki->base_obj, E_LAYER_DESKLOCK + 1);
   return ki;
}

EAPI void
e_kbd_int_free(E_Kbd_Int *ki)
{
   e_kbd_int_hide(ki);
   if (ki->themedir) eina_stringshare_del(ki->themedir);
   if (ki->syskbds) eina_stringshare_del(ki->syskbds);
   if (ki->sysdicts) eina_stringshare_del(ki->sysdicts);
   _e_kbd_int_layouts_free(ki);
   _e_kbd_int_matches_free(ki);
   _e_kbd_int_layout_free(ki);
   e_kbd_buf_free(ki->kbuf);
   evas_object_del(ki->layout_obj);
   evas_object_del(ki->event_obj);
   evas_object_del(ki->box_obj);
   evas_object_del(ki->boxgrid_obj);
   evas_object_del(ki->base_obj);
   E_FREE(ki);
}

EAPI void
e_kbd_int_update(E_Kbd_Int *ki)
{
   Evas_Coord mw, mh;

   if (e_kbd_buf_string_matches_get(ki->kbuf))
     {
        evas_object_size_hint_min_get(ki->box_obj, &mw, &mh);
        if (mh < (10 * e_scale * il_kbd_cfg->size))
          mh = 10 * e_scale * il_kbd_cfg->size;
        evas_object_size_hint_min_set(ki->boxgrid_obj, 0, mh);
        evas_smart_objects_calculate(evas_object_evas_get(ki->boxgrid_obj));
        edje_object_part_swallow(ki->base_obj, "e.swallow.completion",
                                 ki->boxgrid_obj);
     }
   mw = ki->layout.w * e_scale * il_kbd_cfg->size;
   mh = ki->layout.h * e_scale * il_kbd_cfg->size;
   if (mw > ki->zone->w)
     {
        mh = (mw * mh) / ki->zone->w;
        mw = ki->zone->w;
     }
   evas_object_size_hint_min_set(ki->layout_obj, mw, mh);
   edje_object_part_swallow(ki->base_obj, "e.swallow.content", ki->layout_obj);
   _e_kbd_int_recenter(ki);
}

EAPI void
e_kbd_int_show(E_Kbd_Int *ki)
{
   if (ki->visible) return;
   ki->visible = EINA_TRUE;
   evas_object_show(ki->base_obj);
   edje_object_signal_emit(ki->base_obj, "e,state,visible", "e");
}

EAPI void
e_kbd_int_hide(E_Kbd_Int *ki)
{
   if (!ki->visible) return;
   ki->visible = EINA_FALSE;
   edje_object_signal_emit(ki->base_obj, "e,state,invisible", "e");
   // XXX: hide once anim is done...
}
