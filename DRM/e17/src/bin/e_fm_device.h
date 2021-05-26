#ifndef E_FM_DEVICE_H
#define E_FM_DEVICE_H

#include "e.h"
#include "e_fm.h"

E_API void         e_fm2_device_storage_add(E_Storage *s);
E_API void         e_fm2_device_storage_del(E_Storage *s);
E_API E_Storage   *e_fm2_device_storage_find(const char *udi);

E_API void         e_fm2_device_volume_add(E_Volume *s);
E_API void         e_fm2_device_volume_del(E_Volume *s);
E_API E_Volume    *e_fm2_device_volume_find(const char *udi);
E_API E_Volume    *e_fm2_device_volume_find_fast(const char *udi);
E_API const char  *e_fm2_device_volume_mountpoint_get(E_Volume *v);

E_API void         e_fm2_device_mount_add(E_Volume *v, const char *mountpoint);
E_API E_Fm2_Device_Mount_Op *e_fm2_device_mount_op_add(E_Fm2_Mount *m, char *args, size_t size, size_t length);
E_API void e_fm2_device_mount_free(E_Fm2_Mount *m) EINA_ARG_NONNULL(1);
E_API void         e_fm2_device_mount_del(E_Volume *v);
E_API E_Fm2_Mount *e_fm2_device_mount_find(const char *path);
E_API E_Fm2_Mount *e_fm2_device_mount(E_Volume *v,
                                  Ecore_Cb mount_ok, Ecore_Cb mount_fail,
                                  Ecore_Cb unmount_ok, Ecore_Cb unmount_fail,
                                  void *data);
E_API void         e_fm2_device_mount_fail(E_Volume *v);
E_API void         e_fm2_device_unmount(E_Fm2_Mount *m);
E_API void         e_fm2_device_unmount_fail(E_Volume *v);

E_API void         e_fm2_device_show_desktop_icons(void);
E_API void         e_fm2_device_hide_desktop_icons(void);
E_API void         e_fm2_device_check_desktop_icons(void);
E_API Eina_List   *e_fm2_device_volume_list_get(void);

#endif
