/*
 * Copyright (C) 2021 Collabora Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef PANVK_CS_H
#define PANVK_CS_H

#include <vulkan/vulkan.h>

#include "compiler/shader_enums.h"
#include "panfrost-job.h"
#include "pan_cs.h"

struct pan_blend_state;
struct pan_shader_info;
struct panfrost_ptr;
struct pan_pool;

union panvk_sysval_data;
struct panvk_framebuffer;
struct panvk_cmd_state;
struct panvk_compute_dim;
struct panvk_device;
struct panvk_batch;
struct panvk_varyings_info;
struct panvk_attrib_buf;
struct panvk_attribs_info;
struct panvk_pipeline;
struct panvk_draw_info;
struct panvk_descriptor_state;
struct panvk_subpass;
struct panvk_clear_value;

void
panvk_emit_varyings(const struct panvk_device *dev,
                    const struct panvk_varyings_info *varyings,
                    gl_shader_stage stage,
                    void *descs);

void
panvk_emit_varying_bufs(const struct panvk_device *dev,
                        const struct panvk_varyings_info *varyings,
                        void *descs);

void
panvk_emit_attrib_bufs(const struct panvk_device *dev,
                       const struct panvk_attribs_info *info,
                       const struct panvk_attrib_buf *bufs,
                       unsigned buf_count,
                       const struct panvk_draw_info *draw,
                       void *descs);

void
panvk_emit_attribs(const struct panvk_device *dev,
                   const struct panvk_attribs_info *attribs,
                   const struct panvk_attrib_buf *bufs,
                   unsigned buf_count,
                   void *descs);

void
panvk_emit_ubos(const struct panvk_pipeline *pipeline,
                const struct panvk_descriptor_state *state,
                void *descs);

void
panvk_emit_vertex_job(const struct panvk_device *dev,
                      const struct panvk_pipeline *pipeline,
                      const struct panvk_draw_info *draw,
                      void *job);

void
panvk_emit_tiler_job(const struct panvk_device *dev,
                     const struct panvk_pipeline *pipeline,
                     const struct panvk_draw_info *draw,
                     void *job);

void
panvk_emit_fragment_job(const struct panvk_device *dev,
                        const struct panvk_framebuffer *fb,
                        mali_ptr fbdesc,
                        void *job);

void
panvk_emit_viewport(const VkViewport *viewport, const VkRect2D *scissor,
                    void *vpd);

void
panvk_emit_blend(const struct panvk_device *dev,
                 const struct panvk_pipeline *pipeline,
                 unsigned rt, void *bd);

void
panvk_emit_blend_constant(const struct panvk_device *dev,
                          const struct panvk_pipeline *pipeline,
                          unsigned rt, const float *constants, void *bd);

void
panvk_emit_dyn_fs_rsd(const struct panvk_device *dev,
                      const struct panvk_pipeline *pipeline,
                      const struct panvk_cmd_state *state,
                      void *rsd);

void
panvk_emit_base_fs_rsd(const struct panvk_device *dev,
                       const struct panvk_pipeline *pipeline,
                       void *rsd);

void
panvk_emit_non_fs_rsd(const struct panvk_device *dev,
                      const struct pan_shader_info *shader_info,
                      mali_ptr shader_ptr,
                      void *rsd);

void
panvk_emit_bifrost_tiler_context(const struct panvk_device *dev,
                                 unsigned width, unsigned height,
                                 const struct panfrost_ptr *descs);

unsigned
panvk_emit_fb(const struct panvk_device *dev,
              const struct panvk_batch *batch,
              const struct panvk_subpass *subpass,
              const struct panvk_pipeline *pipeline,
              const struct panvk_framebuffer *fb,
              const struct panvk_clear_value *clears,
              const struct pan_tls_info *tlsinfo,
              const struct pan_tiler_context *tilerctx,
              void *desc);

void
panvk_emit_tls(const struct panvk_device *dev,
               const struct panvk_pipeline *pipeline,
               const struct pan_compute_dim *wg_count,
               struct pan_pool *tls_pool,
               void *desc);

void
panvk_sysval_upload_viewport_scale(const VkViewport *viewport,
                                   union panvk_sysval_data *data);

void
panvk_sysval_upload_viewport_offset(const VkViewport *viewport,
                                    union panvk_sysval_data *data);

#endif
