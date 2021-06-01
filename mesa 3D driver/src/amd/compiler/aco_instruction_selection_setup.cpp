/*
 * Copyright © 2018 Valve Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "aco_instruction_selection.h"
#include "vulkan/radv_descriptor_set.h"
#include "vulkan/radv_shader.h"
#include "nir_control_flow.h"
#include "sid.h"
#include "ac_exp_param.h"

namespace aco {

namespace {

unsigned get_interp_input(nir_intrinsic_op intrin, enum glsl_interp_mode interp)
{
   switch (interp) {
   case INTERP_MODE_SMOOTH:
   case INTERP_MODE_NONE:
      if (intrin == nir_intrinsic_load_barycentric_pixel ||
          intrin == nir_intrinsic_load_barycentric_at_sample ||
          intrin == nir_intrinsic_load_barycentric_at_offset)
         return S_0286CC_PERSP_CENTER_ENA(1);
      else if (intrin == nir_intrinsic_load_barycentric_centroid)
         return S_0286CC_PERSP_CENTROID_ENA(1);
      else if (intrin == nir_intrinsic_load_barycentric_sample)
         return S_0286CC_PERSP_SAMPLE_ENA(1);
      break;
   case INTERP_MODE_NOPERSPECTIVE:
      if (intrin == nir_intrinsic_load_barycentric_pixel)
         return S_0286CC_LINEAR_CENTER_ENA(1);
      else if (intrin == nir_intrinsic_load_barycentric_centroid)
         return S_0286CC_LINEAR_CENTROID_ENA(1);
      else if (intrin == nir_intrinsic_load_barycentric_sample)
         return S_0286CC_LINEAR_SAMPLE_ENA(1);
      break;
   default:
      break;
   }
   return 0;
}

/* If one side of a divergent IF ends in a branch and the other doesn't, we
 * might have to emit the contents of the side without the branch at the merge
 * block instead. This is so that we can use any SGPR live-out of the side
 * without the branch without creating a linear phi in the invert or merge block. */
bool
sanitize_if(nir_function_impl *impl, nir_if *nif)
{
   //TODO: skip this if the condition is uniform and there are no divergent breaks/continues?

   nir_block *then_block = nir_if_last_then_block(nif);
   nir_block *else_block = nir_if_last_else_block(nif);
   bool then_jump = nir_block_ends_in_jump(then_block) || nir_block_is_unreachable(then_block);
   bool else_jump = nir_block_ends_in_jump(else_block) || nir_block_is_unreachable(else_block);
   if (then_jump == else_jump)
      return false;

   /* If the continue from block is empty then return as there is nothing to
    * move.
    */
   if (nir_cf_list_is_empty_block(else_jump ? &nif->then_list : &nif->else_list))
      return false;

   /* Even though this if statement has a jump on one side, we may still have
    * phis afterwards.  Single-source phis can be produced by loop unrolling
    * or dead control-flow passes and are perfectly legal.  Run a quick phi
    * removal on the block after the if to clean up any such phis.
    */
   nir_opt_remove_phis_block(nir_cf_node_as_block(nir_cf_node_next(&nif->cf_node)));

   /* Finally, move the continue from branch after the if-statement. */
   nir_block *last_continue_from_blk = else_jump ? then_block : else_block;
   nir_block *first_continue_from_blk = else_jump ?
      nir_if_first_then_block(nif) : nir_if_first_else_block(nif);

   nir_cf_list tmp;
   nir_cf_extract(&tmp, nir_before_block(first_continue_from_blk),
                        nir_after_block(last_continue_from_blk));
   nir_cf_reinsert(&tmp, nir_after_cf_node(&nif->cf_node));

   /* nir_cf_extract() invalidates dominance metadata, but it should still be
    * correct because of the specific type of transformation we did. Block
    * indices are not valid except for block_0's, which is all we care about for
    * nir_block_is_unreachable(). */
   impl->valid_metadata = impl->valid_metadata | nir_metadata_dominance | nir_metadata_block_index;

   return true;
}

bool
sanitize_cf_list(nir_function_impl *impl, struct exec_list *cf_list)
{
   bool progress = false;
   foreach_list_typed(nir_cf_node, cf_node, node, cf_list) {
      switch (cf_node->type) {
      case nir_cf_node_block:
         break;
      case nir_cf_node_if: {
         nir_if *nif = nir_cf_node_as_if(cf_node);
         progress |= sanitize_cf_list(impl, &nif->then_list);
         progress |= sanitize_cf_list(impl, &nif->else_list);
         progress |= sanitize_if(impl, nif);
         break;
      }
      case nir_cf_node_loop: {
         nir_loop *loop = nir_cf_node_as_loop(cf_node);
         progress |= sanitize_cf_list(impl, &loop->body);
         break;
      }
      case nir_cf_node_function:
         unreachable("Invalid cf type");
      }
   }

   return progress;
}

void fill_desc_set_info(isel_context *ctx, nir_function_impl *impl)
{
   radv_pipeline_layout *pipeline_layout = ctx->options->layout;

   unsigned resource_flag_count = 1; /* +1 to reserve flags[0] for aliased resources */
   for (unsigned i = 0; i < pipeline_layout->num_sets; i++) {
      radv_descriptor_set_layout *layout = pipeline_layout->set[i].layout;
      ctx->resource_flag_offsets[i] = resource_flag_count;
      resource_flag_count += layout->binding_count;
   }
   ctx->buffer_resource_flags = std::vector<uint8_t>(resource_flag_count);

   nir_foreach_variable_with_modes(var, impl->function->shader, nir_var_mem_ssbo) {
      if (var->data.access & ACCESS_RESTRICT) {
         uint32_t offset = ctx->resource_flag_offsets[var->data.descriptor_set];
         ctx->buffer_resource_flags[offset + var->data.binding] |= buffer_is_restrict;
      }
   }

   nir_foreach_block(block, impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;
         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (!nir_intrinsic_has_access(intrin))
            continue;

         nir_ssa_def *res = NULL;
         unsigned access = nir_intrinsic_access(intrin);
         unsigned flags = 0;
         bool glc = access & (ACCESS_VOLATILE | ACCESS_COHERENT | ACCESS_NON_READABLE);
         switch (intrin->intrinsic) {
         case nir_intrinsic_load_ssbo: {
            if (nir_dest_is_divergent(intrin->dest) && (!glc || ctx->program->chip_class >= GFX8))
               flags |= glc ? has_glc_vmem_load : has_nonglc_vmem_load;
            res = intrin->src[0].ssa;
            break;
         }
         case nir_intrinsic_ssbo_atomic_add:
         case nir_intrinsic_ssbo_atomic_imin:
         case nir_intrinsic_ssbo_atomic_umin:
         case nir_intrinsic_ssbo_atomic_imax:
         case nir_intrinsic_ssbo_atomic_umax:
         case nir_intrinsic_ssbo_atomic_and:
         case nir_intrinsic_ssbo_atomic_or:
         case nir_intrinsic_ssbo_atomic_xor:
         case nir_intrinsic_ssbo_atomic_exchange:
         case nir_intrinsic_ssbo_atomic_comp_swap:
            flags |= has_glc_vmem_load | has_glc_vmem_store;
            res = intrin->src[0].ssa;
            break;
         case nir_intrinsic_store_ssbo:
            flags |= glc ? has_glc_vmem_store : has_nonglc_vmem_store;
            res = intrin->src[1].ssa;
            break;
         case nir_intrinsic_load_global:
            if (!(access & ACCESS_NON_WRITEABLE))
               flags |= glc ? has_glc_vmem_load : has_nonglc_vmem_load;
            break;
         case nir_intrinsic_store_global:
            flags |= glc ? has_glc_vmem_store : has_nonglc_vmem_store;
            break;
         case nir_intrinsic_global_atomic_add:
         case nir_intrinsic_global_atomic_imin:
         case nir_intrinsic_global_atomic_umin:
         case nir_intrinsic_global_atomic_imax:
         case nir_intrinsic_global_atomic_umax:
         case nir_intrinsic_global_atomic_and:
         case nir_intrinsic_global_atomic_or:
         case nir_intrinsic_global_atomic_xor:
         case nir_intrinsic_global_atomic_exchange:
         case nir_intrinsic_global_atomic_comp_swap:
            flags |= has_glc_vmem_load | has_glc_vmem_store;
            break;
         case nir_intrinsic_image_deref_load:
         case nir_intrinsic_image_deref_sparse_load:
            res = intrin->src[0].ssa;
            flags |= glc ? has_glc_vmem_load : has_nonglc_vmem_load;
            break;
         case nir_intrinsic_image_deref_store:
            res = intrin->src[0].ssa;
            flags |= (glc || ctx->program->chip_class == GFX6) ? has_glc_vmem_store : has_nonglc_vmem_store;
            break;
         case nir_intrinsic_image_deref_atomic_add:
         case nir_intrinsic_image_deref_atomic_umin:
         case nir_intrinsic_image_deref_atomic_imin:
         case nir_intrinsic_image_deref_atomic_umax:
         case nir_intrinsic_image_deref_atomic_imax:
         case nir_intrinsic_image_deref_atomic_and:
         case nir_intrinsic_image_deref_atomic_or:
         case nir_intrinsic_image_deref_atomic_xor:
         case nir_intrinsic_image_deref_atomic_exchange:
         case nir_intrinsic_image_deref_atomic_comp_swap:
            res = intrin->src[0].ssa;
            flags |= has_glc_vmem_load | has_glc_vmem_store;
            break;
         default:
            continue;
         }

         uint8_t *flags_ptr;
         uint32_t count;
         get_buffer_resource_flags(ctx, res, access, &flags_ptr, &count);

         for (unsigned i = 0; i < count; i++)
            flags_ptr[i] |= flags;
      }
   }
}

void apply_nuw_to_ssa(isel_context *ctx, nir_ssa_def *ssa)
{
   nir_ssa_scalar scalar;
   scalar.def = ssa;
   scalar.comp = 0;

   if (!nir_ssa_scalar_is_alu(scalar) || nir_ssa_scalar_alu_op(scalar) != nir_op_iadd)
      return;

   nir_alu_instr *add = nir_instr_as_alu(ssa->parent_instr);

   if (add->no_unsigned_wrap)
      return;

   nir_ssa_scalar src0 = nir_ssa_scalar_chase_alu_src(scalar, 0);
   nir_ssa_scalar src1 = nir_ssa_scalar_chase_alu_src(scalar, 1);

   if (nir_ssa_scalar_is_const(src0)) {
      nir_ssa_scalar tmp = src0;
      src0 = src1;
      src1 = tmp;
   }

   uint32_t src1_ub = nir_unsigned_upper_bound(ctx->shader, ctx->range_ht,
                                               src1, &ctx->ub_config);
   add->no_unsigned_wrap =
      !nir_addition_might_overflow(ctx->shader, ctx->range_ht, src0, src1_ub,
                                   &ctx->ub_config);
}

void apply_nuw_to_offsets(isel_context *ctx, nir_function_impl *impl)
{
   nir_metadata_require(impl, nir_metadata_dominance);

   nir_foreach_block(block, impl) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;
         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         switch (intrin->intrinsic) {
         case nir_intrinsic_load_constant:
         case nir_intrinsic_load_uniform:
         case nir_intrinsic_load_push_constant:
            if (!nir_src_is_divergent(intrin->src[0]))
               apply_nuw_to_ssa(ctx, intrin->src[0].ssa);
            break;
         case nir_intrinsic_load_ubo:
         case nir_intrinsic_load_ssbo:
            if (!nir_src_is_divergent(intrin->src[1]))
               apply_nuw_to_ssa(ctx, intrin->src[1].ssa);
            break;
         case nir_intrinsic_store_ssbo:
            if (!nir_src_is_divergent(intrin->src[2]))
               apply_nuw_to_ssa(ctx, intrin->src[2].ssa);
            break;
         default:
            break;
         }
      }
   }
}

RegClass get_reg_class(isel_context *ctx, RegType type, unsigned components, unsigned bitsize)
{
   if (bitsize == 1)
      return RegClass(RegType::sgpr, ctx->program->lane_mask.size() * components);
   else
      return RegClass::get(type, components * bitsize / 8u);
}

void
setup_vs_output_info(isel_context *ctx, nir_shader *nir,
                     bool export_prim_id, bool export_clip_dists,
                     radv_vs_output_info *outinfo)
{
   memset(outinfo->vs_output_param_offset, AC_EXP_PARAM_UNDEFINED,
          sizeof(outinfo->vs_output_param_offset));

   outinfo->param_exports = 0;
   int pos_written = 0x1;
   bool writes_primitive_shading_rate = outinfo->writes_primitive_shading_rate ||
                                        ctx->options->force_vrs_rates;
   if (outinfo->writes_pointsize || outinfo->writes_viewport_index || outinfo->writes_layer ||
       writes_primitive_shading_rate)
      pos_written |= 1 << 1;

   uint64_t mask = nir->info.outputs_written;
   while (mask) {
      int idx = u_bit_scan64(&mask);
      if (idx >= VARYING_SLOT_VAR0 || idx == VARYING_SLOT_LAYER ||
          idx == VARYING_SLOT_PRIMITIVE_ID || idx == VARYING_SLOT_VIEWPORT ||
          ((idx == VARYING_SLOT_CLIP_DIST0 || idx == VARYING_SLOT_CLIP_DIST1) && export_clip_dists)) {
         if (outinfo->vs_output_param_offset[idx] == AC_EXP_PARAM_UNDEFINED)
            outinfo->vs_output_param_offset[idx] = outinfo->param_exports++;
      }
   }
   if (outinfo->writes_layer &&
       outinfo->vs_output_param_offset[VARYING_SLOT_LAYER] == AC_EXP_PARAM_UNDEFINED) {
      /* when ctx->options->key.has_multiview_view_index = true, the layer
       * variable isn't declared in NIR and it's isel's job to get the layer */
      outinfo->vs_output_param_offset[VARYING_SLOT_LAYER] = outinfo->param_exports++;
   }

   if (export_prim_id) {
      assert(outinfo->vs_output_param_offset[VARYING_SLOT_PRIMITIVE_ID] == AC_EXP_PARAM_UNDEFINED);
      outinfo->vs_output_param_offset[VARYING_SLOT_PRIMITIVE_ID] = outinfo->param_exports++;
   }

   ctx->export_clip_dists = export_clip_dists;
   ctx->num_clip_distances = util_bitcount(outinfo->clip_dist_mask);
   ctx->num_cull_distances = util_bitcount(outinfo->cull_dist_mask);

   assert(ctx->num_clip_distances + ctx->num_cull_distances <= 8);

   if (ctx->num_clip_distances + ctx->num_cull_distances > 0)
      pos_written |= 1 << 2;
   if (ctx->num_clip_distances + ctx->num_cull_distances > 4)
      pos_written |= 1 << 3;

   outinfo->pos_exports = util_bitcount(pos_written);

   /* GFX10+ early rasterization:
    * When there are no param exports in an NGG (or legacy VS) shader,
    * RADV sets NO_PC_EXPORT=1, which means the HW will start clipping and rasterization
    * as soon as it encounters a DONE pos export. When this happens, PS waves can launch
    * before the NGG (or VS) waves finish.
    */
   ctx->program->early_rast = ctx->program->chip_class >= GFX10 &&
                              outinfo->param_exports == 0;
}

void
setup_vs_variables(isel_context *ctx, nir_shader *nir)
{
   if (ctx->stage == vertex_vs || ctx->stage == vertex_ngg) {
      radv_vs_output_info *outinfo = &ctx->program->info->vs.outinfo;
      setup_vs_output_info(ctx, nir, outinfo->export_prim_id,
                           ctx->options->key.vs_common_out.export_clip_dists, outinfo);

      /* TODO: NGG streamout */
      if (ctx->stage.hw == HWStage::NGG)
         assert(!ctx->args->shader_info->so.num_outputs);

      /* TODO: check if the shader writes edge flags (not in Vulkan) */
      ctx->ngg_nogs_early_prim_export = exec_list_is_singular(&nir_shader_get_entrypoint(nir)->body);
   }

   if (ctx->stage == vertex_ngg && ctx->args->options->key.vs_common_out.export_prim_id) {
      /* We need to store the primitive IDs in LDS */
      unsigned lds_size = ctx->program->info->ngg_info.esgs_ring_size;
      ctx->program->config->lds_size = DIV_ROUND_UP(lds_size, ctx->program->dev.lds_encoding_granule);
   }
}

void setup_gs_variables(isel_context *ctx, nir_shader *nir)
{
   if (ctx->stage == vertex_geometry_gs || ctx->stage == tess_eval_geometry_gs) {
      ctx->program->config->lds_size = ctx->program->info->gs_ring_info.lds_size; /* Already in units of the alloc granularity */
   } else if (ctx->stage == vertex_geometry_ngg || ctx->stage == tess_eval_geometry_ngg) {
      radv_vs_output_info *outinfo = &ctx->program->info->vs.outinfo;
      setup_vs_output_info(ctx, nir, false,
                           ctx->options->key.vs_common_out.export_clip_dists, outinfo);

      unsigned ngg_gs_scratch_bytes = ctx->args->shader_info->so.num_outputs ? (44u * 4u) : (8u * 4u);
      unsigned ngg_emit_bytes = ctx->args->shader_info->ngg_info.ngg_emit_size * 4u;
      unsigned esgs_ring_bytes = ctx->args->shader_info->ngg_info.esgs_ring_size;

      ctx->ngg_gs_primflags_offset = ctx->args->shader_info->gs.gsvs_vertex_size;
      ctx->ngg_gs_emit_vtx_bytes = ctx->ngg_gs_primflags_offset + 4u;
      ctx->ngg_gs_emit_addr = esgs_ring_bytes;
      ctx->ngg_gs_scratch_addr = ctx->ngg_gs_emit_addr + ngg_emit_bytes;
      ctx->ngg_gs_scratch_addr = ALIGN(ctx->ngg_gs_scratch_addr, 16u);

      unsigned total_lds_bytes = ctx->ngg_gs_scratch_addr + ngg_gs_scratch_bytes;
      assert(total_lds_bytes >= ctx->ngg_gs_emit_addr);
      assert(total_lds_bytes >= ctx->ngg_gs_scratch_addr);
      ctx->program->config->lds_size = DIV_ROUND_UP(total_lds_bytes, ctx->program->dev.lds_encoding_granule);

      /* Make sure we have enough room for emitted GS vertices */
      if (nir->info.gs.vertices_out)
         assert((ngg_emit_bytes % (ctx->ngg_gs_emit_vtx_bytes * nir->info.gs.vertices_out)) == 0);

      /* See if the number of vertices and primitives are compile-time known */
      nir_gs_count_vertices_and_primitives(nir, ctx->ngg_gs_const_vtxcnt, ctx->ngg_gs_const_prmcnt, 4u);
      ctx->ngg_gs_early_alloc = ctx->ngg_gs_const_vtxcnt[0] == nir->info.gs.vertices_out && ctx->ngg_gs_const_prmcnt[0] != -1;
   }

   if (ctx->stage.has(SWStage::VS))
      ctx->program->info->gs.es_type = MESA_SHADER_VERTEX;
   else if (ctx->stage.has(SWStage::TES))
      ctx->program->info->gs.es_type = MESA_SHADER_TESS_EVAL;
}

void
setup_tcs_info(isel_context *ctx, nir_shader *nir, nir_shader *vs)
{
   ctx->tcs_in_out_eq = ctx->args->shader_info->vs.tcs_in_out_eq;
   ctx->tcs_temp_only_inputs = ctx->args->shader_info->vs.tcs_temp_only_input_mask;
   ctx->tcs_num_patches = ctx->args->shader_info->num_tess_patches;
   ctx->program->config->lds_size = ctx->args->shader_info->tcs.num_lds_blocks;
}

void
setup_tes_variables(isel_context *ctx, nir_shader *nir)
{
   ctx->tcs_num_patches = ctx->args->shader_info->num_tess_patches;

   if (ctx->stage == tess_eval_vs || ctx->stage == tess_eval_ngg) {
      radv_vs_output_info *outinfo = &ctx->program->info->tes.outinfo;
      setup_vs_output_info(ctx, nir, outinfo->export_prim_id,
                           ctx->options->key.vs_common_out.export_clip_dists, outinfo);

      /* TODO: NGG streamout */
      if (ctx->stage.hw == HWStage::NGG)
         assert(!ctx->args->shader_info->so.num_outputs);

      ctx->ngg_nogs_early_prim_export = exec_list_is_singular(&nir_shader_get_entrypoint(nir)->body);
   }
}

void
setup_variables(isel_context *ctx, nir_shader *nir)
{
   switch (nir->info.stage) {
   case MESA_SHADER_FRAGMENT: {
      break;
   }
   case MESA_SHADER_COMPUTE: {
      ctx->program->config->lds_size = DIV_ROUND_UP(nir->info.shared_size, ctx->program->dev.lds_encoding_granule);
      break;
   }
   case MESA_SHADER_VERTEX: {
      setup_vs_variables(ctx, nir);
      break;
   }
   case MESA_SHADER_GEOMETRY: {
      setup_gs_variables(ctx, nir);
      break;
   }
   case MESA_SHADER_TESS_CTRL: {
      break;
   }
   case MESA_SHADER_TESS_EVAL: {
      setup_tes_variables(ctx, nir);
      break;
   }
   default:
      unreachable("Unhandled shader stage.");
   }

   /* Make sure we fit the available LDS space. */
   assert((ctx->program->config->lds_size * ctx->program->dev.lds_encoding_granule) <= ctx->program->dev.lds_limit);
}

void
setup_nir(isel_context *ctx, nir_shader *nir)
{
   /* the variable setup has to be done before lower_io / CSE */
   setup_variables(ctx, nir);

   nir_convert_to_lcssa(nir, true, false);
   nir_lower_phis_to_scalar(nir);

   nir_function_impl *func = nir_shader_get_entrypoint(nir);
   nir_index_ssa_defs(func);
}

} /* end namespace */

void init_context(isel_context *ctx, nir_shader *shader)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   unsigned lane_mask_size = ctx->program->lane_mask.size();

   ctx->shader = shader;

   /* Init NIR range analysis. */
   ctx->range_ht =_mesa_pointer_hash_table_create(NULL);
   ctx->ub_config.min_subgroup_size = 64;
   ctx->ub_config.max_subgroup_size = 64;
   if (ctx->shader->info.stage == MESA_SHADER_COMPUTE && ctx->options->key.cs.subgroup_size) {
      ctx->ub_config.min_subgroup_size = ctx->options->key.cs.subgroup_size;
      ctx->ub_config.max_subgroup_size = ctx->options->key.cs.subgroup_size;
   }
   ctx->ub_config.max_work_group_invocations = 2048;
   ctx->ub_config.max_work_group_count[0] = 65535;
   ctx->ub_config.max_work_group_count[1] = 65535;
   ctx->ub_config.max_work_group_count[2] = 65535;
   ctx->ub_config.max_work_group_size[0] = 2048;
   ctx->ub_config.max_work_group_size[1] = 2048;
   ctx->ub_config.max_work_group_size[2] = 2048;
   for (unsigned i = 0; i < MAX_VERTEX_ATTRIBS; i++) {
      unsigned attrib_format = ctx->options->key.vs.vertex_attribute_formats[i];
      unsigned dfmt = attrib_format & 0xf;
      unsigned nfmt = (attrib_format >> 4) & 0x7;

      uint32_t max = UINT32_MAX;
      if (nfmt == V_008F0C_BUF_NUM_FORMAT_UNORM) {
         max = 0x3f800000u;
      } else if (nfmt == V_008F0C_BUF_NUM_FORMAT_UINT ||
                 nfmt == V_008F0C_BUF_NUM_FORMAT_USCALED) {
         bool uscaled = nfmt == V_008F0C_BUF_NUM_FORMAT_USCALED;
         switch (dfmt) {
         case V_008F0C_BUF_DATA_FORMAT_8:
         case V_008F0C_BUF_DATA_FORMAT_8_8:
         case V_008F0C_BUF_DATA_FORMAT_8_8_8_8:
            max = uscaled ? 0x437f0000u : UINT8_MAX;
            break;
         case V_008F0C_BUF_DATA_FORMAT_10_10_10_2:
         case V_008F0C_BUF_DATA_FORMAT_2_10_10_10:
            max = uscaled ? 0x447fc000u : 1023;
            break;
         case V_008F0C_BUF_DATA_FORMAT_10_11_11:
         case V_008F0C_BUF_DATA_FORMAT_11_11_10:
            max = uscaled ? 0x44ffe000u : 2047;
            break;
         case V_008F0C_BUF_DATA_FORMAT_16:
         case V_008F0C_BUF_DATA_FORMAT_16_16:
         case V_008F0C_BUF_DATA_FORMAT_16_16_16_16:
            max = uscaled ? 0x477fff00u : UINT16_MAX;
            break;
         case V_008F0C_BUF_DATA_FORMAT_32:
         case V_008F0C_BUF_DATA_FORMAT_32_32:
         case V_008F0C_BUF_DATA_FORMAT_32_32_32:
         case V_008F0C_BUF_DATA_FORMAT_32_32_32_32:
            max = uscaled ? 0x4f800000u : UINT32_MAX;
            break;
         }
      }
      ctx->ub_config.vertex_attrib_max[i] = max;
   }

   nir_divergence_analysis(shader);
   nir_opt_uniform_atomics(shader);

   fill_desc_set_info(ctx, impl);

   apply_nuw_to_offsets(ctx, impl);

   /* sanitize control flow */
   nir_metadata_require(impl, nir_metadata_dominance);
   sanitize_cf_list(impl, &impl->body);
   nir_metadata_preserve(impl, ~nir_metadata_block_index);

   /* we'll need this for isel */
   nir_metadata_require(impl, nir_metadata_block_index);

   if (!ctx->stage.has(SWStage::GSCopy) && ctx->options->dump_preoptir) {
      fprintf(stderr, "NIR shader before instruction selection:\n");
      nir_print_shader(shader, stderr);
   }

   ctx->first_temp_id = ctx->program->peekAllocationId();
   ctx->program->allocateRange(impl->ssa_alloc);
   RegClass *regclasses = ctx->program->temp_rc.data() + ctx->first_temp_id;

   unsigned spi_ps_inputs = 0;

   std::unique_ptr<unsigned[]> nir_to_aco{new unsigned[impl->num_blocks]()};

   /* TODO: make this recursive to improve compile times and merge with fill_desc_set_info() */
   bool done = false;
   while (!done) {
      done = true;
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            switch(instr->type) {
            case nir_instr_type_alu: {
               nir_alu_instr *alu_instr = nir_instr_as_alu(instr);
               RegType type = nir_dest_is_divergent(alu_instr->dest.dest) ? RegType::vgpr : RegType::sgpr;
               switch(alu_instr->op) {
                  case nir_op_fmul:
                  case nir_op_fadd:
                  case nir_op_fsub:
                  case nir_op_fmax:
                  case nir_op_fmin:
                  case nir_op_fneg:
                  case nir_op_fabs:
                  case nir_op_fsat:
                  case nir_op_fsign:
                  case nir_op_frcp:
                  case nir_op_frsq:
                  case nir_op_fsqrt:
                  case nir_op_fexp2:
                  case nir_op_flog2:
                  case nir_op_ffract:
                  case nir_op_ffloor:
                  case nir_op_fceil:
                  case nir_op_ftrunc:
                  case nir_op_fround_even:
                  case nir_op_fsin:
                  case nir_op_fcos:
                  case nir_op_f2f16:
                  case nir_op_f2f16_rtz:
                  case nir_op_f2f16_rtne:
                  case nir_op_f2f32:
                  case nir_op_f2f64:
                  case nir_op_u2f16:
                  case nir_op_u2f32:
                  case nir_op_u2f64:
                  case nir_op_i2f16:
                  case nir_op_i2f32:
                  case nir_op_i2f64:
                  case nir_op_pack_half_2x16_split:
                  case nir_op_unpack_half_2x16_split_x:
                  case nir_op_unpack_half_2x16_split_y:
                  case nir_op_fddx:
                  case nir_op_fddy:
                  case nir_op_fddx_fine:
                  case nir_op_fddy_fine:
                  case nir_op_fddx_coarse:
                  case nir_op_fddy_coarse:
                  case nir_op_fquantize2f16:
                  case nir_op_ldexp:
                  case nir_op_frexp_sig:
                  case nir_op_frexp_exp:
                  case nir_op_cube_face_index:
                  case nir_op_cube_face_coord:
                     type = RegType::vgpr;
                     break;
                  case nir_op_f2i16:
                  case nir_op_f2u16:
                  case nir_op_f2i32:
                  case nir_op_f2u32:
                  case nir_op_f2i64:
                  case nir_op_f2u64:
                  case nir_op_b2i8:
                  case nir_op_b2i16:
                  case nir_op_b2i32:
                  case nir_op_b2i64:
                  case nir_op_b2b32:
                  case nir_op_b2f16:
                  case nir_op_b2f32:
                  case nir_op_mov:
                     break;
                  case nir_op_iadd:
                  case nir_op_isub:
                  case nir_op_imul:
                  case nir_op_imin:
                  case nir_op_imax:
                  case nir_op_umin:
                  case nir_op_umax:
                  case nir_op_ishl:
                  case nir_op_ishr:
                  case nir_op_ushr:
                     /* packed 16bit instructions have to be VGPR */
                     type = alu_instr->dest.dest.ssa.num_components == 2 ? RegType::vgpr : type;
                     FALLTHROUGH;
                  default:
                     for (unsigned i = 0; i < nir_op_infos[alu_instr->op].num_inputs; i++) {
                        if (regclasses[alu_instr->src[i].src.ssa->index].type() == RegType::vgpr)
                           type = RegType::vgpr;
                     }
                     break;
               }

               RegClass rc = get_reg_class(ctx, type, alu_instr->dest.dest.ssa.num_components, alu_instr->dest.dest.ssa.bit_size);
               regclasses[alu_instr->dest.dest.ssa.index] = rc;
               break;
            }
            case nir_instr_type_load_const: {
               unsigned num_components = nir_instr_as_load_const(instr)->def.num_components;
               unsigned bit_size = nir_instr_as_load_const(instr)->def.bit_size;
               RegClass rc = get_reg_class(ctx, RegType::sgpr, num_components, bit_size);
               regclasses[nir_instr_as_load_const(instr)->def.index] = rc;
               break;
            }
            case nir_instr_type_intrinsic: {
               nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(instr);
               if (!nir_intrinsic_infos[intrinsic->intrinsic].has_dest)
                  break;
               RegType type = RegType::sgpr;
               switch(intrinsic->intrinsic) {
                  case nir_intrinsic_load_push_constant:
                  case nir_intrinsic_load_work_group_id:
                  case nir_intrinsic_load_num_work_groups:
                  case nir_intrinsic_load_subgroup_id:
                  case nir_intrinsic_load_num_subgroups:
                  case nir_intrinsic_load_first_vertex:
                  case nir_intrinsic_load_base_instance:
                  case nir_intrinsic_vote_all:
                  case nir_intrinsic_vote_any:
                  case nir_intrinsic_read_first_invocation:
                  case nir_intrinsic_read_invocation:
                  case nir_intrinsic_first_invocation:
                  case nir_intrinsic_ballot:
                  case nir_intrinsic_load_ring_tess_factors_amd:
                  case nir_intrinsic_load_ring_tess_factors_offset_amd:
                  case nir_intrinsic_load_ring_tess_offchip_amd:
                  case nir_intrinsic_load_ring_tess_offchip_offset_amd:
                  case nir_intrinsic_load_ring_esgs_amd:
                  case nir_intrinsic_load_ring_es2gs_offset_amd:
                  case nir_intrinsic_image_deref_samples:
                     type = RegType::sgpr;
                     break;
                  case nir_intrinsic_load_sample_id:
                  case nir_intrinsic_load_sample_mask_in:
                  case nir_intrinsic_load_input:
                  case nir_intrinsic_load_output:
                  case nir_intrinsic_load_input_vertex:
                  case nir_intrinsic_load_per_vertex_input:
                  case nir_intrinsic_load_per_vertex_output:
                  case nir_intrinsic_load_vertex_id:
                  case nir_intrinsic_load_vertex_id_zero_base:
                  case nir_intrinsic_load_barycentric_sample:
                  case nir_intrinsic_load_barycentric_pixel:
                  case nir_intrinsic_load_barycentric_model:
                  case nir_intrinsic_load_barycentric_centroid:
                  case nir_intrinsic_load_barycentric_at_sample:
                  case nir_intrinsic_load_barycentric_at_offset:
                  case nir_intrinsic_load_interpolated_input:
                  case nir_intrinsic_load_frag_coord:
                  case nir_intrinsic_load_frag_shading_rate:
                  case nir_intrinsic_load_sample_pos:
                  case nir_intrinsic_load_layer_id:
                  case nir_intrinsic_load_local_invocation_id:
                  case nir_intrinsic_load_local_invocation_index:
                  case nir_intrinsic_load_subgroup_invocation:
                  case nir_intrinsic_load_tess_coord:
                  case nir_intrinsic_write_invocation_amd:
                  case nir_intrinsic_mbcnt_amd:
                  case nir_intrinsic_load_instance_id:
                  case nir_intrinsic_ssbo_atomic_add:
                  case nir_intrinsic_ssbo_atomic_imin:
                  case nir_intrinsic_ssbo_atomic_umin:
                  case nir_intrinsic_ssbo_atomic_imax:
                  case nir_intrinsic_ssbo_atomic_umax:
                  case nir_intrinsic_ssbo_atomic_and:
                  case nir_intrinsic_ssbo_atomic_or:
                  case nir_intrinsic_ssbo_atomic_xor:
                  case nir_intrinsic_ssbo_atomic_exchange:
                  case nir_intrinsic_ssbo_atomic_comp_swap:
                  case nir_intrinsic_global_atomic_add:
                  case nir_intrinsic_global_atomic_imin:
                  case nir_intrinsic_global_atomic_umin:
                  case nir_intrinsic_global_atomic_imax:
                  case nir_intrinsic_global_atomic_umax:
                  case nir_intrinsic_global_atomic_and:
                  case nir_intrinsic_global_atomic_or:
                  case nir_intrinsic_global_atomic_xor:
                  case nir_intrinsic_global_atomic_exchange:
                  case nir_intrinsic_global_atomic_comp_swap:
                  case nir_intrinsic_image_deref_atomic_add:
                  case nir_intrinsic_image_deref_atomic_umin:
                  case nir_intrinsic_image_deref_atomic_imin:
                  case nir_intrinsic_image_deref_atomic_umax:
                  case nir_intrinsic_image_deref_atomic_imax:
                  case nir_intrinsic_image_deref_atomic_and:
                  case nir_intrinsic_image_deref_atomic_or:
                  case nir_intrinsic_image_deref_atomic_xor:
                  case nir_intrinsic_image_deref_atomic_exchange:
                  case nir_intrinsic_image_deref_atomic_comp_swap:
                  case nir_intrinsic_image_deref_size:
                  case nir_intrinsic_shared_atomic_add:
                  case nir_intrinsic_shared_atomic_imin:
                  case nir_intrinsic_shared_atomic_umin:
                  case nir_intrinsic_shared_atomic_imax:
                  case nir_intrinsic_shared_atomic_umax:
                  case nir_intrinsic_shared_atomic_and:
                  case nir_intrinsic_shared_atomic_or:
                  case nir_intrinsic_shared_atomic_xor:
                  case nir_intrinsic_shared_atomic_exchange:
                  case nir_intrinsic_shared_atomic_comp_swap:
                  case nir_intrinsic_shared_atomic_fadd:
                  case nir_intrinsic_load_scratch:
                  case nir_intrinsic_load_invocation_id:
                  case nir_intrinsic_load_primitive_id:
                  case nir_intrinsic_load_buffer_amd:
                  case nir_intrinsic_load_tess_rel_patch_id_amd:
                  case nir_intrinsic_load_gs_vertex_offset_amd:
                     type = RegType::vgpr;
                     break;
                  case nir_intrinsic_shuffle:
                  case nir_intrinsic_quad_broadcast:
                  case nir_intrinsic_quad_swap_horizontal:
                  case nir_intrinsic_quad_swap_vertical:
                  case nir_intrinsic_quad_swap_diagonal:
                  case nir_intrinsic_quad_swizzle_amd:
                  case nir_intrinsic_masked_swizzle_amd:
                  case nir_intrinsic_inclusive_scan:
                  case nir_intrinsic_exclusive_scan:
                  case nir_intrinsic_reduce:
                  case nir_intrinsic_load_ubo:
                  case nir_intrinsic_load_ssbo:
                  case nir_intrinsic_load_global:
                  case nir_intrinsic_vulkan_resource_index:
                  case nir_intrinsic_load_shared:
                  case nir_intrinsic_get_ssbo_size:
                     type = nir_dest_is_divergent(intrinsic->dest) ? RegType::vgpr : RegType::sgpr;
                     break;
                  case nir_intrinsic_load_view_index:
                     type = ctx->stage == fragment_fs ? RegType::vgpr : RegType::sgpr;
                     break;
                  default:
                     for (unsigned i = 0; i < nir_intrinsic_infos[intrinsic->intrinsic].num_srcs; i++) {
                        if (regclasses[intrinsic->src[i].ssa->index].type() == RegType::vgpr)
                           type = RegType::vgpr;
                     }
                     break;
               }
               RegClass rc = get_reg_class(ctx, type, intrinsic->dest.ssa.num_components, intrinsic->dest.ssa.bit_size);
               regclasses[intrinsic->dest.ssa.index] = rc;

               switch(intrinsic->intrinsic) {
                  case nir_intrinsic_load_barycentric_sample:
                  case nir_intrinsic_load_barycentric_pixel:
                  case nir_intrinsic_load_barycentric_centroid:
                  case nir_intrinsic_load_barycentric_at_sample:
                  case nir_intrinsic_load_barycentric_at_offset: {
                     glsl_interp_mode mode = (glsl_interp_mode)nir_intrinsic_interp_mode(intrinsic);
                     spi_ps_inputs |= get_interp_input(intrinsic->intrinsic, mode);
                     break;
                  }
                  case nir_intrinsic_load_barycentric_model:
                     spi_ps_inputs |= S_0286CC_PERSP_PULL_MODEL_ENA(1);
                     break;
                  case nir_intrinsic_load_front_face:
                     spi_ps_inputs |= S_0286CC_FRONT_FACE_ENA(1);
                     break;
                  case nir_intrinsic_load_frag_coord:
                  case nir_intrinsic_load_sample_pos: {
                     uint8_t mask = nir_ssa_def_components_read(&intrinsic->dest.ssa);
                     for (unsigned i = 0; i < 4; i++) {
                        if (mask & (1 << i))
                           spi_ps_inputs |= S_0286CC_POS_X_FLOAT_ENA(1) << i;

                     }

                     if (ctx->options->adjust_frag_coord_z &&
                         intrinsic->intrinsic == nir_intrinsic_load_frag_coord &&
                         G_0286CC_POS_Z_FLOAT_ENA(spi_ps_inputs)) {
                        /* Enable ancillary for adjusting gl_FragCoord.z for
                         * VRS due to a hw bug on some GFX10.3 chips.
                         */
                        spi_ps_inputs |= S_0286CC_ANCILLARY_ENA(1);
                     }
                     break;
                  }
                  case nir_intrinsic_load_sample_id:
                  case nir_intrinsic_load_frag_shading_rate:
                     spi_ps_inputs |= S_0286CC_ANCILLARY_ENA(1);
                     break;
                  case nir_intrinsic_load_sample_mask_in:
                     spi_ps_inputs |= S_0286CC_ANCILLARY_ENA(1);
                     spi_ps_inputs |= S_0286CC_SAMPLE_COVERAGE_ENA(1);
                     break;
                  default:
                     break;
               }
               break;
            }
            case nir_instr_type_tex: {
               nir_tex_instr* tex = nir_instr_as_tex(instr);
               RegType type = nir_dest_is_divergent(tex->dest) ? RegType::vgpr : RegType::sgpr;

               if (tex->op == nir_texop_texture_samples) {
                  assert(!tex->dest.ssa.divergent);
               }

               RegClass rc = get_reg_class(ctx, type, tex->dest.ssa.num_components,
                                           tex->dest.ssa.bit_size);
               regclasses[tex->dest.ssa.index] = rc;
               break;
            }
            case nir_instr_type_parallel_copy: {
               nir_foreach_parallel_copy_entry(entry, nir_instr_as_parallel_copy(instr)) {
                  regclasses[entry->dest.ssa.index] = regclasses[entry->src.ssa->index];
               }
               break;
            }
            case nir_instr_type_ssa_undef: {
               unsigned num_components = nir_instr_as_ssa_undef(instr)->def.num_components;
               unsigned bit_size = nir_instr_as_ssa_undef(instr)->def.bit_size;
               RegClass rc = get_reg_class(ctx, RegType::sgpr, num_components, bit_size);
               regclasses[nir_instr_as_ssa_undef(instr)->def.index] = rc;
               break;
            }
            case nir_instr_type_phi: {
               nir_phi_instr* phi = nir_instr_as_phi(instr);
               RegType type;
               unsigned size = phi->dest.ssa.num_components;

               if (phi->dest.ssa.bit_size == 1) {
                  assert(size == 1 && "multiple components not yet supported on boolean phis.");
                  type = RegType::sgpr;
                  size *= lane_mask_size;
                  regclasses[phi->dest.ssa.index] = RegClass(type, size);
                  break;
               }

               if (nir_dest_is_divergent(phi->dest)) {
                  type = RegType::vgpr;
               } else {
                  type = RegType::sgpr;
                  nir_foreach_phi_src (src, phi) {
                     if (regclasses[src->src.ssa->index].type() == RegType::vgpr)
                        type = RegType::vgpr;
                     if (regclasses[src->src.ssa->index].type() == RegType::none)
                        done = false;
                  }
               }

               RegClass rc = get_reg_class(ctx, type, phi->dest.ssa.num_components, phi->dest.ssa.bit_size);
               if (rc != regclasses[phi->dest.ssa.index]) {
                  done = false;
               } else {
                  nir_foreach_phi_src(src, phi)
                     assert(regclasses[src->src.ssa->index].size() == rc.size());
               }
               regclasses[phi->dest.ssa.index] = rc;
               break;
            }
            default:
               break;
            }
         }
      }
   }

   if (G_0286CC_POS_W_FLOAT_ENA(spi_ps_inputs)) {
      /* If POS_W_FLOAT (11) is enabled, at least one of PERSP_* must be enabled too */
      spi_ps_inputs |= S_0286CC_PERSP_CENTER_ENA(1);
   }

   if (!(spi_ps_inputs & 0x7F)) {
      /* At least one of PERSP_* (0xF) or LINEAR_* (0x70) must be enabled */
      spi_ps_inputs |= S_0286CC_PERSP_CENTER_ENA(1);
   }

   ctx->program->config->spi_ps_input_ena = spi_ps_inputs;
   ctx->program->config->spi_ps_input_addr = spi_ps_inputs;

   ctx->cf_info.nir_to_aco.reset(nir_to_aco.release());

   /* align and copy constant data */
   while (ctx->program->constant_data.size() % 4u)
      ctx->program->constant_data.push_back(0);
   ctx->constant_data_offset = ctx->program->constant_data.size();
   ctx->program->constant_data.insert(ctx->program->constant_data.end(),
                                      (uint8_t*)shader->constant_data,
                                      (uint8_t*)shader->constant_data + shader->constant_data_size);
}

void cleanup_context(isel_context *ctx)
{
   _mesa_hash_table_destroy(ctx->range_ht, NULL);
}

isel_context
setup_isel_context(Program* program,
                   unsigned shader_count,
                   struct nir_shader *const *shaders,
                   ac_shader_config* config,
                   struct radv_shader_args *args,
                   bool is_gs_copy_shader)
{
   SWStage sw_stage = SWStage::None;
   for (unsigned i = 0; i < shader_count; i++) {
      switch (shaders[i]->info.stage) {
      case MESA_SHADER_VERTEX:
         sw_stage = sw_stage | SWStage::VS;
         break;
      case MESA_SHADER_TESS_CTRL:
         sw_stage = sw_stage | SWStage::TCS;
         break;
      case MESA_SHADER_TESS_EVAL:
         sw_stage = sw_stage | SWStage::TES;
         break;
      case MESA_SHADER_GEOMETRY:
         sw_stage = sw_stage | (is_gs_copy_shader ? SWStage::GSCopy : SWStage::GS);
         break;
      case MESA_SHADER_FRAGMENT:
         sw_stage = sw_stage | SWStage::FS;
         break;
      case MESA_SHADER_COMPUTE:
         sw_stage = sw_stage | SWStage::CS;
         break;
      default:
         unreachable("Shader stage not implemented");
      }
   }
   bool gfx9_plus = args->options->chip_class >= GFX9;
   bool ngg = args->shader_info->is_ngg && args->options->chip_class >= GFX10;
   HWStage hw_stage { };
   if (sw_stage == SWStage::VS && args->shader_info->vs.as_es && !ngg)
      hw_stage = HWStage::ES;
   else if (sw_stage == SWStage::VS && !args->shader_info->vs.as_ls && !ngg)
      hw_stage = HWStage::VS;
   else if (sw_stage == SWStage::VS && ngg)
      hw_stage = HWStage::NGG; /* GFX10/NGG: VS without GS uses the HW GS stage */
   else if (sw_stage == SWStage::GS)
      hw_stage = HWStage::GS;
   else if (sw_stage == SWStage::FS)
      hw_stage = HWStage::FS;
   else if (sw_stage == SWStage::CS)
      hw_stage = HWStage::CS;
   else if (sw_stage == SWStage::GSCopy)
      hw_stage = HWStage::VS;
   else if (sw_stage == SWStage::VS_GS && gfx9_plus && !ngg)
      hw_stage = HWStage::GS; /* GFX6-9: VS+GS merged into a GS (and GFX10/legacy) */
   else if (sw_stage == SWStage::VS_GS && ngg)
      hw_stage = HWStage::NGG; /* GFX10+: VS+GS merged into an NGG GS */
   else if (sw_stage == SWStage::VS && args->shader_info->vs.as_ls)
      hw_stage = HWStage::LS; /* GFX6-8: VS is a Local Shader, when tessellation is used */
   else if (sw_stage == SWStage::TCS)
      hw_stage = HWStage::HS; /* GFX6-8: TCS is a Hull Shader */
   else if (sw_stage == SWStage::VS_TCS)
      hw_stage = HWStage::HS; /* GFX9-10: VS+TCS merged into a Hull Shader */
   else if (sw_stage == SWStage::TES && !args->shader_info->tes.as_es && !ngg)
      hw_stage = HWStage::VS; /* GFX6-9: TES without GS uses the HW VS stage (and GFX10/legacy) */
   else if (sw_stage == SWStage::TES && !args->shader_info->tes.as_es && ngg)
      hw_stage = HWStage::NGG; /* GFX10/NGG: TES without GS */
   else if (sw_stage == SWStage::TES && args->shader_info->tes.as_es && !ngg)
      hw_stage = HWStage::ES; /* GFX6-8: TES is an Export Shader */
   else if (sw_stage == SWStage::TES_GS && gfx9_plus && !ngg)
      hw_stage = HWStage::GS; /* GFX9: TES+GS merged into a GS (and GFX10/legacy) */
   else if (sw_stage == SWStage::TES_GS && ngg)
      hw_stage = HWStage::NGG; /* GFX10+: TES+GS merged into an NGG GS */
   else
      unreachable("Shader stage not implemented");

   init_program(program, Stage { hw_stage, sw_stage }, args->shader_info,
                args->options->chip_class, args->options->family, args->options->wgp_mode, config);

   isel_context ctx = {};
   ctx.program = program;
   ctx.args = args;
   ctx.options = args->options;
   ctx.stage = program->stage;

   /* TODO: Check if we need to adjust min_waves for unknown workgroup sizes. */
   if (program->stage.hw == HWStage::VS || program->stage.hw == HWStage::FS) {
      /* PS and legacy VS have separate waves, no workgroups */
      program->workgroup_size = program->wave_size;
   } else if (program->stage == compute_cs) {
      /* CS sets the workgroup size explicitly */
      program->workgroup_size = shaders[0]->info.cs.local_size[0] *
                                shaders[0]->info.cs.local_size[1] *
                                shaders[0]->info.cs.local_size[2];
   } else if (program->stage.hw == HWStage::ES || program->stage == geometry_gs) {
      /* Unmerged ESGS operate in workgroups if on-chip GS (LDS rings) are enabled on GFX7-8 (not implemented in Mesa)  */
      program->workgroup_size = program->wave_size;
   } else if (program->stage.hw == HWStage::GS) {
      /* If on-chip GS (LDS rings) are enabled on GFX9 or later, merged GS operates in workgroups */
      assert(program->chip_class >= GFX9);
      uint32_t es_verts_per_subgrp = G_028A44_ES_VERTS_PER_SUBGRP(program->info->gs_ring_info.vgt_gs_onchip_cntl);
      uint32_t gs_instr_prims_in_subgrp = G_028A44_GS_INST_PRIMS_IN_SUBGRP(program->info->gs_ring_info.vgt_gs_onchip_cntl);
      uint32_t workgroup_size = MAX2(es_verts_per_subgrp, gs_instr_prims_in_subgrp);
      program->workgroup_size = MAX2(MIN2(workgroup_size, 256), 1);
   } else if (program->stage == vertex_ls) {
      /* Unmerged LS operates in workgroups */
      program->workgroup_size = UINT_MAX; /* TODO: probably tcs_num_patches * tcs_vertices_in, but those are not plumbed to ACO for LS */
   } else if (program->stage == tess_control_hs) {
      /* Unmerged HS operates in workgroups, size is determined by the output vertices */
      setup_tcs_info(&ctx, shaders[0], NULL);
      program->workgroup_size = ctx.tcs_num_patches * shaders[0]->info.tess.tcs_vertices_out;
   } else if (program->stage == vertex_tess_control_hs) {
      /* Merged LSHS operates in workgroups, but can still have a different number of LS and HS invocations */
      setup_tcs_info(&ctx, shaders[1], shaders[0]);
      program->workgroup_size = ctx.tcs_num_patches * MAX2(shaders[1]->info.tess.tcs_vertices_out, ctx.args->options->key.tcs.input_vertices);
   } else if (program->stage.hw == HWStage::NGG) {
      gfx10_ngg_info &ngg_info = args->shader_info->ngg_info;
      unsigned num_gs_invocations = (program->stage.has(SWStage::GS)) ? MAX2(shaders[1]->info.gs.invocations, 1) : 1;

      /* Max ES (SW VS/TES) threads */
      uint32_t max_esverts = ngg_info.hw_max_esverts;
      /* Max GS input primitives = max GS threads */
      uint32_t max_gs_input_prims = ngg_info.max_gsprims * num_gs_invocations;
      /* Maximum output vertices -- each thread can export only 1 vertex */
      uint32_t max_out_vtx = ngg_info.max_out_verts;
      /* Maximum output primitives -- each thread can export only 1 or 0 primitive */
      uint32_t max_out_prm = ngg_info.max_gsprims * num_gs_invocations * ngg_info.prim_amp_factor;

      program->workgroup_size = MAX4(max_esverts, max_gs_input_prims, max_out_vtx, max_out_prm);
   } else {
      unreachable("Unsupported shader stage.");
   }

   calc_min_waves(program);

   unsigned scratch_size = 0;
   if (program->stage == gs_copy_vs) {
      assert(shader_count == 1);
      setup_vs_output_info(&ctx, shaders[0], false, true, &args->shader_info->vs.outinfo);
   } else {
      for (unsigned i = 0; i < shader_count; i++) {
         nir_shader *nir = shaders[i];
         setup_nir(&ctx, nir);
      }

      for (unsigned i = 0; i < shader_count; i++)
         scratch_size = std::max(scratch_size, shaders[i]->scratch_size);
   }

   ctx.program->config->scratch_bytes_per_wave = align(scratch_size * ctx.program->wave_size, 1024);

   ctx.block = ctx.program->create_and_insert_block();
   ctx.block->kind = block_kind_top_level;

   return ctx;
}

}
