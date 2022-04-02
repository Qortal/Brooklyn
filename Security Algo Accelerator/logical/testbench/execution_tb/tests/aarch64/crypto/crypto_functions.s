//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
//            (C) COPYRIGHT 2012-2020 ARM Limited or its affiliates.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//
//            Release Information : HERCULESAE-MP106-r0p1-00eac0
//
//------------------------------------------------------------------------------
// Description:
//
//   This file defines assembler functions that are called from C in the main
//   crypto test
//------------------------------------------------------------------------------

        .section .text, "ax", %progbits

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

        .macro aes128_key_expand_step rcon
        TBL     v18.16B, {v16.16B}, v19.16B
        AESE    v18.16B, v17.16B
        MOVI    v20.4S, #\rcon
        EOR     v18.16B, v18.16B, v20.16B
        EXT     v20.16B, v17.16B, v16.16B, #12
        EOR     v16.16B, v16.16B, v20.16B
        EXT     v20.16B, v17.16B, v20.16B, #12
        EOR     v16.16B, v16.16B, v20.16B
        EXT     v20.16B, v17.16B, v20.16B, #12
        EOR     v16.16B, v16.16B, v20.16B
        EOR     v16.16B, v16.16B, v18.16B
        .endm


//------------------------------------------------------------------------------
// Function: aes128_key_expand
//------------------------------------------------------------------------------

.global aes128_key_expand
.type   aes128_key_expand, %function

aes128_key_expand:
        // C arguments:
        //      r0: const unsigned char *key_in
        //      r1: unsigned char *key_out
        // Return: void
        LD1     {v16.16B}, [x0]
        MOVZ    w2, #0x0e0d
        DUP     v17.16B, wzr
        movk    w2, #0xc0f, lsl #16
        DUP     v19.4S, w2
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x01
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x02
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x04
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x08
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x10
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x20
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x40
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x80
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x1B
        ST1     {v16.16B}, [x1], #16
        aes128_key_expand_step  0x36
        ST1     {v16.16B}, [x1]
        RET

//------------------------------------------------------------------------------
// Function: aes128_ecb_encrypt
//------------------------------------------------------------------------------

.global aes128_ecb_encrypt
.type   aes128_ecb_encrypt, %function

aes128_ecb_encrypt:
        // C arguments:
        //      r0: const unsigned char *key
        //      r1: const unsigned char *in_data
        //      r2: unsigned char *out_data
        //      r3: unsigned int size
        // Return: void
        LD1     {v16.16B-v19.16B}, [x0], #64
        LD1     {v20.16B-v23.16B}, [x0], #64
        LD1     {v24.16B-v26.16B}, [x0]

aes128_ecb_enc_loop:
        // Load data
        LD1     {v0.16B-v1.16B}, [x1], #32
        // Round 1
        AESE    v0.16B, v16.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v16.16B
        AESMC   v1.16B, v1.16B
        // Round 2
        AESE    v0.16B, v17.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v17.16B
        AESMC   v1.16B, v1.16B
        // Round 3
        AESE    v0.16B, v18.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v18.16B
        AESMC   v1.16B, v1.16B
        // Round 4
        AESE    v0.16B, v19.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v19.16B
        AESMC   v1.16B, v1.16B
        // Round 5
        AESE    v0.16B, v20.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v20.16B
        AESMC   v1.16B, v1.16B
        // Round 6
        AESE    v0.16B, v21.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v21.16B
        AESMC   v1.16B, v1.16B
        // Round 7
        AESE    v0.16B, v22.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v22.16B
        AESMC   v1.16B, v1.16B
        // Round 8
        AESE    v0.16B, v23.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v23.16B
        AESMC   v1.16B, v1.16B
        // Round 9
        AESE    v0.16B, v24.16B
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, v24.16B
        AESMC   v1.16B, v1.16B
        // Round 10
        AESE    v0.16B, v25.16B
        PRFM    PLDL1KEEP, [x1, #64]
        AESE    v1.16B, v25.16B
        EOR     v0.16B, v0.16B, v26.16B
        SUBS    x3, x3, #16
        ST1     {v0.16B}, [x2], #16
        B.EQ    aes128_ecb_enc_exit

        EOR     v1.16B, v1.16B, v26.16B
        SUBS    x3, x3, #16
        ST1     {v1.16B}, [x2], #16
        B.GT    aes128_ecb_enc_loop

aes128_ecb_enc_exit:
        RET

