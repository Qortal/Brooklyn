/*
 * Copyright © 2019 Valve Corporation.
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
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
 */

#include "radv_shader_args.h"
#include "radv_private.h"
#include "radv_shader.h"

static void
set_loc(struct radv_userdata_info *ud_info, uint8_t *sgpr_idx, uint8_t num_sgprs)
{
   ud_info->sgpr_idx = *sgpr_idx;
   ud_info->num_sgprs = num_sgprs;
   *sgpr_idx += num_sgprs;
}

static void
set_loc_shader(struct radv_shader_args *args, int idx, uint8_t *sgpr_idx, uint8_t num_sgprs)
{
   struct radv_userdata_info *ud_info = &args->user_sgprs_locs.shader_data[idx];
   assert(ud_info);

   set_loc(ud_info, sgpr_idx, num_sgprs);
}

static void
set_loc_shader_ptr(struct radv_shader_args *args, int idx, uint8_t *sgpr_idx)
{
   bool use_32bit_pointers = idx != AC_UD_SCRATCH_RING_OFFSETS;

   set_loc_shader(args, idx, sgpr_idx, use_32bit_pointers ? 1 : 2);
}

static void
set_loc_desc(struct radv_shader_args *args, int idx, uint8_t *sgpr_idx)
{
   struct radv_userdata_locations *locs = &args->user_sgprs_locs;
   struct radv_userdata_info *ud_info = &locs->descriptor_sets[idx];
   assert(ud_info);

   set_loc(ud_info, sgpr_idx, 1);

   locs->descriptor_sets_enabled |= 1u << idx;
}

struct user_sgpr_info {
   bool indirect_all_descriptor_sets;
   uint8_t remaining_sgprs;
   unsigned num_inline_push_consts;
   bool inlined_all_push_consts;
};

static bool
needs_view_index_sgpr(const struct radv_nir_compiler_options *options,
                      const struct radv_shader_info *info, gl_shader_stage stage)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
      if (info->uses_view_index ||
          (!info->vs.as_es && !info->vs.as_ls &&
           options->key.has_multiview_view_index))
         return true;
      break;
   case MESA_SHADER_TESS_EVAL:
      if (info->uses_view_index ||
          (!info->tes.as_es && options->key.has_multiview_view_index))
         return true;
      break;
   case MESA_SHADER_TESS_CTRL:
      if (info->uses_view_index)
         return true;
      break;
   case MESA_SHADER_GEOMETRY:
      if (info->uses_view_index ||
          (info->is_ngg && options->key.has_multiview_view_index))
         return true;
      break;
   case MESA_SHADER_MESH:
      if (info->uses_view_index || options->key.has_multiview_view_index)
         return true;
      break;
   default:
      break;
   }
   return false;
}

static uint8_t
count_vs_user_sgprs(const struct radv_shader_info *info)
{
   uint8_t count = 1; /* vertex offset */

   if (info->vs.vb_desc_usage_mask)
      count++;
   if (info->vs.needs_draw_id)
      count++;
   if (info->vs.needs_base_instance)
      count++;

   return count;
}

static uint8_t
count_ms_user_sgprs(const struct radv_shader_info *info)
{
   uint8_t count = 1 + 3; /* firstTask + num_work_groups[3] */

   if (info->vs.needs_draw_id)
      count++;

   return count;
}

static unsigned
count_ngg_sgprs(const struct radv_shader_info *info, bool has_api_gs)
{
   unsigned count = 0;

   if (has_api_gs)
      count += 1; /* ngg_gs_state */
   if (info->has_ngg_culling)
      count += 5; /* ngg_culling_settings + 4x ngg_viewport_* */

   return count;
}

static void
allocate_inline_push_consts(const struct radv_shader_info *info,
                            struct user_sgpr_info *user_sgpr_info)
{
   uint8_t remaining_sgprs = user_sgpr_info->remaining_sgprs;

   /* Only supported if shaders use push constants. */
   if (info->min_push_constant_used == UINT16_MAX)
      return;

   /* Only supported if shaders don't have indirect push constants. */
   if (info->has_indirect_push_constants)
      return;

   /* Only supported for 32-bit push constants. */
   if (!info->has_only_32bit_push_constants)
      return;

   uint8_t num_push_consts =
      (info->max_push_constant_used - info->min_push_constant_used) / 4;

   /* Check if the number of user SGPRs is large enough. */
   if (num_push_consts < remaining_sgprs) {
      user_sgpr_info->num_inline_push_consts = num_push_consts;
   } else {
      user_sgpr_info->num_inline_push_consts = remaining_sgprs;
   }

   /* Clamp to the maximum number of allowed inlined push constants. */
   if (user_sgpr_info->num_inline_push_consts > AC_MAX_INLINE_PUSH_CONSTS)
      user_sgpr_info->num_inline_push_consts = AC_MAX_INLINE_PUSH_CONSTS;

   if (user_sgpr_info->num_inline_push_consts == num_push_consts &&
       !info->loads_dynamic_offsets) {
      /* Disable the default push constants path if all constants are
       * inlined and if shaders don't use dynamic descriptors.
       */
      user_sgpr_info->inlined_all_push_consts = true;
   }
}

static void
allocate_user_sgprs(const struct radv_nir_compiler_options *options,
                    const struct radv_shader_info *info, gl_shader_stage stage,
                    bool has_previous_stage, gl_shader_stage previous_stage, bool needs_view_index,
                    bool has_api_gs, bool is_gs_copy_shader, struct user_sgpr_info *user_sgpr_info)
{
   uint8_t user_sgpr_count = 0;

   memset(user_sgpr_info, 0, sizeof(struct user_sgpr_info));

   /* 2 user sgprs will always be allocated for scratch/rings */
   user_sgpr_count += 2;

   /* prolog inputs */
   if (info->vs.has_prolog)
      user_sgpr_count += 2;

   switch (stage) {
   case MESA_SHADER_COMPUTE:
      if (info->cs.uses_sbt)
         user_sgpr_count += 1;
      if (info->cs.uses_grid_size)
         user_sgpr_count += 3;
      if (info->cs.uses_ray_launch_size)
         user_sgpr_count += 3;
      break;
   case MESA_SHADER_FRAGMENT:
      break;
   case MESA_SHADER_VERTEX:
      if (!is_gs_copy_shader)
         user_sgpr_count += count_vs_user_sgprs(info);
      break;
   case MESA_SHADER_TESS_CTRL:
      if (has_previous_stage) {
         if (previous_stage == MESA_SHADER_VERTEX)
            user_sgpr_count += count_vs_user_sgprs(info);
      }
      break;
   case MESA_SHADER_TESS_EVAL:
      break;
   case MESA_SHADER_GEOMETRY:
      if (has_previous_stage) {
         if (info->is_ngg)
            user_sgpr_count += count_ngg_sgprs(info, has_api_gs);

         if (previous_stage == MESA_SHADER_VERTEX) {
            user_sgpr_count += count_vs_user_sgprs(info);
         } else if (previous_stage == MESA_SHADER_MESH) {
            user_sgpr_count += count_ms_user_sgprs(info);
         }
      }
      break;
   default:
      break;
   }

   if (needs_view_index)
      user_sgpr_count++;

   if (info->force_vrs_per_vertex)
      user_sgpr_count++;

   if (info->loads_push_constants)
      user_sgpr_count++;

   if (info->so.num_outputs)
      user_sgpr_count++;

   uint32_t available_sgprs =
      options->chip_class >= GFX9 && stage != MESA_SHADER_COMPUTE ? 32 : 16;
   uint32_t remaining_sgprs = available_sgprs - user_sgpr_count;
   uint32_t num_desc_set = util_bitcount(info->desc_set_used_mask);

   if (remaining_sgprs < num_desc_set) {
      user_sgpr_info->indirect_all_descriptor_sets = true;
      user_sgpr_info->remaining_sgprs = remaining_sgprs - 1;
   } else {
      user_sgpr_info->remaining_sgprs = remaining_sgprs - num_desc_set;
   }

   allocate_inline_push_consts(info, user_sgpr_info);
}

static void
declare_global_input_sgprs(const struct radv_shader_info *info,
                           const struct user_sgpr_info *user_sgpr_info,
                           struct radv_shader_args *args)
{
   /* 1 for each descriptor set */
   if (!user_sgpr_info->indirect_all_descriptor_sets) {
      uint32_t mask = info->desc_set_used_mask;

      while (mask) {
         int i = u_bit_scan(&mask);

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_PTR, &args->descriptor_sets[i]);
      }
   } else {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_PTR_PTR, &args->descriptor_sets[0]);
   }

   if (info->loads_push_constants && !user_sgpr_info->inlined_all_push_consts) {
      /* 1 for push constants and dynamic descriptors */
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_PTR, &args->ac.push_constants);
   }

   for (unsigned i = 0; i < user_sgpr_info->num_inline_push_consts; i++) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.inline_push_consts[i]);
   }
   args->ac.base_inline_push_consts = info->min_push_constant_used / 4;

   if (info->so.num_outputs) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_DESC_PTR, &args->streamout_buffers);
   }
}

static void
declare_vs_specific_input_sgprs(const struct radv_shader_info *info, struct radv_shader_args *args,
                                gl_shader_stage stage, bool has_previous_stage,
                                gl_shader_stage previous_stage)
{
   if (info->vs.has_prolog)
      ac_add_arg(&args->ac, AC_ARG_SGPR, 2, AC_ARG_INT, &args->prolog_inputs);

   if (!args->is_gs_copy_shader && (stage == MESA_SHADER_VERTEX ||
                                    (has_previous_stage && previous_stage == MESA_SHADER_VERTEX))) {
      if (info->vs.vb_desc_usage_mask) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_DESC_PTR, &args->ac.vertex_buffers);
      }
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.base_vertex);
      if (info->vs.needs_draw_id) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.draw_id);
      }
      if (info->vs.needs_base_instance) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.start_instance);
      }
   }
}

static void
declare_vs_input_vgprs(const struct radv_nir_compiler_options *options,
                       const struct radv_shader_info *info, struct radv_shader_args *args)
{
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vertex_id);
   if (!args->is_gs_copy_shader) {
      if (info->vs.as_ls) {
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vs_rel_patch_id);
         if (options->chip_class >= GFX10) {
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user vgpr */
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
         } else {
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* unused */
         }
      } else {
         if (options->chip_class >= GFX10) {
            if (info->is_ngg) {
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user vgpr */
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user vgpr */
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
            } else {
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* unused */
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vs_prim_id);
               ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
            }
         } else {
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.instance_id);
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vs_prim_id);
            ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* unused */
         }
      }
   }

   if (info->vs.dynamic_inputs) {
      assert(info->vs.use_per_attribute_vb_descs);
      unsigned num_attributes = util_last_bit(info->vs.vb_desc_usage_mask);
      for (unsigned i = 0; i < num_attributes; i++)
         ac_add_arg(&args->ac, AC_ARG_VGPR, 4, AC_ARG_INT, &args->vs_inputs[i]);
      /* Ensure the main shader doesn't use less vgprs than the prolog. The prolog requires one
       * VGPR more than the number of shader arguments in the case of non-trivial divisors on GFX8.
       */
      ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL);
   }
}

static void
declare_streamout_sgprs(const struct radv_shader_info *info, struct radv_shader_args *args,
                        gl_shader_stage stage)
{
   int i;

   /* Streamout SGPRs. */
   if (info->so.num_outputs) {
      assert(stage == MESA_SHADER_VERTEX || stage == MESA_SHADER_TESS_EVAL);

      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.streamout_config);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.streamout_write_index);
   } else if (stage == MESA_SHADER_TESS_EVAL) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
   }

   /* A streamout buffer offset is loaded if the stride is non-zero. */
   for (i = 0; i < 4; i++) {
      if (!info->so.strides[i])
         continue;

      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.streamout_offset[i]);
   }
}

static void
declare_tes_input_vgprs(struct radv_shader_args *args)
{
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.tes_u);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.tes_v);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tes_rel_patch_id);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tes_patch_id);
}

static void
declare_ms_input_sgprs(const struct radv_shader_info *info, struct radv_shader_args *args)
{
   ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.base_vertex);
   ac_add_arg(&args->ac, AC_ARG_SGPR, 3, AC_ARG_INT, &args->ac.num_work_groups);
   if (info->vs.needs_draw_id) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.draw_id);
   }
}

static void
declare_ms_input_vgprs(struct radv_shader_args *args)
{
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.vertex_id);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user vgpr */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* user vgpr */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* instance_id */
}

static void
declare_ps_input_vgprs(const struct radv_shader_info *info, struct radv_shader_args *args,
                       bool remap_spi_ps_input)
{
   unsigned spi_ps_input = info->ps.spi_ps_input;

   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.persp_sample);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.persp_center);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.persp_centroid);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 3, AC_ARG_INT, &args->ac.pull_model);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.linear_sample);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.linear_center);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 2, AC_ARG_INT, &args->ac.linear_centroid);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, NULL); /* line stipple tex */
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[0]);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[1]);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[2]);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_FLOAT, &args->ac.frag_pos[3]);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.front_face);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.ancillary);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.sample_coverage);
   ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, NULL); /* fixed pt */

   if (remap_spi_ps_input) {
      /* LLVM optimizes away unused FS inputs and computes spi_ps_input_addr itself and then
       * communicates the results back via the ELF binary. Mirror what LLVM does by re-mapping the
       * VGPR arguments here.
       */
      unsigned arg_count = 0;
      for (unsigned i = 0, vgpr_arg = 0, vgpr_reg = 0; i < args->ac.arg_count; i++) {
         if (args->ac.args[i].file != AC_ARG_VGPR) {
            arg_count++;
            continue;
         }

         if (!(spi_ps_input & (1 << vgpr_arg))) {
            args->ac.args[i].skip = true;
         } else {
            args->ac.args[i].offset = vgpr_reg;
            vgpr_reg += args->ac.args[i].size;
            arg_count++;
         }
         vgpr_arg++;
      }
   }
}

static void
declare_ngg_sgprs(const struct radv_shader_info *info, struct radv_shader_args *args,
                  bool has_api_gs)
{
   if (has_api_gs) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ngg_gs_state);
   }

   if (info->has_ngg_culling) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ngg_culling_settings);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ngg_viewport_scale[0]);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ngg_viewport_scale[1]);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ngg_viewport_translate[0]);
      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ngg_viewport_translate[1]);
   }
}

static void
set_global_input_locs(struct radv_shader_args *args, const struct user_sgpr_info *user_sgpr_info,
                      uint8_t *user_sgpr_idx)
{
   unsigned num_inline_push_consts = 0;

   if (!user_sgpr_info->indirect_all_descriptor_sets) {
      for (unsigned i = 0; i < ARRAY_SIZE(args->descriptor_sets); i++) {
         if (args->descriptor_sets[i].used)
            set_loc_desc(args, i, user_sgpr_idx);
      }
   } else {
      set_loc_shader_ptr(args, AC_UD_INDIRECT_DESCRIPTOR_SETS, user_sgpr_idx);
   }

   if (args->ac.push_constants.used) {
      set_loc_shader_ptr(args, AC_UD_PUSH_CONSTANTS, user_sgpr_idx);
   }

   for (unsigned i = 0; i < ARRAY_SIZE(args->ac.inline_push_consts); i++) {
      if (args->ac.inline_push_consts[i].used)
         num_inline_push_consts++;
   }

   if (num_inline_push_consts) {
      set_loc_shader(args, AC_UD_INLINE_PUSH_CONSTANTS, user_sgpr_idx, num_inline_push_consts);
   }

   if (args->streamout_buffers.used) {
      set_loc_shader_ptr(args, AC_UD_STREAMOUT_BUFFERS, user_sgpr_idx);
   }
}

static void
set_vs_specific_input_locs(struct radv_shader_args *args, gl_shader_stage stage,
                           bool has_previous_stage, gl_shader_stage previous_stage,
                           uint8_t *user_sgpr_idx)
{
   if (args->prolog_inputs.used)
      set_loc_shader(args, AC_UD_VS_PROLOG_INPUTS, user_sgpr_idx, 2);

   if (!args->is_gs_copy_shader && (stage == MESA_SHADER_VERTEX ||
                                    (has_previous_stage && previous_stage == MESA_SHADER_VERTEX))) {
      if (args->ac.vertex_buffers.used) {
         set_loc_shader_ptr(args, AC_UD_VS_VERTEX_BUFFERS, user_sgpr_idx);
      }

      unsigned vs_num = args->ac.base_vertex.used + args->ac.draw_id.used +
                        args->ac.start_instance.used;
      set_loc_shader(args, AC_UD_VS_BASE_VERTEX_START_INSTANCE, user_sgpr_idx, vs_num);
   }
}

static void
set_ms_input_locs(struct radv_shader_args *args, uint8_t *user_sgpr_idx)
{
   unsigned vs_num =
      args->ac.base_vertex.used + 3 * args->ac.num_work_groups.used + args->ac.draw_id.used;
   set_loc_shader(args, AC_UD_VS_BASE_VERTEX_START_INSTANCE, user_sgpr_idx, vs_num);
}

void
radv_declare_shader_args(const struct radv_nir_compiler_options *options,
                         const struct radv_shader_info *info, gl_shader_stage stage,
                         bool has_previous_stage, gl_shader_stage previous_stage,
                         struct radv_shader_args *args)
{
   struct user_sgpr_info user_sgpr_info;
   bool needs_view_index = needs_view_index_sgpr(options, info, stage);
   bool has_api_gs = stage == MESA_SHADER_GEOMETRY;

   if (options->chip_class >= GFX10 && info->is_ngg && stage != MESA_SHADER_GEOMETRY) {
      /* Handle all NGG shaders as GS to simplify the code here. */
      previous_stage = stage;
      stage = MESA_SHADER_GEOMETRY;
      has_previous_stage = true;
   }

   for (int i = 0; i < MAX_SETS; i++)
      args->user_sgprs_locs.descriptor_sets[i].sgpr_idx = -1;
   for (int i = 0; i < AC_UD_MAX_UD; i++)
      args->user_sgprs_locs.shader_data[i].sgpr_idx = -1;

   allocate_user_sgprs(options, info, stage, has_previous_stage, previous_stage, needs_view_index,
                       has_api_gs, args->is_gs_copy_shader, &user_sgpr_info);

   if (options->explicit_scratch_args) {
      ac_add_arg(&args->ac, AC_ARG_SGPR, 2, AC_ARG_CONST_DESC_PTR, &args->ring_offsets);
   }

   /* To ensure prologs match the main VS, VS specific input SGPRs have to be placed before other
    * sgprs.
    */

   switch (stage) {
   case MESA_SHADER_COMPUTE:
      declare_global_input_sgprs(info, &user_sgpr_info, args);

      if (info->cs.uses_sbt) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_CONST_DESC_PTR, &args->ac.sbt_descriptors);
      }

      if (info->cs.uses_grid_size) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 3, AC_ARG_INT, &args->ac.num_work_groups);
      }

      if (info->cs.uses_ray_launch_size) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 3, AC_ARG_INT, &args->ac.ray_launch_size);
      }

      for (int i = 0; i < 3; i++) {
         if (info->cs.uses_block_id[i]) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.workgroup_ids[i]);
         }
      }

      if (info->cs.uses_local_invocation_idx) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tg_size);
      }

      if (options->explicit_scratch_args) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
      }

      ac_add_arg(&args->ac, AC_ARG_VGPR, 3, AC_ARG_INT, &args->ac.local_invocation_ids);
      break;
   case MESA_SHADER_VERTEX:
      /* NGG is handled by the GS case */
      assert(!info->is_ngg);

      declare_vs_specific_input_sgprs(info, args, stage, has_previous_stage, previous_stage);

      declare_global_input_sgprs(info, &user_sgpr_info, args);

      if (needs_view_index) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.view_index);
      }

      if (info->force_vrs_per_vertex) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.force_vrs_rates);
      }

      if (info->vs.as_es) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.es2gs_offset);
      } else if (info->vs.as_ls) {
         /* no extra parameters */
      } else {
         declare_streamout_sgprs(info, args, stage);
      }

      if (options->explicit_scratch_args) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
      }

      declare_vs_input_vgprs(options, info, args);
      break;
   case MESA_SHADER_TESS_CTRL:
      if (has_previous_stage) {
         // First 6 system regs
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.merged_wave_info);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tcs_factor_offset);

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); // unknown
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); // unknown

         declare_vs_specific_input_sgprs(info, args, stage, has_previous_stage, previous_stage);

         declare_global_input_sgprs(info, &user_sgpr_info, args);

         if (needs_view_index) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.view_index);
         }

         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_patch_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_rel_ids);

         declare_vs_input_vgprs(options, info, args);
      } else {
         declare_global_input_sgprs(info, &user_sgpr_info, args);

         if (needs_view_index) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.view_index);
         }

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tcs_factor_offset);
         if (options->explicit_scratch_args) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
         }
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_patch_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.tcs_rel_ids);
      }
      break;
   case MESA_SHADER_TESS_EVAL:
      /* NGG is handled by the GS case */
      assert(!info->is_ngg);

      declare_global_input_sgprs(info, &user_sgpr_info, args);

      if (needs_view_index)
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.view_index);

      if (info->tes.as_es) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.es2gs_offset);
      } else {
         declare_streamout_sgprs(info, args, stage);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);
      }
      if (options->explicit_scratch_args) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
      }
      declare_tes_input_vgprs(args);
      break;
   case MESA_SHADER_GEOMETRY:
      if (has_previous_stage) {
         // First 6 system regs
         if (info->is_ngg) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs_tg_info);
         } else {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs2vs_offset);
         }

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.merged_wave_info);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.tess_offchip_offset);

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); // unknown
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, NULL); // unknown

         if (previous_stage == MESA_SHADER_VERTEX) {
            declare_vs_specific_input_sgprs(info, args, stage, has_previous_stage, previous_stage);
         } else if (previous_stage == MESA_SHADER_MESH) {
            declare_ms_input_sgprs(info, args);
         }

         declare_global_input_sgprs(info, &user_sgpr_info, args);

         if (needs_view_index) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.view_index);
         }

         if (info->force_vrs_per_vertex) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.force_vrs_rates);
         }

         if (info->is_ngg) {
            declare_ngg_sgprs(info, args, has_api_gs);
         }

         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[0]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[1]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_prim_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_invocation_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[2]);

         if (previous_stage == MESA_SHADER_VERTEX) {
            declare_vs_input_vgprs(options, info, args);
         } else if (previous_stage == MESA_SHADER_TESS_EVAL) {
            declare_tes_input_vgprs(args);
         } else if (previous_stage == MESA_SHADER_MESH) {
            declare_ms_input_vgprs(args);
         }
      } else {
         declare_global_input_sgprs(info, &user_sgpr_info, args);

         if (needs_view_index) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.view_index);
         }

         if (info->force_vrs_per_vertex) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.force_vrs_rates);
         }

         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs2vs_offset);
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.gs_wave_id);
         if (options->explicit_scratch_args) {
            ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
         }
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[0]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[1]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_prim_id);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[2]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[3]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[4]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_vtx_offset[5]);
         ac_add_arg(&args->ac, AC_ARG_VGPR, 1, AC_ARG_INT, &args->ac.gs_invocation_id);
      }
      break;
   case MESA_SHADER_FRAGMENT:
      declare_global_input_sgprs(info, &user_sgpr_info, args);

      ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.prim_mask);
      if (options->explicit_scratch_args) {
         ac_add_arg(&args->ac, AC_ARG_SGPR, 1, AC_ARG_INT, &args->ac.scratch_offset);
      }

      declare_ps_input_vgprs(info, args, options->remap_spi_ps_input);
      break;
   default:
      unreachable("Shader stage not implemented");
   }

   uint8_t user_sgpr_idx = 0;

   set_loc_shader_ptr(args, AC_UD_SCRATCH_RING_OFFSETS, &user_sgpr_idx);

   /* For merged shaders the user SGPRs start at 8, with 8 system SGPRs in front (including
    * the rw_buffers at s0/s1. With user SGPR0 = s8, lets restart the count from 0 */
   if (has_previous_stage)
      user_sgpr_idx = 0;

   if (stage == MESA_SHADER_VERTEX || (has_previous_stage && previous_stage == MESA_SHADER_VERTEX))
      set_vs_specific_input_locs(args, stage, has_previous_stage, previous_stage, &user_sgpr_idx);
   else if (has_previous_stage && previous_stage == MESA_SHADER_MESH)
      set_ms_input_locs(args, &user_sgpr_idx);

   set_global_input_locs(args, &user_sgpr_info, &user_sgpr_idx);

   switch (stage) {
   case MESA_SHADER_COMPUTE:
      if (args->ac.sbt_descriptors.used) {
         set_loc_shader_ptr(args, AC_UD_CS_SBT_DESCRIPTORS, &user_sgpr_idx);
      }
      if (args->ac.num_work_groups.used) {
         set_loc_shader(args, AC_UD_CS_GRID_SIZE, &user_sgpr_idx, 3);
      }
      if (args->ac.ray_launch_size.used) {
         set_loc_shader(args, AC_UD_CS_RAY_LAUNCH_SIZE, &user_sgpr_idx, 3);
      }
      break;
   case MESA_SHADER_VERTEX:
      if (args->ac.view_index.used)
         set_loc_shader(args, AC_UD_VIEW_INDEX, &user_sgpr_idx, 1);
      if (args->ac.force_vrs_rates.used)
         set_loc_shader(args, AC_UD_FORCE_VRS_RATES, &user_sgpr_idx, 1);
      break;
   case MESA_SHADER_TESS_CTRL:
      if (args->ac.view_index.used)
         set_loc_shader(args, AC_UD_VIEW_INDEX, &user_sgpr_idx, 1);
      break;
   case MESA_SHADER_TESS_EVAL:
      if (args->ac.view_index.used)
         set_loc_shader(args, AC_UD_VIEW_INDEX, &user_sgpr_idx, 1);
      break;
   case MESA_SHADER_GEOMETRY:
      if (args->ac.view_index.used)
         set_loc_shader(args, AC_UD_VIEW_INDEX, &user_sgpr_idx, 1);

      if (args->ac.force_vrs_rates.used)
         set_loc_shader(args, AC_UD_FORCE_VRS_RATES, &user_sgpr_idx, 1);

      if (args->ngg_gs_state.used) {
         set_loc_shader(args, AC_UD_NGG_GS_STATE, &user_sgpr_idx, 1);
      }

      if (args->ngg_culling_settings.used) {
         set_loc_shader(args, AC_UD_NGG_CULLING_SETTINGS, &user_sgpr_idx, 1);
      }

      if (args->ngg_viewport_scale[0].used) {
         assert(args->ngg_viewport_scale[1].used &&
                args->ngg_viewport_translate[0].used &&
                args->ngg_viewport_translate[1].used);
         set_loc_shader(args, AC_UD_NGG_VIEWPORT, &user_sgpr_idx, 4);
      }
      break;
   case MESA_SHADER_FRAGMENT:
      break;
   default:
      unreachable("Shader stage not implemented");
   }

   args->num_user_sgprs = user_sgpr_idx;
}
