/*
 * Copyright © 2019 Valve Corporation
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

/**
 * Although it's called a load/store "vectorization" pass, this also combines
 * intersecting and identical loads/stores. It currently supports derefs, ubo,
 * ssbo and push constant loads/stores.
 *
 * This doesn't handle copy_deref intrinsics and assumes that
 * nir_lower_alu_to_scalar() has been called and that the IR is free from ALU
 * modifiers. It also assumes that derefs have explicitly laid out types.
 *
 * After vectorization, the backend may want to call nir_lower_alu_to_scalar()
 * and nir_lower_pack(). Also this creates cast instructions taking derefs as a
 * source and some parts of NIR may not be able to handle that well.
 *
 * There are a few situations where this doesn't vectorize as well as it could:
 * - It won't turn four consecutive vec3 loads into 3 vec4 loads.
 * - It doesn't do global vectorization.
 * Handling these cases probably wouldn't provide much benefit though.
 *
 * This probably doesn't handle big-endian GPUs correctly.
*/

#include "nir.h"
#include "nir_deref.h"
#include "nir_builder.h"
#include "nir_worklist.h"
#include "util/u_dynarray.h"

#include <stdlib.h>

struct intrinsic_info {
   nir_variable_mode mode; /* 0 if the mode is obtained from the deref. */
   nir_intrinsic_op op;
   bool is_atomic;
   /* Indices into nir_intrinsic::src[] or -1 if not applicable. */
   int resource_src; /* resource (e.g. from vulkan_resource_index) */
   int base_src; /* offset which it loads/stores from */
   int deref_src; /* deref which is loads/stores from */
   int value_src; /* the data it is storing */
};

static const struct intrinsic_info *
get_info(nir_intrinsic_op op) {
   switch (op) {
#define INFO(mode, op, atomic, res, base, deref, val) \
case nir_intrinsic_##op: {\
   static const struct intrinsic_info op##_info = {mode, nir_intrinsic_##op, atomic, res, base, deref, val};\
   return &op##_info;\
}
#define LOAD(mode, op, res, base, deref) INFO(mode, load_##op, false, res, base, deref, -1)
#define STORE(mode, op, res, base, deref, val) INFO(mode, store_##op, false, res, base, deref, val)
#define ATOMIC(mode, type, op, res, base, deref, val) INFO(mode, type##_atomic_##op, true, res, base, deref, val)
   LOAD(nir_var_mem_push_const, push_constant, -1, 0, -1)
   LOAD(nir_var_mem_ubo, ubo, 0, 1, -1)
   LOAD(nir_var_mem_ssbo, ssbo, 0, 1, -1)
   STORE(nir_var_mem_ssbo, ssbo, 1, 2, -1, 0)
   LOAD(0, deref, -1, -1, 0)
   STORE(0, deref, -1, -1, 0, 1)
   LOAD(nir_var_mem_shared, shared, -1, 0, -1)
   STORE(nir_var_mem_shared, shared, -1, 1, -1, 0)
   LOAD(nir_var_mem_global, global, -1, 0, -1)
   STORE(nir_var_mem_global, global, -1, 1, -1, 0)
   ATOMIC(nir_var_mem_ssbo, ssbo, add, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, imin, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, umin, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, imax, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, umax, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, and, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, or, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, xor, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, exchange, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, comp_swap, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, fadd, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, fmin, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, fmax, 0, 1, -1, 2)
   ATOMIC(nir_var_mem_ssbo, ssbo, fcomp_swap, 0, 1, -1, 2)
   ATOMIC(0, deref, add, -1, -1, 0, 1)
   ATOMIC(0, deref, imin, -1, -1, 0, 1)
   ATOMIC(0, deref, umin, -1, -1, 0, 1)
   ATOMIC(0, deref, imax, -1, -1, 0, 1)
   ATOMIC(0, deref, umax, -1, -1, 0, 1)
   ATOMIC(0, deref, and, -1, -1, 0, 1)
   ATOMIC(0, deref, or, -1, -1, 0, 1)
   ATOMIC(0, deref, xor, -1, -1, 0, 1)
   ATOMIC(0, deref, exchange, -1, -1, 0, 1)
   ATOMIC(0, deref, comp_swap, -1, -1, 0, 1)
   ATOMIC(0, deref, fadd, -1, -1, 0, 1)
   ATOMIC(0, deref, fmin, -1, -1, 0, 1)
   ATOMIC(0, deref, fmax, -1, -1, 0, 1)
   ATOMIC(0, deref, fcomp_swap, -1, -1, 0, 1)
   ATOMIC(nir_var_mem_shared, shared, add, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, imin, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, umin, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, imax, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, umax, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, and, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, or, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, xor, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, exchange, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, comp_swap, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, fadd, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, fmin, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, fmax, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_shared, shared, fcomp_swap, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, add, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, imin, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, umin, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, imax, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, umax, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, and, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, or, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, xor, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, exchange, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, comp_swap, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, fadd, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, fmin, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, fmax, -1, 0, -1, 1)
   ATOMIC(nir_var_mem_global, global, fcomp_swap, -1, 0, -1, 1)
   default:
      break;
#undef ATOMIC
#undef STORE
#undef LOAD
#undef INFO
   }
   return NULL;
}

/*
 * Information used to compare memory operations.
 * It canonically represents an offset as:
 * `offset_defs[0]*offset_defs_mul[0] + offset_defs[1]*offset_defs_mul[1] + ...`
 * "offset_defs" is sorted in ascenting order by the ssa definition's index.
 * "resource" or "var" may be NULL.
 */
struct entry_key {
   nir_ssa_def *resource;
   nir_variable *var;
   unsigned offset_def_count;
   nir_ssa_def **offset_defs;
   uint64_t *offset_defs_mul;
};

/* Information on a single memory operation. */
struct entry {
   struct list_head head;
   unsigned index;

   struct entry_key *key;
   union {
      uint64_t offset; /* sign-extended */
      int64_t offset_signed;
   };
   uint32_t align_mul;
   uint32_t align_offset;

   nir_instr *instr;
   nir_intrinsic_instr *intrin;
   const struct intrinsic_info *info;
   enum gl_access_qualifier access;
   bool is_store;

   nir_deref_instr *deref;
};

struct vectorize_ctx {
   nir_variable_mode modes;
   nir_should_vectorize_mem_func callback;
   nir_variable_mode robust_modes;
   struct list_head entries[nir_num_variable_modes];
   struct hash_table *loads[nir_num_variable_modes];
   struct hash_table *stores[nir_num_variable_modes];
};

static uint32_t hash_entry_key(const void *key_)
{
   /* this is careful to not include pointers in the hash calculation so that
    * the order of the hash table walk is deterministic */
   struct entry_key *key = (struct entry_key*)key_;

   uint32_t hash = 0;
   if (key->resource)
      hash = XXH32(&key->resource->index, sizeof(key->resource->index), hash);
   if (key->var) {
      hash = XXH32(&key->var->index, sizeof(key->var->index), hash);
      unsigned mode = key->var->data.mode;
      hash = XXH32(&mode, sizeof(mode), hash);
   }

   for (unsigned i = 0; i < key->offset_def_count; i++)
      hash = XXH32(&key->offset_defs[i]->index, sizeof(key->offset_defs[i]->index), hash);

   hash = XXH32(key->offset_defs_mul, key->offset_def_count * sizeof(uint64_t), hash);

   return hash;
}

static bool entry_key_equals(const void *a_, const void *b_)
{
   struct entry_key *a = (struct entry_key*)a_;
   struct entry_key *b = (struct entry_key*)b_;

   if (a->var != b->var || a->resource != b->resource)
      return false;

   if (a->offset_def_count != b->offset_def_count)
      return false;

   size_t offset_def_size = a->offset_def_count * sizeof(nir_ssa_def *);
   size_t offset_def_mul_size = a->offset_def_count * sizeof(uint64_t);
   if (a->offset_def_count &&
       (memcmp(a->offset_defs, b->offset_defs, offset_def_size) ||
        memcmp(a->offset_defs_mul, b->offset_defs_mul, offset_def_mul_size)))
      return false;

   return true;
}

static void delete_entry_dynarray(struct hash_entry *entry)
{
   struct util_dynarray *arr = (struct util_dynarray *)entry->data;
   ralloc_free(arr);
}

static int sort_entries(const void *a_, const void *b_)
{
   struct entry *a = *(struct entry*const*)a_;
   struct entry *b = *(struct entry*const*)b_;

   if (a->offset_signed > b->offset_signed)
      return 1;
   else if (a->offset_signed < b->offset_signed)
      return -1;
   else
      return 0;
}

static unsigned
get_bit_size(struct entry *entry)
{
   unsigned size = entry->is_store ?
                   entry->intrin->src[entry->info->value_src].ssa->bit_size :
                   entry->intrin->dest.ssa.bit_size;
   return size == 1 ? 32u : size;
}

/* If "def" is from an alu instruction with the opcode "op" and one of it's
 * sources is a constant, update "def" to be the non-constant source, fill "c"
 * with the constant and return true. */
static bool
parse_alu(nir_ssa_def **def, nir_op op, uint64_t *c)
{
   nir_ssa_scalar scalar;
   scalar.def = *def;
   scalar.comp = 0;

   if (!nir_ssa_scalar_is_alu(scalar) || nir_ssa_scalar_alu_op(scalar) != op)
      return false;

   nir_ssa_scalar src0 = nir_ssa_scalar_chase_alu_src(scalar, 0);
   nir_ssa_scalar src1 = nir_ssa_scalar_chase_alu_src(scalar, 1);
   if (op != nir_op_ishl && nir_ssa_scalar_is_const(src0) && src1.comp == 0) {
      *c = nir_ssa_scalar_as_uint(src0);
      *def = src1.def;
   } else if (nir_ssa_scalar_is_const(src1) && src0.comp == 0) {
      *c = nir_ssa_scalar_as_uint(src1);
      *def = src0.def;
   } else {
      return false;
   }
   return true;
}

/* Parses an offset expression such as "a * 16 + 4" and "(a * 16 + 4) * 64 + 32". */
static void
parse_offset(nir_ssa_def **base, uint64_t *base_mul, uint64_t *offset)
{
   if ((*base)->parent_instr->type == nir_instr_type_load_const) {
      *offset = nir_src_comp_as_uint(nir_src_for_ssa(*base), 0);
      *base = NULL;
      return;
   }

   uint64_t mul = 1;
   uint64_t add = 0;
   bool progress = false;
   do {
      uint64_t mul2 = 1, add2 = 0;

      progress = parse_alu(base, nir_op_imul, &mul2);
      mul *= mul2;

      mul2 = 0;
      progress |= parse_alu(base, nir_op_ishl, &mul2);
      mul <<= mul2;

      progress |= parse_alu(base, nir_op_iadd, &add2);
      add += add2 * mul;
   } while (progress);

   *base_mul = mul;
   *offset = add;
}

static unsigned
type_scalar_size_bytes(const struct glsl_type *type)
{
   assert(glsl_type_is_vector_or_scalar(type) ||
          glsl_type_is_matrix(type));
   return glsl_type_is_boolean(type) ? 4u : glsl_get_bit_size(type) / 8u;
}

static uint64_t
mask_sign_extend(uint64_t val, unsigned bit_size)
{
   return (int64_t)(val << (64 - bit_size)) >> (64 - bit_size);
}

static unsigned
add_to_entry_key(nir_ssa_def **offset_defs, uint64_t *offset_defs_mul,
                 unsigned offset_def_count, nir_ssa_def *def, uint64_t mul)
{
   mul = mask_sign_extend(mul, def->bit_size);

   for (unsigned i = 0; i <= offset_def_count; i++) {
      if (i == offset_def_count || def->index > offset_defs[i]->index) {
         /* insert before i */
         memmove(offset_defs + i + 1, offset_defs + i,
                 (offset_def_count - i) * sizeof(nir_ssa_def *));
         memmove(offset_defs_mul + i + 1, offset_defs_mul + i,
                 (offset_def_count - i) * sizeof(uint64_t));
         offset_defs[i] = def;
         offset_defs_mul[i] = mul;
         return 1;
      } else if (def->index == offset_defs[i]->index) {
         /* merge with offset_def at i */
         offset_defs_mul[i] += mul;
         return 0;
      }
   }
   unreachable("Unreachable.");
   return 0;
}

static struct entry_key *
create_entry_key_from_deref(void *mem_ctx,
                            struct vectorize_ctx *ctx,
                            nir_deref_path *path,
                            uint64_t *offset_base)
{
   unsigned path_len = 0;
   while (path->path[path_len])
      path_len++;

   nir_ssa_def *offset_defs_stack[32];
   uint64_t offset_defs_mul_stack[32];
   nir_ssa_def **offset_defs = offset_defs_stack;
   uint64_t *offset_defs_mul = offset_defs_mul_stack;
   if (path_len > 32) {
      offset_defs = malloc(path_len * sizeof(nir_ssa_def *));
      offset_defs_mul = malloc(path_len * sizeof(uint64_t));
   }
   unsigned offset_def_count = 0;

   struct entry_key* key = ralloc(mem_ctx, struct entry_key);
   key->resource = NULL;
   key->var = NULL;
   *offset_base = 0;

   for (unsigned i = 0; i < path_len; i++) {
      nir_deref_instr *parent = i ? path->path[i - 1] : NULL;
      nir_deref_instr *deref = path->path[i];

      switch (deref->deref_type) {
      case nir_deref_type_var: {
         assert(!parent);
         key->var = deref->var;
         break;
      }
      case nir_deref_type_array:
      case nir_deref_type_ptr_as_array: {
         assert(parent);
         nir_ssa_def *index = deref->arr.index.ssa;
         uint32_t stride = nir_deref_instr_array_stride(deref);

         nir_ssa_def *base = index;
         uint64_t offset = 0, base_mul = 1;
         parse_offset(&base, &base_mul, &offset);
         offset = mask_sign_extend(offset, index->bit_size);

         *offset_base += offset * stride;
         if (base) {
            offset_def_count += add_to_entry_key(offset_defs, offset_defs_mul,
                                                 offset_def_count,
                                                 base, base_mul * stride);
         }
         break;
      }
      case nir_deref_type_struct: {
         assert(parent);
         int offset = glsl_get_struct_field_offset(parent->type, deref->strct.index);
         *offset_base += offset;
         break;
      }
      case nir_deref_type_cast: {
         if (!parent)
            key->resource = deref->parent.ssa;
         break;
      }
      default:
         unreachable("Unhandled deref type");
      }
   }

   key->offset_def_count = offset_def_count;
   key->offset_defs = ralloc_array(mem_ctx, nir_ssa_def *, offset_def_count);
   key->offset_defs_mul = ralloc_array(mem_ctx, uint64_t, offset_def_count);
   memcpy(key->offset_defs, offset_defs, offset_def_count * sizeof(nir_ssa_def *));
   memcpy(key->offset_defs_mul, offset_defs_mul, offset_def_count * sizeof(uint64_t));

   if (offset_defs != offset_defs_stack)
      free(offset_defs);
   if (offset_defs_mul != offset_defs_mul_stack)
      free(offset_defs_mul);

   return key;
}

static unsigned
parse_entry_key_from_offset(struct entry_key *key, unsigned size, unsigned left,
                            nir_ssa_def *base, uint64_t base_mul, uint64_t *offset)
{
   uint64_t new_mul;
   uint64_t new_offset;
   parse_offset(&base, &new_mul, &new_offset);
   *offset += new_offset * base_mul;

   if (!base)
      return 0;

   base_mul *= new_mul;

   assert(left >= 1);

   if (left >= 2) {
      nir_ssa_scalar scalar;
      scalar.def = base;
      scalar.comp = 0;
      if (nir_ssa_scalar_is_alu(scalar) && nir_ssa_scalar_alu_op(scalar) == nir_op_iadd) {
         nir_ssa_scalar src0 = nir_ssa_scalar_chase_alu_src(scalar, 0);
         nir_ssa_scalar src1 = nir_ssa_scalar_chase_alu_src(scalar, 1);
         if (src0.comp == 0 && src1.comp == 0) {
            unsigned amount = parse_entry_key_from_offset(key, size, left - 1, src0.def, base_mul, offset);
            amount += parse_entry_key_from_offset(key, size + amount, left - amount, src1.def, base_mul, offset);
            return amount;
         }
      }
   }

   return add_to_entry_key(key->offset_defs, key->offset_defs_mul, size, base, base_mul);
}

static struct entry_key *
create_entry_key_from_offset(void *mem_ctx, nir_ssa_def *base, uint64_t base_mul, uint64_t *offset)
{
   struct entry_key *key = ralloc(mem_ctx, struct entry_key);
   key->resource = NULL;
   key->var = NULL;
   if (base) {
      nir_ssa_def *offset_defs[32];
      uint64_t offset_defs_mul[32];
      key->offset_defs = offset_defs;
      key->offset_defs_mul = offset_defs_mul;

      key->offset_def_count = parse_entry_key_from_offset(key, 0, 32, base, base_mul, offset);

      key->offset_defs = ralloc_array(mem_ctx, nir_ssa_def *, key->offset_def_count);
      key->offset_defs_mul = ralloc_array(mem_ctx, uint64_t, key->offset_def_count);
      memcpy(key->offset_defs, offset_defs, key->offset_def_count * sizeof(nir_ssa_def *));
      memcpy(key->offset_defs_mul, offset_defs_mul, key->offset_def_count * sizeof(uint64_t));
   } else {
      key->offset_def_count = 0;
      key->offset_defs = NULL;
      key->offset_defs_mul = NULL;
   }
   return key;
}

static nir_variable_mode
get_variable_mode(struct entry *entry)
{
   if (entry->info->mode)
      return entry->info->mode;
   assert(entry->deref && util_bitcount(entry->deref->modes) == 1);
   return entry->deref->modes;
}

static unsigned
mode_to_index(nir_variable_mode mode)
{
   assert(util_bitcount(mode) == 1);

   /* Globals and SSBOs should be tracked together */
   if (mode == nir_var_mem_global)
      mode = nir_var_mem_ssbo;

   return ffs(mode) - 1;
}

static nir_variable_mode
aliasing_modes(nir_variable_mode modes)
{
   /* Global and SSBO can alias */
   if (modes & (nir_var_mem_ssbo | nir_var_mem_global))
      modes |= nir_var_mem_ssbo | nir_var_mem_global;
   return modes;
}

static void
calc_alignment(struct entry *entry)
{
   uint32_t align_mul = 31;
   for (unsigned i = 0; i < entry->key->offset_def_count; i++) {
      if (entry->key->offset_defs_mul[i])
         align_mul = MIN2(align_mul, ffsll(entry->key->offset_defs_mul[i]));
   }

   entry->align_mul = 1u << (align_mul - 1);
   bool has_align = nir_intrinsic_infos[entry->intrin->intrinsic].index_map[NIR_INTRINSIC_ALIGN_MUL];
   if (!has_align || entry->align_mul >= nir_intrinsic_align_mul(entry->intrin)) {
      entry->align_offset = entry->offset % entry->align_mul;
   } else {
      entry->align_mul = nir_intrinsic_align_mul(entry->intrin);
      entry->align_offset = nir_intrinsic_align_offset(entry->intrin);
   }
}

static struct entry *
create_entry(struct vectorize_ctx *ctx,
             const struct intrinsic_info *info,
             nir_intrinsic_instr *intrin)
{
   struct entry *entry = rzalloc(ctx, struct entry);
   entry->intrin = intrin;
   entry->instr = &intrin->instr;
   entry->info = info;
   entry->is_store = entry->info->value_src >= 0;

   if (entry->info->deref_src >= 0) {
      entry->deref = nir_src_as_deref(intrin->src[entry->info->deref_src]);
      nir_deref_path path;
      nir_deref_path_init(&path, entry->deref, NULL);
      entry->key = create_entry_key_from_deref(entry, ctx, &path, &entry->offset);
      nir_deref_path_finish(&path);
   } else {
      nir_ssa_def *base = entry->info->base_src >= 0 ?
                          intrin->src[entry->info->base_src].ssa : NULL;
      uint64_t offset = 0;
      if (nir_intrinsic_has_base(intrin))
         offset += nir_intrinsic_base(intrin);
      entry->key = create_entry_key_from_offset(entry, base, 1, &offset);
      entry->offset = offset;

      if (base)
         entry->offset = mask_sign_extend(entry->offset, base->bit_size);
   }

   if (entry->info->resource_src >= 0)
      entry->key->resource = intrin->src[entry->info->resource_src].ssa;

   if (nir_intrinsic_has_access(intrin))
      entry->access = nir_intrinsic_access(intrin);
   else if (entry->key->var)
      entry->access = entry->key->var->data.access;

   uint32_t restrict_modes = nir_var_shader_in | nir_var_shader_out;
   restrict_modes |= nir_var_shader_temp | nir_var_function_temp;
   restrict_modes |= nir_var_uniform | nir_var_mem_push_const;
   restrict_modes |= nir_var_system_value | nir_var_mem_shared;
   if (get_variable_mode(entry) & restrict_modes)
      entry->access |= ACCESS_RESTRICT;

   calc_alignment(entry);

   return entry;
}

static nir_deref_instr *
cast_deref(nir_builder *b, unsigned num_components, unsigned bit_size, nir_deref_instr *deref)
{
   if (glsl_get_components(deref->type) == num_components &&
       type_scalar_size_bytes(deref->type)*8u == bit_size)
      return deref;

   enum glsl_base_type types[] = {
      GLSL_TYPE_UINT8, GLSL_TYPE_UINT16, GLSL_TYPE_UINT, GLSL_TYPE_UINT64};
   enum glsl_base_type base = types[ffs(bit_size / 8u) - 1u];
   const struct glsl_type *type = glsl_vector_type(base, num_components);

   if (deref->type == type)
      return deref;

   return nir_build_deref_cast(b, &deref->dest.ssa, deref->modes, type, 0);
}

/* Return true if "new_bit_size" is a usable bit size for a vectorized load/store
 * of "low" and "high". */
static bool
new_bitsize_acceptable(struct vectorize_ctx *ctx, unsigned new_bit_size,
                       struct entry *low, struct entry *high, unsigned size)
{
   if (size % new_bit_size != 0)
      return false;

   unsigned new_num_components = size / new_bit_size;
   if (!nir_num_components_valid(new_num_components))
      return false;

   unsigned high_offset = high->offset_signed - low->offset_signed;

   /* check nir_extract_bits limitations */
   unsigned common_bit_size = MIN2(get_bit_size(low), get_bit_size(high));
   common_bit_size = MIN2(common_bit_size, new_bit_size);
   if (high_offset > 0)
      common_bit_size = MIN2(common_bit_size, (1u << (ffs(high_offset * 8) - 1)));
   if (new_bit_size / common_bit_size > NIR_MAX_VEC_COMPONENTS)
      return false;

   if (!ctx->callback(low->align_mul,
                      low->align_offset,
                      new_bit_size, new_num_components,
                      low->intrin, high->intrin))
      return false;

   if (low->is_store) {
      unsigned low_size = low->intrin->num_components * get_bit_size(low);
      unsigned high_size = high->intrin->num_components * get_bit_size(high);

      if (low_size % new_bit_size != 0)
         return false;
      if (high_size % new_bit_size != 0)
         return false;

      unsigned write_mask = nir_intrinsic_write_mask(low->intrin);
      if (!nir_component_mask_can_reinterpret(write_mask, get_bit_size(low), new_bit_size))
         return false;

      write_mask = nir_intrinsic_write_mask(high->intrin);
      if (!nir_component_mask_can_reinterpret(write_mask, get_bit_size(high), new_bit_size))
         return false;
   }

   return true;
}

static nir_deref_instr *subtract_deref(nir_builder *b, nir_deref_instr *deref, int64_t offset)
{
   /* avoid adding another deref to the path */
   if (deref->deref_type == nir_deref_type_ptr_as_array &&
       nir_src_is_const(deref->arr.index) &&
       offset % nir_deref_instr_array_stride(deref) == 0) {
      unsigned stride = nir_deref_instr_array_stride(deref);
      nir_ssa_def *index = nir_imm_intN_t(b, nir_src_as_int(deref->arr.index) - offset / stride,
                                          deref->dest.ssa.bit_size);
      return nir_build_deref_ptr_as_array(b, nir_deref_instr_parent(deref), index);
   }

   if (deref->deref_type == nir_deref_type_array &&
       nir_src_is_const(deref->arr.index)) {
      nir_deref_instr *parent = nir_deref_instr_parent(deref);
      unsigned stride = glsl_get_explicit_stride(parent->type);
      if (offset % stride == 0)
         return nir_build_deref_array_imm(
            b, parent, nir_src_as_int(deref->arr.index) - offset / stride);
   }


   deref = nir_build_deref_cast(b, &deref->dest.ssa, deref->modes,
                                glsl_scalar_type(GLSL_TYPE_UINT8), 1);
   return nir_build_deref_ptr_as_array(
      b, deref, nir_imm_intN_t(b, -offset, deref->dest.ssa.bit_size));
}

static void
vectorize_loads(nir_builder *b, struct vectorize_ctx *ctx,
                struct entry *low, struct entry *high,
                struct entry *first, struct entry *second,
                unsigned new_bit_size, unsigned new_num_components,
                unsigned high_start)
{
   unsigned low_bit_size = get_bit_size(low);
   unsigned high_bit_size = get_bit_size(high);
   bool low_bool = low->intrin->dest.ssa.bit_size == 1;
   bool high_bool = high->intrin->dest.ssa.bit_size == 1;
   nir_ssa_def *data = &first->intrin->dest.ssa;

   b->cursor = nir_after_instr(first->instr);

   /* update the load's destination size and extract data for each of the original loads */
   data->num_components = new_num_components;
   data->bit_size = new_bit_size;

   nir_ssa_def *low_def = nir_extract_bits(
      b, &data, 1, 0, low->intrin->num_components, low_bit_size);
   nir_ssa_def *high_def = nir_extract_bits(
      b, &data, 1, high_start, high->intrin->num_components, high_bit_size);

   /* convert booleans */
   low_def = low_bool ? nir_i2b(b, low_def) : nir_mov(b, low_def);
   high_def = high_bool ? nir_i2b(b, high_def) : nir_mov(b, high_def);

   /* update uses */
   if (first == low) {
      nir_ssa_def_rewrite_uses_after(&low->intrin->dest.ssa, nir_src_for_ssa(low_def),
                                     high_def->parent_instr);
      nir_ssa_def_rewrite_uses(&high->intrin->dest.ssa, nir_src_for_ssa(high_def));
   } else {
      nir_ssa_def_rewrite_uses(&low->intrin->dest.ssa, nir_src_for_ssa(low_def));
      nir_ssa_def_rewrite_uses_after(&high->intrin->dest.ssa, nir_src_for_ssa(high_def),
                                     high_def->parent_instr);
   }

   /* update the intrinsic */
   first->intrin->num_components = new_num_components;

   const struct intrinsic_info *info = first->info;

   /* update the offset */
   if (first != low && info->base_src >= 0) {
      /* let nir_opt_algebraic() remove this addition. this doesn't have much
       * issues with subtracting 16 from expressions like "(i + 1) * 16" because
       * nir_opt_algebraic() turns them into "i * 16 + 16" */
      b->cursor = nir_before_instr(first->instr);

      nir_ssa_def *new_base = first->intrin->src[info->base_src].ssa;
      new_base = nir_iadd_imm(b, new_base, -(int)(high_start / 8u));

      nir_instr_rewrite_src(first->instr, &first->intrin->src[info->base_src],
                            nir_src_for_ssa(new_base));
   }

   /* update the deref */
   if (info->deref_src >= 0) {
      b->cursor = nir_before_instr(first->instr);

      nir_deref_instr *deref = nir_src_as_deref(first->intrin->src[info->deref_src]);
      if (first != low && high_start != 0)
         deref = subtract_deref(b, deref, high_start / 8u);
      first->deref = cast_deref(b, new_num_components, new_bit_size, deref);

      nir_instr_rewrite_src(first->instr, &first->intrin->src[info->deref_src],
                            nir_src_for_ssa(&first->deref->dest.ssa));
   }

   /* update base/align */
   if (first != low && nir_intrinsic_has_base(first->intrin))
      nir_intrinsic_set_base(first->intrin, nir_intrinsic_base(low->intrin));

   if (nir_intrinsic_has_range_base(first->intrin)) {
      uint32_t low_base = nir_intrinsic_range_base(low->intrin);
      uint32_t high_base = nir_intrinsic_range_base(high->intrin);
      uint32_t low_end = low_base + nir_intrinsic_range(low->intrin);
      uint32_t high_end = high_base + nir_intrinsic_range(high->intrin);

      nir_intrinsic_set_range_base(first->intrin, low_base);
      nir_intrinsic_set_range(first->intrin, MAX2(low_end, high_end) - low_base);
   }

   first->key = low->key;
   first->offset = low->offset;

   first->align_mul = low->align_mul;
   first->align_offset = low->align_offset;

   nir_instr_remove(second->instr);
}

static void
vectorize_stores(nir_builder *b, struct vectorize_ctx *ctx,
                 struct entry *low, struct entry *high,
                 struct entry *first, struct entry *second,
                 unsigned new_bit_size, unsigned new_num_components,
                 unsigned high_start)
{
   ASSERTED unsigned low_size = low->intrin->num_components * get_bit_size(low);
   assert(low_size % new_bit_size == 0);

   b->cursor = nir_before_instr(second->instr);

   /* get new writemasks */
   uint32_t low_write_mask = nir_intrinsic_write_mask(low->intrin);
   uint32_t high_write_mask = nir_intrinsic_write_mask(high->intrin);
   low_write_mask = nir_component_mask_reinterpret(low_write_mask,
                                                   get_bit_size(low),
                                                   new_bit_size);
   high_write_mask = nir_component_mask_reinterpret(high_write_mask,
                                                    get_bit_size(high),
                                                    new_bit_size);
   high_write_mask <<= high_start / new_bit_size;

   uint32_t write_mask = low_write_mask | high_write_mask;

   /* convert booleans */
   nir_ssa_def *low_val = low->intrin->src[low->info->value_src].ssa;
   nir_ssa_def *high_val = high->intrin->src[high->info->value_src].ssa;
   low_val = low_val->bit_size == 1 ? nir_b2i(b, low_val, 32) : low_val;
   high_val = high_val->bit_size == 1 ? nir_b2i(b, high_val, 32) : high_val;

   /* combine the data */
   nir_ssa_def *data_channels[NIR_MAX_VEC_COMPONENTS];
   for (unsigned i = 0; i < new_num_components; i++) {
      bool set_low = low_write_mask & (1 << i);
      bool set_high = high_write_mask & (1 << i);

      if (set_low && (!set_high || low == second)) {
         unsigned offset = i * new_bit_size;
         data_channels[i] = nir_extract_bits(b, &low_val, 1, offset, 1, new_bit_size);
      } else if (set_high) {
         assert(!set_low || high == second);
         unsigned offset = i * new_bit_size - high_start;
         data_channels[i] = nir_extract_bits(b, &high_val, 1, offset, 1, new_bit_size);
      } else {
         data_channels[i] = nir_ssa_undef(b, 1, new_bit_size);
      }
   }
   nir_ssa_def *data = nir_vec(b, data_channels, new_num_components);

   /* update the intrinsic */
   nir_intrinsic_set_write_mask(second->intrin, write_mask);
   second->intrin->num_components = data->num_components;

   const struct intrinsic_info *info = second->info;
   assert(info->value_src >= 0);
   nir_instr_rewrite_src(second->instr, &second->intrin->src[info->value_src],
                         nir_src_for_ssa(data));

   /* update the offset */
   if (second != low && info->base_src >= 0)
      nir_instr_rewrite_src(second->instr, &second->intrin->src[info->base_src],
                            low->intrin->src[info->base_src]);

   /* update the deref */
   if (info->deref_src >= 0) {
      b->cursor = nir_before_instr(second->instr);
      second->deref = cast_deref(b, new_num_components, new_bit_size,
                                 nir_src_as_deref(low->intrin->src[info->deref_src]));
      nir_instr_rewrite_src(second->instr, &second->intrin->src[info->deref_src],
                            nir_src_for_ssa(&second->deref->dest.ssa));
   }

   /* update base/align */
   if (second != low && nir_intrinsic_has_base(second->intrin))
      nir_intrinsic_set_base(second->intrin, nir_intrinsic_base(low->intrin));

   second->key = low->key;
   second->offset = low->offset;

   second->align_mul = low->align_mul;
   second->align_offset = low->align_offset;

   list_del(&first->head);
   nir_instr_remove(first->instr);
}

/* Returns true if it can prove that "a" and "b" point to different resources. */
static bool
resources_different(nir_ssa_def *a, nir_ssa_def *b)
{
   if (!a || !b)
      return false;

   if (a->parent_instr->type == nir_instr_type_load_const &&
       b->parent_instr->type == nir_instr_type_load_const) {
      return nir_src_as_uint(nir_src_for_ssa(a)) != nir_src_as_uint(nir_src_for_ssa(b));
   }

   if (a->parent_instr->type == nir_instr_type_intrinsic &&
       b->parent_instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *aintrin = nir_instr_as_intrinsic(a->parent_instr);
      nir_intrinsic_instr *bintrin = nir_instr_as_intrinsic(b->parent_instr);
      if (aintrin->intrinsic == nir_intrinsic_vulkan_resource_index &&
          bintrin->intrinsic == nir_intrinsic_vulkan_resource_index) {
         return nir_intrinsic_desc_set(aintrin) != nir_intrinsic_desc_set(bintrin) ||
                nir_intrinsic_binding(aintrin) != nir_intrinsic_binding(bintrin) ||
                resources_different(aintrin->src[0].ssa, bintrin->src[0].ssa);
      }
   }

   return false;
}

static int64_t
compare_entries(struct entry *a, struct entry *b)
{
   if (!entry_key_equals(a->key, b->key))
      return INT64_MAX;
   return b->offset_signed - a->offset_signed;
}

static bool
may_alias(struct entry *a, struct entry *b)
{
   assert(mode_to_index(get_variable_mode(a)) ==
          mode_to_index(get_variable_mode(b)));

   /* if the resources/variables are definitively different and both have
    * ACCESS_RESTRICT, we can assume they do not alias. */
   bool res_different = a->key->var != b->key->var ||
                        resources_different(a->key->resource, b->key->resource);
   if (res_different && (a->access & ACCESS_RESTRICT) && (b->access & ACCESS_RESTRICT))
      return false;

   /* we can't compare offsets if the resources/variables might be different */
   if (a->key->var != b->key->var || a->key->resource != b->key->resource)
      return true;

   /* use adjacency information */
   /* TODO: we can look closer at the entry keys */
   int64_t diff = compare_entries(a, b);
   if (diff != INT64_MAX) {
      /* with atomics, intrin->num_components can be 0 */
      if (diff < 0)
         return llabs(diff) < MAX2(b->intrin->num_components, 1u) * (get_bit_size(b) / 8u);
      else
         return diff < MAX2(a->intrin->num_components, 1u) * (get_bit_size(a) / 8u);
   }

   /* TODO: we can use deref information */

   return true;
}

static bool
check_for_aliasing(struct vectorize_ctx *ctx, struct entry *first, struct entry *second)
{
   nir_variable_mode mode = get_variable_mode(first);
   if (mode & (nir_var_uniform | nir_var_system_value |
               nir_var_mem_push_const | nir_var_mem_ubo))
      return false;

   unsigned mode_index = mode_to_index(mode);
   if (first->is_store) {
      /* find first entry that aliases "first" */
      list_for_each_entry_from(struct entry, next, first, &ctx->entries[mode_index], head) {
         if (next == first)
            continue;
         if (next == second)
            return false;
         if (may_alias(first, next))
            return true;
      }
   } else {
      /* find previous store that aliases this load */
      list_for_each_entry_from_rev(struct entry, prev, second, &ctx->entries[mode_index], head) {
         if (prev == second)
            continue;
         if (prev == first)
            return false;
         if (prev->is_store && may_alias(second, prev))
            return true;
      }
   }

   return false;
}

static bool
check_for_robustness(struct vectorize_ctx *ctx, struct entry *low)
{
   nir_variable_mode mode = get_variable_mode(low);
   if (mode & ctx->robust_modes) {
      unsigned low_bit_size = get_bit_size(low);
      unsigned low_size = low->intrin->num_components * low_bit_size;

      /* don't attempt to vectorize accesses if the offset can overflow. */
      /* TODO: handle indirect accesses. */
      return low->offset_signed < 0 && low->offset_signed + low_size >= 0;
   }

   return false;
}

static bool
is_strided_vector(const struct glsl_type *type)
{
   if (glsl_type_is_vector(type)) {
      unsigned explicit_stride = glsl_get_explicit_stride(type);
      return explicit_stride != 0 && explicit_stride !=
             type_scalar_size_bytes(glsl_get_array_element(type));
   } else {
      return false;
   }
}

static bool
try_vectorize(nir_function_impl *impl, struct vectorize_ctx *ctx,
              struct entry *low, struct entry *high,
              struct entry *first, struct entry *second)
{
   if (!(get_variable_mode(first) & ctx->modes) ||
       !(get_variable_mode(second) & ctx->modes))
      return false;

   if (check_for_aliasing(ctx, first, second))
      return false;

   if (check_for_robustness(ctx, low))
      return false;

   /* we can only vectorize non-volatile loads/stores of the same type and with
    * the same access */
   if (first->info != second->info || first->access != second->access ||
       (first->access & ACCESS_VOLATILE) || first->info->is_atomic)
      return false;

   /* don't attempt to vectorize accesses of row-major matrix columns */
   if (first->deref) {
      const struct glsl_type *first_type = first->deref->type;
      const struct glsl_type *second_type = second->deref->type;
      if (is_strided_vector(first_type) || is_strided_vector(second_type))
         return false;
   }

   /* gather information */
   uint64_t diff = high->offset_signed - low->offset_signed;
   unsigned low_bit_size = get_bit_size(low);
   unsigned high_bit_size = get_bit_size(high);
   unsigned low_size = low->intrin->num_components * low_bit_size;
   unsigned high_size = high->intrin->num_components * high_bit_size;
   unsigned new_size = MAX2(diff * 8u + high_size, low_size);

   /* find a good bit size for the new load/store */
   unsigned new_bit_size = 0;
   if (new_bitsize_acceptable(ctx, low_bit_size, low, high, new_size)) {
      new_bit_size = low_bit_size;
   } else if (low_bit_size != high_bit_size &&
              new_bitsize_acceptable(ctx, high_bit_size, low, high, new_size)) {
      new_bit_size = high_bit_size;
   } else {
      new_bit_size = 64;
      for (; new_bit_size >= 8; new_bit_size /= 2) {
         /* don't repeat trying out bitsizes */
         if (new_bit_size == low_bit_size || new_bit_size == high_bit_size)
            continue;
         if (new_bitsize_acceptable(ctx, new_bit_size, low, high, new_size))
            break;
      }
      if (new_bit_size < 8)
         return false;
   }
   unsigned new_num_components = new_size / new_bit_size;

   /* vectorize the loads/stores */
   nir_builder b;
   nir_builder_init(&b, impl);

   if (first->is_store)
      vectorize_stores(&b, ctx, low, high, first, second,
                       new_bit_size, new_num_components, diff * 8u);
   else
      vectorize_loads(&b, ctx, low, high, first, second,
                      new_bit_size, new_num_components, diff * 8u);

   return true;
}

static bool
update_align(struct entry *entry)
{
   if (nir_intrinsic_has_align_mul(entry->intrin) &&
       (entry->align_mul != nir_intrinsic_align_mul(entry->intrin) ||
        entry->align_offset != nir_intrinsic_align_offset(entry->intrin))) {
      nir_intrinsic_set_align(entry->intrin, entry->align_mul, entry->align_offset);
      return true;
   }
   return false;
}

static bool
vectorize_entries(struct vectorize_ctx *ctx, nir_function_impl *impl, struct hash_table *ht)
{
   if (!ht)
      return false;

   bool progress = false;
   hash_table_foreach(ht, entry) {
      struct util_dynarray *arr = entry->data;
      if (!arr->size)
         continue;

      qsort(util_dynarray_begin(arr),
            util_dynarray_num_elements(arr, struct entry *),
            sizeof(struct entry *), &sort_entries);

      unsigned num_entries = util_dynarray_num_elements(arr, struct entry *);

      for (unsigned first_idx = 0; first_idx < num_entries; first_idx++) {
         struct entry *low = *util_dynarray_element(arr, struct entry *, first_idx);
         if (!low)
            continue;

         for (unsigned second_idx = first_idx + 1; second_idx < num_entries; second_idx++) {
            struct entry *high = *util_dynarray_element(arr, struct entry *, second_idx);
            if (!high)
               continue;

            uint64_t diff = high->offset_signed - low->offset_signed;
            if (diff > get_bit_size(low) / 8u * low->intrin->num_components)
               break;

            struct entry *first = low->index < high->index ? low : high;
            struct entry *second = low->index < high->index ? high : low;

            if (try_vectorize(impl, ctx, low, high, first, second)) {
               low = low->is_store ? second : first;
               *util_dynarray_element(arr, struct entry *, second_idx) = NULL;
               progress = true;
            }
         }

         *util_dynarray_element(arr, struct entry *, first_idx) = low;
      }

      util_dynarray_foreach(arr, struct entry *, elem) {
         if (*elem)
            progress |= update_align(*elem);
      }
   }

   _mesa_hash_table_clear(ht, delete_entry_dynarray);

   return progress;
}

static bool
handle_barrier(struct vectorize_ctx *ctx, bool *progress, nir_function_impl *impl, nir_instr *instr)
{
   unsigned modes = 0;
   bool acquire = true;
   bool release = true;
   if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_group_memory_barrier:
      case nir_intrinsic_memory_barrier:
         modes = nir_var_mem_ssbo | nir_var_mem_shared | nir_var_mem_global;
         break;
      /* prevent speculative loads/stores */
      case nir_intrinsic_discard_if:
      case nir_intrinsic_discard:
      case nir_intrinsic_terminate_if:
      case nir_intrinsic_terminate:
         modes = nir_var_all;
         break;
      case nir_intrinsic_demote_if:
      case nir_intrinsic_demote:
         acquire = false;
         modes = nir_var_all;
         break;
      case nir_intrinsic_memory_barrier_buffer:
         modes = nir_var_mem_ssbo | nir_var_mem_global;
         break;
      case nir_intrinsic_memory_barrier_shared:
         modes = nir_var_mem_shared;
         break;
      case nir_intrinsic_scoped_barrier:
         if (nir_intrinsic_memory_scope(intrin) == NIR_SCOPE_NONE)
            break;

         modes = nir_intrinsic_memory_modes(intrin) & (nir_var_mem_ssbo |
                                                       nir_var_mem_shared |
                                                       nir_var_mem_global);
         acquire = nir_intrinsic_memory_semantics(intrin) & NIR_MEMORY_ACQUIRE;
         release = nir_intrinsic_memory_semantics(intrin) & NIR_MEMORY_RELEASE;
         switch (nir_intrinsic_memory_scope(intrin)) {
         case NIR_SCOPE_INVOCATION:
         case NIR_SCOPE_SUBGROUP:
            /* a barier should never be required for correctness with these scopes */
            modes = 0;
            break;
         default:
            break;
         }
         break;
      default:
         return false;
      }
   } else if (instr->type == nir_instr_type_call) {
      modes = nir_var_all;
   } else {
      return false;
   }

   while (modes) {
      unsigned mode_index = u_bit_scan(&modes);
      if ((1 << mode_index) == nir_var_mem_global) {
         /* Global should be rolled in with SSBO */
         assert(list_is_empty(&ctx->entries[mode_index]));
         assert(ctx->loads[mode_index] == NULL);
         assert(ctx->stores[mode_index] == NULL);
         continue;
      }

      if (acquire)
         *progress |= vectorize_entries(ctx, impl, ctx->loads[mode_index]);
      if (release)
         *progress |= vectorize_entries(ctx, impl, ctx->stores[mode_index]);
   }

   return true;
}

static bool
process_block(nir_function_impl *impl, struct vectorize_ctx *ctx, nir_block *block)
{
   bool progress = false;

   for (unsigned i = 0; i < nir_num_variable_modes; i++) {
      list_inithead(&ctx->entries[i]);
      if (ctx->loads[i])
         _mesa_hash_table_clear(ctx->loads[i], delete_entry_dynarray);
      if (ctx->stores[i])
         _mesa_hash_table_clear(ctx->stores[i], delete_entry_dynarray);
   }

   /* create entries */
   unsigned next_index = 0;

   nir_foreach_instr_safe(instr, block) {
      if (handle_barrier(ctx, &progress, impl, instr))
         continue;

      /* gather information */
      if (instr->type != nir_instr_type_intrinsic)
         continue;
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

      const struct intrinsic_info *info = get_info(intrin->intrinsic); 
      if (!info)
         continue;

      nir_variable_mode mode = info->mode;
      if (!mode)
         mode = nir_src_as_deref(intrin->src[info->deref_src])->modes;
      if (!(mode & aliasing_modes(ctx->modes)))
         continue;
      unsigned mode_index = mode_to_index(mode);

      /* create entry */
      struct entry *entry = create_entry(ctx, info, intrin);
      entry->index = next_index++;

      list_addtail(&entry->head, &ctx->entries[mode_index]);

      /* add the entry to a hash table */

      struct hash_table *adj_ht = NULL;
      if (entry->is_store) {
         if (!ctx->stores[mode_index])
            ctx->stores[mode_index] = _mesa_hash_table_create(ctx, &hash_entry_key, &entry_key_equals);
         adj_ht = ctx->stores[mode_index];
      } else {
         if (!ctx->loads[mode_index])
            ctx->loads[mode_index] = _mesa_hash_table_create(ctx, &hash_entry_key, &entry_key_equals);
         adj_ht = ctx->loads[mode_index];
      }

      uint32_t key_hash = hash_entry_key(entry->key);
      struct hash_entry *adj_entry = _mesa_hash_table_search_pre_hashed(adj_ht, key_hash, entry->key);
      struct util_dynarray *arr;
      if (adj_entry && adj_entry->data) {
         arr = (struct util_dynarray *)adj_entry->data;
      } else {
         arr = ralloc(ctx, struct util_dynarray);
         util_dynarray_init(arr, arr);
         _mesa_hash_table_insert_pre_hashed(adj_ht, key_hash, entry->key, arr);
      }
      util_dynarray_append(arr, struct entry *, entry);
   }

   /* sort and combine entries */
   for (unsigned i = 0; i < nir_num_variable_modes; i++) {
      progress |= vectorize_entries(ctx, impl, ctx->loads[i]);
      progress |= vectorize_entries(ctx, impl, ctx->stores[i]);
   }

   return progress;
}

bool
nir_opt_load_store_vectorize(nir_shader *shader, nir_variable_mode modes,
                             nir_should_vectorize_mem_func callback,
                             nir_variable_mode robust_modes)
{
   bool progress = false;

   struct vectorize_ctx *ctx = rzalloc(NULL, struct vectorize_ctx);
   ctx->modes = modes;
   ctx->callback = callback;
   ctx->robust_modes = robust_modes;

   nir_shader_index_vars(shader, modes);

   nir_foreach_function(function, shader) {
      if (function->impl) {
         if (modes & nir_var_function_temp)
            nir_function_impl_index_vars(function->impl);

         nir_foreach_block(block, function->impl)
            progress |= process_block(function->impl, ctx, block);

         nir_metadata_preserve(function->impl,
                               nir_metadata_block_index |
                               nir_metadata_dominance |
                               nir_metadata_live_ssa_defs);
      }
   }

   ralloc_free(ctx);
   return progress;
}
