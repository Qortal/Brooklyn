#include "e.h"
#ifdef HAVE_WAYLAND
# include "e_comp_wl.h"
#endif

#define OVER_FLOW 1
//#define BORDER_ZOOMAPS
//////////////////////////////////////////////////////////////////////////
//
// TODO (no specific order):
//   1. abstract evas object and compwin so we can duplicate the object N times
//      in N canvases - for winlist, everything, pager etc. too
//   2. implement "unmapped composite cache" -> N pixels worth of unmapped
//      windows to be fully composited. only the most active/recent.
//   3. for unmapped windows - when window goes out of unmapped comp cache
//      make a miniature copy (1/4 width+height?) and set property on window
//      with pixmap id
//
//////////////////////////////////////////////////////////////////////////

static Eina_List *handlers = NULL;
static Eina_List *hooks = NULL;
E_API E_Comp *e_comp = NULL;
E_API E_Comp_X_Data *e_comp_x = NULL;
E_API E_Comp_Wl_Data *e_comp_wl = NULL;
static Eina_Hash *ignores = NULL;
static Eina_List *actions = NULL;

static E_Comp_Config *conf = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_match_edd = NULL;

static Ecore_Timer *action_timeout = NULL;
static Eina_Bool gl_avail = EINA_FALSE;
static Eina_Bool shape_debug = EINA_FALSE;

static int _e_comp_log_dom = -1;

E_API int E_EVENT_COMPOSITOR_UPDATE = -1;
E_API int E_EVENT_COMPOSITOR_DISABLE = -1;
E_API int E_EVENT_COMPOSITOR_ENABLE = -1;
E_API int E_EVENT_COMPOSITOR_XWAYLAND_INIT = -1;

//////////////////////////////////////////////////////////////////////////
#undef DBG
#undef INF
#undef WRN
#undef ERR
#undef CRI

#define SHAPE_DBG(...) do { if (shape_debug) EINA_LOG_DOM_DBG(_e_comp_log_dom, __VA_ARGS__); } while (0)
#define SHAPE_INF(...) do { if (shape_debug) EINA_LOG_DOM_INFO(_e_comp_log_dom, __VA_ARGS__); } while (0)
#define SHAPE_WRN(...) do { if (shape_debug) EINA_LOG_DOM_WARN(_e_comp_log_dom, __VA_ARGS__); } while (0)
#define SHAPE_ERR(...) do { if (shape_debug) EINA_LOG_DOM_ERR(_e_comp_log_dom, __VA_ARGS__); } while (0)
#define SHAPE_CRI(...) do { if (shape_debug) EINA_LOG_DOM_CRIT(_e_comp_log_dom, __VA_ARGS__); } while (0)

#define DBG(...)            EINA_LOG_DOM_DBG(_e_comp_log_dom, __VA_ARGS__)
#define INF(...)            EINA_LOG_DOM_INFO(_e_comp_log_dom, __VA_ARGS__)
#define WRN(...)            EINA_LOG_DOM_WARN(_e_comp_log_dom, __VA_ARGS__)
#define ERR(...)            EINA_LOG_DOM_ERR(_e_comp_log_dom, __VA_ARGS__)
#define CRI(...)            EINA_LOG_DOM_CRIT(_e_comp_log_dom, __VA_ARGS__)

static Eina_Bool
_e_comp_visible_object_clip_is(Evas_Object *obj)
{
   Evas_Object *clip;
   int a;

   clip = evas_object_clip_get(obj);
   if (!evas_object_visible_get(clip)) return EINA_FALSE;
   evas_object_color_get(clip, NULL, NULL, NULL, &a);
   if (a <= 0) return EINA_FALSE;
   if (evas_object_clip_get(clip))
     return _e_comp_visible_object_clip_is(clip);
   return EINA_TRUE;
}

static Eina_Bool
_e_comp_visible_object_is(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   const char *type = evas_object_type_get(obj);
   Evas_Coord xx, yy, ww, hh;

   if ((!type) || (!e_util_strcmp(type, "e_comp_object"))) return EINA_FALSE;
   if (evas_object_data_get(obj, "comp_skip")) return EINA_FALSE;
   evas_object_geometry_get(obj, &xx, &yy, &ww, &hh);
   if (E_INTERSECTS(x, y, w, h, xx, yy, ww, hh))
     {
        if ((evas_object_visible_get(obj))
            && (!evas_object_clipees_get(obj))
           )
          {
             int a;

             evas_object_color_get(obj, NULL, NULL, NULL, &a);
             if (a > 0)
               {
                  if ((!strcmp(type, "rectangle")) ||
                      (!strcmp(type, "image")) ||
                      (!strcmp(type, "text")) ||
                      (!strcmp(type, "textblock")) ||
                      (!strcmp(type, "textgrid")) ||
                      (!strcmp(type, "polygon")) ||
                      (!strcmp(type, "line")))
                    {
                       if (evas_object_clip_get(obj))
                         return _e_comp_visible_object_clip_is(obj);
                       return EINA_TRUE;
                    }
                  else
                    {
                       Eina_List *children;

                       if ((children = evas_object_smart_members_get(obj)))
                         {
                            Eina_List *l;
                            Evas_Object *o;

                            EINA_LIST_FOREACH(children, l, o)
                              {
                                 if (_e_comp_visible_object_is(o, x, y, w, h))
                                   {
                                      if (evas_object_clip_get(o))
                                        {
                                           eina_list_free(children);
                                           return _e_comp_visible_object_clip_is(o);
                                        }
                                      eina_list_free(children);
                                      return !!evas_object_data_get(o, "comp_skip");
                                   }
                              }
                            eina_list_free(children);
                         }
                    }
               }
          }
     }
   return EINA_FALSE;
}

static Eina_Bool
_e_comp_visible_object_is_above(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Object *above;

   for (above = evas_object_above_get(obj); above;
        above = evas_object_above_get(above))
     {
        if (_e_comp_visible_object_is(above, x, y, w, h)) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static E_Client *
_e_comp_fullscreen_check(void)
{
   E_Client *ec;

//   printf("------------------------\n");
   E_CLIENT_REVERSE_FOREACH(ec)
     {
        Evas_Object *o = ec->frame;

//        printf("FSCK %p [%i %i %ix%i] [%s]\n",
//               ec, ec->x, ec->y, ec->w, ec->h,
//               e_client_util_name_get(ec));
        if (ec->ignored || ec->input_only || (!evas_object_visible_get(ec->frame)))
          continue;
        if (!e_comp_util_client_is_fullscreen(ec))
          {
             if (evas_object_data_get(ec->frame, "comp_skip")) continue;
             return NULL;
          }
        while (o)
          {
             if (_e_comp_visible_object_is_above
                 (o, 0, 0, e_comp->w, e_comp->h)) return NULL;
             o = evas_object_smart_parent_get(o);
          }
        return ec;
     }
   return NULL;
}

static void
_e_comp_fps_update(void)
{
   if (conf->fps_show)
     {
        if (e_comp->canvas->fps_bg) return;

        e_comp->canvas->fps_bg = evas_object_rectangle_add(e_comp->evas);
        evas_object_color_set(e_comp->canvas->fps_bg, 0, 0, 0, 192);
        evas_object_layer_set(e_comp->canvas->fps_bg, E_LAYER_MAX);
        evas_object_name_set(e_comp->canvas->fps_bg, "e_comp->canvas->fps_bg");
        evas_object_lower(e_comp->canvas->fps_bg);
        evas_object_pass_events_set(e_comp->canvas->fps_bg, EINA_TRUE);
        evas_object_show(e_comp->canvas->fps_bg);

        e_comp->canvas->fps_fg = evas_object_text_add(e_comp->evas);
        evas_object_text_font_set(e_comp->canvas->fps_fg, "Sans", 10);
        evas_object_text_text_set(e_comp->canvas->fps_fg, "???");
        evas_object_color_set(e_comp->canvas->fps_fg, 255, 255, 255, 255);
        evas_object_layer_set(e_comp->canvas->fps_fg, E_LAYER_MAX);
        evas_object_name_set(e_comp->canvas->fps_fg, "e_comp->canvas->fps_fg");
        evas_object_stack_above(e_comp->canvas->fps_fg, e_comp->canvas->fps_bg);
        evas_object_pass_events_set(e_comp->canvas->fps_fg, EINA_TRUE);
        evas_object_show(e_comp->canvas->fps_fg);

        e_comp->canvas->fps_gr = evas_object_image_filled_add(e_comp->evas);
        evas_object_image_smooth_scale_set(e_comp->canvas->fps_gr, EINA_FALSE);
        evas_object_image_alpha_set(e_comp->canvas->fps_gr, EINA_TRUE);
        evas_object_image_size_set(e_comp->canvas->fps_gr, 1, 1);
        evas_object_color_set(e_comp->canvas->fps_gr, 255, 255, 255, 255);
        evas_object_layer_set(e_comp->canvas->fps_gr, E_LAYER_MAX);
        evas_object_name_set(e_comp->canvas->fps_gr, "e_comp->canvas->fps_gr");
        evas_object_stack_above(e_comp->canvas->fps_gr, e_comp->canvas->fps_fg);
        evas_object_pass_events_set(e_comp->canvas->fps_gr, EINA_TRUE);
        evas_object_show(e_comp->canvas->fps_gr);
     }
   else
     {
        E_FREE_FUNC(e_comp->canvas->fps_fg, evas_object_del);
        E_FREE_FUNC(e_comp->canvas->fps_bg, evas_object_del);
        E_FREE_FUNC(e_comp->canvas->fps_gr, evas_object_del);
     }
}

static void
_e_comp_cb_nocomp_begin(void)
{
   E_Client *ec, *ecf;

   if (e_comp->nocomp) return;

   E_FREE_FUNC(e_comp->nocomp_delay_timer, ecore_timer_del);

   ecf = _e_comp_fullscreen_check();
   if (!ecf) return;
   e_object_ref(E_OBJECT(ecf));
   e_comp->nocomp_ec = ecf;
   E_CLIENT_FOREACH(ec)
     if (ec != ecf) e_client_redirected_set(ec, 0);

   INF("NOCOMP %p: frame %p", ecf, ecf->frame);
   e_comp->nocomp = 1;

   {
      Eina_Bool fs;

      fs = e_comp->nocomp_ec->fullscreen;
      if (!fs)
        e_comp->nocomp_ec->saved.layer = e_comp->nocomp_ec->layer;
      e_comp->nocomp_ec->fullscreen = 0;
      e_comp->nocomp_ec->layer = E_LAYER_CLIENT_PRIO;
      evas_object_layer_set(e_comp->nocomp_ec->frame, E_LAYER_CLIENT_PRIO);
      e_comp->nocomp_ec->fullscreen = fs;
   }
   e_client_redirected_set(ecf, 0);

   //ecore_evas_manual_render_set(e_comp->ee, EINA_TRUE);
   ecore_evas_hide(e_comp->ee);
   edje_file_cache_flush();
   edje_collection_cache_flush();
   evas_image_cache_flush(e_comp->evas);
   evas_font_cache_flush(e_comp->evas);
   evas_render_dump(e_comp->evas);

   DBG("JOB2...");
   e_comp_render_queue();
   e_comp_shape_queue_block(1);
   ecore_event_add(E_EVENT_COMPOSITOR_DISABLE, NULL, NULL, NULL);
   printf("COMP: suspend\n");
}

static void
_e_comp_cb_nocomp_end(void)
{
   E_Client *ec;

   if (!e_comp->nocomp) return;

   printf("COMP: resume\n");
   INF("COMP RESUME!");
   //ecore_evas_manual_render_set(e_comp->ee, EINA_FALSE);
   ecore_evas_show(e_comp->ee);
   E_CLIENT_FOREACH(ec)
     {
        e_client_redirected_set(ec, 1);
        if (ec->visible && (!ec->input_only))
          e_comp_object_damage(ec->frame, 0, 0, ec->w, ec->h);
     }
#ifndef HAVE_WAYLAND_ONLY
   e_comp_x_nocomp_end();
#endif
   e_comp_render_queue();
   e_comp_shape_queue_block(0);
   ecore_event_add(E_EVENT_COMPOSITOR_ENABLE, NULL, NULL, NULL);
}

static Eina_Bool
_e_comp_cb_nocomp_begin_timeout(void *data EINA_UNUSED)
{
   e_comp->nocomp_delay_timer = NULL;
   if (e_comp->nocomp_override == 0)
     {
        if (_e_comp_fullscreen_check()) e_comp->nocomp_want = 1;
        _e_comp_cb_nocomp_begin();
     }
   return EINA_FALSE;
}


static void
_e_comp_client_update(E_Client *ec)
{
   int pw, ph;

   DBG("UPDATE [%p] pm = %p", ec, ec->pixmap);
   if (e_object_is_del(E_OBJECT(ec))) return;

   e_pixmap_size_get(ec->pixmap, &pw, &ph);

   if (e_pixmap_dirty_get(ec->pixmap) && (!e_comp->nocomp))
     {
        int w, h;

        if (e_pixmap_refresh(ec->pixmap) &&
            e_pixmap_size_get(ec->pixmap, &w, &h) &&
            e_pixmap_size_changed(ec->pixmap, pw, ph))
          {
             e_pixmap_image_clear(ec->pixmap, 0);
             e_comp_object_render_update_del(ec->frame); //clear update
             if (ec->changes.visible && (!evas_object_visible_get(ec->frame)))
               evas_object_show(ec->frame);
          }
        else if (!e_pixmap_size_get(ec->pixmap, NULL, NULL))
          {
             WRN("FAIL %p", ec);
             e_comp_object_redirected_set(ec->frame, 0);
             if (e_pixmap_failures_get(ec->pixmap) > 3)
               e_comp_object_render_update_add(ec->frame);
          }
     }
   if (!(e_comp->saver && ecore_evas_manual_render_get(e_comp->ee)) &&
       e_pixmap_size_get(ec->pixmap, &pw, &ph))
     {
        //INF("PX DIRTY: PX(%dx%d) CLI(%dx%d)", pw, ph, ec->client.w, ec->client.h);
        e_pixmap_image_refresh(ec->pixmap);
        e_comp_object_dirty(ec->frame);
        if (e_pixmap_is_x(ec->pixmap) && (!ec->override))
          evas_object_resize(ec->frame, ec->w, ec->h);
     }
}

static void
_e_comp_nocomp_end(void)
{
   e_comp->nocomp_want = 0;
   E_FREE_FUNC(e_comp->nocomp_delay_timer, ecore_timer_del);
   _e_comp_cb_nocomp_end();
   if (e_comp->nocomp_ec)
     {
        E_Layer layer = MAX(e_comp->nocomp_ec->saved.layer, E_LAYER_CLIENT_NORMAL);
        Eina_Bool fs;

        fs = e_comp->nocomp_ec->fullscreen;
        e_comp->nocomp_ec->fullscreen = 0;
        if (fs)
          {
             if (!e_config->allow_above_fullscreen)
               layer = E_LAYER_CLIENT_FULLSCREEN;
          }
        evas_object_layer_set(e_comp->nocomp_ec->frame, layer);
        e_comp->nocomp_ec->fullscreen = fs;
     }
   E_FREE_FUNC(e_comp->nocomp_ec, e_object_unref);
}

static double
_e_comp_frame_event_fps_calc(int info)
{
   int i, count = 0, iind, start, end, end2;
   Eina_Bool first = EINA_FALSE;
   double t0 = 0.0, dt;

   start = e_comp->frame_event_now - 1 + E_COMP_FRAME_EVENT_COUNT;
   end = e_comp->frame_event_now + E_COMP_FRAME_EVENT_COUNT ;
   end2 = e_comp->frame_event_now + E_COMP_FRAME_EVENT_COUNT + 1;
   for (i = start;;)
     {
        iind = i % E_COMP_FRAME_EVENT_COUNT;
        i--;
        if (iind == (end % E_COMP_FRAME_EVENT_COUNT)) break;
        if (e_comp->frame_events[iind].info[0] != info) continue;
        count++;
        if (!first)
          {
             t0 =  e_comp->frame_events[iind].t;
             first = EINA_TRUE;
          }
        else
          {
             dt = t0 - e_comp->frame_events[iind].t;
             if ((dt > 0.5) || (iind == (end2 % E_COMP_FRAME_EVENT_COUNT)))
               {
                  if (dt > 0.0) return ((double)(count - 1)) / dt;
               }
          }
     }
   return 0.0;
}

E_API void
e_comp_frame_event_add(int info[4], double t)
{
   e_comp->frame_events[e_comp->frame_event_now].info[0] = info[0];
   e_comp->frame_events[e_comp->frame_event_now].info[1] = info[1];
   e_comp->frame_events[e_comp->frame_event_now].info[2] = info[2];
   e_comp->frame_events[e_comp->frame_event_now].info[3] = info[3];
   e_comp->frame_events[e_comp->frame_event_now].t = t;
   e_comp->frame_event_now++;
   if (e_comp->frame_event_now >= E_COMP_FRAME_EVENT_COUNT)
     e_comp->frame_event_now = 0;
}

E_API void
e_comp_client_frame_add(Evas_Object *obj EINA_UNUSED)
{
   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_CLIENT_DAMAGE, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_time_get());
     }
}

static inline void
_e_comp_fps_draw_point(unsigned int *pix, int stride, int w, int y, int col, int x)
{
   unsigned int *pixrow = pix + (y * (stride / 4));
   // out of bounds abort draw
   if (x <  0) return;
   if (x >= w) return;
   pixrow[x] = col;
}

/* for future
static inline void
_e_comp_fps_draw_span(unsigned int *pix, int stride, int w, int y, int col, int x1, int x2)
{
   unsigned int *pixrow = pix + (y * (stride / 4));
   int x;

   if (x2 < x1) // swap so x2 >= x1
     {
        x = x2;
        x2 = x1;
        x1 = x;
     }
   // out of bounds abort draw
   if (x2 < 0) return;
   if (x1 >= w) return;
   // clip to span area
   if (x1 < 0) x1 = 0;
   if (x2 >= w) x2 = w - 1;
   for (x = x1; x <= x2; x++)
     pixrow[x] = col;
}
*/

E_API void
e_comp_fps_update(void)
{
   char buf[128];
   double fps, comp_fps;
   Evas_Coord x = 0, y = 0, w = 0, h = 0;
   Evas_Coord gx = 0, gy = 0, gw = 0, gh = 0;
   Evas_Coord bx = 0, by = 0, bw = 0, bh = 0;
   E_Zone *z;

   e_comp->frameskip++;
   if (e_comp->frameskip >= 20)
     {
        unsigned int *pix;
        int i, pixw, pixh, pixsz, pixstride, px, pixscale, info0, start, end, iind;
        double t;

        t = ecore_time_get();
        e_comp->frameskip = 0;

        pixh = 12;
        pixsz = 2;

        fps = _e_comp_frame_event_fps_calc(E_COMP_FRAME_EVENT_HANDLE_DAMAGE);
        comp_fps = _e_comp_frame_event_fps_calc(E_COMP_FRAME_EVENT_RENDER_END);

        snprintf(buf, sizeof(buf), "FPS: (in) %1.1f (out) %1.1f", fps, comp_fps);
        evas_object_text_text_set(e_comp->canvas->fps_fg, buf);

        evas_object_geometry_get(e_comp->canvas->fps_fg, NULL, NULL, &w, &h);
        w += 16;
        h += 16;
        z = e_zone_current_get();
        pixscale = 1000;
        if (z)
          {
             if (z->w >= 2048) pixw = 2000;
             else if (z->w >= 1600) pixw = 1500;
             else if (z->w >= 1024) pixw = 1000;
             else if (z->w >= 512) pixw = 500;
             else if (z->w >= 256) pixw = 250;
             else pixw = 125;

             switch (conf->fps_corner)
               {
                case 3: // bottom-right
                  x = z->x + z->w - w;
                  y = z->y + z->h - h;

                  gw = pixw;
                  gh = pixh * pixsz;
                  gx = x + w - gw;
                  gy = y - gh - 8;

                  bw = (gw > bw) ? gw : bw;
                  bh = h + gh + 16;
                  bx = (x > gx) ? gx: x;
                  by = gy - 8;
                  break;
                case 2: // bottom-left
                  x = z->x;
                  y = z->y + z->h - h;

                  gw = pixw;
                  gh = pixh * pixsz;
                  gx = x;
                  gy = y - gh - 8;

                  bw = (gw > bw) ? gw : bw;
                  bh = h + gh + 16;
                  bx = x;
                  by = gy - 8;
                  break;
                case 1: // top-right
                  x = z->x + z->w - w;
                  y = z->y;

                  gw = pixw;
                  gh = pixh * pixsz;
                  gx = x + w - gw;
                  gy = y + h;

                  bw = (gw > bw) ? gw : bw;
                  bh = h + gh + 16;
                  bx = (x > gx) ? gx: x;
                  by = y;
                  break;
                case 0: // top-left
                default:
                  x = z->x;
                  y = z->y;

                  gw = pixw;
                  gh = pixh * pixsz;
                  gx = x;
                  gy = y + h;

                  bw = (gw > bw) ? gw : bw;
                  bh = h + gh + 16;
                  bx = x;
                  by = y;
                  break;
               }
          }
        else pixw = 500;

        evas_object_image_size_set(e_comp->canvas->fps_gr, pixw, pixh);
        evas_object_image_alpha_set(e_comp->canvas->fps_gr, EINA_TRUE);
        pixstride = evas_object_image_stride_get(e_comp->canvas->fps_gr);
        pix = evas_object_image_data_get(e_comp->canvas->fps_gr, EINA_TRUE);
        if (pix)
          {
             memset(pix, 0, pixstride * pixh);

             // go backwards from newest to oldest
             start = e_comp->frame_event_now + E_COMP_FRAME_EVENT_COUNT - 1;
             end = e_comp->frame_event_now + E_COMP_FRAME_EVENT_COUNT ;
             for (i = start;;)
               {
                  iind = i % E_COMP_FRAME_EVENT_COUNT;
                  i--;
                  if (iind == (end % E_COMP_FRAME_EVENT_COUNT)) break;

                  info0 = e_comp->frame_events[iind].info[0];
                  px = (t - e_comp->frame_events[iind].t) * pixscale;
                  px = pixw - px - 1;
                  if (px < 0) break;
                  if (info0 == E_COMP_FRAME_EVENT_RENDER2_END)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 0, 0xffffffff, px);
                  else if (info0 == E_COMP_FRAME_EVENT_RENDER2_BEGIN)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 1, 0xffffee88, px);
                  else if (info0 == E_COMP_FRAME_EVENT_RENDER_END)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 2, 0xffff9944, px);
                  else if (info0 == E_COMP_FRAME_EVENT_RENDER_BEGIN)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 3, 0xffff4433, px);
                  else if (info0 == E_COMP_FRAME_EVENT_IDLE_ENTER)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 4, 0xffff88ff, px);
                  else if (info0 == E_COMP_FRAME_EVENT_HANDLE_DAMAGE)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 5, 0xff44ff22, px);
                  else if (info0 == E_COMP_FRAME_EVENT_CLIENT_DAMAGE)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 6, 0xff4466ff, px);
                  else if (info0 == E_COMP_FRAME_EVENT_IDLE_EXIT)
                    _e_comp_fps_draw_point(pix, pixstride, pixw, 7, 0xff994499, px);
               }
             for (t = 0.0; t < 10.0; t += (1.0 / 60.0))
               {
                  px = t * pixscale;
                  px = pixw - px - 1;
                  if (px < 0) break;
                  _e_comp_fps_draw_point(pix, pixstride, pixw, 10, 0xffffffff, px);
                  _e_comp_fps_draw_point(pix, pixstride, pixw, 11, 0xffffffff, px - 1);
                  _e_comp_fps_draw_point(pix, pixstride, pixw, 11, 0xffffffff, px);
                  _e_comp_fps_draw_point(pix, pixstride, pixw, 11, 0xffffffff, px + 1);
               }
             evas_object_image_data_set(e_comp->canvas->fps_gr, pix);
             evas_object_image_data_update_add(e_comp->canvas->fps_gr,
                                               0, 0, pixw, pixh);
          }

        evas_object_color_set(e_comp->canvas->fps_bg, 0, 0, 0, 192);
        evas_object_geometry_set(e_comp->canvas->fps_bg, bx, by, bw, bh);
        evas_object_geometry_set(e_comp->canvas->fps_gr, gx, gy, gw, gh);
        evas_object_move(e_comp->canvas->fps_fg, x + 8, y + 8);
     }
}

static Eina_Bool
_e_comp_cb_update(void)
{
   E_Client *ec;
   Eina_List *l;
   //   static int doframeinfo = -1;

   if (!e_comp) return EINA_FALSE;
   DBG("UPDATE ALL");
   if (e_comp->nocomp) goto nocomp;
//   if (conf->grab && (!e_comp->grabbed))
//     {
//        if (e_comp->grab_cb) e_comp->grab_cb();
//        e_comp->grabbed = 1;
//     }
   e_comp->updating = 1;
   l = e_comp->updates;
   e_comp->updates = NULL;
   EINA_LIST_FREE(l, ec)
     {
        /* clear update flag */
        e_comp_object_render_update_del(ec->frame);
        _e_comp_client_update(ec);
     }
   e_comp->updating = 0;
   _e_comp_fps_update();
   if (conf->fps_show)
     {
        int info[4] = { E_COMP_FRAME_EVENT_HANDLE_DAMAGE, 0, 0, 0 };
        e_comp_frame_event_add(info, ecore_loop_time_get());
        e_comp_fps_update();
     }
nocomp:
   ec = _e_comp_fullscreen_check();
   if (ec)
     {
        if (conf->nocomp_fs)
          {
             if (e_comp->nocomp && e_comp->nocomp_ec)
               {
                  E_Client *nec = NULL;
                  for (ec = e_client_top_get(), nec = e_client_below_get(ec);
                       (ec && nec) && (ec != nec); ec = nec, nec = e_client_below_get(ec))
                    {
                       if (ec == e_comp->nocomp_ec) break;
                       if (evas_object_layer_get(ec->frame) < evas_object_layer_get(e_comp->nocomp_ec->frame)) break;
                       if (e_client_is_stacking(ec)) continue;
                       if (!ec->visible) continue;
                       if (evas_object_data_get(ec->frame, "comp_skip")) continue;
                       if (e_object_is_del(E_OBJECT(ec)) || (!e_client_util_desk_visible(ec, e_desk_current_get(ec->zone)))) continue;
                       if (ec->override || (e_config->allow_above_fullscreen))
                         {
                            _e_comp_nocomp_end();
                            break;
                         }
                       else
                         evas_object_stack_below(ec->frame, e_comp->nocomp_ec->frame);
                    }
               }
             else if ((!e_comp->nocomp) && (!e_comp->nocomp_override))
               {
                  if (!e_comp->nocomp_delay_timer)
                    e_comp->nocomp_delay_timer = ecore_timer_loop_add(1.0, _e_comp_cb_nocomp_begin_timeout, NULL);
               }
          }
     }
   else
     _e_comp_nocomp_end();

   return ECORE_CALLBACK_RENEW;
}

static void
_e_comp_cb_job(void *data EINA_UNUSED)
{
   DBG("UPDATE ALL JOB...");
   if (e_comp->update_job) e_comp->update_job = NULL;
   _e_comp_cb_update();
}

//////////////////////////////////////////////////////////////////////////


static void
_e_comp_shape_debug_rect(Eina_Rectangle *rect, E_Color *color)
{
   Evas_Object *o;

#define COLOR_INCREMENT 30
   o = evas_object_rectangle_add(e_comp->evas);
   if (color->r < 256 - COLOR_INCREMENT)
     evas_object_color_set(o, (color->r += COLOR_INCREMENT), 0, 0, 255);
   else if (color->g < 256 - COLOR_INCREMENT)
     evas_object_color_set(o, 0, (color->g += COLOR_INCREMENT), 0, 255);
   else
     evas_object_color_set(o, 0, 0, (color->b += COLOR_INCREMENT), 255);
   evas_object_pass_events_set(o, 1);
   evas_object_layer_set(o, E_LAYER_MENU - 1);
   evas_object_move(o, rect->x, rect->y);
   evas_object_resize(o, rect->w, rect->h);
   e_comp->debug_rects = eina_list_append(e_comp->debug_rects, o);
   evas_object_show(o);
}

static Eina_Bool
_e_comp_shapes_update_object_checker_function_thingy(Evas_Object *o)
{
   Eina_List *l;
   E_Zone *zone;

   if (o == e_comp->canvas->resize_object) return EINA_TRUE;
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        if ((o == zone->over) || (o == zone->base)) return EINA_TRUE;
        if ((o == zone->bg_object) || (o == zone->bg_event_object) ||
            (o == zone->bg_clip_object) || (o == zone->prev_bg_object) ||
            (o == zone->transition_object))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_e_comp_shapes_update_comp_client_shape_comp_helper(E_Client *ec, Eina_Tiler *tb, Eina_List **rl)
{
   int x, y, w, h;

   /* ignore deleted shapes */
   if (e_object_is_del(E_OBJECT(ec)))
     {
        SHAPE_INF("IGNORING DELETED: %p", ec);
        return;
     }
   if ((!ec->visible) || (ec->hidden) || (!evas_object_visible_get(ec->frame)) || evas_object_pass_events_get(ec->frame))
     {
        SHAPE_DBG("SKIPPING SHAPE FOR %p", ec);
        return;
     }

   SHAPE_INF("COMP EC: %p", ec);

   if (ec->shaped || ec->shaped_input)
     {
        int num, tot;
        int l, r, t, b;
        Eina_Rectangle *rect, *rects;

        /* add the frame */
        e_comp_object_frame_geometry_get(ec->frame, &l, &r, &t, &b);
        e_comp_object_frame_extends_get(ec->frame, &x, &y, &w, &h);
        if ((l + x) || (r + (w - ec->w + x)) || (t - y) || (b + (h - ec->h + y)))
          {
             if (t - y)
               {
                  eina_tiler_rect_add(tb, &(Eina_Rectangle){ec->x + x, ec->y + y, w, t - y});
                  SHAPE_INF("ADD: %d,%d@%dx%d", ec->x + x, ec->y + y, w, t - y);
               }
             if (l - x)
               {
                  eina_tiler_rect_add(tb, &(Eina_Rectangle){ec->x + x, ec->y + y, l - x, h});
                  SHAPE_INF("ADD: %d,%d@%dx%d", ec->x + x, ec->y + y, l - x, h);
               }
             if (r + (w - ec->w + x))
               {
                  eina_tiler_rect_add(tb, &(Eina_Rectangle){ec->x + l + ec->client.w + x, ec->y + y, r + (w - ec->w + x), h});
                  SHAPE_INF("ADD: %d,%d@%dx%d", ec->x + l + ec->client.w + x, ec->y + y, r + (w - ec->w + x), h);
               }
             if (b + (h - ec->h + y))
               {
                  eina_tiler_rect_add(tb, &(Eina_Rectangle){ec->x + x, ec->y + t + ec->client.h + y, w, b + (h - ec->h + y)});
                  SHAPE_INF("ADD: %d,%d@%dx%d", ec->x + x, ec->y + t + ec->client.h + y, w, b + (h - ec->h + y));
               }
          }
        rects = ec->shape_rects ?: ec->shape_input_rects;
        tot = ec->shape_rects_num ?: ec->shape_input_rects_num;
        for (num = 0, rect = rects; num < tot; num++, rect++)
          {
             x = rect->x, y = rect->y, w = rect->w, h = rect->h;
             x += ec->client.x, y += ec->client.y;
             E_RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, e_comp->w, e_comp->h);
             if ((w < 1) || (h < 1)) continue;
   //#ifdef SHAPE_DEBUG not sure we can shape check these?
             //r = E_NEW(Eina_Rectangle, 1);
             //EINA_RECTANGLE_SET(r, x, y, w, h);
             //rl = eina_list_append(rl, r);
   //#endif
             eina_tiler_rect_del(tb, &(Eina_Rectangle){x, y, w, h});
             SHAPE_INF("DEL: %d,%d@%dx%d", x, y, w, h);
          }
        return;
     }

   if (shape_debug)
     {
        Eina_Rectangle *r;

        r = E_NEW(Eina_Rectangle, 1);
        EINA_RECTANGLE_SET(r, ec->client.x, ec->client.y, ec->client.w, ec->client.h);
        *rl = eina_list_append(*rl, r);
     }

   if (!e_client_util_borderless(ec))
     {
        e_comp_object_frame_extends_get(ec->frame, &x, &y, &w, &h);
        /* add the frame */
        eina_tiler_rect_add(tb, &(Eina_Rectangle){ec->x + x, ec->y + y, w, h});
        SHAPE_INF("ADD: %d,%d@%dx%d", ec->x + x, ec->y + y, w, h);
     }

   if ((!ec->shaded) && (!ec->shading))
     {
        /* delete the client if not shaded */
        eina_tiler_rect_del(tb, &(Eina_Rectangle){ec->client.x, ec->client.y, ec->client.w, ec->client.h});
        SHAPE_INF("DEL: %d,%d@%dx%d", ec->client.x, ec->client.y, ec->client.w, ec->client.h);
     }
}

static void
_e_comp_shapes_update_object_shape_comp_helper(Evas_Object *o, Eina_Tiler *tb)
{
   int x, y, w, h;

   /* ignore hidden and pass-event objects */
   if ((!evas_object_visible_get(o)) || evas_object_pass_events_get(o)) return;
   if (evas_object_repeat_events_get(o) && (!evas_object_data_get(o, "comp_repeat"))) return;
   /* ignore canvas objects */
   if (_e_comp_shapes_update_object_checker_function_thingy(o)) return;
   SHAPE_INF("OBJ: %p:%s", o, evas_object_name_get(o) ?: evas_object_type_get(o));
   evas_object_geometry_get(o, &x, &y, &w, &h);
   if ((!w) && (!h) && elm_object_widget_check(o))
     {
        Evas_Object *content = elm_object_content_get(o);
        if (content)
          evas_object_geometry_get(content, &x, &y, &w, &h);
     }
   eina_tiler_rect_add(tb, &(Eina_Rectangle){x, y, w, h});
   SHAPE_INF("ADD: %d,%d@%dx%d", x, y, w, h);
}

static void
_e_comp_shapes_update_job(void *d EINA_UNUSED)
{
   Eina_Tiler *tb;
   E_Client *ec;
   Evas_Object *o = NULL;
   Eina_Rectangle *tr;
   Eina_Iterator *ti;
   Eina_Rectangle *exr;
   unsigned int i, tile_count;
   Ecore_Window win;
   Eina_Rectangle *r;
   Eina_List *rl = NULL;
   E_Color color = {0};
   const char *type;

   SHAPE_INF("---------------------");

   if (e_comp->comp_type == E_PIXMAP_TYPE_X)
     win = e_comp->win;
   else
     win = e_comp->cm_selection;
   E_FREE_LIST(e_comp->debug_rects, evas_object_del);
   tb = eina_tiler_new(e_comp->w, e_comp->h);
   eina_tiler_tile_size_set(tb, 1, 1);
   /* background */
   eina_tiler_rect_add(tb, &(Eina_Rectangle){0, 0, e_comp->w, e_comp->h});

   ec = e_client_bottom_get();
   if (ec) o = ec->frame;
   for (; o; o = evas_object_above_get(o))
     {
        int layer;

        layer = evas_object_layer_get(o);
        if (e_comp_canvas_client_layer_map(layer) == 9999) //not a client layer
          {
             _e_comp_shapes_update_object_shape_comp_helper(o, tb);
             continue;
          }
        ec = NULL;
        type = evas_object_type_get(o);
        if ((type) && (!strcmp(type, "e_comp_object")))
          ec = e_comp_object_client_get(o);
        if (ec && (!ec->no_shape_cut))
          _e_comp_shapes_update_comp_client_shape_comp_helper(ec, tb
                                                              ,&rl
                                                             );

        else
          _e_comp_shapes_update_object_shape_comp_helper(o, tb);
     }

   ti = eina_tiler_iterator_new(tb);
   tile_count = 128;
   exr = malloc(sizeof(Eina_Rectangle) * tile_count);
   i = 0;
   EINA_ITERATOR_FOREACH(ti, tr)
     {
        exr[i++] = *((Eina_Rectangle *)tr);
        if (i == tile_count - 1)
          exr = realloc(exr, sizeof(Eina_Rectangle) * (tile_count *= 2));
        if (shape_debug)
          {
             Eina_List *l;

             _e_comp_shape_debug_rect(&exr[i - 1], &color);
             SHAPE_INF("%d,%d @ %dx%d", exr[i - 1].x, exr[i - 1].y, exr[i - 1].w, exr[i - 1].h);
             EINA_LIST_FOREACH(rl, l, r)
               {
                  if (E_INTERSECTS(r->x, r->y, r->w, r->h, tr->x, tr->y, tr->w, tr->h))
                    SHAPE_ERR("POSSIBLE RECT FAIL!!!!");
               }
          }
     }

#ifndef HAVE_WAYLAND_ONLY
   ecore_x_window_shape_input_rectangles_set(win, (Ecore_X_Rectangle*)exr, i);
#endif

   if (shape_debug)
     {
        E_FREE_LIST(rl, free);
        printf("\n");
     }
   free(exr);
   eina_iterator_free(ti);
   eina_tiler_free(tb);
   e_comp->shape_job = NULL;
}

//////////////////////////////////////////////////////////////////////////


static Eina_Bool
_e_comp_key_down(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Event_Key *ev)
{
   if ((!strcasecmp(ev->key, "f")) &&
       (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT) &&
       (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) &&
       (ev->modifiers & ECORE_EVENT_MODIFIER_ALT))
     {
        e_comp_canvas_fps_toggle();
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_comp_signal_user(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Event_Signal_User *ev)
{
   siginfo_t sig;

   sig = ev->data;
   /* anything sent via 'kill' will set this code to SI_USER */
   if (sig.si_code != SI_USER) return ECORE_CALLBACK_PASS_ON;

   if (ev->number == 1)
     {
        // e uses this to pop up config panel
     }
   else if (ev->number == 2)
     {
        e_comp_canvas_fps_toggle();
     }
   return ECORE_CALLBACK_PASS_ON;
}

//////////////////////////////////////////////////////////////////////////

static void
_e_comp_free(E_Comp *c)
{
   E_FREE_LIST(c->zones, e_object_del);

   e_comp_canvas_clear();

   e_randr2_shutdown();
#ifdef HAVE_WAYLAND
   if (c->comp_type == E_PIXMAP_TYPE_WL)
     e_comp_wl_shutdown();
#endif
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     e_comp_x_shutdown();
#endif

   ecore_evas_free(c->ee);

   if (c->update_job) ecore_job_del(c->update_job);
   if (c->nocomp_delay_timer) ecore_timer_del(c->nocomp_delay_timer);
   if (c->nocomp_override_timer) ecore_timer_del(c->nocomp_override_timer);
   ecore_job_del(c->shape_job);
   free(c->canvas);
   free(c);
}

//////////////////////////////////////////////////////////////////////////

static Eina_Bool
_e_comp_object_add(void *d EINA_UNUSED, int t EINA_UNUSED, E_Event_Comp_Object *ev)
{
   if ((!e_comp->nocomp) || (!e_comp->nocomp_ec)) return ECORE_CALLBACK_RENEW;
   if (evas_object_layer_get(ev->comp_object) > MAX(e_comp->nocomp_ec->saved.layer, E_LAYER_CLIENT_NORMAL))
     _e_comp_nocomp_end();
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_e_comp_override_expire(void *data EINA_UNUSED)
{
   e_comp->nocomp_override_timer = NULL;
   e_comp->nocomp_override--;

   if (e_comp->nocomp_override <= 0)
     {
        e_comp->nocomp_override = 0;
        if (e_comp->nocomp_want) _e_comp_cb_nocomp_begin();
     }
   return EINA_FALSE;
}

//////////////////////////////////////////////////////////////////////////

static Eina_Bool
_e_comp_screensaver_on(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eina_List *l;
   E_Zone *zone;

   if (e_comp->saver) return ECORE_CALLBACK_RENEW;
   e_comp_override_add();
   e_comp->saver = EINA_TRUE;
   // XXX: this is not quite right - need to wait for signals from theme
   // before freezing render animator
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        e_zone_fade_handle(zone, 1, 3.0);
        edje_object_signal_emit(zone->base, "e,state,screensaver,on", "e");
        edje_object_signal_emit(zone->over, "e,state,screensaver,on", "e");
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_comp_screensaver_off(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Eina_List *l;
   E_Zone *zone;
   E_Client *ec;

   if (!e_comp->saver) return ECORE_CALLBACK_RENEW;
   e_comp_override_del();
   e_comp->saver = EINA_FALSE;
   if (!e_comp->nocomp)
     ecore_evas_manual_render_set(e_comp->ee, EINA_FALSE);
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        edje_object_signal_emit(zone->base, "e,state,screensaver,off", "e");
        edje_object_signal_emit(zone->over, "e,state,screensaver,off", "e");
        e_zone_fade_handle(zone, 0, 0.5);
     }
   E_CLIENT_FOREACH(ec)
     if (e_comp_object_damage_exists(ec->frame))
       e_comp_object_render_update_add(ec->frame);

   return ECORE_CALLBACK_PASS_ON;
}

static Evas_Object *
_e_comp_act_opacity_obj_finder(E_Object *obj)
{
   E_Client *ec;

   switch (obj->type)
     {
      case E_CLIENT_TYPE:
        return ((E_Client *)(void *)obj)->frame;
      case E_ZONE_TYPE:
      case E_COMP_TYPE:
      case E_MENU_TYPE:
        ec = e_client_focused_get();
        return ec ? ec->frame : NULL;
     }
   if (e_obj_is_win(obj))
     return e_win_client_get((void *)obj)->frame;
   ec = e_client_focused_get();
   return ec ? ec->frame : NULL;
}

static void
_e_comp_act_opacity_change_go(E_Object *obj, const char *params)
{
   int opacity, cur;
   Evas_Object *o;

   if ((!params) || (!params[0])) return;
   o = _e_comp_act_opacity_obj_finder(obj);
   if (!o) return;
   opacity = atoi(params);
   opacity = E_CLAMP(opacity, -255, 255);
   evas_object_color_get(o, NULL, NULL, NULL, &cur);
   opacity += cur;
   opacity = E_CLAMP(opacity, 0, 255);
   evas_object_color_set(o, opacity, opacity, opacity, opacity);
}

static void
_e_comp_act_opacity_set_go(E_Object * obj EINA_UNUSED, const char *params)
{
   int opacity;
   Evas_Object *o;

   if ((!params) || (!params[0])) return;
   o = _e_comp_act_opacity_obj_finder(obj);
   if (!o) return;
   opacity = atoi(params);
   opacity = E_CLAMP(opacity, 0, 255);
   evas_object_color_set(o, opacity, opacity, opacity, opacity);
}

static void
_e_comp_act_redirect_toggle_go(E_Object * obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   E_Client *ec;

   ec = e_client_focused_get();
   if ((!ec) || (!e_pixmap_is_x(ec->pixmap)) || (ec == e_comp->nocomp_ec)) return;
   e_comp_client_redirect_toggle(ec);
}

//////////////////////////////////////////////////////////////////////////

static void
_e_comp_resize(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if ((w == e_comp->w) && (h == e_comp->h)) return;
   e_randr2_screens_setup(w, h);
   e_comp_canvas_update();
}

static void
_e_comp_delete_request()
{
   e_sys_action_do(E_SYS_LOGOUT, NULL);
}

EINTERN Eina_Bool
e_comp_init(void)
{
   _e_comp_log_dom = eina_log_domain_register("e_comp", EINA_COLOR_YELLOW);

   shape_debug = !!getenv("E_SHAPE_DEBUG");

   E_EVENT_COMPOSITOR_UPDATE = ecore_event_type_new();
   E_EVENT_COMP_OBJECT_ADD = ecore_event_type_new();
   E_EVENT_COMPOSITOR_DISABLE = ecore_event_type_new();
   E_EVENT_COMPOSITOR_ENABLE = ecore_event_type_new();
   E_EVENT_COMPOSITOR_XWAYLAND_INIT = ecore_event_type_new();

   ignores = eina_hash_pointer_new(NULL);

   e_comp_cfdata_edd_init(&conf_edd, &conf_match_edd);
   conf = e_config_domain_load("e_comp", conf_edd);
   if (conf)
     {
        conf->max_unmapped_pixels = 32 * 1024;
        conf->keep_unmapped = 1;
     }
   else
     conf = e_comp_cfdata_config_new();

   // comp config versioning - add this in. over time add epochs etc. if
   // necessary, but for now a simple version number will do
   if (conf->version < E_COMP_VERSION)
     {
        switch (conf->version)
          {
           case 0:
             // going from version 0 we should disable grab for smoothness
             conf->grab = 0;
             /* fallthrough */
           default:
             break;
          }
        e_config_save_queue();
        conf->version = E_COMP_VERSION;
     }

   if (conf->no_dither)
     e_util_env_set("EVAS_GL_RENDER_DISABLE_DITHER", "1");
   else
     e_util_env_set("EVAS_GL_RENDER_DISABLE_DITHER", NULL);

   {
      E_Action *act;

      act = e_action_add("opacity_change");
      act->func.go = _e_comp_act_opacity_change_go;
      e_action_predef_name_set(N_("Compositor"),
                               N_("Change current window opacity"), "opacity_change",
                               NULL, "syntax: +/- the amount to change opacity by (>0 for more opaque)", 1);
      actions = eina_list_append(actions, act);
      act = e_action_add("opacity_set");
      act->func.go = _e_comp_act_opacity_set_go;
      e_action_predef_name_set(N_("Compositor"),
                               N_("Set current window opacity"), "opacity_set",
                               "255", "syntax: number between 0-255 to set for transparent-opaque", 1);
      actions = eina_list_append(actions, act);
      act = e_action_add("redirect_toggle");
      act->func.go = _e_comp_act_redirect_toggle_go;
      e_action_predef_name_set(N_("Compositor"),
                               N_("Toggle focused client's redirect state"), "redirect_toggle",
                               NULL, NULL, 0);
      actions = eina_list_append(actions, act);
   }

   e_comp_new();
   e_comp->comp_type = E_PIXMAP_TYPE_NONE;

   {
      const char *gl;

      gl = getenv("E_COMP_ENGINE");
      if (gl)
        {
           int val;

           val = strtol(gl, NULL, 10);
           if ((val == E_COMP_ENGINE_SW) || (val == E_COMP_ENGINE_GL))
             e_comp_config_get()->engine = val;
           else if (!strcmp(gl, "gl"))
             e_comp_config_get()->engine = E_COMP_ENGINE_GL;
           else if (!strcmp(gl, "sw"))
             e_comp_config_get()->engine = E_COMP_ENGINE_SW;
           else
             {
                fprintf(stderr,
                        "ERROR: E_COMP_ENGINE value '%s' invalid.\n"
                        "Please use 'gl' or 'sw'.\n"
                        "Valid values below that request a type of acceleration:\n"
                        "  'gl' : OpenGL based acceleration.\n"
                        "  'sw' : CPU based Software rendering.\n",
                        gl);
                exit(101);
             }
        }
   }
   {
      const char *eng;

      eng = getenv("E_WL_FORCE");
      if (eng)
        {
           if ((!strcmp(eng, "buffer")) ||
               (!strcmp(eng, "drm")) ||
               (!strcmp(eng, "wl")) ||
               (!strcmp(eng, "x11")))
             {
                char buf[128];

                snprintf(buf, sizeof(buf), "wl_%s", eng);
                e_xkb_init(E_PIXMAP_TYPE_WL);
                if (e_module_enable(e_module_new(buf)))
                  {
                     e_comp->comp_type = E_PIXMAP_TYPE_WL;
                     goto out;
                  }
             }
           else
             {
                fprintf(stderr,
                        "ERROR: E_WL_FORCE value '%s' invalid.\n"
                        "Please use 'buffer', 'drm', 'wl' or 'x11'\n"
                        "Valid values below that request a type of rendering output target:\n"
                        "  'buffer' : Invisible memory buffer. (Debugging/testing)\n"
                        "  'drm'    : DRM/KMS framebuffer. (You want this as a normal full-screen stand-alone Wayland compositor)\n"
                        "  'wl'     : Window as a Wayland application. (Compositor in a Window for debugging/development/testing)\n"
                        "  'x11'    : Window as a X11 application. (Compositor in a Window for debugging/development/testing)\n",
                        eng);
                exit(101);
             }
        }
   }

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_x_init())
     {
        e_comp->comp_type = E_PIXMAP_TYPE_X;
        e_xkb_init(E_PIXMAP_TYPE_X);
     }
   else
#endif
     {
        const char **test, *eng[] =
        {
#ifdef USE_MODULE_WL_WL
           "wl_wl",
#endif
#ifdef USE_MODULE_WL_X11
           "wl_x11",
#endif
#ifdef USE_MODULE_WL_DRM
           "wl_drm",
#endif
/* probably add other engines here; fb should be last? */
#ifdef USE_MODULE_WL_FB
           "wl_fb",
#endif
           NULL
        };

        e_xkb_init(E_PIXMAP_TYPE_WL);
        e_util_env_set("HYBRIS_EGLPLATFORM", "wayland");
        for (test = eng; *test; test++)
          {
             if (e_module_enable(e_module_new(*test)))
               {
                  e_comp->comp_type = E_PIXMAP_TYPE_WL;
                  goto out;
               }
          }
        return EINA_FALSE;
     }
//#ifdef HAVE_WAYLAND_CLIENTS
   //e_comp_wl_init();
//#endif
   if (e_comp->comp_type == E_PIXMAP_TYPE_NONE) return EINA_FALSE;
out:
   if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
     {
        e_comp_canvas_fake_layers_init();
        e_screensaver_update();
     }
   e_comp->elm = elm_win_fake_add(e_comp->ee);
   evas_object_event_callback_add(e_comp->elm, EVAS_CALLBACK_RESIZE, _e_comp_resize, NULL);
   elm_win_fullscreen_set(e_comp->elm, 1);
   elm_win_autodel_set(e_comp->elm, 1);
   if (!e_comp->screen)
     evas_object_smart_callback_add(e_comp->elm, "delete,request", _e_comp_delete_request, NULL);
   ecore_evas_focus_set(e_comp->ee, 0);
   ecore_evas_focus_set(e_comp->ee, 1);
   evas_object_show(e_comp->elm);
   e_util_env_set("HYBRIS_EGLPLATFORM", NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_SCREENSAVER_ON, _e_comp_screensaver_on, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_SCREENSAVER_OFF, _e_comp_screensaver_off, NULL);

   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_KEY_DOWN, _e_comp_key_down, NULL);
   E_LIST_HANDLER_APPEND(handlers, ECORE_EVENT_SIGNAL_USER, _e_comp_signal_user, NULL);
   E_LIST_HANDLER_APPEND(handlers, E_EVENT_COMP_OBJECT_ADD, _e_comp_object_add, NULL);

   return EINA_TRUE;
}


static Eina_Bool
_style_demo(void *data)
{
   Eina_List *style_shadows, *l;
   int demo_state;
   const E_Comp_Demo_Style_Item *it;

   demo_state = (long)evas_object_data_get(data, "style_demo_state");
   demo_state = (demo_state + 1) % 4;
   evas_object_data_set(data, "style_demo_state", (void *)(long)demo_state);

   style_shadows = evas_object_data_get(data, "style_shadows");
   EINA_LIST_FOREACH(style_shadows, l, it)
     {
        Evas_Object *ob = it->preview;
        Evas_Object *of = it->frame;

        switch (demo_state)
          {
           case 0:
             edje_object_signal_emit(ob, "e,state,visible", "e");
             edje_object_signal_emit(ob, "e,state,focused", "e");
             edje_object_part_text_set(of, "e.text.label", _("Visible"));
             break;

           case 1:
             edje_object_signal_emit(ob, "e,state,unfocused", "e");
             edje_object_part_text_set(of, "e.text.label", _("Focus-Out"));
             break;

           case 2:
             edje_object_signal_emit(ob, "e,state,focused", "e");
             edje_object_part_text_set(of, "e.text.label", _("Focus-In"));
             break;

           case 3:
             edje_object_signal_emit(ob, "e,state,hidden", "e");
             edje_object_part_text_set(of, "e.text.label", _("Hidden"));
             break;

           default:
             break;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_style_selector_del(void *data       EINA_UNUSED,
                    Evas *e,
                    Evas_Object *o,
                    void *event_info EINA_UNUSED)
{
   Eina_List *style_shadows, *style_list;
   Ecore_Timer *timer;
   Evas_Object *orec0;

   orec0 = evas_object_name_find(e, "style_shadows");
   style_list = evas_object_data_get(orec0, "list");

   style_shadows = evas_object_data_get(o, "style_shadows");
   if (style_shadows)
     {
        E_Comp_Demo_Style_Item *ds_it;

        EINA_LIST_FREE(style_shadows, ds_it)
          {
             style_list = eina_list_remove(style_list, ds_it);

             evas_object_del(ds_it->frame);
             evas_object_del(ds_it->livethumb);
             free(ds_it);
          }
        evas_object_data_set(o, "style_shadows", NULL);
     }

   timer = evas_object_data_get(o, "style_timer");
   if (timer)
     {
        ecore_timer_del(timer);
        evas_object_data_set(o, "style_timer", NULL);
     }

   evas_object_data_set(orec0, "list", style_list);
}

EINTERN Evas_Object *
e_comp_style_selector_create(Evas *evas, const char **source)
{
   Evas_Object *oi, *ob, *oo, *obd, *orec, *oly, *orec0;
   Eina_List *styles, *l, *style_shadows = NULL, *style_list;
   char *style;
   const char *str;
   int n, sel;
   Evas_Coord wmw, wmh;
   Ecore_Timer *timer;

   orec0 = evas_object_name_find(evas, "style_shadows");
   style_list = evas_object_data_get(orec0, "list");
   oi = e_widget_ilist_add(evas, 80, 80, source);
   evas_object_event_callback_add(oi, EVAS_CALLBACK_DEL,
                                  _style_selector_del, oi);
   sel = 0;
   styles = e_theme_comp_frame_list();
   n = 0;
   EINA_LIST_FOREACH(styles, l, style)
     {
        E_Comp_Demo_Style_Item *ds_it;
        char buf[4096];

        ds_it = malloc(sizeof(E_Comp_Demo_Style_Item));

        ob = e_livethumb_add(evas);
        ds_it->livethumb = ob;
        e_livethumb_vsize_set(ob, 240, 240);

        oly = e_layout_add(e_livethumb_evas_get(ob));
        ds_it->layout = ob;
        e_layout_virtual_size_set(oly, 240, 240);
        e_livethumb_thumb_set(ob, oly);
        evas_object_show(oly);

        oo = edje_object_add(e_livethumb_evas_get(ob));
        ds_it->preview = oo;
        snprintf(buf, sizeof(buf), "e/comp/frame/%s", style);
        e_theme_edje_object_set(oo, "base/theme/borders", buf);
        e_layout_pack(oly, oo);
        e_layout_child_move(oo, 39, 39);
        e_layout_child_resize(oo, 162, 162);
        edje_object_signal_emit(oo, "e,state,visible", "e");
        edje_object_signal_emit(oo, "e,state,focused", "e");
        evas_object_show(oo);

        ds_it->frame = edje_object_add(evas);
        e_theme_edje_object_set
          (ds_it->frame, "base/theme/comp", "e/comp/preview");
        edje_object_part_swallow(ds_it->frame, "e.swallow.preview", ob);
        evas_object_show(ds_it->frame);
        style_shadows = eina_list_append(style_shadows, ds_it);

        obd = edje_object_add(e_livethumb_evas_get(ob));
        ds_it->border = obd;
        e_theme_edje_object_set(obd, "base/theme/borders",
                                "e/widgets/border/default/border");
        edje_object_part_text_set(obd, "e.text.title", _("Title"));
        edje_object_signal_emit(obd, "e,state,shadow,on", "e");
        edje_object_part_swallow(oo, "e.swallow.content", obd);
        evas_object_show(obd);

        orec = evas_object_rectangle_add(e_livethumb_evas_get(ob));
        ds_it->client = orec;
        evas_object_color_set(orec, 0, 0, 0, 128);
        edje_object_part_swallow(obd, "e.swallow.client", orec);
        evas_object_show(orec);

        e_widget_ilist_append(oi, ds_it->frame, style, NULL, NULL, style);
        evas_object_show(ob);
        if (*source)
          {
             if (!strcmp(*source, style)) sel = n;
          }
        n++;

        style_list = eina_list_append(style_list, ds_it);
     }
   evas_object_data_set(orec0, "list", style_list);
   evas_object_data_set(oi, "style_shadows", style_shadows);
   timer = ecore_timer_loop_add(3.0, _style_demo, oi);
   evas_object_data_set(oi, "style_timer", timer);
   evas_object_data_set(oi, "style_demo_state", (void *)1);
   e_widget_size_min_get(oi, &wmw, &wmh);
   e_widget_size_min_set(oi, 160, 100);
   e_widget_ilist_selected_set(oi, sel);
   e_widget_ilist_go(oi);

   EINA_LIST_FREE(styles, str)
     eina_stringshare_del(str);

   return oi;
}

E_API E_Comp *
e_comp_new(void)
{
   if (e_comp)
     CRI("CANNOT REPLACE EXISTING COMPOSITOR");
   e_comp = E_OBJECT_ALLOC(E_Comp, E_COMP_TYPE, _e_comp_free);
   if (!e_comp) return NULL;
   e_comp->canvas = E_NEW(E_Comp_Canvas, 1);
   return e_comp;
}

E_API int
e_comp_internal_save(void)
{
   return e_config_domain_save("e_comp", conf_edd, conf);
}

EINTERN int
e_comp_shutdown(void)
{
   Eina_List *l, *ll;
   E_Client *ec;

   E_FREE_FUNC(action_timeout, ecore_timer_del);
   EINA_LIST_FOREACH_SAFE(e_comp->clients, l, ll, ec)
     {
        DELD(ec, 99999);
        e_object_del(E_OBJECT(ec));
     }

   e_object_del(E_OBJECT(e_comp));
   e_comp = NULL;
   E_FREE_LIST(handlers, ecore_event_handler_del);
   E_FREE_LIST(actions, e_object_del);
   E_FREE_LIST(hooks, e_client_hook_del);

   gl_avail = EINA_FALSE;
   e_comp_cfdata_config_free(conf);
   E_CONFIG_DD_FREE(conf_match_edd);
   E_CONFIG_DD_FREE(conf_edd);
   conf = NULL;
   conf_match_edd = NULL;
   conf_edd = NULL;

   E_FREE_FUNC(ignores, eina_hash_free);

   return 1;
}

E_API void
e_comp_render_queue(void)
{
   if (e_comp->update_job)
     {
        DBG("UPDATE JOB DEL...");
        E_FREE_FUNC(e_comp->update_job, ecore_job_del);
     }
   DBG("UPDATE JOB ADD...");
   e_comp->update_job = ecore_job_add(_e_comp_cb_job, e_comp);
}

E_API void
e_comp_client_post_update_add(E_Client *ec)
{
   if (ec->on_post_updates) return;
   ec->on_post_updates = EINA_TRUE;
   e_comp->post_updates = eina_list_append(e_comp->post_updates, ec);
   REFD(ec, 111);
   e_object_ref(E_OBJECT(ec));
}

E_API void
e_comp_shape_queue(void)
{
   if ((e_comp->comp_type != E_PIXMAP_TYPE_X) && (!e_comp_util_has_x())) return;
   if (!e_comp->shape_job)
     e_comp->shape_job = ecore_job_add(_e_comp_shapes_update_job, NULL);
}

E_API void
e_comp_shape_queue_block(Eina_Bool block)
{
   e_comp->shape_queue_blocked = !!block;
   if (block)
     E_FREE_FUNC(e_comp->shape_job, ecore_job_del);
   else
     e_comp_shape_queue();
}

E_API E_Comp_Config *
e_comp_config_get(void)
{
   return conf;
}

E_API void
e_comp_shadows_reset(void)
{
   E_Client *ec;

   _e_comp_fps_update();
   E_LIST_FOREACH(e_comp->zones, e_comp_canvas_zone_update);
   E_CLIENT_FOREACH(ec)
     e_comp_object_frame_theme_set(ec->frame, E_COMP_OBJECT_FRAME_RESHADOW);
}

E_API Ecore_Window
e_comp_top_window_at_xy_get(Evas_Coord x, Evas_Coord y)
{
   E_Client *ec;
   Evas_Object *o;

   EINA_SAFETY_ON_NULL_RETURN_VAL(e_comp, 0);
   o = evas_object_top_at_xy_get(e_comp->evas, x, y, 0, 0);
   if (!o) return e_comp->ee_win;
   ec = evas_object_data_get(o, "E_Client");
   if (ec) return e_client_util_pwin_get(ec);
   return e_comp->ee_win;
}

E_API void
e_comp_util_wins_print(void)
{
   Evas_Object *o;

   o = evas_object_top_get(e_comp->evas);
   while (o)
     {
        E_Client *ec;
        int x, y, w, h;

        ec = evas_object_data_get(o, "E_Client");
        evas_object_geometry_get(o, &x, &y, &w, &h);
        fprintf(stderr, "LAYER %d  ", evas_object_layer_get(o));
        if (ec)
          fprintf(stderr, "EC%s%s:  %p - '%s:%s' || %d,%d @ %dx%d\n",
                  ec->override ? "O" : "", ec->focused ? "*" : "", ec,
                  e_client_util_name_get(ec) ?: ec->icccm.name, ec->icccm.class, x, y, w, h);
        else
          fprintf(stderr, "OBJ: %p - %s || %d,%d @ %dx%d\n", o, evas_object_name_get(o), x, y, w, h);
        o = evas_object_below_get(o);
     }
   fputc('\n', stderr);
}

E_API void
e_comp_ignore_win_add(E_Pixmap_Type type, Ecore_Window win)
{
   E_Client *ec;

   eina_hash_add(ignores, &win, (void*)1);
   ec = e_pixmap_find_client(type, win);
   if (!ec) return;
   ec->ignored = 1;
   if (ec->visible) evas_object_hide(ec->frame);
}

E_API void
e_comp_ignore_win_del(E_Pixmap_Type type, Ecore_Window win)
{
   E_Client *ec;

   eina_hash_del_by_key(ignores, &win);
   ec = e_pixmap_find_client(type, win);
   if ((!ec) || (e_object_is_del(E_OBJECT(ec)))) return;
   ec->ignored = 0;
   if (ec->visible) evas_object_show(ec->frame);
}

E_API Eina_Bool
e_comp_ignore_win_find(Ecore_Window win)
{
   return !!eina_hash_find(ignores, &win);
}

E_API void
e_comp_override_del(void)
{
   e_comp->nocomp_override--;
   if (e_comp->nocomp_override <= 0)
     {
        e_comp->nocomp_override = 0;
        if (e_comp->nocomp_want) _e_comp_cb_nocomp_begin();
     }
}

E_API void
e_comp_override_add(void)
{
   e_comp->nocomp_override++;
   if ((e_comp->nocomp_override > 0) && (e_comp->nocomp)) _e_comp_nocomp_end();
}

E_API E_Comp *
e_comp_find_by_window(Ecore_Window win)
{
   if ((e_comp->win == win) || (e_comp->ee_win == win) || (e_comp->root == win)) return e_comp;
   return NULL;
}

E_API void
e_comp_override_timed_pop(void)
{
   if (e_comp->nocomp_override <= 0) return;
   if (e_comp->nocomp_override_timer)
     e_comp->nocomp_override--;
   else
     e_comp->nocomp_override_timer = ecore_timer_loop_add(1.0, _e_comp_override_expire, NULL);
}

E_API unsigned int
e_comp_e_object_layer_get(const E_Object *obj)
{
   E_Gadcon *gc = NULL;

   if (!obj) return 0;

   switch (obj->type)
     {
      case E_GADCON_CLIENT_TYPE:
        gc = ((E_Gadcon_Client *)(void *)(obj))->gadcon;
        EINA_SAFETY_ON_NULL_RETURN_VAL(gc, 0);
        EINA_FALLTHROUGH;
        /* no break */

      case E_GADCON_TYPE:
        if (!gc) gc = (E_Gadcon *)obj;
        if (gc->shelf) return gc->shelf->layer;
        if (!gc->toolbar) return E_LAYER_DESKTOP;
        return e_win_client_get(gc->toolbar->fwin)->layer;

      case E_ZONE_TYPE:
        return E_LAYER_DESKTOP;

      case E_CLIENT_TYPE:
        return ((E_Client *)(void *)(obj))->layer;

      /* FIXME: add more types as needed */
      default:
        break;
     }
   if (e_obj_is_win(obj))
     return e_win_client_get((void*)obj)->layer;
   return 0;
}

E_API Eina_Bool
e_comp_grab_input(Eina_Bool mouse, Eina_Bool kbd)
{
   Eina_Bool ret = EINA_FALSE;
   Ecore_Window mwin = 0, kwin = 0;

   mouse = !!mouse;
   kbd = !!kbd;
   if (mouse || e_comp->input_mouse_grabs)
     mwin = e_comp->ee_win;
   if (kbd || e_comp->input_mouse_grabs)
     kwin = e_comp->ee_win;
   e_comp_override_add();
   if ((e_comp->input_mouse_grabs && e_comp->input_key_grabs) ||
       e_grabinput_get(mwin, 0, kwin))
     {
        E_Client *ec = e_client_focused_get();

        if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
          {
             if (ec)
               evas_object_focus_set(ec->frame, 0);

#ifdef HAVE_WAYLAND
             e_comp_wl_extension_pointer_unconstrain(NULL);
#endif
          }

        ret = EINA_TRUE;
        e_comp->input_mouse_grabs += mouse;
        e_comp->input_key_grabs += kbd;

        if (e_comp->comp_type == E_PIXMAP_TYPE_WL)
          {
             if (ec && (!e_object_is_del(E_OBJECT(ec))))
               evas_object_focus_set(ec->frame, 1);
          }
     }
   return ret;
}

E_API void
e_comp_ungrab_input(Eina_Bool mouse, Eina_Bool kbd)
{
   Ecore_Window mwin = 0, kwin = 0;

   mouse = !!mouse;
   kbd = !!kbd;
   if (e_comp->input_mouse_grabs)
     e_comp->input_mouse_grabs -= mouse;
   if (e_comp->input_key_grabs)
     e_comp->input_key_grabs -= kbd;
   if (mouse && (!e_comp->input_mouse_grabs))
     mwin = e_comp->ee_win;
   if (kbd && (!e_comp->input_key_grabs))
     kwin = e_comp->ee_win;
   e_comp_override_timed_pop();
   if ((!mwin) && (!kwin)) return;
   e_grabinput_release(mwin, kwin);
   evas_event_feed_mouse_out(e_comp->evas, 0, NULL);
   evas_event_feed_mouse_in(e_comp->evas, 0, NULL);
   if (e_client_focused_get())
     {
        E_Client *ec = e_client_focused_get();

        if ((!e_object_is_del(E_OBJECT(ec))) && (e_comp->comp_type == E_PIXMAP_TYPE_WL))
          {
             Eina_Bool mouse_in = ec->mouse.in;
             int x, y;

             x = ec->mouse.current.mx;
             y = ec->mouse.current.my;
             evas_object_focus_set(ec->frame, 0);
             evas_object_focus_set(ec->frame, 1);
             if (mouse_in)
               e_client_mouse_in(ec, x, y);
          }
        return;
     }
   if (e_config->focus_policy != E_FOCUS_MOUSE)
     e_client_refocus();
}

E_API Eina_Bool
e_comp_util_kbd_grabbed(void)
{
   return e_menu_is_active() || e_client_action_get() || e_grabinput_key_win_get();
}

E_API Eina_Bool
e_comp_util_mouse_grabbed(void)
{
   return e_menu_is_active() || e_client_action_get() || e_grabinput_mouse_win_get();
}

E_API void
e_comp_gl_set(Eina_Bool set)
{
   gl_avail = !!set;
}

E_API Eina_Bool
e_comp_gl_get(void)
{
   return gl_avail;
}

E_API void
e_comp_button_bindings_ungrab_all(void)
{
   if (e_comp->bindings_ungrab_cb)
     e_comp->bindings_ungrab_cb();
}

E_API void
e_comp_button_bindings_grab_all(void)
{
   if (e_comp->bindings_grab_cb)
     e_comp->bindings_grab_cb();
}

E_API void
e_comp_client_redirect_toggle(E_Client *ec)
{
   EINA_SAFETY_ON_NULL_RETURN(ec);
   if (!conf->enable_advanced_features) return;
   if (e_pixmap_type_get(ec->pixmap) != E_PIXMAP_TYPE_X) return;
   ec->unredirected_single = !ec->unredirected_single;
   e_client_redirected_set(ec, !ec->redirected);
   ec->no_shape_cut = !ec->redirected;
   e_comp_shape_queue();
}

E_API Eina_Bool
e_comp_util_object_is_above_nocomp(Evas_Object *obj)
{
   Evas_Object *o;
   int cl, ol;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   if (!evas_object_visible_get(obj)) return EINA_FALSE;
   if (!e_comp->nocomp_ec) return EINA_FALSE;
   cl = evas_object_layer_get(e_comp->nocomp_ec->frame);
   ol = evas_object_layer_get(obj);
   if (cl > ol) return EINA_FALSE;
   o = evas_object_above_get(e_comp->nocomp_ec->frame);
   if ((cl == ol) && (evas_object_layer_get(o) == cl))
     {
        do {
           if (o == obj)
             return EINA_TRUE;
           o = evas_object_above_get(o);
        } while (o && (evas_object_layer_get(o) == cl));
     }
   else
     return EINA_TRUE;
   return EINA_FALSE;
}

E_API void
e_comp_clients_rescale(void)
{
   int i;

   if (stopping) return;
   for (i = 0; i < 11; i++)
     {
        Eina_List *tmp = NULL;
        E_Client *ec;

        if (!e_comp->layers[i].clients) continue;
        /* Make temporary list as e_client_res_change_geometry_restore
         * rearranges the order. */
        EINA_INLIST_FOREACH(e_comp->layers[i].clients, ec)
          {
             if ((!e_client_util_ignored_get(ec)) && (!e_object_is_del(E_OBJECT(ec))))
               tmp = eina_list_append(tmp, ec);
          }

        EINA_LIST_FREE(tmp, ec)
          {
             ec->pre_res_change.valid = 0;
             if (ec->maximized || ec->fullscreen)
               e_client_rescale(ec);
          }
     }
}
