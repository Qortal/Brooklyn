#include "e.h"

typedef struct _Video Video;

struct _Video
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Evas_Object *clip, *o_vid;
   Eina_Bool lowqual E_BITFIELD;
};

typedef struct _Vidimg Vidimg;

struct _Vidimg
{
   Evas_Object *obj;
   Eina_List *copies;
   int iw, ih, piw, pih;
   Ecore_Job *restart_job;
   Ecore_Event_Handler *offhandler;
   Ecore_Event_Handler *onhandler;
   Ecore_Timer *offtimer;
};

static Evas_Smart *_smart = NULL;
static Evas_Smart_Class _parent_sc = EVAS_SMART_CLASS_INIT_NULL;

static Eina_List *vidimgs = NULL;

static void _ob_resize(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

static void
_cb_vid_frame(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Vidimg *vi = data;
   Eina_List *l;
   Evas_Object *o, *oimg;

   EINA_LIST_FOREACH(vi->copies, l, o)
     {
        Evas_Coord ox, oy, ow, oh;
        Video *sd;

        obj = evas_object_data_get(o, "obj");
        oimg = emotion_object_image_get(vi->obj);
        evas_object_image_source_set(o, oimg);
        evas_object_image_source_clip_set(o, EINA_FALSE);
        evas_object_image_source_visible_set(o, EINA_FALSE);
        sd = evas_object_smart_data_get(obj);
        evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
        evas_object_show(sd->o_vid);
        evas_object_show(sd->clip);
        _ob_resize(obj, ox, oy, ow, oh);
     }
}

static void
_cb_vid_resize(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Vidimg *vi = data;
   Eina_List *l;
   Evas_Object *o;

   EINA_LIST_FOREACH(vi->copies, l, o)
     {
        Evas_Coord ox, oy, ow, oh;

        obj = evas_object_data_get(o, "obj");
        evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
        _ob_resize(obj, ox, oy, ow, oh);
     }
}

static void
_cb_restart(void *data)
{
   Vidimg *vi = data;

   vi->restart_job = NULL;
   emotion_object_position_set(vi->obj, 0.0);
   emotion_object_play_set(vi->obj, EINA_TRUE);
}

static void
_cb_vid_stop(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Vidimg *vi = data;

   if (vi->restart_job) ecore_job_del(vi->restart_job);
   vi->restart_job = ecore_job_add(_cb_restart, vi);
}

static Eina_Bool
vidimg_cb_suspend(void *data)
{
   Vidimg *vi = data;

   vi->offtimer = NULL;
   emotion_object_play_set(vi->obj, EINA_FALSE);
   return EINA_FALSE;
}

static Eina_Bool
vidimg_cb_screensaver_off(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Vidimg *vi = data;

   if (vi->offtimer)
     {
        ecore_timer_del(vi->offtimer);
        vi->offtimer = NULL;
     }
   else
     {
        emotion_object_play_set(vi->obj, EINA_TRUE);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
vidimg_cb_screensaver_on(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Vidimg *vi = data;

   if (vi->offtimer) ecore_timer_del(vi->offtimer);
   vi->offtimer = ecore_timer_loop_add(10.0, vidimg_cb_suspend, vi);
   return ECORE_CALLBACK_PASS_ON;
}

static Evas_Object *
vidimg_find(Evas_Object *obj, const char *file)
{
   Eina_List *l;
   Evas_Object *o, *oimg, *oproxy;

   EINA_LIST_FOREACH(vidimgs, l, o)
     {
        if (evas_object_evas_get(o) == evas_object_evas_get(obj))
          {
             const char *f = emotion_object_file_get(o);

             if ((f) && (file) && (!strcmp(f, file)))
               {
                  Vidimg *vi = evas_object_data_get(o, "vidimg");
                  oproxy = evas_object_image_filled_add(evas_object_evas_get(obj));
                  evas_object_data_set(oproxy, "source", o);
                  oimg = emotion_object_image_get(o);
                  evas_object_image_source_set(oproxy, oimg);
                  evas_object_image_source_clip_set(oproxy, EINA_FALSE);
                  evas_object_image_source_visible_set(oproxy, EINA_FALSE);
                  vi->copies = eina_list_append(vi->copies, oproxy);
                  evas_object_data_set(oproxy, "obj", obj);
                  return oproxy;
               }
          }
     }
   return NULL;
}

static Evas_Object *
vidimg_video_add(Evas_Object *obj, const char *file)
{
   Evas_Object *o;
   Vidimg *vi;

   o = emotion_object_add(evas_object_evas_get(obj));
   if (!emotion_object_init(o, "gstreamer1"))
     {
        evas_object_del(o);
        return NULL;
     }
   vi = calloc(1, sizeof(Vidimg));
   vi->obj = o;
   vi->offhandler = ecore_event_handler_add(E_EVENT_SCREENSAVER_OFF, vidimg_cb_screensaver_off, vi);
   vi->onhandler = ecore_event_handler_add(E_EVENT_SCREENSAVER_ON, vidimg_cb_screensaver_on, vi);
   evas_object_data_set(o, "vidimg", vi);
   evas_object_smart_callback_add(o, "frame_decode", _cb_vid_frame, vi);
   evas_object_smart_callback_add(o, "frame_resize", _cb_vid_resize, vi);
   evas_object_smart_callback_add(o, "decode_stop", _cb_vid_stop, vi);
   emotion_object_file_set(o, file);
   emotion_object_audio_mute_set(o, EINA_TRUE);
   emotion_object_keep_aspect_set(o, EMOTION_ASPECT_KEEP_NONE);
   emotion_object_play_set(o, EINA_TRUE);
   evas_object_pass_events_set(o, EINA_TRUE);
   evas_object_move(o, -999, -999);
   evas_object_resize(o, 9, 0);
   evas_object_show(o);
   vidimgs = eina_list_append(vidimgs, o);
   return o;
}

static void
vidimg_release(Evas_Object *o)
{
   Vidimg *vi;
   Evas_Object *src = evas_object_data_get(o, "source");
   if (!src) return;
   vi = evas_object_data_get(src, "vidimg");
   vi->copies = eina_list_remove(vi->copies, o);
   if (!vi->copies)
     {
        vidimgs = eina_list_remove(vidimgs, src);
        evas_object_data_del(src, "vidimg");
        if (vi->restart_job) ecore_job_del(vi->restart_job);
        ecore_event_handler_del(vi->offhandler);
        ecore_event_handler_del(vi->onhandler);
        if (vi->offtimer) ecore_timer_del(vi->offtimer);
        free(vi);
        evas_object_del(src);
     }
}





static void
_ob_resize(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Video *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_move(sd->o_vid, x, y);
   evas_object_resize(sd->o_vid, w, h);
}

static void _smart_calculate(Evas_Object *obj);

static void
_smart_add(Evas_Object *obj)
{
   Video *sd;
   Evas_Object *o;

   sd = calloc(1, sizeof(Video));
   EINA_SAFETY_ON_NULL_RETURN(sd);
   evas_object_smart_data_set(obj, sd);

   _parent_sc.add(obj);

   o = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(o, obj);
   sd->clip = o;
   evas_object_color_set(o, 255, 255, 255, 255);
}

static void
_smart_del(Evas_Object *obj)
{
   Video *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->clip) evas_object_del(sd->clip);

   vidimg_release(sd->o_vid);

   _parent_sc.del(obj);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Video *sd = evas_object_smart_data_get(obj);
   Evas_Coord ox, oy, ow, oh;
   if (!sd) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   evas_object_smart_changed(obj);
   evas_object_resize(sd->clip, ow, oh);
}

static void
_smart_calculate(Evas_Object *obj)
{
   Video *sd = evas_object_smart_data_get(obj);
   Evas_Coord ox, oy, ow, oh;

   if (!sd) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   _ob_resize(obj, ox, oy, ow, oh);
   evas_object_move(sd->clip, ox, oy);
   evas_object_resize(sd->clip, ow, oh);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x EINA_UNUSED, Evas_Coord y EINA_UNUSED)
{
   Video *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_smart_changed(obj);
}

static void
_smart_init(void)
{
   static Evas_Smart_Class sc;

   evas_object_smart_clipped_smart_set(&_parent_sc);
   sc           = _parent_sc;
   sc.name      = "video";
   sc.version   = EVAS_SMART_CLASS_VERSION;
   sc.add       = _smart_add;
   sc.del       = _smart_del;
   sc.resize    = _smart_resize;
   sc.move      = _smart_move;
   sc.calculate = _smart_calculate;
   _smart = evas_smart_class_new(&sc);
}

E_API Evas_Object *
e_video_add(Evas_Object *parent, const char *file, Eina_Bool lowq)
{
   Evas *e;
   Evas_Object *obj, *o;
   Video *sd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   e = evas_object_evas_get(parent);
   if (!e) return NULL;

   if (!_smart) _smart_init();
   obj = evas_object_smart_add(e, _smart);
   sd = evas_object_smart_data_get(obj);
   if (!sd) return obj;

   sd->lowqual = lowq;

   o = sd->o_vid = vidimg_find(obj, file);
   if (!o)
     {
        o = vidimg_video_add(obj, file);
        if (!o) return obj;
        o = sd->o_vid = vidimg_find(obj, file);
        if (!o) return obj;
     }
   evas_object_smart_member_add(o, obj);
   evas_object_clip_set(o, sd->clip);
   evas_object_image_smooth_scale_set(o, !sd->lowqual);
   return obj;
}

E_API const char *
e_video_file_get(Evas_Object *obj)
{
   Video *sd;
   Evas_Object *src;

   if (evas_object_smart_smart_get(obj) != _smart) return NULL;
   if (!(sd = evas_object_smart_data_get(obj))) return NULL;
   src = evas_object_data_get(sd->o_vid, "source");
   if (src) return emotion_object_file_get(src);
   return NULL;
}
