/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 * All Rights Reserved.
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

#ifndef AC_PERFCOUNTER_H
#define AC_PERFCOUNTER_H

#include <stdbool.h>

#include "sid.h"

#include "ac_gpu_info.h"

/* Max counters per HW block */
#define AC_QUERY_MAX_COUNTERS 16

#define AC_PC_SHADERS_WINDOWING (1u << 31)

enum ac_pc_block_flags
{
   /* This block is part of the shader engine */
   AC_PC_BLOCK_SE = (1 << 0),

   /* Expose per-instance groups instead of summing all instances (within
    * an SE). */
   AC_PC_BLOCK_INSTANCE_GROUPS = (1 << 1),

   /* Expose per-SE groups instead of summing instances across SEs. */
   AC_PC_BLOCK_SE_GROUPS = (1 << 2),

   /* Shader block */
   AC_PC_BLOCK_SHADER = (1 << 3),

   /* Non-shader block with perfcounters windowed by shaders. */
   AC_PC_BLOCK_SHADER_WINDOWED = (1 << 4),
};

enum ac_pc_reg_layout
{
   /* All secondary selector dwords follow as one block after the primary
    * selector dwords for the counters that have secondary selectors.
    *
    * Example:
    *    PERFCOUNTER0_SELECT
    *    PERFCOUNTER1_SELECT
    *    PERFCOUNTER0_SELECT1
    *    PERFCOUNTER1_SELECT1
    *    PERFCOUNTER2_SELECT
    *    PERFCOUNTER3_SELECT
    */
   AC_PC_MULTI_BLOCK = 0,

   /* Each secondary selector dword follows immediately after the
    * corresponding primary.
    *
    * Example:
    *    PERFCOUNTER0_SELECT
    *    PERFCOUNTER0_SELECT1
    *    PERFCOUNTER1_SELECT
    *    PERFCOUNTER1_SELECT1
    *    PERFCOUNTER2_SELECT
    *    PERFCOUNTER3_SELECT
    */
   AC_PC_MULTI_ALTERNATE = 1,

   /* All secondary selector dwords follow as one block after all primary
    * selector dwords.
    *
    * Example:
    *    PERFCOUNTER0_SELECT
    *    PERFCOUNTER1_SELECT
    *    PERFCOUNTER2_SELECT
    *    PERFCOUNTER3_SELECT
    *    PERFCOUNTER0_SELECT1
    *    PERFCOUNTER1_SELECT1
    */
   AC_PC_MULTI_TAIL = 2,

   /* Free-form arrangement of selector registers. */
   AC_PC_MULTI_CUSTOM = 3,

   AC_PC_MULTI_MASK = 3,

   /* Registers are laid out in decreasing rather than increasing order. */
   AC_PC_REG_REVERSE = 4,

   AC_PC_FAKE = 8,
};

struct ac_pc_block_base {
   const char *name;
   unsigned num_counters;
   unsigned flags;

   unsigned select_or;
   unsigned select0;
   unsigned counter0_lo;
   unsigned *select;
   unsigned *counters;
   unsigned num_multi;
   unsigned num_prelude;
   unsigned layout;
};

struct ac_pc_block_gfxdescr {
   struct ac_pc_block_base *b;
   unsigned selectors;
   unsigned instances;
};

struct ac_pc_block {
   const struct ac_pc_block_gfxdescr *b;
   unsigned num_instances;

   unsigned num_groups;
   char *group_names;
   unsigned group_name_stride;

   char *selector_names;
   unsigned selector_name_stride;
};

struct ac_perfcounters {
   unsigned num_groups;
   unsigned num_blocks;
   struct ac_pc_block *blocks;

   bool separate_se;
   bool separate_instance;
};

/* The order is chosen to be compatible with GPUPerfStudio's hardcoding of
 * performance counter group IDs.
 */
static const char *const ac_pc_shader_type_suffixes[] = {"",    "_ES", "_GS", "_VS",
                                                         "_PS", "_LS", "_HS", "_CS"};

static const unsigned ac_pc_shader_type_bits[] = {
   0x7f,
   S_036780_ES_EN(1),
   S_036780_GS_EN(1),
   S_036780_VS_EN(1),
   S_036780_PS_EN(1),
   S_036780_LS_EN(1),
   S_036780_HS_EN(1),
   S_036780_CS_EN(1),
};

static inline bool
ac_pc_block_has_per_se_groups(const struct ac_perfcounters *pc,
                              const struct ac_pc_block *block)
{
   return block->b->b->flags & AC_PC_BLOCK_SE_GROUPS ||
          (block->b->b->flags & AC_PC_BLOCK_SE && pc->separate_se);
}

static inline bool
ac_pc_block_has_per_instance_groups(const struct ac_perfcounters *pc,
                                    const struct ac_pc_block *block)
{
   return block->b->b->flags & AC_PC_BLOCK_INSTANCE_GROUPS ||
          (block->num_instances > 1 && pc->separate_instance);
}

struct ac_pc_block *ac_lookup_counter(const struct ac_perfcounters *pc,
                                      unsigned index, unsigned *base_gid,
                                      unsigned *sub_index);
struct ac_pc_block *ac_lookup_group(const struct ac_perfcounters *pc,
                                    unsigned *index);

bool ac_init_block_names(const struct radeon_info *info,
                         const struct ac_perfcounters *pc,
                         struct ac_pc_block *block);

bool ac_init_perfcounters(const struct radeon_info *info,
                          bool separate_se,
                          bool separate_instance,
                          struct ac_perfcounters *pc);
void ac_destroy_perfcounters(struct ac_perfcounters *pc);

#endif
