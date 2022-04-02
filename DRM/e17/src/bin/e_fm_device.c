#include "e_fm_device.h"
#include "e_fm_shared_codec.h"
#include "e_fm_shared_device.h"

static void _e_fm2_volume_write(E_Volume *v) EINA_ARG_NONNULL(1);
static void _e_fm2_volume_erase(E_Volume *v) EINA_ARG_NONNULL(1);
static void _e_fm2_device_mount_ok(E_Fm2_Mount *m) EINA_ARG_NONNULL(1);
static void _e_fm2_device_mount_fail(E_Fm2_Mount *m) EINA_ARG_NONNULL(1);
static void _e_fm2_device_unmount_ok(E_Fm2_Mount *m) EINA_ARG_NONNULL(1);
static void _e_fm2_device_unmount_fail(E_Fm2_Mount *m) EINA_ARG_NONNULL(1);

static Eina_List *_e_stores = NULL;
static Eina_List *_e_vols = NULL;

static Eina_Bool _check_run_show = EINA_FALSE;
static Eina_Bool _check_run_hide = EINA_FALSE;
static Ecore_Thread *_check_vols = NULL;

static inline E_Fm2_Device_Mount_Op *
_e_fm2_device_mount_op_new(char *args, size_t size, size_t length)
{
   E_Fm2_Device_Mount_Op *mop;

   mop = E_NEW(E_Fm2_Device_Mount_Op, 1);
   mop->args = args;
   mop->size = size;
   mop->length = length;
   return mop;
}

static inline void
_e_fm2_device_mount_op_free(E_Fm2_Device_Mount_Op *mop)
{
   if (!mop) return;
   free(mop->args);
   free(mop);
}

static void
_e_fm2_device_volume_setup(E_Volume *v)
{
   char label[1024] = {0};
   char *size = NULL;
   const char *icon = NULL;
   /* unsigned long long sz; */

   /* Compute the size in a readable form */
   if (v->size)
     size = e_util_size_string_get(v->size);

   /* Choose the label */
   if ((v->label) && (v->label[0]))
     {}else if ((v->partition_label) && (v->partition_label[0]))
     snprintf(label, sizeof(label) - 1, "%s", v->partition_label);
   else if (((v->storage->vendor) && (v->storage->vendor[0])) &&
            ((v->storage->model) && (v->storage->model[0])))
     {
        if (size && (size[0] != '\0'))
          snprintf(label, sizeof(label) - 1, _("%s %s—%s"), v->storage->vendor, v->storage->model, size);
        else
          snprintf(label, sizeof(label) - 1, "%s %s", v->storage->vendor, v->storage->model);
     }
   else if ((v->storage->model) && (v->storage->model[0]))
     {
        if (size && (size[0] != '\0'))
          snprintf(label, sizeof(label) - 1, _("%s—%s"), v->storage->model, size);
        else
          snprintf(label, sizeof(label) - 1, "%s", v->storage->model);
     }
   else if ((v->storage->vendor) && (v->storage->vendor[0]))
     {
        if (size && (size[0] != '\0'))
          snprintf(label, sizeof(label) - 1, _("%s—%s"), v->storage->vendor, size);
        else
          snprintf(label, sizeof(label) - 1, "%s", v->storage->vendor);
     }
   else if (v->storage->drive_type && (!strcmp(v->storage->drive_type, "sd_mmc")))
     snprintf(label, sizeof(label) - 1, _("Flash Card—%s"), size);
   else
     snprintf(label, sizeof(label), _("Unknown Volume"));

   if (label[0]) eina_stringshare_replace(&v->label, label);

   /* Choose the icon */
   /* http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html */
   if (v->storage->icon.volume)
     icon = v->storage->icon.volume;
   else
     {
        if ((!v->storage->drive_type) || (!strcmp(v->storage->drive_type, "disk")) || (!strcmp(v->storage->drive_type, "partition")))
          {
             if (v->storage->removable == 0)
               icon = "drive-harddisk";
             else
               icon = "drive-removable-media";
          }
        else if ((!strcmp(v->storage->drive_type, "cdrom")) || (!strncmp(v->storage->drive_type, "optical", 7)))
          icon = "drive-optical";
        else if (!strncmp(v->storage->drive_type, "floppy", 6))
          icon = "media-floppy";
        else if (!strcmp(v->storage->drive_type, "tape"))
          icon = "media-tape";
        else if (!strcmp(v->storage->drive_type, "thumb"))
          // XXX: maybe a spcial type?
          icon = "drive-removable-media";
        else if (!strcmp(v->storage->drive_type, "compact_flash")
                 || !strcmp(v->storage->drive_type, "memory_stick")
                 || !strcmp(v->storage->drive_type, "smart_media")
                 || !strcmp(v->storage->drive_type, "sd_mmc")
                 || !strncmp(v->storage->drive_type, "flash", 5))
          icon = "media-flash";
        else // XXX: fallback if we don't know
          icon = "media-floppy";
     }
   if (icon) eina_stringshare_replace(&v->icon, icon);

   if ((!v->mount_point) ||
       // filter out these mountouts - hack hack
       (strcmp(v->mount_point, "/") &&
        strcmp(v->mount_point, "/home") &&
        strcmp(v->mount_point, "/tmp")))
     _e_fm2_volume_write(v);

   E_FREE(size);
}

E_API void
e_fm2_device_storage_add(E_Storage *s)
{
   Eina_List *l;
   E_Volume *v;

   if (e_fm2_device_storage_find(s->udi)) return;

   s->validated = EINA_TRUE;
   _e_stores = eina_list_append(_e_stores, s);
/*
   printf("STO+\n"
          "  udi: %s\n"
          "  bus: %s\n"
          "  drive_type: %s\n"
          "  model: %s\n"
          "  vendor: %s\n"
          "  serial: %s\n"
          "  removable: %i\n"
          "  media_available: %i\n"
          "  media_size: %lli\n"
          "  requires_eject: %i\n"
          "  hotpluggable: %i\n"
          "  media_check_enabled: %i\n"
          "  icon.drive: %s\n"
          "  icon.volume: %s\n\n"
          ,
          s->udi,
          s->bus,
          s->drive_type,
          s->model,
          s->vendor,
          s->serial,
          s->removable,
          s->media_available,
          s->media_size,
          s->requires_eject,
          s->hotpluggable,
          s->media_check_enabled,
          s->icon.drive,
          s->icon.volume);
 */
   if ((s->removable == 0) &&
       (s->media_available == 0) &&
       (s->media_size == 0) &&
       (s->requires_eject == 0) &&
       (s->hotpluggable == 0) &&
       (s->media_check_enabled == 0))
     {
//	printf("      Ignore this storage\n");
     }
   else
     {
        s->trackable = EINA_TRUE;
     }

   EINA_LIST_FOREACH(_e_vols, l, v) /* catch volumes which were added before their storage */
     {
        if ((!v->storage) && (s->udi == v->parent))
          {
             v->storage = s;
             _e_fm2_device_volume_setup(v);
          }
     }
}

E_API void
e_fm2_device_storage_del(E_Storage *s)
{
//   printf("STO- %s\n", s->udi);
   _e_stores = eina_list_remove(_e_stores, s);
   _e_fm_shared_device_storage_free(s);
}

E_API E_Storage *
e_fm2_device_storage_find(const char *udi)
{
   Eina_List *l;
   E_Storage *s;

   if (!udi) return NULL;
   EINA_LIST_FOREACH(_e_stores, l, s)
     {
        if (!strcmp(udi, s->udi)) return s;
     }
   return NULL;
}

E_API void
e_fm2_device_volume_add(E_Volume *v)
{
   E_Storage *s;

   if (e_fm2_device_volume_find(v->udi)) return;

   v->validated = EINA_TRUE;
   _e_vols = eina_list_append(_e_vols, v);
/*
   printf("VOL+\n"
          "  udi: %s\n"
          "  uuid: %s\n"
          "  fstype: %s\n"
          "  size: %llu\n"
          "  label: %s\n"
          "  partition: %d\n"
          "  partition_number: %d\n"
          "  partition_label: %s\n"
          "  mounted: %d\n"
          "  mount_point: %s\n"
          "  parent: %s\n"
          ,
          v->udi,
          v->uuid,
          v->fstype,
          v->size,
          v->label,
          v->partition,
          v->partition_number,
          v->partition ? v->partition_label : "(not a partition)",
          v->mounted,
          v->mount_point,
          v->parent);
 */
/* Check mount point */
   if ((v->efm_mode == EFM_MODE_USING_HAL_MOUNT) &&
       ((!v->mount_point) || (!v->mount_point[0])))
     {
        eina_stringshare_del(v->mount_point);
        v->mount_point = NULL;
        v->mount_point = e_fm2_device_volume_mountpoint_get(v);
        if ((!v->mount_point) || (!v->mount_point[0]))
          {
             char buf[PATH_MAX];
             const char *id;

             if (v->mount_point) eina_stringshare_del(v->mount_point);
             v->mount_point = NULL;
             id = "disk";
             if ((v->uuid) && (v->uuid[0])) id = v->uuid;
             if (ecore_file_is_dir("/media"))
               snprintf(buf, sizeof(buf), "/media/%s", id);
             else if (ecore_file_is_dir("/mnt"))
               snprintf(buf, sizeof(buf), "/mnt/%s", id);
             else if (ecore_file_is_dir("/tmp"))
               snprintf(buf, sizeof(buf), "/tmp/%s", id);
             else
               buf[0] = 0;
             v->mount_point = eina_stringshare_add(buf);
          }
     }

   /* Search parent storage */
   if ((s = e_fm2_device_storage_find(v->parent)))
     {
        v->storage = s;
        s->volumes = eina_list_append(s->volumes, v);
     }

   if (v->storage) _e_fm2_device_volume_setup(v);
}

E_API void
e_fm2_device_volume_del(E_Volume *v)
{
   E_Fm2_Mount *m;

//   printf("VOL- %s\n", v->udi);
   _e_vols = eina_list_remove(_e_vols, v);
   _e_fm2_volume_erase(v);
   if (v->storage)
     {
        v->storage->volumes = eina_list_remove(v->storage->volumes, v);
        v->storage = NULL;
     }
   EINA_LIST_FREE(v->mounts, m)
     {
        _e_fm2_device_unmount_ok(m);
        e_fm2_device_mount_free(m);
     }
   while (v->mount_ops)
     {
        E_Fm2_Device_Mount_Op *mop;

        mop = (E_Fm2_Device_Mount_Op *)v->mount_ops;
        v->mount_ops = eina_inlist_remove(v->mount_ops, v->mount_ops);
        _e_fm2_device_mount_op_free(mop);
     }
   _e_fm_shared_device_volume_free(v);
}

static void
_e_fm2_volume_write(E_Volume *v)
{
   char buf[PATH_MAX], buf2[PATH_MAX];
   FILE *f;
   const char *id;

   if (!v->storage) return;
   id = ecore_file_file_get(v->storage->udi);
   e_user_dir_snprintf(buf, sizeof(buf), "fileman/favorites/|%s_%d.desktop",
                       id, v->partition_number);

   f = fopen(buf, "w");
   if (f)
     {
        fprintf(f,
                "[Desktop Entry]\n"
                "Encoding=UTF-8\n"
                "Type=Link\n"
                "X-Enlightenment-Type=Removable\n"
                "X-Enlightenment-Removable-State=Full\n"
                "Name=%s\n"
                "Icon=%s\n"
                "Comment=%s\n"
                "URL=file:/%s\n"
                ,
                v->label,
                v->icon,
                _("Removable Device"),
                v->udi);
        fclose(f);

        if (e_config->device_desktop)
          {
             snprintf(buf2, sizeof(buf2), "%s/|%s_%d.desktop",
                      efreet_desktop_dir_get(), id, v->partition_number);
             ecore_file_symlink(buf, buf2);
          }

        /* FIXME: manipulate icon directly */
        _e_fm2_file_force_update(buf);
        _e_fm2_file_force_update(buf2);
     }
}

static void
_e_fm2_volume_erase(E_Volume *v)
{
   char buf[PATH_MAX] = {0};
   const char *id;

   if (!v->storage) return;
   id = ecore_file_file_get(v->storage->udi);
   if (e_config->device_desktop)
     {
        snprintf(buf, sizeof(buf), "%s/|%s_%d.desktop",
                 efreet_desktop_dir_get(), id, v->partition_number);
        ecore_file_unlink(buf);
        _e_fm2_file_force_update(buf);
     }
   e_user_dir_snprintf(buf, sizeof(buf),
                       "fileman/favorites/|%s_%d.desktop",
                       id, v->partition_number);
   ecore_file_unlink(buf);
   _e_fm2_file_force_update(buf);
}

E_API E_Volume *
e_fm2_device_volume_find(const char *udi)
{
   Eina_List *l;
   E_Volume *v;

   if (!udi) return NULL;

   EINA_LIST_FOREACH(_e_vols, l, v)
     {
        if (!v->udi) continue;
        if (!strcmp(udi, v->udi)) return v;
     }

   return NULL;
}

E_API E_Volume *
e_fm2_device_volume_find_fast(const char *udi)
{
   Eina_List *l;
   E_Volume *v;

   if (!udi) return NULL;

   EINA_LIST_FOREACH(_e_vols, l, v)
     if (udi == v->udi) return v;

   return NULL;
}

E_API const char *
e_fm2_device_volume_mountpoint_get(E_Volume *v)
{
   char buf[PATH_MAX] = {0};

   if (!v) return NULL;
   if (v->mount_point)
     {
        //	printf("GET MOUNTPOINT = %s\n", v->mount_point);
        return eina_stringshare_add(v->mount_point);
     }
   else if (v->efm_mode != EFM_MODE_USING_HAL_MOUNT)
     return NULL;

   if (v->label && v->label[0] != '\0')
     snprintf(buf, sizeof(buf) - 1, "/media/%s", v->label);
   else if (v->uuid && v->uuid[0] != '\0')
     snprintf(buf, sizeof(buf) - 1, "/media/%s", v->uuid);
   else if ((v->storage) && (v->storage->serial) && v->storage->serial[0] != '\0')
     snprintf(buf, sizeof(buf) - 1, "/media/%s", v->storage->serial);
   else
     {
        static int mount_count = 1;
        snprintf(buf, sizeof(buf) - 1, "/media/unknown-%i", mount_count++);
     }
//   printf("GET MOUNTPOINT = %s\n", buf);
   return eina_stringshare_add(buf);
}

E_API E_Fm2_Device_Mount_Op *
e_fm2_device_mount_op_add(E_Fm2_Mount *m, char *args, size_t size, size_t length)
{
   E_Fm2_Device_Mount_Op *mop;

   mop = _e_fm2_device_mount_op_new(args, size, length);
   m->volume->mount_ops = eina_inlist_append(m->volume->mount_ops, EINA_INLIST_GET(mop));
   return mop;
}

E_API void
e_fm2_device_mount_add(E_Volume *v,
                       const char *mountpoint)
{
   v->mounted = EINA_TRUE;
   if (mountpoint && (mountpoint[0]))
     eina_stringshare_replace(&v->mount_point, mountpoint);

   E_LIST_FOREACH(v->mounts, _e_fm2_device_mount_ok);
//   printf("MOUNT %s %s\n", v->udi, v->mount_point);
}

E_API void
e_fm2_device_mount_del(E_Volume *v)
{
   E_Fm2_Mount *m;

   v->mounted = EINA_FALSE;
   if (v->mount_point)
     {
        eina_stringshare_del(v->mount_point);
        v->mount_point = NULL;
     }

   EINA_LIST_FREE(v->mounts, m)
     {
        _e_fm2_device_unmount_ok(m);
        e_fm2_device_mount_free(m);
     }
}

E_API void
e_fm2_device_mount_free(E_Fm2_Mount *m)
{
   if (!m) return;

   eina_stringshare_del(m->udi);
   eina_stringshare_del(m->mount_point);

   free(m);
}

E_API E_Fm2_Mount *
e_fm2_device_mount_find(const char *path)
{
   Eina_List *l;
   E_Volume *v;

   if (!path) return NULL;

   EINA_LIST_FOREACH(_e_vols, l, v)
     {
        if (v->mounted
            && v->mount_point
            && !strncmp(path, v->mount_point, strlen(v->mount_point))
            && v->mounts)
          return eina_list_data_get(v->mounts);
     }
   return NULL;
}

E_API E_Fm2_Mount *
e_fm2_device_mount(E_Volume *v,
                   Ecore_Cb mount_ok,
                   Ecore_Cb mount_fail,
                   Ecore_Cb unmount_ok,
                   Ecore_Cb unmount_fail,
                   void *data)
{
   E_Fm2_Mount *m;

   if (!v) return NULL;

   m = calloc(1, sizeof(E_Fm2_Mount));
   if (!m) return NULL;
   m->udi = eina_stringshare_add(v->udi);
   m->mount_ok = mount_ok;
   m->mount_fail = mount_fail;
   m->unmount_ok = unmount_ok;
   m->unmount_fail = unmount_fail;
   m->data = data;
   m->volume = v;
   m->mounted = v->mounted;

   v->mounts = eina_list_prepend(v->mounts, m);

   if (!v->mounted)
     {
        //printf("BEGIN MOUNT %p '%s'\n", m, v->mount_point);
        v->auto_unmount = EINA_TRUE;
        _e_fm2_client_mount(v->udi, v->mount_point);
     }
   else
     {
        v->auto_unmount = EINA_FALSE;
        m->mount_point = eina_stringshare_add(v->mount_point);
     }

   return m;
}

E_API void
e_fm2_device_mount_fail(E_Volume *v)
{
   E_Fm2_Mount *m;

   v->mounted = EINA_FALSE;
   if (v->mount_point)
     {
        eina_stringshare_del(v->mount_point);
        v->mount_point = NULL;
     }

   EINA_LIST_FREE(v->mounts, m)
     {
        _e_fm2_device_mount_fail(m);
        e_fm2_device_mount_free(m);
     }
}

E_API void
e_fm2_device_unmount(E_Fm2_Mount *m)
{
   E_Volume *v;

   if (!(v = m->volume)) return;
   if (v->mount_ops)
     {
        m->deleted = EINA_TRUE;
        return;
     }
   v->mounts = eina_list_remove(v->mounts, m);
   e_fm2_device_mount_free(m);

   if (v->auto_unmount && v->mounted && !eina_list_count(v->mounts))
     {
        //printf("BEGIN UNMOUNT %p '%s'\n", m, v->udi);
        _e_fm2_client_unmount(v->udi);
     }
}

E_API void
e_fm2_device_unmount_fail(E_Volume *v)
{
   Eina_List *l;
   E_Fm2_Mount *m;

   v->mounted = EINA_TRUE;

   EINA_LIST_FOREACH(v->mounts, l, m)
     _e_fm2_device_unmount_fail(m);
}

static void
_e_fm2_device_mount_ok(E_Fm2_Mount *m)
{
   if (m->mounted) return;
   m->mounted = EINA_TRUE;
   if (m->volume)
     m->mount_point = eina_stringshare_add(m->volume->mount_point);
   if (m->mount_ok)
     m->mount_ok(m->data);
   if (m->deleted) ecore_job_add((Ecore_Cb)e_fm2_device_unmount, m);
   //printf("MOUNT OK '%s'\n", m->mount_point);
}

static void
_e_fm2_device_mount_fail(E_Fm2_Mount *m)
{
   //printf("MOUNT FAIL '%s'\n", m->mount_point);
   m->mounted = EINA_FALSE;
   if (m->mount_point)
     {
        eina_stringshare_del(m->mount_point);
        m->mount_point = NULL;
     }
   if (m->mount_fail)
     m->mount_fail(m->data);
}

static void
_e_fm2_device_unmount_ok(E_Fm2_Mount *m)
{
   if (!m->mounted) return;
   m->mounted = EINA_FALSE;
   if (m->mount_point)
     {
        eina_stringshare_del(m->mount_point);
        m->mount_point = NULL;
     }
   if (m->unmount_ok)
     m->unmount_ok(m->data);
}

static void
_e_fm2_device_unmount_fail(E_Fm2_Mount *m)
{
   if (m->mounted) return;
   m->mounted = EINA_TRUE;
   if (m->unmount_fail)
     m->unmount_fail(m->data);
}

static void
_e_fm2_device_check_desktop_icons_cb_end(void *data EINA_UNUSED, Ecore_Thread *eth EINA_UNUSED)
{
   _check_vols = NULL;
   if (_check_run_show)
     e_fm2_device_show_desktop_icons();
   else if (_check_run_hide)
     e_fm2_device_hide_desktop_icons();
   _check_run_hide = _check_run_show = EINA_FALSE;
}

static void
_e_fm2_device_check_desktop_icons_list_cb(const char *name, const char *path, void *data)
{
   char buf[PATH_MAX], buf2[PATH_MAX];
   Eina_List *l;
   char *file;

   if (path[0] != '|') return;  // not e17 link
   if (!eina_str_has_extension(name, ".desktop")) return;
   snprintf(buf, sizeof(buf), "%s/%s", path, name);
   if (ecore_file_size(buf) > 200) return;  // unlikely that we're going to write a desktop file this long
   e_user_dir_snprintf(buf2, sizeof(buf2), "fileman/favorites/%s", name);
   if (!ecore_file_exists(buf2)) return;
   EINA_LIST_FOREACH(data, l, file)
     if (!strcmp(file, buf)) return;  // valid link

   ecore_file_unlink(buf);
   ecore_file_unlink(buf2);
}

static void
_e_fm2_device_check_desktop_icons_cb(void *data, Ecore_Thread *eth EINA_UNUSED)
{
   Eina_List *list = data;

   eina_file_dir_list(efreet_desktop_dir_get(), EINA_FALSE, _e_fm2_device_check_desktop_icons_list_cb, data);

   E_FREE_LIST(list, free);
}

E_API void
e_fm2_device_show_desktop_icons(void)
{
   Eina_List *l;
   E_Volume *v;
   char buf[PATH_MAX] = {0};
   char buf2[PATH_MAX] = {0};
   const char *id;

   if (_check_vols)
     {
        _check_run_hide = EINA_FALSE;
        _check_run_show = EINA_TRUE;
        return;
     }

   EINA_LIST_FOREACH(_e_vols, l, v)
     {
        if (!v) continue;
        if (!v->storage) continue;

        id = ecore_file_file_get(v->storage->udi);

        e_user_dir_snprintf(buf, sizeof(buf),
                            "fileman/favorites/|%s_%d.desktop",
                            id, v->partition_number);

        snprintf(buf2, sizeof(buf2), "%s/|%s_%d.desktop",
                 efreet_desktop_dir_get(), id, v->partition_number);

        if (ecore_file_exists(buf) && !ecore_file_exists(buf2))
          {
             ecore_file_symlink(buf, buf2);
             _e_fm2_file_force_update(buf2);
          }
     }
}

E_API void
e_fm2_device_hide_desktop_icons(void)
{
   Eina_List *l;
   E_Volume *v;
   char buf[PATH_MAX] = {0};
   const char *id;

   if (_check_vols)
     {
        _check_run_show = EINA_FALSE;
        _check_run_hide = EINA_TRUE;
        return;
     }

   EINA_LIST_FOREACH(_e_vols, l, v)
     {
        if (!v) continue;
        if (!v->storage) continue;

        id = ecore_file_file_get(v->storage->udi);

        snprintf(buf, sizeof(buf), "%s/|%s_%d.desktop",
                 efreet_desktop_dir_get(), id, v->partition_number);

        if (ecore_file_exists(buf))
          {
             ecore_file_unlink(buf);
             _e_fm2_file_force_update(buf);
          }
     }
}

E_API void
e_fm2_device_check_desktop_icons(void)
{
   Eina_List *l, *thd = NULL;
   E_Volume *v;
   char buf[PATH_MAX] = {0};
   const char *id;

   if (_check_vols) return;
   EINA_LIST_FOREACH(_e_vols, l, v)
     {
        if (!v) continue;
        if (!v->storage) continue;

        id = ecore_file_file_get(v->storage->udi);

        snprintf(buf, sizeof(buf), "%s/|%s_%d.desktop",
                 efreet_desktop_dir_get(), id, v->partition_number);
        thd = eina_list_append(thd, strdup(buf));
     }
   _check_vols = ecore_thread_run(_e_fm2_device_check_desktop_icons_cb, _e_fm2_device_check_desktop_icons_cb_end, _e_fm2_device_check_desktop_icons_cb_end, thd);
}

E_API Eina_List *
e_fm2_device_volume_list_get(void)
{
   return _e_vols;
}

