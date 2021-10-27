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


module herculesae_vx_aes
(


  input wire            clk,
      input wire   reset,
  input wire            ival_v1_q,
  
  input wire            aesd_v1_q,       
  input wire            aese_v1_q,       
  input wire            aesd_or_e_v1_q,  
  input wire            aesmc_v1_q,      
  input wire            aesimc_v1_q,     
  input wire            aesdimc_v1_q,    
  input wire            aesemc_v1_q,     

  input wire [127:0]    opa_v1,          
  input wire [127:0]    opb_v1,          

  output wire [127:0]   aesout_v2        
);







  wire [15:0]                            aes_shf_v1;
  wire [127:0]                           aesd_out_v2;
  wire [15:0]                            aesd_shf_v1;
  wire [127:0]                           aesd_v1;
  reg                                    aesdimc_h_v2_q;
  reg                                    aesdimc_l_v2_q;
  wire [127:0]                           aesdimc_out_v2;
  wire [127:0]                           aese_out_v2;
  wire [15:0]                            aese_shf_v1;
  wire [127:0]                           aese_v1;
  wire [127:0]                           aesed_lut_in_v1;
  wire [127:0]                           aesed_lut_out_v1;
  reg                                    aesemc_h_v2_q;
  reg                                    aesemc_l_v2_q;
  wire [127:0]                           aesemc_out_v2;
  reg                                    aesimc_h_v2_q;
  reg                                    aesimc_l_v2_q;
  wire [127:0]                           aesimc_out_v2;
  reg                                    aesmc_h_v2_q;
  reg                                    aesmc_l_v2_q;
  wire [127:0]                           aesmc_out_v2;
  wire                                   block_opa_passthrough;
  wire [127:0]                           opa_aes_nxt_v1;
  reg  [127:0]                           opa_aes_v2_q;
  wire [127:0]                           qx_v1;
  reg                                    sel_aesd_h_v2_q;
  reg                                    sel_aesd_l_v2_q;
  wire                                   sel_aesd_v1;
  reg                                    sel_aese_h_v2_q;
  reg                                    sel_aese_l_v2_q;
  wire                                   sel_aese_v1;


  assign sel_aesd_v1 = aesd_v1_q & ~aesdimc_v1_q;
  assign sel_aese_v1 = aese_v1_q & ~aesemc_v1_q;

  assign block_opa_passthrough =  aesd_or_e_v1_q;


  always_ff @(posedge clk or posedge reset)
  begin: u_aesmc_h_v2_q_grp
    if (reset == 1'b1) begin
      aesmc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      aesmc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      aesimc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      aesimc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      aesdimc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      aesdimc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      aesemc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      aesemc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sel_aesd_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sel_aesd_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sel_aese_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sel_aese_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v1_q == 1'b1) begin
      aesmc_h_v2_q <= `HERCULESAE_DFF_DELAY aesmc_v1_q;
      aesmc_l_v2_q <= `HERCULESAE_DFF_DELAY aesmc_v1_q;
      aesimc_h_v2_q <= `HERCULESAE_DFF_DELAY aesimc_v1_q;
      aesimc_l_v2_q <= `HERCULESAE_DFF_DELAY aesimc_v1_q;
      aesdimc_h_v2_q <= `HERCULESAE_DFF_DELAY aesdimc_v1_q;
      aesdimc_l_v2_q <= `HERCULESAE_DFF_DELAY aesdimc_v1_q;
      aesemc_h_v2_q <= `HERCULESAE_DFF_DELAY aesemc_v1_q;
      aesemc_l_v2_q <= `HERCULESAE_DFF_DELAY aesemc_v1_q;
      sel_aesd_h_v2_q <= `HERCULESAE_DFF_DELAY sel_aesd_v1;
      sel_aesd_l_v2_q <= `HERCULESAE_DFF_DELAY sel_aesd_v1;
      sel_aese_h_v2_q <= `HERCULESAE_DFF_DELAY sel_aese_v1;
      sel_aese_l_v2_q <= `HERCULESAE_DFF_DELAY sel_aese_v1;
    end
    else if (reset == 1'b0 && ival_v1_q == 1'b0)
    begin
    end
    else begin
      aesmc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      aesmc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      aesimc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      aesimc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      aesdimc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      aesdimc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      aesemc_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      aesemc_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sel_aesd_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sel_aesd_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sel_aese_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sel_aese_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
    end
`else
    else if (ival_v1_q == 1'b1) begin
      aesmc_h_v2_q <= `HERCULESAE_DFF_DELAY aesmc_v1_q;
      aesmc_l_v2_q <= `HERCULESAE_DFF_DELAY aesmc_v1_q;
      aesimc_h_v2_q <= `HERCULESAE_DFF_DELAY aesimc_v1_q;
      aesimc_l_v2_q <= `HERCULESAE_DFF_DELAY aesimc_v1_q;
      aesdimc_h_v2_q <= `HERCULESAE_DFF_DELAY aesdimc_v1_q;
      aesdimc_l_v2_q <= `HERCULESAE_DFF_DELAY aesdimc_v1_q;
      aesemc_h_v2_q <= `HERCULESAE_DFF_DELAY aesemc_v1_q;
      aesemc_l_v2_q <= `HERCULESAE_DFF_DELAY aesemc_v1_q;
      sel_aesd_h_v2_q <= `HERCULESAE_DFF_DELAY sel_aesd_v1;
      sel_aesd_l_v2_q <= `HERCULESAE_DFF_DELAY sel_aesd_v1;
      sel_aese_h_v2_q <= `HERCULESAE_DFF_DELAY sel_aese_v1;
      sel_aese_l_v2_q <= `HERCULESAE_DFF_DELAY sel_aese_v1;
    end
`endif
  end



  assign qx_v1[127:0] = {128{aesd_or_e_v1_q}} & (opb_v1[127:0] ^ opa_v1[127:0]);

  herculesae_vx_aese1 u_aese1(
        .q              (qx_v1[127:0]),
        .aese_out       (aese_v1[127:0]),
        .aese_shf       (aese_shf_v1[15:0]));

  herculesae_vx_aesd1 u_aesd1(
        .q              (qx_v1[127:0]),
        .aesd_out       (aesd_v1[127:0]),
        .aesd_shf       (aesd_shf_v1[15:0]));

  assign aes_shf_v1[15:0] = {16{aese_v1_q}} & aese_shf_v1[15:0] |
                            {16{aesd_v1_q}} & aesd_shf_v1[15:0];

  assign aesed_lut_in_v1[127:0] =  ({128{aese_v1_q}} & aese_v1[127:0]) | ({128{aesd_v1_q}} & aesd_v1[127:0]);

  herculesae_vx_aesed2_lut u_aesed2_lut_v1(
  .lut_in           (aesed_lut_in_v1[127:0]),
  .lut_out          (aesed_lut_out_v1[127:0]));

  assign opa_aes_nxt_v1[127:0] = ({128{aesd_or_e_v1_q}}         & aesed_lut_out_v1[127:0])
                               | ({128{~block_opa_passthrough}} & opa_v1[127:0]);


  always_ff @(posedge clk or posedge reset)
  begin: u_opa_aes_v2_q_127_0
    if (reset == 1'b1)
      opa_aes_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v1_q == 1'b1)
      opa_aes_v2_q[127:0] <= `HERCULESAE_DFF_DELAY opa_aes_nxt_v1[127:0];
    else if (reset == 1'b0 && ival_v1_q == 1'b0)
    begin
    end
    else
      opa_aes_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
`else
    else if (ival_v1_q == 1'b1)
      opa_aes_v2_q[127:0] <= `HERCULESAE_DFF_DELAY opa_aes_nxt_v1[127:0];
`endif
  end



  herculesae_vx_aesmc u_aesmc(
        .d_in          (opa_aes_v2_q[127:0]),
        .mc            (aesmc_out_v2[127:0]));

  herculesae_vx_aesimc u_aesimc(
        .d_in          (opa_aes_v2_q[127:0]),
        .imc           (aesimc_out_v2[127:0]));

  herculesae_vx_aesed2 u_aesed2(
        .clk           (clk),
        .reset         (reset),
        .ival_v1_q     (ival_v1_q),
        .aes_din_v1    (aesed_lut_out_v1[127:0]),
        .aes_shf_v1    (aes_shf_v1[15:0]),
        .aesd_out      (aesd_out_v2[127:0]),
        .aese_out      (aese_out_v2[127:0]),
        .aesemc_out    (aesemc_out_v2[127:0]),
        .aesdimc_out   (aesdimc_out_v2[127:0]));

  assign aesout_v2[127:64] = ({64{sel_aesd_h_v2_q}}        & aesd_out_v2[127:64])
                           | ({64{sel_aese_h_v2_q}}        & aese_out_v2[127:64])
                           | ({64{aesmc_h_v2_q}}           & aesmc_out_v2[127:64])
                           | ({64{aesemc_h_v2_q}}          & aesemc_out_v2[127:64])
                           | ({64{aesimc_h_v2_q}}          & aesimc_out_v2[127:64])
                           | ({64{aesdimc_h_v2_q}}         & aesdimc_out_v2[127:64]);

  assign aesout_v2[63:0] =   ({64{sel_aesd_l_v2_q}}        & aesd_out_v2[63:0])
                           | ({64{sel_aese_l_v2_q}}        & aese_out_v2[63:0])
                           | ({64{aesmc_l_v2_q}}           & aesmc_out_v2[63:0])
                           | ({64{aesemc_l_v2_q}}          & aesemc_out_v2[63:0])
                           | ({64{aesimc_l_v2_q}}          & aesimc_out_v2[63:0])
                           | ({64{aesdimc_l_v2_q}}         & aesdimc_out_v2[63:0]);


endmodule  


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
