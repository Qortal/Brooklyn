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


module herculesae_vx_sha1
(


  input wire            sha1h_v1_i,
  input wire            sha1su0_v1_i,
  input wire            sha1su1_v1_i,
  input wire [31:0]     sha1h_qn,
  input wire [127:0]    sha1su0_qd,
  input wire [127:0]    sha1su1_qd,
  input wire [127:0]    sha1su1_qn,

  output wire [127:0]   sha1_v1_o 
);







  wire [31:0]                            sha1h_v1;
  wire [63:0]                            sha1su0_opa_v1;
  wire [127:0]                           sha1su0_opb_v1;
  wire [127:0]                           sha1su0_opc_v1;
  wire [127:0]                           sha1su0_v1;
  wire [127:0]                           sha1su1_v1;
  wire [127:0]                           t;

  assign sha1h_v1[31:0] = {sha1h_qn[1:0], sha1h_qn[31:2]};

  assign sha1su0_opa_v1[63:0]  = sha1su1_qn[63:0];
  assign sha1su0_opb_v1[127:0] = sha1su1_qd[127:0];
  assign sha1su0_opc_v1[127:0] = sha1su0_qd[127:0];

  assign sha1su0_v1 [127:0] = sha1su0_opc_v1[127:0]
                           ^ {sha1su0_opa_v1[63:0], sha1su0_opc_v1[127:64]}
                           ^  sha1su0_opb_v1[127:0];
  assign t[127:0] = sha1su1_qd[127:0] ^ {{32{1'b0}}, sha1su1_qn[127:32]};
  assign sha1su1_v1[127:96] = {t[126:96], t[127]} ^ {t[29:0], t[31:30]}; 
  assign sha1su1_v1[95:64] = {t[94:64], t[95]};
  assign sha1su1_v1[63:32] = {t[62:32], t[63]};
  assign sha1su1_v1[31:0]  = {t[30:0],  t[31]};

  assign sha1_v1_o[127:0] = {128{sha1su0_v1_i}} & sha1su0_v1[127:0] |
                            {128{sha1su1_v1_i}} & sha1su1_v1[127:0] |
                            {128{sha1h_v1_i}}   & {{96{1'b0}}, sha1h_v1[31:0]};
endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
