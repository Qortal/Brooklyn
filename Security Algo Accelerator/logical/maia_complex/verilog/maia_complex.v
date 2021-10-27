
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
//     Filename            : $RCSfile: maia_complex.v $
//     Checked In          : $Date: 2014-08-29 00:16:46 -0500 (Fri, 29 Aug 2014) $
//     Revision            : $Revision: 70482 $
//     Release Information : Cortex-A72-r1p0-00rel0
//
//-----------------------------------------------------------------------------
//     Verilog-2001 (IEEE Std 1364-2001)
//-----------------------------------------------------------------------------
//#
//# Overview
//# ========
//#
//# This is top-level interconnect layer for the Complex Execute unit.
//#

`include "maia_header.v"

module maia_complex( // outputs
                     cx_active,
                     cx_credit_j,
                     cx_credit_k,
                     cx_dstp_tag_ke1,
                     cx_dstp_tag_vld_ke1,
                     cx_resp_data_w1,
                     cx_resp_qfbit_gid_w1,
                     cx_resp_qfbit_vld_w1,
                     cx_resp_qfbit_w2,
                     cx_resp_tag_vld_w0,
                     cx_resp_tag_w0,
                     cx_resx_data_w2,
                     cx_resx_dw_w1,
                     cx_resx_qfbit_gid_w1,
                     cx_resx_qfbit_vld_w1,
                     cx_resx_qfbit_w2,
                     cx_resx_tag_vld_w1,
                     cx_resx_tag_w1,
                     cx_resy_data_w2,
                     cx_resy_dw_w1,
                     cx_resy_qfbit_gid_w1,
                     cx_resy_qfbit_vld_w1,
                     cx_resy_qfbit_w2,
                     cx_resy_tag_vld_w1,
                     cx_resy_tag_w1,
                     cx_resz_data_w2,
                     cx_resz_dw_w1,
                     cx_resz_qfbit_gid_w1,
                     cx_resz_qfbit_vld_w1,
                     cx_resz_qfbit_w2,
                     cx_resz_tag_vld_w1,
                     cx_resz_tag_w1,

                     // inputs
                     ck_areset,
                     ck_gclkcx,
                     ck_reset1_n_cx,
                     dftrstdisable_cpu,
                     dftse_cpu,
                     ds_cx_aarch32_state,
                     ds_cx_aarch64_state,
                     ds_cx_cpuactlr_frc_cpu_rcg_active,
                     ds_cx_dstp_tag_jp2,
                     ds_cx_dstp_tag_kp2,
                     ds_cx_dstp_tag_vld_jp2,
                     ds_cx_dstp_tag_vld_kp2,
                     ds_cx_dstx_dw_jp2,
                     ds_cx_dstx_dw_kp2,
                     ds_cx_dstx_tag_jp2,
                     ds_cx_dstx_tag_kp2,
                     ds_cx_dstx_tag_vld_jp2,
                     ds_cx_dstx_tag_vld_kp2,
                     ds_cx_dsty_dw_jp2,
                     ds_cx_dsty_dw_kp2,
                     ds_cx_dsty_tag_jp2,
                     ds_cx_dsty_tag_kp2,
                     ds_cx_dsty_tag_vld_jp2,
                     ds_cx_dsty_tag_vld_kp2,
                     ds_cx_flush_gid,
                     ds_cx_flush_seq,
                     ds_cx_flush_u1,
                     ds_cx_fpscr_ahp,
                     ds_cx_fpscr_dn,
                     ds_cx_fpscr_fz,
                     ds_cx_fpscr_rmode,
                     ds_cx_gid_jp2,
                     ds_cx_gid_kp2,
                     ds_cx_prt_sel_jp1,
                     ds_cx_prt_sel_kp1,
                     ds_cx_srca_data_jp2,
                     ds_cx_srca_data_kp2,
                     ds_cx_srca_data_vld_jp2,
                     ds_cx_srca_data_vld_kp2,
                     ds_cx_srcb_data_jp2,
                     ds_cx_srcb_data_kp2,
                     ds_cx_srcb_data_vld_jp2,
                     ds_cx_srcb_data_vld_kp2,
                     ds_cx_srcc_data_jp2,
                     ds_cx_srcc_data_kp2,
                     ds_cx_srcc_data_vld_jp2,
                     ds_cx_srcc_data_vld_kp2,
                     ds_cx_srcd_data_jp2,
                     ds_cx_srcd_data_kp2,
                     ds_cx_srcd_data_vld_jp2,
                     ds_cx_srcd_data_vld_kp2,
                     ds_cx_srcp_data_jp2,
                     ds_cx_srcp_data_kp2,
                     ds_cx_srcp_data_vld_jp2,
                     ds_cx_srcp_data_vld_kp2,
                     ds_cx_swdw_nuke_jp2,
                     ds_cx_swdw_nuke_kp2,
                     ds_cx_uop_ctl_jp2,
                     ds_cx_uop_ctl_kp2,
                     ds_cx_uop_vld_jp2,
                     ds_cx_uop_vld_kp2,
                     ds_srca_dw_0p1,
                     ds_srca_dw_1p1,
                     ds_srca_dw_2p1,
                     ds_srca_prdcr_dw_0p1,
                     ds_srca_prdcr_dw_1p1,
                     ds_srca_prdcr_dw_2p1,
                     ds_srca_tag_0p1,
                     ds_srca_tag_1p1,
                     ds_srca_tag_2p1,
                     ds_srca_tag_vld_0p1,
                     ds_srca_tag_vld_1p1,
                     ds_srca_tag_vld_2p1,
                     ds_srcb_dw_0p1,
                     ds_srcb_dw_1p1,
                     ds_srcb_dw_2p1,
                     ds_srcb_prdcr_dw_0p1,
                     ds_srcb_prdcr_dw_1p1,
                     ds_srcb_prdcr_dw_2p1,
                     ds_srcb_tag_0p1,
                     ds_srcb_tag_1p1,
                     ds_srcb_tag_2p1,
                     ds_srcb_tag_vld_0p1,
                     ds_srcb_tag_vld_1p1,
                     ds_srcb_tag_vld_2p1,
                     ds_srcc_dw_0p1,
                     ds_srcc_dw_1p1,
                     ds_srcc_dw_2p1,
                     ds_srcc_prdcr_dw_0p1,
                     ds_srcc_prdcr_dw_1p1,
                     ds_srcc_prdcr_dw_2p1,
                     ds_srcc_tag_0p1,
                     ds_srcc_tag_1p1,
                     ds_srcc_tag_2p1,
                     ds_srcc_tag_vld_0p1,
                     ds_srcc_tag_vld_1p1,
                     ds_srcc_tag_vld_2p1,
                     ds_srcd_dw_0p1,
                     ds_srcd_dw_1p1,
                     ds_srcd_dw_2p1,
                     ds_srcd_prdcr_dw_0p1,
                     ds_srcd_prdcr_dw_1p1,
                     ds_srcd_prdcr_dw_2p1,
                     ds_srcd_tag_0p1,
                     ds_srcd_tag_1p1,
                     ds_srcd_tag_2p1,
                     ds_srcd_tag_vld_0p1,
                     ds_srcd_tag_vld_1p1,
                     ds_srcd_tag_vld_2p1,
                     ds_srcp_tag_0p1,
                     ds_srcp_tag_1p1,
                     ds_srcp_tag_2p1,
                     ds_srcp_tag_vld_0p1,
                     ds_srcp_tag_vld_1p1,
                     ds_srcp_tag_vld_2p1,
                     ls_resx_data_cancel_w1,
                     ls_resx_data_cancel_w2,
                     ls_resx_data_w2,
                     ls_resx_dw_w0,
                     ls_resx_dw_w1,
                     ls_resx_tag_vld_w0,
                     ls_resx_tag_vld_w1,
                     ls_resx_tag_w0,
                     ls_resx_tag_w1,
                     ls_resy_data_cancel_w1,
                     ls_resy_data_cancel_w2,
                     ls_resy_data_w2,
                     ls_resy_dw_w0,
                     ls_resy_dw_w1,
                     ls_resy_tag_vld_w0,
                     ls_resy_tag_vld_w1,
                     ls_resy_tag_w0,
                     ls_resy_tag_w1,
                     mx_resp_data_w2,
                     mx_resp_tag_vld_w0,
                     mx_resp_tag_vld_w1,
                     mx_resp_tag_w0,
                     mx_resp_tag_w1,
                     sx_ldxcancel_sel_jw0,
                     sx_ldxcancel_sel_jw1,
                     sx_ldxcancel_sel_kw0,
                     sx_ldxcancel_sel_kw1,
                     sx_resp_data_jw2,
                     sx_resp_data_kw2,
                     sx_resp_tag_jw0,
                     sx_resp_tag_jw1,
                     sx_resp_tag_kw0,
                     sx_resp_tag_kw1,
                     sx_resp_tag_vld_jw0,
                     sx_resp_tag_vld_jw1,
                     sx_resp_tag_vld_kw0,
                     sx_resp_tag_vld_kw1,
                     sx_uop_vld_jw0,
                     sx_uop_vld_kw0
                   );

wire [3:0] unused1;

    // outputs
    output        cx_active;
    output [2:0]  cx_credit_j;
    output [2:0]  cx_credit_k;
    output [4:0]  cx_dstp_tag_ke1;
    output        cx_dstp_tag_vld_ke1;
    output [3:0]  cx_resp_data_w1;
    output [5:0]  cx_resp_qfbit_gid_w1;
    output        cx_resp_qfbit_vld_w1;
    output [6:0]  cx_resp_qfbit_w2;
    output        cx_resp_tag_vld_w0;
    output [4:0]  cx_resp_tag_w0;
    output [63:0] cx_resx_data_w2;
    output        cx_resx_dw_w1;
    output [5:0]  cx_resx_qfbit_gid_w1;
    output        cx_resx_qfbit_vld_w1;
    output [6:0]  cx_resx_qfbit_w2;
    output        cx_resx_tag_vld_w1;
    output [6:0]  cx_resx_tag_w1;
    output [63:0] cx_resy_data_w2;
    output        cx_resy_dw_w1;
    output [5:0]  cx_resy_qfbit_gid_w1;
    output        cx_resy_qfbit_vld_w1;
    output [6:0]  cx_resy_qfbit_w2;
    output        cx_resy_tag_vld_w1;
    output [6:0]  cx_resy_tag_w1;
    output [63:0] cx_resz_data_w2;
    output        cx_resz_dw_w1;
    output [5:0]  cx_resz_qfbit_gid_w1;
    output        cx_resz_qfbit_vld_w1;
    output [6:0]  cx_resz_qfbit_w2;
    output        cx_resz_tag_vld_w1;
    output [6:0]  cx_resz_tag_w1;

    // inputs
    input        ck_areset;
    input        ck_gclkcx;
    input        ck_reset1_n_cx;
    input        dftrstdisable_cpu;
    input        dftse_cpu;
    input        ds_cx_aarch32_state;
    input        ds_cx_aarch64_state;
    input        ds_cx_cpuactlr_frc_cpu_rcg_active;
    input [4:0]  ds_cx_dstp_tag_jp2;
    input [4:0]  ds_cx_dstp_tag_kp2;
    input        ds_cx_dstp_tag_vld_jp2;
    input        ds_cx_dstp_tag_vld_kp2;
    input        ds_cx_dstx_dw_jp2;
    input        ds_cx_dstx_dw_kp2;
    input [6:0]  ds_cx_dstx_tag_jp2;
    input [6:0]  ds_cx_dstx_tag_kp2;
    input        ds_cx_dstx_tag_vld_jp2;
    input        ds_cx_dstx_tag_vld_kp2;
    input        ds_cx_dsty_dw_jp2;
    input        ds_cx_dsty_dw_kp2;
    input [6:0]  ds_cx_dsty_tag_jp2;
    input [6:0]  ds_cx_dsty_tag_kp2;
    input        ds_cx_dsty_tag_vld_jp2;
    input        ds_cx_dsty_tag_vld_kp2;
    input [6:0]  ds_cx_flush_gid;
    input        ds_cx_flush_seq;
    input        ds_cx_flush_u1;
    input        ds_cx_fpscr_ahp;
    input        ds_cx_fpscr_dn;
    input        ds_cx_fpscr_fz;
    input [1:0]  ds_cx_fpscr_rmode;
    input [6:0]  ds_cx_gid_jp2;
    input [6:0]  ds_cx_gid_kp2;
    input [2:0]  ds_cx_prt_sel_jp1;
    input [2:0]  ds_cx_prt_sel_kp1;
    input [63:0] ds_cx_srca_data_jp2;
    input [63:0] ds_cx_srca_data_kp2;
    input [1:0]  ds_cx_srca_data_vld_jp2;
    input [1:0]  ds_cx_srca_data_vld_kp2;
    input [63:0] ds_cx_srcb_data_jp2;
    input [63:0] ds_cx_srcb_data_kp2;
    input [1:0]  ds_cx_srcb_data_vld_jp2;
    input [1:0]  ds_cx_srcb_data_vld_kp2;
    input [63:0] ds_cx_srcc_data_jp2;
    input [63:0] ds_cx_srcc_data_kp2;
    input [1:0]  ds_cx_srcc_data_vld_jp2;
    input [1:0]  ds_cx_srcc_data_vld_kp2;
    input [63:0] ds_cx_srcd_data_jp2;
    input [63:0] ds_cx_srcd_data_kp2;
    input [1:0]  ds_cx_srcd_data_vld_jp2;
    input [1:0]  ds_cx_srcd_data_vld_kp2;
    input [3:0]  ds_cx_srcp_data_jp2;
    input [3:0]  ds_cx_srcp_data_kp2;
    input        ds_cx_srcp_data_vld_jp2;
    input        ds_cx_srcp_data_vld_kp2;
    input        ds_cx_swdw_nuke_jp2;
    input        ds_cx_swdw_nuke_kp2;
    input [58:0] ds_cx_uop_ctl_jp2;
    input [58:0] ds_cx_uop_ctl_kp2;
    input        ds_cx_uop_vld_jp2;
    input        ds_cx_uop_vld_kp2;
    input        ds_srca_dw_0p1;
    input        ds_srca_dw_1p1;
    input        ds_srca_dw_2p1;
    input        ds_srca_prdcr_dw_0p1;
    input        ds_srca_prdcr_dw_1p1;
    input        ds_srca_prdcr_dw_2p1;
    input [6:0]  ds_srca_tag_0p1;
    input [6:0]  ds_srca_tag_1p1;
    input [6:0]  ds_srca_tag_2p1;
    input        ds_srca_tag_vld_0p1;
    input        ds_srca_tag_vld_1p1;
    input        ds_srca_tag_vld_2p1;
    input        ds_srcb_dw_0p1;
    input        ds_srcb_dw_1p1;
    input        ds_srcb_dw_2p1;
    input        ds_srcb_prdcr_dw_0p1;
    input        ds_srcb_prdcr_dw_1p1;
    input        ds_srcb_prdcr_dw_2p1;
    input [6:0]  ds_srcb_tag_0p1;
    input [6:0]  ds_srcb_tag_1p1;
    input [6:0]  ds_srcb_tag_2p1;
    input        ds_srcb_tag_vld_0p1;
    input        ds_srcb_tag_vld_1p1;
    input        ds_srcb_tag_vld_2p1;
    input        ds_srcc_dw_0p1;
    input        ds_srcc_dw_1p1;
    input        ds_srcc_dw_2p1;
    input        ds_srcc_prdcr_dw_0p1;
    input        ds_srcc_prdcr_dw_1p1;
    input        ds_srcc_prdcr_dw_2p1;
    input [6:0]  ds_srcc_tag_0p1;
    input [6:0]  ds_srcc_tag_1p1;
    input [6:0]  ds_srcc_tag_2p1;
    input        ds_srcc_tag_vld_0p1;
    input        ds_srcc_tag_vld_1p1;
    input        ds_srcc_tag_vld_2p1;
    input        ds_srcd_dw_0p1;
    input        ds_srcd_dw_1p1;
    input        ds_srcd_dw_2p1;
    input        ds_srcd_prdcr_dw_0p1;
    input        ds_srcd_prdcr_dw_1p1;
    input        ds_srcd_prdcr_dw_2p1;
    input [6:0]  ds_srcd_tag_0p1;
    input [6:0]  ds_srcd_tag_1p1;
    input [6:0]  ds_srcd_tag_2p1;
    input        ds_srcd_tag_vld_0p1;
    input        ds_srcd_tag_vld_1p1;
    input        ds_srcd_tag_vld_2p1;
    input [4:0]  ds_srcp_tag_0p1;
    input [4:0]  ds_srcp_tag_1p1;
    input [4:0]  ds_srcp_tag_2p1;
    input        ds_srcp_tag_vld_0p1;
    input        ds_srcp_tag_vld_1p1;
    input        ds_srcp_tag_vld_2p1;
    input        ls_resx_data_cancel_w1;
    input        ls_resx_data_cancel_w2;
    input [63:0] ls_resx_data_w2;
    input        ls_resx_dw_w0;
    input        ls_resx_dw_w1;
    input        ls_resx_tag_vld_w0;
    input        ls_resx_tag_vld_w1;
    input [6:0]  ls_resx_tag_w0;
    input [6:0]  ls_resx_tag_w1;
    input        ls_resy_data_cancel_w1;
    input        ls_resy_data_cancel_w2;
    input [63:0] ls_resy_data_w2;
    input        ls_resy_dw_w0;
    input        ls_resy_dw_w1;
    input        ls_resy_tag_vld_w0;
    input        ls_resy_tag_vld_w1;
    input [6:0]  ls_resy_tag_w0;
    input [6:0]  ls_resy_tag_w1;
    input [3:0]  mx_resp_data_w2;
    input        mx_resp_tag_vld_w0;
    input        mx_resp_tag_vld_w1;
    input [4:0]  mx_resp_tag_w0;
    input [4:0]  mx_resp_tag_w1;
    input        sx_ldxcancel_sel_jw0;
    input        sx_ldxcancel_sel_jw1;
    input        sx_ldxcancel_sel_kw0;
    input        sx_ldxcancel_sel_kw1;
    input [3:0]  sx_resp_data_jw2;
    input [3:0]  sx_resp_data_kw2;
    input [4:0]  sx_resp_tag_jw0;
    input [4:0]  sx_resp_tag_jw1;
    input [4:0]  sx_resp_tag_kw0;
    input [4:0]  sx_resp_tag_kw1;
    input        sx_resp_tag_vld_jw0;
    input        sx_resp_tag_vld_jw1;
    input        sx_resp_tag_vld_kw0;
    input        sx_resp_tag_vld_kw1;
    input        sx_uop_vld_jw0;
    input        sx_uop_vld_kw0;

    // wires
    wire         acc_size_eq64_e3_q;
    wire         acc_size_ge32_e3_q;
    wire         acc_size_ne08_e3_q;
    wire         addj_active;
    wire         addk_active;
    wire         aesd_e1;
    wire         aesdimc_e1;
    wire         aese_e1;
    wire         aesemc_e1;
    wire         aesimc_e1;
    wire         aesmc_e1;
    wire         ahp_mode_e1_q;
    wire [3:0]   c00_x_sel_e1;
    wire         c00_y_sel_e1;
    wire [5:0]   c01_x_sel_e1;
    wire [2:0]   c01_y_sel_e1;
    wire [5:0]   c02_x_sel_e1;
    wire [2:0]   c02_y_sel_e1;
    wire [6:0]   c03_x_sel_e1;
    wire [4:0]   c03_y_sel_e1;
    wire [5:0]   c04_x_sel_e1;
    wire [2:0]   c04_y_sel_e1;
    wire [7:0]   c05_x_sel_e1;
    wire [4:0]   c05_y_sel_e1;
    wire [5:0]   c06_x_sel_e1;
    wire [4:0]   c06_y_sel_e1;
    wire [6:0]   c07_x_sel_e1;
    wire [4:0]   c07_y_sel_e1;
    wire [6:0]   c08_x_sel_e1;
    wire [2:0]   c08_y_sel_e1;
    wire [8:0]   c09_x_sel_e1;
    wire [5:0]   c09_y_sel_e1;
    wire [8:0]   c10_x_sel_e1;
    wire [5:0]   c10_y_sel_e1;
    wire [9:0]   c11_x_sel_e1;
    wire [7:0]   c11_y_sel_e1;
    wire [7:0]   c12_x_sel_e1;
    wire [5:0]   c12_y_sel_e1;
    wire [9:0]   c13_x_sel_e1;
    wire [6:0]   c13_y_sel_e1;
    wire [7:0]   c14_x_sel_e1;
    wire [6:0]   c14_y_sel_e1;
    wire [7:0]   c15_x_sel_e1;
    wire [6:0]   c15_y_sel_e1;
    wire         ccpass_ke2;
    wire         ck_gclkcx_crypt;
    wire         ck_gclkcx_floatj;
    wire         ck_gclkcx_floatk;
    wire         ck_gclkcx_intj;
    wire         ck_gclkcx_intk;
    wire         crp3_vld_je1;
    wire         crypt2_active;
    wire [127:0] crypt2_out_e3_q;
    wire         crypt2_vld_e1;
    wire         crypt3_active;
    wire [127:0] crypt3_out_e6_q;
    wire         crypt3_vld0_e1;
    wire         crypt3_vld1_e1;
    wire         cvtj_active;
    wire [3:0]   cx_acc_type_je1;
    wire [3:1]   cx_acc_type_je2;
    wire [3:0]   cx_acc_type_ke1;
    wire [3:1]   cx_acc_type_ke2;
    wire         cx_ccpass_je1;
    wire         cx_ccpass_ke1;
    wire         cx_ctl_dp_fp_valid_ji1;
    wire         cx_ctl_dp_fp_valid_ki1;
    wire         cx_ctl_dp_int_valid_ji1;
    wire         cx_ctl_dp_int_valid_ki1;
    wire [4:0]   cx_dst_sel_je1;
    wire [4:0]   cx_dst_sel_ke1;
    wire [4:0]   cx_dstp_tag_je1;
    wire [4:0]   cx_dstp_tag_ke1;
    wire         cx_dstp_tag_vld_je1;
    wire         cx_dstp_tag_vld_ke1;
    wire         cx_dstx_dw_je1;
    wire         cx_dstx_dw_ke1;
    wire [6:0]   cx_dstx_tag_je1;
    wire [6:0]   cx_dstx_tag_je2;
    wire [6:0]   cx_dstx_tag_ke1;
    wire [6:0]   cx_dstx_tag_ke2;
    wire         cx_dstx_tag_vld_je1;
    wire         cx_dstx_tag_vld_ke1;
    wire         cx_dsty_dw_je1;
    wire         cx_dsty_dw_ke1;
    wire [6:0]   cx_dsty_tag_je1;
    wire [6:0]   cx_dsty_tag_ke1;
    wire         cx_dsty_tag_vld_je1;
    wire         cx_dsty_tag_vld_ke1;
    wire [63:0]  cx_fadd_srca_fp_data32_je1;
    wire [63:0]  cx_fadd_srca_fp_data32_ke1;
    wire [63:0]  cx_fadd_srca_fp_data64_je1;
    wire [63:0]  cx_fadd_srca_fp_data64_ke1;
    wire [55:0]  cx_fadd_srcb_fp_data32_h_je1;
    wire [55:0]  cx_fadd_srcb_fp_data32_h_ke1;
    wire [55:0]  cx_fadd_srcb_fp_data32_l_je1;
    wire [55:0]  cx_fadd_srcb_fp_data32_l_ke1;
    wire [116:0] cx_fadd_srcb_fp_data64_je1;
    wire [116:0] cx_fadd_srcb_fp_data64_ke1;
    wire [63:0]  cx_fadd_srcc_fp_data32_je1;
    wire [63:0]  cx_fadd_srcc_fp_data32_ke1;
    wire [31:0]  cx_fadd_srcc_fp_data64_je1;
    wire [31:0]  cx_fadd_srcc_fp_data64_ke1;
    wire [31:0]  cx_fadd_srcd_fp_data64_je1;
    wire [31:0]  cx_fadd_srcd_fp_data64_ke1;
    wire [63:0]  cx_fmul_srca_fp_data32_je1;
    wire [63:0]  cx_fmul_srca_fp_data32_ke1;
    wire [63:0]  cx_fmul_srcb_fp_data32_je1;
    wire [63:0]  cx_fmul_srcb_fp_data32_ke1;
    wire [6:0]   cx_gid_je1;
    wire [6:0]   cx_gid_ke1;
    wire [12:0]  cx_imac_cmd_e1;
    wire         cx_imac_vld_e1;
    wire         cx_ls_resx_data_cancel_w2;
    wire         cx_ls_resx_data_cancel_w3;
    wire         cx_ls_resx_dw_w1;
    wire         cx_ls_resx_dw_w2;
    wire         cx_ls_resx_tag_vld_w1;
    wire         cx_ls_resx_tag_vld_w2;
    wire [6:0]   cx_ls_resx_tag_w1;
    wire [6:0]   cx_ls_resx_tag_w2;
    wire         cx_ls_resy_data_cancel_w2;
    wire         cx_ls_resy_data_cancel_w3;
    wire         cx_ls_resy_dw_w1;
    wire         cx_ls_resy_dw_w2;
    wire         cx_ls_resy_tag_vld_w1;
    wire         cx_ls_resy_tag_vld_w2;
    wire [6:0]   cx_ls_resy_tag_w1;
    wire [6:0]   cx_ls_resy_tag_w2;
    wire [4:0]   cx_mla_fwd_sel_je1;
    wire [4:0]   cx_mla_fwd_sel_je2;
    wire [4:0]   cx_mla_fwd_sel_je3;
    wire [4:0]   cx_mla_fwd_sel_ke1;
    wire [4:0]   cx_mla_fwd_sel_ke2;
    wire [4:0]   cx_mla_fwd_sel_ke3;
    wire         cx_mx_resp_tag_vld_w1;
    wire         cx_mx_resp_tag_vld_w2;
    wire [4:0]   cx_mx_resp_tag_w1;
    wire [4:0]   cx_mx_resp_tag_w2;
    wire [2:0]   cx_region_je1;
    wire [2:0]   cx_region_ke1;
    wire         cx_res128_je1;
    wire         cx_res128_ke1;
    wire         cx_reset3;
    wire [3:0]   cx_resp_data_w1;
    wire [3:0]   cx_resp_data_w2;
    wire         cx_resp_tag_vld_w0;
    wire         cx_resp_tag_vld_w1;
    wire         cx_resp_tag_vld_w2;
    wire [4:0]   cx_resp_tag_w0;
    wire [4:0]   cx_resp_tag_w1;
    wire [4:0]   cx_resp_tag_w2;
    wire [63:0]  cx_resx_data_w2;
    wire         cx_resx_dw_w0;
    wire         cx_resx_dw_w0m1;
    wire         cx_resx_dw_w1;
    wire         cx_resx_dw_w2;
    wire [2:0]   cx_resx_region_w0m1;
    wire [49:32] cx_resx_sel_dec_jw0;
    wire [13:0]  cx_resx_sel_dec_kw0;
    wire         cx_resx_selj_w0;
    wire         cx_resx_tag_vld_w0;
    wire         cx_resx_tag_vld_w0m1;
    wire         cx_resx_tag_vld_w1;
    wire         cx_resx_tag_vld_w2;
    wire [6:0]   cx_resx_tag_w0;
    wire [6:0]   cx_resx_tag_w0m1;
    wire [6:0]   cx_resx_tag_w1;
    wire [6:0]   cx_resx_tag_w2;
    wire [63:0]  cx_resy_data_w2;
    wire         cx_resy_dw_w0;
    wire         cx_resy_dw_w0m1;
    wire         cx_resy_dw_w1;
    wire         cx_resy_dw_w2;
    wire [2:0]   cx_resy_region_w0m1;
    wire [49:32] cx_resy_sel_dec_jw0;
    wire [13:0]  cx_resy_sel_dec_kw0;
    wire         cx_resy_selj_w0;
    wire         cx_resy_tag_vld_w0;
    wire         cx_resy_tag_vld_w0m1;
    wire         cx_resy_tag_vld_w1;
    wire         cx_resy_tag_vld_w2;
    wire [6:0]   cx_resy_tag_w0;
    wire [6:0]   cx_resy_tag_w0m1;
    wire [6:0]   cx_resy_tag_w1;
    wire [6:0]   cx_resy_tag_w2;
    wire [63:0]  cx_resz_data_w2;
    wire         cx_resz_dw_w0;
    wire         cx_resz_dw_w0m1;
    wire         cx_resz_dw_w1;
    wire         cx_resz_dw_w2;
    wire [2:0]   cx_resz_region_w0m1;
    wire [49:32] cx_resz_sel_dec_jw0;
    wire [13:0]  cx_resz_sel_dec_kw0;
    wire         cx_resz_selj_w0;
    wire         cx_resz_tag_vld_w0;
    wire         cx_resz_tag_vld_w0m1;
    wire         cx_resz_tag_vld_w1;
    wire         cx_resz_tag_vld_w2;
    wire [6:0]   cx_resz_tag_w0;
    wire [6:0]   cx_resz_tag_w0m1;
    wire [6:0]   cx_resz_tag_w1;
    wire [6:0]   cx_resz_tag_w2;
    wire [63:0]  cx_srca_crypt_data_je1;
    wire [63:0]  cx_srca_data_ji1;
    wire [63:0]  cx_srca_data_ki1;
    wire         cx_srca_en_je1;
    wire         cx_srca_en_ke1;
    wire [63:0]  cx_srca_fp_data32_je1;
    wire [63:0]  cx_srca_fp_data32_ke1;
    wire [63:0]  cx_srca_fp_data64_je1;
    wire [63:0]  cx_srca_fp_data64_ke1;
    wire         cx_srca_fp_h_en_ji1;
    wire         cx_srca_fp_h_en_ki1;
    wire         cx_srca_fp_l_en_ji1;
    wire         cx_srca_fp_l_en_ki1;
    wire [63:0]  cx_srca_int_data_je1;
    wire [63:0]  cx_srca_int_data_ke1;
    wire         cx_srca_int_h_en_ji1;
    wire         cx_srca_int_h_en_ki1;
    wire         cx_srca_int_l_en_ji1;
    wire         cx_srca_int_l_en_ki1;
    wire [63:0]  cx_srcb_crypt_data_je1;
    wire [63:0]  cx_srcb_data_ji1;
    wire [63:0]  cx_srcb_data_ki1;
    wire         cx_srcb_en_je1;
    wire         cx_srcb_en_ke1;
    wire [63:0]  cx_srcb_fp_data32_je1;
    wire [63:0]  cx_srcb_fp_data64_je1;
    wire [63:0]  cx_srcb_fp_data64_ke1;
    wire         cx_srcb_fp_h_en_ji1;
    wire         cx_srcb_fp_h_en_ki1;
    wire         cx_srcb_fp_l_en_ji1;
    wire         cx_srcb_fp_l_en_ki1;
    wire [63:0]  cx_srcb_int_data_je1;
    wire [63:0]  cx_srcb_int_data_ke1;
    wire         cx_srcb_int_h_en_ji1;
    wire         cx_srcb_int_h_en_ki1;
    wire         cx_srcb_int_l_en_ji1;
    wire         cx_srcb_int_l_en_ki1;
    wire [63:0]  cx_srcc_crypt_data_je1;
    wire [63:0]  cx_srcc_data_ji1;
    wire [63:0]  cx_srcc_data_ke3;
    wire [63:0]  cx_srcc_data_ki1;
    wire         cx_srcc_en_je1;
    wire         cx_srcc_en_ke1;
    wire [63:0]  cx_srcc_fp_data32_je1;
    wire [63:0]  cx_srcc_fp_data32_ke1;
    wire         cx_srcc_fp_h_en_ji1;
    wire         cx_srcc_fp_h_en_ki1;
    wire         cx_srcc_fp_l_en_ji1;
    wire         cx_srcc_fp_l_en_ki1;
    wire [63:0]  cx_srcc_int_data_je1;
    wire [63:0]  cx_srcc_int_data_ke1;
    wire         cx_srcc_int_h_en_ji1;
    wire         cx_srcc_int_h_en_ki1;
    wire         cx_srcc_int_l_en_ji1;
    wire         cx_srcc_int_l_en_ki1;
    wire [63:0]  cx_srcd_crypt_data_je1;
    wire [63:0]  cx_srcd_data_ji1;
    wire [63:0]  cx_srcd_data_ke3;
    wire [63:0]  cx_srcd_data_ki1;
    wire         cx_srcd_en_je1;
    wire         cx_srcd_en_ke1;
    wire [31:0]  cx_srcd_fp_data32_je1;
    wire [31:0]  cx_srcd_fp_data32_ke1;
    wire         cx_srcd_fp_h_en_ji1;
    wire         cx_srcd_fp_h_en_ki1;
    wire         cx_srcd_fp_l_en_ji1;
    wire         cx_srcd_fp_l_en_ki1;
    wire [63:0]  cx_srcd_int_data_je1;
    wire [63:0]  cx_srcd_int_data_ke1;
    wire         cx_srcd_int_h_en_ji1;
    wire         cx_srcd_int_h_en_ki1;
    wire         cx_srcd_int_l_en_ji1;
    wire         cx_srcd_int_l_en_ki1;
    wire [3:0]   cx_srcp_data_je1;
    wire [3:0]   cx_srcp_data_ke1;
    wire         cx_sx_ldxcancel_sel_jw1;
    wire         cx_sx_ldxcancel_sel_kw1;
    wire [4:0]   cx_sx_resp_tag_jw1;
    wire [4:0]   cx_sx_resp_tag_jw2;
    wire [4:0]   cx_sx_resp_tag_kw1;
    wire [4:0]   cx_sx_resp_tag_kw2;
    wire         cx_sx_resp_tag_vld_jw1;
    wire         cx_sx_resp_tag_vld_jw2;
    wire         cx_sx_resp_tag_vld_kw1;
    wire         cx_sx_resp_tag_vld_kw2;
    wire [58:0]  cx_uop_ctl_ji1;
    wire [58:0]  cx_uop_ctl_ki1;
    wire [2:0]   cx_uop_res_latency_je1;
    wire [2:0]   cx_uop_res_latency_ke1;
    wire         cx_uop_vld_je1;
    wire         cx_uop_vld_ji1;
    wire         cx_uop_vld_ke1;
    wire         cx_uop_vld_ki1;
    wire         dn_fadd_je1;
    wire         dn_fadd_ke1;
    wire         dn_je1;
    wire         dn_ke1;
    wire         dn_raw_e1_q;
    wire [39:0]  dstx_bytesel_je;
    wire [39:0]  dstx_bytesel_ke;
    wire [39:0]  dsty_bytesel_je;
    wire [39:0]  dsty_bytesel_ke;
    wire [5:0]   fadd32_ex_h_je4;
    wire [5:0]   fadd32_ex_h_ke4;
    wire [5:0]   fadd32_ex_l_je4;
    wire [5:0]   fadd32_ex_l_ke4;
    wire [5:0]   fadd64_ex_je4;
    wire [5:0]   fadd64_ex_ke4;
    wire         fadd_absin_je1;
    wire         fadd_absin_ke1;
    wire         fadd_absout_je1;
    wire         fadd_absout_ke1;
    wire         fadd_ccpass_je1;
    wire         fadd_ccpass_ke1;
    wire         fadd_hazard1_j;
    wire         fadd_hazard1_k;
    wire [2:0]   fadd_srca_sel_h_je1;
    wire [2:0]   fadd_srca_sel_h_ke1;
    wire         fadd_srca_sel_l_je1;
    wire         fadd_srca_sel_l_ke1;
    wire [2:0]   fadd_srcb_sel_h_je1;
    wire [2:0]   fadd_srcb_sel_h_ke1;
    wire [2:0]   fadd_srcb_sel_l_je1;
    wire [2:0]   fadd_srcb_sel_l_ke1;
    wire         fadd_sub_je1;
    wire         fadd_sub_ke1;
    wire [2:0]   fadd_vld_je1;
    wire [2:0]   fadd_vld_je4;
    wire [2:0]   fadd_vld_ke1;
    wire [2:0]   fadd_vld_ke4;
    wire [31:0]  faddout32_h_je4;
    wire [31:0]  faddout32_h_ke4;
    wire [31:0]  faddout32_l_je4;
    wire [31:0]  faddout32_l_ke4;
    wire [63:0]  faddout64_je4;
    wire [63:0]  faddout64_ke4;
    wire         fcvt_cvt_f_to_f_je1;
    wire         fcvt_cvt_f_to_i_je1;
    wire         fcvt_cvt_i_to_f_je1;
    wire         fcvt_cvts_je1;
    wire [5:0]   fcvt_ex_h_e3;
    wire [5:0]   fcvt_ex_l_e3;
    wire         fcvt_frint_je1;
    wire         fcvt_hp_sel_top_je1;
    wire         fcvt_imm_je1;
    wire [5:0]   fcvt_immv_je1;
    wire [1:0]   fcvt_isize_je1;
    wire         fcvt_noixc_je1;
    wire [1:0]   fcvt_osize_je1;
    wire         fcvt_recpe_je1;
    wire         fcvt_recpx_je1;
    wire         fcvt_restf_je1;
    wire         fcvt_rsqrte_je1;
    wire         fcvt_scalar_je1;
    wire [1:0]   fcvt_vld_je1;
    wire [1:0]   fcvt_vld_je3;
    wire [127:0] fcvtout_e3;
    wire         fdiv_active;
    wire [1:0]   fdiv_busy_q;
    wire [2:0]   fdiv_cmd_e1;
    wire [1:0]   fdiv_done;
    wire [1:0]   fdiv_done_ack;
    wire [1:0]   fdiv_done_hold;
    wire [1:0]   fdiv_flush;
    wire [1:0]   fdiv_scalar;
    wire         fdiv_scalar_je1;
    wire [1:0]   fdiv_uop_vld_je1;
    wire [5:0]   fdivexc32_q;
    wire [5:0]   fdivexc64_q;
    wire [31:0]  fdivout32_q;
    wire [63:0]  fdivout64_q;
    wire [63:0]  fmla_acc_je4;
    wire [63:0]  fmla_acc_ke4;
    wire         fmla_fused_je1;
    wire         fmla_fused_je4;
    wire         fmla_fused_ke1;
    wire         fmla_fused_ke4;
    wire [1:0]   fmla_fwd_je3;
    wire [1:0]   fmla_fwd_je4;
    wire [1:0]   fmla_fwd_ke3;
    wire [1:0]   fmla_fwd_ke4;
    wire         fmla_je1;
    wire         fmla_je2;
    wire         fmla_je3;
    wire         fmla_je4;
    wire         fmla_ke1;
    wire         fmla_ke2;
    wire         fmla_ke3;
    wire         fmla_ke4;
    wire         fmla_negopa_je4;
    wire         fmla_negopa_ke4;
    wire [4:0]   fmul32_ex_h_je4;
    wire [4:0]   fmul32_ex_h_ke4;
    wire [4:0]   fmul32_ex_l_je4;
    wire [4:0]   fmul32_ex_l_ke4;
    wire [4:0]   fmul64_ex_je4;
    wire [4:0]   fmul64_ex_ke4;
    wire         fmul_c_on_d_je1;
    wire         fmul_c_on_d_ke1;
    wire         fmul_div_je4;
    wire         fmul_div_ke4;
    wire         fmul_ext_je1;
    wire         fmul_ext_ke1;
    wire         fmul_f_exp_ovfl_h_je4;
    wire         fmul_f_exp_ovfl_h_ke4;
    wire         fmul_f_exp_ovfl_je4;
    wire         fmul_f_exp_ovfl_ke4;
    wire         fmul_f_exp_ovfl_l_je4;
    wire         fmul_f_exp_ovfl_l_ke4;
    wire         fmul_f_infnanzero_h_je4;
    wire         fmul_f_infnanzero_h_ke4;
    wire         fmul_f_infnanzero_je4;
    wire         fmul_f_infnanzero_ke4;
    wire         fmul_f_infnanzero_l_je4;
    wire         fmul_f_infnanzero_l_ke4;
    wire         fmul_f_prod_inf_zero_h_je4;
    wire         fmul_f_prod_inf_zero_h_ke4;
    wire         fmul_f_prod_inf_zero_je4;
    wire         fmul_f_prod_inf_zero_ke4;
    wire         fmul_f_prod_inf_zero_l_je4;
    wire         fmul_f_prod_inf_zero_l_ke4;
    wire         fmul_negmul_je1;
    wire         fmul_negmul_ke1;
    wire         fmul_srca_sel_l_je1;
    wire         fmul_srca_sel_l_ke1;
    wire         fmul_srcb_sel_h_je1;
    wire         fmul_srcb_sel_h_ke1;
    wire         fmul_srcb_sel_l_je1;
    wire         fmul_srcb_sel_l_ke1;
    wire         fmul_step_je1;
    wire         fmul_step_ke1;
    wire [2:0]   fmul_vld_je1;
    wire [2:0]   fmul_vld_je2;
    wire [2:0]   fmul_vld_je3;
    wire [2:0]   fmul_vld_je4;
    wire [2:0]   fmul_vld_ke1;
    wire [2:0]   fmul_vld_ke2;
    wire [2:0]   fmul_vld_ke3;
    wire [2:0]   fmul_vld_ke4;
    wire         fmulj_active;
    wire         fmulk_active;
    wire [55:0]  fmulout32_h_je4;
    wire [55:0]  fmulout32_h_ke4;
    wire [55:0]  fmulout32_l_je4;
    wire [55:0]  fmulout32_l_ke4;
    wire [116:0] fmulout64_je4;
    wire [116:0] fmulout64_ke4;
    wire         fsqrt_active;
    wire [1:0]   fsqrt_busy_q;
    wire [2:0]   fsqrt_cmd_e1;
    wire [1:0]   fsqrt_done;
    wire [1:0]   fsqrt_done_ack;
    wire [1:0]   fsqrt_done_hold;
    wire [1:0]   fsqrt_flush;
    wire [1:0]   fsqrt_scalar;
    wire         fsqrt_scalar_ke1;
    wire [1:0]   fsqrt_uop_vld_ke1;
    wire [5:0]   fsqrtexc32_q;
    wire [5:0]   fsqrtexc64_q;
    wire [31:0]  fsqrtout32_q;
    wire [63:0]  fsqrtout64_q;
    wire         fz_fadd_je1;
    wire         fz_fadd_ke1;
    wire         fz_je1;
    wire         fz_ke1;
    wire         fz_raw_e1_q;
    wire [7:1]   iacc_cin_sel_e3_q;
    wire         iacc_en_e1;
    wire         iacc_en_e2;
    wire         iacc_en_e4;
    wire         iacc_shfsel_e2;
    wire         iacce4_fwd_e2;
    wire         ialu_acc_en_ke1;
    wire [21:0]  ialu_ctl_je1;
    wire [21:0]  ialu_ctl_ke1;
    wire         ialu_en_je1;
    wire         ialu_en_ke1;
    wire         ialu_en_ke3;
    wire [1:0]   ialu_esize_je1;
    wire [1:0]   ialu_esize_ke1;
    wire         ialu_fp_dn_je1;
    wire         ialu_fp_dn_ke1;
    wire         ialu_fp_fz_je1;
    wire         ialu_fp_fz_ke1;
    wire [4:0]   ialu_fpex_je3_q;
    wire [4:0]   ialu_fpex_ke3_q;
    wire [3:0]   ialu_nzcv_ke3_q;
    wire         ialu_qc_je3_q;
    wire         ialu_qc_ke3_q;
    wire         ialu_qc_vld_je2;
    wire         ialu_qc_vld_ke2;
    wire         ialu_res128_je1_q;
    wire         ialu_res128_ke1_q;
    wire         ialuj_active;
    wire         ialuk_active;
    wire [127:0] ialuout_je3_q;
    wire [127:0] ialuout_ke3_q;
    wire         imac_active;
    wire         imac_qc_e4_q;
    wire         imac_qc_vld_e3;
    wire [6:0]   iqj_flush_gid;
    wire         iqj_flush_u2;
    wire [6:0]   iqk_flush_gid;
    wire         iqk_flush_u2;
    wire [1:0]   ired_esize_ke1;
    wire         ired_long_ke1;
    wire         ired_opb_en_ke1;
    wire         ired_seladd_ke1;
    wire         ired_selmax_ke1;
    wire         ired_selmin_ke1;
    wire         ired_selusgn_ke1;
    wire         ired_vrop_ke1;
    wire         iredk_active;
    wire [63:0]  iredout_ke2;
    wire         ishf3_iss_e2;
    wire         ishf3_iss_e3;
    wire         ishf3_uiss_e2;
    wire         ishf_active;
    wire         ishf_imm_e1;
    wire [7:0]   ishf_immv_e1;
    wire         ishf_insert_e1;
    wire         ishf_iss_e1;
    wire         ishf_iss_e2;
    wire         ishf_left_e1;
    wire         ishf_narrow_e1;
    wire         ishf_qc_e4_q;
    wire         ishf_qc_vld_e3;
    wire         ishf_round_e1;
    wire [2:0]   ishf_s1_sel_e1;
    wire [2:0]   ishf_s2_sel_e1;
    wire [3:0]   ishf_s3_sel_e1;
    wire [2:0]   ishf_s4_sel_e1;
    wire [3:0]   ishf_s5_sel_e1;
    wire [3:0]   ishf_s6_sel_e1;
    wire [4:0]   ishf_s7_sel_e1;
    wire         ishf_saturate_e1;
    wire         ishf_scalar_e1;
    wire         ishf_sel16_e1;
    wire         ishf_sel32_e1;
    wire         ishf_sel64_e1;
    wire         ishf_sel8_e1;
    wire         ishf_selqsat_e3;
    wire         ishf_signed_e1;
    wire         ishf_stous_e1;
    wire         ishf_uiss_e2;
    wire         ishf_widen_e1;
    wire [127:0] ishfaccout_e4_q;
    wire [127:0] ishfout_e3_q;
    wire         issq_active;
    wire         issq_crypt_active;
    wire         issq_floatj_active;
    wire         issq_floatk_active;
    wire         issq_intj_active;
    wire         issq_intk_active;
    wire [127:0] iwbout_e4_q;
    wire [127:0] lspout_je3_q;
    wire [127:0] lspout_ke3_q;
    wire         perm_en_je1;
    wire         perm_en_je2;
    wire         perm_en_ke1;
    wire         perm_en_ke2;
    wire         perm_opa_en_je1;
    wire         perm_opa_en_ke1;
    wire         perm_opb_en_je1;
    wire         perm_opb_en_ke1;
    wire         perm_opc_en_je1;
    wire         perm_opc_en_ke1;
    wire         perm_opd_en_je1;
    wire         perm_opd_en_ke1;
    wire [2:0]   perm_sign_sel0_je1;
    wire [2:0]   perm_sign_sel0_ke1;
    wire [2:0]   perm_sign_sel1_je1;
    wire [2:0]   perm_sign_sel1_ke1;
    wire [2:0]   perm_sign_sel2_je1;
    wire [2:0]   perm_sign_sel2_ke1;
    wire [2:0]   perm_sign_sel3_je1;
    wire [2:0]   perm_sign_sel3_ke1;
    wire         perm_uen_je1;
    wire         perm_uen_je2;
    wire         perm_uen_ke1;
    wire         perm_uen_ke2;
    wire         pmull_e1;
    wire         res128_e2;
    wire [1:0]   rmode_fadd_je1;
    wire [1:0]   rmode_fadd_ke1;
    wire [2:0]   rmode_fcvt_je1;
    wire [1:0]   rmode_fpscr_e1_q;
    wire [1:0]   rmode_je1;
    wire [1:0]   rmode_ke1;
    wire         selusgn_je1_q;
    wire         selusgn_ke1_q;
    wire         sha1c_e1;
    wire         sha1h_e1;
    wire         sha1m_e1;
    wire         sha1p_e1;
    wire         sha1su1_e1;
    wire         sha256h2_e1;
    wire         sha256h_e1;
    wire         sha256su0_e1;
    wire         sha256su1_e1;
    wire         shf_size_eq64_e3_q;
    wire         shf_size_ge32_e3_q;
    wire         shf_size_ne08_e3_q;
    wire [11:1]  srca_dec_hi_sel_ji1;
    wire [11:1]  srca_dec_hi_sel_ki1;
    wire [11:1]  srca_dec_sel_ji1;
    wire [11:1]  srca_dec_sel_ki1;
    wire         srca_hi_sel_ji1;
    wire         srca_hi_sel_ki1;
    wire [11:1]  srcb_dec_hi_sel_ji1;
    wire [11:1]  srcb_dec_hi_sel_ki1;
    wire [11:1]  srcb_dec_sel_ji1;
    wire [11:1]  srcb_dec_sel_ki1;
    wire         srcb_hi_sel_ji1;
    wire         srcb_hi_sel_ki1;
    wire [11:1]  srcc_dec_hi_sel_ji1;
    wire [11:1]  srcc_dec_hi_sel_ki1;
    wire [11:1]  srcc_dec_sel_ji1;
    wire [11:1]  srcc_dec_sel_ki1;
    wire         srcc_hi_sel_ji1;
    wire         srcc_hi_sel_ki1;
    wire [11:1]  srcd_dec_hi_sel_ji1;
    wire [11:1]  srcd_dec_hi_sel_ki1;
    wire [11:1]  srcd_dec_sel_ji1;
    wire [11:1]  srcd_dec_sel_ki1;
    wire         srcd_hi_sel_ji1;
    wire         srcd_hi_sel_ki1;
    wire         tbl_inst_je;
    wire         tbl_inst_ke;
    wire         tbltbx_qdest_je;
    wire         tbltbx_qdest_ke;
    wire [3:0]   tbltbx_reg_bitmask_je;
    wire [3:0]   tbltbx_reg_bitmask_ke;
    wire         tbx_inst_je;
    wire         tbx_inst_ke;
    wire [2:0]   uopnum_je;
    wire [2:0]   uopnum_ke;
    wire         vcmp_inst_je;
    wire         vcmp_inst_ke;

    maia_cx_rb urb( // outputs
                    .ahp_mode_e1_q                     (ahp_mode_e1_q),
                    .ck_gclkcx_crypt                   (ck_gclkcx_crypt),
                    .ck_gclkcx_floatj                  (ck_gclkcx_floatj),
                    .ck_gclkcx_floatk                  (ck_gclkcx_floatk),
                    .ck_gclkcx_intj                    (ck_gclkcx_intj),
                    .ck_gclkcx_intk                    (ck_gclkcx_intk),
                    .cx_acc_type_je2                   (cx_acc_type_je2[3:1]),
                    .cx_acc_type_ke2                   (cx_acc_type_ke2[3:1]),
                    .cx_active                         (cx_active),
                    .cx_dstx_tag_je2                   (cx_dstx_tag_je2[6:0]),
                    .cx_dstx_tag_ke2                   (cx_dstx_tag_ke2[6:0]),
                    .cx_ls_resx_data_cancel_w2         (cx_ls_resx_data_cancel_w2),
                    .cx_ls_resx_data_cancel_w3         (cx_ls_resx_data_cancel_w3),
                    .cx_ls_resx_dw_w1                  (cx_ls_resx_dw_w1),
                    .cx_ls_resx_dw_w2                  (cx_ls_resx_dw_w2),
                    .cx_ls_resx_tag_vld_w1             (cx_ls_resx_tag_vld_w1),
                    .cx_ls_resx_tag_vld_w2             (cx_ls_resx_tag_vld_w2),
                    .cx_ls_resx_tag_w1                 (cx_ls_resx_tag_w1[6:0]),
                    .cx_ls_resx_tag_w2                 (cx_ls_resx_tag_w2[6:0]),
                    .cx_ls_resy_data_cancel_w2         (cx_ls_resy_data_cancel_w2),
                    .cx_ls_resy_data_cancel_w3         (cx_ls_resy_data_cancel_w3),
                    .cx_ls_resy_dw_w1                  (cx_ls_resy_dw_w1),
                    .cx_ls_resy_dw_w2                  (cx_ls_resy_dw_w2),
                    .cx_ls_resy_tag_vld_w1             (cx_ls_resy_tag_vld_w1),
                    .cx_ls_resy_tag_vld_w2             (cx_ls_resy_tag_vld_w2),
                    .cx_ls_resy_tag_w1                 (cx_ls_resy_tag_w1[6:0]),
                    .cx_ls_resy_tag_w2                 (cx_ls_resy_tag_w2[6:0]),
                    .cx_mx_resp_tag_vld_w1             (cx_mx_resp_tag_vld_w1),
                    .cx_mx_resp_tag_vld_w2             (cx_mx_resp_tag_vld_w2),
                    .cx_mx_resp_tag_w1                 (cx_mx_resp_tag_w1[4:0]),
                    .cx_mx_resp_tag_w2                 (cx_mx_resp_tag_w2[4:0]),
                    .cx_reset3                         (cx_reset3),
                    .cx_resp_data_w1                   (cx_resp_data_w1[3:0]),
                    .cx_resp_data_w2                   (cx_resp_data_w2[3:0]),
                    .cx_resp_qfbit_gid_w1              (cx_resp_qfbit_gid_w1[5:0]),
                    .cx_resp_qfbit_vld_w1              (cx_resp_qfbit_vld_w1),
                    .cx_resp_qfbit_w2                  (cx_resp_qfbit_w2[6:0]),
                    .cx_resp_tag_vld_w0                (cx_resp_tag_vld_w0),
                    .cx_resp_tag_vld_w1                (cx_resp_tag_vld_w1),
                    .cx_resp_tag_vld_w2                (cx_resp_tag_vld_w2),
                    .cx_resp_tag_w0                    (cx_resp_tag_w0[4:0]),
                    .cx_resp_tag_w1                    (cx_resp_tag_w1[4:0]),
                    .cx_resp_tag_w2                    (cx_resp_tag_w2[4:0]),
                    .cx_resx_dw_w0                     (cx_resx_dw_w0),
                    .cx_resx_dw_w0m1                   (cx_resx_dw_w0m1),
                    .cx_resx_dw_w1                     (cx_resx_dw_w1),
                    .cx_resx_dw_w2                     (cx_resx_dw_w2),
                    .cx_resx_qfbit_gid_w1              (cx_resx_qfbit_gid_w1[5:0]),
                    .cx_resx_qfbit_vld_w1              (cx_resx_qfbit_vld_w1),
                    .cx_resx_qfbit_w2                  (cx_resx_qfbit_w2[6:0]),
                    .cx_resx_region_w0m1               (cx_resx_region_w0m1[2:0]),
                    .cx_resx_sel_dec_jw0               (cx_resx_sel_dec_jw0[49:32]),
                    .cx_resx_sel_dec_kw0               (cx_resx_sel_dec_kw0[13:0]),
                    .cx_resx_selj_w0                   (cx_resx_selj_w0),
                    .cx_resx_tag_vld_w0                (cx_resx_tag_vld_w0),
                    .cx_resx_tag_vld_w0m1              (cx_resx_tag_vld_w0m1),
                    .cx_resx_tag_vld_w1                (cx_resx_tag_vld_w1),
                    .cx_resx_tag_vld_w2                (cx_resx_tag_vld_w2),
                    .cx_resx_tag_w0                    (cx_resx_tag_w0[6:0]),
                    .cx_resx_tag_w0m1                  (cx_resx_tag_w0m1[6:0]),
                    .cx_resx_tag_w1                    (cx_resx_tag_w1[6:0]),
                    .cx_resx_tag_w2                    (cx_resx_tag_w2[6:0]),
                    .cx_resy_dw_w0                     (cx_resy_dw_w0),
                    .cx_resy_dw_w0m1                   (cx_resy_dw_w0m1),
                    .cx_resy_dw_w1                     (cx_resy_dw_w1),
                    .cx_resy_dw_w2                     (cx_resy_dw_w2),
                    .cx_resy_qfbit_gid_w1              (cx_resy_qfbit_gid_w1[5:0]),
                    .cx_resy_qfbit_vld_w1              (cx_resy_qfbit_vld_w1),
                    .cx_resy_qfbit_w2                  (cx_resy_qfbit_w2[6:0]),
                    .cx_resy_region_w0m1               (cx_resy_region_w0m1[2:0]),
                    .cx_resy_sel_dec_jw0               (cx_resy_sel_dec_jw0[49:32]),
                    .cx_resy_sel_dec_kw0               (cx_resy_sel_dec_kw0[13:0]),
                    .cx_resy_selj_w0                   (cx_resy_selj_w0),
                    .cx_resy_tag_vld_w0                (cx_resy_tag_vld_w0),
                    .cx_resy_tag_vld_w0m1              (cx_resy_tag_vld_w0m1),
                    .cx_resy_tag_vld_w1                (cx_resy_tag_vld_w1),
                    .cx_resy_tag_vld_w2                (cx_resy_tag_vld_w2),
                    .cx_resy_tag_w0                    (cx_resy_tag_w0[6:0]),
                    .cx_resy_tag_w0m1                  (cx_resy_tag_w0m1[6:0]),
                    .cx_resy_tag_w1                    (cx_resy_tag_w1[6:0]),
                    .cx_resy_tag_w2                    (cx_resy_tag_w2[6:0]),
                    .cx_resz_dw_w0                     (cx_resz_dw_w0),
                    .cx_resz_dw_w0m1                   (cx_resz_dw_w0m1),
                    .cx_resz_dw_w1                     (cx_resz_dw_w1),
                    .cx_resz_dw_w2                     (cx_resz_dw_w2),
                    .cx_resz_qfbit_gid_w1              (cx_resz_qfbit_gid_w1[5:0]),
                    .cx_resz_qfbit_vld_w1              (cx_resz_qfbit_vld_w1),
                    .cx_resz_qfbit_w2                  (cx_resz_qfbit_w2[6:0]),
                    .cx_resz_region_w0m1               (cx_resz_region_w0m1[2:0]),
                    .cx_resz_sel_dec_jw0               (cx_resz_sel_dec_jw0[49:32]),
                    .cx_resz_sel_dec_kw0               (cx_resz_sel_dec_kw0[13:0]),
                    .cx_resz_selj_w0                   (cx_resz_selj_w0),
                    .cx_resz_tag_vld_w0                (cx_resz_tag_vld_w0),
                    .cx_resz_tag_vld_w0m1              (cx_resz_tag_vld_w0m1),
                    .cx_resz_tag_vld_w1                (cx_resz_tag_vld_w1),
                    .cx_resz_tag_vld_w2                (cx_resz_tag_vld_w2),
                    .cx_resz_tag_w0                    (cx_resz_tag_w0[6:0]),
                    .cx_resz_tag_w0m1                  (cx_resz_tag_w0m1[6:0]),
                    .cx_resz_tag_w1                    (cx_resz_tag_w1[6:0]),
                    .cx_resz_tag_w2                    (cx_resz_tag_w2[6:0]),
                    .cx_sx_ldxcancel_sel_jw1           (cx_sx_ldxcancel_sel_jw1),
                    .cx_sx_ldxcancel_sel_kw1           (cx_sx_ldxcancel_sel_kw1),
                    .cx_sx_resp_tag_jw1                (cx_sx_resp_tag_jw1[4:0]),
                    .cx_sx_resp_tag_jw2                (cx_sx_resp_tag_jw2[4:0]),
                    .cx_sx_resp_tag_kw1                (cx_sx_resp_tag_kw1[4:0]),
                    .cx_sx_resp_tag_kw2                (cx_sx_resp_tag_kw2[4:0]),
                    .cx_sx_resp_tag_vld_jw1            (cx_sx_resp_tag_vld_jw1),
                    .cx_sx_resp_tag_vld_jw2            (cx_sx_resp_tag_vld_jw2),
                    .cx_sx_resp_tag_vld_kw1            (cx_sx_resp_tag_vld_kw1),
                    .cx_sx_resp_tag_vld_kw2            (cx_sx_resp_tag_vld_kw2),
                    .dn_raw_e1_q                       (dn_raw_e1_q),
                    .fdiv_busy_q                       (fdiv_busy_q[1:0]),
                    .fdiv_done_hold                    (fdiv_done_hold[1:0]),
                    .fdiv_flush                        (fdiv_flush[1:0]),
                    .fdiv_scalar                       (fdiv_scalar[1:0]),
                    .fsqrt_busy_q                      (fsqrt_busy_q[1:0]),
                    .fsqrt_done_hold                   (fsqrt_done_hold[1:0]),
                    .fsqrt_flush                       (fsqrt_flush[1:0]),
                    .fsqrt_scalar                      (fsqrt_scalar[1:0]),
                    .fz_raw_e1_q                       (fz_raw_e1_q),
                    .iqj_flush_gid                     (iqj_flush_gid[6:0]),
                    .iqj_flush_u2                      (iqj_flush_u2),
                    .iqk_flush_gid                     (iqk_flush_gid[6:0]),
                    .iqk_flush_u2                      (iqk_flush_u2),
                    .rmode_fpscr_e1_q                  (rmode_fpscr_e1_q[1:0]),

                    // inputs
                    .ccpass_ke2                        (ccpass_ke2),
                    .ck_areset                         (ck_areset),
                    .ck_gclkcx                         (ck_gclkcx),
                    .ck_reset1_n_cx                    (ck_reset1_n_cx),
                    .crypt2_active                     (crypt2_active),
                    .crypt3_active                     (crypt3_active),
                    .cx_acc_type_je1                   (cx_acc_type_je1[3:0]),
                    .cx_acc_type_ke1                   (cx_acc_type_ke1[3:0]),
                    .cx_ccpass_je1                     (cx_ccpass_je1),
                    .cx_ccpass_ke1                     (cx_ccpass_ke1),
                    .cx_dst_sel_je1                    (cx_dst_sel_je1[4:0]),
                    .cx_dst_sel_ke1                    (cx_dst_sel_ke1[4:0]),
                    .cx_dstp_tag_je1                   (cx_dstp_tag_je1[4:0]),
                    .cx_dstp_tag_ke1                   (cx_dstp_tag_ke1[4:0]),
                    .cx_dstp_tag_vld_je1               (cx_dstp_tag_vld_je1),
                    .cx_dstp_tag_vld_ke1               (cx_dstp_tag_vld_ke1),
                    .cx_dstx_dw_je1                    (cx_dstx_dw_je1),
                    .cx_dstx_dw_ke1                    (cx_dstx_dw_ke1),
                    .cx_dstx_tag_je1                   (cx_dstx_tag_je1[6:0]),
                    .cx_dstx_tag_ke1                   (cx_dstx_tag_ke1[6:0]),
                    .cx_dstx_tag_vld_je1               (cx_dstx_tag_vld_je1),
                    .cx_dstx_tag_vld_ke1               (cx_dstx_tag_vld_ke1),
                    .cx_dsty_dw_je1                    (cx_dsty_dw_je1),
                    .cx_dsty_dw_ke1                    (cx_dsty_dw_ke1),
                    .cx_dsty_tag_je1                   (cx_dsty_tag_je1[6:0]),
                    .cx_dsty_tag_ke1                   (cx_dsty_tag_ke1[6:0]),
                    .cx_dsty_tag_vld_je1               (cx_dsty_tag_vld_je1),
                    .cx_dsty_tag_vld_ke1               (cx_dsty_tag_vld_ke1),
                    .cx_gid_je1                        (cx_gid_je1[6:0]),
                    .cx_gid_ke1                        (cx_gid_ke1[6:0]),
                    .cx_region_je1                     (cx_region_je1[2:0]),
                    .cx_region_ke1                     (cx_region_ke1[2:0]),
                    .cx_uop_res_latency_je1            (cx_uop_res_latency_je1[2:0]),
                    .cx_uop_res_latency_ke1            (cx_uop_res_latency_ke1[2:0]),
                    .dftrstdisable_cpu                 (dftrstdisable_cpu),
                    .dftse_cpu                         (dftse_cpu),
                    .ds_cx_cpuactlr_frc_cpu_rcg_active (ds_cx_cpuactlr_frc_cpu_rcg_active),
                    .ds_cx_flush_gid                   (ds_cx_flush_gid[6:0]),
                    .ds_cx_flush_seq                   (ds_cx_flush_seq),
                    .ds_cx_flush_u1                    (ds_cx_flush_u1),
                    .ds_cx_fpscr_ahp                   (ds_cx_fpscr_ahp),
                    .ds_cx_fpscr_dn                    (ds_cx_fpscr_dn),
                    .ds_cx_fpscr_fz                    (ds_cx_fpscr_fz),
                    .ds_cx_fpscr_rmode                 (ds_cx_fpscr_rmode[1:0]),
                    .fadd32_ex_h_je4                   (fadd32_ex_h_je4[5:0]),
                    .fadd32_ex_h_ke4                   (fadd32_ex_h_ke4[5:0]),
                    .fadd32_ex_l_je4                   (fadd32_ex_l_je4[5:0]),
                    .fadd32_ex_l_ke4                   (fadd32_ex_l_ke4[5:0]),
                    .fadd64_ex_je4                     (fadd64_ex_je4[5:0]),
                    .fadd64_ex_ke4                     (fadd64_ex_ke4[5:0]),
                    .fadd_vld_je4                      (fadd_vld_je4[2:0]),
                    .fadd_vld_ke4                      (fadd_vld_ke4[2:0]),
                    .faddj_active                      (addj_active),
                    .faddk_active                      (addk_active),
                    .fcvt_ex_h_je3                     (fcvt_ex_h_e3[5:0]),
                    .fcvt_ex_l_je3                     (fcvt_ex_l_e3[5:0]),
                    .fcvt_vld_je3                      (fcvt_vld_je3[1:0]),
                    .fcvtj_active                      (cvtj_active),
                    .fdiv_done                         (fdiv_done[1:0]),
                    .fdiv_done_ack                     (fdiv_done_ack[1:0]),
                    .fdiv_scalar_je1                   (fdiv_scalar_je1),
                    .fdiv_uop_vld_je1                  (fdiv_uop_vld_je1[1:0]),
                    .fdivexc32_q                       (fdivexc32_q[5:0]),
                    .fdivexc64_q                       (fdivexc64_q[5:0]),
                    .fdivj_active                      (fdiv_active),
                    .fmul32_ex_h_je4                   (fmul32_ex_h_je4[4:0]),
                    .fmul32_ex_h_ke4                   (fmul32_ex_h_ke4[4:0]),
                    .fmul32_ex_l_je4                   (fmul32_ex_l_je4[4:0]),
                    .fmul32_ex_l_ke4                   (fmul32_ex_l_ke4[4:0]),
                    .fmul64_ex_je4                     (fmul64_ex_je4[4:0]),
                    .fmul64_ex_ke4                     (fmul64_ex_ke4[4:0]),
                    .fmul_vld_je4                      (fmul_vld_je4[2:0]),
                    .fmul_vld_ke4                      (fmul_vld_ke4[2:0]),
                    .fmulj_active                      (fmulj_active),
                    .fmulk_active                      (fmulk_active),
                    .fsqrt_done                        (fsqrt_done[1:0]),
                    .fsqrt_done_ack                    (fsqrt_done_ack[1:0]),
                    .fsqrt_scalar_ke1                  (fsqrt_scalar_ke1),
                    .fsqrt_uop_vld_ke1                 (fsqrt_uop_vld_ke1[1:0]),
                    .fsqrtexc32_q                      (fsqrtexc32_q[5:0]),
                    .fsqrtexc64_q                      (fsqrtexc64_q[5:0]),
                    .fsqrtk_active                     (fsqrt_active),
                    .ialu_fpex_je3_q                   (ialu_fpex_je3_q[4:0]),
                    .ialu_fpex_ke3_q                   (ialu_fpex_ke3_q[4:0]),
                    .ialu_nzcv_ke3_q                   (ialu_nzcv_ke3_q[3:0]),
                    .ialu_qc_je3_q                     (ialu_qc_je3_q),
                    .ialu_qc_ke3_q                     (ialu_qc_ke3_q),
                    .ialu_qc_vld_je2                   (ialu_qc_vld_je2),
                    .ialu_qc_vld_ke2                   (ialu_qc_vld_ke2),
                    .ialuj_active                      (ialuj_active),
                    .ialuk_active                      (ialuk_active),
                    .imac_qc_e4_q                      (imac_qc_e4_q),
                    .imac_qc_vld_e3                    (imac_qc_vld_e3),
                    .imacj_active                      (imac_active),
                    .iredk_active                      (iredk_active),
                    .ishf_qc_e4_q                      (ishf_qc_e4_q),
                    .ishf_qc_vld_e3                    (ishf_qc_vld_e3),
                    .ishfk_active                      (ishf_active),
                    .issq_active                       (issq_active),
                    .issq_crypt_active                 (issq_crypt_active),
                    .issq_floatj_active                (issq_floatj_active),
                    .issq_floatk_active                (issq_floatk_active),
                    .issq_intj_active                  (issq_intj_active),
                    .issq_intk_active                  (issq_intk_active),
                    .ls_resx_data_cancel_w1            (ls_resx_data_cancel_w1),
                    .ls_resx_data_cancel_w2            (ls_resx_data_cancel_w2),
                    .ls_resx_dw_w0                     (ls_resx_dw_w0),
                    .ls_resx_dw_w1                     (ls_resx_dw_w1),
                    .ls_resx_tag_vld_w0                (ls_resx_tag_vld_w0),
                    .ls_resx_tag_vld_w1                (ls_resx_tag_vld_w1),
                    .ls_resx_tag_w0                    (ls_resx_tag_w0[6:0]),
                    .ls_resx_tag_w1                    (ls_resx_tag_w1[6:0]),
                    .ls_resy_data_cancel_w1            (ls_resy_data_cancel_w1),
                    .ls_resy_data_cancel_w2            (ls_resy_data_cancel_w2),
                    .ls_resy_dw_w0                     (ls_resy_dw_w0),
                    .ls_resy_dw_w1                     (ls_resy_dw_w1),
                    .ls_resy_tag_vld_w0                (ls_resy_tag_vld_w0),
                    .ls_resy_tag_vld_w1                (ls_resy_tag_vld_w1),
                    .ls_resy_tag_w0                    (ls_resy_tag_w0[6:0]),
                    .ls_resy_tag_w1                    (ls_resy_tag_w1[6:0]),
                    .mx_resp_tag_vld_w0                (mx_resp_tag_vld_w0),
                    .mx_resp_tag_vld_w1                (mx_resp_tag_vld_w1),
                    .mx_resp_tag_w0                    (mx_resp_tag_w0[4:0]),
                    .mx_resp_tag_w1                    (mx_resp_tag_w1[4:0]),
                    .sx_ldxcancel_sel_jw0              (sx_ldxcancel_sel_jw0),
                    .sx_ldxcancel_sel_jw1              (sx_ldxcancel_sel_jw1),
                    .sx_ldxcancel_sel_kw0              (sx_ldxcancel_sel_kw0),
                    .sx_ldxcancel_sel_kw1              (sx_ldxcancel_sel_kw1),
                    .sx_resp_tag_jw0                   (sx_resp_tag_jw0[4:0]),
                    .sx_resp_tag_jw1                   (sx_resp_tag_jw1[4:0]),
                    .sx_resp_tag_kw0                   (sx_resp_tag_kw0[4:0]),
                    .sx_resp_tag_kw1                   (sx_resp_tag_kw1[4:0]),
                    .sx_resp_tag_vld_jw0               (sx_resp_tag_vld_jw0),
                    .sx_resp_tag_vld_jw1               (sx_resp_tag_vld_jw1),
                    .sx_resp_tag_vld_kw0               (sx_resp_tag_vld_kw0),
                    .sx_resp_tag_vld_kw1               (sx_resp_tag_vld_kw1),
                    .sx_uop_vld_jw0                    (sx_uop_vld_jw0),
                    .sx_uop_vld_kw0                    (sx_uop_vld_kw0)
                  );    // urb

    maia_cx_rb_dp urb_dp( // outputs
                          .cx_fadd_srca_fp_data32_je1   (cx_fadd_srca_fp_data32_je1[63:0]),
                          .cx_fadd_srca_fp_data32_ke1   (cx_fadd_srca_fp_data32_ke1[63:0]),
                          .cx_fadd_srca_fp_data64_je1   (cx_fadd_srca_fp_data64_je1[63:0]),
                          .cx_fadd_srca_fp_data64_ke1   (cx_fadd_srca_fp_data64_ke1[63:0]),
                          .cx_fadd_srcb_fp_data32_h_je1 (cx_fadd_srcb_fp_data32_h_je1[55:0]),
                          .cx_fadd_srcb_fp_data32_h_ke1 (cx_fadd_srcb_fp_data32_h_ke1[55:0]),
                          .cx_fadd_srcb_fp_data32_l_je1 (cx_fadd_srcb_fp_data32_l_je1[55:0]),
                          .cx_fadd_srcb_fp_data32_l_ke1 (cx_fadd_srcb_fp_data32_l_ke1[55:0]),
                          .cx_fadd_srcb_fp_data64_je1   (cx_fadd_srcb_fp_data64_je1[116:0]),
                          .cx_fadd_srcb_fp_data64_ke1   (cx_fadd_srcb_fp_data64_ke1[116:0]),
                          .cx_fadd_srcc_fp_data32_je1   (cx_fadd_srcc_fp_data32_je1[63:0]),
                          .cx_fadd_srcc_fp_data32_ke1   (cx_fadd_srcc_fp_data32_ke1[63:0]),
                          .cx_fadd_srcc_fp_data64_je1   (cx_fadd_srcc_fp_data64_je1[31:0]),
                          .cx_fadd_srcc_fp_data64_ke1   (cx_fadd_srcc_fp_data64_ke1[31:0]),
                          .cx_fadd_srcd_fp_data64_je1   (cx_fadd_srcd_fp_data64_je1[31:0]),
                          .cx_fadd_srcd_fp_data64_ke1   (cx_fadd_srcd_fp_data64_ke1[31:0]),
                          .cx_fmul_srca_fp_data32_je1   (cx_fmul_srca_fp_data32_je1[63:0]),
                          .cx_fmul_srca_fp_data32_ke1   (cx_fmul_srca_fp_data32_ke1[63:0]),
                          .cx_fmul_srcb_fp_data32_je1   (cx_fmul_srcb_fp_data32_je1[63:0]),
                          .cx_fmul_srcb_fp_data32_ke1   (cx_fmul_srcb_fp_data32_ke1[63:0]),
                          .cx_resx_data_w2              (cx_resx_data_w2[63:0]),
                          .cx_resy_data_w2              (cx_resy_data_w2[63:0]),
                          .cx_resz_data_w2              (cx_resz_data_w2[63:0]),
                          .cx_srca_crypt_data_je1       (cx_srca_crypt_data_je1[63:0]),
                          .cx_srca_fp_data32_je1        (cx_srca_fp_data32_je1[63:0]),
                          .cx_srca_fp_data32_ke1        (cx_srca_fp_data32_ke1[63:0]),
                          .cx_srca_fp_data64_je1        (cx_srca_fp_data64_je1[63:0]),
                          .cx_srca_fp_data64_ke1        (cx_srca_fp_data64_ke1[63:0]),
                          .cx_srca_int_data_je1         (cx_srca_int_data_je1[63:0]),
                          .cx_srca_int_data_ke1         (cx_srca_int_data_ke1[63:0]),
                          .cx_srcb_crypt_data_je1       (cx_srcb_crypt_data_je1[63:0]),
                          .cx_srcb_fp_data32_je1        (cx_srcb_fp_data32_je1[63:0]),
                          .cx_srcb_fp_data64_je1        (cx_srcb_fp_data64_je1[63:0]),
                          .cx_srcb_fp_data64_ke1        (cx_srcb_fp_data64_ke1[63:0]),
                          .cx_srcb_int_data_je1         (cx_srcb_int_data_je1[63:0]),
                          .cx_srcb_int_data_ke1         (cx_srcb_int_data_ke1[63:0]),
                          .cx_srcc_crypt_data_je1       (cx_srcc_crypt_data_je1[63:0]),
                          .cx_srcc_fp_data32_je1        (cx_srcc_fp_data32_je1[63:0]),
                          .cx_srcc_fp_data32_ke1        (cx_srcc_fp_data32_ke1[63:0]),
                          .cx_srcc_int_data_je1         (cx_srcc_int_data_je1[63:0]),
                          .cx_srcc_int_data_ke1         (cx_srcc_int_data_ke1[63:0]),
                          .cx_srcd_crypt_data_je1       (cx_srcd_crypt_data_je1[63:0]),
                          .cx_srcd_fp_data32_je1        (cx_srcd_fp_data32_je1[31:0]),
                          .cx_srcd_fp_data32_ke1        (cx_srcd_fp_data32_ke1[31:0]),
                          .cx_srcd_int_data_je1         (cx_srcd_int_data_je1[63:0]),
                          .cx_srcd_int_data_ke1         (cx_srcd_int_data_ke1[63:0]),

                          // inputs
                          .ck_gclkcx                    (ck_gclkcx),
                          .crypt2_out_e3_q              (crypt2_out_e3_q[127:0]),
                          .crypt3_out_e6_q              (crypt3_out_e6_q[127:0]),
                          .cx_ctl_dp_fp_valid_ji1       (cx_ctl_dp_fp_valid_ji1),
                          .cx_ctl_dp_fp_valid_ki1       (cx_ctl_dp_fp_valid_ki1),
                          .cx_ctl_dp_int_valid_ji1      (cx_ctl_dp_int_valid_ji1),
                          .cx_ctl_dp_int_valid_ki1      (cx_ctl_dp_int_valid_ki1),
                          .cx_reset3                    (cx_reset3),
                          .cx_resx_dw_w1                (cx_resx_dw_w1),
                          .cx_resx_sel_dec_jw0          (cx_resx_sel_dec_jw0[49:32]),
                          .cx_resx_sel_dec_kw0          (cx_resx_sel_dec_kw0[13:0]),
                          .cx_resx_selj_w0              (cx_resx_selj_w0),
                          .cx_resx_tag_vld_w0           (cx_resx_tag_vld_w0),
                          .cx_resx_tag_vld_w1           (cx_resx_tag_vld_w1),
                          .cx_resy_dw_w1                (cx_resy_dw_w1),
                          .cx_resy_sel_dec_jw0          (cx_resy_sel_dec_jw0[49:32]),
                          .cx_resy_sel_dec_kw0          (cx_resy_sel_dec_kw0[13:0]),
                          .cx_resy_selj_w0              (cx_resy_selj_w0),
                          .cx_resy_tag_vld_w0           (cx_resy_tag_vld_w0),
                          .cx_resy_tag_vld_w1           (cx_resy_tag_vld_w1),
                          .cx_resz_dw_w1                (cx_resz_dw_w1),
                          .cx_resz_sel_dec_jw0          (cx_resz_sel_dec_jw0[49:32]),
                          .cx_resz_sel_dec_kw0          (cx_resz_sel_dec_kw0[13:0]),
                          .cx_resz_selj_w0              (cx_resz_selj_w0),
                          .cx_resz_tag_vld_w0           (cx_resz_tag_vld_w0),
                          .cx_resz_tag_vld_w1           (cx_resz_tag_vld_w1),
                          .cx_srca_data_ji1             (cx_srca_data_ji1[63:0]),
                          .cx_srca_data_ki1             (cx_srca_data_ki1[63:0]),
                          .cx_srca_fp_h_en_ji1          (cx_srca_fp_h_en_ji1),
                          .cx_srca_fp_h_en_ki1          (cx_srca_fp_h_en_ki1),
                          .cx_srca_fp_l_en_ji1          (cx_srca_fp_l_en_ji1),
                          .cx_srca_fp_l_en_ki1          (cx_srca_fp_l_en_ki1),
                          .cx_srca_int_h_en_ji1         (cx_srca_int_h_en_ji1),
                          .cx_srca_int_h_en_ki1         (cx_srca_int_h_en_ki1),
                          .cx_srca_int_l_en_ji1         (cx_srca_int_l_en_ji1),
                          .cx_srca_int_l_en_ki1         (cx_srca_int_l_en_ki1),
                          .cx_srcb_data_ji1             (cx_srcb_data_ji1[63:0]),
                          .cx_srcb_data_ki1             (cx_srcb_data_ki1[63:0]),
                          .cx_srcb_fp_h_en_ji1          (cx_srcb_fp_h_en_ji1),
                          .cx_srcb_fp_h_en_ki1          (cx_srcb_fp_h_en_ki1),
                          .cx_srcb_fp_l_en_ji1          (cx_srcb_fp_l_en_ji1),
                          .cx_srcb_fp_l_en_ki1          (cx_srcb_fp_l_en_ki1),
                          .cx_srcb_int_h_en_ji1         (cx_srcb_int_h_en_ji1),
                          .cx_srcb_int_h_en_ki1         (cx_srcb_int_h_en_ki1),
                          .cx_srcb_int_l_en_ji1         (cx_srcb_int_l_en_ji1),
                          .cx_srcb_int_l_en_ki1         (cx_srcb_int_l_en_ki1),
                          .cx_srcc_data_ji1             (cx_srcc_data_ji1[63:0]),
                          .cx_srcc_data_ki1             (cx_srcc_data_ki1[63:0]),
                          .cx_srcc_fp_h_en_ji1          (cx_srcc_fp_h_en_ji1),
                          .cx_srcc_fp_h_en_ki1          (cx_srcc_fp_h_en_ki1),
                          .cx_srcc_fp_l_en_ji1          (cx_srcc_fp_l_en_ji1),
                          .cx_srcc_fp_l_en_ki1          (cx_srcc_fp_l_en_ki1),
                          .cx_srcc_int_h_en_ji1         (cx_srcc_int_h_en_ji1),
                          .cx_srcc_int_h_en_ki1         (cx_srcc_int_h_en_ki1),
                          .cx_srcc_int_l_en_ji1         (cx_srcc_int_l_en_ji1),
                          .cx_srcc_int_l_en_ki1         (cx_srcc_int_l_en_ki1),
                          .cx_srcd_data_ji1             (cx_srcd_data_ji1[63:0]),
                          .cx_srcd_data_ki1             (cx_srcd_data_ki1[63:0]),
                          .cx_srcd_fp_h_en_ji1          (cx_srcd_fp_h_en_ji1),
                          .cx_srcd_fp_h_en_ki1          (cx_srcd_fp_h_en_ki1),
                          .cx_srcd_fp_l_en_ji1          (cx_srcd_fp_l_en_ji1),
                          .cx_srcd_fp_l_en_ki1          (cx_srcd_fp_l_en_ki1),
                          .cx_srcd_int_h_en_ji1         (cx_srcd_int_h_en_ji1),
                          .cx_srcd_int_h_en_ki1         (cx_srcd_int_h_en_ki1),
                          .cx_srcd_int_l_en_ji1         (cx_srcd_int_l_en_ji1),
                          .cx_srcd_int_l_en_ki1         (cx_srcd_int_l_en_ki1),
                          .cx_uop_vld_ji1               (cx_uop_vld_ji1),
                          .cx_uop_vld_ki1               (cx_uop_vld_ki1),
                          .fadd_srca_sel_h_je1          (fadd_srca_sel_h_je1[2:0]),
                          .fadd_srca_sel_h_ke1          (fadd_srca_sel_h_ke1[2:0]),
                          .fadd_srca_sel_l_je1          (fadd_srca_sel_l_je1),
                          .fadd_srca_sel_l_ke1          (fadd_srca_sel_l_ke1),
                          .fadd_srcb_sel_h_je1          (fadd_srcb_sel_h_je1[2:0]),
                          .fadd_srcb_sel_h_ke1          (fadd_srcb_sel_h_ke1[2:0]),
                          .fadd_srcb_sel_l_je1          (fadd_srcb_sel_l_je1[2:0]),
                          .fadd_srcb_sel_l_ke1          (fadd_srcb_sel_l_ke1[2:0]),
                          .fadd_vld_je4                 (fadd_vld_je4[0]),
                          .fadd_vld_ke4                 (fadd_vld_ke4[0]),
                          .faddout32_h_je4              (faddout32_h_je4[31:0]),
                          .faddout32_h_ke4              (faddout32_h_ke4[31:0]),
                          .faddout32_l_je4              (faddout32_l_je4[31:0]),
                          .faddout32_l_ke4              (faddout32_l_ke4[31:0]),
                          .faddout64_je4                (faddout64_je4[63:0]),
                          .faddout64_ke4                (faddout64_ke4[63:0]),
                          .fcvtout_je3                  (fcvtout_e3[127:0]),
                          .fdivout32_q                  (fdivout32_q[31:0]),
                          .fdivout64_q                  (fdivout64_q[63:0]),
                          .fmla_acc_je4                 (fmla_acc_je4[63:0]),
                          .fmla_acc_ke4                 (fmla_acc_ke4[63:0]),
                          .fmla_fwd_je3                 (fmla_fwd_je3[1:0]),
                          .fmla_fwd_je4                 (fmla_fwd_je4[1:0]),
                          .fmla_fwd_ke3                 (fmla_fwd_ke3[1:0]),
                          .fmla_fwd_ke4                 (fmla_fwd_ke4[1:0]),
                          .fmla_je3                     (fmla_je3),
                          .fmla_je4                     (fmla_je4),
                          .fmla_ke3                     (fmla_ke3),
                          .fmla_ke4                     (fmla_ke4),
                          .fmul_srca_sel_l_je1          (fmul_srca_sel_l_je1),
                          .fmul_srca_sel_l_ke1          (fmul_srca_sel_l_ke1),
                          .fmul_srcb_sel_h_je1          (fmul_srcb_sel_h_je1),
                          .fmul_srcb_sel_h_ke1          (fmul_srcb_sel_h_ke1),
                          .fmul_srcb_sel_l_je1          (fmul_srcb_sel_l_je1),
                          .fmul_srcb_sel_l_ke1          (fmul_srcb_sel_l_ke1),
                          .fmul_vld_je4                 (fmul_vld_je4[0]),
                          .fmul_vld_ke4                 (fmul_vld_ke4[0]),
                          .fmulout32_h_je4              (fmulout32_h_je4[55:0]),
                          .fmulout32_h_ke4              (fmulout32_h_ke4[55:0]),
                          .fmulout32_l_je4              (fmulout32_l_je4[55:0]),
                          .fmulout32_l_ke4              (fmulout32_l_ke4[55:0]),
                          .fmulout64_je4                (fmulout64_je4[116:0]),
                          .fmulout64_ke4                (fmulout64_ke4[116:0]),
                          .fsqrtout32_q                 (fsqrtout32_q[31:0]),
                          .fsqrtout64_q                 (fsqrtout64_q[63:0]),
                          .ialuout_je3_q                (ialuout_je3_q[127:0]),
                          .ialuout_ke3_q                (ialuout_ke3_q[127:0]),
                          .iredout_ke3_q                (iredout_ke2[63:0]),
                          .ishfaccout_e4_q              (ishfaccout_e4_q[127:0]),
                          .ishfout_e3_q                 (ishfout_e3_q[127:0]),
                          .iwbout_e4_q                  (iwbout_e4_q[127:0]),
                          .ls_resx_data_w2              (ls_resx_data_w2[63:0]),
                          .ls_resy_data_w2              (ls_resy_data_w2[63:0]),
                          .lspout_je3_q                 (lspout_je3_q[127:0]),
                          .lspout_ke3_q                 (lspout_ke3_q[127:0]),
                          .srca_dec_hi_sel_ji1          (srca_dec_hi_sel_ji1[11:1]),
                          .srca_dec_hi_sel_ki1          (srca_dec_hi_sel_ki1[11:1]),
                          .srca_dec_sel_ji1             (srca_dec_sel_ji1[11:1]),
                          .srca_dec_sel_ki1             (srca_dec_sel_ki1[11:1]),
                          .srca_hi_sel_ji1              (srca_hi_sel_ji1),
                          .srca_hi_sel_ki1              (srca_hi_sel_ki1),
                          .srcb_dec_hi_sel_ji1          (srcb_dec_hi_sel_ji1[11:1]),
                          .srcb_dec_hi_sel_ki1          (srcb_dec_hi_sel_ki1[11:1]),
                          .srcb_dec_sel_ji1             (srcb_dec_sel_ji1[11:1]),
                          .srcb_dec_sel_ki1             (srcb_dec_sel_ki1[11:1]),
                          .srcb_hi_sel_ji1              (srcb_hi_sel_ji1),
                          .srcb_hi_sel_ki1              (srcb_hi_sel_ki1),
                          .srcc_dec_hi_sel_ji1          (srcc_dec_hi_sel_ji1[11:1]),
                          .srcc_dec_hi_sel_ki1          (srcc_dec_hi_sel_ki1[11:1]),
                          .srcc_dec_sel_ji1             (srcc_dec_sel_ji1[11:1]),
                          .srcc_dec_sel_ki1             (srcc_dec_sel_ki1[11:1]),
                          .srcc_hi_sel_ji1              (srcc_hi_sel_ji1),
                          .srcc_hi_sel_ki1              (srcc_hi_sel_ki1),
                          .srcd_dec_hi_sel_ji1          (srcd_dec_hi_sel_ji1[11:1]),
                          .srcd_dec_hi_sel_ki1          (srcd_dec_hi_sel_ki1[11:1]),
                          .srcd_dec_sel_ji1             (srcd_dec_sel_ji1[11:1]),
                          .srcd_dec_sel_ki1             (srcd_dec_sel_ki1[11:1]),
                          .srcd_hi_sel_ji1              (srcd_hi_sel_ji1),
                          .srcd_hi_sel_ki1              (srcd_hi_sel_ki1)
                        );    // urb_dp

    maia_cx_issq_top uissq_top( // outputs
                                .cx_acc_type_je1           (cx_acc_type_je1[3:0]),
                                .cx_acc_type_ke1           (cx_acc_type_ke1[3:0]),
                                .cx_credit_j               (cx_credit_j[2:0]),
                                .cx_credit_k               (cx_credit_k[2:0]),
                                .cx_ctl_dp_fp_valid_ji1    (cx_ctl_dp_fp_valid_ji1),
                                .cx_ctl_dp_fp_valid_ki1    (cx_ctl_dp_fp_valid_ki1),
                                .cx_ctl_dp_int_valid_ji1   (cx_ctl_dp_int_valid_ji1),
                                .cx_ctl_dp_int_valid_ki1   (cx_ctl_dp_int_valid_ki1),
                                .cx_dstp_tag_je1           (cx_dstp_tag_je1[4:0]),
                                .cx_dstp_tag_ke1           (cx_dstp_tag_ke1[4:0]),
                                .cx_dstp_tag_vld_je1       (cx_dstp_tag_vld_je1),
                                .cx_dstp_tag_vld_ke1       (cx_dstp_tag_vld_ke1),
                                .cx_dstx_dw_je1            (cx_dstx_dw_je1),
                                .cx_dstx_dw_ke1            (cx_dstx_dw_ke1),
                                .cx_dstx_tag_je1           (cx_dstx_tag_je1[6:0]),
                                .cx_dstx_tag_ke1           (cx_dstx_tag_ke1[6:0]),
                                .cx_dstx_tag_vld_je1       (cx_dstx_tag_vld_je1),
                                .cx_dstx_tag_vld_ke1       (cx_dstx_tag_vld_ke1),
                                .cx_dsty_dw_je1            (cx_dsty_dw_je1),
                                .cx_dsty_dw_ke1            (cx_dsty_dw_ke1),
                                .cx_dsty_tag_je1           (cx_dsty_tag_je1[6:0]),
                                .cx_dsty_tag_ke1           (cx_dsty_tag_ke1[6:0]),
                                .cx_dsty_tag_vld_je1       (cx_dsty_tag_vld_je1),
                                .cx_dsty_tag_vld_ke1       (cx_dsty_tag_vld_ke1),
                                .cx_gid_je1                (cx_gid_je1[6:0]),
                                .cx_gid_ke1                (cx_gid_ke1[6:0]),
                                .cx_mla_fwd_sel_je1        (cx_mla_fwd_sel_je1[4:0]),
                                .cx_mla_fwd_sel_ke1        (cx_mla_fwd_sel_ke1[4:0]),
                                .cx_region_je1             (cx_region_je1[2:0]),
                                .cx_region_ke1             (cx_region_ke1[2:0]),
                                .cx_res128_je1             (cx_res128_je1),
                                .cx_res128_ke1             (cx_res128_ke1),
                                .cx_srca_data_ji1          (cx_srca_data_ji1[63:0]),
                                .cx_srca_data_ki1          (cx_srca_data_ki1[63:0]),
                                .cx_srca_en_je1            (cx_srca_en_je1),
                                .cx_srca_en_ke1            (cx_srca_en_ke1),
                                .cx_srca_fp_h_en_ji1       (cx_srca_fp_h_en_ji1),
                                .cx_srca_fp_h_en_ki1       (cx_srca_fp_h_en_ki1),
                                .cx_srca_fp_l_en_ji1       (cx_srca_fp_l_en_ji1),
                                .cx_srca_fp_l_en_ki1       (cx_srca_fp_l_en_ki1),
                                .cx_srca_int_h_en_ji1      (cx_srca_int_h_en_ji1),
                                .cx_srca_int_h_en_ki1      (cx_srca_int_h_en_ki1),
                                .cx_srca_int_l_en_ji1      (cx_srca_int_l_en_ji1),
                                .cx_srca_int_l_en_ki1      (cx_srca_int_l_en_ki1),
                                .cx_srcb_data_ji1          (cx_srcb_data_ji1[63:0]),
                                .cx_srcb_data_ki1          (cx_srcb_data_ki1[63:0]),
                                .cx_srcb_en_je1            (cx_srcb_en_je1),
                                .cx_srcb_en_ke1            (cx_srcb_en_ke1),
                                .cx_srcb_fp_h_en_ji1       (cx_srcb_fp_h_en_ji1),
                                .cx_srcb_fp_h_en_ki1       (cx_srcb_fp_h_en_ki1),
                                .cx_srcb_fp_l_en_ji1       (cx_srcb_fp_l_en_ji1),
                                .cx_srcb_fp_l_en_ki1       (cx_srcb_fp_l_en_ki1),
                                .cx_srcb_int_h_en_ji1      (cx_srcb_int_h_en_ji1),
                                .cx_srcb_int_h_en_ki1      (cx_srcb_int_h_en_ki1),
                                .cx_srcb_int_l_en_ji1      (cx_srcb_int_l_en_ji1),
                                .cx_srcb_int_l_en_ki1      (cx_srcb_int_l_en_ki1),
                                .cx_srcc_data_ji1          (cx_srcc_data_ji1[63:0]),
                                .cx_srcc_data_ki1          (cx_srcc_data_ki1[63:0]),
                                .cx_srcc_en_je1            (cx_srcc_en_je1),
                                .cx_srcc_en_ke1            (cx_srcc_en_ke1),
                                .cx_srcc_fp_h_en_ji1       (cx_srcc_fp_h_en_ji1),
                                .cx_srcc_fp_h_en_ki1       (cx_srcc_fp_h_en_ki1),
                                .cx_srcc_fp_l_en_ji1       (cx_srcc_fp_l_en_ji1),
                                .cx_srcc_fp_l_en_ki1       (cx_srcc_fp_l_en_ki1),
                                .cx_srcc_int_h_en_ji1      (cx_srcc_int_h_en_ji1),
                                .cx_srcc_int_h_en_ki1      (cx_srcc_int_h_en_ki1),
                                .cx_srcc_int_l_en_ji1      (cx_srcc_int_l_en_ji1),
                                .cx_srcc_int_l_en_ki1      (cx_srcc_int_l_en_ki1),
                                .cx_srcd_data_ji1          (cx_srcd_data_ji1[63:0]),
                                .cx_srcd_data_ki1          (cx_srcd_data_ki1[63:0]),
                                .cx_srcd_en_je1            (cx_srcd_en_je1),
                                .cx_srcd_en_ke1            (cx_srcd_en_ke1),
                                .cx_srcd_fp_h_en_ji1       (cx_srcd_fp_h_en_ji1),
                                .cx_srcd_fp_h_en_ki1       (cx_srcd_fp_h_en_ki1),
                                .cx_srcd_fp_l_en_ji1       (cx_srcd_fp_l_en_ji1),
                                .cx_srcd_fp_l_en_ki1       (cx_srcd_fp_l_en_ki1),
                                .cx_srcd_int_h_en_ji1      (cx_srcd_int_h_en_ji1),
                                .cx_srcd_int_h_en_ki1      (cx_srcd_int_h_en_ki1),
                                .cx_srcd_int_l_en_ji1      (cx_srcd_int_l_en_ji1),
                                .cx_srcd_int_l_en_ki1      (cx_srcd_int_l_en_ki1),
                                .cx_srcp_data_je1          (cx_srcp_data_je1[3:0]),
                                .cx_srcp_data_ke1          (cx_srcp_data_ke1[3:0]),
                                .cx_uop_ctl_ji1            (cx_uop_ctl_ji1[58:0]),
                                .cx_uop_ctl_ki1            (cx_uop_ctl_ki1[58:0]),
                                .cx_uop_vld_je1            (cx_uop_vld_je1),
                                .cx_uop_vld_ji1            (cx_uop_vld_ji1),
                                .cx_uop_vld_ke1            (cx_uop_vld_ke1),
                                .cx_uop_vld_ki1            (cx_uop_vld_ki1),
                                .fdiv_done_ack             (fdiv_done_ack[1:0]),
                                .fdiv_uop_vld_je1          (fdiv_uop_vld_je1[1:0]),
                                .fsqrt_done_ack            (fsqrt_done_ack[1:0]),
                                .fsqrt_uop_vld_ke1         (fsqrt_uop_vld_ke1[1:0]),
                                .issq_active               (issq_active),
                                .issq_crypt_active         (issq_crypt_active),
                                .issq_floatj_active        (issq_floatj_active),
                                .issq_floatk_active        (issq_floatk_active),
                                .issq_intj_active          (issq_intj_active),
                                .issq_intk_active          (issq_intk_active),
                                .srca_dec_hi_sel_ji1       (srca_dec_hi_sel_ji1[11:1]),
                                .srca_dec_hi_sel_ki1       (srca_dec_hi_sel_ki1[11:1]),
                                .srca_dec_sel_ji1          (srca_dec_sel_ji1[11:1]),
                                .srca_dec_sel_ki1          (srca_dec_sel_ki1[11:1]),
                                .srca_hi_sel_ji1           (srca_hi_sel_ji1),
                                .srca_hi_sel_ki1           (srca_hi_sel_ki1),
                                .srcb_dec_hi_sel_ji1       (srcb_dec_hi_sel_ji1[11:1]),
                                .srcb_dec_hi_sel_ki1       (srcb_dec_hi_sel_ki1[11:1]),
                                .srcb_dec_sel_ji1          (srcb_dec_sel_ji1[11:1]),
                                .srcb_dec_sel_ki1          (srcb_dec_sel_ki1[11:1]),
                                .srcb_hi_sel_ji1           (srcb_hi_sel_ji1),
                                .srcb_hi_sel_ki1           (srcb_hi_sel_ki1),
                                .srcc_dec_hi_sel_ji1       (srcc_dec_hi_sel_ji1[11:1]),
                                .srcc_dec_hi_sel_ki1       (srcc_dec_hi_sel_ki1[11:1]),
                                .srcc_dec_sel_ji1          (srcc_dec_sel_ji1[11:1]),
                                .srcc_dec_sel_ki1          (srcc_dec_sel_ki1[11:1]),
                                .srcc_hi_sel_ji1           (srcc_hi_sel_ji1),
                                .srcc_hi_sel_ki1           (srcc_hi_sel_ki1),
                                .srcd_dec_hi_sel_ji1       (srcd_dec_hi_sel_ji1[11:1]),
                                .srcd_dec_hi_sel_ki1       (srcd_dec_hi_sel_ki1[11:1]),
                                .srcd_dec_sel_ji1          (srcd_dec_sel_ji1[11:1]),
                                .srcd_dec_sel_ki1          (srcd_dec_sel_ki1[11:1]),
                                .srcd_hi_sel_ji1           (srcd_hi_sel_ji1),
                                .srcd_hi_sel_ki1           (srcd_hi_sel_ki1),

                                // inputs
                                .ck_gclkcx                 (ck_gclkcx),
                                .crp3_vld_je1              (crp3_vld_je1),
                                .cx_acc_type_je2           (cx_acc_type_je2[3:1]),
                                .cx_acc_type_ke2           (cx_acc_type_ke2[3:1]),
                                .cx_dstx_tag_je2           (cx_dstx_tag_je2[6:0]),
                                .cx_dstx_tag_ke2           (cx_dstx_tag_ke2[6:0]),
                                .cx_ls_resx_data_cancel_w2 (cx_ls_resx_data_cancel_w2),
                                .cx_ls_resx_data_cancel_w3 (cx_ls_resx_data_cancel_w3),
                                .cx_ls_resx_dw_w1          (cx_ls_resx_dw_w1),
                                .cx_ls_resx_dw_w2          (cx_ls_resx_dw_w2),
                                .cx_ls_resx_tag_vld_w1     (cx_ls_resx_tag_vld_w1),
                                .cx_ls_resx_tag_vld_w2     (cx_ls_resx_tag_vld_w2),
                                .cx_ls_resx_tag_w1         (cx_ls_resx_tag_w1[6:0]),
                                .cx_ls_resx_tag_w2         (cx_ls_resx_tag_w2[6:0]),
                                .cx_ls_resy_data_cancel_w2 (cx_ls_resy_data_cancel_w2),
                                .cx_ls_resy_data_cancel_w3 (cx_ls_resy_data_cancel_w3),
                                .cx_ls_resy_dw_w1          (cx_ls_resy_dw_w1),
                                .cx_ls_resy_dw_w2          (cx_ls_resy_dw_w2),
                                .cx_ls_resy_tag_vld_w1     (cx_ls_resy_tag_vld_w1),
                                .cx_ls_resy_tag_vld_w2     (cx_ls_resy_tag_vld_w2),
                                .cx_ls_resy_tag_w1         (cx_ls_resy_tag_w1[6:0]),
                                .cx_ls_resy_tag_w2         (cx_ls_resy_tag_w2[6:0]),
                                .cx_mx_resp_tag_vld_w1     (cx_mx_resp_tag_vld_w1),
                                .cx_mx_resp_tag_vld_w2     (cx_mx_resp_tag_vld_w2),
                                .cx_mx_resp_tag_w1         (cx_mx_resp_tag_w1[4:0]),
                                .cx_mx_resp_tag_w2         (cx_mx_resp_tag_w2[4:0]),
                                .cx_reset3                 (cx_reset3),
                                .cx_resp_data_w1           (cx_resp_data_w1[3:0]),
                                .cx_resp_data_w2           (cx_resp_data_w2[3:0]),
                                .cx_resp_tag_vld_w0        (cx_resp_tag_vld_w0),
                                .cx_resp_tag_vld_w1        (cx_resp_tag_vld_w1),
                                .cx_resp_tag_vld_w2        (cx_resp_tag_vld_w2),
                                .cx_resp_tag_w0            (cx_resp_tag_w0[4:0]),
                                .cx_resp_tag_w1            (cx_resp_tag_w1[4:0]),
                                .cx_resp_tag_w2            (cx_resp_tag_w2[4:0]),
                                .cx_resx_data_w2           (cx_resx_data_w2[63:0]),
                                .cx_resx_dw_w0             (cx_resx_dw_w0),
                                .cx_resx_dw_w0m1           (cx_resx_dw_w0m1),
                                .cx_resx_dw_w1             (cx_resx_dw_w1),
                                .cx_resx_dw_w2             (cx_resx_dw_w2),
                                .cx_resx_region_w0m1       (cx_resx_region_w0m1[2:0]),
                                .cx_resx_tag_vld_w0        (cx_resx_tag_vld_w0),
                                .cx_resx_tag_vld_w0m1      (cx_resx_tag_vld_w0m1),
                                .cx_resx_tag_vld_w1        (cx_resx_tag_vld_w1),
                                .cx_resx_tag_vld_w2        (cx_resx_tag_vld_w2),
                                .cx_resx_tag_w0            (cx_resx_tag_w0[6:0]),
                                .cx_resx_tag_w0m1          (cx_resx_tag_w0m1[6:0]),
                                .cx_resx_tag_w1            (cx_resx_tag_w1[6:0]),
                                .cx_resx_tag_w2            (cx_resx_tag_w2[6:0]),
                                .cx_resy_data_w2           (cx_resy_data_w2[63:0]),
                                .cx_resy_dw_w0             (cx_resy_dw_w0),
                                .cx_resy_dw_w0m1           (cx_resy_dw_w0m1),
                                .cx_resy_dw_w1             (cx_resy_dw_w1),
                                .cx_resy_dw_w2             (cx_resy_dw_w2),
                                .cx_resy_region_w0m1       (cx_resy_region_w0m1[2:0]),
                                .cx_resy_tag_vld_w0        (cx_resy_tag_vld_w0),
                                .cx_resy_tag_vld_w0m1      (cx_resy_tag_vld_w0m1),
                                .cx_resy_tag_vld_w1        (cx_resy_tag_vld_w1),
                                .cx_resy_tag_vld_w2        (cx_resy_tag_vld_w2),
                                .cx_resy_tag_w0            (cx_resy_tag_w0[6:0]),
                                .cx_resy_tag_w0m1          (cx_resy_tag_w0m1[6:0]),
                                .cx_resy_tag_w1            (cx_resy_tag_w1[6:0]),
                                .cx_resy_tag_w2            (cx_resy_tag_w2[6:0]),
                                .cx_resz_data_w2           (cx_resz_data_w2[63:0]),
                                .cx_resz_dw_w0             (cx_resz_dw_w0),
                                .cx_resz_dw_w0m1           (cx_resz_dw_w0m1),
                                .cx_resz_dw_w1             (cx_resz_dw_w1),
                                .cx_resz_dw_w2             (cx_resz_dw_w2),
                                .cx_resz_region_w0m1       (cx_resz_region_w0m1[2:0]),
                                .cx_resz_tag_vld_w0        (cx_resz_tag_vld_w0),
                                .cx_resz_tag_vld_w0m1      (cx_resz_tag_vld_w0m1),
                                .cx_resz_tag_vld_w1        (cx_resz_tag_vld_w1),
                                .cx_resz_tag_vld_w2        (cx_resz_tag_vld_w2),
                                .cx_resz_tag_w0            (cx_resz_tag_w0[6:0]),
                                .cx_resz_tag_w0m1          (cx_resz_tag_w0m1[6:0]),
                                .cx_resz_tag_w1            (cx_resz_tag_w1[6:0]),
                                .cx_resz_tag_w2            (cx_resz_tag_w2[6:0]),
                                .cx_sx_ldxcancel_sel_jw1   (cx_sx_ldxcancel_sel_jw1),
                                .cx_sx_ldxcancel_sel_kw1   (cx_sx_ldxcancel_sel_kw1),
                                .cx_sx_resp_tag_jw1        (cx_sx_resp_tag_jw1[4:0]),
                                .cx_sx_resp_tag_jw2        (cx_sx_resp_tag_jw2[4:0]),
                                .cx_sx_resp_tag_kw1        (cx_sx_resp_tag_kw1[4:0]),
                                .cx_sx_resp_tag_kw2        (cx_sx_resp_tag_kw2[4:0]),
                                .cx_sx_resp_tag_vld_jw1    (cx_sx_resp_tag_vld_jw1),
                                .cx_sx_resp_tag_vld_jw2    (cx_sx_resp_tag_vld_jw2),
                                .cx_sx_resp_tag_vld_kw1    (cx_sx_resp_tag_vld_kw1),
                                .cx_sx_resp_tag_vld_kw2    (cx_sx_resp_tag_vld_kw2),
                                .ds_cx_dstp_tag_jp2        (ds_cx_dstp_tag_jp2[4:0]),
                                .ds_cx_dstp_tag_kp2        (ds_cx_dstp_tag_kp2[4:0]),
                                .ds_cx_dstp_tag_vld_jp2    (ds_cx_dstp_tag_vld_jp2),
                                .ds_cx_dstp_tag_vld_kp2    (ds_cx_dstp_tag_vld_kp2),
                                .ds_cx_dstx_dw_jp2         (ds_cx_dstx_dw_jp2),
                                .ds_cx_dstx_dw_kp2         (ds_cx_dstx_dw_kp2),
                                .ds_cx_dstx_tag_jp2        (ds_cx_dstx_tag_jp2[6:0]),
                                .ds_cx_dstx_tag_kp2        (ds_cx_dstx_tag_kp2[6:0]),
                                .ds_cx_dstx_tag_vld_jp2    (ds_cx_dstx_tag_vld_jp2),
                                .ds_cx_dstx_tag_vld_kp2    (ds_cx_dstx_tag_vld_kp2),
                                .ds_cx_dsty_dw_jp2         (ds_cx_dsty_dw_jp2),
                                .ds_cx_dsty_dw_kp2         (ds_cx_dsty_dw_kp2),
                                .ds_cx_dsty_tag_jp2        (ds_cx_dsty_tag_jp2[6:0]),
                                .ds_cx_dsty_tag_kp2        (ds_cx_dsty_tag_kp2[6:0]),
                                .ds_cx_dsty_tag_vld_jp2    (ds_cx_dsty_tag_vld_jp2),
                                .ds_cx_dsty_tag_vld_kp2    (ds_cx_dsty_tag_vld_kp2),
                                .ds_cx_gid_jp2             (ds_cx_gid_jp2[6:0]),
                                .ds_cx_gid_kp2             (ds_cx_gid_kp2[6:0]),
                                .ds_cx_prt_sel_jp1         (ds_cx_prt_sel_jp1[2:0]),
                                .ds_cx_prt_sel_kp1         (ds_cx_prt_sel_kp1[2:0]),
                                .ds_cx_srca_data_jp2       (ds_cx_srca_data_jp2[63:0]),
                                .ds_cx_srca_data_kp2       (ds_cx_srca_data_kp2[63:0]),
                                .ds_cx_srca_data_vld_jp2   (ds_cx_srca_data_vld_jp2[1:0]),
                                .ds_cx_srca_data_vld_kp2   (ds_cx_srca_data_vld_kp2[1:0]),
                                .ds_cx_srcb_data_jp2       (ds_cx_srcb_data_jp2[63:0]),
                                .ds_cx_srcb_data_kp2       (ds_cx_srcb_data_kp2[63:0]),
                                .ds_cx_srcb_data_vld_jp2   (ds_cx_srcb_data_vld_jp2[1:0]),
                                .ds_cx_srcb_data_vld_kp2   (ds_cx_srcb_data_vld_kp2[1:0]),
                                .ds_cx_srcc_data_jp2       (ds_cx_srcc_data_jp2[63:0]),
                                .ds_cx_srcc_data_kp2       (ds_cx_srcc_data_kp2[63:0]),
                                .ds_cx_srcc_data_vld_jp2   (ds_cx_srcc_data_vld_jp2[1:0]),
                                .ds_cx_srcc_data_vld_kp2   (ds_cx_srcc_data_vld_kp2[1:0]),
                                .ds_cx_srcd_data_jp2       (ds_cx_srcd_data_jp2[63:0]),
                                .ds_cx_srcd_data_kp2       (ds_cx_srcd_data_kp2[63:0]),
                                .ds_cx_srcd_data_vld_jp2   (ds_cx_srcd_data_vld_jp2[1:0]),
                                .ds_cx_srcd_data_vld_kp2   (ds_cx_srcd_data_vld_kp2[1:0]),
                                .ds_cx_srcp_data_jp2       (ds_cx_srcp_data_jp2[3:0]),
                                .ds_cx_srcp_data_kp2       (ds_cx_srcp_data_kp2[3:0]),
                                .ds_cx_srcp_data_vld_jp2   (ds_cx_srcp_data_vld_jp2),
                                .ds_cx_srcp_data_vld_kp2   (ds_cx_srcp_data_vld_kp2),
                                .ds_cx_swdw_nuke_jp2       (ds_cx_swdw_nuke_jp2),
                                .ds_cx_swdw_nuke_kp2       (ds_cx_swdw_nuke_kp2),
                                .ds_cx_uop_ctl_jp2         (ds_cx_uop_ctl_jp2[58:0]),
                                .ds_cx_uop_ctl_kp2         (ds_cx_uop_ctl_kp2[58:0]),
                                .ds_cx_uop_vld_jp2         (ds_cx_uop_vld_jp2),
                                .ds_cx_uop_vld_kp2         (ds_cx_uop_vld_kp2),
                                .ds_srca_dw_0p1            (ds_srca_dw_0p1),
                                .ds_srca_dw_1p1            (ds_srca_dw_1p1),
                                .ds_srca_dw_2p1            (ds_srca_dw_2p1),
                                .ds_srca_prdcr_dw_0p1      (ds_srca_prdcr_dw_0p1),
                                .ds_srca_prdcr_dw_1p1      (ds_srca_prdcr_dw_1p1),
                                .ds_srca_prdcr_dw_2p1      (ds_srca_prdcr_dw_2p1),
                                .ds_srca_tag_0p1           (ds_srca_tag_0p1[6:0]),
                                .ds_srca_tag_1p1           (ds_srca_tag_1p1[6:0]),
                                .ds_srca_tag_2p1           (ds_srca_tag_2p1[6:0]),
                                .ds_srca_tag_vld_0p1       (ds_srca_tag_vld_0p1),
                                .ds_srca_tag_vld_1p1       (ds_srca_tag_vld_1p1),
                                .ds_srca_tag_vld_2p1       (ds_srca_tag_vld_2p1),
                                .ds_srcb_dw_0p1            (ds_srcb_dw_0p1),
                                .ds_srcb_dw_1p1            (ds_srcb_dw_1p1),
                                .ds_srcb_dw_2p1            (ds_srcb_dw_2p1),
                                .ds_srcb_prdcr_dw_0p1      (ds_srcb_prdcr_dw_0p1),
                                .ds_srcb_prdcr_dw_1p1      (ds_srcb_prdcr_dw_1p1),
                                .ds_srcb_prdcr_dw_2p1      (ds_srcb_prdcr_dw_2p1),
                                .ds_srcb_tag_0p1           (ds_srcb_tag_0p1[6:0]),
                                .ds_srcb_tag_1p1           (ds_srcb_tag_1p1[6:0]),
                                .ds_srcb_tag_2p1           (ds_srcb_tag_2p1[6:0]),
                                .ds_srcb_tag_vld_0p1       (ds_srcb_tag_vld_0p1),
                                .ds_srcb_tag_vld_1p1       (ds_srcb_tag_vld_1p1),
                                .ds_srcb_tag_vld_2p1       (ds_srcb_tag_vld_2p1),
                                .ds_srcc_dw_0p1            (ds_srcc_dw_0p1),
                                .ds_srcc_dw_1p1            (ds_srcc_dw_1p1),
                                .ds_srcc_dw_2p1            (ds_srcc_dw_2p1),
                                .ds_srcc_prdcr_dw_0p1      (ds_srcc_prdcr_dw_0p1),
                                .ds_srcc_prdcr_dw_1p1      (ds_srcc_prdcr_dw_1p1),
                                .ds_srcc_prdcr_dw_2p1      (ds_srcc_prdcr_dw_2p1),
                                .ds_srcc_tag_0p1           (ds_srcc_tag_0p1[6:0]),
                                .ds_srcc_tag_1p1           (ds_srcc_tag_1p1[6:0]),
                                .ds_srcc_tag_2p1           (ds_srcc_tag_2p1[6:0]),
                                .ds_srcc_tag_vld_0p1       (ds_srcc_tag_vld_0p1),
                                .ds_srcc_tag_vld_1p1       (ds_srcc_tag_vld_1p1),
                                .ds_srcc_tag_vld_2p1       (ds_srcc_tag_vld_2p1),
                                .ds_srcd_dw_0p1            (ds_srcd_dw_0p1),
                                .ds_srcd_dw_1p1            (ds_srcd_dw_1p1),
                                .ds_srcd_dw_2p1            (ds_srcd_dw_2p1),
                                .ds_srcd_prdcr_dw_0p1      (ds_srcd_prdcr_dw_0p1),
                                .ds_srcd_prdcr_dw_1p1      (ds_srcd_prdcr_dw_1p1),
                                .ds_srcd_prdcr_dw_2p1      (ds_srcd_prdcr_dw_2p1),
                                .ds_srcd_tag_0p1           (ds_srcd_tag_0p1[6:0]),
                                .ds_srcd_tag_1p1           (ds_srcd_tag_1p1[6:0]),
                                .ds_srcd_tag_2p1           (ds_srcd_tag_2p1[6:0]),
                                .ds_srcd_tag_vld_0p1       (ds_srcd_tag_vld_0p1),
                                .ds_srcd_tag_vld_1p1       (ds_srcd_tag_vld_1p1),
                                .ds_srcd_tag_vld_2p1       (ds_srcd_tag_vld_2p1),
                                .ds_srcp_tag_0p1           (ds_srcp_tag_0p1[4:0]),
                                .ds_srcp_tag_1p1           (ds_srcp_tag_1p1[4:0]),
                                .ds_srcp_tag_2p1           (ds_srcp_tag_2p1[4:0]),
                                .ds_srcp_tag_vld_0p1       (ds_srcp_tag_vld_0p1),
                                .ds_srcp_tag_vld_1p1       (ds_srcp_tag_vld_1p1),
                                .ds_srcp_tag_vld_2p1       (ds_srcp_tag_vld_2p1),
                                .fadd_hazard1_j            (fadd_hazard1_j),
                                .fadd_hazard1_k            (fadd_hazard1_k),
                                .fdiv_busy_q               (fdiv_busy_q[1:0]),
                                .fdiv_done                 (fdiv_done[1:0]),
                                .fdiv_done_hold            (fdiv_done_hold[1:0]),
                                .fdiv_flush                (fdiv_flush[1:0]),
                                .fdiv_scalar               (fdiv_scalar[1:0]),
                                .fmla_je2                  (fmla_je2),
                                .fmla_ke2                  (fmla_ke2),
                                .fsqrt_busy_q              (fsqrt_busy_q[1:0]),
                                .fsqrt_done                (fsqrt_done[1:0]),
                                .fsqrt_done_hold           (fsqrt_done_hold[1:0]),
                                .fsqrt_flush               (fsqrt_flush[1:0]),
                                .fsqrt_scalar              (fsqrt_scalar[1:0]),
                                .iqj_flush_gid             (iqj_flush_gid[6:0]),
                                .iqj_flush_u2              (iqj_flush_u2),
                                .iqk_flush_gid             (iqk_flush_gid[6:0]),
                                .iqk_flush_u2              (iqk_flush_u2),
                                .ls_resx_data_w2           (ls_resx_data_w2[63:0]),
                                .ls_resy_data_w2           (ls_resy_data_w2[63:0]),
                                .mx_resp_data_w2           (mx_resp_data_w2[3:0]),
                                .sx_resp_data_jw2          (sx_resp_data_jw2[3:0]),
                                .sx_resp_data_kw2          (sx_resp_data_kw2[3:0])
                              );    // uissq_top

    maia_cx_dpj_ctl udpj_ctl( // outputs
                              .aesd_e1                (aesd_e1),
                              .aesdimc_e1             (aesdimc_e1),
                              .aese_e1                (aese_e1),
                              .aesemc_e1              (aesemc_e1),
                              .aesimc_e1              (aesimc_e1),
                              .aesmc_e1               (aesmc_e1),
                              .crp3_vld_je1           (crp3_vld_je1),
                              .crypt2_vld_e1          (crypt2_vld_e1),
                              .crypt3_vld0_e1         (crypt3_vld0_e1),
                              .crypt3_vld1_e1         (crypt3_vld1_e1),
                              .cx_ccpass_je1          (cx_ccpass_je1),
                              .cx_dst_sel_je1         (cx_dst_sel_je1[4:0]),
                              .cx_imac_cmd_e1         (cx_imac_cmd_e1[12:0]),
                              .cx_imac_vld_e1         (cx_imac_vld_e1),
                              .cx_mla_fwd_sel_je2     (cx_mla_fwd_sel_je2[4:0]),
                              .cx_mla_fwd_sel_je3     (cx_mla_fwd_sel_je3[4:0]),
                              .cx_uop_res_latency_je1 (cx_uop_res_latency_je1[2:0]),
                              .dn_fadd_je1            (dn_fadd_je1),
                              .dn_je1                 (dn_je1),
                              .dstx_bytesel_je1       (dstx_bytesel_je[39:0]),
                              .dsty_bytesel_je1       (dsty_bytesel_je[39:0]),
                              .fadd_absin_je1         (fadd_absin_je1),
                              .fadd_absout_je1        (fadd_absout_je1),
                              .fadd_ccpass_je1        (fadd_ccpass_je1),
                              .fadd_hazard1_j         (fadd_hazard1_j),
                              .fadd_srca_sel_h_je1    (fadd_srca_sel_h_je1[2:0]),
                              .fadd_srca_sel_l_je1    (fadd_srca_sel_l_je1),
                              .fadd_srcb_sel_h_je1    (fadd_srcb_sel_h_je1[2:0]),
                              .fadd_srcb_sel_l_je1    (fadd_srcb_sel_l_je1[2:0]),
                              .fadd_sub_je1           (fadd_sub_je1),
                              .fadd_vld_je1           (fadd_vld_je1[2:0]),
                              .fadd_vld_je4           (fadd_vld_je4[2:0]),
                              .fcvt_cvt_f_to_f_je1    (fcvt_cvt_f_to_f_je1),
                              .fcvt_cvt_f_to_i_je1    (fcvt_cvt_f_to_i_je1),
                              .fcvt_cvt_i_to_f_je1    (fcvt_cvt_i_to_f_je1),
                              .fcvt_cvts_je1          (fcvt_cvts_je1),
                              .fcvt_frint_je1         (fcvt_frint_je1),
                              .fcvt_hp_sel_top_je1    (fcvt_hp_sel_top_je1),
                              .fcvt_imm_je1           (fcvt_imm_je1),
                              .fcvt_immv_je1          (fcvt_immv_je1[5:0]),
                              .fcvt_isize_je1         (fcvt_isize_je1[1:0]),
                              .fcvt_noixc_je1         (fcvt_noixc_je1),
                              .fcvt_osize_je1         (fcvt_osize_je1[1:0]),
                              .fcvt_recpe_je1         (fcvt_recpe_je1),
                              .fcvt_recpx_je1         (fcvt_recpx_je1),
                              .fcvt_restf_je1         (fcvt_restf_je1),
                              .fcvt_rsqrte_je1        (fcvt_rsqrte_je1),
                              .fcvt_scalar_je1        (fcvt_scalar_je1),
                              .fcvt_vld_je1           (fcvt_vld_je1[1:0]),
                              .fcvt_vld_je3           (fcvt_vld_je3[1:0]),
                              .fdiv_cmd_e1            (fdiv_cmd_e1[2:0]),
                              .fdiv_scalar_je1        (fdiv_scalar_je1),
                              .fmla_fused_je1         (fmla_fused_je1),
                              .fmla_fused_je4         (fmla_fused_je4),
                              .fmla_fwd_je3           (fmla_fwd_je3[1:0]),
                              .fmla_fwd_je4           (fmla_fwd_je4[1:0]),
                              .fmla_je1               (fmla_je1),
                              .fmla_je2               (fmla_je2),
                              .fmla_je3               (fmla_je3),
                              .fmla_je4               (fmla_je4),
                              .fmla_negopa_je4        (fmla_negopa_je4),
                              .fmul_c_on_d_je1        (fmul_c_on_d_je1),
                              .fmul_div_je4           (fmul_div_je4),
                              .fmul_ext_je1           (fmul_ext_je1),
                              .fmul_negmul_je1        (fmul_negmul_je1),
                              .fmul_srca_sel_l_je1    (fmul_srca_sel_l_je1),
                              .fmul_srcb_sel_h_je1    (fmul_srcb_sel_h_je1),
                              .fmul_srcb_sel_l_je1    (fmul_srcb_sel_l_je1),
                              .fmul_step_je1          (fmul_step_je1),
                              .fmul_vld_je1           (fmul_vld_je1[2:0]),
                              .fmul_vld_je2           (fmul_vld_je2[2:0]),
                              .fmul_vld_je3           (fmul_vld_je3[2:0]),
                              .fmul_vld_je4           (fmul_vld_je4[2:0]),
                              .fz_fadd_je1            (fz_fadd_je1),
                              .fz_je1                 (fz_je1),
                              .ialu_ctl_je1           (ialu_ctl_je1[21:0]),
                              .ialu_en_je1            (ialu_en_je1),
                              .ialu_esize_je1         (ialu_esize_je1[1:0]),
                              .ialu_fp_dn_je1         (ialu_fp_dn_je1),
                              .ialu_fp_fz_je1         (ialu_fp_fz_je1),
                              .ialu_res128_je1        (ialu_res128_je1_q),
                              .ialu_selusgn_je1       (selusgn_je1_q),
                              .perm_en_je1            (perm_en_je1),
                              .perm_en_je2            (perm_en_je2),
                              .perm_opa_en_je1        (perm_opa_en_je1),
                              .perm_opb_en_je1        (perm_opb_en_je1),
                              .perm_opc_en_je1        (perm_opc_en_je1),
                              .perm_opd_en_je1        (perm_opd_en_je1),
                              .perm_sign_sel0_je1     (perm_sign_sel0_je1[2:0]),
                              .perm_sign_sel1_je1     (perm_sign_sel1_je1[2:0]),
                              .perm_sign_sel2_je1     (perm_sign_sel2_je1[2:0]),
                              .perm_sign_sel3_je1     (perm_sign_sel3_je1[2:0]),
                              .perm_uen_je1           (perm_uen_je1),
                              .perm_uen_je2           (perm_uen_je2),
                              .pmull_e1               (pmull_e1),
                              .rmode_fadd_je1         (rmode_fadd_je1[1:0]),
                              .rmode_fcvt_je1         (rmode_fcvt_je1[2:0]),
                              .rmode_je1              (rmode_je1[1:0]),
                              .sha1c_e1               (sha1c_e1),
                              .sha1h_e1               (sha1h_e1),
                              .sha1m_e1               (sha1m_e1),
                              .sha1p_e1               (sha1p_e1),
                              .sha1su1_e1             (sha1su1_e1),
                              .sha256h2_e1            (sha256h2_e1),
                              .sha256h_e1             (sha256h_e1),
                              .sha256su0_e1           (sha256su0_e1),
                              .sha256su1_e1           (sha256su1_e1),
                              .tbl_inst_je1           (tbl_inst_je),
                              .tbltbx_qdest_je1       (tbltbx_qdest_je),
                              .tbltbx_reg_bitmask_je1 (tbltbx_reg_bitmask_je[3:0]),
                              .tbx_inst_je1           (tbx_inst_je),
                              .uopnum_je1             (uopnum_je[2:0]),
                              .vcmp_inst_je1          (vcmp_inst_je),

                              // inputs
                              .ck_gclkcx              (ck_gclkcx),
                              .cx_mla_fwd_sel_je1     (cx_mla_fwd_sel_je1[4:0]),
                              .cx_res128_je1          (cx_res128_je1),
                              .cx_reset3              (cx_reset3),
                              .cx_srca_en_je1         (cx_srca_en_je1),
                              .cx_srcb_en_je1         (cx_srcb_en_je1),
                              .cx_srcc_en_je1         (cx_srcc_en_je1),
                              .cx_srcd_en_je1         (cx_srcd_en_je1),
                              .cx_srcp_data_je1       (cx_srcp_data_je1[3:0]),
                              .cx_uop_ctl_ji1         (cx_uop_ctl_ji1[58:0]),
                              .cx_uop_vld_je1         (cx_uop_vld_je1),
                              .cx_uop_vld_ji1         (cx_uop_vld_ji1),
                              .dn_raw_e1_q            (dn_raw_e1_q),
                              .ds_cx_aarch32_state    (ds_cx_aarch32_state),
                              .ds_cx_aarch64_state    (ds_cx_aarch64_state),
                              .fz_raw_e1_q            (fz_raw_e1_q),
                              .rmode_fpscr_e1_q       (rmode_fpscr_e1_q[1:0]),
                              .srca_hi_sel_ji1        (srca_hi_sel_ji1),
                              .srcb_hi_sel_ji1        (srcb_hi_sel_ji1)
                            );    // udpj_ctl

    maia_cx_dpk_ctl udpk_ctl( // outputs
                              .acc_size_eq64_e3_q     (acc_size_eq64_e3_q),
                              .acc_size_ge32_e3_q     (acc_size_ge32_e3_q),
                              .acc_size_ne08_e3_q     (acc_size_ne08_e3_q),
                              .c00_x_sel_e1           (c00_x_sel_e1[3:0]),
                              .c00_y_sel_e1           (c00_y_sel_e1),
                              .c01_x_sel_e1           (c01_x_sel_e1[5:0]),
                              .c01_y_sel_e1           (c01_y_sel_e1[2:0]),
                              .c02_x_sel_e1           (c02_x_sel_e1[5:0]),
                              .c02_y_sel_e1           (c02_y_sel_e1[2:0]),
                              .c03_x_sel_e1           (c03_x_sel_e1[6:0]),
                              .c03_y_sel_e1           (c03_y_sel_e1[4:0]),
                              .c04_x_sel_e1           (c04_x_sel_e1[5:0]),
                              .c04_y_sel_e1           (c04_y_sel_e1[2:0]),
                              .c05_x_sel_e1           (c05_x_sel_e1[7:0]),
                              .c05_y_sel_e1           (c05_y_sel_e1[4:0]),
                              .c06_x_sel_e1           (c06_x_sel_e1[5:0]),
                              .c06_y_sel_e1           (c06_y_sel_e1[4:0]),
                              .c07_x_sel_e1           (c07_x_sel_e1[6:0]),
                              .c07_y_sel_e1           (c07_y_sel_e1[4:0]),
                              .c08_x_sel_e1           (c08_x_sel_e1[6:0]),
                              .c08_y_sel_e1           (c08_y_sel_e1[2:0]),
                              .c09_x_sel_e1           (c09_x_sel_e1[8:0]),
                              .c09_y_sel_e1           (c09_y_sel_e1[5:0]),
                              .c10_x_sel_e1           (c10_x_sel_e1[8:0]),
                              .c10_y_sel_e1           (c10_y_sel_e1[5:0]),
                              .c11_x_sel_e1           (c11_x_sel_e1[9:0]),
                              .c11_y_sel_e1           (c11_y_sel_e1[7:0]),
                              .c12_x_sel_e1           (c12_x_sel_e1[7:0]),
                              .c12_y_sel_e1           (c12_y_sel_e1[5:0]),
                              .c13_x_sel_e1           (c13_x_sel_e1[9:0]),
                              .c13_y_sel_e1           (c13_y_sel_e1[6:0]),
                              .c14_x_sel_e1           (c14_x_sel_e1[7:0]),
                              .c14_y_sel_e1           (c14_y_sel_e1[6:0]),
                              .c15_x_sel_e1           (c15_x_sel_e1[7:0]),
                              .c15_y_sel_e1           (c15_y_sel_e1[6:0]),
                              .ccpass_ke2             (ccpass_ke2),
                              .cx_ccpass_ke1          (cx_ccpass_ke1),
                              .cx_dst_sel_ke1         (cx_dst_sel_ke1[4:0]),
                              .cx_mla_fwd_sel_ke2     (cx_mla_fwd_sel_ke2[4:0]),
                              .cx_mla_fwd_sel_ke3     (cx_mla_fwd_sel_ke3[4:0]),
                              .cx_uop_res_latency_ke1 (cx_uop_res_latency_ke1[2:0]),
                              .dn_fadd_ke1            (dn_fadd_ke1),
                              .dn_ke1                 (dn_ke1),
                              .dstx_bytesel_ke1       (dstx_bytesel_ke[39:0]),
                              .dsty_bytesel_ke1       (dsty_bytesel_ke[39:0]),
                              .fadd_absin_ke1         (fadd_absin_ke1),
                              .fadd_absout_ke1        (fadd_absout_ke1),
                              .fadd_ccpass_ke1        (fadd_ccpass_ke1),
                              .fadd_hazard1_k         (fadd_hazard1_k),
                              .fadd_srca_sel_h_ke1    (fadd_srca_sel_h_ke1[2:0]),
                              .fadd_srca_sel_l_ke1    (fadd_srca_sel_l_ke1),
                              .fadd_srcb_sel_h_ke1    (fadd_srcb_sel_h_ke1[2:0]),
                              .fadd_srcb_sel_l_ke1    (fadd_srcb_sel_l_ke1[2:0]),
                              .fadd_sub_ke1           (fadd_sub_ke1),
                              .fadd_vld_ke1           (fadd_vld_ke1[2:0]),
                              .fadd_vld_ke4           (fadd_vld_ke4[2:0]),
                              .fmla_fused_ke1         (fmla_fused_ke1),
                              .fmla_fused_ke4         (fmla_fused_ke4),
                              .fmla_fwd_ke3           (fmla_fwd_ke3[1:0]),
                              .fmla_fwd_ke4           (fmla_fwd_ke4[1:0]),
                              .fmla_ke1               (fmla_ke1),
                              .fmla_ke2               (fmla_ke2),
                              .fmla_ke3               (fmla_ke3),
                              .fmla_ke4               (fmla_ke4),
                              .fmla_negopa_ke4        (fmla_negopa_ke4),
                              .fmul_c_on_d_ke1        (fmul_c_on_d_ke1),
                              .fmul_div_ke4           (fmul_div_ke4),
                              .fmul_ext_ke1           (fmul_ext_ke1),
                              .fmul_negmul_ke1        (fmul_negmul_ke1),
                              .fmul_srca_sel_l_ke1    (fmul_srca_sel_l_ke1),
                              .fmul_srcb_sel_h_ke1    (fmul_srcb_sel_h_ke1),
                              .fmul_srcb_sel_l_ke1    (fmul_srcb_sel_l_ke1),
                              .fmul_step_ke1          (fmul_step_ke1),
                              .fmul_vld_ke1           (fmul_vld_ke1[2:0]),
                              .fmul_vld_ke2           (fmul_vld_ke2[2:0]),
                              .fmul_vld_ke3           (fmul_vld_ke3[2:0]),
                              .fmul_vld_ke4           (fmul_vld_ke4[2:0]),
                              .fsqrt_cmd_e1           (fsqrt_cmd_e1[2:0]),
                              .fsqrt_scalar_ke1       (fsqrt_scalar_ke1),
                              .fz_fadd_ke1            (fz_fadd_ke1),
                              .fz_ke1                 (fz_ke1),
                              .iacc_cin_sel_e3_q      (iacc_cin_sel_e3_q[7:1]),
                              .iacc_en_e1             (iacc_en_e1),
                              .iacc_en_e2             (iacc_en_e2),
                              .iacc_en_e4             (iacc_en_e4),
                              .iacc_shfsel_e2         (iacc_shfsel_e2),
                              .iacce4_fwd_e2          (iacce4_fwd_e2),
                              .ialu_acc_en_ke1        (ialu_acc_en_ke1),
                              .ialu_ctl_ke1           (ialu_ctl_ke1[21:0]),
                              .ialu_en_ke1            (ialu_en_ke1),
                              .ialu_en_ke3            (ialu_en_ke3),
                              .ialu_esize_ke1         (ialu_esize_ke1[1:0]),
                              .ialu_fp_dn_ke1         (ialu_fp_dn_ke1),
                              .ialu_fp_fz_ke1         (ialu_fp_fz_ke1),
                              .ialu_res128_ke1        (ialu_res128_ke1_q),
                              .ialu_selusgn_ke1       (selusgn_ke1_q),
                              .ired_esize_ke1         (ired_esize_ke1[1:0]),
                              .ired_long_ke1          (ired_long_ke1),
                              .ired_opb_en_ke1        (ired_opb_en_ke1),
                              .ired_seladd_ke1        (ired_seladd_ke1),
                              .ired_selmax_ke1        (ired_selmax_ke1),
                              .ired_selmin_ke1        (ired_selmin_ke1),
                              .ired_selusgn_ke1       (ired_selusgn_ke1),
                              .ired_vrop_ke1          (ired_vrop_ke1),
                              .ishf3_iss_e2           (ishf3_iss_e2),
                              .ishf3_iss_e3           (ishf3_iss_e3),
                              .ishf3_uiss_e2          (ishf3_uiss_e2),
                              .ishf_imm_e1            (ishf_imm_e1),
                              .ishf_immv_e1           (ishf_immv_e1[7:0]),
                              .ishf_insert_e1         (ishf_insert_e1),
                              .ishf_iss_e1            (ishf_iss_e1),
                              .ishf_iss_e2            (ishf_iss_e2),
                              .ishf_left_e1           (ishf_left_e1),
                              .ishf_narrow_e1         (ishf_narrow_e1),
                              .ishf_qc_vld_e3         (ishf_qc_vld_e3),
                              .ishf_round_e1          (ishf_round_e1),
                              .ishf_s1_sel_e1         (ishf_s1_sel_e1[2:0]),
                              .ishf_s2_sel_e1         (ishf_s2_sel_e1[2:0]),
                              .ishf_s3_sel_e1         (ishf_s3_sel_e1[3:0]),
                              .ishf_s4_sel_e1         (ishf_s4_sel_e1[2:0]),
                              .ishf_s5_sel_e1         (ishf_s5_sel_e1[3:0]),
                              .ishf_s6_sel_e1         (ishf_s6_sel_e1[3:0]),
                              .ishf_s7_sel_e1         (ishf_s7_sel_e1[4:0]),
                              .ishf_saturate_e1       (ishf_saturate_e1),
                              .ishf_scalar_e1         (ishf_scalar_e1),
                              .ishf_sel16_e1          (ishf_sel16_e1),
                              .ishf_sel32_e1          (ishf_sel32_e1),
                              .ishf_sel64_e1          (ishf_sel64_e1),
                              .ishf_sel8_e1           (ishf_sel8_e1),
                              .ishf_selqsat_e3        (ishf_selqsat_e3),
                              .ishf_signed_e1         (ishf_signed_e1),
                              .ishf_stous_e1          (ishf_stous_e1),
                              .ishf_uiss_e2           (ishf_uiss_e2),
                              .ishf_widen_e1          (ishf_widen_e1),
                              .perm_en_ke1            (perm_en_ke1),
                              .perm_en_ke2            (perm_en_ke2),
                              .perm_opa_en_ke1        (perm_opa_en_ke1),
                              .perm_opb_en_ke1        (perm_opb_en_ke1),
                              .perm_opc_en_ke1        (perm_opc_en_ke1),
                              .perm_opd_en_ke1        (perm_opd_en_ke1),
                              .perm_sign_sel0_ke1     (perm_sign_sel0_ke1[2:0]),
                              .perm_sign_sel1_ke1     (perm_sign_sel1_ke1[2:0]),
                              .perm_sign_sel2_ke1     (perm_sign_sel2_ke1[2:0]),
                              .perm_sign_sel3_ke1     (perm_sign_sel3_ke1[2:0]),
                              .perm_uen_ke1           (perm_uen_ke1),
                              .perm_uen_ke2           (perm_uen_ke2),
                              .res128_e2              (res128_e2),
                              .rmode_fadd_ke1         (rmode_fadd_ke1[1:0]),
                              .rmode_ke1              (rmode_ke1[1:0]),
                              .shf_size_eq64_e3_q     (shf_size_eq64_e3_q),
                              .shf_size_ge32_e3_q     (shf_size_ge32_e3_q),
                              .shf_size_ne08_e3_q     (shf_size_ne08_e3_q),
                              .tbl_inst_ke1           (tbl_inst_ke),
                              .tbltbx_qdest_ke1       (tbltbx_qdest_ke),
                              .tbltbx_reg_bitmask_ke1 (tbltbx_reg_bitmask_ke[3:0]),
                              .tbx_inst_ke1           (tbx_inst_ke),
                              .uopnum_ke1             (uopnum_ke[2:0]),
                              .vcmp_inst_ke1          (vcmp_inst_ke),

                              // inputs
                              .ck_gclkcx              (ck_gclkcx),
                              .cx_mla_fwd_sel_ke1     (cx_mla_fwd_sel_ke1[4:0]),
                              .cx_res128_ke1          (cx_res128_ke1),
                              .cx_reset3              (cx_reset3),
                              .cx_srca_en_ke1         (cx_srca_en_ke1),
                              .cx_srcb_en_ke1         (cx_srcb_en_ke1),
                              .cx_srcc_en_ke1         (cx_srcc_en_ke1),
                              .cx_srcd_en_ke1         (cx_srcd_en_ke1),
                              .cx_srcp_data_ke1       (cx_srcp_data_ke1[3:0]),
                              .cx_uop_ctl_ki1         (cx_uop_ctl_ki1[58:0]),
                              .cx_uop_vld_ke1         (cx_uop_vld_ke1),
                              .cx_uop_vld_ki1         (cx_uop_vld_ki1),
                              .dn_raw_e1_q            (dn_raw_e1_q),
                              .ds_cx_aarch32_state    (ds_cx_aarch32_state),
                              .ds_cx_aarch64_state    (ds_cx_aarch64_state),
                              .fz_raw_e1_q            (fz_raw_e1_q),
                              .rmode_fpscr_e1_q       (rmode_fpscr_e1_q[1:0]),
                              .srca_hi_sel_ki1        (srca_hi_sel_ki1),
                              .srcb_hi_sel_ki1        (srcb_hi_sel_ki1)
                            );    // udpk_ctl

    maia_cx_iacc_ff uiacck_ff( // outputs
                               .cx_srcc_data_e3_q (cx_srcc_data_ke3[63:0]),
                               .cx_srcd_data_e3_q (cx_srcd_data_ke3[63:0]),

                               // inputs
                               .ck_gclkcx_int     (ck_gclkcx_intk),
                               .cx_mla_fwd_sel_e1 (cx_mla_fwd_sel_ke1[1:0]),
                               .cx_mla_fwd_sel_e2 (cx_mla_fwd_sel_ke2[1:0]),
                               .cx_reset3         (cx_reset3),
                               .cx_srcc_data_e1   (cx_srcc_int_data_ke1[63:0]),
                               .cx_srcc_en_e1     (cx_srcc_en_ke1),
                               .cx_srcd_data_e1   (cx_srcd_int_data_ke1[63:0]),
                               .iacc_en_e1        (iacc_en_e1),
                               .iacc_en_e4        (iacc_en_e4),
                               .ialu_en_ke3       (ialu_en_ke3),
                               .ialuout_e3_q      (ialuout_ke3_q[127:0]),
                               .ishf3_iss_e3      (ishf3_iss_e3),
                               .ishfaccout_e4_q   (ishfaccout_e4_q[127:0]),
                               .ishfout_e3_q      (ishfout_e3_q[127:0]),
                               .res128_e1         (cx_res128_ke1),
                               .res128_e2         (res128_e2)
                             );    // uiacck_ff

    maia_cx_fmla_ff ufmlaj_ff( // outputs
                               .fmla_acc_e4        (fmla_acc_je4[63:0]),

                               // inputs
                               .ck_gclkcx_float    (ck_gclkcx_floatj),
                               .cx_mla_fwd_sel_e1  (cx_mla_fwd_sel_je1[4:0]),
                               .cx_mla_fwd_sel_e2  (cx_mla_fwd_sel_je2[4:0]),
                               .cx_mla_fwd_sel_e3  (cx_mla_fwd_sel_je3[4:0]),
                               .cx_reset3          (cx_reset3),
                               .cx_srcc_en_e1      (cx_srcc_en_je1),
                               .cx_srcc_fp_data_e1 (cx_srcc_fp_data32_je1[63:0]),
                               .cx_srcd_fp_data_e1 (cx_srcd_fp_data32_je1[31:0]),
                               .faddout32_h_e4     (faddout32_h_je4[31:0]),
                               .faddout32_h_oe4    (faddout32_h_ke4[31:0]),
                               .faddout32_l_e4     (faddout32_l_je4[31:0]),
                               .faddout32_l_oe4    (faddout32_l_ke4[31:0]),
                               .faddout64_e4       (faddout64_je4[63:0]),
                               .faddout64_oe4      (faddout64_ke4[63:0]),
                               .fmla_e1            (fmla_je1),
                               .fmla_e2            (fmla_je2),
                               .fmla_e3            (fmla_je3),
                               .fmul_c_on_d_e1     (fmul_c_on_d_je1),
                               .fmul_vld_e1        (fmul_vld_je1[2:0]),
                               .fmul_vld_e2        (fmul_vld_je2[2:0]),
                               .fmul_vld_e3        (fmul_vld_je3[2:0]),
                               .fmulout32_h_e4     (fmulout32_h_je4[55:24]),
                               .fmulout32_h_oe4    (fmulout32_h_ke4[55:24]),
                               .fmulout32_l_e4     (fmulout32_l_je4[55:24]),
                               .fmulout32_l_oe4    (fmulout32_l_ke4[55:24]),
                               .fmulout64_e4       (fmulout64_je4[116:53]),
                               .fmulout64_oe4      (fmulout64_ke4[116:53])
                             );    // ufmlaj_ff

    maia_cx_fmla_ff ufmlak_ff( // outputs
                               .fmla_acc_e4        (fmla_acc_ke4[63:0]),

                               // inputs
                               .ck_gclkcx_float    (ck_gclkcx_floatk),
                               .cx_mla_fwd_sel_e1  (cx_mla_fwd_sel_ke1[4:0]),
                               .cx_mla_fwd_sel_e2  (cx_mla_fwd_sel_ke2[4:0]),
                               .cx_mla_fwd_sel_e3  (cx_mla_fwd_sel_ke3[4:0]),
                               .cx_reset3          (cx_reset3),
                               .cx_srcc_en_e1      (cx_srcc_en_ke1),
                               .cx_srcc_fp_data_e1 (cx_srcc_fp_data32_ke1[63:0]),
                               .cx_srcd_fp_data_e1 (cx_srcd_fp_data32_ke1[31:0]),
                               .faddout32_h_e4     (faddout32_h_ke4[31:0]),
                               .faddout32_h_oe4    (faddout32_h_je4[31:0]),
                               .faddout32_l_e4     (faddout32_l_ke4[31:0]),
                               .faddout32_l_oe4    (faddout32_l_je4[31:0]),
                               .faddout64_e4       (faddout64_ke4[63:0]),
                               .faddout64_oe4      (faddout64_je4[63:0]),
                               .fmla_e1            (fmla_ke1),
                               .fmla_e2            (fmla_ke2),
                               .fmla_e3            (fmla_ke3),
                               .fmul_c_on_d_e1     (fmul_c_on_d_ke1),
                               .fmul_vld_e1        (fmul_vld_ke1[2:0]),
                               .fmul_vld_e2        (fmul_vld_ke2[2:0]),
                               .fmul_vld_e3        (fmul_vld_ke3[2:0]),
                               .fmulout32_h_e4     (fmulout32_h_ke4[55:24]),
                               .fmulout32_h_oe4    (fmulout32_h_je4[55:24]),
                               .fmulout32_l_e4     (fmulout32_l_ke4[55:24]),
                               .fmulout32_l_oe4    (fmulout32_l_je4[55:24]),
                               .fmulout64_e4       (fmulout64_ke4[116:53]),
                               .fmulout64_oe4      (fmulout64_je4[116:53])
                             );    // ufmlak_ff

    maia_cx_fmul ufmulj( // outputs
                         .fmul32_ex_h_e4            (fmul32_ex_h_je4[4:0]),
                         .fmul32_ex_l_e4            (fmul32_ex_l_je4[4:0]),
                         .fmul64_ex_e4              (fmul64_ex_je4[4:0]),
                         .fmul_active               (fmulj_active),
                         .fmul_f_exp_ovfl_e4        (fmul_f_exp_ovfl_je4),
                         .fmul_f_exp_ovfl_h_e4      (fmul_f_exp_ovfl_h_je4),
                         .fmul_f_exp_ovfl_l_e4      (fmul_f_exp_ovfl_l_je4),
                         .fmul_f_infnanzero_e4      (fmul_f_infnanzero_je4),
                         .fmul_f_infnanzero_h_e4    (fmul_f_infnanzero_h_je4),
                         .fmul_f_infnanzero_l_e4    (fmul_f_infnanzero_l_je4),
                         .fmul_f_prod_inf_zero_e4   (fmul_f_prod_inf_zero_je4),
                         .fmul_f_prod_inf_zero_h_e4 (fmul_f_prod_inf_zero_h_je4),
                         .fmul_f_prod_inf_zero_l_e4 (fmul_f_prod_inf_zero_l_je4),
                         .fmulout32_h_e4            (fmulout32_h_je4[55:0]),
                         .fmulout32_l_e4            (fmulout32_l_je4[55:0]),
                         .fmulout64_e4              (fmulout64_je4[116:0]),

                         // inputs
                         .ck_gclkcx_float           (ck_gclkcx_floatj),
                         .cx_ccpass_e1              (cx_ccpass_je1),
                         .cx_reset3                 (cx_reset3),
                         .cx_srca_fp_data32_e1      (cx_fmul_srca_fp_data32_je1[63:0]),
                         .cx_srca_fp_data64_e1      (cx_srca_fp_data64_je1[63:0]),
                         .cx_srcb_fp_data32_e1      (cx_fmul_srcb_fp_data32_je1[63:0]),
                         .cx_srcb_fp_data64_e1      (cx_srcb_fp_data64_je1[63:0]),
                         .cx_srcc_fp_data32_e1      (cx_srcc_fp_data32_je1[63:0]),
                         .cx_srcc_fp_data64_e1      (cx_srcc_fp_data32_je1[31:0]),
                         .cx_srcd_fp_data64_e1      (cx_srcd_fp_data32_je1[31:0]),
                         .dn_e1                     (dn_je1),
                         .fmla_fused_e1_q           (fmla_fused_je1),
                         .fmul_ext_e1               (fmul_ext_je1),
                         .fmul_negmul_e1_q          (fmul_negmul_je1),
                         .fmul_step_e1_q            (fmul_step_je1),
                         .fmul_vld_e1               (fmul_vld_je1[2:0]),
                         .fz_e1                     (fz_je1),
                         .rmode_e1_q                (rmode_je1[1:0])
                       );    // ufmulj

    maia_cx_fadd ufaddj( // outputs
                         .fadd32_ex_h_e4            (fadd32_ex_h_je4[5:0]),
                         .fadd32_ex_l_e4            (fadd32_ex_l_je4[5:0]),
                         .fadd64_ex_e4              (fadd64_ex_je4[5:0]),
                         .fadd_active               (addj_active),
                         .faddout32_h_e4            (faddout32_h_je4[31:0]),
                         .faddout32_l_e4            (faddout32_l_je4[31:0]),
                         .faddout64_e4              (faddout64_je4[63:0]),

                         // inputs
                         .ck_gclkcx_float           (ck_gclkcx_floatj),
                         .cx_reset3                 (cx_reset3),
                         .cx_srca_fp_data32_e1      (cx_fadd_srca_fp_data32_je1[63:0]),
                         .cx_srca_fp_data64_e1      (cx_fadd_srca_fp_data64_je1[63:0]),
                         .cx_srcb_fp_data32_h_e1    (cx_fadd_srcb_fp_data32_h_je1[55:0]),
                         .cx_srcb_fp_data32_l_e1    (cx_fadd_srcb_fp_data32_l_je1[55:0]),
                         .cx_srcb_fp_data64_e1      (cx_fadd_srcb_fp_data64_je1[116:0]),
                         .cx_srcc_fp_data32_e1      (cx_fadd_srcc_fp_data32_je1[63:0]),
                         .cx_srcc_fp_data64_e1      (cx_fadd_srcc_fp_data64_je1[31:0]),
                         .cx_srcd_fp_data64_e1      (cx_fadd_srcd_fp_data64_je1[31:0]),
                         .dn_e1                     (dn_fadd_je1),
                         .fadd_absin_e1_q           (fadd_absin_je1),
                         .fadd_absout_e1_q          (fadd_absout_je1),
                         .fadd_ccpass_e1            (fadd_ccpass_je1),
                         .fadd_sub_e1_q             (fadd_sub_je1),
                         .fadd_vld_e1               (fadd_vld_je1[2:0]),
                         .fmla_e4_q                 (fmla_je4),
                         .fmla_fused_e4_q           (fmla_fused_je4),
                         .fmla_negopa_e4_q          (fmla_negopa_je4),
                         .fmul32_ex_h_e4            (fmul32_ex_h_je4[4:0]),
                         .fmul32_ex_l_e4            (fmul32_ex_l_je4[4:0]),
                         .fmul64_ex_e4              (fmul64_ex_je4[4:0]),
                         .fmul_div_e4_q             (fmul_div_je4),
                         .fmul_f_exp_ovfl_e4        (fmul_f_exp_ovfl_je4),
                         .fmul_f_exp_ovfl_h_e4      (fmul_f_exp_ovfl_h_je4),
                         .fmul_f_exp_ovfl_l_e4      (fmul_f_exp_ovfl_l_je4),
                         .fmul_f_infnanzero_e4      (fmul_f_infnanzero_je4),
                         .fmul_f_infnanzero_h_e4    (fmul_f_infnanzero_h_je4),
                         .fmul_f_infnanzero_l_e4    (fmul_f_infnanzero_l_je4),
                         .fmul_f_prod_inf_zero_e4   (fmul_f_prod_inf_zero_je4),
                         .fmul_f_prod_inf_zero_h_e4 (fmul_f_prod_inf_zero_h_je4),
                         .fmul_f_prod_inf_zero_l_e4 (fmul_f_prod_inf_zero_l_je4),
                         .fz_e1                     (fz_fadd_je1),
                         .rmode_e1_q                (rmode_fadd_je1[1:0])
                       );    // ufaddj

    maia_cx_ialu uialuj( // outputs
                         .ialu_active    (ialuj_active),
                         .ialu_fpex_e3_q (ialu_fpex_je3_q[4:0]),
                         .ialu_nzcv_e3_q (unused1[3:0]),
                         .ialu_qc_e3_q   (ialu_qc_je3_q),
                         .ialu_qc_vld_e2 (ialu_qc_vld_je2),
                         .ialuout_e3_q   (ialuout_je3_q[127:0]),

                         // inputs
                         .ck_gclkcx_int  (ck_gclkcx_intj),
                         .cx_reset3      (cx_reset3),
                         .esize_e1       (ialu_esize_je1[1:0]),
                         .ialu_acc_en_e1 (1'b0),
                         .ialu_ccpass_e1 (1'b1),
                         .ialu_ctl_e1    (ialu_ctl_je1[21:0]),
                         .ialu_en_e1     (ialu_en_je1),
                         .ialu_fp_dn_e1  (ialu_fp_dn_je1),
                         .ialu_fp_fz_e1  (ialu_fp_fz_je1),
                         .ialu_nzcv_e1   (4'b0000),
                         .opa_e1_q       (cx_srca_int_data_je1[63:0]),
                         .opb_e1_q       (cx_srcb_int_data_je1[63:0]),
                         .opc_e1_q       (cx_srcc_int_data_je1[63:0]),
                         .opd_e1_q       (cx_srcd_int_data_je1[63:0]),
                         .res128_e1      (ialu_res128_je1_q),
                         .unsigned_e1    (selusgn_je1_q)
                       );    // uialuj

    maia_cx_perm upermj( // outputs
                         .lspout_e3_q           (lspout_je3_q[127:0]),

                         // inputs
                         .ck_gclkcx             (ck_gclkcx),
                         .ds_cx_aarch64_state   (ds_cx_aarch64_state),
                         .dstx_bytesel_e1       (dstx_bytesel_je[39:0]),
                         .dsty_bytesel_e1       (dsty_bytesel_je[39:0]),
                         .fpscr_e1_q            (cx_srcp_data_je1[3:0]),
                         .opa_e1_q              (cx_srca_int_data_je1[63:0]),
                         .opb_e1_q              (cx_srcb_int_data_je1[63:0]),
                         .opc_e1_q              (cx_srcc_int_data_je1[63:0]),
                         .opd_e1_q              (cx_srcd_int_data_je1[63:0]),
                         .perm_en_e1            (perm_en_je1),
                         .perm_en_e2            (perm_en_je2),
                         .perm_opa_en_e1        (perm_opa_en_je1),
                         .perm_opb_en_e1        (perm_opb_en_je1),
                         .perm_opc_en_e1        (perm_opc_en_je1),
                         .perm_opd_en_e1        (perm_opd_en_je1),
                         .perm_sign_sel0_e1     (perm_sign_sel0_je1[2:0]),
                         .perm_sign_sel1_e1     (perm_sign_sel1_je1[2:0]),
                         .perm_sign_sel2_e1     (perm_sign_sel2_je1[2:0]),
                         .perm_sign_sel3_e1     (perm_sign_sel3_je1[2:0]),
                         .perm_uen_e1           (perm_uen_je1),
                         .perm_uen_e2           (perm_uen_je2),
                         .tbl_inst_e1           (tbl_inst_je),
                         .tbltbx_qdest_e1       (tbltbx_qdest_je),
                         .tbltbx_reg_bitmask_e1 (tbltbx_reg_bitmask_je[3:0]),
                         .tbx_inst_e1           (tbx_inst_je),
                         .uopnum_e1             (uopnum_je[2:0]),
                         .vcmp_inst_e1          (vcmp_inst_je)
                       );    // upermj

    maia_cx_imac uimacj( // outputs
                         .imac_active          (imac_active),
                         .imac_qc_e4_q         (imac_qc_e4_q),
                         .imac_qc_vld_e3       (imac_qc_vld_e3),
                         .iwbout_e4_q          (iwbout_e4_q[127:0]),

                         // inputs
                         .ck_gclkcx_int        (ck_gclkcx_intj),
                         .cx_imac_cmd_e1_q     (cx_imac_cmd_e1[12:0]),
                         .cx_imac_fwd_sel_e1_q (cx_mla_fwd_sel_je1[2:0]),
                         .cx_imac_vld_e1_q     (cx_imac_vld_e1),
                         .cx_reset3            (cx_reset3),
                         .opa_e1_q             (cx_srca_int_data_je1[63:0]),
                         .opb_e1_q             (cx_srcb_int_data_je1[63:0]),
                         .opc_e1_q             (cx_srcc_int_data_je1[63:0]),
                         .opd_e1_q             (cx_srcd_int_data_je1[63:0])
                       );    // uimacj

    maia_cx_fdiv ufdivj( // outputs
                         .fdiv_active     (fdiv_active),
                         .fdiv_done       (fdiv_done[1:0]),
                         .fdivexc32_q     (fdivexc32_q[5:0]),
                         .fdivexc64_q     (fdivexc64_q[5:0]),
                         .fdivout32_q     (fdivout32_q[31:0]),
                         .fdivout64_q     (fdivout64_q[63:0]),

                         // inputs
                         .ck_gclkcx_float (ck_gclkcx_floatj),
                         .cx_reset3       (cx_reset3),
                         .dn_e1_q         (dn_je1),
                         .fdiv_ccpass_e1  (cx_ccpass_je1),
                         .fdiv_cmd_e1_q   (fdiv_cmd_e1[2:0]),
                         .fdiv_flush      (fdiv_flush[1:0]),
                         .fdiv_vld_e1     (fdiv_uop_vld_je1[1:0]),
                         .fz_e1_q         (fz_je1),
                         .opa_e1_q        (cx_srca_fp_data32_je1[63:0]),
                         .opb_e1_q        (cx_srcb_fp_data32_je1[63:0]),
                         .opc_e1_q        (cx_srcc_fp_data32_je1[31:0]),
                         .opd_e1_q        (cx_srcd_fp_data32_je1[31:0]),
                         .rmode_e1_q      (rmode_je1[1:0])
                       );    // ufdivj

    maia_cx_fcvt ufcvtj( // outputs
                         .fcvt_active        (cvtj_active),
                         .fcvt_ex_h_e3       (fcvt_ex_h_e3[5:0]),
                         .fcvt_ex_l_e3       (fcvt_ex_l_e3[5:0]),
                         .fcvtout_e3         (fcvtout_e3[127:0]),

                         // inputs
                         .ahp_mode_e1_q      (ahp_mode_e1_q),
                         .ccpass_e1          (cx_ccpass_je1),
                         .ck_gclkcx_float    (ck_gclkcx_floatj),
                         .cx_reset3          (cx_reset3),
                         .cx_srca_fp_data_e1 (cx_srca_fp_data32_je1[63:0]),
                         .cx_srcb_fp_data_e1 (cx_srcb_fp_data32_je1[63:0]),
                         .cx_srcc_fp_data_e1 (cx_srcc_fp_data32_je1[31:0]),
                         .cx_srcd_fp_data_e1 (cx_srcd_fp_data32_je1[31:0]),
                         .dn_e1_q            (dn_je1),
                         .fcvt_cvt_f_to_f_e1 (fcvt_cvt_f_to_f_je1),
                         .fcvt_cvt_f_to_i_e1 (fcvt_cvt_f_to_i_je1),
                         .fcvt_cvt_i_to_f_e1 (fcvt_cvt_i_to_f_je1),
                         .fcvt_cvts_e1       (fcvt_cvts_je1),
                         .fcvt_frint_e1      (fcvt_frint_je1),
                         .fcvt_hp_sel_top_e1 (fcvt_hp_sel_top_je1),
                         .fcvt_imm_e1        (fcvt_imm_je1),
                         .fcvt_immv_e1       (fcvt_immv_je1[5:0]),
                         .fcvt_isize_e1      (fcvt_isize_je1[1:0]),
                         .fcvt_noixc_e1      (fcvt_noixc_je1),
                         .fcvt_osize_e1      (fcvt_osize_je1[1:0]),
                         .fcvt_recpe_e1      (fcvt_recpe_je1),
                         .fcvt_recpx_e1      (fcvt_recpx_je1),
                         .fcvt_restf_e1      (fcvt_restf_je1),
                         .fcvt_rsqrte_e1     (fcvt_rsqrte_je1),
                         .fcvt_scalar_e1     (fcvt_scalar_je1),
                         .fcvt_vld_e1        (fcvt_vld_je1[1:0]),
                         .fz_e1_q            (fz_je1),
                         .rmode_e1_q         (rmode_fcvt_je1[2:0])
                       );    // ufcvtj

    maia_cx_fmul ufmulk( // outputs
                         .fmul32_ex_h_e4            (fmul32_ex_h_ke4[4:0]),
                         .fmul32_ex_l_e4            (fmul32_ex_l_ke4[4:0]),
                         .fmul64_ex_e4              (fmul64_ex_ke4[4:0]),
                         .fmul_active               (fmulk_active),
                         .fmul_f_exp_ovfl_e4        (fmul_f_exp_ovfl_ke4),
                         .fmul_f_exp_ovfl_h_e4      (fmul_f_exp_ovfl_h_ke4),
                         .fmul_f_exp_ovfl_l_e4      (fmul_f_exp_ovfl_l_ke4),
                         .fmul_f_infnanzero_e4      (fmul_f_infnanzero_ke4),
                         .fmul_f_infnanzero_h_e4    (fmul_f_infnanzero_h_ke4),
                         .fmul_f_infnanzero_l_e4    (fmul_f_infnanzero_l_ke4),
                         .fmul_f_prod_inf_zero_e4   (fmul_f_prod_inf_zero_ke4),
                         .fmul_f_prod_inf_zero_h_e4 (fmul_f_prod_inf_zero_h_ke4),
                         .fmul_f_prod_inf_zero_l_e4 (fmul_f_prod_inf_zero_l_ke4),
                         .fmulout32_h_e4            (fmulout32_h_ke4[55:0]),
                         .fmulout32_l_e4            (fmulout32_l_ke4[55:0]),
                         .fmulout64_e4              (fmulout64_ke4[116:0]),

                         // inputs
                         .ck_gclkcx_float           (ck_gclkcx_floatk),
                         .cx_ccpass_e1              (cx_ccpass_ke1),
                         .cx_reset3                 (cx_reset3),
                         .cx_srca_fp_data32_e1      (cx_fmul_srca_fp_data32_ke1[63:0]),
                         .cx_srca_fp_data64_e1      (cx_srca_fp_data64_ke1[63:0]),
                         .cx_srcb_fp_data32_e1      (cx_fmul_srcb_fp_data32_ke1[63:0]),
                         .cx_srcb_fp_data64_e1      (cx_srcb_fp_data64_ke1[63:0]),
                         .cx_srcc_fp_data32_e1      (cx_srcc_fp_data32_ke1[63:0]),
                         .cx_srcc_fp_data64_e1      (cx_srcc_fp_data32_ke1[31:0]),
                         .cx_srcd_fp_data64_e1      (cx_srcd_fp_data32_ke1[31:0]),
                         .dn_e1                     (dn_ke1),
                         .fmla_fused_e1_q           (fmla_fused_ke1),
                         .fmul_ext_e1               (fmul_ext_ke1),
                         .fmul_negmul_e1_q          (fmul_negmul_ke1),
                         .fmul_step_e1_q            (fmul_step_ke1),
                         .fmul_vld_e1               (fmul_vld_ke1[2:0]),
                         .fz_e1                     (fz_ke1),
                         .rmode_e1_q                (rmode_ke1[1:0])
                       );    // ufmulk

    maia_cx_fadd ufaddk( // outputs
                         .fadd32_ex_h_e4            (fadd32_ex_h_ke4[5:0]),
                         .fadd32_ex_l_e4            (fadd32_ex_l_ke4[5:0]),
                         .fadd64_ex_e4              (fadd64_ex_ke4[5:0]),
                         .fadd_active               (addk_active),
                         .faddout32_h_e4            (faddout32_h_ke4[31:0]),
                         .faddout32_l_e4            (faddout32_l_ke4[31:0]),
                         .faddout64_e4              (faddout64_ke4[63:0]),

                         // inputs
                         .ck_gclkcx_float           (ck_gclkcx_floatk),
                         .cx_reset3                 (cx_reset3),
                         .cx_srca_fp_data32_e1      (cx_fadd_srca_fp_data32_ke1[63:0]),
                         .cx_srca_fp_data64_e1      (cx_fadd_srca_fp_data64_ke1[63:0]),
                         .cx_srcb_fp_data32_h_e1    (cx_fadd_srcb_fp_data32_h_ke1[55:0]),
                         .cx_srcb_fp_data32_l_e1    (cx_fadd_srcb_fp_data32_l_ke1[55:0]),
                         .cx_srcb_fp_data64_e1      (cx_fadd_srcb_fp_data64_ke1[116:0]),
                         .cx_srcc_fp_data32_e1      (cx_fadd_srcc_fp_data32_ke1[63:0]),
                         .cx_srcc_fp_data64_e1      (cx_fadd_srcc_fp_data64_ke1[31:0]),
                         .cx_srcd_fp_data64_e1      (cx_fadd_srcd_fp_data64_ke1[31:0]),
                         .dn_e1                     (dn_fadd_ke1),
                         .fadd_absin_e1_q           (fadd_absin_ke1),
                         .fadd_absout_e1_q          (fadd_absout_ke1),
                         .fadd_ccpass_e1            (fadd_ccpass_ke1),
                         .fadd_sub_e1_q             (fadd_sub_ke1),
                         .fadd_vld_e1               (fadd_vld_ke1[2:0]),
                         .fmla_e4_q                 (fmla_ke4),
                         .fmla_fused_e4_q           (fmla_fused_ke4),
                         .fmla_negopa_e4_q          (fmla_negopa_ke4),
                         .fmul32_ex_h_e4            (fmul32_ex_h_ke4[4:0]),
                         .fmul32_ex_l_e4            (fmul32_ex_l_ke4[4:0]),
                         .fmul64_ex_e4              (fmul64_ex_ke4[4:0]),
                         .fmul_div_e4_q             (fmul_div_ke4),
                         .fmul_f_exp_ovfl_e4        (fmul_f_exp_ovfl_ke4),
                         .fmul_f_exp_ovfl_h_e4      (fmul_f_exp_ovfl_h_ke4),
                         .fmul_f_exp_ovfl_l_e4      (fmul_f_exp_ovfl_l_ke4),
                         .fmul_f_infnanzero_e4      (fmul_f_infnanzero_ke4),
                         .fmul_f_infnanzero_h_e4    (fmul_f_infnanzero_h_ke4),
                         .fmul_f_infnanzero_l_e4    (fmul_f_infnanzero_l_ke4),
                         .fmul_f_prod_inf_zero_e4   (fmul_f_prod_inf_zero_ke4),
                         .fmul_f_prod_inf_zero_h_e4 (fmul_f_prod_inf_zero_h_ke4),
                         .fmul_f_prod_inf_zero_l_e4 (fmul_f_prod_inf_zero_l_ke4),
                         .fz_e1                     (fz_fadd_ke1),
                         .rmode_e1_q                (rmode_fadd_ke1[1:0])
                       );    // ufaddk

    maia_cx_ialu uialuk( // outputs
                         .ialu_active    (ialuk_active),
                         .ialu_fpex_e3_q (ialu_fpex_ke3_q[4:0]),
                         .ialu_nzcv_e3_q (ialu_nzcv_ke3_q[3:0]),
                         .ialu_qc_e3_q   (ialu_qc_ke3_q),
                         .ialu_qc_vld_e2 (ialu_qc_vld_ke2),
                         .ialuout_e3_q   (ialuout_ke3_q[127:0]),

                         // inputs
                         .ck_gclkcx_int  (ck_gclkcx_intk),
                         .cx_reset3      (cx_reset3),
                         .esize_e1       (ialu_esize_ke1[1:0]),
                         .ialu_acc_en_e1 (ialu_acc_en_ke1),
                         .ialu_ccpass_e1 (cx_ccpass_ke1),
                         .ialu_ctl_e1    (ialu_ctl_ke1[21:0]),
                         .ialu_en_e1     (ialu_en_ke1),
                         .ialu_fp_dn_e1  (ialu_fp_dn_ke1),
                         .ialu_fp_fz_e1  (ialu_fp_fz_ke1),
                         .ialu_nzcv_e1   (cx_srcp_data_ke1[3:0]),
                         .opa_e1_q       (cx_srca_int_data_ke1[63:0]),
                         .opb_e1_q       (cx_srcb_int_data_ke1[63:0]),
                         .opc_e1_q       (cx_srcc_int_data_ke1[63:0]),
                         .opd_e1_q       (cx_srcd_int_data_ke1[63:0]),
                         .res128_e1      (ialu_res128_ke1_q),
                         .unsigned_e1    (selusgn_ke1_q)
                       );    // uialuk

    maia_cx_ired uiredk( // outputs
                         .ired_active   (iredk_active),
                         .iredout_e3_q  (iredout_ke2[63:0]),

                         // inputs
                         .ck_gclkcx_int (ck_gclkcx_intk),
                         .cx_reset3     (cx_reset3),
                         .esize_e1      (ired_esize_ke1[1:0]),
                         .long_e1       (ired_long_ke1),
                         .opa_e1_q      (cx_srca_int_data_ke1[63:0]),
                         .opb_e1_q      (cx_srcb_int_data_ke1[63:0]),
                         .opb_en_e1     (ired_opb_en_ke1),
                         .seladd_e1     (ired_seladd_ke1),
                         .selmax_e1     (ired_selmax_ke1),
                         .selmin_e1     (ired_selmin_ke1),
                         .unsigned_e1   (ired_selusgn_ke1),
                         .vrop_e1       (ired_vrop_ke1)
                       );    // uiredk

    maia_cx_perm upermk( // outputs
                         .lspout_e3_q           (lspout_ke3_q[127:0]),

                         // inputs
                         .ck_gclkcx             (ck_gclkcx),
                         .ds_cx_aarch64_state   (ds_cx_aarch64_state),
                         .dstx_bytesel_e1       (dstx_bytesel_ke[39:0]),
                         .dsty_bytesel_e1       (dsty_bytesel_ke[39:0]),
                         .fpscr_e1_q            (cx_srcp_data_ke1[3:0]),
                         .opa_e1_q              (cx_srca_int_data_ke1[63:0]),
                         .opb_e1_q              (cx_srcb_int_data_ke1[63:0]),
                         .opc_e1_q              (cx_srcc_int_data_ke1[63:0]),
                         .opd_e1_q              (cx_srcd_int_data_ke1[63:0]),
                         .perm_en_e1            (perm_en_ke1),
                         .perm_en_e2            (perm_en_ke2),
                         .perm_opa_en_e1        (perm_opa_en_ke1),
                         .perm_opb_en_e1        (perm_opb_en_ke1),
                         .perm_opc_en_e1        (perm_opc_en_ke1),
                         .perm_opd_en_e1        (perm_opd_en_ke1),
                         .perm_sign_sel0_e1     (perm_sign_sel0_ke1[2:0]),
                         .perm_sign_sel1_e1     (perm_sign_sel1_ke1[2:0]),
                         .perm_sign_sel2_e1     (perm_sign_sel2_ke1[2:0]),
                         .perm_sign_sel3_e1     (perm_sign_sel3_ke1[2:0]),
                         .perm_uen_e1           (perm_uen_ke1),
                         .perm_uen_e2           (perm_uen_ke2),
                         .tbl_inst_e1           (tbl_inst_ke),
                         .tbltbx_qdest_e1       (tbltbx_qdest_ke),
                         .tbltbx_reg_bitmask_e1 (tbltbx_reg_bitmask_ke[3:0]),
                         .tbx_inst_e1           (tbx_inst_ke),
                         .uopnum_e1             (uopnum_ke[2:0]),
                         .vcmp_inst_e1          (vcmp_inst_ke)
                       );    // upermk

    maia_cx_ishf uishfk( // outputs
                         .ishf_active        (ishf_active),
                         .ishf_qc_e4_q       (ishf_qc_e4_q),
                         .ishfaccout_e4_q    (ishfaccout_e4_q[127:0]),
                         .ishfout_e3_q       (ishfout_e3_q[127:0]),

                         // inputs
                         .acc_size_eq64_e3_q (acc_size_eq64_e3_q),
                         .acc_size_ge32_e3_q (acc_size_ge32_e3_q),
                         .acc_size_ne08_e3_q (acc_size_ne08_e3_q),
                         .c00_x_sel_e1_q     (c00_x_sel_e1[3:0]),
                         .c00_y_sel_e1_q     (c00_y_sel_e1),
                         .c01_x_sel_e1_q     (c01_x_sel_e1[5:0]),
                         .c01_y_sel_e1_q     (c01_y_sel_e1[2:0]),
                         .c02_x_sel_e1_q     (c02_x_sel_e1[5:0]),
                         .c02_y_sel_e1_q     (c02_y_sel_e1[2:0]),
                         .c03_x_sel_e1_q     (c03_x_sel_e1[6:0]),
                         .c03_y_sel_e1_q     (c03_y_sel_e1[4:0]),
                         .c04_x_sel_e1_q     (c04_x_sel_e1[5:0]),
                         .c04_y_sel_e1_q     (c04_y_sel_e1[2:0]),
                         .c05_x_sel_e1_q     (c05_x_sel_e1[7:0]),
                         .c05_y_sel_e1_q     (c05_y_sel_e1[4:0]),
                         .c06_x_sel_e1_q     (c06_x_sel_e1[5:0]),
                         .c06_y_sel_e1_q     (c06_y_sel_e1[4:0]),
                         .c07_x_sel_e1_q     (c07_x_sel_e1[6:0]),
                         .c07_y_sel_e1_q     (c07_y_sel_e1[4:0]),
                         .c08_x_sel_e1_q     (c08_x_sel_e1[6:0]),
                         .c08_y_sel_e1_q     (c08_y_sel_e1[2:0]),
                         .c09_x_sel_e1_q     (c09_x_sel_e1[8:0]),
                         .c09_y_sel_e1_q     (c09_y_sel_e1[5:0]),
                         .c10_x_sel_e1_q     (c10_x_sel_e1[8:0]),
                         .c10_y_sel_e1_q     (c10_y_sel_e1[5:0]),
                         .c11_x_sel_e1_q     (c11_x_sel_e1[9:0]),
                         .c11_y_sel_e1_q     (c11_y_sel_e1[7:0]),
                         .c12_x_sel_e1_q     (c12_x_sel_e1[7:0]),
                         .c12_y_sel_e1_q     (c12_y_sel_e1[5:0]),
                         .c13_x_sel_e1_q     (c13_x_sel_e1[9:0]),
                         .c13_y_sel_e1_q     (c13_y_sel_e1[6:0]),
                         .c14_x_sel_e1_q     (c14_x_sel_e1[7:0]),
                         .c14_y_sel_e1_q     (c14_y_sel_e1[6:0]),
                         .c15_x_sel_e1_q     (c15_x_sel_e1[7:0]),
                         .c15_y_sel_e1_q     (c15_y_sel_e1[6:0]),
                         .ck_gclkcx_int      (ck_gclkcx_intk),
                         .cx_reset3          (cx_reset3),
                         .iacc_cin_sel_e3_q  (iacc_cin_sel_e3_q[7:1]),
                         .iacc_en_e2         (iacc_en_e2),
                         .iacc_shfsel_e2     (iacc_shfsel_e2),
                         .iacce4_fwd_e2      (iacce4_fwd_e2),
                         .ialuout_e3_q       (ialuout_ke3_q[127:0]),
                         .ishf3_iss_e2_q     (ishf3_iss_e2),
                         .ishf3_iss_e3_q     (ishf3_iss_e3),
                         .ishf3_uiss_e2_q    (ishf3_uiss_e2),
                         .ishf_imm_e1_q      (ishf_imm_e1),
                         .ishf_immv_e1_q     (ishf_immv_e1[7:0]),
                         .ishf_insert_e1_q   (ishf_insert_e1),
                         .ishf_iss_e1_q      (ishf_iss_e1),
                         .ishf_iss_e2_q      (ishf_iss_e2),
                         .ishf_left_e1_q     (ishf_left_e1),
                         .ishf_narrow_e1_q   (ishf_narrow_e1),
                         .ishf_round_e1_q    (ishf_round_e1),
                         .ishf_s1_sel_e1_q   (ishf_s1_sel_e1[2:0]),
                         .ishf_s2_sel_e1_q   (ishf_s2_sel_e1[2:0]),
                         .ishf_s3_sel_e1_q   (ishf_s3_sel_e1[3:0]),
                         .ishf_s4_sel_e1_q   (ishf_s4_sel_e1[2:0]),
                         .ishf_s5_sel_e1_q   (ishf_s5_sel_e1[3:0]),
                         .ishf_s6_sel_e1_q   (ishf_s6_sel_e1[3:0]),
                         .ishf_s7_sel_e1_q   (ishf_s7_sel_e1[4:0]),
                         .ishf_saturate_e1_q (ishf_saturate_e1),
                         .ishf_scalar_e1     (ishf_scalar_e1),
                         .ishf_sel16_e1_q    (ishf_sel16_e1),
                         .ishf_sel32_e1_q    (ishf_sel32_e1),
                         .ishf_sel64_e1_q    (ishf_sel64_e1),
                         .ishf_sel8_e1_q     (ishf_sel8_e1),
                         .ishf_selqsat_e3_q  (ishf_selqsat_e3),
                         .ishf_signed_e1_q   (ishf_signed_e1),
                         .ishf_stous_e1_q    (ishf_stous_e1),
                         .ishf_uiss_e2_q     (ishf_uiss_e2),
                         .ishf_widen_e1_q    (ishf_widen_e1),
                         .opa_e1_q           (cx_srca_int_data_ke1[63:0]),
                         .opb_e1_q           (cx_srcb_int_data_ke1[63:0]),
                         .opc_e3_q           (cx_srcc_data_ke3[63:0]),
                         .opd_e3_q           (cx_srcd_data_ke3[63:0]),
                         .shf_size_eq64_e3_q (shf_size_eq64_e3_q),
                         .shf_size_ge32_e3_q (shf_size_ge32_e3_q),
                         .shf_size_ne08_e3_q (shf_size_ne08_e3_q)
                       );    // uishfk

    maia_cx_crypt2 ucrypt2( // outputs
                            .crypt2_active   (crypt2_active),
                            .crypt2_out_e3_q (crypt2_out_e3_q[127:0]),

                            // inputs
                            .aesd_e1_q       (aesd_e1),
                            .aesdimc_e1_q    (aesdimc_e1),
                            .aese_e1_q       (aese_e1),
                            .aesemc_e1_q     (aesemc_e1),
                            .aesimc_e1_q     (aesimc_e1),
                            .aesmc_e1_q      (aesmc_e1),
                            .ck_gclkcx_crypt (ck_gclkcx_crypt),
                            .cx_reset3       (cx_reset3),
                            .ival_e1_q       (crypt2_vld_e1),
                            .pmull_e1_q      (pmull_e1),
                            .qd              ({cx_srcd_crypt_data_je1[63:0], cx_srcc_crypt_data_je1[63:0]}),
                            .qn              ({cx_srcb_crypt_data_je1[63:0], cx_srca_crypt_data_je1[63:0]}),
                            .sha1h_e1_q      (sha1h_e1),
                            .sha1su1_e1_q    (sha1su1_e1),
                            .sha256su0_e1_q  (sha256su0_e1)
                          );    // ucrypt2

    maia_cx_crypt3 ucrypt3( // outputs
                            .crypt3_active   (crypt3_active),
                            .crypt3_out_e6_q (crypt3_out_e6_q[127:0]),

                            // inputs
                            .ck_gclkcx_crypt (ck_gclkcx_crypt),
                            .cx_reset3       (cx_reset3),
                            .ival_e1_q       (crypt3_vld0_e1),
                            .ival_e2_q       (crypt3_vld1_e1),
                            .qd_e2_q         ({cx_srcb_crypt_data_je1[63:0], cx_srca_crypt_data_je1[63:0]}),
                            .qm_e1_q         ({cx_srcd_crypt_data_je1[63:0], cx_srcc_crypt_data_je1[63:0]}),
                            .qn_e1_q         ({cx_srcb_crypt_data_je1[63:0], cx_srca_crypt_data_je1[63:0]}),
                            .sha1c_e1_q      (sha1c_e1),
                            .sha1m_e1_q      (sha1m_e1),
                            .sha1p_e1_q      (sha1p_e1),
                            .sha256h2_e1_q   (sha256h2_e1),
                            .sha256h_e1_q    (sha256h_e1),
                            .sha256su1_e1_q  (sha256su1_e1)
                          );    // ucrypt3

    maia_cx_fsqrt ufsqrtk( // outputs
                           .fsqrt_active    (fsqrt_active),
                           .fsqrt_done      (fsqrt_done[1:0]),
                           .fsqrtexc32_q    (fsqrtexc32_q[5:0]),
                           .fsqrtexc64_q    (fsqrtexc64_q[5:0]),
                           .fsqrtout32_q    (fsqrtout32_q[31:0]),
                           .fsqrtout64_q    (fsqrtout64_q[63:0]),

                           // inputs
                           .ck_gclkcx_float (ck_gclkcx_floatk),
                           .cx_reset3       (cx_reset3),
                           .dn_e1_q         (dn_ke1),
                           .fsqrt_ccpass_e1 (cx_ccpass_ke1),
                           .fsqrt_cmd_e1_q  (fsqrt_cmd_e1[2:0]),
                           .fsqrt_flush     (fsqrt_flush[1:0]),
                           .fsqrt_vld_e1    (fsqrt_uop_vld_ke1[1:0]),
                           .fz_e1_q         (fz_ke1),
                           .opa_e1_q        (cx_srca_fp_data32_ke1[63:0]),
                           .opc_e1_q        (cx_srcc_fp_data32_ke1[31:0]),
                           .opd_e1_q        (cx_srcd_fp_data32_ke1[31:0]),
                           .rmode_e1_q      (rmode_ke1[1:0])
                         );    // ufsqrtk
endmodule    // maia_complex


//ARMAUTO UNDEF START
`define MAIA_UNDEFINE
`include "maia_header.v"
`undef MAIA_UNDEFINE
//ARMAUTO UNDEF END
