/*
 * Copyright (C) 2019-2020 Collabora, Ltd.
 * © Copyright 2018 Alyssa Rosenzweig
 * Copyright © 2014-2017 Broadcom
 * Copyright (C) 2017 Intel Corporation
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
 */

#include <sys/poll.h>
#include <errno.h>

#include "pan_bo.h"
#include "pan_context.h"
#include "pan_minmax_cache.h"
#include "panfrost-quirks.h"

#include "util/macros.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_upload_mgr.h"
#include "util/u_memory.h"
#include "util/u_vbuf.h"
#include "util/half_float.h"
#include "util/u_helpers.h"
#include "util/format/u_format.h"
#include "util/u_prim.h"
#include "util/u_prim_restart.h"
#include "indices/u_primconvert.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_from_mesa.h"
#include "util/u_math.h"

#include "midgard_pack.h"
#include "pan_screen.h"
#include "pan_cmdstream.h"
#include "pan_indirect_draw.h"
#include "pan_util.h"
#include "decode.h"
#include "util/pan_lower_framebuffer.h"

static void
panfrost_clear(
        struct pipe_context *pipe,
        unsigned buffers,
        const struct pipe_scissor_state *scissor_state,
        const union pipe_color_union *color,
        double depth, unsigned stencil)
{
        struct panfrost_context *ctx = pan_context(pipe);

        if (!panfrost_render_condition_check(ctx))
                return;

        /* TODO: panfrost_get_fresh_batch_for_fbo() instantiates a new batch if
         * the existing batch targeting this FBO has draws. We could probably
         * avoid that by replacing plain clears by quad-draws with a specific
         * color/depth/stencil value, thus avoiding the generation of extra
         * fragment jobs.
         */
        struct panfrost_batch *batch = panfrost_get_fresh_batch_for_fbo(ctx);
        panfrost_batch_clear(batch, buffers, color, depth, stencil);
}

bool
panfrost_writes_point_size(struct panfrost_context *ctx)
{
        assert(ctx->shader[PIPE_SHADER_VERTEX]);
        struct panfrost_shader_state *vs = panfrost_get_shader_state(ctx, PIPE_SHADER_VERTEX);

        return vs->info.vs.writes_point_size && ctx->active_prim == PIPE_PRIM_POINTS;
}

/* The entire frame is in memory -- send it off to the kernel! */

void
panfrost_flush(
        struct pipe_context *pipe,
        struct pipe_fence_handle **fence,
        unsigned flags)
{
        struct panfrost_context *ctx = pan_context(pipe);
        struct panfrost_device *dev = pan_device(pipe->screen);


        /* Submit all pending jobs */
        panfrost_flush_all_batches(ctx);

        if (fence) {
                struct panfrost_fence *f = panfrost_fence_create(ctx);
                pipe->screen->fence_reference(pipe->screen, fence, NULL);
                *fence = (struct pipe_fence_handle *)f;
        }

        if (dev->debug & PAN_DBG_TRACE)
                pandecode_next_frame();
}

static void
panfrost_texture_barrier(struct pipe_context *pipe, unsigned flags)
{
        struct panfrost_context *ctx = pan_context(pipe);
        panfrost_flush_all_batches(ctx);
}

static void
panfrost_set_frontend_noop(struct pipe_context *pipe, bool enable)
{
        struct panfrost_context *ctx = pan_context(pipe);
        panfrost_flush_all_batches(ctx);
        ctx->is_noop = enable;
}

#define DEFINE_CASE(c) case PIPE_PRIM_##c: return MALI_DRAW_MODE_##c;

static int
pan_draw_mode(enum pipe_prim_type mode)
{
        switch (mode) {
                DEFINE_CASE(POINTS);
                DEFINE_CASE(LINES);
                DEFINE_CASE(LINE_LOOP);
                DEFINE_CASE(LINE_STRIP);
                DEFINE_CASE(TRIANGLES);
                DEFINE_CASE(TRIANGLE_STRIP);
                DEFINE_CASE(TRIANGLE_FAN);
                DEFINE_CASE(QUADS);
                DEFINE_CASE(QUAD_STRIP);
                DEFINE_CASE(POLYGON);

        default:
                unreachable("Invalid draw mode");
        }
}

#undef DEFINE_CASE

static bool
panfrost_scissor_culls_everything(struct panfrost_context *ctx)
{
        const struct pipe_scissor_state *ss = &ctx->scissor;

        /* Check if we're scissoring at all */

        if (!ctx->rasterizer->base.scissor)
                return false;

        return (ss->minx == ss->maxx) || (ss->miny == ss->maxy);
}

/* Count generated primitives (when there is no geom/tess shaders) for
 * transform feedback */

static void
panfrost_statistics_record(
                struct panfrost_context *ctx,
                const struct pipe_draw_info *info,
                const struct pipe_draw_start_count_bias *draw)
{
        if (!ctx->active_queries)
                return;

        uint32_t prims = u_prims_for_vertices(info->mode, draw->count);
        ctx->prims_generated += prims;

        if (!ctx->streamout.num_targets)
                return;

        ctx->tf_prims_generated += prims;
}

static void
panfrost_update_streamout_offsets(struct panfrost_context *ctx)
{
        for (unsigned i = 0; i < ctx->streamout.num_targets; ++i) {
                unsigned count;

                count = u_stream_outputs_for_vertices(ctx->active_prim,
                                                      ctx->vertex_count);
                pan_so_target(ctx->streamout.targets[i])->offset += count;
        }
}

static inline void
pan_emit_draw_descs(struct panfrost_batch *batch,
                struct MALI_DRAW *d, enum pipe_shader_type st)
{
        d->offset_start = batch->ctx->offset_start;
        d->instance_size = batch->ctx->instance_count > 1 ?
                           batch->ctx->padded_count : 1;

        d->uniform_buffers = panfrost_emit_const_buf(batch, st, &d->push_uniforms);
        d->textures = panfrost_emit_texture_descriptors(batch, st);
        d->samplers = panfrost_emit_sampler_descriptors(batch, st);
}

static enum mali_index_type
panfrost_translate_index_size(unsigned size)
{
        switch (size) {
        case 1: return MALI_INDEX_TYPE_UINT8;
        case 2: return MALI_INDEX_TYPE_UINT16;
        case 4: return MALI_INDEX_TYPE_UINT32;
        default: unreachable("Invalid index size");
        }
}

static void
panfrost_draw_emit_vertex(struct panfrost_batch *batch,
                          const struct pipe_draw_info *info,
                          void *invocation_template,
                          mali_ptr shared_mem, mali_ptr vs_vary,
                          mali_ptr varyings,
                          mali_ptr attribs, mali_ptr attrib_bufs,
                          void *job)
{
        struct panfrost_context *ctx = batch->ctx;
        struct panfrost_device *device = pan_device(ctx->base.screen);

        void *section =
                pan_section_ptr(job, COMPUTE_JOB, INVOCATION);
        memcpy(section, invocation_template, MALI_INVOCATION_LENGTH);

        pan_section_pack(job, COMPUTE_JOB, PARAMETERS, cfg) {
                cfg.job_task_split = 5;
        }

        pan_section_pack(job, COMPUTE_JOB, DRAW, cfg) {
                cfg.draw_descriptor_is_64b = true;
                if (!pan_is_bifrost(device))
                        cfg.texture_descriptor_is_64b = true;
                cfg.state = panfrost_emit_compute_shader_meta(batch, PIPE_SHADER_VERTEX);
                cfg.attributes = attribs;
                cfg.attribute_buffers = attrib_bufs;
                cfg.varyings = vs_vary;
                cfg.varying_buffers = vs_vary ? varyings : 0;
                cfg.thread_storage = shared_mem;
                pan_emit_draw_descs(batch, &cfg, PIPE_SHADER_VERTEX);
        }

        pan_section_pack(job, COMPUTE_JOB, DRAW_PADDING, cfg);
}

static void
panfrost_emit_primitive_size(struct panfrost_context *ctx,
                             bool points, mali_ptr size_array,
                             void *prim_size)
{
        struct panfrost_rasterizer *rast = ctx->rasterizer;

        pan_pack(prim_size, PRIMITIVE_SIZE, cfg) {
                if (panfrost_writes_point_size(ctx)) {
                        cfg.size_array = size_array;
                } else {
                        cfg.constant = points ?
                                       rast->base.point_size :
                                       rast->base.line_width;
                }
        }
}

static bool
panfrost_is_implicit_prim_restart(const struct pipe_draw_info *info)
{
        unsigned implicit_index = (1 << (info->index_size * 8)) - 1;
        bool implicit = info->restart_index == implicit_index;
        return info->primitive_restart && implicit;
}

static void
panfrost_draw_emit_tiler(struct panfrost_batch *batch,
                         const struct pipe_draw_info *info,
                         const struct pipe_draw_start_count_bias *draw,
                         void *invocation_template,
                         mali_ptr shared_mem, mali_ptr indices,
                         mali_ptr fs_vary, mali_ptr varyings,
                         mali_ptr pos, mali_ptr psiz, void *job)
{
        struct panfrost_context *ctx = batch->ctx;
        struct pipe_rasterizer_state *rast = &ctx->rasterizer->base;
        struct panfrost_device *device = pan_device(ctx->base.screen);

        void *section = pan_is_bifrost(device) ?
                        pan_section_ptr(job, BIFROST_TILER_JOB, INVOCATION) :
                        pan_section_ptr(job, MIDGARD_TILER_JOB, INVOCATION);
        memcpy(section, invocation_template, MALI_INVOCATION_LENGTH);

        section = pan_is_bifrost(device) ?
                  pan_section_ptr(job, BIFROST_TILER_JOB, PRIMITIVE) :
                  pan_section_ptr(job, MIDGARD_TILER_JOB, PRIMITIVE);
        pan_pack(section, PRIMITIVE, cfg) {
                cfg.draw_mode = pan_draw_mode(info->mode);
                if (panfrost_writes_point_size(ctx))
                        cfg.point_size_array_format = MALI_POINT_SIZE_ARRAY_FORMAT_FP16;

                /* For line primitives, PRIMITIVE.first_provoking_vertex must
                 * be set to true and the provoking vertex is selected with
                 * DRAW.flat_shading_vertex.
                 */
                if (info->mode == PIPE_PRIM_LINES ||
                    info->mode == PIPE_PRIM_LINE_LOOP ||
                    info->mode == PIPE_PRIM_LINE_STRIP)
                        cfg.first_provoking_vertex = true;
                else
                        cfg.first_provoking_vertex = rast->flatshade_first;

                if (panfrost_is_implicit_prim_restart(info)) {
                        cfg.primitive_restart = MALI_PRIMITIVE_RESTART_IMPLICIT;
                } else if (info->primitive_restart) {
                        cfg.primitive_restart = MALI_PRIMITIVE_RESTART_EXPLICIT;
                        cfg.primitive_restart_index = info->restart_index;
                }

                cfg.job_task_split = 6;

                cfg.index_count = ctx->indirect_draw ? 1 : draw->count;
                if (info->index_size) {
                        cfg.index_type = panfrost_translate_index_size(info->index_size);
                        cfg.indices = indices;
                        cfg.base_vertex_offset = draw->index_bias - ctx->offset_start;
                }
        }

        bool points = info->mode == PIPE_PRIM_POINTS;
        void *prim_size = pan_is_bifrost(device) ?
                          pan_section_ptr(job, BIFROST_TILER_JOB, PRIMITIVE_SIZE) :
                          pan_section_ptr(job, MIDGARD_TILER_JOB, PRIMITIVE_SIZE);

        if (pan_is_bifrost(device)) {
                panfrost_emit_primitive_size(ctx, points, psiz, prim_size);
                pan_section_pack(job, BIFROST_TILER_JOB, TILER, cfg) {
                        cfg.address = panfrost_batch_get_bifrost_tiler(batch, ~0);
                }
                pan_section_pack(job, BIFROST_TILER_JOB, PADDING, padding) {}
        }

        section = pan_is_bifrost(device) ?
                  pan_section_ptr(job, BIFROST_TILER_JOB, DRAW) :
                  pan_section_ptr(job, MIDGARD_TILER_JOB, DRAW);
        pan_pack(section, DRAW, cfg) {
                cfg.four_components_per_vertex = true;
                cfg.draw_descriptor_is_64b = true;
                if (!pan_is_bifrost(device))
                        cfg.texture_descriptor_is_64b = true;
                cfg.front_face_ccw = rast->front_ccw;
                cfg.cull_front_face = rast->cull_face & PIPE_FACE_FRONT;
                cfg.cull_back_face = rast->cull_face & PIPE_FACE_BACK;
                cfg.position = pos;
                cfg.state = panfrost_emit_frag_shader_meta(batch);
                cfg.attributes = panfrost_emit_image_attribs(batch, &cfg.attribute_buffers, PIPE_SHADER_FRAGMENT);
                cfg.viewport = panfrost_emit_viewport(batch);
                cfg.varyings = fs_vary;
                cfg.varying_buffers = fs_vary ? varyings : 0;
                cfg.thread_storage = shared_mem;

                /* For all primitives but lines DRAW.flat_shading_vertex must
                 * be set to 0 and the provoking vertex is selected with the
                 * PRIMITIVE.first_provoking_vertex field.
                 */
                if (info->mode == PIPE_PRIM_LINES ||
                    info->mode == PIPE_PRIM_LINE_LOOP ||
                    info->mode == PIPE_PRIM_LINE_STRIP) {
                        /* The logic is inverted on bifrost. */
                        cfg.flat_shading_vertex =
                                pan_is_bifrost(device) ?
                                rast->flatshade_first : !rast->flatshade_first;
                }

                pan_emit_draw_descs(batch, &cfg, PIPE_SHADER_FRAGMENT);

                if (ctx->occlusion_query && ctx->active_queries) {
                        if (ctx->occlusion_query->type == PIPE_QUERY_OCCLUSION_COUNTER)
                                cfg.occlusion_query = MALI_OCCLUSION_MODE_COUNTER;
                        else
                                cfg.occlusion_query = MALI_OCCLUSION_MODE_PREDICATE;
                        cfg.occlusion = ctx->occlusion_query->bo->ptr.gpu;
                        panfrost_batch_add_bo(ctx->batch, ctx->occlusion_query->bo,
                                              PAN_BO_ACCESS_SHARED |
                                              PAN_BO_ACCESS_RW |
                                              PAN_BO_ACCESS_FRAGMENT);
                }
        }

        if (!pan_is_bifrost(device))
                panfrost_emit_primitive_size(ctx, points, psiz, prim_size);
        else
                pan_section_pack(job, BIFROST_TILER_JOB, DRAW_PADDING, cfg);
}

static void
panfrost_direct_draw(struct panfrost_context *ctx,
                     const struct pipe_draw_info *info,
                     unsigned drawid_offset,
                     const struct pipe_draw_start_count_bias *draw)
{
        if (!draw->count || !info->instance_count)
                return;

        struct panfrost_device *device = pan_device(ctx->base.screen);

        if (!panfrost_render_condition_check(ctx))
                return;

        int mode = info->mode;

        /* Fallback for unsupported modes */

        assert(ctx->rasterizer != NULL);

        if (!(ctx->draw_modes & (1 << mode))) {
                if (draw->count < 4) {
                        /* Degenerate case? */
                        return;
                }

                util_primconvert_save_rasterizer_state(ctx->primconvert, &ctx->rasterizer->base);
                util_primconvert_draw_vbo(ctx->primconvert, info, drawid_offset, NULL, draw, 1);
                return;
        }

        /* Now that we have a guaranteed terminating path, find the job. */

        struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);

        /* Don't add too many jobs to a single batch */
        if (batch->scoreboard.job_index > 10000)
                batch = panfrost_get_fresh_batch_for_fbo(ctx);

        panfrost_batch_set_requirements(batch);

        /* Take into account a negative bias */
        ctx->indirect_draw = false;
        ctx->vertex_count = draw->count + (info->index_size ? abs(draw->index_bias) : 0);
        ctx->instance_count = info->instance_count;
        ctx->active_prim = info->mode;

        struct panfrost_ptr tiler =
                pan_is_bifrost(device) ?
                panfrost_pool_alloc_desc(&batch->pool, BIFROST_TILER_JOB) :
                panfrost_pool_alloc_desc(&batch->pool, MIDGARD_TILER_JOB);
        struct panfrost_ptr vertex =
                panfrost_pool_alloc_desc(&batch->pool, COMPUTE_JOB);

        unsigned vertex_count = ctx->vertex_count;

        mali_ptr shared_mem = panfrost_batch_reserve_tls(batch, false);

        unsigned min_index = 0, max_index = 0;
        mali_ptr indices = 0;

        if (info->index_size) {
                indices = panfrost_get_index_buffer_bounded(ctx, info, draw,
                                                            &min_index,
                                                            &max_index);

                /* Use the corresponding values */
                vertex_count = max_index - min_index + 1;
                ctx->offset_start = min_index + draw->index_bias;
        } else {
                ctx->offset_start = draw->start;
        }

        /* Encode the padded vertex count */

        if (info->instance_count > 1)
                ctx->padded_count = panfrost_padded_vertex_count(vertex_count);
        else
                ctx->padded_count = vertex_count;

        panfrost_statistics_record(ctx, info, draw);

        struct mali_invocation_packed invocation;
        if (info->instance_count > 1) {
                panfrost_pack_work_groups_compute(&invocation,
                                                  1, vertex_count, info->instance_count,
                                                  1, 1, 1, true);
        } else {
                pan_pack(&invocation, INVOCATION, cfg) {
                        cfg.invocations = MALI_POSITIVE(vertex_count);
                        cfg.size_y_shift = 0;
                        cfg.size_z_shift = 0;
                        cfg.workgroups_x_shift = 0;
                        cfg.workgroups_y_shift = 0;
                        cfg.workgroups_z_shift = 32;
                        cfg.unknown_shift = 2;
                }
        }

        /* Emit all sort of descriptors. */
        mali_ptr varyings = 0, vs_vary = 0, fs_vary = 0, pos = 0, psiz = 0;

        bool point_coord_replace = (info->mode == PIPE_PRIM_POINTS);

        panfrost_emit_varying_descriptor(batch,
                                         ctx->padded_count *
                                         ctx->instance_count,
                                         &vs_vary, &fs_vary, &varyings,
                                         NULL, &pos, &psiz,
                                         point_coord_replace);

        mali_ptr attribs, attrib_bufs;
        attribs = panfrost_emit_vertex_data(batch, &attrib_bufs);

        /* Fire off the draw itself */
        panfrost_draw_emit_vertex(batch, info, &invocation, shared_mem,
                                  vs_vary, varyings, attribs, attrib_bufs, vertex.cpu);
        panfrost_draw_emit_tiler(batch, info, draw, &invocation, shared_mem, indices,
                                 fs_vary, varyings, pos, psiz, tiler.cpu);
        panfrost_emit_vertex_tiler_jobs(batch, &vertex, &tiler);

        /* Adjust the batch stack size based on the new shader stack sizes. */
        panfrost_batch_adjust_stack_size(batch);

        /* Increment transform feedback offsets */
        panfrost_update_streamout_offsets(ctx);
}

static void
panfrost_indirect_draw(struct panfrost_context *ctx,
                       const struct pipe_draw_info *info,
                       unsigned drawid_offset,
                       const struct pipe_draw_indirect_info *indirect,
                       const struct pipe_draw_start_count_bias *draw)
{
        if (indirect->count_from_stream_output) {
                struct pipe_draw_start_count_bias tmp_draw = *draw;
                struct panfrost_streamout_target *so =
                        pan_so_target(indirect->count_from_stream_output);

                tmp_draw.start = 0;
                tmp_draw.count = so->offset;
                tmp_draw.index_bias = 0;
                panfrost_direct_draw(ctx, info, drawid_offset, &tmp_draw);
                return;
        }

        /* Indirect draw count and multi-draw not supported. */
        assert(indirect->draw_count == 1 && !indirect->indirect_draw_count);

        /* TODO: update statistics (see panfrost_statistics_record()) */
        /* TODO: Increment transform feedback offsets */
        assert(ctx->streamout.num_targets == 0);

        struct panfrost_device *dev = pan_device(ctx->base.screen);

        assert(ctx->draw_modes & (1 << info->mode));
        ctx->active_prim = info->mode;
        ctx->indirect_draw = true;

        struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);

        /* Don't add too many jobs to a single batch */
        if (batch->scoreboard.job_index + (indirect->draw_count * 3) > 10000)
                batch = panfrost_get_fresh_batch_for_fbo(ctx);

        panfrost_batch_set_requirements(batch);

        mali_ptr shared_mem = panfrost_batch_reserve_tls(batch, false);

        struct panfrost_ptr tiler =
                panfrost_pool_alloc_aligned(&batch->pool,
                                            pan_is_bifrost(dev) ?
                                            MALI_BIFROST_TILER_JOB_LENGTH :
                                            MALI_MIDGARD_TILER_JOB_LENGTH,
                                            64);
        struct panfrost_ptr vertex =
                panfrost_pool_alloc_aligned(&batch->pool,
                                            MALI_COMPUTE_JOB_LENGTH,
                                            64);

        struct panfrost_shader_state *vs =
                panfrost_get_shader_state(ctx, PIPE_SHADER_VERTEX);

        struct panfrost_bo *index_buf = NULL;

        if (info->index_size) {
                assert(!info->has_user_indices);
                index_buf = pan_resource(info->index.resource)->image.data.bo;
                panfrost_batch_add_bo(batch,
                                      index_buf,
                                      PAN_BO_ACCESS_SHARED | PAN_BO_ACCESS_READ |
                                      PAN_BO_ACCESS_VERTEX_TILER);
        }

        mali_ptr varyings = 0, vs_vary = 0, fs_vary = 0, pos = 0, psiz = 0;
        unsigned varying_buf_count;

        /* We want to create templates, set all count fields to 0 to reflect
         * that.
         */
        ctx->instance_count = ctx->vertex_count = ctx->padded_count = 0;
        ctx->offset_start = 0;

        bool point_coord_replace = (info->mode == PIPE_PRIM_POINTS);

        panfrost_emit_varying_descriptor(batch, 0,
                                         &vs_vary, &fs_vary, &varyings,
                                         &varying_buf_count, &pos, &psiz,
                                         point_coord_replace);

        mali_ptr attribs, attrib_bufs;
        attribs = panfrost_emit_vertex_data(batch, &attrib_bufs);

        /* Zero-ed invocation, the compute job will update it. */
        static struct mali_invocation_packed invocation;

        /* Fire off the draw itself */
        panfrost_draw_emit_vertex(batch, info, &invocation, shared_mem,
                                  vs_vary, varyings, attribs, attrib_bufs,
                                  vertex.cpu);
        panfrost_draw_emit_tiler(batch, info, draw, &invocation, shared_mem,
                                 index_buf ? index_buf->ptr.gpu : 0,
                                 fs_vary, varyings, pos, psiz, tiler.cpu);

        /* Add the varying heap BO to the batch if we're allocating varyings. */
        if (varyings) {
                panfrost_batch_add_bo(batch,
                                      dev->indirect_draw_shaders.varying_heap,
                                      PAN_BO_ACCESS_SHARED | PAN_BO_ACCESS_RW |
                                      PAN_BO_ACCESS_VERTEX_TILER);
        }

        assert(indirect->buffer);

        struct panfrost_resource *draw_buf = pan_resource(indirect->buffer);

        /* Don't count images: those attributes don't need to be patched. */
        unsigned attrib_count =
                vs->info.attribute_count -
                util_bitcount(ctx->image_mask[PIPE_SHADER_VERTEX]);

        panfrost_batch_add_bo(batch, draw_buf->image.data.bo,
                              PAN_BO_ACCESS_SHARED | PAN_BO_ACCESS_READ |
                              PAN_BO_ACCESS_VERTEX_TILER);

        struct pan_indirect_draw_info draw_info = {
                .last_indirect_draw = batch->indirect_draw_job_id,
                .draw_buf = draw_buf->image.data.bo->ptr.gpu + indirect->offset,
                .index_buf = index_buf ? index_buf->ptr.gpu : 0,
                .vertex_job = vertex.gpu,
                .tiler_job = tiler.gpu,
                .attrib_bufs = attrib_bufs,
                .attribs = attribs,
                .attrib_count = attrib_count,
                .varying_bufs = varyings,
                .index_size = info->index_size,
        };

        if (panfrost_writes_point_size(ctx))
                draw_info.flags |= PAN_INDIRECT_DRAW_UPDATE_PRIM_SIZE;

        if (vs->info.vs.writes_point_size)
                draw_info.flags |= PAN_INDIRECT_DRAW_HAS_PSIZ;


        if (info->primitive_restart) {
                draw_info.restart_index = info->restart_index;
                draw_info.flags |= PAN_INDIRECT_DRAW_PRIMITIVE_RESTART;
        }

        batch->indirect_draw_job_id =
                panfrost_emit_indirect_draw(&batch->pool,
                                            &batch->scoreboard,
                                            &draw_info,
                                            &batch->indirect_draw_ctx);

        panfrost_emit_vertex_tiler_jobs(batch, &vertex, &tiler);

        /* Adjust the batch stack size based on the new shader stack sizes. */
        panfrost_batch_adjust_stack_size(batch);
}

static void
panfrost_draw_vbo(struct pipe_context *pipe,
                  const struct pipe_draw_info *info,
                  unsigned drawid_offset,
                  const struct pipe_draw_indirect_info *indirect,
                  const struct pipe_draw_start_count_bias *draws,
                  unsigned num_draws)
{
        struct panfrost_context *ctx = pan_context(pipe);

        /* First of all, check the scissor to see if anything is drawn at all.
         * If it's not, we drop the draw (mostly a conformance issue;
         * well-behaved apps shouldn't hit this) */

        if (panfrost_scissor_culls_everything(ctx))
                return;

        if (indirect) {
                assert(num_draws == 1);
                panfrost_indirect_draw(ctx, info, drawid_offset, indirect, &draws[0]);
                return;
        }

        struct pipe_draw_info tmp_info = *info;
        unsigned drawid = drawid_offset;

        for (unsigned i = 0; i < num_draws; i++) {
                panfrost_direct_draw(ctx, &tmp_info, drawid, &draws[i]);
                if (tmp_info.increment_draw_id)
                       drawid++;
        }

}

/* CSO state */

static void
panfrost_generic_cso_delete(struct pipe_context *pctx, void *hwcso)
{
        free(hwcso);
}

static void *
panfrost_create_rasterizer_state(
        struct pipe_context *pctx,
        const struct pipe_rasterizer_state *cso)
{
        struct panfrost_rasterizer *so = CALLOC_STRUCT(panfrost_rasterizer);

        so->base = *cso;

        /* Gauranteed with the core GL call, so don't expose ARB_polygon_offset */
        assert(cso->offset_clamp == 0.0);

        return so;
}

static void
panfrost_bind_rasterizer_state(
        struct pipe_context *pctx,
        void *hwcso)
{
        struct panfrost_context *ctx = pan_context(pctx);
        ctx->rasterizer = hwcso;
}

static void
panfrost_set_shader_images(
        struct pipe_context *pctx,
        enum pipe_shader_type shader,
        unsigned start_slot, unsigned count, unsigned unbind_num_trailing_slots,
        const struct pipe_image_view *iviews)
{
        struct panfrost_context *ctx = pan_context(pctx);

        /* Unbind start_slot...start_slot+count */
        if (!iviews) {
                for (int i = start_slot; i < start_slot + count + unbind_num_trailing_slots; i++) {
                        pipe_resource_reference(&ctx->images[shader][i].resource, NULL);
                }

                ctx->image_mask[shader] &= ~(((1ull << count) - 1) << start_slot);
                return;
        }

        /* Bind start_slot...start_slot+count */
        for (int i = 0; i < count; i++) {
                const struct pipe_image_view *image = &iviews[i];
                SET_BIT(ctx->image_mask[shader], 1 << (start_slot + i), image->resource);

                if (!image->resource) {
                        util_copy_image_view(&ctx->images[shader][start_slot+i], NULL);
                        continue;
                }

                struct panfrost_resource *rsrc = pan_resource(image->resource);

                /* Images don't work with AFBC, since they require pixel-level granularity */
                if (drm_is_afbc(rsrc->image.layout.modifier)) {
                        pan_resource_modifier_convert(ctx, rsrc,
                                        DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED);
                }

                util_copy_image_view(&ctx->images[shader][start_slot+i], image);
        }

        /* Unbind start_slot+count...start_slot+count+unbind_num_trailing_slots */
        for (int i = 0; i < unbind_num_trailing_slots; i++) {
                SET_BIT(ctx->image_mask[shader], 1 << (start_slot + count + i), NULL);
                util_copy_image_view(&ctx->images[shader][start_slot+count+i], NULL);
        }
}

static void *
panfrost_create_vertex_elements_state(
        struct pipe_context *pctx,
        unsigned num_elements,
        const struct pipe_vertex_element *elements)
{
        struct panfrost_vertex_state *so = CALLOC_STRUCT(panfrost_vertex_state);
        struct panfrost_device *dev = pan_device(pctx->screen);

        so->num_elements = num_elements;
        memcpy(so->pipe, elements, sizeof(*elements) * num_elements);

        for (int i = 0; i < num_elements; ++i) {
                enum pipe_format fmt = elements[i].src_format;
                const struct util_format_description *desc = util_format_description(fmt);
                so->formats[i] = dev->formats[desc->format].hw;
                assert(so->formats[i]);
        }

        /* Let's also prepare vertex builtins */
        so->formats[PAN_VERTEX_ID] = dev->formats[PIPE_FORMAT_R32_UINT].hw;
        so->formats[PAN_INSTANCE_ID] = dev->formats[PIPE_FORMAT_R32_UINT].hw;

        return so;
}

static void
panfrost_bind_vertex_elements_state(
        struct pipe_context *pctx,
        void *hwcso)
{
        struct panfrost_context *ctx = pan_context(pctx);
        ctx->vertex = hwcso;
}

static void *
panfrost_create_shader_state(
        struct pipe_context *pctx,
        const struct pipe_shader_state *cso,
        enum pipe_shader_type stage)
{
        struct panfrost_shader_variants *so = CALLOC_STRUCT(panfrost_shader_variants);
        struct panfrost_device *dev = pan_device(pctx->screen);
        so->base = *cso;

        /* Token deep copy to prevent memory corruption */

        if (cso->type == PIPE_SHADER_IR_TGSI)
                so->base.tokens = tgsi_dup_tokens(so->base.tokens);

        /* Precompile for shader-db if we need to */
        if (unlikely((dev->debug & PAN_DBG_PRECOMPILE) && cso->type == PIPE_SHADER_IR_NIR)) {
                struct panfrost_context *ctx = pan_context(pctx);

                struct panfrost_shader_state state = { 0 };

                panfrost_shader_compile(ctx, PIPE_SHADER_IR_NIR,
                                        so->base.ir.nir,
                                        tgsi_processor_to_shader_stage(stage),
                                        &state);
        }

        return so;
}

static void
panfrost_delete_shader_state(
        struct pipe_context *pctx,
        void *so)
{
        struct panfrost_shader_variants *cso = (struct panfrost_shader_variants *) so;

        if (cso->base.type == PIPE_SHADER_IR_TGSI) {
                /* TODO: leaks TGSI tokens! */
        }

        for (unsigned i = 0; i < cso->variant_count; ++i) {
                struct panfrost_shader_state *shader_state = &cso->variants[i];
                panfrost_bo_unreference(shader_state->bo);

                if (shader_state->upload.rsrc)
                        pipe_resource_reference(&shader_state->upload.rsrc, NULL);

                shader_state->bo = NULL;
        }
        free(cso->variants);


        free(so);
}

static void *
panfrost_create_sampler_state(
        struct pipe_context *pctx,
        const struct pipe_sampler_state *cso)
{
        struct panfrost_sampler_state *so = CALLOC_STRUCT(panfrost_sampler_state);
        struct panfrost_device *device = pan_device(pctx->screen);

        so->base = *cso;

        if (pan_is_bifrost(device))
                panfrost_sampler_desc_init_bifrost(cso, (struct mali_bifrost_sampler_packed *) &so->hw);
        else
                panfrost_sampler_desc_init(cso, &so->hw);

        return so;
}

static void
panfrost_bind_sampler_states(
        struct pipe_context *pctx,
        enum pipe_shader_type shader,
        unsigned start_slot, unsigned num_sampler,
        void **sampler)
{
        assert(start_slot == 0);

        struct panfrost_context *ctx = pan_context(pctx);

        /* XXX: Should upload, not just copy? */
        ctx->sampler_count[shader] = num_sampler;
        if (sampler)
                memcpy(ctx->samplers[shader], sampler, num_sampler * sizeof (void *));
        else
                memset(ctx->samplers[shader], 0, num_sampler * sizeof (void *));
}

static bool
panfrost_variant_matches(
        struct panfrost_context *ctx,
        struct panfrost_shader_state *variant,
        enum pipe_shader_type type)
{
        struct panfrost_device *dev = pan_device(ctx->base.screen);

        if (variant->info.stage == MESA_SHADER_FRAGMENT &&
            variant->info.fs.outputs_read) {
                struct pipe_framebuffer_state *fb = &ctx->pipe_framebuffer;

                unsigned i;
                BITSET_FOREACH_SET(i, &variant->info.fs.outputs_read, 8) {
                        enum pipe_format fmt = PIPE_FORMAT_R8G8B8A8_UNORM;

                        if ((fb->nr_cbufs > i) && fb->cbufs[i])
                                fmt = fb->cbufs[i]->format;

                        const struct util_format_description *desc =
                                util_format_description(fmt);

                        if (pan_format_class_load(desc, dev->quirks) == PAN_FORMAT_NATIVE)
                                fmt = PIPE_FORMAT_NONE;

                        if (variant->rt_formats[i] != fmt)
                                return false;
                }
        }

        if (variant->info.stage == MESA_SHADER_FRAGMENT &&
            variant->nr_cbufs != ctx->pipe_framebuffer.nr_cbufs)
                return false;

        /* Otherwise, we're good to go */
        return true;
}

/**
 * Fix an uncompiled shader's stream output info, and produce a bitmask
 * of which VARYING_SLOT_* are captured for stream output.
 *
 * Core Gallium stores output->register_index as a "slot" number, where
 * slots are assigned consecutively to all outputs in info->outputs_written.
 * This naive packing of outputs doesn't work for us - we too have slots,
 * but the layout is defined by the VUE map, which we won't have until we
 * compile a specific shader variant.  So, we remap these and simply store
 * VARYING_SLOT_* in our copy's output->register_index fields.
 *
 * We then produce a bitmask of outputs which are used for SO.
 *
 * Implementation from iris.
 */

static uint64_t
update_so_info(struct pipe_stream_output_info *so_info,
               uint64_t outputs_written)
{
	uint64_t so_outputs = 0;
	uint8_t reverse_map[64] = {0};
	unsigned slot = 0;

	while (outputs_written)
		reverse_map[slot++] = u_bit_scan64(&outputs_written);

	for (unsigned i = 0; i < so_info->num_outputs; i++) {
		struct pipe_stream_output *output = &so_info->output[i];

		/* Map Gallium's condensed "slots" back to real VARYING_SLOT_* enums */
		output->register_index = reverse_map[output->register_index];

		so_outputs |= 1ull << output->register_index;
	}

	return so_outputs;
}

static void
panfrost_bind_shader_state(
        struct pipe_context *pctx,
        void *hwcso,
        enum pipe_shader_type type)
{
        struct panfrost_context *ctx = pan_context(pctx);
        struct panfrost_device *dev = pan_device(ctx->base.screen);
        ctx->shader[type] = hwcso;

        if (!hwcso) return;

        /* Match the appropriate variant */

        signed variant = -1;
        struct panfrost_shader_variants *variants = (struct panfrost_shader_variants *) hwcso;

        for (unsigned i = 0; i < variants->variant_count; ++i) {
                if (panfrost_variant_matches(ctx, &variants->variants[i], type)) {
                        variant = i;
                        break;
                }
        }

        if (variant == -1) {
                /* No variant matched, so create a new one */
                variant = variants->variant_count++;

                if (variants->variant_count > variants->variant_space) {
                        unsigned old_space = variants->variant_space;

                        variants->variant_space *= 2;
                        if (variants->variant_space == 0)
                                variants->variant_space = 1;

                        /* Arbitrary limit to stop runaway programs from
                         * creating an unbounded number of shader variants. */
                        assert(variants->variant_space < 1024);

                        unsigned msize = sizeof(struct panfrost_shader_state);
                        variants->variants = realloc(variants->variants,
                                                     variants->variant_space * msize);

                        memset(&variants->variants[old_space], 0,
                               (variants->variant_space - old_space) * msize);
                }

                struct panfrost_shader_state *v =
                                &variants->variants[variant];

                if (type == PIPE_SHADER_FRAGMENT) {
                        struct pipe_framebuffer_state *fb = &ctx->pipe_framebuffer;
                        v->nr_cbufs = fb->nr_cbufs;

                        for (unsigned i = 0; i < fb->nr_cbufs; ++i) {
                                enum pipe_format fmt = PIPE_FORMAT_R8G8B8A8_UNORM;

                                if ((fb->nr_cbufs > i) && fb->cbufs[i])
                                        fmt = fb->cbufs[i]->format;

                                const struct util_format_description *desc =
                                        util_format_description(fmt);

                                if (pan_format_class_load(desc, dev->quirks) == PAN_FORMAT_NATIVE)
                                        fmt = PIPE_FORMAT_NONE;

                                v->rt_formats[i] = fmt;
                        }
                }
        }

        /* Select this variant */
        variants->active_variant = variant;

        struct panfrost_shader_state *shader_state = &variants->variants[variant];
        assert(panfrost_variant_matches(ctx, shader_state, type));

        /* We finally have a variant, so compile it */

        if (!shader_state->compiled) {
                panfrost_shader_compile(ctx, variants->base.type,
                                        variants->base.type == PIPE_SHADER_IR_NIR ?
                                        variants->base.ir.nir :
                                        variants->base.tokens,
                                        tgsi_processor_to_shader_stage(type),
                                        shader_state);

                shader_state->compiled = true;

                /* Fixup the stream out information, since what Gallium returns
                 * normally is mildly insane */

                shader_state->stream_output = variants->base.stream_output;
                shader_state->so_mask =
                        update_so_info(&shader_state->stream_output,
                                       shader_state->info.outputs_written);
        }
}

static void *
panfrost_create_vs_state(struct pipe_context *pctx, const struct pipe_shader_state *hwcso)
{
        return panfrost_create_shader_state(pctx, hwcso, PIPE_SHADER_VERTEX);
}

static void *
panfrost_create_fs_state(struct pipe_context *pctx, const struct pipe_shader_state *hwcso)
{
        return panfrost_create_shader_state(pctx, hwcso, PIPE_SHADER_FRAGMENT);
}

static void
panfrost_bind_vs_state(struct pipe_context *pctx, void *hwcso)
{
        panfrost_bind_shader_state(pctx, hwcso, PIPE_SHADER_VERTEX);
}

static void
panfrost_bind_fs_state(struct pipe_context *pctx, void *hwcso)
{
        panfrost_bind_shader_state(pctx, hwcso, PIPE_SHADER_FRAGMENT);
}

static void
panfrost_set_vertex_buffers(
        struct pipe_context *pctx,
        unsigned start_slot,
        unsigned num_buffers,
        unsigned unbind_num_trailing_slots,
        bool take_ownership,
        const struct pipe_vertex_buffer *buffers)
{
        struct panfrost_context *ctx = pan_context(pctx);

        util_set_vertex_buffers_mask(ctx->vertex_buffers, &ctx->vb_mask, buffers,
                                     start_slot, num_buffers, unbind_num_trailing_slots,
                                     take_ownership);
}

static void
panfrost_set_constant_buffer(
        struct pipe_context *pctx,
        enum pipe_shader_type shader, uint index, bool take_ownership,
        const struct pipe_constant_buffer *buf)
{
        struct panfrost_context *ctx = pan_context(pctx);
        struct panfrost_constant_buffer *pbuf = &ctx->constant_buffer[shader];

        util_copy_constant_buffer(&pbuf->cb[index], buf, take_ownership);

        unsigned mask = (1 << index);

        if (unlikely(!buf)) {
                pbuf->enabled_mask &= ~mask;
                pbuf->dirty_mask &= ~mask;
                return;
        }

        pbuf->enabled_mask |= mask;
        pbuf->dirty_mask |= mask;
}

static void
panfrost_set_stencil_ref(
        struct pipe_context *pctx,
        const struct pipe_stencil_ref ref)
{
        struct panfrost_context *ctx = pan_context(pctx);
        ctx->stencil_ref = ref;
}

void
panfrost_create_sampler_view_bo(struct panfrost_sampler_view *so,
                                struct pipe_context *pctx,
                                struct pipe_resource *texture)
{
        struct panfrost_device *device = pan_device(pctx->screen);
        struct panfrost_resource *prsrc = (struct panfrost_resource *)texture;
        enum pipe_format format = so->base.format;
        assert(prsrc->image.data.bo);

        /* Format to access the stencil portion of a Z32_S8 texture */
        if (format == PIPE_FORMAT_X32_S8X24_UINT) {
                assert(prsrc->separate_stencil);
                texture = &prsrc->separate_stencil->base;
                prsrc = (struct panfrost_resource *)texture;
                format = texture->format;
        }

        const struct util_format_description *desc = util_format_description(format);

        bool fake_rgtc = !panfrost_supports_compressed_format(device, MALI_BC4_UNORM);

        if (desc->layout == UTIL_FORMAT_LAYOUT_RGTC && fake_rgtc) {
                if (desc->is_snorm)
                        format = PIPE_FORMAT_R8G8B8A8_SNORM;
                else
                        format = PIPE_FORMAT_R8G8B8A8_UNORM;
                desc = util_format_description(format);
        }

        so->texture_bo = prsrc->image.data.bo->ptr.gpu;
        so->modifier = prsrc->image.layout.modifier;

        /* MSAA only supported for 2D textures */

        assert(texture->nr_samples <= 1 ||
               so->base.target == PIPE_TEXTURE_2D ||
               so->base.target == PIPE_TEXTURE_2D_ARRAY);

        enum mali_texture_dimension type =
                panfrost_translate_texture_dimension(so->base.target);

        bool is_buffer = (so->base.target == PIPE_BUFFER);

        unsigned first_level = is_buffer ? 0 : so->base.u.tex.first_level;
        unsigned last_level = is_buffer ? 0 : so->base.u.tex.last_level;
        unsigned first_layer = is_buffer ? 0 : so->base.u.tex.first_layer;
        unsigned last_layer = is_buffer ? 0 : so->base.u.tex.last_layer;
        unsigned buf_offset = is_buffer ? so->base.u.buf.offset : 0;
        unsigned buf_size = (is_buffer ? so->base.u.buf.size : 0) /
                            util_format_get_blocksize(format);

        if (so->base.target == PIPE_TEXTURE_3D) {
                first_layer /= prsrc->image.layout.depth;
                last_layer /= prsrc->image.layout.depth;
                assert(!first_layer && !last_layer);
        }

        unsigned size =
                (pan_is_bifrost(device) ? 0 : MALI_MIDGARD_TEXTURE_LENGTH) +
                panfrost_estimate_texture_payload_size(device,
                                                       first_level, last_level,
                                                       first_layer, last_layer,
                                                       texture->nr_samples,
                                                       type,
                                                       prsrc->image.layout.modifier);

        so->bo = panfrost_bo_create(device, size, 0);

        struct panfrost_ptr payload = so->bo->ptr;
        void *tex = pan_is_bifrost(device) ?
                    &so->bifrost_descriptor : so->bo->ptr.cpu;

        if (!pan_is_bifrost(device)) {
                payload.cpu += MALI_MIDGARD_TEXTURE_LENGTH;
                payload.gpu += MALI_MIDGARD_TEXTURE_LENGTH;
        }

        struct pan_image_view iview = {
                .format = format,
                .dim = type,
                .first_level = first_level,
                .last_level = last_level,
                .first_layer = first_layer,
                .last_layer = last_layer,
                .swizzle = {
                        so->base.swizzle_r,
                        so->base.swizzle_g,
                        so->base.swizzle_b,
                        so->base.swizzle_a,
                },
                .image = &prsrc->image,

                .buf.offset = buf_offset,
                .buf.size = buf_size,
        };

        panfrost_new_texture(device, &iview, tex, &payload);
}

static struct pipe_sampler_view *
panfrost_create_sampler_view(
        struct pipe_context *pctx,
        struct pipe_resource *texture,
        const struct pipe_sampler_view *template)
{
        struct panfrost_sampler_view *so = rzalloc(pctx, struct panfrost_sampler_view);

        pipe_reference(NULL, &texture->reference);

        so->base = *template;
        so->base.texture = texture;
        so->base.reference.count = 1;
        so->base.context = pctx;

        panfrost_create_sampler_view_bo(so, pctx, texture);

        return (struct pipe_sampler_view *) so;
}

static void
panfrost_set_sampler_views(
        struct pipe_context *pctx,
        enum pipe_shader_type shader,
        unsigned start_slot, unsigned num_views,
        unsigned unbind_num_trailing_slots,
        struct pipe_sampler_view **views)
{
        struct panfrost_context *ctx = pan_context(pctx);
        unsigned new_nr = 0;
        unsigned i;

        assert(start_slot == 0);

        if (!views)
                num_views = 0;

        for (i = 0; i < num_views; ++i) {
                if (views[i])
                        new_nr = i + 1;
		pipe_sampler_view_reference((struct pipe_sampler_view **)&ctx->sampler_views[shader][i],
		                            views[i]);
        }

        for (; i < ctx->sampler_view_count[shader]; i++) {
		pipe_sampler_view_reference((struct pipe_sampler_view **)&ctx->sampler_views[shader][i],
		                            NULL);
        }
        ctx->sampler_view_count[shader] = new_nr;
}

static void
panfrost_sampler_view_destroy(
        struct pipe_context *pctx,
        struct pipe_sampler_view *pview)
{
        struct panfrost_sampler_view *view = (struct panfrost_sampler_view *) pview;

        pipe_resource_reference(&pview->texture, NULL);
        panfrost_bo_unreference(view->bo);
        ralloc_free(view);
}

static void
panfrost_set_shader_buffers(
        struct pipe_context *pctx,
        enum pipe_shader_type shader,
        unsigned start, unsigned count,
        const struct pipe_shader_buffer *buffers,
        unsigned writable_bitmask)
{
        struct panfrost_context *ctx = pan_context(pctx);

        util_set_shader_buffers_mask(ctx->ssbo[shader], &ctx->ssbo_mask[shader],
                        buffers, start, count);
}

static void
panfrost_set_framebuffer_state(struct pipe_context *pctx,
                               const struct pipe_framebuffer_state *fb)
{
        struct panfrost_context *ctx = pan_context(pctx);

        util_copy_framebuffer_state(&ctx->pipe_framebuffer, fb);
        ctx->batch = NULL;

        /* We may need to generate a new variant if the fragment shader is
         * keyed to the framebuffer format (due to EXT_framebuffer_fetch) */
        struct panfrost_shader_variants *fs = ctx->shader[PIPE_SHADER_FRAGMENT];

        if (fs && fs->variant_count &&
            fs->variants[fs->active_variant].info.fs.outputs_read)
                ctx->base.bind_fs_state(&ctx->base, fs);
}

static inline unsigned
pan_pipe_to_stencil_op(enum pipe_stencil_op in)
{
        switch (in) {
        case PIPE_STENCIL_OP_KEEP: return MALI_STENCIL_OP_KEEP;
        case PIPE_STENCIL_OP_ZERO: return MALI_STENCIL_OP_ZERO;
        case PIPE_STENCIL_OP_REPLACE: return MALI_STENCIL_OP_REPLACE;
        case PIPE_STENCIL_OP_INCR: return MALI_STENCIL_OP_INCR_SAT;
        case PIPE_STENCIL_OP_DECR: return MALI_STENCIL_OP_DECR_SAT;
        case PIPE_STENCIL_OP_INCR_WRAP: return MALI_STENCIL_OP_INCR_WRAP;
        case PIPE_STENCIL_OP_DECR_WRAP: return MALI_STENCIL_OP_DECR_WRAP;
        case PIPE_STENCIL_OP_INVERT: return MALI_STENCIL_OP_INVERT;
        default: unreachable("Invalid stencil op");
        }
}

static inline void
pan_pipe_to_stencil(const struct pipe_stencil_state *in, struct MALI_STENCIL *out)
{
        pan_prepare(out, STENCIL);
        out->mask = in->valuemask;
        out->compare_function = panfrost_translate_compare_func(in->func);
        out->stencil_fail = pan_pipe_to_stencil_op(in->fail_op);
        out->depth_fail = pan_pipe_to_stencil_op(in->zfail_op);
        out->depth_pass = pan_pipe_to_stencil_op(in->zpass_op);
}

static void *
panfrost_create_depth_stencil_state(struct pipe_context *pipe,
                                    const struct pipe_depth_stencil_alpha_state *zsa)
{
        struct panfrost_zsa_state *so = CALLOC_STRUCT(panfrost_zsa_state);
        so->base = *zsa;

        pan_pipe_to_stencil(&zsa->stencil[0], &so->stencil_front);
        so->stencil_mask_front = zsa->stencil[0].writemask;

        if (zsa->stencil[1].enabled) {
                pan_pipe_to_stencil(&zsa->stencil[1], &so->stencil_back);
                so->stencil_mask_back = zsa->stencil[1].writemask;
	} else {
                so->stencil_back = so->stencil_front;
                so->stencil_mask_back = so->stencil_mask_front;
        }

        so->alpha_func = zsa->alpha_enabled ?
                panfrost_translate_compare_func(zsa->alpha_func) :
                MALI_FUNC_ALWAYS;

        /* TODO: Bounds test should be easy */
        assert(!zsa->depth_bounds_test);

        return so;
}

static void
panfrost_bind_depth_stencil_state(struct pipe_context *pipe,
                                  void *cso)
{
        struct panfrost_context *ctx = pan_context(pipe);
        ctx->depth_stencil = cso;
}

static void
panfrost_delete_depth_stencil_state(struct pipe_context *pipe, void *depth)
{
        free( depth );
}

static void
panfrost_set_sample_mask(struct pipe_context *pipe,
                         unsigned sample_mask)
{
        struct panfrost_context *ctx = pan_context(pipe);
        ctx->sample_mask = sample_mask;
}

static void
panfrost_set_min_samples(struct pipe_context *pipe,
                         unsigned min_samples)
{
        struct panfrost_context *ctx = pan_context(pipe);
        ctx->min_samples = min_samples;
}

static void
panfrost_get_sample_position(struct pipe_context *context,
                             unsigned sample_count,
                             unsigned sample_index,
                             float *out_value)
{
        panfrost_query_sample_position(
                        panfrost_sample_pattern(sample_count),
                        sample_index,
                        out_value);
}

static void
panfrost_set_clip_state(struct pipe_context *pipe,
                        const struct pipe_clip_state *clip)
{
        //struct panfrost_context *panfrost = pan_context(pipe);
}

static void
panfrost_set_viewport_states(struct pipe_context *pipe,
                             unsigned start_slot,
                             unsigned num_viewports,
                             const struct pipe_viewport_state *viewports)
{
        struct panfrost_context *ctx = pan_context(pipe);

        assert(start_slot == 0);
        assert(num_viewports == 1);

        ctx->pipe_viewport = *viewports;
}

static void
panfrost_set_scissor_states(struct pipe_context *pipe,
                            unsigned start_slot,
                            unsigned num_scissors,
                            const struct pipe_scissor_state *scissors)
{
        struct panfrost_context *ctx = pan_context(pipe);

        assert(start_slot == 0);
        assert(num_scissors == 1);

        ctx->scissor = *scissors;
}

static void
panfrost_set_polygon_stipple(struct pipe_context *pipe,
                             const struct pipe_poly_stipple *stipple)
{
        //struct panfrost_context *panfrost = pan_context(pipe);
}

static void
panfrost_set_active_query_state(struct pipe_context *pipe,
                                bool enable)
{
        struct panfrost_context *ctx = pan_context(pipe);
        ctx->active_queries = enable;
}

static void
panfrost_render_condition(struct pipe_context *pipe,
                          struct pipe_query *query,
                          bool condition,
                          enum pipe_render_cond_flag mode)
{
        struct panfrost_context *ctx = pan_context(pipe);

        ctx->cond_query = (struct panfrost_query *)query;
        ctx->cond_cond = condition;
        ctx->cond_mode = mode;
}

static void
panfrost_destroy(struct pipe_context *pipe)
{
        struct panfrost_context *panfrost = pan_context(pipe);

        if (panfrost->blitter)
                util_blitter_destroy(panfrost->blitter);

        util_unreference_framebuffer_state(&panfrost->pipe_framebuffer);
        u_upload_destroy(pipe->stream_uploader);
        u_upload_destroy(panfrost->state_uploader);

        ralloc_free(pipe);
}

static struct pipe_query *
panfrost_create_query(struct pipe_context *pipe,
                      unsigned type,
                      unsigned index)
{
        struct panfrost_query *q = rzalloc(pipe, struct panfrost_query);

        q->type = type;
        q->index = index;

        return (struct pipe_query *) q;
}

static void
panfrost_destroy_query(struct pipe_context *pipe, struct pipe_query *q)
{
        struct panfrost_query *query = (struct panfrost_query *) q;

        if (query->bo) {
                panfrost_bo_unreference(query->bo);
                query->bo = NULL;
        }

        ralloc_free(q);
}

static bool
panfrost_begin_query(struct pipe_context *pipe, struct pipe_query *q)
{
        struct panfrost_context *ctx = pan_context(pipe);
        struct panfrost_device *dev = pan_device(ctx->base.screen);
        struct panfrost_query *query = (struct panfrost_query *) q;

        switch (query->type) {
        case PIPE_QUERY_OCCLUSION_COUNTER:
        case PIPE_QUERY_OCCLUSION_PREDICATE:
        case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE: {
                unsigned size = sizeof(uint64_t) * dev->core_count;

                /* Allocate a bo for the query results to be stored */
                if (!query->bo) {
                        query->bo = panfrost_bo_create(dev, size, 0);
                }

                /* Default to 0 if nothing at all drawn. */
                memset(query->bo->ptr.cpu, 0, size);

                query->msaa = (ctx->pipe_framebuffer.samples > 1);
                ctx->occlusion_query = query;
                break;
        }

        /* Geometry statistics are computed in the driver. XXX: geom/tess
         * shaders.. */

        case PIPE_QUERY_PRIMITIVES_GENERATED:
                query->start = ctx->prims_generated;
                break;
        case PIPE_QUERY_PRIMITIVES_EMITTED:
                query->start = ctx->tf_prims_generated;
                break;

        default:
                /* TODO: timestamp queries, etc? */
                break;
        }

        return true;
}

static bool
panfrost_end_query(struct pipe_context *pipe, struct pipe_query *q)
{
        struct panfrost_context *ctx = pan_context(pipe);
        struct panfrost_query *query = (struct panfrost_query *) q;

        switch (query->type) {
        case PIPE_QUERY_OCCLUSION_COUNTER:
        case PIPE_QUERY_OCCLUSION_PREDICATE:
        case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
                ctx->occlusion_query = NULL;
                break;
        case PIPE_QUERY_PRIMITIVES_GENERATED:
                query->end = ctx->prims_generated;
                break;
        case PIPE_QUERY_PRIMITIVES_EMITTED:
                query->end = ctx->tf_prims_generated;
                break;
        }

        return true;
}

static bool
panfrost_get_query_result(struct pipe_context *pipe,
                          struct pipe_query *q,
                          bool wait,
                          union pipe_query_result *vresult)
{
        struct panfrost_query *query = (struct panfrost_query *) q;
        struct panfrost_context *ctx = pan_context(pipe);
        struct panfrost_device *dev = pan_device(ctx->base.screen);

        switch (query->type) {
        case PIPE_QUERY_OCCLUSION_COUNTER:
        case PIPE_QUERY_OCCLUSION_PREDICATE:
        case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
                panfrost_flush_batches_accessing_bo(ctx, query->bo, false);
                panfrost_bo_wait(query->bo, INT64_MAX, false);

                /* Read back the query results */
                uint64_t *result = (uint64_t *) query->bo->ptr.cpu;

                if (query->type == PIPE_QUERY_OCCLUSION_COUNTER) {
                        uint64_t passed = 0;
                        for (int i = 0; i < dev->core_count; ++i)
                                passed += result[i];

                        if (!pan_is_bifrost(dev) && !query->msaa)
                                passed /= 4;

                        vresult->u64 = passed;
                } else {
                        vresult->b = !!result[0];
                }

                break;

        case PIPE_QUERY_PRIMITIVES_GENERATED:
        case PIPE_QUERY_PRIMITIVES_EMITTED:
                panfrost_flush_all_batches(ctx);
                vresult->u64 = query->end - query->start;
                break;

        default:
                /* TODO: more queries */
                break;
        }

        return true;
}

bool
panfrost_render_condition_check(struct panfrost_context *ctx)
{
	if (!ctx->cond_query)
		return true;

	union pipe_query_result res = { 0 };
	bool wait =
		ctx->cond_mode != PIPE_RENDER_COND_NO_WAIT &&
		ctx->cond_mode != PIPE_RENDER_COND_BY_REGION_NO_WAIT;

        struct pipe_query *pq = (struct pipe_query *)ctx->cond_query;

        if (panfrost_get_query_result(&ctx->base, pq, wait, &res))
                return res.u64 != ctx->cond_cond;

	return true;
}

static struct pipe_stream_output_target *
panfrost_create_stream_output_target(struct pipe_context *pctx,
                                     struct pipe_resource *prsc,
                                     unsigned buffer_offset,
                                     unsigned buffer_size)
{
        struct pipe_stream_output_target *target;

        target = &rzalloc(pctx, struct panfrost_streamout_target)->base;

        if (!target)
                return NULL;

        pipe_reference_init(&target->reference, 1);
        pipe_resource_reference(&target->buffer, prsc);

        target->context = pctx;
        target->buffer_offset = buffer_offset;
        target->buffer_size = buffer_size;

        return target;
}

static void
panfrost_stream_output_target_destroy(struct pipe_context *pctx,
                                      struct pipe_stream_output_target *target)
{
        pipe_resource_reference(&target->buffer, NULL);
        ralloc_free(target);
}

static void
panfrost_set_stream_output_targets(struct pipe_context *pctx,
                                   unsigned num_targets,
                                   struct pipe_stream_output_target **targets,
                                   const unsigned *offsets)
{
        struct panfrost_context *ctx = pan_context(pctx);
        struct panfrost_streamout *so = &ctx->streamout;

        assert(num_targets <= ARRAY_SIZE(so->targets));

        for (unsigned i = 0; i < num_targets; i++) {
                if (offsets[i] != -1)
                        pan_so_target(targets[i])->offset = offsets[i];

                pipe_so_target_reference(&so->targets[i], targets[i]);
        }

        for (unsigned i = 0; i < so->num_targets; i++)
                pipe_so_target_reference(&so->targets[i], NULL);

        so->num_targets = num_targets;
}

struct pipe_context *
panfrost_create_context(struct pipe_screen *screen, void *priv, unsigned flags)
{
        struct panfrost_context *ctx = rzalloc(screen, struct panfrost_context);
        struct pipe_context *gallium = (struct pipe_context *) ctx;
        struct panfrost_device *dev = pan_device(screen);

        gallium->screen = screen;

        gallium->destroy = panfrost_destroy;

        gallium->set_framebuffer_state = panfrost_set_framebuffer_state;

        gallium->flush = panfrost_flush;
        gallium->clear = panfrost_clear;
        gallium->draw_vbo = panfrost_draw_vbo;
        gallium->texture_barrier = panfrost_texture_barrier;
        gallium->set_frontend_noop = panfrost_set_frontend_noop;

        gallium->set_vertex_buffers = panfrost_set_vertex_buffers;
        gallium->set_constant_buffer = panfrost_set_constant_buffer;
        gallium->set_shader_buffers = panfrost_set_shader_buffers;
        gallium->set_shader_images = panfrost_set_shader_images;

        gallium->set_stencil_ref = panfrost_set_stencil_ref;

        gallium->create_sampler_view = panfrost_create_sampler_view;
        gallium->set_sampler_views = panfrost_set_sampler_views;
        gallium->sampler_view_destroy = panfrost_sampler_view_destroy;

        gallium->create_rasterizer_state = panfrost_create_rasterizer_state;
        gallium->bind_rasterizer_state = panfrost_bind_rasterizer_state;
        gallium->delete_rasterizer_state = panfrost_generic_cso_delete;

        gallium->create_vertex_elements_state = panfrost_create_vertex_elements_state;
        gallium->bind_vertex_elements_state = panfrost_bind_vertex_elements_state;
        gallium->delete_vertex_elements_state = panfrost_generic_cso_delete;

        gallium->create_fs_state = panfrost_create_fs_state;
        gallium->delete_fs_state = panfrost_delete_shader_state;
        gallium->bind_fs_state = panfrost_bind_fs_state;

        gallium->create_vs_state = panfrost_create_vs_state;
        gallium->delete_vs_state = panfrost_delete_shader_state;
        gallium->bind_vs_state = panfrost_bind_vs_state;

        gallium->create_sampler_state = panfrost_create_sampler_state;
        gallium->delete_sampler_state = panfrost_generic_cso_delete;
        gallium->bind_sampler_states = panfrost_bind_sampler_states;

        gallium->create_depth_stencil_alpha_state = panfrost_create_depth_stencil_state;
        gallium->bind_depth_stencil_alpha_state   = panfrost_bind_depth_stencil_state;
        gallium->delete_depth_stencil_alpha_state = panfrost_delete_depth_stencil_state;

        gallium->set_sample_mask = panfrost_set_sample_mask;
        gallium->set_min_samples = panfrost_set_min_samples;
        gallium->get_sample_position = panfrost_get_sample_position;

        gallium->set_clip_state = panfrost_set_clip_state;
        gallium->set_viewport_states = panfrost_set_viewport_states;
        gallium->set_scissor_states = panfrost_set_scissor_states;
        gallium->set_polygon_stipple = panfrost_set_polygon_stipple;
        gallium->set_active_query_state = panfrost_set_active_query_state;
        gallium->render_condition = panfrost_render_condition;

        gallium->create_query = panfrost_create_query;
        gallium->destroy_query = panfrost_destroy_query;
        gallium->begin_query = panfrost_begin_query;
        gallium->end_query = panfrost_end_query;
        gallium->get_query_result = panfrost_get_query_result;

        gallium->create_stream_output_target = panfrost_create_stream_output_target;
        gallium->stream_output_target_destroy = panfrost_stream_output_target_destroy;
        gallium->set_stream_output_targets = panfrost_set_stream_output_targets;

        panfrost_resource_context_init(gallium);
        panfrost_blend_context_init(gallium);
        panfrost_compute_context_init(gallium);

        gallium->stream_uploader = u_upload_create_default(gallium);
        gallium->const_uploader = gallium->stream_uploader;

        ctx->state_uploader = u_upload_create(gallium, 4096,
                        PIPE_BIND_CONSTANT_BUFFER, PIPE_USAGE_DYNAMIC, 0);

        /* All of our GPUs support ES mode. Midgard supports additionally
         * QUADS/QUAD_STRIPS/POLYGON. Bifrost supports just QUADS. */

        ctx->draw_modes = (1 << (PIPE_PRIM_QUADS + 1)) - 1;

        if (!pan_is_bifrost(dev)) {
                ctx->draw_modes |= (1 << PIPE_PRIM_QUAD_STRIP);
                ctx->draw_modes |= (1 << PIPE_PRIM_POLYGON);
        }

        ctx->primconvert = util_primconvert_create(gallium, ctx->draw_modes);

        ctx->blitter = util_blitter_create(gallium);

        assert(ctx->blitter);

        /* Prepare for render! */

        panfrost_batch_init(ctx);

        /* By default mask everything on */
        ctx->sample_mask = ~0;
        ctx->active_queries = true;

        int ASSERTED ret;

        /* Create a syncobj in a signaled state. Will be updated to point to the
         * last queued job out_sync every time we submit a new job.
         */
        ret = drmSyncobjCreate(dev->fd, DRM_SYNCOBJ_CREATE_SIGNALED, &ctx->syncobj);
        assert(!ret && ctx->syncobj);

        return gallium;
}
