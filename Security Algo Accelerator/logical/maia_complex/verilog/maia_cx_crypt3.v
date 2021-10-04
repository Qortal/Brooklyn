
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
//     Filename            : $RCSfile: maia_cx_crypt3.v $
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
//   - SHA 3-input operations: sha1cpm, sha1su0, sha256h, sha256h2, sha256su1 

//#
//# Module Declaration
//# ==================
//#

`include "maia_header.v"

module maia_cx_crypt3 (

//#
//# Interface Signals
//# =================
//#

// Global inputs
  ck_gclkcx_crypt,
  cx_reset3,
  
// Control inputs
//
// This block has 3x128-bit inputs for each instruction, so it requires two cycles just to
// get its operands. In E1, we receive two of the operands (qn and qm) and ival_e1_q,
// which allows the operands to be stored in flops. We also get inputs indicating which
// instruction is to be computed.
//
// At some later cycle, we receive the 3rd operand, qd, and ival_e2_q, indicating that
// we should begin the computation.
//
// There are 4 execution stages, E2-E5.
  ival_e1_q,
  sha1c_e1_q,
  sha1p_e1_q,
  sha1m_e1_q,
  sha256h_e1_q,
  sha256h2_e1_q,
  sha256su1_e1_q,
  ival_e2_q,
// Data inputs
  qn_e1_q,
  qm_e1_q,
  qd_e2_q,
// Outputs
  crypt3_out_e6_q,
  crypt3_active
);


//#
//# Interface Signals
//# =================
//#

// Global inputs
  input            ck_gclkcx_crypt;
  input            cx_reset3;
  
// Control inputs
//
// This block has 3x128-bit inputs for each instruction, so it requires two cycles just to
// get its operands. In E1, we receive two of the operands (qn and qm) and ival_e1_q,
// which allows the operands to be stored in flops. We also get inputs indicating which
// instruction is to be computed.
//
// At some later cycle, we receive the 3rd operand, qd, and ival_e2_q, indicating that
// we should begin the computation.
//
// There are 4 execution stages, E2-E5.
  input            ival_e1_q;
  input            sha1c_e1_q;      // sha hash update (choose)
  input            sha1p_e1_q;      // sha hash update (parity)
  input            sha1m_e1_q;      // sha hash update (majority)
  input            sha256h_e1_q;    // sha256 hash update
  input            sha256h2_e1_q;   // sha256 hash update 2
  input            sha256su1_e1_q;  // sha256 schedule update 1
  input            ival_e2_q;
// Data inputs
  input [127:0]    qn_e1_q;         // qn arrives with first uop on {srcb,srca}
  input [127:0]    qm_e1_q;         // qm arrives with first uop on {srcd,srcc}
  input [127:0]    qd_e2_q;         // qd arrives with second uop on {srcb,srca}
// Outputs
  output [127:0]   crypt3_out_e6_q;
  output           crypt3_active;

//#
//# Internal Signals - Automatic Declarations
//# =========================================
//#
  wire [127:  0] crypt3_out_e5;
  reg  [127:  0] crypt3_out_e6_q;
  wire           firstop_recvd_e1;
  reg            firstop_recvd_e2_q;
  reg            ival_e3_q;
  reg            ival_e4_q;
  reg            ival_e5_q;
  wire [127:  0] newx_e2;
  wire [127:  0] newx_e3;
  wire [127:  0] newx_e4;
  wire [127:  0] newy_e2;
  wire [127:  0] newy_e3;
  wire [127:  0] newy_e4;
  reg  [127:  0] qm_e2_q;
  reg  [127:  0] qn_e2_q;
  wire [127:  0] sha1_xin_e2;
  wire [ 31:  0] sha1_yin_e2;
  wire [ 31:  0] sha1_zin_e2;
  wire           sha1c_e2;
  reg            sha1c_e2_q;
  reg            sha1c_e3_q;
  reg            sha1c_e4_q;
  reg            sha1c_e5_q;
  wire           sha1cpm_e2;
  wire           sha1cpm_e3;
  wire           sha1cpm_e4;
  wire           sha1cpm_e5;
  wire [127:  0] sha1cpm_x_e2;
  wire [127:  0] sha1cpm_x_e3;
  wire [127:  0] sha1cpm_x_e4;
  wire [127:  0] sha1cpm_x_e5;
  wire [127:  0] sha1cpm_y_e2;
  wire [127:  0] sha1cpm_y_e3;
  wire [127:  0] sha1cpm_y_e4;
// verilint unused_sigs off
  wire [ 31:  0] sha1cpm_y_e5;
// verilint unused_sigs on
  wire           sha1m_e2;
  reg            sha1m_e2_q;
  reg            sha1m_e3_q;
  reg            sha1m_e4_q;
  reg            sha1m_e5_q;
  wire           sha1p_e2;
  reg            sha1p_e2_q;
  reg            sha1p_e3_q;
  reg            sha1p_e4_q;
  reg            sha1p_e5_q;
  wire [127:  0] sha256_xin_e2;
  wire [127:  0] sha256_yin_e2;
  wire [ 31:  0] sha256_zin_e2;
  wire           sha256h2_e2;
  reg            sha256h2_e2_q;
  reg            sha256h2_e3_q;
  reg            sha256h2_e4_q;
  reg            sha256h2_e5_q;
  wire           sha256h_e2;
  reg            sha256h_e2_q;
  reg            sha256h_e3_q;
  reg            sha256h_e4_q;
  reg            sha256h_e5_q;
  wire [127:  0] sha256h_x_e2;
  wire [127:  0] sha256h_x_e3;
  wire [127:  0] sha256h_x_e4;
  wire [127:  0] sha256h_x_e5;
  wire [127:  0] sha256h_y_e2;
  wire [127:  0] sha256h_y_e3;
  wire [127:  0] sha256h_y_e4;
  wire [127:  0] sha256h_y_e5;
  wire           sha256hh2_e2;
  wire           sha256hh2_e3;
  wire           sha256hh2_e4;
  wire           sha256su1_e2;
  reg            sha256su1_e2_q;
  reg            sha256su1_e3_q;
  reg            sha256su1_e4_q;
  reg            sha256su1_e5_q;
  wire [ 63:  0] sha256su1_x_e3;
  wire [ 63:  0] sha256su1_x_e4;
  wire [127:  0] x_e2;
  wire [127:  0] x_e3;
  reg  [127:  0] x_e3_q;
  wire [127:  0] x_e4;
  reg  [127:  0] x_e4_q;
  wire [127:  0] x_e5;
  reg  [127:  0] x_e5_q;
  wire [127:  0] y_e2;
  wire [127:  0] y_e3;
  reg  [127:  0] y_e3_q;
  wire [127:  0] y_e4;
  reg  [127:  0] y_e4_q;
  wire [127:  0] y_e5;
  reg  [127:  0] y_e5_q;
  wire [127:  0] z_e2;
  wire [ 95:  0] z_e3;
  reg  [ 95:  0] z_e3_q;
  wire [ 63:  0] z_e4;
  reg  [ 63:  0] z_e4_q;
  wire [ 31:  0] z_e5;
  reg  [ 31:  0] z_e5_q;

//#
//# Main Code
//# =========
//#     
//

// set when ival_e1_q first received, and held until the 2nd uop (ival_e2_q) is received
assign firstop_recvd_e1 = (ival_e1_q | (firstop_recvd_e2_q & ~ival_e2_q));
  
// ival and instruction flops
  
// reset flop since 1st uop of crypto pair can be flushed due to SWDW nuke, thus might
// have received ival_e2_q without ever receiving ival_e1_q (since it was flushed). thus
// want firstop_recvd_e2_q to be 0 (not X) to stop X-prop
  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt or posedge cx_reset3)
  begin: ufirstop_recvd_e2_q
    if (cx_reset3 == 1'b1)
      firstop_recvd_e2_q <= `MAIA_DFF_DELAY {1{1'b0}};
`ifdef MAIA_XPROP_FLOP
    else if (cx_reset3==1'b0)
      firstop_recvd_e2_q <= `MAIA_DFF_DELAY firstop_recvd_e1;
    else
      firstop_recvd_e2_q <= `MAIA_DFF_DELAY {1{1'bx}};
`else
    else
      firstop_recvd_e2_q <= `MAIA_DFF_DELAY firstop_recvd_e1;
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// reset flop(s) since feeds into active signal used for RCG
  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt or posedge cx_reset3)
  begin: uival_e3_q
    if (cx_reset3 == 1'b1)
      ival_e3_q <= `MAIA_DFF_DELAY {1{1'b0}};
`ifdef MAIA_XPROP_FLOP
    else if (cx_reset3==1'b0)
      ival_e3_q <= `MAIA_DFF_DELAY ival_e2_q;
    else
      ival_e3_q <= `MAIA_DFF_DELAY {1{1'bx}};
`else
    else
      ival_e3_q <= `MAIA_DFF_DELAY ival_e2_q;
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF
  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt or posedge cx_reset3)
  begin: uival_e4_q
    if (cx_reset3 == 1'b1)
      ival_e4_q <= `MAIA_DFF_DELAY {1{1'b0}};
`ifdef MAIA_XPROP_FLOP
    else if (cx_reset3==1'b0)
      ival_e4_q <= `MAIA_DFF_DELAY ival_e3_q;
    else
      ival_e4_q <= `MAIA_DFF_DELAY {1{1'bx}};
`else
    else
      ival_e4_q <= `MAIA_DFF_DELAY ival_e3_q;
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF
  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt or posedge cx_reset3)
  begin: uival_e5_q
    if (cx_reset3 == 1'b1)
      ival_e5_q <= `MAIA_DFF_DELAY {1{1'b0}};
`ifdef MAIA_XPROP_FLOP
    else if (cx_reset3==1'b0)
      ival_e5_q <= `MAIA_DFF_DELAY ival_e4_q;
    else
      ival_e5_q <= `MAIA_DFF_DELAY {1{1'bx}};
`else
    else
      ival_e5_q <= `MAIA_DFF_DELAY ival_e4_q;
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uinst_e2
    if (ival_e1_q==1'b1) begin 
      sha1c_e2_q     <= `MAIA_DFF_DELAY sha1c_e1_q;
      sha1p_e2_q     <= `MAIA_DFF_DELAY sha1p_e1_q;
      sha1m_e2_q     <= `MAIA_DFF_DELAY sha1m_e1_q;
      sha256h_e2_q   <= `MAIA_DFF_DELAY sha256h_e1_q;
      sha256h2_e2_q  <= `MAIA_DFF_DELAY sha256h2_e1_q;
      sha256su1_e2_q <= `MAIA_DFF_DELAY sha256su1_e1_q;
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e1_q==1'b0));
    else begin
      sha1c_e2_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1p_e2_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1m_e2_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h_e2_q   <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h2_e2_q  <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256su1_e2_q <= `MAIA_DFF_DELAY {1{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// stop X-prop if 1st uop was nuked due to swdw_nuke and 2nd was issued

assign sha1c_e2     = firstop_recvd_e2_q & sha1c_e2_q;
assign sha1p_e2     = firstop_recvd_e2_q & sha1p_e2_q;
assign sha1m_e2     = firstop_recvd_e2_q & sha1m_e2_q;
assign sha256h_e2   = firstop_recvd_e2_q & sha256h_e2_q;
assign sha256h2_e2  = firstop_recvd_e2_q & sha256h2_e2_q;
assign sha256su1_e2 = firstop_recvd_e2_q & sha256su1_e2_q;

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uinst_e3
    if (ival_e2_q==1'b1) begin 
      sha1c_e3_q     <= `MAIA_DFF_DELAY sha1c_e2;
      sha1p_e3_q     <= `MAIA_DFF_DELAY sha1p_e2;
      sha1m_e3_q     <= `MAIA_DFF_DELAY sha1m_e2;
      sha256h_e3_q   <= `MAIA_DFF_DELAY sha256h_e2;
      sha256h2_e3_q  <= `MAIA_DFF_DELAY sha256h2_e2;
      sha256su1_e3_q <= `MAIA_DFF_DELAY sha256su1_e2;
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e2_q==1'b0));
    else begin
      sha1c_e3_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1p_e3_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1m_e3_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h_e3_q   <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h2_e3_q  <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256su1_e3_q <= `MAIA_DFF_DELAY {1{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uinst_e4
    if (ival_e3_q==1'b1) begin 
      sha1c_e4_q     <= `MAIA_DFF_DELAY sha1c_e3_q;
      sha1p_e4_q     <= `MAIA_DFF_DELAY sha1p_e3_q;
      sha1m_e4_q     <= `MAIA_DFF_DELAY sha1m_e3_q;
      sha256h_e4_q   <= `MAIA_DFF_DELAY sha256h_e3_q;
      sha256h2_e4_q  <= `MAIA_DFF_DELAY sha256h2_e3_q;
      sha256su1_e4_q <= `MAIA_DFF_DELAY sha256su1_e3_q;
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e3_q==1'b0));
    else begin
      sha1c_e4_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1p_e4_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1m_e4_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h_e4_q   <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h2_e4_q  <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256su1_e4_q <= `MAIA_DFF_DELAY {1{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uinst_e5
    if (ival_e4_q==1'b1) begin 
      sha1c_e5_q     <= `MAIA_DFF_DELAY sha1c_e4_q;
      sha1p_e5_q     <= `MAIA_DFF_DELAY sha1p_e4_q;
      sha1m_e5_q     <= `MAIA_DFF_DELAY sha1m_e4_q;
      sha256h_e5_q   <= `MAIA_DFF_DELAY sha256h_e4_q;
      sha256h2_e5_q  <= `MAIA_DFF_DELAY sha256h2_e4_q;
      sha256su1_e5_q <= `MAIA_DFF_DELAY sha256su1_e4_q;
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e4_q==1'b0));
    else begin
      sha1c_e5_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1p_e5_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha1m_e5_q     <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h_e5_q   <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256h2_e5_q  <= `MAIA_DFF_DELAY {1{1'bx}};
      sha256su1_e5_q <= `MAIA_DFF_DELAY {1{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// E1

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uops_e2
    if (ival_e1_q==1'b1) begin 
      qm_e2_q[127:0] <= `MAIA_DFF_DELAY qm_e1_q[127:0];
      qn_e2_q[127:0] <= `MAIA_DFF_DELAY qn_e1_q[127:0];
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e1_q==1'b0));
    else begin
      qm_e2_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      qn_e2_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// E2
assign x_e2[127:0] = qd_e2_q[127:0];
assign y_e2[127:0] = qn_e2_q[127:0];
assign z_e2[127:0] = qm_e2_q[127:0];

assign sha1_xin_e2[127:0] = {128{sha1cpm_e2}} & x_e2[127:0];
assign sha1_yin_e2[ 31:0] = { 32{sha1cpm_e2}} & y_e2[ 31:0];
assign sha1_zin_e2[ 31:0] = { 32{sha1cpm_e2}} & z_e2[ 31:0];

// sha1 hash update
  maia_cx_sha1cpm usha1cpm_e2(
  .choose           (sha1c_e2_q),
  .parity           (sha1p_e2_q),
  .majority         (sha1m_e2_q),
  .x                (sha1_xin_e2[127:0]),
  .y                (sha1_yin_e2[31:0]),
  .z                (sha1_zin_e2[31:0]),
  .newx             (sha1cpm_x_e2[127:0]),
  .newy             (sha1cpm_y_e2[31:0])
);
assign sha1cpm_y_e2[127:32] = {96{sha1cpm_e2}} & y_e2[127:32]; 

assign sha256_xin_e2[127:0] = {128{sha256hh2_e2}} & x_e2[127:0];
assign sha256_yin_e2[127:0] = {128{sha256hh2_e2}} & y_e2[127:0];
assign sha256_zin_e2[ 31:0] = { 32{sha256hh2_e2}} & z_e2[ 31:0];

// sha256 hash update (1 and 2)
  maia_cx_sha256h32 usha256h32_e2(
  .x                (sha256_xin_e2[127:0]),
  .y                (sha256_yin_e2[127:0]),
  .z                (sha256_zin_e2[31:0]),
  .newx             (sha256h_x_e2[127:0]),
  .newy             (sha256h_y_e2[127:0])
);

// mux results
assign sha1cpm_e2 = sha1c_e2 | sha1p_e2 | sha1m_e2;
assign sha256hh2_e2 = sha256h_e2 | sha256h2_e2;
assign newx_e2[127:0] = ({128{sha1cpm_e2  }} & sha1cpm_x_e2[127:0])
                      | ({128{sha256hh2_e2}} & sha256h_x_e2[127:0])
                      | ({128{sha256su1_e2}} & x_e2[127:0]);
assign newy_e2[127:0] = ({128{sha1cpm_e2  }} & sha1cpm_y_e2[127:0])
                      | ({128{sha256hh2_e2}} & sha256h_y_e2[127:0])
                      | ({128{sha256su1_e2}} & {z_e2[31:0], y_e2[127:32]});
  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uops_e3
    if (ival_e2_q==1'b1) begin 
      x_e3_q[127:0] <= `MAIA_DFF_DELAY newx_e2[127:0];
      y_e3_q[127:0] <= `MAIA_DFF_DELAY newy_e2[127:0];
      z_e3_q[95:0]  <= `MAIA_DFF_DELAY z_e2[127:32];
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e2_q==1'b0));
    else begin
      x_e3_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      y_e3_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      z_e3_q[95:0]  <= `MAIA_DFF_DELAY {96{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// E3
assign x_e3[127:0] = x_e3_q[127:0];
assign y_e3[127:0] = y_e3_q[127:0];
assign z_e3[95:0] =  z_e3_q[95:0];

// sha1 hash update
  maia_cx_sha1cpm usha1cpm_e3(
  .choose           (sha1c_e3_q),
  .parity           (sha1p_e3_q),
  .majority         (sha1m_e3_q),
  .x                (x_e3[127:0]),
  .y                (y_e3[31:0]),
  .z                (z_e3[31:0]),
  .newx             (sha1cpm_x_e3[127:0]),
  .newy             (sha1cpm_y_e3[31:0])
);
assign sha1cpm_y_e3[127:32] = y_e3[127:32]; 

// sha256 hash update (1 and 2)
  maia_cx_sha256h32 usha256h32_e3(
  .x                (x_e3[127:0]),
  .y                (y_e3[127:0]),
  .z                (z_e3[31:0]),
  .newx             (sha256h_x_e3[127:0]),
  .newy             (sha256h_y_e3[127:0])
);

// sha256 schedule update 1, cycle 1
  maia_cx_sha256su1 usha256su1_e3(
  .sha256su1_op     (sha256su1_e3_q),
  .x                (x_e3[63:0]),              // qd[63:0]
  .y                (y_e3[63:0]),              // qn[95:32]
  .z                (z_e3[95:32]),             // qm[127:64]
  .newx             (sha256su1_x_e3[63:0])
);

// mux results
assign sha1cpm_e3 = sha1c_e3_q | sha1p_e3_q | sha1m_e3_q;
assign sha256hh2_e3 = sha256h_e3_q | sha256h2_e3_q;
assign newx_e3[127:0] = ({128{sha1cpm_e3    }} & sha1cpm_x_e3[127:0])
                      | ({128{sha256hh2_e3  }} & sha256h_x_e3[127:0])
                      | ({128{sha256su1_e3_q}} & {x_e3[127:64], sha256su1_x_e3[63:0]});
assign newy_e3[127:0] = ({128{sha1cpm_e3    }} & sha1cpm_y_e3[127:0])
                      | ({128{sha256hh2_e3  }} & sha256h_y_e3[127:0])
                      | ({128{sha256su1_e3_q}} & {y_e3[127:0]});

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uops_e4
    if (ival_e3_q==1'b1) begin 
      x_e4_q[127:0] <= `MAIA_DFF_DELAY newx_e3[127:0];
      y_e4_q[127:0] <= `MAIA_DFF_DELAY newy_e3[127:0];
      z_e4_q[63:0]  <= `MAIA_DFF_DELAY z_e3[95:32];
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e3_q==1'b0));
    else begin
      x_e4_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      y_e4_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      z_e4_q[63:0]  <= `MAIA_DFF_DELAY {64{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// E4
assign x_e4[127:0] = x_e4_q[127:0];
assign y_e4[127:0] = y_e4_q[127:0];
assign z_e4[63:0] =  z_e4_q[63:0];

// sha1 hash update
  maia_cx_sha1cpm usha1cpm_e4(
  .choose           (sha1c_e4_q),
  .parity           (sha1p_e4_q),
  .majority         (sha1m_e4_q),
  .x                (x_e4[127:0]),
  .y                (y_e4[31:0]),
  .z                (z_e4[31:0]),
  .newx             (sha1cpm_x_e4[127:0]),
  .newy             (sha1cpm_y_e4[31:0])
);
assign sha1cpm_y_e4[127:32] = y_e4[127:32]; 

// sha256 hash update (1 and 2)
  maia_cx_sha256h32 usha256h32_e4(
  .x                (x_e4[127:0]),
  .y                (y_e4[127:0]),
  .z                (z_e4[31:0]),
  .newx             (sha256h_x_e4[127:0]),
  .newy             (sha256h_y_e4[127:0])
);

// sha256 schedule update 1, cycle 2
  maia_cx_sha256su1 usha256su1_e4(
  .sha256su1_op     (sha256su1_e4_q),
  .x                (x_e4[127:64]),            // qd[127:64]
  .y                (y_e4[127:64]),            // {qm[31:0], qn[127:96]}
  .z                (x_e4[63:0]),              // sha256su1_x_e3[63:0]
  .newx             (sha256su1_x_e4[63:0])
);

// mux results
assign sha1cpm_e4 = sha1c_e4_q | sha1p_e4_q | sha1m_e4_q;
assign sha256hh2_e4 = sha256h_e4_q | sha256h2_e4_q;
assign newx_e4[127:0] = ({128{sha1cpm_e4    }} & sha1cpm_x_e4[127:0])
                      | ({128{sha256hh2_e4  }} & sha256h_x_e4[127:0])
                      | ({128{sha256su1_e4_q}} & {sha256su1_x_e4[63:0], x_e4[63:0]});
assign newy_e4[127:0] = ({128{sha1cpm_e4    }} & sha1cpm_y_e4[127:0])
                      | ({128{sha256hh2_e4  }} & sha256h_y_e4[127:0]);

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: uops_e5
    if (ival_e4_q==1'b1) begin 
      x_e5_q[127:0] <= `MAIA_DFF_DELAY newx_e4[127:0];
      y_e5_q[127:0] <= `MAIA_DFF_DELAY newy_e4[127:0];
      z_e5_q[31:0]  <= `MAIA_DFF_DELAY z_e4[63:32];
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e4_q==1'b0));
    else begin
      x_e5_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      y_e5_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
      z_e5_q[31:0]  <= `MAIA_DFF_DELAY {32{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF

// E5
assign x_e5[127:0] = x_e5_q[127:0];
assign y_e5[127:0] = y_e5_q[127:0];
assign z_e5[31:0] =  z_e5_q[31:0];

// sha1 hash update
  maia_cx_sha1cpm usha1cpm_e5(
  .choose           (sha1c_e5_q),
  .parity           (sha1p_e5_q),
  .majority         (sha1m_e5_q),
  .x                (x_e5[127:0]),
  .y                (y_e5[31:0]),
  .z                (z_e5[31:0]),
  .newx             (sha1cpm_x_e5[127:0]),
  .newy             (sha1cpm_y_e5[31:0])
);

// sha256 hash update (1 and 2)
  maia_cx_sha256h32 usha256h32_e5(
  .x                (x_e5[127:0]),
  .y                (y_e5[127:0]),
  .z                (z_e5[31:0]),
  .newx             (sha256h_x_e5[127:0]),
  .newy             (sha256h_y_e5[127:0])
);

// mux results
assign sha1cpm_e5 = sha1c_e5_q | sha1p_e5_q | sha1m_e5_q;
assign crypt3_out_e5[127:0] = ({128{sha1cpm_e5}}     & sha1cpm_x_e5[127:0])
                            | ({128{sha256h_e5_q}}   & sha256h_x_e5[127:0])
                            | ({128{sha256h2_e5_q}}  & sha256h_y_e5[127:0])
                            | ({128{sha256su1_e5_q}} & x_e5[127:0]);

  // Macro DFF called
  // verilint flop_checks off
  always @(posedge ck_gclkcx_crypt)
  begin: ures_e6
    if (ival_e5_q==1'b1) begin 
      crypt3_out_e6_q[127:0] <= `MAIA_DFF_DELAY crypt3_out_e5[127:0];
    end
`ifdef MAIA_XPROP_FLOP
    else if ((ival_e5_q==1'b0));
    else begin
      crypt3_out_e6_q[127:0] <= `MAIA_DFF_DELAY {128{1'bx}};
    end
`endif
  end
  // verilint flop_checks on
  // end of Macro DFF


//-----------------------------------------------------------------------------
// regional clock gating (RCG) terms
//-----------------------------------------------------------------------------

assign crypt3_active = (ival_e1_q |
                        ival_e2_q |
                        ival_e3_q |
                        ival_e4_q |
                        ival_e5_q
                        );


endmodule

//ARMAUTO UNDEF START
`define MAIA_UNDEFINE
`include "maia_header.v"
`undef MAIA_UNDEFINE
//ARMAUTO UNDEF END
