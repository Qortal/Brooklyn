/* This file is generated by venus-protocol.  See vn_protocol_driver.h. */

/*
 * Copyright 2020 Google LLC
 * SPDX-License-Identifier: MIT
 */

#ifndef VN_PROTOCOL_DRIVER_INFO_H
#define VN_PROTOCOL_DRIVER_INFO_H

#include "vn_protocol_driver_defines.h"

struct vn_info_extension {
   uint32_t index;

   const char *name;
   uint32_t spec_version;
};

static inline uint32_t
vn_info_wire_format_version(void)
{
    return 0;
}

static inline uint32_t
vn_info_vk_xml_version(void)
{
    return VK_MAKE_API_VERSION(0, 1, 3, 204);
}

static inline int
vn_info_extension_compare(const void *name, const void *ext)
{
   return strcmp(name, ((const struct vn_info_extension *)ext)->name);
}

static inline const struct vn_info_extension *
vn_info_extension_get(const char *name)
{
   static const struct vn_info_extension vn_info_extensions[81] = {
      { 0, "VK_EXT_4444_formats", 1 },
      { 1, "VK_EXT_calibrated_timestamps", 2 },
      { 2, "VK_EXT_command_serialization", 0 },
      { 3, "VK_EXT_custom_border_color", 12 },
      { 4, "VK_EXT_descriptor_indexing", 2 },
      { 5, "VK_EXT_extended_dynamic_state", 1 },
      { 6, "VK_EXT_extended_dynamic_state2", 1 },
      { 7, "VK_EXT_external_memory_dma_buf", 1 },
      { 8, "VK_EXT_host_query_reset", 1 },
      { 9, "VK_EXT_image_drm_format_modifier", 2 },
      { 10, "VK_EXT_image_robustness", 1 },
      { 11, "VK_EXT_inline_uniform_block", 1 },
      { 12, "VK_EXT_line_rasterization", 1 },
      { 13, "VK_EXT_pipeline_creation_cache_control", 3 },
      { 14, "VK_EXT_pipeline_creation_feedback", 1 },
      { 15, "VK_EXT_private_data", 1 },
      { 16, "VK_EXT_provoking_vertex", 1 },
      { 17, "VK_EXT_queue_family_foreign", 1 },
      { 18, "VK_EXT_sampler_filter_minmax", 2 },
      { 19, "VK_EXT_scalar_block_layout", 1 },
      { 20, "VK_EXT_separate_stencil_usage", 1 },
      { 21, "VK_EXT_shader_demote_to_helper_invocation", 1 },
      { 22, "VK_EXT_shader_viewport_index_layer", 1 },
      { 23, "VK_EXT_subgroup_size_control", 2 },
      { 24, "VK_EXT_texel_buffer_alignment", 1 },
      { 25, "VK_EXT_texture_compression_astc_hdr", 1 },
      { 26, "VK_EXT_tooling_info", 1 },
      { 27, "VK_EXT_transform_feedback", 1 },
      { 28, "VK_EXT_ycbcr_2plane_444_formats", 1 },
      { 29, "VK_KHR_16bit_storage", 1 },
      { 30, "VK_KHR_8bit_storage", 1 },
      { 31, "VK_KHR_bind_memory2", 1 },
      { 32, "VK_KHR_buffer_device_address", 1 },
      { 33, "VK_KHR_copy_commands2", 1 },
      { 34, "VK_KHR_create_renderpass2", 1 },
      { 35, "VK_KHR_dedicated_allocation", 3 },
      { 36, "VK_KHR_depth_stencil_resolve", 1 },
      { 37, "VK_KHR_descriptor_update_template", 1 },
      { 38, "VK_KHR_device_group", 4 },
      { 39, "VK_KHR_device_group_creation", 1 },
      { 40, "VK_KHR_draw_indirect_count", 1 },
      { 41, "VK_KHR_driver_properties", 1 },
      { 42, "VK_KHR_dynamic_rendering", 1 },
      { 43, "VK_KHR_external_fence", 1 },
      { 44, "VK_KHR_external_fence_capabilities", 1 },
      { 45, "VK_KHR_external_memory", 1 },
      { 46, "VK_KHR_external_memory_capabilities", 1 },
      { 47, "VK_KHR_external_memory_fd", 1 },
      { 48, "VK_KHR_external_semaphore", 1 },
      { 49, "VK_KHR_external_semaphore_capabilities", 1 },
      { 50, "VK_KHR_format_feature_flags2", 1 },
      { 51, "VK_KHR_get_memory_requirements2", 1 },
      { 52, "VK_KHR_get_physical_device_properties2", 2 },
      { 53, "VK_KHR_image_format_list", 1 },
      { 54, "VK_KHR_imageless_framebuffer", 1 },
      { 55, "VK_KHR_maintenance1", 2 },
      { 56, "VK_KHR_maintenance2", 1 },
      { 57, "VK_KHR_maintenance3", 1 },
      { 58, "VK_KHR_maintenance4", 2 },
      { 59, "VK_KHR_multiview", 1 },
      { 60, "VK_KHR_relaxed_block_layout", 1 },
      { 61, "VK_KHR_sampler_mirror_clamp_to_edge", 3 },
      { 62, "VK_KHR_sampler_ycbcr_conversion", 14 },
      { 63, "VK_KHR_separate_depth_stencil_layouts", 1 },
      { 64, "VK_KHR_shader_atomic_int64", 1 },
      { 65, "VK_KHR_shader_draw_parameters", 1 },
      { 66, "VK_KHR_shader_float16_int8", 1 },
      { 67, "VK_KHR_shader_float_controls", 4 },
      { 68, "VK_KHR_shader_integer_dot_product", 1 },
      { 69, "VK_KHR_shader_non_semantic_info", 1 },
      { 70, "VK_KHR_shader_subgroup_extended_types", 1 },
      { 71, "VK_KHR_shader_terminate_invocation", 1 },
      { 72, "VK_KHR_spirv_1_4", 1 },
      { 73, "VK_KHR_storage_buffer_storage_class", 1 },
      { 74, "VK_KHR_synchronization2", 1 },
      { 75, "VK_KHR_timeline_semaphore", 2 },
      { 76, "VK_KHR_uniform_buffer_standard_layout", 1 },
      { 77, "VK_KHR_variable_pointers", 1 },
      { 78, "VK_KHR_vulkan_memory_model", 3 },
      { 79, "VK_KHR_zero_initialize_workgroup_memory", 1 },
      { 80, "VK_MESA_venus_protocol", 100000 },
   };

   return bsearch(name, vn_info_extensions, 81,
         sizeof(*vn_info_extensions), vn_info_extension_compare);
}

#endif /* VN_PROTOCOL_DRIVER_INFO_H */
