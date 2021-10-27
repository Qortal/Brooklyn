//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
//            (C) COPYRIGHT 2013-2020 ARM Limited or its affiliates.
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

        .section testcode, "ax", %progbits

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------

        .macro aes128_key_expand_step rcon
        VTBL.8   d26, {d22, d23}, d28
        VTBL.8   d27, {d22, d23}, d29
        AESE.8   q13, q12
        VMOV.I32 q15, #\rcon
        VEOR     q13, q13, q15
        VEXT.8   q15, q12, q11, #12
        VEOR     q11, q11, q15
        VEXT.8   q15, q12, q15, #12
        VEOR     q11, q11, q15
        VEXT.8   q15, q12, q15, #12
        VEOR     q11, q11, q15
        VEOR     q11, q11, q13
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
        VLD1.8  {d22-d23}, [r0]
        MOV     r2, #0
        VDUP.8  q12, r2
        LDR     r2, =0x0c0f0e0d
        VDUP.32 q14, r2
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x01
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x02
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x04
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x08
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x10
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x20
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x40
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x80
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x1B
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8
        aes128_key_expand_step  0x36
        VST1.8 {d22}, [r1]
        ADD     r1, r1, #8
        VST1.8 {d23}, [r1]
        ADD     r1, r1, #8

        BX      lr

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
        VLD1.8  {d10-d13}, [r0]
        ADD     r0, r0, #32
        VLD1.8  {d14-d17}, [r0]
        ADD     r0, r0, #32
        VLD1.8  {d18-d21}, [r0]
        ADD     r0, r0, #32
        VLD1.8  {d22-d25}, [r0]
        ADD     r0, r0, #32
        VLD1.8  {d26-d29}, [r0]
        ADD     r0, r0, #32
        VLD1.8  {d30-d31}, [r0]

aes128_ecb_enc_loop:
        // Load data
        VLD1.8  {d0-d3}, [r1]
        ADD     r1, r1, #32
        // Round 1
        AESE.8  q0, q5
        AESMC.8 q0, q0
        AESE.8  q1, q5
        AESMC.8 q1, q1
        // Round 2
        AESE.8  q0, q6
        AESMC.8 q0, q0
        AESE.8  q1, q6
        AESMC.8 q1, q1
        // Round 3
        AESE.8  q0, q7
        AESMC.8 q0, q0
        AESE.8  q1, q7
        AESMC.8 q1, q1
        // Round 4
        AESE.8  q0, q8
        AESMC.8 q0, q0
        AESE.8  q1, q8
        AESMC.8 q1, q1
        // Round 5
        AESE.8  q0, q9
        AESMC.8 q0, q0
        AESE.8  q1, q9
        AESMC.8 q1, q1
        // Round 6
        AESE.8  q0, q10
        AESMC.8 q0, q0
        AESE.8  q1, q10
        AESMC.8 q1, q1
        // Round 7
        AESE.8  q0, q11
        AESMC.8 q0, q0
        AESE.8  q1, q11
        AESMC.8 q1, q1
        // Round 8
        AESE.8  q0, q12
        AESMC.8 q0, q0
        AESE.8  q1, q12
        AESMC.8 q1, q1
        // Round 9
        AESE.8  q0, q13
        AESMC.8 q0, q0
        AESE.8  q1, q13
        AESMC.8 q1, q1

        // Round 10
        AESE.8  q0, q14
        PLD     [r1, #64]
        AESE.8  q1, q14
        VEOR    q0, q0, q15
        SUBS    r3, r3, #16
        VST1.8  {d0-d1}, [r2]
        ADD     r2, r2, #16
        BEQ     aes128_ecb_enc_exit

        VEOR    q1, q1, q15
        SUBS    r3, r3, #16
        VST1.8  {d2-d3}, [r2]
        ADD     r2, r2, #16
        BGT     aes128_ecb_enc_loop

aes128_ecb_enc_exit:
        BX      lr

