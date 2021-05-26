#include <Elementary.h>
#include "emix.h"

#   ifdef HAVE_GETTEXT
#include <libintl.h>
#define _(str) gettext(str)
#define d_(str, dom) dgettext(PACKAGE dom, str)
#define P_(str, str_p, n) ngettext(str, str_p, n)
#define dP_(str, str_p, n, dom) dngettext(PACKAGE dom, str, str_p, n)
#   else
#define _(str) (str)
#define d_(str, dom) (str)
#define P_(str, str_p, n) (str_p)
#define dP_(str, str_p, n, dom) (str_p)
#   endif

Evas_Object *win;
Evas_Object *source_scroller, *sink_input_scroller, *sink_scroller, *card_scroller;
Evas_Object *source_box, *sink_input_box, *sink_box, *card_box;
Evas_Object *category_current = NULL;
Eina_List *source_list = NULL, *sink_input_list = NULL, *sink_list = NULL, *card_list = NULL;

//////////////////////////////////////////////////////////////////////////////
static int
_glob_case_match(const char *str, const char *pattern)
{
   const char *p;
   char *tstr, *tglob, *tp;

   if (str == pattern) return 1;
   if (pattern[0] == 0)
     {
        if (str[0] == 0) return 1;
        return 0;
     }
   if (!strcmp(pattern, "*")) return 1;
   tstr = alloca(strlen(str) + 1);
   for (tp = tstr, p = str; *p != 0; p++, tp++)
   *tp = tolower(*p);
   *tp = 0;
   tglob = alloca(strlen(pattern) + 1);
   for (tp = tglob, p = pattern; *p != 0; p++, tp++)
   *tp = tolower(*p);
   *tp = 0;
   if (eina_fnmatch(tglob, tstr, 0)) return 1;
   return 0;
}

static char *
_sink_icon_find(const char *name)
{
   const char *file;
   char buf[PATH_MAX], *res = NULL, **strs, *glob, *icon;
   FILE *f;
   size_t len;

   file = getenv("EMIX_SINK_ICONS");
   if (!file) return NULL;
   f = fopen(file, "r");
   if (!f) return NULL;
   while (fgets(buf, sizeof(buf), f))
     {
        buf[sizeof(buf) - 1] = 0;
        len = strlen(buf);
        if (len > 0)
          {
             buf[len - 1] = 0;
             strs = eina_str_split(buf, "|", 0);
             if (strs)
               {
                  glob = strs[0];
                  icon = strs[1];
                  if (icon)
                    {
                       if (_glob_case_match(name, glob))
                         {
                            res = strdup(icon);
                         }
                    }
                  free(strs[0]);
                  free(strs);
               }
             if (res) break;
          }
        else break;
     }
   fclose(f);
   return res;
}

//////////////////////////////////////////////////////////////////////////////

typedef struct _Mon_Data
{
   Emix_Sink *sink;
   Emix_Sink_Input *input;
   Emix_Source *source;
   Evas_Object *fr;
   Evas_Object *vu;
   Ecore_Animator *animator;
   float samp_max;
   int mon_skips;
   int mon_update;
   int mon_samps;
} Mon_Data;

static Eina_List *_monitor_data_list = NULL;

static Eina_Bool
_cb_emix_monitor_update(void *data)
{
   Mon_Data *md = data;

   if (md->mon_update == 0)
     {
        md->mon_skips++;
        if (md->mon_skips > 5)
          {
             elm_progressbar_value_set(md->vu, 0.0);
             md->animator = NULL;
             return EINA_FALSE;
          }
        return EINA_TRUE;
     }
   elm_progressbar_value_set(md->vu, md->samp_max);
   md->mon_update = 0;
   md->samp_max = 0;
   md->mon_skips = 0;
   md->mon_samps = 0;
   return EINA_TRUE;
}

static void
_cb_emix_sink_monitor_event(void *data, enum Emix_Event event, void *event_info)
{
   Mon_Data *md = data;
   Emix_Sink *sink = event_info;

   if (sink != md->sink) return;
   if (event == EMIX_SINK_MONITOR_EVENT)
     {
        unsigned int i, num = sink->mon_num * 2;
        float samp, max = 0.0;

        for (i = 0; i < num; i++)
          {
             samp = fabs(sink->mon_buf[i]);
             if (samp > max) max = samp;
          }
        md->mon_samps += num;
        if (md->samp_max < max) md->samp_max = max;
        md->mon_update++;
        if (!md->animator)
          md->animator = ecore_animator_add(_cb_emix_monitor_update, md);
     }
}

static void
_cb_emix_sink_input_monitor_event(void *data, enum Emix_Event event, void *event_info)
{
   Mon_Data *md = data;
   Emix_Sink_Input *input = event_info;

   if (input != md->input) return;
   if (event == EMIX_SINK_INPUT_MONITOR_EVENT)
     {
        unsigned int i, num = input->mon_num * 2;
        float samp, max = 0.0;

        for (i = 0; i < num; i++)
          {
             samp = fabs(input->mon_buf[i]);
             if (samp > max) max = samp;
          }
        md->mon_samps += num;
        if (md->samp_max < max) md->samp_max = max;
        md->mon_update++;
        if (!md->animator)
          md->animator = ecore_animator_add(_cb_emix_monitor_update, md);
     }
}

static void
_cb_emix_source_monitor_event(void *data, enum Emix_Event event, void *event_info)
{
   Mon_Data *md = data;
   Emix_Source *source = event_info;

   if (source != md->source) return;
   if (event == EMIX_SOURCE_MONITOR_EVENT)
     {
        unsigned int i, num = source->mon_num * 2;
        float samp, max = 0.0;

        for (i = 0; i < num; i++)
          {
             samp = fabs(source->mon_buf[i]);
             if (samp > max) max = samp;
          }
        md->mon_samps += num;
        if (md->samp_max < max) md->samp_max = max;
        md->mon_update++;
        if (!md->animator)
          md->animator = ecore_animator_add(_cb_emix_monitor_update, md);
     }
}

//////////////////////////////////////////////////////////////////////////////

static void _emix_sink_volume_fill(Emix_Sink *sink, Evas_Object *bxv, Evas_Object *bx, Eina_Bool locked);
static Evas_Object *_icon(Evas_Object *base, const char *name);

static Eina_Bool
_backend_init(const char *back)
{
   const Eina_List *l;
   const char *name;

   if (!back) back = "PULSEAUDIO";
   if (emix_backend_set(back)) return EINA_TRUE;
   EINA_LIST_FOREACH(emix_backends_available(), l, name)
     {
        if (emix_backend_set(name)) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static char *
_cb_vu_format_cb(double v EINA_UNUSED)
{
   return "";
}

static void
_cb_sink_port_change(void *data,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Emix_Port *port = data;
   Evas_Object *fr = evas_object_data_get(obj, "parent");
   Emix_Sink *sink = evas_object_data_get(fr, "sink");
   elm_object_text_set(obj, port->description);
   emix_sink_port_set(sink, port);
}

static void
_cb_sink_volume_change(void *data,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink *sink = evas_object_data_get(fr, "sink");
   double vol = elm_slider_value_get(obj);
   VOLSET(vol, sink->volume, sink, emix_sink_volume_set);
   elm_slider_value_set(obj, vol);
}

static void
_cb_sink_volume_channel_change(void *data,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink *sink = evas_object_data_get(fr, "sink");
   double vol = elm_slider_value_get(obj);
   sink->volume.volumes[(uintptr_t)evas_object_data_get(obj, "channel")] = vol;
   elm_slider_value_set(obj, vol);
   emix_sink_volume_set(sink, &sink->volume);
}


static void
_cb_sink_mute_change(void *data,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink *sink = evas_object_data_get(fr, "sink");
   Evas_Object *lock = evas_object_data_get(fr, "lock");
   Eina_Bool mute = elm_check_state_get(obj);
   Eina_List *l;
   Evas_Object *o;

   if (lock && !elm_check_state_get(lock))
     {
        EINA_LIST_FOREACH(evas_object_data_get(fr, "volumes"), l, o)
          {
             elm_object_disabled_set(o, mute);
             o = evas_object_data_get(o, "lb");
             elm_object_disabled_set(o, mute);
          }
     }
   else
     {
        o = evas_object_data_get(fr, "volume");
        elm_object_disabled_set(o, mute);
     }
   emix_sink_mute_set(sink, mute);
}

static void
_cb_sink_lock_change(void *data,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink *sink = evas_object_data_get(fr, "sink");
   Evas_Object *bx = evas_object_data_get(fr, "volume_bx");
   Eina_Bool lock = elm_check_state_get(obj);
   if (lock)
     VOLSET(sink->volume.volumes[0], sink->volume,
            sink, emix_sink_volume_set);
   _emix_sink_volume_fill(sink, fr, bx, lock);
}

static void
_cb_sink_default_change(void *data,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink *sink = evas_object_data_get(fr, "sink");
   Eina_Bool is_default = elm_check_state_get(obj);
   if (is_default) {
     emix_sink_default_set(sink);
     elm_object_disabled_set(obj, EINA_TRUE);
  }
}

static void
_emix_sink_volume_fill(Emix_Sink *sink, Evas_Object *fr, Evas_Object *bx, Eina_Bool locked)
{
   Evas_Object *bxhv, /**ic,*/ *bxv, *sl, *ck, *lb, *hv;
   Eina_List *sls = NULL;
   unsigned int i;

   eina_list_free(evas_object_data_get(fr, "volumes"));
   bxhv = evas_object_data_get(fr, "mutebox");
   hv = evas_object_data_get(fr, "port");
   if ((bxhv) && (hv)) elm_box_unpack(bxhv, hv);
   elm_box_clear(bx);

   bxv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxv, EVAS_HINT_FILL, 0.5);
   elm_box_horizontal_set(bxv, 0);
   elm_box_pack_end(bx, bxv);
   evas_object_show(bxv);

   bxhv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxhv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxhv, EVAS_HINT_FILL, 0.5);
   elm_box_horizontal_set(bxhv, 1);
   elm_box_pack_end(bxv, bxhv);
   evas_object_show(bxhv);

//   ic = _icon(win, "audio-volume");
//   elm_box_pack_end(bxhv, ic);
//   evas_object_show(ic);

   if (locked)
     {
        sl = elm_slider_add(bx);
        evas_object_data_set(fr, "volume", sl);
        elm_slider_min_max_set(sl, 0.0, emix_max_volume_get());
        elm_slider_span_size_set(sl, emix_max_volume_get() * elm_config_scale_get());
        elm_slider_unit_format_set(sl, "%1.0f");
        elm_slider_indicator_format_set(sl, "%1.0f");
        evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_slider_value_set(sl, sink->volume.volumes[0]);
        elm_box_pack_end(bxhv, sl);
        evas_object_show(sl);
        evas_object_smart_callback_add(sl, "changed", _cb_sink_volume_change, fr);
        elm_object_disabled_set(sl, sink->mute);
     }
   else
     {
        for (i = 0; i < sink->volume.channel_count; ++i)
          {
             lb = elm_label_add(bx);
             if (!sink->volume.channel_names)
               {
                  char buf[1024];
                  snprintf(buf, sizeof(buf), "Channel %d", i);
                  elm_object_text_set(lb, buf);
               }
             else
               elm_object_text_set(lb, sink->volume.channel_names[i]);
             evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.5);
             evas_object_size_hint_align_set(lb, 0.0, 0.5);
             elm_box_pack_end(bxhv, lb);
             elm_object_disabled_set(lb, sink->mute);
             evas_object_show(lb);

             sl = elm_slider_add(bx);
             evas_object_data_set(sl, "lb", lb);
             evas_object_data_set(sl, "channel", (uintptr_t *)(uintptr_t)i);
             elm_slider_min_max_set(sl, 0.0, emix_max_volume_get());
             elm_slider_span_size_set(sl, (emix_max_volume_get()) * elm_config_scale_get());
             elm_slider_unit_format_set(sl, "%1.0f");
             elm_slider_indicator_format_set(sl, "%1.0f");
             evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
             elm_slider_value_set(sl, sink->volume.volumes[i]);
             elm_box_pack_end(bxhv, sl);
             evas_object_show(sl);
             evas_object_smart_callback_add(sl, "changed", _cb_sink_volume_channel_change, fr);
             elm_object_disabled_set(sl, sink->mute);
             sls = eina_list_append(sls, sl);
          }
     }
   evas_object_data_set(fr, "volumes", sls);

   bxhv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxhv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxhv, EVAS_HINT_FILL, 0.5);
   elm_box_align_set(bxhv, 0.0, 0.5);
   elm_box_horizontal_set(bxhv, 1);
   elm_box_pack_end(bxv, bxhv);
   evas_object_data_set(fr, "mutebox", bxhv);
   evas_object_show(bxhv);

   ck = elm_check_add(bx);
   evas_object_data_set(fr, "mute", ck);
   elm_object_text_set(ck, "Mute");
   elm_check_state_set(ck, sink->mute);
   elm_box_pack_end(bxhv, ck);
   evas_object_show(ck);
   evas_object_smart_callback_add(ck, "changed", _cb_sink_mute_change, fr);

   if (sink->volume.channel_count > 1)
     {
        ck = elm_check_add(bx);
        elm_object_text_set(ck, "Lock");
        evas_object_data_set(fr, "lock", ck);
        elm_check_state_set(ck, locked);
        elm_box_pack_end(bxhv, ck);
        evas_object_show(ck);
        evas_object_smart_callback_add(ck, "changed", _cb_sink_lock_change, fr);
     }

   ck = elm_check_add(bx);
   evas_object_data_set(fr, "default", ck);
   elm_object_text_set(ck, "Default");
   elm_check_state_set(ck, sink->default_sink);
   elm_box_pack_end(bxhv, ck);
   evas_object_show(ck);
   evas_object_smart_callback_add(ck, "changed", _cb_sink_default_change, fr);

   if (hv) elm_box_pack_end(bxhv, hv);
}

static void
_emix_sink_add(Emix_Sink *sink)
{
   Evas_Object *bxv, *bx, *lb, *hv, *ic, *fr, *sep, *vu, *pd;
   const Eina_List *l;
   Emix_Port *port;
   Eina_Bool locked = EINA_TRUE;
   Mon_Data *md;
   char *icname;
   unsigned int i;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_object_style_set(fr, "pad_large");
   sink_list = eina_list_append(sink_list, fr);
   evas_object_data_set(fr, "sink", sink);

   bxv = elm_box_add(win);
   evas_object_size_hint_weight_set(bxv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxv, EVAS_HINT_FILL, 0.0);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   icname = _sink_icon_find(sink->name);
   if (!icname) icname = strdup("speaker-box");
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, icname);
   free(icname);
   evas_object_size_hint_min_set(ic, ELM_SCALE_SIZE(20), ELM_SCALE_SIZE(20));
   evas_object_size_hint_weight_set(ic, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_FILL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   elm_object_text_set(lb, eina_slstr_printf("<heading>%s</>", sink->name));
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, 0.0, 0.5);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   /* Compare each volume level and check if they differ. If they differ unlock
   the volume control and let user set each channel volume level */
   for (i = 1; i < sink->volume.channel_count; ++i)
     {
        if (sink->volume.volumes[i - 1] != sink->volume.volumes[i])
          {
             locked = EINA_FALSE;
             break;
          }
     }

   bx = elm_box_add(bxv);
   evas_object_data_set(fr, "volume_bx", bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   _emix_sink_volume_fill(sink, fr, bx, locked);

   bx = evas_object_data_get(fr, "mutebox");
   hv = elm_hoversel_add(win);
   evas_object_size_hint_weight_set(hv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(hv, EVAS_HINT_FILL, 0.5);
   evas_object_data_set(hv, "parent", fr);
   evas_object_data_set(fr, "port", hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(sink->ports, l, port)
     {
        elm_hoversel_item_add(hv, port->description,
                              NULL, ELM_ICON_NONE,
                              _cb_sink_port_change, port);
        if (port->active) elm_object_text_set(hv, port->description);
     }
   elm_box_pack_end(bx, hv);
   evas_object_show(hv);

   vu = elm_progressbar_add(win);
   elm_progressbar_unit_format_function_set(vu, _cb_vu_format_cb, NULL);
   evas_object_data_set(fr, "vu", vu);
   evas_object_size_hint_weight_set(vu, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(vu, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, vu);
   evas_object_show(vu);

   elm_object_content_set(fr, bxv);
   evas_object_show(bxv);

   elm_box_pack_end(sink_box, fr);
   evas_object_show(fr);

   sep = elm_separator_add(win);
   evas_object_data_set(fr, "extra", sep);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_size_hint_weight_set(sep, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sep, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(sink_box, sep);
   evas_object_show(sep);

   md = calloc(1, sizeof(Mon_Data));
   if (md)
     {
        md->sink = sink;
        md->fr = fr;
        md->vu = vu;
        emix_event_callback_add(_cb_emix_sink_monitor_event, md);
        _monitor_data_list = eina_list_append(_monitor_data_list, md);
     }
   emix_sink_monitor(sink, EINA_TRUE);
}

static void
_emix_sink_del(Emix_Sink *sink)
{
   Eina_List *l, *ll;
   Evas_Object *fr;
   Mon_Data *md;

   emix_sink_monitor(sink, EINA_FALSE);
   EINA_LIST_FOREACH_SAFE(_monitor_data_list, l, ll, md)
     {
        if (md->sink == sink)
          {
             emix_event_callback_del(_cb_emix_sink_monitor_event, md);
             _monitor_data_list = eina_list_remove_list(_monitor_data_list, l);
             if (md->animator) ecore_animator_del(md->animator);
             free(md);
          }
     }
   EINA_LIST_FOREACH(sink_list, l, fr)
     {
        if (evas_object_data_get(fr, "sink") == sink)
          {
             sink_list = eina_list_remove_list(sink_list, l);
             evas_object_del(evas_object_data_get(fr, "extra"));
             evas_object_del(fr);
             return;
          }
     }
}

static void
_emix_sink_change(Emix_Sink *sink)
{
   const Eina_List *l;
   Eina_List *ll;
   Evas_Object *fr, *hv, *ck, *sl, *lb;
   Emix_Port *port;

   EINA_LIST_FOREACH(sink_list, l, fr)
     {
        if (evas_object_data_get(fr, "sink") == sink) break;
     }
   if (!l) return;
   hv = evas_object_data_get(fr, "port");
   elm_hoversel_clear(hv);
   EINA_LIST_FOREACH(sink->ports, l, port)
     {
        elm_hoversel_item_add(hv, port->description,
                              NULL, ELM_ICON_NONE,
                              _cb_sink_port_change, port);
        if (port->active) elm_object_text_set(hv, port->description);
     }
   ck = evas_object_data_get(fr, "lock");

   if (ck && !elm_check_state_get(ck))
     {
        ck = evas_object_data_get(fr, "mute");
        elm_check_state_set(ck, sink->mute);
        EINA_LIST_FOREACH(evas_object_data_get(fr, "volumes"), ll, sl)
          {
             elm_slider_value_set(sl,
                                  sink->volume.volumes[(uintptr_t)evas_object_data_get(sl, "channel")]);
             elm_object_disabled_set(sl, sink->mute);
             lb = evas_object_data_get(sl, "lb");
             elm_object_disabled_set(lb, sink->mute);
          }
     }
   else
     {
        ck = evas_object_data_get(fr, "mute");
        elm_check_state_set(ck, sink->mute);

        sl = evas_object_data_get(fr, "volume");
        elm_slider_value_set(sl, sink->volume.volumes[0]);
        elm_object_disabled_set(sl, sink->mute);
     }

     ck = evas_object_data_get(fr, "default");
     elm_check_state_set(ck, sink->default_sink);
     elm_object_disabled_set(ck, sink->default_sink);
}

//////////////////////////////////////////////////////////////////////////////
static void _emix_sink_input_volume_fill(Emix_Sink_Input *input, Evas_Object *fr, Evas_Object *bx, Eina_Bool locked);

static void
_cb_sink_input_port_change(void *data,
                           Evas_Object *obj,
                           void *event_info)
{
   Elm_Object_Item *it = event_info;
   Emix_Sink *sink = data;
   const char *icname = NULL;
   Evas_Object *ic;
   Evas_Object *fr = evas_object_data_get(obj, "parent");
   Emix_Sink_Input *input = evas_object_data_get(fr, "input");
   elm_object_text_set(obj, sink->name);

   elm_hoversel_item_icon_get(it, &icname, NULL, NULL);
   ic = elm_object_part_content_get(obj, "icon");
   evas_object_del(ic);
   ic = _icon(win, icname);
   elm_object_part_content_set(obj, "icon", ic);
   evas_object_show(ic);

   emix_sink_input_sink_change(input, sink);
}

static void
_cb_sink_input_volume_change(void *data,
                             Evas_Object *obj,
                             void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink_Input *input = evas_object_data_get(fr, "input");
   double vol = elm_slider_value_get(obj);
   VOLSET(vol, input->volume, input, emix_sink_input_volume_set);
   elm_slider_value_set(obj, vol);
}

static void
_cb_sink_input_volume_channel_change(void *data,
                                     Evas_Object *obj,
                                     void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink_Input *input = evas_object_data_get(fr, "input");
   double vol = elm_slider_value_get(obj);
   input->volume.volumes[(uintptr_t)evas_object_data_get(obj, "channel")] = vol;
   elm_slider_value_set(obj, vol);
   emix_sink_input_volume_set(input, &input->volume);
}

static void
_cb_sink_input_volume_drag_stop(void *data,
                                Evas_Object *obj,
                                void *event EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink_Input *input = evas_object_data_get(fr, "input");
   int vol = input->volume.volumes[0];
   elm_slider_value_set(obj, vol);
}

static void
_cb_sink_input_volume_channel_drag_stop(void *data,
                                        Evas_Object *obj,
                                        void *event EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink_Input *input = evas_object_data_get(fr, "input");
   int vol = input->volume.volumes[(uintptr_t)evas_object_data_get(obj, "channel")];
   elm_slider_value_set(obj, vol);
}

static void
_cb_sink_input_mute_change(void *data,
                           Evas_Object *obj,
                           void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink_Input *input = evas_object_data_get(fr, "input");
   Evas_Object *sl;
   Evas_Object *lock = evas_object_data_get(fr, "lock");
   Eina_Bool mute = elm_check_state_get(obj);
   Evas_Object *lb;
   Eina_List *l;

   if (lock && !elm_check_state_get(lock))
     {
        EINA_LIST_FOREACH(evas_object_data_get(fr, "volumes"), l, sl)
          {
             elm_object_disabled_set(sl, mute);
             lb = evas_object_data_get(sl, "lb");
             elm_object_disabled_set(lb, mute);
          }
     }
   else
     {
        sl = evas_object_data_get(fr, "volume");
        elm_object_disabled_set(sl, mute);
     }
   emix_sink_input_mute_set(input, mute);
}

static void
_cb_sink_input_lock_change(void *data,
                           Evas_Object *obj,
                           void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Sink_Input *input = evas_object_data_get(fr, "input");
   Evas_Object *bx = evas_object_data_get(fr, "volume_bx");
   Eina_Bool lock = elm_check_state_get(obj);
   if (lock)
     VOLSET(input->volume.volumes[0], input->volume,
            input, emix_sink_input_volume_set);
   _emix_sink_input_volume_fill(input, fr, bx, lock);
}

static void
_emix_sink_input_volume_fill(Emix_Sink_Input *input, Evas_Object *fr, Evas_Object *bx, Eina_Bool locked)
{
   Evas_Object *bxhv, *bxv, /**ic,*/ *lb, *sl = NULL, *ck, *hv;
   unsigned int i;
   Eina_List *sls = NULL, *l;

   eina_list_free(evas_object_data_get(fr, "volumes"));
   bxhv = evas_object_data_get(fr, "mutebox");
   hv = evas_object_data_get(fr, "port");
   if ((bxhv) && (hv)) elm_box_unpack(bxhv, hv);
   elm_box_clear(bx);

   bxv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxv, EVAS_HINT_FILL, 0.5);
   elm_box_horizontal_set(bxv, 0);
   elm_box_pack_end(bx, bxv);
   evas_object_show(bxv);

   bxhv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxhv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxhv, EVAS_HINT_FILL, 0.5);
   elm_box_horizontal_set(bxhv, 1);
   elm_box_pack_end(bxv, bxhv);
   evas_object_show(bxhv);

//   ic = _icon(win, "audio-volume");
//   elm_box_pack_end(bxhv, ic);
//   evas_object_show(ic);

   if (locked)
     {
        sl = elm_slider_add(bx);
        evas_object_data_set(fr, "volume", sl);
        elm_slider_min_max_set(sl, 0.0, emix_max_volume_get());
        elm_slider_span_size_set(sl, (emix_max_volume_get()) * elm_config_scale_get());
        elm_slider_unit_format_set(sl, "%1.0f");
        elm_slider_indicator_format_set(sl, "%1.0f");
        evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
        elm_slider_value_set(sl, input->volume.volumes[0]);
        elm_box_pack_end(bxhv, sl);
        evas_object_show(sl);
        evas_object_smart_callback_add(sl, "changed",
                                       _cb_sink_input_volume_change, fr);
        evas_object_smart_callback_add(sl, "slider,drag,stop",
                                       _cb_sink_input_volume_drag_stop, fr);

     }
   else
     {
        for (i = 0; i < input->volume.channel_count; ++i)
          {
             lb = elm_label_add(bx);
             if (!input->volume.channel_names)
               {
                  char buf[1024];
                  snprintf(buf, sizeof(buf), "Channel %d", i);
                  elm_object_text_set(lb, buf);
               }
             else
               elm_object_text_set(lb, input->volume.channel_names[i]);
             evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.5);
             evas_object_size_hint_align_set(lb, 0.0, 0.5);
             elm_box_pack_end(bxhv, lb);
             elm_object_disabled_set(lb, input->mute);
             evas_object_show(lb);

             sl = elm_slider_add(bx);
             evas_object_data_set(sl, "lb", lb);
             evas_object_data_set(sl, "channel", (uintptr_t *)(uintptr_t)i);
             elm_slider_min_max_set(sl, 0.0, emix_max_volume_get());
             elm_slider_span_size_set(sl, (emix_max_volume_get()) * elm_config_scale_get());
             elm_slider_unit_format_set(sl, "%1.0f");
             elm_slider_indicator_format_set(sl, "%1.0f");
             evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
             elm_slider_value_set(sl, input->volume.volumes[i]);
             elm_box_pack_end(bxhv, sl);
             evas_object_show(sl);
             evas_object_smart_callback_add(sl, "changed",
                                            _cb_sink_input_volume_channel_change, fr);
             evas_object_smart_callback_add(sl, "slider,drag,stop",
                                            _cb_sink_input_volume_channel_drag_stop, fr);
             elm_object_disabled_set(sl, input->mute);
             sls = eina_list_append(sls, sl);
          }
        sl = NULL;
     }
   evas_object_data_set(fr, "volumes", sls);

   bxhv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxhv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxhv, EVAS_HINT_FILL, 0.5);
   elm_box_align_set(bxhv, 0.0, 0.5);
   elm_box_horizontal_set(bxhv, 1);
   elm_box_pack_end(bxv, bxhv);
   evas_object_data_set(fr, "mutebox", bxhv);
   evas_object_show(bxhv);

   ck = elm_check_add(bx);
   evas_object_data_set(fr, "mute", ck);
   elm_object_text_set(ck, "Mute");
   elm_check_state_set(ck, input->mute);
   if (sl) elm_object_disabled_set(sl, input->mute);
   else if (sls)
     {
        EINA_LIST_FOREACH(sls, l, sl)
          {
             elm_object_disabled_set(sl, input->mute);
          }
     }
   elm_box_pack_end(bxhv, ck);
   evas_object_show(ck);
   evas_object_smart_callback_add(ck, "changed",
                                  _cb_sink_input_mute_change, fr);

   if (input->volume.channel_count > 1)
     {
        ck = elm_check_add(bx);
        elm_object_disabled_set(ck, 1);
        evas_object_data_set(fr, "lock", ck);
        elm_object_text_set(ck, "Lock");
        elm_check_state_set(ck, locked);
        elm_box_pack_end(bxhv, ck);
        evas_object_show(ck);
        evas_object_smart_callback_add(ck, "changed",
                                       _cb_sink_input_lock_change, fr);
     }

   if (hv) elm_box_pack_end(bxhv, hv);
}

static void
_emix_sink_input_add(Emix_Sink_Input *input)
{
   Evas_Object *bxv, *bx, *lb, *hv, *ic, *fr, *sep, *vu, *pd;
   const Eina_List *l;
   Emix_Sink *sink;
   Eina_Bool locked = EINA_TRUE;
   Mon_Data *md;
   unsigned int i;
   const char *icname;

   if (!input->sink) return;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_object_style_set(fr, "pad_large");
   sink_input_list = eina_list_append(sink_input_list, fr);
   evas_object_data_set(fr, "input", input);

   bxv = elm_box_add(win);
   evas_object_size_hint_weight_set(bxv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxv, EVAS_HINT_FILL, 0.0);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   icname = input->icon;
   if (!icname) icname = "multimedia-player";
   elm_icon_standard_set(ic, icname);
   evas_object_size_hint_min_set(ic, ELM_SCALE_SIZE(20), ELM_SCALE_SIZE(20));
   evas_object_size_hint_weight_set(ic, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_FILL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   elm_object_text_set(lb, eina_slstr_printf("<heading>%s</>", input->name));
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, 0.0, 0.5);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bx = elm_box_add(win);
   evas_object_data_set(fr, "volume_bx", bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   /* Compare each volume level and check if they differ. If they differ unlock
   the volume control and let user set each channel volume level */
   for (i = 1; i < input->volume.channel_count; ++i)
     {
        if (input->volume.volumes[i - 1] != input->volume.volumes[i])
          {
             locked = EINA_FALSE;
             break;
          }
     }

   _emix_sink_input_volume_fill(input, fr, bx, locked);

   bx = evas_object_data_get(fr, "mutebox");
   hv = elm_hoversel_add(win);
   evas_object_size_hint_weight_set(hv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(hv, EVAS_HINT_FILL, 0.5);
   evas_object_data_set(hv, "parent", fr);
   evas_object_data_set(fr, "port", hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(emix_sinks_get(), l, sink)
     {
        char *icname2 = _sink_icon_find(sink->name);
        if (!icname2) icname2 = strdup("speaker-box");
        elm_hoversel_item_add(hv, sink->name,
                              icname2, ELM_ICON_STANDARD,
                              _cb_sink_input_port_change, sink);
        if (input->sink == sink)
          {
             ic = _icon(win, icname2);
             elm_object_part_content_set(hv, "icon", ic);
             evas_object_show(ic);
             elm_object_text_set(hv, sink->name);
          }
        free(icname2);
     }
   elm_box_pack_end(bx, hv);
   evas_object_show(hv);

   vu = elm_progressbar_add(win);
   elm_progressbar_unit_format_function_set(vu, _cb_vu_format_cb, NULL);
   evas_object_data_set(fr, "vu", vu);
   evas_object_size_hint_weight_set(vu, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(vu, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, vu);
   evas_object_show(vu);

   elm_object_content_set(fr, bxv);
   evas_object_show(bxv);

   elm_box_pack_end(sink_input_box, fr);
   evas_object_show(fr);

   sep = elm_separator_add(win);
   evas_object_data_set(fr, "extra", sep);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_size_hint_weight_set(sep, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sep, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(sink_input_box, sep);
   evas_object_show(sep);

   md = calloc(1, sizeof(Mon_Data));
   if (md)
     {
        md->input = input;
        md->fr = fr;
        md->vu = vu;
        emix_event_callback_add(_cb_emix_sink_input_monitor_event, md);
        _monitor_data_list = eina_list_append(_monitor_data_list, md);
     }
   emix_sink_input_monitor(input, EINA_TRUE);
}

static void
_emix_sink_input_del(Emix_Sink_Input *input)
{
   Eina_List *l, *ll;
   Evas_Object *fr;
   Mon_Data *md;

   emix_sink_input_monitor(input, EINA_FALSE);
   EINA_LIST_FOREACH_SAFE(_monitor_data_list, l, ll, md)
     {
        if (md->input == input)
          {
             emix_event_callback_del(_cb_emix_sink_input_monitor_event, md);
             _monitor_data_list = eina_list_remove_list(_monitor_data_list, l);
             if (md->animator) ecore_animator_del(md->animator);
             free(md);
          }
     }
   EINA_LIST_FOREACH(sink_input_list, l, fr)
     {
        if (evas_object_data_get(fr, "input") == input)
          {
             sink_input_list = eina_list_remove_list(sink_input_list, l);
             evas_object_del(evas_object_data_get(fr, "extra"));
             evas_object_del(fr);
             return;
          }
     }
}

static void
_emix_sink_input_change(Emix_Sink_Input *input)
{
   const Eina_List *l;
   Eina_List *ll;
   Evas_Object *fr, *hv, *ck, *sl, *lb, *ic;
   Emix_Sink *sink;

   EINA_LIST_FOREACH(sink_input_list, l, fr)
     {
        if (evas_object_data_get(fr, "input") == input) break;
     }
   if (!l) return;
   hv = evas_object_data_get(fr, "port");
   elm_hoversel_clear(hv);
   EINA_LIST_FOREACH(emix_sinks_get(), l, sink)
     {
        char *icname2 = _sink_icon_find(sink->name);
        if (!icname2) icname2 = strdup("speaker-box");
        elm_hoversel_item_add(hv, sink->name,
                              icname2, ELM_ICON_STANDARD,
                              _cb_sink_input_port_change, sink);
        if (input->sink == sink)
          {
             ic = _icon(win, icname2);
             elm_object_part_content_set(hv, "icon", ic);
             evas_object_show(ic);
             elm_object_text_set(hv, sink->name);
          }
        free(icname2);
     }

   ck = evas_object_data_get(fr, "lock");

   if (ck && !elm_check_state_get(ck))
     {
        ck = evas_object_data_get(fr, "mute");
        elm_check_state_set(ck, input->mute);
        EINA_LIST_FOREACH(evas_object_data_get(fr, "volumes"), ll, sl)
          {
             elm_slider_value_set(sl,
                                  input->volume.volumes[(uintptr_t)evas_object_data_get(sl, "channel")]);
             elm_object_disabled_set(sl, input->mute);
             lb = evas_object_data_get(sl, "lb");
             elm_object_disabled_set(lb, input->mute);
          }
     }
   else
     {
        ck = evas_object_data_get(fr, "mute");
        elm_check_state_set(ck, input->mute);

        sl = evas_object_data_get(fr, "volume");
        elm_slider_value_set(sl, input->volume.volumes[0]);
        elm_object_disabled_set(sl, input->mute);
     }
}

//////////////////////////////////////////////////////////////////////////////
static void _emix_source_volume_fill(Emix_Source *source, Evas_Object *fr, Evas_Object *bx, Eina_Bool locked);

static void
_cb_source_volume_change(void *data,
                         Evas_Object *obj,
                         void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Source *source = evas_object_data_get(fr, "source");
   double vol = elm_slider_value_get(obj);
   VOLSET(vol, source->volume, source, emix_source_volume_set);
   elm_slider_value_set(obj, vol);
}

static void
_cb_source_volume_channel_change(void *data,
                         Evas_Object *obj,
                         void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Source *source = evas_object_data_get(fr, "source");
   double vol = elm_slider_value_get(obj);
   source->volume.volumes[(uintptr_t)evas_object_data_get(obj, "channel")] = vol;
   elm_slider_value_set(obj, vol);
   emix_source_volume_set(source, &source->volume);
}


static void
_cb_source_volume_drag_stop(void *data,
                            Evas_Object *obj,
                            void *event EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Source *source = evas_object_data_get(fr, "source");
   int vol = source->volume.volumes[0];
   elm_slider_value_set(obj, vol);
}

static void
_cb_source_volume_channel_drag_stop(void *data,
                            Evas_Object *obj,
                            void *event EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Source *source = evas_object_data_get(fr, "source");
   int vol = source->volume.volumes[(uintptr_t)evas_object_data_get(obj, "channel")];
   elm_slider_value_set(obj, vol);
}

static void
_cb_source_mute_change(void *data,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Source *source = evas_object_data_get(fr, "source");
   Evas_Object *sl;
   Evas_Object *lock = evas_object_data_get(fr, "lock");
   Eina_Bool mute = elm_check_state_get(obj);
   Evas_Object *lb;
   Eina_List *l;

   if (lock && !elm_check_state_get(lock))
     {
        EINA_LIST_FOREACH(evas_object_data_get(fr, "volumes"), l, sl)
          {
             elm_object_disabled_set(sl, mute);
             lb = evas_object_data_get(sl, "lb");
             elm_object_disabled_set(lb, mute);
          }
     }
   else
     {
        sl = evas_object_data_get(fr, "volume");
        elm_object_disabled_set(sl, mute);
     }
   emix_source_mute_set(source, mute);
}

static void
_cb_source_lock_change(void *data,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Source *source = evas_object_data_get(fr, "source");
   Evas_Object *bx = evas_object_data_get(fr, "volume_bx");
   Eina_Bool lock = elm_check_state_get(obj);
   if (lock)
     VOLSET(source->volume.volumes[0], source->volume,
            source, emix_source_volume_set);
   _emix_source_volume_fill(source, fr, bx, lock);
}

static void
_cb_source_default_change(void *data,
                     Evas_Object *obj,
                     void *event_info EINA_UNUSED)
{
   Evas_Object *fr = data;
   Emix_Source *source = evas_object_data_get(fr, "source");
   Eina_Bool is_default = elm_check_state_get(obj);
   if (is_default) {
     emix_source_default_set(source);
     elm_object_disabled_set(obj, EINA_TRUE);
  }
}

static void
_emix_source_volume_fill(Emix_Source *source, Evas_Object *fr, Evas_Object *bx, Eina_Bool locked)
{
   Evas_Object *bxhv, *bxv, *lb, /**ic,*/ *sl, *ck, *hv;
   Eina_List *sls = NULL;
   unsigned int i;

   eina_list_free(evas_object_data_get(fr, "volumes"));
   bxhv = evas_object_data_get(fr, "mutebox");
   hv = evas_object_data_get(fr, "port");
   if ((bxhv) && (hv)) elm_box_unpack(bxhv, hv);
   elm_box_clear(bx);

   bxv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxv, EVAS_HINT_FILL, 0.5);
   elm_box_horizontal_set(bxv, 0);
   elm_box_pack_end(bx, bxv);
   evas_object_show(bxv);

   bxhv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxhv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxhv, EVAS_HINT_FILL, 0.5);
   elm_box_horizontal_set(bxhv, 1);
   elm_box_pack_end(bxv, bxhv);
   evas_object_show(bxhv);

//   ic = _icon(win, "audio-input-microphone");
//   elm_box_pack_end(bxhv, ic);
//   evas_object_show(ic);

   if (locked)
     {
        sl = elm_slider_add(bx);
        evas_object_data_set(fr, "volume", sl);
        elm_slider_min_max_set(sl, 0.0, emix_max_volume_get());
        elm_slider_span_size_set(sl, (emix_max_volume_get()) * elm_config_scale_get());
        elm_slider_unit_format_set(sl, "%1.0f");
        elm_slider_indicator_format_set(sl, "%1.0f");
        evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_slider_value_set(sl, source->volume.volumes[0]);
        elm_box_pack_end(bxhv, sl);
        evas_object_show(sl);
        evas_object_smart_callback_add(sl, "changed",
                                       _cb_source_volume_change, fr);
        evas_object_smart_callback_add(sl, "slider,drag,stop",
                                       _cb_source_volume_drag_stop, fr);
        elm_object_disabled_set(sl, source->mute);
     }
   else
     {
        for (i = 0; i < source->volume.channel_count; ++i)
          {
             lb = elm_label_add(bx);
             if (!source->volume.channel_names)
               {
                  char buf[1024];
                  snprintf(buf, sizeof(buf), "Channel %d", i);
                  elm_object_text_set(lb, buf);
               }
             else
               elm_object_text_set(lb, source->volume.channel_names[i]);
             evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.5);
             evas_object_size_hint_align_set(lb, 0.0, 0.5);
             elm_box_pack_end(bxhv, lb);
             elm_object_disabled_set(lb, source->mute);
             evas_object_show(lb);

             sl = elm_slider_add(bx);
             evas_object_data_set(sl, "lb", lb);
             evas_object_data_set(sl, "channel", (uintptr_t *)(uintptr_t)i);
             elm_slider_min_max_set(sl, 0.0, emix_max_volume_get());
             elm_slider_span_size_set(sl, (emix_max_volume_get()) * elm_config_scale_get());
             elm_slider_unit_format_set(sl, "%1.0f");
             elm_slider_indicator_format_set(sl, "%1.0f");
             evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
             elm_slider_value_set(sl, source->volume.volumes[i]);
             elm_box_pack_end(bxhv, sl);
             evas_object_show(sl);
             evas_object_smart_callback_add(sl, "changed",
                                            _cb_source_volume_channel_change, fr);
             evas_object_smart_callback_add(sl, "slider,drag,stop",
                                            _cb_source_volume_channel_drag_stop, fr);
             elm_object_disabled_set(sl, source->mute);
             sls = eina_list_append(sls, sl);
          }
     }
   evas_object_data_set(fr, "volumes", sls);

   bxhv = elm_box_add(bx);
   evas_object_size_hint_weight_set(bxhv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxhv, EVAS_HINT_FILL, 0.5);
   elm_box_align_set(bxhv, 0.0, 0.5);
   elm_box_horizontal_set(bxhv, 1);
   elm_box_pack_end(bxv, bxhv);
   evas_object_data_set(fr, "mutebox", bxhv);
   evas_object_show(bxhv);

   ck = elm_check_add(bx);
   evas_object_data_set(fr, "mute", ck);
   elm_object_text_set(ck, "Mute");
   elm_check_state_set(ck, source->mute);
   elm_box_pack_end(bxhv, ck);
   evas_object_show(ck);
   evas_object_smart_callback_add(ck, "changed",
                                  _cb_source_mute_change, fr);

   if (source->volume.channel_count > 1)
     {
        ck = elm_check_add(bx);
        elm_object_disabled_set(ck, 1);
        evas_object_data_set(fr, "lock", ck);
        elm_object_text_set(ck, "Lock");
        elm_check_state_set(ck, locked);
        elm_box_pack_end(bxhv, ck);
        evas_object_show(ck);
        evas_object_smart_callback_add(ck, "changed",
                                       _cb_source_lock_change, fr);
     }

   ck = elm_check_add(bx);
   evas_object_data_set(fr, "default", ck);
   elm_object_text_set(ck, "Default");
   elm_check_state_set(ck, source->default_source);
   elm_box_pack_end(bxhv, ck);
   evas_object_show(ck);
   evas_object_smart_callback_add(ck, "changed", _cb_source_default_change, fr);

   if (hv) elm_box_pack_end(bxhv, hv);
}


static void
_emix_source_add(Emix_Source *source)
{
   Evas_Object *bxv, *bx, *ic, *fr, *lb, *sep, *vu, *pd;
   unsigned int i;
   Eina_Bool locked = EINA_TRUE;
   Mon_Data *md;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_object_style_set(fr, "pad_large");
   source_list = eina_list_append(source_list, fr);
   evas_object_data_set(fr, "source", source);

   bxv = elm_box_add(win);
   evas_object_size_hint_weight_set(bxv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxv, EVAS_HINT_FILL, 0.0);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "audio-input-microphone");
   evas_object_size_hint_min_set(ic, ELM_SCALE_SIZE(20), ELM_SCALE_SIZE(20));
   evas_object_size_hint_weight_set(ic, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_FILL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   pd = elm_frame_add(win);
   elm_object_style_set(pd, "pad_medium");
   elm_box_pack_end(bx, pd);
   evas_object_show(pd);

   lb = elm_label_add(win);
   elm_object_text_set(lb, eina_slstr_printf("<heading>%s</>", source->name));
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, 0.0, 0.5);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   bx = elm_box_add(win);
   evas_object_data_set(fr, "volume_bx", bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   /* Compare each volume level and check if they differ. If they differ unlock
   the volume control and let user set each channel volume level */
   for (i = 1; i < source->volume.channel_count; ++i)
     {
        if (source->volume.volumes[i - 1] != source->volume.volumes[i])
          {
             locked = EINA_FALSE;
             break;
          }
     }

   _emix_source_volume_fill(source, fr, bx, locked);

   elm_object_content_set(fr, bxv);
   evas_object_show(bxv);

   vu = elm_progressbar_add(win);
   elm_progressbar_unit_format_function_set(vu, _cb_vu_format_cb, NULL);
   evas_object_data_set(fr, "vu", vu);
   evas_object_size_hint_weight_set(vu, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(vu, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, vu);
   evas_object_show(vu);

   elm_box_pack_end(source_box, fr);
   evas_object_show(fr);

   sep = elm_separator_add(win);
   evas_object_data_set(fr, "extra", sep);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_size_hint_weight_set(sep, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sep, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(source_box, sep);
   evas_object_show(sep);

   md = calloc(1, sizeof(Mon_Data));
   if (md)
     {
        md->source = source;
        md->fr = fr;
        md->vu = vu;
        emix_event_callback_add(_cb_emix_source_monitor_event, md);
        _monitor_data_list = eina_list_append(_monitor_data_list, md);
     }
   emix_source_monitor(source, EINA_TRUE);
}

static void
_emix_source_del(Emix_Source *source)
{
   Eina_List *l, *ll;
   Evas_Object *fr;
   Mon_Data *md;

   emix_source_monitor(source, EINA_FALSE);
   EINA_LIST_FOREACH_SAFE(_monitor_data_list, l, ll, md)
     {
        if (md->source == source)
          {
             emix_event_callback_del(_cb_emix_sink_monitor_event, md);
             _monitor_data_list = eina_list_remove_list(_monitor_data_list, l);
             if (md->animator) ecore_animator_del(md->animator);
             free(md);
          }
     }
   EINA_LIST_FOREACH(source_list, l, fr)
     {
        if (evas_object_data_get(fr, "source") == source)
          {
             source_list = eina_list_remove_list(source_list, l);
             evas_object_del(evas_object_data_get(fr, "extra"));
             evas_object_del(fr);
             return;
          }
     }
}

static void
_emix_source_change(Emix_Source *source)
{
   const Eina_List *l;
   Eina_List *ll;
   Evas_Object *fr, *ck, *sl, *lb;

   EINA_LIST_FOREACH(source_list, l, fr)
     {
        if (evas_object_data_get(fr, "source") == source) break;
     }
   if (!l) return;

   ck = evas_object_data_get(fr, "lock");

   if (ck && !elm_check_state_get(ck))
     {
        ck = evas_object_data_get(fr, "mute");
        elm_check_state_set(ck, source->mute);
        EINA_LIST_FOREACH(evas_object_data_get(fr, "volumes"), ll, sl)
          {
             elm_slider_value_set(sl,
                                  source->volume.volumes[(uintptr_t)evas_object_data_get(sl, "channel")]);
             elm_object_disabled_set(sl, source->mute);
             lb = evas_object_data_get(sl, "lb");
             elm_object_disabled_set(lb, source->mute);
          }
     }
   else
     {
        ck = evas_object_data_get(fr, "mute");
        elm_check_state_set(ck, source->mute);

        sl = evas_object_data_get(fr, "volume");
        elm_slider_value_set(sl, source->volume.volumes[0]);
        elm_object_disabled_set(sl, source->mute);
     }

   ck = evas_object_data_get(fr, "default");
   elm_check_state_set(ck, source->default_source);
   elm_object_disabled_set(ck, source->default_source);
}

//////////////////////////////////////////////////////////////////////////////
static void
_cb_card_profile_change(void *data,
                        Evas_Object *obj,
                        void *event_info EINA_UNUSED)
{
   Emix_Profile *profile = data;
   Evas_Object *fr = evas_object_data_get(obj, "parent");
   Emix_Card *card = evas_object_data_get(fr, "card");
   elm_object_text_set(obj, profile->description);
   emix_card_profile_set(card, profile);
}

static void
_emix_card_add(Emix_Card *card)
{
   Evas_Object *bxv, *bx, *hv, *fr, *lb, *sep;
   Eina_List *l;
   Emix_Profile *profile;
   int cards = 0;

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 1.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_object_style_set(fr, "pad_large");
   card_list = eina_list_append(card_list, fr);
   evas_object_data_set(fr, "card", card);

   bxv = elm_box_add(win);
   evas_object_size_hint_weight_set(bxv, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bxv, EVAS_HINT_FILL, 0.0);

   lb = elm_label_add(win);
   evas_object_size_hint_align_set(lb, 0.0, EVAS_HINT_FILL);
   elm_object_text_set(lb, eina_slstr_printf("<heading>%s</>", card->name));
   evas_object_show(lb);
   elm_box_pack_end(bxv, lb);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(bxv, bx);
   evas_object_show(bx);

   hv = elm_hoversel_add(win);
   evas_object_size_hint_weight_set(hv, 1.0, 1.0);
   evas_object_size_hint_align_set(hv, 0.5, 0.5);
   evas_object_data_set(hv, "parent", fr);
   evas_object_data_set(fr, "profile", hv);
   elm_hoversel_hover_parent_set(hv, win);
   EINA_LIST_FOREACH(card->profiles, l, profile)
     {
        if (!profile->plugged) continue;
        elm_hoversel_item_add(hv, profile->description,
                              NULL, ELM_ICON_NONE,
                              _cb_card_profile_change, profile);
        if (profile->active) elm_object_text_set(hv, profile->description);
        cards++;
     }
   if (cards == 0) elm_object_text_set(hv, "Not Connected");
   evas_object_size_hint_weight_set(hv, 0.0, 0.5);
   evas_object_size_hint_align_set(hv, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, hv);
   evas_object_show(hv);

   elm_object_content_set(fr, bxv);
   evas_object_show(bxv);

   elm_box_pack_end(card_box, fr);
   evas_object_show(fr);

   sep = elm_separator_add(win);
   evas_object_data_set(fr, "extra", sep);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   evas_object_size_hint_weight_set(sep, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sep, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(card_box, sep);
   evas_object_show(sep);
}

static void
_emix_card_change(Emix_Card *card)
{
   const Eina_List *l;
   Evas_Object *fr, *hv;
   Emix_Profile *profile;

   EINA_LIST_FOREACH(card_list, l, fr)
     {
        if (evas_object_data_get(fr, "card") == card) break;
     }
   if (!l) return;
   hv = evas_object_data_get(fr, "profile");
   elm_hoversel_clear(hv);
   EINA_LIST_FOREACH(card->profiles, l, profile)
     {
        if (!profile->plugged) continue;
        elm_hoversel_item_add(hv, profile->description,
                              NULL, ELM_ICON_NONE,
                              _cb_card_profile_change, profile);
        if (profile->active) elm_object_text_set(hv, profile->description);
     }
}


static void
_emix_card_del(Emix_Card *card)
{
   Eina_List *l;
   Evas_Object *fr;
   EINA_LIST_FOREACH(card_list, l, fr)
     {
        if (evas_object_data_get(fr, "card") == card)
          {
             card_list = eina_list_remove_list(card_list, l);
             evas_object_del(evas_object_data_get(fr, "extra"));
             evas_object_del(fr);
             return;
          }
     }
}


//////////////////////////////////////////////////////////////////////////////


static void
_cb_emix_event(void *data EINA_UNUSED, enum Emix_Event event, void *event_info)
{
   switch (event)
     {
      case EMIX_READY_EVENT:
        break;
      case EMIX_DISCONNECTED_EVENT:
        elm_exit();
        break;
      case EMIX_SINK_ADDED_EVENT:
        _emix_sink_add(event_info);
        break;
      case EMIX_SINK_REMOVED_EVENT:
        _emix_sink_del(event_info);
        break;
      case EMIX_SINK_CHANGED_EVENT:
        _emix_sink_change(event_info);
        break;
      case EMIX_SINK_INPUT_ADDED_EVENT:
        _emix_sink_input_add(event_info);
        break;
      case EMIX_SINK_INPUT_REMOVED_EVENT:
        _emix_sink_input_del(event_info);
        break;
      case EMIX_SINK_INPUT_CHANGED_EVENT:
        _emix_sink_input_change(event_info);
        break;
      case EMIX_SOURCE_ADDED_EVENT:
        _emix_source_add(event_info);
        break;
      case EMIX_SOURCE_REMOVED_EVENT:
        _emix_source_del(event_info);
        break;
      case EMIX_SOURCE_CHANGED_EVENT:
        _emix_source_change(event_info);
        break;
      case EMIX_CARD_ADDED_EVENT:
        _emix_card_add(event_info);
        break;
      case EMIX_CARD_REMOVED_EVENT:
        _emix_card_del(event_info);
        break;
      case EMIX_CARD_CHANGED_EVENT:
        _emix_card_change(event_info);
        break;
      case EMIX_SINK_MONITOR_EVENT:
        break;
      case EMIX_SINK_INPUT_MONITOR_EVENT:
        break;
      case EMIX_SOURCE_MONITOR_EVENT:
        break;
      default:
        break;
     }
}

//////////////////////////////////////////////////////////////////////////////

static void
_cb_playback(void *data EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   evas_object_hide(source_scroller);
   evas_object_show(sink_input_scroller);
   evas_object_hide(sink_scroller);
   evas_object_hide(card_scroller);
   category_current = sink_input_scroller;
}

static void
_cb_outputs(void *data EINA_UNUSED,
            Evas_Object *obj EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   evas_object_hide(source_scroller);
   evas_object_hide(sink_input_scroller);
   evas_object_show(sink_scroller);
   evas_object_hide(card_scroller);
   category_current = sink_scroller;
}

static void
_cb_inputs(void *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   evas_object_show(source_scroller);
   evas_object_hide(sink_input_scroller);
   evas_object_hide(sink_scroller);
   evas_object_hide(card_scroller);
   category_current = source_scroller;
}

static void
_cb_card(void *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   evas_object_hide(source_scroller);
   evas_object_hide(sink_input_scroller);
   evas_object_hide(sink_scroller);
   evas_object_show(card_scroller);
   category_current = card_scroller;
}


//////////////////////////////////////////////////////////////////////////////

static void
_event_init(void)
{
   emix_event_callback_add(_cb_emix_event, NULL);
}

static void
_fill_source(void)
{
   const Eina_List *l;
   Emix_Source *source;

   EINA_LIST_FOREACH(emix_sources_get(), l, source)
     {
        _emix_source_add(source);
     }
}

static void
_fill_sink_input(void)
{
   const Eina_List *l;
   Emix_Sink_Input *input;

   EINA_LIST_FOREACH(emix_sink_inputs_get(), l, input)
     {
        _emix_sink_input_add(input);
     }
}

static void
_fill_sink(void)
{
   const Eina_List *l;
   Emix_Sink *sink;

   EINA_LIST_FOREACH(emix_sinks_get(), l, sink)
     {
        _emix_sink_add(sink);
     }
}

static void
_fill_card(void)
{
   const Eina_List *l;
   Emix_Card *card;

   EINA_LIST_FOREACH(emix_cards_get(), l, card)
     {
        _emix_card_add(card);
     }
}

static Evas_Object *
_icon(Evas_Object *base, const char *name)
{
   Evas_Object *ic = elm_icon_add(base);
   elm_icon_standard_set(ic, name);
   evas_object_size_hint_min_set(ic, ELM_SCALE_SIZE(20), ELM_SCALE_SIZE(20));
   evas_object_show(ic);

   return ic;
}

static void
_cb_category_selected(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                      void *event_info)
{
   Elm_Transit *trans;
   Evas_Object *category_prev = category_current;
   Elm_Object_Item *it = event_info;
   Evas_Smart_Cb func = elm_object_item_data_get(it);

   func(NULL, NULL, NULL);

   if (!category_prev) return;

   trans = elm_transit_add();
   elm_transit_object_add(trans, category_prev);
   elm_transit_object_add(trans, category_current);
   elm_transit_duration_set(trans, 0.2);
   elm_transit_effect_blend_add(trans);
   elm_transit_go(trans);
}


static void
_cb_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev;

   ev = event_info;
   if ((!ev) || (!ev->keyname)) return;

   if (!strcmp(ev->keyname, "Escape"))
     evas_object_del(win);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *tb, *lst, *ic, *sc, *rect, *bx;
   Elm_Object_Item *it;
   const char *back = NULL;

   emix_init();
   if (argc > 1) back = argv[1];
   if (!_backend_init(back)) goto done;
   _event_init();

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("emix", _("Mixer"));
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_show(tb);
   evas_object_event_callback_add(tb, EVAS_CALLBACK_KEY_DOWN, _cb_key_down, win);

   lst = elm_list_add(win);
   elm_list_mode_set(lst, ELM_LIST_LIMIT);
   evas_object_size_hint_weight_set(lst, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lst, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(lst);

   ic = _icon(win, "media-playback-start");
   it = elm_list_item_append(lst, _("Playback"), ic, NULL, NULL, _cb_playback);
   ic = _icon(win, "audio-volume");
   elm_list_item_append(lst, _("Outputs"), ic, NULL, NULL, _cb_outputs);
   ic = _icon(win, "audio-input-microphone");
   elm_list_item_append(lst, _("Inputs"), ic, NULL, NULL, _cb_inputs);
   ic = _icon(win, "audio-card");
   elm_list_item_append(lst, _("Cards"), ic, NULL, NULL, _cb_card);
   evas_object_smart_callback_add(lst, "selected", _cb_category_selected, NULL);
   elm_table_pack(tb, lst, 0, 0, 1, 1);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(rect, ELM_SCALE_SIZE(120), -1);
   elm_table_pack(tb, rect, 0, 0, 1, 1);

   sc = elm_scroller_add(win);
   source_scroller = sc;
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sc, 1, 0, 1, 1);

   sc = elm_scroller_add(win);
   sink_input_scroller = sc;
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sc, 1, 0, 1, 1);
   evas_object_show(sc);

   sc = elm_scroller_add(win);
   sink_scroller = sc;
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sc, 1, 0, 1, 1);

   sc = elm_scroller_add(win);
   card_scroller = sc;
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sc, 1, 0, 1, 1);

   bx = elm_box_add(win);
   source_box = bx;
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(source_scroller, bx);
   evas_object_show(bx);

   bx = elm_box_add(win);
   sink_input_box = bx;
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(sink_input_scroller, bx);
   evas_object_show(bx);

   bx = elm_box_add(win);
   sink_box = bx;
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(sink_scroller, bx);
   evas_object_show(bx);

   bx = elm_box_add(win);
   card_box = bx;
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(card_scroller, bx);
   evas_object_show(bx);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(rect,
                                 520 * elm_config_scale_get(),
                                 320 * elm_config_scale_get());
   elm_table_pack(tb, rect, 1, 0, 1, 1);

   _fill_source();
   _fill_sink_input();
   _fill_sink();
   _fill_card();

   elm_list_go(lst);
   elm_list_item_selected_set(it, 1);

   elm_win_center(win, 1, 1);
   evas_object_show(win);

   elm_run();
done:
   emix_shutdown();
   return 0;
}
ELM_MAIN()
