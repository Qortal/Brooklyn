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


module herculesae_vx_aesd1
(


  input wire [127:0]    q,

  output wire [127:0]   aesd_out,
  output wire  [15:0]   aesd_shf 
);







  wire [127:0]                           aesd_noshf;
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
  wire [7:0]                             sp00;
  wire [7:0]                             sp01;
  wire [7:0]                             sp02;
  wire [7:0]                             sp03;
  wire [7:0]                             sp10;
  wire [7:0]                             sp11;
  wire [7:0]                             sp12;
  wire [7:0]                             sp13;
  wire [7:0]                             sp20;
  wire [7:0]                             sp21;
  wire [7:0]                             sp22;
  wire [7:0]                             sp23;
  wire [7:0]                             sp30;
  wire [7:0]                             sp31;
  wire [7:0]                             sp32;
  wire [7:0]                             sp33;



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


assign sp00[7:0] = s00[7:0];
assign sp01[7:0] = s01[7:0];
assign sp02[7:0] = s02[7:0];
assign sp03[7:0] = s03[7:0];

assign sp10[7:0] = s13[7:0];
assign sp11[7:0] = s10[7:0];
assign sp12[7:0] = s11[7:0];
assign sp13[7:0] = s12[7:0];

assign sp20[7:0] = s22[7:0];
assign sp21[7:0] = s23[7:0];
assign sp22[7:0] = s20[7:0];
assign sp23[7:0] = s21[7:0];

assign sp30[7:0] = s31[7:0];
assign sp31[7:0] = s32[7:0];
assign sp32[7:0] = s33[7:0];
assign sp33[7:0] = s30[7:0];


assign aesd_noshf[0] = sp00[2] ^ sp00[5] ^ sp00[7] ^ 1'b1;
assign aesd_noshf[1] = sp00[0] ^ sp00[3] ^ sp00[6] ^ 1'b0;
assign aesd_noshf[2] = sp00[1] ^ sp00[4] ^ sp00[7] ^ 1'b1;
assign aesd_noshf[3] = sp00[0] ^ sp00[2] ^ sp00[5] ^ 1'b0;
assign aesd_noshf[4] = sp00[1] ^ sp00[3] ^ sp00[6] ^ 1'b0;
assign aesd_noshf[5] = sp00[2] ^ sp00[4] ^ sp00[7] ^ 1'b0;
assign aesd_noshf[6] = sp00[0] ^ sp00[3] ^ sp00[5] ^ 1'b0;
assign aesd_noshf[7] = sp00[1] ^ sp00[4] ^ sp00[6] ^ 1'b0;
assign aesd_noshf[32] = sp01[2] ^ sp01[5] ^ sp01[7] ^ 1'b1;
assign aesd_noshf[33] = sp01[0] ^ sp01[3] ^ sp01[6] ^ 1'b0;
assign aesd_noshf[34] = sp01[1] ^ sp01[4] ^ sp01[7] ^ 1'b1;
assign aesd_noshf[35] = sp01[0] ^ sp01[2] ^ sp01[5] ^ 1'b0;
assign aesd_noshf[36] = sp01[1] ^ sp01[3] ^ sp01[6] ^ 1'b0;
assign aesd_noshf[37] = sp01[2] ^ sp01[4] ^ sp01[7] ^ 1'b0;
assign aesd_noshf[38] = sp01[0] ^ sp01[3] ^ sp01[5] ^ 1'b0;
assign aesd_noshf[39] = sp01[1] ^ sp01[4] ^ sp01[6] ^ 1'b0;
assign aesd_noshf[64] = sp02[2] ^ sp02[5] ^ sp02[7] ^ 1'b1;
assign aesd_noshf[65] = sp02[0] ^ sp02[3] ^ sp02[6] ^ 1'b0;
assign aesd_noshf[66] = sp02[1] ^ sp02[4] ^ sp02[7] ^ 1'b1;
assign aesd_noshf[67] = sp02[0] ^ sp02[2] ^ sp02[5] ^ 1'b0;
assign aesd_noshf[68] = sp02[1] ^ sp02[3] ^ sp02[6] ^ 1'b0;
assign aesd_noshf[69] = sp02[2] ^ sp02[4] ^ sp02[7] ^ 1'b0;
assign aesd_noshf[70] = sp02[0] ^ sp02[3] ^ sp02[5] ^ 1'b0;
assign aesd_noshf[71] = sp02[1] ^ sp02[4] ^ sp02[6] ^ 1'b0;
assign aesd_noshf[96] = sp03[2] ^ sp03[5] ^ sp03[7] ^ 1'b1;
assign aesd_noshf[97] = sp03[0] ^ sp03[3] ^ sp03[6] ^ 1'b0;
assign aesd_noshf[98] = sp03[1] ^ sp03[4] ^ sp03[7] ^ 1'b1;
assign aesd_noshf[99] = sp03[0] ^ sp03[2] ^ sp03[5] ^ 1'b0;
assign aesd_noshf[100] = sp03[1] ^ sp03[3] ^ sp03[6] ^ 1'b0;
assign aesd_noshf[101] = sp03[2] ^ sp03[4] ^ sp03[7] ^ 1'b0;
assign aesd_noshf[102] = sp03[0] ^ sp03[3] ^ sp03[5] ^ 1'b0;
assign aesd_noshf[103] = sp03[1] ^ sp03[4] ^ sp03[6] ^ 1'b0;

assign aesd_noshf[8] = sp10[2] ^ sp10[5] ^ sp10[7] ^ 1'b1;
assign aesd_noshf[9] = sp10[0] ^ sp10[3] ^ sp10[6] ^ 1'b0;
assign aesd_noshf[10] = sp10[1] ^ sp10[4] ^ sp10[7] ^ 1'b1;
assign aesd_noshf[11] = sp10[0] ^ sp10[2] ^ sp10[5] ^ 1'b0;
assign aesd_noshf[12] = sp10[1] ^ sp10[3] ^ sp10[6] ^ 1'b0;
assign aesd_noshf[13] = sp10[2] ^ sp10[4] ^ sp10[7] ^ 1'b0;
assign aesd_noshf[14] = sp10[0] ^ sp10[3] ^ sp10[5] ^ 1'b0;
assign aesd_noshf[15] = sp10[1] ^ sp10[4] ^ sp10[6] ^ 1'b0;
assign aesd_noshf[40] = sp11[2] ^ sp11[5] ^ sp11[7] ^ 1'b1;
assign aesd_noshf[41] = sp11[0] ^ sp11[3] ^ sp11[6] ^ 1'b0;
assign aesd_noshf[42] = sp11[1] ^ sp11[4] ^ sp11[7] ^ 1'b1;
assign aesd_noshf[43] = sp11[0] ^ sp11[2] ^ sp11[5] ^ 1'b0;
assign aesd_noshf[44] = sp11[1] ^ sp11[3] ^ sp11[6] ^ 1'b0;
assign aesd_noshf[45] = sp11[2] ^ sp11[4] ^ sp11[7] ^ 1'b0;
assign aesd_noshf[46] = sp11[0] ^ sp11[3] ^ sp11[5] ^ 1'b0;
assign aesd_noshf[47] = sp11[1] ^ sp11[4] ^ sp11[6] ^ 1'b0;
assign aesd_noshf[72] = sp12[2] ^ sp12[5] ^ sp12[7] ^ 1'b1;
assign aesd_noshf[73] = sp12[0] ^ sp12[3] ^ sp12[6] ^ 1'b0;
assign aesd_noshf[74] = sp12[1] ^ sp12[4] ^ sp12[7] ^ 1'b1;
assign aesd_noshf[75] = sp12[0] ^ sp12[2] ^ sp12[5] ^ 1'b0;
assign aesd_noshf[76] = sp12[1] ^ sp12[3] ^ sp12[6] ^ 1'b0;
assign aesd_noshf[77] = sp12[2] ^ sp12[4] ^ sp12[7] ^ 1'b0;
assign aesd_noshf[78] = sp12[0] ^ sp12[3] ^ sp12[5] ^ 1'b0;
assign aesd_noshf[79] = sp12[1] ^ sp12[4] ^ sp12[6] ^ 1'b0;
assign aesd_noshf[104] = sp13[2] ^ sp13[5] ^ sp13[7] ^ 1'b1;
assign aesd_noshf[105] = sp13[0] ^ sp13[3] ^ sp13[6] ^ 1'b0;
assign aesd_noshf[106] = sp13[1] ^ sp13[4] ^ sp13[7] ^ 1'b1;
assign aesd_noshf[107] = sp13[0] ^ sp13[2] ^ sp13[5] ^ 1'b0;
assign aesd_noshf[108] = sp13[1] ^ sp13[3] ^ sp13[6] ^ 1'b0;
assign aesd_noshf[109] = sp13[2] ^ sp13[4] ^ sp13[7] ^ 1'b0;
assign aesd_noshf[110] = sp13[0] ^ sp13[3] ^ sp13[5] ^ 1'b0;
assign aesd_noshf[111] = sp13[1] ^ sp13[4] ^ sp13[6] ^ 1'b0;

assign aesd_noshf[16] = sp20[2] ^ sp20[5] ^ sp20[7] ^ 1'b1;
assign aesd_noshf[17] = sp20[0] ^ sp20[3] ^ sp20[6] ^ 1'b0;
assign aesd_noshf[18] = sp20[1] ^ sp20[4] ^ sp20[7] ^ 1'b1;
assign aesd_noshf[19] = sp20[0] ^ sp20[2] ^ sp20[5] ^ 1'b0;
assign aesd_noshf[20] = sp20[1] ^ sp20[3] ^ sp20[6] ^ 1'b0;
assign aesd_noshf[21] = sp20[2] ^ sp20[4] ^ sp20[7] ^ 1'b0;
assign aesd_noshf[22] = sp20[0] ^ sp20[3] ^ sp20[5] ^ 1'b0;
assign aesd_noshf[23] = sp20[1] ^ sp20[4] ^ sp20[6] ^ 1'b0;
assign aesd_noshf[48] = sp21[2] ^ sp21[5] ^ sp21[7] ^ 1'b1;
assign aesd_noshf[49] = sp21[0] ^ sp21[3] ^ sp21[6] ^ 1'b0;
assign aesd_noshf[50] = sp21[1] ^ sp21[4] ^ sp21[7] ^ 1'b1;
assign aesd_noshf[51] = sp21[0] ^ sp21[2] ^ sp21[5] ^ 1'b0;
assign aesd_noshf[52] = sp21[1] ^ sp21[3] ^ sp21[6] ^ 1'b0;
assign aesd_noshf[53] = sp21[2] ^ sp21[4] ^ sp21[7] ^ 1'b0;
assign aesd_noshf[54] = sp21[0] ^ sp21[3] ^ sp21[5] ^ 1'b0;
assign aesd_noshf[55] = sp21[1] ^ sp21[4] ^ sp21[6] ^ 1'b0;
assign aesd_noshf[80] = sp22[2] ^ sp22[5] ^ sp22[7] ^ 1'b1;
assign aesd_noshf[81] = sp22[0] ^ sp22[3] ^ sp22[6] ^ 1'b0;
assign aesd_noshf[82] = sp22[1] ^ sp22[4] ^ sp22[7] ^ 1'b1;
assign aesd_noshf[83] = sp22[0] ^ sp22[2] ^ sp22[5] ^ 1'b0;
assign aesd_noshf[84] = sp22[1] ^ sp22[3] ^ sp22[6] ^ 1'b0;
assign aesd_noshf[85] = sp22[2] ^ sp22[4] ^ sp22[7] ^ 1'b0;
assign aesd_noshf[86] = sp22[0] ^ sp22[3] ^ sp22[5] ^ 1'b0;
assign aesd_noshf[87] = sp22[1] ^ sp22[4] ^ sp22[6] ^ 1'b0;
assign aesd_noshf[112] = sp23[2] ^ sp23[5] ^ sp23[7] ^ 1'b1;
assign aesd_noshf[113] = sp23[0] ^ sp23[3] ^ sp23[6] ^ 1'b0;
assign aesd_noshf[114] = sp23[1] ^ sp23[4] ^ sp23[7] ^ 1'b1;
assign aesd_noshf[115] = sp23[0] ^ sp23[2] ^ sp23[5] ^ 1'b0;
assign aesd_noshf[116] = sp23[1] ^ sp23[3] ^ sp23[6] ^ 1'b0;
assign aesd_noshf[117] = sp23[2] ^ sp23[4] ^ sp23[7] ^ 1'b0;
assign aesd_noshf[118] = sp23[0] ^ sp23[3] ^ sp23[5] ^ 1'b0;
assign aesd_noshf[119] = sp23[1] ^ sp23[4] ^ sp23[6] ^ 1'b0;

assign aesd_noshf[24] = sp30[2] ^ sp30[5] ^ sp30[7] ^ 1'b1;
assign aesd_noshf[25] = sp30[0] ^ sp30[3] ^ sp30[6] ^ 1'b0;
assign aesd_noshf[26] = sp30[1] ^ sp30[4] ^ sp30[7] ^ 1'b1;
assign aesd_noshf[27] = sp30[0] ^ sp30[2] ^ sp30[5] ^ 1'b0;
assign aesd_noshf[28] = sp30[1] ^ sp30[3] ^ sp30[6] ^ 1'b0;
assign aesd_noshf[29] = sp30[2] ^ sp30[4] ^ sp30[7] ^ 1'b0;
assign aesd_noshf[30] = sp30[0] ^ sp30[3] ^ sp30[5] ^ 1'b0;
assign aesd_noshf[31] = sp30[1] ^ sp30[4] ^ sp30[6] ^ 1'b0;
assign aesd_noshf[56] = sp31[2] ^ sp31[5] ^ sp31[7] ^ 1'b1;
assign aesd_noshf[57] = sp31[0] ^ sp31[3] ^ sp31[6] ^ 1'b0;
assign aesd_noshf[58] = sp31[1] ^ sp31[4] ^ sp31[7] ^ 1'b1;
assign aesd_noshf[59] = sp31[0] ^ sp31[2] ^ sp31[5] ^ 1'b0;
assign aesd_noshf[60] = sp31[1] ^ sp31[3] ^ sp31[6] ^ 1'b0;
assign aesd_noshf[61] = sp31[2] ^ sp31[4] ^ sp31[7] ^ 1'b0;
assign aesd_noshf[62] = sp31[0] ^ sp31[3] ^ sp31[5] ^ 1'b0;
assign aesd_noshf[63] = sp31[1] ^ sp31[4] ^ sp31[6] ^ 1'b0;
assign aesd_noshf[88] = sp32[2] ^ sp32[5] ^ sp32[7] ^ 1'b1;
assign aesd_noshf[89] = sp32[0] ^ sp32[3] ^ sp32[6] ^ 1'b0;
assign aesd_noshf[90] = sp32[1] ^ sp32[4] ^ sp32[7] ^ 1'b1;
assign aesd_noshf[91] = sp32[0] ^ sp32[2] ^ sp32[5] ^ 1'b0;
assign aesd_noshf[92] = sp32[1] ^ sp32[3] ^ sp32[6] ^ 1'b0;
assign aesd_noshf[93] = sp32[2] ^ sp32[4] ^ sp32[7] ^ 1'b0;
assign aesd_noshf[94] = sp32[0] ^ sp32[3] ^ sp32[5] ^ 1'b0;
assign aesd_noshf[95] = sp32[1] ^ sp32[4] ^ sp32[6] ^ 1'b0;
assign aesd_noshf[120] = sp33[2] ^ sp33[5] ^ sp33[7] ^ 1'b1;
assign aesd_noshf[121] = sp33[0] ^ sp33[3] ^ sp33[6] ^ 1'b0;
assign aesd_noshf[122] = sp33[1] ^ sp33[4] ^ sp33[7] ^ 1'b1;
assign aesd_noshf[123] = sp33[0] ^ sp33[2] ^ sp33[5] ^ 1'b0;
assign aesd_noshf[124] = sp33[1] ^ sp33[3] ^ sp33[6] ^ 1'b0;
assign aesd_noshf[125] = sp33[2] ^ sp33[4] ^ sp33[7] ^ 1'b0;
assign aesd_noshf[126] = sp33[0] ^ sp33[3] ^ sp33[5] ^ 1'b0;
assign aesd_noshf[127] = sp33[1] ^ sp33[4] ^ sp33[6] ^ 1'b0;

assign aesd_shf[15] =  ~aesd_noshf[127];
assign aesd_shf[14] =  ~aesd_noshf[119];
assign aesd_shf[13] =  ~aesd_noshf[111];
assign aesd_shf[12] =  ~aesd_noshf[103];
assign aesd_shf[11] =  ~aesd_noshf[ 95];
assign aesd_shf[10] =  ~aesd_noshf[ 87];
assign aesd_shf[ 9] =  ~aesd_noshf[ 79];
assign aesd_shf[ 8] =  ~aesd_noshf[ 71];
assign aesd_shf[ 7] =  ~aesd_noshf[ 63];
assign aesd_shf[ 6] =  ~aesd_noshf[ 55];
assign aesd_shf[ 5] =  ~aesd_noshf[ 47];
assign aesd_shf[ 4] =  ~aesd_noshf[ 39];
assign aesd_shf[ 3] =  ~aesd_noshf[ 31];
assign aesd_shf[ 2] =  ~aesd_noshf[ 23];
assign aesd_shf[ 1] =  ~aesd_noshf[ 15];
assign aesd_shf[ 0] =  ~aesd_noshf[  7];

assign aesd_out[127:120] = {8{ aesd_shf[15]}} & {aesd_noshf[126:120], 1'b0} |
                           {8{~aesd_shf[15]}} &  aesd_noshf[127:120];
assign aesd_out[119:112] = {8{ aesd_shf[14]}} & {aesd_noshf[118:112], 1'b0} |
                           {8{~aesd_shf[14]}} &  aesd_noshf[119:112];
assign aesd_out[111:104] = {8{ aesd_shf[13]}} & {aesd_noshf[110:104], 1'b0} |
                           {8{~aesd_shf[13]}} &  aesd_noshf[111:104];
assign aesd_out[103: 96] = {8{ aesd_shf[12]}} & {aesd_noshf[102: 96], 1'b0} |
                           {8{~aesd_shf[12]}} &  aesd_noshf[103: 96];
assign aesd_out[ 95: 88] = {8{ aesd_shf[11]}} & {aesd_noshf[ 94: 88], 1'b0} |
                           {8{~aesd_shf[11]}} &  aesd_noshf[ 95: 88];
assign aesd_out[ 87: 80] = {8{ aesd_shf[10]}} & {aesd_noshf[ 86: 80], 1'b0} |
                           {8{~aesd_shf[10]}} &  aesd_noshf[ 87: 80];
assign aesd_out[ 79: 72] = {8{ aesd_shf[ 9]}} & {aesd_noshf[ 78: 72], 1'b0} |
                           {8{~aesd_shf[ 9]}} &  aesd_noshf[ 79: 72];
assign aesd_out[ 71: 64] = {8{ aesd_shf[ 8]}} & {aesd_noshf[ 70: 64], 1'b0} |
                           {8{~aesd_shf[ 8]}} &  aesd_noshf[ 71: 64];
assign aesd_out[ 63: 56] = {8{ aesd_shf[ 7]}} & {aesd_noshf[ 62: 56], 1'b0} |
                           {8{~aesd_shf[ 7]}} &  aesd_noshf[ 63: 56];
assign aesd_out[ 55: 48] = {8{ aesd_shf[ 6]}} & {aesd_noshf[ 54: 48], 1'b0} |
                           {8{~aesd_shf[ 6]}} &  aesd_noshf[ 55: 48];
assign aesd_out[ 47: 40] = {8{ aesd_shf[ 5]}} & {aesd_noshf[ 46: 40], 1'b0} |
                           {8{~aesd_shf[ 5]}} &  aesd_noshf[ 47: 40];
assign aesd_out[ 39: 32] = {8{ aesd_shf[ 4]}} & {aesd_noshf[ 38: 32], 1'b0} |
                           {8{~aesd_shf[ 4]}} &  aesd_noshf[ 39: 32];
assign aesd_out[ 31: 24] = {8{ aesd_shf[ 3]}} & {aesd_noshf[ 30: 24], 1'b0} |
                           {8{~aesd_shf[ 3]}} &  aesd_noshf[ 31: 24];
assign aesd_out[ 23: 16] = {8{ aesd_shf[ 2]}} & {aesd_noshf[ 22: 16], 1'b0} |
                           {8{~aesd_shf[ 2]}} &  aesd_noshf[ 23: 16];
assign aesd_out[ 15:  8] = {8{ aesd_shf[ 1]}} & {aesd_noshf[ 14:  8], 1'b0} |
                           {8{~aesd_shf[ 1]}} &  aesd_noshf[ 15:  8];
assign aesd_out[  7:  0] = {8{ aesd_shf[ 0]}} & {aesd_noshf[  6:  0], 1'b0} |
                           {8{~aesd_shf[ 0]}} &  aesd_noshf[  7:  0];

endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
