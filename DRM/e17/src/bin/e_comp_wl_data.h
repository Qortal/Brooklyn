/*
 * Copyright © 2011 Kristian Høgsberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef E_TYPEDEFS
#else
# ifndef E_COMP_WL_DATA_H
#  define E_COMP_WL_DATA_H

#  include "e_comp_wl.h"

#  define CLIPBOARD_CHUNK 1024

typedef struct _E_Comp_Wl_Data_Source E_Comp_Wl_Data_Source;
typedef struct _E_Comp_Wl_Data_Offer E_Comp_Wl_Data_Offer;
typedef struct _E_Comp_Wl_Clipboard_Source E_Comp_Wl_Clipboard_Source;
typedef struct _E_Comp_Wl_Clipboard_Offer E_Comp_Wl_Clipboard_Offer;

struct _E_Comp_Wl_Data_Source
{
   struct wl_resource *resource; //resource of wl_data_source

   Eina_Array *mime_types; //mime_type list to offer from source
   struct wl_signal destroy_signal; //signal to emit when wl_data_source resource is destroyed

   void (*target) (E_Comp_Wl_Data_Source *source, uint32_t serial, const char* mime_type);
   void (*send) (E_Comp_Wl_Data_Source *source, const char* mime_type, int32_t fd);
   void (*cancelled) (E_Comp_Wl_Data_Source *source);
   E_Comp_Wl_Data_Offer *offer;
   uint32_t dnd_actions;
   enum wl_data_device_manager_dnd_action current_dnd_action;
   enum wl_data_device_manager_dnd_action compositor_action;
   uint32_t serial;

   Eina_Bool accepted E_BITFIELD;
   Eina_Bool actions_set E_BITFIELD;
};

struct _E_Comp_Wl_Data_Offer
{
   struct wl_resource *resource; //resource of wl_data_offer

   E_Comp_Wl_Data_Source *source; //indicates source client data
   struct wl_listener source_destroy_listener; //listener for destroy of source
   uint32_t dnd_actions;
   enum wl_data_device_manager_dnd_action preferred_dnd_action;
   Eina_Bool in_ask E_BITFIELD;
};

struct _E_Comp_Wl_Clipboard_Source
{
   E_Comp_Wl_Data_Source data_source;
   Ecore_Fd_Handler *fd_handler;
   uint32_t serial;

   struct wl_array contents; //for extendable buffer
   int ref;
   int fd;
};

struct _E_Comp_Wl_Clipboard_Offer
{
   E_Comp_Wl_Clipboard_Source *source;
   Ecore_Fd_Handler *fd_handler;
   size_t offset;
};

E_API void e_comp_wl_data_device_send_enter(E_Client *ec);
E_API void e_comp_wl_data_device_send_leave(E_Client *ec);
EINTERN void *e_comp_wl_data_device_send_offer(E_Client *ec);
E_API void e_comp_wl_data_device_keyboard_focus_set(void);
EINTERN Eina_Bool e_comp_wl_data_manager_init(void);
EINTERN void e_comp_wl_data_manager_shutdown(void);
E_API struct wl_resource *e_comp_wl_data_find_for_client(struct wl_client *client);
E_API E_Comp_Wl_Data_Source *e_comp_wl_data_manager_source_create(struct wl_client *client, struct wl_resource *resource, uint32_t id);
E_API void e_comp_wl_clipboard_source_unref(E_Comp_Wl_Clipboard_Source *source);
E_API E_Comp_Wl_Clipboard_Source *e_comp_wl_clipboard_source_create(const char *mime_type, uint32_t serial, int fd);
# endif
#endif
