//-----------------------------------------------------------------------------
//     The confidential and proprietary information contained in this file may
//     only be used by a person authorised under and to the extent permitted
//     by a subsisting licensing agreement from ARM Limited or its affiliates.
//
//            (C) COPYRIGHT 2015-2020 ARM Limited or its affiliates.
//                ALL RIGHTS RESERVED
//
//     This entire notice must be reproduced on all copies of this file
//     and copies of this file may only be made by a person if such person is
//     permitted to do so under the terms of a subsisting license agreement
//     from ARM Limited or its affiliates.
//
//     Release Information : HERCULESAE-MP106-r0p1-00eac0
//
//-----------------------------------------------------------------------------
// SystemVerilog (IEEE Std 1800-2012)
//-----------------------------------------------------------------------------




`include "herculesae_header.sv"


module herculesae_vx_aesinv
(

    
  input wire  [7:0] lut_in,
  output wire [7:0] lut_out 
);










  assign lut_out[7] = (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[0]) | (
    lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]
    &!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[7]&lut_in[6]
    &!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[6]
    &lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (
    !lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]) | (lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]
    &lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[5]&!lut_in[4]&lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]
    &lut_in[4]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]
    &lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]) | (
    !lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]
    &!lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]
    &lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[6]&lut_in[4]&!lut_in[3]
    &lut_in[2]&lut_in[1]&lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]) | (lut_in[7]&!lut_in[6]
    &lut_in[5]&!lut_in[4]&lut_in[2]&!lut_in[1]) | (lut_in[7]&!lut_in[6]
    &!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (
    lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[1]
    &!lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (lut_in[7]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (lut_in[7]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]
    &lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (
    !lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]
    &lut_in[0]) | (lut_in[6]&lut_in[5]&lut_in[3]&!lut_in[2]&lut_in[1]
    &lut_in[0]) | (!lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &lut_in[0]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&lut_in[1]) | (!lut_in[7]&lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&lut_in[2]) | (lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[6]&!lut_in[5]
    &lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[6]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&!lut_in[5]&!lut_in[3]&lut_in[2]&!lut_in[1]) | (lut_in[6]
    &!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (!lut_in[7]
    &lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (lut_in[7]
    &lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[2]&lut_in[1]) | (!lut_in[7]
    &lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[5]
    &lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[5]
    &lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]
    &lut_in[6]&lut_in[5]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[4]
    &!lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[6]
    &lut_in[5]&lut_in[3]&!lut_in[2]) | (lut_in[6]&!lut_in[5]&!lut_in[4]
    &lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[3]&lut_in[1]) | (!lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]
    &lut_in[1]) | (lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[1]&lut_in[0]);

  assign lut_out[6] = (lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &!lut_in[6]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (
    !lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]
    &lut_in[1]) | (lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]
    &lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[4]
    &lut_in[3]&lut_in[2]&!lut_in[1]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[7]&lut_in[6]
    &!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[6]
    &!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]
    &lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    !lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]
    &lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[7]&!lut_in[6]
    &lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]) | (lut_in[7]&lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[6]
    &!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]) | (!lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]) | (
    !lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[0]) | (
    !lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]
    &!lut_in[1]) | (lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (lut_in[6]&lut_in[4]&!lut_in[3]&lut_in[2]
    &!lut_in[1]&lut_in[0]) | (lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]
    &lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[7]&lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]
    &lut_in[2]&lut_in[1]&lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &!lut_in[4]&!lut_in[2]&lut_in[1]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]
    &!lut_in[4]&!lut_in[3]&lut_in[2]) | (!lut_in[6]&lut_in[5]&!lut_in[4]
    &lut_in[1]&lut_in[0]) | (!lut_in[6]&!lut_in[5]&!lut_in[3]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]
    &!lut_in[1]&lut_in[0]) | (lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[1]
    &lut_in[0]) | (!lut_in[6]&!lut_in[5]&!lut_in[3]&!lut_in[2]&!lut_in[1]
    &lut_in[0]) | (!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[1]&lut_in[0]) | (
    lut_in[6]&!lut_in[5]&lut_in[3]&lut_in[2]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&!lut_in[5]&lut_in[2]&lut_in[1]) | (!lut_in[7]&!lut_in[5]
    &lut_in[3]&lut_in[2]&lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[4]
    &lut_in[3]&!lut_in[1]) | (lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[2]
    &!lut_in[1]&lut_in[0]);

  assign lut_out[5] = (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &!lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]
    &!lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]
    &lut_in[5]&lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&lut_in[1]) | (!lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]
    &!lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[6]&!lut_in[5]
    &!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[7]
    &lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    !lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&lut_in[0]) | (
    !lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]) | (
    !lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]
    &lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &!lut_in[2]&lut_in[1]) | (lut_in[7]&lut_in[5]&lut_in[4]&lut_in[3]
    &!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[4]
    &!lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[7]
    &lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (
    !lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]&lut_in[1]) | (
    !lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]
    &!lut_in[1]) | (!lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&lut_in[1]) | (lut_in[6]&lut_in[5]&lut_in[3]&!lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[5]&!lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[6]&!lut_in[5]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[6]
    &!lut_in[5]&lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[6]
    &!lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (lut_in[6]
    &!lut_in[5]&!lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[7]
    &lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (lut_in[5]
    &lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[0]) | (!lut_in[7]&lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[5]&lut_in[3]
    &lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]&lut_in[0]) | (!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[1]
    &lut_in[0]) | (lut_in[6]&!lut_in[5]&lut_in[3]&lut_in[2]&lut_in[0]) | (
    !lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[2]&lut_in[1]) | (!lut_in[7]
    &!lut_in[5]&lut_in[3]&lut_in[2]&lut_in[1]) | (!lut_in[6]&!lut_in[4]
    &!lut_in[3]&!lut_in[1]&lut_in[0]) | (!lut_in[5]&!lut_in[4]&lut_in[3]
    &lut_in[2]&lut_in[1]) | (lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[1]
    &lut_in[0]) | (!lut_in[7]&!lut_in[4]&lut_in[2]&lut_in[1]);

  assign lut_out[4] = (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[6]
    &!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (
    lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[3]&!lut_in[2]&lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]
    &lut_in[2]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]&lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &!lut_in[4]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]
    &!lut_in[5]&!lut_in[4]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (
    lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]
    &lut_in[0]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]) | (lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]
    &lut_in[3]&!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[4]&lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[4]
    &lut_in[3]&lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    !lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]
    &lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]
    &!lut_in[2]&lut_in[1]) | (!lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[1]) | (lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[4]
    &lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[6]&lut_in[5]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (lut_in[7]&lut_in[6]
    &lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]) | (lut_in[7]&lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &!lut_in[6]&lut_in[4]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[5]&!lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]) | (
    !lut_in[7]&!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    !lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]
    &!lut_in[0]) | (lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (!lut_in[6]&lut_in[4]&lut_in[3]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (!lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]
    &!lut_in[1]&lut_in[0]) | (lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[3]
    &lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[6]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[3]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &lut_in[1]&lut_in[0]) | (!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]
    &!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]) | (!lut_in[7]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]) | (
    !lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    !lut_in[7]&!lut_in[6]&lut_in[4]&lut_in[3]&!lut_in[1]) | (!lut_in[7]
    &lut_in[6]&lut_in[5]&lut_in[3]&!lut_in[2]) | (!lut_in[7]&!lut_in[6]
    &lut_in[4]&lut_in[2]&!lut_in[1]) | (!lut_in[6]&!lut_in[4]&!lut_in[3]
    &!lut_in[1]&lut_in[0]) | (lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[2]
    &!lut_in[1]&lut_in[0]);

  assign lut_out[3] = (lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]
    &lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]&!lut_in[0]) | (!lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]
    &!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]
    &lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (lut_in[7]
    &lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[2]&!lut_in[0]) | (lut_in[7]
    &!lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (
    lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]
    &lut_in[0]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]
    &lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]
    &!lut_in[3]&!lut_in[2]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]
    &lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &!lut_in[6]&!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]
    &!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (lut_in[7]&lut_in[5]
    &lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]&lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (!lut_in[7]
    &!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]) | (
    !lut_in[7]&!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    !lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]
    &!lut_in[0]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]
    &!lut_in[1]&lut_in[0]) | (lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]&lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&lut_in[1]) | (!lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]
    &!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[6]&!lut_in[4]
    &lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[6]&lut_in[4]
    &lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[6]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[1]&lut_in[0]) | (lut_in[7]&lut_in[6]
    &!lut_in[5]&lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&!lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[6]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]) | (lut_in[6]&lut_in[4]
    &lut_in[2]&lut_in[1]&lut_in[0]) | (!lut_in[6]&lut_in[3]&!lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[6]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]
    &!lut_in[1]&lut_in[0]) | (lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &lut_in[0]) | (!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[3]&!lut_in[2]&!lut_in[1]
    &lut_in[0]) | (!lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[3]&!lut_in[1]) | (
    !lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[1]);

  assign lut_out[2] = (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[3]&!lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]
    &lut_in[5]&!lut_in[4]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[3]
    &lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (!lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (
    !lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]
    &lut_in[1]) | (!lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[3]
    &lut_in[2]&!lut_in[1]) | (lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[7]&!lut_in[6]
    &lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]) | (lut_in[7]
    &!lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[4]&lut_in[3]&!lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[3]&!lut_in[2]&lut_in[1]) | (lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[6]&!lut_in[5]
    &lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[7]
    &lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]) | (
    !lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]
    &!lut_in[1]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[4]
    &lut_in[3]&!lut_in[2]&!lut_in[1]) | (lut_in[7]&!lut_in[6]&lut_in[5]
    &lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[6]&lut_in[5]
    &lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[5]
    &lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (!lut_in[6]
    &lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (
    !lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]) | (!lut_in[6]&!lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]
    &lut_in[0]) | (lut_in[7]&lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]
    &lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]
    &!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]
    &lut_in[5]&lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (!lut_in[6]
    &lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (
    lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]
    &!lut_in[0]) | (lut_in[6]&!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]
    &lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (!lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]
    &!lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]&lut_in[4]&lut_in[3]
    &lut_in[2]&!lut_in[1]) | (lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[6]&lut_in[4]&lut_in[3]&lut_in[2]
    &!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[6]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[3]&lut_in[2]) | (!lut_in[6]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &lut_in[1]&lut_in[0]) | (lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[3]
    &lut_in[0]) | (!lut_in[5]&!lut_in[4]&lut_in[2]&lut_in[1]&lut_in[0]) | (
    !lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (
    lut_in[6]&lut_in[5]&!lut_in[3]&lut_in[2]&lut_in[0]) | (lut_in[6]
    &lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[7]
    &lut_in[6]&!lut_in[5]&!lut_in[3]&lut_in[1]) | (lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[6]
    &!lut_in[5]&!lut_in[3]&!lut_in[1]);

  assign lut_out[1] = (lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[7]
    &lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]) | (
    lut_in[7]&!lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[3]&lut_in[2]
    &lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&!lut_in[4]
    &lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[7]&!lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (lut_in[7]
    &lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]) | (
    lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]
    &!lut_in[0]) | (lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]&lut_in[0]) | (lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (lut_in[7]&!lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[6]
    &lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (
    !lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]) | (lut_in[7]&!lut_in[6]&lut_in[4]&!lut_in[2]&lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[4]&lut_in[3]&!lut_in[2]
    &lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[3]
    &!lut_in[2]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]
    &!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (
    !lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]&lut_in[1]) | (
    lut_in[6]&!lut_in[5]&lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]
    &lut_in[0]) | (lut_in[7]&!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (!lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&lut_in[1]) | (!lut_in[7]&!lut_in[6]&lut_in[5]&lut_in[4]
    &!lut_in[3]&lut_in[2]&!lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (!lut_in[6]&lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[6]
    &lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]&lut_in[0]) | (lut_in[6]
    &lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[5]
    &!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (lut_in[6]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]) | (lut_in[6]
    &!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]&lut_in[1]) | (lut_in[7]
    &!lut_in[6]&!lut_in[4]&!lut_in[1]&!lut_in[0]) | (lut_in[6]&!lut_in[5]
    &!lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[5]
    &lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]) | (lut_in[6]&!lut_in[5]
    &lut_in[2]&lut_in[1]&lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &!lut_in[4]&!lut_in[2]&lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]) | (!lut_in[7]&lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (!lut_in[6]&!lut_in[5]&!lut_in[3]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[5]&lut_in[4]&!lut_in[3]&lut_in[2]
    &!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]) | (lut_in[7]&lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[1]) | (
    !lut_in[6]&!lut_in[5]&!lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]);

  assign lut_out[0] = (lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]
    &!lut_in[4]&!lut_in[3]&lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[7]
    &!lut_in[6]&!lut_in[5]&lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (
    lut_in[7]&!lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&!lut_in[6]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]
    &!lut_in[3]&lut_in[2]&lut_in[1]&!lut_in[0]) | (lut_in[7]&!lut_in[6]
    &!lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]&!lut_in[0]) | (
    !lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]
    &!lut_in[1]) | (lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (lut_in[6]&!lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[6]&!lut_in[5]&!lut_in[4]
    &!lut_in[3]&lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]
    &!lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]) | (lut_in[6]
    &lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (
    lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[3]&!lut_in[2]&lut_in[1]
    &!lut_in[0]) | (lut_in[7]&!lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]
    &!lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[3]&lut_in[1]&lut_in[0]) | (lut_in[6]&!lut_in[5]&lut_in[4]
    &!lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (lut_in[7]&!lut_in[6]
    &lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (!lut_in[7]
    &!lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]&!lut_in[2]&!lut_in[1]) | (
    !lut_in[6]&lut_in[5]&!lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]
    &lut_in[0]) | (lut_in[6]&lut_in[5]&lut_in[3]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &!lut_in[0]) | (lut_in[7]&lut_in[6]&!lut_in[4]&lut_in[3]&!lut_in[2]
    &lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[5]&lut_in[4]&lut_in[3]
    &!lut_in[1]&!lut_in[0]) | (lut_in[7]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&!lut_in[1]&!lut_in[0]) | (lut_in[6]&!lut_in[5]&lut_in[4]
    &lut_in[3]&!lut_in[2]&!lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[6]
    &!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]) | (!lut_in[6]&!lut_in[5]
    &!lut_in[4]&lut_in[3]&!lut_in[2]&lut_in[1]&lut_in[0]) | (!lut_in[7]
    &!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]&lut_in[1]) | (
    lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]&!lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (lut_in[5]&!lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]
    &lut_in[0]) | (!lut_in[7]&lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]
    &lut_in[2]) | (!lut_in[5]&lut_in[4]&lut_in[3]&lut_in[2]&!lut_in[1]
    &!lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]
    &!lut_in[2]&lut_in[1]) | (lut_in[6]&!lut_in[4]&!lut_in[3]&lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[7]&lut_in[5]&!lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]) | (!lut_in[7]&lut_in[6]&!lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]) | (lut_in[6]&!lut_in[5]&!lut_in[4]&lut_in[3]
    &!lut_in[2]&!lut_in[1]) | (!lut_in[6]&lut_in[5]&lut_in[4]&!lut_in[3]
    &lut_in[2]&!lut_in[1]) | (lut_in[6]&!lut_in[5]&!lut_in[3]&!lut_in[2]
    &lut_in[1]&lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&lut_in[3]
    &!lut_in[2]&lut_in[1]) | (lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[2]
    &!lut_in[0]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[2]&lut_in[1]
    &lut_in[0]) | (!lut_in[7]&!lut_in[6]&!lut_in[5]&!lut_in[4]&!lut_in[3]
    &lut_in[2]) | (!lut_in[6]&lut_in[4]&!lut_in[3]&!lut_in[2]&lut_in[1]
    &lut_in[0]) | (!lut_in[6]&lut_in[5]&lut_in[4]&lut_in[3]&lut_in[1]
    &lut_in[0]) | (!lut_in[7]&lut_in[6]&!lut_in[4]&lut_in[3]&lut_in[1]) | (
    !lut_in[7]&lut_in[6]&lut_in[5]&!lut_in[3]&!lut_in[2]&lut_in[1]) | (
    !lut_in[7]&lut_in[6]&!lut_in[5]&!lut_in[3]&lut_in[1]) | (!lut_in[7]
    &!lut_in[6]&lut_in[4]&lut_in[2]&!lut_in[1]) | (lut_in[6]&lut_in[5]
    &!lut_in[4]&lut_in[1]&lut_in[0]);





endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
