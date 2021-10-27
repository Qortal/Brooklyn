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


module herculesae_vx_aese1
(


  input wire [127:0]    q,

  output wire [127:0]   aese_out,
  output wire  [15:0]   aese_shf 
);







  wire [127:0]                           aese_noshf;
  wire [7:0]                             s00;
  wire [7:0]                             s01;
  wire [7:0]                             s02;
  wire [7:0]                             s03;
  wire [7:0]                             s10;
  wire [7:0]                             s11;
  wire [7:0]                             s12;
  wire [7:0]                             s13;
  wire [7:0]                             s20;
  wire [7:0]                             s21;
  wire [7:0]                             s22;
  wire [7:0]                             s23;
  wire [7:0]                             s30;
  wire [7:0]                             s31;
  wire [7:0]                             s32;
  wire [7:0]                             s33;



assign s33[7:0] = q[127:120];
assign s23[7:0] = q[119:112];
assign s13[7:0] = q[111:104];
assign s03[7:0] = q[103:96];

assign s32[7:0] = q[95:88];
assign s22[7:0] = q[87:80];
assign s12[7:0] = q[79:72];
assign s02[7:0] = q[71:64];

assign s31[7:0] = q[63:56];
assign s21[7:0] = q[55:48];
assign s11[7:0] = q[47:40];
assign s01[7:0] = q[39:32];

assign s30[7:0] = q[31:24];
assign s20[7:0] = q[23:16];
assign s10[7:0] = q[15:8];
assign s00[7:0] = q[7:0];


assign aese_noshf[7:0]     = s00[7:0];
assign aese_noshf[39:32]   = s01[7:0];
assign aese_noshf[71:64]   = s02[7:0];
assign aese_noshf[103:96]  = s03[7:0];

assign aese_noshf[15:8]    = s11[7:0];
assign aese_noshf[47:40]   = s12[7:0];
assign aese_noshf[79:72]   = s13[7:0];
assign aese_noshf[111:104] = s10[7:0];

assign aese_noshf[23:16]   = s22[7:0];
assign aese_noshf[55:48]   = s23[7:0];
assign aese_noshf[87:80]   = s20[7:0];
assign aese_noshf[119:112] = s21[7:0];

assign aese_noshf[31:24]   = s33[7:0];
assign aese_noshf[63:56]   = s30[7:0];
assign aese_noshf[95:88]   = s31[7:0];
assign aese_noshf[127:120] = s32[7:0];

assign aese_shf[15] =  ~aese_noshf[127];
assign aese_shf[14] =  ~aese_noshf[119];
assign aese_shf[13] =  ~aese_noshf[111];
assign aese_shf[12] =  ~aese_noshf[103];
assign aese_shf[11] =  ~aese_noshf[ 95];
assign aese_shf[10] =  ~aese_noshf[ 87];
assign aese_shf[ 9] =  ~aese_noshf[ 79];
assign aese_shf[ 8] =  ~aese_noshf[ 71];
assign aese_shf[ 7] =  ~aese_noshf[ 63];
assign aese_shf[ 6] =  ~aese_noshf[ 55];
assign aese_shf[ 5] =  ~aese_noshf[ 47];
assign aese_shf[ 4] =  ~aese_noshf[ 39];
assign aese_shf[ 3] =  ~aese_noshf[ 31];
assign aese_shf[ 2] =  ~aese_noshf[ 23];
assign aese_shf[ 1] =  ~aese_noshf[ 15];
assign aese_shf[ 0] =  ~aese_noshf[  7];

assign aese_out[127:120] = {8{ aese_shf[15]}} & {aese_noshf[126:120], 1'b0} |
                           {8{~aese_shf[15]}} &  aese_noshf[127:120];
assign aese_out[119:112] = {8{ aese_shf[14]}} & {aese_noshf[118:112], 1'b0} |
                           {8{~aese_shf[14]}} &  aese_noshf[119:112];
assign aese_out[111:104] = {8{ aese_shf[13]}} & {aese_noshf[110:104], 1'b0} |
                           {8{~aese_shf[13]}} &  aese_noshf[111:104];
assign aese_out[103: 96] = {8{ aese_shf[12]}} & {aese_noshf[102: 96], 1'b0} |
                           {8{~aese_shf[12]}} &  aese_noshf[103: 96];
assign aese_out[ 95: 88] = {8{ aese_shf[11]}} & {aese_noshf[ 94: 88], 1'b0} |
                           {8{~aese_shf[11]}} &  aese_noshf[ 95: 88];
assign aese_out[ 87: 80] = {8{ aese_shf[10]}} & {aese_noshf[ 86: 80], 1'b0} |
                           {8{~aese_shf[10]}} &  aese_noshf[ 87: 80];
assign aese_out[ 79: 72] = {8{ aese_shf[ 9]}} & {aese_noshf[ 78: 72], 1'b0} |
                           {8{~aese_shf[ 9]}} &  aese_noshf[ 79: 72];
assign aese_out[ 71: 64] = {8{ aese_shf[ 8]}} & {aese_noshf[ 70: 64], 1'b0} |
                           {8{~aese_shf[ 8]}} &  aese_noshf[ 71: 64];
assign aese_out[ 63: 56] = {8{ aese_shf[ 7]}} & {aese_noshf[ 62: 56], 1'b0} |
                           {8{~aese_shf[ 7]}} &  aese_noshf[ 63: 56];
assign aese_out[ 55: 48] = {8{ aese_shf[ 6]}} & {aese_noshf[ 54: 48], 1'b0} |
                           {8{~aese_shf[ 6]}} &  aese_noshf[ 55: 48];
assign aese_out[ 47: 40] = {8{ aese_shf[ 5]}} & {aese_noshf[ 46: 40], 1'b0} |
                           {8{~aese_shf[ 5]}} &  aese_noshf[ 47: 40];
assign aese_out[ 39: 32] = {8{ aese_shf[ 4]}} & {aese_noshf[ 38: 32], 1'b0} |
                           {8{~aese_shf[ 4]}} &  aese_noshf[ 39: 32];
assign aese_out[ 31: 24] = {8{ aese_shf[ 3]}} & {aese_noshf[ 30: 24], 1'b0} |
                           {8{~aese_shf[ 3]}} &  aese_noshf[ 31: 24];
assign aese_out[ 23: 16] = {8{ aese_shf[ 2]}} & {aese_noshf[ 22: 16], 1'b0} |
                           {8{~aese_shf[ 2]}} &  aese_noshf[ 23: 16];
assign aese_out[ 15:  8] = {8{ aese_shf[ 1]}} & {aese_noshf[ 14:  8], 1'b0} |
                           {8{~aese_shf[ 1]}} &  aese_noshf[ 15:  8];
assign aese_out[  7:  0] = {8{ aese_shf[ 0]}} & {aese_noshf[  6:  0], 1'b0} |
                           {8{~aese_shf[ 0]}} &  aese_noshf[  7:  0];
                           
endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
