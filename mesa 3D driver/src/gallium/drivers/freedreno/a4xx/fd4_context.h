/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD4_CONTEXT_H_
#define FD4_CONTEXT_H_

#include "util/u_upload_mgr.h"

#include "freedreno_context.h"

#include "ir3/ir3_shader.h"

struct fd4_context {
   struct fd_context base;

   struct fd_bo *vs_pvt_mem, *fs_pvt_mem;

   /* This only needs to be 4 * num_of_pipes bytes (ie. 32 bytes).  We
    * could combine it with another allocation.
    *
    * (upper area used as scratch bo.. see fd4_query)
    */
   struct fd_bo *vsc_size_mem;

   struct u_upload_mgr *border_color_uploader;
   struct pipe_resource *border_color_buf;

   /* bitmask of samplers which need astc srgb workaround: */
   uint16_t vastc_srgb, fastc_srgb;

   /* samplers swizzles, needed for tg4 workaround: */
   uint16_t vsampler_swizzles[16], fsampler_swizzles[16];

   /* storage for ctx->last.key: */
   struct ir3_shader_key last_key;
};

static inline struct fd4_context *
fd4_context(struct fd_context *ctx)
{
   return (struct fd4_context *)ctx;
}

struct pipe_context *fd4_context_create(struct pipe_screen *pscreen, void *priv,
                                        unsigned flags);

#endif /* FD4_CONTEXT_H_ */
