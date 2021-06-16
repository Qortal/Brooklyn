/*
 * Copyright (C) 2021 Collabora, Ltd.
 * Copyright (C) 2021 Alyssa Rosenzweig <alyssa@rosenzweig.io>
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

#include "compiler.h"

static bool
bi_takes_fabs(bi_instr *I, bi_index repl, unsigned s)
{
        switch (I->op) {
        case BI_OPCODE_FCMP_V2F16:
        case BI_OPCODE_FMAX_V2F16:
        case BI_OPCODE_FMIN_V2F16:
                /* Encoding restriction: can't have both abs if equal sources */
                return !(I->src[1 - s].abs && bi_is_word_equiv(I->src[1 - s], repl));
        case BI_OPCODE_V2F32_TO_V2F16:
                /* TODO: Needs both match or lower */
                return false;
        case BI_OPCODE_FLOG_TABLE_F32:
                /* TODO: Need to check mode */
                return false;
        default:
                return bi_opcode_props[I->op].abs & BITFIELD_BIT(s);
        }
}

static bool
bi_takes_fneg(bi_instr *I, unsigned s)
{
        switch (I->op) {
        case BI_OPCODE_CUBE_SSEL:
        case BI_OPCODE_CUBE_TSEL:
        case BI_OPCODE_CUBEFACE:
                /* TODO: Needs match or lower */
                return false;
        case BI_OPCODE_FREXPE_F32:
        case BI_OPCODE_FREXPE_V2F16:
        case BI_OPCODE_FLOG_TABLE_F32:
                /* TODO: Need to check mode */
                return false;
        default:
                return bi_opcode_props[I->op].neg & BITFIELD_BIT(s);
        }
}

static bool
bi_is_fabsneg(bi_instr *I)
{
        return (I->op == BI_OPCODE_FADD_F32 || I->op == BI_OPCODE_FADD_V2F16) &&
                (I->src[1].type == BI_INDEX_CONSTANT && I->src[1].value == 0) &&
                (I->clamp == BI_CLAMP_NONE);
}

static enum bi_swizzle
bi_compose_swizzle_16(enum bi_swizzle a, enum bi_swizzle b)
{
        assert(a <= BI_SWIZZLE_H11);
        assert(b <= BI_SWIZZLE_H11);

        bool al = (a & BI_SWIZZLE_H10);
        bool ar = (a & BI_SWIZZLE_H01);
        bool bl = (b & BI_SWIZZLE_H10);
        bool br = (b & BI_SWIZZLE_H01);

        return ((al ? br : bl) ? BI_SWIZZLE_H10 : 0) |
               ((ar ? br : bl) ? BI_SWIZZLE_H01 : 0);
}

/* Like bi_replace_index, but composes instead of overwrites */

static inline bi_index
bi_compose_float_index(bi_index old, bi_index repl)
{
        /* abs(-x) = abs(+x) so ignore repl.neg if old.abs is set, otherwise
         * -(-x) = x but -(+x) = +(-x) so need to exclusive-or the negates */
        repl.neg = old.neg ^ (repl.neg && !old.abs);

        /* +/- abs(+/- abs(x)) = +/- abs(x), etc so just or the two */
        repl.abs |= old.abs;

        /* Use the old swizzle to select from the replacement swizzle */
        repl.swizzle = bi_compose_swizzle_16(old.swizzle, repl.swizzle);

        return repl;
}

void
bi_opt_mod_prop_forward(bi_context *ctx)
{
        bi_instr **lut = calloc(sizeof(bi_instr *), ((ctx->ssa_alloc + 1) << 2));

        bi_foreach_instr_global_safe(ctx, I) {
                if (bi_is_ssa(I->dest[0]))
                        lut[bi_word_node(I->dest[0])] = I;

                bi_foreach_src(I, s) {
                        if (!bi_is_ssa(I->src[s]))
                                continue;

                        bi_instr *mod = lut[bi_word_node(I->src[s])];

                        if (!mod)
                                continue;

                        if (bi_opcode_props[mod->op].size != bi_opcode_props[I->op].size)
                                continue;

                        if (bi_is_fabsneg(mod)) {
                                if (mod->src[0].abs && !bi_takes_fabs(I, mod->src[0], s))
                                        continue;

                                if (mod->src[0].neg && !bi_takes_fneg(I, s))
                                        continue;

                                I->src[s] = bi_compose_float_index(I->src[s], mod->src[0]);
                        }
                }
        }

        free(lut);
}

/* RSCALE has restrictions on how the clamp may be used, only used for
 * specialized transcendental sequences that set the clamp explicitly anyway */

static bool
bi_takes_clamp(bi_instr *I)
{
        switch (I->op) {
        case BI_OPCODE_FMA_RSCALE_F32:
        case BI_OPCODE_FMA_RSCALE_V2F16:
        case BI_OPCODE_FADD_RSCALE_F32:
                return false;
        default:
                return bi_opcode_props[I->op].clamp;
        }
}

/* Treating clamps as functions, compute the composition f circ g. For {NONE,
 * SAT, SAT_SIGNED, CLAMP_POS}, anything left- or right-composed with NONE is
 * unchanged, anything composed with itself is unchanged, and any two
 * nontrivial distinct clamps compose to SAT (left as an exercise) */

static enum bi_clamp
bi_compose_clamp(enum bi_clamp f, enum bi_clamp g)
{
        return  (f == BI_CLAMP_NONE) ? g :
                (g == BI_CLAMP_NONE) ? f :
                (f == g)             ? f :
                BI_CLAMP_CLAMP_0_1;
}

static bool
bi_is_fclamp(bi_instr *I)
{
        return (I->op == BI_OPCODE_FADD_F32 || I->op == BI_OPCODE_FADD_V2F16) &&
                (!I->src[0].abs && !I->src[0].neg) &&
                (I->src[1].type == BI_INDEX_CONSTANT && I->src[1].value == 0) &&
                (I->clamp != BI_CLAMP_NONE);
}

static bool
bi_optimizer_clamp(bi_instr *I, bi_instr *use)
{
        if (!bi_is_fclamp(use)) return false;
        if (!bi_takes_clamp(I)) return false;
        if (use->src[0].neg || use->src[0].abs) return false;

        I->clamp = bi_compose_clamp(I->clamp, use->clamp);
        I->dest[0] = use->dest[0];
        return true;
}

void
bi_opt_mod_prop_backward(bi_context *ctx)
{
        unsigned count = ((ctx->ssa_alloc + 1) << 2);
        bi_instr **uses = calloc(count, sizeof(*uses));
        BITSET_WORD *multiple = calloc(BITSET_WORDS(count), sizeof(*multiple));

        bi_foreach_instr_global_rev(ctx, I) {
                bi_foreach_src(I, s) {
                        if (bi_is_ssa(I->src[s])) {
                                unsigned v = bi_word_node(I->src[s]);

                                if (uses[v])
                                        BITSET_SET(multiple, v);
                                else
                                        uses[v] = I;
                        }
                }

                if (!bi_is_ssa(I->dest[0]))
                        continue;

                bi_instr *use = uses[bi_word_node(I->dest[0])];

                if (!use || BITSET_TEST(multiple, bi_word_node(I->dest[0])))
                        continue;

                if (bi_opcode_props[use->op].size != bi_opcode_props[I->op].size)
                        continue;

                /* Destination has a single use, try to propagate */
                if (bi_optimizer_clamp(I, use)) {
                        bi_remove_instruction(use);
                        continue;
                }
        }

        free(uses);
        free(multiple);
}
