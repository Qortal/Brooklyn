#include "e_system.h"

typedef struct
{
   char *dev;
   int val, max, val_set, val_get_count;
   Eina_Bool prefer : 1;
   Eina_Bool set : 1;
} Light;

static Eina_Lock _devices_lock;
static Eina_List *_devices = NULL;
static Eina_Semaphore _worker_sem;

static void
_light_set(Light *lig, int val)
{ // val == 0->1000
   int nval = ((lig->max * val) + 500) / 1000;
   if (nval < 0) nval = 0;
   else if (nval > lig->max) nval = lig->max;
   lig->val = nval;
#ifdef HAVE_EEZE
   char buf[PATH_MAX];
   int fd;
   snprintf(buf, sizeof(buf), "%s/brightness", lig->dev);
   fd = open(buf, O_WRONLY);
   if (fd >= 0)
     {
        char buf2[32];
        snprintf(buf2, sizeof(buf2), "%i", lig->val);
        if (write(fd, buf2, strlen(buf2)) <= 0)
          ERR("Write failed of [%s] to [%s]\n", buf2, buf);
        close(fd);
     }
   snprintf(buf, sizeof(buf), "%s/bl_power", lig->dev);
   fd = open(buf, O_WRONLY);
   if (fd >= 0)
     {
        int ret;

        if (val == 0) ret = write(fd, "4", 1);
        else ret = write(fd, "0", 1);
        if (ret < 0) ERR("Write failed to [%s]\n", buf);
        close(fd);
     }
#elif defined(__FreeBSD_kernel__)
#if __FreeBSD__ > 12
   int fd = open("/dev/backlight/backlight0", O_RDWR);
   if (fd >= 0)
     {
        struct backlight_props props;
        props.brightness = lig->val;
        ioctl(fd, BACKLIGHTUPDATESTATUS, &props);
        close(fd);
     }
#else
   sysctlbyname(lig->dev, NULL, NULL, &(lig->val), sizeof(lig->val));
#endif
#endif
}

static void
_light_get(Light *lig)
{
#ifdef HAVE_EEZE
   const char *s;
   s = eeze_udev_syspath_get_sysattr(lig->dev, "bl_power");
   if (s)
     {
        if (atoi(s) != 0)
          {
             lig->val = 0;
             eina_stringshare_del(s);
             return;
          }
        eina_stringshare_del(s);
     }
   s = eeze_udev_syspath_get_sysattr(lig->dev, "actual_brightness");
   if (!s) s = eeze_udev_syspath_get_sysattr(lig->dev, "brightness");
   if (s)
     {
        lig->val = atoi(s);
        eina_stringshare_del(s);
     }
#elif defined(__FreeBSD_kernel__)
#if __FreeBSD__ > 12
   struct backlight_props props;
   int fd = open("/dev/backlight/backlight0", O_RDWR);
   if (fd >= 0)
     {
        if (ioctl(fd, BACKLIGHTGETSTATUS, &props) != -1)
          {
             lig->val = props.brightness;
          }
        close(fd);
     }
#else
   size_t plen = sizeof(lig->val);
   sysctlbyname(lig->dev, &(lig->val), &plen, NULL, 0);
#endif
#endif
}

static void
_cb_worker(void *data EINA_UNUSED, Ecore_Thread *th)
{
   for (;;)
     {
        Eina_List *l;
        Light *lig;

        eina_semaphore_lock(&_worker_sem);

        eina_lock_take(&_devices_lock);
        EINA_LIST_FOREACH(_devices, l, lig)
          {
             if (lig->val_get_count > 0)
               {
                  _light_get(lig);
                  while (lig->val_get_count > 0)
                    {
                       Light *lig2 = calloc(1, sizeof(Light));
                       lig->val_get_count--;
                       if (lig2)
                         {
                            lig2->dev = strdup(lig->dev);
                            lig2->max = lig->max;
                            if (lig2->dev)
                              {
                                 lig2->val = lig->val;
                                 ecore_thread_feedback(th, lig2);
                              }
                            else free(lig2);
                         }
                    }
               }
             if (lig->set)
               {
                  lig->set = EINA_FALSE;
                  lig->val = lig->val_set;
                  _light_set(lig, lig->val);
               }
          }
        eina_lock_release(&_devices_lock);
     }
}

static void
_cb_worker_message(void *data EINA_UNUSED, Ecore_Thread *th EINA_UNUSED, void *msg_data)
{
   Light *lig = msg_data;
   int val = 0;

   if (lig->max > 0)
     {
        val = ((1000 * lig->val) + 500) / lig->max;
        if (val < 0) val = 0;
        else if (val > 1000) val = 1000;
     }
   e_system_inout_command_send("bklight-val", "%s %i", lig->dev, val);
   free(lig->dev);
   free(lig);
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
_light_add(const char *dev)
{
   Light *lig = calloc(1, sizeof(Light));

   if (!lig) abort();
   lig->dev = strdup(dev);
   if (!lig->dev) abort();
   lig->val = -1; // unknown
#ifdef HAVE_EEZE
   const char *s;

   s = eeze_udev_syspath_get_sysattr(lig->dev, "brightness");
   if (!s)
     {
        free(lig->dev);
        free(lig);
        return;
     }
   lig->val = atoi(s);
   eina_stringshare_del(s);

   s = eeze_udev_syspath_get_sysattr(lig->dev, "max_brightness");
   if (s)
     {
        lig->max = atoi(s);
        eina_stringshare_del(s);
     }
   if (lig->max <= 0) lig->max = 255;
   lig->prefer = eeze_udev_syspath_check_sysattr(lig->dev, "type", "firmware");
#elif defined (__FreeBSD_kernel__)
#if __FreeBSD__ > 12
   struct backlight_props props;
   int fd = open("/dev/backlight/backlight0", O_RDWR);
   if (fd >= 0)
     {
        if (ioctl(fd, BACKLIGHTGETSTATUS, &props) != -1)
          {
             lig->val = props.brightness;
          }
        close(fd);
     }
# else
   size_t plen = sizeof(lig->val);
   sysctlbyname(lig->dev, &(lig->val), &plen, NULL, 0);
#endif
   lig->max = 100;
#endif
   _devices = eina_list_append(_devices, lig);
   if (alert_backlight_reset)
     {  // set brightness to max if alert mode is on
        eina_lock_take(&_devices_lock);
        lig->val_set = lig->max;
        lig->set = EINA_TRUE;
        eina_semaphore_release(&_worker_sem, 1);
        eina_lock_release(&_devices_lock);
     }
}

#ifdef HAVE_EEZE
static Eina_Bool
_light_device_include(const char *dev)
{ // filter out known undesirable devices
   fprintf(stderr, "BL: found [%s]\n", dev);
   if (strstr(dev, "::capslock")) return EINA_FALSE;
   if (strstr(dev, "::numlock")) return EINA_FALSE;
   if (strstr(dev, "::scrolllock")) return EINA_FALSE;
   if (strstr(dev, "::compose")) return EINA_FALSE;
   if (strstr(dev, "::kana")) return EINA_FALSE;
   return EINA_TRUE;
}
#endif

static void
_light_refresh_devices()
{
   Light *lig;

   EINA_LIST_FREE(_devices, lig)
     {
        free(lig->dev);
        free(lig);
     }
#ifdef HAVE_EEZE
   Eina_List *devs;
   const char *s;

   devs = eeze_udev_find_by_filter("backlight", NULL, NULL);
   fprintf(stderr, "BL: backlight devices...\n");
   EINA_LIST_FREE(devs, s)
     {
        if (_light_device_include(s)) _light_add(s);
        eina_stringshare_del(s);
     }
   devs = eeze_udev_find_by_filter("leds", NULL, NULL);
   fprintf(stderr, "BL: led devices...\n");
   EINA_LIST_FREE(devs, s)
     {
        if (_light_device_include(s)) _light_add(s);
        eina_stringshare_del(s);
     }
#elif defined(__FreeBSD_kernel__)
   // XXX; shouldn't we scan for devices?
#if __FreeBSD__ > 12
   _light_add("backlight0");
#else
   _light_add("hw.acpi.video.lcd0.brightness");
#endif
#endif
}

static Light *
_light_find(const char *dev)
{
   Eina_List *l;
   Light *lig;

   if (!dev) return NULL;
   EINA_LIST_FOREACH(_devices, l, lig)
     {
        if (!strcmp(lig->dev, dev)) return lig;
     }
   return NULL;
}

static void
_cb_bklight_list(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{ // reply = "dev1 -|p dev2 -|p ..."
   Eina_List *l;
   Light *lig;
   char *rep = NULL, *p = NULL;
   size_t repsize = 0;

   eina_lock_take(&_devices_lock);
   EINA_LIST_FOREACH(_devices, l, lig)
     {
        repsize += strlen(lig->dev) + 2 + 1;
     }
   if (repsize > 0)
     {
        rep = malloc(repsize);
        if (!rep) abort();
        p = rep;
        EINA_LIST_FOREACH(_devices, l, lig)
          {
             size_t len = strlen(lig->dev);
             memcpy(p, lig->dev, len + 1);
             p[len] = ' '; len++;
             if (lig->prefer) p[len] = 'p';
             else p[len] = '-';
             len++;
             if (l->next) p[len] = ' ';
             else p[len] = '\0';
             p += len + 1;
          }
     }
   eina_lock_release(&_devices_lock);
   e_system_inout_command_send("bklight-list", "%s", rep);
   free(rep);
}

static void
_cb_bklight_refresh(void *data EINA_UNUSED, const char *params EINA_UNUSED)
{
   Eina_Bool tmp = alert_backlight_reset;
   alert_backlight_reset = EINA_FALSE;
   eina_lock_take(&_devices_lock);
   _light_refresh_devices();
   eina_lock_release(&_devices_lock);
   alert_backlight_reset = tmp;
}

static void
_cb_bklight_get(void *data EINA_UNUSED, const char *params)
{
   Light *lig;

   eina_lock_take(&_devices_lock);
   lig = _light_find(params);
   if (!lig) goto done;
   lig->val_get_count++;
   eina_semaphore_release(&_worker_sem, 1);
   _light_get(lig);
done:
   eina_lock_release(&_devices_lock);
}

static void
_cb_bklight_set(void *data EINA_UNUSED, const char *params)
{
   Light *lig;
   char dev[1024] = "";
   int val = 0;

   if (!params) return;
   if (sscanf(params, "%1023s %i", dev, &val) != 2) return;
   eina_lock_take(&_devices_lock);
//   fprintf(stderr, "BL: set [%s] -> %i\n", dev, val);
   lig = _light_find(dev);
   if (!lig) goto done;
   lig->val_set = val;
   lig->set = EINA_TRUE;
   eina_semaphore_release(&_worker_sem, 1);
done:
   eina_lock_release(&_devices_lock);
}

void
e_system_backlight_init(void)
{
   eina_lock_new(&_devices_lock);
   eina_semaphore_new(&_worker_sem, 0);
   _light_refresh_devices();
   ecore_thread_feedback_run(_cb_worker, _cb_worker_message,
                             _cb_worker_end, _cb_worker_cancel,
                             NULL, EINA_TRUE);
   e_system_inout_command_register("bklight-list",    _cb_bklight_list, NULL);
   e_system_inout_command_register("bklight-refresh", _cb_bklight_refresh, NULL);
   e_system_inout_command_register("bklight-get",     _cb_bklight_get, NULL);
   e_system_inout_command_register("bklight-set",     _cb_bklight_set, NULL);
}

void
e_system_backlight_shutdown(void)
{
   // only shutdown things we really have to - no need to free mem etc.
}
