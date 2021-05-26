#include <e.h>
#include "process.h"

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
# include <sys/types.h>
# include <sys/sysctl.h>
#endif

static int64_t      _mem_total;
static Eina_List   *_clients = NULL;
static Ecore_Timer *_clients_timer = NULL;

#define POLL_TIME 3.0

typedef struct _Proc_Stats Proc_Stats;
struct _Proc_Stats
{
   E_Client    *client;
   Evas_Object *obj;
   Evas_Object *obj_swallow;
   Evas_Object *popup;
   pid_t        pid;
   uint64_t     mem_size;
   uint64_t     cpu_time;
   uint64_t     cpu_time_prev;
};

static void _proc_stats_item_display(Proc_Stats *item);

static Eina_Bool
_memory_total(void)
{
#if defined(__linux__)
   char line[256];
   FILE *f = fopen("/proc/meminfo", "r");
   if (!f) return 0;

   while (fgets(line, sizeof(line), f) != NULL)
     {
        if (!strncmp("MemTotal:", line, 9))
          {
             char *p, *t;
             p = strchr(line, ':') + 1;
             while (isspace(*p)) p++;
             t = strtok(p, " ");
             _mem_total = atoll(t) * 1024;
             break;
          }
     }
   fclose(f);
   if(!_mem_total) return 0;
#else
   size_t len = sizeof(_mem_total);
   int mib[5] = { CTL_HW, HW_PHYSMEM, 0, 0, 0 };
   if (sysctl(mib, 2, &_mem_total, &len, NULL, 0) == -1)
     return 0;
#endif
   return 1;
}

static Eina_Bool
_proc_stats_item_exists(E_Client *ec)
{
   Proc_Stats *item;
   Eina_List *l;

   EINA_LIST_FOREACH(_clients, l, item)
     {
        if (item->pid == ec->netwm.pid) return 1;
     }
   return 0;
}

static void
_proc_stats_item_del(Proc_Stats *item)
{
   if (item->popup) evas_object_del(item->popup);
   item->popup = NULL;
   edje_object_signal_emit(item->obj, "e,state,procstats,off", "e");
   evas_object_del(item->obj_swallow);
   free(item);
   item = NULL;
}

static void
_proc_stats_client_del_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Proc_Stats *item = data;

   if (item->popup) evas_object_del(item->popup);
   item->popup = NULL;
   evas_object_hide(item->obj_swallow);
   edje_object_signal_emit(obj, "e,state,procstats,off", "e");
}

static void
_proc_stats_client_move_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Proc_Stats *item;
   Evas_Coord ox, oy, ow, oh;

   item = data;

   if ((!item) || (!item->popup)) return;

   evas_object_geometry_get(item->obj_swallow, &ox, &oy, &ow, &oh);
   evas_object_move(item->popup, ox + (ow / 2), oy);

   if ((item->client->hidden) || (item->client->iconic))
     evas_object_hide(item->popup);
   else
     evas_object_show(item->popup);
}

static void
_proc_stats_icon_clicked_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Proc_Stats *item;
   Evas_Object *o, *tb;
   Evas_Object *pb;
   Evas_Coord ox, oy, ow, oh;

   ev = event_info;
   item = data;

   if (ev->button != 1) return;

   if (!item) return;

   if (item->popup)
     {
        elm_ctxpopup_dismiss(item->popup);
        item->popup = NULL;
        return;
     }

   evas_object_geometry_get(item->obj_swallow, &ox, &oy, &ow, &oh);

   item->popup = o = elm_ctxpopup_add(e_comp->elm);
   E_FILL(o); E_EXPAND(o);
   elm_object_style_set(o, "noblock");
   evas_object_layer_set(o, E_LAYER_MENU);

   tb = elm_table_add(o);
   E_FILL(tb); E_EXPAND(tb);
   elm_object_content_set(o, tb);
   evas_object_show(tb);

   pb = elm_progressbar_add(o);
   elm_progressbar_span_size_set(pb, 140);
   E_FILL(pb); E_EXPAND(pb);
   elm_table_pack(tb, pb, 0, 0, 1, 1);
   evas_object_data_set(o, "pb_cpu", pb);
   evas_object_show(pb);

   pb = elm_progressbar_add(o);
   elm_progressbar_span_size_set(pb, 140);
   E_FILL(pb); E_EXPAND(pb);
   elm_table_pack(tb, pb, 0, 1, 1, 1);
   evas_object_data_set(o, "pb_mem", pb);
   evas_object_show(pb);

   _proc_stats_item_display(item);

   elm_ctxpopup_direction_priority_set(o, ELM_CTXPOPUP_DIRECTION_UP,
                                       ELM_CTXPOPUP_DIRECTION_DOWN,
                                       ELM_CTXPOPUP_DIRECTION_LEFT,
                                       ELM_CTXPOPUP_DIRECTION_RIGHT);
   evas_object_move(o, ox + (ow / 2), oy);
   evas_object_show(o);
}

static void
_proc_stats_item_add(E_Client *ec, E_Module *module)
{
   Evas_Object *o, *ic;
   Proc_Stats *item;
   char buf[PATH_MAX];

   if (ec->internal || ec->netwm.pid == -1) return;
   if (!ec->frame_object) return;
   if (_proc_stats_item_exists(ec)) return;

   o = edje_object_add(evas_object_evas_get(ec->frame));

   e_theme_edje_object_set(o, "base/theme/borders",
                           "e/widgets/border/default/border");

   if (!edje_object_part_exists(o, "e.procstats.swallow")) return;

   snprintf(buf, sizeof(buf), "%s/e-module-procstats.edj", e_module_dir_get(module));

   ic = elm_icon_add(e_comp->elm);
   elm_image_file_set(ic, buf, "icon");
   evas_object_size_hint_min_set(ic, ELM_SCALE_SIZE(16), ELM_SCALE_SIZE(16));
   evas_object_size_hint_max_set(ic, ELM_SCALE_SIZE(16), ELM_SCALE_SIZE(16));
   E_FILL(ic); E_EXPAND(ic);
   evas_object_show(ic);

   item = calloc(1, sizeof(Proc_Stats));
   EINA_SAFETY_ON_NULL_RETURN(item);
   item->pid = ec->netwm.pid;
   item->client = ec;
   item->obj = ec->frame_object;
   item->obj_swallow = ic;

   edje_object_part_swallow(ec->frame_object, "e.procstats.swallow", ic);
   edje_object_signal_emit(ec->frame_object, "e,state,procstats,on", "e");

   evas_object_event_callback_add(item->obj, EVAS_CALLBACK_DEL, _proc_stats_client_del_cb, item);
   evas_object_event_callback_add(item->obj, EVAS_CALLBACK_MOVE, _proc_stats_client_move_cb, item);
   evas_object_event_callback_add(ic, EVAS_CALLBACK_MOUSE_UP, _proc_stats_icon_clicked_cb, item);

   _clients = eina_list_append(_clients, item);
}

static void
_proc_stats_item_remove(Proc_Stats *item)
{
   Eina_List *l, *l_next;
   Proc_Stats *it;

   EINA_LIST_FOREACH_SAFE(_clients, l, l_next, it)
     {
        if (it == item)
          {
             _proc_stats_item_del(item);
             _clients = eina_list_remove_list(_clients, l);
             return;
          }
     }
}

static Eina_Bool
_proc_stats_item_gone(Proc_Stats *item)
{
   Eina_List *l;
   E_Client *ec;

   EINA_LIST_FOREACH(e_comp->clients, l, ec)
     {
        if (item->client == ec) return 0;
     }

   return 1;
}

static void
_proc_stats_item_children_update(Eina_List *children, Proc_Stats *item)
{
   Eina_List *l;
   Proc_Info *child;

   EINA_LIST_FOREACH(children, l, child)
    {
       item->mem_size += child->mem_size;
       item->cpu_time += child->cpu_time;
       if (child->children)
         _proc_stats_item_children_update(child->children, item);
    }
}

static char *
_size_format(unsigned long long bytes)
{
   char buf[1024];
   unsigned long powi = 1;
   unsigned long long value;
   unsigned int precision = 2, powj = 1;
   int i = 0;
   const char *units[8] = {
      _("B"), _("KiB"), _("MiB"), _("GiB"),
      _("TiB"), _("PiB"), _("EiB"), _("ZiB"),
   };

   value = bytes;
   while (value > 1024)
     {
       if ((value / 1024) < powi) break;
       powi *= 1024;
       ++i;
       if (i == 7) break;
     }
   if (!i) precision = 0;
   while (precision > 0)
     {
        powj *= 10;
        if ((value / powi) < powj) break;
        --precision;
     }
   snprintf(buf, sizeof(buf), "%1.*f%s", precision, (double) value / powi, units[i]);

   return strdup(buf);
}

static void
_proc_stats_item_display(Proc_Stats *item)
{
   Edje_Message_Int_Set *msg;
   Evas_Object *pb;
   Eina_Strbuf *buf;
   char *s;
   double val = 0.0;

   if (item->cpu_time_prev > item->cpu_time)
     return;

   if (!item->cpu_time_prev) item->cpu_time_prev = item->cpu_time;

   msg = malloc(sizeof(Edje_Message_Int_Set) + (sizeof(int) * 4));
   EINA_SAFETY_ON_NULL_RETURN(msg);
   msg->count = 5;
   msg->val[0] = eina_cpu_count();
   msg->val[1] = (item->cpu_time - item->cpu_time_prev) / POLL_TIME;
   msg->val[2] = (int) _mem_total / 4096;
   msg->val[3] = (int) item->mem_size / 4096;
   msg->val[4] = 0;
   edje_object_message_send(item->obj, EDJE_MESSAGE_INT_SET, 1, msg);
   free(msg);

   if (!item->popup) return;

   pb = evas_object_data_get(item->popup, "pb_cpu");

   val = ((item->cpu_time - item->cpu_time_prev) / POLL_TIME);
   elm_progressbar_value_set(pb, val / 100.0);

   buf = eina_strbuf_new();

   eina_strbuf_append_printf(buf, "%1.0f %%", val);
   elm_object_part_text_set(pb, "elm.text.status", eina_strbuf_string_get(buf));
   eina_strbuf_reset(buf);

   pb = evas_object_data_get(item->popup, "pb_mem");
   val = item->mem_size / (_mem_total / 100.0);
   elm_progressbar_value_set(pb, val / 100.0);

   s = _size_format(item->mem_size);
   eina_strbuf_append_printf(buf, "%s / ", s);
   free(s);
   s = _size_format(_mem_total);
   eina_strbuf_append(buf, s);
   free(s);

   elm_object_part_text_set(pb, "elm.text.status", eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static void
_proc_stats_item_update(Eina_List *procs, Proc_Stats *item)
{
   Proc_Info *proc;
   Eina_List *l;

   EINA_LIST_FOREACH(procs, l, proc)
     {
        if (proc->pid == item->pid)
          {
             item->cpu_time = item->mem_size = 0;
             item->mem_size += proc->mem_size;
             item->cpu_time +=  proc->cpu_time;
             _proc_stats_item_children_update(proc->children, item);
             break;
          }
     }
   _proc_stats_item_display(item);
   item->cpu_time_prev = item->cpu_time;
}

static Eina_Bool
_proc_stats_timer_cb(void *data)
{
   E_Module *module;
   Eina_List *procs, *l;
   E_Client *ec;
   Proc_Info *proc;
   Proc_Stats *item;

   module = data;

   EINA_LIST_FOREACH(e_comp->clients, l, ec)
     {
        if (!_proc_stats_item_exists(ec))
          _proc_stats_item_add(ec, module);
     }

   procs = proc_info_all_children_get();

   EINA_LIST_FOREACH(_clients, l, item)
     {
        if (_proc_stats_item_gone(item))
          _proc_stats_item_remove(item);
        else
         _proc_stats_item_update(procs, item);
     }

   EINA_LIST_FREE(procs, proc)
     proc_info_free(proc);

   return 1;
}

E_API E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "Procstats"
};

E_API int
e_modapi_init(E_Module *m)
{
   if (!_memory_total())
     return 0;

   _proc_stats_timer_cb(m);

   _clients_timer = ecore_timer_add(POLL_TIME, _proc_stats_timer_cb, m);

   return 1;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   Proc_Stats *item;

   if (_clients_timer)
     ecore_timer_del(_clients_timer);

   _clients_timer = NULL;

   EINA_LIST_FREE(_clients, item)
     _proc_stats_item_del(item);

   _clients = NULL;

   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   return 1;
}
