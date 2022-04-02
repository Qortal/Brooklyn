#include "e.h"

typedef struct
{
   const char *dev;
   const char *output;
   const char *edid;
   double val, expected_val;
   double from_val, to_val;
   Ecore_Animator *anim;
   Ecore_Timer *retry_timer;
   int ddc_max;
   int retries;
} Backlight_Device;

E_API int E_EVENT_BACKLIGHT_CHANGE = -1;

static Eina_List *bl_devs = NULL;
static Eina_List *handlers = NULL;
static Ecore_Job *zone_change_job = NULL;
static Eina_List *_devices = NULL;
static int        _devices_pending_ops = 0;
static Eina_Bool  _devices_zones_update = EINA_FALSE;
static Eina_Bool  _own_vt = EINA_TRUE;
static Eina_Bool  _e_bl_suspend = EINA_FALSE;

static void _backlight_devices_device_set(Backlight_Device *bd, double val);
static void _backlight_devices_device_update(Backlight_Device *bd);

static Eina_Bool
_backlight_retry_timer_cb(void *data)
{
   Backlight_Device *bd = data;

   bd->retry_timer = NULL;
   _backlight_devices_device_set(bd, bd->expected_val);
   _backlight_devices_device_update(bd);
   return EINA_FALSE;
}

static void
_backlight_mismatch_retry(Backlight_Device *bd)
{
   if (// if want 1.0 or normal or dim
       ((fabs(bd->expected_val - 1.0) < DBL_EPSILON) ||
        (fabs(bd->expected_val - e_config->backlight.normal) < DBL_EPSILON) ||
        (fabs(bd->expected_val - e_config->backlight.dim) < DBL_EPSILON)) &&
       // and the delta between expected and val >= 0.05
       (fabs(bd->expected_val - bd->val) >= 0.05) &&
       // and we retried < 20 times
       (bd->retries < 10) &&
       (_own_vt))
     { // try again
        printf("RETRY backlight set as %1.2f != %1.2f (expected) try=%i\n",
               bd->val, bd->expected_val, bd->retries);
        bd->retries++;
        if (bd->retry_timer) ecore_timer_del(bd->retry_timer);
        bd->retry_timer = ecore_timer_add(0.1, _backlight_retry_timer_cb, bd);
     } // or give up
   else bd->retries = 0;
}

static void
_backlight_system_get_cb(void *data, const char *params)
{
   char dev[1024];
   int val = 0;
   double fval;
   Backlight_Device *bd = data;

   if (!params) return;
   if (sscanf(params, "%1023s %i", dev, &val) != 2) return;
   if (!!strcmp(bd->dev, dev)) return;
   e_system_handler_del("bklight-val", _backlight_system_get_cb, bd);
   fval = (double)val / 1000.0;
   if (fabs(fval - bd->val) >= DBL_EPSILON)
     {
        bd->val = fval;
        ecore_event_add(E_EVENT_BACKLIGHT_CHANGE, NULL, NULL, NULL);
        _backlight_mismatch_retry(bd);
     }
}

static void
_backlight_system_ddc_get_cb(void *data, const char *params)
{
   char edid[257];
   int id = -1, val = -1, max = -1;
   double fval;
   Backlight_Device *bd = data;

   if (!params) return;
   if (sscanf(params, "%256s %i %i %i", edid, &id, &val, &max) != 4) return;
   if (!bd->edid) return;
   if (!!strncmp(bd->edid, edid, strlen(edid))) return;
   e_system_handler_del("ddc-val-get", _backlight_system_ddc_get_cb, bd);
   if (val < 0) fval = -1.0;
   else fval = (double)val / 100.0;
   bd->ddc_max = max;
   if ((fabs(fval - bd->val) >= DBL_EPSILON) || (val == -1))
     {
        bd->val = fval;
        ecore_event_add(E_EVENT_BACKLIGHT_CHANGE, NULL, NULL, NULL);
        _backlight_mismatch_retry(bd);
     }
}

static void
_backlight_devices_clear(void)
{
   const char *s;
   Backlight_Device *bd;

   EINA_LIST_FREE(bl_devs, s)
     eina_stringshare_del(s);
   EINA_LIST_FREE(_devices, bd)
     {
        eina_stringshare_del(bd->dev);
        eina_stringshare_del(bd->output);
        eina_stringshare_del(bd->edid);
        if (bd->anim) ecore_animator_del(bd->anim);
        if (bd->retry_timer) ecore_timer_del(bd->retry_timer);
        e_system_handler_del("bklight-val", _backlight_system_get_cb, bd);
        e_system_handler_del("ddc-val-get", _backlight_system_ddc_get_cb, bd);
        free(bd);
     }
}

static Backlight_Device *
_backlight_devices_zone_device_find(E_Zone *zone)
{
   Eina_List *l;
   Backlight_Device *bd;
   char *tmp, *sep;
   const char *out, *edid, *id;

   id = zone->randr2_id;
   if (!id) id = "/";
   tmp = strdup(id);
   if (!tmp) return NULL;
   sep = strchr(tmp, '/');
   if (!sep)
     {
        free(tmp);
        return NULL;
     }
   *sep = '\0';
   out = tmp;
   edid = sep + 1;
   EINA_LIST_FOREACH(_devices, l, bd)
     {
        if ((bd->output) && (!strcmp(out, bd->output)))
          {
             if ((edid[0] && (!strcmp(edid, bd->edid))) || (!edid[0]))
               {
                  free(tmp);
                  return bd;
               }
          }
     }
   free(tmp);
   return NULL;
}

#ifndef HAVE_WAYLAND_ONLY
static const char *
_backlight_devices_randr_out_edid_str_get(Ecore_X_Window root, Ecore_X_Randr_Output *out)
{
   const char *str = NULL;
   unsigned long edid_len = 0;
   unsigned char *edid = ecore_x_randr_output_edid_get(root, *out, &edid_len);
   if (edid)
     {
        char *edid_str = malloc((edid_len * 2) + 1);

        if (edid_str)
          {
             const char *hexch = "0123456789abcdef";
             char *dc = edid_str;
             unsigned long i;

             for (i = 0; i < edid_len; i++)
               {
                  dc[0] = hexch[(edid[i] >> 4) & 0xf];
                  dc[1] = hexch[(edid[i]     ) & 0xf];
                  dc += 2;
               }
             *dc = 0;
             str = eina_stringshare_add(edid_str);
             free(edid_str);
          }
        free(edid);
     }
   return str;
}

static Ecore_X_Randr_Output
_backlight_devices_randr_output_get(Ecore_X_Window root, const char *output, const char *edid)
{
   Ecore_X_Randr_Output *out;
   int i, num = 0;

   out = ecore_x_randr_window_outputs_get(root, &num);
   if ((out) && (num > 0))
     {
        for (i = 0; i < num; i++)
          {
             char *name = ecore_x_randr_output_name_get(root, out[i], NULL);
             if (!name) continue;
             const char *edid_str = _backlight_devices_randr_out_edid_str_get(root, &(out[i]));
             if (edid_str)
               {
                  if ((!strcmp(output, name)) && (!strcmp(edid, edid_str)))
                    {
                       eina_stringshare_del(edid_str);
                       free(name);
                       return out[i];
                    }
                  eina_stringshare_del(edid_str);
               }
             free(name);
          }
     }
   return 0;
}
#endif

static void
_backlight_devices_device_set(Backlight_Device *bd, double val)
{
   if (fabs(bd->expected_val - val) > DBL_EPSILON) bd->retries = 0;
   bd->val = bd->expected_val = val;
   if (!_own_vt) return;
#ifndef HAVE_WAYLAND_ONLY
   if (!strcmp(bd->dev, "randr"))
     {
        if ((e_comp) && (e_comp->comp_type == E_PIXMAP_TYPE_X) && (e_comp->root))
          {
             Ecore_X_Randr_Output o = _backlight_devices_randr_output_get(e_comp->root, bd->output, bd->edid);
             if (o)
               {
//                  fprintf(stderr, "BL: randr bklight %1.3f @ %1.3f\n", bd->val, ecore_time_get());
                  ecore_x_randr_output_backlight_level_set(e_comp->root, o, bd->val);
                  ecore_event_add(E_EVENT_BACKLIGHT_CHANGE, NULL, NULL, NULL);
               }
          }
     }
   else
#endif
   if (!strncmp(bd->dev, "ddc:", 4))
     {
        double fval;

//        fprintf(stderr, "BL: ddc bklight %1.3f @ %1.3f\n", bd->val, ecore_time_get());
        if (bd->ddc_max) fval = bd->val * (double)bd->ddc_max;
        else fval = bd->val * 100.0;
        e_system_send("ddc-val-set", "%s %i %i", bd->dev + 4, 0x10, (int)(fval)); // backlight val in e_system_ddc.c
        ecore_event_add(E_EVENT_BACKLIGHT_CHANGE, NULL, NULL, NULL);
     }
   else
     {
//        fprintf(stderr, "BL: internal bklight %1.3f @ %1.3f\n", bd->val, ecore_time_get());
        e_system_send("bklight-set", "%s %i", bd->dev, (int)(bd->val * 1000.0));
        ecore_event_add(E_EVENT_BACKLIGHT_CHANGE, NULL, NULL, NULL);
     }
}

static E_Zone *
_backlight_devices_device_zone_get(Backlight_Device *bd)
{
   char buf[1024];
   Eina_List *l;
   E_Zone *zone;

   snprintf(buf, sizeof(buf), "%s/%s", bd->output? bd->output: "", bd->edid ? bd->edid : "");
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
     {
        const char *id = zone->randr2_id;
        if (!id)
          {
             const char *id2 = bd->edid;
             if (!id2) id2 = "";
             id = "";
             if (!strcmp(id, id2)) return zone;
          }
        else
          {
             if (!strcmp(id, buf)) return zone;
          }
     }
   return NULL;
}

static void
_backlight_devices_device_update(Backlight_Device *bd)
{
   if (!bd->dev) return;
#ifndef HAVE_WAYLAND_ONLY
   if (!strcmp(bd->dev, "randr"))
     {
        if ((e_comp) && (e_comp->comp_type == E_PIXMAP_TYPE_X) && (e_comp->root))
          {
             Ecore_X_Randr_Output o = _backlight_devices_randr_output_get(e_comp->root, bd->output, bd->edid);
             if (o)
               {
                  double x_bl = ecore_x_randr_output_backlight_level_get(e_comp->root, o);
                  if ((x_bl >= 0.0) && (fabs(x_bl - bd->val) >= DBL_EPSILON))
                    {
                       bd->val = x_bl;
                       E_Zone *zone = _backlight_devices_device_zone_get(bd);
                       if (zone) zone->bl = bd->val;
                       ecore_event_add(E_EVENT_BACKLIGHT_CHANGE, NULL, NULL, NULL);
                    }
               }
          }
     }
   else
#endif
   if (!strncmp(bd->dev, "ddc:", 4))
     {
        e_system_handler_del("ddc-val-get", _backlight_system_ddc_get_cb, bd);
        e_system_handler_add("ddc-val-get", _backlight_system_ddc_get_cb, bd);
        e_system_send("ddc-val-get", "%s %i", bd->dev + 4, 0x10); // backlight val in e_system_ddc.c
     }
   else
     {
        e_system_handler_del("bklight-val", _backlight_system_get_cb, bd);
        e_system_handler_add("bklight-val", _backlight_system_get_cb, bd);
        e_system_send("bklight-get", "%s", bd->dev);
     }
}

static void
_backlight_devices_update(void)
{
   Eina_List *l;
   Backlight_Device *bd;

   EINA_LIST_FOREACH(_devices, l, bd)
     _backlight_devices_device_update(bd);
}

static void
_backlight_devices_pending_done(void)
{
   _devices_pending_ops--;
   if (_devices_pending_ops > 0) return;
   e_backlight_level_set(NULL, e_config->backlight.normal, -1.0);
   _backlight_devices_update();
}

static E_Randr2_Screen *
_backlight_devices_screen_lid_get(void)
{
   Eina_List *l;
   E_Randr2_Screen *sc;

   if (!e_randr2) return NULL;
   EINA_LIST_FOREACH(e_randr2->screens, l, sc)
     {
        if (sc->info.is_lid) return sc;
     }
   EINA_LIST_FOREACH(e_randr2->screens, l, sc)
     {
        if (sc->info.backlight) return sc;
     }
   return NULL;
}

static E_Randr2_Screen *
_backlight_devices_screen_edid_get(const char *edid)
{
   Eina_List *l;
   E_Randr2_Screen *sc;
   const char *id;

   if (!e_randr2) return NULL;
   EINA_LIST_FOREACH(e_randr2->screens, l, sc)
     {
        id = sc->info.edid;
        if (!id) id = "";
        if (!strncmp(id, edid, strlen(edid))) return sc;
     }
   return NULL;
}

static Backlight_Device *
_backlight_devices_edid_find(const char *edid)
{
   Eina_List *l;
   Backlight_Device *bd;

   EINA_LIST_FOREACH(_devices, l, bd)
     {
        if ((bd->edid) && (edid) && (!strcmp(edid, bd->edid)))
          return bd;
     }
   return NULL;
}

static void
_backlight_devices_lid_register(const char *dev, Eina_Bool force)
{
   E_Randr2_Screen *sc = _backlight_devices_screen_lid_get();
   Backlight_Device *bd = NULL;
   const char *id;

   if (!sc) return;
   id = sc->info.edid;
   if (!id) id = "";
   bd = _backlight_devices_edid_find(id);
   if (!bd)
     {
        E_Zone *zone;

        bd = calloc(1, sizeof(Backlight_Device));
        if (!bd) return;
        bd->edid = eina_stringshare_add(id);
        bd->output = eina_stringshare_add(sc->info.name);
        _devices = eina_list_append(_devices, bd);
        zone = _backlight_devices_device_zone_get(bd);
        if (zone)
          {
             double bl = zone->bl;

             zone->bl = 0.0;
             e_backlight_level_set(zone, bl, -1.0);
          }
     }
   if (bd->dev)
     {
        if (!strcmp(bd->dev, "randr")) return; // randr devices win
        if (!strncmp(bd->dev, "ddc:", 4)) return; // ddc devices also beat these "sys" devices
     }
   // if bl device has a device assigned and we're not forcing a change
   // then skip assigning this device to the backlight device
   if ((bd->dev) && (!force)) return;
   eina_stringshare_replace(&(bd->dev), dev);
}

static void
_backlight_devices_edid_register(const char *dev, const char *edid)
{
   E_Randr2_Screen *sc = _backlight_devices_screen_edid_get(edid);
   Backlight_Device *bd;
   if (!sc) return;
   bd = _backlight_devices_edid_find(sc->info.edid);
   if (!bd)
     {
        E_Zone *zone;
        const char *id;

        bd = calloc(1, sizeof(Backlight_Device));
        if (!bd) return;
        id = sc->info.edid;
        if (!id) id = "";
        bd->edid = eina_stringshare_add(id);
        bd->output = eina_stringshare_add(sc->info.name);
        _devices = eina_list_append(_devices, bd);
        zone = _backlight_devices_device_zone_get(bd);
        if (zone)
          {
             double bl = zone->bl;

             zone->bl = 0.0;
             e_backlight_level_set(zone, bl, -1.0);
          }
     }
   if (bd->dev)
     {
        if (!strcmp(bd->dev, "randr")) return; // randr devices win
     }
   eina_stringshare_replace(&(bd->dev), dev);
}

static void
_backlight_system_list_cb(void *data EINA_UNUSED, const char *params)
{
   // params "dev flag dev flag ..."
   const char *p = params;
   char dev[1024], flag, devnum = 0;

   e_system_handler_del("bklight-list", _backlight_system_list_cb, NULL);
   while ((p) && (*p))
     {
        if (sscanf(p, "%1023s", dev) == 1)
          {
             p += strlen(dev);
             if (*p == ' ')
               {
                  p++;
                  flag = *p;
                  if (flag)
                    {
                       bl_devs = eina_list_append
                         (bl_devs, eina_stringshare_add(dev));
                       if ((devnum == 0) || (flag == 'p') ||
                           (strstr(dev, "/drm/")) ||
                           (strstr(dev, ".video.")))
                         {
                            _backlight_devices_lid_register(dev, EINA_TRUE);
                         }
                       else
                         {
                            // XXX: what to do with other bl devices?
                            // for now just assign spare bl devices to screens that do not have one already
                            _backlight_devices_lid_register(dev, EINA_FALSE);
                         }
                       devnum++;
                       // XXX: find bed bl dev for screens not supporting randr
                       // note that this is a vallback and ddc should take precedence
                       // it matched up to a screen over this, but randr should be
                       // the top priority if found
                    }
                  else break;
                  p++;
                  if (*p != ' ') break;
                  p++;
                  if (*p == '\0') break;
               }
             else break;
          }
        else break;
     }
   _backlight_devices_pending_done();
}

static void
_backlight_system_ddc_list_cb(void *data EINA_UNUSED, const char *params)
{
   const char *p = params;
   char dev[257], buf[343];
   Eina_Hash *tmphash;

   tmphash = eina_hash_string_superfast_new(NULL);
   e_system_handler_del("ddc-list", _backlight_system_ddc_list_cb, NULL);
   while ((p) && (*p))
     {
        if (sscanf(p, "%256s", dev) == 1)
          {
             p += strlen(dev);
             snprintf(buf, sizeof(buf), "ddc:%s", dev);
             bl_devs = eina_list_append
               (bl_devs, eina_stringshare_add(buf));
             _backlight_devices_edid_register(buf, dev);
             if (eina_hash_find(tmphash, dev))
               printf("BL: DDC ERROR: You have multiple DDC screens with the same EDID [%s] - this will lead to weirdness.\n", dev);
             eina_hash_add(tmphash, dev, dev);
             if (*p != ' ') break;
          }
        else break;
     }
   eina_hash_free(tmphash);
   _backlight_devices_pending_done();
}

static void
_backlight_devices_probe(Eina_Bool initial)
{
   _backlight_devices_clear();
#ifndef HAVE_WAYLAND_ONLY
   // pass 1: if in x look at all the randr outputs to see if they have some backlight
   // property to swizzle then make the randr device the kind of device to use -
   // record this in our devices list
   if ((e_comp) && (e_comp->comp_type == E_PIXMAP_TYPE_X))
     {
        Eina_Bool avail = ecore_x_randr_output_backlight_available();
        if ((avail) && (e_comp->root))
          {
             double x_bl;
             Ecore_X_Window root = e_comp->root;
             Ecore_X_Randr_Output *out;
             int i, num = 0;
             Eina_Bool found = EINA_FALSE;

             bl_devs = eina_list_append(bl_devs, eina_stringshare_add("randr"));
             out = ecore_x_randr_window_outputs_get(root, &num);
             if ((out) && (num > 0))
               {
                  for (i = 0; i < num; i++)
                    {
                       char *name = ecore_x_randr_output_name_get(root, out[i], NULL);
                       if (!name) continue;
                       x_bl = ecore_x_randr_output_backlight_level_get(root, out[0]);
                       if (x_bl >= 0.0)
                         {
                            Backlight_Device *bd = calloc(1, sizeof(Backlight_Device));
                            if (bd)
                              {
                                 const char *edid_str = _backlight_devices_randr_out_edid_str_get(root, &(out[i]));

                                 if (edid_str)
                                   {
                                      bd->dev = eina_stringshare_add("randr");
                                      bd->output = eina_stringshare_add(name);
                                      bd->edid = edid_str;
                                      _devices = eina_list_append(_devices, bd);
                                      found = EINA_TRUE;
                                   }
                                 else free(bd);
                              }
                         }
                       free(name);
                    }
               }
             free(out);
             if (found)
               e_backlight_level_set(NULL, e_config->backlight.normal, -1.0);
          }
     }
#endif
   // ask enlightenment_system to list backlight devices. this is async so we have
   // to respond to the device listing later
   _devices_pending_ops++;
   e_system_handler_del("bklight-list", _backlight_system_list_cb, NULL);
   e_system_handler_add("bklight-list", _backlight_system_list_cb, NULL);
   if (!initial) e_system_send("bklight-refresh", NULL);
   e_system_send("bklight-list", NULL);
   _devices_pending_ops++;
   e_system_handler_del("ddc-list", _backlight_system_ddc_list_cb, NULL);
   e_system_handler_add("ddc-list", _backlight_system_ddc_list_cb, NULL);
   if (!initial) e_system_send("ddc-refresh", NULL);
   e_system_send("ddc-list", NULL);
   // XXXX: add ddc to e_syystem and query that too
}

static Eina_Bool
_bl_anim(void *data, double pos)
{
   Backlight_Device *bd = data;

   // FIXME: if zone is deleted while anim going... bad things.
   pos = ecore_animator_pos_map(pos, ECORE_POS_MAP_DECELERATE, 0.0, 0.0);
   bd->val  = (bd->from_val * (1.0 - pos)) + (bd->to_val * pos);
   _backlight_devices_device_set(bd, bd->val);
   if (pos >= 1.0)
     {
        bd->anim = NULL;
        _backlight_devices_device_update(bd);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_cb_job_zone_change(void *data EINA_UNUSED)
{
   zone_change_job = NULL;
   _backlight_devices_probe(EINA_FALSE);
   e_backlight_update();
}

static Eina_Bool
_cb_handler_zone_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *info EINA_UNUSED)
{
   if (zone_change_job) ecore_job_del(zone_change_job);
   zone_change_job = ecore_job_add(_cb_job_zone_change, NULL);
   return EINA_TRUE;
}

#ifndef HAVE_WAYLAND_ONLY
static Eina_Bool
_cb_handler_x_window_property(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_X_Event_Window_Property *ev = event;

   if ((e_comp) && (e_comp->comp_type == E_PIXMAP_TYPE_X))
     {
        static Ecore_X_Atom atom_xfree86_has_vt = 0;

        if (!atom_xfree86_has_vt)
          atom_xfree86_has_vt = ecore_x_atom_get("XFree86_has_VT");
        if ((ev->win) && (e_comp->root) && (ev->win == e_comp->root) &&
            (ev->atom == atom_xfree86_has_vt))
          {
             int *val = NULL, num = 0;

             if (ecore_x_window_prop_property_get(ev->win, atom_xfree86_has_vt,
                                                  ECORE_X_ATOM_INTEGER, 32,
                                                  (unsigned char **)&val,
                                                  &num))
               {
                  if (num > 0)
                    {
                       if ((*val == 1) && (!_own_vt)) // have vt
                         {
                            printf("BL: gained VT\n");
                            _own_vt = EINA_TRUE;
                            // we just go back to normal backlight if we gain
                            // the vt again
                            e_backlight_mode_set(NULL, E_BACKLIGHT_MODE_NORMAL);
                         }
                       else if (_own_vt)
                         {
                            printf("BL: lost VT\n");
                            _own_vt = EINA_FALSE;
                         }
                    }
                  free(data);
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}
#endif

EINTERN int
e_backlight_init(void)
{
   E_EVENT_BACKLIGHT_CHANGE = ecore_event_type_new();
   _backlight_devices_probe(EINA_TRUE);
#define H(ev, cb) \
   handlers = eina_list_append(handlers, \
                               ecore_event_handler_add(ev, cb, NULL));
   H(E_EVENT_ZONE_ADD, _cb_handler_zone_change);
   H(E_EVENT_ZONE_DEL, _cb_handler_zone_change);
   H(E_EVENT_ZONE_MOVE_RESIZE, _cb_handler_zone_change);
   H(E_EVENT_ZONE_STOW, _cb_handler_zone_change);
   H(E_EVENT_ZONE_UNSTOW, _cb_handler_zone_change);
#ifndef HAVE_WAYLAND_ONLY
   H(ECORE_X_EVENT_WINDOW_PROPERTY, _cb_handler_x_window_property);
#endif
   e_backlight_update();
   if (!getenv("E_RESTART"))
     e_backlight_level_set(NULL, e_config->backlight.normal, -1.0);
   return 1;
}

EINTERN int
e_backlight_shutdown(void)
{
   Ecore_Event_Handler *h;
   const char *s;

   if (zone_change_job)
     {
        ecore_job_del(zone_change_job);
        zone_change_job = NULL;
     }
   EINA_LIST_FREE(handlers, h)
     ecore_event_handler_del(h);
   EINA_LIST_FREE(bl_devs, s)
     eina_stringshare_del(s);
   _backlight_devices_clear();
   return 1;
}

E_API Eina_Bool
e_backlight_exists(void)
{
   // XXX: properly check backlight devices
   return EINA_TRUE;
}

E_API void
e_backlight_update(void)
{
   if (_devices_pending_ops > 0) _devices_zones_update = EINA_TRUE;
   else _backlight_devices_update();
}

E_API void
e_backlight_level_set(E_Zone *zone, double val, double tim)
{
   Backlight_Device *bd;
   double bl_now;

   if (!zone) // zone == NULL == everything
     {
        Eina_List *l;

        if (!e_comp) return;
        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          e_backlight_level_set(zone, val, tim);
        return;
     }
   bl_now = zone->bl;
   zone->bl = val;

   bd = _backlight_devices_zone_device_find(zone);
   if (!bd) return;
   // set backlight associated with zone to val over period of tim
   // if tim == 0.0 - then do it instantnly, if time == -1 use some default
   // transition time
   if ((!e_comp->screen) || (!e_comp->screen->backlight_enabled)) return;
   if (val < 0.0) val = 0.0;
   else if (val > 1.0) val = 1.0;
   if ((fabs(val - bl_now) < DBL_EPSILON) && (!bd->anim)) return;

   if (fabs(tim) < DBL_EPSILON)
     {
        _backlight_devices_device_set(bd, val);
        _backlight_devices_device_update(bd);
        ecore_event_add(E_EVENT_BACKLIGHT_CHANGE, NULL, NULL, NULL);
        return;
     }
   if (zone->bl_mode == E_BACKLIGHT_MODE_NORMAL) tim = 0.5;
   else if (tim < 0.0) tim = e_config->backlight.transition;

   if ((bd->anim) && (fabs(bd->to_val - val) < DBL_EPSILON)) return;

   E_FREE_FUNC(bd->retry_timer, ecore_timer_del);
   E_FREE_FUNC(bd->anim, ecore_animator_del);
   bd->from_val = bl_now;
   bd->to_val = val;
   bd->anim = ecore_animator_timeline_add(tim, _bl_anim, bd);
}

E_API double
e_backlight_level_get(E_Zone *zone)
{
   Backlight_Device *bd;

   if (!zone)
     {
        zone = e_zone_current_get();
        if (!zone) return -1.0;
     }
   bd = _backlight_devices_zone_device_find(zone);
   if (!bd) return -1.0;
   zone->bl = bd->val;
   return bd->val;
}

E_API void
e_backlight_mode_set(E_Zone *zone, E_Backlight_Mode mode)
{
   E_Backlight_Mode pmode;

   if (!zone) // zone == NULL == everything
     {
        Eina_List *l;

        if (!e_comp) return;
        EINA_LIST_FOREACH(e_comp->zones, l, zone)
          e_backlight_mode_set(zone, mode);
        return;
     }
   if (zone->bl_mode == mode) return;
   if (eina_streq(ecore_evas_engine_name_get(e_comp->ee), "buffer") ||
       strstr(ecore_evas_engine_name_get(e_comp->ee), "wayland"))
     return;
   pmode = zone->bl_mode;
   zone->bl_mode = mode;
   if (zone->bl_mode == E_BACKLIGHT_MODE_NORMAL)
     {
        e_backlight_level_set(zone, e_config->backlight.normal, -1.0);
     }
   else if (zone->bl_mode == E_BACKLIGHT_MODE_OFF)
     {
        e_backlight_level_set(zone, 0.0, -1.0);
     }
   else if (zone->bl_mode == E_BACKLIGHT_MODE_DIM)
     {
        if ((pmode != E_BACKLIGHT_MODE_NORMAL) ||
            ((pmode == E_BACKLIGHT_MODE_NORMAL) &&
             (e_config->backlight.normal > e_config->backlight.dim)))
          e_backlight_level_set(zone, e_config->backlight.dim, -1.0);
     }
   else if (zone->bl_mode == E_BACKLIGHT_MODE_MAX)
     e_backlight_level_set(zone, 1.0, -1.0);
}

E_API E_Backlight_Mode
e_backlight_mode_get(E_Zone *zone)
{
   if (!zone)
     {
        zone = e_zone_current_get();
        if (!zone) return E_BACKLIGHT_MODE_NORMAL;
     }
   return zone->bl_mode;
}

E_API const Eina_List *
e_backlight_devices_get(void)
{
   return bl_devs;
}

E_API void
e_backlight_suspend_set(Eina_Bool suspend)
{
   if (_e_bl_suspend == suspend) return;
   _e_bl_suspend = suspend;
   _own_vt = !_e_bl_suspend;
   if (_own_vt)
     {
        // we just go back to normal backlight if we gain
        // the vt again
        e_backlight_mode_set(NULL, E_BACKLIGHT_MODE_NORMAL);
     }
}
