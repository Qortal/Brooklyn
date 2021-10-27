;#-----------------------------------------------------------------------------
;# The confidential and proprietary information contained in this file may
;# only be used by a person authorised under and to the extent permitted
;# by a subsisting licensing agreement from ARM Limited.
;#
;#            (C) COPYRIGHT 2012-2013 ARM Limited.
;#                ALL RIGHTS RESERVED
;#
;# This entire notice must be reproduced on all copies of this file
;# and copies of this file may only be made by a person if such person is
;# permitted to do so under the terms of a subsisting license agreement
;# from ARM Limited.
;#
;#      SVN Information
;#
;#      Checked In          : $Date: 2013-03-19 09:12:51 +0000 (Tue, 19 Mar 2013) $
;#
;#      Revision            : $Revision: 241584 $
;#
;#      Release Information :
;#
;#-----------------------------------------------------------------------------

        .section aes_code, "ax"

        .global aes128_key_expand
        .global aes128_ecb_encrypt

        .align 6
rcon_array:
        .word     0x00000001
        .word     0x00000002
        .word     0x00000004
        .word     0x00000008
        .word     0x00000010
        .word     0x00000020
        .word     0x00000040
        .word     0x00000080
        .word     0x0000001b
        .word     0x00000036

        .align 6
;# void aes128_key_expand(const unsigned char *key_in, unsigned char *key_out)
        .type aes128_key_expand STT_FUNC
aes128_key_expand:
        LD1     {v16.16B}, [x0]
        MOVZ    w2, #0x0e0d
        DUP     v17.16B, wzr
        MOVK    w2, #0x0c0f, lsl #16
        DUP     v19.4S, w2
        ADR     x3, rcon_array
        MOV     w4, #10
exp:
        TBL     v18.16B, {v16.16B}, v19.16B
        LD1R    {v21.4S}, [x3], #4
        AESE    v18.16B, v17.16B
        EXT     v20.16B, v17.16B, v16.16B, #12
        SHA1SU0 v21.4S, v18.4S, v17.4S
        EOR     v22.16B, v16.16B, v20.16B
        ST1     {v16.16B}, [x1], #16
        SHA1SU0 v21.4S, v22.4S, v22.4S

        TBL     v18.16B, {v21.16B}, v19.16B
        LD1R    {v16.4S}, [x3], #4
        AESE    v18.16B, v17.16B
        EXT     v20.16B, v17.16B, v21.16B, #12
        SHA1SU0 v16.4S, v18.4S, v17.4S
        EOR     v22.16B, v21.16B, v20.16B
        ST1     {v21.16B}, [x1], #16
        SUBS    w4, w4, #2
        SHA1SU0 v16.4S, v22.4S, v22.4S

        B.NE    exp
        ST1     {v16.16B}, [x1]
        RET

        .macro aes_enc_round keyreg
        AESE    v0.16B, \keyreg
        AESMC   v0.16B, v0.16B
        AESE    v1.16B, \keyreg
        AESMC   v1.16B, v1.16B
        AESE    v2.16B, \keyreg
        AESMC   v2.16B, v2.16B
        .endm

        .macro aes_dec_round keyreg
        AESD    v0.16B, \keyreg
        AESIMC  v0.16B, v0.16B
        AESD    v1.16B, \keyreg
        AESIMC  v1.16B, v1.16B
        AESD    v2.16B, \keyreg
        AESIMC  v2.16B, v2.16B
        .endm

;# void aes128_ecb_encrypt(const unsigned char *key, const unsigned char *in_data, unsigned char *out_data, unsigned int size)
        .type aes128_ecb_encrypt STT_FUNC
aes128_ecb_encrypt:
        ;# Load the key
        LD1     {v16.16B-v19.16B}, [x0], #64
        LD1     {v20.16B-v23.16B}, [x0], #64
        LD1     {v24.16B-v26.16B}, [x0]

load_ip:
        ;# Load data
        LD1     {v0.16B-v2.16B}, [x1], #48
        ;# Rounds 1-9
        aes_enc_round v16.16B
        aes_enc_round v17.16B
        aes_enc_round v18.16B
        aes_enc_round v19.16B
        aes_enc_round v20.16B
        aes_enc_round v21.16B
        aes_enc_round v22.16B
        aes_enc_round v23.16B
        aes_enc_round v24.16B
        ;# Round 10
        AESE    v0.16B, v25.16B
        PRFM    PLDL1KEEP, [x1, #64]
        EOR     v0.16B, v0.16B, v26.16B
        SUBS    x3, x3, #16
        ST1     {v0.16B}, [x2], #16
        B.EQ    end_enc

        AESE    v1.16B, v25.16B
        EOR     v1.16B, v1.16B, v26.16B
        SUBS    x3, x3, #16
        ST1     {v1.16B}, [x2], #16
        B.EQ    end_enc

        AESE    v2.16B, v25.16B
        EOR     v2.16B, v2.16B, v26.16B
        SUBS    x3, x3, #16
        ST1     {v2.16B}, [x2], #16
        B.GT    load_ip
end_enc:
        RET

        .end
