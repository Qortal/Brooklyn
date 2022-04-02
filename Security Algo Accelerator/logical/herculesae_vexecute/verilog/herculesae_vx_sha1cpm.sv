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


module herculesae_vx_sha1cpm
(


  input wire            choose,
  input wire            parity,
  input wire            majority,
  input wire            cpm,
  input wire [127:0]    x,
  input wire [31:0]     y,
  input wire [31:0]     z,
  input wire  [31:0]    t2, 
  output wire [31:0]    fa1_s, 
  output wire [31:0]    fa1_c, 
  output wire [127:0]   newx,
  output wire [31:0]    newy 
);







  wire [32:0]                            fa0_c;
  wire [31:0]                            fa0_s;
  wire                                   shacpm_nop;
  wire [31:0]                            t1_nop;
  wire [31:0]                            t1c;
  wire [31:0]                            t1m;
  wire [31:0]                            t1p;
  wire [127:0]                           x1;
  wire [127:0]                           x_nop;
  wire [31:0]                            x_rol5_nop;
  wire [31:0]                            y_nop;
  wire [31:0]                            z_nop;

  assign shacpm_nop = ~cpm;
  

  assign t1c[31:0] = (x[63:32] & x[95:64]) | (~x[63:32] & x[127:96]);
  assign t1p[31:0] = x[63:32] ^ x[95:64] ^ x[127:96];
  assign t1m[31:0] = (x[63:32] & x[95:64])
                      | (x[63:32] & x[127:96])
                      | (x[95:64] & x[127:96]);
  assign t1_nop[31:0] = ({32{choose}}   & t1c[31:0])
                  | ({32{parity}}   & t1p[31:0])
                  | ({32{majority}} & t1m[31:0]);
  assign x_rol5_nop[31:0] = {32{~shacpm_nop}} & {x[26:0], x[31:27]};


  assign y_nop[31:0] = {32{~shacpm_nop}} & y[31:0];
  assign z_nop[31:0] = {32{~shacpm_nop}} & z[31:0];
    
  assign fa0_s[31:0] = y_nop[31:0] ^ x_rol5_nop[31:0] ^ z_nop[31:0];
  assign fa0_c[32:0] = {y_nop[31:0] & x_rol5_nop[31:0] | z_nop[31:0] & (y_nop[31:0] | x_rol5_nop[31:0]), 1'b0};
  
  assign fa1_s[31:0] = fa0_s[31:0] ^ fa0_c[31:0] ^ t1_nop[31:0];
  assign fa1_c[31:0] = {fa0_s[30:0] & fa0_c[30:0] | t1_nop[30:0] & (fa0_s[30:0] | fa0_c[30:0]), 1'b0};


  assign x_nop[127:0] = {128{~shacpm_nop}} & x[127:0];

  assign x1[127:64] = x_nop[127:64];
  assign x1[63:32] = {x_nop[33:32], x_nop[63:34]}; 
  assign x1[31:0] = x_nop[31:0];

  assign newx[127:0] = {x1[95:0], t2[31:0]};
  assign newy[31:0] = x1[127:96];  

endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
