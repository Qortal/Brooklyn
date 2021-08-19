#include "zink_compiler.h"
#include "zink_context.h"
#include "zink_program.h"
#include "zink_query.h"
#include "zink_resource.h"
#include "zink_screen.h"
#include "zink_state.h"
#include "zink_surface.h"
#include "zink_inlines.h"

#include "tgsi/tgsi_from_mesa.h"
#include "util/hash_table.h"
#include "util/u_debug.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_prim.h"
#include "util/u_prim_restart.h"


static void
zink_emit_xfb_counter_barrier(struct zink_context *ctx)
{
   /* Between the pause and resume there needs to be a memory barrier for the counter buffers
    * with a source access of VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT
    * at pipeline stage VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
    * to a destination access of VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT
    * at pipeline stage VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT.
    *
    * - from VK_EXT_transform_feedback spec
    */
   for (unsigned i = 0; i < ctx->num_so_targets; i++) {
      struct zink_so_target *t = zink_so_target(ctx->so_targets[i]);
      if (!t)
         continue;
      struct zink_resource *res = zink_resource(t->counter_buffer);
      if (t->counter_buffer_valid)
          zink_resource_buffer_barrier(ctx, NULL, res, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
                                       VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
      else
          zink_resource_buffer_barrier(ctx, NULL, res, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
                                       VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT);
   }
   ctx->xfb_barrier = false;
}

static void
zink_emit_xfb_vertex_input_barrier(struct zink_context *ctx, struct zink_resource *res)
{
   /* A pipeline barrier is required between using the buffers as
    * transform feedback buffers and vertex buffers to
    * ensure all writes to the transform feedback buffers are visible
    * when the data is read as vertex attributes.
    * The source access is VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT
    * and the destination access is VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
    * for the pipeline stages VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT
    * and VK_PIPELINE_STAGE_VERTEX_INPUT_BIT respectively.
    *
    * - 20.3.1. Drawing Transform Feedback
    */
   zink_resource_buffer_barrier(ctx, NULL, res, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
}

static void
zink_emit_stream_output_targets(struct pipe_context *pctx)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_batch *batch = &ctx->batch;
   VkBuffer buffers[PIPE_MAX_SO_OUTPUTS] = {0};
   VkDeviceSize buffer_offsets[PIPE_MAX_SO_OUTPUTS] = {0};
   VkDeviceSize buffer_sizes[PIPE_MAX_SO_OUTPUTS] = {0};

   for (unsigned i = 0; i < ctx->num_so_targets; i++) {
      struct zink_so_target *t = (struct zink_so_target *)ctx->so_targets[i];
      if (!t) {
         /* no need to reference this or anything */
         buffers[i] = zink_resource(ctx->dummy_xfb_buffer)->obj->buffer;
         buffer_offsets[i] = 0;
         buffer_sizes[i] = sizeof(uint8_t);
         continue;
      }
      struct zink_resource *res = zink_resource(t->base.buffer);
      if (!(res->bind_history & ZINK_RESOURCE_USAGE_STREAMOUT))
         /* resource has been rebound */
         t->counter_buffer_valid = false;
      buffers[i] = res->obj->buffer;
      zink_resource_buffer_barrier(ctx, NULL, zink_resource(t->base.buffer),
                                   VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT, VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT);
      zink_batch_reference_resource_rw(batch, res, true);
      buffer_offsets[i] = t->base.buffer_offset;
      buffer_sizes[i] = t->base.buffer_size;
      res->bind_history |= ZINK_RESOURCE_USAGE_STREAMOUT;
      util_range_add(t->base.buffer, &res->valid_buffer_range, t->base.buffer_offset,
                     t->base.buffer_offset + t->base.buffer_size);
   }

   screen->vk.CmdBindTransformFeedbackBuffersEXT(batch->state->cmdbuf, 0, ctx->num_so_targets,
                                                 buffers, buffer_offsets,
                                                 buffer_sizes);
   ctx->dirty_so_targets = false;
}

ALWAYS_INLINE static void
check_buffer_barrier(struct zink_context *ctx, struct pipe_resource *pres, VkAccessFlags flags, VkPipelineStageFlags pipeline)
{
   struct zink_resource *res = zink_resource(pres);
   zink_resource_buffer_barrier(ctx, NULL, res, flags, pipeline);
}

ALWAYS_INLINE static void
barrier_draw_buffers(struct zink_context *ctx, const struct pipe_draw_info *dinfo,
                     const struct pipe_draw_indirect_info *dindirect, struct pipe_resource *index_buffer)
{
   if (index_buffer)
      check_buffer_barrier(ctx, index_buffer, VK_ACCESS_INDEX_READ_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
   if (dindirect && dindirect->buffer) {
      check_buffer_barrier(ctx, dindirect->buffer,
                           VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
      if (dindirect->indirect_draw_count)
         check_buffer_barrier(ctx, dindirect->indirect_draw_count,
                              VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
   }
}

template <zink_dynamic_state HAS_DYNAMIC_STATE>
static void
zink_bind_vertex_buffers(struct zink_batch *batch, struct zink_context *ctx)
{
   VkBuffer buffers[PIPE_MAX_ATTRIBS];
   VkDeviceSize buffer_offsets[PIPE_MAX_ATTRIBS];
   VkDeviceSize buffer_strides[PIPE_MAX_ATTRIBS];
   struct zink_vertex_elements_state *elems = ctx->element_state;
   struct zink_screen *screen = zink_screen(ctx->base.screen);

   if (!elems->hw_state.num_bindings)
      return;

   for (unsigned i = 0; i < elems->hw_state.num_bindings; i++) {
      const unsigned buffer_id = ctx->element_state->binding_map[i];
      struct pipe_vertex_buffer *vb = ctx->vertex_buffers + buffer_id;
      assert(vb);
      if (vb->buffer.resource) {
         buffers[i] = ctx->vbufs[buffer_id];
         assert(buffers[i]);
         if (screen->info.have_EXT_vertex_input_dynamic_state)
            elems->hw_state.dynbindings[i].stride = vb->stride;
         buffer_offsets[i] = ctx->vbuf_offsets[buffer_id];
         buffer_strides[i] = vb->stride;
      } else {
         buffers[i] = zink_resource(ctx->dummy_vertex_buffer)->obj->buffer;
         buffer_offsets[i] = 0;
         buffer_strides[i] = 0;
         if (screen->info.have_EXT_vertex_input_dynamic_state)
            elems->hw_state.dynbindings[i].stride = 0;
      }
   }

   if (HAS_DYNAMIC_STATE)
      screen->vk.CmdBindVertexBuffers2EXT(batch->state->cmdbuf, 0,
                                          elems->hw_state.num_bindings,
                                          buffers, buffer_offsets, NULL, buffer_strides);
   else
      vkCmdBindVertexBuffers(batch->state->cmdbuf, 0,
                             elems->hw_state.num_bindings,
                             buffers, buffer_offsets);

   if (screen->info.have_EXT_vertex_input_dynamic_state)
      screen->vk.CmdSetVertexInputEXT(batch->state->cmdbuf,
                                      elems->hw_state.num_bindings, elems->hw_state.dynbindings,
                                      elems->hw_state.num_attribs, elems->hw_state.dynattribs);

   ctx->vertex_state_changed = false;
   ctx->vertex_buffers_dirty = false;
}

static void
update_compute_program(struct zink_context *ctx)
{
   const unsigned bits = 1 << PIPE_SHADER_COMPUTE;
   if (ctx->dirty_shader_stages & bits) {
      struct zink_compute_program *comp = zink_create_compute_program(ctx, ctx->compute_stage);
      _mesa_hash_table_insert(ctx->compute_program_cache, comp->shader, comp);
      ctx->compute_pipeline_state.dirty = true;
      ctx->curr_compute = comp;
      ctx->dirty_shader_stages &= bits;
      zink_batch_reference_program(&ctx->batch, &ctx->curr_compute->base);
   }
}

static void
update_gfx_program(struct zink_context *ctx)
{
   if (ctx->last_vertex_stage_dirty) {
      if (ctx->gfx_stages[PIPE_SHADER_GEOMETRY])
         ctx->dirty_shader_stages |= BITFIELD_BIT(PIPE_SHADER_GEOMETRY);
      else if (ctx->gfx_stages[PIPE_SHADER_TESS_EVAL])
         ctx->dirty_shader_stages |= BITFIELD_BIT(PIPE_SHADER_TESS_EVAL);
      else
         ctx->dirty_shader_stages |= BITFIELD_BIT(PIPE_SHADER_VERTEX);
      ctx->last_vertex_stage_dirty = false;
   }
   unsigned bits = u_bit_consecutive(PIPE_SHADER_VERTEX, 5);
   if (ctx->dirty_shader_stages & bits) {
      struct zink_gfx_program *prog = NULL;
      struct hash_entry *entry = _mesa_hash_table_search(ctx->program_cache,
                                                         ctx->gfx_stages);
      if (entry)
         zink_update_gfx_program(ctx, (struct zink_gfx_program*)entry->data);
      else {
         prog = zink_create_gfx_program(ctx, ctx->gfx_stages);
         entry = _mesa_hash_table_insert(ctx->program_cache, prog->shaders, prog);
      }
      prog = (struct zink_gfx_program*)(entry ? entry->data : NULL);
      if (prog && prog != ctx->curr_program) {
         ctx->gfx_pipeline_state.combined_dirty = true;
         zink_batch_reference_program(&ctx->batch, &prog->base);
      }
      ctx->curr_program = prog;
      ctx->dirty_shader_stages &= ~bits;
   }
}

static bool
line_width_needed(enum pipe_prim_type reduced_prim,
                  VkPolygonMode polygon_mode)
{
   switch (reduced_prim) {
   case PIPE_PRIM_POINTS:
      return false;

   case PIPE_PRIM_LINES:
      return true;

   case PIPE_PRIM_TRIANGLES:
      return polygon_mode == VK_POLYGON_MODE_LINE;

   default:
      unreachable("unexpected reduced prim");
   }
}

ALWAYS_INLINE static void
update_drawid(struct zink_context *ctx, unsigned draw_id)
{
   vkCmdPushConstants(ctx->batch.state->cmdbuf, ctx->curr_program->base.layout, VK_SHADER_STAGE_VERTEX_BIT,
                      offsetof(struct zink_gfx_push_constant, draw_id), sizeof(unsigned),
                      &draw_id);
}

ALWAYS_INLINE static void
draw_indexed_need_index_buffer_unref(struct zink_context *ctx,
             const struct pipe_draw_info *dinfo,
             const struct pipe_draw_start_count_bias *draws,
             unsigned num_draws,
             unsigned draw_id,
             bool needs_drawid)
{
   VkCommandBuffer cmdbuf = ctx->batch.state->cmdbuf;
   if (dinfo->increment_draw_id && needs_drawid) {
      for (unsigned i = 0; i < num_draws; i++) {
         update_drawid(ctx, draw_id);
         vkCmdDrawIndexed(cmdbuf,
            draws[i].count, dinfo->instance_count,
            0, draws[i].index_bias, dinfo->start_instance);
         draw_id++;
      }
   } else {
      if (needs_drawid)
         update_drawid(ctx, draw_id);
      for (unsigned i = 0; i < num_draws; i++)
         vkCmdDrawIndexed(cmdbuf,
            draws[i].count, dinfo->instance_count,
            0, draws[i].index_bias, dinfo->start_instance);

   }
}

template <zink_multidraw HAS_MULTIDRAW>
ALWAYS_INLINE static void
draw_indexed(struct zink_context *ctx,
             const struct pipe_draw_info *dinfo,
             const struct pipe_draw_start_count_bias *draws,
             unsigned num_draws,
             unsigned draw_id,
             bool needs_drawid)
{
   VkCommandBuffer cmdbuf = ctx->batch.state->cmdbuf;
   if (dinfo->increment_draw_id && needs_drawid) {
      for (unsigned i = 0; i < num_draws; i++) {
         update_drawid(ctx, draw_id);
         vkCmdDrawIndexed(cmdbuf,
            draws[i].count, dinfo->instance_count,
            draws[i].start, draws[i].index_bias, dinfo->start_instance);
         draw_id++;
      }
   } else {
      if (needs_drawid)
         update_drawid(ctx, draw_id);
      if (HAS_MULTIDRAW) {
         zink_screen(ctx->base.screen)->vk.CmdDrawMultiIndexedEXT(cmdbuf, num_draws, (const VkMultiDrawIndexedInfoEXT*)draws,
                                                                   dinfo->instance_count,
                                                                   dinfo->start_instance, sizeof(struct pipe_draw_start_count_bias),
                                                                   dinfo->index_bias_varies ? NULL : &draws[0].index_bias);
      } else {
         for (unsigned i = 0; i < num_draws; i++)
            vkCmdDrawIndexed(cmdbuf,
               draws[i].count, dinfo->instance_count,
               draws[i].start, draws[i].index_bias, dinfo->start_instance);
      }
   }
}

template <zink_multidraw HAS_MULTIDRAW>
ALWAYS_INLINE static void
draw(struct zink_context *ctx,
     const struct pipe_draw_info *dinfo,
     const struct pipe_draw_start_count_bias *draws,
     unsigned num_draws,
     unsigned draw_id,
     bool needs_drawid)
{
   VkCommandBuffer cmdbuf = ctx->batch.state->cmdbuf;
   if (dinfo->increment_draw_id && needs_drawid) {
      for (unsigned i = 0; i < num_draws; i++) {
         update_drawid(ctx, draw_id);
         vkCmdDraw(cmdbuf, draws[i].count, dinfo->instance_count, draws[i].start, dinfo->start_instance);
         draw_id++;
      }
   } else {
      if (needs_drawid)
         update_drawid(ctx, draw_id);
      if (HAS_MULTIDRAW)
         zink_screen(ctx->base.screen)->vk.CmdDrawMultiEXT(cmdbuf, num_draws, (const VkMultiDrawInfoEXT*)draws,
                                                            dinfo->instance_count, dinfo->start_instance,
                                                            sizeof(struct pipe_draw_start_count_bias));
      else {
         for (unsigned i = 0; i < num_draws; i++)
            vkCmdDraw(cmdbuf, draws[i].count, dinfo->instance_count, draws[i].start, dinfo->start_instance);

      }
   }
}

static void
update_barriers(struct zink_context *ctx, bool is_compute)
{
   if (!ctx->need_barriers[is_compute]->entries)
      return;
   struct set *need_barriers = ctx->need_barriers[is_compute];
   ctx->barrier_set_idx[is_compute] = !ctx->barrier_set_idx[is_compute];
   ctx->need_barriers[is_compute] = &ctx->update_barriers[is_compute][ctx->barrier_set_idx[is_compute]];
   set_foreach(need_barriers, he) {
      struct zink_resource *res = (struct zink_resource *)he->key;
      VkPipelineStageFlags pipeline = 0;
      VkAccessFlags access = 0;
      if (res->bind_count[is_compute]) {
         if (res->write_bind_count[is_compute])
            access |= VK_ACCESS_SHADER_WRITE_BIT;
         if (res->write_bind_count[is_compute] != res->bind_count[is_compute]) {
            unsigned bind_count = res->bind_count[is_compute] - res->write_bind_count[is_compute];
            if (res->obj->is_buffer) {
               if (res->ubo_bind_count[is_compute]) {
                  access |= VK_ACCESS_UNIFORM_READ_BIT;
                  bind_count -= res->ubo_bind_count[is_compute];
               }
               if (!is_compute && res->vbo_bind_mask) {
                  access |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
                  pipeline |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
                  bind_count -= util_bitcount(res->vbo_bind_mask);
               }
            }
            if (bind_count)
               access |= VK_ACCESS_SHADER_READ_BIT;
         }
         if (is_compute)
            pipeline = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
         else {
            u_foreach_bit(stage, res->bind_history) {
               if ((1 << stage) != ZINK_RESOURCE_USAGE_STREAMOUT)
                  pipeline |= zink_pipeline_flags_from_pipe_stage((enum pipe_shader_type)stage);
            }
         }
         if (res->base.b.target == PIPE_BUFFER)
            zink_resource_buffer_barrier(ctx, NULL, res, access, pipeline);
         else {
            VkImageLayout layout = zink_descriptor_util_image_layout_eval(res, is_compute);
            if (layout != res->layout)
               zink_resource_image_barrier(ctx, NULL, res, layout, access, pipeline);
         }
         /* always barrier on draw if this resource has either multiple image write binds or
          * image write binds and image read binds
          */
         if (res->write_bind_count[is_compute] && res->bind_count[is_compute] > 1)
            _mesa_set_add_pre_hashed(ctx->need_barriers[is_compute], he->hash, res);
      }
      _mesa_set_remove(need_barriers, he);
      if (!need_barriers->entries)
         break;
   }
}

template <zink_multidraw HAS_MULTIDRAW, zink_dynamic_state HAS_DYNAMIC_STATE, bool BATCH_CHANGED>
void
zink_draw_vbo(struct pipe_context *pctx,
              const struct pipe_draw_info *dinfo,
              unsigned drawid_offset,
              const struct pipe_draw_indirect_info *dindirect,
              const struct pipe_draw_start_count_bias *draws,
              unsigned num_draws)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_rasterizer_state *rast_state = ctx->rast_state;
   struct zink_depth_stencil_alpha_state *dsa_state = ctx->dsa_state;
   struct zink_batch *batch = &ctx->batch;
   struct zink_so_target *so_target =
      dindirect && dindirect->count_from_stream_output ?
         zink_so_target(dindirect->count_from_stream_output) : NULL;
   VkBuffer counter_buffers[PIPE_MAX_SO_OUTPUTS];
   VkDeviceSize counter_buffer_offsets[PIPE_MAX_SO_OUTPUTS];
   bool need_index_buffer_unref = false;
   bool mode_changed = ctx->gfx_pipeline_state.mode != dinfo->mode;
   bool reads_drawid = ctx->shader_reads_drawid;
   bool reads_basevertex = ctx->shader_reads_basevertex;
   unsigned work_count = ctx->batch.work_count;
   enum pipe_prim_type mode = dinfo->mode;

   update_barriers(ctx, false);

   if (ctx->gfx_pipeline_state.vertices_per_patch != dinfo->vertices_per_patch)
      ctx->gfx_pipeline_state.dirty = true;
   bool drawid_broken = ctx->drawid_broken;
   ctx->drawid_broken = false;
   if (reads_drawid && (!dindirect || !dindirect->buffer))
      ctx->drawid_broken = (drawid_offset != 0 ||
                           (!HAS_MULTIDRAW && num_draws > 1) ||
                           (HAS_MULTIDRAW && num_draws > 1 && !dinfo->increment_draw_id));
   if (drawid_broken != ctx->drawid_broken)
      ctx->dirty_shader_stages |= BITFIELD_BIT(PIPE_SHADER_VERTEX);
   ctx->gfx_pipeline_state.vertices_per_patch = dinfo->vertices_per_patch;
   if (ctx->rast_state->base.point_quad_rasterization &&
       ctx->gfx_prim_mode != mode) {
      if (ctx->gfx_prim_mode == PIPE_PRIM_POINTS || mode == PIPE_PRIM_POINTS)
         ctx->dirty_shader_stages |= BITFIELD_BIT(PIPE_SHADER_FRAGMENT);
   }
   ctx->gfx_prim_mode = mode;
   update_gfx_program(ctx);

   if (ctx->gfx_pipeline_state.primitive_restart != dinfo->primitive_restart)
      ctx->gfx_pipeline_state.dirty = true;
   ctx->gfx_pipeline_state.primitive_restart = dinfo->primitive_restart;

   unsigned index_offset = 0;
   unsigned index_size = dinfo->index_size;
   struct pipe_resource *index_buffer = NULL;
   if (index_size > 0) {
      if (dinfo->has_user_indices) {
         if (!util_upload_index_buffer(pctx, dinfo, &draws[0], &index_buffer, &index_offset, 4)) {
            debug_printf("util_upload_index_buffer() failed\n");
            return;
         }
         zink_batch_reference_resource_move(batch, zink_resource(index_buffer));
      } else {
         index_buffer = dinfo->index.resource;
         zink_batch_reference_resource_rw(batch, zink_resource(index_buffer), false);
      }
      assert(index_size <= 4 && index_size != 3);
      assert(index_size != 1 || screen->info.have_EXT_index_type_uint8);
      const VkIndexType index_type[3] = {
         VK_INDEX_TYPE_UINT8_EXT,
         VK_INDEX_TYPE_UINT16,
         VK_INDEX_TYPE_UINT32,
      };
      struct zink_resource *res = zink_resource(index_buffer);
      vkCmdBindIndexBuffer(batch->state->cmdbuf, res->obj->buffer, index_offset, index_type[index_size >> 1]);
   }

   if (zink_program_has_descriptors(&ctx->curr_program->base))
      screen->descriptors_update(ctx, false);

   bool have_streamout = !!ctx->num_so_targets;
   if (have_streamout) {
      if (ctx->xfb_barrier)
         zink_emit_xfb_counter_barrier(ctx);
      if (ctx->dirty_so_targets)
         zink_emit_stream_output_targets(pctx);
   }

   if (so_target)
      zink_emit_xfb_vertex_input_barrier(ctx, zink_resource(so_target->base.buffer));

   barrier_draw_buffers(ctx, dinfo, dindirect, index_buffer);

   if (BATCH_CHANGED)
      zink_update_descriptor_refs(ctx, false);

   batch = zink_batch_rp(ctx);

   VkPipeline prev_pipeline = ctx->gfx_pipeline_state.pipeline;
   VkPipeline pipeline = zink_get_gfx_pipeline(ctx, ctx->curr_program,
                                               &ctx->gfx_pipeline_state,
                                               mode);
   bool pipeline_changed = prev_pipeline != pipeline;
   if (BATCH_CHANGED || pipeline_changed)
      vkCmdBindPipeline(batch->state->cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

   if (BATCH_CHANGED || ctx->vp_state_changed || (!HAS_DYNAMIC_STATE && pipeline_changed)) {
      VkViewport viewports[PIPE_MAX_VIEWPORTS];
      for (unsigned i = 0; i < ctx->vp_state.num_viewports; i++) {
         VkViewport viewport = {
            ctx->vp_state.viewport_states[i].translate[0] - ctx->vp_state.viewport_states[i].scale[0],
            ctx->vp_state.viewport_states[i].translate[1] - ctx->vp_state.viewport_states[i].scale[1],
            ctx->vp_state.viewport_states[i].scale[0] * 2,
            ctx->vp_state.viewport_states[i].scale[1] * 2,
            ctx->rast_state->base.clip_halfz ?
               ctx->vp_state.viewport_states[i].translate[2] :
               ctx->vp_state.viewport_states[i].translate[2] - ctx->vp_state.viewport_states[i].scale[2],
            ctx->vp_state.viewport_states[i].translate[2] + ctx->vp_state.viewport_states[i].scale[2]
         };
         viewports[i] = viewport;
      }
      if (HAS_DYNAMIC_STATE)
         screen->vk.CmdSetViewportWithCountEXT(batch->state->cmdbuf, ctx->vp_state.num_viewports, viewports);
      else
         vkCmdSetViewport(batch->state->cmdbuf, 0, ctx->vp_state.num_viewports, viewports);
   }
   if (BATCH_CHANGED || ctx->scissor_changed || ctx->vp_state_changed || (!HAS_DYNAMIC_STATE && pipeline_changed)) {
      VkRect2D scissors[PIPE_MAX_VIEWPORTS];
      if (ctx->rast_state->base.scissor) {
         for (unsigned i = 0; i < ctx->vp_state.num_viewports; i++) {
            scissors[i].offset.x = ctx->vp_state.scissor_states[i].minx;
            scissors[i].offset.y = ctx->vp_state.scissor_states[i].miny;
            scissors[i].extent.width = ctx->vp_state.scissor_states[i].maxx - ctx->vp_state.scissor_states[i].minx;
            scissors[i].extent.height = ctx->vp_state.scissor_states[i].maxy - ctx->vp_state.scissor_states[i].miny;
         }
      } else {
         for (unsigned i = 0; i < ctx->vp_state.num_viewports; i++) {
            scissors[i].offset.x = 0;
            scissors[i].offset.y = 0;
            scissors[i].extent.width = ctx->fb_state.width;
            scissors[i].extent.height = ctx->fb_state.height;
         }
      }
      if (HAS_DYNAMIC_STATE)
         screen->vk.CmdSetScissorWithCountEXT(batch->state->cmdbuf, ctx->vp_state.num_viewports, scissors);
      else
         vkCmdSetScissor(batch->state->cmdbuf, 0, ctx->vp_state.num_viewports, scissors);
   }
   ctx->vp_state_changed = false;
   ctx->scissor_changed = false;

   if (BATCH_CHANGED || ctx->stencil_ref_changed) {
      vkCmdSetStencilReference(batch->state->cmdbuf, VK_STENCIL_FACE_FRONT_BIT,
                               ctx->stencil_ref.ref_value[0]);
      vkCmdSetStencilReference(batch->state->cmdbuf, VK_STENCIL_FACE_BACK_BIT,
                               ctx->stencil_ref.ref_value[1]);
      ctx->stencil_ref_changed = false;
   }

   if (HAS_DYNAMIC_STATE && (BATCH_CHANGED || ctx->dsa_state_changed)) {
      screen->vk.CmdSetDepthBoundsTestEnableEXT(batch->state->cmdbuf, dsa_state->hw_state.depth_bounds_test);
      if (dsa_state->hw_state.depth_bounds_test)
         vkCmdSetDepthBounds(batch->state->cmdbuf,
                             dsa_state->hw_state.min_depth_bounds,
                             dsa_state->hw_state.max_depth_bounds);
      screen->vk.CmdSetDepthTestEnableEXT(batch->state->cmdbuf, dsa_state->hw_state.depth_test);
      if (dsa_state->hw_state.depth_test)
         screen->vk.CmdSetDepthCompareOpEXT(batch->state->cmdbuf, dsa_state->hw_state.depth_compare_op);
      screen->vk.CmdSetDepthWriteEnableEXT(batch->state->cmdbuf, dsa_state->hw_state.depth_write);
      screen->vk.CmdSetStencilTestEnableEXT(batch->state->cmdbuf, dsa_state->hw_state.stencil_test);
      if (dsa_state->hw_state.stencil_test) {
         screen->vk.CmdSetStencilOpEXT(batch->state->cmdbuf, VK_STENCIL_FACE_FRONT_BIT,
                                       dsa_state->hw_state.stencil_front.failOp,
                                       dsa_state->hw_state.stencil_front.passOp,
                                       dsa_state->hw_state.stencil_front.depthFailOp,
                                       dsa_state->hw_state.stencil_front.compareOp);
         screen->vk.CmdSetStencilOpEXT(batch->state->cmdbuf, VK_STENCIL_FACE_BACK_BIT,
                                       dsa_state->hw_state.stencil_back.failOp,
                                       dsa_state->hw_state.stencil_back.passOp,
                                       dsa_state->hw_state.stencil_back.depthFailOp,
                                       dsa_state->hw_state.stencil_back.compareOp);
      }
      if (dsa_state->base.stencil[0].enabled) {
         if (dsa_state->base.stencil[1].enabled) {
            vkCmdSetStencilWriteMask(batch->state->cmdbuf, VK_STENCIL_FACE_FRONT_BIT, dsa_state->hw_state.stencil_front.writeMask);
            vkCmdSetStencilWriteMask(batch->state->cmdbuf, VK_STENCIL_FACE_BACK_BIT, dsa_state->hw_state.stencil_back.writeMask);
            vkCmdSetStencilCompareMask(batch->state->cmdbuf, VK_STENCIL_FACE_FRONT_BIT, dsa_state->hw_state.stencil_front.compareMask);
            vkCmdSetStencilCompareMask(batch->state->cmdbuf, VK_STENCIL_FACE_BACK_BIT, dsa_state->hw_state.stencil_back.compareMask);
         } else {
            vkCmdSetStencilWriteMask(batch->state->cmdbuf, VK_STENCIL_FACE_FRONT_AND_BACK, dsa_state->hw_state.stencil_front.writeMask);
            vkCmdSetStencilCompareMask(batch->state->cmdbuf, VK_STENCIL_FACE_FRONT_AND_BACK, dsa_state->hw_state.stencil_front.compareMask);
         }
      }
   }
   ctx->dsa_state_changed = false;

   bool rast_state_changed = ctx->rast_state_changed;
   if (HAS_DYNAMIC_STATE && (BATCH_CHANGED || rast_state_changed))
      screen->vk.CmdSetFrontFaceEXT(batch->state->cmdbuf, ctx->gfx_pipeline_state.front_face);
   if ((BATCH_CHANGED || rast_state_changed) &&
       screen->info.have_EXT_line_rasterization && rast_state->base.line_stipple_enable)
      screen->vk.CmdSetLineStippleEXT(batch->state->cmdbuf, rast_state->base.line_stipple_factor, rast_state->base.line_stipple_pattern);

   if (BATCH_CHANGED || ctx->rast_state_changed || mode_changed) {
      enum pipe_prim_type reduced_prim = u_reduced_prim(mode);

      bool depth_bias = false;
      switch (reduced_prim) {
      case PIPE_PRIM_POINTS:
         depth_bias = rast_state->offset_point;
         break;

      case PIPE_PRIM_LINES:
         depth_bias = rast_state->offset_line;
         break;

      case PIPE_PRIM_TRIANGLES:
         depth_bias = rast_state->offset_tri;
         break;

      default:
         unreachable("unexpected reduced prim");
      }

      if (line_width_needed(reduced_prim, rast_state->hw_state.polygon_mode)) {
         if (screen->info.feats.features.wideLines || rast_state->line_width == 1.0f)
            vkCmdSetLineWidth(batch->state->cmdbuf, rast_state->line_width);
         else
            debug_printf("BUG: wide lines not supported, needs fallback!");
      }
      if (depth_bias)
         vkCmdSetDepthBias(batch->state->cmdbuf, rast_state->offset_units, rast_state->offset_clamp, rast_state->offset_scale);
      else
         vkCmdSetDepthBias(batch->state->cmdbuf, 0.0f, 0.0f, 0.0f);
   }
   ctx->rast_state_changed = false;

   if (HAS_DYNAMIC_STATE) {
      if (ctx->sample_locations_changed) {
         VkSampleLocationsInfoEXT loc;
         zink_init_vk_sample_locations(ctx, &loc);
         screen->vk.CmdSetSampleLocationsEXT(batch->state->cmdbuf, &loc);
      }
      ctx->sample_locations_changed = false;
   }

   if ((BATCH_CHANGED || ctx->blend_state_changed) &&
       ctx->gfx_pipeline_state.blend_state->need_blend_constants) {
      vkCmdSetBlendConstants(batch->state->cmdbuf, ctx->blend_constants);
   }
   ctx->blend_state_changed = false;

   if (BATCH_CHANGED || ctx->vertex_buffers_dirty)
      zink_bind_vertex_buffers<HAS_DYNAMIC_STATE>(batch, ctx);

   if (reads_basevertex) {
      unsigned draw_mode_is_indexed = index_size > 0;
      vkCmdPushConstants(batch->state->cmdbuf, ctx->curr_program->base.layout, VK_SHADER_STAGE_VERTEX_BIT,
                         offsetof(struct zink_gfx_push_constant, draw_mode_is_indexed), sizeof(unsigned),
                         &draw_mode_is_indexed);
   }
   if (ctx->curr_program->shaders[PIPE_SHADER_TESS_CTRL] && ctx->curr_program->shaders[PIPE_SHADER_TESS_CTRL]->is_generated)
      vkCmdPushConstants(batch->state->cmdbuf, ctx->curr_program->base.layout, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                         offsetof(struct zink_gfx_push_constant, default_inner_level), sizeof(float) * 6,
                         &ctx->tess_levels[0]);

   zink_query_update_gs_states(ctx);

   if (have_streamout) {
      for (unsigned i = 0; i < ctx->num_so_targets; i++) {
         struct zink_so_target *t = zink_so_target(ctx->so_targets[i]);
         counter_buffers[i] = VK_NULL_HANDLE;
         if (t) {
            struct zink_resource *res = zink_resource(t->counter_buffer);
            t->stride = ctx->last_vertex_stage->streamout.so_info.stride[i] * sizeof(uint32_t);
            zink_batch_reference_resource_rw(batch, res, true);
            if (t->counter_buffer_valid) {
               counter_buffers[i] = res->obj->buffer;
               counter_buffer_offsets[i] = t->counter_buffer_offset;
            }
         }
      }
      screen->vk.CmdBeginTransformFeedbackEXT(batch->state->cmdbuf, 0, ctx->num_so_targets, counter_buffers, counter_buffer_offsets);
   }

   if (BATCH_CHANGED) {
      ctx->pipeline_changed[0] = false;
      zink_select_draw_vbo(ctx);
   }

   bool needs_drawid = reads_drawid && ctx->drawid_broken;
   work_count += num_draws;
   if (index_size > 0) {
      if (dindirect && dindirect->buffer) {
         assert(num_draws == 1);
         if (needs_drawid)
            update_drawid(ctx, drawid_offset);
         struct zink_resource *indirect = zink_resource(dindirect->buffer);
         zink_batch_reference_resource_rw(batch, indirect, false);
         if (dindirect->indirect_draw_count) {
             struct zink_resource *indirect_draw_count = zink_resource(dindirect->indirect_draw_count);
             zink_batch_reference_resource_rw(batch, indirect_draw_count, false);
             screen->vk.CmdDrawIndexedIndirectCount(batch->state->cmdbuf, indirect->obj->buffer, dindirect->offset,
                                           indirect_draw_count->obj->buffer, dindirect->indirect_draw_count_offset,
                                           dindirect->draw_count, dindirect->stride);
         } else
            vkCmdDrawIndexedIndirect(batch->state->cmdbuf, indirect->obj->buffer, dindirect->offset, dindirect->draw_count, dindirect->stride);
      } else {
         if (need_index_buffer_unref)
            draw_indexed_need_index_buffer_unref(ctx, dinfo, draws, num_draws, drawid_offset, needs_drawid);
         else
            draw_indexed<HAS_MULTIDRAW>(ctx, dinfo, draws, num_draws, drawid_offset, needs_drawid);
      }
   } else {
      if (so_target && screen->info.tf_props.transformFeedbackDraw) {
         if (needs_drawid)
            update_drawid(ctx, drawid_offset);
         zink_batch_reference_resource_rw(batch, zink_resource(so_target->base.buffer), false);
         zink_batch_reference_resource_rw(batch, zink_resource(so_target->counter_buffer), true);
         screen->vk.CmdDrawIndirectByteCountEXT(batch->state->cmdbuf, dinfo->instance_count, dinfo->start_instance,
                                       zink_resource(so_target->counter_buffer)->obj->buffer, so_target->counter_buffer_offset, 0,
                                       MIN2(so_target->stride, screen->info.tf_props.maxTransformFeedbackBufferDataStride));
      } else if (dindirect && dindirect->buffer) {
         assert(num_draws == 1);
         if (needs_drawid)
            update_drawid(ctx, drawid_offset);
         struct zink_resource *indirect = zink_resource(dindirect->buffer);
         zink_batch_reference_resource_rw(batch, indirect, false);
         if (dindirect->indirect_draw_count) {
             struct zink_resource *indirect_draw_count = zink_resource(dindirect->indirect_draw_count);
             zink_batch_reference_resource_rw(batch, indirect_draw_count, false);
             screen->vk.CmdDrawIndirectCount(batch->state->cmdbuf, indirect->obj->buffer, dindirect->offset,
                                           indirect_draw_count->obj->buffer, dindirect->indirect_draw_count_offset,
                                           dindirect->draw_count, dindirect->stride);
         } else
            vkCmdDrawIndirect(batch->state->cmdbuf, indirect->obj->buffer, dindirect->offset, dindirect->draw_count, dindirect->stride);
      } else {
         draw<HAS_MULTIDRAW>(ctx, dinfo, draws, num_draws, drawid_offset, needs_drawid);
      }
   }

   if (have_streamout) {
      for (unsigned i = 0; i < ctx->num_so_targets; i++) {
         struct zink_so_target *t = zink_so_target(ctx->so_targets[i]);
         if (t) {
            counter_buffers[i] = zink_resource(t->counter_buffer)->obj->buffer;
            counter_buffer_offsets[i] = t->counter_buffer_offset;
            t->counter_buffer_valid = true;
         }
      }
      screen->vk.CmdEndTransformFeedbackEXT(batch->state->cmdbuf, 0, ctx->num_so_targets, counter_buffers, counter_buffer_offsets);
   }
   batch->has_work = true;
   ctx->batch.work_count = work_count;
   /* flush if there's >100k draws */
   if (unlikely(work_count >= 30000) || ctx->oom_flush)
      pctx->flush(pctx, NULL, 0);
}

template <bool BATCH_CHANGED>
static void
zink_launch_grid(struct pipe_context *pctx, const struct pipe_grid_info *info)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_batch *batch = &ctx->batch;

   update_barriers(ctx, true);

   update_compute_program(ctx);

   if (zink_program_has_descriptors(&ctx->curr_compute->base))
      screen->descriptors_update(ctx, true);

   zink_program_update_compute_pipeline_state(ctx, ctx->curr_compute, info->block);
   VkPipeline prev_pipeline = ctx->compute_pipeline_state.pipeline;
   VkPipeline pipeline = zink_get_compute_pipeline(screen, ctx->curr_compute,
                                               &ctx->compute_pipeline_state);

   if (BATCH_CHANGED) {
      zink_update_descriptor_refs(ctx, true);
      zink_batch_reference_program(&ctx->batch, &ctx->curr_compute->base);
   }

   if (prev_pipeline != pipeline || BATCH_CHANGED)
      vkCmdBindPipeline(batch->state->cmdbuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
   if (BATCH_CHANGED) {
      ctx->pipeline_changed[1] = false;
      zink_select_launch_grid(ctx);
   }

   if (BITSET_TEST(ctx->compute_stage->nir->info.system_values_read, SYSTEM_VALUE_WORK_DIM))
      vkCmdPushConstants(batch->state->cmdbuf, ctx->curr_compute->base.layout, VK_SHADER_STAGE_COMPUTE_BIT,
                         offsetof(struct zink_cs_push_constant, work_dim), sizeof(uint32_t),
                         &info->work_dim);

   batch->work_count++;
   if (info->indirect) {
      vkCmdDispatchIndirect(batch->state->cmdbuf, zink_resource(info->indirect)->obj->buffer, info->indirect_offset);
      zink_batch_reference_resource_rw(batch, zink_resource(info->indirect), false);
   } else
      vkCmdDispatch(batch->state->cmdbuf, info->grid[0], info->grid[1], info->grid[2]);
   batch->has_work = true;
   /* flush if there's >100k computes */
   if (unlikely(ctx->batch.work_count >= 30000) || ctx->oom_flush)
      pctx->flush(pctx, NULL, 0);
}

template <zink_multidraw HAS_MULTIDRAW, zink_dynamic_state HAS_DYNAMIC_STATE, bool BATCH_CHANGED>
static void
init_batch_changed_functions(struct zink_context *ctx, pipe_draw_vbo_func draw_vbo_array[2][2][2])
{
   draw_vbo_array[HAS_MULTIDRAW][HAS_DYNAMIC_STATE][BATCH_CHANGED] =
   zink_draw_vbo<HAS_MULTIDRAW, HAS_DYNAMIC_STATE, BATCH_CHANGED>;
}

template <zink_multidraw HAS_MULTIDRAW, zink_dynamic_state HAS_DYNAMIC_STATE>
static void
init_dynamic_state_functions(struct zink_context *ctx, pipe_draw_vbo_func draw_vbo_array[2][2][2])
{
   init_batch_changed_functions<HAS_MULTIDRAW, HAS_DYNAMIC_STATE, false>(ctx, draw_vbo_array);
   init_batch_changed_functions<HAS_MULTIDRAW, HAS_DYNAMIC_STATE, true>(ctx, draw_vbo_array);
}

template <zink_multidraw HAS_MULTIDRAW>
static void
init_multidraw_functions(struct zink_context *ctx, pipe_draw_vbo_func draw_vbo_array[2][2][2])
{
   init_dynamic_state_functions<HAS_MULTIDRAW, ZINK_NO_DYNAMIC_STATE>(ctx, draw_vbo_array);
   init_dynamic_state_functions<HAS_MULTIDRAW, ZINK_DYNAMIC_STATE>(ctx, draw_vbo_array);
}

static void
init_all_draw_functions(struct zink_context *ctx, pipe_draw_vbo_func draw_vbo_array[2][2][2])
{
   init_multidraw_functions<ZINK_NO_MULTIDRAW>(ctx, draw_vbo_array);
   init_multidraw_functions<ZINK_MULTIDRAW>(ctx, draw_vbo_array);
}

template <bool BATCH_CHANGED>
static void
init_grid_batch_changed_functions(struct zink_context *ctx)
{
   ctx->launch_grid[BATCH_CHANGED] = zink_launch_grid<BATCH_CHANGED>;
}

static void
init_all_grid_functions(struct zink_context *ctx)
{
   init_grid_batch_changed_functions<false>(ctx);
   init_grid_batch_changed_functions<true>(ctx);
}

static void
zink_invalid_draw_vbo(struct pipe_context *pipe,
                      const struct pipe_draw_info *dinfo,
                      unsigned drawid_offset,
                      const struct pipe_draw_indirect_info *dindirect,
                      const struct pipe_draw_start_count_bias *draws,
                      unsigned num_draws)
{
   unreachable("vertex shader not bound");
}

static void
zink_invalid_launch_grid(struct pipe_context *pctx, const struct pipe_grid_info *info)
{
   unreachable("compute shader not bound");
}

extern "C"
void
zink_init_draw_functions(struct zink_context *ctx, struct zink_screen *screen)
{
   pipe_draw_vbo_func draw_vbo_array[2][2][2]; //multidraw, dynamic state, batch changed
   init_all_draw_functions(ctx, draw_vbo_array);
   memcpy(ctx->draw_vbo, &draw_vbo_array[screen->info.have_EXT_multi_draw]
                                        [screen->info.have_EXT_extended_dynamic_state],
                                        sizeof(ctx->draw_vbo));

   /* Bind a fake draw_vbo, so that draw_vbo isn't NULL, which would skip
    * initialization of callbacks in upper layers (such as u_threaded_context).
    */
   ctx->base.draw_vbo = zink_invalid_draw_vbo;
}

void
zink_init_grid_functions(struct zink_context *ctx)
{
   init_all_grid_functions(ctx);
   /* Bind a fake launch_grid, so that draw_vbo isn't NULL, which would skip
    * initialization of callbacks in upper layers (such as u_threaded_context).
    */
   ctx->base.launch_grid = zink_invalid_launch_grid;
}
