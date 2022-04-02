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


module herculesae_vx_sha256su0
(


  input wire [127:0]    qd,
  input wire [31:0]     qn,
  input wire  [31:0]    sumd, 
  output wire [31:0]    suma, 
  output wire [31:0]    sumb, 
  output wire [127:0]   d 
);







  wire                                   d1_cout;
  wire                                   d2_cout;
  wire                                   d3_cout;
  wire [127:0]                           t;
  wire [127:0]                           t0;

assign t[127:0] = {qn[31:0], qd[127:32]};
assign t0[127:96] = {t[102:96], t[127:103]} ^ {t[113:96], t[127:114]} ^ {3'b000, t[127:99]};
assign t0[95:64]  = {t[ 70:64], t[ 95: 71]} ^ {t[ 81:64], t[ 95: 82]} ^ {3'b000, t[ 95:67]};
assign t0[63:32]  = {t[ 38:32], t[ 63: 39]} ^ {t[ 49:32], t[ 63: 50]} ^ {3'b000, t[ 63:35]};
assign t0[31:0]   = {t[  6: 0], t[ 31:  7]} ^ {t[ 17: 0], t[ 31: 18]} ^ {3'b000, t[ 31: 3]};


  assign {d3_cout, d[127:96]} = t0[127:96] + qd[127:96] + {{31{1'b0}}, 1'b0};


  assign {d2_cout, d[95:64]} = t0[95:64] + qd[95:64] + {{31{1'b0}}, 1'b0};


  assign {d1_cout, d[63:32]} = t0[63:32] + qd[63:32] + {{31{1'b0}}, 1'b0};


assign suma[31:0] = t0[31:0];
assign sumb[31:0] = qd[31:0];
assign d[31:0] = sumd[31:0];


endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
