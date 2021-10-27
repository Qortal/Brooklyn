#include "e.h"

static void _e_resize_begin(void *data, E_Client *ec);
static void _e_resize_update(void *data, E_Client *ec);
static void _e_resize_end(void *data, E_Client *ec);
static void _e_move_begin(void *data, E_Client *ec);
static void _e_move_update(void *data, E_Client *ec);
static void _e_move_end(void *data, E_Client *ec);

static Evas_Object *_disp_obj = NULL;
static Evas_Object *_disp_content = NULL;
static Eina_List *hooks = NULL;

static Eina_Bool _e_moveresize_enabled = EINA_TRUE;

EINTERN int
e_moveresize_init(void)
{
   E_Client_Hook *h;

   h = e_client_hook_add(E_CLIENT_HOOK_RESIZE_BEGIN, _e_resize_begin, NULL);
   if (h) hooks = eina_list_append(hooks, h);
   h = e_client_hook_add(E_CLIENT_HOOK_RESIZE_UPDATE, _e_resize_update, NULL);
   if (h) hooks = eina_list_append(hooks, h);
   h = e_client_hook_add(E_CLIENT_HOOK_RESIZE_END, _e_resize_end, NULL);
   if (h) hooks = eina_list_append(hooks, h);
   h = e_client_hook_add(E_CLIENT_HOOK_MOVE_BEGIN, _e_move_begin, NULL);
   if (h) hooks = eina_list_append(hooks, h);
   h = e_client_hook_add(E_CLIENT_HOOK_MOVE_UPDATE, _e_move_update, NULL);
   if (h) hooks = eina_list_append(hooks, h);
   h = e_client_hook_add(E_CLIENT_HOOK_MOVE_END, _e_move_end, NULL);
   if (h) hooks = eina_list_append(hooks, h);

   return 1;
}

EINTERN int
e_moveresize_shutdown(void)
{
   E_Client_Hook *h;

   EINA_LIST_FREE(hooks, h)
     e_client_hook_del(h);

   _e_moveresize_enabled = EINA_TRUE;

   return 1;
}

E_API void
e_moveresize_replace(Eina_Bool enable)
{
   _e_moveresize_enabled = !enable;
}

E_API void
e_moveresize_client_extents(const E_Client *ec, int *w, int *h)
{
   int l, r, t, b, bw, bh;

   e_comp_object_frame_geometry_get(ec->frame, &l, &r, &t, &b);
   bw = ec->icccm.base_w;
   bh = ec->icccm.base_h;
   if (e_comp_object_frame_allowed(ec->frame))
     *w = ec->client.w, *h = ec->client.h;
   else
     {
        *w = ec->w;
        *h = ec->h;
        bw -= -l + -r;
        bh -= -t + -b;
     }
   if ((bw >= 0) &&
       (bh >= 0))
     {
        if (ec->icccm.step_w > 0)
          *w = (*w - bw) / ec->icccm.step_w;
        if (ec->icccm.step_h > 0)
          *h = (*h - bh) / ec->icccm.step_h;
     }
}

static void
_e_resize_begin(void *data EINA_UNUSED, E_Client *ec)
{
   Evas_Object *o;
   Evas_Coord ew, eh;
   char buf[40];
   int w, h;

   if (_disp_obj) evas_object_hide(_disp_obj);
   E_FREE_FUNC(_disp_obj, evas_object_del);

   if ((!e_config->resize_info_visible) || (!_e_moveresize_enabled))
     return;

   e_moveresize_client_extents(ec, &w, &h);

   _disp_content = o = edje_object_add(e_comp->evas);
   evas_object_name_set(o, "resizeinfo->_disp_content");
   e_theme_edje_object_set(o, "base/theme/borders",
                           "e/widgets/border/default/resize");
   snprintf(buf, sizeof(buf), "9999x9999");
   edje_object_part_text_set(o, "e.text.label", buf);

   edje_object_size_min_calc(o, &ew, &eh);
   snprintf(buf, sizeof(buf), _("%i×%i"), w, h);
   edje_object_part_text_set(o, "e.text.label", buf);

   _disp_obj = e_comp_object_util_add(o, E_COMP_OBJECT_TYPE_POPUP);
   evas_object_name_set(o, "resizeinfo->_disp_obj");
   evas_object_layer_set(_disp_obj, E_LAYER_POPUP);
   evas_object_pass_events_set(_disp_obj, 1);

   evas_object_resize(_disp_obj, ew, eh);
   if (e_config->resize_info_follows)
     e_comp_object_util_center_on(_disp_obj, ec->frame);
   else
     e_comp_object_util_center(_disp_obj);
   evas_object_show(_disp_obj);
}

static void
_e_resize_end(void *data EINA_UNUSED, E_Client *ec EINA_UNUSED)
{
   if (e_config->resize_info_visible)
     {
        if (_disp_obj)
          {
             evas_object_hide(_disp_obj);
             evas_object_del(_disp_obj);
             _disp_obj = NULL;
             _disp_content = NULL;
          }
     }

}

static void
_e_resize_update(void *data EINA_UNUSED, E_Client *ec)
{
   char buf[40];
   int w, h;

   if (!_disp_obj) return;

   if (e_config->resize_info_follows)
     e_comp_object_util_center_on(_disp_obj, ec->frame);

   e_moveresize_client_extents(ec, &w, &h);

   snprintf(buf, sizeof(buf), _("%i×%i"), w, h);
   edje_object_part_text_set(_disp_content, "e.text.label", buf);
}

static void
_e_move_begin(void *data EINA_UNUSED, E_Client *ec)
{
   Evas_Object *o;
   Evas_Coord ew, eh;
   char buf[40];

   if (_disp_obj)
     {
        evas_object_hide(_disp_obj);
        evas_object_del(_disp_obj);
     }
   _disp_obj = NULL;

   if ((!e_config->move_info_visible) || (!_e_moveresize_enabled))
     return;

   _disp_content = o = edje_object_add(e_comp->evas);
   evas_object_name_set(o, "moveinfo->_disp_content");
   e_theme_edje_object_set(o, "base/theme/borders",
                           "e/widgets/border/default/move");
   snprintf(buf, sizeof(buf), "9999 9999");
   edje_object_part_text_set(o, "e.text.label", buf);

   edje_object_size_min_calc(o, &ew, &eh);
   snprintf(buf, sizeof(buf), "%i %i", ec->x, ec->y);
   edje_object_part_text_set(o, "e.text.label", buf);

   _disp_obj = e_comp_object_util_add(o, E_COMP_OBJECT_TYPE_POPUP);
   evas_object_name_set(o, "moveinfo->_disp_obj");
   evas_object_layer_set(_disp_obj, E_LAYER_POPUP);
   evas_object_pass_events_set(_disp_obj, 1);

   evas_object_resize(_disp_obj, ew, eh);
   if (e_config->move_info_follows)
     e_comp_object_util_center_on(_disp_obj, ec->frame);
   else
     e_comp_object_util_center(_disp_obj);
   evas_object_show(_disp_obj);
}

static void
_e_move_end(void *data EINA_UNUSED, E_Client *ec EINA_UNUSED)
{
   if (e_config->move_info_visible)
     {
        if (_disp_obj)
          {
             evas_object_hide(_disp_obj);
             E_FREE_FUNC(_disp_obj, evas_object_del);
             _disp_content = NULL;
          }
     }
}

static void
_e_move_update(void *data EINA_UNUSED, E_Client *ec)
{
   char buf[40];

   if (!_disp_obj) return;

   if (e_config->move_info_follows)
     e_comp_object_util_center_on(_disp_obj, ec->frame);

   snprintf(buf, sizeof(buf), "%i %i", ec->x, ec->y);
   edje_object_part_text_set(_disp_content, "e.text.label", buf);
}
