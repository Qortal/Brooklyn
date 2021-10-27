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


module herculesae_vx_aesmc
(


  input wire [127:0]    d_in,

  output wire [127:0]   mc 
);







  wire [7:0]                             i00;
  wire [7:0]                             i01;
  wire [7:0]                             i02;
  wire [7:0]                             i03;
  wire [7:0]                             i10;
  wire [7:0]                             i11;
  wire [7:0]                             i12;
  wire [7:0]                             i13;
  wire [7:0]                             i20;
  wire [7:0]                             i21;
  wire [7:0]                             i22;
  wire [7:0]                             i23;
  wire [7:0]                             i30;
  wire [7:0]                             i31;
  wire [7:0]                             i32;
  wire [7:0]                             i33;
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


assign i33[7:0] = d_in[127:120];
assign i23[7:0] = d_in[119:112];
assign i13[7:0] = d_in[111:104];
assign i03[7:0] = d_in[103:96];

assign i32[7:0] = d_in[95:88];
assign i22[7:0] = d_in[87:80];
assign i12[7:0] = d_in[79:72];
assign i02[7:0] = d_in[71:64];

assign i31[7:0] = d_in[63:56];
assign i21[7:0] = d_in[55:48];
assign i11[7:0] = d_in[47:40];
assign i01[7:0] = d_in[39:32];

assign i30[7:0] = d_in[31:24];
assign i20[7:0] = d_in[23:16];
assign i10[7:0] = d_in[15:8];
assign i00[7:0] = d_in[7:0];


assign s00[7:0] = {i00[6:0],1'b0} ^ ({8{i00[7]}} & 8'h1b)
                ^ {i10[6:0],1'b0} ^ ({8{i10[7]}} & 8'h1b) ^ i10[7:0]
                ^  i20[7:0]
                ^  i30[7:0];
assign s01[7:0] = {i01[6:0],1'b0} ^ ({8{i01[7]}} & 8'h1b)
                ^ {i11[6:0],1'b0} ^ ({8{i11[7]}} & 8'h1b) ^ i11[7:0]
                ^  i21[7:0]
                ^  i31[7:0];
assign s02[7:0] = {i02[6:0],1'b0} ^ ({8{i02[7]}} & 8'h1b)
                ^ {i12[6:0],1'b0} ^ ({8{i12[7]}} & 8'h1b) ^ i12[7:0]
                ^  i22[7:0]
                ^  i32[7:0];
assign s03[7:0] = {i03[6:0],1'b0} ^ ({8{i03[7]}} & 8'h1b)
                ^ {i13[6:0],1'b0} ^ ({8{i13[7]}} & 8'h1b) ^ i13[7:0]
                ^  i23[7:0]
                ^  i33[7:0];
assign s10[7:0] =  i00[7:0]
                ^ {i10[6:0],1'b0} ^ ({8{i10[7]}} & 8'h1b)
                ^ {i20[6:0],1'b0} ^ ({8{i20[7]}} & 8'h1b) ^ i20[7:0]
                ^  i30[7:0];
assign s11[7:0] =  i01[7:0]
                ^ {i11[6:0],1'b0} ^ ({8{i11[7]}} & 8'h1b)
                ^ {i21[6:0],1'b0} ^ ({8{i21[7]}} & 8'h1b) ^ i21[7:0]
                ^  i31[7:0];
assign s12[7:0] =  i02[7:0]
                ^ {i12[6:0],1'b0} ^ ({8{i12[7]}} & 8'h1b)
                ^ {i22[6:0],1'b0} ^ ({8{i22[7]}} & 8'h1b) ^ i22[7:0]
                ^  i32[7:0];
assign s13[7:0] =  i03[7:0]
                ^ {i13[6:0],1'b0} ^ ({8{i13[7]}} & 8'h1b)
                ^ {i23[6:0],1'b0} ^ ({8{i23[7]}} & 8'h1b) ^ i23[7:0]
                ^  i33[7:0];
assign s20[7:0] =  i00[7:0]
                ^  i10[7:0]
                ^ {i20[6:0],1'b0} ^ ({8{i20[7]}} & 8'h1b)
                ^ {i30[6:0],1'b0} ^ ({8{i30[7]}} & 8'h1b) ^ i30[7:0];
assign s21[7:0] =  i01[7:0]
                ^  i11[7:0]
                ^ {i21[6:0],1'b0} ^ ({8{i21[7]}} & 8'h1b)
                ^ {i31[6:0],1'b0} ^ ({8{i31[7]}} & 8'h1b) ^ i31[7:0];
assign s22[7:0] =  i02[7:0]
                ^  i12[7:0]
                ^ {i22[6:0],1'b0} ^ ({8{i22[7]}} & 8'h1b)
                ^ {i32[6:0],1'b0} ^ ({8{i32[7]}} & 8'h1b) ^ i32[7:0];
assign s23[7:0] =  i03[7:0]
                ^  i13[7:0]
                ^ {i23[6:0],1'b0} ^ ({8{i23[7]}} & 8'h1b)
                ^ {i33[6:0],1'b0} ^ ({8{i33[7]}} & 8'h1b) ^ i33[7:0];
assign s30[7:0] = {i00[6:0],1'b0} ^ ({8{i00[7]}} & 8'h1b) ^ i00[7:0]
                ^  i10[7:0]
                ^  i20[7:0]
                ^ {i30[6:0],1'b0} ^ ({8{i30[7]}} & 8'h1b);
assign s31[7:0] = {i01[6:0],1'b0} ^ ({8{i01[7]}} & 8'h1b) ^ i01[7:0]
                ^  i11[7:0]
                ^  i21[7:0]
                ^ {i31[6:0],1'b0} ^ ({8{i31[7]}} & 8'h1b);
assign s32[7:0] = {i02[6:0],1'b0} ^ ({8{i02[7]}} & 8'h1b) ^ i02[7:0]
                ^  i12[7:0]
                ^  i22[7:0]
                ^ {i32[6:0],1'b0} ^ ({8{i32[7]}} & 8'h1b);
assign s33[7:0] = {i03[6:0],1'b0} ^ ({8{i03[7]}} & 8'h1b) ^ i03[7:0]
                ^  i13[7:0]
                ^  i23[7:0]
                ^ {i33[6:0],1'b0} ^ ({8{i33[7]}} & 8'h1b);

assign mc[127:120] = s33[7:0];
assign mc[119:112] = s23[7:0];
assign mc[111:104] = s13[7:0];
assign mc[103:96]  = s03[7:0];

assign mc[95:88]   = s32[7:0];
assign mc[87:80]   = s22[7:0];
assign mc[79:72]   = s12[7:0];
assign mc[71:64]   = s02[7:0];

assign mc[63:56]   = s31[7:0];
assign mc[55:48]   = s21[7:0];
assign mc[47:40]   = s11[7:0];
assign mc[39:32]   = s01[7:0];

assign mc[31:24]   = s30[7:0];
assign mc[23:16]   = s20[7:0];
assign mc[15:8]    = s10[7:0];
assign mc[7:0]     = s00[7:0];

endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
