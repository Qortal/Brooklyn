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


module herculesae_vx_aesed2
(


  input wire            clk,
  input wire            reset,
  input wire            ival_v1_q,

  input wire [127:0]    aes_din_v1,
  input wire  [15:0]    aes_shf_v1,

  output wire [127:0]   aesd_out,
  output wire [127:0]   aese_out,
  output wire [127:0]   aesemc_out,
  output wire [127:0]   aesdimc_out 
);







  wire [127:0]                           aesimc_in;
  wire [127:0]                           aesmc_in;
  wire [7:0]                             b00_corr_v1;
  wire [7:0]                             b00_redn_v1;
  reg  [7:0]                             b00_redn_v2_q;
  wire [7:0]                             b00_shf_v1;
  wire [7:0]                             b00_v1;
  wire [7:0]                             b01_corr_v1;
  wire [7:0]                             b01_redn_v1;
  reg  [7:0]                             b01_redn_v2_q;
  wire [7:0]                             b01_shf_v1;
  wire [7:0]                             b01_v1;
  wire [7:0]                             b02_corr_v1;
  wire [7:0]                             b02_redn_v1;
  reg  [7:0]                             b02_redn_v2_q;
  wire [7:0]                             b02_shf_v1;
  wire [7:0]                             b02_v1;
  wire [7:0]                             b03_corr_v1;
  wire [7:0]                             b03_redn_v1;
  reg  [7:0]                             b03_redn_v2_q;
  wire [7:0]                             b03_shf_v1;
  wire [7:0]                             b03_v1;
  wire [7:0]                             b10_corr_v1;
  wire [7:0]                             b10_redn_v1;
  reg  [7:0]                             b10_redn_v2_q;
  wire [7:0]                             b10_shf_v1;
  wire [7:0]                             b10_v1;
  wire [7:0]                             b11_corr_v1;
  wire [7:0]                             b11_redn_v1;
  reg  [7:0]                             b11_redn_v2_q;
  wire [7:0]                             b11_shf_v1;
  wire [7:0]                             b11_v1;
  wire [7:0]                             b12_corr_v1;
  wire [7:0]                             b12_redn_v1;
  reg  [7:0]                             b12_redn_v2_q;
  wire [7:0]                             b12_shf_v1;
  wire [7:0]                             b12_v1;
  wire [7:0]                             b13_corr_v1;
  wire [7:0]                             b13_redn_v1;
  reg  [7:0]                             b13_redn_v2_q;
  wire [7:0]                             b13_shf_v1;
  wire [7:0]                             b13_v1;
  wire [7:0]                             b20_corr_v1;
  wire [7:0]                             b20_redn_v1;
  reg  [7:0]                             b20_redn_v2_q;
  wire [7:0]                             b20_shf_v1;
  wire [7:0]                             b20_v1;
  wire [7:0]                             b21_corr_v1;
  wire [7:0]                             b21_redn_v1;
  reg  [7:0]                             b21_redn_v2_q;
  wire [7:0]                             b21_shf_v1;
  wire [7:0]                             b21_v1;
  wire [7:0]                             b22_corr_v1;
  wire [7:0]                             b22_redn_v1;
  reg  [7:0]                             b22_redn_v2_q;
  wire [7:0]                             b22_shf_v1;
  wire [7:0]                             b22_v1;
  wire [7:0]                             b23_corr_v1;
  wire [7:0]                             b23_redn_v1;
  reg  [7:0]                             b23_redn_v2_q;
  wire [7:0]                             b23_shf_v1;
  wire [7:0]                             b23_v1;
  wire [7:0]                             b30_corr_v1;
  wire [7:0]                             b30_redn_v1;
  reg  [7:0]                             b30_redn_v2_q;
  wire [7:0]                             b30_shf_v1;
  wire [7:0]                             b30_v1;
  wire [7:0]                             b31_corr_v1;
  wire [7:0]                             b31_redn_v1;
  reg  [7:0]                             b31_redn_v2_q;
  wire [7:0]                             b31_shf_v1;
  wire [7:0]                             b31_v1;
  wire [7:0]                             b32_corr_v1;
  wire [7:0]                             b32_redn_v1;
  reg  [7:0]                             b32_redn_v2_q;
  wire [7:0]                             b32_shf_v1;
  wire [7:0]                             b32_v1;
  wire [7:0]                             b33_corr_v1;
  wire [7:0]                             b33_redn_v1;
  reg  [7:0]                             b33_redn_v2_q;
  wire [7:0]                             b33_shf_v1;
  wire [7:0]                             b33_v1;



assign b33_v1[7:0] = aes_din_v1[127:120];
assign b23_v1[7:0] = aes_din_v1[119:112];
assign b13_v1[7:0] = aes_din_v1[111:104];
assign b03_v1[7:0] = aes_din_v1[103:96];
assign b32_v1[7:0] = aes_din_v1[95:88];
assign b22_v1[7:0] = aes_din_v1[87:80];
assign b12_v1[7:0] = aes_din_v1[79:72];
assign b02_v1[7:0] = aes_din_v1[71:64];
assign b31_v1[7:0] = aes_din_v1[63:56];
assign b21_v1[7:0] = aes_din_v1[55:48];
assign b11_v1[7:0] = aes_din_v1[47:40];
assign b01_v1[7:0] = aes_din_v1[39:32];
assign b30_v1[7:0] = aes_din_v1[31:24];
assign b20_v1[7:0] = aes_din_v1[23:16];
assign b10_v1[7:0] = aes_din_v1[15:8];
assign b00_v1[7:0] = aes_din_v1[7:0];
assign b33_shf_v1[7:0] = {8{ aes_shf_v1[15]}} & {b33_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[15]}} & {b33_v1[7:0]};
assign b23_shf_v1[7:0] = {8{ aes_shf_v1[14]}} & {b23_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[14]}} & {b23_v1[7:0]};
assign b13_shf_v1[7:0] = {8{ aes_shf_v1[13]}} & {b13_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[13]}} & {b13_v1[7:0]};
assign b03_shf_v1[7:0] = {8{ aes_shf_v1[12]}} & {b03_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[12]}} & {b03_v1[7:0]};
assign b32_shf_v1[7:0] = {8{ aes_shf_v1[11]}} & {b32_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[11]}} & {b32_v1[7:0]};
assign b22_shf_v1[7:0] = {8{ aes_shf_v1[10]}} & {b22_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[10]}} & {b22_v1[7:0]};
assign b12_shf_v1[7:0] = {8{ aes_shf_v1[ 9]}} & {b12_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 9]}} & {b12_v1[7:0]};
assign b02_shf_v1[7:0] = {8{ aes_shf_v1[ 8]}} & {b02_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 8]}} & {b02_v1[7:0]};
assign b31_shf_v1[7:0] = {8{ aes_shf_v1[ 7]}} & {b31_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 7]}} & {b31_v1[7:0]};
assign b21_shf_v1[7:0] = {8{ aes_shf_v1[ 6]}} & {b21_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 6]}} & {b21_v1[7:0]};
assign b11_shf_v1[7:0] = {8{ aes_shf_v1[ 5]}} & {b11_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 5]}} & {b11_v1[7:0]};
assign b01_shf_v1[7:0] = {8{ aes_shf_v1[ 4]}} & {b01_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 4]}} & {b01_v1[7:0]};
assign b30_shf_v1[7:0] = {8{ aes_shf_v1[ 3]}} & {b30_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 3]}} & {b30_v1[7:0]};
assign b20_shf_v1[7:0] = {8{ aes_shf_v1[ 2]}} & {b20_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 2]}} & {b20_v1[7:0]};
assign b10_shf_v1[7:0] = {8{ aes_shf_v1[ 1]}} & {b10_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 1]}} & {b10_v1[7:0]};
assign b00_shf_v1[7:0] = {8{ aes_shf_v1[ 0]}} & {b00_v1[6:0], 1'b0} |
                         {8{~aes_shf_v1[ 0]}} & {b00_v1[7:0]};




assign b33_corr_v1[7:0] = {8{aes_shf_v1[15]}} & {8{b33_v1[7]}} & 8'h1b;
assign b23_corr_v1[7:0] = {8{aes_shf_v1[14]}} & {8{b23_v1[7]}} & 8'h1b;
assign b13_corr_v1[7:0] = {8{aes_shf_v1[13]}} & {8{b13_v1[7]}} & 8'h1b;
assign b03_corr_v1[7:0] = {8{aes_shf_v1[12]}} & {8{b03_v1[7]}} & 8'h1b;
assign b32_corr_v1[7:0] = {8{aes_shf_v1[11]}} & {8{b32_v1[7]}} & 8'h1b;
assign b22_corr_v1[7:0] = {8{aes_shf_v1[10]}} & {8{b22_v1[7]}} & 8'h1b;
assign b12_corr_v1[7:0] = {8{aes_shf_v1[ 9]}} & {8{b12_v1[7]}} & 8'h1b;
assign b02_corr_v1[7:0] = {8{aes_shf_v1[ 8]}} & {8{b02_v1[7]}} & 8'h1b;
assign b31_corr_v1[7:0] = {8{aes_shf_v1[ 7]}} & {8{b31_v1[7]}} & 8'h1b;
assign b21_corr_v1[7:0] = {8{aes_shf_v1[ 6]}} & {8{b21_v1[7]}} & 8'h1b;
assign b11_corr_v1[7:0] = {8{aes_shf_v1[ 5]}} & {8{b11_v1[7]}} & 8'h1b;
assign b01_corr_v1[7:0] = {8{aes_shf_v1[ 4]}} & {8{b01_v1[7]}} & 8'h1b;
assign b30_corr_v1[7:0] = {8{aes_shf_v1[ 3]}} & {8{b30_v1[7]}} & 8'h1b;
assign b20_corr_v1[7:0] = {8{aes_shf_v1[ 2]}} & {8{b20_v1[7]}} & 8'h1b;
assign b10_corr_v1[7:0] = {8{aes_shf_v1[ 1]}} & {8{b10_v1[7]}} & 8'h1b;
assign b00_corr_v1[7:0] = {8{aes_shf_v1[ 0]}} & {8{b00_v1[7]}} & 8'h1b;

assign b33_redn_v1[7:0] = b33_corr_v1[7:0] ^ b33_shf_v1[7:0]; 
assign b23_redn_v1[7:0] = b23_corr_v1[7:0] ^ b23_shf_v1[7:0]; 
assign b13_redn_v1[7:0] = b13_corr_v1[7:0] ^ b13_shf_v1[7:0]; 
assign b03_redn_v1[7:0] = b03_corr_v1[7:0] ^ b03_shf_v1[7:0]; 
assign b32_redn_v1[7:0] = b32_corr_v1[7:0] ^ b32_shf_v1[7:0]; 
assign b22_redn_v1[7:0] = b22_corr_v1[7:0] ^ b22_shf_v1[7:0]; 
assign b12_redn_v1[7:0] = b12_corr_v1[7:0] ^ b12_shf_v1[7:0]; 
assign b02_redn_v1[7:0] = b02_corr_v1[7:0] ^ b02_shf_v1[7:0]; 
assign b31_redn_v1[7:0] = b31_corr_v1[7:0] ^ b31_shf_v1[7:0]; 
assign b21_redn_v1[7:0] = b21_corr_v1[7:0] ^ b21_shf_v1[7:0]; 
assign b11_redn_v1[7:0] = b11_corr_v1[7:0] ^ b11_shf_v1[7:0]; 
assign b01_redn_v1[7:0] = b01_corr_v1[7:0] ^ b01_shf_v1[7:0]; 
assign b30_redn_v1[7:0] = b30_corr_v1[7:0] ^ b30_shf_v1[7:0]; 
assign b20_redn_v1[7:0] = b20_corr_v1[7:0] ^ b20_shf_v1[7:0]; 
assign b10_redn_v1[7:0] = b10_corr_v1[7:0] ^ b10_shf_v1[7:0]; 
assign b00_redn_v1[7:0] = b00_corr_v1[7:0] ^ b00_shf_v1[7:0]; 


  always_ff @(posedge clk or posedge reset)
  begin: u_b33_redn_v2_q_7_0_grp
    if (reset == 1'b1) begin
      b33_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b23_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b13_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b03_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b32_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b22_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b12_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b02_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b31_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b21_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b11_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b01_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b30_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b20_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b10_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
      b00_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v1_q == 1'b1) begin
      b33_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b33_redn_v1[7:0];
      b23_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b23_redn_v1[7:0];
      b13_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b13_redn_v1[7:0];
      b03_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b03_redn_v1[7:0];
      b32_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b32_redn_v1[7:0];
      b22_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b22_redn_v1[7:0];
      b12_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b12_redn_v1[7:0];
      b02_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b02_redn_v1[7:0];
      b31_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b31_redn_v1[7:0];
      b21_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b21_redn_v1[7:0];
      b11_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b11_redn_v1[7:0];
      b01_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b01_redn_v1[7:0];
      b30_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b30_redn_v1[7:0];
      b20_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b20_redn_v1[7:0];
      b10_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b10_redn_v1[7:0];
      b00_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b00_redn_v1[7:0];
    end
    else if (reset == 1'b0 && ival_v1_q == 1'b0)
    begin
    end
    else begin
      b33_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b23_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b13_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b03_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b32_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b22_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b12_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b02_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b31_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b21_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b11_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b01_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b30_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b20_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b10_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
      b00_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY {8{1'bx}};
    end
`else
    else if (ival_v1_q == 1'b1) begin
      b33_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b33_redn_v1[7:0];
      b23_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b23_redn_v1[7:0];
      b13_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b13_redn_v1[7:0];
      b03_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b03_redn_v1[7:0];
      b32_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b32_redn_v1[7:0];
      b22_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b22_redn_v1[7:0];
      b12_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b12_redn_v1[7:0];
      b02_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b02_redn_v1[7:0];
      b31_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b31_redn_v1[7:0];
      b21_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b21_redn_v1[7:0];
      b11_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b11_redn_v1[7:0];
      b01_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b01_redn_v1[7:0];
      b30_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b30_redn_v1[7:0];
      b20_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b20_redn_v1[7:0];
      b10_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b10_redn_v1[7:0];
      b00_redn_v2_q[7:0] <= `HERCULESAE_DFF_DELAY b00_redn_v1[7:0];
    end
`endif
  end



assign aesd_out[127:120] = b33_redn_v2_q[7:0];
assign aesd_out[119:112] = b23_redn_v2_q[7:0];
assign aesd_out[111:104] = b13_redn_v2_q[7:0];
assign aesd_out[103:96]  = b03_redn_v2_q[7:0];

assign aesd_out[95:88] = b32_redn_v2_q[7:0];
assign aesd_out[87:80] = b22_redn_v2_q[7:0];
assign aesd_out[79:72] = b12_redn_v2_q[7:0];
assign aesd_out[71:64] = b02_redn_v2_q[7:0];

assign aesd_out[63:56] = b31_redn_v2_q[7:0];
assign aesd_out[55:48] = b21_redn_v2_q[7:0];
assign aesd_out[47:40] = b11_redn_v2_q[7:0];
assign aesd_out[39:32] = b01_redn_v2_q[7:0];

assign aesd_out[31:24] = b30_redn_v2_q[7:0];
assign aesd_out[23:16] = b20_redn_v2_q[7:0];
assign aesd_out[15:8]  = b10_redn_v2_q[7:0];
assign aesd_out[7:0]   = b00_redn_v2_q[7:0];


assign aesimc_in[127:0] = aesd_out[127:0];

herculesae_vx_aesimc u_aesimc(
        .d_in          (aesimc_in[127:0]),
        .imc            (aesdimc_out[127:0])
);

assign aese_out[0] = b00_redn_v2_q[0] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b1;
assign aese_out[1] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b1;
assign aese_out[2] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b0;
assign aese_out[3] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[7] ^ 1'b0;
assign aese_out[4] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ 1'b0;
assign aese_out[5] = b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ 1'b1;
assign aese_out[6] = b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ 1'b1;
assign aese_out[7] = b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b0;
assign aese_out[8] = b10_redn_v2_q[0] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b1;
assign aese_out[9] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b1;
assign aese_out[10] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b0;
assign aese_out[11] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[7] ^ 1'b0;
assign aese_out[12] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ 1'b0;
assign aese_out[13] = b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ 1'b1;
assign aese_out[14] = b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ 1'b1;
assign aese_out[15] = b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b0;
assign aese_out[16] = b20_redn_v2_q[0] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b1;
assign aese_out[17] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b1;
assign aese_out[18] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b0;
assign aese_out[19] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[7] ^ 1'b0;
assign aese_out[20] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ 1'b0;
assign aese_out[21] = b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ 1'b1;
assign aese_out[22] = b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ 1'b1;
assign aese_out[23] = b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b0;
assign aese_out[24] = b30_redn_v2_q[0] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b1;
assign aese_out[25] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b1;
assign aese_out[26] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b0;
assign aese_out[27] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[7] ^ 1'b0;
assign aese_out[28] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ 1'b0;
assign aese_out[29] = b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ 1'b1;
assign aese_out[30] = b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ 1'b1;
assign aese_out[31] = b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b0;

assign aese_out[32] = b01_redn_v2_q[0] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b1;
assign aese_out[33] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b1;
assign aese_out[34] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b0;
assign aese_out[35] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[7] ^ 1'b0;
assign aese_out[36] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ 1'b0;
assign aese_out[37] = b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ 1'b1;
assign aese_out[38] = b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ 1'b1;
assign aese_out[39] = b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b0;
assign aese_out[40] = b11_redn_v2_q[0] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b1;
assign aese_out[41] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b1;
assign aese_out[42] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b0;
assign aese_out[43] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[7] ^ 1'b0;
assign aese_out[44] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ 1'b0;
assign aese_out[45] = b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ 1'b1;
assign aese_out[46] = b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ 1'b1;
assign aese_out[47] = b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b0;
assign aese_out[48] = b21_redn_v2_q[0] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b1;
assign aese_out[49] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b1;
assign aese_out[50] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b0;
assign aese_out[51] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[7] ^ 1'b0;
assign aese_out[52] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ 1'b0;
assign aese_out[53] = b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ 1'b1;
assign aese_out[54] = b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ 1'b1;
assign aese_out[55] = b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b0;
assign aese_out[56] = b31_redn_v2_q[0] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b1;
assign aese_out[57] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b1;
assign aese_out[58] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b0;
assign aese_out[59] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[7] ^ 1'b0;
assign aese_out[60] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ 1'b0;
assign aese_out[61] = b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ 1'b1;
assign aese_out[62] = b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ 1'b1;
assign aese_out[63] = b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b0;

assign aese_out[64] = b02_redn_v2_q[0] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b1;
assign aese_out[65] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b1;
assign aese_out[66] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b0;
assign aese_out[67] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[7] ^ 1'b0;
assign aese_out[68] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ 1'b0;
assign aese_out[69] = b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ 1'b1;
assign aese_out[70] = b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ 1'b1;
assign aese_out[71] = b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b0;
assign aese_out[72] = b12_redn_v2_q[0] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b1;
assign aese_out[73] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b1;
assign aese_out[74] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b0;
assign aese_out[75] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[7] ^ 1'b0;
assign aese_out[76] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ 1'b0;
assign aese_out[77] = b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ 1'b1;
assign aese_out[78] = b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ 1'b1;
assign aese_out[79] = b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b0;
assign aese_out[80] = b22_redn_v2_q[0] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b1;
assign aese_out[81] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b1;
assign aese_out[82] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b0;
assign aese_out[83] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[7] ^ 1'b0;
assign aese_out[84] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ 1'b0;
assign aese_out[85] = b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ 1'b1;
assign aese_out[86] = b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ 1'b1;
assign aese_out[87] = b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b0;
assign aese_out[88] = b32_redn_v2_q[0] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b1;
assign aese_out[89] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b1;
assign aese_out[90] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b0;
assign aese_out[91] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[7] ^ 1'b0;
assign aese_out[92] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ 1'b0;
assign aese_out[93] = b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ 1'b1;
assign aese_out[94] = b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ 1'b1;
assign aese_out[95] = b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b0;

assign aese_out[96] = b03_redn_v2_q[0] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b1;
assign aese_out[97] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b1;
assign aese_out[98] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b0;
assign aese_out[99] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[7] ^ 1'b0;
assign aese_out[100] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ 1'b0;
assign aese_out[101] = b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ 1'b1;
assign aese_out[102] = b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ 1'b1;
assign aese_out[103] = b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b0;
assign aese_out[104] = b13_redn_v2_q[0] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b1;
assign aese_out[105] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b1;
assign aese_out[106] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b0;
assign aese_out[107] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[7] ^ 1'b0;
assign aese_out[108] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ 1'b0;
assign aese_out[109] = b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ 1'b1;
assign aese_out[110] = b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ 1'b1;
assign aese_out[111] = b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b0;
assign aese_out[112] = b23_redn_v2_q[0] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b1;
assign aese_out[113] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b1;
assign aese_out[114] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b0;
assign aese_out[115] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[7] ^ 1'b0;
assign aese_out[116] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ 1'b0;
assign aese_out[117] = b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ 1'b1;
assign aese_out[118] = b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ 1'b1;
assign aese_out[119] = b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b0;
assign aese_out[120] = b33_redn_v2_q[0] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b1;
assign aese_out[121] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b1;
assign aese_out[122] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b0;
assign aese_out[123] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[7] ^ 1'b0;
assign aese_out[124] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ 1'b0;
assign aese_out[125] = b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ 1'b1;
assign aese_out[126] = b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ 1'b1;
assign aese_out[127] = b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b0;



assign aesmc_in[0] = b00_redn_v2_q[0] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[1] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[2] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[3] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[4] = b00_redn_v2_q[0] ^ b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[5] = b00_redn_v2_q[1] ^ b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[6] = b00_redn_v2_q[2] ^ b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[7] = b00_redn_v2_q[3] ^ b00_redn_v2_q[4] ^ b00_redn_v2_q[5] ^ b00_redn_v2_q[6] ^ b00_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[8] = b10_redn_v2_q[0] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[9] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[10] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[11] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[12] = b10_redn_v2_q[0] ^ b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[13] = b10_redn_v2_q[1] ^ b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[14] = b10_redn_v2_q[2] ^ b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[15] = b10_redn_v2_q[3] ^ b10_redn_v2_q[4] ^ b10_redn_v2_q[5] ^ b10_redn_v2_q[6] ^ b10_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[16] = b20_redn_v2_q[0] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[17] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[18] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[19] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[20] = b20_redn_v2_q[0] ^ b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[21] = b20_redn_v2_q[1] ^ b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[22] = b20_redn_v2_q[2] ^ b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[23] = b20_redn_v2_q[3] ^ b20_redn_v2_q[4] ^ b20_redn_v2_q[5] ^ b20_redn_v2_q[6] ^ b20_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[24] = b30_redn_v2_q[0] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[25] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[26] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[27] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[28] = b30_redn_v2_q[0] ^ b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[29] = b30_redn_v2_q[1] ^ b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[30] = b30_redn_v2_q[2] ^ b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[31] = b30_redn_v2_q[3] ^ b30_redn_v2_q[4] ^ b30_redn_v2_q[5] ^ b30_redn_v2_q[6] ^ b30_redn_v2_q[7] ^ 1'b0;

assign aesmc_in[32] = b01_redn_v2_q[0] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[33] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[34] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[35] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[36] = b01_redn_v2_q[0] ^ b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[37] = b01_redn_v2_q[1] ^ b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[38] = b01_redn_v2_q[2] ^ b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[39] = b01_redn_v2_q[3] ^ b01_redn_v2_q[4] ^ b01_redn_v2_q[5] ^ b01_redn_v2_q[6] ^ b01_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[40] = b11_redn_v2_q[0] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[41] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[42] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[43] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[44] = b11_redn_v2_q[0] ^ b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[45] = b11_redn_v2_q[1] ^ b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[46] = b11_redn_v2_q[2] ^ b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[47] = b11_redn_v2_q[3] ^ b11_redn_v2_q[4] ^ b11_redn_v2_q[5] ^ b11_redn_v2_q[6] ^ b11_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[48] = b21_redn_v2_q[0] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[49] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[50] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[51] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[52] = b21_redn_v2_q[0] ^ b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[53] = b21_redn_v2_q[1] ^ b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[54] = b21_redn_v2_q[2] ^ b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[55] = b21_redn_v2_q[3] ^ b21_redn_v2_q[4] ^ b21_redn_v2_q[5] ^ b21_redn_v2_q[6] ^ b21_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[56] = b31_redn_v2_q[0] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[57] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[58] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[59] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[60] = b31_redn_v2_q[0] ^ b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[61] = b31_redn_v2_q[1] ^ b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[62] = b31_redn_v2_q[2] ^ b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[63] = b31_redn_v2_q[3] ^ b31_redn_v2_q[4] ^ b31_redn_v2_q[5] ^ b31_redn_v2_q[6] ^ b31_redn_v2_q[7] ^ 1'b0;

assign aesmc_in[64] = b02_redn_v2_q[0] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[65] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[66] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[67] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[68] = b02_redn_v2_q[0] ^ b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[69] = b02_redn_v2_q[1] ^ b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[70] = b02_redn_v2_q[2] ^ b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[71] = b02_redn_v2_q[3] ^ b02_redn_v2_q[4] ^ b02_redn_v2_q[5] ^ b02_redn_v2_q[6] ^ b02_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[72] = b12_redn_v2_q[0] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[73] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[74] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[75] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[76] = b12_redn_v2_q[0] ^ b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[77] = b12_redn_v2_q[1] ^ b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[78] = b12_redn_v2_q[2] ^ b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[79] = b12_redn_v2_q[3] ^ b12_redn_v2_q[4] ^ b12_redn_v2_q[5] ^ b12_redn_v2_q[6] ^ b12_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[80] = b22_redn_v2_q[0] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[81] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[82] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[83] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[84] = b22_redn_v2_q[0] ^ b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[85] = b22_redn_v2_q[1] ^ b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[86] = b22_redn_v2_q[2] ^ b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[87] = b22_redn_v2_q[3] ^ b22_redn_v2_q[4] ^ b22_redn_v2_q[5] ^ b22_redn_v2_q[6] ^ b22_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[88] = b32_redn_v2_q[0] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[89] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[90] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[91] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[92] = b32_redn_v2_q[0] ^ b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[93] = b32_redn_v2_q[1] ^ b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[94] = b32_redn_v2_q[2] ^ b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[95] = b32_redn_v2_q[3] ^ b32_redn_v2_q[4] ^ b32_redn_v2_q[5] ^ b32_redn_v2_q[6] ^ b32_redn_v2_q[7] ^ 1'b0;

assign aesmc_in[96] = b03_redn_v2_q[0] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[97] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[98] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[99] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[100] = b03_redn_v2_q[0] ^ b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[101] = b03_redn_v2_q[1] ^ b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[102] = b03_redn_v2_q[2] ^ b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[103] = b03_redn_v2_q[3] ^ b03_redn_v2_q[4] ^ b03_redn_v2_q[5] ^ b03_redn_v2_q[6] ^ b03_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[104] = b13_redn_v2_q[0] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[105] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[106] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[107] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[108] = b13_redn_v2_q[0] ^ b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[109] = b13_redn_v2_q[1] ^ b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[110] = b13_redn_v2_q[2] ^ b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[111] = b13_redn_v2_q[3] ^ b13_redn_v2_q[4] ^ b13_redn_v2_q[5] ^ b13_redn_v2_q[6] ^ b13_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[112] = b23_redn_v2_q[0] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[113] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[114] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[115] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[116] = b23_redn_v2_q[0] ^ b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[117] = b23_redn_v2_q[1] ^ b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[118] = b23_redn_v2_q[2] ^ b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[119] = b23_redn_v2_q[3] ^ b23_redn_v2_q[4] ^ b23_redn_v2_q[5] ^ b23_redn_v2_q[6] ^ b23_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[120] = b33_redn_v2_q[0] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[121] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b1;
assign aesmc_in[122] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[123] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[7] ^ 1'b0;
assign aesmc_in[124] = b33_redn_v2_q[0] ^ b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ 1'b0;
assign aesmc_in[125] = b33_redn_v2_q[1] ^ b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ 1'b1;
assign aesmc_in[126] = b33_redn_v2_q[2] ^ b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ 1'b1;
assign aesmc_in[127] = b33_redn_v2_q[3] ^ b33_redn_v2_q[4] ^ b33_redn_v2_q[5] ^ b33_redn_v2_q[6] ^ b33_redn_v2_q[7] ^ 1'b0;

herculesae_vx_aesmc u_aesmc(
        .d_in          (aesmc_in[127:0]),
        .mc            (aesemc_out[127:0])
);


endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
