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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cryptolib.h"
#include "cryptodata.h"
#include "benchmark.h"

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 1024
#endif

#ifndef ITERATIONS
#define ITERATIONS 10
#endif

uint8_t get_aes_index( int block_size)
{
  uint8_t index = 0; 
  uint8_t i;
  for (i=4; i<13; i++)
  {
      if ((block_size >> i) & 0x1)
      {
          index = i-4;
          break;
      }
  } 
  return index;
}

int main()
{
    uint32_t block_size;
    uint8_t index;
    uint32_t cmpres = 0;
    uint8_t i;

    block_size = BLOCK_SIZE; 

    uint8_t kv[176];
    printf("AES128-ECB encryption\n");
    index = get_aes_index(block_size);
    BENCHSTART
    for ( i = 0; i < ITERATIONS; i++)
    {
         aes128_key_expand(aes128_ecb_encrypt_key[index], kv);
         LOOPSTART
         aes128_ecb_encrypt(kv, aes128_ecb_encrypt_input[index], aes128_ecb_encrypt_output[index], block_size);
         LOOPEND
    }
    cmpres |= memcmp(aes128_ecb_encrypt_output[index], aes128_ecb_encrypt_ref_output[index], block_size);
    if (cmpres)
        printf("AES128-ECB encryption failed\n");
    BENCHFINISHED
}

