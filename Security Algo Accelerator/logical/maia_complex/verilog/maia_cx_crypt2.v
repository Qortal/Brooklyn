
//-----------------------------------------------------------------------------
//     The confidential and proprietary information contained in this file may
//     only be used by a person authorised under and to the extent permitted
//     by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2013-2014 ARM Limited.
//                ALL RIGHTS RESERVED
//
//     This entire notice must be reproduced on all copies of this file
//     and copies of this file may only be made by a person if such person is
//     permitted to do so under the terms of a subsisting license agreement
//     from ARM Limited.
//
//     Filename            : $RCSfile: maia_cx_crypt2.v $
//     Checked In          : $Date: 2014-08-29 00:16:46 -0500 (Fri, 29 Aug 2014) $
//     Revision            : $Revision: 70482 $
//     Release Information : Cortex-A72-r1p0-00rel0
//
//-----------------------------------------------------------------------------
// Verilog-2001 (IEEE Std 1364-2001)
//-----------------------------------------------------------------------------

//#
//# Overview
//# ========
//#

// This block does the following operations:
//   - AES encrypt and decrypt operations: aesd, aese, aesmc, aesimc
//   - SHA single-cycle operations: sha1h, sha1su1, sha256su0

//#
//# Module Declaration
//# ==================
//#

`include "maia_header.v"

module maia_cx_crypt2 (

//#
//# Interface Signals
//# =================
//#

// Global inputs
  ck_gclkcx_crypt,
  cx_reset3,
  
// Control inputs
  ival_e1_q,
  aesd_e1_q,
  aese_e1_q,
  aesmc_e1_q,
  aesimc_e1_q,
  aesdimc_e1_q,
  aesemc_e1_q,
  pmull_e1_q,
  sha1h_e1_q,
  sha1su1_e1_q,
  sha256su0_e1_q,


// Data inputs
  qd,
  qn,
 
   
// Outputs
  crypt2_out_e3_q,
  crypt2_active
);


//#
//# Interface Signals
//# =================
//#

// Global inputs
  input            ck_gclkcx_crypt;
  input            cx_reset3;
  
// Control inputs
  input            ival_e1_q;
  input            aesd_e1_q;      // aes encode
  input            aese_e1_q;      // aes decode
  input            aesmc_e1_q;     // ae smix columns
  input            aesimc_e1_q;    // aes inverse mix columns
  input            aesdimc_e1_q;   // aes decode superop
  input            aesemc_e1_q;    // aes encode superop  
  input            pmull_e1_q;     // polynomial multiplication
  input            sha1h_e1_q;     // sha1 fixed rotate
  input            sha1su1_e1_q;   // sha1 schedule update 1
  input            sha256su0_e1_q; // sha256 schedule update 0


// Data inputs
  input [127:0]    qd;
  input [127:0]    qn;
 
   
// Outputs
  output [127:0]   crypt2_out_e3_q;
  output           crypt2_active;

//#
//# Internal Signals - Automatic Declarations
//# =========================================
//#
  wire [ 15:  0] aes_shf_e1;
  reg  [ 15:  0] aes_shf_e2_q;
  wire [127:  0] aesd_e1;
  reg            aesd_e2_q;
  wire           aesd_or_e_e1;
  wire [127:  0] aesd_out;
  wire [ 15:  0] aesd_shf_e1;
  reg            aesdimc_e2_q;
  wire [127:  0] aesdimc_out;
  wire [127:  0] aese_e1;
  reg            aese_e2_q;
  wire [127:  0] aese_out;
  wire [ 15:  0] aese_shf_e1;
  reg            aesemc_e2_q;
  wire [127:  0] aesemc_out;
  reg            aesimc_e2_q;
  wire [127:  0] aesimc_in;
  wire [127:  0] aesimc_out;
  reg            aesmc_e2_q;
  wire [127:  0] aesmc_in;
  wire [127:  0] aesmc_out;
  wire [127:  0] crypt2_d_e1;
  reg  [127:  0] crypt2_d_e2_q;
  wire [127:  0] crypt2_out_e2;
  reg  [127:  0] crypt2_out_e3_q;
  reg            ival_e2_q;
  reg            pmull_e2_q;
  wire [127:  0] pmull_out;
  wire [127:  0] qx_e1;
  wire [ 31:  0] sha1h_in_e1;
  wire [ 31:  0] sha1h_out_e1;
  wire [127:  0] sha1su1_out_e1;
  wire [127:  0] sha1su1_qdin_e1;
  wire [127:  0] sha1su1_qnin_e1;
  wire [127:  0] sha256su0_out_e1;
  wire           sha_inst_e1;
  reg            sha_inst_e2_q;
  
//#
//# Main Code
//# =========
//#     
//

// aes functions are all in the same block because of limited result bus bandwidth.
// Mais CX has 3x64-bit result buses, and each of these instructions produces
// a 128-bit result.  Two instructions could be issued in a cycle, but there is
// no value in doing this because they could not both write results.
//
// The single-cycle 2-input SHA instructions are in the same block because they have the same inputs
// and latency as the aes instructions.
//
// Originally, all functions in this block had single-cycle latency, but CX is unable to make use
// of single-cycle latency.  To reduce area, functionality is spread across E1 and E2 
// In particular, the AES SBOX and ISBOX functions are split into LUT(mult inverse) -> affine transform
// & affine inverse transform -> LUT(mult inverse), so that they can share the same LUT.

// E1
// 38% of this cycle is used up to drive qd and qn from the issq block.  Therefore, the relatively 
// shallow SHA operations are performed in this cycle, along with some preliminary processing for AESE and AESD

assign qx_e1[127:0] = {128{aesd_or_e_e1}} & (qd[127:0] ^ qn[127:0]);
  
  maia_cx_aese1 uaese1(
        .q              (qx_e1[127:0]),
        .aese_out       (aese_e1[127:0]),
        .aese_shf       (aese_shf_e1[15:0])
);

  maia_cx_aesd1 uaesd1(
        .q              (qx_e1[127:0]),
        .aesd_out       (aesd_e1[127:0]),
        .aesd_shf       (aesd_shf_e1[15:0])
);

assign aesd_or_e_e1 = aesd_e1_q | aese_e1_q;

// Perform sha functions in E1 to save pipeline flops
// and reduce complexity of multiplexer in E2

assign sha1h_in_e1[31:0] = {32{sha1h_e1_q}} & qn[31:0];

  maia_cx_sha1h usha1h(
        .qn             (sha1h_in_e1[31:0]),
        .d              (sha1h_out_e1[31:0])
);

assign sha1su1_qdin_e1[127:0] = {128{sha1su1_e1_q}} & qd[127:0];
assign sha1su1_qnin_e1[127:0] = {128{sha1su1_e1_q}} & qn[127:0];

  maia_cx_sha1su1 usha1su1(
        .qd             (sha1su1_qdin_e1[127:0]),
        .qn             (sha1su1_qnin_e1[127:0]),
        .d              (sha1su1_out_e1[127:0])
);


  maia_cx_sha256su0 usha256su0(
        .qd             (qd[127:0]),
        .qn             (qn[127:0]),
        .d              (sha256su0_out_e1[127:0])
);

assign sha_inst_e1 = sha1h_e1_q | sha1su1_e1_q | sha256su0_e1_q;

assign crypt2_d_e1[127:0] = ({128{sha1h_e1_q}}      & {{96{1'b0}}, sha1h_out_e1[31:0]})
                          | ({128{sha1su1_e1_q}}    & sha1su1_out_e1[127:0])
                          | ({128{sha256su0_e1_q}}  & sha256su0_out_e1[127:0])
                          | ({128{aese_e1_q}}       & aese_e1[127:0])
                          | ({128{aesd_e1_q}}       & aesd_e1[127:0])
                          | ({128{~(aesd_or_e_e1 | sha_inst_e1)}} & qn[127:0]);

assign aes_shf_e1[15:0] = {16{aese_e1_q}} & aese_shf_e1[15:0] |
                          {16{aesd_e1_q}} & aesd_shf_e1[15:0];

// reset flop(s) since feeds into active signal used for RCG
  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt or posedge cx_reset3)
  begin: uival_e2_q
    if (cx_reset3 == 1'b1)
      ival_e2_q <= `MAIA_DFF_DELAY {1{1'b0}};
`ifdef MAIA_XPROP_FLOP
    else if (cx_reset3==1'b0)
      ival_e2_q <= `MAIA_DFF_DELAY ival_e1_q;
    else
      ival_e2_q <= `MAIA_DFF_DELAY {1{1'bx}};
`else
    else
      ival_e2_q <= `MAIA_DFF_DELAY ival_e1_q;
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF


  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: ucrypt2_e2
    if (ival_e1_q==1'b1) begin 
      crypt2_d_e2_q[127:0] <= `MAIA_DFF_DELAY crypt2_d_e1[127:0];
      aes_shf_e2_q[15:0]   <= `MAIA_DFF_DELAY aes_shf_e1[15:0];
      aesd_e2_q            <= `MAIA_DFF_DELAY aesd_e1_q;
      aese_e2_q            <= `MAIA_DFF_DELAY aese_e1_q;
      aesmc_e2_q           <= `MAIA_DFF_DELAY aesmc_e1_q;
      aesimc_e2_q          <= `MAIA_DFF_DELAY aesimc_e1_q;
      aesemc_e2_q          <= `MAIA_DFF_DELAY aesemc_e1_q;
      aesdimc_e2_q         <= `MAIA_DFF_DELAY aesdimc_e1_q;
      pmull_e2_q           <= `MAIA_DFF_DELAY pmull_e1_q;
      sha_inst_e2_q        <= `MAIA_DFF_DELAY sha_inst_e1;
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e1_q==1'b0));
    else begin
      crypt2_d_e2_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      aes_shf_e2_q[15:0]   <= `MAIA_DFF_DELAY {16{1'bx}};
      aesd_e2_q            <= `MAIA_DFF_DELAY {1{1'bx}};
      aese_e2_q            <= `MAIA_DFF_DELAY {1{1'bx}};
      aesmc_e2_q           <= `MAIA_DFF_DELAY {1{1'bx}};
      aesimc_e2_q          <= `MAIA_DFF_DELAY {1{1'bx}};
      aesemc_e2_q          <= `MAIA_DFF_DELAY {1{1'bx}};
      aesdimc_e2_q         <= `MAIA_DFF_DELAY {1{1'bx}};
      pmull_e2_q           <= `MAIA_DFF_DELAY {1{1'bx}};
      sha_inst_e2_q        <= `MAIA_DFF_DELAY {1{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// Enable data inputs for selected operation (glitch suppression in unused datapaths) 
  assign aesmc_in[127:0]  = {128{aesmc_e2_q }} & crypt2_d_e2_q[127:0];
  assign aesimc_in[127:0] = {128{aesimc_e2_q}} & crypt2_d_e2_q[127:0];

  maia_cx_aesed2 uaesed2(
        .aes_din       (crypt2_d_e2_q[127:0]),
        .aes_shf       (aes_shf_e2_q[15:0]),
        .aesd_out      (aesd_out[127:0]),
        .aese_out      (aese_out[127:0]),
        .aesemc_out    (aesemc_out[127:0]),
        .aesdimc_out   (aesdimc_out[127:0])
);

  maia_cx_aesmc uaesmc(
        .d_in          (aesmc_in[127:0]),
        .mc            (aesmc_out[127:0])
);

  maia_cx_aesimc uaesimc(
        .d_in          (aesimc_in[127:0]),
        .imc           (aesimc_out[127:0])
);

  maia_cx_pmull upmull(
        .a_in          (crypt2_d_e2_q[63:0]),
        .b_in          (crypt2_d_e2_q[127:64]),
        .p_out         (pmull_out[127:0])
);

assign crypt2_out_e2[127:0] = ({128{aesd_e2_q & ~aesdimc_e2_q}} & aesd_out[127:0])
                            | ({128{aese_e2_q & ~aesemc_e2_q}}  & aese_out[127:0])
                            | ({128{aesmc_e2_q}}                & aesmc_out[127:0])
                            | ({128{aesemc_e2_q}}               & aesemc_out[127:0])
                            | ({128{aesimc_e2_q}}               & aesimc_out[127:0])
                            | ({128{aesdimc_e2_q}}              & aesdimc_out[127:0])
                            | ({128{sha_inst_e2_q}}             & crypt2_d_e2_q[127:0])
                            | ({128{pmull_e2_q}}                & pmull_out[127:0]);

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: ucrypt2_e3
    if (ival_e2_q==1'b1) begin 
      crypt2_out_e3_q[127:0] <= `MAIA_DFF_DELAY crypt2_out_e2[127:0];
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e2_q==1'b0));
    else begin
      crypt2_out_e3_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

//-----------------------------------------------------------------------------
// regional clock gating (RCG) terms
//-----------------------------------------------------------------------------

assign crypt2_active = (ival_e1_q | ival_e2_q);


endmodule

//ARMAUTO UNDEF START
`define MAIA_UNDEFINE
`include "maia_header.v"
`undef MAIA_UNDEFINE
//ARMAUTO UNDEF END
