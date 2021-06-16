/*
 * Copyright 2018 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ZINK_RESOURCE_H
#define ZINK_RESOURCE_H

struct pipe_screen;
struct sw_displaytarget;
struct zink_batch;
struct zink_context;

#define ZINK_RESOURCE_USAGE_STREAMOUT (1 << 10) //much greater than ZINK_DESCRIPTOR_TYPES

#include "util/simple_mtx.h"
#include "util/u_transfer.h"
#include "util/u_range.h"
#include "util/u_dynarray.h"
#include "util/u_threaded_context.h"

#include "zink_batch.h"
#include "zink_descriptors.h"

#include <vulkan/vulkan.h>

enum zink_resource_access {
   ZINK_RESOURCE_ACCESS_READ = 1,
   ZINK_RESOURCE_ACCESS_WRITE = 32,
   ZINK_RESOURCE_ACCESS_RW = ZINK_RESOURCE_ACCESS_READ | ZINK_RESOURCE_ACCESS_WRITE,
};

struct mem_key {
   VkMemoryRequirements reqs;
   VkMemoryPropertyFlags flags;
};

struct zink_resource_object {
   struct pipe_reference reference;
   union {
      VkBuffer buffer;
      VkImage image;
   };

   VkBuffer sbuffer;
   bool storage_init; //layout was set for image
   bool transfer_dst;

   VkDeviceMemory mem;
   uint32_t mem_hash;
   struct mem_key mkey;
   VkDeviceSize offset, size;

   VkSampleLocationsInfoEXT zs_evaluate;
   bool needs_zs_evaluate;

   unsigned persistent_maps; //if nonzero, requires vkFlushMappedMemoryRanges during batch use
   struct zink_descriptor_refs desc_set_refs;

   struct zink_batch_usage reads;
   struct zink_batch_usage writes;
   void *map;
   bool is_buffer;
   bool host_visible;
   bool coherent;
};

struct zink_resource {
   struct threaded_resource base;

   enum pipe_format internal_format:16;

   VkPipelineStageFlagBits access_stage;
   VkAccessFlags access;
   bool unordered_barrier;

   struct zink_resource_object *obj;
   struct zink_resource_object *scanout_obj; //TODO: remove for wsi
   bool scanout_obj_init;
   bool scanout_dirty;
   union {
      struct {
         struct util_range valid_buffer_range;
         uint16_t vbo_bind_count;
         uint16_t ubo_bind_count[2];
      };
      struct {
         VkFormat format;
         VkImageLayout layout;
         VkImageAspectFlags aspect;
         bool optimal_tiling;
         uint32_t sampler_binds[PIPE_SHADER_TYPES];
         uint8_t fb_binds;
         uint16_t image_bind_count[2]; //gfx, compute
      };
   };
   uint16_t write_bind_count[2]; //gfx, compute
   uint16_t bind_count[2]; //gfx, compute

   struct sw_displaytarget *dt;
   unsigned dt_stride;

   uint32_t bind_history; // enum zink_descriptor_type bitmask
   uint32_t bind_stages;
};

struct zink_transfer {
   struct threaded_transfer base;
   struct pipe_resource *staging_res;
   unsigned offset;
   unsigned depthPitch;
};

static inline struct zink_resource *
zink_resource(struct pipe_resource *r)
{
   return (struct zink_resource *)r;
}

bool
zink_screen_resource_init(struct pipe_screen *pscreen);

void
zink_context_resource_init(struct pipe_context *pctx);

void
zink_get_depth_stencil_resources(struct pipe_resource *res,
                                 struct zink_resource **out_z,
                                 struct zink_resource **out_s);

void
zink_resource_setup_transfer_layouts(struct zink_context *ctx, struct zink_resource *src, struct zink_resource *dst);

bool
zink_resource_has_usage(struct zink_resource *res, enum zink_resource_access usage);

bool
zink_resource_has_curr_read_usage(struct zink_context *ctx, struct zink_resource *res);

void
zink_destroy_resource_object(struct zink_screen *screen, struct zink_resource_object *resource_object);

void
debug_describe_zink_resource_object(char *buf, const struct zink_resource_object *ptr);

static inline void
zink_resource_object_reference(struct zink_screen *screen,
                             struct zink_resource_object **dst,
                             struct zink_resource_object *src)
{
   struct zink_resource_object *old_dst = dst ? *dst : NULL;

   if (pipe_reference_described(old_dst ? &old_dst->reference : NULL, &src->reference,
                                (debug_reference_descriptor)debug_describe_zink_resource_object))
      zink_destroy_resource_object(screen, old_dst);
   if (dst) *dst = src;
}

bool
zink_resource_object_init_storage(struct zink_context *ctx, struct zink_resource *res);
#endif
