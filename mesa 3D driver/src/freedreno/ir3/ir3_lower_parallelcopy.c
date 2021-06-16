/*
 * Copyright (C) 2021 Valve Corporation
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
 */

#include "ir3_ra.h"
#include "ir3_shader.h"

struct copy_src {
	unsigned flags;
	union {
		uint32_t imm;
		physreg_t reg;
		unsigned const_num;
	};
};

struct copy_entry {
	physreg_t dst;
	unsigned flags;
	bool done;

	struct copy_src src;
};

static unsigned
copy_entry_size(const struct copy_entry *entry)
{
	return (entry->flags & IR3_REG_HALF) ? 1 : 2;
}

static struct copy_src
get_copy_src(const struct ir3_register *reg, unsigned offset)
{
	if (reg->flags & IR3_REG_IMMED) {
		return (struct copy_src) {
			.flags = IR3_REG_IMMED,
			.imm = reg->uim_val,
		};
	} else if (reg->flags & IR3_REG_CONST) {
		return (struct copy_src) {
			.flags = IR3_REG_CONST,
			.const_num = reg->num,
		};
	} else {
		return (struct copy_src) {
			.flags = 0,
			.reg = ra_reg_get_physreg(reg) + offset,
		};
	}
}

static void
do_xor(struct ir3_instruction *instr, unsigned dst_num, unsigned src1_num, unsigned src2_num, unsigned flags)
{
	struct ir3_instruction *xor = ir3_instr_create(instr->block, OPC_XOR_B, 3);
	struct ir3_register *dst = ir3_reg_create(xor, dst_num, flags | IR3_REG_DEST);
	dst->wrmask = 1;
	struct ir3_register *src1 = ir3_reg_create(xor, src1_num, flags);
	src1->wrmask = 1;
	struct ir3_register *src2 = ir3_reg_create(xor, src2_num, flags);
	src2->wrmask = 1;

	ir3_instr_move_before(xor, instr);
}

static void
do_swap(struct ir3_instruction *instr, const struct copy_entry *entry)
{
	assert(!entry->src.flags);
	/* TODO implement shared swaps */
	assert(!(entry->flags & IR3_REG_SHARED));

	if (entry->flags & IR3_REG_HALF) {
		/* We currently make sure to never emit parallel copies where the
		 * source/destination is a half-reg above the range accessable to half
		 * registers. However, when a full-reg source overlaps a half-reg
		 * destination or vice versa, it can be very, very complicated to come
		 * up with a series of "legal" swaps and copies to resolve the
		 * parallel copy. So here we provide a fallback to implement the
		 * "illegal" swap instead. This may also be useful for implementing
		 * "spilling" half-regs to the inaccessable space.
		 */
		if (entry->src.reg >= RA_HALF_SIZE) {
			/* Choose a temporary that doesn't overlap src or dst */
			physreg_t tmp = entry->dst < 2 ? 2 : 0;

			/* Swap src and the temporary */
			do_swap(instr, &(struct copy_entry) {
				.src = { .reg = entry->src.reg & ~1u },
				.dst = tmp,
				.flags = entry->flags & ~IR3_REG_HALF,
			});

			/* Do the original swap with src replaced with tmp */
			do_swap(instr, &(struct copy_entry) {
				.src = { .reg = tmp + (entry->src.reg & 1) },
				.dst = entry->dst,
				.flags = entry->flags,
			});

			/* Swap src and the temporary back */
			do_swap(instr, &(struct copy_entry) {
				.src = { .reg = entry->src.reg & ~1u },
				.dst = tmp,
				.flags = entry->flags & ~IR3_REG_HALF,
			});
			return;
		}

		/* If dst is not addressable, we only need to swap the arguments and
		 * let the case above handle it.
		 */
		if (entry->dst >= RA_HALF_SIZE) {
			do_swap(instr, &(struct copy_entry) {
				.src = { .reg = entry->dst },
				.dst = entry->src.reg,
				.flags = entry->flags,
			});
			return;
		}
	}

	unsigned src_num = ra_physreg_to_num(entry->src.reg, entry->flags);
	unsigned dst_num = ra_physreg_to_num(entry->dst, entry->flags);

	do_xor(instr, dst_num, dst_num, src_num, entry->flags);
	do_xor(instr, src_num, src_num, dst_num, entry->flags);
	do_xor(instr, dst_num, dst_num, src_num, entry->flags);
}

static void
do_copy(struct ir3_instruction *instr, const struct copy_entry *entry)
{
	/* TODO implement shared copies */
	assert(!(entry->flags & IR3_REG_SHARED));

	if (entry->flags & IR3_REG_HALF) {
		/* See do_swap() for why this is here. */
		if (entry->dst >= RA_HALF_SIZE) {
			/* TODO: is there a hw instruction we can use for this case? */
			physreg_t tmp = !entry->src.flags && entry->src.reg < 2 ? 2 : 0;

			do_swap(instr, &(struct copy_entry) {
				.src = { .reg = entry->dst & ~1u },
				.dst = tmp,
				.flags = entry->flags & ~IR3_REG_HALF,
			});

			do_copy(instr, &(struct copy_entry) {
				.src = entry->src,
				.dst = tmp + (entry->dst & 1),
				.flags = entry->flags,
			});

			do_swap(instr, &(struct copy_entry) {
				.src = { .reg = entry->dst & ~1u },
				.dst = tmp,
				.flags = entry->flags & ~IR3_REG_HALF,
			});
			return;
		}

		if (!entry->src.flags && entry->src.reg >= RA_HALF_SIZE) {
			unsigned src_num =
				ra_physreg_to_num(entry->src.reg & ~1u, entry->flags & ~IR3_REG_HALF);
			unsigned dst_num = ra_physreg_to_num(entry->dst, entry->flags);
			
			if (entry->src.reg % 2 == 0) {
				/* cov.u32u16 dst, src */
				struct ir3_instruction *cov = ir3_instr_create(instr->block, OPC_MOV, 2);
				ir3_reg_create(cov, dst_num, entry->flags | IR3_REG_DEST)->wrmask = 1;
				ir3_reg_create(cov, src_num, entry->flags & ~IR3_REG_HALF)->wrmask = 1;
				cov->cat1.dst_type = TYPE_U16;
				cov->cat1.src_type = TYPE_U32;
				ir3_instr_move_before(cov, instr);
			} else {
				/* shr.b dst, src, h(16) */
				struct ir3_instruction *shr = ir3_instr_create(instr->block, OPC_SHR_B, 3);
				ir3_reg_create(shr, dst_num, entry->flags | IR3_REG_DEST)->wrmask = 1;
				ir3_reg_create(shr, src_num, entry->flags & ~IR3_REG_HALF)->wrmask = 1;
				ir3_reg_create(shr, 0, entry->flags | IR3_REG_IMMED)->uim_val = 16;
				ir3_instr_move_before(shr, instr);
			}
			return;
		}
	}

	unsigned src_num = ra_physreg_to_num(entry->src.reg, entry->flags);
	unsigned dst_num = ra_physreg_to_num(entry->dst, entry->flags);

	struct ir3_instruction *mov = ir3_instr_create(instr->block, OPC_MOV, 2);
	ir3_reg_create(mov, dst_num, entry->flags | IR3_REG_DEST)->wrmask = 1;
	ir3_reg_create(mov, src_num, entry->flags | entry->src.flags)->wrmask = 1;
	mov->cat1.dst_type = (entry->flags & IR3_REG_HALF) ? TYPE_U16 : TYPE_U32;
	mov->cat1.src_type = (entry->flags & IR3_REG_HALF) ? TYPE_U16 : TYPE_U32;
	if (entry->src.flags & IR3_REG_IMMED)
		mov->regs[1]->uim_val = entry->src.imm;
	else if (entry->src.flags & IR3_REG_CONST)
		mov->regs[1]->num = entry->src.const_num;
	ir3_instr_move_before(mov, instr);
}

struct copy_ctx {
	/* For each physreg, the number of pending copy entries that use it as a
	 * source. Once this drops to zero, then the physreg is unblocked and can
	 * be moved to.
	 */
	unsigned physreg_use_count[RA_MAX_FILE_SIZE];

	/* For each physreg, the pending copy_entry that uses it as a dest. */
	struct copy_entry *physreg_dst[RA_MAX_FILE_SIZE];

	struct copy_entry entries[RA_MAX_FILE_SIZE];
	unsigned entry_count;
};

static bool
entry_blocked(struct copy_entry *entry, struct copy_ctx *ctx)
{
	for (unsigned i = 0; i < copy_entry_size(entry); i++) {
		if (ctx->physreg_use_count[entry->dst + i] != 0)
			return true;
	}

	return false;
}

static void
split_32bit_copy(struct copy_ctx *ctx, struct copy_entry *entry)
{
	assert(!entry->done);
	assert(!(entry->flags & (IR3_REG_IMMED | IR3_REG_CONST)));
	assert(copy_entry_size(entry) == 2);
	struct copy_entry *new_entry = &ctx->entries[ctx->entry_count++];

	new_entry->dst = entry->dst + 1;
	new_entry->src.flags = entry->src.flags;
	new_entry->src.reg = entry->src.reg + 1;
	new_entry->done = false;
	entry->flags |= IR3_REG_HALF;
	new_entry->flags = entry->flags;
	ctx->physreg_dst[entry->dst + 1] = new_entry;
}

static void
_handle_copies(struct ir3_instruction *instr, struct copy_ctx *ctx)
{
	/* Set up the bookkeeping */
	memset(ctx->physreg_dst, 0, sizeof(ctx->physreg_dst));
	memset(ctx->physreg_use_count, 0, sizeof(ctx->physreg_use_count));

	for (unsigned i = 0; i < ctx->entry_count; i++) {
		struct copy_entry *entry = &ctx->entries[i];
		for (unsigned j = 0; j < copy_entry_size(entry); j++) {
			if (!entry->src.flags)
				ctx->physreg_use_count[entry->src.reg + j]++;

			/* Copies should not have overlapping destinations. */
			assert(!ctx->physreg_dst[entry->dst + j]);
			ctx->physreg_dst[entry->dst + j] = entry;
		}
	}

	bool progress = true;
	while (progress) {
		progress = false;

		/* Step 1: resolve paths in the transfer graph. This means finding
		 * copies whose destination aren't blocked by something else and then
		 * emitting them, continuing this process until every copy is blocked
		 * and there are only cycles left.
		 *
		 * TODO: We should note that src is also available in dst to unblock
		 * cycles that src is involved in.
		 */

		for (unsigned i = 0; i < ctx->entry_count; i++) {
			struct copy_entry *entry = &ctx->entries[i];
			if (!entry->done && !entry_blocked(entry, ctx)) {
				entry->done = true;
				progress = true;
				do_copy(instr, entry);
				for (unsigned j = 0; j < copy_entry_size(entry); j++) {
					if (!entry->src.flags)
						ctx->physreg_use_count[entry->src.reg + j]--;
					ctx->physreg_dst[entry->dst + j] = NULL;
				}
			}
		}

		if (progress)
			continue;

		/* Step 2: Find partially blocked copies and split them. In the
		 * mergedregs case, we can 32-bit copies which are only blocked on one
		 * 16-bit half, and splitting them helps get things moving.
		 *
		 * We can skip splitting copies if the source isn't a register,
		 * however, because it does not unblock anything and therefore doesn't
		 * contribute to making forward progress with step 1. These copies
		 * should still be resolved eventually in step 1 because they can't be
		 * part of a cycle.
		 */
		for (unsigned i = 0; i < ctx->entry_count; i++) {
			struct copy_entry *entry = &ctx->entries[i];
			if (entry->done || entry->flags & IR3_REG_HALF)
				continue;

			if (((ctx->physreg_use_count[entry->dst] == 0 ||
				  ctx->physreg_use_count[entry->dst + 1] == 0)) &&
				 !(entry->flags & (IR3_REG_IMMED | IR3_REG_CONST))) {
				split_32bit_copy(ctx, entry);
				progress = true;
			}
		}
	}

	/* Step 3: resolve cycles through swapping.
	 *
	 * At this point, the transfer graph should consist of only cycles.
	 * The reason is that, given any physreg n_1 that's the source of a
	 * remaining entry, it has a destination n_2, which (because every
	 * copy is blocked) is the source of some other copy whose destination
	 * is n_3, and so we can follow the chain until we get a cycle. If we
	 * reached some other node than n_1:
	 *
	 *  n_1 -> n_2 -> ... -> n_i
	 *          ^             |
	 *          |-------------|
	 *
	 *  then n_2 would be the destination of 2 copies, which is illegal
	 *  (checked above in an assert). So n_1 must be part of a cycle:
	 *
	 *  n_1 -> n_2 -> ... -> n_i
	 *  ^                     |
	 *  |---------------------|
	 *
	 *  and this must be only cycle n_1 is involved in, because any other
	 *  path starting from n_1 would also have to end in n_1, resulting in
	 *  a node somewhere along the way being the destination of 2 copies
	 *  when the 2 paths merge.
	 *
	 *  The way we resolve the cycle is through picking a copy (n_1, n_2)
	 *  and swapping n_1 and n_2. This moves n_1 to n_2, so n_2 is taken
	 *  out of the cycle:
	 *
	 *  n_1 -> ... -> n_i
	 *  ^              |
	 *  |--------------|
	 *
	 *  and we can keep repeating this until the cycle is empty.
	 */

	for (unsigned i = 0; i < ctx->entry_count; i++) {
		struct copy_entry *entry = &ctx->entries[i];
		if (entry->done)
			continue;

		assert(!entry->src.flags);

		/* catch trivial copies */
		if (entry->dst == entry->src.reg) {
			entry->done = true;
			continue;
		}

		do_swap(instr, entry);

		/* Split any blocking copies whose sources are only partially
		 * contained within our destination.
		 */
		if (entry->flags & IR3_REG_HALF) {
			for (unsigned j = 0; j < ctx->entry_count; j++) {
				struct copy_entry *blocking = &ctx->entries[j];

				if (blocking->done)
					continue;

				if (blocking->src.reg <= entry->dst &&
					blocking->src.reg + 1 >= entry->dst &&
					!(blocking->flags & IR3_REG_HALF)) {
					split_32bit_copy(ctx, blocking);
				}
			}
		}

		/* Update sources of blocking copies.
		 *
		 * Note: at this point, every blocking copy's source should be
		 * contained within our destination.
		 */
		for (unsigned j = 0; j < ctx->entry_count; j++) {
			struct copy_entry *blocking = &ctx->entries[j];
			if (blocking->src.reg >= entry->dst &&
				blocking->src.reg < entry->dst + copy_entry_size(entry)) {
				blocking->src.reg = entry->src.reg + (blocking->src.reg - entry->dst);
			}
		}
	}
}

static void
handle_copies(struct ir3_instruction *instr, struct copy_entry *entries,
			  unsigned entry_count, bool mergedregs)
{
	struct copy_ctx ctx;	

	if (mergedregs) {
		/* Half regs and full regs are in the same file, so handle everything
		 * at once.
		 */
		memcpy(ctx.entries, entries, sizeof(struct copy_entry) * entry_count);
		ctx.entry_count = entry_count;
		_handle_copies(instr, &ctx);
	} else {
		/* There may be both half copies and full copies, so we have to split
		 * them up since they don't interfere.
		 */
		ctx.entry_count = 0;
		for (unsigned i = 0; i < entry_count; i++) {
			if (entries[i].flags & IR3_REG_HALF)
				ctx.entries[ctx.entry_count++] = entries[i];
		}
		_handle_copies(instr, &ctx);

		ctx.entry_count = 0;
		for (unsigned i = 0; i < entry_count; i++) {
			if (!(entries[i].flags & IR3_REG_HALF))
				ctx.entries[ctx.entry_count++] = entries[i];
		}
		_handle_copies(instr, &ctx);
	}
}

void
ir3_lower_copies(struct ir3_shader_variant *v)
{
	DECLARE_ARRAY(struct copy_entry, copies);
	copies_count = copies_sz = 0;
	copies = NULL;

	foreach_block (block, &v->ir->block_list) {
		foreach_instr_safe (instr, &block->instr_list) {
			if (instr->opc == OPC_META_PARALLEL_COPY) {
				copies_count = 0;
				for (unsigned i = 0; i < instr->regs_count / 2; i++) {
					struct ir3_register *dst = instr->regs[i];
					struct ir3_register *src = instr->regs[i + instr->regs_count / 2];
					unsigned flags = src->flags & (IR3_REG_HALF | IR3_REG_SHARED);
					for (unsigned j = 0; j < reg_elems(dst); j++) {
						array_insert(NULL, copies, (struct copy_entry) {
							.dst = ra_num_to_physreg(dst->num + j, flags),
							.src = get_copy_src(src, j * reg_elem_size(dst)),
							.flags = flags,
						});
					}
				}
				handle_copies(instr, copies, copies_count, v->mergedregs);
				list_del(&instr->node);
			} else if (instr->opc == OPC_META_COLLECT) {
				copies_count = 0;
				struct ir3_register *dst = instr->regs[0];
				unsigned flags = dst->flags & (IR3_REG_HALF | IR3_REG_SHARED);
				for (unsigned i = 1; i < instr->regs_count; i++) {
					struct ir3_register *src = instr->regs[i];
					array_insert(NULL, copies, (struct copy_entry) {
						.dst = ra_num_to_physreg(dst->num + i - 1, flags),
						.src = get_copy_src(src, 0),
						.flags = flags,
					});
				}
				handle_copies(instr, copies, copies_count, v->mergedregs);
				list_del(&instr->node);
			} else if (instr->opc == OPC_META_SPLIT) {
				copies_count = 0;
				struct ir3_register *dst = instr->regs[0];
				struct ir3_register *src = instr->regs[1];
				unsigned flags = src->flags & (IR3_REG_HALF | IR3_REG_SHARED);
				array_insert(NULL, copies, (struct copy_entry) {
					.dst = ra_reg_get_physreg(dst),
					.src = get_copy_src(src, instr->split.off * reg_elem_size(dst)),
					.flags = flags,
				});
				handle_copies(instr, copies, copies_count, v->mergedregs);
				list_del(&instr->node);
			} else if (instr->opc == OPC_META_PHI) {
				list_del(&instr->node);
			}
		}
	}

	if (copies)
		ralloc_free(copies);
}

