#include "e.h"

typedef struct _E_Thumb E_Thumb;

struct _E_Thumb
{
   int           objid;
   int           w, h;
   const char   *file;
   const char   *key;
   char         *sort_id;
   struct {
      int x, y, x_count, y_count;
   } desk_pan;
   Eina_List    *sigsrc;
   unsigned char queued E_BITFIELD;
   unsigned char busy E_BITFIELD;
   unsigned char done E_BITFIELD;
};

/* local subsystem functions */
static void         _e_thumb_gen_begin(int objid, const char *file, const char *key, int w, int h, int desk_x, int desk_y, int desk_x_count, int desk_y_count, Eina_List *sigsrc);
static void         _e_thumb_gen_end(int objid);
static void         _e_thumb_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void         _e_thumb_hash_add(int objid, Evas_Object *obj);
static void         _e_thumb_hash_del(int objid);
static Evas_Object *_e_thumb_hash_find(int objid);
static void         _e_thumb_thumbnailers_kill(void);
static void         _e_thumb_thumbnailers_kill_cancel(void);
static Eina_Bool    _e_thumb_cb_kill(void *data);
static Eina_Bool    _e_thumb_cb_exe_event_del(void *data, int type, void *event);

/* local subsystem globals */
static Eina_List *_thumbnailers = NULL;
static Eina_List *_thumbnailers_exe = NULL;
static Eina_List *_thumb_queue = NULL;
static int _objid = 0;
static Eina_Hash *_thumbs = NULL;
static int _pending = 0;
static int _num_thumbnailers = 1;
static Ecore_Event_Handler *_exe_del_handler = NULL;
static Ecore_Timer *_kill_timer = NULL;

/* externally accessible functions */
EINTERN int
e_thumb_init(void)
{
   _exe_del_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                              _e_thumb_cb_exe_event_del,
                                              NULL);
   _thumbs = eina_hash_string_superfast_new(NULL);
   return 1;
}

EINTERN int
e_thumb_shutdown(void)
{
   _e_thumb_thumbnailers_kill_cancel();
   _e_thumb_cb_kill(NULL);
   if (_exe_del_handler) ecore_event_handler_del(_exe_del_handler);
   _exe_del_handler = NULL;
   _thumbnailers = eina_list_free(_thumbnailers);
   E_FREE_LIST(_thumbnailers_exe, ecore_exe_free);
   _thumb_queue = eina_list_free(_thumb_queue);
   _objid = 0;
   eina_hash_free(_thumbs);
   _thumbs = NULL;
   _pending = 0;
   return 1;
}

static void
_thumb_preloaded(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   evas_object_smart_callback_call(data, "e_thumb_gen", NULL);
}

E_API Evas_Object *
e_thumb_icon_add(Evas *evas)
{
   Evas_Object *obj;
   E_Thumb *eth;

   obj = e_icon_add(evas);
   evas_object_smart_callback_add(obj, "preloaded", _thumb_preloaded, obj);
   _objid++;
   eth = E_NEW(E_Thumb, 1);
   eth->objid = _objid;
   eth->w = 64;
   eth->h = 64;
   evas_object_data_set(obj, "e_thumbdata", eth);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE,
                                  _e_thumb_del_hook, NULL);
   _e_thumb_hash_add(eth->objid, obj);
   return obj;
}

E_API void
e_thumb_icon_file_set(Evas_Object *obj, const char *file, const char *key)
{
   E_Thumb *eth;

   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;
   eina_stringshare_replace(&eth->file, file);
   eina_stringshare_replace(&eth->key, key);
   E_FREE(eth->sort_id);
}

E_API void
e_thumb_icon_size_set(Evas_Object *obj, int w, int h)
{
   E_Thumb *eth;

   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;
   if ((w < 1) || (h < 1)) return;
   eth->w = w;
   eth->h = h;
}

E_API void
e_thumb_icon_begin(Evas_Object *obj)
{
   E_Thumb *eth, *eth2;
   char buf[4096];

   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;
   if (eth->queued) return;
   if (eth->busy) return;
   if (eth->done) return;
   if (!eth->file) return;
   if (!_thumbnailers)
     {
        while ((int)eina_list_count(_thumbnailers_exe) < _num_thumbnailers)
          {
             Ecore_Exe *exe;

             snprintf(buf, sizeof(buf), "%s/enlightenment/utils/enlightenment_thumb --nice=%d", e_prefix_lib_get(),
                      e_config->thumb_nice);
             exe = e_util_exe_safe_run(buf, NULL);
             _thumbnailers_exe = eina_list_append(_thumbnailers_exe, exe);
          }
        _thumb_queue = eina_list_append(_thumb_queue, eth);
        eth->queued = 1;
        return;
     }
   EINA_LIST_FREE(_thumb_queue, eth2)
     {
        eth2->queued = 0;
        eth2->busy = 1;
        _pending++;
        if (_pending == 1) _e_thumb_thumbnailers_kill_cancel();
        _e_thumb_gen_begin(eth2->objid, eth2->file, eth2->key,
                           eth2->w, eth2->h,
                           eth2->desk_pan.x, eth2->desk_pan.y,
                           eth2->desk_pan.x_count, eth2->desk_pan.y_count,
                           eth2->sigsrc);
     }
   eth->busy = 1;
   _pending++;
   if (_pending == 1) _e_thumb_thumbnailers_kill_cancel();
   _e_thumb_gen_begin(eth->objid, eth->file, eth->key,
                      eth->w, eth->h,
                      eth->desk_pan.x, eth->desk_pan.y,
                      eth->desk_pan.x_count, eth->desk_pan.y_count,
                      eth->sigsrc);
}

E_API void
e_thumb_icon_end(Evas_Object *obj)
{
   E_Thumb *eth;

   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;
   if (eth->queued)
     {
        _thumb_queue = eina_list_remove(_thumb_queue, eth);
        eth->queued = 0;
     }
   if (eth->busy)
     {
        _e_thumb_gen_end(eth->objid);
        eth->busy = 0;
        _pending--;
        if (_pending == 0) _e_thumb_thumbnailers_kill();
     }
}

E_API void
e_thumb_icon_rethumb(Evas_Object *obj)
{
   E_Thumb *eth;
   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;

   if (eth->done) eth->done = 0;
   else e_thumb_icon_end(obj);

   e_thumb_icon_begin(obj);
}

E_API void
e_thumb_desk_pan_set(Evas_Object *obj, int x, int y, int x_count, int y_count)
{
   E_Thumb *eth;
   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;
   eth->desk_pan.x = x;
   eth->desk_pan.y = y;
   eth->desk_pan.x_count = x_count;
   eth->desk_pan.y_count = y_count;
}

E_API void
e_thumb_signal_add(Evas_Object *obj, const char *sig, const char *src)
{
   E_Thumb *eth;
   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;
   eth->sigsrc = eina_list_append(eth->sigsrc, eina_stringshare_add(sig));
   eth->sigsrc = eina_list_append(eth->sigsrc, eina_stringshare_add(src));
}

#define A(v)          (((v) >> 24) & 0xff)
#define R(v)          (((v) >> 16) & 0xff)
#define G(v)          (((v) >> 8) & 0xff)
#define B(v)          (((v)) & 0xff)
#define PIX(p, x, y)  p[((y) << 2) + (x)]
#define PIX2(p, x, y) p[((y) << 1) + (x)]

static void
_e_thumb_key_load(E_Thumb *eth, const char *icon)
{
   Eet_File *ef;
   int size = 0;

   ef = eet_open(icon, EET_FILE_MODE_READ);
   if (!ef) return;
   eth->sort_id = eet_read(ef, "/thumbnail/sort_id", &size);
   if (eth->sort_id)
     {
        if (size > 0) eth->sort_id[size - 1] = 0;
        else
          {
             free(eth->sort_id);
             eth->sort_id = NULL;
          }
     }
   eet_close(ef);
}

E_API const char *
e_thumb_sort_id_get(Evas_Object *obj)
{
   E_Thumb *eth;
   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return "";
   if (!eth->sort_id) return "";
   return eth->sort_id;
}

E_API void
e_thumb_client_data(Ecore_Ipc_Event_Client_Data *e)
{
   int objid;
   char *icon;
   E_Thumb *eth;
   Evas_Object *obj;

   if (!eina_list_data_find(_thumbnailers, e->client))
     _thumbnailers = eina_list_prepend(_thumbnailers, e->client);
   if (e->minor == 2)
     {
        objid = e->ref;
        icon = e->data;
        if ((icon) && (e->size > 1) && (icon[e->size - 1] == 0))
          {
             obj = _e_thumb_hash_find(objid);
             if (obj)
               {
                  eth = evas_object_data_get(obj, "e_thumbdata");
                  if (eth)
                    {
                       eth->busy = 0;
                       _pending--;
                       eth->done = 1;
                       if (_pending == 0) _e_thumb_thumbnailers_kill();
                       if (ecore_file_exists(icon))
                         {
                            e_icon_preload_set(obj, 1);
                            e_icon_file_key_set(obj, icon, "/thumbnail/data");
                            _e_thumb_key_load(eth, icon);
                         }
                       evas_object_smart_callback_call(obj, "e_thumb_gen", NULL);
                    }
               }
          }
     }
   if (e->minor == 1)
     {
        /* hello message */
        EINA_LIST_FREE(_thumb_queue, eth)
          {
             eth->queued = 0;
             eth->busy = 1;
             _pending++;
             if (_pending == 1) _e_thumb_thumbnailers_kill_cancel();
             _e_thumb_gen_begin(eth->objid, eth->file, eth->key,
                                eth->w, eth->h,
                                eth->desk_pan.x, eth->desk_pan.y,
                                eth->desk_pan.x_count, eth->desk_pan.y_count,
                                eth->sigsrc);
          }
     }
}

E_API void
e_thumb_client_del(Ecore_Ipc_Event_Client_Del *e)
{
   if (!eina_list_data_find(_thumbnailers, e->client)) return;
   _thumbnailers = eina_list_remove(_thumbnailers, e->client);
   if ((!_thumbs) && (!_thumbnailers)) _objid = 0;
}

/* local subsystem functions */
static void
_e_thumb_gen_begin(int objid, const char *file, const char *key, int w, int h,
                   int desk_x, int desk_y, int desk_x_count, int desk_y_count,
                   Eina_List *sigsrc)
{
   char *buf, *p;
   int l1, l2, size, *desk;
   Ecore_Ipc_Client *cli;
   Eina_List *l;
   const char *s;

   /* send thumb req */
   // figure out buffer size needed
   l1 = strlen(file);
   l2 = 0;
   if (key) l2 = strlen(key);
   size = (4 * sizeof(int)); // desk_x/y/count
   size += l1 + 1; // file
   size += l2 + 1; // key
   EINA_LIST_FOREACH(sigsrc, l, s)
     {
        size += strlen(s) + 1;
     }
   buf = alloca(size);
   p = buf;

   // fill in buffer data
   // data is:
   //  [int]desk_x
   //  [int]desk_y
   //  [int]desk_x_count
   //  [int]desk_y_count
   //  [char[]]file
   //  [char[]]key
   // optional:
   //  [char[]]sig1
   //  [char[]]src1
   //  [char[]]sig2
   //  [char[]]src2
   //  ...
   desk = (int *)(void *)buf;
   desk[0] = desk_x;
   desk[1] = desk_y;
   desk[2] = desk_x_count;
   desk[3] = desk_y_count;
   p += (4 * sizeof(int));
   strcpy(p, file);
   p += l1 + 1;
   if (key)
     {
        strcpy(p, key);
        p += l2 + 1;
     }
   else
     {
        p[0] = 0;
        p += 1;
     }
   EINA_LIST_FOREACH(sigsrc, l, s)
     {
        strcpy(p, s);
        p += strlen(s) + 1;
     }

   // actually send it off
   cli = eina_list_data_get(_thumbnailers);
   if (!cli) return;
   _thumbnailers = eina_list_remove_list(_thumbnailers, _thumbnailers);
   _thumbnailers = eina_list_append(_thumbnailers, cli);
   ecore_ipc_client_send(cli, E_IPC_DOMAIN_THUMB, 1, objid, w, h, buf, size);
}

static void
_e_thumb_gen_end(int objid)
{
   Eina_List *l;
   Ecore_Ipc_Client *cli;

   /* send thumb cancel */
   EINA_LIST_FOREACH(_thumbnailers, l, cli)
     {
        ecore_ipc_client_send(cli, E_IPC_DOMAIN_THUMB, 2, objid, 0, 0, NULL, 0);
     }
}

static void
_e_thumb_del_hook(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   E_Thumb *eth;
   const char *s;

   eth = evas_object_data_get(obj, "e_thumbdata");
   if (!eth) return;
   evas_object_data_del(obj, "e_thumbdata");
   _e_thumb_hash_del(eth->objid);
   if (eth->busy)
     {
        _e_thumb_gen_end(eth->objid);
        eth->busy = 0;
        _pending--;
        if (_pending == 0) _e_thumb_thumbnailers_kill();
     }
   if (eth->queued)
     _thumb_queue = eina_list_remove(_thumb_queue, eth);
   if (eth->file) eina_stringshare_del(eth->file);
   if (eth->key) eina_stringshare_del(eth->key);
   free(eth->sort_id);
   EINA_LIST_FREE(eth->sigsrc, s) eina_stringshare_del(s);
   free(eth);
}

static void
_e_thumb_hash_add(int objid, Evas_Object *obj)
{
   char buf[32];

   snprintf(buf, sizeof(buf), "%i", objid);
   eina_hash_add(_thumbs, buf, obj);
}

static void
_e_thumb_hash_del(int objid)
{
   char buf[32];

   snprintf(buf, sizeof(buf), "%i", objid);
   if (_thumbs) eina_hash_del(_thumbs, buf, NULL);
   if ((!_thumbs) && (!_thumbnailers)) _objid = 0;
}

static Evas_Object *
_e_thumb_hash_find(int objid)
{
   char buf[32];

   snprintf(buf, sizeof(buf), "%i", objid);
   return eina_hash_find(_thumbs, buf);
}

static void
_e_thumb_thumbnailers_kill(void)
{
   if (_kill_timer) ecore_timer_del(_kill_timer);
   _kill_timer = ecore_timer_loop_add(1.0, _e_thumb_cb_kill, NULL);
}

static void
_e_thumb_thumbnailers_kill_cancel(void)
{
   if (_kill_timer) ecore_timer_del(_kill_timer);
   _kill_timer = NULL;
}

static Eina_Bool
_e_thumb_cb_kill(void *data EINA_UNUSED)
{
   Eina_List *l;
   Ecore_Exe *exe;

   EINA_LIST_FOREACH(_thumbnailers_exe, l, exe)
     ecore_exe_terminate(exe);
   _kill_timer = NULL;
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_e_thumb_cb_exe_event_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev;
   Ecore_Exe *exe;
   Eina_List *l;

   ev = event;
   EINA_LIST_FOREACH(_thumbnailers_exe, l, exe)
     {
        if (exe == ev->exe)
          {
             _thumbnailers_exe = eina_list_remove_list(_thumbnailers_exe, l);
             break;
          }
     }
   if ((!_thumbnailers_exe) && (_thumb_queue))
     {
        while ((int)eina_list_count(_thumbnailers_exe) < _num_thumbnailers)
          {
             Ecore_Exe *exe_thumb;
             char buf[4096];

             snprintf(buf, sizeof(buf), "%s/enlightenment/utils/enlightenment_thumb --nice=%d", e_prefix_lib_get(),
                      e_config->thumb_nice);
             exe_thumb = e_util_exe_safe_run(buf, NULL);
             _thumbnailers_exe = eina_list_append(_thumbnailers_exe, exe_thumb);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

