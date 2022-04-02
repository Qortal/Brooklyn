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


module herculesae_vx_sha256h32
(


  input wire [127:0]    x,
  input wire [127:0]    y,
  input wire [31:0]     z,

  input wire  [63:0]    sumnr, 
  output wire [63:0]    sum, 
  output wire [63:0]    carry, 
  output wire [127:0]   newx,
  output wire [127:0]   newy 
);







  wire [31:0]                            sigma0;
  wire [31:0]                            sigma1;
  wire [31:0]                            tchoose;
  wire [31:0]                            tmajority;
  wire [32:0]                            x_fa2_c;
  wire [31:0]                            x_fa2_s;
  wire [127:96]                          xprime;
  wire [32:0]                            xy_fa0_c;
  wire [31:0]                            xy_fa0_s;
  wire [32:0]                            xy_fa1_c;
  wire [31:0]                            xy_fa1_s;
  wire [32:0]                            y_fa2_c;
  wire [31:0]                            y_fa2_s;
  wire [31:0]                            y_fa3_c;
  wire [31:0]                            y_fa3_s;
  wire [127:96]                          yprime;


  assign tchoose[31:0] = (y[31:0] & y[63:32]) | (~y[31:0] & y[95:64]);
  assign tmajority[31:0] = (x[31:0] & x[63:32]) | (x[31:0] & x[95:64]) | (x[63:32] & x[95:64]);
  assign sigma0[31:0] = {x[1:0],  x[31:2]}    
                      ^ {x[12:0], x[31:13]}   
                      ^ {x[21:0], x[31:22]};  
  assign sigma1[31:0] = {y[5:0],  y[31:6]}    
                      ^ {y[10:0], y[31:11]}   
                      ^ {y[24:0], y[31:25]};  


  assign xy_fa0_s[31:0] = y[127:96] ^ z[31:0] ^ tchoose[31:0];  
  assign xy_fa0_c[32:0] = {y[127:96] & z[31:0] | tchoose[31:0] & (y[127:96] | z[31:0]), 1'b0};

  assign xy_fa1_s[31:0] = xy_fa0_s[31:0] ^ xy_fa0_c[31:0] ^ sigma1[31:0];
  assign xy_fa1_c[32:0] = {xy_fa0_s[31:0] & xy_fa0_c[31:0] | sigma1[31:0] & (xy_fa0_s[31:0] | xy_fa0_c[31:0]), 1'b0};
  
  assign x_fa2_s[31:0] = xy_fa1_s[31:0] ^ xy_fa1_c[31:0] ^ x[127:96];
  assign x_fa2_c[32:0] = {xy_fa1_s[31:0] & xy_fa1_c[31:0] | x[127:96] & (xy_fa1_s[31:0] | xy_fa1_c[31:0]), 1'b0};

  assign y_fa2_s[31:0] = sigma0[31:0] ^ tmajority[31:0] ^ xy_fa1_c[31:0];
  assign y_fa2_c[32:0] = {sigma0[31:0] & tmajority[31:0] | xy_fa1_c[31:0] & (sigma0[31:0] | tmajority[31:0]), 1'b0};

  assign y_fa3_s[31:0] = y_fa2_s[31:0] ^ y_fa2_c[31:0] ^ xy_fa1_s[31:0];
  assign y_fa3_c[31:0] = {y_fa2_s[30:0] & y_fa2_c[30:0] | xy_fa1_s[30:0] & (y_fa2_s[30:0] | y_fa2_c[30:0]), 1'b0};

  assign sum[63:0]   = {x_fa2_s[31:0], y_fa3_s[31:0]};
  assign carry[63:0] = {x_fa2_c[31:0], y_fa3_c[31:0]};
  assign xprime[127:96] = sumnr[63:32];
  assign yprime[127:96] = sumnr[31:0];
 
  assign newx[127:0] = {x[95:0], yprime[127:96]};
  assign newy[127:0] = {y[95:0], xprime[127:96]};

endmodule



`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
