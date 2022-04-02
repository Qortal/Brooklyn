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


module herculesae_vx_sha256su1
(


  input wire            sha256su1_x_op,
  input wire            sha256su1_y_op,
  input wire            sha256su1_z_op,
  input wire [63:0]     x,
  input wire [63:0]     y,
  input wire [63:0]     z,
  input wire  [63:0]    sumnr, 
  output wire [63:0]    sum_3to2, 
  output wire [63:0]    carry_3to2, 
  output wire [63:0]   newx 
);







  wire [63:0]                            carry;
  wire [63:0]                            sum;
  wire [63:0]                            x_nop;
  wire [63:0]                            y_nop;
  wire [63:0]                            z_nop;
  wire [63:0]                            z_rot;
  wire [63:0]                            zror17;
  wire [63:0]                            zror19;
  wire [63:0]                            zshr10;




assign x_nop[63:0] = x[63:0] &  {64{sha256su1_x_op}};
assign y_nop[63:0] = y[63:0] &  {64{sha256su1_y_op}};
assign z_nop[63:0] = z[63:0] &  {64{sha256su1_z_op}};

assign zror17[63:0] = {z_nop[48:32],  z_nop[63:49],
                       z_nop[16:0],   z_nop[31:17]};
assign zror19[63:0] = {z_nop[50:32],  z_nop[63:51],
                       z_nop[18:0],   z_nop[31:19]};
assign zshr10[63:0] = {10'b00_0000_0000, z_nop[63:42],
                       10'b00_0000_0000, z_nop[31:10]};
assign z_rot[63:0] = zror17[63:0] ^ zror19[63:0] ^ zshr10[63:0];

assign sum[63:0] = (x_nop[63:0] ^ y_nop[63:0]) ^ z_rot[63:0];
assign carry[63:32] = {(x_nop[62:32] & y_nop[62:32]) 
                    |  (y_nop[62:32] & z_rot[62:32])
                    |  (x_nop[62:32] & z_rot[62:32]), 1'b0};
assign carry[31:0] = {(x_nop[30:0]   & y_nop[30:0]) 
                   |  (y_nop[30:0]   & z_rot[30:0])
                   |  (x_nop[30:0]   & z_rot[30:0]), 1'b0};

assign sum_3to2[63:0] = sum[63:0];
assign carry_3to2[63:0] = carry[63:0];
assign newx[63:0] = sumnr[63:0];

endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
