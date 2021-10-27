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


module herculesae_vx_aesed2_lut
(


  input wire [127:0]    lut_in,


  output wire [127:0]   lut_out

);







  wire [7:0]                             b00;
  wire [7:0]                             b01;
  wire [7:0]                             b02;
  wire [7:0]                             b03;
  wire [7:0]                             b10;
  wire [7:0]                             b11;
  wire [7:0]                             b12;
  wire [7:0]                             b13;
  wire [7:0]                             b20;
  wire [7:0]                             b21;
  wire [7:0]                             b22;
  wire [7:0]                             b23;
  wire [7:0]                             b30;
  wire [7:0]                             b31;
  wire [7:0]                             b32;
  wire [7:0]                             b33;

herculesae_vx_aesinv u_inv_lut0(.lut_in(lut_in[127:120]), .lut_out(b33[7:0]));
herculesae_vx_aesinv u_inv_lut1(.lut_in(lut_in[119:112]), .lut_out(b23[7:0]));
herculesae_vx_aesinv u_inv_lut2(.lut_in(lut_in[111:104]), .lut_out(b13[7:0]));
herculesae_vx_aesinv u_inv_lut3(.lut_in(lut_in[103:96]),  .lut_out(b03[7:0]));

herculesae_vx_aesinv u_inv_lut4(.lut_in(lut_in[95:88]),  .lut_out(b32[7:0]));
herculesae_vx_aesinv u_inv_lut5(.lut_in(lut_in[87:80]),  .lut_out(b22[7:0]));
herculesae_vx_aesinv u_inv_lut6(.lut_in(lut_in[79:72]),  .lut_out(b12[7:0]));
herculesae_vx_aesinv u_inv_lut7(.lut_in(lut_in[71:64]),  .lut_out(b02[7:0]));

herculesae_vx_aesinv u_inv_lut8 (.lut_in(lut_in[63:56]), .lut_out(b31[7:0]));
herculesae_vx_aesinv u_inv_lut9 (.lut_in(lut_in[55:48]), .lut_out(b21[7:0]));
herculesae_vx_aesinv u_inv_lut10(.lut_in(lut_in[47:40]), .lut_out(b11[7:0]));
herculesae_vx_aesinv u_inv_lut11(.lut_in(lut_in[39:32]), .lut_out(b01[7:0]));

herculesae_vx_aesinv u_inv_lut12(.lut_in(lut_in[31:24]), .lut_out(b30[7:0]));
herculesae_vx_aesinv u_inv_lut13(.lut_in(lut_in[23:16]), .lut_out(b20[7:0]));
herculesae_vx_aesinv u_inv_lut14(.lut_in(lut_in[15:8]),  .lut_out(b10[7:0]));
herculesae_vx_aesinv u_inv_lut15(.lut_in(lut_in[7:0]),   .lut_out(b00[7:0]));

assign lut_out[127:0] = {b33[7:0],b23[7:0],b13[7:0],b03[7:0], 
                         b32[7:0],b22[7:0],b12[7:0],b02[7:0], 
                         b31[7:0],b21[7:0],b11[7:0],b01[7:0], 
                         b30[7:0],b20[7:0],b10[7:0],b00[7:0]  
                        };



endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
