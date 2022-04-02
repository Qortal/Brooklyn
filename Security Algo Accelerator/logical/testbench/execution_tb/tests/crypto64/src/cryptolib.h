//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2012-2013 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2013-03-19 09:12:51 +0000 (Tue, 19 Mar 2013) $
//
//      Revision            : $Revision: 241584 $
//
//      Release Information :
//
//-----------------------------------------------------------------------------

extern void aes128_key_expand(const unsigned char *key_in, unsigned char *key_out);
extern void aes128_ecb_encrypt(const unsigned char *key, const unsigned char *in_data, unsigned char *out_data, unsigned int size);

