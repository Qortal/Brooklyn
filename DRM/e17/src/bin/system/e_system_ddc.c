#include "e_system.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
   char *edid;
   int screen;
} Dev;

typedef struct
{
   char *req, *params; // don't free - part of alloc for req struct at offset
} Req;

static Eina_Lock _devices_lock;
static Eina_List *_devices = NULL;
static Eina_List *_req = NULL;
static Eina_Semaphore _worker_sem;

//////////////////////////////////////////////////////////////////////////////
// needed ddc types
#define DDCA_EDID_MFG_ID_FIELD_SIZE 4
#define DDCA_EDID_MODEL_NAME_FIELD_SIZE 14
#define DDCA_EDID_SN_ASCII_FIELD_SIZE 14

typedef int DDCA_Status;
typedef void * DDCA_Display_Ref;
typedef void * DDCA_Display_Handle;
typedef uint8_t DDCA_Vcp_Feature_Code;

typedef enum
{
   DDCA_IO_I2C,
   DDCA_IO_ADL,
   DDCA_IO_USB
} DDCA_IO_Mode;

typedef struct
{
   int iAdapterIndex;
   int iDisplayIndex;
} DDCA_Adlno;

typedef struct
{
   DDCA_IO_Mode io_mode;
   union {
      int i2c_busno;
      DDCA_Adlno adlno;
      int hiddev_devno;
   } path;
} DDCA_IO_Path;

typedef struct
{
   uint8_t major;
   uint8_t minor;
} DDCA_MCCS_Version_Spec;

typedef struct
{
   char marker[4];
   int dispno;
   DDCA_IO_Path path;
   int usb_bus;
   int usb_device;
   char mfg_id[DDCA_EDID_MFG_ID_FIELD_SIZE];
   char model_name[DDCA_EDID_MODEL_NAME_FIELD_SIZE];
   char sn[DDCA_EDID_SN_ASCII_FIELD_SIZE];
   uint16_t product_code;
   uint8_t edid_bytes[128];
   DDCA_MCCS_Version_Spec vcp_version;
   DDCA_Display_Ref dref;
} DDCA_Display_Info;

typedef struct
{
   int ct;
   DDCA_Display_Info info[];
} DDCA_Display_Info_List;

typedef struct
{
   uint8_t mh;
   uint8_t ml;
   uint8_t sh;
   uint8_t sl;
} DDCA_Non_Table_Vcp_Value;

//////////////////////////////////////////////////////////////////////////////
// ddc feature codes we plan to support
// 0x12 contrast (0->100, 75)
// 0x16 video gain r (0->100, 50)
// 0x18 video gain g (0->100, 50)
// 0x1a video gain b (0->100, 50)
// 0xaa screen rotation (read only)
//  {0x01, "0 degrees"},
//  {0x02, "90 degrees"},
//  {0x03, "180 degrees"},
//  {0x04, "270 degrees"},
//  {0xff, "Display cannot supply orientation"},
// 0x6c video black level r (0->255, 128)
// 0x6e video black level g (0->255, 128)
// 0x70 video black level b (0->255, 128)
// 0x6b backlight level w
// 0x6d backlight level r
// 0x6f backlight level g
// 0x71 backlight level b
// 0xd6 power mode (10x = on, 0x4 = off)
//  {0x01, "DPM: On,  DPMS: Off"},
//  {0x02, "DPM: Off, DPMS: Standby"},
//  {0x03, "DPM: Off, DPMS: Suspend"},
//  {0x04, "DPM: Off, DPMS: Off" },
// 0xb6 display tech (readonly) (0x03 = LCD active matrix)
//  {0x01, "CRT (shadow mask)"},
//  {0x02, "CRT (aperture grill)"},
//  {0x03, "LCD (active matrix)"},   // TFT in 2.0
//  {0x04, "LCos"},
//  {0x05, "Plasma"},
//  {0x06, "OLED"},
//  {0x07, "EL"},
//  {0x08, "Dynamic MEM"},     // MEM in 2.0
//  {0x09, "Static MEM"},      // not in 2.0
// 0x62 speaker volume (0-100)
// 0x8d audio mute (1=mute, 2=unmute)
// 0xca OSD (1=disabled, 2=enabled)
// 0xda scan mode
//  {0x00, "Normal operation"},
//  {0x01, "Underscan"},
//  {0x02, "Overscan"},
//  {0x03, "Widescreen" }
// 0xdb image mode
//  {0x00, "No effect"},
//  {0x01, "Full mode"},
//  {0x02, "Zoom mode"},
//  {0x03, "Squeeze mode" },
//  {0x04, "Variable"},
// 0x8d blank state (1=blank, 2 = unblack)
// 0x82 horiz mirror (0=normal, 1 = mirror)
// 0x84 vert  mirror (0=normal, 1 = mirror)
// 0x63 speaker sel (0=front l/r, 1=side l/r, 2=rear l/r, 3=subwoofer)
// 0x86 scaling values
//  {0x01, "No scaling"},
//  {0x02, "Max image, no aspect ration distortion"},
//  {0x03, "Max vertical image, no aspect ratio distortion"},
//  {0x04, "Max horizontal image, no aspect ratio distortion"},
//  {0x05, "Max vertical image with aspect ratio distortion"},
//  {0x06, "Max horizontal image with aspect ratio distortion"},
//  {0x07, "Linear expansion (compression) on horizontal axis"},   // Full mode
//  {0x08, "Linear expansion (compression) on h and v axes"},      // Zoom mode
//  {0x09, "Squeeze mode"},
//  {0x0a, "Non-linear expansion"},                                // Variable
// 0x94 stereo mode
//  {0x00,  "Speaker off/Audio not supported"},
//  {0x01,  "Mono"},
//  {0x02,  "Stereo"},
//  {0x03,  "Stereo expanded"},
//  {0x11,  "SRS 2.0"},
//  {0x12,  "SRS 2.1"},
//  {0x13,  "SRS 3.1"},
//  {0x14,  "SRS 4.1"},
//  {0x15,  "SRS 5.1"},
//  {0x16,  "SRS 6.1"},
//  {0x17,  "SRS 7.1"},
//  {0x21,  "Dolby 2.0"},
//  {0x22,  "Dolby 2.1"},
//  {0x23,  "Dolby 3.1"},
//  {0x24,  "Dolby 4.1"},
//  {0x25,  "Dolby 5.1"},
//  {0x26,  "Dolby 6.1"},
//  {0x27,  "Dolby 7.1"},
//  {0x31,  "THX 2.0"},
//  {0x32,  "THX 2.1"},
//  {0x33,  "THX 3.1"},
//  {0x34,  "THX 4.1"},
//  {0x35,  "THX 5.1"},
//  {0x36,  "THX 6.1"},
//  {0x37,  "THX 7.1"},

//////////////////////////////////////////////////////////////////////////////

// ddc lib handle and func symbols
static void *ddc_lib = NULL;
struct {
   DDCA_Status (*ddca_get_display_info_list2)
     (bool include_invalid_displays, DDCA_Display_Info_List **dlist_loc);
   void (*ddca_free_display_info_list)
     (DDCA_Display_Info_List *dlist);
   DDCA_Status (*ddca_open_display2)
     (DDCA_Display_Ref ddca_dref, bool wait, DDCA_Display_Handle *ddca_dh_loc);
   DDCA_Status (*ddca_get_non_table_vcp_value)
     (DDCA_Display_Handle ddca_dh, DDCA_Vcp_Feature_Code feature_code, DDCA_Non_Table_Vcp_Value *valrec);
   DDCA_Status (*ddca_set_non_table_vcp_value)
     (DDCA_Display_Handle ddca_dh, DDCA_Vcp_Feature_Code feature_code, uint8_t hi_byte, uint8_t lo_byte);
   DDCA_Status (*ddca_close_display)
     (DDCA_Display_Handle ddca_dh);
   void (*ddca_set_global_sleep_multiplier)
     (double multiplier);
   bool (*ddca_enable_sleep_suppression)
     (bool newval);
} ddc_func;

static DDCA_Display_Info_List *ddc_dlist = NULL;
static DDCA_Display_Handle *ddc_dh = NULL;

static void
_ddc_clean(void)
{
   int i;

   if (!ddc_lib) return;
   if (ddc_dlist)
     {
        if (ddc_dh)
          {
             for (i = 0; i < ddc_dlist->ct; i++)
               {
                  ddc_func.ddca_close_display(ddc_dh[i]);
               }
              free(ddc_dh);
             ddc_dh = NULL;
          }
        ddc_func.ddca_free_display_info_list(ddc_dlist);
        ddc_dlist = NULL;
     }
}

static Eina_Bool
_ddc_probe(void)
{
   Dev *d;
   int i;

   if (!ddc_lib) return EINA_FALSE;
   eina_lock_take(&_devices_lock);
   EINA_LIST_FREE(_devices, d)
     {
        free(d->edid);
        free(d);
     }
   eina_lock_release(&_devices_lock);
   _ddc_clean();

   // the below can be quite sluggish, so we don't want to do this
   // often, even though this is isolated in a worker thread. it will
   // block the ddc worker thread while this is done.
   if (ddc_func.ddca_get_display_info_list2(false, &ddc_dlist) != 0) goto err;
   if (!ddc_dlist) goto err;
   ddc_dh = calloc(ddc_dlist->ct, sizeof(DDCA_Display_Handle));
   if (!ddc_dh)
     {
        ddc_func.ddca_free_display_info_list(ddc_dlist);
        ddc_dlist = NULL;
        free(ddc_dh);
        goto err;
     }
   for (i = 0; i < ddc_dlist->ct; i++)
     {
        DDCA_Display_Info *dinfo = &(ddc_dlist->info[i]);
        DDCA_Display_Ref dref = dinfo->dref;
        int j;

        if (ddc_func.ddca_open_display2(dref, false, &(ddc_dh[i])) != 0)
          {
             for (i = i - 1; i >= 0; i--)
               {
                  ddc_func.ddca_close_display(ddc_dh[i]);
               }
             free(ddc_dh);
             ddc_dh = NULL;
             ddc_func.ddca_free_display_info_list(ddc_dlist);
             ddc_dlist = NULL;
             goto err;
          }
        d = calloc(1, sizeof(Dev));
        if (d)
          {
             d->edid = malloc((128 * 2) + 1);
             if (d->edid)
               {
                  for (j = 0; j < 128; j++)
                    snprintf(&(d->edid[j * 2]), 3, "%02x", dinfo->edid_bytes[j]);
                  d->edid[j * 2] = 0;
                  d->screen = i;
                  eina_lock_take(&_devices_lock);
                  _devices = eina_list_append(_devices, d);
                  eina_lock_release(&_devices_lock);
                  if (alert_backlight_reset)
                    {  // set brightness to max if alert mode is on
                       ddc_func.ddca_set_non_table_vcp_value
                         (ddc_dh[i], 0x10, 0, 100);
                    }
               }
             else free(d);
          }
     }
   return EINA_TRUE;
err:
   return EINA_FALSE;
}

static Eina_Bool
_ddc_init(void)
{
   // .so.3 is ABI compatible twith .so.2 for out uses - see
   // https://www.ddcutil.com/c_api_99/ for changes between them
   ddc_lib = dlopen("libddcutil.so.3", RTLD_NOW | RTLD_LOCAL);
   if (!ddc_lib)
     ddc_lib = dlopen("libddcutil.so.2", RTLD_NOW | RTLD_LOCAL);
   if (!ddc_lib) return EINA_FALSE;
#define SYM(_x) \
   do { \
      ddc_func._x = dlsym(ddc_lib, #_x); \
      if (!ddc_func._x) return EINA_FALSE; \
   } while (0)
   SYM(ddca_get_display_info_list2);
   SYM(ddca_free_display_info_list);
   SYM(ddca_open_display2);
   SYM(ddca_get_non_table_vcp_value);
   SYM(ddca_set_non_table_vcp_value);
   SYM(ddca_close_display);
   // optional api's
#define SYM_OPT(_x) \
   do { \
      ddc_func._x = dlsym(ddc_lib, #_x); \
   } while (0)
   SYM_OPT(ddca_set_global_sleep_multiplier);
   SYM_OPT(ddca_enable_sleep_suppression);

   // brute force modprobe this as it likely is needed - probe will fail
   // if this doesn't work or find devices anyway
   if (system("modprobe i2c-dev") == 0)
     usleep(200 * 1000); // and wait for the module to come up... 200ms

   if (!_ddc_probe()) return EINA_FALSE;

   // try improve performance by limiting sleeps in ddcutil
   if (ddc_func.ddca_enable_sleep_suppression)
     ddc_func.ddca_enable_sleep_suppression(true);

   return EINA_TRUE;
}

static Req *
_req_alloc(const char *req, const char *params)
{
   Req *r;

   if (!params) params = "";
   r = calloc(1, sizeof(Req) + strlen(req) + 1 + strlen(params) + 1);
   if (!r) return NULL;
   r->req = ((char *)r) + sizeof(Req);
   strcpy(r->req, req);
   r->params = r->req + strlen(r->req) + 1;
   strcpy(r->params, params);
   return r;
}

static void
_request(const char *req, const char *params)
{
   Eina_List *l;
   Req *r2, *r = _req_alloc(req, params);
   if (!r) return;

   eina_lock_take(&_devices_lock);
   EINA_LIST_FOREACH(_req, l, r2)
     {
        if (!strcmp(r2->req, r->req))
          {
             if (!strcmp(req, "refresh"))
               {
                  _req = eina_list_remove_list(_req, l);
                  free(r2);
                  break;
               }
             else if ((!strcmp(req, "val-set")) ||
                      (!strcmp(req, "val-get")))
               {
                  if ((!strncmp(params, r2->params, 256)) &&
                      (params[256] == ' '))
                    {
                       const char *space = strchr(params + 256 + 1, ' ');
                       if (!strncmp(params, r2->params, (space - params)))
                         {
                            _req = eina_list_remove_list(_req, l);
                            free(r2);
                            break;
                         }
                    }
               }
          }
     }
   _req = eina_list_append(_req, r);
   eina_semaphore_release(&_worker_sem, 1);
   eina_lock_release(&_devices_lock);
}

static void
_do_list(Ecore_Thread *th)
{
   Eina_List *l;
   Dev *d;
   Req *r;
   Eina_Strbuf *sbuf;

   sbuf = eina_strbuf_new();
   if (sbuf)
     {
        eina_lock_take(&_devices_lock);
        EINA_LIST_FOREACH(_devices, l, d)
          {
             eina_strbuf_append(sbuf, d->edid);
             if (l->next) eina_strbuf_append(sbuf, " ");
          }
        eina_lock_release(&_devices_lock);

        r = _req_alloc("ddc-list", eina_strbuf_string_get(sbuf));
        if (r) ecore_thread_feedback(th, r);
        eina_strbuf_free(sbuf);
     }
}

static Eina_Bool
_id_ok(int id)
{
   if ((id < 0) || (id > 0xff)) return EINA_FALSE; // limited range
   if (id == 0x10) return EINA_TRUE; // backlight allowed
   return EINA_FALSE;
}

static Dev *
_dev_find(const char *edid)
{
   Eina_List *l;
   Dev *d;

   EINA_LIST_FOREACH(_devices, l, d)
     {
        if (!strcmp(d->edid, edid))
          {
             return d;
          }
     }
   return NULL;
}

static void
_do_val_set(Ecore_Thread *th, const char *edid, int id, int val)
{
   Dev *d;
   Req *r;
   int screen;
   char buf[512];

   if (!ddc_lib) goto err;
   if (!edid) goto err;
   if (!_id_ok(id)) goto err;
   if ((val < 0) || (val >= 65536)) goto err;

   eina_lock_take(&_devices_lock);
   d = _dev_find(edid);
   if (!d)
     {
        eina_lock_release(&_devices_lock);
        goto err;
     }
   screen = d->screen;
   eina_lock_release(&_devices_lock);

   if (ddc_func.ddca_set_non_table_vcp_value
       (ddc_dh[screen], id, (val >> 8) & 0xff, val & 0xff) == 0)
     {
        fprintf(stderr, "DDC: set ok %s 0x%02x %i\n", edid, id, val);
        snprintf(buf, sizeof(buf), "%s %i %i ok", edid, id, val);
     }
   else
     {
        fprintf(stderr, "DDC: set fail %s 0x%02x %i\n", edid, id, val);
err:
        snprintf(buf, sizeof(buf), "%s %i %i err", edid, id, val);
     }
   r = _req_alloc("ddc-val-set", buf);
   if (r) ecore_thread_feedback(th, r);
}

static void
_do_val_get(Ecore_Thread *th, const char *edid, int id)
{
   Dev *d;
   Req *r;
   int screen, val, max;
   char buf[512];
   DDCA_Non_Table_Vcp_Value valrec;

   if (!ddc_lib) goto err;
   if (!edid) goto err;
   if (!_id_ok(id)) goto err;

   eina_lock_take(&_devices_lock);
   d = _dev_find(edid);
   if (!d)
     {
        eina_lock_release(&_devices_lock);
        goto err;
     }
   screen = d->screen;
   eina_lock_release(&_devices_lock);

   if (ddc_func.ddca_get_non_table_vcp_value
       (ddc_dh[screen], id, &valrec) == 0)
     {
        max = valrec.ml | valrec.mh << 8;
        val = valrec.sl | (valrec.sh << 8);
        fprintf(stderr, "DDC: get ok %s 0x%02x val=%i max=%i\n", edid, id, val, max);
        snprintf(buf, sizeof(buf), "%s %i %i %i", edid, id, val, max);
     }
   else
     {
        fprintf(stderr, "DDC: get fail %s 0x%02x\n", edid, id);
err:
        snprintf(buf, sizeof(buf), "%s %i -1 -1", edid, id);
     }
   r = _req_alloc("ddc-val-get", buf);
   if (r) ecore_thread_feedback(th, r);
}

static void
_cb_worker(void *data EINA_UNUSED, Ecore_Thread *th)
{
   Req *r;

   _ddc_init();
   _do_list(th);

   for (;;)
     {
        Eina_List *local_req;

        // wait for requests
        eina_semaphore_lock(&_worker_sem);
        local_req = NULL;
        eina_lock_take(&_devices_lock);
        if (_req)
          {
             local_req = _req;
             _req = NULL;
          }
        eina_lock_release(&_devices_lock);
        while (local_req)
          {
             r = local_req->data;
             local_req = eina_list_remove_list(local_req, local_req);
             if (!r) continue;
             if (!strcmp(r->req, "list"))
               {
                  _do_list(th);
               }
             else if (!strcmp(r->req, "refresh"))
               {
                  int i = 0;

                  for (i = 0; i < 10; i++)
                    {
                       if (_ddc_probe()) break;
                       usleep(10 * 1000);
                    }
                  if (i == 10)
                    fprintf(stderr, "DDC: PROBE FAILED.\n");
               }
             else if (!strcmp(r->req, "val-set"))
               {
                  int id, val;
                  char edid[257];
                  if (sscanf(r->params, "%256s %i %i", edid, &id, &val) == 3)
                  _do_val_set(th, edid, id, val);
               }
             else if (!strcmp(r->req, "val-get"))
               {
                  int id;
                  char edid[257];
                  if (sscanf(r->params, "%256s %i", edid, &id) == 2)
                  _do_val_get(th, edid, id);
               }
             free(r);
          }
     }
}

static void
_cb_worker_message(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED, void *msg_data)
{
   Req *r = msg_data;

   if (!r) return;
   e_system_inout_command_send(r->req, "%s", r->params);
   free(r);
}

static void
_cb_worker_end(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
}

static void
_cb_worker_cancel(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED)
{
}

static void
_cb_ddc_list(void *data EINA_UNUSED, const char *params)
{
   _request("list", params);
}

static void
_cb_ddc_refresh(void *data EINA_UNUSED, const char *params)
{
   _request("refresh", params);
}

static void
_cb_ddc_val_set(void *data EINA_UNUSED, const char *params)
{
   _request("val-set", params);
}

static void
_cb_ddc_val_get(void *data EINA_UNUSED, const char *params)
{
   _request("val-get", params);
}

void
e_system_ddc_init(void)
{
   eina_lock_new(&_devices_lock);
   eina_semaphore_new(&_worker_sem, 0);
   ecore_thread_feedback_run(_cb_worker, _cb_worker_message,
                             _cb_worker_end, _cb_worker_cancel,
                             NULL, EINA_TRUE);
   e_system_inout_command_register("ddc-list",    _cb_ddc_list, NULL);
   e_system_inout_command_register("ddc-refresh", _cb_ddc_refresh, NULL);
   e_system_inout_command_register("ddc-val-set", _cb_ddc_val_set, NULL);
   e_system_inout_command_register("ddc-val-get", _cb_ddc_val_get, NULL);
}

void
e_system_ddc_shutdown(void)
{
   // only shutdown things we really have to - no need to free mem etc.
}
