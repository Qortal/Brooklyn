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


module herculesae_vx_crypt
(

  input wire            clk,
  input wire            reset,
  
  input wire            ival_v1_q,
  input wire            sha1c_v1_q,      
  input wire            sha1p_v1_q,      
  input wire            sha1m_v1_q,      
  input wire            sha1cpm_v1_q,    
  input wire            sha256h_v1_q,    
  input wire            sha256h2_v1_q,   
  input wire            sha256hh2_v1_q,  
  input wire            sha1h_v1_q,      
  input wire            sha1su0_v1_q,    
  input wire            sha1su1_v1_q,    
  input wire            sha256su0_v1_q,  
  input wire            sha256su1_v1_q,  
  input wire            sha256su1_dup_x_v1_q,  
  input wire            sha256su1_dup_y_v1_q,  
  input wire            sha256su1_dup_z_v1_q,  

  input wire [127:0]    opa_v1,          
  input wire [127:0]    opb_v1,          
  input wire [127:0]    opc_v1,          

  output wire [127:0]   cryptout_v2,     
  output wire [127:0]   cryptout_v4,     
  output wire           crypt_active
);







  wire                                   block_opa_passthrough;
  wire [63:0]                            carry1c_v1;
  wire [63:0]                            carry4c_v3;
  wire [31:0]                            carry4c_v4;
  wire [63:0]                            carry_2c4c_v2;
  wire [31:0]                            carry_sha1cpm_v1;
  wire [31:0]                            carry_sha1cpm_v2;
  wire [31:0]                            carry_sha1cpm_v3;
  wire [31:0]                            carry_sha1cpm_v4;
  wire [63:0]                            carry_sha256h32_v1;
  wire [63:0]                            carry_sha256h32_v2;
  wire [63:0]                            carry_sha256h32_v3;
  wire [31:0]                            carry_sha256h32_v4;
  wire [31:0]                            carry_sha256su0_v1;
  wire [63:0]                            carry_sha256su1_v1;
  wire [63:0]                            carry_sha256su1_v2;
  wire                                   ival_en;
  wire                                   ival_v1_or_v2;
  wire                                   ival_v2_4latency;
  reg                                    ival_v2_q;
  reg                                    ival_v3_q;
  wire [127:0]                           newa_v1;
  wire [127:0]                           newa_v2;
  wire [127:0]                           newa_v3;
  wire [127:0]                           newb_v1;
  wire [127:0]                           newb_v2;
  wire [127:0]                           newb_v3;
  wire [95:0]                            newc_v1;
  wire [127:0]                           newx_v4;
  wire [127:0]                           newy_v4;
  reg  [127:0]                           opa_v2_q;
  reg  [127:0]                           opa_v3_q;
  reg  [127:0]                           opa_v4_q;
  reg  [127:0]                           opb_v2_q;
  reg  [127:0]                           opb_v3_q;
  reg  [127:0]                           opb_v4_q;
  reg  [95:0]                            opc_v2_q;
  reg  [63:0]                            opc_v3_q;
  reg  [31:0]                            opc_v4_q;
  wire [127:0]                           sha1_out_v1;
  wire                                   sha1_v1;
  wire [127:0]                           sha1_xin_v1;
  wire [31:0]                            sha1_yin_v1;
  wire [31:0]                            sha1_zin_v1;
  reg                                    sha1c_v2_q;
  reg                                    sha1c_v3_q;
  reg                                    sha1c_v4_q;
  reg                                    sha1cpm_h_v4_q;
  reg                                    sha1cpm_l_v4_q;
  reg                                    sha1cpm_v2_q;
  reg                                    sha1cpm_v3_q;
  reg                                    sha1cpm_v4_q;
  wire [127:0]                           sha1cpm_x_v1;
  wire [127:0]                           sha1cpm_x_v2;
  wire [127:0]                           sha1cpm_x_v3;
  wire [127:0]                           sha1cpm_x_v4;
  wire [127:0]                           sha1cpm_y_v1;
  wire [127:0]                           sha1cpm_y_v2;
  wire [127:0]                           sha1cpm_y_v3;
  wire [31:0]                            sha1cpm_y_v4;
  wire [31:0]                            sha1h_qnin_v1;
  reg                                    sha1m_v2_q;
  reg                                    sha1m_v3_q;
  reg                                    sha1m_v4_q;
  reg                                    sha1p_v2_q;
  reg                                    sha1p_v3_q;
  reg                                    sha1p_v4_q;
  wire [127:0]                           sha1su0_q_v1;
  reg                                    sha1su0_v2_q;
  wire [127:0]                           sha1su1_qdin_v1;
  wire [127:0]                           sha1su1_qnin_v1;
  wire [127:0]                           sha256_xin_v1;
  wire [127:0]                           sha256_yin_v1;
  wire [31:0]                            sha256_zin_v1;
  reg                                    sha256h2_h_v4_q;
  reg                                    sha256h2_l_v4_q;
  reg                                    sha256h2_v2_q;
  reg                                    sha256h2_v3_q;
  reg                                    sha256h2_v4_q;
  reg                                    sha256h_h_v4_q;
  reg                                    sha256h_l_v4_q;
  reg                                    sha256h_v2_q;
  reg                                    sha256h_v3_q;
  reg                                    sha256h_v4_q;
  wire [127:0]                           sha256h_x_v1;
  wire [127:0]                           sha256h_x_v2;
  wire [127:0]                           sha256h_x_v3;
  wire [127:0]                           sha256h_x_v4;
  wire [127:0]                           sha256h_y_v1;
  wire [127:0]                           sha256h_y_v2;
  wire [127:0]                           sha256h_y_v3;
  wire [127:0]                           sha256h_y_v4;
  wire                                   sha256hh2_v2;
  wire                                   sha256hh2_v3;
  wire                                   sha256hh2_v4;
  wire [127:0]                           sha256su0_out_v1;
  wire [127:0]                           sha256su0_qdin_v1;
  wire [127:0]                           sha256su0_qnin_v1;
  reg                                    sha256su1_dup_x_v2_q;
  reg                                    sha256su1_dup_y_v2_q;
  reg                                    sha256su1_dup_z_v2_q;
  reg                                    sha256su1_h_v2_q;
  reg                                    sha256su1_l_v2_q;
  reg                                    sha256su1_v2_q;
  wire [63:0]                            sha256su1_x_v1;
  wire [63:0]                            sha256su1_x_v2;
  reg                                    sha_inst_h_v2_q;
  reg                                    sha_inst_l_v2_q;
  wire                                   sha_inst_v1;
  reg                                    sha_inst_v2_q;
  wire                                   short_pipe_out_v3_en;
  wire [31:0]                            sigma0_v3;
  wire [31:0]                            sigma0_v4;
  wire [31:0]                            sigma1_v3;
  wire [63:0]                            sum1c_v1;
  wire [63:0]                            sum4c_v3;
  wire [31:0]                            sum4c_v4;
  wire [63:0]                            sum_2c4c_v2;
  wire [31:0]                            sum_sha1cpm_v1;
  wire [31:0]                            sum_sha1cpm_v2;
  wire [31:0]                            sum_sha1cpm_v3;
  wire [31:0]                            sum_sha1cpm_v4;
  wire [63:0]                            sum_sha256h32_v1;
  wire [63:0]                            sum_sha256h32_v2;
  wire [63:0]                            sum_sha256h32_v3;
  wire [31:0]                            sum_sha256h32_v4;
  wire [31:0]                            sum_sha256su0_v1;
  wire [63:0]                            sum_sha256su1_v1;
  wire [63:0]                            sum_sha256su1_v2;
  wire [63:0]                            sumnr1c_v1;
  wire [63:0]                            sumnr4c_v3;
  wire [31:0]                            sumnr4c_v4;
  wire [63:0]                            sumnr_2c4c_v2;
  wire [63:0]                            sumres_sha256su1_v2;
  wire [31:0]                            tchoose_v3;
  wire [31:0]                            tmajority_v3;
  wire [31:0]                            tmajority_v4;
  wire                                   unused_cout1c2_v1;
  wire                                   unused_cout1c_v1;
  wire                                   unused_cout2_2c4c_v2;
  wire                                   unused_cout2_4c_v3;
  wire                                   unused_cout4c_v3;
  wire                                   unused_cout4c_v4;
  wire                                   unused_cout_2c4c_v2;
  wire                                   unused_cout_sha256su1h_v2;
  wire                                   unused_cout_sha256su1l_v2;
  wire [32:0]                            x_fa2_c_v4;
  wire [31:0]                            x_fa2_s_v4;
  wire [127:0]                           x_v1;
  wire [127:0]                           x_v2;
  wire [127:0]                           x_v3;
  wire [127:0]                           x_v4;
  wire                                   xprime_carry;
  wire [127:96]                          xprime_v4;
  wire [32:0]                            xy_fa0_c_v3;
  wire [31:0]                            xy_fa0_s_v3;
  wire [32:0]                            xy_fa1_c_v3;
  wire [31:0]                            xy_fa1_c_v4;
  wire [31:0]                            xy_fa1_s_v3;
  wire [31:0]                            xy_fa1_s_v4;
  wire [32:0]                            y_fa2_c_v4;
  wire [31:0]                            y_fa2_s_v4;
  wire [32:0]                            y_fa3_c_v4;
  wire [31:0]                            y_fa3_s_v4;
  wire [127:0]                           y_v1;
  wire [127:0]                           y_v2;
  wire [127:0]                           y_v3;
  wire [127:0]                           y_v4;
  wire [127:96]                          yprime_v4;
  wire [127:0]                           z_v1;
  wire [95:0]                            z_v2;
  wire [63:0]                            z_v3;
  wire [31:0]                            z_v4;

  

  assign ival_en = ival_v1_q | ival_v2_q | ival_v3_q;

  assign short_pipe_out_v3_en = sha_inst_v2_q | sha1su0_v2_q | sha256su1_v2_q;

  assign ival_v2_4latency = ~short_pipe_out_v3_en & ival_v2_q; 

  assign ival_v1_or_v2 = ival_v1_q | ival_v2_q;


  always_ff @(posedge clk or posedge reset)
  begin: u_ival_v2_q
    if (reset == 1'b1)
      ival_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_en == 1'b1)
      ival_v2_q <= `HERCULESAE_DFF_DELAY ival_v1_q;
    else if (reset == 1'b0 && ival_en == 1'b0)
    begin
    end
    else
      ival_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
`else
    else if (ival_en == 1'b1)
      ival_v2_q <= `HERCULESAE_DFF_DELAY ival_v1_q;
`endif
  end


  always_ff @(posedge clk or posedge reset)
  begin: u_ival_v3_q
    if (reset == 1'b1)
      ival_v3_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_en == 1'b1)
      ival_v3_q <= `HERCULESAE_DFF_DELAY ival_v2_4latency;
    else if (reset == 1'b0 && ival_en == 1'b0)
    begin
    end
    else
      ival_v3_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
`else
    else if (ival_en == 1'b1)
      ival_v3_q <= `HERCULESAE_DFF_DELAY ival_v2_4latency;
`endif
  end



  always_ff @(posedge clk or posedge reset)
  begin: u_sha1c_v2_q_grp
    if (reset == 1'b1) begin
      sha1c_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1p_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1m_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h2_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1cpm_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha_inst_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha_inst_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha_inst_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1su0_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256su1_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256su1_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256su1_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256su1_dup_x_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256su1_dup_y_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256su1_dup_z_v2_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v1_q == 1'b1) begin
      sha1c_v2_q <= `HERCULESAE_DFF_DELAY sha1c_v1_q;
      sha1p_v2_q <= `HERCULESAE_DFF_DELAY sha1p_v1_q;
      sha1m_v2_q <= `HERCULESAE_DFF_DELAY sha1m_v1_q;
      sha256h_v2_q <= `HERCULESAE_DFF_DELAY sha256h_v1_q;
      sha256h2_v2_q <= `HERCULESAE_DFF_DELAY sha256h2_v1_q;
      sha1cpm_v2_q <= `HERCULESAE_DFF_DELAY sha1cpm_v1_q;
      sha_inst_v2_q <= `HERCULESAE_DFF_DELAY sha_inst_v1;
      sha_inst_h_v2_q <= `HERCULESAE_DFF_DELAY sha_inst_v1;
      sha_inst_l_v2_q <= `HERCULESAE_DFF_DELAY sha_inst_v1;
      sha1su0_v2_q <= `HERCULESAE_DFF_DELAY sha1su0_v1_q;
      sha256su1_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_h_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_l_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_dup_x_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_dup_y_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_dup_z_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
    end
    else if (reset == 1'b0 && ival_v1_q == 1'b0)
    begin
    end
    else begin
      sha1c_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1p_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1m_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h2_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1cpm_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha_inst_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha_inst_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha_inst_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1su0_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256su1_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256su1_h_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256su1_l_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256su1_dup_x_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256su1_dup_y_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256su1_dup_z_v2_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
    end
`else
    else if (ival_v1_q == 1'b1) begin
      sha1c_v2_q <= `HERCULESAE_DFF_DELAY sha1c_v1_q;
      sha1p_v2_q <= `HERCULESAE_DFF_DELAY sha1p_v1_q;
      sha1m_v2_q <= `HERCULESAE_DFF_DELAY sha1m_v1_q;
      sha256h_v2_q <= `HERCULESAE_DFF_DELAY sha256h_v1_q;
      sha256h2_v2_q <= `HERCULESAE_DFF_DELAY sha256h2_v1_q;
      sha1cpm_v2_q <= `HERCULESAE_DFF_DELAY sha1cpm_v1_q;
      sha_inst_v2_q <= `HERCULESAE_DFF_DELAY sha_inst_v1;
      sha_inst_h_v2_q <= `HERCULESAE_DFF_DELAY sha_inst_v1;
      sha_inst_l_v2_q <= `HERCULESAE_DFF_DELAY sha_inst_v1;
      sha1su0_v2_q <= `HERCULESAE_DFF_DELAY sha1su0_v1_q;
      sha256su1_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_h_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_l_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_dup_x_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_dup_y_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
      sha256su1_dup_z_v2_q <= `HERCULESAE_DFF_DELAY sha256su1_v1_q;
    end
`endif
  end



  always_ff @(posedge clk or posedge reset)
  begin: u_sha1c_v3_q_grp
    if (reset == 1'b1) begin
      sha1c_v3_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1p_v3_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1m_v3_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1cpm_v3_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h_v3_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h2_v3_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v2_4latency == 1'b1) begin
      sha1c_v3_q <= `HERCULESAE_DFF_DELAY sha1c_v2_q;
      sha1p_v3_q <= `HERCULESAE_DFF_DELAY sha1p_v2_q;
      sha1m_v3_q <= `HERCULESAE_DFF_DELAY sha1m_v2_q;
      sha1cpm_v3_q <= `HERCULESAE_DFF_DELAY sha1cpm_v2_q;
      sha256h_v3_q <= `HERCULESAE_DFF_DELAY sha256h_v2_q;
      sha256h2_v3_q <= `HERCULESAE_DFF_DELAY sha256h2_v2_q;
    end
    else if (reset == 1'b0 && ival_v2_4latency == 1'b0)
    begin
    end
    else begin
      sha1c_v3_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1p_v3_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1m_v3_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1cpm_v3_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h_v3_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h2_v3_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
    end
`else
    else if (ival_v2_4latency == 1'b1) begin
      sha1c_v3_q <= `HERCULESAE_DFF_DELAY sha1c_v2_q;
      sha1p_v3_q <= `HERCULESAE_DFF_DELAY sha1p_v2_q;
      sha1m_v3_q <= `HERCULESAE_DFF_DELAY sha1m_v2_q;
      sha1cpm_v3_q <= `HERCULESAE_DFF_DELAY sha1cpm_v2_q;
      sha256h_v3_q <= `HERCULESAE_DFF_DELAY sha256h_v2_q;
      sha256h2_v3_q <= `HERCULESAE_DFF_DELAY sha256h2_v2_q;
    end
`endif
  end



  always_ff @(posedge clk or posedge reset)
  begin: u_sha1c_v4_q_grp
    if (reset == 1'b1) begin
      sha1c_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1p_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1m_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1cpm_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1cpm_h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha1cpm_l_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h_h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h_l_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h2_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h2_h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
      sha256h2_l_v4_q <= `HERCULESAE_DFF_DELAY {1{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v3_q == 1'b1) begin
      sha1c_v4_q <= `HERCULESAE_DFF_DELAY sha1c_v3_q;
      sha1p_v4_q <= `HERCULESAE_DFF_DELAY sha1p_v3_q;
      sha1m_v4_q <= `HERCULESAE_DFF_DELAY sha1m_v3_q;
      sha1cpm_v4_q <= `HERCULESAE_DFF_DELAY sha1cpm_v3_q;
      sha1cpm_h_v4_q <= `HERCULESAE_DFF_DELAY sha1cpm_v3_q;
      sha1cpm_l_v4_q <= `HERCULESAE_DFF_DELAY sha1cpm_v3_q;
      sha256h_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h_h_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h_l_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h2_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h2_h_v4_q <= `HERCULESAE_DFF_DELAY sha256h2_v3_q;
      sha256h2_l_v4_q <= `HERCULESAE_DFF_DELAY sha256h2_v3_q;
    end
    else if (reset == 1'b0 && ival_v3_q == 1'b0)
    begin
    end
    else begin
      sha1c_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1p_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1m_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1cpm_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1cpm_h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha1cpm_l_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h_h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h_l_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h2_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h2_h_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
      sha256h2_l_v4_q <= `HERCULESAE_DFF_DELAY {1{1'bx}};
    end
`else
    else if (ival_v3_q == 1'b1) begin
      sha1c_v4_q <= `HERCULESAE_DFF_DELAY sha1c_v3_q;
      sha1p_v4_q <= `HERCULESAE_DFF_DELAY sha1p_v3_q;
      sha1m_v4_q <= `HERCULESAE_DFF_DELAY sha1m_v3_q;
      sha1cpm_v4_q <= `HERCULESAE_DFF_DELAY sha1cpm_v3_q;
      sha1cpm_h_v4_q <= `HERCULESAE_DFF_DELAY sha1cpm_v3_q;
      sha1cpm_l_v4_q <= `HERCULESAE_DFF_DELAY sha1cpm_v3_q;
      sha256h_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h_h_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h_l_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h2_v4_q <= `HERCULESAE_DFF_DELAY sha256h_v3_q;
      sha256h2_h_v4_q <= `HERCULESAE_DFF_DELAY sha256h2_v3_q;
      sha256h2_l_v4_q <= `HERCULESAE_DFF_DELAY sha256h2_v3_q;
    end
`endif
  end






  assign sha1_v1 = sha1h_v1_q | sha1su0_v1_q | sha1su1_v1_q;

  assign sha1h_qnin_v1[ 31:0]   = {32{sha1h_v1_q}}    & opa_v1[ 31:0];
  assign sha1su1_qdin_v1[127:0] = {128{sha1su0_v1_q | sha1su1_v1_q}} & opb_v1[127:0];
  assign sha1su1_qnin_v1[127:0] = {128{sha1su0_v1_q | sha1su1_v1_q}} & opa_v1[127:0];
  assign sha1su0_q_v1[127:0]    = {128{sha1su0_v1_q}}                & opc_v1[127:0];

  herculesae_vx_sha1 u_sha1(
        .sha1h_v1_i    (sha1h_v1_q),
        .sha1su0_v1_i  (sha1su0_v1_q),
        .sha1su1_v1_i  (sha1su1_v1_q),
        .sha1h_qn      (sha1h_qnin_v1[31:0]),
        .sha1su0_qd    (sha1su0_q_v1[127:0]),
        .sha1su1_qd    (sha1su1_qdin_v1[127:0]),
        .sha1su1_qn    (sha1su1_qnin_v1[127:0]),
        .sha1_v1_o     (sha1_out_v1[127:0]));



  assign sha256su0_qdin_v1[127:0] = {128{sha256su0_v1_q}} & opb_v1[127:0];
  assign sha256su0_qnin_v1[127:0] = {128{sha256su0_v1_q}} & opa_v1[127:0];

  herculesae_vx_sha256su0 u_sha256su0(
        .qd     (sha256su0_qdin_v1[127:0]),
        .qn     (sha256su0_qnin_v1[ 31:0]),
        .sumd   (sumnr1c_v1[31:0]),          
        .suma   (sum_sha256su0_v1[31:0]),
        .sumb   (carry_sha256su0_v1[31:0]),
        .d      (sha256su0_out_v1[127:0]));




  herculesae_vx_sha256su1 u_sha256su1_v1(
     .sha256su1_x_op   (sha256su1_dup_x_v1_q),
     .sha256su1_y_op   (sha256su1_dup_y_v1_q),
     .sha256su1_z_op   (sha256su1_dup_z_v1_q),
     .x                (opc_v1[63:0]),            
     .y                (opa_v1[95:32]),           
     .z                (opb_v1[127:64]),          
     .sumnr            (sumnr1c_v1[63:0]),        
     .sum_3to2         (sum_sha256su1_v1[63:0]),
     .carry_3to2       (carry_sha256su1_v1[63:0]),
     .newx             (sha256su1_x_v1[63:0]));



  assign x_v1[127:0] = opc_v1[127:0]; 
  assign y_v1[127:0] = opa_v1[127:0];
  assign z_v1[127:0] = opb_v1[127:0];

  assign sha1_xin_v1[127:0] = {128{sha1cpm_v1_q}} & x_v1[127:0];
  assign sha1_yin_v1[31:0]  = { 32{sha1cpm_v1_q}} & y_v1[31:0];
  assign sha1_zin_v1[31:0]  = { 32{sha1cpm_v1_q}} & z_v1[31:0];

  herculesae_vx_sha1cpm u_sha1cpm_v1(
     .choose           (sha1c_v1_q),
     .parity           (sha1p_v1_q),
     .majority         (sha1m_v1_q),
     .cpm              (sha1cpm_v1_q),
     .x                (sha1_xin_v1[127:0]),
     .y                (sha1_yin_v1[31:0]),
     .z                (sha1_zin_v1[31:0]),
     .t2               (sumnr1c_v1[31:0]),
     .fa1_s            (sum_sha1cpm_v1[31:0]),
     .fa1_c            (carry_sha1cpm_v1[31:0]),
     .newx             (sha1cpm_x_v1[127:0]),
     .newy             (sha1cpm_y_v1[31:0]));
  assign sha1cpm_y_v1[127:32] = {96{sha1cpm_v1_q}} & y_v1[127:32];  

  assign sha256_xin_v1[127:0] = {128{sha256hh2_v1_q}} & x_v1[127:0];
  assign sha256_yin_v1[127:0] = {128{sha256hh2_v1_q}} & y_v1[127:0];
  assign sha256_zin_v1[ 31:0] = {32{ sha256hh2_v1_q}} & z_v1[31:0];

  herculesae_vx_sha256h32 u_sha256h32_v1(
     .x                (sha256_xin_v1[127:0]),
     .y                (sha256_yin_v1[127:0]),
     .z                (sha256_zin_v1[31:0]),
     .sumnr            (sumnr1c_v1[63:0]),
     .sum              (sum_sha256h32_v1[63:0]),
     .carry            (carry_sha256h32_v1[63:0]),
     .newx             (sha256h_x_v1[127:0]),
     .newy             (sha256h_y_v1[127:0]));


  assign sum1c_v1[31:0] = {32{sha256su0_v1_q}} & sum_sha256su0_v1[31:0]
                        | {32{sha1cpm_v1_q  }} & sum_sha1cpm_v1[31:0]
                        | {32{sha256hh2_v1_q}} & sum_sha256h32_v1[31:0]
                        | {32{sha256su1_v1_q}} & sum_sha256su1_v1[31:0];
  assign carry1c_v1[31:0] = {32{sha256su0_v1_q}} & carry_sha256su0_v1[31:0]
                          | {32{sha1cpm_v1_q  }} & carry_sha1cpm_v1[31:0]
                          | {32{sha256hh2_v1_q}} & carry_sha256h32_v1[31:0]
                          | {32{sha256su1_v1_q}} & carry_sha256su1_v1[31:0];


  assign {unused_cout1c_v1, sumnr1c_v1[31:0]} = sum1c_v1[31:0] + carry1c_v1[31:0] + {{31{1'b0}}, 1'b0};


  assign sum1c_v1[63:32] = {32{sha256hh2_v1_q}} & sum_sha256h32_v1[63:32]
                         | {32{sha256su1_v1_q}} & sum_sha256su1_v1[63:32];
  assign carry1c_v1[63:32] = {32{sha256hh2_v1_q}} & carry_sha256h32_v1[63:32]
                           | {32{sha256su1_v1_q}} & carry_sha256su1_v1[63:32];


  assign {unused_cout1c2_v1, sumnr1c_v1[63:32]} = sum1c_v1[63:32] + carry1c_v1[63:32] + {{31{1'b0}}, 1'b0};



  assign sha_inst_v1 = sha1h_v1_q | sha1su0_v1_q | sha1su1_v1_q | sha256su0_v1_q;

  assign block_opa_passthrough = sha_inst_v1    | sha256su1_v1_q | 
                                 sha256hh2_v1_q | sha1cpm_v1_q;

  assign newa_v1[127:0] = ({128{sha1cpm_v1_q  }}  & sha1cpm_y_v1[127:0])
                        | ({128{sha256hh2_v1_q}}  & sha256h_y_v1[127:0])
                        | ({128{sha256su1_v1_q}}  & {opb_v1[31:0],opa_v1[127:32]})
                        | ({128{sha256su0_v1_q}}  & sha256su0_out_v1[127:0])
                        | ({128{sha1_v1}}         & sha1_out_v1[127:0])
                        | ({128{~(block_opa_passthrough)}} & opa_v1[127:0]);

  assign newb_v1[127:0] = ({128{sha1cpm_v1_q  }}  & sha1cpm_x_v1[127:0])
                        | ({128{sha256hh2_v1_q}}  & sha256h_x_v1[127:0])
                        | ({128{sha256su1_v1_q}}  & {opc_v1[127:64], sha256su1_x_v1[63:0]});

  assign newc_v1[95:0] = opb_v1[127:32]; 



  always_ff @(posedge clk or posedge reset)
  begin: u_opa_v2_q_127_0_grp
    if (reset == 1'b1) begin
      opa_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
      opb_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
      opc_v2_q[95:0] <= `HERCULESAE_DFF_DELAY {96{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v1_q == 1'b1) begin
      opa_v2_q[127:0] <= `HERCULESAE_DFF_DELAY newa_v1[127:0];
      opb_v2_q[127:0] <= `HERCULESAE_DFF_DELAY newb_v1[127:0];
      opc_v2_q[95:0] <= `HERCULESAE_DFF_DELAY newc_v1[95:0];
    end
    else if (reset == 1'b0 && ival_v1_q == 1'b0)
    begin
    end
    else begin
      opa_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
      opb_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
      opc_v2_q[95:0] <= `HERCULESAE_DFF_DELAY {96{1'bx}};
    end
`else
    else if (ival_v1_q == 1'b1) begin
      opa_v2_q[127:0] <= `HERCULESAE_DFF_DELAY newa_v1[127:0];
      opb_v2_q[127:0] <= `HERCULESAE_DFF_DELAY newb_v1[127:0];
      opc_v2_q[95:0] <= `HERCULESAE_DFF_DELAY newc_v1[95:0];
    end
`endif
  end


  assign x_v2[127:0] = opb_v2_q[127:0];
  assign y_v2[127:0] = opa_v2_q[127:0];
  assign z_v2[95:0]  = opc_v2_q[95:0];


  herculesae_vx_sha256su1 u_sha256su1_v2(
     .sha256su1_x_op   (sha256su1_dup_x_v2_q),
     .sha256su1_y_op   (sha256su1_dup_y_v2_q),
     .sha256su1_z_op   (sha256su1_dup_z_v2_q),
     .x                (x_v2[127:64]),         
     .y                (y_v2[127:64]),         
     .z                (x_v2[63:0]),           
     .sumnr            (sumres_sha256su1_v2[63:0]),  
     .sum_3to2         (sum_sha256su1_v2[63:0]),
     .carry_3to2       (carry_sha256su1_v2[63:0]),
     .newx             (sha256su1_x_v2[63:0]));



  herculesae_vx_sha1cpm u_sha1cpm_v2(
     .choose           (sha1c_v2_q),
     .parity           (sha1p_v2_q),
     .majority         (sha1m_v2_q),
     .cpm              (sha1cpm_v2_q),
     .x                (x_v2[127:0]),
     .y                (y_v2[31:0]),
     .z                (z_v2[31:0]),
     .t2               (sumnr_2c4c_v2[31:0]),
     .fa1_s            (sum_sha1cpm_v2[31:0]),
     .fa1_c            (carry_sha1cpm_v2[31:0]),
     .newx             (sha1cpm_x_v2[127:0]),
     .newy             (sha1cpm_y_v2[31:0]));

  assign sha1cpm_y_v2[127:32] = y_v2[127:32];
  
  herculesae_vx_sha256h32 u_sha256h32_v2(
     .x                (x_v2[127:0]),
     .y                (y_v2[127:0]),
     .z                (z_v2[31:0]),
     .sumnr            (sumnr_2c4c_v2[63:0]),
     .sum              (sum_sha256h32_v2[63:0]),
     .carry            (carry_sha256h32_v2[63:0]),
     .newx             (sha256h_x_v2[127:0]),
     .newy             (sha256h_y_v2[127:0]));


  assign sha256hh2_v2 = sha256h_v2_q | sha256h2_v2_q;


  assign sum_2c4c_v2[31:0] =   {32{sha1cpm_v2_q}}   & sum_sha1cpm_v2[31:0] |
                               {32{sha256hh2_v2}}   & sum_sha256h32_v2[31:0];
  assign carry_2c4c_v2[31:0] = {32{sha1cpm_v2_q}}   & carry_sha1cpm_v2[31:0] |
                               {32{sha256hh2_v2}}   & carry_sha256h32_v2[31:0];

  assign {unused_cout_2c4c_v2, sumnr_2c4c_v2[31:0]} = sum_2c4c_v2[31:0] + carry_2c4c_v2[31:0] + {{31{1'b0}}, 1'b0};




  assign sum_2c4c_v2[63:32] =   {32{sha256hh2_v2}}   & sum_sha256h32_v2[63:32];
  assign carry_2c4c_v2[63:32] = {32{sha256hh2_v2}}   & carry_sha256h32_v2[63:32];

  assign {unused_cout2_2c4c_v2, sumnr_2c4c_v2[63:32]} = sum_2c4c_v2[63:32] + carry_2c4c_v2[63:32] + {{31{1'b0}}, 1'b0};



  assign {unused_cout_sha256su1l_v2, sumres_sha256su1_v2[31:0]} = sum_sha256su1_v2[31:0] + carry_sha256su1_v2[31:0] + {{31{1'b0}}, 1'b0};


  assign {unused_cout_sha256su1h_v2, sumres_sha256su1_v2[63:32]} = sum_sha256su1_v2[63:32] + carry_sha256su1_v2[63:32] + {{31{1'b0}}, 1'b0};



  assign newb_v2[127:0] = ({128{sha1cpm_v2_q}} & sha1cpm_x_v2[127:0])
                        | ({128{sha256hh2_v2}} & sha256h_x_v2[127:0]);

  assign newa_v2[127:0] = ({128{sha1cpm_v2_q}} & sha1cpm_y_v2[127:0])
                        | ({128{sha256hh2_v2}} & sha256h_y_v2[127:0]);


  assign cryptout_v2[127:64] =  ({64{sha256su1_h_v2_q}}  & sha256su1_x_v2[63:0])
                              | ({64{sha_inst_h_v2_q}}   & opa_v2_q[127:64]);

  assign cryptout_v2[63:0] =    ({64{sha256su1_l_v2_q}}  & opb_v2_q[63:0])
                              | ({64{sha_inst_l_v2_q}}   & opa_v2_q[63:0]);



  always_ff @(posedge clk or posedge reset)
  begin: u_opa_v3_q_127_0_grp
    if (reset == 1'b1) begin
      opa_v3_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
      opb_v3_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
      opc_v3_q[63:0] <= `HERCULESAE_DFF_DELAY {64{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v2_4latency == 1'b1) begin
      opa_v3_q[127:0] <= `HERCULESAE_DFF_DELAY newa_v2[127:0];
      opb_v3_q[127:0] <= `HERCULESAE_DFF_DELAY newb_v2[127:0];
      opc_v3_q[63:0] <= `HERCULESAE_DFF_DELAY opc_v2_q[95:32];
    end
    else if (reset == 1'b0 && ival_v2_4latency == 1'b0)
    begin
    end
    else begin
      opa_v3_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
      opb_v3_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
      opc_v3_q[63:0] <= `HERCULESAE_DFF_DELAY {64{1'bx}};
    end
`else
    else if (ival_v2_4latency == 1'b1) begin
      opa_v3_q[127:0] <= `HERCULESAE_DFF_DELAY newa_v2[127:0];
      opb_v3_q[127:0] <= `HERCULESAE_DFF_DELAY newb_v2[127:0];
      opc_v3_q[63:0] <= `HERCULESAE_DFF_DELAY opc_v2_q[95:32];
    end
`endif
  end



  assign x_v3[127:0] = opb_v3_q[127:0];
  assign y_v3[127:0] = opa_v3_q[127:0];
  assign z_v3[63:0] =  opc_v3_q[63:0];

  herculesae_vx_sha1cpm u_sha1cpm_v3(
     .choose           (sha1c_v3_q),
     .parity           (sha1p_v3_q),
     .majority         (sha1m_v3_q),
     .cpm              (sha1cpm_v3_q),
     .x                (x_v3[127:0]),
     .y                (y_v3[31:0]),
     .z                (z_v3[31:0]),
     .t2               (sumnr4c_v3[31:0]),
     .fa1_s            (sum_sha1cpm_v3[31:0]),
     .fa1_c            (carry_sha1cpm_v3[31:0]),
     .newx             (sha1cpm_x_v3[127:0]),
     .newy             (sha1cpm_y_v3[31:0]));
  assign sha1cpm_y_v3[127:32] = y_v3[127:32];  

  herculesae_vx_sha256h32 u_sha256h32_v3(
     .x                (x_v3[127:0]),
     .y                (y_v3[127:0]),
     .z                (z_v3[31:0]),
     .sumnr            (sumnr4c_v3[63:0]),
     .sum              (sum_sha256h32_v3[63:0]),
     .carry            (carry_sha256h32_v3[63:0]),
     .newx             (sha256h_x_v3[127:0]),
     .newy             (sha256h_y_v3[127:0]));

  assign tchoose_v3[31:0] = (sha256h_y_v3[31:0] & sha256h_y_v3[63:32]) | 
                            (~sha256h_y_v3[31:0] & sha256h_y_v3[95:64]);

  assign tmajority_v3[31:0] = (sha256h_x_v3[31:0] & sha256h_x_v3[63:32]) |
                              (sha256h_x_v3[31:0] & sha256h_x_v3[95:64]) | 
                              (sha256h_x_v3[63:32] & sha256h_x_v3[95:64]);

  assign sigma0_v3[31:0] = {sha256h_x_v3[1:0],  sha256h_x_v3[31:2]}    
                         ^ {sha256h_x_v3[12:0], sha256h_x_v3[31:13]}   
                         ^ {sha256h_x_v3[21:0], sha256h_x_v3[31:22]};  

  assign sigma1_v3[31:0] = {sha256h_y_v3[5:0],  sha256h_y_v3[31:6]}    
                         ^ {sha256h_y_v3[10:0], sha256h_y_v3[31:11]}   
                         ^ {sha256h_y_v3[24:0], sha256h_y_v3[31:25]};  


  assign xy_fa0_s_v3[31:0] = sha256h_y_v3[127:96] ^ z_v3[63:32] ^ tchoose_v3[31:0];

  assign xy_fa0_c_v3[32:0] = {sha256h_y_v3[127:96] & z_v3[63:32] | tchoose_v3[31:0] & 
                             (sha256h_y_v3[127:96] | z_v3[63:32]), 1'b0};



  assign xy_fa1_s_v3[31:0] = xy_fa0_s_v3[31:0] ^ xy_fa0_c_v3[31:0] ^ sigma1_v3[31:0];

  assign xy_fa1_c_v3[32:0] = {xy_fa0_s_v3[31:0] & xy_fa0_c_v3[31:0] | sigma1_v3[31:0] & 
                             (xy_fa0_s_v3[31:0] | xy_fa0_c_v3[31:0]), 1'b0};


  assign sha256hh2_v3 = sha256h_v3_q | sha256h2_v3_q;

  assign sum4c_v3[31:0] =   {32{sha1cpm_v3_q}} & sum_sha1cpm_v3[31:0] |
                            {32{sha256hh2_v3}} & sum_sha256h32_v3[31:0];
  assign carry4c_v3[31:0] = {32{sha1cpm_v3_q}} & carry_sha1cpm_v3[31:0] |
                            {32{sha256hh2_v3}} & carry_sha256h32_v3[31:0];

  assign {unused_cout4c_v3, sumnr4c_v3[31:0]} = sum4c_v3[31:0] + carry4c_v3[31:0] + {{31{1'b0}}, 1'b0};


  assign sum4c_v3[63:32]   = sum_sha256h32_v3[63:32];
  assign carry4c_v3[63:32] = carry_sha256h32_v3[63:32];

  assign {unused_cout2_4c_v3, sumnr4c_v3[63:32]} = sum4c_v3[63:32] + carry4c_v3[63:32] + {{31{1'b0}}, 1'b0};

 


  assign newa_v3[127:0] = ({128{sha1cpm_v3_q}}  & sha1cpm_y_v3[127:0])
                        | ({128{sha256h_v3_q}}  & {sigma0_v3[31:0],   tmajority_v3[31:0],
                                                   xy_fa1_s_v3[31:0], xy_fa1_c_v3[31:0]})
                        | ({128{sha256h2_v3_q}} & {sigma0_v3[31:0],tmajority_v3[31:0],
                                                   xy_fa1_s_v3[31:0],xy_fa1_c_v3[31:0]});

  assign newb_v3[127:0] = ({128{sha1cpm_v3_q}}   &  sha1cpm_x_v3[127:0])
                        | ({128{sha256h_v3_q  }} &  sha256h_x_v3[127:0]) 
                        | ({128{sha256h2_v3_q }} & {sha256h_x_v3[127:96], sha256h_y_v3[95:0]});


  always_ff @(posedge clk or posedge reset)
  begin: u_opb_v4_q_127_0_grp
    if (reset == 1'b1) begin
      opb_v4_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
      opa_v4_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
      opc_v4_q[31:0] <= `HERCULESAE_DFF_DELAY {32{1'b0}};
    end
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v3_q == 1'b1) begin
      opb_v4_q[127:0] <= `HERCULESAE_DFF_DELAY newb_v3[127:0];
      opa_v4_q[127:0] <= `HERCULESAE_DFF_DELAY newa_v3[127:0];
      opc_v4_q[31:0] <= `HERCULESAE_DFF_DELAY opc_v3_q[63:32];
    end
    else if (reset == 1'b0 && ival_v3_q == 1'b0)
    begin
    end
    else begin
      opb_v4_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
      opa_v4_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
      opc_v4_q[31:0] <= `HERCULESAE_DFF_DELAY {32{1'bx}};
    end
`else
    else if (ival_v3_q == 1'b1) begin
      opb_v4_q[127:0] <= `HERCULESAE_DFF_DELAY newb_v3[127:0];
      opa_v4_q[127:0] <= `HERCULESAE_DFF_DELAY newa_v3[127:0];
      opc_v4_q[31:0] <= `HERCULESAE_DFF_DELAY opc_v3_q[63:32];
    end
`endif
  end



  assign x_v4[127:0] = opb_v4_q[127:0];
  assign y_v4[127:0] = opa_v4_q[127:0];
  assign z_v4[31:0] =  opc_v4_q[31:0];

  herculesae_vx_sha1cpm u_sha1cpm_v4(
     .choose           (sha1c_v4_q),
     .parity           (sha1p_v4_q),
     .majority         (sha1m_v4_q),
     .cpm              (sha1cpm_v4_q),
     .x                (x_v4[127:0]),
     .y                (y_v4[31:0]),
     .z                (z_v4[31:0]),
     .t2               (sumnr4c_v4[31:0]),
     .fa1_s            (sum_sha1cpm_v4[31:0]),
     .fa1_c            (carry_sha1cpm_v4[31:0]),
     .newx             (sha1cpm_x_v4[127:0]),
     .newy             (sha1cpm_y_v4[31:0]));

  assign sigma0_v4[31:0]    = y_v4[127:96];
  assign tmajority_v4[31:0] = y_v4[95:64];
  assign xy_fa1_s_v4[31:0]  = y_v4[63:32];
  assign xy_fa1_c_v4[31:0]  = y_v4[31:0];

  assign x_fa2_s_v4[31:0] = xy_fa1_s_v4[31:0] ^ xy_fa1_c_v4[31:0] ^ x_v4[127:96];
  assign x_fa2_c_v4[32:0] = {xy_fa1_s_v4[31:0] & xy_fa1_c_v4[31:0] |
                             x_v4[127:96] & (xy_fa1_s_v4[31:0] | 
                             xy_fa1_c_v4[31:0]), 1'b0};

  assign y_fa2_s_v4[31:0] = sigma0_v4[31:0] ^ tmajority_v4[31:0] ^ xy_fa1_c_v4[31:0];

  assign y_fa2_c_v4[32:0] = {sigma0_v4[31:0] & tmajority_v4[31:0] | 
                             xy_fa1_c_v4[31:0] & (sigma0_v4[31:0] | 
                             tmajority_v4[31:0]), 1'b0};


  assign y_fa3_s_v4[31:0] = y_fa2_s_v4[31:0] ^ y_fa2_c_v4[31:0] ^ xy_fa1_s_v4[31:0];
  assign y_fa3_c_v4[32:0] = {y_fa2_s_v4[31:0] & y_fa2_c_v4[31:0] | 
                             xy_fa1_s_v4[31:0] & (y_fa2_s_v4[31:0] | 
                             y_fa2_c_v4[31:0]), 1'b0};


  assign {xprime_carry, xprime_v4[127:96]} = x_fa2_s_v4[31:0] + x_fa2_c_v4[31:0] + {{31{1'b0}}, 1'b0};



  assign sha256hh2_v4 = sha256h_v4_q | sha256h2_v4_q;
  assign sum_sha256h32_v4[31:0] = y_fa3_s_v4[31:0];
  assign carry_sha256h32_v4[31:0] = y_fa3_c_v4[31:0];

  assign sum4c_v4[31:0] =   {32{sha1cpm_v4_q}} & sum_sha1cpm_v4[31:0] |
                            {32{sha256hh2_v4}} & sum_sha256h32_v4[31:0];
  assign carry4c_v4[31:0] = {32{sha1cpm_v4_q}} & carry_sha1cpm_v4[31:0] |
                            {32{sha256hh2_v4}} & carry_sha256h32_v4[31:0];

  assign {unused_cout4c_v4, sumnr4c_v4[31:0]} = sum4c_v4[31:0] + carry4c_v4[31:0] + {{31{1'b0}}, 1'b0};

  assign yprime_v4[127:96] = sumnr4c_v4[31:0];
  
  assign newx_v4[127:0] = {x_v4[95:0], yprime_v4[127:96]};
  assign newy_v4[127:0] = {x_v4[95:0], xprime_v4[127:96]};

  assign sha256h_x_v4[127:0] = newx_v4[127:0];
  assign sha256h_y_v4[127:0] = newy_v4[127:0];


  assign cryptout_v4[63:0] = ({64{sha1cpm_l_v4_q}}   & sha1cpm_x_v4[63:0])
                           | ({64{sha256h_l_v4_q}}   & sha256h_x_v4[63:0])
                           | ({64{sha256h2_l_v4_q}}  & sha256h_y_v4[63:0]);

  assign cryptout_v4[127:64] = ({64{sha1cpm_h_v4_q}}   & sha1cpm_x_v4[127:64])
                             | ({64{sha256h_h_v4_q}}   & sha256h_x_v4[127:64])
                             | ({64{sha256h2_h_v4_q}}  & sha256h_y_v4[127:64]);


  assign crypt_active  = ival_v1_or_v2;

endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
