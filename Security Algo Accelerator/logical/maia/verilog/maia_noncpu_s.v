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
//     Filename            : $RCSfile: maia_noncpu.v $
//     Checked In          : $Date: 2015-05-06 10:47:09 -0500 (Wed, 06 May 2015) $
//     Revision            : $Revision: 73443 $
//     Release Information : Cortex-A72-r1p0-00rel0
//
//-----------------------------------------------------------------------------
// Verilog-2001 (IEEE Std 1364-2001)
//-----------------------------------------------------------------------------

//#
//# Overview
//# ========
//#

//
// This is top-level interconnect layer for the non-CPU blocks at the Maia top-level.
//

//#
//# Module Declaration
//# ==================
//#

`include "maia_header.v"

`define MAIA_CN 3

module maia_noncpu_s (
                  CLK,
                  CLKEN,
                  nCPUPORESET,
                  nCORERESET,
                  nL2RESET,
                  L2RSTDISABLE,
                  WARMRSTREQ,
                  CFGEND,
                  VINITHI,
                  CFGTE,
                  CP15SDISABLE,
                  CLUSTERIDAFF1,
                  CLUSTERIDAFF2,
                  AA64nAA32,
                  RVBARADDR0,
// BEGIN INCLUDE FOR CPU1
                  RVBARADDR1,
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
                  RVBARADDR2,
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
                  RVBARADDR3,
// END INCLUDE FOR CPU3
                  CRYPTODISABLE,
                  nFIQ,
                  nIRQ,
                  nSEI,
                  nREI,
                  nVFIQ,
                  nVIRQ,
                  nVSEI,
// BEGIN NO-GIC pins
                  nVCPUMNTIRQ,
// END NO-GIC pins
                  PERIPHBASE,
// BEGIN NO-GIC pins
                  GICCDISABLE,
                  ICDTVALID,
                  ICDTREADY,
                  ICDTDATA,
                  ICDTLAST,
                  ICDTDEST,
                  ICCTVALID,
                  ICCTREADY,
                  ICCTDATA,
                  ICCTLAST,
                  ICCTID,
// END NO-GIC pins
                  CNTVALUEB,
                  CNTCLKEN,
                  nCNTPNSIRQ,
                  nCNTPSIRQ,
                  nCNTVIRQ,
                  nCNTHPIRQ,
                  CLREXMONREQ,
                  CLREXMONACK,
                  EVENTI,
                  EVENTO,
                  STANDBYWFI,
                  STANDBYWFE,
                  STANDBYWFIL2,
                  SMPEN,
                  CPUQACTIVE,
                  CPUQREQn,
                  CPUQACCEPTn,
                  CPUQDENY,
                  L2QACTIVE,
                  L2QREQn,
                  L2QACCEPTn,
                  L2QDENY,
                  L2FLUSHREQ,
                  L2FLUSHDONE,
                  nINTERRIRQ,
                  nEXTERRIRQ,
                  SYSBARDISABLE,
                  BROADCASTINNER,
                  BROADCASTOUTER,
                  BROADCASTCACHEMAINT,
                  SCLKEN,
                  SINACT,
                  NODEID,
                  TXSACTIVE,
                  RXSACTIVE,
                  TXLINKACTIVEREQ,
                  TXLINKACTIVEACK,
                  RXLINKACTIVEREQ,
                  RXLINKACTIVEACK,
                  TXREQFLITPEND,
                  TXREQFLITV,
                  TXREQFLIT,
                  REQMEMATTR,
                  TXREQLCRDV,
                  TXRSPFLITPEND,
                  TXRSPFLITV,
                  TXRSPFLIT,
                  TXRSPLCRDV,
                  TXDATFLITPEND,
                  TXDATFLITV,
                  TXDATFLIT,
                  TXDATLCRDV,
                  RXSNPFLITPEND,
                  RXSNPFLITV,
                  RXSNPFLIT,
                  RXSNPLCRDV,
                  RXRSPFLITPEND,
                  RXRSPFLITV,
                  RXRSPFLIT,
                  RXRSPLCRDV,
                  RXDATFLITPEND,
                  RXDATFLITV,
                  RXDATFLIT,
                  RXDATLCRDV,
                  SAMMNBASE,
                  SAMADDRMAP0,
                  SAMADDRMAP1,
                  SAMADDRMAP2,
                  SAMADDRMAP3,
                  SAMADDRMAP4,
                  SAMADDRMAP5,
                  SAMADDRMAP6,
                  SAMADDRMAP7,
                  SAMADDRMAP8,
                  SAMADDRMAP9,
                  SAMADDRMAP10,
                  SAMADDRMAP11,
                  SAMADDRMAP12,
                  SAMADDRMAP13,
                  SAMADDRMAP14,
                  SAMADDRMAP15,
                  SAMADDRMAP16,
                  SAMADDRMAP17,
                  SAMADDRMAP18,
                  SAMADDRMAP19,
                  SAMMNNODEID,
                  SAMHNI0NODEID,
                  SAMHNI1NODEID,
                  SAMHNF0NODEID,
                  SAMHNF1NODEID,
                  SAMHNF2NODEID,
                  SAMHNF3NODEID,
                  SAMHNF4NODEID,
                  SAMHNF5NODEID,
                  SAMHNF6NODEID,
                  SAMHNF7NODEID,
                  SAMHNFMODE,
// BEGIN NO-ACP pins
                  ACLKENS,
                  AINACTS,
                  AWREADYS,
                  AWVALIDS,
                  AWIDS,
                  AWADDRS,
                  AWLENS,
                  AWCACHES,
                  AWUSERS,
                  AWPROTS,
                  WREADYS,
                  WVALIDS,
                  WDATAS,
                  WSTRBS,
                  WLASTS,
                  BREADYS,
                  BVALIDS,
                  BIDS,
                  BRESPS,
                  ARREADYS,
                  ARVALIDS,
                  ARIDS,
                  ARADDRS,
                  ARLENS,
                  ARCACHES,
                  ARUSERS,
                  ARPROTS,
                  RREADYS,
                  RVALIDS,
                  RIDS,
                  RDATAS,
                  RRESPS,
                  RLASTS,
// END NO-ACP pins
                  DBGROMADDR,
                  DBGROMADDRV,
                  DBGACK,
                  nCOMMIRQ,
                  COMMRX,
                  COMMTX,
                  DBGRSTREQ,
                  DBGNOPWRDWN,
                  DBGL1RSTDISABLE,
                  nPMUIRQ,
                  PMUEVENT0,
// BEGIN INCLUDE FOR CPU1
                  PMUEVENT1,
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
                  PMUEVENT2,
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
                  PMUEVENT3,
// END INCLUDE FOR CPU3
                  ATCLKEN,
                  TSVALUEB,
                  ATREADYM0,
                  AFVALIDM0,
                  ATDATAM0,
                  ATVALIDM0,
                  ATBYTESM0,
                  AFREADYM0,
                  ATIDM0,
                  SYNCREQM0,
// BEGIN INCLUDE FOR CPU1
                  ATREADYM1,
                  AFVALIDM1,
                  ATDATAM1,
                  ATVALIDM1,
                  ATBYTESM1,
                  AFREADYM1,
                  ATIDM1,
                  SYNCREQM1,
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
                  ATREADYM2,
                  AFVALIDM2,
                  ATDATAM2,
                  ATVALIDM2,
                  ATBYTESM2,
                  AFREADYM2,
                  ATIDM2,
                  SYNCREQM2,
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
                  ATREADYM3,
                  AFVALIDM3,
                  ATDATAM3,
                  ATVALIDM3,
                  ATBYTESM3,
                  AFREADYM3,
                  ATIDM3,
                  SYNCREQM3,
// END INCLUDE FOR CPU3
                  PCLKDBG,
                  PCLKENDBG,
                  nPRESETDBG,
                  PSELDBG,
                  PADDRDBG,
                  PADDRDBG31,
                  PENABLEDBG,
                  PWRITEDBG,
                  PWDATADBG,
                  PRDATADBG,
                  PREADYDBG,
                  PSLVERRDBG,
                  EDBGRQ,
                  PMUSNAPSHOTREQ, 
                  PMUSNAPSHOTACK, 
                  DBGPWRDUP,
                  DBGPWRUPREQ,
                  CTICHIN,
                  CTICHOUTACK,
                  CTICHOUT,
                  CTICHINACK,
                  CISBYPASS,
                  CIHSBYPASS,
                  CTIIRQ,
                  CTIIRQACK,
                  DBGEN,
                  NIDEN,
                  SPIDEN,
                  SPNIDEN,
                  DFTSE,
                  DFTRSTDISABLE,
                  DFTCRCLKDISABLE,
                  DFTL2CLKDISABLE,
                  DFTRAMHOLD,
                  DFTCLKBYPASS,
                  DFTMCPHOLD,
                  nMBISTRESET,
                  MBISTREQ,

//-----------------------------------------------------------------------------
// Signals from maia -> maia_cpu_io -> maia_cpu
//-----------------------------------------------------------------------------
// Outputs to maia_cpu
                  ncpuporeset_cpu0_o,
                  ncorereset_cpu0_o,

                  cfgend_cpu0_o,
                  cfgte_cpu0_o,
                  cp15sdisable_cpu0_o,
                  vinithi_cpu0_o,
                  clusteridaff1_cpu0_o,
                  clusteridaff2_cpu0_o,
                  cpuid_cpu0_o,
                  aa64naa32_cpu0_o,
                  rvbaraddr_cpu0_o,
                  cryptodisable_cpu0_o,
                  giccdisable_cpu0_o,

                  dbgromaddr_cpu0_o,
                  dbgromaddrv_cpu0_o,
                  dbgl1rstdisable_cpu0_o,

                  dbgen_cpu0_o,
                  niden_cpu0_o,
                  spiden_cpu0_o,
                  spniden_cpu0_o,

                  tsvalueb_cpu0_o,
 
                  atclken_cpu0_o,
                  afvalidm_cpu0_o,
                  atreadym_cpu0_o,
                  syncreqm_cpu0_o,

                  dftse_cpu0_o,
                  dftrstdisable_cpu0_o,
                  dftcrclkdisable_cpu0_o,
                  dftramhold_cpu0_o,

                  nmbistreset_cpu0_o,

// BEGIN INCLUDE FOR CPU1
                  ncpuporeset_cpu1_o,
                  ncorereset_cpu1_o,

                  cfgend_cpu1_o,
                  cfgte_cpu1_o,
                  cp15sdisable_cpu1_o,
                  vinithi_cpu1_o,
                  clusteridaff1_cpu1_o,
                  clusteridaff2_cpu1_o,
                  cpuid_cpu1_o,
                  aa64naa32_cpu1_o,
                  rvbaraddr_cpu1_o,
                  cryptodisable_cpu1_o,
                  giccdisable_cpu1_o,

                  dbgromaddr_cpu1_o,
                  dbgromaddrv_cpu1_o,
                  dbgl1rstdisable_cpu1_o,

                  dbgen_cpu1_o,
                  niden_cpu1_o,
                  spiden_cpu1_o,
                  spniden_cpu1_o,

                  tsvalueb_cpu1_o,
 
                  atclken_cpu1_o,
                  afvalidm_cpu1_o,
                  atreadym_cpu1_o,
                  syncreqm_cpu1_o,

                  dftse_cpu1_o,
                  dftrstdisable_cpu1_o,
                  dftcrclkdisable_cpu1_o,
                  dftramhold_cpu1_o,

                  nmbistreset_cpu1_o,
// END INCLUDE FOR CPU1

// BEGIN INCLUDE FOR CPU2
                  ncpuporeset_cpu2_o,
                  ncorereset_cpu2_o,

                  cfgend_cpu2_o,
                  cfgte_cpu2_o,
                  cp15sdisable_cpu2_o,
                  vinithi_cpu2_o,
                  clusteridaff1_cpu2_o,
                  clusteridaff2_cpu2_o,
                  cpuid_cpu2_o,
                  aa64naa32_cpu2_o,
                  rvbaraddr_cpu2_o,
                  cryptodisable_cpu2_o,
                  giccdisable_cpu2_o,

                  dbgromaddr_cpu2_o,
                  dbgromaddrv_cpu2_o,
                  dbgl1rstdisable_cpu2_o,

                  dbgen_cpu2_o,
                  niden_cpu2_o,
                  spiden_cpu2_o,
                  spniden_cpu2_o,

                  tsvalueb_cpu2_o,
 
                  atclken_cpu2_o,
                  afvalidm_cpu2_o,
                  atreadym_cpu2_o,
                  syncreqm_cpu2_o,

                  dftse_cpu2_o,
                  dftrstdisable_cpu2_o,
                  dftcrclkdisable_cpu2_o,
                  dftramhold_cpu2_o,

                  nmbistreset_cpu2_o,
// END INCLUDE FOR CPU2

// BEGIN INCLUDE FOR CPU3
                  ncpuporeset_cpu3_o,
                  ncorereset_cpu3_o,

                  cfgend_cpu3_o,
                  cfgte_cpu3_o,
                  cp15sdisable_cpu3_o,
                  vinithi_cpu3_o,
                  clusteridaff1_cpu3_o,
                  clusteridaff2_cpu3_o,
                  cpuid_cpu3_o,
                  aa64naa32_cpu3_o,
                  rvbaraddr_cpu3_o,
                  cryptodisable_cpu3_o,
                  giccdisable_cpu3_o,

                  dbgromaddr_cpu3_o,
                  dbgromaddrv_cpu3_o,
                  dbgl1rstdisable_cpu3_o,

                  dbgen_cpu3_o,
                  niden_cpu3_o,
                  spiden_cpu3_o,
                  spniden_cpu3_o,

                  tsvalueb_cpu3_o,
 
                  atclken_cpu3_o,
                  afvalidm_cpu3_o,
                  atreadym_cpu3_o,
                  syncreqm_cpu3_o,

                  dftse_cpu3_o,
                  dftrstdisable_cpu3_o,
                  dftcrclkdisable_cpu3_o,
                  dftramhold_cpu3_o,

                  nmbistreset_cpu3_o,
// END INCLUDE FOR CPU3

//-----------------------------------------------------------------------------
// Signals from maia_cpu -> maia_cpu_io -> maia
//-----------------------------------------------------------------------------
// Inputs from maia_cpu
                  ds_cpu0_sev_req,
                  ds_cpu0_sevl_req,
                  ds_cpu0_cpuectlr_smp,

                  ncommirq_cpu0_i,
                  commrx_cpu0_i,
                  commtx_cpu0_i,
                  dbgack_cpu0_i,
                  dbgrstreq_cpu0_i,
                  dbgnopwrdwn_cpu0_i,

                  npmuirq_cpu0_i,
                  pmuevent_cpu0_i,
                  pm_export_cpu0_i,

                  etclken_cpu0_i,
                  afreadym_cpu0_i,
                  atbytesm_cpu0_i,
                  atdatam_cpu0_i,
                  atidm_cpu0_i,
                  atvalidm_cpu0_i,

// BEGIN INCLUDE FOR CPU1
                  ds_cpu1_sev_req,
                  ds_cpu1_sevl_req,
                  ds_cpu1_cpuectlr_smp,

                  ncommirq_cpu1_i,
                  commrx_cpu1_i,
                  commtx_cpu1_i,
                  dbgack_cpu1_i,
                  dbgrstreq_cpu1_i,
                  dbgnopwrdwn_cpu1_i,

                  npmuirq_cpu1_i,
                  pmuevent_cpu1_i,
                  pm_export_cpu1_i,

                  etclken_cpu1_i,
                  afreadym_cpu1_i,
                  atbytesm_cpu1_i,
                  atdatam_cpu1_i,
                  atidm_cpu1_i,
                  atvalidm_cpu1_i,
// END INCLUDE FOR CPU1

// BEGIN INCLUDE FOR CPU2
                  ds_cpu2_sev_req,
                  ds_cpu2_sevl_req,
                  ds_cpu2_cpuectlr_smp,

                  ncommirq_cpu2_i,
                  commrx_cpu2_i,
                  commtx_cpu2_i,
                  dbgack_cpu2_i,
                  dbgrstreq_cpu2_i,
                  dbgnopwrdwn_cpu2_i,

                  npmuirq_cpu2_i,
                  pmuevent_cpu2_i,
                  pm_export_cpu2_i,

                  etclken_cpu2_i,
                  afreadym_cpu2_i,
                  atbytesm_cpu2_i,
                  atdatam_cpu2_i,
                  atidm_cpu2_i,
                  atvalidm_cpu2_i,
// END INCLUDE FOR CPU2

// BEGIN INCLUDE FOR CPU3
                  ds_cpu3_sev_req,
                  ds_cpu3_sevl_req,
                  ds_cpu3_cpuectlr_smp,

                  ncommirq_cpu3_i,
                  commrx_cpu3_i,
                  commtx_cpu3_i,
                  dbgack_cpu3_i,
                  dbgrstreq_cpu3_i,
                  dbgnopwrdwn_cpu3_i,

                  npmuirq_cpu3_i,
                  pmuevent_cpu3_i,
                  pm_export_cpu3_i,

                  etclken_cpu3_i,
                  afreadym_cpu3_i,
                  atbytesm_cpu3_i,
                  atdatam_cpu3_i,
                  atidm_cpu3_i,
                  atvalidm_cpu3_i,
// END INCLUDE FOR CPU3

//-----------------------------------------------------------------------------
// L2 interface
//-----------------------------------------------------------------------------
                  l2_cpu0_mbist1_addr_b1,
                  l2_cpu0_mbist1_array_b1,
                  l2_cpu0_mbist1_be_b1,
                  l2_cpu0_mbist1_en_b1,
                  l2_cpu0_mbist1_rd_en_b1,
                  l2_cpu0_mbist1_wr_en_b1,
                  l2_cpu0_mbist1_all_b1,
// BEGIN INCLUDE FOR CPU1
                  l2_cpu1_mbist1_addr_b1,
                  l2_cpu1_mbist1_array_b1,
                  l2_cpu1_mbist1_be_b1,
                  l2_cpu1_mbist1_en_b1,
                  l2_cpu1_mbist1_rd_en_b1,
                  l2_cpu1_mbist1_wr_en_b1,
                  l2_cpu1_mbist1_all_b1,
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
                  l2_cpu2_mbist1_addr_b1,
                  l2_cpu2_mbist1_array_b1,
                  l2_cpu2_mbist1_be_b1,
                  l2_cpu2_mbist1_en_b1,
                  l2_cpu2_mbist1_rd_en_b1,
                  l2_cpu2_mbist1_wr_en_b1,
                  l2_cpu2_mbist1_all_b1,
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
                  l2_cpu3_mbist1_addr_b1,
                  l2_cpu3_mbist1_array_b1,
                  l2_cpu3_mbist1_be_b1,
                  l2_cpu3_mbist1_en_b1,
                  l2_cpu3_mbist1_rd_en_b1,
                  l2_cpu3_mbist1_wr_en_b1,
                  l2_cpu3_mbist1_all_b1,
// END INCLUDE FOR CPU3

// BEGIN L2-CPU interface

//-----------------------------------------------------------------------------
// ucpu0
//-----------------------------------------------------------------------------
                  l2_cpu0_cfg_ecc_en,
                  l2_cpu0_arb_thrshld_timeout_en,
                  l2_cpu0_disable_clean_evict_opt,
                  l2_cpu0_dext_err_r2,
                  l2_cpu0_dext_err_type_r2,
                  l2_cpu0_dsngl_ecc_err_r3,
                  l2_cpu0_ddbl_ecc_err_r3,
                  l2_cpu0_ddata_r2,
                  l2_cpu0_barrier_done,
                  l2_cpu0_spec_valid,
                  l2_cpu0_spec_bufid,
                  l2_cpu0_rvalid,
                  l2_cpu0_rstate,
                  l2_cpu0_rexfail,
                  l2_cpu0_rbufid,
                  l2_cpu0_dvalid_r1,
                  l2_cpu0_dlast_r1,
                  l2_cpu0_dbufid_r1,
                  l2_cpu0_iext_err_r2,
                  l2_cpu0_iext_err_type_r2,
                  l2_cpu0_isngl_ecc_err_r3,
                  l2_cpu0_idbl_ecc_err_r3,
                  l2_cpu0_idata_r2,
                  l2_cpu0_ivalid_r1,
                  l2_cpu0_ibufid_r1,
                  l2_cpu0_ls_sync_req,
                  l2_cpu0_ccb_req_addr_c3,
                  l2_cpu0_ccb_dbg_req_c3,
                  l2_cpu0_ls_ccb_clken_c3,
                  l2_cpu0_ls_ccb_req_c3,
                  l2_cpu0_ccb_req_id_c3,
                  l2_cpu0_ccb_req_type_c3,
                  l2_cpu0_ccb_req_info_c3,
                  l2_cpu0_if_ccb_clken_c3,
                  l2_cpu0_if_ccb_req_c3,
                  l2_cpu0_if_sync_req,
                  l2_cpu0_tlb_ccb_clken_c3,
                  l2_cpu0_tlb_ccb_req_c3,
                  l2_cpu0_tlb_sync_req,
                  l2_cpu0_tlb_sync_complete,
                  l2_cpu0_tbw_desc_vld,
                  l2_cpu0_tbw_ext_err,
                  l2_cpu0_tbw_ext_err_type,
                  l2_cpu0_tbw_dbl_ecc_err,
                  l2_cpu0_tbw_desc_data,
                  l2_cpu0_spr_rd_data,
                  l2_cpu0_l2_cache_size,
                  l2_cpu0_pf_throttle_q,

                  l2_cpu0_wr_ex_resp,
                  l2_cpu0_wr_ex_fail,

                  l2_cpu0_ic_base,
                  l2_cpu0_no_intctrl,


                  l2_cpu0_pmu_events,

                  ds_cpu0_l2_spr_en,
                  ds_cpu0_l2_spr_rd,
                  ds_cpu0_l2_spr_wr,
                  ds_cpu0_l2_spr_addr,
                  ds_cpu0_l2_spr_dw,
                  ds_cpu0_l2_spr_wr_data,

                  l2_cpu0_wr_data_vld_x1_q,
                  l2_cpu0_wr_evict_x1_q,
                  l2_cpu0_wr_data,
                  l2_cpu0_ls_rd_haz_vld_arb_q,
                  l2_cpu0_ls_wr_haz_vld_arb_q,
                  l2_cpu0_dt_pmu_evt_en,

   
//-----------------------------------------------------------------------------
// ucpu1
//-----------------------------------------------------------------------------
                  l2_cpu1_cfg_ecc_en,
                  l2_cpu1_arb_thrshld_timeout_en,
                  l2_cpu1_disable_clean_evict_opt,
                  l2_cpu1_dext_err_r2,
                  l2_cpu1_dext_err_type_r2,
                  l2_cpu1_dsngl_ecc_err_r3,
                  l2_cpu1_ddbl_ecc_err_r3,
                  l2_cpu1_ddata_r2,
                  l2_cpu1_barrier_done,
                  l2_cpu1_spec_valid,
                  l2_cpu1_spec_bufid,
                  l2_cpu1_rvalid,
                  l2_cpu1_rstate,
                  l2_cpu1_rexfail,
                  l2_cpu1_rbufid,
                  l2_cpu1_dvalid_r1,
                  l2_cpu1_dlast_r1,
                  l2_cpu1_dbufid_r1,
                  l2_cpu1_iext_err_r2,
                  l2_cpu1_iext_err_type_r2,
                  l2_cpu1_isngl_ecc_err_r3,
                  l2_cpu1_idbl_ecc_err_r3,
                  l2_cpu1_idata_r2,
                  l2_cpu1_ivalid_r1,
                  l2_cpu1_ibufid_r1,
                  l2_cpu1_ls_sync_req,
                  l2_cpu1_ccb_req_addr_c3,
                  l2_cpu1_ccb_dbg_req_c3,
                  l2_cpu1_ls_ccb_clken_c3,
                  l2_cpu1_ls_ccb_req_c3,
                  l2_cpu1_ccb_req_id_c3,
                  l2_cpu1_ccb_req_type_c3,
                  l2_cpu1_ccb_req_info_c3,
                  l2_cpu1_if_ccb_clken_c3,
                  l2_cpu1_if_ccb_req_c3,
                  l2_cpu1_if_sync_req,
                  l2_cpu1_tlb_ccb_clken_c3,
                  l2_cpu1_tlb_ccb_req_c3,
                  l2_cpu1_tlb_sync_req,
                  l2_cpu1_tlb_sync_complete,
                  l2_cpu1_tbw_desc_vld,
                  l2_cpu1_tbw_ext_err,
                  l2_cpu1_tbw_ext_err_type,
                  l2_cpu1_tbw_dbl_ecc_err,
                  l2_cpu1_tbw_desc_data,
                  l2_cpu1_spr_rd_data,
                  l2_cpu1_l2_cache_size,
                  l2_cpu1_pf_throttle_q,
  
                  l2_cpu1_wr_ex_resp,
                  l2_cpu1_wr_ex_fail,

                  l2_cpu1_ic_base,
                  l2_cpu1_no_intctrl,

                  l2_cpu1_pmu_events,

                  ds_cpu1_l2_spr_en,
                  ds_cpu1_l2_spr_rd,
                  ds_cpu1_l2_spr_wr,
                  ds_cpu1_l2_spr_addr,
                  ds_cpu1_l2_spr_dw,
                  ds_cpu1_l2_spr_wr_data,

                  l2_cpu1_wr_data_vld_x1_q,
                  l2_cpu1_wr_evict_x1_q,
                  l2_cpu1_wr_data,
                  l2_cpu1_ls_rd_haz_vld_arb_q,
                  l2_cpu1_ls_wr_haz_vld_arb_q,
                  l2_cpu1_dt_pmu_evt_en,

//-----------------------------------------------------------------------------
// ucpu2
//-----------------------------------------------------------------------------
                  l2_cpu2_cfg_ecc_en,
                  l2_cpu2_arb_thrshld_timeout_en,
                  l2_cpu2_disable_clean_evict_opt,
                  l2_cpu2_dext_err_r2,
                  l2_cpu2_dext_err_type_r2,
                  l2_cpu2_dsngl_ecc_err_r3,
                  l2_cpu2_ddbl_ecc_err_r3,
                  l2_cpu2_ddata_r2,
                  l2_cpu2_barrier_done,
                  l2_cpu2_spec_valid,
                  l2_cpu2_spec_bufid,
                  l2_cpu2_rvalid,
                  l2_cpu2_rstate,
                  l2_cpu2_rexfail,
                  l2_cpu2_rbufid,
                  l2_cpu2_dvalid_r1,
                  l2_cpu2_dlast_r1,
                  l2_cpu2_dbufid_r1,
                  l2_cpu2_iext_err_r2,
                  l2_cpu2_iext_err_type_r2,
                  l2_cpu2_isngl_ecc_err_r3,
                  l2_cpu2_idbl_ecc_err_r3,
                  l2_cpu2_idata_r2,
                  l2_cpu2_ivalid_r1,
                  l2_cpu2_ibufid_r1,
                  l2_cpu2_ls_sync_req,
                  l2_cpu2_ccb_req_addr_c3,
                  l2_cpu2_ccb_dbg_req_c3,
                  l2_cpu2_ls_ccb_clken_c3,
                  l2_cpu2_ls_ccb_req_c3,
                  l2_cpu2_ccb_req_id_c3,
                  l2_cpu2_ccb_req_type_c3,
                  l2_cpu2_ccb_req_info_c3,
                  l2_cpu2_if_ccb_clken_c3,
                  l2_cpu2_if_ccb_req_c3,
                  l2_cpu2_if_sync_req,
                  l2_cpu2_tlb_ccb_clken_c3,
                  l2_cpu2_tlb_ccb_req_c3,
                  l2_cpu2_tlb_sync_req,
                  l2_cpu2_tlb_sync_complete,
                  l2_cpu2_tbw_desc_vld,
                  l2_cpu2_tbw_ext_err,
                  l2_cpu2_tbw_ext_err_type,
                  l2_cpu2_tbw_dbl_ecc_err,
                  l2_cpu2_tbw_desc_data,
                  l2_cpu2_spr_rd_data,
                  l2_cpu2_l2_cache_size,
                  l2_cpu2_pf_throttle_q,

                  l2_cpu2_wr_ex_resp,
                  l2_cpu2_wr_ex_fail,

                  l2_cpu2_ic_base,
                  l2_cpu2_no_intctrl,

                  l2_cpu2_pmu_events,

                  ds_cpu2_l2_spr_en,
                  ds_cpu2_l2_spr_rd,
                  ds_cpu2_l2_spr_wr,
                  ds_cpu2_l2_spr_addr,
                  ds_cpu2_l2_spr_dw,
                  ds_cpu2_l2_spr_wr_data,

                  l2_cpu2_wr_data_vld_x1_q,
                  l2_cpu2_wr_evict_x1_q,
                  l2_cpu2_wr_data,
                  l2_cpu2_ls_rd_haz_vld_arb_q,
                  l2_cpu2_ls_wr_haz_vld_arb_q,
                  l2_cpu2_dt_pmu_evt_en,

//-----------------------------------------------------------------------------
// ucpu3
//-----------------------------------------------------------------------------
                  l2_cpu3_cfg_ecc_en,
                  l2_cpu3_arb_thrshld_timeout_en,
                  l2_cpu3_disable_clean_evict_opt,
                  l2_cpu3_dext_err_r2,
                  l2_cpu3_dext_err_type_r2,
                  l2_cpu3_dsngl_ecc_err_r3,
                  l2_cpu3_ddbl_ecc_err_r3,
                  l2_cpu3_ddata_r2,
                  l2_cpu3_barrier_done,
                  l2_cpu3_spec_valid,
                  l2_cpu3_spec_bufid,
                  l2_cpu3_rvalid,
                  l2_cpu3_rstate,
                  l2_cpu3_rexfail,
                  l2_cpu3_rbufid,
                  l2_cpu3_dvalid_r1,
                  l2_cpu3_dlast_r1,
                  l2_cpu3_dbufid_r1,
                  l2_cpu3_iext_err_r2,
                  l2_cpu3_iext_err_type_r2,
                  l2_cpu3_isngl_ecc_err_r3,
                  l2_cpu3_idbl_ecc_err_r3,
                  l2_cpu3_idata_r2,
                  l2_cpu3_ivalid_r1,
                  l2_cpu3_ibufid_r1,
                  l2_cpu3_ls_sync_req,
                  l2_cpu3_ccb_req_addr_c3,
                  l2_cpu3_ccb_dbg_req_c3,
                  l2_cpu3_ls_ccb_clken_c3,
                  l2_cpu3_ls_ccb_req_c3,
                  l2_cpu3_ccb_req_id_c3,
                  l2_cpu3_ccb_req_type_c3,
                  l2_cpu3_ccb_req_info_c3,
                  l2_cpu3_if_ccb_clken_c3,
                  l2_cpu3_if_ccb_req_c3,
                  l2_cpu3_if_sync_req,
                  l2_cpu3_tlb_ccb_clken_c3,
                  l2_cpu3_tlb_ccb_req_c3,
                  l2_cpu3_tlb_sync_req,
                  l2_cpu3_tlb_sync_complete,
                  l2_cpu3_tbw_desc_vld,
                  l2_cpu3_tbw_ext_err,
                  l2_cpu3_tbw_ext_err_type,
                  l2_cpu3_tbw_dbl_ecc_err,
                  l2_cpu3_tbw_desc_data,
                  l2_cpu3_spr_rd_data,
                  l2_cpu3_l2_cache_size,
                  l2_cpu3_pf_throttle_q,

                  l2_cpu3_wr_ex_resp,
                  l2_cpu3_wr_ex_fail,

                  l2_cpu3_ic_base,
                  l2_cpu3_no_intctrl,

                  l2_cpu3_pmu_events,

                  ds_cpu3_l2_spr_en,
                  ds_cpu3_l2_spr_rd,
                  ds_cpu3_l2_spr_wr,
                  ds_cpu3_l2_spr_addr,
                  ds_cpu3_l2_spr_dw,
                  ds_cpu3_l2_spr_wr_data,

                  l2_cpu3_wr_data_vld_x1_q,
                  l2_cpu3_wr_evict_x1_q,
                  l2_cpu3_wr_data,
                  l2_cpu3_ls_rd_haz_vld_arb_q,
                  l2_cpu3_ls_wr_haz_vld_arb_q,
                  l2_cpu3_dt_pmu_evt_en,

//-----------------------------------------------------------------------------
// tag_pipe / cpu slave
//-----------------------------------------------------------------------------
                  l2_cpu0_flsh_ls_rd_l2_dly,
                  l2_cpu0_flsh_ls_wr_l2_dly,

                  l2_cpu0_wr_data_stall,

                  l2_cpu1_flsh_ls_rd_l2_dly,
                  l2_cpu1_flsh_ls_wr_l2_dly,

                  l2_cpu1_wr_data_stall,

                  l2_cpu2_flsh_ls_rd_l2_dly,
                  l2_cpu2_flsh_ls_wr_l2_dly,

                  l2_cpu2_wr_data_stall,

                  l2_cpu3_flsh_ls_rd_l2_dly,
                  l2_cpu3_flsh_ls_wr_l2_dly,

                  l2_cpu3_wr_data_stall,

                  l2_cpu0_flsh_ls_rd_id_l2_dly,
                  l2_cpu0_flsh_ls_wr_id_l2_dly,

                  l2_cpu1_flsh_ls_rd_id_l2_dly,
                  l2_cpu1_flsh_ls_wr_id_l2_dly,

                  l2_cpu2_flsh_ls_rd_id_l2_dly,
                  l2_cpu2_flsh_ls_wr_id_l2_dly,

                  l2_cpu3_flsh_ls_rd_id_l2_dly,
                  l2_cpu3_flsh_ls_wr_id_l2_dly,

                  l2_cpu0_flsh_ls_rd_l4_dly,
                  l2_cpu0_flsh_if_rd_l4_dly,
                  l2_cpu0_flsh_tw_rd_l4_dly,
                  l2_cpu0_flsh_ls_wr_l4_dly,

                  l2_cpu1_flsh_ls_rd_l4_dly,
                  l2_cpu1_flsh_if_rd_l4_dly,
                  l2_cpu1_flsh_tw_rd_l4_dly,
                  l2_cpu1_flsh_ls_wr_l4_dly,

                  l2_cpu2_flsh_ls_rd_l4_dly,
                  l2_cpu2_flsh_if_rd_l4_dly,
                  l2_cpu2_flsh_tw_rd_l4_dly,
                  l2_cpu2_flsh_ls_wr_l4_dly,

                  l2_cpu3_flsh_ls_rd_l4_dly,
                  l2_cpu3_flsh_if_rd_l4_dly,
                  l2_cpu3_flsh_tw_rd_l4_dly,
                  l2_cpu3_flsh_ls_wr_l4_dly,

                  l2_cpu0_flsh_ls_rd_id_l4_dly,
                  l2_cpu0_flsh_if_rd_id_l4_dly,
                  l2_cpu0_flsh_ls_wr_id_l4_dly,
                  l2_cpu0_flsh_ls_wr_evict_l4_dly,

                  l2_cpu1_flsh_ls_rd_id_l4_dly,
                  l2_cpu1_flsh_if_rd_id_l4_dly,
                  l2_cpu1_flsh_ls_wr_id_l4_dly,
                  l2_cpu1_flsh_ls_wr_evict_l4_dly,

                  l2_cpu2_flsh_ls_rd_id_l4_dly,
                  l2_cpu2_flsh_if_rd_id_l4_dly,
                  l2_cpu2_flsh_ls_wr_id_l4_dly,
                  l2_cpu2_flsh_ls_wr_evict_l4_dly,

                  l2_cpu3_flsh_ls_rd_id_l4_dly,
                  l2_cpu3_flsh_if_rd_id_l4_dly,
                  l2_cpu3_flsh_ls_wr_id_l4_dly,
                  l2_cpu3_flsh_ls_wr_evict_l4_dly,

                  l2_cpu0_lrq_haz_pending,
                  l2_cpu1_lrq_haz_pending,
                  l2_cpu2_lrq_haz_pending,
                  l2_cpu3_lrq_haz_pending,

                  l2_cpu0_ifq_haz_pending,
                  l2_cpu1_ifq_haz_pending,
                  l2_cpu2_ifq_haz_pending,
                  l2_cpu3_ifq_haz_pending,

                  l2_cpu0_trq_haz_pending,
                  l2_cpu1_trq_haz_pending,
                  l2_cpu2_trq_haz_pending,
                  l2_cpu3_trq_haz_pending,

                  l2_cpu0_wrq_haz_pending,
                  l2_cpu1_wrq_haz_pending,
                  l2_cpu2_wrq_haz_pending,
                  l2_cpu3_wrq_haz_pending,

                  l2_cpu0_idle_block_reqs_q,
                  l2_cpu1_idle_block_reqs_q,
                  l2_cpu2_idle_block_reqs_q,
                  l2_cpu3_idle_block_reqs_q,

                  l2_cpu0_ls_peq_coll_l4_dly,
                  l2_cpu1_ls_peq_coll_l4_dly,
                  l2_cpu2_ls_peq_coll_l4_dly,
                  l2_cpu3_ls_peq_coll_l4_dly,

//-----------------------------------------------------------------------------
// tag_pipe
//-----------------------------------------------------------------------------
                  l2_tbnk0_cpu0_lrq_clr_l4_dly2_q,
                  l2_tbnk0_cpu1_lrq_clr_l4_dly2_q,
                  l2_tbnk0_cpu2_lrq_clr_l4_dly2_q,
                  l2_tbnk0_cpu3_lrq_clr_l4_dly2_q,

                  l2_tbnk1_cpu0_lrq_clr_l4_dly2_q,
                  l2_tbnk1_cpu1_lrq_clr_l4_dly2_q,
                  l2_tbnk1_cpu2_lrq_clr_l4_dly2_q,
                  l2_tbnk1_cpu3_lrq_clr_l4_dly2_q,

                  l2_tbnk0_cpu0_ifq_clr_l4_dly2_q,
                  l2_tbnk0_cpu1_ifq_clr_l4_dly2_q,
                  l2_tbnk0_cpu2_ifq_clr_l4_dly2_q,
                  l2_tbnk0_cpu3_ifq_clr_l4_dly2_q,

                  l2_tbnk1_cpu0_ifq_clr_l4_dly2_q,
                  l2_tbnk1_cpu1_ifq_clr_l4_dly2_q,
                  l2_tbnk1_cpu2_ifq_clr_l4_dly2_q,
                  l2_tbnk1_cpu3_ifq_clr_l4_dly2_q,

                  l2_tbnk0_cpu0_trq_clr_l4_dly2_q,
                  l2_tbnk0_cpu1_trq_clr_l4_dly2_q,
                  l2_tbnk0_cpu2_trq_clr_l4_dly2_q,
                  l2_tbnk0_cpu3_trq_clr_l4_dly2_q,

                  l2_tbnk1_cpu0_trq_clr_l4_dly2_q,
                  l2_tbnk1_cpu1_trq_clr_l4_dly2_q,
                  l2_tbnk1_cpu2_trq_clr_l4_dly2_q,
                  l2_tbnk1_cpu3_trq_clr_l4_dly2_q,

                  l2_tbnk0_cpu0_wrq_clr_l4_dly2_q,
                  l2_tbnk0_cpu1_wrq_clr_l4_dly2_q,
                  l2_tbnk0_cpu2_wrq_clr_l4_dly2_q,
                  l2_tbnk0_cpu3_wrq_clr_l4_dly2_q,

                  l2_tbnk1_cpu0_wrq_clr_l4_dly2_q,
                  l2_tbnk1_cpu1_wrq_clr_l4_dly2_q,
                  l2_tbnk1_cpu2_wrq_clr_l4_dly2_q,
                  l2_tbnk1_cpu3_wrq_clr_l4_dly2_q,


//-----------------------------------------------------------------------------
// cpu_logic / cpu slave
//-----------------------------------------------------------------------------
                  l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly,
                  l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly,

                  l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly,
                  l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly,

                  l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly,
                  l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly,

                  l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly,
                  l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly,


//-----------------------------------------------------------------------------
// feq / cpu slave
//-----------------------------------------------------------------------------
                  l2_cpu0_dsq_rd_data_q,
                  l2_cpu0_dsq_rd_byte_strb_q,
                  l2_cpu1_dsq_rd_data_q,
                  l2_cpu1_dsq_rd_byte_strb_q,
                  l2_cpu2_dsq_rd_data_q,
                  l2_cpu2_dsq_rd_byte_strb_q,
                  l2_cpu3_dsq_rd_data_q,
                  l2_cpu3_dsq_rd_byte_strb_q,
 
                  l2_cpu0_dsq_clr_vld_q,
                  l2_cpu0_dsq_clr_id_q,
                  l2_cpu0_dsq_rd_en,
                  l2_cpu0_dsq_rd_en_x2,
                  l2_cpu0_dsq_rd_buf_id,
                  l2_cpu1_dsq_clr_vld_q,
                  l2_cpu1_dsq_clr_id_q,
                  l2_cpu1_dsq_rd_en,
                  l2_cpu1_dsq_rd_en_x2,
                  l2_cpu1_dsq_rd_buf_id,
                  l2_cpu2_dsq_clr_vld_q,
                  l2_cpu2_dsq_clr_id_q,
                  l2_cpu2_dsq_rd_en,
                  l2_cpu2_dsq_rd_en_x2,
                  l2_cpu2_dsq_rd_buf_id,
                  l2_cpu3_dsq_clr_vld_q,
                  l2_cpu3_dsq_rd_en,
                  l2_cpu3_dsq_rd_en_x2,
                  l2_cpu3_dsq_clr_id_q,
                  l2_cpu3_dsq_rd_buf_id,

//-----------------------------------------------------------------------------
// arbitration
//-----------------------------------------------------------------------------
                  l2_cpu0_rd_vld_skid,
                  l2_cpu1_rd_vld_skid,
                  l2_cpu2_rd_vld_skid,
                  l2_cpu3_rd_vld_skid,

                  l2_cpu0_pf_rd_vld_skid_popped,
                  l2_cpu1_pf_rd_vld_skid_popped,
                  l2_cpu2_pf_rd_vld_skid_popped,
                  l2_cpu3_pf_rd_vld_skid_popped,

                  l2_cpu0_rd_arb,
                  l2_cpu1_rd_arb,
                  l2_cpu2_rd_arb,
                  l2_cpu3_rd_arb,

                  l2_cpu0_wr_vld_skid,
                  l2_cpu1_wr_vld_skid,
                  l2_cpu2_wr_vld_skid,
                  l2_cpu3_wr_vld_skid,

                  l2_cpu0_wr_arb,
                  l2_cpu1_wr_arb,
                  l2_cpu2_wr_arb,
                  l2_cpu3_wr_arb,

                  l2_cpu0_ic_vld_skid,
                  l2_cpu1_ic_vld_skid,
                  l2_cpu2_ic_vld_skid,
                  l2_cpu3_ic_vld_skid,
  
                  l2_cpu0_ic_barrier_stall_q,
                  l2_cpu1_ic_barrier_stall_q,
                  l2_cpu2_ic_barrier_stall_q,
                  l2_cpu3_ic_barrier_stall_q,

                  l2_cpu0_blk_non_evict_wr,
                  l2_cpu1_blk_non_evict_wr,
                  l2_cpu2_blk_non_evict_wr,
                  l2_cpu3_blk_non_evict_wr,

//-----------------------------------------------------------------------------
// ucpu0
//-----------------------------------------------------------------------------
                  l2_cpu0_idle_wakeup_q,
                  l2_cpu0_rd_arb_fast,
                  l2_cpu0_rd_id_arb_set,
                  l2_cpu0_rd_lrq_id_arb_set,
                  l2_cpu0_rd_type_arb_set,
                  l2_cpu0_rd_cache_attr_arb_set,
                  l2_cpu0_rd_page_attr_arb_set,
                  l2_cpu0_rd_elem_size_arb_set,
                  l2_cpu0_rd_way_arb_set,
                  l2_cpu0_rd_replayed_arb_set,
                  l2_cpu0_rd_excl_arb_set,
                  l2_cpu0_rd_priv_arb_set,
                  l2_cpu0_rd_shared_arb_set,
                  l2_cpu0_rd_va48_arb_set,
                  l2_cpu0_rd_aarch64_arb_set,
                  l2_cpu0_rd_asid_arb_set,
                  l2_cpu0_rd_prfm_arb_set,
                  l2_cpu0_rd_addr_arb_set,
                  l2_cpu0_rd_bypass_arb_set,
                  l2_cpu0_rd_bypass_req_can_e5,
                  l2_cpu0_early_rd_reqe4_e5_q,
                  l2_cpu0_rd_bypass_way_e5,
                  l2_cpu0_rd_bypass_bufid_e5,
                  l2_cpu0_rd_bypass_lrq_id_e5,

                  l2_cpu0_wr_arb_fast,
                  l2_cpu0_wr_id_arb_set,
                  l2_cpu0_wr_partial_dw_arb_set,
                  l2_cpu0_wr_cache_attr_arb_set,
                  l2_cpu0_wr_page_attr_arb_set,
                  l2_cpu0_wr_elem_size_arb_set,
                  l2_cpu0_wr_type_arb_set,
                  l2_cpu0_wr_cl_id_arb_set,
                  l2_cpu0_wr_priv_arb_set,
                  l2_cpu0_wr_shared_arb_set,
                  l2_cpu0_wr_last_arb_set,
                  l2_cpu0_wr_clean_evict_arb_set,
                  l2_cpu0_wr_err_arb_set,
                  l2_cpu0_wr_way_arb_set,
                  l2_cpu0_wr_dirty_arb_set,
                  l2_cpu0_wr_1st_replayed_arb_set,
                  l2_cpu0_wr_addr_arb_set,
                  l2_cpu0_ic_arb_fast,
                  l2_cpu0_ic_id_arb_set,
                  l2_cpu0_ic_write_arb_set,
                  l2_cpu0_ic_excl_arb_set,
                  l2_cpu0_ic_elem_size_arb_set,
                  l2_cpu0_ic_ns_arb_set,
                  l2_cpu0_ic_addr_arb_set,
                  l2_cpu0_ic_data_arb_set,

                  l2_cpu0_wrq_almost_full,

                  l2_cpu0_ls_wr_req_w2a,
                  l2_cpu0_ls_wr_last_w2a,
                  l2_cpu0_ls_wr_dirty_w2a,
                  l2_cpu0_ls_wr_err_w2a,
                  l2_cpu0_ls_wr_type_w2a,
                  l2_cpu0_ls_wr_ccb_id_w2a,
                  l2_cpu0_ls_wr_data_w2a,

                  l2_cpu0_ls_ccb_resp,
                  l2_cpu0_ls_ccb_resp_id,
                  l2_cpu0_ls_ccb_data_wr,

                  l2_cpu0_if_ccb_resp,
                  l2_cpu0_if_ccb_resp_id,
   
                  l2_cpu0_tw_ccb_resp,
                  l2_cpu0_tw_ccb_resp_id,

                  l2_cpu0_if_sync_done_q,
                  l2_cpu0_tlb_sync_done_q,

                  l2_cpu0_lrq_haz_clr_id_dcd_q,
                  l2_cpu0_wrq_haz_clr_id_dcd_q,
                  l2_cpu0_ls_rd_haz_id_arb_q,
                  l2_cpu0_ls_wr_haz_id_arb_q,

//-----------------------------------------------------------------------------
// ucpu1
//-----------------------------------------------------------------------------
                  l2_cpu1_idle_wakeup_q,
                  l2_cpu1_rd_arb_fast,
                  l2_cpu1_rd_id_arb_set,
                  l2_cpu1_rd_lrq_id_arb_set,
                  l2_cpu1_rd_type_arb_set,
                  l2_cpu1_rd_cache_attr_arb_set,
                  l2_cpu1_rd_page_attr_arb_set,
                  l2_cpu1_rd_elem_size_arb_set,
                  l2_cpu1_rd_way_arb_set,
                  l2_cpu1_rd_replayed_arb_set,
                  l2_cpu1_rd_excl_arb_set,
                  l2_cpu1_rd_priv_arb_set,
                  l2_cpu1_rd_shared_arb_set,
                  l2_cpu1_rd_va48_arb_set,
                  l2_cpu1_rd_aarch64_arb_set,
                  l2_cpu1_rd_asid_arb_set,
                  l2_cpu1_rd_prfm_arb_set,
                  l2_cpu1_rd_addr_arb_set,
                  l2_cpu1_rd_bypass_arb_set,
                  l2_cpu1_rd_bypass_req_can_e5,
                  l2_cpu1_early_rd_reqe4_e5_q,
                  l2_cpu1_rd_bypass_way_e5,
                  l2_cpu1_rd_bypass_bufid_e5,
                  l2_cpu1_rd_bypass_lrq_id_e5,

                  l2_cpu1_wr_arb_fast,
                  l2_cpu1_wr_id_arb_set,
                  l2_cpu1_wr_partial_dw_arb_set,
                  l2_cpu1_wr_cache_attr_arb_set,
                  l2_cpu1_wr_page_attr_arb_set,
                  l2_cpu1_wr_elem_size_arb_set,
                  l2_cpu1_wr_type_arb_set,
                  l2_cpu1_wr_cl_id_arb_set,
                  l2_cpu1_wr_priv_arb_set,
                  l2_cpu1_wr_shared_arb_set,
                  l2_cpu1_wr_last_arb_set,
                  l2_cpu1_wr_clean_evict_arb_set,
                  l2_cpu1_wr_err_arb_set,
                  l2_cpu1_wr_way_arb_set,
                  l2_cpu1_wr_dirty_arb_set,
                  l2_cpu1_wr_1st_replayed_arb_set,
                  l2_cpu1_wr_addr_arb_set,
                  l2_cpu1_ic_arb_fast,
                  l2_cpu1_ic_id_arb_set,
                  l2_cpu1_ic_write_arb_set,
                  l2_cpu1_ic_excl_arb_set,
                  l2_cpu1_ic_elem_size_arb_set,
                  l2_cpu1_ic_ns_arb_set,
                  l2_cpu1_ic_addr_arb_set,
                  l2_cpu1_ic_data_arb_set,

                  l2_cpu1_wrq_almost_full,

                  l2_cpu1_ls_wr_req_w2a,
                  l2_cpu1_ls_wr_last_w2a,
                  l2_cpu1_ls_wr_dirty_w2a,
                  l2_cpu1_ls_wr_err_w2a,
                  l2_cpu1_ls_wr_type_w2a,
                  l2_cpu1_ls_wr_ccb_id_w2a,
                  l2_cpu1_ls_wr_data_w2a,

                  l2_cpu1_ls_ccb_resp,
                  l2_cpu1_ls_ccb_resp_id,
                  l2_cpu1_ls_ccb_data_wr,

                  l2_cpu1_if_ccb_resp,
                  l2_cpu1_if_ccb_resp_id,
   
                  l2_cpu1_tw_ccb_resp,
                  l2_cpu1_tw_ccb_resp_id,

                  l2_cpu1_if_sync_done_q,
                  l2_cpu1_tlb_sync_done_q,

                  l2_cpu1_lrq_haz_clr_id_dcd_q,
                  l2_cpu1_wrq_haz_clr_id_dcd_q,
                  l2_cpu1_ls_rd_haz_id_arb_q,
                  l2_cpu1_ls_wr_haz_id_arb_q,

//-----------------------------------------------------------------------------
// ucpu2
//-----------------------------------------------------------------------------
                  l2_cpu2_idle_wakeup_q,
                  l2_cpu2_rd_arb_fast,
                  l2_cpu2_rd_id_arb_set,
                  l2_cpu2_rd_lrq_id_arb_set,
                  l2_cpu2_rd_type_arb_set,
                  l2_cpu2_rd_cache_attr_arb_set,
                  l2_cpu2_rd_page_attr_arb_set,
                  l2_cpu2_rd_elem_size_arb_set,
                  l2_cpu2_rd_way_arb_set,
                  l2_cpu2_rd_replayed_arb_set,
                  l2_cpu2_rd_excl_arb_set,
                  l2_cpu2_rd_priv_arb_set,
                  l2_cpu2_rd_shared_arb_set,
                  l2_cpu2_rd_va48_arb_set,
                  l2_cpu2_rd_aarch64_arb_set,
                  l2_cpu2_rd_asid_arb_set,
                  l2_cpu2_rd_prfm_arb_set,
                  l2_cpu2_rd_addr_arb_set,
                  l2_cpu2_rd_bypass_arb_set,
                  l2_cpu2_rd_bypass_req_can_e5,
                  l2_cpu2_early_rd_reqe4_e5_q,
                  l2_cpu2_rd_bypass_way_e5,
                  l2_cpu2_rd_bypass_bufid_e5,
                  l2_cpu2_rd_bypass_lrq_id_e5,

                  l2_cpu2_wr_arb_fast,
                  l2_cpu2_wr_id_arb_set,
                  l2_cpu2_wr_partial_dw_arb_set,
                  l2_cpu2_wr_cache_attr_arb_set,
                  l2_cpu2_wr_page_attr_arb_set,
                  l2_cpu2_wr_elem_size_arb_set,
                  l2_cpu2_wr_type_arb_set,
                  l2_cpu2_wr_cl_id_arb_set,
                  l2_cpu2_wr_priv_arb_set,
                  l2_cpu2_wr_shared_arb_set,
                  l2_cpu2_wr_last_arb_set,
                  l2_cpu2_wr_clean_evict_arb_set,
                  l2_cpu2_wr_err_arb_set,
                  l2_cpu2_wr_way_arb_set,
                  l2_cpu2_wr_dirty_arb_set,
                  l2_cpu2_wr_1st_replayed_arb_set,
                  l2_cpu2_wr_addr_arb_set,
                  l2_cpu2_ic_arb_fast,
                  l2_cpu2_ic_id_arb_set,
                  l2_cpu2_ic_write_arb_set,
                  l2_cpu2_ic_excl_arb_set,
                  l2_cpu2_ic_elem_size_arb_set,
                  l2_cpu2_ic_ns_arb_set,
                  l2_cpu2_ic_addr_arb_set,
                  l2_cpu2_ic_data_arb_set,

                  l2_cpu2_wrq_almost_full,

                  l2_cpu2_ls_wr_req_w2a,
                  l2_cpu2_ls_wr_last_w2a,
                  l2_cpu2_ls_wr_dirty_w2a,
                  l2_cpu2_ls_wr_err_w2a,
                  l2_cpu2_ls_wr_type_w2a,
                  l2_cpu2_ls_wr_ccb_id_w2a,
                  l2_cpu2_ls_wr_data_w2a,

                  l2_cpu2_ls_ccb_resp,
                  l2_cpu2_ls_ccb_resp_id,
                  l2_cpu2_ls_ccb_data_wr,

                  l2_cpu2_if_ccb_resp,
                  l2_cpu2_if_ccb_resp_id,
   
                  l2_cpu2_tw_ccb_resp,
                  l2_cpu2_tw_ccb_resp_id,

                  l2_cpu2_if_sync_done_q,
                  l2_cpu2_tlb_sync_done_q,

                  l2_cpu2_lrq_haz_clr_id_dcd_q,
                  l2_cpu2_wrq_haz_clr_id_dcd_q,
                  l2_cpu2_ls_rd_haz_id_arb_q,
                  l2_cpu2_ls_wr_haz_id_arb_q,

//-----------------------------------------------------------------------------
// ucpu3
//-----------------------------------------------------------------------------
                  l2_cpu3_idle_wakeup_q,
                  l2_cpu3_rd_arb_fast,
                  l2_cpu3_rd_id_arb_set,
                  l2_cpu3_rd_lrq_id_arb_set,
                  l2_cpu3_rd_type_arb_set,
                  l2_cpu3_rd_cache_attr_arb_set,
                  l2_cpu3_rd_page_attr_arb_set,
                  l2_cpu3_rd_elem_size_arb_set,
                  l2_cpu3_rd_way_arb_set,
                  l2_cpu3_rd_replayed_arb_set,
                  l2_cpu3_rd_excl_arb_set,
                  l2_cpu3_rd_priv_arb_set,
                  l2_cpu3_rd_shared_arb_set,
                  l2_cpu3_rd_va48_arb_set,
                  l2_cpu3_rd_aarch64_arb_set,
                  l2_cpu3_rd_asid_arb_set,
                  l2_cpu3_rd_prfm_arb_set,
                  l2_cpu3_rd_addr_arb_set,
                  l2_cpu3_rd_bypass_arb_set,
                  l2_cpu3_rd_bypass_req_can_e5,
                  l2_cpu3_early_rd_reqe4_e5_q,
                  l2_cpu3_rd_bypass_way_e5,
                  l2_cpu3_rd_bypass_bufid_e5,
                  l2_cpu3_rd_bypass_lrq_id_e5,

                  l2_cpu3_wr_arb_fast,
                  l2_cpu3_wr_id_arb_set,
                  l2_cpu3_wr_partial_dw_arb_set,
                  l2_cpu3_wr_cache_attr_arb_set,
                  l2_cpu3_wr_page_attr_arb_set,
                  l2_cpu3_wr_elem_size_arb_set,
                  l2_cpu3_wr_type_arb_set,
                  l2_cpu3_wr_cl_id_arb_set,
                  l2_cpu3_wr_priv_arb_set,
                  l2_cpu3_wr_shared_arb_set,
                  l2_cpu3_wr_last_arb_set,
                  l2_cpu3_wr_clean_evict_arb_set,
                  l2_cpu3_wr_err_arb_set,
                  l2_cpu3_wr_way_arb_set,
                  l2_cpu3_wr_dirty_arb_set,
                  l2_cpu3_wr_1st_replayed_arb_set,
                  l2_cpu3_wr_addr_arb_set,
                  l2_cpu3_ic_arb_fast,
                  l2_cpu3_ic_id_arb_set,
                  l2_cpu3_ic_write_arb_set,
                  l2_cpu3_ic_excl_arb_set,
                  l2_cpu3_ic_elem_size_arb_set,
                  l2_cpu3_ic_ns_arb_set,
                  l2_cpu3_ic_addr_arb_set,
                  l2_cpu3_ic_data_arb_set,

                  l2_cpu3_wrq_almost_full,

                  l2_cpu3_ls_wr_req_w2a,
                  l2_cpu3_ls_wr_last_w2a,
                  l2_cpu3_ls_wr_dirty_w2a,
                  l2_cpu3_ls_wr_err_w2a,
                  l2_cpu3_ls_wr_type_w2a,
                  l2_cpu3_ls_wr_ccb_id_w2a,
                  l2_cpu3_ls_wr_data_w2a,

                  l2_cpu3_ls_ccb_resp,
                  l2_cpu3_ls_ccb_resp_id,
                  l2_cpu3_ls_ccb_data_wr,

                  l2_cpu3_if_ccb_resp,
                  l2_cpu3_if_ccb_resp_id,
   
                  l2_cpu3_tw_ccb_resp,
                  l2_cpu3_tw_ccb_resp_id,

                  l2_cpu3_if_sync_done_q,
                  l2_cpu3_tlb_sync_done_q,

                  l2_cpu3_lrq_haz_clr_id_dcd_q,
                  l2_cpu3_wrq_haz_clr_id_dcd_q,
                  l2_cpu3_ls_rd_haz_id_arb_q,
                  l2_cpu3_ls_wr_haz_id_arb_q,

// END L2-CPU interface

//-------------------------------------------------------------------
// TM interface 
//-------------------------------------------------------------------
// BEGIN TIMER-CPU interface
                  tm_cpu0_cntkctl_usr,
                  tm_cpu0_cnthctl_kernel,

                  tm_cpu1_cntkctl_usr,
                  tm_cpu1_cnthctl_kernel,

                  tm_cpu2_cntkctl_usr,
                  tm_cpu2_cnthctl_kernel,

                  tm_cpu3_cntkctl_usr,
                  tm_cpu3_cnthctl_kernel,
// END TIMER-CPU interface

//-----------------------------------------------------------------------------
// IC interface
//-----------------------------------------------------------------------------
                  ls_cpu0_imp_abort_slv,
                  ls_cpu0_imp_abort_ecc,
                  ls_cpu0_imp_abort_dec,
                  ls_cpu0_imp_abort_containable,
                  ls_cpu0_raw_eae_nonsec,
                  ls_cpu0_raw_eae_secure,

                  ds_cpu0_ic_cpsr_mode,
                  ds_cpu0_ic_sample_spr,
                  ds_cpu0_ic_aa64naa32,
                  ds_cpu0_ic_hcr_change,
                  ds_cpu0_ic_scr_change,
// BEGIN INCLUDE FOR CPU1
                  ds_cpu1_ic_cpsr_mode,
                  ds_cpu1_ic_sample_spr,
                  ds_cpu1_ic_aa64naa32,
                  ds_cpu1_ic_hcr_change,
                  ds_cpu1_ic_scr_change,
                  ls_cpu1_imp_abort_slv,
                  ls_cpu1_imp_abort_ecc,
                  ls_cpu1_imp_abort_dec,
                  ls_cpu1_imp_abort_containable,
                  ls_cpu1_raw_eae_nonsec,
                  ls_cpu1_raw_eae_secure,
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
                  ds_cpu2_ic_cpsr_mode,
                  ds_cpu2_ic_sample_spr,
                  ds_cpu2_ic_aa64naa32,
                  ds_cpu2_ic_hcr_change,
                  ds_cpu2_ic_scr_change,
                  ls_cpu2_imp_abort_slv,
                  ls_cpu2_imp_abort_ecc,
                  ls_cpu2_imp_abort_dec,
                  ls_cpu2_imp_abort_containable,
                  ls_cpu2_raw_eae_nonsec,
                  ls_cpu2_raw_eae_secure,
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
                  ds_cpu3_ic_cpsr_mode,
                  ds_cpu3_ic_sample_spr,
                  ds_cpu3_ic_aa64naa32,
                  ds_cpu3_ic_hcr_change,
                  ds_cpu3_ic_scr_change,
                  ls_cpu3_imp_abort_slv,
                  ls_cpu3_imp_abort_ecc,
                  ls_cpu3_imp_abort_dec,
                  ls_cpu3_imp_abort_containable,
                  ls_cpu3_raw_eae_nonsec,
                  ls_cpu3_raw_eae_secure,
// END INCLUDE FOR CPU3

                  ic_nfiq,
                  ic_nirq,
                  ic_nsei,
                  ic_nvfiq,
                  ic_nvirq,
                  ic_nvsei,
                  ic_p_valid,

                  ic_sample_spr,
                  ic_hcr_change_complete,
                  ic_scr_change_complete,
                  ic_el_change_complete,
                  ic_ich_el2_tc,
                  ic_ich_el2_tall0,
                  ic_ich_el2_tall1,
                  ic_sra_el3_en,
                  ic_sra_el1s_en,
                  ic_sra_el2_en,
                  ic_sra_el1ns_en,
                  ic_sre_el1ns_hyp_trap,
                  ic_sre_el1ns_mon_trap,
                  ic_sre_el1s_mon_trap,
                  ic_sre_el2_mon_trap,
                  ic_block_eoi_sgi_wr,

//-----------------------------------------------------------------------------
// DT interface
//-----------------------------------------------------------------------------
// BEGIN DT-CPU interface
//-----------------------------------------------------------------------------
// ucpu0
//-----------------------------------------------------------------------------
                  dt_cpu0_et_oslock_gclk,
                  dt_cpu0_os_double_lock_gclk,
                  dt_cpu0_halt_ack_gclk,
                  dt_cpu0_coredbg_in_reset_gclk,
                  dt_cpu0_wfx_dbg_req_gclk,
                  dt_cpu0_hlt_dbgevt_ok_gclk,
                  dt_cpu0_dbif_ack_gclk,
                  dt_cpu0_dbif_err_gclk,
                  dt_cpu0_dbif_rddata_gclk,

                  dt_cpu0_dbif_addr_pclk,
                  dt_cpu0_dbif_locked_pclk,
                  dt_cpu0_dbif_req_pclk,
                  dt_cpu0_dbif_wrdata_pclk,
                  dt_cpu0_dbif_write_pclk,
                  dt_cpu0_edecr_osuce_pclk,
                  dt_cpu0_edecr_rce_pclk,
                  dt_cpu0_edecr_ss_pclk,
                  dt_cpu0_edbgrq_pclk,
                  dt_cpu0_edacr_frc_idleack_pclk,
                  dt_cpu0_edprcr_corepurq_pclk,

                  dt_cpu0_pmusnapshot_ack_gclk,
                  dt_cpu0_pmusnapshot_req_pclk,

                  dt_cpu0_cti_trigin_7to4_gclk,
                  dt_cpu0_cti_trigin_1to0_gclk,
                  dt_cpu0_cti_trigoutack_7to4_gclk,
                  dt_cpu0_cti_trigoutack_bit1_gclk,

                  dt_cpu0_cti_trigout_7to4_pclk,
                  dt_cpu0_cti_trigout_1to0_pclk,
                  dt_cpu0_cti_triginack_7to4_pclk,
                  dt_cpu0_cti_triginack_1to0_pclk, 

                  dt_cpu0_wfx_wakeup_pclk,
                  dt_cpu0_noclkstop_pclk,
//-----------------------------------------------------------------------------
// ucpu1
//-----------------------------------------------------------------------------
                  dt_cpu1_et_oslock_gclk,
                  dt_cpu1_os_double_lock_gclk,
                  dt_cpu1_halt_ack_gclk,
                  dt_cpu1_coredbg_in_reset_gclk,
                  dt_cpu1_wfx_dbg_req_gclk,
                  dt_cpu1_hlt_dbgevt_ok_gclk,
                  dt_cpu1_dbif_ack_gclk,
                  dt_cpu1_dbif_err_gclk,
                  dt_cpu1_dbif_rddata_gclk,

                  dt_cpu1_dbif_addr_pclk,
                  dt_cpu1_dbif_locked_pclk,
                  dt_cpu1_dbif_req_pclk,
                  dt_cpu1_dbif_wrdata_pclk,
                  dt_cpu1_dbif_write_pclk,
                  dt_cpu1_edecr_osuce_pclk,
                  dt_cpu1_edecr_rce_pclk,
                  dt_cpu1_edecr_ss_pclk,
                  dt_cpu1_edbgrq_pclk,
                  dt_cpu1_edacr_frc_idleack_pclk,
                  dt_cpu1_edprcr_corepurq_pclk,

                  dt_cpu1_pmusnapshot_ack_gclk,
                  dt_cpu1_pmusnapshot_req_pclk,

                  dt_cpu1_cti_trigin_7to4_gclk,
                  dt_cpu1_cti_trigin_1to0_gclk,
                  dt_cpu1_cti_trigoutack_7to4_gclk,
                  dt_cpu1_cti_trigoutack_bit1_gclk,

                  dt_cpu1_cti_trigout_7to4_pclk,
                  dt_cpu1_cti_trigout_1to0_pclk,
                  dt_cpu1_cti_triginack_7to4_pclk,
                  dt_cpu1_cti_triginack_1to0_pclk, 

                  dt_cpu1_wfx_wakeup_pclk,
                  dt_cpu1_noclkstop_pclk,
//-----------------------------------------------------------------------------
// ucpu2
//-----------------------------------------------------------------------------
                  dt_cpu2_et_oslock_gclk,
                  dt_cpu2_os_double_lock_gclk,
                  dt_cpu2_halt_ack_gclk,
                  dt_cpu2_coredbg_in_reset_gclk,
                  dt_cpu2_wfx_dbg_req_gclk,
                  dt_cpu2_hlt_dbgevt_ok_gclk,
                  dt_cpu2_dbif_ack_gclk,
                  dt_cpu2_dbif_err_gclk,
                  dt_cpu2_dbif_rddata_gclk,

                  dt_cpu2_dbif_addr_pclk,
                  dt_cpu2_dbif_locked_pclk,
                  dt_cpu2_dbif_req_pclk,
                  dt_cpu2_dbif_wrdata_pclk,
                  dt_cpu2_dbif_write_pclk,
                  dt_cpu2_edecr_osuce_pclk,
                  dt_cpu2_edecr_rce_pclk,
                  dt_cpu2_edecr_ss_pclk,
                  dt_cpu2_edbgrq_pclk,
                  dt_cpu2_edacr_frc_idleack_pclk,
                  dt_cpu2_edprcr_corepurq_pclk,

                  dt_cpu2_pmusnapshot_ack_gclk,
                  dt_cpu2_pmusnapshot_req_pclk,

                  dt_cpu2_cti_trigin_7to4_gclk,
                  dt_cpu2_cti_trigin_1to0_gclk,
                  dt_cpu2_cti_trigoutack_7to4_gclk,
                  dt_cpu2_cti_trigoutack_bit1_gclk,

                  dt_cpu2_cti_trigout_7to4_pclk,
                  dt_cpu2_cti_trigout_1to0_pclk,
                  dt_cpu2_cti_triginack_7to4_pclk,
                  dt_cpu2_cti_triginack_1to0_pclk, 

                  dt_cpu2_wfx_wakeup_pclk,
                  dt_cpu2_noclkstop_pclk,
//-----------------------------------------------------------------------------
// ucpu3
//-----------------------------------------------------------------------------
                  dt_cpu3_et_oslock_gclk,
                  dt_cpu3_os_double_lock_gclk,
                  dt_cpu3_halt_ack_gclk,
                  dt_cpu3_coredbg_in_reset_gclk,
                  dt_cpu3_wfx_dbg_req_gclk,
                  dt_cpu3_hlt_dbgevt_ok_gclk,
                  dt_cpu3_dbif_ack_gclk,
                  dt_cpu3_dbif_err_gclk,
                  dt_cpu3_dbif_rddata_gclk,

                  dt_cpu3_dbif_addr_pclk,
                  dt_cpu3_dbif_locked_pclk,
                  dt_cpu3_dbif_req_pclk,
                  dt_cpu3_dbif_wrdata_pclk,
                  dt_cpu3_dbif_write_pclk,
                  dt_cpu3_edecr_osuce_pclk,
                  dt_cpu3_edecr_rce_pclk,
                  dt_cpu3_edecr_ss_pclk,
                  dt_cpu3_edbgrq_pclk,
                  dt_cpu3_edacr_frc_idleack_pclk,
                  dt_cpu3_edprcr_corepurq_pclk,

                  dt_cpu3_pmusnapshot_ack_gclk,
                  dt_cpu3_pmusnapshot_req_pclk,

                  dt_cpu3_cti_trigin_7to4_gclk,
                  dt_cpu3_cti_trigin_1to0_gclk,
                  dt_cpu3_cti_trigoutack_7to4_gclk,
                  dt_cpu3_cti_trigoutack_bit1_gclk,

                  dt_cpu3_cti_trigout_7to4_pclk,
                  dt_cpu3_cti_trigout_1to0_pclk,
                  dt_cpu3_cti_triginack_7to4_pclk,
                  dt_cpu3_cti_triginack_1to0_pclk, 

                  dt_cpu3_wfx_wakeup_pclk,
                  dt_cpu3_noclkstop_pclk,
// END DT-CPU interface

//-----------------------------------------------------------------------------
// CK interface
//-----------------------------------------------------------------------------
// BEGIN CK-CPU interface
                  ds_cpu0_reset_req,
                  ds_cpu0_wfi_req,
                  ds_cpu0_wfe_req,
                  ds_cpu0_flush,
                  ds_cpu0_flush_type,
                  ds_cpu0_imp_abrt_wfi_qual,
                  ds_cpu0_irq_wfi_qual,
                  ds_cpu0_fiq_wfi_qual,
                  ds_cpu0_vimp_abrt_wfi_qual,
                  ds_cpu0_virq_wfi_qual,
                  ds_cpu0_vfiq_wfi_qual,
                  ds_cpu0_imp_abrt_wfe_qual,
                  ds_cpu0_irq_wfe_qual,
                  ds_cpu0_fiq_wfe_qual,
                  ds_cpu0_vimp_abrt_wfe_qual,
                  ds_cpu0_virq_wfe_qual,
                  ds_cpu0_vfiq_wfe_qual,
                  ds_cpu0_hcr_va,
                  ds_cpu0_hcr_vi,
                  ds_cpu0_hcr_vf,
                  ds_cpu0_cpuectlr_ret,
                  ck_cpu0_event_reg,
                  ck_cpu0_wfi_ack,
                  ck_cpu0_wfe_ack,
                  ck_cpu0_crcx_clk_en_n,
 
                  ds_cpu1_reset_req,
                  ds_cpu1_wfi_req,
                  ds_cpu1_wfe_req,
                  ds_cpu1_flush,
                  ds_cpu1_flush_type,
                  ds_cpu1_imp_abrt_wfi_qual,
                  ds_cpu1_irq_wfi_qual,
                  ds_cpu1_fiq_wfi_qual,
                  ds_cpu1_vimp_abrt_wfi_qual,
                  ds_cpu1_virq_wfi_qual,
                  ds_cpu1_vfiq_wfi_qual,
                  ds_cpu1_imp_abrt_wfe_qual,
                  ds_cpu1_irq_wfe_qual,
                  ds_cpu1_fiq_wfe_qual,
                  ds_cpu1_vimp_abrt_wfe_qual,
                  ds_cpu1_virq_wfe_qual,
                  ds_cpu1_vfiq_wfe_qual,
                  ds_cpu1_hcr_va,
                  ds_cpu1_hcr_vi,
                  ds_cpu1_hcr_vf,
                  ds_cpu1_cpuectlr_ret,
                  ck_cpu1_event_reg,
                  ck_cpu1_wfi_ack,
                  ck_cpu1_wfe_ack,
                  ck_cpu1_crcx_clk_en_n,
 
                  ds_cpu2_reset_req,
                  ds_cpu2_wfi_req,
                  ds_cpu2_wfe_req,
                  ds_cpu2_flush,
                  ds_cpu2_flush_type,
                  ds_cpu2_imp_abrt_wfi_qual,
                  ds_cpu2_irq_wfi_qual,
                  ds_cpu2_fiq_wfi_qual,
                  ds_cpu2_vimp_abrt_wfi_qual,
                  ds_cpu2_virq_wfi_qual,
                  ds_cpu2_vfiq_wfi_qual,
                  ds_cpu2_imp_abrt_wfe_qual,
                  ds_cpu2_irq_wfe_qual,
                  ds_cpu2_fiq_wfe_qual,
                  ds_cpu2_vimp_abrt_wfe_qual,
                  ds_cpu2_virq_wfe_qual,
                  ds_cpu2_vfiq_wfe_qual,
                  ds_cpu2_hcr_va,
                  ds_cpu2_hcr_vi,
                  ds_cpu2_hcr_vf,
                  ds_cpu2_cpuectlr_ret,
                  ck_cpu2_event_reg,
                  ck_cpu2_wfi_ack,
                  ck_cpu2_wfe_ack,
                  ck_cpu2_crcx_clk_en_n,
 
                  ds_cpu3_reset_req,
                  ds_cpu3_wfi_req,
                  ds_cpu3_wfe_req,
                  ds_cpu3_flush,
                  ds_cpu3_flush_type,
                  ds_cpu3_imp_abrt_wfi_qual,
                  ds_cpu3_irq_wfi_qual,
                  ds_cpu3_fiq_wfi_qual,
                  ds_cpu3_vimp_abrt_wfi_qual,
                  ds_cpu3_virq_wfi_qual,
                  ds_cpu3_vfiq_wfi_qual,
                  ds_cpu3_imp_abrt_wfe_qual,
                  ds_cpu3_irq_wfe_qual,
                  ds_cpu3_fiq_wfe_qual,
                  ds_cpu3_vimp_abrt_wfe_qual,
                  ds_cpu3_virq_wfe_qual,
                  ds_cpu3_vfiq_wfe_qual,
                  ds_cpu3_hcr_va,
                  ds_cpu3_hcr_vi,
                  ds_cpu3_hcr_vf,
                  ds_cpu3_cpuectlr_ret,
                  ck_cpu3_event_reg,
                  ck_cpu3_wfi_ack,
                  ck_cpu3_wfe_ack,
                  ck_cpu3_crcx_clk_en_n,
 
                  ls_cpu0_clrexmon,
                  ls_cpu1_clrexmon,
                  ls_cpu2_clrexmon,
                  ls_cpu3_clrexmon,
// END CK-CPU interface

                  ck_gclkt
);

//#
//# Interface Signals
//# =================
//#

//-----------------------------------------------------------------------------
// Clock and Reset Signals
//-----------------------------------------------------------------------------
  input              CLK;                            // Fast Clock
  input              CLKEN;                          // Fast Clock Enable

  input  [`MAIA_CN:0] nCPUPORESET;                    // CPU Power-on reset
  input  [`MAIA_CN:0] nCORERESET;                     // CPU reset (excluding DBG & ETM)
  input              nL2RESET;                       // L2 reset
  input              L2RSTDISABLE;                   // L2 RAMs hardware reset disable
  output [`MAIA_CN:0] WARMRSTREQ;                     // CPU Warm reset request
//See also           nPRESETDBG;                     // Debug APB reset (PCLK)

//-----------------------------------------------------------------------------
// Static Configuration Signals
//-----------------------------------------------------------------------------
// Static configuration signals that should be tied off and not change dynamically.
// Many of the initial values specified by these inputs
// may be overridden in software using CP15 registers.

  input  [`MAIA_CN:0] CFGEND;                         // Endianness EE bit (1:big endian)
  input  [`MAIA_CN:0] VINITHI;                        // 1: start up using high vectors
  input  [`MAIA_CN:0] CFGTE;                          // Exception handling state (0:ARM/1:Thumb)
  input  [`MAIA_CN:0] CP15SDISABLE;                   // Disable write access to some secure CP15 registers

  input  [7:0]       CLUSTERIDAFF1;                  // Value read in ClusterID Affinity1 field, MPIDR bits[15:8]
  input  [7:0]       CLUSTERIDAFF2;                  // Value read in ClusterID Affinity2 field, MPIDR bits[23:16]

  input  [`MAIA_CN:0] AA64nAA32;                      // Register Width (1:AArch64/0:AArch32)
  input  [43:2]      RVBARADDR0;                     // RVBAR address
// BEGIN INCLUDE FOR CPU1
  input  [43:2]      RVBARADDR1;                     // RVBAR address
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
  input  [43:2]      RVBARADDR2;                     // RVBAR address
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
  input  [43:2]      RVBARADDR3;                     // RVBAR address
// END INCLUDE FOR CPU3
  input  [`MAIA_CN:0] CRYPTODISABLE;                  // Disable Cryptography Extension

//-----------------------------------------------------------------------------
// Interrupt Controller Signals
//-----------------------------------------------------------------------------
  input  [`MAIA_CN:0] nFIQ;                           // Fast Interrupt request
  input  [`MAIA_CN:0] nIRQ;                           // Interrupt request
  input  [`MAIA_CN:0] nSEI;                           // System Error Interrupt
  input  [`MAIA_CN:0] nREI;                           // RAM Error Interrupt
  input  [`MAIA_CN:0] nVFIQ;                          // Virtual Fast Interrupt request
  input  [`MAIA_CN:0] nVIRQ;                          // Virtual Interrupt request
  input  [`MAIA_CN:0] nVSEI;                          // Virtual System Error Interrupt

// BEGIN NO-GIC pins
  output [`MAIA_CN:0] nVCPUMNTIRQ;                    // Virtual Maintenance Interrupt output
// END NO-GIC pins

  input  [43:18]     PERIPHBASE;                     // Base address for IC memory-mapped registers
// BEGIN NO-GIC pins
  input              GICCDISABLE;                    // Put GIC into bypass mode

  input              ICDTVALID;                      // Distrubuter AXI4 SP Message Valid 
  output             ICDTREADY;                      // GIC Ready for Distrubuter AXI4 SP Message      
  input  [15:0]      ICDTDATA;                       // Distrubuter AXI4 SP Message Data
  input              ICDTLAST;                       // Distrubuter AXI4 SP Message Last Packet
  input  [1:0]       ICDTDEST;                       // Distrubuter AXI4 SP Message CPU ID

  output             ICCTVALID;                      // GIC to Distributer AXI4 SP Message Valid
  input              ICCTREADY;                      // Distributer Ready for GIC AXI4 SP Message
  output [15:0]      ICCTDATA;                       // GIC to Distributer AXI4 SP Message Data
  output             ICCTLAST;                       // GIC to Distributer AXI4 SP Message Last Packet
  output [1:0]       ICCTID;                         // GIC to Distributer AXI4 SP Message CPU ID
// END NO-GIC pins

//-----------------------------------------------------------------------------
// Timer Signals
//-----------------------------------------------------------------------------
  input  [63:0]      CNTVALUEB;                      // Counter value in binary
  input              CNTCLKEN;                       // Counter clock enable
  output [`MAIA_CN:0] nCNTPNSIRQ;                     // NS Physical Timer event
  output [`MAIA_CN:0] nCNTPSIRQ;                      // S Physical Timer event
  output [`MAIA_CN:0] nCNTVIRQ;                       // Virtual Timer event
  output [`MAIA_CN:0] nCNTHPIRQ;                      // Hyp Physical Timer event

//-----------------------------------------------------------------------------
// Power Management Signals
//-----------------------------------------------------------------------------
  input              CLREXMONREQ;                    // Clearing of external global exclusive monitor (REQ)
  output             CLREXMONACK;                    // Clearing of external global exclusive monitor (ACK)
  input              EVENTI;                         // Event input for processor wake-up from WFE state
  output             EVENTO;                         // Event output, signal is active when SEV instruction is executed
  output [`MAIA_CN:0] STANDBYWFI;                     // WFI mode
  output [`MAIA_CN:0] STANDBYWFE;                     // WFE mode 
  output             STANDBYWFIL2;                   // WFI mode for L2
  output [`MAIA_CN:0] SMPEN;                          // CPU SMP bit

  output [`MAIA_CN:0] CPUQACTIVE;                     // CPU Q-channel QACTIVE
  input  [`MAIA_CN:0] CPUQREQn;                       // CPU Q-channel QREQn
  output [`MAIA_CN:0] CPUQACCEPTn;                    // CPU Q-channel QACCEPTn
  output [`MAIA_CN:0] CPUQDENY;                       // CPU Q-channel QDENY

  output             L2QACTIVE;                      // L2 Q-channel QACTIVE
  input              L2QREQn;                        // L2 Q-channel QREQn
  output             L2QACCEPTn;                     // L2 Q-channel QACCEPTn
  output             L2QDENY;                        // L2 Q-channel QDENY

  input              L2FLUSHREQ;                     // L2 hardware flush request
  output             L2FLUSHDONE;                    // L2 hardware flush done

//-----------------------------------------------------------------------------
// Asynchronous Error Signals
//-----------------------------------------------------------------------------
  output             nINTERRIRQ;                     // L2 RAM dbl-bit ECC error
  output             nEXTERRIRQ;                     // Write transaction error

//-----------------------------------------------------------------------------
// Bus Configuration Signals
//-----------------------------------------------------------------------------
  input              SYSBARDISABLE;                  // Disable broadcast of barriers
  input              BROADCASTINNER;                 // Extend Inner Shared Domain
  input              BROADCASTOUTER;                 // Extend Outer Shared Domain
  input              BROADCASTCACHEMAINT;                // Broadcast cache maint ops

//-----------------------------------------------------------------------------
// Skyros RN-F Interface
//-----------------------------------------------------------------------------
  input              SCLKEN;                         // Skyros clock enable
  input              SINACT;                         // Skyros snoop inactive

  input [6:0]        NODEID;                         // Skyros requestor NodeID

  output             TXSACTIVE;                      // Skyros active - indicates pending activity on pins
  input              RXSACTIVE;                      // Skyros active - indicates pending activity on pins

  output             TXLINKACTIVEREQ;                // Skyros transmit link active request
  input              TXLINKACTIVEACK;                // SKyros transmit link active acknowledge

  input              RXLINKACTIVEREQ;                // SKyros receive link active request
  output             RXLINKACTIVEACK;                // Skyros receive link active acknowledge

// TXREQ - outbound requests
  output             TXREQFLITPEND;                  // Skyros TXREQ FLIT pending
  output             TXREQFLITV;                     // Skyros TXREQ FLIT valid
  output [99:0]      TXREQFLIT;                      // Skyros TXREQ FLIT payload
  output [7:0]       REQMEMATTR;                     // Skyros TXREQ raw memory attributes
  input              TXREQLCRDV;                     // Skyros TXREQ link-layer credit valid

// TXRSP - outbound response
  output             TXRSPFLITPEND;                  // Skyros TXRSP FLIT pending
  output             TXRSPFLITV;                     // Skyros TXRSP FLIT valid                       
  output [44:0]      TXRSPFLIT;                      // Skyros TXRSP FLIT payload                     
  input              TXRSPLCRDV;                     // Skyros TXRSP link-layer credit valid          

// TXDAT - outbound data
  output             TXDATFLITPEND;                  // Skyros TXDAT FLIT pending
  output             TXDATFLITV;                     // Skyros TXDAT FLIT valid                       
  output [193:0]     TXDATFLIT;                      // Skyros TXDAT FLIT payload                     
  input              TXDATLCRDV;                     // Skyros TXDAT link-layer credit valid          

// RXSNP - inbound snoops
  input              RXSNPFLITPEND;                  // Skyros RXSNP FLIT pending
  input              RXSNPFLITV;                     // Skyros RXSNP FLIT valid                       
  input  [64:0]      RXSNPFLIT;                      // Skyros RXSNP FLIT payload                     
  output             RXSNPLCRDV;                     // Skyros RXSNP link-layer credit valid          

// RXRSP - inbound response
  input              RXRSPFLITPEND;                  // Skyros RXRSP FLIT pending
  input              RXRSPFLITV;                     // Skyros RXRSP FLIT valid                       
  input  [44:0]      RXRSPFLIT;                      // Skyros RXRSP FLIT payload                     
  output             RXRSPLCRDV;                     // Skyros RXRSP link-layer credit valid          

// RXDAT - inbound data
  input              RXDATFLITPEND;                  // Skyros RXDAT FLIT pending
  input              RXDATFLITV;                     // Skyros RXDAT FLIT valid                       
  input  [193:0]     RXDATFLIT;                      // Skyros RXDAT FLIT payload                     
  output             RXDATLCRDV;                     // Skyros RXDAT link-layer credit valid          

  input [43:24]      SAMMNBASE;                      // Skyros SAM MN base address
  input [1:0]        SAMADDRMAP0;                    // Skyros SAM address region 0 mapping
  input [1:0]        SAMADDRMAP1;                    // Skyros SAM address region 1 mapping
  input [1:0]        SAMADDRMAP2;                    // Skyros SAM address region 2 mapping
  input [1:0]        SAMADDRMAP3;                    // Skyros SAM address region 3 mapping
  input [1:0]        SAMADDRMAP4;                    // Skyros SAM address region 4 mapping
  input [1:0]        SAMADDRMAP5;                    // Skyros SAM address region 5 mapping
  input [1:0]        SAMADDRMAP6;                    // Skyros SAM address region 6 mapping
  input [1:0]        SAMADDRMAP7;                    // Skyros SAM address region 7 mapping
  input [1:0]        SAMADDRMAP8;                    // Skyros SAM address region 8 mapping
  input [1:0]        SAMADDRMAP9;                    // Skyros SAM address region 9 mapping
  input [1:0]        SAMADDRMAP10;                   // Skyros SAM address region 10 mapping
  input [1:0]        SAMADDRMAP11;                   // Skyros SAM address region 11 mapping
  input [1:0]        SAMADDRMAP12;                   // Skyros SAM address region 12 mapping
  input [1:0]        SAMADDRMAP13;                   // Skyros SAM address region 13 mapping
  input [1:0]        SAMADDRMAP14;                   // Skyros SAM address region 14 mapping
  input [1:0]        SAMADDRMAP15;                   // Skyros SAM address region 15 mapping
  input [1:0]        SAMADDRMAP16;                   // Skyros SAM address region 16 mapping
  input [1:0]        SAMADDRMAP17;                   // Skyros SAM address region 17 mapping
  input [1:0]        SAMADDRMAP18;                   // Skyros SAM address region 18 mapping
  input [1:0]        SAMADDRMAP19;                   // Skyros SAM address region 19 mapping
  input [6:0]        SAMMNNODEID;                    // Skyros SAM MN target ID
  input [6:0]        SAMHNI0NODEID;                  // Skyros SAM HNI0 target ID
  input [6:0]        SAMHNI1NODEID;                  // Skyros SAM HNI1 target ID
  input [6:0]        SAMHNF0NODEID;                  // Skyros SAM HNF0 target ID
  input [6:0]        SAMHNF1NODEID;                  // Skyros SAM HNF1 target ID
  input [6:0]        SAMHNF2NODEID;                  // Skyros SAM HNF2 target ID
  input [6:0]        SAMHNF3NODEID;                  // Skyros SAM HNF3 target ID
  input [6:0]        SAMHNF4NODEID;                  // Skyros SAM HNF4 target ID
  input [6:0]        SAMHNF5NODEID;                  // Skyros SAM HNF5 target ID
  input [6:0]        SAMHNF6NODEID;                  // Skyros SAM HNF6 target ID
  input [6:0]        SAMHNF7NODEID;                  // Skyros SAM HNF7 target ID
  input [2:0]        SAMHNFMODE;                     // Skyros SAM HNF interleaving mode

// BEGIN NO-ACP pins
//-----------------------------------------------------------------------------
// ACP AXI Slave
//-----------------------------------------------------------------------------
  input              ACLKENS;                        // AXI slave clock enable
  input              AINACTS;                        // AXI slave interface no longer active or accepting requests
// Write Address channel signals
  output             AWREADYS;                       // Write Address ready (slave ready to accept write address)
  input              AWVALIDS;                       // Write Address valid
  input  [4:0]       AWIDS;                          // Write Address ID
  input  [43:0]      AWADDRS;                        // Write Address
  input  [7:0]       AWLENS;                         // Write Burst Length
  input  [3:0]       AWCACHES;                       // Write Cache type
  input  [1:0]       AWUSERS;                        // Write inner & outer shareability
  input  [2:0]       AWPROTS;                        // Write Protection type

// Write Data channel signals
  output             WREADYS;                        // Write Data ready (slave ready to accept data)
  input              WVALIDS;                        // Write Data valid
  input  [127:0]     WDATAS;                         // Write Data
  input  [15:0]      WSTRBS;                         // Write byte-lane strobes
  input              WLASTS;                         // Write Data last transfer indicator

// Write Response channel signals
  input              BREADYS;                        // Write Response ready (master ready to accept response) 
  output             BVALIDS;                        // Write Response Valid
  output [4:0]       BIDS;                           // Write Response ID tag
  output [1:0]       BRESPS;                         // Write Response

// Read Address channel signals
  output             ARREADYS;                       // Read Address ready (slave ready to accept read address) 
  input              ARVALIDS;                       // Read Address valid
  input  [4:0]       ARIDS;                          // Read Address ID
  input  [43:0]      ARADDRS;                        // Read Address
  input  [7:0]       ARLENS;                         // Read Burst Length
  input  [3:0]       ARCACHES;                       // Read Cache type
  input  [1:0]       ARUSERS;                        // Read inner & outer shareability
  input  [2:0]       ARPROTS;                        // Read Protection type

// Read Data channel signals
  input              RREADYS;                        // Read Data ready (master ready to accept data) 
  output             RVALIDS;                        // Read Data valid
  output [4:0]       RIDS;                           // Read Data ID
  output [127:0]     RDATAS;                         // Read Data
  output [1:0]       RRESPS;                         // Read Data response
  output             RLASTS;                         // Read Data last transfer indicator
// END NO-ACP pins

//-----------------------------------------------------------------------------
// Debug/ETM/PMU Interface (CLK)
//-----------------------------------------------------------------------------
// Debug CLK interface
  input  [43:12]     DBGROMADDR;                     // Debug ROM base address
  input              DBGROMADDRV;                    // Debug ROM base address valid

  output [`MAIA_CN:0] DBGACK;                         // Debug acknowledge
  output [`MAIA_CN:0] nCOMMIRQ;                       // Comms channel receive/transmit interrupt
  output [`MAIA_CN:0] COMMRX;                         // Comms channel receive
  output [`MAIA_CN:0] COMMTX;                         // Comms channel transmit

  output [`MAIA_CN:0] DBGRSTREQ;                      // Warm reset request
  output [`MAIA_CN:0] DBGNOPWRDWN;                    // No power-down request

  input              DBGL1RSTDISABLE;                // L1 DCache hardware reset disable

// PMU CLK interface
  output [`MAIA_CN:0] nPMUIRQ;                        // PMU IRQ request
  output [24:0]      PMUEVENT0;                      // PMU Event bus
// BEGIN INCLUDE FOR CPU1
  output [24:0]      PMUEVENT1;                      // PMU Event bus
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
  output [24:0]      PMUEVENT2;                      // PMU Event bus
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
  output [24:0]      PMUEVENT3;                      // PMU Event bus
// END INCLUDE FOR CPU3

//-----------------------------------------------------------------------------
// Debug/ETM/PMU Interface (ATCLK)
//-----------------------------------------------------------------------------
// ETM ATB interface and Misc signals
  input              ATCLKEN;                        // ATB Clock Enable
  input  [63:0]      TSVALUEB;                       // ATB Timestamp in binary

  input              ATREADYM0;                      // ATDATA can be accepted
  input              AFVALIDM0;                      // ATB Fifo Flush Request
  output [31:0]      ATDATAM0;                       // ATB Data
  output             ATVALIDM0;                      // ATB Data Valid
  output [1:0]       ATBYTESM0;                      // ATB Data Size
  output             AFREADYM0;                      // ATB Fifo Flush Finished
  output [6:0]       ATIDM0;                         // ATB Trace Source ID
  input              SYNCREQM0;                      // ATB External synchronization request

// BEGIN INCLUDE FOR CPU1
  input              ATREADYM1;                      // ATDATA can be accepted
  input              AFVALIDM1;                      // ATB Fifo Flush Request
  output [31:0]      ATDATAM1;                       // ATB Data
  output             ATVALIDM1;                      // ATB Data Valid
  output [1:0]       ATBYTESM1;                      // ATB Data Size
  output             AFREADYM1;                      // ATB Fifo Flush Finished
  output [6:0]       ATIDM1;                         // ATB Trace Source ID
  input              SYNCREQM1;                      // ATB External synchronization request
// END INCLUDE FOR CPU1

// BEGIN INCLUDE FOR CPU2
  input              ATREADYM2;                      // ATDATA can be accepted
  input              AFVALIDM2;                      // ATB Fifo Flush Request
  output [31:0]      ATDATAM2;                       // ATB Data
  output             ATVALIDM2;                      // ATB Data Valid
  output [1:0]       ATBYTESM2;                      // ATB Data Size
  output             AFREADYM2;                      // ATB Fifo Flush Finished
  output [6:0]       ATIDM2;                         // ATB Trace Source ID
  input              SYNCREQM2;                      // ATB External synchronization request
// END INCLUDE FOR CPU2

// BEGIN INCLUDE FOR CPU3
  input              ATREADYM3;                      // ATDATA can be accepted
  input              AFVALIDM3;                      // ATB Fifo Flush Request
  output [31:0]      ATDATAM3;                       // ATB Data
  output             ATVALIDM3;                      // ATB Data Valid
  output [1:0]       ATBYTESM3;                      // ATB Data Size
  output             AFREADYM3;                      // ATB Fifo Flush Finished
  output [6:0]       ATIDM3;                         // ATB Trace Source ID
  input              SYNCREQM3;                      // ATB External synchronization request
// END INCLUDE FOR CPU3

//-----------------------------------------------------------------------------
// Debug/ETM/PMU Interface (PCLK)
//-----------------------------------------------------------------------------
// Debug-APBv3 port (APB)
  input              PCLKDBG;                        // APB Clock
  input              PCLKENDBG;                      // APB Clock Enable
  input              nPRESETDBG;                     // APB Reset
  input              PSELDBG;                        // Debug bus access
  input  [21:2]      PADDRDBG;                       // APB address
  input              PADDRDBG31;                     // APB address bit[31]
  input              PENABLEDBG;                     // APB transfer complete flag
  input              PWRITEDBG;                      // APB read/write indicator
  input  [31:0]      PWDATADBG;                      // APB write data
  output [31:0]      PRDATADBG;                      // APB read data
  output             PREADYDBG;                      // APB slave ready, used to extend a transfer
  output             PSLVERRDBG;                     // APB slave transfer error

// Misc interface
  input  [`MAIA_CN:0] EDBGRQ;                         // External debug request

// PMU Snapshot interface
  input  [`MAIA_CN:0] PMUSNAPSHOTREQ;                 // PMU snapshot trigger request
  output [`MAIA_CN:0] PMUSNAPSHOTACK;                 // PMU snapshot trigger acknowledge

// Power-related interface
  input  [`MAIA_CN:0] DBGPWRDUP;                      // Processor power-up status
  output [`MAIA_CN:0] DBGPWRUPREQ;                    // Processor power-up request

// CTI interface
  input  [3:0]       CTICHIN;                        // Channel In
  input  [3:0]       CTICHOUTACK;                    // Channel Out acknowledge
  output [3:0]       CTICHOUT;                       // Channel Out
  output [3:0]       CTICHINACK;                     // Channel In acknowledge
  input              CISBYPASS;                      // Channel interface sync bypass
  input  [3:0]       CIHSBYPASS;                     // Channel interface H/S bypass
  output [`MAIA_CN:0] CTIIRQ;                         // CTI Interrupt
  input  [`MAIA_CN:0] CTIIRQACK;                      // CTI Interrupt acknowledge

//-----------------------------------------------------------------------------
// Debug Authentication Interface (CLK & PCLK)
//-----------------------------------------------------------------------------
  input  [`MAIA_CN:0] DBGEN;                          // Invasive debug enable
  input  [`MAIA_CN:0] NIDEN;                          // Non-invasive debug enable
  input  [`MAIA_CN:0] SPIDEN;                         // Secure Priviledge invasive debug enable
  input  [`MAIA_CN:0] SPNIDEN;                        // Secure Priviledge non-invasive debug enable

//-----------------------------------------------------------------------------
// DFT Signals
//-----------------------------------------------------------------------------
  input              DFTSE;                          // Scan enable
  input              DFTRSTDISABLE;                  // Disable reset to cells during scan shift
  input  [`MAIA_CN:0] DFTCRCLKDISABLE;                // Clock grid control for ck_gclkcr
  input              DFTL2CLKDISABLE;                // Clock grid control for ck_gclkl2
  input              DFTRAMHOLD;                     // Holds data in RAMs
  input              DFTCLKBYPASS;                   // L2 RAM strobe clock bypass
  input              DFTMCPHOLD;                     // Disable multi-cycle RAM paths

//-----------------------------------------------------------------------------
// MBIST Interface
//-----------------------------------------------------------------------------
  input              nMBISTRESET;                    // MBIST reset
  input              MBISTREQ;                       // MBIST mode request

//-----------------------------------------------------------------------------
// Signals from maia -> maia_cpu_io -> maia_cpu
//-----------------------------------------------------------------------------
// Outputs to maia_cpu
  output         ncpuporeset_cpu0_o;
  output         ncorereset_cpu0_o;

  output         cfgend_cpu0_o;
  output         cfgte_cpu0_o;
  output         cp15sdisable_cpu0_o;
  output         vinithi_cpu0_o;
  output [7:0]   clusteridaff1_cpu0_o;
  output [7:0]   clusteridaff2_cpu0_o;
  output [1:0]   cpuid_cpu0_o;
  output         aa64naa32_cpu0_o;
  output [43:2]  rvbaraddr_cpu0_o;
  output         cryptodisable_cpu0_o;
  output         giccdisable_cpu0_o;

  output [43:12] dbgromaddr_cpu0_o;
  output         dbgromaddrv_cpu0_o;
  output         dbgl1rstdisable_cpu0_o;

  output         dbgen_cpu0_o;
  output         niden_cpu0_o;
  output         spiden_cpu0_o;
  output         spniden_cpu0_o;

  output [63:0]  tsvalueb_cpu0_o;

  output         atclken_cpu0_o;
  output         afvalidm_cpu0_o;
  output         atreadym_cpu0_o;
  output         syncreqm_cpu0_o;

  output         dftse_cpu0_o;
  output         dftrstdisable_cpu0_o;
  output         dftcrclkdisable_cpu0_o;
  output         dftramhold_cpu0_o;
  output         nmbistreset_cpu0_o;

// BEGIN INCLUDE FOR CPU1
  output         ncpuporeset_cpu1_o;
  output         ncorereset_cpu1_o;

  output         cfgend_cpu1_o;
  output         cfgte_cpu1_o;
  output         cp15sdisable_cpu1_o;
  output         vinithi_cpu1_o;
  output [7:0]   clusteridaff1_cpu1_o;
  output [7:0]   clusteridaff2_cpu1_o;
  output [1:0]   cpuid_cpu1_o;
  output         aa64naa32_cpu1_o;
  output [43:2]  rvbaraddr_cpu1_o;
  output         cryptodisable_cpu1_o;
  output         giccdisable_cpu1_o;

  output [43:12] dbgromaddr_cpu1_o;
  output         dbgromaddrv_cpu1_o;
  output         dbgl1rstdisable_cpu1_o;

  output         dbgen_cpu1_o;
  output         niden_cpu1_o;
  output         spiden_cpu1_o;
  output         spniden_cpu1_o;

  output [63:0]  tsvalueb_cpu1_o;

  output         atclken_cpu1_o;
  output         afvalidm_cpu1_o;
  output         atreadym_cpu1_o;
  output         syncreqm_cpu1_o;

  output         dftse_cpu1_o;
  output         dftrstdisable_cpu1_o;
  output         dftcrclkdisable_cpu1_o;
  output         dftramhold_cpu1_o;
  output         nmbistreset_cpu1_o;
// END INCLUDE FOR CPU1

// BEGIN INCLUDE FOR CPU2
  output         ncpuporeset_cpu2_o;
  output         ncorereset_cpu2_o;

  output         cfgend_cpu2_o;
  output         cfgte_cpu2_o;
  output         cp15sdisable_cpu2_o;
  output         vinithi_cpu2_o;
  output [7:0]   clusteridaff1_cpu2_o;
  output [7:0]   clusteridaff2_cpu2_o;
  output [1:0]   cpuid_cpu2_o;
  output         aa64naa32_cpu2_o;
  output [43:2]  rvbaraddr_cpu2_o;
  output         cryptodisable_cpu2_o;
  output         giccdisable_cpu2_o;

  output [43:12] dbgromaddr_cpu2_o;
  output         dbgromaddrv_cpu2_o;
  output         dbgl1rstdisable_cpu2_o;

  output         dbgen_cpu2_o;
  output         niden_cpu2_o;
  output         spiden_cpu2_o;
  output         spniden_cpu2_o;

  output [63:0]  tsvalueb_cpu2_o;

  output         atclken_cpu2_o;
  output         afvalidm_cpu2_o;
  output         atreadym_cpu2_o;
  output         syncreqm_cpu2_o;

  output         dftse_cpu2_o;
  output         dftrstdisable_cpu2_o;
  output         dftcrclkdisable_cpu2_o;
  output         dftramhold_cpu2_o;
  output         nmbistreset_cpu2_o;
// END INCLUDE FOR CPU2

// BEGIN INCLUDE FOR CPU3
  output         ncpuporeset_cpu3_o;
  output         ncorereset_cpu3_o;

  output         cfgend_cpu3_o;
  output         cfgte_cpu3_o;
  output         cp15sdisable_cpu3_o;
  output         vinithi_cpu3_o;
  output [7:0]   clusteridaff1_cpu3_o;
  output [7:0]   clusteridaff2_cpu3_o;
  output [1:0]   cpuid_cpu3_o;
  output         aa64naa32_cpu3_o;
  output [43:2]  rvbaraddr_cpu3_o;
  output         cryptodisable_cpu3_o;
  output         giccdisable_cpu3_o;

  output [43:12] dbgromaddr_cpu3_o;
  output         dbgromaddrv_cpu3_o;
  output         dbgl1rstdisable_cpu3_o;

  output         dbgen_cpu3_o;
  output         niden_cpu3_o;
  output         spiden_cpu3_o;
  output         spniden_cpu3_o;

  output [63:0]  tsvalueb_cpu3_o;

  output         atclken_cpu3_o;
  output         afvalidm_cpu3_o;
  output         atreadym_cpu3_o;
  output         syncreqm_cpu3_o;

  output         dftse_cpu3_o;
  output         dftrstdisable_cpu3_o;
  output         dftcrclkdisable_cpu3_o;
  output         dftramhold_cpu3_o;
  output         nmbistreset_cpu3_o;
// END INCLUDE FOR CPU3

//-----------------------------------------------------------------------------
// Signals from maia_cpu -> maia_cpu_io -> maia
//-----------------------------------------------------------------------------
// Inputs from maia_cpu
  input          ds_cpu0_sev_req;
  input          ds_cpu0_sevl_req;
  input          ds_cpu0_cpuectlr_smp;

  input          ncommirq_cpu0_i;
  input          commrx_cpu0_i;
  input          commtx_cpu0_i;
  input          dbgack_cpu0_i;
  input          dbgrstreq_cpu0_i;
  input          dbgnopwrdwn_cpu0_i;

  input          npmuirq_cpu0_i;
  input  [24:0]  pmuevent_cpu0_i;
  input          pm_export_cpu0_i;

  input          etclken_cpu0_i;
  input          afreadym_cpu0_i;
  input  [1:0]   atbytesm_cpu0_i;
  input  [31:0]  atdatam_cpu0_i;
  input  [6:0]   atidm_cpu0_i;
  input          atvalidm_cpu0_i;

// BEGIN INCLUDE FOR CPU1
  input          ds_cpu1_sev_req;
  input          ds_cpu1_sevl_req;
  input          ds_cpu1_cpuectlr_smp;

  input          ncommirq_cpu1_i;
  input          commrx_cpu1_i;
  input          commtx_cpu1_i;
  input          dbgack_cpu1_i;
  input          dbgrstreq_cpu1_i;
  input          dbgnopwrdwn_cpu1_i;

  input          npmuirq_cpu1_i;
  input  [24:0]  pmuevent_cpu1_i;
  input          pm_export_cpu1_i;

  input          etclken_cpu1_i;
  input          afreadym_cpu1_i;
  input  [1:0]   atbytesm_cpu1_i;
  input  [31:0]  atdatam_cpu1_i;
  input  [6:0]   atidm_cpu1_i;
  input          atvalidm_cpu1_i;
// END INCLUDE FOR CPU1

// BEGIN INCLUDE FOR CPU2
  input          ds_cpu2_sev_req;
  input          ds_cpu2_sevl_req;
  input          ds_cpu2_cpuectlr_smp;

  input          ncommirq_cpu2_i;
  input          commrx_cpu2_i;
  input          commtx_cpu2_i;
  input          dbgack_cpu2_i;
  input          dbgrstreq_cpu2_i;
  input          dbgnopwrdwn_cpu2_i;

  input          npmuirq_cpu2_i;
  input  [24:0]  pmuevent_cpu2_i;
  input          pm_export_cpu2_i;

  input          etclken_cpu2_i;
  input          afreadym_cpu2_i;
  input  [1:0]   atbytesm_cpu2_i;
  input  [31:0]  atdatam_cpu2_i;
  input  [6:0]   atidm_cpu2_i;
  input          atvalidm_cpu2_i;
// END INCLUDE FOR CPU2

// BEGIN INCLUDE FOR CPU3
  input          ds_cpu3_sev_req;
  input          ds_cpu3_sevl_req;
  input          ds_cpu3_cpuectlr_smp;

  input          ncommirq_cpu3_i;
  input          commrx_cpu3_i;
  input          commtx_cpu3_i;
  input          dbgack_cpu3_i;
  input          dbgrstreq_cpu3_i;
  input          dbgnopwrdwn_cpu3_i;

  input          npmuirq_cpu3_i;
  input  [24:0]  pmuevent_cpu3_i;
  input          pm_export_cpu3_i;

  input          etclken_cpu3_i;
  input          afreadym_cpu3_i;
  input  [1:0]   atbytesm_cpu3_i;
  input  [31:0]  atdatam_cpu3_i;
  input  [6:0]   atidm_cpu3_i;
  input          atvalidm_cpu3_i;
// END INCLUDE FOR CPU3

//-----------------------------------------------------------------------------
// L2 interface
//-----------------------------------------------------------------------------
  output [12:0]  l2_cpu0_mbist1_addr_b1;
  output [3:0]   l2_cpu0_mbist1_array_b1;
  output [7:0]   l2_cpu0_mbist1_be_b1;
  output         l2_cpu0_mbist1_en_b1;
  output         l2_cpu0_mbist1_rd_en_b1;
  output         l2_cpu0_mbist1_wr_en_b1;
  output         l2_cpu0_mbist1_all_b1;

// BEGIN INCLUDE FOR CPU1
  output [12:0]  l2_cpu1_mbist1_addr_b1;
  output [3:0]   l2_cpu1_mbist1_array_b1;
  output [7:0]   l2_cpu1_mbist1_be_b1;
  output         l2_cpu1_mbist1_en_b1;
  output         l2_cpu1_mbist1_rd_en_b1;
  output         l2_cpu1_mbist1_wr_en_b1;
  output         l2_cpu1_mbist1_all_b1;
// END INCLUDE FOR CPU1
                         
// BEGIN INCLUDE FOR CPU2
  output [12:0]  l2_cpu2_mbist1_addr_b1;
  output [3:0]   l2_cpu2_mbist1_array_b1;
  output [7:0]   l2_cpu2_mbist1_be_b1;
  output         l2_cpu2_mbist1_en_b1;
  output         l2_cpu2_mbist1_rd_en_b1;
  output         l2_cpu2_mbist1_wr_en_b1;
  output         l2_cpu2_mbist1_all_b1;
// END INCLUDE FOR CPU2
                         
// BEGIN INCLUDE FOR CPU3
  output [12:0]  l2_cpu3_mbist1_addr_b1;
  output [3:0]   l2_cpu3_mbist1_array_b1;
  output [7:0]   l2_cpu3_mbist1_be_b1;
  output         l2_cpu3_mbist1_en_b1;
  output         l2_cpu3_mbist1_rd_en_b1;
  output         l2_cpu3_mbist1_wr_en_b1;
  output         l2_cpu3_mbist1_all_b1;
// END INCLUDE FOR CPU3

// BEGIN L2-CPU interface

//-----------------------------------------------------------------------------
// ucpu0
//-----------------------------------------------------------------------------
  output         l2_cpu0_cfg_ecc_en;               
  output         l2_cpu0_arb_thrshld_timeout_en;   
  output         l2_cpu0_disable_clean_evict_opt;  
  output         l2_cpu0_dext_err_r2;                 // LS external error
  output         l2_cpu0_dext_err_type_r2;            // LS external error type
  output         l2_cpu0_dsngl_ecc_err_r3;            // LS single-bit ecc error
  output         l2_cpu0_ddbl_ecc_err_r3;             // LS double-bit ecc error
  output [129:0] l2_cpu0_ddata_r2;                    // LS read data
  output         l2_cpu0_barrier_done;                // LS barrier complete
  output         l2_cpu0_spec_valid;                  // LS read speculative response valid
  output [2:0]   l2_cpu0_spec_bufid;                  // LS read speculative response buffer id
  output         l2_cpu0_rvalid;                      // LS read response valid
  output [1:0]   l2_cpu0_rstate;                      // LS read response state
  output         l2_cpu0_rexfail;                     // LS read response exclusive fail
  output [2:0]   l2_cpu0_rbufid;                      // LS read response buffer id
  output         l2_cpu0_dvalid_r1;                   // LS read data valid
  output         l2_cpu0_dlast_r1;                    // LS read last indicator
  output [2:0]   l2_cpu0_dbufid_r1;                   // LS read data fill buffer id
  output         l2_cpu0_iext_err_r2;                 // IF external error
  output         l2_cpu0_iext_err_type_r2;            // IF external error type
  output         l2_cpu0_isngl_ecc_err_r3;            // IF single-bit ecc error
  output         l2_cpu0_idbl_ecc_err_r3;             // IF double-bit ecc error
  output [127:0] l2_cpu0_idata_r2;                    // IF read data
  output         l2_cpu0_ivalid_r1;                   // IF read data valid
  output [1:0]   l2_cpu0_ibufid_r1;                   // IF read data fill buffer id
  output         l2_cpu0_ls_sync_req;                 // LS sync req
  output [48:0]  l2_cpu0_ccb_req_addr_c3;             // LS/IF/TLB ccb req addr
  output         l2_cpu0_ccb_dbg_req_c3;              // CCB req is a dbg array rd
  output         l2_cpu0_ls_ccb_clken_c3;             // LS ccb clken
  output         l2_cpu0_ls_ccb_req_c3;               // LS ccb req
  output [4:0]   l2_cpu0_ccb_req_id_c3;               // LS ccb req id
  output [8:0]   l2_cpu0_ccb_req_type_c3;             // LS ccb req type
  output [23:0]  l2_cpu0_ccb_req_info_c3;             // LS ccb req info
  output         l2_cpu0_if_ccb_clken_c3;             // IF ccb clken
  output         l2_cpu0_if_ccb_req_c3;               // IF ccb req
  output         l2_cpu0_if_sync_req;                 // IF sync req
  output         l2_cpu0_tlb_ccb_clken_c3;            // TLB ccb clken
  output         l2_cpu0_tlb_ccb_req_c3;              // TLB ccb req
  output         l2_cpu0_tlb_sync_req;                // TLB sync req 
  output         l2_cpu0_tlb_sync_complete;           // TLB sync complete
  output         l2_cpu0_tbw_desc_vld;                // TBW descriptor valid
  output         l2_cpu0_tbw_ext_err;                 // TBW descriptor external error
  output         l2_cpu0_tbw_ext_err_type;            // TBW descriptor external error type
  output         l2_cpu0_tbw_dbl_ecc_err;             // TBW descriptor double-bit ecc error
  output [63:0]  l2_cpu0_tbw_desc_data;               // TBW descriptor data
  output [63:0]  l2_cpu0_spr_rd_data;                 // DS spr read data
  output [1:0]   l2_cpu0_l2_cache_size;               // DS L2 cache size
  output         l2_cpu0_pf_throttle_q;               // PF throttling

  output         l2_cpu0_wr_ex_resp;                  // store exclusive response
  output         l2_cpu0_wr_ex_fail;                  // store exclusive failed

  output [43:18] l2_cpu0_ic_base;                     // PERIPHBASE 
  output         l2_cpu0_no_intctrl;                  // INTCTLR not present 


  output [33:0]  l2_cpu0_pmu_events;                  // L2 PMU events 

  input          ds_cpu0_l2_spr_en;                   // cpu0 early spr req for clk enables
  input          ds_cpu0_l2_spr_rd;                   // cpu0 spr read op
  input          ds_cpu0_l2_spr_wr;                   // cpu0 spr write op
  input  [8:0]   ds_cpu0_l2_spr_addr;                 // cpu0 spr address
  input          ds_cpu0_l2_spr_dw;                   // cpu0 spr access dw
  input  [63:0]  ds_cpu0_l2_spr_wr_data;              // cpu0 spr write data

  input          l2_cpu0_wr_data_vld_x1_q;            // cpu0 write data vld x1 stage
  input          l2_cpu0_wr_evict_x1_q;               // cpu0 write evict x1 stage
  input  [143:0] l2_cpu0_wr_data;                    
  input          l2_cpu0_ls_rd_haz_vld_arb_q;        
  input          l2_cpu0_ls_wr_haz_vld_arb_q;        
  input          l2_cpu0_dt_pmu_evt_en;               // PMU enabled.

   
//-----------------------------------------------------------------------------
// ucpu1
//-----------------------------------------------------------------------------
  output         l2_cpu1_cfg_ecc_en;               
  output         l2_cpu1_arb_thrshld_timeout_en;   
  output         l2_cpu1_disable_clean_evict_opt;  
  output         l2_cpu1_dext_err_r2;                 // LS external error
  output         l2_cpu1_dext_err_type_r2;            // LS external error type
  output         l2_cpu1_dsngl_ecc_err_r3;            // LS single-bit ecc error
  output         l2_cpu1_ddbl_ecc_err_r3;             // LS double-bit ecc error
  output [129:0] l2_cpu1_ddata_r2;                    // LS read data
  output         l2_cpu1_barrier_done;                // LS barrier complete
  output         l2_cpu1_spec_valid;                  // LS read speculative response valid
  output [2:0]   l2_cpu1_spec_bufid;                  // LS read speculative response buffer id
  output         l2_cpu1_rvalid;                      // LS read response valid
  output [1:0]   l2_cpu1_rstate;                      // LS read response state
  output         l2_cpu1_rexfail;                     // LS read response exclusive fail
  output [2:0]   l2_cpu1_rbufid;                      // LS read response buffer id
  output         l2_cpu1_dvalid_r1;                   // LS read data valid
  output         l2_cpu1_dlast_r1;                    // LS read last indicator
  output [2:0]   l2_cpu1_dbufid_r1;                   // LS read data fill buffer id
  output         l2_cpu1_iext_err_r2;                 // IF external error
  output         l2_cpu1_iext_err_type_r2;            // IF external error type
  output         l2_cpu1_isngl_ecc_err_r3;            // IF single-bit ecc error
  output         l2_cpu1_idbl_ecc_err_r3;             // IF double-bit ecc error
  output [127:0] l2_cpu1_idata_r2;                    // IF read data
  output         l2_cpu1_ivalid_r1;                   // IF read data valid
  output [1:0]   l2_cpu1_ibufid_r1;                   // IF read data fill buffer id
  output         l2_cpu1_ls_sync_req;                 // LS sync req
  output [48:0]  l2_cpu1_ccb_req_addr_c3;             // LS/IF/TLB ccb req addr
  output         l2_cpu1_ccb_dbg_req_c3;              // CCB req is a dbg array rd
  output         l2_cpu1_ls_ccb_clken_c3;             // LS ccb clken
  output         l2_cpu1_ls_ccb_req_c3;               // LS ccb req
  output [4:0]   l2_cpu1_ccb_req_id_c3;               // LS ccb req id
  output [8:0]   l2_cpu1_ccb_req_type_c3;             // LS ccb req type
  output [23:0]  l2_cpu1_ccb_req_info_c3;             // LS ccb req info
  output         l2_cpu1_if_ccb_clken_c3;             // IF ccb clken
  output         l2_cpu1_if_ccb_req_c3;               // IF ccb req
  output         l2_cpu1_if_sync_req;                 // IF sync req
  output         l2_cpu1_tlb_ccb_clken_c3;            // IF ccb clken
  output         l2_cpu1_tlb_ccb_req_c3;              // TLB ccb req
  output         l2_cpu1_tlb_sync_req;                // TLB sync req 
  output         l2_cpu1_tlb_sync_complete;           // TLB sync complete
  output         l2_cpu1_tbw_desc_vld;                // TBW descriptor valid
  output         l2_cpu1_tbw_ext_err;                 // TBW descriptor external error
  output         l2_cpu1_tbw_ext_err_type;            // TBW descriptor external error type
  output         l2_cpu1_tbw_dbl_ecc_err;             // TBW descriptor double-bit ecc error
  output [63:0]  l2_cpu1_tbw_desc_data;               // TBW descriptor data
  output [63:0]  l2_cpu1_spr_rd_data;                 // DS spr read data
  output [1:0]   l2_cpu1_l2_cache_size;               // DS L2 cache size
  output         l2_cpu1_pf_throttle_q;               // PF throttling
  
  output         l2_cpu1_wr_ex_resp;                  // store exclusive response
  output         l2_cpu1_wr_ex_fail;                  // store exclusive failed

  output [43:18] l2_cpu1_ic_base;                     // PERIPHBASE 
  output         l2_cpu1_no_intctrl;                  // INTCTLR not present 

  output [33:0]  l2_cpu1_pmu_events;                  // L2 PMU events 

  input          ds_cpu1_l2_spr_en;                   // cpu1 early spr req for clk enables
  input          ds_cpu1_l2_spr_rd;                   // cpu1 spr read op
  input          ds_cpu1_l2_spr_wr;                   // cpu1 spr write op
  input  [8:0]   ds_cpu1_l2_spr_addr;                 // cpu1 spr address
  input          ds_cpu1_l2_spr_dw;                   // cpu1 spr access dw
  input  [63:0]  ds_cpu1_l2_spr_wr_data;              // cpu1 spr write data

  input          l2_cpu1_wr_data_vld_x1_q;            // cpu1 write data vld x1 stage
  input          l2_cpu1_wr_evict_x1_q;               // cpu1 write evict x1 stage
  input  [143:0] l2_cpu1_wr_data;                    
  input          l2_cpu1_ls_rd_haz_vld_arb_q;        
  input          l2_cpu1_ls_wr_haz_vld_arb_q;        
  input          l2_cpu1_dt_pmu_evt_en;               // PMU enabled.

//-----------------------------------------------------------------------------
// ucpu2
//-----------------------------------------------------------------------------
  output         l2_cpu2_cfg_ecc_en;               
  output         l2_cpu2_arb_thrshld_timeout_en;   
  output         l2_cpu2_disable_clean_evict_opt;  
  output         l2_cpu2_dext_err_r2;                 // LS external error
  output         l2_cpu2_dext_err_type_r2;            // LS external error type
  output         l2_cpu2_dsngl_ecc_err_r3;            // LS single-bit ecc error
  output         l2_cpu2_ddbl_ecc_err_r3;             // LS double-bit ecc error
  output [129:0] l2_cpu2_ddata_r2;                    // LS read data
  output         l2_cpu2_barrier_done;                // LS barrier complete
  output         l2_cpu2_spec_valid;                  // LS read speculative response valid
  output [2:0]   l2_cpu2_spec_bufid;                  // LS read speculative response buffer id
  output         l2_cpu2_rvalid;                      // LS read response valid
  output [1:0]   l2_cpu2_rstate;                      // LS read response state
  output         l2_cpu2_rexfail;                     // LS read response exclusive fail
  output [2:0]   l2_cpu2_rbufid;                      // LS read response buffer id
  output         l2_cpu2_dvalid_r1;                   // LS read data valid
  output         l2_cpu2_dlast_r1;                    // LS read last indicator
  output [2:0]   l2_cpu2_dbufid_r1;                   // LS read data fill buffer id
  output         l2_cpu2_iext_err_r2;                 // IF external error
  output         l2_cpu2_iext_err_type_r2;            // IF external error type
  output         l2_cpu2_isngl_ecc_err_r3;            // IF single-bit ecc error
  output         l2_cpu2_idbl_ecc_err_r3;             // IF double-bit ecc error
  output [127:0] l2_cpu2_idata_r2;                    // IF read data
  output         l2_cpu2_ivalid_r1;                   // IF read data valid
  output [1:0]   l2_cpu2_ibufid_r1;                   // IF read data fill buffer id
  output         l2_cpu2_ls_sync_req;                 // LS sync req
  output [48:0]  l2_cpu2_ccb_req_addr_c3;             // LS/IF/TLB ccb req addr
  output         l2_cpu2_ccb_dbg_req_c3;              // CCB req is a dbg array rd
  output         l2_cpu2_ls_ccb_clken_c3;             // LS ccb clken
  output         l2_cpu2_ls_ccb_req_c3;               // LS ccb req
  output [4:0]   l2_cpu2_ccb_req_id_c3;               // LS ccb req id
  output [8:0]   l2_cpu2_ccb_req_type_c3;             // LS ccb req type
  output [23:0]  l2_cpu2_ccb_req_info_c3;             // LS ccb req info
  output         l2_cpu2_if_ccb_clken_c3;             // IF ccb clken
  output         l2_cpu2_if_ccb_req_c3;               // IF ccb req
  output         l2_cpu2_if_sync_req;                 // IF sync req
  output         l2_cpu2_tlb_ccb_clken_c3;            // TLB ccb clken
  output         l2_cpu2_tlb_ccb_req_c3;              // TLB ccb req
  output         l2_cpu2_tlb_sync_req;                // TLB sync req 
  output         l2_cpu2_tlb_sync_complete;           // TLB sync complete
  output         l2_cpu2_tbw_desc_vld;                // TBW descriptor valid
  output         l2_cpu2_tbw_ext_err;                 // TBW descriptor external error
  output         l2_cpu2_tbw_ext_err_type;            // TBW descriptor external error type
  output         l2_cpu2_tbw_dbl_ecc_err;             // TBW descriptor double-bit ecc error
  output [63:0]  l2_cpu2_tbw_desc_data;               // TBW descriptor data
  output [63:0]  l2_cpu2_spr_rd_data;                 // DS spr read data
  output [1:0]   l2_cpu2_l2_cache_size;               // DS L2 cache size
  output         l2_cpu2_pf_throttle_q;               // PF throttling

  output         l2_cpu2_wr_ex_resp;                  // store exclusive response
  output         l2_cpu2_wr_ex_fail;                  // store exclusive failed

  output [43:18] l2_cpu2_ic_base;                     // PERIPHBASE 
  output         l2_cpu2_no_intctrl;                  // INTCTLR not present 

  output [33:0]  l2_cpu2_pmu_events;                  // L2 PMU events 

  input          ds_cpu2_l2_spr_en;                   // cpu2 early spr req for clk enables
  input          ds_cpu2_l2_spr_rd;                   // cpu2 spr read op
  input          ds_cpu2_l2_spr_wr;                   // cpu2 spr write op
  input  [8:0]   ds_cpu2_l2_spr_addr;                 // cpu2 spr address
  input          ds_cpu2_l2_spr_dw;                   // cpu2 spr access dw
  input  [63:0]  ds_cpu2_l2_spr_wr_data;              // cpu2 spr write data

  input          l2_cpu2_wr_data_vld_x1_q;            // cpu2 write data vld x1 stage
  input          l2_cpu2_wr_evict_x1_q;               // cpu2 write evict x1 stage
  input  [143:0] l2_cpu2_wr_data;                    
  input          l2_cpu2_ls_rd_haz_vld_arb_q;        
  input          l2_cpu2_ls_wr_haz_vld_arb_q;        
  input          l2_cpu2_dt_pmu_evt_en;               // PMU enabled.

//-----------------------------------------------------------------------------
// ucpu3
//-----------------------------------------------------------------------------
  output         l2_cpu3_cfg_ecc_en;               
  output         l2_cpu3_arb_thrshld_timeout_en;   
  output         l2_cpu3_disable_clean_evict_opt;  
  output         l2_cpu3_dext_err_r2;                 // LS external error
  output         l2_cpu3_dext_err_type_r2;            // LS external error type
  output         l2_cpu3_dsngl_ecc_err_r3;            // LS single-bit ecc error
  output         l2_cpu3_ddbl_ecc_err_r3;             // LS double-bit ecc error
  output [129:0] l2_cpu3_ddata_r2;                    // LS read data
  output         l2_cpu3_barrier_done;                // LS barrier complete
  output         l2_cpu3_spec_valid;                  // LS read speculative response valid
  output [2:0]   l2_cpu3_spec_bufid;                  // LS read speculative response buffer id
  output         l2_cpu3_rvalid;                      // LS read response valid
  output [1:0]   l2_cpu3_rstate;                      // LS read response state
  output         l2_cpu3_rexfail;                     // LS read response exclusive fail
  output [2:0]   l2_cpu3_rbufid;                      // LS read response buffer id
  output         l2_cpu3_dvalid_r1;                   // LS read data valid
  output         l2_cpu3_dlast_r1;                    // LS read last indicator
  output [2:0]   l2_cpu3_dbufid_r1;                   // LS read data fill buffer id
  output         l2_cpu3_iext_err_r2;                 // IF external error
  output         l2_cpu3_iext_err_type_r2;            // IF external error type
  output         l2_cpu3_isngl_ecc_err_r3;            // IF single-bit ecc error
  output         l2_cpu3_idbl_ecc_err_r3;             // IF double-bit ecc error
  output [127:0] l2_cpu3_idata_r2;                    // IF read data
  output         l2_cpu3_ivalid_r1;                   // IF read data valid
  output [1:0]   l2_cpu3_ibufid_r1;                   // IF read data fill buffer id
  output         l2_cpu3_ls_sync_req;                 // LS sync req
  output [48:0]  l2_cpu3_ccb_req_addr_c3;             // LS/IF/TLB ccb req addr
  output         l2_cpu3_ccb_dbg_req_c3;              // CCB req is a dbg array rd
  output         l2_cpu3_ls_ccb_clken_c3;             // LS ccb clken
  output         l2_cpu3_ls_ccb_req_c3;               // LS ccb req
  output [4:0]   l2_cpu3_ccb_req_id_c3;               // LS ccb req id
  output [8:0]   l2_cpu3_ccb_req_type_c3;             // LS ccb req type
  output [23:0]  l2_cpu3_ccb_req_info_c3;             // LS ccb req info
  output         l2_cpu3_if_ccb_clken_c3;             // IF ccb clken
  output         l2_cpu3_if_ccb_req_c3;               // IF ccb req
  output         l2_cpu3_if_sync_req;                 // IF sync req
  output         l2_cpu3_tlb_ccb_clken_c3;            // TLB ccb clken
  output         l2_cpu3_tlb_ccb_req_c3;              // TLB ccb req
  output         l2_cpu3_tlb_sync_req;                // TLB sync req 
  output         l2_cpu3_tlb_sync_complete;           // TLB sync complete
  output         l2_cpu3_tbw_desc_vld;                // TBW descriptor valid
  output         l2_cpu3_tbw_ext_err;                 // TBW descriptor external error
  output         l2_cpu3_tbw_ext_err_type;            // TBW descriptor external error type
  output         l2_cpu3_tbw_dbl_ecc_err;             // TBW descriptor double-bit ecc error
  output [63:0]  l2_cpu3_tbw_desc_data;               // TBW descriptor data
  output [63:0]  l2_cpu3_spr_rd_data;                 // DS spr read data
  output [1:0]   l2_cpu3_l2_cache_size;               // DS L2 cache size
  output         l2_cpu3_pf_throttle_q;               // PF throttling

  output         l2_cpu3_wr_ex_resp;                  // store exclusive response
  output         l2_cpu3_wr_ex_fail;                  // store exclusive failed

  output [43:18] l2_cpu3_ic_base;                     // PERIPHBASE 
  output         l2_cpu3_no_intctrl;                  // INTCTLR not present 

  output [33:0]  l2_cpu3_pmu_events;                  // L2 PMU events 

  input          ds_cpu3_l2_spr_en;                   // cpu3 early spr req for clk enables
  input          ds_cpu3_l2_spr_rd;                   // cpu3 spr read op
  input          ds_cpu3_l2_spr_wr;                   // cpu3 spr write op
  input  [8:0]   ds_cpu3_l2_spr_addr;                 // cpu3 spr address
  input          ds_cpu3_l2_spr_dw;                   // cpu3 spr access dw
  input  [63:0]  ds_cpu3_l2_spr_wr_data;              // cpu3 spr write data

  input          l2_cpu3_wr_data_vld_x1_q;            // cpu3 write data vld x1 stage
  input          l2_cpu3_wr_evict_x1_q;               // cpu3 write evict x1 stage
  input  [143:0] l2_cpu3_wr_data;                    
  input          l2_cpu3_ls_rd_haz_vld_arb_q;        
  input          l2_cpu3_ls_wr_haz_vld_arb_q;        
  input          l2_cpu3_dt_pmu_evt_en;               // PMU enabled.

//-----------------------------------------------------------------------------
// tag_pipe / cpu slave
//-----------------------------------------------------------------------------
  output         l2_cpu0_flsh_ls_rd_l2_dly;           // cpu0 ls local hazard flush
  output         l2_cpu0_flsh_ls_wr_l2_dly;           // cpu0 ls local hazard flush

  output         l2_cpu0_wr_data_stall;               // cpu0 write data stall

  output         l2_cpu1_flsh_ls_rd_l2_dly;           // cpu1 ls local hazard flush
  output         l2_cpu1_flsh_ls_wr_l2_dly;           // cpu1 ls local hazard flush

  output         l2_cpu1_wr_data_stall;               // cpu1 write data stall

  output         l2_cpu2_flsh_ls_rd_l2_dly;           // cpu2 ls local hazard flush
  output         l2_cpu2_flsh_ls_wr_l2_dly;           // cpu2 ls local hazard flush

  output         l2_cpu2_wr_data_stall;               // cpu2 write data stall

  output         l2_cpu3_flsh_ls_rd_l2_dly;           // cpu3 ls local hazard flush
  output         l2_cpu3_flsh_ls_wr_l2_dly;           // cpu3 ls local hazard flush

  output         l2_cpu3_wr_data_stall;               // cpu3 write data stall

  output [2:0]   l2_cpu0_flsh_ls_rd_id_l2_dly;        // cpu0 ls id local hazard flush
  output [3:0]   l2_cpu0_flsh_ls_wr_id_l2_dly;        // cpu0 ls id local hazard flush

  output [2:0]   l2_cpu1_flsh_ls_rd_id_l2_dly;        // cpu1 ls id local hazard flush
  output [3:0]   l2_cpu1_flsh_ls_wr_id_l2_dly;        // cpu1 ls id local hazard flush

  output [2:0]   l2_cpu2_flsh_ls_rd_id_l2_dly;        // cpu2 ls id local hazard flush
  output [3:0]   l2_cpu2_flsh_ls_wr_id_l2_dly;        // cpu2 ls id local hazard flush

  output [2:0]   l2_cpu3_flsh_ls_rd_id_l2_dly;        // cpu3 ls id local hazard flush
  output [3:0]   l2_cpu3_flsh_ls_wr_id_l2_dly;        // cpu3 ls id local hazard flush

  output         l2_cpu0_flsh_ls_rd_l4_dly;           // cpu0 ls global hazard flush
  output         l2_cpu0_flsh_if_rd_l4_dly;           // cpu0 if global hazard flush
  output         l2_cpu0_flsh_tw_rd_l4_dly;           // cpu0 tw global hazard flush
  output         l2_cpu0_flsh_ls_wr_l4_dly;           // cpu0 ls global hazard flush

  output         l2_cpu1_flsh_ls_rd_l4_dly;           // cpu1 ls global hazard flush
  output         l2_cpu1_flsh_if_rd_l4_dly;           // cpu1 if global hazard flush
  output         l2_cpu1_flsh_tw_rd_l4_dly;           // cpu1 tw global hazard flush
  output         l2_cpu1_flsh_ls_wr_l4_dly;           // cpu1 ls global hazard flush

  output         l2_cpu2_flsh_ls_rd_l4_dly;           // cpu2 ls global hazard flush
  output         l2_cpu2_flsh_if_rd_l4_dly;           // cpu2 if global hazard flush
  output         l2_cpu2_flsh_tw_rd_l4_dly;           // cpu2 tw global hazard flush
  output         l2_cpu2_flsh_ls_wr_l4_dly;           // cpu2 ls global hazard flush

  output         l2_cpu3_flsh_ls_rd_l4_dly;           // cpu3 ls global hazard flush
  output         l2_cpu3_flsh_if_rd_l4_dly;           // cpu3 if global hazard flush
  output         l2_cpu3_flsh_tw_rd_l4_dly;           // cpu3 tw global hazard flush
  output         l2_cpu3_flsh_ls_wr_l4_dly;           // cpu3 ls global hazard flush

  output [2:0]   l2_cpu0_flsh_ls_rd_id_l4_dly;        // cpu0 ls id global hazard flush
  output [1:0]   l2_cpu0_flsh_if_rd_id_l4_dly;        // cpu0 if id global hazard flush
  output [3:0]   l2_cpu0_flsh_ls_wr_id_l4_dly;        // cpu0 ls id global hazard flush
  output         l2_cpu0_flsh_ls_wr_evict_l4_dly;     // cpu0 ls evict hazard

  output [2:0]   l2_cpu1_flsh_ls_rd_id_l4_dly;        // cpu1 ls id global hazard flush
  output [1:0]   l2_cpu1_flsh_if_rd_id_l4_dly;        // cpu1 if id global hazard flush
  output [3:0]   l2_cpu1_flsh_ls_wr_id_l4_dly;        // cpu1 ls id global hazard flush
  output         l2_cpu1_flsh_ls_wr_evict_l4_dly;     // cpu1 ls evict hazard

  output [2:0]   l2_cpu2_flsh_ls_rd_id_l4_dly;        // cpu2 ls id global hazard flush
  output [1:0]   l2_cpu2_flsh_if_rd_id_l4_dly;        // cpu2 if id global hazard flush
  output [3:0]   l2_cpu2_flsh_ls_wr_id_l4_dly;        // cpu2 ls id global hazard flush
  output         l2_cpu2_flsh_ls_wr_evict_l4_dly;     // cpu2 ls evict hazard

  output [2:0]   l2_cpu3_flsh_ls_rd_id_l4_dly;        // cpu3 ls id global hazard flush
  output [1:0]   l2_cpu3_flsh_if_rd_id_l4_dly;        // cpu3 if id global hazard flush
  output [3:0]   l2_cpu3_flsh_ls_wr_id_l4_dly;        // cpu3 ls id global hazard flush
  output         l2_cpu3_flsh_ls_wr_evict_l4_dly;     // cpu3 ls evict hazard

  output         l2_cpu0_lrq_haz_pending;             // cpu0 lrq hazard pending
  output         l2_cpu1_lrq_haz_pending;             // cpu1 lrq hazard pending
  output         l2_cpu2_lrq_haz_pending;             // cpu2 lrq hazard pending
  output         l2_cpu3_lrq_haz_pending;             // cpu3 lrq hazard pending

  output         l2_cpu0_ifq_haz_pending;             // cpu0 ifq hazard pending
  output         l2_cpu1_ifq_haz_pending;             // cpu1 ifq hazard pending
  output         l2_cpu2_ifq_haz_pending;             // cpu2 ifq hazard pending
  output         l2_cpu3_ifq_haz_pending;             // cpu3 ifq hazard pending

  output         l2_cpu0_trq_haz_pending;             // cpu0 trq hazard pending
  output         l2_cpu1_trq_haz_pending;             // cpu1 trq hazard pending
  output         l2_cpu2_trq_haz_pending;             // cpu2 trq hazard pending
  output         l2_cpu3_trq_haz_pending;             // cpu3 trq hazard pending

  output         l2_cpu0_wrq_haz_pending;             // cpu0 wrq hazard pending
  output         l2_cpu1_wrq_haz_pending;             // cpu1 wrq hazard pending
  output         l2_cpu2_wrq_haz_pending;             // cpu2 wrq hazard pending
  output         l2_cpu3_wrq_haz_pending;             // cpu3 wrq hazard pending

  output         l2_cpu0_idle_block_reqs_q;           // cpu0 idle block requests
  output         l2_cpu1_idle_block_reqs_q;           // cpu1 idle block requests
  output         l2_cpu2_idle_block_reqs_q;           // cpu2 idle block requests
  output         l2_cpu3_idle_block_reqs_q;           // cpu3 idle block requests

  output         l2_cpu0_ls_peq_coll_l4_dly;          // cpu0 peq collision detected
  output         l2_cpu1_ls_peq_coll_l4_dly;          // cpu1 peq collision detected
  output         l2_cpu2_ls_peq_coll_l4_dly;          // cpu2 peq collision detected
  output         l2_cpu3_ls_peq_coll_l4_dly;          // cpu3 peq collision detected

//-----------------------------------------------------------------------------
// tag_pipe
//-----------------------------------------------------------------------------
  output [3:0]   l2_tbnk0_cpu0_lrq_clr_l4_dly2_q;     // tbnk0 clear cpu0 lrq entry
  output [3:0]   l2_tbnk0_cpu1_lrq_clr_l4_dly2_q;     // tbnk0 clear cpu1 lrq entry
  output [3:0]   l2_tbnk0_cpu2_lrq_clr_l4_dly2_q;     // tbnk0 clear cpu2 lrq entry
  output [3:0]   l2_tbnk0_cpu3_lrq_clr_l4_dly2_q;     // tbnk0 clear cpu3 lrq entry

  output [3:0]   l2_tbnk1_cpu0_lrq_clr_l4_dly2_q;     // tbnk1 clear cpu0 lrq entry
  output [3:0]   l2_tbnk1_cpu1_lrq_clr_l4_dly2_q;     // tbnk1 clear cpu1 lrq entry
  output [3:0]   l2_tbnk1_cpu2_lrq_clr_l4_dly2_q;     // tbnk1 clear cpu2 lrq entry
  output [3:0]   l2_tbnk1_cpu3_lrq_clr_l4_dly2_q;     // tbnk1 clear cpu3 lrq entry

  output [2:0]   l2_tbnk0_cpu0_ifq_clr_l4_dly2_q;     // tbnk0 clear cpu0 ifq entry
  output [2:0]   l2_tbnk0_cpu1_ifq_clr_l4_dly2_q;     // tbnk0 clear cpu1 ifq entry
  output [2:0]   l2_tbnk0_cpu2_ifq_clr_l4_dly2_q;     // tbnk0 clear cpu2 ifq entry
  output [2:0]   l2_tbnk0_cpu3_ifq_clr_l4_dly2_q;     // tbnk0 clear cpu3 ifq entry

  output [2:0]   l2_tbnk1_cpu0_ifq_clr_l4_dly2_q;     // tbnk1 clear cpu0 ifq entry
  output [2:0]   l2_tbnk1_cpu1_ifq_clr_l4_dly2_q;     // tbnk1 clear cpu1 ifq entry
  output [2:0]   l2_tbnk1_cpu2_ifq_clr_l4_dly2_q;     // tbnk1 clear cpu2 ifq entry
  output [2:0]   l2_tbnk1_cpu3_ifq_clr_l4_dly2_q;     // tbnk1 clear cpu3 ifq entry

  output         l2_tbnk0_cpu0_trq_clr_l4_dly2_q;     // tbnk0 clear cpu0 trq entry
  output         l2_tbnk0_cpu1_trq_clr_l4_dly2_q;     // tbnk0 clear cpu1 trq entry
  output         l2_tbnk0_cpu2_trq_clr_l4_dly2_q;     // tbnk0 clear cpu2 trq entry
  output         l2_tbnk0_cpu3_trq_clr_l4_dly2_q;     // tbnk0 clear cpu3 trq entry

  output         l2_tbnk1_cpu0_trq_clr_l4_dly2_q;     // tbnk1 clear cpu0 trq entry
  output         l2_tbnk1_cpu1_trq_clr_l4_dly2_q;     // tbnk1 clear cpu1 trq entry
  output         l2_tbnk1_cpu2_trq_clr_l4_dly2_q;     // tbnk1 clear cpu2 trq entry
  output         l2_tbnk1_cpu3_trq_clr_l4_dly2_q;     // tbnk1 clear cpu3 trq entry

  output [5:0]   l2_tbnk0_cpu0_wrq_clr_l4_dly2_q;     // tbnk0 clear cpu0 wrq entry
  output [5:0]   l2_tbnk0_cpu1_wrq_clr_l4_dly2_q;     // tbnk0 clear cpu1 wrq entry
  output [5:0]   l2_tbnk0_cpu2_wrq_clr_l4_dly2_q;     // tbnk0 clear cpu2 wrq entry
  output [5:0]   l2_tbnk0_cpu3_wrq_clr_l4_dly2_q;     // tbnk0 clear cpu3 wrq entry

  output [5:0]   l2_tbnk1_cpu0_wrq_clr_l4_dly2_q;     // tbnk1 clear cpu0 wrq entry
  output [5:0]   l2_tbnk1_cpu1_wrq_clr_l4_dly2_q;     // tbnk1 clear cpu1 wrq entry
  output [5:0]   l2_tbnk1_cpu2_wrq_clr_l4_dly2_q;     // tbnk1 clear cpu2 wrq entry
  output [5:0]   l2_tbnk1_cpu3_wrq_clr_l4_dly2_q;     // tbnk1 clear cpu3 wrq entry


//-----------------------------------------------------------------------------
// cpu_logic / cpu slave
//-----------------------------------------------------------------------------
  output         l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly;        // cpu0 ls rd flsh l4 active
  output         l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly;        // cpu0 wr rd flsh l4 active

  output         l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly;        // cpu1 ls rd flsh l4 active
  output         l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly;        // cpu1 wr rd flsh l4 active

  output         l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly;        // cpu2 ls rd flsh l4 active
  output         l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly;        // cpu2 wr rd flsh l4 active

  output         l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly;        // cpu3 ls rd flsh l4 active
  output         l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly;        // cpu3 wr rd flsh l4 active


//-----------------------------------------------------------------------------
// feq / cpu slave
//-----------------------------------------------------------------------------
  input  [129:0] l2_cpu0_dsq_rd_data_q;               // cpu0 wrq/dsq data
  input  [15:0]  l2_cpu0_dsq_rd_byte_strb_q;          // cpu0 wrq/dsq byte strobes
  input  [129:0] l2_cpu1_dsq_rd_data_q;               // cpu1 wrq/dsq data
  input  [15:0]  l2_cpu1_dsq_rd_byte_strb_q;          // cpu1 wrq/dsq byte strobes
  input  [129:0] l2_cpu2_dsq_rd_data_q;               // cpu2 wrq/dsq data
  input  [15:0]  l2_cpu2_dsq_rd_byte_strb_q;          // cpu2 wrq/dsq byte strobes
  input  [129:0] l2_cpu3_dsq_rd_data_q;               // cpu3 wrq/dsq data
  input  [15:0]  l2_cpu3_dsq_rd_byte_strb_q;          // cpu3 wrq/dsq byte strobes
 
  output         l2_cpu0_dsq_clr_vld_q;               // cpu0 dsq clear wrq vld entry
  output [3:0]   l2_cpu0_dsq_clr_id_q;                // cpu0 dsq clear wrq buffer id
  output         l2_cpu0_dsq_rd_en;                   // cpu0 dsq/wrq data enable
  output         l2_cpu0_dsq_rd_en_x2;                // cpu0 dsq/wrq data enable x2
  output [3:0]   l2_cpu0_dsq_rd_buf_id;               // cpu0 dsq/wrq data select
  output         l2_cpu1_dsq_clr_vld_q;               // cpu1 dsq clear wrq vld entry
  output [3:0]   l2_cpu1_dsq_clr_id_q;                // cpu1 dsq clear wrq buffer id
  output         l2_cpu1_dsq_rd_en;                   // cpu1 dsq/wrq data enable
  output         l2_cpu1_dsq_rd_en_x2;                // cpu1 dsq/wrq data enable x2
  output [3:0]   l2_cpu1_dsq_rd_buf_id;               // cpu1 dsq/wrq data select
  output         l2_cpu2_dsq_clr_vld_q;               // cpu2 dsq clear wrq vld entry
  output [3:0]   l2_cpu2_dsq_clr_id_q;                // cpu2 dsq clear wrq buffer id
  output         l2_cpu2_dsq_rd_en;                   // cpu2 dsq/wrq data enable
  output         l2_cpu2_dsq_rd_en_x2;                // cpu2 dsq/wrq data enable x2
  output [3:0]   l2_cpu2_dsq_rd_buf_id;               // cpu2 dsq/wrq data select
  output         l2_cpu3_dsq_clr_vld_q;               // cpu3 dsq clear wrq vld entry
  output         l2_cpu3_dsq_rd_en;                   // cpu3 dsq/wrq data enable
  output         l2_cpu3_dsq_rd_en_x2;                // cpu3 dsq/wrq data enable x2
  output [3:0]   l2_cpu3_dsq_clr_id_q;                // cpu3 dsq clear wrq buffer id
  output [3:0]   l2_cpu3_dsq_rd_buf_id;               // cpu3 dsq/wrq data select

//-----------------------------------------------------------------------------
// arbitration
//-----------------------------------------------------------------------------
  output         l2_cpu0_rd_vld_skid;                 // cpu0 read skid buffer valid
  output         l2_cpu1_rd_vld_skid;                 // cpu1 read skid buffer valid
  output         l2_cpu2_rd_vld_skid;                 // cpu2 read skid buffer valid
  output         l2_cpu3_rd_vld_skid;                 // cpu3 read skid buffer valid

  output         l2_cpu0_pf_rd_vld_skid_popped;       // cpu0 pf read skid buffer popped
  output         l2_cpu1_pf_rd_vld_skid_popped;       // cpu1 pf read skid buffer popped
  output         l2_cpu2_pf_rd_vld_skid_popped;       // cpu2 pf read skid buffer popped
  output         l2_cpu3_pf_rd_vld_skid_popped;       // cpu3 pf read skid buffer popped

  output         l2_cpu0_rd_arb;                      // 
  output         l2_cpu1_rd_arb;                      // 
  output         l2_cpu2_rd_arb;                      // 
  output         l2_cpu3_rd_arb;                      // 

  output         l2_cpu0_wr_vld_skid;                 // cpu0 write skid buffer valid
  output         l2_cpu1_wr_vld_skid;                 // cpu1 write skid buffer valid
  output         l2_cpu2_wr_vld_skid;                 // cpu2 write skid buffer valid
  output         l2_cpu3_wr_vld_skid;                 // cpu3 write skid buffer valid

  output         l2_cpu0_wr_arb;                      // 
  output         l2_cpu1_wr_arb;                      // 
  output         l2_cpu2_wr_arb;                      // 
  output         l2_cpu3_wr_arb;                      // 

  output         l2_cpu0_ic_vld_skid;                 // cpu0 peripheral (ic) skid buffer valid
  output         l2_cpu1_ic_vld_skid;                 // cpu1 peripheral (ic) skid buffer valid
  output         l2_cpu2_ic_vld_skid;                 // cpu2 peripheral (ic) skid buffer valid
  output         l2_cpu3_ic_vld_skid;                 // cpu3 peripheral (ic) skid buffer valid
  
  output         l2_cpu0_ic_barrier_stall_q;          // cpu0 (ic) barrier stall
  output         l2_cpu1_ic_barrier_stall_q;          // cpu1 (ic) barrier stall
  output         l2_cpu2_ic_barrier_stall_q;          // cpu2 (ic) barrier stall
  output         l2_cpu3_ic_barrier_stall_q;          // cpu3 (ic) barrier stall

  output         l2_cpu0_blk_non_evict_wr;            // cpu0 block non-evict writes from arbitrating
  output         l2_cpu1_blk_non_evict_wr;            // cpu1 block non-evict writes from arbitrating
  output         l2_cpu2_blk_non_evict_wr;            // cpu2 block non-evict writes from arbitrating
  output         l2_cpu3_blk_non_evict_wr;            // cpu3 block non-evict writes from arbitrating

//-----------------------------------------------------------------------------
// ucpu0
//-----------------------------------------------------------------------------
  input          l2_cpu0_idle_wakeup_q;               // cpu0 idle wakeup
  input          l2_cpu0_rd_arb_fast;                 // cpu0 read arbitration fast request
  input  [4:0]   l2_cpu0_rd_id_arb_set;               // cpu0 read arbitration fill buffer id + I/D indicator
  input  [2:0]   l2_cpu0_rd_lrq_id_arb_set;           // cpu0 read arbitration fill buffer id + I/D indicator
  input  [6:0]   l2_cpu0_rd_type_arb_set;             // cpu0 read arbitration type
  input  [2:0]   l2_cpu0_rd_cache_attr_arb_set;       // cpu0 read arbitration cache attributes
  input  [7:0]   l2_cpu0_rd_page_attr_arb_set;        // cpu0 read arbitration page attributes
  input  [2:0]   l2_cpu0_rd_elem_size_arb_set;        // cpu0 read arbitration element size
  input          l2_cpu0_rd_way_arb_set;              // cpu0 read arbitration way
  input          l2_cpu0_rd_replayed_arb_set;         // cpu0 read arbitration replayed
  input          l2_cpu0_rd_excl_arb_set;             // cpu0 read arbitration exclusive
  input          l2_cpu0_rd_priv_arb_set;             // cpu0 read arbitration priv
  input  [1:0]   l2_cpu0_rd_shared_arb_set;           // cpu0 read arbitration shared
  input          l2_cpu0_rd_va48_arb_set;             // cpu0 read arbitration va48
  input          l2_cpu0_rd_aarch64_arb_set;          // cpu0 read arbitration aarch64
  input  [15:8]  l2_cpu0_rd_asid_arb_set;             // cpu0 read arbitration asid
  input          l2_cpu0_rd_prfm_arb_set;             // cpu0 read arbitration prfm
  input  [44:0]  l2_cpu0_rd_addr_arb_set;             // cpu0 read arbitration address
  input          l2_cpu0_rd_bypass_arb_set;           // cpu0 read arbitration bypass
  input          l2_cpu0_rd_bypass_req_can_e5;        // cpu0 read arbitration bypass cancelled request
  input          l2_cpu0_early_rd_reqe4_e5_q;         // cpu0 read arbitration bypass cancelled request
  input          l2_cpu0_rd_bypass_way_e5;            // cpu0 read arbitration bypass way
  input  [2:0]   l2_cpu0_rd_bypass_bufid_e5;          // cpu0 read arbitration bypass bufid
  input  [2:0]   l2_cpu0_rd_bypass_lrq_id_e5;         // cpu0 read arbitration bypass bufid

  input          l2_cpu0_wr_arb_fast;                 // cpu0 write arbitration fast request
  input  [3:0]   l2_cpu0_wr_id_arb_set;               // cpu0 write arbitration id for 1st qw
  input  [3:0]   l2_cpu0_wr_partial_dw_arb_set;       // cpu0 write partial qw byte strobe indicator
  input  [2:0]   l2_cpu0_wr_cache_attr_arb_set;       // cpu0 write arbitration cache attributes
  input  [7:0]   l2_cpu0_wr_page_attr_arb_set;        // cpu0 write arbitration page attributes
  input  [2:0]   l2_cpu0_wr_elem_size_arb_set;        // cpu0 write arbitration element size
  input  [2:0]   l2_cpu0_wr_type_arb_set;             // cpu0 write arbitration type
  input  [11:0]  l2_cpu0_wr_cl_id_arb_set;            // cpu0 write arbitration cacheline ids for 2nd, 3rd, 4th qws
  input          l2_cpu0_wr_priv_arb_set;             // cpu0 write arbitration priv
  input  [1:0]   l2_cpu0_wr_shared_arb_set;           // cpu0 write arbitration shared
  input          l2_cpu0_wr_last_arb_set;             // cpu0 write arbitration last
  input          l2_cpu0_wr_clean_evict_arb_set;      // cpu0 write arbitration clean eviction
  input          l2_cpu0_wr_err_arb_set;              // cpu0 write arbitration error
  input          l2_cpu0_wr_way_arb_set;              // cpu0 write arbitration way
  input          l2_cpu0_wr_dirty_arb_set;            // cpu0 write arbitration dirty
  input          l2_cpu0_wr_1st_replayed_arb_set;     // cpu0 write arbitration 1st replay indicator
  input  [44:0]  l2_cpu0_wr_addr_arb_set;             // cpu0 write arbitration address
  input          l2_cpu0_ic_arb_fast;                 // cpu0 peripheral (ic) arbitration fast request
  input  [2:0]   l2_cpu0_ic_id_arb_set;               // cpu0 peripheral (ic) fill buffer id
  input          l2_cpu0_ic_write_arb_set;            // cpu0 peripheral (ic) write indicator
  input          l2_cpu0_ic_excl_arb_set;             // cpu0 peripheral (ic) exclusive indicator
  input  [2:0]   l2_cpu0_ic_elem_size_arb_set;        // cpu0 peripheral (ic) element size
  input          l2_cpu0_ic_ns_arb_set;               // cpu0 peripheral (ic) non-secure
  input  [15:0]  l2_cpu0_ic_addr_arb_set;             // cpu0 peripheral (ic) address
  input  [31:0]  l2_cpu0_ic_data_arb_set;             // cpu0 peripheral (ic) write data

  input          l2_cpu0_wrq_almost_full;             // cpu0 wrq almost full indicator

  input          l2_cpu0_ls_wr_req_w2a;               // cpu0 ls write request
  input          l2_cpu0_ls_wr_last_w2a;              // cpu0 ls last indicator
  input          l2_cpu0_ls_wr_dirty_w2a;             // cpu0 ls dirty indicator
  input          l2_cpu0_ls_wr_err_w2a;               // cpu0 ls error indicator
  input  [2:0]   l2_cpu0_ls_wr_type_w2a;              // cpu0 ls write type
  input  [4:0]   l2_cpu0_ls_wr_ccb_id_w2a;            // cpu0 ls ccb id
  input  [127:0] l2_cpu0_ls_wr_data_w2a;              // cpu0 ls write data

  input          l2_cpu0_ls_ccb_resp;                 // cpu0 ls ccb resp
  input  [4:0]   l2_cpu0_ls_ccb_resp_id;              // cpu0 ls ccb id
  input          l2_cpu0_ls_ccb_data_wr;              // cpu0 ls ccb data xfer

  input          l2_cpu0_if_ccb_resp;                 // cpu0 if ccb resp
  input  [4:0]   l2_cpu0_if_ccb_resp_id;              // cpu0 if ccb id
   
  input          l2_cpu0_tw_ccb_resp;                 // cpu0 tw ccb resp
  input  [4:0]   l2_cpu0_tw_ccb_resp_id;              // cpu0 tw ccb id

  input          l2_cpu0_if_sync_done_q;              // cpu0 sync response
  input          l2_cpu0_tlb_sync_done_q;             // cpu0 tlb sync response 

  input  [5:0]   l2_cpu0_lrq_haz_clr_id_dcd_q;        // cpu0 lrq clear hazard id
  input  [15:0]  l2_cpu0_wrq_haz_clr_id_dcd_q;        // cpu0 wrq clear hazard id
  input  [3:0]   l2_cpu0_ls_rd_haz_id_arb_q;           // cpu0 ls rd wrq hazard id
  input  [2:0]   l2_cpu0_ls_wr_haz_id_arb_q;           // cpu0 ls wr lrq hazard id

//-----------------------------------------------------------------------------
// ucpu1
//-----------------------------------------------------------------------------
  input          l2_cpu1_idle_wakeup_q;               // cpu1 idle wakeup
  input          l2_cpu1_rd_arb_fast;                 // cpu1 read arbitration fast request
  input  [4:0]   l2_cpu1_rd_id_arb_set;               // cpu1 read arbitration fill buffer id + I/D indicator
  input  [2:0]   l2_cpu1_rd_lrq_id_arb_set;           // cpu1 read arbitration fill buffer id + I/D indicator
  input  [6:0]   l2_cpu1_rd_type_arb_set;             // cpu1 read arbitration type
  input  [2:0]   l2_cpu1_rd_cache_attr_arb_set;       // cpu1 read arbitration cache attributes
  input  [7:0]   l2_cpu1_rd_page_attr_arb_set;        // cpu1 read arbitration page attributes
  input  [2:0]   l2_cpu1_rd_elem_size_arb_set;        // cpu1 read arbitration element size
  input          l2_cpu1_rd_way_arb_set;              // cpu1 read arbitration way
  input          l2_cpu1_rd_replayed_arb_set;         // cpu1 read arbitration replayed
  input          l2_cpu1_rd_excl_arb_set;             // cpu1 read arbitration exclusive
  input          l2_cpu1_rd_priv_arb_set;             // cpu1 read arbitration priv
  input  [1:0]   l2_cpu1_rd_shared_arb_set;           // cpu1 read arbitration shared
  input          l2_cpu1_rd_va48_arb_set;             // cpu1 read arbitration va48
  input          l2_cpu1_rd_aarch64_arb_set;          // cpu1 read arbitration aarch64
  input  [15:8]  l2_cpu1_rd_asid_arb_set;             // cpu1 read arbitration asid
  input          l2_cpu1_rd_prfm_arb_set;             // cpu1 read arbitration prfm
  input  [44:0]  l2_cpu1_rd_addr_arb_set;             // cpu1 read arbitration address
  input          l2_cpu1_rd_bypass_arb_set;           // cpu1 read arbitration bypass
  input          l2_cpu1_rd_bypass_req_can_e5;        // cpu1 read arbitration bypass cancelled request
  input          l2_cpu1_early_rd_reqe4_e5_q;         // cpu1 read arbitration bypass cancelled request
  input          l2_cpu1_rd_bypass_way_e5;            // cpu1 read arbitration bypass way
  input  [2:0]   l2_cpu1_rd_bypass_bufid_e5;          // cpu1 read arbitration bypass bufid
  input  [2:0]   l2_cpu1_rd_bypass_lrq_id_e5;         // cpu1 read arbitration bypass bufid

  input          l2_cpu1_wr_arb_fast;                 // cpu1 write arbitration fast request
  input  [3:0]   l2_cpu1_wr_id_arb_set;               // cpu1 write arbitration id for 1st qw
  input  [3:0]   l2_cpu1_wr_partial_dw_arb_set;       // cpu1 write partial qw byte strobe indicator
  input  [2:0]   l2_cpu1_wr_cache_attr_arb_set;       // cpu1 write arbitration cache attributes
  input  [7:0]   l2_cpu1_wr_page_attr_arb_set;        // cpu1 write arbitration page attributes
  input  [2:0]   l2_cpu1_wr_elem_size_arb_set;        // cpu1 write arbitration element size
  input  [2:0]   l2_cpu1_wr_type_arb_set;             // cpu1 write arbitration type
  input  [11:0]  l2_cpu1_wr_cl_id_arb_set;            // cpu1 write arbitration cacheline ids for 2nd, 3rd, 4th qws
  input          l2_cpu1_wr_priv_arb_set;             // cpu1 write arbitration priv
  input  [1:0]   l2_cpu1_wr_shared_arb_set;           // cpu1 write arbitration shared
  input          l2_cpu1_wr_last_arb_set;             // cpu1 write arbitration last
  input          l2_cpu1_wr_clean_evict_arb_set;      // cpu1 write arbitration clean eviction
  input          l2_cpu1_wr_err_arb_set;              // cpu1 write arbitration error
  input          l2_cpu1_wr_way_arb_set;              // cpu1 write arbitration way
  input          l2_cpu1_wr_dirty_arb_set;            // cpu1 write arbitration dirty
  input          l2_cpu1_wr_1st_replayed_arb_set;     // cpu1 write arbitration 1st replay indicator
  input  [44:0]  l2_cpu1_wr_addr_arb_set;             // cpu1 write arbitration address
  input          l2_cpu1_ic_arb_fast;                 // cpu1 peripheral (ic) arbitration fast request
  input  [2:0]   l2_cpu1_ic_id_arb_set;               // cpu1 peripheral (ic) fill buffer id
  input          l2_cpu1_ic_write_arb_set;            // cpu1 peripheral (ic) write indicator
  input          l2_cpu1_ic_excl_arb_set;             // cpu1 peripheral (ic) exclusive indicator
  input  [2:0]   l2_cpu1_ic_elem_size_arb_set;        // cpu1 peripheral (ic) element size
  input          l2_cpu1_ic_ns_arb_set;               // cpu1 peripheral (ic) non-secure
  input  [15:0]  l2_cpu1_ic_addr_arb_set;             // cpu1 peripheral (ic) address
  input  [31:0]  l2_cpu1_ic_data_arb_set;             // cpu1 peripheral (ic) write data

  input          l2_cpu1_wrq_almost_full;             // cpu1 wrq almost full indicator

  input          l2_cpu1_ls_wr_req_w2a;               // cpu1 ls write request
  input          l2_cpu1_ls_wr_last_w2a;              // cpu1 ls last indicator
  input          l2_cpu1_ls_wr_dirty_w2a;             // cpu1 ls dirty indicator
  input          l2_cpu1_ls_wr_err_w2a;               // cpu1 ls error indicator
  input  [2:0]   l2_cpu1_ls_wr_type_w2a;              // cpu1 ls write type
  input  [4:0]   l2_cpu1_ls_wr_ccb_id_w2a;            // cpu1 ls ccb id
  input  [127:0] l2_cpu1_ls_wr_data_w2a;              // cpu1 ls write data

  input          l2_cpu1_ls_ccb_resp;                 // cpu1 ls ccb resp
  input  [4:0]   l2_cpu1_ls_ccb_resp_id;              // cpu1 ls ccb id
  input          l2_cpu1_ls_ccb_data_wr;              // cpu1 ls ccb data xfer

  input          l2_cpu1_if_ccb_resp;                 // cpu1 if ccb resp
  input  [4:0]   l2_cpu1_if_ccb_resp_id;              // cpu1 if ccb id
   
  input          l2_cpu1_tw_ccb_resp;                 // cpu1 tw ccb resp
  input  [4:0]   l2_cpu1_tw_ccb_resp_id;              // cpu1 tw ccb id

  input          l2_cpu1_if_sync_done_q;              // cpu1 sync response
  input          l2_cpu1_tlb_sync_done_q;             // cpu1 tlb sync response 

  input  [5:0]   l2_cpu1_lrq_haz_clr_id_dcd_q;        // cpu1 lrq clear hazard id
  input  [15:0]  l2_cpu1_wrq_haz_clr_id_dcd_q;        // cpu1 wrq clear hazard id
  input  [3:0]   l2_cpu1_ls_rd_haz_id_arb_q;           // cpu1 ls rd wrq hazard id
  input  [2:0]   l2_cpu1_ls_wr_haz_id_arb_q;           // cpu1 ls wr lrq hazard id

//-----------------------------------------------------------------------------
// ucpu2
//-----------------------------------------------------------------------------
  input          l2_cpu2_idle_wakeup_q;               // cpu2 idle wakeup
  input          l2_cpu2_rd_arb_fast;                 // cpu2 read arbitration fast request
  input  [4:0]   l2_cpu2_rd_id_arb_set;               // cpu2 read arbitration fill buffer id + I/D indicator
  input  [2:0]   l2_cpu2_rd_lrq_id_arb_set;           // cpu2 read arbitration fill buffer id + I/D indicator
  input  [6:0]   l2_cpu2_rd_type_arb_set;             // cpu2 read arbitration type
  input  [2:0]   l2_cpu2_rd_cache_attr_arb_set;       // cpu2 read arbitration cache attributes
  input  [7:0]   l2_cpu2_rd_page_attr_arb_set;        // cpu2 read arbitration page attributes
  input  [2:0]   l2_cpu2_rd_elem_size_arb_set;        // cpu2 read arbitration element size
  input          l2_cpu2_rd_way_arb_set;              // cpu2 read arbitration way
  input          l2_cpu2_rd_replayed_arb_set;         // cpu2 read arbitration replayed
  input          l2_cpu2_rd_excl_arb_set;             // cpu2 read arbitration exclusive
  input          l2_cpu2_rd_priv_arb_set;             // cpu2 read arbitration priv
  input  [1:0]   l2_cpu2_rd_shared_arb_set;           // cpu2 read arbitration shared
  input          l2_cpu2_rd_va48_arb_set;             // cpu0 read arbitration va48
  input          l2_cpu2_rd_aarch64_arb_set;          // cpu2 read arbitration aarch64
  input  [15:8]  l2_cpu2_rd_asid_arb_set;             // cpu2 read arbitration asid
  input          l2_cpu2_rd_prfm_arb_set;             // cpu2 read arbitration prfm
  input  [44:0]  l2_cpu2_rd_addr_arb_set;             // cpu2 read arbitration address
  input          l2_cpu2_rd_bypass_arb_set;           // cpu2 read arbitration bypass
  input          l2_cpu2_rd_bypass_req_can_e5;        // cpu2 read arbitration bypass cancelled request
  input          l2_cpu2_early_rd_reqe4_e5_q;         // cpu2 read arbitration bypass cancelled request
  input          l2_cpu2_rd_bypass_way_e5;            // cpu2 read arbitration bypass way
  input  [2:0]   l2_cpu2_rd_bypass_bufid_e5;          // cpu2 read arbitration bypass bufid
  input  [2:0]   l2_cpu2_rd_bypass_lrq_id_e5;         // cpu2 read arbitration bypass bufid

  input          l2_cpu2_wr_arb_fast;                 // cpu2 write arbitration fast request
  input  [3:0]   l2_cpu2_wr_id_arb_set;               // cpu2 write arbitration id for 1st qw
  input  [3:0]   l2_cpu2_wr_partial_dw_arb_set;       // cpu2 write partial qw byte strobe indicator
  input  [2:0]   l2_cpu2_wr_cache_attr_arb_set;       // cpu2 write arbitration cache attributes
  input  [7:0]   l2_cpu2_wr_page_attr_arb_set;        // cpu2 write arbitration page attributes
  input  [2:0]   l2_cpu2_wr_elem_size_arb_set;        // cpu2 write arbitration element size
  input  [2:0]   l2_cpu2_wr_type_arb_set;             // cpu2 write arbitration type
  input  [11:0]  l2_cpu2_wr_cl_id_arb_set;            // cpu2 write arbitration cacheline ids for 2nd, 3rd, 4th qws
  input          l2_cpu2_wr_priv_arb_set;             // cpu2 write arbitration priv
  input  [1:0]   l2_cpu2_wr_shared_arb_set;           // cpu2 write arbitration shared
  input          l2_cpu2_wr_last_arb_set;             // cpu2 write arbitration last
  input          l2_cpu2_wr_clean_evict_arb_set;      // cpu2 write arbitration clean eviction
  input          l2_cpu2_wr_err_arb_set;              // cpu2 write arbitration error
  input          l2_cpu2_wr_way_arb_set;              // cpu2 write arbitration way
  input          l2_cpu2_wr_dirty_arb_set;            // cpu2 write arbitration dirty
  input          l2_cpu2_wr_1st_replayed_arb_set;     // cpu2 write arbitration 1st replay indicator
  input  [44:0]  l2_cpu2_wr_addr_arb_set;             // cpu2 write arbitration address
  input          l2_cpu2_ic_arb_fast;                 // cpu2 peripheral (ic) arbitration fast request
  input  [2:0]   l2_cpu2_ic_id_arb_set;               // cpu2 peripheral (ic) fill buffer id
  input          l2_cpu2_ic_write_arb_set;            // cpu2 peripheral (ic) write indicator
  input          l2_cpu2_ic_excl_arb_set;             // cpu2 peripheral (ic) exclusive indicator
  input  [2:0]   l2_cpu2_ic_elem_size_arb_set;        // cpu2 peripheral (ic) element size
  input          l2_cpu2_ic_ns_arb_set;               // cpu2 peripheral (ic) non-secure
  input  [15:0]  l2_cpu2_ic_addr_arb_set;             // cpu2 peripheral (ic) address
  input  [31:0]  l2_cpu2_ic_data_arb_set;             // cpu2 peripheral (ic) write data

  input          l2_cpu2_wrq_almost_full;             // cpu2 wrq almost full indicator

  input          l2_cpu2_ls_wr_req_w2a;               // cpu2 ls write request
  input          l2_cpu2_ls_wr_last_w2a;              // cpu2 ls last indicator
  input          l2_cpu2_ls_wr_dirty_w2a;             // cpu2 ls dirty indicator
  input          l2_cpu2_ls_wr_err_w2a;               // cpu2 ls error indicator
  input  [2:0]   l2_cpu2_ls_wr_type_w2a;              // cpu2 ls write type
  input  [4:0]   l2_cpu2_ls_wr_ccb_id_w2a;            // cpu2 ls ccb id
  input  [127:0] l2_cpu2_ls_wr_data_w2a;              // cpu2 ls write data

  input          l2_cpu2_ls_ccb_resp;                 // cpu2 ls ccb resp
  input  [4:0]   l2_cpu2_ls_ccb_resp_id;              // cpu2 ls ccb id
  input          l2_cpu2_ls_ccb_data_wr;              // cpu2 ls ccb data xfer

  input          l2_cpu2_if_ccb_resp;                 // cpu2 if ccb resp
  input  [4:0]   l2_cpu2_if_ccb_resp_id;              // cpu2 if ccb id
   
  input          l2_cpu2_tw_ccb_resp;                 // cpu2 tw ccb resp
  input  [4:0]   l2_cpu2_tw_ccb_resp_id;              // cpu2 tw ccb id

  input          l2_cpu2_if_sync_done_q;              // cpu2 sync response
  input          l2_cpu2_tlb_sync_done_q;             // cpu2 tlb sync response 

  input  [5:0]   l2_cpu2_lrq_haz_clr_id_dcd_q;        // cpu2 lrq clear hazard id
  input  [15:0]  l2_cpu2_wrq_haz_clr_id_dcd_q;        // cpu2 wrq clear hazard id
  input  [3:0]   l2_cpu2_ls_rd_haz_id_arb_q;           // cpu2 ls rd wrq hazard id
  input  [2:0]   l2_cpu2_ls_wr_haz_id_arb_q;           // cpu2 ls wr lrq hazard id

//-----------------------------------------------------------------------------
// ucpu3
//-----------------------------------------------------------------------------
  input          l2_cpu3_idle_wakeup_q;               // cpu3 idle wakeup
  input          l2_cpu3_rd_arb_fast;                 // cpu3 read arbitration fast request
  input  [4:0]   l2_cpu3_rd_id_arb_set;               // cpu3 read arbitration fill buffer id + I/D indicator
  input  [2:0]   l2_cpu3_rd_lrq_id_arb_set;           // cpu3 read arbitration fill buffer id + I/D indicator
  input  [6:0]   l2_cpu3_rd_type_arb_set;             // cpu3 read arbitration type
  input  [2:0]   l2_cpu3_rd_cache_attr_arb_set;       // cpu3 read arbitration cache attributes
  input  [7:0]   l2_cpu3_rd_page_attr_arb_set;        // cpu3 read arbitration page attributes
  input  [2:0]   l2_cpu3_rd_elem_size_arb_set;        // cpu3 read arbitration element size
  input          l2_cpu3_rd_way_arb_set;              // cpu3 read arbitration way
  input          l2_cpu3_rd_replayed_arb_set;         // cpu3 read arbitration replayed
  input          l2_cpu3_rd_excl_arb_set;             // cpu3 read arbitration exclusive
  input          l2_cpu3_rd_priv_arb_set;             // cpu3 read arbitration priv
  input  [1:0]   l2_cpu3_rd_shared_arb_set;           // cpu3 read arbitration shared
  input          l2_cpu3_rd_va48_arb_set;             // cpu3 read arbitration va48
  input          l2_cpu3_rd_aarch64_arb_set;          // cpu3 read arbitration aarch64
  input  [15:8]  l2_cpu3_rd_asid_arb_set;             // cpu3 read arbitration asid
  input          l2_cpu3_rd_prfm_arb_set;             // cpu3 read arbitration prfm
  input  [44:0]  l2_cpu3_rd_addr_arb_set;             // cpu3 read arbitration address
  input          l2_cpu3_rd_bypass_arb_set;           // cpu3 read arbitration bypass
  input          l2_cpu3_rd_bypass_req_can_e5;        // cpu3 read arbitration bypass cancelled request
  input          l2_cpu3_early_rd_reqe4_e5_q;         // cpu3 read arbitration bypass cancelled request
  input          l2_cpu3_rd_bypass_way_e5;            // cpu3 read arbitration bypass way
  input  [2:0]   l2_cpu3_rd_bypass_bufid_e5;          // cpu3 read arbitration bypass bufid
  input  [2:0]   l2_cpu3_rd_bypass_lrq_id_e5;         // cpu3 read arbitration bypass bufid

  input          l2_cpu3_wr_arb_fast;                 // cpu3 write arbitration fast request
  input  [3:0]   l2_cpu3_wr_id_arb_set;               // cpu3 write arbitration id for 1st qw
  input  [3:0]   l2_cpu3_wr_partial_dw_arb_set;       // cpu3 write partial qw byte strobe indicator
  input  [2:0]   l2_cpu3_wr_cache_attr_arb_set;       // cpu3 write arbitration cache attributes
  input  [7:0]   l2_cpu3_wr_page_attr_arb_set;        // cpu3 write arbitration page attributes
  input  [2:0]   l2_cpu3_wr_elem_size_arb_set;        // cpu3 write arbitration element size
  input  [2:0]   l2_cpu3_wr_type_arb_set;             // cpu3 write arbitration type
  input  [11:0]  l2_cpu3_wr_cl_id_arb_set;            // cpu3 write arbitration cacheline ids for 2nd, 3rd, 4th qws
  input          l2_cpu3_wr_priv_arb_set;             // cpu3 write arbitration priv
  input  [1:0]   l2_cpu3_wr_shared_arb_set;           // cpu3 write arbitration shared
  input          l2_cpu3_wr_last_arb_set;             // cpu3 write arbitration last
  input          l2_cpu3_wr_clean_evict_arb_set;      // cpu3 write arbitration clean eviction
  input          l2_cpu3_wr_err_arb_set;              // cpu3 write arbitration error
  input          l2_cpu3_wr_way_arb_set;              // cpu3 write arbitration way
  input          l2_cpu3_wr_dirty_arb_set;            // cpu3 write arbitration dirty
  input          l2_cpu3_wr_1st_replayed_arb_set;     // cpu3 write arbitration 1st replay indicator
  input  [44:0]  l2_cpu3_wr_addr_arb_set;             // cpu3 write arbitration address
  input          l2_cpu3_ic_arb_fast;                 // cpu3 peripheral (ic) arbitration fast request
  input  [2:0]   l2_cpu3_ic_id_arb_set;               // cpu3 peripheral (ic) fill buffer id
  input          l2_cpu3_ic_write_arb_set;            // cpu3 peripheral (ic) write indicator
  input          l2_cpu3_ic_excl_arb_set;             // cpu3 peripheral (ic) exclusive indicator
  input  [2:0]   l2_cpu3_ic_elem_size_arb_set;        // cpu3 peripheral (ic) element size
  input          l2_cpu3_ic_ns_arb_set;               // cpu3 peripheral (ic) non-secure
  input  [15:0]  l2_cpu3_ic_addr_arb_set;             // cpu3 peripheral (ic) address
  input  [31:0]  l2_cpu3_ic_data_arb_set;             // cpu3 peripheral (ic) write data

  input          l2_cpu3_wrq_almost_full;             // cpu3 wrq almost full indicator

  input          l2_cpu3_ls_wr_req_w2a;               // cpu3 ls write request
  input          l2_cpu3_ls_wr_last_w2a;              // cpu3 ls last indicator
  input          l2_cpu3_ls_wr_dirty_w2a;             // cpu3 ls dirty indicator
  input          l2_cpu3_ls_wr_err_w2a;               // cpu3 ls error indicator
  input  [2:0]   l2_cpu3_ls_wr_type_w2a;              // cpu3 ls write type
  input  [4:0]   l2_cpu3_ls_wr_ccb_id_w2a;            // cpu3 ls ccb id
  input  [127:0] l2_cpu3_ls_wr_data_w2a;              // cpu3 ls write data

  input          l2_cpu3_ls_ccb_resp;                 // cpu3 ls ccb resp
  input  [4:0]   l2_cpu3_ls_ccb_resp_id;              // cpu3 ls ccb id
  input          l2_cpu3_ls_ccb_data_wr;              // cpu3 ls ccb data xfer

  input          l2_cpu3_if_ccb_resp;                 // cpu3 if ccb resp
  input  [4:0]   l2_cpu3_if_ccb_resp_id;              // cpu3 if ccb id
   
  input          l2_cpu3_tw_ccb_resp;                 // cpu3 tw ccb resp
  input  [4:0]   l2_cpu3_tw_ccb_resp_id;              // cpu3 tw ccb id

  input          l2_cpu3_if_sync_done_q;              // cpu3 sync response
  input          l2_cpu3_tlb_sync_done_q;             // cpu3 tlb sync response 

  input  [5:0]   l2_cpu3_lrq_haz_clr_id_dcd_q;        // cpu3 lrq clear hazard id
  input  [15:0]  l2_cpu3_wrq_haz_clr_id_dcd_q;        // cpu3 wrq clear hazard id
  input  [3:0]   l2_cpu3_ls_rd_haz_id_arb_q;           // cpu3 ls rd wrq hazard id
  input  [2:0]   l2_cpu3_ls_wr_haz_id_arb_q;           // cpu3 ls wr lrq hazard id

// END L2-CPU interface

//-------------------------------------------------------------------
// TM interface 
//-------------------------------------------------------------------
// BEGIN TIMER-CPU interface
  output [3:0]   tm_cpu0_cntkctl_usr;                 // User accessibility of CNTPCT,CNTVCT,CNTV_<>,CNTP_<>
  output [1:0]   tm_cpu0_cnthctl_kernel;              // NS Kernel access of CNTPCT,CNTP_<>

  output [3:0]   tm_cpu1_cntkctl_usr;                 // User accessibility of CNTPCT,CNTVCT,CNTV_<>,CNTP_<>
  output [1:0]   tm_cpu1_cnthctl_kernel;              // NS Kernel access of CNTPCT,CNTP_<>

  output [3:0]   tm_cpu2_cntkctl_usr;                 // User accessibility of CNTPCT,CNTVCT,CNTV_<>,CNTP_<>
  output [1:0]   tm_cpu2_cnthctl_kernel;              // NS Kernel access of CNTPCT,CNTP_<>

  output [3:0]   tm_cpu3_cntkctl_usr;                 // User accessibility of CNTPCT,CNTVCT,CNTV_<>,CNTP_<>
  output [1:0]   tm_cpu3_cnthctl_kernel;              // NS Kernel access of CNTPCT,CNTP_<>
// END TIMER-CPU interface

//-----------------------------------------------------------------------------
// IC interface
//-----------------------------------------------------------------------------
  input              ls_cpu0_imp_abort_slv;                // LS Imprecise Abort SEI
  input              ls_cpu0_imp_abort_ecc;                // LS Imprecise Abort REI
  input              ls_cpu0_imp_abort_dec;                // LS Imprecise Abort DEC
  input              ls_cpu0_imp_abort_containable;        // LS Imprecise Abort is Containable
  input              ls_cpu0_raw_eae_nonsec;               // LS NS LPAE to IC
  input              ls_cpu0_raw_eae_secure;               // LS S  LPAE to IC

  input              ds_cpu0_ic_sample_spr;
  input  [4:0]       ds_cpu0_ic_cpsr_mode;
  input              ds_cpu0_ic_aa64naa32;
  input              ds_cpu0_ic_hcr_change;
  input              ds_cpu0_ic_scr_change;
// BEGIN INCLUDE FOR CPU1
  input              ds_cpu1_ic_sample_spr;
  input  [4:0]       ds_cpu1_ic_cpsr_mode;
  input              ds_cpu1_ic_aa64naa32;
  input              ds_cpu1_ic_hcr_change;
  input              ds_cpu1_ic_scr_change;
  input              ls_cpu1_imp_abort_slv;                // LS Imprecise Abort SEI
  input              ls_cpu1_imp_abort_ecc;                // LS Imprecise Abort REI
  input              ls_cpu1_imp_abort_dec;                // LS Imprecise Abort DEC
  input              ls_cpu1_imp_abort_containable;        // LS Imprecise Abort is Containable
  input              ls_cpu1_raw_eae_nonsec;               // LS NS LPAE to IC
  input              ls_cpu1_raw_eae_secure;               // LS S  LPAE to IC
// END INCLUDE FOR CPU1
// BEGIN INCLUDE FOR CPU2
  input              ds_cpu2_ic_sample_spr;
  input  [4:0]       ds_cpu2_ic_cpsr_mode;
  input              ds_cpu2_ic_hcr_change;
  input              ds_cpu2_ic_scr_change;
  input              ds_cpu2_ic_aa64naa32;
  input              ls_cpu2_imp_abort_slv;                // LS Imprecise Abort SEI
  input              ls_cpu2_imp_abort_ecc;                // LS Imprecise Abort REI
  input              ls_cpu2_imp_abort_dec;                // LS Imprecise Abort DEC
  input              ls_cpu2_imp_abort_containable;        // LS Imprecise Abort is Containable
  input              ls_cpu2_raw_eae_nonsec;               // LS NS LPAE to IC
  input              ls_cpu2_raw_eae_secure;               // LS S  LPAE to IC
// END INCLUDE FOR CPU2
// BEGIN INCLUDE FOR CPU3
  input              ds_cpu3_ic_sample_spr;
  input  [4:0]       ds_cpu3_ic_cpsr_mode;
  input              ds_cpu3_ic_hcr_change;
  input              ds_cpu3_ic_scr_change;
  input              ds_cpu3_ic_aa64naa32;
  input              ls_cpu3_imp_abort_slv;                // LS Imprecise Abort SEI
  input              ls_cpu3_imp_abort_ecc;                // LS Imprecise Abort REI
  input              ls_cpu3_imp_abort_dec;                // LS Imprecise Abort DEC
  input              ls_cpu3_imp_abort_containable;        // LS Imprecise Abort is Containable
  input              ls_cpu3_raw_eae_nonsec;               // LS NS LPAE to IC
  input              ls_cpu3_raw_eae_secure;               // LS S  LPAE to IC
// END INCLUDE FOR CPU3

  output [`MAIA_CN:0] ic_nfiq;                             // IC physical FIQ
  output [`MAIA_CN:0] ic_nirq;                             // IC physical IRQ
  output [`MAIA_CN:0] ic_nsei;                             // IC physical SEI
  output [`MAIA_CN:0] ic_nvfiq;                            // IC virtual  FIQ
  output [`MAIA_CN:0] ic_nvirq;                            // IC virtual  IRQ
  output [`MAIA_CN:0] ic_nvsei;                            // IC virtual  SEI
  output [`MAIA_CN:0] ic_p_valid;                          // IC is present

  output [`MAIA_CN:0] ic_sample_spr;                   // IC sample signal for TC, TALL*, EL* signals
  output [`MAIA_CN:0] ic_hcr_change_complete;         
  output [`MAIA_CN:0] ic_scr_change_complete;
  output [`MAIA_CN:0] ic_el_change_complete;
  output [`MAIA_CN:0] ic_ich_el2_tc;                   // IC trap common
  output [`MAIA_CN:0] ic_ich_el2_tall0;                // IC trap all grp0
  output [`MAIA_CN:0] ic_ich_el2_tall1;                // IC trap all grp1
  output [`MAIA_CN:0] ic_sra_el3_en;                   // IC System Registers enabled in EL3
  output [`MAIA_CN:0] ic_sra_el1s_en;                  // IC System Registers enabled in EL1S 
  output [`MAIA_CN:0] ic_sra_el2_en;                   // IC System Registers enabled in EL2
  output [`MAIA_CN:0] ic_sra_el1ns_en;                 // IC System Registers enabled in EL1NS
  output [`MAIA_CN:0] ic_sre_el1ns_hyp_trap;           // IC HYP_TRAP EL1NS accesses
  output [`MAIA_CN:0] ic_sre_el1ns_mon_trap;           // IC MON_TRAP EL1NS accesses 
  output [`MAIA_CN:0] ic_sre_el1s_mon_trap;            // IC MON_TRAP EL1S  accesses 
  output [`MAIA_CN:0] ic_sre_el2_mon_trap;             // IC MON_TRAP EL2   accesses 
  output [`MAIA_CN:0] ic_block_eoi_sgi_wr;             // IC Block all EOI and SGI write accesses

//-----------------------------------------------------------------------------
// DT interface
//-----------------------------------------------------------------------------
// BEGIN DT-CPU interface
//-----------------------------------------------------------------------------
// ucpu0
//-----------------------------------------------------------------------------
  output         dt_cpu0_dbif_req_pclk;               // Debug Interface Req
  output         dt_cpu0_dbif_write_pclk;             // Debug Interface Write/!Read
  output         dt_cpu0_dbif_locked_pclk;            // Debug Interface Lock Value
  output [31:0]  dt_cpu0_dbif_wrdata_pclk;            // Debug Interface Write Data
  output [14:2]  dt_cpu0_dbif_addr_pclk;              // Debug Interface Addr
  output         dt_cpu0_edecr_osuce_pclk;            // OS Unlock Catch Enable Bit 
  output         dt_cpu0_edecr_rce_pclk;              // EDECR Reset Catch Enable Bit
  output         dt_cpu0_edecr_ss_pclk;               // EDECR Halting Step Enable Bit
  output         dt_cpu0_edbgrq_pclk;                 // External Debug Request
  output         dt_cpu0_edacr_frc_idleack_pclk;      // EDACR Force Debug Idle Ack 
  output         dt_cpu0_edprcr_corepurq_pclk;        // PRCR Power Up Request

  input          dt_cpu0_pmusnapshot_ack_gclk;        // PMU Snapshot Trigger acknowledge
  output         dt_cpu0_pmusnapshot_req_pclk;        // PMU Snapshot Trigger request

  input          dt_cpu0_et_oslock_gclk;              // ETM OS Lock
  input          dt_cpu0_os_double_lock_gclk;         // Debug OS Double Lock
  input          dt_cpu0_halt_ack_gclk;               // Core Halted
  input          dt_cpu0_coredbg_in_reset_gclk;       // Core debug logic is in reset state
  input          dt_cpu0_wfx_dbg_req_gclk;            // Debug request when core is in stand by mode
  input          dt_cpu0_hlt_dbgevt_ok_gclk;          // Halt debug permitted to wake up the cpu from wfi/wfe
  input          dt_cpu0_dbif_ack_gclk;               // Debug Interface Ack
  input          dt_cpu0_dbif_err_gclk;               // Debug Interface Error
  input  [31:0]  dt_cpu0_dbif_rddata_gclk;            // Debug Interface Read Data

  output [3:0]   dt_cpu0_cti_trigout_7to4_pclk;       // Trigger output bits [7:4] to cpu
  output [1:0]   dt_cpu0_cti_trigout_1to0_pclk;       // Trigger output bits [1:0] to cpu
  output [3:0]   dt_cpu0_cti_triginack_7to4_pclk;     // Trigger input ack bits [7:4] to cpu                   
  output [1:0]   dt_cpu0_cti_triginack_1to0_pclk;     // Trigger input ack bits [1:0] to cpu

  input  [3:0]   dt_cpu0_cti_trigin_7to4_gclk;        // Trigger input bits [7:4] from cpu
  input  [1:0]   dt_cpu0_cti_trigin_1to0_gclk;        // Trigger input bits [1:0] from cpu
  input  [3:0]   dt_cpu0_cti_trigoutack_7to4_gclk;    // Trigger output ack bits [7:4] from cpu
  input          dt_cpu0_cti_trigoutack_bit1_gclk;    // Trigger output ack bit 1 from cpu
  
  output         dt_cpu0_wfx_wakeup_pclk;             // WFI/WFE wakeup debug event
  output         dt_cpu0_noclkstop_pclk;              // force CPU clock on from DT-PCLK

//-----------------------------------------------------------------------------
// ucpu1
//-----------------------------------------------------------------------------
  output         dt_cpu1_dbif_req_pclk;               // Debug Interface Req
  output         dt_cpu1_dbif_write_pclk;             // Debug Interface Write/!Read
  output         dt_cpu1_dbif_locked_pclk;            // Debug Interface Lock Value
  output [31:0]  dt_cpu1_dbif_wrdata_pclk;            // Debug Interface Write Data
  output [14:2]  dt_cpu1_dbif_addr_pclk;              // Debug Interface Addr
  output         dt_cpu1_edecr_osuce_pclk;            // OS Unlock Catch Enable Bit 
  output         dt_cpu1_edecr_rce_pclk;              // EDECR Reset Catch Enable Bit
  output         dt_cpu1_edecr_ss_pclk;               // EDECR Halting Step Enable Bit
  output         dt_cpu1_edbgrq_pclk;                 // External Debug Request
  output         dt_cpu1_edacr_frc_idleack_pclk;      // EDACR Force Debug Idle Ack 
  output         dt_cpu1_edprcr_corepurq_pclk;        // PRCR Power Up Request

  input          dt_cpu1_pmusnapshot_ack_gclk;        // PMU Snapshot Trigger acknowledge
  output         dt_cpu1_pmusnapshot_req_pclk;        // PMU Snapshot Trigger request

  input          dt_cpu1_et_oslock_gclk;              // ETM OS Lock
  input          dt_cpu1_os_double_lock_gclk;         // Debug OS Double Lock
  input          dt_cpu1_halt_ack_gclk;               // Core Halted
  input          dt_cpu1_coredbg_in_reset_gclk;       // Core debug logic is in reset state
  input          dt_cpu1_wfx_dbg_req_gclk;            // Debug request when core is in stand by mode
  input          dt_cpu1_hlt_dbgevt_ok_gclk;          // Halt debug permitted to wake up the cpu from wfi/wfe
  input          dt_cpu1_dbif_ack_gclk;               // Debug Interface Ack
  input          dt_cpu1_dbif_err_gclk;               // Debug Interface Error
  input  [31:0]  dt_cpu1_dbif_rddata_gclk;            // Debug Interface Read Data

  output [3:0]   dt_cpu1_cti_trigout_7to4_pclk;       // Trigger output bits [7:4] to cpu
  output [1:0]   dt_cpu1_cti_trigout_1to0_pclk;       // Trigger output bits [1:0] to cpu
  output [3:0]   dt_cpu1_cti_triginack_7to4_pclk;     // Trigger input ack bits [7:4] to cpu                   
  output [1:0]   dt_cpu1_cti_triginack_1to0_pclk;     // Trigger input ack bits [1:0] to cpu

  input  [3:0]   dt_cpu1_cti_trigin_7to4_gclk;        // Trigger input bits [7:4] from cpu
  input  [1:0]   dt_cpu1_cti_trigin_1to0_gclk;        // Trigger input bits [1:0] from cpu
  input  [3:0]   dt_cpu1_cti_trigoutack_7to4_gclk;    // Trigger output ack bits [7:4] from cpu
  input          dt_cpu1_cti_trigoutack_bit1_gclk;    // Trigger output ack bit 1 from cpu

  output         dt_cpu1_wfx_wakeup_pclk;             // WFI/WFE wakeup debug event
  output         dt_cpu1_noclkstop_pclk;              // force CPU clock on from DT-PCLK

//-----------------------------------------------------------------------------
// ucpu2
//-----------------------------------------------------------------------------
  output         dt_cpu2_dbif_req_pclk;               // Debug Interface Req
  output         dt_cpu2_dbif_write_pclk;             // Debug Interface Write/!Read
  output         dt_cpu2_dbif_locked_pclk;            // Debug Interface Lock Value
  output [31:0]  dt_cpu2_dbif_wrdata_pclk;            // Debug Interface Write Data
  output [14:2]  dt_cpu2_dbif_addr_pclk;              // Debug Interface Addr
  output         dt_cpu2_edecr_osuce_pclk;            // OS Unlock Catch Enable Bit 
  output         dt_cpu2_edecr_rce_pclk;              // EDECR Reset Catch Enable Bit
  output         dt_cpu2_edecr_ss_pclk;               // EDECR Halting Step Enable Bit
  output         dt_cpu2_edbgrq_pclk;                 // External Debug Request
  output         dt_cpu2_edacr_frc_idleack_pclk;      // EDACR Force Debug Idle Ack 
  output         dt_cpu2_edprcr_corepurq_pclk;        // PRCR Power Up Request

  input          dt_cpu2_pmusnapshot_ack_gclk;        // PMU Snapshot Trigger acknowledge
  output         dt_cpu2_pmusnapshot_req_pclk;        // PMU Snapshot Trigger request

  input          dt_cpu2_et_oslock_gclk;              // ETM OS Lock
  input          dt_cpu2_os_double_lock_gclk;         // Debug OS Double Lock
  input          dt_cpu2_halt_ack_gclk;               // Core Halted
  input          dt_cpu2_coredbg_in_reset_gclk;       // Core debug logic is in reset state
  input          dt_cpu2_wfx_dbg_req_gclk;            // Debug request when core is in stand by mode
  input          dt_cpu2_hlt_dbgevt_ok_gclk;          // Halt debug permitted to wake up the cpu from wfi/wfe
  input          dt_cpu2_dbif_ack_gclk;               // Debug Interface Ack
  input          dt_cpu2_dbif_err_gclk;               // Debug Interface Error
  input  [31:0]  dt_cpu2_dbif_rddata_gclk;            // Debug Interface Read Data

  output [3:0]   dt_cpu2_cti_trigout_7to4_pclk;       // Trigger output bits [7:4] to cpu
  output [1:0]   dt_cpu2_cti_trigout_1to0_pclk;       // Trigger output bits [1:0] to cpu
  output [3:0]   dt_cpu2_cti_triginack_7to4_pclk;     // Trigger input ack bits [7:4] to cpu                   
  output [1:0]   dt_cpu2_cti_triginack_1to0_pclk;     // Trigger input ack bits [1:0] to cpu

  input  [3:0]   dt_cpu2_cti_trigin_7to4_gclk;        // Trigger input bits [7:4] from cpu
  input  [1:0]   dt_cpu2_cti_trigin_1to0_gclk;        // Trigger input bits [1:0] from cpu
  input  [3:0]   dt_cpu2_cti_trigoutack_7to4_gclk;    // Trigger output ack bits [7:4] from cpu
  input          dt_cpu2_cti_trigoutack_bit1_gclk;    // Trigger output ack bit 1 from cpu

  output         dt_cpu2_wfx_wakeup_pclk;             // WFI/WFE wakeup debug event
  output         dt_cpu2_noclkstop_pclk;              // force CPU clock on from DT-PCLK

//-----------------------------------------------------------------------------
// ucpu3
//-----------------------------------------------------------------------------
  output         dt_cpu3_dbif_req_pclk;               // Debug Interface Req
  output         dt_cpu3_dbif_write_pclk;             // Debug Interface Write/!Read
  output         dt_cpu3_dbif_locked_pclk;            // Debug Interface Lock Value
  output [31:0]  dt_cpu3_dbif_wrdata_pclk;            // Debug Interface Write Data
  output [14:2]  dt_cpu3_dbif_addr_pclk;              // Debug Interface Addr
  output         dt_cpu3_edecr_osuce_pclk;            // OS Unlock Catch Enable Bit 
  output         dt_cpu3_edecr_rce_pclk;              // EDECR Reset Catch Enable Bit
  output         dt_cpu3_edecr_ss_pclk;               // EDECR Halting Step Enable Bit
  output         dt_cpu3_edbgrq_pclk;                 // External Debug Request
  output         dt_cpu3_edacr_frc_idleack_pclk;      // EDACR Force Debug Idle Ack 
  output         dt_cpu3_edprcr_corepurq_pclk;        // PRCR Power Up Request

  input          dt_cpu3_pmusnapshot_ack_gclk;        // PMU Snapshot Trigger acknowledge
  output         dt_cpu3_pmusnapshot_req_pclk;        // PMU Snapshot Trigger request

  input          dt_cpu3_et_oslock_gclk;              // ETM OS Lock
  input          dt_cpu3_os_double_lock_gclk;         // Debug OS Double Lock
  input          dt_cpu3_halt_ack_gclk;               // Core Halted
  input          dt_cpu3_coredbg_in_reset_gclk;       // Core debug logic is in reset state
  input          dt_cpu3_wfx_dbg_req_gclk;            // Debug request when core is in stand by mode
  input          dt_cpu3_hlt_dbgevt_ok_gclk;          // Halt debug permitted to wake up the cpu from wfi/wfe
  input          dt_cpu3_dbif_ack_gclk;               // Debug Interface Ack
  input          dt_cpu3_dbif_err_gclk;               // Debug Interface Error
  input  [31:0]  dt_cpu3_dbif_rddata_gclk;            // Debug Interface Read Data

  output [3:0]   dt_cpu3_cti_trigout_7to4_pclk;       // Trigger output bits [7:4] to cpu
  output [1:0]   dt_cpu3_cti_trigout_1to0_pclk;       // Trigger output bits [1:0] to cpu
  output [3:0]   dt_cpu3_cti_triginack_7to4_pclk;     // Trigger input ack bits [7:4] to cpu                   
  output [1:0]   dt_cpu3_cti_triginack_1to0_pclk;     // Trigger input ack bits [1:0] to cpu

  input  [3:0]   dt_cpu3_cti_trigin_7to4_gclk;        // Trigger input bits [7:4] from cpu
  input  [1:0]   dt_cpu3_cti_trigin_1to0_gclk;        // Trigger input bits [1:0] from cpu
  input  [3:0]   dt_cpu3_cti_trigoutack_7to4_gclk;    // Trigger output ack bits [7:4] from cpu
  input          dt_cpu3_cti_trigoutack_bit1_gclk;    // Trigger output ack bit 1 from cpu

  output         dt_cpu3_wfx_wakeup_pclk;             // WFI/WFE wakeup debug event
  output         dt_cpu3_noclkstop_pclk;              // force CPU clock on from DT-PCLK
// END DT-CPU interface

//-----------------------------------------------------------------------------
// CK interface
//-----------------------------------------------------------------------------
// BEGIN CK-CPU interface
  input          ds_cpu0_reset_req;                   // Warm Reset request
  input          ds_cpu0_wfi_req;                     // WFI request
  input          ds_cpu0_wfe_req;                     // WFI request
  input          ds_cpu0_flush;                       // flush for exception rtn
  input  [5:0]   ds_cpu0_flush_type;                  // flush type
  input          ds_cpu0_imp_abrt_wfi_qual;           // physical abort qual for WFI
  input          ds_cpu0_irq_wfi_qual;                // physical IRQ qual for WFI
  input          ds_cpu0_fiq_wfi_qual;                // physical FIQ qual for WFI
  input          ds_cpu0_vimp_abrt_wfi_qual;          // virtual abort qual for WFI
  input          ds_cpu0_virq_wfi_qual;               // virtual IRQ qual for WFI
  input          ds_cpu0_vfiq_wfi_qual;               // virtual FIQ qual for WFI
  input          ds_cpu0_imp_abrt_wfe_qual;           // physical abort qual for WFE
  input          ds_cpu0_irq_wfe_qual;                // physical IRQ qual for WFE
  input          ds_cpu0_fiq_wfe_qual;                // physical FIQ qual for WFE
  input          ds_cpu0_vimp_abrt_wfe_qual;          // virtual abort qual for WFE
  input          ds_cpu0_virq_wfe_qual;               // virtual IRQ qual for WFE
  input          ds_cpu0_vfiq_wfe_qual;               // virtual FIQ qual for WFE
  input          ds_cpu0_hcr_va;                      // virtual abort
  input          ds_cpu0_hcr_vi;                      // virtual IRQ
  input          ds_cpu0_hcr_vf;                      // virtual FIQ
  input  [2:0]   ds_cpu0_cpuectlr_ret;                // CPU Retention control
  output         ck_cpu0_event_reg;                   // WFE event reg
  output         ck_cpu0_wfi_ack;                     // WFI acknowledge to DS
  output         ck_cpu0_wfe_ack;                     // WFE acknowledge to DS
  output         ck_cpu0_crcx_clk_en_n;               // 2nd-level CPU clock-gating enable

  input          ds_cpu1_reset_req;                   // Warm Reset request
  input          ds_cpu1_wfi_req;                     // WFI request
  input          ds_cpu1_wfe_req;                     // WFI request
  input          ds_cpu1_flush;                       // flush for exception rtn
  input  [5:0]   ds_cpu1_flush_type;                  // flush type
  input          ds_cpu1_imp_abrt_wfi_qual;           // physical abort qual for WFI
  input          ds_cpu1_irq_wfi_qual;                // physical IRQ qual for WFI
  input          ds_cpu1_fiq_wfi_qual;                // physical FIQ qual for WFI
  input          ds_cpu1_vimp_abrt_wfi_qual;          // virtual abort qual for WFI
  input          ds_cpu1_virq_wfi_qual;               // virtual IRQ qual for WFI
  input          ds_cpu1_vfiq_wfi_qual;               // virtual FIQ qual for WFI
  input          ds_cpu1_imp_abrt_wfe_qual;           // physical abort qual for WFE
  input          ds_cpu1_irq_wfe_qual;                // physical IRQ qual for WFE
  input          ds_cpu1_fiq_wfe_qual;                // physical FIQ qual for WFE
  input          ds_cpu1_vimp_abrt_wfe_qual;          // virtual abort qual for WFE
  input          ds_cpu1_virq_wfe_qual;               // virtual IRQ qual for WFE
  input          ds_cpu1_vfiq_wfe_qual;               // virtual FIQ qual for WFE
  input          ds_cpu1_hcr_va;                      // virtual abort
  input          ds_cpu1_hcr_vi;                      // virtual IRQ
  input          ds_cpu1_hcr_vf;                      // virtual FIQ
  input  [2:0]   ds_cpu1_cpuectlr_ret;                // CPU Retention control
  output         ck_cpu1_event_reg;                   // WFE event reg
  output         ck_cpu1_wfi_ack;                     // WFI acknowledge to DS
  output         ck_cpu1_wfe_ack;                     // WFE acknowledge to DS
  output         ck_cpu1_crcx_clk_en_n;               // 2nd-level CPU clock-gating enable

  input          ds_cpu2_reset_req;                   // Warm Reset request
  input          ds_cpu2_wfi_req;                     // WFI request
  input          ds_cpu2_wfe_req;                     // WFI request
  input          ds_cpu2_flush;                       // flush for exception rtn
  input  [5:0]   ds_cpu2_flush_type;                  // flush type
  input          ds_cpu2_imp_abrt_wfi_qual;           // physical abort qual for WFI
  input          ds_cpu2_irq_wfi_qual;                // physical IRQ qual for WFI
  input          ds_cpu2_fiq_wfi_qual;                // physical FIQ qual for WFI
  input          ds_cpu2_vimp_abrt_wfi_qual;          // virtual abort qual for WFI
  input          ds_cpu2_virq_wfi_qual;               // virtual IRQ qual for WFI
  input          ds_cpu2_vfiq_wfi_qual;               // virtual FIQ qual for WFI
  input          ds_cpu2_imp_abrt_wfe_qual;           // physical abort qual for WFE
  input          ds_cpu2_irq_wfe_qual;                // physical IRQ qual for WFE
  input          ds_cpu2_fiq_wfe_qual;                // physical FIQ qual for WFE
  input          ds_cpu2_vimp_abrt_wfe_qual;          // virtual abort qual for WFE
  input          ds_cpu2_virq_wfe_qual;               // virtual IRQ qual for WFE
  input          ds_cpu2_vfiq_wfe_qual;               // virtual FIQ qual for WFE
  input          ds_cpu2_hcr_va;                      // virtual abort
  input          ds_cpu2_hcr_vi;                      // virtual IRQ
  input          ds_cpu2_hcr_vf;                      // virtual FIQ
  input  [2:0]   ds_cpu2_cpuectlr_ret;                // CPU Retention control
  output         ck_cpu2_event_reg;                   // WFE event reg
  output         ck_cpu2_wfi_ack;                     // WFI acknowledge to DS
  output         ck_cpu2_wfe_ack;                     // WFE acknowledge to DS
  output         ck_cpu2_crcx_clk_en_n;               // 2nd-level CPU clock-gating enable

  input          ds_cpu3_reset_req;                   // Warm Reset request
  input          ds_cpu3_wfi_req;                     // WFI request
  input          ds_cpu3_wfe_req;                     // WFI request
  input          ds_cpu3_flush;                       // flush for exception rtn
  input  [5:0]   ds_cpu3_flush_type;                  // flush type
  input          ds_cpu3_imp_abrt_wfi_qual;           // physical abort qual for WFI
  input          ds_cpu3_irq_wfi_qual;                // physical IRQ qual for WFI
  input          ds_cpu3_fiq_wfi_qual;                // physical FIQ qual for WFI
  input          ds_cpu3_vimp_abrt_wfi_qual;          // virtual abort qual for WFI
  input          ds_cpu3_virq_wfi_qual;               // virtual IRQ qual for WFI
  input          ds_cpu3_vfiq_wfi_qual;               // virtual FIQ qual for WFI
  input          ds_cpu3_imp_abrt_wfe_qual;           // physical abort qual for WFE
  input          ds_cpu3_irq_wfe_qual;                // physical IRQ qual for WFE
  input          ds_cpu3_fiq_wfe_qual;                // physical FIQ qual for WFE
  input          ds_cpu3_vimp_abrt_wfe_qual;          // virtual abort qual for WFE
  input          ds_cpu3_virq_wfe_qual;               // virtual IRQ qual for WFE
  input          ds_cpu3_vfiq_wfe_qual;               // virtual FIQ qual for WFE
  input          ds_cpu3_hcr_va;                      // virtual abort
  input          ds_cpu3_hcr_vi;                      // virtual IRQ
  input          ds_cpu3_hcr_vf;                      // virtual FIQ
  input  [2:0]   ds_cpu3_cpuectlr_ret;                // CPU Retention control
  output         ck_cpu3_event_reg;                   // WFE event reg
  output         ck_cpu3_wfi_ack;                     // WFI acknowledge to DS
  output         ck_cpu3_wfe_ack;                     // WFE acknowledge to DS
  output         ck_cpu3_crcx_clk_en_n;               // 2nd-level CPU clock-gating enable

  input          ls_cpu0_clrexmon;                    // LS global exclusive monitor
  input          ls_cpu1_clrexmon;                    // LS global exclusive monitor
  input          ls_cpu2_clrexmon;                    // LS global exclusive monitor
  input          ls_cpu3_clrexmon;                    // LS global exclusive monitor

// END CK-CPU interface

  output [`MAIA_CN:0] ck_gclkt;



    // wires
    wire              ck_areset_l2;
    wire              ck_cpu0_areset_l2cpu;
    wire              ck_cpu0_areset_l2dt;
    wire              ck_cpu0_commrx;
    wire              ck_cpu0_commtx;
    wire              ck_cpu0_crcx_clk_en_n_ic;
    wire              ck_cpu0_dbgnopwrdwn;
    wire              ck_cpu0_dbgrstreq;
    wire              ck_cpu0_dt_standbywfx;
    wire              ck_cpu0_dt_wfx_ack;
    wire              ck_cpu0_l2_standbywfi;
    wire              ck_cpu0_l2_standbywfx;
    wire              ck_cpu0_ncommirq;
    wire              ck_cpu0_npmuirq;
    wire              ck_cpu0_poreset_status;
    wire              ck_cpu0_reset1_n_l2cpu;
    wire              ck_cpu0_reset1_n_l2dt;
    wire              ck_cpu1_areset_l2cpu;
    wire              ck_cpu1_areset_l2dt;
    wire              ck_cpu1_commrx;
    wire              ck_cpu1_commtx;
    wire              ck_cpu1_crcx_clk_en_n_ic;
    wire              ck_cpu1_dbgnopwrdwn;
    wire              ck_cpu1_dbgrstreq;
    wire              ck_cpu1_dt_standbywfx;
    wire              ck_cpu1_dt_wfx_ack;
    wire              ck_cpu1_l2_standbywfi;
    wire              ck_cpu1_l2_standbywfx;
    wire              ck_cpu1_ncommirq;
    wire              ck_cpu1_npmuirq;
    wire              ck_cpu1_poreset_status;
    wire              ck_cpu1_reset1_n_l2cpu;
    wire              ck_cpu1_reset1_n_l2dt;
    wire              ck_cpu2_areset_l2cpu;
    wire              ck_cpu2_areset_l2dt;
    wire              ck_cpu2_commrx;
    wire              ck_cpu2_commtx;
    wire              ck_cpu2_crcx_clk_en_n_ic;
    wire              ck_cpu2_dbgnopwrdwn;
    wire              ck_cpu2_dbgrstreq;
    wire              ck_cpu2_dt_standbywfx;
    wire              ck_cpu2_dt_wfx_ack;
    wire              ck_cpu2_l2_standbywfi;
    wire              ck_cpu2_l2_standbywfx;
    wire              ck_cpu2_ncommirq;
    wire              ck_cpu2_npmuirq;
    wire              ck_cpu2_poreset_status;
    wire              ck_cpu2_reset1_n_l2cpu;
    wire              ck_cpu2_reset1_n_l2dt;
    wire              ck_cpu3_areset_l2cpu;
    wire              ck_cpu3_areset_l2dt;
    wire              ck_cpu3_commrx;
    wire              ck_cpu3_commtx;
    wire              ck_cpu3_crcx_clk_en_n_ic;
    wire              ck_cpu3_dbgnopwrdwn;
    wire              ck_cpu3_dbgrstreq;
    wire              ck_cpu3_dt_standbywfx;
    wire              ck_cpu3_dt_wfx_ack;
    wire              ck_cpu3_l2_standbywfi;
    wire              ck_cpu3_l2_standbywfx;
    wire              ck_cpu3_ncommirq;
    wire              ck_cpu3_npmuirq;
    wire              ck_cpu3_poreset_status;
    wire              ck_cpu3_reset1_n_l2cpu;
    wire              ck_cpu3_reset1_n_l2dt;
    wire              ck_dt_cpu0_coredbg_in_reset_gclk;
    wire [1:0]        ck_dt_cpu0_cti_trigin_1to0_gclk;
    wire              ck_dt_cpu0_et_oslock_gclk;
    wire              ck_dt_cpu0_hlt_dbgevt_ok_gclk;
    wire              ck_dt_cpu0_os_double_lock_gclk;
    wire              ck_dt_cpu0_pmusnapshot_ack_gclk;
    wire              ck_dt_cpu0_wfx_dbg_req_gclk;
    wire              ck_dt_cpu1_coredbg_in_reset_gclk;
    wire [1:0]        ck_dt_cpu1_cti_trigin_1to0_gclk;
    wire              ck_dt_cpu1_et_oslock_gclk;
    wire              ck_dt_cpu1_hlt_dbgevt_ok_gclk;
    wire              ck_dt_cpu1_os_double_lock_gclk;
    wire              ck_dt_cpu1_pmusnapshot_ack_gclk;
    wire              ck_dt_cpu1_wfx_dbg_req_gclk;
    wire              ck_dt_cpu2_coredbg_in_reset_gclk;
    wire [1:0]        ck_dt_cpu2_cti_trigin_1to0_gclk;
    wire              ck_dt_cpu2_et_oslock_gclk;
    wire              ck_dt_cpu2_hlt_dbgevt_ok_gclk;
    wire              ck_dt_cpu2_os_double_lock_gclk;
    wire              ck_dt_cpu2_pmusnapshot_ack_gclk;
    wire              ck_dt_cpu2_wfx_dbg_req_gclk;
    wire              ck_dt_cpu3_coredbg_in_reset_gclk;
    wire [1:0]        ck_dt_cpu3_cti_trigin_1to0_gclk;
    wire              ck_dt_cpu3_et_oslock_gclk;
    wire              ck_dt_cpu3_hlt_dbgevt_ok_gclk;
    wire              ck_dt_cpu3_os_double_lock_gclk;
    wire              ck_dt_cpu3_pmusnapshot_ack_gclk;
    wire              ck_dt_cpu3_wfx_dbg_req_gclk;
    wire              ck_gclkb0;
    wire              ck_gclkb1;
    wire              ck_gclkfr;
    wire              ck_gclkl2;
    wire              ck_gclktl2;
    wire              ck_l2_ace_inactive;
    wire              ck_l2_acp_inactive;
    wire              ck_l2_logic_clk_en;
    wire              ck_l2_sky_link_deactivate;
    wire              ck_l2_tbnk0_clk_en;
    wire              ck_l2_tbnk1_clk_en;
    wire              ck_reset1_n_l2;
    wire              clrexmon_c1;
    wire              ds_cpu0_ic_aa64naa32_i;
    wire [4:0]        ds_cpu0_ic_cpsr_mode_i;
    wire              ds_cpu0_ic_hcr_change_i;
    wire              ds_cpu0_ic_sample_spr_i;
    wire              ds_cpu0_ic_scr_change_i;
    wire              ds_cpu1_ic_aa64naa32_i;
    wire [4:0]        ds_cpu1_ic_cpsr_mode_i;
    wire              ds_cpu1_ic_hcr_change_i;
    wire              ds_cpu1_ic_sample_spr_i;
    wire              ds_cpu1_ic_scr_change_i;
    wire              ds_cpu2_ic_aa64naa32_i;
    wire [4:0]        ds_cpu2_ic_cpsr_mode_i;
    wire              ds_cpu2_ic_hcr_change_i;
    wire              ds_cpu2_ic_sample_spr_i;
    wire              ds_cpu2_ic_scr_change_i;
    wire              ds_cpu3_ic_aa64naa32_i;
    wire [4:0]        ds_cpu3_ic_cpsr_mode_i;
    wire              ds_cpu3_ic_hcr_change_i;
    wire              ds_cpu3_ic_sample_spr_i;
    wire              ds_cpu3_ic_scr_change_i;
    wire              dt_cpu0_apb_active_pclk;
    wire              dt_cpu0_poreset_status_ack_pclk;
    wire              dt_cpu0_trcauxctlr_sb_rcg_disable_pclk;
    wire              dt_cpu0_wfx_wakeup_pclk;
    wire              dt_cpu1_apb_active_pclk;
    wire              dt_cpu1_poreset_status_ack_pclk;
    wire              dt_cpu1_trcauxctlr_sb_rcg_disable_pclk;
    wire              dt_cpu1_wfx_wakeup_pclk;
    wire              dt_cpu2_apb_active_pclk;
    wire              dt_cpu2_poreset_status_ack_pclk;
    wire              dt_cpu2_trcauxctlr_sb_rcg_disable_pclk;
    wire              dt_cpu2_wfx_wakeup_pclk;
    wire              dt_cpu3_apb_active_pclk;
    wire              dt_cpu3_poreset_status_ack_pclk;
    wire              dt_cpu3_trcauxctlr_sb_rcg_disable_pclk;
    wire              dt_cpu3_wfx_wakeup_pclk;
    wire              eventi_sev;
    wire [`MAIA_CN:0] ic_block_eoi_sgi_wr_o;
    wire              ic_cpu0_l2_dsb_block;
    wire [63:0]       ic_cpu0_spr_rd_data;
    wire              ic_cpu1_l2_dsb_block;
    wire [63:0]       ic_cpu1_spr_rd_data;
    wire              ic_cpu2_l2_dsb_block;
    wire [63:0]       ic_cpu2_spr_rd_data;
    wire              ic_cpu3_l2_dsb_block;
    wire [63:0]       ic_cpu3_spr_rd_data;
    wire [`MAIA_CN:0] ic_el_change_complete_o;
    wire [`MAIA_CN:0] ic_hcr_change_complete_o;
    wire [`MAIA_CN:0] ic_ich_el2_tall0_o;
    wire [`MAIA_CN:0] ic_ich_el2_tall1_o;
    wire [`MAIA_CN:0] ic_ich_el2_tc_o;
    wire [`MAIA_CN:0] ic_nfiq_o;
    wire [`MAIA_CN:0] ic_nirq_o;
    wire [`MAIA_CN:0] ic_nsei_o;
    wire [`MAIA_CN:0] ic_nvfiq_o;
    wire [`MAIA_CN:0] ic_nvirq_o;
    wire [`MAIA_CN:0] ic_nvsei_o;
    wire [31:0]       ic_p_rdata;
    wire              ic_p_rdata_valid;
    wire              ic_p_ready;
    wire [`MAIA_CN:0] ic_sample_spr_o;
    wire [`MAIA_CN:0] ic_scr_change_complete_o;
    wire [`MAIA_CN:0] ic_sra_el1ns_en_o;
    wire [`MAIA_CN:0] ic_sra_el1s_en_o;
    wire [`MAIA_CN:0] ic_sra_el2_en_o;
    wire [`MAIA_CN:0] ic_sra_el3_en_o;
    wire [`MAIA_CN:0] ic_sre_el1ns_hyp_trap_o;
    wire [`MAIA_CN:0] ic_sre_el1ns_mon_trap_o;
    wire [`MAIA_CN:0] ic_sre_el1s_mon_trap_o;
    wire [`MAIA_CN:0] ic_sre_el2_mon_trap_o;
    wire              l2_acp_flsh_rd_cnt_active_glb_l2_dly;
    wire              l2_acp_flsh_wr_cnt_active_glb_l2_dly;
    wire              l2_acp_rd_haz_vld_l2_dly_q;
    wire              l2_acp_wr_haz_vld_l2_dly_q;
    wire              l2_actlr_disable_b2b_setway_hzd_opt_x2_ns;
    wire              l2_actlr_disable_setway_opt;
    wire              l2_actlr_ncpu_rcg_enable;
    wire              l2_actlr_plru_dynamic;
    wire              l2_actlr_plru_en;
    wire [1:0]        l2_actlr_plru_mode;
    wire              l2_actlr_writeunique_disable;
    wire              l2_cfg_broadcastinner;
    wire              l2_cfg_broadcastouter;
    wire              l2_cpu0_ls_rd_haz_vld_l2_dly_q;
    wire              l2_cpu0_ls_wr_haz_vld_l2_dly_q;
    wire              l2_cpu0_snp_active;
    wire              l2_cpu0_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu0_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu0_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu0_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu0_wr_decerr_q;
    wire              l2_cpu0_wr_slverr_q;
    wire              l2_cpu1_ls_rd_haz_vld_l2_dly_q;
    wire              l2_cpu1_ls_wr_haz_vld_l2_dly_q;
    wire              l2_cpu1_snp_active;
    wire              l2_cpu1_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu1_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu1_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu1_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu1_wr_decerr_q;
    wire              l2_cpu1_wr_slverr_q;
    wire              l2_cpu2_ls_rd_haz_vld_l2_dly_q;
    wire              l2_cpu2_ls_wr_haz_vld_l2_dly_q;
    wire              l2_cpu2_snp_active;
    wire              l2_cpu2_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu2_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu2_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu2_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu2_wr_decerr_q;
    wire              l2_cpu2_wr_slverr_q;
    wire              l2_cpu3_ls_rd_haz_vld_l2_dly_q;
    wire              l2_cpu3_ls_wr_haz_vld_l2_dly_q;
    wire              l2_cpu3_snp_active;
    wire              l2_cpu3_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu3_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu3_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire              l2_cpu3_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu3_wr_decerr_q;
    wire              l2_cpu3_wr_slverr_q;
    wire              l2_ctlr_x1_wr_q;
    wire [9:0]        l2_ctlr_x2_ns;
    wire              l2_idle;
    wire [`MAIA_CN:0] l2_mbist1_en_b1;
    wire [16:0]       l2_mbist2_tbnk0_addr_b1;
    wire              l2_mbist2_tbnk0_all_b1;
    wire [2:0]        l2_mbist2_tbnk0_array_b1;
    wire [17:0]       l2_mbist2_tbnk0_be_b1;
    wire              l2_mbist2_tbnk0_en_b1;
    wire [143:0]      l2_mbist2_tbnk0_indata_b1;
    wire [143:0]      l2_mbist2_tbnk0_outdata_b3;
    wire              l2_mbist2_tbnk0_sel_b1;
    wire [79:0]       l2_mbist2_tbnk0_snp0_outdata_b2;
    wire              l2_mbist2_tbnk0_snp0_outdata_vld_b2;
    wire              l2_mbist2_tbnk0_snp0_sel_b1;
    wire [79:0]       l2_mbist2_tbnk0_snp1_outdata_b2;
    wire              l2_mbist2_tbnk0_snp1_outdata_vld_b2;
    wire              l2_mbist2_tbnk0_snp1_sel_b1;
    wire [79:0]       l2_mbist2_tbnk0_snp2_outdata_b2;
    wire              l2_mbist2_tbnk0_snp2_outdata_vld_b2;
    wire              l2_mbist2_tbnk0_snp2_sel_b1;
    wire [79:0]       l2_mbist2_tbnk0_snp3_outdata_b2;
    wire              l2_mbist2_tbnk0_snp3_outdata_vld_b2;
    wire              l2_mbist2_tbnk0_snp3_sel_b1;
    wire              l2_mbist2_tbnk0_wr_en_b1;
    wire [16:0]       l2_mbist2_tbnk1_addr_b1;
    wire              l2_mbist2_tbnk1_all_b1;
    wire [2:0]        l2_mbist2_tbnk1_array_b1;
    wire [17:0]       l2_mbist2_tbnk1_be_b1;
    wire              l2_mbist2_tbnk1_en_b1;
    wire [143:0]      l2_mbist2_tbnk1_indata_b1;
    wire [143:0]      l2_mbist2_tbnk1_outdata_b3;
    wire              l2_mbist2_tbnk1_sel_b1;
    wire [79:0]       l2_mbist2_tbnk1_snp0_outdata_b2;
    wire              l2_mbist2_tbnk1_snp0_outdata_vld_b2;
    wire              l2_mbist2_tbnk1_snp0_sel_b1;
    wire [79:0]       l2_mbist2_tbnk1_snp1_outdata_b2;
    wire              l2_mbist2_tbnk1_snp1_outdata_vld_b2;
    wire              l2_mbist2_tbnk1_snp1_sel_b1;
    wire [79:0]       l2_mbist2_tbnk1_snp2_outdata_b2;
    wire              l2_mbist2_tbnk1_snp2_outdata_vld_b2;
    wire              l2_mbist2_tbnk1_snp2_sel_b1;
    wire [79:0]       l2_mbist2_tbnk1_snp3_outdata_b2;
    wire              l2_mbist2_tbnk1_snp3_outdata_vld_b2;
    wire              l2_mbist2_tbnk1_snp3_sel_b1;
    wire              l2_mbist2_tbnk1_wr_en_b1;
    wire              l2_no_ram_acc_nxt_cycle;
    wire [13:0]       l2_p_addr;
    wire [1:0]        l2_p_cpu;
    wire              l2_p_nsecure;
    wire [2:0]        l2_p_sel;
    wire [31:0]       l2_p_wdata;
    wire              l2_p_write;
    wire              l2_reset3;
    wire              l2_rstdisable_x1_q;
    wire              l2_sky_link_stopped;
    wire              l2_tbnk0_addr44_l3_q;
    wire [44:0]       l2_tbnk0_addr_l1;
    wire [5:2]        l2_tbnk0_addr_l6;
    wire              l2_tbnk0_all_tag_incl_active_l3;
    wire              l2_tbnk0_asq_cmp_evict_l3_q;
    wire              l2_tbnk0_asq_full_flsh;
    wire              l2_tbnk0_asq_nc_so_dev_limit;
    wire [2:0]        l2_tbnk0_cache_attr_l1;
    wire              l2_tbnk0_cfg_ecc_en;
    wire              l2_tbnk0_cmo_setway_l2_inv_incl_l4;
    wire              l2_tbnk0_cpu0_ccb_xfer_l4_dly2;
    wire              l2_tbnk0_cpu0_hit_l4;
    wire              l2_tbnk0_cpu0_l2_inv_l4_dly2;
    wire              l2_tbnk0_cpu0_l2hit_e_l4;
    wire              l2_tbnk0_cpu0_l2hit_s_l4;
    wire              l2_tbnk0_cpu0_peq_full_q;
    wire              l2_tbnk0_cpu0_peq_hit_q;
    wire              l2_tbnk0_cpu0_peq_self_evict_l3_q;
    wire              l2_tbnk0_cpu0_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk0_cpu0_rd_access_l4_dly;
    wire              l2_tbnk0_cpu0_self_evict_l4_dly_q;
    wire              l2_tbnk0_cpu0_single_ecc_err_l7_q;
    wire              l2_tbnk0_cpu0_snp_hit_e_l3;
    wire              l2_tbnk0_cpu0_snp_hit_s_l3;
    wire [44:14]      l2_tbnk0_cpu0_snp_setway_addr_l3;
    wire              l2_tbnk0_cpu0_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk0_cpu0_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk0_cpu0_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk0_cpu0_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk0_cpu0_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk0_cpu0_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk0_cpu0_vld_nxt_l5;
    wire              l2_tbnk0_cpu0_wr_access_l4_dly;
    wire              l2_tbnk0_cpu1_ccb_xfer_l4_dly2;
    wire              l2_tbnk0_cpu1_hit_l4;
    wire              l2_tbnk0_cpu1_l2_inv_l4_dly2;
    wire              l2_tbnk0_cpu1_l2hit_e_l4;
    wire              l2_tbnk0_cpu1_l2hit_s_l4;
    wire              l2_tbnk0_cpu1_peq_full_q;
    wire              l2_tbnk0_cpu1_peq_hit_q;
    wire              l2_tbnk0_cpu1_peq_self_evict_l3_q;
    wire              l2_tbnk0_cpu1_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk0_cpu1_rd_access_l4_dly;
    wire              l2_tbnk0_cpu1_self_evict_l4_dly_q;
    wire              l2_tbnk0_cpu1_single_ecc_err_l7_q;
    wire              l2_tbnk0_cpu1_snp_hit_e_l3;
    wire              l2_tbnk0_cpu1_snp_hit_s_l3;
    wire [44:14]      l2_tbnk0_cpu1_snp_setway_addr_l3;
    wire              l2_tbnk0_cpu1_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk0_cpu1_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk0_cpu1_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk0_cpu1_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk0_cpu1_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk0_cpu1_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk0_cpu1_vld_nxt_l5;
    wire              l2_tbnk0_cpu1_wr_access_l4_dly;
    wire              l2_tbnk0_cpu2_ccb_xfer_l4_dly2;
    wire              l2_tbnk0_cpu2_hit_l4;
    wire              l2_tbnk0_cpu2_l2_inv_l4_dly2;
    wire              l2_tbnk0_cpu2_l2hit_e_l4;
    wire              l2_tbnk0_cpu2_l2hit_s_l4;
    wire              l2_tbnk0_cpu2_peq_full_q;
    wire              l2_tbnk0_cpu2_peq_hit_q;
    wire              l2_tbnk0_cpu2_peq_self_evict_l3_q;
    wire              l2_tbnk0_cpu2_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk0_cpu2_rd_access_l4_dly;
    wire              l2_tbnk0_cpu2_self_evict_l4_dly_q;
    wire              l2_tbnk0_cpu2_single_ecc_err_l7_q;
    wire              l2_tbnk0_cpu2_snp_hit_e_l3;
    wire              l2_tbnk0_cpu2_snp_hit_s_l3;
    wire [44:14]      l2_tbnk0_cpu2_snp_setway_addr_l3;
    wire              l2_tbnk0_cpu2_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk0_cpu2_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk0_cpu2_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk0_cpu2_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk0_cpu2_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk0_cpu2_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk0_cpu2_vld_nxt_l5;
    wire              l2_tbnk0_cpu2_wr_access_l4_dly;
    wire              l2_tbnk0_cpu3_ccb_xfer_l4_dly2;
    wire              l2_tbnk0_cpu3_hit_l4;
    wire              l2_tbnk0_cpu3_l2_inv_l4_dly2;
    wire              l2_tbnk0_cpu3_l2hit_e_l4;
    wire              l2_tbnk0_cpu3_l2hit_s_l4;
    wire              l2_tbnk0_cpu3_peq_full_q;
    wire              l2_tbnk0_cpu3_peq_hit_q;
    wire              l2_tbnk0_cpu3_peq_self_evict_l3_q;
    wire              l2_tbnk0_cpu3_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk0_cpu3_rd_access_l4_dly;
    wire              l2_tbnk0_cpu3_self_evict_l4_dly_q;
    wire              l2_tbnk0_cpu3_single_ecc_err_l7_q;
    wire              l2_tbnk0_cpu3_snp_hit_e_l3;
    wire              l2_tbnk0_cpu3_snp_hit_s_l3;
    wire [44:14]      l2_tbnk0_cpu3_snp_setway_addr_l3;
    wire              l2_tbnk0_cpu3_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk0_cpu3_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk0_cpu3_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk0_cpu3_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk0_cpu3_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk0_cpu3_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk0_cpu3_vld_nxt_l5;
    wire              l2_tbnk0_cpu3_wr_access_l4_dly;
    wire [3:0]        l2_tbnk0_cpu_rvalid_init_nxt_l5;
    wire [3:0]        l2_tbnk0_cpu_rvalid_nxt_l5;
    wire [3:0]        l2_tbnk0_cpu_snp_hit_e_l4_q;
    wire              l2_tbnk0_crit_qw_nxt_l5;
    wire [143:0]      l2_tbnk0_data_corrected_l7_q;
    wire [127:0]      l2_tbnk0_data_l6;
    wire              l2_tbnk0_dbg_ram_acc_l5a;
    wire [2:0]        l2_tbnk0_dbg_ram_acc_unit_nxt;
    wire [7:0]        l2_tbnk0_dbg_ram_id_nxt_l5;
    wire              l2_tbnk0_dirty_l1;
    wire              l2_tbnk0_dirty_l3_q;
    wire              l2_tbnk0_dis_ns_dbg_arr_acc_x2;
    wire              l2_tbnk0_double_ecc_err_l7_q;
    wire              l2_tbnk0_early_rvalid_l4_q;
    wire              l2_tbnk0_ecc_fixup_blk_arb;
    wire              l2_tbnk0_ecc_fixup_inprog_dly_q;
    wire              l2_tbnk0_ecc_rmw_snp_tag_rd_l3_q;
    wire [31:0]       l2_tbnk0_ecc_syndrome_reg_q;
    wire              l2_tbnk0_evict_special_hazard_l3_q;
    wire              l2_tbnk0_evict_special_hazard_rwvic_l3_q;
    wire              l2_tbnk0_excl_l1;
    wire              l2_tbnk0_excl_l4_q;
    wire [44:6]       l2_tbnk0_feq_addr_upd;
    wire              l2_tbnk0_feq_alloc_failed_l4;
    wire              l2_tbnk0_feq_axi_wr_vld_not_popped;
    wire              l2_tbnk0_feq_clr_l4;
    wire [15:0]       l2_tbnk0_feq_frc_incl_l3a;
    wire              l2_tbnk0_feq_kill_l3;
    wire [4:0]        l2_tbnk0_feq_last_id_q;
    wire              l2_tbnk0_feq_tbnk_id_update_hit_prfm_or_l3;
    wire              l2_tbnk0_feq_tbnk_id_update_or_l3;
    wire              l2_tbnk0_full_miss_l4_q;
    wire              l2_tbnk0_hit_l4;
    wire              l2_tbnk0_hit_l7_q;
    wire [3:0]        l2_tbnk0_hit_way_l4_q;
    wire [9:0]        l2_tbnk0_id_l1;
    wire [9:0]        l2_tbnk0_id_l6_q;
    wire [9:0]        l2_tbnk0_id_nxt_l5;
    wire              l2_tbnk0_idle;
    wire              l2_tbnk0_init_req_l1;
    wire              l2_tbnk0_kill_l2;
    wire              l2_tbnk0_l2bb_fake_wr_l1;
    wire              l2_tbnk0_l2bb_wr_l1;
    wire              l2_tbnk0_l2hit_e_l4;
    wire              l2_tbnk0_l2hit_s_l4;
    wire              l2_tbnk0_l2v_s_q;
    wire              l2_tbnk0_l2v_vld_q;
    wire              l2_tbnk0_last_qw_l1;
    wire              l2_tbnk0_last_qw_l6_q;
    wire              l2_tbnk0_last_qw_nxt_l5;
    wire [2:0]        l2_tbnk0_lock_l1;
    wire [2:0]        l2_tbnk0_lock_l4;
    wire [32:0]       l2_tbnk0_merrsr_data;
    wire [9:0]        l2_tbnk0_page_attr_l1;
    wire              l2_tbnk0_partial_dw_wr_l1;
    wire              l2_tbnk0_pf_cnt_dec_l4_dly;
    wire              l2_tbnk0_pf_hazard_l3;
    wire              l2_tbnk0_pf_req_sel_for_fwd_l4;
    wire              l2_tbnk0_prfm_l1;
    wire              l2_tbnk0_prfm_nxt_l5;
    wire [3:0]        l2_tbnk0_prot_l1;
    wire [3:0]        l2_tbnk0_prot_l4_q;
    wire [1:0]        l2_tbnk0_qw_cnt_l1;
    wire [1:0]        l2_tbnk0_qw_cnt_l3_q;
    wire              l2_tbnk0_raw_hit_l4_q;
    wire [2:0]        l2_tbnk0_rbufid_nxt_l5;
    wire              l2_tbnk0_rd_en_nxt_l5;
    wire              l2_tbnk0_rd_fail_hazchk_feq_l3;
    wire              l2_tbnk0_rwvic_axi_read_err_l1;
    wire              l2_tbnk0_rwvic_axi_read_err_l3_q;
    wire              l2_tbnk0_rwvic_ccb_dirty_l6_q;
    wire              l2_tbnk0_rwvic_ccb_ls_xfer_l1;
    wire              l2_tbnk0_rwvic_ccb_ls_xfer_l3_q;
    wire              l2_tbnk0_rwvic_ccb_ls_xfer_l6_q;
    wire [3:0]        l2_tbnk0_rwvic_ccb_way_l1;
    wire              l2_tbnk0_rwvic_cmo_clean_l1;
    wire              l2_tbnk0_rwvic_cmo_inv_l1;
    wire              l2_tbnk0_rwvic_cmo_inv_l7_q;
    wire              l2_tbnk0_rwvic_cmo_l7_q;
    wire              l2_tbnk0_rwvic_cmo_pou_l1;
    wire              l2_tbnk0_rwvic_cmo_pou_l6_q;
    wire              l2_tbnk0_rwvic_cmo_setway_l1;
    wire              l2_tbnk0_rwvic_cmo_setway_ls_full_miss_l1;
    wire              l2_tbnk0_rwvic_cmo_setway_ls_l6_q;
    wire [2:0]        l2_tbnk0_rwvic_cpu_fb_id_l1;
    wire [3:0]        l2_tbnk0_rwvic_cpu_id_dcd_l1;
    wire              l2_tbnk0_rwvic_ddi_l6_q;
    wire              l2_tbnk0_rwvic_feq_cmp_l3_q;
    wire              l2_tbnk0_rwvic_frc_l2hit_fwd_l1;
    wire              l2_tbnk0_rwvic_l2hit_e_l1;
    wire              l2_tbnk0_rwvic_l2hit_e_l3_q;
    wire              l2_tbnk0_rwvic_l2hit_e_l7_q;
    wire              l2_tbnk0_rwvic_l2v_dirty_l7_q;
    wire [3:0]        l2_tbnk0_rwvic_l2v_page_attr_l7_q;
    wire              l2_tbnk0_rwvic_l2v_vld_l6_q;
    wire              l2_tbnk0_rwvic_mesi_sh_l1;
    wire              l2_tbnk0_rwvic_non_snp_fail_hazchk_l3;
    wire [2:0]        l2_tbnk0_rwvic_owner_l1;
    wire [2:0]        l2_tbnk0_rwvic_owner_l7_q;
    wire              l2_tbnk0_rwvic_rd_type_l6_q;
    wire              l2_tbnk0_rwvic_snp_clr_dirty_l1;
    wire              l2_tbnk0_rwvic_snp_inv_l1;
    wire              l2_tbnk0_rwvic_snp_l1;
    wire              l2_tbnk0_rwvic_snp_l3_q;
    wire              l2_tbnk0_rwvic_snp_l6_q;
    wire              l2_tbnk0_rwvic_tag_wr_l0;
    wire [3:0]        l2_tbnk0_rwvic_type_l1;
    wire              l2_tbnk0_rwvic_wa_l1;
    wire              l2_tbnk0_rwvic_wa_l6_q;
    wire [13:0]       l2_tbnk0_sel_l1;
    wire [2:0]        l2_tbnk0_size_l1;
    wire [2:0]        l2_tbnk0_size_l4_q;
    wire              l2_tbnk0_snp_byp_peq_haz_pending_q;
    wire              l2_tbnk0_snp_dvm_cmpl_l1;
    wire              l2_tbnk0_snp_hit_e_l4_q;
    wire              l2_tbnk0_snp_hit_feq_evict_l4_dly;
    wire              l2_tbnk0_snp_hit_s_l4_q;
    wire [4:0]        l2_tbnk0_snp_rd_feq_id_cmp_l4_dly_q;
    wire [7:0]        l2_tbnk0_snp_tag_double_ecc_err_l4_dly_q;
    wire [7:0]        l2_tbnk0_snp_tag_single_ecc_err_l4_dly_q;
    wire [44:7]       l2_tbnk0_snp_tag_wr_l2_hit_addr_l1;
    wire [1:0]        l2_tbnk0_snp_tag_wr_l2_hit_state_l1;
    wire              l2_tbnk0_snp_tag_wr_l2_hit_way_l1;
    wire              l2_tbnk0_special_evict_hazard_l3;
    wire              l2_tbnk0_special_hazard_l3_q;
    wire              l2_tbnk0_sync_l1;
    wire              l2_tbnk0_tag_ecc_dbl_rmw_wr_l1;
    wire              l2_tbnk0_tag_ecc_err_cpu0_l4;
    wire              l2_tbnk0_tag_ecc_err_cpu1_l4;
    wire              l2_tbnk0_tag_ecc_err_cpu2_l4;
    wire              l2_tbnk0_tag_ecc_err_cpu3_l4;
    wire              l2_tbnk0_tag_ecc_err_l4;
    wire [6:0]        l2_tbnk0_type_l1;
    wire [1:0]        l2_tbnk0_ulen_l1;
    wire [1:0]        l2_tbnk0_ulen_l4_q;
    wire              l2_tbnk0_vld_init_l6_q;
    wire              l2_tbnk0_vld_l6_q;
    wire              l2_tbnk0_way_l1;
    wire              l2_tbnk0_way_l4_q;
    wire              l2_tbnk0_way_nxt_l3a;
    wire [143:0]      l2_tbnk0_wr_data_l3;
    wire [127:0]      l2_tbnk0_wr_data_l3a_q;
    wire              l2_tbnk0_wr_data_l4_en;
    wire              l2_tbnk0_wr_err_l1;
    wire              l2_tbnk0_wr_fail_feq_full_l3;
    wire              l2_tbnk0_wr_fail_hazchk_feq_l3;
    wire [11:0]       l2_tbnk0_wr_non_crit_id_l1;
    wire [11:0]       l2_tbnk0_wr_non_crit_id_l4_q;
    wire [15:0]       l2_tbnk0_wr_strb_mask_l3a_q;
    wire              l2_tbnk1_addr44_l3_q;
    wire [44:0]       l2_tbnk1_addr_l1;
    wire [5:2]        l2_tbnk1_addr_l6;
    wire              l2_tbnk1_all_tag_incl_active_l3;
    wire              l2_tbnk1_asq_cmp_evict_l3_q;
    wire              l2_tbnk1_asq_full_flsh;
    wire              l2_tbnk1_asq_nc_so_dev_limit;
    wire [2:0]        l2_tbnk1_cache_attr_l1;
    wire              l2_tbnk1_cfg_ecc_en;
    wire              l2_tbnk1_cmo_setway_l2_inv_incl_l4;
    wire              l2_tbnk1_cpu0_ccb_xfer_l4_dly2;
    wire              l2_tbnk1_cpu0_hit_l4;
    wire              l2_tbnk1_cpu0_l2_inv_l4_dly2;
    wire              l2_tbnk1_cpu0_l2hit_e_l4;
    wire              l2_tbnk1_cpu0_l2hit_s_l4;
    wire              l2_tbnk1_cpu0_peq_full_q;
    wire              l2_tbnk1_cpu0_peq_hit_q;
    wire              l2_tbnk1_cpu0_peq_self_evict_l3_q;
    wire              l2_tbnk1_cpu0_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk1_cpu0_rd_access_l4_dly;
    wire              l2_tbnk1_cpu0_self_evict_l4_dly_q;
    wire              l2_tbnk1_cpu0_single_ecc_err_l7_q;
    wire              l2_tbnk1_cpu0_snp_hit_e_l3;
    wire              l2_tbnk1_cpu0_snp_hit_s_l3;
    wire [44:14]      l2_tbnk1_cpu0_snp_setway_addr_l3;
    wire              l2_tbnk1_cpu0_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk1_cpu0_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk1_cpu0_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk1_cpu0_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk1_cpu0_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk1_cpu0_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk1_cpu0_vld_nxt_l5;
    wire              l2_tbnk1_cpu0_wr_access_l4_dly;
    wire              l2_tbnk1_cpu1_ccb_xfer_l4_dly2;
    wire              l2_tbnk1_cpu1_hit_l4;
    wire              l2_tbnk1_cpu1_l2_inv_l4_dly2;
    wire              l2_tbnk1_cpu1_l2hit_e_l4;
    wire              l2_tbnk1_cpu1_l2hit_s_l4;
    wire              l2_tbnk1_cpu1_peq_full_q;
    wire              l2_tbnk1_cpu1_peq_hit_q;
    wire              l2_tbnk1_cpu1_peq_self_evict_l3_q;
    wire              l2_tbnk1_cpu1_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk1_cpu1_rd_access_l4_dly;
    wire              l2_tbnk1_cpu1_self_evict_l4_dly_q;
    wire              l2_tbnk1_cpu1_single_ecc_err_l7_q;
    wire              l2_tbnk1_cpu1_snp_hit_e_l3;
    wire              l2_tbnk1_cpu1_snp_hit_s_l3;
    wire [44:14]      l2_tbnk1_cpu1_snp_setway_addr_l3;
    wire              l2_tbnk1_cpu1_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk1_cpu1_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk1_cpu1_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk1_cpu1_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk1_cpu1_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk1_cpu1_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk1_cpu1_vld_nxt_l5;
    wire              l2_tbnk1_cpu1_wr_access_l4_dly;
    wire              l2_tbnk1_cpu2_ccb_xfer_l4_dly2;
    wire              l2_tbnk1_cpu2_hit_l4;
    wire              l2_tbnk1_cpu2_l2_inv_l4_dly2;
    wire              l2_tbnk1_cpu2_l2hit_e_l4;
    wire              l2_tbnk1_cpu2_l2hit_s_l4;
    wire              l2_tbnk1_cpu2_peq_full_q;
    wire              l2_tbnk1_cpu2_peq_hit_q;
    wire              l2_tbnk1_cpu2_peq_self_evict_l3_q;
    wire              l2_tbnk1_cpu2_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk1_cpu2_rd_access_l4_dly;
    wire              l2_tbnk1_cpu2_self_evict_l4_dly_q;
    wire              l2_tbnk1_cpu2_single_ecc_err_l7_q;
    wire              l2_tbnk1_cpu2_snp_hit_e_l3;
    wire              l2_tbnk1_cpu2_snp_hit_s_l3;
    wire [44:14]      l2_tbnk1_cpu2_snp_setway_addr_l3;
    wire              l2_tbnk1_cpu2_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk1_cpu2_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk1_cpu2_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk1_cpu2_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk1_cpu2_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk1_cpu2_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk1_cpu2_vld_nxt_l5;
    wire              l2_tbnk1_cpu2_wr_access_l4_dly;
    wire              l2_tbnk1_cpu3_ccb_xfer_l4_dly2;
    wire              l2_tbnk1_cpu3_hit_l4;
    wire              l2_tbnk1_cpu3_l2_inv_l4_dly2;
    wire              l2_tbnk1_cpu3_l2hit_e_l4;
    wire              l2_tbnk1_cpu3_l2hit_s_l4;
    wire              l2_tbnk1_cpu3_peq_full_q;
    wire              l2_tbnk1_cpu3_peq_hit_q;
    wire              l2_tbnk1_cpu3_peq_self_evict_l3_q;
    wire              l2_tbnk1_cpu3_peq_self_evict_wbna_l3_q;
    wire              l2_tbnk1_cpu3_rd_access_l4_dly;
    wire              l2_tbnk1_cpu3_self_evict_l4_dly_q;
    wire              l2_tbnk1_cpu3_single_ecc_err_l7_q;
    wire              l2_tbnk1_cpu3_snp_hit_e_l3;
    wire              l2_tbnk1_cpu3_snp_hit_s_l3;
    wire [44:14]      l2_tbnk1_cpu3_snp_setway_addr_l3;
    wire              l2_tbnk1_cpu3_snp_tag_ecc_err_tp_l4_q;
    wire [1:0]        l2_tbnk1_cpu3_snp_tag_ecc_rmw_rd_l0;
    wire [1:0]        l2_tbnk1_cpu3_snp_tag_ecc_rmw_wr_l0;
    wire              l2_tbnk1_cpu3_snp_tag_wr_evict_early_l4_dly;
    wire              l2_tbnk1_cpu3_snp_tag_wr_evict_qual_l4_dly;
    wire              l2_tbnk1_cpu3_snp_tag_wr_l2_hit_early_l4_dly;
    wire              l2_tbnk1_cpu3_vld_nxt_l5;
    wire              l2_tbnk1_cpu3_wr_access_l4_dly;
    wire [3:0]        l2_tbnk1_cpu_rvalid_init_nxt_l5;
    wire [3:0]        l2_tbnk1_cpu_rvalid_nxt_l5;
    wire [3:0]        l2_tbnk1_cpu_snp_hit_e_l4_q;
    wire              l2_tbnk1_crit_qw_nxt_l5;
    wire [143:0]      l2_tbnk1_data_corrected_l7_q;
    wire [127:0]      l2_tbnk1_data_l6;
    wire              l2_tbnk1_dbg_ram_acc_l5a;
    wire [2:0]        l2_tbnk1_dbg_ram_acc_unit_nxt;
    wire [7:0]        l2_tbnk1_dbg_ram_id_nxt_l5;
    wire              l2_tbnk1_dirty_l1;
    wire              l2_tbnk1_dirty_l3_q;
    wire              l2_tbnk1_dis_ns_dbg_arr_acc_x2;
    wire              l2_tbnk1_double_ecc_err_l7_q;
    wire              l2_tbnk1_early_rvalid_l4_q;
    wire              l2_tbnk1_ecc_fixup_blk_arb;
    wire              l2_tbnk1_ecc_fixup_inprog_dly_q;
    wire              l2_tbnk1_ecc_rmw_snp_tag_rd_l3_q;
    wire [31:0]       l2_tbnk1_ecc_syndrome_reg_q;
    wire              l2_tbnk1_evict_special_hazard_l3_q;
    wire              l2_tbnk1_evict_special_hazard_rwvic_l3_q;
    wire              l2_tbnk1_excl_l1;
    wire              l2_tbnk1_excl_l4_q;
    wire [44:6]       l2_tbnk1_feq_addr_upd;
    wire              l2_tbnk1_feq_alloc_failed_l4;
    wire              l2_tbnk1_feq_axi_wr_vld_not_popped;
    wire              l2_tbnk1_feq_clr_l4;
    wire [15:0]       l2_tbnk1_feq_frc_incl_l3a;
    wire              l2_tbnk1_feq_kill_l3;
    wire [4:0]        l2_tbnk1_feq_last_id_q;
    wire              l2_tbnk1_feq_tbnk_id_update_hit_prfm_or_l3;
    wire              l2_tbnk1_feq_tbnk_id_update_or_l3;
    wire              l2_tbnk1_full_miss_l4_q;
    wire              l2_tbnk1_hit_l4;
    wire              l2_tbnk1_hit_l7_q;
    wire [3:0]        l2_tbnk1_hit_way_l4_q;
    wire [9:0]        l2_tbnk1_id_l1;
    wire [9:0]        l2_tbnk1_id_l6_q;
    wire [9:0]        l2_tbnk1_id_nxt_l5;
    wire              l2_tbnk1_idle;
    wire              l2_tbnk1_init_req_l1;
    wire              l2_tbnk1_kill_l2;
    wire              l2_tbnk1_l2bb_fake_wr_l1;
    wire              l2_tbnk1_l2bb_wr_l1;
    wire              l2_tbnk1_l2hit_e_l4;
    wire              l2_tbnk1_l2hit_s_l4;
    wire              l2_tbnk1_l2v_s_q;
    wire              l2_tbnk1_l2v_vld_q;
    wire              l2_tbnk1_last_qw_l1;
    wire              l2_tbnk1_last_qw_l6_q;
    wire              l2_tbnk1_last_qw_nxt_l5;
    wire [2:0]        l2_tbnk1_lock_l1;
    wire [2:0]        l2_tbnk1_lock_l4;
    wire [32:0]       l2_tbnk1_merrsr_data;
    wire [9:0]        l2_tbnk1_page_attr_l1;
    wire              l2_tbnk1_partial_dw_wr_l1;
    wire              l2_tbnk1_pf_cnt_dec_l4_dly;
    wire              l2_tbnk1_pf_hazard_l3;
    wire              l2_tbnk1_pf_req_sel_for_fwd_l4;
    wire              l2_tbnk1_prfm_l1;
    wire              l2_tbnk1_prfm_nxt_l5;
    wire [3:0]        l2_tbnk1_prot_l1;
    wire [3:0]        l2_tbnk1_prot_l4_q;
    wire [1:0]        l2_tbnk1_qw_cnt_l1;
    wire [1:0]        l2_tbnk1_qw_cnt_l3_q;
    wire              l2_tbnk1_raw_hit_l4_q;
    wire [2:0]        l2_tbnk1_rbufid_nxt_l5;
    wire              l2_tbnk1_rd_en_nxt_l5;
    wire              l2_tbnk1_rd_fail_hazchk_feq_l3;
    wire              l2_tbnk1_rwvic_axi_read_err_l1;
    wire              l2_tbnk1_rwvic_axi_read_err_l3_q;
    wire              l2_tbnk1_rwvic_ccb_dirty_l6_q;
    wire              l2_tbnk1_rwvic_ccb_ls_xfer_l1;
    wire              l2_tbnk1_rwvic_ccb_ls_xfer_l3_q;
    wire              l2_tbnk1_rwvic_ccb_ls_xfer_l6_q;
    wire [3:0]        l2_tbnk1_rwvic_ccb_way_l1;
    wire              l2_tbnk1_rwvic_cmo_clean_l1;
    wire              l2_tbnk1_rwvic_cmo_inv_l1;
    wire              l2_tbnk1_rwvic_cmo_inv_l7_q;
    wire              l2_tbnk1_rwvic_cmo_l7_q;
    wire              l2_tbnk1_rwvic_cmo_pou_l1;
    wire              l2_tbnk1_rwvic_cmo_pou_l6_q;
    wire              l2_tbnk1_rwvic_cmo_setway_l1;
    wire              l2_tbnk1_rwvic_cmo_setway_ls_full_miss_l1;
    wire              l2_tbnk1_rwvic_cmo_setway_ls_l6_q;
    wire [2:0]        l2_tbnk1_rwvic_cpu_fb_id_l1;
    wire [3:0]        l2_tbnk1_rwvic_cpu_id_dcd_l1;
    wire              l2_tbnk1_rwvic_ddi_l6_q;
    wire              l2_tbnk1_rwvic_feq_cmp_l3_q;
    wire              l2_tbnk1_rwvic_frc_l2hit_fwd_l1;
    wire              l2_tbnk1_rwvic_l2hit_e_l1;
    wire              l2_tbnk1_rwvic_l2hit_e_l3_q;
    wire              l2_tbnk1_rwvic_l2hit_e_l7_q;
    wire              l2_tbnk1_rwvic_l2v_dirty_l7_q;
    wire [3:0]        l2_tbnk1_rwvic_l2v_page_attr_l7_q;
    wire              l2_tbnk1_rwvic_l2v_vld_l6_q;
    wire              l2_tbnk1_rwvic_mesi_sh_l1;
    wire              l2_tbnk1_rwvic_non_snp_fail_hazchk_l3;
    wire [2:0]        l2_tbnk1_rwvic_owner_l1;
    wire [2:0]        l2_tbnk1_rwvic_owner_l7_q;
    wire              l2_tbnk1_rwvic_rd_type_l6_q;
    wire              l2_tbnk1_rwvic_snp_clr_dirty_l1;
    wire              l2_tbnk1_rwvic_snp_inv_l1;
    wire              l2_tbnk1_rwvic_snp_l1;
    wire              l2_tbnk1_rwvic_snp_l3_q;
    wire              l2_tbnk1_rwvic_snp_l6_q;
    wire              l2_tbnk1_rwvic_tag_wr_l0;
    wire [3:0]        l2_tbnk1_rwvic_type_l1;
    wire              l2_tbnk1_rwvic_wa_l1;
    wire              l2_tbnk1_rwvic_wa_l6_q;
    wire [13:0]       l2_tbnk1_sel_l1;
    wire [2:0]        l2_tbnk1_size_l1;
    wire [2:0]        l2_tbnk1_size_l4_q;
    wire              l2_tbnk1_snp_byp_peq_haz_pending_q;
    wire              l2_tbnk1_snp_dvm_cmpl_l1;
    wire              l2_tbnk1_snp_hit_e_l4_q;
    wire              l2_tbnk1_snp_hit_feq_evict_l4_dly;
    wire              l2_tbnk1_snp_hit_s_l4_q;
    wire [4:0]        l2_tbnk1_snp_rd_feq_id_cmp_l4_dly_q;
    wire [7:0]        l2_tbnk1_snp_tag_double_ecc_err_l4_dly_q;
    wire [7:0]        l2_tbnk1_snp_tag_single_ecc_err_l4_dly_q;
    wire [44:7]       l2_tbnk1_snp_tag_wr_l2_hit_addr_l1;
    wire [1:0]        l2_tbnk1_snp_tag_wr_l2_hit_state_l1;
    wire              l2_tbnk1_snp_tag_wr_l2_hit_way_l1;
    wire              l2_tbnk1_special_evict_hazard_l3;
    wire              l2_tbnk1_special_hazard_l3_q;
    wire              l2_tbnk1_sync_l1;
    wire              l2_tbnk1_tag_ecc_dbl_rmw_wr_l1;
    wire              l2_tbnk1_tag_ecc_err_cpu0_l4;
    wire              l2_tbnk1_tag_ecc_err_cpu1_l4;
    wire              l2_tbnk1_tag_ecc_err_cpu2_l4;
    wire              l2_tbnk1_tag_ecc_err_cpu3_l4;
    wire              l2_tbnk1_tag_ecc_err_l4;
    wire [6:0]        l2_tbnk1_type_l1;
    wire [1:0]        l2_tbnk1_ulen_l1;
    wire [1:0]        l2_tbnk1_ulen_l4_q;
    wire              l2_tbnk1_vld_init_l6_q;
    wire              l2_tbnk1_vld_l6_q;
    wire              l2_tbnk1_way_l1;
    wire              l2_tbnk1_way_l4_q;
    wire              l2_tbnk1_way_nxt_l3a;
    wire [143:0]      l2_tbnk1_wr_data_l3;
    wire [127:0]      l2_tbnk1_wr_data_l3a_q;
    wire              l2_tbnk1_wr_data_l4_en;
    wire              l2_tbnk1_wr_err_l1;
    wire              l2_tbnk1_wr_fail_feq_full_l3;
    wire              l2_tbnk1_wr_fail_hazchk_feq_l3;
    wire [11:0]       l2_tbnk1_wr_non_crit_id_l1;
    wire [11:0]       l2_tbnk1_wr_non_crit_id_l4_q;
    wire [15:0]       l2_tbnk1_wr_strb_mask_l3a_q;
    wire              l2_tbnk_hwrst_done_x2;
    wire [13:0]       l2_tbnk_hwrst_idx_x1_q;
    wire [8:0]        tm_cntpct_q;
    wire              tm_cpu0_event_sev;
    wire [63:0]       tm_cpu0_spr_rd_data;
    wire              tm_cpu1_event_sev;
    wire [63:0]       tm_cpu1_spr_rd_data;
    wire              tm_cpu2_event_sev;
    wire [63:0]       tm_cpu2_spr_rd_data;
    wire              tm_cpu3_event_sev;
    wire [63:0]       tm_cpu3_spr_rd_data;
    wire [63:0]       tm_tval_cpu0_spr_rd_data;
    wire [63:0]       tm_tval_cpu1_spr_rd_data;
    wire [63:0]       tm_tval_cpu2_spr_rd_data;
    wire [63:0]       tm_tval_cpu3_spr_rd_data;

    maia_timer utm( // outputs
                    .nCNTHPIRQ                (nCNTHPIRQ[`MAIA_CN:0]),
                    .nCNTPNSIRQ               (nCNTPNSIRQ[`MAIA_CN:0]),
                    .nCNTPSIRQ                (nCNTPSIRQ[`MAIA_CN:0]),
                    .nCNTVIRQ                 (nCNTVIRQ[`MAIA_CN:0]),
                    .tm_cntpct_q              (tm_cntpct_q[8:0]),
                    .tm_cpu0_cnthctl_kernel   (tm_cpu0_cnthctl_kernel[1:0]),
                    .tm_cpu0_cntkctl_usr      (tm_cpu0_cntkctl_usr[3:0]),
                    .tm_cpu0_event_sev        (tm_cpu0_event_sev),
                    .tm_cpu0_spr_rd_data      (tm_cpu0_spr_rd_data[63:0]),
                    .tm_cpu1_cnthctl_kernel   (tm_cpu1_cnthctl_kernel[1:0]),
                    .tm_cpu1_cntkctl_usr      (tm_cpu1_cntkctl_usr[3:0]),
                    .tm_cpu1_event_sev        (tm_cpu1_event_sev),
                    .tm_cpu1_spr_rd_data      (tm_cpu1_spr_rd_data[63:0]),
                    .tm_cpu2_cnthctl_kernel   (tm_cpu2_cnthctl_kernel[1:0]),
                    .tm_cpu2_cntkctl_usr      (tm_cpu2_cntkctl_usr[3:0]),
                    .tm_cpu2_event_sev        (tm_cpu2_event_sev),
                    .tm_cpu2_spr_rd_data      (tm_cpu2_spr_rd_data[63:0]),
                    .tm_cpu3_cnthctl_kernel   (tm_cpu3_cnthctl_kernel[1:0]),
                    .tm_cpu3_cntkctl_usr      (tm_cpu3_cntkctl_usr[3:0]),
                    .tm_cpu3_event_sev        (tm_cpu3_event_sev),
                    .tm_cpu3_spr_rd_data      (tm_cpu3_spr_rd_data[63:0]),
                    .tm_tval_cpu0_spr_rd_data (tm_tval_cpu0_spr_rd_data[63:0]),
                    .tm_tval_cpu1_spr_rd_data (tm_tval_cpu1_spr_rd_data[63:0]),
                    .tm_tval_cpu2_spr_rd_data (tm_tval_cpu2_spr_rd_data[63:0]),
                    .tm_tval_cpu3_spr_rd_data (tm_tval_cpu3_spr_rd_data[63:0]),

                    // inputs
                    .CNTCLKEN                 (CNTCLKEN),
                    .CNTVALUEB                (CNTVALUEB[63:0]),
                    .DFTRSTDISABLE            (DFTRSTDISABLE),
                    .DFTSE                    (DFTSE),
                    .ck_areset_l2             (ck_areset_l2),
                    .ck_cpu0_areset_l2cpu     (ck_cpu0_areset_l2cpu),
                    .ck_cpu0_reset1_n_l2cpu   (ck_cpu0_reset1_n_l2cpu),
                    .ck_cpu1_areset_l2cpu     (ck_cpu1_areset_l2cpu),
                    .ck_cpu1_reset1_n_l2cpu   (ck_cpu1_reset1_n_l2cpu),
                    .ck_cpu2_areset_l2cpu     (ck_cpu2_areset_l2cpu),
                    .ck_cpu2_reset1_n_l2cpu   (ck_cpu2_reset1_n_l2cpu),
                    .ck_cpu3_areset_l2cpu     (ck_cpu3_areset_l2cpu),
                    .ck_cpu3_reset1_n_l2cpu   (ck_cpu3_reset1_n_l2cpu),
                    .ck_gclkfr                (ck_gclkfr),
                    .ck_reset1_n_l2           (ck_reset1_n_l2),
                    .ds_cpu0_l2_spr_addr      (ds_cpu0_l2_spr_addr[8:0]),
                    .ds_cpu0_l2_spr_dw        (ds_cpu0_l2_spr_dw),
                    .ds_cpu0_l2_spr_en        (ds_cpu0_l2_spr_en),
                    .ds_cpu0_l2_spr_rd        (ds_cpu0_l2_spr_rd),
                    .ds_cpu0_l2_spr_wr        (ds_cpu0_l2_spr_wr),
                    .ds_cpu0_l2_spr_wr_data   (ds_cpu0_l2_spr_wr_data[63:0]),
                    .ds_cpu1_l2_spr_addr      (ds_cpu1_l2_spr_addr[8:0]),
                    .ds_cpu1_l2_spr_dw        (ds_cpu1_l2_spr_dw),
                    .ds_cpu1_l2_spr_en        (ds_cpu1_l2_spr_en),
                    .ds_cpu1_l2_spr_rd        (ds_cpu1_l2_spr_rd),
                    .ds_cpu1_l2_spr_wr        (ds_cpu1_l2_spr_wr),
                    .ds_cpu1_l2_spr_wr_data   (ds_cpu1_l2_spr_wr_data[63:0]),
                    .ds_cpu2_l2_spr_addr      (ds_cpu2_l2_spr_addr[8:0]),
                    .ds_cpu2_l2_spr_dw        (ds_cpu2_l2_spr_dw),
                    .ds_cpu2_l2_spr_en        (ds_cpu2_l2_spr_en),
                    .ds_cpu2_l2_spr_rd        (ds_cpu2_l2_spr_rd),
                    .ds_cpu2_l2_spr_wr        (ds_cpu2_l2_spr_wr),
                    .ds_cpu2_l2_spr_wr_data   (ds_cpu2_l2_spr_wr_data[63:0]),
                    .ds_cpu3_l2_spr_addr      (ds_cpu3_l2_spr_addr[8:0]),
                    .ds_cpu3_l2_spr_dw        (ds_cpu3_l2_spr_dw),
                    .ds_cpu3_l2_spr_en        (ds_cpu3_l2_spr_en),
                    .ds_cpu3_l2_spr_rd        (ds_cpu3_l2_spr_rd),
                    .ds_cpu3_l2_spr_wr        (ds_cpu3_l2_spr_wr),
                    .ds_cpu3_l2_spr_wr_data   (ds_cpu3_l2_spr_wr_data[63:0]),
                    .eventi_sev               (eventi_sev),
                    .l2_actlr_ncpu_rcg_enable (l2_actlr_ncpu_rcg_enable)
                  );    // utm

    maia_l2_logic_s ul2_logic( // outputs
                               .ARREADYS                                       (ARREADYS),
                               .AWREADYS                                       (AWREADYS),
                               .BIDS                                           (BIDS[4:0]),
                               .BRESPS                                         (BRESPS[1:0]),
                               .BVALIDS                                        (BVALIDS),
                               .L2FLUSHDONE                                    (L2FLUSHDONE),
                               .L2QACCEPTn                                     (L2QACCEPTn),
                               .L2QACTIVE                                      (L2QACTIVE),
                               .L2QDENY                                        (L2QDENY),
                               .RDATAS                                         (RDATAS[127:0]),
                               .REQMEMATTR                                     (REQMEMATTR[7:0]),
                               .RIDS                                           (RIDS[4:0]),
                               .RLASTS                                         (RLASTS),
                               .RRESPS                                         (RRESPS[1:0]),
                               .RVALIDS                                        (RVALIDS),
                               .RXDATLCRDV                                     (RXDATLCRDV),
                               .RXLINKACTIVEACK                                (RXLINKACTIVEACK),
                               .RXRSPLCRDV                                     (RXRSPLCRDV),
                               .RXSNPLCRDV                                     (RXSNPLCRDV),
                               .TXDATFLIT                                      (TXDATFLIT[193:0]),
                               .TXDATFLITPEND                                  (TXDATFLITPEND),
                               .TXDATFLITV                                     (TXDATFLITV),
                               .TXLINKACTIVEREQ                                (TXLINKACTIVEREQ),
                               .TXREQFLIT                                      (TXREQFLIT[99:0]),
                               .TXREQFLITPEND                                  (TXREQFLITPEND),
                               .TXREQFLITV                                     (TXREQFLITV),
                               .TXRSPFLIT                                      (TXRSPFLIT[44:0]),
                               .TXRSPFLITPEND                                  (TXRSPFLITPEND),
                               .TXRSPFLITV                                     (TXRSPFLITV),
                               .TXSACTIVE                                      (TXSACTIVE),
                               .WREADYS                                        (WREADYS),
                               .ck_areset_l2                                   (ck_areset_l2),
                               .ck_l2_logic_clk_en                             (ck_l2_logic_clk_en),
                               .ck_l2_tbnk0_clk_en                             (ck_l2_tbnk0_clk_en),
                               .ck_l2_tbnk1_clk_en                             (ck_l2_tbnk1_clk_en),
                               .ck_reset1_n_l2                                 (ck_reset1_n_l2),
                               .l2_acp_flsh_rd_cnt_active_glb_l2_dly           (l2_acp_flsh_rd_cnt_active_glb_l2_dly),
                               .l2_acp_flsh_wr_cnt_active_glb_l2_dly           (l2_acp_flsh_wr_cnt_active_glb_l2_dly),
                               .l2_acp_rd_haz_vld_l2_dly_q                     (l2_acp_rd_haz_vld_l2_dly_q),
                               .l2_acp_wr_haz_vld_l2_dly_q                     (l2_acp_wr_haz_vld_l2_dly_q),
                               .l2_actlr_disable_b2b_setway_hzd_opt_x2_ns      (l2_actlr_disable_b2b_setway_hzd_opt_x2_ns),
                               .l2_actlr_disable_setway_opt                    (l2_actlr_disable_setway_opt),
                               .l2_actlr_ncpu_rcg_enable                       (l2_actlr_ncpu_rcg_enable),
                               .l2_actlr_plru_dynamic                          (l2_actlr_plru_dynamic),
                               .l2_actlr_plru_en                               (l2_actlr_plru_en),
                               .l2_actlr_plru_mode                             (l2_actlr_plru_mode[1:0]),
                               .l2_actlr_writeunique_disable                   (l2_actlr_writeunique_disable),
                               .l2_cfg_broadcastinner                          (l2_cfg_broadcastinner),
                               .l2_cfg_broadcastouter                          (l2_cfg_broadcastouter),
                               .l2_cpu0_arb_thrshld_timeout_en                 (l2_cpu0_arb_thrshld_timeout_en),
                               .l2_cpu0_barrier_done                           (l2_cpu0_barrier_done),
                               .l2_cpu0_blk_non_evict_wr                       (l2_cpu0_blk_non_evict_wr),
                               .l2_cpu0_ccb_dbg_req_c3                         (l2_cpu0_ccb_dbg_req_c3),
                               .l2_cpu0_ccb_req_addr_c3                        (l2_cpu0_ccb_req_addr_c3[48:0]),
                               .l2_cpu0_ccb_req_id_c3                          (l2_cpu0_ccb_req_id_c3[4:0]),
                               .l2_cpu0_ccb_req_info_c3                        (l2_cpu0_ccb_req_info_c3[23:0]),
                               .l2_cpu0_ccb_req_type_c3                        (l2_cpu0_ccb_req_type_c3[8:0]),
                               .l2_cpu0_cfg_ecc_en                             (l2_cpu0_cfg_ecc_en),
                               .l2_cpu0_dbufid_r1                              (l2_cpu0_dbufid_r1[2:0]),
                               .l2_cpu0_ddata_r2                               (l2_cpu0_ddata_r2[129:0]),
                               .l2_cpu0_ddbl_ecc_err_r3                        (l2_cpu0_ddbl_ecc_err_r3),
                               .l2_cpu0_dext_err_r2                            (l2_cpu0_dext_err_r2),
                               .l2_cpu0_dext_err_type_r2                       (l2_cpu0_dext_err_type_r2),
                               .l2_cpu0_disable_clean_evict_opt                (l2_cpu0_disable_clean_evict_opt),
                               .l2_cpu0_dlast_r1                               (l2_cpu0_dlast_r1),
                               .l2_cpu0_dsngl_ecc_err_r3                       (l2_cpu0_dsngl_ecc_err_r3),
                               .l2_cpu0_dsq_clr_id_q                           (l2_cpu0_dsq_clr_id_q[3:0]),
                               .l2_cpu0_dsq_clr_vld_q                          (l2_cpu0_dsq_clr_vld_q),
                               .l2_cpu0_dsq_rd_buf_id                          (l2_cpu0_dsq_rd_buf_id[3:0]),
                               .l2_cpu0_dsq_rd_en                              (l2_cpu0_dsq_rd_en),
                               .l2_cpu0_dsq_rd_en_x2                           (l2_cpu0_dsq_rd_en_x2),
                               .l2_cpu0_dvalid_r1                              (l2_cpu0_dvalid_r1),
                               .l2_cpu0_flsh_if_rd_id_l4_dly                   (l2_cpu0_flsh_if_rd_id_l4_dly[1:0]),
                               .l2_cpu0_flsh_if_rd_l4_dly                      (l2_cpu0_flsh_if_rd_l4_dly),
                               .l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly       (l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu0_flsh_ls_rd_id_l2_dly                   (l2_cpu0_flsh_ls_rd_id_l2_dly[2:0]),
                               .l2_cpu0_flsh_ls_rd_id_l4_dly                   (l2_cpu0_flsh_ls_rd_id_l4_dly[2:0]),
                               .l2_cpu0_flsh_ls_rd_l2_dly                      (l2_cpu0_flsh_ls_rd_l2_dly),
                               .l2_cpu0_flsh_ls_rd_l4_dly                      (l2_cpu0_flsh_ls_rd_l4_dly),
                               .l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly       (l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu0_flsh_ls_wr_evict_l4_dly                (l2_cpu0_flsh_ls_wr_evict_l4_dly),
                               .l2_cpu0_flsh_ls_wr_id_l2_dly                   (l2_cpu0_flsh_ls_wr_id_l2_dly[3:0]),
                               .l2_cpu0_flsh_ls_wr_id_l4_dly                   (l2_cpu0_flsh_ls_wr_id_l4_dly[3:0]),
                               .l2_cpu0_flsh_ls_wr_l2_dly                      (l2_cpu0_flsh_ls_wr_l2_dly),
                               .l2_cpu0_flsh_ls_wr_l4_dly                      (l2_cpu0_flsh_ls_wr_l4_dly),
                               .l2_cpu0_flsh_tw_rd_l4_dly                      (l2_cpu0_flsh_tw_rd_l4_dly),
                               .l2_cpu0_ibufid_r1                              (l2_cpu0_ibufid_r1[1:0]),
                               .l2_cpu0_ic_barrier_stall_q                     (l2_cpu0_ic_barrier_stall_q),
                               .l2_cpu0_ic_base                                (l2_cpu0_ic_base[43:18]),
                               .l2_cpu0_ic_vld_skid                            (l2_cpu0_ic_vld_skid),
                               .l2_cpu0_idata_r2                               (l2_cpu0_idata_r2[127:0]),
                               .l2_cpu0_idbl_ecc_err_r3                        (l2_cpu0_idbl_ecc_err_r3),
                               .l2_cpu0_idle_block_reqs_q                      (l2_cpu0_idle_block_reqs_q),
                               .l2_cpu0_iext_err_r2                            (l2_cpu0_iext_err_r2),
                               .l2_cpu0_iext_err_type_r2                       (l2_cpu0_iext_err_type_r2),
                               .l2_cpu0_if_ccb_clken_c3                        (l2_cpu0_if_ccb_clken_c3),
                               .l2_cpu0_if_ccb_req_c3                          (l2_cpu0_if_ccb_req_c3),
                               .l2_cpu0_if_sync_req                            (l2_cpu0_if_sync_req),
                               .l2_cpu0_ifq_haz_pending                        (l2_cpu0_ifq_haz_pending),
                               .l2_cpu0_isngl_ecc_err_r3                       (l2_cpu0_isngl_ecc_err_r3),
                               .l2_cpu0_ivalid_r1                              (l2_cpu0_ivalid_r1),
                               .l2_cpu0_l2_cache_size                          (l2_cpu0_l2_cache_size[1:0]),
                               .l2_cpu0_lrq_haz_pending                        (l2_cpu0_lrq_haz_pending),
                               .l2_cpu0_ls_ccb_clken_c3                        (l2_cpu0_ls_ccb_clken_c3),
                               .l2_cpu0_ls_ccb_req_c3                          (l2_cpu0_ls_ccb_req_c3),
                               .l2_cpu0_ls_peq_coll_l4_dly                     (l2_cpu0_ls_peq_coll_l4_dly),
                               .l2_cpu0_ls_rd_haz_vld_l2_dly_q                 (l2_cpu0_ls_rd_haz_vld_l2_dly_q),
                               .l2_cpu0_ls_sync_req                            (l2_cpu0_ls_sync_req),
                               .l2_cpu0_ls_wr_haz_vld_l2_dly_q                 (l2_cpu0_ls_wr_haz_vld_l2_dly_q),
                               .l2_cpu0_mbist1_addr_b1                         (l2_cpu0_mbist1_addr_b1[12:0]),
                               .l2_cpu0_mbist1_all_b1                          (l2_cpu0_mbist1_all_b1),
                               .l2_cpu0_mbist1_array_b1                        (l2_cpu0_mbist1_array_b1[3:0]),
                               .l2_cpu0_mbist1_be_b1                           (l2_cpu0_mbist1_be_b1[7:0]),
                               .l2_cpu0_mbist1_en_b1                           (l2_cpu0_mbist1_en_b1),
                               .l2_cpu0_mbist1_rd_en_b1                        (l2_cpu0_mbist1_rd_en_b1),
                               .l2_cpu0_mbist1_wr_en_b1                        (l2_cpu0_mbist1_wr_en_b1),
                               .l2_cpu0_no_intctrl                             (l2_cpu0_no_intctrl),
                               .l2_cpu0_pf_rd_vld_skid_popped                  (l2_cpu0_pf_rd_vld_skid_popped),
                               .l2_cpu0_pf_throttle_q                          (l2_cpu0_pf_throttle_q),
                               .l2_cpu0_pmu_events                             (l2_cpu0_pmu_events[33:0]),
                               .l2_cpu0_rbufid                                 (l2_cpu0_rbufid[2:0]),
                               .l2_cpu0_rd_arb                                 (l2_cpu0_rd_arb),
                               .l2_cpu0_rd_vld_skid                            (l2_cpu0_rd_vld_skid),
                               .l2_cpu0_rexfail                                (l2_cpu0_rexfail),
                               .l2_cpu0_rstate                                 (l2_cpu0_rstate[1:0]),
                               .l2_cpu0_rvalid                                 (l2_cpu0_rvalid),
                               .l2_cpu0_snp_active                             (l2_cpu0_snp_active),
                               .l2_cpu0_spec_bufid                             (l2_cpu0_spec_bufid[2:0]),
                               .l2_cpu0_spec_valid                             (l2_cpu0_spec_valid),
                               .l2_cpu0_spr_rd_data                            (l2_cpu0_spr_rd_data[63:0]),
                               .l2_cpu0_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu0_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu0_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu0_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu0_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu0_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu0_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu0_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu0_tbw_dbl_ecc_err                        (l2_cpu0_tbw_dbl_ecc_err),
                               .l2_cpu0_tbw_desc_data                          (l2_cpu0_tbw_desc_data[63:0]),
                               .l2_cpu0_tbw_desc_vld                           (l2_cpu0_tbw_desc_vld),
                               .l2_cpu0_tbw_ext_err                            (l2_cpu0_tbw_ext_err),
                               .l2_cpu0_tbw_ext_err_type                       (l2_cpu0_tbw_ext_err_type),
                               .l2_cpu0_tlb_ccb_clken_c3                       (l2_cpu0_tlb_ccb_clken_c3),
                               .l2_cpu0_tlb_ccb_req_c3                         (l2_cpu0_tlb_ccb_req_c3),
                               .l2_cpu0_tlb_sync_complete                      (l2_cpu0_tlb_sync_complete),
                               .l2_cpu0_tlb_sync_req                           (l2_cpu0_tlb_sync_req),
                               .l2_cpu0_trq_haz_pending                        (l2_cpu0_trq_haz_pending),
                               .l2_cpu0_wr_arb                                 (l2_cpu0_wr_arb),
                               .l2_cpu0_wr_data_stall                          (l2_cpu0_wr_data_stall),
                               .l2_cpu0_wr_decerr_q                            (l2_cpu0_wr_decerr_q),
                               .l2_cpu0_wr_ex_fail                             (l2_cpu0_wr_ex_fail),
                               .l2_cpu0_wr_ex_resp                             (l2_cpu0_wr_ex_resp),
                               .l2_cpu0_wr_slverr_q                            (l2_cpu0_wr_slverr_q),
                               .l2_cpu0_wr_vld_skid                            (l2_cpu0_wr_vld_skid),
                               .l2_cpu0_wrq_haz_pending                        (l2_cpu0_wrq_haz_pending),
                               .l2_cpu1_arb_thrshld_timeout_en                 (l2_cpu1_arb_thrshld_timeout_en),
                               .l2_cpu1_barrier_done                           (l2_cpu1_barrier_done),
                               .l2_cpu1_blk_non_evict_wr                       (l2_cpu1_blk_non_evict_wr),
                               .l2_cpu1_ccb_dbg_req_c3                         (l2_cpu1_ccb_dbg_req_c3),
                               .l2_cpu1_ccb_req_addr_c3                        (l2_cpu1_ccb_req_addr_c3[48:0]),
                               .l2_cpu1_ccb_req_id_c3                          (l2_cpu1_ccb_req_id_c3[4:0]),
                               .l2_cpu1_ccb_req_info_c3                        (l2_cpu1_ccb_req_info_c3[23:0]),
                               .l2_cpu1_ccb_req_type_c3                        (l2_cpu1_ccb_req_type_c3[8:0]),
                               .l2_cpu1_cfg_ecc_en                             (l2_cpu1_cfg_ecc_en),
                               .l2_cpu1_dbufid_r1                              (l2_cpu1_dbufid_r1[2:0]),
                               .l2_cpu1_ddata_r2                               (l2_cpu1_ddata_r2[129:0]),
                               .l2_cpu1_ddbl_ecc_err_r3                        (l2_cpu1_ddbl_ecc_err_r3),
                               .l2_cpu1_dext_err_r2                            (l2_cpu1_dext_err_r2),
                               .l2_cpu1_dext_err_type_r2                       (l2_cpu1_dext_err_type_r2),
                               .l2_cpu1_disable_clean_evict_opt                (l2_cpu1_disable_clean_evict_opt),
                               .l2_cpu1_dlast_r1                               (l2_cpu1_dlast_r1),
                               .l2_cpu1_dsngl_ecc_err_r3                       (l2_cpu1_dsngl_ecc_err_r3),
                               .l2_cpu1_dsq_clr_id_q                           (l2_cpu1_dsq_clr_id_q[3:0]),
                               .l2_cpu1_dsq_clr_vld_q                          (l2_cpu1_dsq_clr_vld_q),
                               .l2_cpu1_dsq_rd_buf_id                          (l2_cpu1_dsq_rd_buf_id[3:0]),
                               .l2_cpu1_dsq_rd_en                              (l2_cpu1_dsq_rd_en),
                               .l2_cpu1_dsq_rd_en_x2                           (l2_cpu1_dsq_rd_en_x2),
                               .l2_cpu1_dvalid_r1                              (l2_cpu1_dvalid_r1),
                               .l2_cpu1_flsh_if_rd_id_l4_dly                   (l2_cpu1_flsh_if_rd_id_l4_dly[1:0]),
                               .l2_cpu1_flsh_if_rd_l4_dly                      (l2_cpu1_flsh_if_rd_l4_dly),
                               .l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly       (l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu1_flsh_ls_rd_id_l2_dly                   (l2_cpu1_flsh_ls_rd_id_l2_dly[2:0]),
                               .l2_cpu1_flsh_ls_rd_id_l4_dly                   (l2_cpu1_flsh_ls_rd_id_l4_dly[2:0]),
                               .l2_cpu1_flsh_ls_rd_l2_dly                      (l2_cpu1_flsh_ls_rd_l2_dly),
                               .l2_cpu1_flsh_ls_rd_l4_dly                      (l2_cpu1_flsh_ls_rd_l4_dly),
                               .l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly       (l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu1_flsh_ls_wr_evict_l4_dly                (l2_cpu1_flsh_ls_wr_evict_l4_dly),
                               .l2_cpu1_flsh_ls_wr_id_l2_dly                   (l2_cpu1_flsh_ls_wr_id_l2_dly[3:0]),
                               .l2_cpu1_flsh_ls_wr_id_l4_dly                   (l2_cpu1_flsh_ls_wr_id_l4_dly[3:0]),
                               .l2_cpu1_flsh_ls_wr_l2_dly                      (l2_cpu1_flsh_ls_wr_l2_dly),
                               .l2_cpu1_flsh_ls_wr_l4_dly                      (l2_cpu1_flsh_ls_wr_l4_dly),
                               .l2_cpu1_flsh_tw_rd_l4_dly                      (l2_cpu1_flsh_tw_rd_l4_dly),
                               .l2_cpu1_ibufid_r1                              (l2_cpu1_ibufid_r1[1:0]),
                               .l2_cpu1_ic_barrier_stall_q                     (l2_cpu1_ic_barrier_stall_q),
                               .l2_cpu1_ic_base                                (l2_cpu1_ic_base[43:18]),
                               .l2_cpu1_ic_vld_skid                            (l2_cpu1_ic_vld_skid),
                               .l2_cpu1_idata_r2                               (l2_cpu1_idata_r2[127:0]),
                               .l2_cpu1_idbl_ecc_err_r3                        (l2_cpu1_idbl_ecc_err_r3),
                               .l2_cpu1_idle_block_reqs_q                      (l2_cpu1_idle_block_reqs_q),
                               .l2_cpu1_iext_err_r2                            (l2_cpu1_iext_err_r2),
                               .l2_cpu1_iext_err_type_r2                       (l2_cpu1_iext_err_type_r2),
                               .l2_cpu1_if_ccb_clken_c3                        (l2_cpu1_if_ccb_clken_c3),
                               .l2_cpu1_if_ccb_req_c3                          (l2_cpu1_if_ccb_req_c3),
                               .l2_cpu1_if_sync_req                            (l2_cpu1_if_sync_req),
                               .l2_cpu1_ifq_haz_pending                        (l2_cpu1_ifq_haz_pending),
                               .l2_cpu1_isngl_ecc_err_r3                       (l2_cpu1_isngl_ecc_err_r3),
                               .l2_cpu1_ivalid_r1                              (l2_cpu1_ivalid_r1),
                               .l2_cpu1_l2_cache_size                          (l2_cpu1_l2_cache_size[1:0]),
                               .l2_cpu1_lrq_haz_pending                        (l2_cpu1_lrq_haz_pending),
                               .l2_cpu1_ls_ccb_clken_c3                        (l2_cpu1_ls_ccb_clken_c3),
                               .l2_cpu1_ls_ccb_req_c3                          (l2_cpu1_ls_ccb_req_c3),
                               .l2_cpu1_ls_peq_coll_l4_dly                     (l2_cpu1_ls_peq_coll_l4_dly),
                               .l2_cpu1_ls_rd_haz_vld_l2_dly_q                 (l2_cpu1_ls_rd_haz_vld_l2_dly_q),
                               .l2_cpu1_ls_sync_req                            (l2_cpu1_ls_sync_req),
                               .l2_cpu1_ls_wr_haz_vld_l2_dly_q                 (l2_cpu1_ls_wr_haz_vld_l2_dly_q),
                               .l2_cpu1_mbist1_addr_b1                         (l2_cpu1_mbist1_addr_b1[12:0]),
                               .l2_cpu1_mbist1_all_b1                          (l2_cpu1_mbist1_all_b1),
                               .l2_cpu1_mbist1_array_b1                        (l2_cpu1_mbist1_array_b1[3:0]),
                               .l2_cpu1_mbist1_be_b1                           (l2_cpu1_mbist1_be_b1[7:0]),
                               .l2_cpu1_mbist1_en_b1                           (l2_cpu1_mbist1_en_b1),
                               .l2_cpu1_mbist1_rd_en_b1                        (l2_cpu1_mbist1_rd_en_b1),
                               .l2_cpu1_mbist1_wr_en_b1                        (l2_cpu1_mbist1_wr_en_b1),
                               .l2_cpu1_no_intctrl                             (l2_cpu1_no_intctrl),
                               .l2_cpu1_pf_rd_vld_skid_popped                  (l2_cpu1_pf_rd_vld_skid_popped),
                               .l2_cpu1_pf_throttle_q                          (l2_cpu1_pf_throttle_q),
                               .l2_cpu1_pmu_events                             (l2_cpu1_pmu_events[33:0]),
                               .l2_cpu1_rbufid                                 (l2_cpu1_rbufid[2:0]),
                               .l2_cpu1_rd_arb                                 (l2_cpu1_rd_arb),
                               .l2_cpu1_rd_vld_skid                            (l2_cpu1_rd_vld_skid),
                               .l2_cpu1_rexfail                                (l2_cpu1_rexfail),
                               .l2_cpu1_rstate                                 (l2_cpu1_rstate[1:0]),
                               .l2_cpu1_rvalid                                 (l2_cpu1_rvalid),
                               .l2_cpu1_snp_active                             (l2_cpu1_snp_active),
                               .l2_cpu1_spec_bufid                             (l2_cpu1_spec_bufid[2:0]),
                               .l2_cpu1_spec_valid                             (l2_cpu1_spec_valid),
                               .l2_cpu1_spr_rd_data                            (l2_cpu1_spr_rd_data[63:0]),
                               .l2_cpu1_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu1_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu1_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu1_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu1_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu1_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu1_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu1_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu1_tbw_dbl_ecc_err                        (l2_cpu1_tbw_dbl_ecc_err),
                               .l2_cpu1_tbw_desc_data                          (l2_cpu1_tbw_desc_data[63:0]),
                               .l2_cpu1_tbw_desc_vld                           (l2_cpu1_tbw_desc_vld),
                               .l2_cpu1_tbw_ext_err                            (l2_cpu1_tbw_ext_err),
                               .l2_cpu1_tbw_ext_err_type                       (l2_cpu1_tbw_ext_err_type),
                               .l2_cpu1_tlb_ccb_clken_c3                       (l2_cpu1_tlb_ccb_clken_c3),
                               .l2_cpu1_tlb_ccb_req_c3                         (l2_cpu1_tlb_ccb_req_c3),
                               .l2_cpu1_tlb_sync_complete                      (l2_cpu1_tlb_sync_complete),
                               .l2_cpu1_tlb_sync_req                           (l2_cpu1_tlb_sync_req),
                               .l2_cpu1_trq_haz_pending                        (l2_cpu1_trq_haz_pending),
                               .l2_cpu1_wr_arb                                 (l2_cpu1_wr_arb),
                               .l2_cpu1_wr_data_stall                          (l2_cpu1_wr_data_stall),
                               .l2_cpu1_wr_decerr_q                            (l2_cpu1_wr_decerr_q),
                               .l2_cpu1_wr_ex_fail                             (l2_cpu1_wr_ex_fail),
                               .l2_cpu1_wr_ex_resp                             (l2_cpu1_wr_ex_resp),
                               .l2_cpu1_wr_slverr_q                            (l2_cpu1_wr_slverr_q),
                               .l2_cpu1_wr_vld_skid                            (l2_cpu1_wr_vld_skid),
                               .l2_cpu1_wrq_haz_pending                        (l2_cpu1_wrq_haz_pending),
                               .l2_cpu2_arb_thrshld_timeout_en                 (l2_cpu2_arb_thrshld_timeout_en),
                               .l2_cpu2_barrier_done                           (l2_cpu2_barrier_done),
                               .l2_cpu2_blk_non_evict_wr                       (l2_cpu2_blk_non_evict_wr),
                               .l2_cpu2_ccb_dbg_req_c3                         (l2_cpu2_ccb_dbg_req_c3),
                               .l2_cpu2_ccb_req_addr_c3                        (l2_cpu2_ccb_req_addr_c3[48:0]),
                               .l2_cpu2_ccb_req_id_c3                          (l2_cpu2_ccb_req_id_c3[4:0]),
                               .l2_cpu2_ccb_req_info_c3                        (l2_cpu2_ccb_req_info_c3[23:0]),
                               .l2_cpu2_ccb_req_type_c3                        (l2_cpu2_ccb_req_type_c3[8:0]),
                               .l2_cpu2_cfg_ecc_en                             (l2_cpu2_cfg_ecc_en),
                               .l2_cpu2_dbufid_r1                              (l2_cpu2_dbufid_r1[2:0]),
                               .l2_cpu2_ddata_r2                               (l2_cpu2_ddata_r2[129:0]),
                               .l2_cpu2_ddbl_ecc_err_r3                        (l2_cpu2_ddbl_ecc_err_r3),
                               .l2_cpu2_dext_err_r2                            (l2_cpu2_dext_err_r2),
                               .l2_cpu2_dext_err_type_r2                       (l2_cpu2_dext_err_type_r2),
                               .l2_cpu2_disable_clean_evict_opt                (l2_cpu2_disable_clean_evict_opt),
                               .l2_cpu2_dlast_r1                               (l2_cpu2_dlast_r1),
                               .l2_cpu2_dsngl_ecc_err_r3                       (l2_cpu2_dsngl_ecc_err_r3),
                               .l2_cpu2_dsq_clr_id_q                           (l2_cpu2_dsq_clr_id_q[3:0]),
                               .l2_cpu2_dsq_clr_vld_q                          (l2_cpu2_dsq_clr_vld_q),
                               .l2_cpu2_dsq_rd_buf_id                          (l2_cpu2_dsq_rd_buf_id[3:0]),
                               .l2_cpu2_dsq_rd_en                              (l2_cpu2_dsq_rd_en),
                               .l2_cpu2_dsq_rd_en_x2                           (l2_cpu2_dsq_rd_en_x2),
                               .l2_cpu2_dvalid_r1                              (l2_cpu2_dvalid_r1),
                               .l2_cpu2_flsh_if_rd_id_l4_dly                   (l2_cpu2_flsh_if_rd_id_l4_dly[1:0]),
                               .l2_cpu2_flsh_if_rd_l4_dly                      (l2_cpu2_flsh_if_rd_l4_dly),
                               .l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly       (l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu2_flsh_ls_rd_id_l2_dly                   (l2_cpu2_flsh_ls_rd_id_l2_dly[2:0]),
                               .l2_cpu2_flsh_ls_rd_id_l4_dly                   (l2_cpu2_flsh_ls_rd_id_l4_dly[2:0]),
                               .l2_cpu2_flsh_ls_rd_l2_dly                      (l2_cpu2_flsh_ls_rd_l2_dly),
                               .l2_cpu2_flsh_ls_rd_l4_dly                      (l2_cpu2_flsh_ls_rd_l4_dly),
                               .l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly       (l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu2_flsh_ls_wr_evict_l4_dly                (l2_cpu2_flsh_ls_wr_evict_l4_dly),
                               .l2_cpu2_flsh_ls_wr_id_l2_dly                   (l2_cpu2_flsh_ls_wr_id_l2_dly[3:0]),
                               .l2_cpu2_flsh_ls_wr_id_l4_dly                   (l2_cpu2_flsh_ls_wr_id_l4_dly[3:0]),
                               .l2_cpu2_flsh_ls_wr_l2_dly                      (l2_cpu2_flsh_ls_wr_l2_dly),
                               .l2_cpu2_flsh_ls_wr_l4_dly                      (l2_cpu2_flsh_ls_wr_l4_dly),
                               .l2_cpu2_flsh_tw_rd_l4_dly                      (l2_cpu2_flsh_tw_rd_l4_dly),
                               .l2_cpu2_ibufid_r1                              (l2_cpu2_ibufid_r1[1:0]),
                               .l2_cpu2_ic_barrier_stall_q                     (l2_cpu2_ic_barrier_stall_q),
                               .l2_cpu2_ic_base                                (l2_cpu2_ic_base[43:18]),
                               .l2_cpu2_ic_vld_skid                            (l2_cpu2_ic_vld_skid),
                               .l2_cpu2_idata_r2                               (l2_cpu2_idata_r2[127:0]),
                               .l2_cpu2_idbl_ecc_err_r3                        (l2_cpu2_idbl_ecc_err_r3),
                               .l2_cpu2_idle_block_reqs_q                      (l2_cpu2_idle_block_reqs_q),
                               .l2_cpu2_iext_err_r2                            (l2_cpu2_iext_err_r2),
                               .l2_cpu2_iext_err_type_r2                       (l2_cpu2_iext_err_type_r2),
                               .l2_cpu2_if_ccb_clken_c3                        (l2_cpu2_if_ccb_clken_c3),
                               .l2_cpu2_if_ccb_req_c3                          (l2_cpu2_if_ccb_req_c3),
                               .l2_cpu2_if_sync_req                            (l2_cpu2_if_sync_req),
                               .l2_cpu2_ifq_haz_pending                        (l2_cpu2_ifq_haz_pending),
                               .l2_cpu2_isngl_ecc_err_r3                       (l2_cpu2_isngl_ecc_err_r3),
                               .l2_cpu2_ivalid_r1                              (l2_cpu2_ivalid_r1),
                               .l2_cpu2_l2_cache_size                          (l2_cpu2_l2_cache_size[1:0]),
                               .l2_cpu2_lrq_haz_pending                        (l2_cpu2_lrq_haz_pending),
                               .l2_cpu2_ls_ccb_clken_c3                        (l2_cpu2_ls_ccb_clken_c3),
                               .l2_cpu2_ls_ccb_req_c3                          (l2_cpu2_ls_ccb_req_c3),
                               .l2_cpu2_ls_peq_coll_l4_dly                     (l2_cpu2_ls_peq_coll_l4_dly),
                               .l2_cpu2_ls_rd_haz_vld_l2_dly_q                 (l2_cpu2_ls_rd_haz_vld_l2_dly_q),
                               .l2_cpu2_ls_sync_req                            (l2_cpu2_ls_sync_req),
                               .l2_cpu2_ls_wr_haz_vld_l2_dly_q                 (l2_cpu2_ls_wr_haz_vld_l2_dly_q),
                               .l2_cpu2_mbist1_addr_b1                         (l2_cpu2_mbist1_addr_b1[12:0]),
                               .l2_cpu2_mbist1_all_b1                          (l2_cpu2_mbist1_all_b1),
                               .l2_cpu2_mbist1_array_b1                        (l2_cpu2_mbist1_array_b1[3:0]),
                               .l2_cpu2_mbist1_be_b1                           (l2_cpu2_mbist1_be_b1[7:0]),
                               .l2_cpu2_mbist1_en_b1                           (l2_cpu2_mbist1_en_b1),
                               .l2_cpu2_mbist1_rd_en_b1                        (l2_cpu2_mbist1_rd_en_b1),
                               .l2_cpu2_mbist1_wr_en_b1                        (l2_cpu2_mbist1_wr_en_b1),
                               .l2_cpu2_no_intctrl                             (l2_cpu2_no_intctrl),
                               .l2_cpu2_pf_rd_vld_skid_popped                  (l2_cpu2_pf_rd_vld_skid_popped),
                               .l2_cpu2_pf_throttle_q                          (l2_cpu2_pf_throttle_q),
                               .l2_cpu2_pmu_events                             (l2_cpu2_pmu_events[33:0]),
                               .l2_cpu2_rbufid                                 (l2_cpu2_rbufid[2:0]),
                               .l2_cpu2_rd_arb                                 (l2_cpu2_rd_arb),
                               .l2_cpu2_rd_vld_skid                            (l2_cpu2_rd_vld_skid),
                               .l2_cpu2_rexfail                                (l2_cpu2_rexfail),
                               .l2_cpu2_rstate                                 (l2_cpu2_rstate[1:0]),
                               .l2_cpu2_rvalid                                 (l2_cpu2_rvalid),
                               .l2_cpu2_snp_active                             (l2_cpu2_snp_active),
                               .l2_cpu2_spec_bufid                             (l2_cpu2_spec_bufid[2:0]),
                               .l2_cpu2_spec_valid                             (l2_cpu2_spec_valid),
                               .l2_cpu2_spr_rd_data                            (l2_cpu2_spr_rd_data[63:0]),
                               .l2_cpu2_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu2_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu2_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu2_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu2_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu2_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu2_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu2_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu2_tbw_dbl_ecc_err                        (l2_cpu2_tbw_dbl_ecc_err),
                               .l2_cpu2_tbw_desc_data                          (l2_cpu2_tbw_desc_data[63:0]),
                               .l2_cpu2_tbw_desc_vld                           (l2_cpu2_tbw_desc_vld),
                               .l2_cpu2_tbw_ext_err                            (l2_cpu2_tbw_ext_err),
                               .l2_cpu2_tbw_ext_err_type                       (l2_cpu2_tbw_ext_err_type),
                               .l2_cpu2_tlb_ccb_clken_c3                       (l2_cpu2_tlb_ccb_clken_c3),
                               .l2_cpu2_tlb_ccb_req_c3                         (l2_cpu2_tlb_ccb_req_c3),
                               .l2_cpu2_tlb_sync_complete                      (l2_cpu2_tlb_sync_complete),
                               .l2_cpu2_tlb_sync_req                           (l2_cpu2_tlb_sync_req),
                               .l2_cpu2_trq_haz_pending                        (l2_cpu2_trq_haz_pending),
                               .l2_cpu2_wr_arb                                 (l2_cpu2_wr_arb),
                               .l2_cpu2_wr_data_stall                          (l2_cpu2_wr_data_stall),
                               .l2_cpu2_wr_decerr_q                            (l2_cpu2_wr_decerr_q),
                               .l2_cpu2_wr_ex_fail                             (l2_cpu2_wr_ex_fail),
                               .l2_cpu2_wr_ex_resp                             (l2_cpu2_wr_ex_resp),
                               .l2_cpu2_wr_slverr_q                            (l2_cpu2_wr_slverr_q),
                               .l2_cpu2_wr_vld_skid                            (l2_cpu2_wr_vld_skid),
                               .l2_cpu2_wrq_haz_pending                        (l2_cpu2_wrq_haz_pending),
                               .l2_cpu3_arb_thrshld_timeout_en                 (l2_cpu3_arb_thrshld_timeout_en),
                               .l2_cpu3_barrier_done                           (l2_cpu3_barrier_done),
                               .l2_cpu3_blk_non_evict_wr                       (l2_cpu3_blk_non_evict_wr),
                               .l2_cpu3_ccb_dbg_req_c3                         (l2_cpu3_ccb_dbg_req_c3),
                               .l2_cpu3_ccb_req_addr_c3                        (l2_cpu3_ccb_req_addr_c3[48:0]),
                               .l2_cpu3_ccb_req_id_c3                          (l2_cpu3_ccb_req_id_c3[4:0]),
                               .l2_cpu3_ccb_req_info_c3                        (l2_cpu3_ccb_req_info_c3[23:0]),
                               .l2_cpu3_ccb_req_type_c3                        (l2_cpu3_ccb_req_type_c3[8:0]),
                               .l2_cpu3_cfg_ecc_en                             (l2_cpu3_cfg_ecc_en),
                               .l2_cpu3_dbufid_r1                              (l2_cpu3_dbufid_r1[2:0]),
                               .l2_cpu3_ddata_r2                               (l2_cpu3_ddata_r2[129:0]),
                               .l2_cpu3_ddbl_ecc_err_r3                        (l2_cpu3_ddbl_ecc_err_r3),
                               .l2_cpu3_dext_err_r2                            (l2_cpu3_dext_err_r2),
                               .l2_cpu3_dext_err_type_r2                       (l2_cpu3_dext_err_type_r2),
                               .l2_cpu3_disable_clean_evict_opt                (l2_cpu3_disable_clean_evict_opt),
                               .l2_cpu3_dlast_r1                               (l2_cpu3_dlast_r1),
                               .l2_cpu3_dsngl_ecc_err_r3                       (l2_cpu3_dsngl_ecc_err_r3),
                               .l2_cpu3_dsq_clr_id_q                           (l2_cpu3_dsq_clr_id_q[3:0]),
                               .l2_cpu3_dsq_clr_vld_q                          (l2_cpu3_dsq_clr_vld_q),
                               .l2_cpu3_dsq_rd_buf_id                          (l2_cpu3_dsq_rd_buf_id[3:0]),
                               .l2_cpu3_dsq_rd_en                              (l2_cpu3_dsq_rd_en),
                               .l2_cpu3_dsq_rd_en_x2                           (l2_cpu3_dsq_rd_en_x2),
                               .l2_cpu3_dvalid_r1                              (l2_cpu3_dvalid_r1),
                               .l2_cpu3_flsh_if_rd_id_l4_dly                   (l2_cpu3_flsh_if_rd_id_l4_dly[1:0]),
                               .l2_cpu3_flsh_if_rd_l4_dly                      (l2_cpu3_flsh_if_rd_l4_dly),
                               .l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly       (l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu3_flsh_ls_rd_id_l2_dly                   (l2_cpu3_flsh_ls_rd_id_l2_dly[2:0]),
                               .l2_cpu3_flsh_ls_rd_id_l4_dly                   (l2_cpu3_flsh_ls_rd_id_l4_dly[2:0]),
                               .l2_cpu3_flsh_ls_rd_l2_dly                      (l2_cpu3_flsh_ls_rd_l2_dly),
                               .l2_cpu3_flsh_ls_rd_l4_dly                      (l2_cpu3_flsh_ls_rd_l4_dly),
                               .l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly       (l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu3_flsh_ls_wr_evict_l4_dly                (l2_cpu3_flsh_ls_wr_evict_l4_dly),
                               .l2_cpu3_flsh_ls_wr_id_l2_dly                   (l2_cpu3_flsh_ls_wr_id_l2_dly[3:0]),
                               .l2_cpu3_flsh_ls_wr_id_l4_dly                   (l2_cpu3_flsh_ls_wr_id_l4_dly[3:0]),
                               .l2_cpu3_flsh_ls_wr_l2_dly                      (l2_cpu3_flsh_ls_wr_l2_dly),
                               .l2_cpu3_flsh_ls_wr_l4_dly                      (l2_cpu3_flsh_ls_wr_l4_dly),
                               .l2_cpu3_flsh_tw_rd_l4_dly                      (l2_cpu3_flsh_tw_rd_l4_dly),
                               .l2_cpu3_ibufid_r1                              (l2_cpu3_ibufid_r1[1:0]),
                               .l2_cpu3_ic_barrier_stall_q                     (l2_cpu3_ic_barrier_stall_q),
                               .l2_cpu3_ic_base                                (l2_cpu3_ic_base[43:18]),
                               .l2_cpu3_ic_vld_skid                            (l2_cpu3_ic_vld_skid),
                               .l2_cpu3_idata_r2                               (l2_cpu3_idata_r2[127:0]),
                               .l2_cpu3_idbl_ecc_err_r3                        (l2_cpu3_idbl_ecc_err_r3),
                               .l2_cpu3_idle_block_reqs_q                      (l2_cpu3_idle_block_reqs_q),
                               .l2_cpu3_iext_err_r2                            (l2_cpu3_iext_err_r2),
                               .l2_cpu3_iext_err_type_r2                       (l2_cpu3_iext_err_type_r2),
                               .l2_cpu3_if_ccb_clken_c3                        (l2_cpu3_if_ccb_clken_c3),
                               .l2_cpu3_if_ccb_req_c3                          (l2_cpu3_if_ccb_req_c3),
                               .l2_cpu3_if_sync_req                            (l2_cpu3_if_sync_req),
                               .l2_cpu3_ifq_haz_pending                        (l2_cpu3_ifq_haz_pending),
                               .l2_cpu3_isngl_ecc_err_r3                       (l2_cpu3_isngl_ecc_err_r3),
                               .l2_cpu3_ivalid_r1                              (l2_cpu3_ivalid_r1),
                               .l2_cpu3_l2_cache_size                          (l2_cpu3_l2_cache_size[1:0]),
                               .l2_cpu3_lrq_haz_pending                        (l2_cpu3_lrq_haz_pending),
                               .l2_cpu3_ls_ccb_clken_c3                        (l2_cpu3_ls_ccb_clken_c3),
                               .l2_cpu3_ls_ccb_req_c3                          (l2_cpu3_ls_ccb_req_c3),
                               .l2_cpu3_ls_peq_coll_l4_dly                     (l2_cpu3_ls_peq_coll_l4_dly),
                               .l2_cpu3_ls_rd_haz_vld_l2_dly_q                 (l2_cpu3_ls_rd_haz_vld_l2_dly_q),
                               .l2_cpu3_ls_sync_req                            (l2_cpu3_ls_sync_req),
                               .l2_cpu3_ls_wr_haz_vld_l2_dly_q                 (l2_cpu3_ls_wr_haz_vld_l2_dly_q),
                               .l2_cpu3_mbist1_addr_b1                         (l2_cpu3_mbist1_addr_b1[12:0]),
                               .l2_cpu3_mbist1_all_b1                          (l2_cpu3_mbist1_all_b1),
                               .l2_cpu3_mbist1_array_b1                        (l2_cpu3_mbist1_array_b1[3:0]),
                               .l2_cpu3_mbist1_be_b1                           (l2_cpu3_mbist1_be_b1[7:0]),
                               .l2_cpu3_mbist1_en_b1                           (l2_cpu3_mbist1_en_b1),
                               .l2_cpu3_mbist1_rd_en_b1                        (l2_cpu3_mbist1_rd_en_b1),
                               .l2_cpu3_mbist1_wr_en_b1                        (l2_cpu3_mbist1_wr_en_b1),
                               .l2_cpu3_no_intctrl                             (l2_cpu3_no_intctrl),
                               .l2_cpu3_pf_rd_vld_skid_popped                  (l2_cpu3_pf_rd_vld_skid_popped),
                               .l2_cpu3_pf_throttle_q                          (l2_cpu3_pf_throttle_q),
                               .l2_cpu3_pmu_events                             (l2_cpu3_pmu_events[33:0]),
                               .l2_cpu3_rbufid                                 (l2_cpu3_rbufid[2:0]),
                               .l2_cpu3_rd_arb                                 (l2_cpu3_rd_arb),
                               .l2_cpu3_rd_vld_skid                            (l2_cpu3_rd_vld_skid),
                               .l2_cpu3_rexfail                                (l2_cpu3_rexfail),
                               .l2_cpu3_rstate                                 (l2_cpu3_rstate[1:0]),
                               .l2_cpu3_rvalid                                 (l2_cpu3_rvalid),
                               .l2_cpu3_snp_active                             (l2_cpu3_snp_active),
                               .l2_cpu3_spec_bufid                             (l2_cpu3_spec_bufid[2:0]),
                               .l2_cpu3_spec_valid                             (l2_cpu3_spec_valid),
                               .l2_cpu3_spr_rd_data                            (l2_cpu3_spr_rd_data[63:0]),
                               .l2_cpu3_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu3_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu3_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu3_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu3_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu3_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                               .l2_cpu3_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu3_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                               .l2_cpu3_tbw_dbl_ecc_err                        (l2_cpu3_tbw_dbl_ecc_err),
                               .l2_cpu3_tbw_desc_data                          (l2_cpu3_tbw_desc_data[63:0]),
                               .l2_cpu3_tbw_desc_vld                           (l2_cpu3_tbw_desc_vld),
                               .l2_cpu3_tbw_ext_err                            (l2_cpu3_tbw_ext_err),
                               .l2_cpu3_tbw_ext_err_type                       (l2_cpu3_tbw_ext_err_type),
                               .l2_cpu3_tlb_ccb_clken_c3                       (l2_cpu3_tlb_ccb_clken_c3),
                               .l2_cpu3_tlb_ccb_req_c3                         (l2_cpu3_tlb_ccb_req_c3),
                               .l2_cpu3_tlb_sync_complete                      (l2_cpu3_tlb_sync_complete),
                               .l2_cpu3_tlb_sync_req                           (l2_cpu3_tlb_sync_req),
                               .l2_cpu3_trq_haz_pending                        (l2_cpu3_trq_haz_pending),
                               .l2_cpu3_wr_arb                                 (l2_cpu3_wr_arb),
                               .l2_cpu3_wr_data_stall                          (l2_cpu3_wr_data_stall),
                               .l2_cpu3_wr_decerr_q                            (l2_cpu3_wr_decerr_q),
                               .l2_cpu3_wr_ex_fail                             (l2_cpu3_wr_ex_fail),
                               .l2_cpu3_wr_ex_resp                             (l2_cpu3_wr_ex_resp),
                               .l2_cpu3_wr_slverr_q                            (l2_cpu3_wr_slverr_q),
                               .l2_cpu3_wr_vld_skid                            (l2_cpu3_wr_vld_skid),
                               .l2_cpu3_wrq_haz_pending                        (l2_cpu3_wrq_haz_pending),
                               .l2_ctlr_x1_wr_q                                (l2_ctlr_x1_wr_q),
                               .l2_ctlr_x2_ns                                  (l2_ctlr_x2_ns[9:0]),
                               .l2_idle                                        (l2_idle),
                               .l2_mbist1_en_b1                                (l2_mbist1_en_b1[`MAIA_CN:0]),
                               .l2_mbist2_tbnk0_snp0_outdata_b2                (l2_mbist2_tbnk0_snp0_outdata_b2[79:0]),
                               .l2_mbist2_tbnk0_snp0_outdata_vld_b2            (l2_mbist2_tbnk0_snp0_outdata_vld_b2),
                               .l2_mbist2_tbnk0_snp1_outdata_b2                (l2_mbist2_tbnk0_snp1_outdata_b2[79:0]),
                               .l2_mbist2_tbnk0_snp1_outdata_vld_b2            (l2_mbist2_tbnk0_snp1_outdata_vld_b2),
                               .l2_mbist2_tbnk0_snp2_outdata_b2                (l2_mbist2_tbnk0_snp2_outdata_b2[79:0]),
                               .l2_mbist2_tbnk0_snp2_outdata_vld_b2            (l2_mbist2_tbnk0_snp2_outdata_vld_b2),
                               .l2_mbist2_tbnk0_snp3_outdata_b2                (l2_mbist2_tbnk0_snp3_outdata_b2[79:0]),
                               .l2_mbist2_tbnk0_snp3_outdata_vld_b2            (l2_mbist2_tbnk0_snp3_outdata_vld_b2),
                               .l2_mbist2_tbnk1_snp0_outdata_b2                (l2_mbist2_tbnk1_snp0_outdata_b2[79:0]),
                               .l2_mbist2_tbnk1_snp0_outdata_vld_b2            (l2_mbist2_tbnk1_snp0_outdata_vld_b2),
                               .l2_mbist2_tbnk1_snp1_outdata_b2                (l2_mbist2_tbnk1_snp1_outdata_b2[79:0]),
                               .l2_mbist2_tbnk1_snp1_outdata_vld_b2            (l2_mbist2_tbnk1_snp1_outdata_vld_b2),
                               .l2_mbist2_tbnk1_snp2_outdata_b2                (l2_mbist2_tbnk1_snp2_outdata_b2[79:0]),
                               .l2_mbist2_tbnk1_snp2_outdata_vld_b2            (l2_mbist2_tbnk1_snp2_outdata_vld_b2),
                               .l2_mbist2_tbnk1_snp3_outdata_b2                (l2_mbist2_tbnk1_snp3_outdata_b2[79:0]),
                               .l2_mbist2_tbnk1_snp3_outdata_vld_b2            (l2_mbist2_tbnk1_snp3_outdata_vld_b2),
                               .l2_no_ram_acc_nxt_cycle                        (l2_no_ram_acc_nxt_cycle),
                               .l2_p_addr                                      (l2_p_addr[13:0]),
                               .l2_p_cpu                                       (l2_p_cpu[1:0]),
                               .l2_p_nsecure                                   (l2_p_nsecure),
                               .l2_p_sel                                       (l2_p_sel[2:0]),
                               .l2_p_wdata                                     (l2_p_wdata[31:0]),
                               .l2_p_write                                     (l2_p_write),
                               .l2_reset3                                      (l2_reset3),
                               .l2_rstdisable_x1_q                             (l2_rstdisable_x1_q),
                               .l2_sky_link_stopped                            (l2_sky_link_stopped),
                               .l2_tbnk0_addr_l1                               (l2_tbnk0_addr_l1[44:0]),
                               .l2_tbnk0_asq_cmp_evict_l3_q                    (l2_tbnk0_asq_cmp_evict_l3_q),
                               .l2_tbnk0_asq_full_flsh                         (l2_tbnk0_asq_full_flsh),
                               .l2_tbnk0_asq_nc_so_dev_limit                   (l2_tbnk0_asq_nc_so_dev_limit),
                               .l2_tbnk0_cache_attr_l1                         (l2_tbnk0_cache_attr_l1[2:0]),
                               .l2_tbnk0_cfg_ecc_en                            (l2_tbnk0_cfg_ecc_en),
                               .l2_tbnk0_cpu0_ifq_clr_l4_dly2_q                (l2_tbnk0_cpu0_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk0_cpu0_lrq_clr_l4_dly2_q                (l2_tbnk0_cpu0_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk0_cpu0_peq_full_q                       (l2_tbnk0_cpu0_peq_full_q),
                               .l2_tbnk0_cpu0_peq_hit_q                        (l2_tbnk0_cpu0_peq_hit_q),
                               .l2_tbnk0_cpu0_peq_self_evict_l3_q              (l2_tbnk0_cpu0_peq_self_evict_l3_q),
                               .l2_tbnk0_cpu0_peq_self_evict_wbna_l3_q         (l2_tbnk0_cpu0_peq_self_evict_wbna_l3_q),
                               .l2_tbnk0_cpu0_snp_hit_e_l3                     (l2_tbnk0_cpu0_snp_hit_e_l3),
                               .l2_tbnk0_cpu0_snp_hit_s_l3                     (l2_tbnk0_cpu0_snp_hit_s_l3),
                               .l2_tbnk0_cpu0_snp_setway_addr_l3               (l2_tbnk0_cpu0_snp_setway_addr_l3[44:14]),
                               .l2_tbnk0_cpu0_snp_tag_ecc_err_tp_l4_q          (l2_tbnk0_cpu0_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk0_cpu0_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk0_cpu0_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk0_cpu0_trq_clr_l4_dly2_q                (l2_tbnk0_cpu0_trq_clr_l4_dly2_q),
                               .l2_tbnk0_cpu0_wrq_clr_l4_dly2_q                (l2_tbnk0_cpu0_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk0_cpu1_ifq_clr_l4_dly2_q                (l2_tbnk0_cpu1_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk0_cpu1_lrq_clr_l4_dly2_q                (l2_tbnk0_cpu1_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk0_cpu1_peq_full_q                       (l2_tbnk0_cpu1_peq_full_q),
                               .l2_tbnk0_cpu1_peq_hit_q                        (l2_tbnk0_cpu1_peq_hit_q),
                               .l2_tbnk0_cpu1_peq_self_evict_l3_q              (l2_tbnk0_cpu1_peq_self_evict_l3_q),
                               .l2_tbnk0_cpu1_peq_self_evict_wbna_l3_q         (l2_tbnk0_cpu1_peq_self_evict_wbna_l3_q),
                               .l2_tbnk0_cpu1_snp_hit_e_l3                     (l2_tbnk0_cpu1_snp_hit_e_l3),
                               .l2_tbnk0_cpu1_snp_hit_s_l3                     (l2_tbnk0_cpu1_snp_hit_s_l3),
                               .l2_tbnk0_cpu1_snp_setway_addr_l3               (l2_tbnk0_cpu1_snp_setway_addr_l3[44:14]),
                               .l2_tbnk0_cpu1_snp_tag_ecc_err_tp_l4_q          (l2_tbnk0_cpu1_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk0_cpu1_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk0_cpu1_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk0_cpu1_trq_clr_l4_dly2_q                (l2_tbnk0_cpu1_trq_clr_l4_dly2_q),
                               .l2_tbnk0_cpu1_wrq_clr_l4_dly2_q                (l2_tbnk0_cpu1_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk0_cpu2_ifq_clr_l4_dly2_q                (l2_tbnk0_cpu2_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk0_cpu2_lrq_clr_l4_dly2_q                (l2_tbnk0_cpu2_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk0_cpu2_peq_full_q                       (l2_tbnk0_cpu2_peq_full_q),
                               .l2_tbnk0_cpu2_peq_hit_q                        (l2_tbnk0_cpu2_peq_hit_q),
                               .l2_tbnk0_cpu2_peq_self_evict_l3_q              (l2_tbnk0_cpu2_peq_self_evict_l3_q),
                               .l2_tbnk0_cpu2_peq_self_evict_wbna_l3_q         (l2_tbnk0_cpu2_peq_self_evict_wbna_l3_q),
                               .l2_tbnk0_cpu2_snp_hit_e_l3                     (l2_tbnk0_cpu2_snp_hit_e_l3),
                               .l2_tbnk0_cpu2_snp_hit_s_l3                     (l2_tbnk0_cpu2_snp_hit_s_l3),
                               .l2_tbnk0_cpu2_snp_setway_addr_l3               (l2_tbnk0_cpu2_snp_setway_addr_l3[44:14]),
                               .l2_tbnk0_cpu2_snp_tag_ecc_err_tp_l4_q          (l2_tbnk0_cpu2_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk0_cpu2_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk0_cpu2_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk0_cpu2_trq_clr_l4_dly2_q                (l2_tbnk0_cpu2_trq_clr_l4_dly2_q),
                               .l2_tbnk0_cpu2_wrq_clr_l4_dly2_q                (l2_tbnk0_cpu2_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk0_cpu3_ifq_clr_l4_dly2_q                (l2_tbnk0_cpu3_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk0_cpu3_lrq_clr_l4_dly2_q                (l2_tbnk0_cpu3_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk0_cpu3_peq_full_q                       (l2_tbnk0_cpu3_peq_full_q),
                               .l2_tbnk0_cpu3_peq_hit_q                        (l2_tbnk0_cpu3_peq_hit_q),
                               .l2_tbnk0_cpu3_peq_self_evict_l3_q              (l2_tbnk0_cpu3_peq_self_evict_l3_q),
                               .l2_tbnk0_cpu3_peq_self_evict_wbna_l3_q         (l2_tbnk0_cpu3_peq_self_evict_wbna_l3_q),
                               .l2_tbnk0_cpu3_snp_hit_e_l3                     (l2_tbnk0_cpu3_snp_hit_e_l3),
                               .l2_tbnk0_cpu3_snp_hit_s_l3                     (l2_tbnk0_cpu3_snp_hit_s_l3),
                               .l2_tbnk0_cpu3_snp_setway_addr_l3               (l2_tbnk0_cpu3_snp_setway_addr_l3[44:14]),
                               .l2_tbnk0_cpu3_snp_tag_ecc_err_tp_l4_q          (l2_tbnk0_cpu3_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk0_cpu3_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk0_cpu3_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk0_cpu3_trq_clr_l4_dly2_q                (l2_tbnk0_cpu3_trq_clr_l4_dly2_q),
                               .l2_tbnk0_cpu3_wrq_clr_l4_dly2_q                (l2_tbnk0_cpu3_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk0_dirty_l1                              (l2_tbnk0_dirty_l1),
                               .l2_tbnk0_dis_ns_dbg_arr_acc_x2                 (l2_tbnk0_dis_ns_dbg_arr_acc_x2),
                               .l2_tbnk0_excl_l1                               (l2_tbnk0_excl_l1),
                               .l2_tbnk0_feq_alloc_failed_l4                   (l2_tbnk0_feq_alloc_failed_l4),
                               .l2_tbnk0_feq_axi_wr_vld_not_popped             (l2_tbnk0_feq_axi_wr_vld_not_popped),
                               .l2_tbnk0_feq_frc_incl_l3a                      (l2_tbnk0_feq_frc_incl_l3a[15:0]),
                               .l2_tbnk0_feq_kill_l3                           (l2_tbnk0_feq_kill_l3),
                               .l2_tbnk0_feq_last_id_q                         (l2_tbnk0_feq_last_id_q[4:0]),
                               .l2_tbnk0_feq_tbnk_id_update_hit_prfm_or_l3     (l2_tbnk0_feq_tbnk_id_update_hit_prfm_or_l3),
                               .l2_tbnk0_feq_tbnk_id_update_or_l3              (l2_tbnk0_feq_tbnk_id_update_or_l3),
                               .l2_tbnk0_id_l1                                 (l2_tbnk0_id_l1[9:0]),
                               .l2_tbnk0_init_req_l1                           (l2_tbnk0_init_req_l1),
                               .l2_tbnk0_kill_l2                               (l2_tbnk0_kill_l2),
                               .l2_tbnk0_l2bb_fake_wr_l1                       (l2_tbnk0_l2bb_fake_wr_l1),
                               .l2_tbnk0_l2bb_wr_l1                            (l2_tbnk0_l2bb_wr_l1),
                               .l2_tbnk0_last_qw_l1                            (l2_tbnk0_last_qw_l1),
                               .l2_tbnk0_lock_l1                               (l2_tbnk0_lock_l1[2:0]),
                               .l2_tbnk0_page_attr_l1                          (l2_tbnk0_page_attr_l1[9:0]),
                               .l2_tbnk0_partial_dw_wr_l1                      (l2_tbnk0_partial_dw_wr_l1),
                               .l2_tbnk0_pf_hazard_l3                          (l2_tbnk0_pf_hazard_l3),
                               .l2_tbnk0_prfm_l1                               (l2_tbnk0_prfm_l1),
                               .l2_tbnk0_prot_l1                               (l2_tbnk0_prot_l1[3:0]),
                               .l2_tbnk0_qw_cnt_l1                             (l2_tbnk0_qw_cnt_l1[1:0]),
                               .l2_tbnk0_rd_fail_hazchk_feq_l3                 (l2_tbnk0_rd_fail_hazchk_feq_l3),
                               .l2_tbnk0_rwvic_axi_read_err_l1                 (l2_tbnk0_rwvic_axi_read_err_l1),
                               .l2_tbnk0_rwvic_ccb_ls_xfer_l1                  (l2_tbnk0_rwvic_ccb_ls_xfer_l1),
                               .l2_tbnk0_rwvic_ccb_way_l1                      (l2_tbnk0_rwvic_ccb_way_l1[3:0]),
                               .l2_tbnk0_rwvic_cmo_clean_l1                    (l2_tbnk0_rwvic_cmo_clean_l1),
                               .l2_tbnk0_rwvic_cmo_inv_l1                      (l2_tbnk0_rwvic_cmo_inv_l1),
                               .l2_tbnk0_rwvic_cmo_pou_l1                      (l2_tbnk0_rwvic_cmo_pou_l1),
                               .l2_tbnk0_rwvic_cmo_setway_l1                   (l2_tbnk0_rwvic_cmo_setway_l1),
                               .l2_tbnk0_rwvic_cmo_setway_ls_full_miss_l1      (l2_tbnk0_rwvic_cmo_setway_ls_full_miss_l1),
                               .l2_tbnk0_rwvic_cpu_fb_id_l1                    (l2_tbnk0_rwvic_cpu_fb_id_l1[2:0]),
                               .l2_tbnk0_rwvic_cpu_id_dcd_l1                   (l2_tbnk0_rwvic_cpu_id_dcd_l1[3:0]),
                               .l2_tbnk0_rwvic_feq_cmp_l3_q                    (l2_tbnk0_rwvic_feq_cmp_l3_q),
                               .l2_tbnk0_rwvic_frc_l2hit_fwd_l1                (l2_tbnk0_rwvic_frc_l2hit_fwd_l1),
                               .l2_tbnk0_rwvic_l2hit_e_l1                      (l2_tbnk0_rwvic_l2hit_e_l1),
                               .l2_tbnk0_rwvic_mesi_sh_l1                      (l2_tbnk0_rwvic_mesi_sh_l1),
                               .l2_tbnk0_rwvic_owner_l1                        (l2_tbnk0_rwvic_owner_l1[2:0]),
                               .l2_tbnk0_rwvic_snp_clr_dirty_l1                (l2_tbnk0_rwvic_snp_clr_dirty_l1),
                               .l2_tbnk0_rwvic_snp_inv_l1                      (l2_tbnk0_rwvic_snp_inv_l1),
                               .l2_tbnk0_rwvic_snp_l1                          (l2_tbnk0_rwvic_snp_l1),
                               .l2_tbnk0_rwvic_type_l1                         (l2_tbnk0_rwvic_type_l1[3:0]),
                               .l2_tbnk0_rwvic_wa_l1                           (l2_tbnk0_rwvic_wa_l1),
                               .l2_tbnk0_sel_l1                                (l2_tbnk0_sel_l1[13:0]),
                               .l2_tbnk0_size_l1                               (l2_tbnk0_size_l1[2:0]),
                               .l2_tbnk0_snp_byp_peq_haz_pending_q             (l2_tbnk0_snp_byp_peq_haz_pending_q),
                               .l2_tbnk0_snp_dvm_cmpl_l1                       (l2_tbnk0_snp_dvm_cmpl_l1),
                               .l2_tbnk0_snp_hit_feq_evict_l4_dly              (l2_tbnk0_snp_hit_feq_evict_l4_dly),
                               .l2_tbnk0_snp_rd_feq_id_cmp_l4_dly_q            (l2_tbnk0_snp_rd_feq_id_cmp_l4_dly_q[4:0]),
                               .l2_tbnk0_snp_tag_double_ecc_err_l4_dly_q       (l2_tbnk0_snp_tag_double_ecc_err_l4_dly_q[7:0]),
                               .l2_tbnk0_snp_tag_single_ecc_err_l4_dly_q       (l2_tbnk0_snp_tag_single_ecc_err_l4_dly_q[7:0]),
                               .l2_tbnk0_sync_l1                               (l2_tbnk0_sync_l1),
                               .l2_tbnk0_type_l1                               (l2_tbnk0_type_l1[6:0]),
                               .l2_tbnk0_ulen_l1                               (l2_tbnk0_ulen_l1[1:0]),
                               .l2_tbnk0_way_l1                                (l2_tbnk0_way_l1),
                               .l2_tbnk0_wr_data_l3a_q                         (l2_tbnk0_wr_data_l3a_q[127:0]),
                               .l2_tbnk0_wr_err_l1                             (l2_tbnk0_wr_err_l1),
                               .l2_tbnk0_wr_fail_feq_full_l3                   (l2_tbnk0_wr_fail_feq_full_l3),
                               .l2_tbnk0_wr_fail_hazchk_feq_l3                 (l2_tbnk0_wr_fail_hazchk_feq_l3),
                               .l2_tbnk0_wr_non_crit_id_l1                     (l2_tbnk0_wr_non_crit_id_l1[11:0]),
                               .l2_tbnk0_wr_strb_mask_l3a_q                    (l2_tbnk0_wr_strb_mask_l3a_q[15:0]),
                               .l2_tbnk1_addr_l1                               (l2_tbnk1_addr_l1[44:0]),
                               .l2_tbnk1_asq_cmp_evict_l3_q                    (l2_tbnk1_asq_cmp_evict_l3_q),
                               .l2_tbnk1_asq_full_flsh                         (l2_tbnk1_asq_full_flsh),
                               .l2_tbnk1_asq_nc_so_dev_limit                   (l2_tbnk1_asq_nc_so_dev_limit),
                               .l2_tbnk1_cache_attr_l1                         (l2_tbnk1_cache_attr_l1[2:0]),
                               .l2_tbnk1_cfg_ecc_en                            (l2_tbnk1_cfg_ecc_en),
                               .l2_tbnk1_cpu0_ifq_clr_l4_dly2_q                (l2_tbnk1_cpu0_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk1_cpu0_lrq_clr_l4_dly2_q                (l2_tbnk1_cpu0_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk1_cpu0_peq_full_q                       (l2_tbnk1_cpu0_peq_full_q),
                               .l2_tbnk1_cpu0_peq_hit_q                        (l2_tbnk1_cpu0_peq_hit_q),
                               .l2_tbnk1_cpu0_peq_self_evict_l3_q              (l2_tbnk1_cpu0_peq_self_evict_l3_q),
                               .l2_tbnk1_cpu0_peq_self_evict_wbna_l3_q         (l2_tbnk1_cpu0_peq_self_evict_wbna_l3_q),
                               .l2_tbnk1_cpu0_snp_hit_e_l3                     (l2_tbnk1_cpu0_snp_hit_e_l3),
                               .l2_tbnk1_cpu0_snp_hit_s_l3                     (l2_tbnk1_cpu0_snp_hit_s_l3),
                               .l2_tbnk1_cpu0_snp_setway_addr_l3               (l2_tbnk1_cpu0_snp_setway_addr_l3[44:14]),
                               .l2_tbnk1_cpu0_snp_tag_ecc_err_tp_l4_q          (l2_tbnk1_cpu0_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk1_cpu0_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk1_cpu0_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk1_cpu0_trq_clr_l4_dly2_q                (l2_tbnk1_cpu0_trq_clr_l4_dly2_q),
                               .l2_tbnk1_cpu0_wrq_clr_l4_dly2_q                (l2_tbnk1_cpu0_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk1_cpu1_ifq_clr_l4_dly2_q                (l2_tbnk1_cpu1_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk1_cpu1_lrq_clr_l4_dly2_q                (l2_tbnk1_cpu1_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk1_cpu1_peq_full_q                       (l2_tbnk1_cpu1_peq_full_q),
                               .l2_tbnk1_cpu1_peq_hit_q                        (l2_tbnk1_cpu1_peq_hit_q),
                               .l2_tbnk1_cpu1_peq_self_evict_l3_q              (l2_tbnk1_cpu1_peq_self_evict_l3_q),
                               .l2_tbnk1_cpu1_peq_self_evict_wbna_l3_q         (l2_tbnk1_cpu1_peq_self_evict_wbna_l3_q),
                               .l2_tbnk1_cpu1_snp_hit_e_l3                     (l2_tbnk1_cpu1_snp_hit_e_l3),
                               .l2_tbnk1_cpu1_snp_hit_s_l3                     (l2_tbnk1_cpu1_snp_hit_s_l3),
                               .l2_tbnk1_cpu1_snp_setway_addr_l3               (l2_tbnk1_cpu1_snp_setway_addr_l3[44:14]),
                               .l2_tbnk1_cpu1_snp_tag_ecc_err_tp_l4_q          (l2_tbnk1_cpu1_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk1_cpu1_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk1_cpu1_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk1_cpu1_trq_clr_l4_dly2_q                (l2_tbnk1_cpu1_trq_clr_l4_dly2_q),
                               .l2_tbnk1_cpu1_wrq_clr_l4_dly2_q                (l2_tbnk1_cpu1_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk1_cpu2_ifq_clr_l4_dly2_q                (l2_tbnk1_cpu2_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk1_cpu2_lrq_clr_l4_dly2_q                (l2_tbnk1_cpu2_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk1_cpu2_peq_full_q                       (l2_tbnk1_cpu2_peq_full_q),
                               .l2_tbnk1_cpu2_peq_hit_q                        (l2_tbnk1_cpu2_peq_hit_q),
                               .l2_tbnk1_cpu2_peq_self_evict_l3_q              (l2_tbnk1_cpu2_peq_self_evict_l3_q),
                               .l2_tbnk1_cpu2_peq_self_evict_wbna_l3_q         (l2_tbnk1_cpu2_peq_self_evict_wbna_l3_q),
                               .l2_tbnk1_cpu2_snp_hit_e_l3                     (l2_tbnk1_cpu2_snp_hit_e_l3),
                               .l2_tbnk1_cpu2_snp_hit_s_l3                     (l2_tbnk1_cpu2_snp_hit_s_l3),
                               .l2_tbnk1_cpu2_snp_setway_addr_l3               (l2_tbnk1_cpu2_snp_setway_addr_l3[44:14]),
                               .l2_tbnk1_cpu2_snp_tag_ecc_err_tp_l4_q          (l2_tbnk1_cpu2_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk1_cpu2_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk1_cpu2_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk1_cpu2_trq_clr_l4_dly2_q                (l2_tbnk1_cpu2_trq_clr_l4_dly2_q),
                               .l2_tbnk1_cpu2_wrq_clr_l4_dly2_q                (l2_tbnk1_cpu2_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk1_cpu3_ifq_clr_l4_dly2_q                (l2_tbnk1_cpu3_ifq_clr_l4_dly2_q[2:0]),
                               .l2_tbnk1_cpu3_lrq_clr_l4_dly2_q                (l2_tbnk1_cpu3_lrq_clr_l4_dly2_q[3:0]),
                               .l2_tbnk1_cpu3_peq_full_q                       (l2_tbnk1_cpu3_peq_full_q),
                               .l2_tbnk1_cpu3_peq_hit_q                        (l2_tbnk1_cpu3_peq_hit_q),
                               .l2_tbnk1_cpu3_peq_self_evict_l3_q              (l2_tbnk1_cpu3_peq_self_evict_l3_q),
                               .l2_tbnk1_cpu3_peq_self_evict_wbna_l3_q         (l2_tbnk1_cpu3_peq_self_evict_wbna_l3_q),
                               .l2_tbnk1_cpu3_snp_hit_e_l3                     (l2_tbnk1_cpu3_snp_hit_e_l3),
                               .l2_tbnk1_cpu3_snp_hit_s_l3                     (l2_tbnk1_cpu3_snp_hit_s_l3),
                               .l2_tbnk1_cpu3_snp_setway_addr_l3               (l2_tbnk1_cpu3_snp_setway_addr_l3[44:14]),
                               .l2_tbnk1_cpu3_snp_tag_ecc_err_tp_l4_q          (l2_tbnk1_cpu3_snp_tag_ecc_err_tp_l4_q),
                               .l2_tbnk1_cpu3_snp_tag_wr_evict_qual_l4_dly     (l2_tbnk1_cpu3_snp_tag_wr_evict_qual_l4_dly),
                               .l2_tbnk1_cpu3_trq_clr_l4_dly2_q                (l2_tbnk1_cpu3_trq_clr_l4_dly2_q),
                               .l2_tbnk1_cpu3_wrq_clr_l4_dly2_q                (l2_tbnk1_cpu3_wrq_clr_l4_dly2_q[5:0]),
                               .l2_tbnk1_dirty_l1                              (l2_tbnk1_dirty_l1),
                               .l2_tbnk1_dis_ns_dbg_arr_acc_x2                 (l2_tbnk1_dis_ns_dbg_arr_acc_x2),
                               .l2_tbnk1_excl_l1                               (l2_tbnk1_excl_l1),
                               .l2_tbnk1_feq_alloc_failed_l4                   (l2_tbnk1_feq_alloc_failed_l4),
                               .l2_tbnk1_feq_axi_wr_vld_not_popped             (l2_tbnk1_feq_axi_wr_vld_not_popped),
                               .l2_tbnk1_feq_frc_incl_l3a                      (l2_tbnk1_feq_frc_incl_l3a[15:0]),
                               .l2_tbnk1_feq_kill_l3                           (l2_tbnk1_feq_kill_l3),
                               .l2_tbnk1_feq_last_id_q                         (l2_tbnk1_feq_last_id_q[4:0]),
                               .l2_tbnk1_feq_tbnk_id_update_hit_prfm_or_l3     (l2_tbnk1_feq_tbnk_id_update_hit_prfm_or_l3),
                               .l2_tbnk1_feq_tbnk_id_update_or_l3              (l2_tbnk1_feq_tbnk_id_update_or_l3),
                               .l2_tbnk1_id_l1                                 (l2_tbnk1_id_l1[9:0]),
                               .l2_tbnk1_init_req_l1                           (l2_tbnk1_init_req_l1),
                               .l2_tbnk1_kill_l2                               (l2_tbnk1_kill_l2),
                               .l2_tbnk1_l2bb_fake_wr_l1                       (l2_tbnk1_l2bb_fake_wr_l1),
                               .l2_tbnk1_l2bb_wr_l1                            (l2_tbnk1_l2bb_wr_l1),
                               .l2_tbnk1_last_qw_l1                            (l2_tbnk1_last_qw_l1),
                               .l2_tbnk1_lock_l1                               (l2_tbnk1_lock_l1[2:0]),
                               .l2_tbnk1_page_attr_l1                          (l2_tbnk1_page_attr_l1[9:0]),
                               .l2_tbnk1_partial_dw_wr_l1                      (l2_tbnk1_partial_dw_wr_l1),
                               .l2_tbnk1_pf_hazard_l3                          (l2_tbnk1_pf_hazard_l3),
                               .l2_tbnk1_prfm_l1                               (l2_tbnk1_prfm_l1),
                               .l2_tbnk1_prot_l1                               (l2_tbnk1_prot_l1[3:0]),
                               .l2_tbnk1_qw_cnt_l1                             (l2_tbnk1_qw_cnt_l1[1:0]),
                               .l2_tbnk1_rd_fail_hazchk_feq_l3                 (l2_tbnk1_rd_fail_hazchk_feq_l3),
                               .l2_tbnk1_rwvic_axi_read_err_l1                 (l2_tbnk1_rwvic_axi_read_err_l1),
                               .l2_tbnk1_rwvic_ccb_ls_xfer_l1                  (l2_tbnk1_rwvic_ccb_ls_xfer_l1),
                               .l2_tbnk1_rwvic_ccb_way_l1                      (l2_tbnk1_rwvic_ccb_way_l1[3:0]),
                               .l2_tbnk1_rwvic_cmo_clean_l1                    (l2_tbnk1_rwvic_cmo_clean_l1),
                               .l2_tbnk1_rwvic_cmo_inv_l1                      (l2_tbnk1_rwvic_cmo_inv_l1),
                               .l2_tbnk1_rwvic_cmo_pou_l1                      (l2_tbnk1_rwvic_cmo_pou_l1),
                               .l2_tbnk1_rwvic_cmo_setway_l1                   (l2_tbnk1_rwvic_cmo_setway_l1),
                               .l2_tbnk1_rwvic_cmo_setway_ls_full_miss_l1      (l2_tbnk1_rwvic_cmo_setway_ls_full_miss_l1),
                               .l2_tbnk1_rwvic_cpu_fb_id_l1                    (l2_tbnk1_rwvic_cpu_fb_id_l1[2:0]),
                               .l2_tbnk1_rwvic_cpu_id_dcd_l1                   (l2_tbnk1_rwvic_cpu_id_dcd_l1[3:0]),
                               .l2_tbnk1_rwvic_feq_cmp_l3_q                    (l2_tbnk1_rwvic_feq_cmp_l3_q),
                               .l2_tbnk1_rwvic_frc_l2hit_fwd_l1                (l2_tbnk1_rwvic_frc_l2hit_fwd_l1),
                               .l2_tbnk1_rwvic_l2hit_e_l1                      (l2_tbnk1_rwvic_l2hit_e_l1),
                               .l2_tbnk1_rwvic_mesi_sh_l1                      (l2_tbnk1_rwvic_mesi_sh_l1),
                               .l2_tbnk1_rwvic_owner_l1                        (l2_tbnk1_rwvic_owner_l1[2:0]),
                               .l2_tbnk1_rwvic_snp_clr_dirty_l1                (l2_tbnk1_rwvic_snp_clr_dirty_l1),
                               .l2_tbnk1_rwvic_snp_inv_l1                      (l2_tbnk1_rwvic_snp_inv_l1),
                               .l2_tbnk1_rwvic_snp_l1                          (l2_tbnk1_rwvic_snp_l1),
                               .l2_tbnk1_rwvic_type_l1                         (l2_tbnk1_rwvic_type_l1[3:0]),
                               .l2_tbnk1_rwvic_wa_l1                           (l2_tbnk1_rwvic_wa_l1),
                               .l2_tbnk1_sel_l1                                (l2_tbnk1_sel_l1[13:0]),
                               .l2_tbnk1_size_l1                               (l2_tbnk1_size_l1[2:0]),
                               .l2_tbnk1_snp_byp_peq_haz_pending_q             (l2_tbnk1_snp_byp_peq_haz_pending_q),
                               .l2_tbnk1_snp_dvm_cmpl_l1                       (l2_tbnk1_snp_dvm_cmpl_l1),
                               .l2_tbnk1_snp_hit_feq_evict_l4_dly              (l2_tbnk1_snp_hit_feq_evict_l4_dly),
                               .l2_tbnk1_snp_rd_feq_id_cmp_l4_dly_q            (l2_tbnk1_snp_rd_feq_id_cmp_l4_dly_q[4:0]),
                               .l2_tbnk1_snp_tag_double_ecc_err_l4_dly_q       (l2_tbnk1_snp_tag_double_ecc_err_l4_dly_q[7:0]),
                               .l2_tbnk1_snp_tag_single_ecc_err_l4_dly_q       (l2_tbnk1_snp_tag_single_ecc_err_l4_dly_q[7:0]),
                               .l2_tbnk1_sync_l1                               (l2_tbnk1_sync_l1),
                               .l2_tbnk1_type_l1                               (l2_tbnk1_type_l1[6:0]),
                               .l2_tbnk1_ulen_l1                               (l2_tbnk1_ulen_l1[1:0]),
                               .l2_tbnk1_way_l1                                (l2_tbnk1_way_l1),
                               .l2_tbnk1_wr_data_l3a_q                         (l2_tbnk1_wr_data_l3a_q[127:0]),
                               .l2_tbnk1_wr_err_l1                             (l2_tbnk1_wr_err_l1),
                               .l2_tbnk1_wr_fail_feq_full_l3                   (l2_tbnk1_wr_fail_feq_full_l3),
                               .l2_tbnk1_wr_fail_hazchk_feq_l3                 (l2_tbnk1_wr_fail_hazchk_feq_l3),
                               .l2_tbnk1_wr_non_crit_id_l1                     (l2_tbnk1_wr_non_crit_id_l1[11:0]),
                               .l2_tbnk1_wr_strb_mask_l3a_q                    (l2_tbnk1_wr_strb_mask_l3a_q[15:0]),
                               .l2_tbnk_hwrst_done_x2                          (l2_tbnk_hwrst_done_x2),
                               .l2_tbnk_hwrst_idx_x1_q                         (l2_tbnk_hwrst_idx_x1_q[13:0]),
                               .nEXTERRIRQ                                     (nEXTERRIRQ),
                               .nINTERRIRQ                                     (nINTERRIRQ),

                               // inputs
                               .ACLKENS                                        (ACLKENS),
                               .ARADDRS                                        (ARADDRS[43:0]),
                               .ARCACHES                                       (ARCACHES[3:0]),
                               .ARIDS                                          (ARIDS[4:0]),
                               .ARLENS                                         (ARLENS[7:0]),
                               .ARPROTS                                        (ARPROTS[2:0]),
                               .ARUSERS                                        (ARUSERS[1:0]),
                               .ARVALIDS                                       (ARVALIDS),
                               .AWADDRS                                        (AWADDRS[43:0]),
                               .AWCACHES                                       (AWCACHES[3:0]),
                               .AWIDS                                          (AWIDS[4:0]),
                               .AWLENS                                         (AWLENS[7:0]),
                               .AWPROTS                                        (AWPROTS[2:0]),
                               .AWUSERS                                        (AWUSERS[1:0]),
                               .AWVALIDS                                       (AWVALIDS),
                               .BREADYS                                        (BREADYS),
                               .BROADCASTCACHEMAINT                            (BROADCASTCACHEMAINT),
                               .BROADCASTINNER                                 (BROADCASTINNER),
                               .BROADCASTOUTER                                 (BROADCASTOUTER),
                               .DBGL1RSTDISABLE                                (DBGL1RSTDISABLE),
                               .DFTRAMHOLD                                     (DFTRAMHOLD),
                               .DFTRSTDISABLE                                  (DFTRSTDISABLE),
                               .DFTSE                                          (DFTSE),
                               .L2FLUSHREQ                                     (L2FLUSHREQ),
                               .L2QREQn                                        (L2QREQn),
                               .L2RSTDISABLE                                   (L2RSTDISABLE),
                               .MBISTREQ                                       (MBISTREQ),
                               .NODEID                                         (NODEID[6:0]),
                               .PERIPHBASE                                     (PERIPHBASE[43:18]),
                               .RREADYS                                        (RREADYS),
                               .RXDATFLIT                                      (RXDATFLIT[193:0]),
                               .RXDATFLITPEND                                  (RXDATFLITPEND),
                               .RXDATFLITV                                     (RXDATFLITV),
                               .RXLINKACTIVEREQ                                (RXLINKACTIVEREQ),
                               .RXRSPFLIT                                      (RXRSPFLIT[44:0]),
                               .RXRSPFLITPEND                                  (RXRSPFLITPEND),
                               .RXRSPFLITV                                     (RXRSPFLITV),
                               .RXSACTIVE                                      (RXSACTIVE),
                               .RXSNPFLIT                                      (RXSNPFLIT[64:0]),
                               .RXSNPFLITPEND                                  (RXSNPFLITPEND),
                               .RXSNPFLITV                                     (RXSNPFLITV),
                               .SAMADDRMAP0                                    (SAMADDRMAP0[1:0]),
                               .SAMADDRMAP1                                    (SAMADDRMAP1[1:0]),
                               .SAMADDRMAP10                                   (SAMADDRMAP10[1:0]),
                               .SAMADDRMAP11                                   (SAMADDRMAP11[1:0]),
                               .SAMADDRMAP12                                   (SAMADDRMAP12[1:0]),
                               .SAMADDRMAP13                                   (SAMADDRMAP13[1:0]),
                               .SAMADDRMAP14                                   (SAMADDRMAP14[1:0]),
                               .SAMADDRMAP15                                   (SAMADDRMAP15[1:0]),
                               .SAMADDRMAP16                                   (SAMADDRMAP16[1:0]),
                               .SAMADDRMAP17                                   (SAMADDRMAP17[1:0]),
                               .SAMADDRMAP18                                   (SAMADDRMAP18[1:0]),
                               .SAMADDRMAP19                                   (SAMADDRMAP19[1:0]),
                               .SAMADDRMAP2                                    (SAMADDRMAP2[1:0]),
                               .SAMADDRMAP3                                    (SAMADDRMAP3[1:0]),
                               .SAMADDRMAP4                                    (SAMADDRMAP4[1:0]),
                               .SAMADDRMAP5                                    (SAMADDRMAP5[1:0]),
                               .SAMADDRMAP6                                    (SAMADDRMAP6[1:0]),
                               .SAMADDRMAP7                                    (SAMADDRMAP7[1:0]),
                               .SAMADDRMAP8                                    (SAMADDRMAP8[1:0]),
                               .SAMADDRMAP9                                    (SAMADDRMAP9[1:0]),
                               .SAMHNF0NODEID                                  (SAMHNF0NODEID[6:0]),
                               .SAMHNF1NODEID                                  (SAMHNF1NODEID[6:0]),
                               .SAMHNF2NODEID                                  (SAMHNF2NODEID[6:0]),
                               .SAMHNF3NODEID                                  (SAMHNF3NODEID[6:0]),
                               .SAMHNF4NODEID                                  (SAMHNF4NODEID[6:0]),
                               .SAMHNF5NODEID                                  (SAMHNF5NODEID[6:0]),
                               .SAMHNF6NODEID                                  (SAMHNF6NODEID[6:0]),
                               .SAMHNF7NODEID                                  (SAMHNF7NODEID[6:0]),
                               .SAMHNFMODE                                     (SAMHNFMODE[2:0]),
                               .SAMHNI0NODEID                                  (SAMHNI0NODEID[6:0]),
                               .SAMHNI1NODEID                                  (SAMHNI1NODEID[6:0]),
                               .SAMMNBASE                                      (SAMMNBASE[43:24]),
                               .SAMMNNODEID                                    (SAMMNNODEID[6:0]),
                               .SCLKEN                                         (SCLKEN),
                               .SYSBARDISABLE                                  (SYSBARDISABLE),
                               .TXDATLCRDV                                     (TXDATLCRDV),
                               .TXLINKACTIVEACK                                (TXLINKACTIVEACK),
                               .TXREQLCRDV                                     (TXREQLCRDV),
                               .TXRSPLCRDV                                     (TXRSPLCRDV),
                               .WDATAS                                         (WDATAS[127:0]),
                               .WLASTS                                         (WLASTS),
                               .WSTRBS                                         (WSTRBS[15:0]),
                               .WVALIDS                                        (WVALIDS),
                               .ck_cpu0_l2_standbywfi                          (ck_cpu0_l2_standbywfi),
                               .ck_cpu0_l2_standbywfx                          (ck_cpu0_l2_standbywfx),
                               .ck_cpu1_l2_standbywfi                          (ck_cpu1_l2_standbywfi),
                               .ck_cpu1_l2_standbywfx                          (ck_cpu1_l2_standbywfx),
                               .ck_cpu2_l2_standbywfi                          (ck_cpu2_l2_standbywfi),
                               .ck_cpu2_l2_standbywfx                          (ck_cpu2_l2_standbywfx),
                               .ck_cpu3_l2_standbywfi                          (ck_cpu3_l2_standbywfi),
                               .ck_cpu3_l2_standbywfx                          (ck_cpu3_l2_standbywfx),
                               .ck_gclkfr                                      (ck_gclkfr),
                               .ck_gclkl2                                      (ck_gclkl2),
                               .ck_l2_ace_inactive                             (ck_l2_ace_inactive),
                               .ck_l2_acp_inactive                             (ck_l2_acp_inactive),
                               .ck_l2_sky_link_deactivate                      (ck_l2_sky_link_deactivate),
                               .ds_cpu0_l2_spr_addr                            (ds_cpu0_l2_spr_addr[8:0]),
                               .ds_cpu0_l2_spr_dw                              (ds_cpu0_l2_spr_dw),
                               .ds_cpu0_l2_spr_en                              (ds_cpu0_l2_spr_en),
                               .ds_cpu0_l2_spr_rd                              (ds_cpu0_l2_spr_rd),
                               .ds_cpu0_l2_spr_wr                              (ds_cpu0_l2_spr_wr),
                               .ds_cpu0_l2_spr_wr_data                         (ds_cpu0_l2_spr_wr_data[63:0]),
                               .ds_cpu1_l2_spr_addr                            (ds_cpu1_l2_spr_addr[8:0]),
                               .ds_cpu1_l2_spr_dw                              (ds_cpu1_l2_spr_dw),
                               .ds_cpu1_l2_spr_en                              (ds_cpu1_l2_spr_en),
                               .ds_cpu1_l2_spr_rd                              (ds_cpu1_l2_spr_rd),
                               .ds_cpu1_l2_spr_wr                              (ds_cpu1_l2_spr_wr),
                               .ds_cpu1_l2_spr_wr_data                         (ds_cpu1_l2_spr_wr_data[63:0]),
                               .ds_cpu2_l2_spr_addr                            (ds_cpu2_l2_spr_addr[8:0]),
                               .ds_cpu2_l2_spr_dw                              (ds_cpu2_l2_spr_dw),
                               .ds_cpu2_l2_spr_en                              (ds_cpu2_l2_spr_en),
                               .ds_cpu2_l2_spr_rd                              (ds_cpu2_l2_spr_rd),
                               .ds_cpu2_l2_spr_wr                              (ds_cpu2_l2_spr_wr),
                               .ds_cpu2_l2_spr_wr_data                         (ds_cpu2_l2_spr_wr_data[63:0]),
                               .ds_cpu3_l2_spr_addr                            (ds_cpu3_l2_spr_addr[8:0]),
                               .ds_cpu3_l2_spr_dw                              (ds_cpu3_l2_spr_dw),
                               .ds_cpu3_l2_spr_en                              (ds_cpu3_l2_spr_en),
                               .ds_cpu3_l2_spr_rd                              (ds_cpu3_l2_spr_rd),
                               .ds_cpu3_l2_spr_wr                              (ds_cpu3_l2_spr_wr),
                               .ds_cpu3_l2_spr_wr_data                         (ds_cpu3_l2_spr_wr_data[63:0]),
                               .ic_cpu0_l2_dsb_block                           (ic_cpu0_l2_dsb_block),
                               .ic_cpu0_spr_rd_data                            (ic_cpu0_spr_rd_data[63:0]),
                               .ic_cpu1_l2_dsb_block                           (ic_cpu1_l2_dsb_block),
                               .ic_cpu1_spr_rd_data                            (ic_cpu1_spr_rd_data[63:0]),
                               .ic_cpu2_l2_dsb_block                           (ic_cpu2_l2_dsb_block),
                               .ic_cpu2_spr_rd_data                            (ic_cpu2_spr_rd_data[63:0]),
                               .ic_cpu3_l2_dsb_block                           (ic_cpu3_l2_dsb_block),
                               .ic_cpu3_spr_rd_data                            (ic_cpu3_spr_rd_data[63:0]),
                               .ic_p_rdata                                     (ic_p_rdata[31:0]),
                               .ic_p_rdata_valid                               (ic_p_rdata_valid),
                               .ic_p_ready                                     (ic_p_ready),
                               .l2_cpu0_dsq_rd_byte_strb_q                     (l2_cpu0_dsq_rd_byte_strb_q[15:0]),
                               .l2_cpu0_dsq_rd_data_q                          (l2_cpu0_dsq_rd_data_q[129:0]),
                               .l2_cpu0_dt_pmu_evt_en                          (l2_cpu0_dt_pmu_evt_en),
                               .l2_cpu0_early_rd_reqe4_e5_q                    (l2_cpu0_early_rd_reqe4_e5_q),
                               .l2_cpu0_ic_addr_arb_set                        (l2_cpu0_ic_addr_arb_set[15:0]),
                               .l2_cpu0_ic_arb_fast                            (l2_cpu0_ic_arb_fast),
                               .l2_cpu0_ic_data_arb_set                        (l2_cpu0_ic_data_arb_set[31:0]),
                               .l2_cpu0_ic_elem_size_arb_set                   (l2_cpu0_ic_elem_size_arb_set[2:0]),
                               .l2_cpu0_ic_excl_arb_set                        (l2_cpu0_ic_excl_arb_set),
                               .l2_cpu0_ic_id_arb_set                          (l2_cpu0_ic_id_arb_set[2:0]),
                               .l2_cpu0_ic_ns_arb_set                          (l2_cpu0_ic_ns_arb_set),
                               .l2_cpu0_ic_write_arb_set                       (l2_cpu0_ic_write_arb_set),
                               .l2_cpu0_idle_wakeup_q                          (l2_cpu0_idle_wakeup_q),
                               .l2_cpu0_if_ccb_resp                            (l2_cpu0_if_ccb_resp),
                               .l2_cpu0_if_ccb_resp_id                         (l2_cpu0_if_ccb_resp_id[4:0]),
                               .l2_cpu0_if_sync_done_q                         (l2_cpu0_if_sync_done_q),
                               .l2_cpu0_lrq_haz_clr_id_dcd_q                   (l2_cpu0_lrq_haz_clr_id_dcd_q[5:0]),
                               .l2_cpu0_ls_ccb_data_wr                         (l2_cpu0_ls_ccb_data_wr),
                               .l2_cpu0_ls_ccb_resp                            (l2_cpu0_ls_ccb_resp),
                               .l2_cpu0_ls_ccb_resp_id                         (l2_cpu0_ls_ccb_resp_id[4:0]),
                               .l2_cpu0_ls_rd_haz_id_arb_q                     (l2_cpu0_ls_rd_haz_id_arb_q[3:0]),
                               .l2_cpu0_ls_rd_haz_vld_arb_q                    (l2_cpu0_ls_rd_haz_vld_arb_q),
                               .l2_cpu0_ls_wr_ccb_id_w2a                       (l2_cpu0_ls_wr_ccb_id_w2a[4:0]),
                               .l2_cpu0_ls_wr_data_w2a                         (l2_cpu0_ls_wr_data_w2a[127:0]),
                               .l2_cpu0_ls_wr_dirty_w2a                        (l2_cpu0_ls_wr_dirty_w2a),
                               .l2_cpu0_ls_wr_err_w2a                          (l2_cpu0_ls_wr_err_w2a),
                               .l2_cpu0_ls_wr_haz_id_arb_q                     (l2_cpu0_ls_wr_haz_id_arb_q[2:0]),
                               .l2_cpu0_ls_wr_haz_vld_arb_q                    (l2_cpu0_ls_wr_haz_vld_arb_q),
                               .l2_cpu0_ls_wr_last_w2a                         (l2_cpu0_ls_wr_last_w2a),
                               .l2_cpu0_ls_wr_req_w2a                          (l2_cpu0_ls_wr_req_w2a),
                               .l2_cpu0_ls_wr_type_w2a                         (l2_cpu0_ls_wr_type_w2a[2:0]),
                               .l2_cpu0_rd_aarch64_arb_set                     (l2_cpu0_rd_aarch64_arb_set),
                               .l2_cpu0_rd_addr_arb_set                        (l2_cpu0_rd_addr_arb_set[44:0]),
                               .l2_cpu0_rd_arb_fast                            (l2_cpu0_rd_arb_fast),
                               .l2_cpu0_rd_asid_arb_set                        (l2_cpu0_rd_asid_arb_set[15:8]),
                               .l2_cpu0_rd_bypass_arb_set                      (l2_cpu0_rd_bypass_arb_set),
                               .l2_cpu0_rd_bypass_bufid_e5                     (l2_cpu0_rd_bypass_bufid_e5[2:0]),
                               .l2_cpu0_rd_bypass_lrq_id_e5                    (l2_cpu0_rd_bypass_lrq_id_e5[2:0]),
                               .l2_cpu0_rd_bypass_req_can_e5                   (l2_cpu0_rd_bypass_req_can_e5),
                               .l2_cpu0_rd_bypass_way_e5                       (l2_cpu0_rd_bypass_way_e5),
                               .l2_cpu0_rd_cache_attr_arb_set                  (l2_cpu0_rd_cache_attr_arb_set[2:0]),
                               .l2_cpu0_rd_elem_size_arb_set                   (l2_cpu0_rd_elem_size_arb_set[2:0]),
                               .l2_cpu0_rd_excl_arb_set                        (l2_cpu0_rd_excl_arb_set),
                               .l2_cpu0_rd_id_arb_set                          (l2_cpu0_rd_id_arb_set[4:0]),
                               .l2_cpu0_rd_lrq_id_arb_set                      (l2_cpu0_rd_lrq_id_arb_set[2:0]),
                               .l2_cpu0_rd_page_attr_arb_set                   (l2_cpu0_rd_page_attr_arb_set[7:0]),
                               .l2_cpu0_rd_prfm_arb_set                        (l2_cpu0_rd_prfm_arb_set),
                               .l2_cpu0_rd_priv_arb_set                        (l2_cpu0_rd_priv_arb_set),
                               .l2_cpu0_rd_replayed_arb_set                    (l2_cpu0_rd_replayed_arb_set),
                               .l2_cpu0_rd_shared_arb_set                      (l2_cpu0_rd_shared_arb_set[1:0]),
                               .l2_cpu0_rd_type_arb_set                        (l2_cpu0_rd_type_arb_set[6:0]),
                               .l2_cpu0_rd_va48_arb_set                        (l2_cpu0_rd_va48_arb_set),
                               .l2_cpu0_rd_way_arb_set                         (l2_cpu0_rd_way_arb_set),
                               .l2_cpu0_tlb_sync_done_q                        (l2_cpu0_tlb_sync_done_q),
                               .l2_cpu0_tw_ccb_resp                            (l2_cpu0_tw_ccb_resp),
                               .l2_cpu0_tw_ccb_resp_id                         (l2_cpu0_tw_ccb_resp_id[4:0]),
                               .l2_cpu0_wr_1st_replayed_arb_set                (l2_cpu0_wr_1st_replayed_arb_set),
                               .l2_cpu0_wr_addr_arb_set                        (l2_cpu0_wr_addr_arb_set[44:0]),
                               .l2_cpu0_wr_arb_fast                            (l2_cpu0_wr_arb_fast),
                               .l2_cpu0_wr_cache_attr_arb_set                  (l2_cpu0_wr_cache_attr_arb_set[2:0]),
                               .l2_cpu0_wr_cl_id_arb_set                       (l2_cpu0_wr_cl_id_arb_set[11:0]),
                               .l2_cpu0_wr_clean_evict_arb_set                 (l2_cpu0_wr_clean_evict_arb_set),
                               .l2_cpu0_wr_data                                (l2_cpu0_wr_data[143:0]),
                               .l2_cpu0_wr_data_vld_x1_q                       (l2_cpu0_wr_data_vld_x1_q),
                               .l2_cpu0_wr_dirty_arb_set                       (l2_cpu0_wr_dirty_arb_set),
                               .l2_cpu0_wr_elem_size_arb_set                   (l2_cpu0_wr_elem_size_arb_set[2:0]),
                               .l2_cpu0_wr_err_arb_set                         (l2_cpu0_wr_err_arb_set),
                               .l2_cpu0_wr_evict_x1_q                          (l2_cpu0_wr_evict_x1_q),
                               .l2_cpu0_wr_id_arb_set                          (l2_cpu0_wr_id_arb_set[3:0]),
                               .l2_cpu0_wr_last_arb_set                        (l2_cpu0_wr_last_arb_set),
                               .l2_cpu0_wr_page_attr_arb_set                   (l2_cpu0_wr_page_attr_arb_set[7:0]),
                               .l2_cpu0_wr_partial_dw_arb_set                  (l2_cpu0_wr_partial_dw_arb_set[3:0]),
                               .l2_cpu0_wr_priv_arb_set                        (l2_cpu0_wr_priv_arb_set),
                               .l2_cpu0_wr_shared_arb_set                      (l2_cpu0_wr_shared_arb_set[1:0]),
                               .l2_cpu0_wr_type_arb_set                        (l2_cpu0_wr_type_arb_set[2:0]),
                               .l2_cpu0_wr_way_arb_set                         (l2_cpu0_wr_way_arb_set),
                               .l2_cpu0_wrq_almost_full                        (l2_cpu0_wrq_almost_full),
                               .l2_cpu0_wrq_haz_clr_id_dcd_q                   (l2_cpu0_wrq_haz_clr_id_dcd_q[15:0]),
                               .l2_cpu1_dsq_rd_byte_strb_q                     (l2_cpu1_dsq_rd_byte_strb_q[15:0]),
                               .l2_cpu1_dsq_rd_data_q                          (l2_cpu1_dsq_rd_data_q[129:0]),
                               .l2_cpu1_dt_pmu_evt_en                          (l2_cpu1_dt_pmu_evt_en),
                               .l2_cpu1_early_rd_reqe4_e5_q                    (l2_cpu1_early_rd_reqe4_e5_q),
                               .l2_cpu1_ic_addr_arb_set                        (l2_cpu1_ic_addr_arb_set[15:0]),
                               .l2_cpu1_ic_arb_fast                            (l2_cpu1_ic_arb_fast),
                               .l2_cpu1_ic_data_arb_set                        (l2_cpu1_ic_data_arb_set[31:0]),
                               .l2_cpu1_ic_elem_size_arb_set                   (l2_cpu1_ic_elem_size_arb_set[2:0]),
                               .l2_cpu1_ic_excl_arb_set                        (l2_cpu1_ic_excl_arb_set),
                               .l2_cpu1_ic_id_arb_set                          (l2_cpu1_ic_id_arb_set[2:0]),
                               .l2_cpu1_ic_ns_arb_set                          (l2_cpu1_ic_ns_arb_set),
                               .l2_cpu1_ic_write_arb_set                       (l2_cpu1_ic_write_arb_set),
                               .l2_cpu1_idle_wakeup_q                          (l2_cpu1_idle_wakeup_q),
                               .l2_cpu1_if_ccb_resp                            (l2_cpu1_if_ccb_resp),
                               .l2_cpu1_if_ccb_resp_id                         (l2_cpu1_if_ccb_resp_id[4:0]),
                               .l2_cpu1_if_sync_done_q                         (l2_cpu1_if_sync_done_q),
                               .l2_cpu1_lrq_haz_clr_id_dcd_q                   (l2_cpu1_lrq_haz_clr_id_dcd_q[5:0]),
                               .l2_cpu1_ls_ccb_data_wr                         (l2_cpu1_ls_ccb_data_wr),
                               .l2_cpu1_ls_ccb_resp                            (l2_cpu1_ls_ccb_resp),
                               .l2_cpu1_ls_ccb_resp_id                         (l2_cpu1_ls_ccb_resp_id[4:0]),
                               .l2_cpu1_ls_rd_haz_id_arb_q                     (l2_cpu1_ls_rd_haz_id_arb_q[3:0]),
                               .l2_cpu1_ls_rd_haz_vld_arb_q                    (l2_cpu1_ls_rd_haz_vld_arb_q),
                               .l2_cpu1_ls_wr_ccb_id_w2a                       (l2_cpu1_ls_wr_ccb_id_w2a[4:0]),
                               .l2_cpu1_ls_wr_data_w2a                         (l2_cpu1_ls_wr_data_w2a[127:0]),
                               .l2_cpu1_ls_wr_dirty_w2a                        (l2_cpu1_ls_wr_dirty_w2a),
                               .l2_cpu1_ls_wr_err_w2a                          (l2_cpu1_ls_wr_err_w2a),
                               .l2_cpu1_ls_wr_haz_id_arb_q                     (l2_cpu1_ls_wr_haz_id_arb_q[2:0]),
                               .l2_cpu1_ls_wr_haz_vld_arb_q                    (l2_cpu1_ls_wr_haz_vld_arb_q),
                               .l2_cpu1_ls_wr_last_w2a                         (l2_cpu1_ls_wr_last_w2a),
                               .l2_cpu1_ls_wr_req_w2a                          (l2_cpu1_ls_wr_req_w2a),
                               .l2_cpu1_ls_wr_type_w2a                         (l2_cpu1_ls_wr_type_w2a[2:0]),
                               .l2_cpu1_rd_aarch64_arb_set                     (l2_cpu1_rd_aarch64_arb_set),
                               .l2_cpu1_rd_addr_arb_set                        (l2_cpu1_rd_addr_arb_set[44:0]),
                               .l2_cpu1_rd_arb_fast                            (l2_cpu1_rd_arb_fast),
                               .l2_cpu1_rd_asid_arb_set                        (l2_cpu1_rd_asid_arb_set[15:8]),
                               .l2_cpu1_rd_bypass_arb_set                      (l2_cpu1_rd_bypass_arb_set),
                               .l2_cpu1_rd_bypass_bufid_e5                     (l2_cpu1_rd_bypass_bufid_e5[2:0]),
                               .l2_cpu1_rd_bypass_lrq_id_e5                    (l2_cpu1_rd_bypass_lrq_id_e5[2:0]),
                               .l2_cpu1_rd_bypass_req_can_e5                   (l2_cpu1_rd_bypass_req_can_e5),
                               .l2_cpu1_rd_bypass_way_e5                       (l2_cpu1_rd_bypass_way_e5),
                               .l2_cpu1_rd_cache_attr_arb_set                  (l2_cpu1_rd_cache_attr_arb_set[2:0]),
                               .l2_cpu1_rd_elem_size_arb_set                   (l2_cpu1_rd_elem_size_arb_set[2:0]),
                               .l2_cpu1_rd_excl_arb_set                        (l2_cpu1_rd_excl_arb_set),
                               .l2_cpu1_rd_id_arb_set                          (l2_cpu1_rd_id_arb_set[4:0]),
                               .l2_cpu1_rd_lrq_id_arb_set                      (l2_cpu1_rd_lrq_id_arb_set[2:0]),
                               .l2_cpu1_rd_page_attr_arb_set                   (l2_cpu1_rd_page_attr_arb_set[7:0]),
                               .l2_cpu1_rd_prfm_arb_set                        (l2_cpu1_rd_prfm_arb_set),
                               .l2_cpu1_rd_priv_arb_set                        (l2_cpu1_rd_priv_arb_set),
                               .l2_cpu1_rd_replayed_arb_set                    (l2_cpu1_rd_replayed_arb_set),
                               .l2_cpu1_rd_shared_arb_set                      (l2_cpu1_rd_shared_arb_set[1:0]),
                               .l2_cpu1_rd_type_arb_set                        (l2_cpu1_rd_type_arb_set[6:0]),
                               .l2_cpu1_rd_va48_arb_set                        (l2_cpu1_rd_va48_arb_set),
                               .l2_cpu1_rd_way_arb_set                         (l2_cpu1_rd_way_arb_set),
                               .l2_cpu1_tlb_sync_done_q                        (l2_cpu1_tlb_sync_done_q),
                               .l2_cpu1_tw_ccb_resp                            (l2_cpu1_tw_ccb_resp),
                               .l2_cpu1_tw_ccb_resp_id                         (l2_cpu1_tw_ccb_resp_id[4:0]),
                               .l2_cpu1_wr_1st_replayed_arb_set                (l2_cpu1_wr_1st_replayed_arb_set),
                               .l2_cpu1_wr_addr_arb_set                        (l2_cpu1_wr_addr_arb_set[44:0]),
                               .l2_cpu1_wr_arb_fast                            (l2_cpu1_wr_arb_fast),
                               .l2_cpu1_wr_cache_attr_arb_set                  (l2_cpu1_wr_cache_attr_arb_set[2:0]),
                               .l2_cpu1_wr_cl_id_arb_set                       (l2_cpu1_wr_cl_id_arb_set[11:0]),
                               .l2_cpu1_wr_clean_evict_arb_set                 (l2_cpu1_wr_clean_evict_arb_set),
                               .l2_cpu1_wr_data                                (l2_cpu1_wr_data[143:0]),
                               .l2_cpu1_wr_data_vld_x1_q                       (l2_cpu1_wr_data_vld_x1_q),
                               .l2_cpu1_wr_dirty_arb_set                       (l2_cpu1_wr_dirty_arb_set),
                               .l2_cpu1_wr_elem_size_arb_set                   (l2_cpu1_wr_elem_size_arb_set[2:0]),
                               .l2_cpu1_wr_err_arb_set                         (l2_cpu1_wr_err_arb_set),
                               .l2_cpu1_wr_evict_x1_q                          (l2_cpu1_wr_evict_x1_q),
                               .l2_cpu1_wr_id_arb_set                          (l2_cpu1_wr_id_arb_set[3:0]),
                               .l2_cpu1_wr_last_arb_set                        (l2_cpu1_wr_last_arb_set),
                               .l2_cpu1_wr_page_attr_arb_set                   (l2_cpu1_wr_page_attr_arb_set[7:0]),
                               .l2_cpu1_wr_partial_dw_arb_set                  (l2_cpu1_wr_partial_dw_arb_set[3:0]),
                               .l2_cpu1_wr_priv_arb_set                        (l2_cpu1_wr_priv_arb_set),
                               .l2_cpu1_wr_shared_arb_set                      (l2_cpu1_wr_shared_arb_set[1:0]),
                               .l2_cpu1_wr_type_arb_set                        (l2_cpu1_wr_type_arb_set[2:0]),
                               .l2_cpu1_wr_way_arb_set                         (l2_cpu1_wr_way_arb_set),
                               .l2_cpu1_wrq_almost_full                        (l2_cpu1_wrq_almost_full),
                               .l2_cpu1_wrq_haz_clr_id_dcd_q                   (l2_cpu1_wrq_haz_clr_id_dcd_q[15:0]),
                               .l2_cpu2_dsq_rd_byte_strb_q                     (l2_cpu2_dsq_rd_byte_strb_q[15:0]),
                               .l2_cpu2_dsq_rd_data_q                          (l2_cpu2_dsq_rd_data_q[129:0]),
                               .l2_cpu2_dt_pmu_evt_en                          (l2_cpu2_dt_pmu_evt_en),
                               .l2_cpu2_early_rd_reqe4_e5_q                    (l2_cpu2_early_rd_reqe4_e5_q),
                               .l2_cpu2_ic_addr_arb_set                        (l2_cpu2_ic_addr_arb_set[15:0]),
                               .l2_cpu2_ic_arb_fast                            (l2_cpu2_ic_arb_fast),
                               .l2_cpu2_ic_data_arb_set                        (l2_cpu2_ic_data_arb_set[31:0]),
                               .l2_cpu2_ic_elem_size_arb_set                   (l2_cpu2_ic_elem_size_arb_set[2:0]),
                               .l2_cpu2_ic_excl_arb_set                        (l2_cpu2_ic_excl_arb_set),
                               .l2_cpu2_ic_id_arb_set                          (l2_cpu2_ic_id_arb_set[2:0]),
                               .l2_cpu2_ic_ns_arb_set                          (l2_cpu2_ic_ns_arb_set),
                               .l2_cpu2_ic_write_arb_set                       (l2_cpu2_ic_write_arb_set),
                               .l2_cpu2_idle_wakeup_q                          (l2_cpu2_idle_wakeup_q),
                               .l2_cpu2_if_ccb_resp                            (l2_cpu2_if_ccb_resp),
                               .l2_cpu2_if_ccb_resp_id                         (l2_cpu2_if_ccb_resp_id[4:0]),
                               .l2_cpu2_if_sync_done_q                         (l2_cpu2_if_sync_done_q),
                               .l2_cpu2_lrq_haz_clr_id_dcd_q                   (l2_cpu2_lrq_haz_clr_id_dcd_q[5:0]),
                               .l2_cpu2_ls_ccb_data_wr                         (l2_cpu2_ls_ccb_data_wr),
                               .l2_cpu2_ls_ccb_resp                            (l2_cpu2_ls_ccb_resp),
                               .l2_cpu2_ls_ccb_resp_id                         (l2_cpu2_ls_ccb_resp_id[4:0]),
                               .l2_cpu2_ls_rd_haz_id_arb_q                     (l2_cpu2_ls_rd_haz_id_arb_q[3:0]),
                               .l2_cpu2_ls_rd_haz_vld_arb_q                    (l2_cpu2_ls_rd_haz_vld_arb_q),
                               .l2_cpu2_ls_wr_ccb_id_w2a                       (l2_cpu2_ls_wr_ccb_id_w2a[4:0]),
                               .l2_cpu2_ls_wr_data_w2a                         (l2_cpu2_ls_wr_data_w2a[127:0]),
                               .l2_cpu2_ls_wr_dirty_w2a                        (l2_cpu2_ls_wr_dirty_w2a),
                               .l2_cpu2_ls_wr_err_w2a                          (l2_cpu2_ls_wr_err_w2a),
                               .l2_cpu2_ls_wr_haz_id_arb_q                     (l2_cpu2_ls_wr_haz_id_arb_q[2:0]),
                               .l2_cpu2_ls_wr_haz_vld_arb_q                    (l2_cpu2_ls_wr_haz_vld_arb_q),
                               .l2_cpu2_ls_wr_last_w2a                         (l2_cpu2_ls_wr_last_w2a),
                               .l2_cpu2_ls_wr_req_w2a                          (l2_cpu2_ls_wr_req_w2a),
                               .l2_cpu2_ls_wr_type_w2a                         (l2_cpu2_ls_wr_type_w2a[2:0]),
                               .l2_cpu2_rd_aarch64_arb_set                     (l2_cpu2_rd_aarch64_arb_set),
                               .l2_cpu2_rd_addr_arb_set                        (l2_cpu2_rd_addr_arb_set[44:0]),
                               .l2_cpu2_rd_arb_fast                            (l2_cpu2_rd_arb_fast),
                               .l2_cpu2_rd_asid_arb_set                        (l2_cpu2_rd_asid_arb_set[15:8]),
                               .l2_cpu2_rd_bypass_arb_set                      (l2_cpu2_rd_bypass_arb_set),
                               .l2_cpu2_rd_bypass_bufid_e5                     (l2_cpu2_rd_bypass_bufid_e5[2:0]),
                               .l2_cpu2_rd_bypass_lrq_id_e5                    (l2_cpu2_rd_bypass_lrq_id_e5[2:0]),
                               .l2_cpu2_rd_bypass_req_can_e5                   (l2_cpu2_rd_bypass_req_can_e5),
                               .l2_cpu2_rd_bypass_way_e5                       (l2_cpu2_rd_bypass_way_e5),
                               .l2_cpu2_rd_cache_attr_arb_set                  (l2_cpu2_rd_cache_attr_arb_set[2:0]),
                               .l2_cpu2_rd_elem_size_arb_set                   (l2_cpu2_rd_elem_size_arb_set[2:0]),
                               .l2_cpu2_rd_excl_arb_set                        (l2_cpu2_rd_excl_arb_set),
                               .l2_cpu2_rd_id_arb_set                          (l2_cpu2_rd_id_arb_set[4:0]),
                               .l2_cpu2_rd_lrq_id_arb_set                      (l2_cpu2_rd_lrq_id_arb_set[2:0]),
                               .l2_cpu2_rd_page_attr_arb_set                   (l2_cpu2_rd_page_attr_arb_set[7:0]),
                               .l2_cpu2_rd_prfm_arb_set                        (l2_cpu2_rd_prfm_arb_set),
                               .l2_cpu2_rd_priv_arb_set                        (l2_cpu2_rd_priv_arb_set),
                               .l2_cpu2_rd_replayed_arb_set                    (l2_cpu2_rd_replayed_arb_set),
                               .l2_cpu2_rd_shared_arb_set                      (l2_cpu2_rd_shared_arb_set[1:0]),
                               .l2_cpu2_rd_type_arb_set                        (l2_cpu2_rd_type_arb_set[6:0]),
                               .l2_cpu2_rd_va48_arb_set                        (l2_cpu2_rd_va48_arb_set),
                               .l2_cpu2_rd_way_arb_set                         (l2_cpu2_rd_way_arb_set),
                               .l2_cpu2_tlb_sync_done_q                        (l2_cpu2_tlb_sync_done_q),
                               .l2_cpu2_tw_ccb_resp                            (l2_cpu2_tw_ccb_resp),
                               .l2_cpu2_tw_ccb_resp_id                         (l2_cpu2_tw_ccb_resp_id[4:0]),
                               .l2_cpu2_wr_1st_replayed_arb_set                (l2_cpu2_wr_1st_replayed_arb_set),
                               .l2_cpu2_wr_addr_arb_set                        (l2_cpu2_wr_addr_arb_set[44:0]),
                               .l2_cpu2_wr_arb_fast                            (l2_cpu2_wr_arb_fast),
                               .l2_cpu2_wr_cache_attr_arb_set                  (l2_cpu2_wr_cache_attr_arb_set[2:0]),
                               .l2_cpu2_wr_cl_id_arb_set                       (l2_cpu2_wr_cl_id_arb_set[11:0]),
                               .l2_cpu2_wr_clean_evict_arb_set                 (l2_cpu2_wr_clean_evict_arb_set),
                               .l2_cpu2_wr_data                                (l2_cpu2_wr_data[143:0]),
                               .l2_cpu2_wr_data_vld_x1_q                       (l2_cpu2_wr_data_vld_x1_q),
                               .l2_cpu2_wr_dirty_arb_set                       (l2_cpu2_wr_dirty_arb_set),
                               .l2_cpu2_wr_elem_size_arb_set                   (l2_cpu2_wr_elem_size_arb_set[2:0]),
                               .l2_cpu2_wr_err_arb_set                         (l2_cpu2_wr_err_arb_set),
                               .l2_cpu2_wr_evict_x1_q                          (l2_cpu2_wr_evict_x1_q),
                               .l2_cpu2_wr_id_arb_set                          (l2_cpu2_wr_id_arb_set[3:0]),
                               .l2_cpu2_wr_last_arb_set                        (l2_cpu2_wr_last_arb_set),
                               .l2_cpu2_wr_page_attr_arb_set                   (l2_cpu2_wr_page_attr_arb_set[7:0]),
                               .l2_cpu2_wr_partial_dw_arb_set                  (l2_cpu2_wr_partial_dw_arb_set[3:0]),
                               .l2_cpu2_wr_priv_arb_set                        (l2_cpu2_wr_priv_arb_set),
                               .l2_cpu2_wr_shared_arb_set                      (l2_cpu2_wr_shared_arb_set[1:0]),
                               .l2_cpu2_wr_type_arb_set                        (l2_cpu2_wr_type_arb_set[2:0]),
                               .l2_cpu2_wr_way_arb_set                         (l2_cpu2_wr_way_arb_set),
                               .l2_cpu2_wrq_almost_full                        (l2_cpu2_wrq_almost_full),
                               .l2_cpu2_wrq_haz_clr_id_dcd_q                   (l2_cpu2_wrq_haz_clr_id_dcd_q[15:0]),
                               .l2_cpu3_dsq_rd_byte_strb_q                     (l2_cpu3_dsq_rd_byte_strb_q[15:0]),
                               .l2_cpu3_dsq_rd_data_q                          (l2_cpu3_dsq_rd_data_q[129:0]),
                               .l2_cpu3_dt_pmu_evt_en                          (l2_cpu3_dt_pmu_evt_en),
                               .l2_cpu3_early_rd_reqe4_e5_q                    (l2_cpu3_early_rd_reqe4_e5_q),
                               .l2_cpu3_ic_addr_arb_set                        (l2_cpu3_ic_addr_arb_set[15:0]),
                               .l2_cpu3_ic_arb_fast                            (l2_cpu3_ic_arb_fast),
                               .l2_cpu3_ic_data_arb_set                        (l2_cpu3_ic_data_arb_set[31:0]),
                               .l2_cpu3_ic_elem_size_arb_set                   (l2_cpu3_ic_elem_size_arb_set[2:0]),
                               .l2_cpu3_ic_excl_arb_set                        (l2_cpu3_ic_excl_arb_set),
                               .l2_cpu3_ic_id_arb_set                          (l2_cpu3_ic_id_arb_set[2:0]),
                               .l2_cpu3_ic_ns_arb_set                          (l2_cpu3_ic_ns_arb_set),
                               .l2_cpu3_ic_write_arb_set                       (l2_cpu3_ic_write_arb_set),
                               .l2_cpu3_idle_wakeup_q                          (l2_cpu3_idle_wakeup_q),
                               .l2_cpu3_if_ccb_resp                            (l2_cpu3_if_ccb_resp),
                               .l2_cpu3_if_ccb_resp_id                         (l2_cpu3_if_ccb_resp_id[4:0]),
                               .l2_cpu3_if_sync_done_q                         (l2_cpu3_if_sync_done_q),
                               .l2_cpu3_lrq_haz_clr_id_dcd_q                   (l2_cpu3_lrq_haz_clr_id_dcd_q[5:0]),
                               .l2_cpu3_ls_ccb_data_wr                         (l2_cpu3_ls_ccb_data_wr),
                               .l2_cpu3_ls_ccb_resp                            (l2_cpu3_ls_ccb_resp),
                               .l2_cpu3_ls_ccb_resp_id                         (l2_cpu3_ls_ccb_resp_id[4:0]),
                               .l2_cpu3_ls_rd_haz_id_arb_q                     (l2_cpu3_ls_rd_haz_id_arb_q[3:0]),
                               .l2_cpu3_ls_rd_haz_vld_arb_q                    (l2_cpu3_ls_rd_haz_vld_arb_q),
                               .l2_cpu3_ls_wr_ccb_id_w2a                       (l2_cpu3_ls_wr_ccb_id_w2a[4:0]),
                               .l2_cpu3_ls_wr_data_w2a                         (l2_cpu3_ls_wr_data_w2a[127:0]),
                               .l2_cpu3_ls_wr_dirty_w2a                        (l2_cpu3_ls_wr_dirty_w2a),
                               .l2_cpu3_ls_wr_err_w2a                          (l2_cpu3_ls_wr_err_w2a),
                               .l2_cpu3_ls_wr_haz_id_arb_q                     (l2_cpu3_ls_wr_haz_id_arb_q[2:0]),
                               .l2_cpu3_ls_wr_haz_vld_arb_q                    (l2_cpu3_ls_wr_haz_vld_arb_q),
                               .l2_cpu3_ls_wr_last_w2a                         (l2_cpu3_ls_wr_last_w2a),
                               .l2_cpu3_ls_wr_req_w2a                          (l2_cpu3_ls_wr_req_w2a),
                               .l2_cpu3_ls_wr_type_w2a                         (l2_cpu3_ls_wr_type_w2a[2:0]),
                               .l2_cpu3_rd_aarch64_arb_set                     (l2_cpu3_rd_aarch64_arb_set),
                               .l2_cpu3_rd_addr_arb_set                        (l2_cpu3_rd_addr_arb_set[44:0]),
                               .l2_cpu3_rd_arb_fast                            (l2_cpu3_rd_arb_fast),
                               .l2_cpu3_rd_asid_arb_set                        (l2_cpu3_rd_asid_arb_set[15:8]),
                               .l2_cpu3_rd_bypass_arb_set                      (l2_cpu3_rd_bypass_arb_set),
                               .l2_cpu3_rd_bypass_bufid_e5                     (l2_cpu3_rd_bypass_bufid_e5[2:0]),
                               .l2_cpu3_rd_bypass_lrq_id_e5                    (l2_cpu3_rd_bypass_lrq_id_e5[2:0]),
                               .l2_cpu3_rd_bypass_req_can_e5                   (l2_cpu3_rd_bypass_req_can_e5),
                               .l2_cpu3_rd_bypass_way_e5                       (l2_cpu3_rd_bypass_way_e5),
                               .l2_cpu3_rd_cache_attr_arb_set                  (l2_cpu3_rd_cache_attr_arb_set[2:0]),
                               .l2_cpu3_rd_elem_size_arb_set                   (l2_cpu3_rd_elem_size_arb_set[2:0]),
                               .l2_cpu3_rd_excl_arb_set                        (l2_cpu3_rd_excl_arb_set),
                               .l2_cpu3_rd_id_arb_set                          (l2_cpu3_rd_id_arb_set[4:0]),
                               .l2_cpu3_rd_lrq_id_arb_set                      (l2_cpu3_rd_lrq_id_arb_set[2:0]),
                               .l2_cpu3_rd_page_attr_arb_set                   (l2_cpu3_rd_page_attr_arb_set[7:0]),
                               .l2_cpu3_rd_prfm_arb_set                        (l2_cpu3_rd_prfm_arb_set),
                               .l2_cpu3_rd_priv_arb_set                        (l2_cpu3_rd_priv_arb_set),
                               .l2_cpu3_rd_replayed_arb_set                    (l2_cpu3_rd_replayed_arb_set),
                               .l2_cpu3_rd_shared_arb_set                      (l2_cpu3_rd_shared_arb_set[1:0]),
                               .l2_cpu3_rd_type_arb_set                        (l2_cpu3_rd_type_arb_set[6:0]),
                               .l2_cpu3_rd_va48_arb_set                        (l2_cpu3_rd_va48_arb_set),
                               .l2_cpu3_rd_way_arb_set                         (l2_cpu3_rd_way_arb_set),
                               .l2_cpu3_tlb_sync_done_q                        (l2_cpu3_tlb_sync_done_q),
                               .l2_cpu3_tw_ccb_resp                            (l2_cpu3_tw_ccb_resp),
                               .l2_cpu3_tw_ccb_resp_id                         (l2_cpu3_tw_ccb_resp_id[4:0]),
                               .l2_cpu3_wr_1st_replayed_arb_set                (l2_cpu3_wr_1st_replayed_arb_set),
                               .l2_cpu3_wr_addr_arb_set                        (l2_cpu3_wr_addr_arb_set[44:0]),
                               .l2_cpu3_wr_arb_fast                            (l2_cpu3_wr_arb_fast),
                               .l2_cpu3_wr_cache_attr_arb_set                  (l2_cpu3_wr_cache_attr_arb_set[2:0]),
                               .l2_cpu3_wr_cl_id_arb_set                       (l2_cpu3_wr_cl_id_arb_set[11:0]),
                               .l2_cpu3_wr_clean_evict_arb_set                 (l2_cpu3_wr_clean_evict_arb_set),
                               .l2_cpu3_wr_data                                (l2_cpu3_wr_data[143:0]),
                               .l2_cpu3_wr_data_vld_x1_q                       (l2_cpu3_wr_data_vld_x1_q),
                               .l2_cpu3_wr_dirty_arb_set                       (l2_cpu3_wr_dirty_arb_set),
                               .l2_cpu3_wr_elem_size_arb_set                   (l2_cpu3_wr_elem_size_arb_set[2:0]),
                               .l2_cpu3_wr_err_arb_set                         (l2_cpu3_wr_err_arb_set),
                               .l2_cpu3_wr_evict_x1_q                          (l2_cpu3_wr_evict_x1_q),
                               .l2_cpu3_wr_id_arb_set                          (l2_cpu3_wr_id_arb_set[3:0]),
                               .l2_cpu3_wr_last_arb_set                        (l2_cpu3_wr_last_arb_set),
                               .l2_cpu3_wr_page_attr_arb_set                   (l2_cpu3_wr_page_attr_arb_set[7:0]),
                               .l2_cpu3_wr_partial_dw_arb_set                  (l2_cpu3_wr_partial_dw_arb_set[3:0]),
                               .l2_cpu3_wr_priv_arb_set                        (l2_cpu3_wr_priv_arb_set),
                               .l2_cpu3_wr_shared_arb_set                      (l2_cpu3_wr_shared_arb_set[1:0]),
                               .l2_cpu3_wr_type_arb_set                        (l2_cpu3_wr_type_arb_set[2:0]),
                               .l2_cpu3_wr_way_arb_set                         (l2_cpu3_wr_way_arb_set),
                               .l2_cpu3_wrq_almost_full                        (l2_cpu3_wrq_almost_full),
                               .l2_cpu3_wrq_haz_clr_id_dcd_q                   (l2_cpu3_wrq_haz_clr_id_dcd_q[15:0]),
                               .l2_mbist2_tbnk0_addr_b1                        (l2_mbist2_tbnk0_addr_b1[16:0]),
                               .l2_mbist2_tbnk0_all_b1                         (l2_mbist2_tbnk0_all_b1),
                               .l2_mbist2_tbnk0_array_b1                       (l2_mbist2_tbnk0_array_b1[2:0]),
                               .l2_mbist2_tbnk0_be_b1                          (l2_mbist2_tbnk0_be_b1[17:0]),
                               .l2_mbist2_tbnk0_en_b1                          (l2_mbist2_tbnk0_en_b1),
                               .l2_mbist2_tbnk0_indata_b1                      (l2_mbist2_tbnk0_indata_b1[143:0]),
                               .l2_mbist2_tbnk0_outdata_b3                     (l2_mbist2_tbnk0_outdata_b3[143:0]),
                               .l2_mbist2_tbnk0_sel_b1                         (l2_mbist2_tbnk0_sel_b1),
                               .l2_mbist2_tbnk0_snp0_sel_b1                    (l2_mbist2_tbnk0_snp0_sel_b1),
                               .l2_mbist2_tbnk0_snp1_sel_b1                    (l2_mbist2_tbnk0_snp1_sel_b1),
                               .l2_mbist2_tbnk0_snp2_sel_b1                    (l2_mbist2_tbnk0_snp2_sel_b1),
                               .l2_mbist2_tbnk0_snp3_sel_b1                    (l2_mbist2_tbnk0_snp3_sel_b1),
                               .l2_mbist2_tbnk0_wr_en_b1                       (l2_mbist2_tbnk0_wr_en_b1),
                               .l2_mbist2_tbnk1_addr_b1                        (l2_mbist2_tbnk1_addr_b1[16:0]),
                               .l2_mbist2_tbnk1_all_b1                         (l2_mbist2_tbnk1_all_b1),
                               .l2_mbist2_tbnk1_array_b1                       (l2_mbist2_tbnk1_array_b1[2:0]),
                               .l2_mbist2_tbnk1_be_b1                          (l2_mbist2_tbnk1_be_b1[17:0]),
                               .l2_mbist2_tbnk1_en_b1                          (l2_mbist2_tbnk1_en_b1),
                               .l2_mbist2_tbnk1_indata_b1                      (l2_mbist2_tbnk1_indata_b1[143:0]),
                               .l2_mbist2_tbnk1_outdata_b3                     (l2_mbist2_tbnk1_outdata_b3[143:0]),
                               .l2_mbist2_tbnk1_sel_b1                         (l2_mbist2_tbnk1_sel_b1),
                               .l2_mbist2_tbnk1_snp0_sel_b1                    (l2_mbist2_tbnk1_snp0_sel_b1),
                               .l2_mbist2_tbnk1_snp1_sel_b1                    (l2_mbist2_tbnk1_snp1_sel_b1),
                               .l2_mbist2_tbnk1_snp2_sel_b1                    (l2_mbist2_tbnk1_snp2_sel_b1),
                               .l2_mbist2_tbnk1_snp3_sel_b1                    (l2_mbist2_tbnk1_snp3_sel_b1),
                               .l2_mbist2_tbnk1_wr_en_b1                       (l2_mbist2_tbnk1_wr_en_b1),
                               .l2_tbnk0_addr44_l3_q                           (l2_tbnk0_addr44_l3_q),
                               .l2_tbnk0_addr_l6                               (l2_tbnk0_addr_l6[5:2]),
                               .l2_tbnk0_all_tag_incl_active_l3                (l2_tbnk0_all_tag_incl_active_l3),
                               .l2_tbnk0_cmo_setway_l2_inv_incl_l4             (l2_tbnk0_cmo_setway_l2_inv_incl_l4),
                               .l2_tbnk0_cpu0_ccb_xfer_l4_dly2                 (l2_tbnk0_cpu0_ccb_xfer_l4_dly2),
                               .l2_tbnk0_cpu0_hit_l4                           (l2_tbnk0_cpu0_hit_l4),
                               .l2_tbnk0_cpu0_l2_inv_l4_dly2                   (l2_tbnk0_cpu0_l2_inv_l4_dly2),
                               .l2_tbnk0_cpu0_l2hit_e_l4                       (l2_tbnk0_cpu0_l2hit_e_l4),
                               .l2_tbnk0_cpu0_l2hit_s_l4                       (l2_tbnk0_cpu0_l2hit_s_l4),
                               .l2_tbnk0_cpu0_rd_access_l4_dly                 (l2_tbnk0_cpu0_rd_access_l4_dly),
                               .l2_tbnk0_cpu0_self_evict_l4_dly_q              (l2_tbnk0_cpu0_self_evict_l4_dly_q),
                               .l2_tbnk0_cpu0_single_ecc_err_l7_q              (l2_tbnk0_cpu0_single_ecc_err_l7_q),
                               .l2_tbnk0_cpu0_snp_tag_ecc_rmw_rd_l0            (l2_tbnk0_cpu0_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk0_cpu0_snp_tag_ecc_rmw_wr_l0            (l2_tbnk0_cpu0_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk0_cpu0_snp_tag_wr_evict_early_l4_dly    (l2_tbnk0_cpu0_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk0_cpu0_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk0_cpu0_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk0_cpu0_vld_nxt_l5                       (l2_tbnk0_cpu0_vld_nxt_l5),
                               .l2_tbnk0_cpu0_wr_access_l4_dly                 (l2_tbnk0_cpu0_wr_access_l4_dly),
                               .l2_tbnk0_cpu1_ccb_xfer_l4_dly2                 (l2_tbnk0_cpu1_ccb_xfer_l4_dly2),
                               .l2_tbnk0_cpu1_hit_l4                           (l2_tbnk0_cpu1_hit_l4),
                               .l2_tbnk0_cpu1_l2_inv_l4_dly2                   (l2_tbnk0_cpu1_l2_inv_l4_dly2),
                               .l2_tbnk0_cpu1_l2hit_e_l4                       (l2_tbnk0_cpu1_l2hit_e_l4),
                               .l2_tbnk0_cpu1_l2hit_s_l4                       (l2_tbnk0_cpu1_l2hit_s_l4),
                               .l2_tbnk0_cpu1_rd_access_l4_dly                 (l2_tbnk0_cpu1_rd_access_l4_dly),
                               .l2_tbnk0_cpu1_self_evict_l4_dly_q              (l2_tbnk0_cpu1_self_evict_l4_dly_q),
                               .l2_tbnk0_cpu1_single_ecc_err_l7_q              (l2_tbnk0_cpu1_single_ecc_err_l7_q),
                               .l2_tbnk0_cpu1_snp_tag_ecc_rmw_rd_l0            (l2_tbnk0_cpu1_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk0_cpu1_snp_tag_ecc_rmw_wr_l0            (l2_tbnk0_cpu1_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk0_cpu1_snp_tag_wr_evict_early_l4_dly    (l2_tbnk0_cpu1_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk0_cpu1_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk0_cpu1_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk0_cpu1_vld_nxt_l5                       (l2_tbnk0_cpu1_vld_nxt_l5),
                               .l2_tbnk0_cpu1_wr_access_l4_dly                 (l2_tbnk0_cpu1_wr_access_l4_dly),
                               .l2_tbnk0_cpu2_ccb_xfer_l4_dly2                 (l2_tbnk0_cpu2_ccb_xfer_l4_dly2),
                               .l2_tbnk0_cpu2_hit_l4                           (l2_tbnk0_cpu2_hit_l4),
                               .l2_tbnk0_cpu2_l2_inv_l4_dly2                   (l2_tbnk0_cpu2_l2_inv_l4_dly2),
                               .l2_tbnk0_cpu2_l2hit_e_l4                       (l2_tbnk0_cpu2_l2hit_e_l4),
                               .l2_tbnk0_cpu2_l2hit_s_l4                       (l2_tbnk0_cpu2_l2hit_s_l4),
                               .l2_tbnk0_cpu2_rd_access_l4_dly                 (l2_tbnk0_cpu2_rd_access_l4_dly),
                               .l2_tbnk0_cpu2_self_evict_l4_dly_q              (l2_tbnk0_cpu2_self_evict_l4_dly_q),
                               .l2_tbnk0_cpu2_single_ecc_err_l7_q              (l2_tbnk0_cpu2_single_ecc_err_l7_q),
                               .l2_tbnk0_cpu2_snp_tag_ecc_rmw_rd_l0            (l2_tbnk0_cpu2_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk0_cpu2_snp_tag_ecc_rmw_wr_l0            (l2_tbnk0_cpu2_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk0_cpu2_snp_tag_wr_evict_early_l4_dly    (l2_tbnk0_cpu2_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk0_cpu2_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk0_cpu2_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk0_cpu2_vld_nxt_l5                       (l2_tbnk0_cpu2_vld_nxt_l5),
                               .l2_tbnk0_cpu2_wr_access_l4_dly                 (l2_tbnk0_cpu2_wr_access_l4_dly),
                               .l2_tbnk0_cpu3_ccb_xfer_l4_dly2                 (l2_tbnk0_cpu3_ccb_xfer_l4_dly2),
                               .l2_tbnk0_cpu3_hit_l4                           (l2_tbnk0_cpu3_hit_l4),
                               .l2_tbnk0_cpu3_l2_inv_l4_dly2                   (l2_tbnk0_cpu3_l2_inv_l4_dly2),
                               .l2_tbnk0_cpu3_l2hit_e_l4                       (l2_tbnk0_cpu3_l2hit_e_l4),
                               .l2_tbnk0_cpu3_l2hit_s_l4                       (l2_tbnk0_cpu3_l2hit_s_l4),
                               .l2_tbnk0_cpu3_rd_access_l4_dly                 (l2_tbnk0_cpu3_rd_access_l4_dly),
                               .l2_tbnk0_cpu3_self_evict_l4_dly_q              (l2_tbnk0_cpu3_self_evict_l4_dly_q),
                               .l2_tbnk0_cpu3_single_ecc_err_l7_q              (l2_tbnk0_cpu3_single_ecc_err_l7_q),
                               .l2_tbnk0_cpu3_snp_tag_ecc_rmw_rd_l0            (l2_tbnk0_cpu3_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk0_cpu3_snp_tag_ecc_rmw_wr_l0            (l2_tbnk0_cpu3_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk0_cpu3_snp_tag_wr_evict_early_l4_dly    (l2_tbnk0_cpu3_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk0_cpu3_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk0_cpu3_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk0_cpu3_vld_nxt_l5                       (l2_tbnk0_cpu3_vld_nxt_l5),
                               .l2_tbnk0_cpu3_wr_access_l4_dly                 (l2_tbnk0_cpu3_wr_access_l4_dly),
                               .l2_tbnk0_cpu_rvalid_init_nxt_l5                (l2_tbnk0_cpu_rvalid_init_nxt_l5[3:0]),
                               .l2_tbnk0_cpu_rvalid_nxt_l5                     (l2_tbnk0_cpu_rvalid_nxt_l5[3:0]),
                               .l2_tbnk0_cpu_snp_hit_e_l4_q                    (l2_tbnk0_cpu_snp_hit_e_l4_q[3:0]),
                               .l2_tbnk0_crit_qw_nxt_l5                        (l2_tbnk0_crit_qw_nxt_l5),
                               .l2_tbnk0_data_corrected_l7_q                   (l2_tbnk0_data_corrected_l7_q[143:0]),
                               .l2_tbnk0_data_l6                               (l2_tbnk0_data_l6[127:0]),
                               .l2_tbnk0_dbg_ram_acc_l5a                       (l2_tbnk0_dbg_ram_acc_l5a),
                               .l2_tbnk0_dbg_ram_acc_unit_nxt                  (l2_tbnk0_dbg_ram_acc_unit_nxt[2:0]),
                               .l2_tbnk0_dbg_ram_id_nxt_l5                     (l2_tbnk0_dbg_ram_id_nxt_l5[7:0]),
                               .l2_tbnk0_dirty_l3_q                            (l2_tbnk0_dirty_l3_q),
                               .l2_tbnk0_double_ecc_err_l7_q                   (l2_tbnk0_double_ecc_err_l7_q),
                               .l2_tbnk0_early_rvalid_l4_q                     (l2_tbnk0_early_rvalid_l4_q),
                               .l2_tbnk0_ecc_fixup_blk_arb                     (l2_tbnk0_ecc_fixup_blk_arb),
                               .l2_tbnk0_ecc_fixup_inprog_dly_q                (l2_tbnk0_ecc_fixup_inprog_dly_q),
                               .l2_tbnk0_ecc_rmw_snp_tag_rd_l3_q               (l2_tbnk0_ecc_rmw_snp_tag_rd_l3_q),
                               .l2_tbnk0_ecc_syndrome_reg_q                    (l2_tbnk0_ecc_syndrome_reg_q[31:0]),
                               .l2_tbnk0_evict_special_hazard_l3_q             (l2_tbnk0_evict_special_hazard_l3_q),
                               .l2_tbnk0_evict_special_hazard_rwvic_l3_q       (l2_tbnk0_evict_special_hazard_rwvic_l3_q),
                               .l2_tbnk0_excl_l4_q                             (l2_tbnk0_excl_l4_q),
                               .l2_tbnk0_feq_addr_upd                          (l2_tbnk0_feq_addr_upd[44:6]),
                               .l2_tbnk0_feq_clr_l4                            (l2_tbnk0_feq_clr_l4),
                               .l2_tbnk0_full_miss_l4_q                        (l2_tbnk0_full_miss_l4_q),
                               .l2_tbnk0_hit_l4                                (l2_tbnk0_hit_l4),
                               .l2_tbnk0_hit_l7_q                              (l2_tbnk0_hit_l7_q),
                               .l2_tbnk0_hit_way_l4_q                          (l2_tbnk0_hit_way_l4_q[3:0]),
                               .l2_tbnk0_id_l6_q                               (l2_tbnk0_id_l6_q[9:0]),
                               .l2_tbnk0_id_nxt_l5                             (l2_tbnk0_id_nxt_l5[9:0]),
                               .l2_tbnk0_idle                                  (l2_tbnk0_idle),
                               .l2_tbnk0_l2hit_e_l4                            (l2_tbnk0_l2hit_e_l4),
                               .l2_tbnk0_l2hit_s_l4                            (l2_tbnk0_l2hit_s_l4),
                               .l2_tbnk0_l2v_s_q                               (l2_tbnk0_l2v_s_q),
                               .l2_tbnk0_l2v_vld_q                             (l2_tbnk0_l2v_vld_q),
                               .l2_tbnk0_last_qw_l6_q                          (l2_tbnk0_last_qw_l6_q),
                               .l2_tbnk0_last_qw_nxt_l5                        (l2_tbnk0_last_qw_nxt_l5),
                               .l2_tbnk0_lock_l4                               (l2_tbnk0_lock_l4[2:0]),
                               .l2_tbnk0_merrsr_data                           (l2_tbnk0_merrsr_data[32:0]),
                               .l2_tbnk0_pf_cnt_dec_l4_dly                     (l2_tbnk0_pf_cnt_dec_l4_dly),
                               .l2_tbnk0_pf_req_sel_for_fwd_l4                 (l2_tbnk0_pf_req_sel_for_fwd_l4),
                               .l2_tbnk0_prfm_nxt_l5                           (l2_tbnk0_prfm_nxt_l5),
                               .l2_tbnk0_prot_l4_q                             (l2_tbnk0_prot_l4_q[3:0]),
                               .l2_tbnk0_qw_cnt_l3_q                           (l2_tbnk0_qw_cnt_l3_q[1:0]),
                               .l2_tbnk0_raw_hit_l4_q                          (l2_tbnk0_raw_hit_l4_q),
                               .l2_tbnk0_rbufid_nxt_l5                         (l2_tbnk0_rbufid_nxt_l5[2:0]),
                               .l2_tbnk0_rd_en_nxt_l5                          (l2_tbnk0_rd_en_nxt_l5),
                               .l2_tbnk0_rwvic_axi_read_err_l3_q               (l2_tbnk0_rwvic_axi_read_err_l3_q),
                               .l2_tbnk0_rwvic_ccb_dirty_l6_q                  (l2_tbnk0_rwvic_ccb_dirty_l6_q),
                               .l2_tbnk0_rwvic_ccb_ls_xfer_l3_q                (l2_tbnk0_rwvic_ccb_ls_xfer_l3_q),
                               .l2_tbnk0_rwvic_ccb_ls_xfer_l6_q                (l2_tbnk0_rwvic_ccb_ls_xfer_l6_q),
                               .l2_tbnk0_rwvic_cmo_inv_l7_q                    (l2_tbnk0_rwvic_cmo_inv_l7_q),
                               .l2_tbnk0_rwvic_cmo_l7_q                        (l2_tbnk0_rwvic_cmo_l7_q),
                               .l2_tbnk0_rwvic_cmo_pou_l6_q                    (l2_tbnk0_rwvic_cmo_pou_l6_q),
                               .l2_tbnk0_rwvic_cmo_setway_ls_l6_q              (l2_tbnk0_rwvic_cmo_setway_ls_l6_q),
                               .l2_tbnk0_rwvic_ddi_l6_q                        (l2_tbnk0_rwvic_ddi_l6_q),
                               .l2_tbnk0_rwvic_l2hit_e_l3_q                    (l2_tbnk0_rwvic_l2hit_e_l3_q),
                               .l2_tbnk0_rwvic_l2hit_e_l7_q                    (l2_tbnk0_rwvic_l2hit_e_l7_q),
                               .l2_tbnk0_rwvic_l2v_dirty_l7_q                  (l2_tbnk0_rwvic_l2v_dirty_l7_q),
                               .l2_tbnk0_rwvic_l2v_page_attr_l7_q              (l2_tbnk0_rwvic_l2v_page_attr_l7_q[3:0]),
                               .l2_tbnk0_rwvic_l2v_vld_l6_q                    (l2_tbnk0_rwvic_l2v_vld_l6_q),
                               .l2_tbnk0_rwvic_non_snp_fail_hazchk_l3          (l2_tbnk0_rwvic_non_snp_fail_hazchk_l3),
                               .l2_tbnk0_rwvic_owner_l7_q                      (l2_tbnk0_rwvic_owner_l7_q[2:0]),
                               .l2_tbnk0_rwvic_rd_type_l6_q                    (l2_tbnk0_rwvic_rd_type_l6_q),
                               .l2_tbnk0_rwvic_snp_l3_q                        (l2_tbnk0_rwvic_snp_l3_q),
                               .l2_tbnk0_rwvic_snp_l6_q                        (l2_tbnk0_rwvic_snp_l6_q),
                               .l2_tbnk0_rwvic_tag_wr_l0                       (l2_tbnk0_rwvic_tag_wr_l0),
                               .l2_tbnk0_rwvic_wa_l6_q                         (l2_tbnk0_rwvic_wa_l6_q),
                               .l2_tbnk0_size_l4_q                             (l2_tbnk0_size_l4_q[2:0]),
                               .l2_tbnk0_snp_hit_e_l4_q                        (l2_tbnk0_snp_hit_e_l4_q),
                               .l2_tbnk0_snp_hit_s_l4_q                        (l2_tbnk0_snp_hit_s_l4_q),
                               .l2_tbnk0_snp_tag_wr_l2_hit_addr_l1             (l2_tbnk0_snp_tag_wr_l2_hit_addr_l1[44:7]),
                               .l2_tbnk0_snp_tag_wr_l2_hit_state_l1            (l2_tbnk0_snp_tag_wr_l2_hit_state_l1[1:0]),
                               .l2_tbnk0_snp_tag_wr_l2_hit_way_l1              (l2_tbnk0_snp_tag_wr_l2_hit_way_l1),
                               .l2_tbnk0_special_evict_hazard_l3               (l2_tbnk0_special_evict_hazard_l3),
                               .l2_tbnk0_special_hazard_l3_q                   (l2_tbnk0_special_hazard_l3_q),
                               .l2_tbnk0_tag_ecc_dbl_rmw_wr_l1                 (l2_tbnk0_tag_ecc_dbl_rmw_wr_l1),
                               .l2_tbnk0_tag_ecc_err_cpu0_l4                   (l2_tbnk0_tag_ecc_err_cpu0_l4),
                               .l2_tbnk0_tag_ecc_err_cpu1_l4                   (l2_tbnk0_tag_ecc_err_cpu1_l4),
                               .l2_tbnk0_tag_ecc_err_cpu2_l4                   (l2_tbnk0_tag_ecc_err_cpu2_l4),
                               .l2_tbnk0_tag_ecc_err_cpu3_l4                   (l2_tbnk0_tag_ecc_err_cpu3_l4),
                               .l2_tbnk0_tag_ecc_err_l4                        (l2_tbnk0_tag_ecc_err_l4),
                               .l2_tbnk0_ulen_l4_q                             (l2_tbnk0_ulen_l4_q[1:0]),
                               .l2_tbnk0_vld_init_l6_q                         (l2_tbnk0_vld_init_l6_q),
                               .l2_tbnk0_vld_l6_q                              (l2_tbnk0_vld_l6_q),
                               .l2_tbnk0_way_l4_q                              (l2_tbnk0_way_l4_q),
                               .l2_tbnk0_way_nxt_l3a                           (l2_tbnk0_way_nxt_l3a),
                               .l2_tbnk0_wr_data_l3                            (l2_tbnk0_wr_data_l3[143:0]),
                               .l2_tbnk0_wr_data_l4_en                         (l2_tbnk0_wr_data_l4_en),
                               .l2_tbnk0_wr_non_crit_id_l4_q                   (l2_tbnk0_wr_non_crit_id_l4_q[11:0]),
                               .l2_tbnk1_addr44_l3_q                           (l2_tbnk1_addr44_l3_q),
                               .l2_tbnk1_addr_l6                               (l2_tbnk1_addr_l6[5:2]),
                               .l2_tbnk1_all_tag_incl_active_l3                (l2_tbnk1_all_tag_incl_active_l3),
                               .l2_tbnk1_cmo_setway_l2_inv_incl_l4             (l2_tbnk1_cmo_setway_l2_inv_incl_l4),
                               .l2_tbnk1_cpu0_ccb_xfer_l4_dly2                 (l2_tbnk1_cpu0_ccb_xfer_l4_dly2),
                               .l2_tbnk1_cpu0_hit_l4                           (l2_tbnk1_cpu0_hit_l4),
                               .l2_tbnk1_cpu0_l2_inv_l4_dly2                   (l2_tbnk1_cpu0_l2_inv_l4_dly2),
                               .l2_tbnk1_cpu0_l2hit_e_l4                       (l2_tbnk1_cpu0_l2hit_e_l4),
                               .l2_tbnk1_cpu0_l2hit_s_l4                       (l2_tbnk1_cpu0_l2hit_s_l4),
                               .l2_tbnk1_cpu0_rd_access_l4_dly                 (l2_tbnk1_cpu0_rd_access_l4_dly),
                               .l2_tbnk1_cpu0_self_evict_l4_dly_q              (l2_tbnk1_cpu0_self_evict_l4_dly_q),
                               .l2_tbnk1_cpu0_single_ecc_err_l7_q              (l2_tbnk1_cpu0_single_ecc_err_l7_q),
                               .l2_tbnk1_cpu0_snp_tag_ecc_rmw_rd_l0            (l2_tbnk1_cpu0_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk1_cpu0_snp_tag_ecc_rmw_wr_l0            (l2_tbnk1_cpu0_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk1_cpu0_snp_tag_wr_evict_early_l4_dly    (l2_tbnk1_cpu0_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk1_cpu0_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk1_cpu0_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk1_cpu0_vld_nxt_l5                       (l2_tbnk1_cpu0_vld_nxt_l5),
                               .l2_tbnk1_cpu0_wr_access_l4_dly                 (l2_tbnk1_cpu0_wr_access_l4_dly),
                               .l2_tbnk1_cpu1_ccb_xfer_l4_dly2                 (l2_tbnk1_cpu1_ccb_xfer_l4_dly2),
                               .l2_tbnk1_cpu1_hit_l4                           (l2_tbnk1_cpu1_hit_l4),
                               .l2_tbnk1_cpu1_l2_inv_l4_dly2                   (l2_tbnk1_cpu1_l2_inv_l4_dly2),
                               .l2_tbnk1_cpu1_l2hit_e_l4                       (l2_tbnk1_cpu1_l2hit_e_l4),
                               .l2_tbnk1_cpu1_l2hit_s_l4                       (l2_tbnk1_cpu1_l2hit_s_l4),
                               .l2_tbnk1_cpu1_rd_access_l4_dly                 (l2_tbnk1_cpu1_rd_access_l4_dly),
                               .l2_tbnk1_cpu1_self_evict_l4_dly_q              (l2_tbnk1_cpu1_self_evict_l4_dly_q),
                               .l2_tbnk1_cpu1_single_ecc_err_l7_q              (l2_tbnk1_cpu1_single_ecc_err_l7_q),
                               .l2_tbnk1_cpu1_snp_tag_ecc_rmw_rd_l0            (l2_tbnk1_cpu1_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk1_cpu1_snp_tag_ecc_rmw_wr_l0            (l2_tbnk1_cpu1_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk1_cpu1_snp_tag_wr_evict_early_l4_dly    (l2_tbnk1_cpu1_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk1_cpu1_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk1_cpu1_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk1_cpu1_vld_nxt_l5                       (l2_tbnk1_cpu1_vld_nxt_l5),
                               .l2_tbnk1_cpu1_wr_access_l4_dly                 (l2_tbnk1_cpu1_wr_access_l4_dly),
                               .l2_tbnk1_cpu2_ccb_xfer_l4_dly2                 (l2_tbnk1_cpu2_ccb_xfer_l4_dly2),
                               .l2_tbnk1_cpu2_hit_l4                           (l2_tbnk1_cpu2_hit_l4),
                               .l2_tbnk1_cpu2_l2_inv_l4_dly2                   (l2_tbnk1_cpu2_l2_inv_l4_dly2),
                               .l2_tbnk1_cpu2_l2hit_e_l4                       (l2_tbnk1_cpu2_l2hit_e_l4),
                               .l2_tbnk1_cpu2_l2hit_s_l4                       (l2_tbnk1_cpu2_l2hit_s_l4),
                               .l2_tbnk1_cpu2_rd_access_l4_dly                 (l2_tbnk1_cpu2_rd_access_l4_dly),
                               .l2_tbnk1_cpu2_self_evict_l4_dly_q              (l2_tbnk1_cpu2_self_evict_l4_dly_q),
                               .l2_tbnk1_cpu2_single_ecc_err_l7_q              (l2_tbnk1_cpu2_single_ecc_err_l7_q),
                               .l2_tbnk1_cpu2_snp_tag_ecc_rmw_rd_l0            (l2_tbnk1_cpu2_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk1_cpu2_snp_tag_ecc_rmw_wr_l0            (l2_tbnk1_cpu2_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk1_cpu2_snp_tag_wr_evict_early_l4_dly    (l2_tbnk1_cpu2_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk1_cpu2_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk1_cpu2_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk1_cpu2_vld_nxt_l5                       (l2_tbnk1_cpu2_vld_nxt_l5),
                               .l2_tbnk1_cpu2_wr_access_l4_dly                 (l2_tbnk1_cpu2_wr_access_l4_dly),
                               .l2_tbnk1_cpu3_ccb_xfer_l4_dly2                 (l2_tbnk1_cpu3_ccb_xfer_l4_dly2),
                               .l2_tbnk1_cpu3_hit_l4                           (l2_tbnk1_cpu3_hit_l4),
                               .l2_tbnk1_cpu3_l2_inv_l4_dly2                   (l2_tbnk1_cpu3_l2_inv_l4_dly2),
                               .l2_tbnk1_cpu3_l2hit_e_l4                       (l2_tbnk1_cpu3_l2hit_e_l4),
                               .l2_tbnk1_cpu3_l2hit_s_l4                       (l2_tbnk1_cpu3_l2hit_s_l4),
                               .l2_tbnk1_cpu3_rd_access_l4_dly                 (l2_tbnk1_cpu3_rd_access_l4_dly),
                               .l2_tbnk1_cpu3_self_evict_l4_dly_q              (l2_tbnk1_cpu3_self_evict_l4_dly_q),
                               .l2_tbnk1_cpu3_single_ecc_err_l7_q              (l2_tbnk1_cpu3_single_ecc_err_l7_q),
                               .l2_tbnk1_cpu3_snp_tag_ecc_rmw_rd_l0            (l2_tbnk1_cpu3_snp_tag_ecc_rmw_rd_l0[1:0]),
                               .l2_tbnk1_cpu3_snp_tag_ecc_rmw_wr_l0            (l2_tbnk1_cpu3_snp_tag_ecc_rmw_wr_l0[1:0]),
                               .l2_tbnk1_cpu3_snp_tag_wr_evict_early_l4_dly    (l2_tbnk1_cpu3_snp_tag_wr_evict_early_l4_dly),
                               .l2_tbnk1_cpu3_snp_tag_wr_l2_hit_early_l4_dly   (l2_tbnk1_cpu3_snp_tag_wr_l2_hit_early_l4_dly),
                               .l2_tbnk1_cpu3_vld_nxt_l5                       (l2_tbnk1_cpu3_vld_nxt_l5),
                               .l2_tbnk1_cpu3_wr_access_l4_dly                 (l2_tbnk1_cpu3_wr_access_l4_dly),
                               .l2_tbnk1_cpu_rvalid_init_nxt_l5                (l2_tbnk1_cpu_rvalid_init_nxt_l5[3:0]),
                               .l2_tbnk1_cpu_rvalid_nxt_l5                     (l2_tbnk1_cpu_rvalid_nxt_l5[3:0]),
                               .l2_tbnk1_cpu_snp_hit_e_l4_q                    (l2_tbnk1_cpu_snp_hit_e_l4_q[3:0]),
                               .l2_tbnk1_crit_qw_nxt_l5                        (l2_tbnk1_crit_qw_nxt_l5),
                               .l2_tbnk1_data_corrected_l7_q                   (l2_tbnk1_data_corrected_l7_q[143:0]),
                               .l2_tbnk1_data_l6                               (l2_tbnk1_data_l6[127:0]),
                               .l2_tbnk1_dbg_ram_acc_l5a                       (l2_tbnk1_dbg_ram_acc_l5a),
                               .l2_tbnk1_dbg_ram_acc_unit_nxt                  (l2_tbnk1_dbg_ram_acc_unit_nxt[2:0]),
                               .l2_tbnk1_dbg_ram_id_nxt_l5                     (l2_tbnk1_dbg_ram_id_nxt_l5[7:0]),
                               .l2_tbnk1_dirty_l3_q                            (l2_tbnk1_dirty_l3_q),
                               .l2_tbnk1_double_ecc_err_l7_q                   (l2_tbnk1_double_ecc_err_l7_q),
                               .l2_tbnk1_early_rvalid_l4_q                     (l2_tbnk1_early_rvalid_l4_q),
                               .l2_tbnk1_ecc_fixup_blk_arb                     (l2_tbnk1_ecc_fixup_blk_arb),
                               .l2_tbnk1_ecc_fixup_inprog_dly_q                (l2_tbnk1_ecc_fixup_inprog_dly_q),
                               .l2_tbnk1_ecc_rmw_snp_tag_rd_l3_q               (l2_tbnk1_ecc_rmw_snp_tag_rd_l3_q),
                               .l2_tbnk1_ecc_syndrome_reg_q                    (l2_tbnk1_ecc_syndrome_reg_q[31:0]),
                               .l2_tbnk1_evict_special_hazard_l3_q             (l2_tbnk1_evict_special_hazard_l3_q),
                               .l2_tbnk1_evict_special_hazard_rwvic_l3_q       (l2_tbnk1_evict_special_hazard_rwvic_l3_q),
                               .l2_tbnk1_excl_l4_q                             (l2_tbnk1_excl_l4_q),
                               .l2_tbnk1_feq_addr_upd                          (l2_tbnk1_feq_addr_upd[44:6]),
                               .l2_tbnk1_feq_clr_l4                            (l2_tbnk1_feq_clr_l4),
                               .l2_tbnk1_full_miss_l4_q                        (l2_tbnk1_full_miss_l4_q),
                               .l2_tbnk1_hit_l4                                (l2_tbnk1_hit_l4),
                               .l2_tbnk1_hit_l7_q                              (l2_tbnk1_hit_l7_q),
                               .l2_tbnk1_hit_way_l4_q                          (l2_tbnk1_hit_way_l4_q[3:0]),
                               .l2_tbnk1_id_l6_q                               (l2_tbnk1_id_l6_q[9:0]),
                               .l2_tbnk1_id_nxt_l5                             (l2_tbnk1_id_nxt_l5[9:0]),
                               .l2_tbnk1_idle                                  (l2_tbnk1_idle),
                               .l2_tbnk1_l2hit_e_l4                            (l2_tbnk1_l2hit_e_l4),
                               .l2_tbnk1_l2hit_s_l4                            (l2_tbnk1_l2hit_s_l4),
                               .l2_tbnk1_l2v_s_q                               (l2_tbnk1_l2v_s_q),
                               .l2_tbnk1_l2v_vld_q                             (l2_tbnk1_l2v_vld_q),
                               .l2_tbnk1_last_qw_l6_q                          (l2_tbnk1_last_qw_l6_q),
                               .l2_tbnk1_last_qw_nxt_l5                        (l2_tbnk1_last_qw_nxt_l5),
                               .l2_tbnk1_lock_l4                               (l2_tbnk1_lock_l4[2:0]),
                               .l2_tbnk1_merrsr_data                           (l2_tbnk1_merrsr_data[32:0]),
                               .l2_tbnk1_pf_cnt_dec_l4_dly                     (l2_tbnk1_pf_cnt_dec_l4_dly),
                               .l2_tbnk1_pf_req_sel_for_fwd_l4                 (l2_tbnk1_pf_req_sel_for_fwd_l4),
                               .l2_tbnk1_prfm_nxt_l5                           (l2_tbnk1_prfm_nxt_l5),
                               .l2_tbnk1_prot_l4_q                             (l2_tbnk1_prot_l4_q[3:0]),
                               .l2_tbnk1_qw_cnt_l3_q                           (l2_tbnk1_qw_cnt_l3_q[1:0]),
                               .l2_tbnk1_raw_hit_l4_q                          (l2_tbnk1_raw_hit_l4_q),
                               .l2_tbnk1_rbufid_nxt_l5                         (l2_tbnk1_rbufid_nxt_l5[2:0]),
                               .l2_tbnk1_rd_en_nxt_l5                          (l2_tbnk1_rd_en_nxt_l5),
                               .l2_tbnk1_rwvic_axi_read_err_l3_q               (l2_tbnk1_rwvic_axi_read_err_l3_q),
                               .l2_tbnk1_rwvic_ccb_dirty_l6_q                  (l2_tbnk1_rwvic_ccb_dirty_l6_q),
                               .l2_tbnk1_rwvic_ccb_ls_xfer_l3_q                (l2_tbnk1_rwvic_ccb_ls_xfer_l3_q),
                               .l2_tbnk1_rwvic_ccb_ls_xfer_l6_q                (l2_tbnk1_rwvic_ccb_ls_xfer_l6_q),
                               .l2_tbnk1_rwvic_cmo_inv_l7_q                    (l2_tbnk1_rwvic_cmo_inv_l7_q),
                               .l2_tbnk1_rwvic_cmo_l7_q                        (l2_tbnk1_rwvic_cmo_l7_q),
                               .l2_tbnk1_rwvic_cmo_pou_l6_q                    (l2_tbnk1_rwvic_cmo_pou_l6_q),
                               .l2_tbnk1_rwvic_cmo_setway_ls_l6_q              (l2_tbnk1_rwvic_cmo_setway_ls_l6_q),
                               .l2_tbnk1_rwvic_ddi_l6_q                        (l2_tbnk1_rwvic_ddi_l6_q),
                               .l2_tbnk1_rwvic_l2hit_e_l3_q                    (l2_tbnk1_rwvic_l2hit_e_l3_q),
                               .l2_tbnk1_rwvic_l2hit_e_l7_q                    (l2_tbnk1_rwvic_l2hit_e_l7_q),
                               .l2_tbnk1_rwvic_l2v_dirty_l7_q                  (l2_tbnk1_rwvic_l2v_dirty_l7_q),
                               .l2_tbnk1_rwvic_l2v_page_attr_l7_q              (l2_tbnk1_rwvic_l2v_page_attr_l7_q[3:0]),
                               .l2_tbnk1_rwvic_l2v_vld_l6_q                    (l2_tbnk1_rwvic_l2v_vld_l6_q),
                               .l2_tbnk1_rwvic_non_snp_fail_hazchk_l3          (l2_tbnk1_rwvic_non_snp_fail_hazchk_l3),
                               .l2_tbnk1_rwvic_owner_l7_q                      (l2_tbnk1_rwvic_owner_l7_q[2:0]),
                               .l2_tbnk1_rwvic_rd_type_l6_q                    (l2_tbnk1_rwvic_rd_type_l6_q),
                               .l2_tbnk1_rwvic_snp_l3_q                        (l2_tbnk1_rwvic_snp_l3_q),
                               .l2_tbnk1_rwvic_snp_l6_q                        (l2_tbnk1_rwvic_snp_l6_q),
                               .l2_tbnk1_rwvic_tag_wr_l0                       (l2_tbnk1_rwvic_tag_wr_l0),
                               .l2_tbnk1_rwvic_wa_l6_q                         (l2_tbnk1_rwvic_wa_l6_q),
                               .l2_tbnk1_size_l4_q                             (l2_tbnk1_size_l4_q[2:0]),
                               .l2_tbnk1_snp_hit_e_l4_q                        (l2_tbnk1_snp_hit_e_l4_q),
                               .l2_tbnk1_snp_hit_s_l4_q                        (l2_tbnk1_snp_hit_s_l4_q),
                               .l2_tbnk1_snp_tag_wr_l2_hit_addr_l1             (l2_tbnk1_snp_tag_wr_l2_hit_addr_l1[44:7]),
                               .l2_tbnk1_snp_tag_wr_l2_hit_state_l1            (l2_tbnk1_snp_tag_wr_l2_hit_state_l1[1:0]),
                               .l2_tbnk1_snp_tag_wr_l2_hit_way_l1              (l2_tbnk1_snp_tag_wr_l2_hit_way_l1),
                               .l2_tbnk1_special_evict_hazard_l3               (l2_tbnk1_special_evict_hazard_l3),
                               .l2_tbnk1_special_hazard_l3_q                   (l2_tbnk1_special_hazard_l3_q),
                               .l2_tbnk1_tag_ecc_dbl_rmw_wr_l1                 (l2_tbnk1_tag_ecc_dbl_rmw_wr_l1),
                               .l2_tbnk1_tag_ecc_err_cpu0_l4                   (l2_tbnk1_tag_ecc_err_cpu0_l4),
                               .l2_tbnk1_tag_ecc_err_cpu1_l4                   (l2_tbnk1_tag_ecc_err_cpu1_l4),
                               .l2_tbnk1_tag_ecc_err_cpu2_l4                   (l2_tbnk1_tag_ecc_err_cpu2_l4),
                               .l2_tbnk1_tag_ecc_err_cpu3_l4                   (l2_tbnk1_tag_ecc_err_cpu3_l4),
                               .l2_tbnk1_tag_ecc_err_l4                        (l2_tbnk1_tag_ecc_err_l4),
                               .l2_tbnk1_ulen_l4_q                             (l2_tbnk1_ulen_l4_q[1:0]),
                               .l2_tbnk1_vld_init_l6_q                         (l2_tbnk1_vld_init_l6_q),
                               .l2_tbnk1_vld_l6_q                              (l2_tbnk1_vld_l6_q),
                               .l2_tbnk1_way_l4_q                              (l2_tbnk1_way_l4_q),
                               .l2_tbnk1_way_nxt_l3a                           (l2_tbnk1_way_nxt_l3a),
                               .l2_tbnk1_wr_data_l3                            (l2_tbnk1_wr_data_l3[143:0]),
                               .l2_tbnk1_wr_data_l4_en                         (l2_tbnk1_wr_data_l4_en),
                               .l2_tbnk1_wr_non_crit_id_l4_q                   (l2_tbnk1_wr_non_crit_id_l4_q[11:0]),
                               .nL2RESET                                       (nL2RESET),
                               .nMBISTRESET                                    (nMBISTRESET),
                               .tm_cntpct_q                                    (tm_cntpct_q[8:0]),
                               .tm_cpu0_spr_rd_data                            (tm_cpu0_spr_rd_data[63:0]),
                               .tm_cpu1_spr_rd_data                            (tm_cpu1_spr_rd_data[63:0]),
                               .tm_cpu2_spr_rd_data                            (tm_cpu2_spr_rd_data[63:0]),
                               .tm_cpu3_spr_rd_data                            (tm_cpu3_spr_rd_data[63:0]),
                               .tm_tval_cpu0_spr_rd_data                       (tm_tval_cpu0_spr_rd_data[63:0]),
                               .tm_tval_cpu1_spr_rd_data                       (tm_tval_cpu1_spr_rd_data[63:0]),
                               .tm_tval_cpu2_spr_rd_data                       (tm_tval_cpu2_spr_rd_data[63:0]),
                               .tm_tval_cpu3_spr_rd_data                       (tm_tval_cpu3_spr_rd_data[63:0])
                             );    // ul2_logic

    maia_l2_tbnk ul2_tbnk0( // outputs
                            .l2_mbist2_addr_b1                           (l2_mbist2_tbnk0_addr_b1[16:0]),
                            .l2_mbist2_array_b1                          (l2_mbist2_tbnk0_array_b1[2:0]),
                            .l2_mbist2_be_b1                             (l2_mbist2_tbnk0_be_b1[17:0]),
                            .l2_mbist2_en_b1                             (l2_mbist2_tbnk0_en_b1),
                            .l2_mbist2_indata_b1                         (l2_mbist2_tbnk0_indata_b1[143:0]),
                            .l2_mbist2_tbnk_all_b1                       (l2_mbist2_tbnk0_all_b1),
                            .l2_mbist2_tbnk_outdata_b3                   (l2_mbist2_tbnk0_outdata_b3[143:0]),
                            .l2_mbist2_tbnk_sel_b1                       (l2_mbist2_tbnk0_sel_b1),
                            .l2_mbist2_tbnk_snp0_sel_b1                  (l2_mbist2_tbnk0_snp0_sel_b1),
                            .l2_mbist2_tbnk_snp1_sel_b1                  (l2_mbist2_tbnk0_snp1_sel_b1),
                            .l2_mbist2_tbnk_snp2_sel_b1                  (l2_mbist2_tbnk0_snp2_sel_b1),
                            .l2_mbist2_tbnk_snp3_sel_b1                  (l2_mbist2_tbnk0_snp3_sel_b1),
                            .l2_mbist2_wr_en_b1                          (l2_mbist2_tbnk0_wr_en_b1),
                            .l2_tbnk_addr44_l3_q                         (l2_tbnk0_addr44_l3_q),
                            .l2_tbnk_addr_l6                             (l2_tbnk0_addr_l6[5:2]),
                            .l2_tbnk_all_tag_incl_active_l3              (l2_tbnk0_all_tag_incl_active_l3),
                            .l2_tbnk_cmo_setway_l2_inv_incl_l4           (l2_tbnk0_cmo_setway_l2_inv_incl_l4),
                            .l2_tbnk_cpu0_ccb_xfer_l4_dly2               (l2_tbnk0_cpu0_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu0_hit_l4                         (l2_tbnk0_cpu0_hit_l4),
                            .l2_tbnk_cpu0_l2_inv_l4_dly2                 (l2_tbnk0_cpu0_l2_inv_l4_dly2),
                            .l2_tbnk_cpu0_l2hit_e_l4                     (l2_tbnk0_cpu0_l2hit_e_l4),
                            .l2_tbnk_cpu0_l2hit_s_l4                     (l2_tbnk0_cpu0_l2hit_s_l4),
                            .l2_tbnk_cpu0_rd_access_l4_dly               (l2_tbnk0_cpu0_rd_access_l4_dly),
                            .l2_tbnk_cpu0_self_evict_l4_dly_q            (l2_tbnk0_cpu0_self_evict_l4_dly_q),
                            .l2_tbnk_cpu0_single_ecc_err_l7_q            (l2_tbnk0_cpu0_single_ecc_err_l7_q),
                            .l2_tbnk_cpu0_snp_tag_ecc_rmw_rd_l0          (l2_tbnk0_cpu0_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu0_snp_tag_ecc_rmw_wr_l0          (l2_tbnk0_cpu0_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu0_snp_tag_wr_evict_early_l4_dly  (l2_tbnk0_cpu0_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu0_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk0_cpu0_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu0_vld_nxt_l5                     (l2_tbnk0_cpu0_vld_nxt_l5),
                            .l2_tbnk_cpu0_wr_access_l4_dly               (l2_tbnk0_cpu0_wr_access_l4_dly),
                            .l2_tbnk_cpu1_ccb_xfer_l4_dly2               (l2_tbnk0_cpu1_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu1_hit_l4                         (l2_tbnk0_cpu1_hit_l4),
                            .l2_tbnk_cpu1_l2_inv_l4_dly2                 (l2_tbnk0_cpu1_l2_inv_l4_dly2),
                            .l2_tbnk_cpu1_l2hit_e_l4                     (l2_tbnk0_cpu1_l2hit_e_l4),
                            .l2_tbnk_cpu1_l2hit_s_l4                     (l2_tbnk0_cpu1_l2hit_s_l4),
                            .l2_tbnk_cpu1_rd_access_l4_dly               (l2_tbnk0_cpu1_rd_access_l4_dly),
                            .l2_tbnk_cpu1_self_evict_l4_dly_q            (l2_tbnk0_cpu1_self_evict_l4_dly_q),
                            .l2_tbnk_cpu1_single_ecc_err_l7_q            (l2_tbnk0_cpu1_single_ecc_err_l7_q),
                            .l2_tbnk_cpu1_snp_tag_ecc_rmw_rd_l0          (l2_tbnk0_cpu1_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu1_snp_tag_ecc_rmw_wr_l0          (l2_tbnk0_cpu1_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu1_snp_tag_wr_evict_early_l4_dly  (l2_tbnk0_cpu1_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu1_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk0_cpu1_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu1_vld_nxt_l5                     (l2_tbnk0_cpu1_vld_nxt_l5),
                            .l2_tbnk_cpu1_wr_access_l4_dly               (l2_tbnk0_cpu1_wr_access_l4_dly),
                            .l2_tbnk_cpu2_ccb_xfer_l4_dly2               (l2_tbnk0_cpu2_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu2_hit_l4                         (l2_tbnk0_cpu2_hit_l4),
                            .l2_tbnk_cpu2_l2_inv_l4_dly2                 (l2_tbnk0_cpu2_l2_inv_l4_dly2),
                            .l2_tbnk_cpu2_l2hit_e_l4                     (l2_tbnk0_cpu2_l2hit_e_l4),
                            .l2_tbnk_cpu2_l2hit_s_l4                     (l2_tbnk0_cpu2_l2hit_s_l4),
                            .l2_tbnk_cpu2_rd_access_l4_dly               (l2_tbnk0_cpu2_rd_access_l4_dly),
                            .l2_tbnk_cpu2_self_evict_l4_dly_q            (l2_tbnk0_cpu2_self_evict_l4_dly_q),
                            .l2_tbnk_cpu2_single_ecc_err_l7_q            (l2_tbnk0_cpu2_single_ecc_err_l7_q),
                            .l2_tbnk_cpu2_snp_tag_ecc_rmw_rd_l0          (l2_tbnk0_cpu2_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu2_snp_tag_ecc_rmw_wr_l0          (l2_tbnk0_cpu2_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu2_snp_tag_wr_evict_early_l4_dly  (l2_tbnk0_cpu2_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu2_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk0_cpu2_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu2_vld_nxt_l5                     (l2_tbnk0_cpu2_vld_nxt_l5),
                            .l2_tbnk_cpu2_wr_access_l4_dly               (l2_tbnk0_cpu2_wr_access_l4_dly),
                            .l2_tbnk_cpu3_ccb_xfer_l4_dly2               (l2_tbnk0_cpu3_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu3_hit_l4                         (l2_tbnk0_cpu3_hit_l4),
                            .l2_tbnk_cpu3_l2_inv_l4_dly2                 (l2_tbnk0_cpu3_l2_inv_l4_dly2),
                            .l2_tbnk_cpu3_l2hit_e_l4                     (l2_tbnk0_cpu3_l2hit_e_l4),
                            .l2_tbnk_cpu3_l2hit_s_l4                     (l2_tbnk0_cpu3_l2hit_s_l4),
                            .l2_tbnk_cpu3_rd_access_l4_dly               (l2_tbnk0_cpu3_rd_access_l4_dly),
                            .l2_tbnk_cpu3_self_evict_l4_dly_q            (l2_tbnk0_cpu3_self_evict_l4_dly_q),
                            .l2_tbnk_cpu3_single_ecc_err_l7_q            (l2_tbnk0_cpu3_single_ecc_err_l7_q),
                            .l2_tbnk_cpu3_snp_tag_ecc_rmw_rd_l0          (l2_tbnk0_cpu3_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu3_snp_tag_ecc_rmw_wr_l0          (l2_tbnk0_cpu3_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu3_snp_tag_wr_evict_early_l4_dly  (l2_tbnk0_cpu3_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu3_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk0_cpu3_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu3_vld_nxt_l5                     (l2_tbnk0_cpu3_vld_nxt_l5),
                            .l2_tbnk_cpu3_wr_access_l4_dly               (l2_tbnk0_cpu3_wr_access_l4_dly),
                            .l2_tbnk_cpu_rvalid_init_nxt_l5              (l2_tbnk0_cpu_rvalid_init_nxt_l5[3:0]),
                            .l2_tbnk_cpu_rvalid_nxt_l5                   (l2_tbnk0_cpu_rvalid_nxt_l5[3:0]),
                            .l2_tbnk_cpu_snp_hit_e_l4_q                  (l2_tbnk0_cpu_snp_hit_e_l4_q[3:0]),
                            .l2_tbnk_crit_qw_nxt_l5                      (l2_tbnk0_crit_qw_nxt_l5),
                            .l2_tbnk_data_corrected_l7_q                 (l2_tbnk0_data_corrected_l7_q[143:0]),
                            .l2_tbnk_data_l6                             (l2_tbnk0_data_l6[127:0]),
                            .l2_tbnk_dbg_ram_acc_l5a                     (l2_tbnk0_dbg_ram_acc_l5a),
                            .l2_tbnk_dbg_ram_acc_unit_nxt                (l2_tbnk0_dbg_ram_acc_unit_nxt[2:0]),
                            .l2_tbnk_dbg_ram_id_nxt_l5                   (l2_tbnk0_dbg_ram_id_nxt_l5[7:0]),
                            .l2_tbnk_dirty_l3_q                          (l2_tbnk0_dirty_l3_q),
                            .l2_tbnk_double_ecc_err_l7_q                 (l2_tbnk0_double_ecc_err_l7_q),
                            .l2_tbnk_early_rvalid_l4_q                   (l2_tbnk0_early_rvalid_l4_q),
                            .l2_tbnk_ecc_fixup_blk_arb                   (l2_tbnk0_ecc_fixup_blk_arb),
                            .l2_tbnk_ecc_fixup_inprog_dly_q              (l2_tbnk0_ecc_fixup_inprog_dly_q),
                            .l2_tbnk_ecc_rmw_snp_tag_rd_l3_q             (l2_tbnk0_ecc_rmw_snp_tag_rd_l3_q),
                            .l2_tbnk_ecc_syndrome_reg_q                  (l2_tbnk0_ecc_syndrome_reg_q[31:0]),
                            .l2_tbnk_evict_special_hazard_l3_q           (l2_tbnk0_evict_special_hazard_l3_q),
                            .l2_tbnk_evict_special_hazard_rwvic_l3_q     (l2_tbnk0_evict_special_hazard_rwvic_l3_q),
                            .l2_tbnk_excl_l4_q                           (l2_tbnk0_excl_l4_q),
                            .l2_tbnk_feq_addr_upd                        (l2_tbnk0_feq_addr_upd[44:6]),
                            .l2_tbnk_feq_clr_l4                          (l2_tbnk0_feq_clr_l4),
                            .l2_tbnk_full_miss_l4_q                      (l2_tbnk0_full_miss_l4_q),
                            .l2_tbnk_hit_l4                              (l2_tbnk0_hit_l4),
                            .l2_tbnk_hit_l7_q                            (l2_tbnk0_hit_l7_q),
                            .l2_tbnk_hit_way_l4_q                        (l2_tbnk0_hit_way_l4_q[3:0]),
                            .l2_tbnk_id_l6_q                             (l2_tbnk0_id_l6_q[9:0]),
                            .l2_tbnk_id_nxt_l5                           (l2_tbnk0_id_nxt_l5[9:0]),
                            .l2_tbnk_idle                                (l2_tbnk0_idle),
                            .l2_tbnk_l2hit_e_l4                          (l2_tbnk0_l2hit_e_l4),
                            .l2_tbnk_l2hit_s_l4                          (l2_tbnk0_l2hit_s_l4),
                            .l2_tbnk_l2v_s_q                             (l2_tbnk0_l2v_s_q),
                            .l2_tbnk_l2v_vld_q                           (l2_tbnk0_l2v_vld_q),
                            .l2_tbnk_last_qw_l6_q                        (l2_tbnk0_last_qw_l6_q),
                            .l2_tbnk_last_qw_nxt_l5                      (l2_tbnk0_last_qw_nxt_l5),
                            .l2_tbnk_lock_l4                             (l2_tbnk0_lock_l4[2:0]),
                            .l2_tbnk_merrsr_data                         (l2_tbnk0_merrsr_data[32:0]),
                            .l2_tbnk_pf_cnt_dec_l4_dly                   (l2_tbnk0_pf_cnt_dec_l4_dly),
                            .l2_tbnk_pf_req_sel_for_fwd_l4               (l2_tbnk0_pf_req_sel_for_fwd_l4),
                            .l2_tbnk_prfm_nxt_l5                         (l2_tbnk0_prfm_nxt_l5),
                            .l2_tbnk_prot_l4_q                           (l2_tbnk0_prot_l4_q[3:0]),
                            .l2_tbnk_qw_cnt_l3_q                         (l2_tbnk0_qw_cnt_l3_q[1:0]),
                            .l2_tbnk_raw_hit_l4_q                        (l2_tbnk0_raw_hit_l4_q),
                            .l2_tbnk_rbufid_nxt_l5                       (l2_tbnk0_rbufid_nxt_l5[2:0]),
                            .l2_tbnk_rd_en_nxt_l5                        (l2_tbnk0_rd_en_nxt_l5),
                            .l2_tbnk_rwvic_axi_read_err_l3_q             (l2_tbnk0_rwvic_axi_read_err_l3_q),
                            .l2_tbnk_rwvic_ccb_dirty_l6_q                (l2_tbnk0_rwvic_ccb_dirty_l6_q),
                            .l2_tbnk_rwvic_ccb_ls_xfer_l3_q              (l2_tbnk0_rwvic_ccb_ls_xfer_l3_q),
                            .l2_tbnk_rwvic_ccb_ls_xfer_l6_q              (l2_tbnk0_rwvic_ccb_ls_xfer_l6_q),
                            .l2_tbnk_rwvic_cmo_inv_l7_q                  (l2_tbnk0_rwvic_cmo_inv_l7_q),
                            .l2_tbnk_rwvic_cmo_l7_q                      (l2_tbnk0_rwvic_cmo_l7_q),
                            .l2_tbnk_rwvic_cmo_pou_l6_q                  (l2_tbnk0_rwvic_cmo_pou_l6_q),
                            .l2_tbnk_rwvic_cmo_setway_ls_l6_q            (l2_tbnk0_rwvic_cmo_setway_ls_l6_q),
                            .l2_tbnk_rwvic_ddi_l6_q                      (l2_tbnk0_rwvic_ddi_l6_q),
                            .l2_tbnk_rwvic_l2hit_e_l3_q                  (l2_tbnk0_rwvic_l2hit_e_l3_q),
                            .l2_tbnk_rwvic_l2hit_e_l7_q                  (l2_tbnk0_rwvic_l2hit_e_l7_q),
                            .l2_tbnk_rwvic_l2v_dirty_l7_q                (l2_tbnk0_rwvic_l2v_dirty_l7_q),
                            .l2_tbnk_rwvic_l2v_page_attr_l7_q            (l2_tbnk0_rwvic_l2v_page_attr_l7_q[3:0]),
                            .l2_tbnk_rwvic_l2v_vld_l6_q                  (l2_tbnk0_rwvic_l2v_vld_l6_q),
                            .l2_tbnk_rwvic_non_snp_fail_hazchk_l3        (l2_tbnk0_rwvic_non_snp_fail_hazchk_l3),
                            .l2_tbnk_rwvic_owner_l7_q                    (l2_tbnk0_rwvic_owner_l7_q[2:0]),
                            .l2_tbnk_rwvic_rd_type_l6_q                  (l2_tbnk0_rwvic_rd_type_l6_q),
                            .l2_tbnk_rwvic_snp_l3_q                      (l2_tbnk0_rwvic_snp_l3_q),
                            .l2_tbnk_rwvic_snp_l6_q                      (l2_tbnk0_rwvic_snp_l6_q),
                            .l2_tbnk_rwvic_tag_wr_l0                     (l2_tbnk0_rwvic_tag_wr_l0),
                            .l2_tbnk_rwvic_wa_l6_q                       (l2_tbnk0_rwvic_wa_l6_q),
                            .l2_tbnk_size_l4_q                           (l2_tbnk0_size_l4_q[2:0]),
                            .l2_tbnk_snp_hit_e_l4_q                      (l2_tbnk0_snp_hit_e_l4_q),
                            .l2_tbnk_snp_hit_s_l4_q                      (l2_tbnk0_snp_hit_s_l4_q),
                            .l2_tbnk_snp_tag_wr_l2_hit_addr_l1           (l2_tbnk0_snp_tag_wr_l2_hit_addr_l1[44:7]),
                            .l2_tbnk_snp_tag_wr_l2_hit_state_l1          (l2_tbnk0_snp_tag_wr_l2_hit_state_l1[1:0]),
                            .l2_tbnk_snp_tag_wr_l2_hit_way_l1            (l2_tbnk0_snp_tag_wr_l2_hit_way_l1),
                            .l2_tbnk_special_evict_hazard_l3             (l2_tbnk0_special_evict_hazard_l3),
                            .l2_tbnk_special_hazard_l3_q                 (l2_tbnk0_special_hazard_l3_q),
                            .l2_tbnk_tag_ecc_dbl_rmw_wr_l1               (l2_tbnk0_tag_ecc_dbl_rmw_wr_l1),
                            .l2_tbnk_tag_ecc_err_cpu0_l4                 (l2_tbnk0_tag_ecc_err_cpu0_l4),
                            .l2_tbnk_tag_ecc_err_cpu1_l4                 (l2_tbnk0_tag_ecc_err_cpu1_l4),
                            .l2_tbnk_tag_ecc_err_cpu2_l4                 (l2_tbnk0_tag_ecc_err_cpu2_l4),
                            .l2_tbnk_tag_ecc_err_cpu3_l4                 (l2_tbnk0_tag_ecc_err_cpu3_l4),
                            .l2_tbnk_tag_ecc_err_l4                      (l2_tbnk0_tag_ecc_err_l4),
                            .l2_tbnk_ulen_l4_q                           (l2_tbnk0_ulen_l4_q[1:0]),
                            .l2_tbnk_vld_init_l6_q                       (l2_tbnk0_vld_init_l6_q),
                            .l2_tbnk_vld_l6_q                            (l2_tbnk0_vld_l6_q),
                            .l2_tbnk_way_l4_q                            (l2_tbnk0_way_l4_q),
                            .l2_tbnk_way_nxt_l3a                         (l2_tbnk0_way_nxt_l3a),
                            .l2_tbnk_wr_data_l3                          (l2_tbnk0_wr_data_l3[143:0]),
                            .l2_tbnk_wr_data_l4_en                       (l2_tbnk0_wr_data_l4_en),
                            .l2_tbnk_wr_non_crit_id_l4_q                 (l2_tbnk0_wr_non_crit_id_l4_q[11:0]),

                            // inputs
                            .DFTCLKBYPASS                                (DFTCLKBYPASS),
                            .DFTMCPHOLD                                  (DFTMCPHOLD),
                            .DFTRAMHOLD                                  (DFTRAMHOLD),
                            .DFTRSTDISABLE                               (DFTRSTDISABLE),
                            .DFTSE                                       (DFTSE),
                            .MBISTREQ                                    (MBISTREQ),
                            .ck_areset_l2                                (ck_areset_l2),
                            .ck_gclkl2                                   (ck_gclkb0),
                            .ck_reset1_n_l2                              (ck_reset1_n_l2),
                            .l2_acp_flsh_rd_cnt_active_glb_l2_dly        (l2_acp_flsh_rd_cnt_active_glb_l2_dly),
                            .l2_acp_flsh_wr_cnt_active_glb_l2_dly        (l2_acp_flsh_wr_cnt_active_glb_l2_dly),
                            .l2_acp_rd_haz_vld_l2_dly_q                  (l2_acp_rd_haz_vld_l2_dly_q),
                            .l2_acp_wr_haz_vld_l2_dly_q                  (l2_acp_wr_haz_vld_l2_dly_q),
                            .l2_actlr_disable_b2b_setway_hzd_opt_x2_ns   (l2_actlr_disable_b2b_setway_hzd_opt_x2_ns),
                            .l2_actlr_disable_setway_opt                 (l2_actlr_disable_setway_opt),
                            .l2_actlr_plru_dynamic                       (l2_actlr_plru_dynamic),
                            .l2_actlr_plru_en                            (l2_actlr_plru_en),
                            .l2_actlr_plru_mode                          (l2_actlr_plru_mode[1:0]),
                            .l2_actlr_writeunique_disable                (l2_actlr_writeunique_disable),
                            .l2_cfg_broadcastinner                       (l2_cfg_broadcastinner),
                            .l2_cfg_broadcastouter                       (l2_cfg_broadcastouter),
                            .l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu0_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu0_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu0_ls_rd_haz_vld_l2_dly_q              (l2_cpu0_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu0_ls_wr_haz_vld_l2_dly_q              (l2_cpu0_ls_wr_haz_vld_l2_dly_q),
                            .l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu1_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu1_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu1_ls_rd_haz_vld_l2_dly_q              (l2_cpu1_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu1_ls_wr_haz_vld_l2_dly_q              (l2_cpu1_ls_wr_haz_vld_l2_dly_q),
                            .l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu2_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu2_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu2_ls_rd_haz_vld_l2_dly_q              (l2_cpu2_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu2_ls_wr_haz_vld_l2_dly_q              (l2_cpu2_ls_wr_haz_vld_l2_dly_q),
                            .l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu3_tbnk0_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu3_tbnk0_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu3_ls_rd_haz_vld_l2_dly_q              (l2_cpu3_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu3_ls_wr_haz_vld_l2_dly_q              (l2_cpu3_ls_wr_haz_vld_l2_dly_q),
                            .l2_ctlr_x1_wr_q                             (l2_ctlr_x1_wr_q),
                            .l2_ctlr_x2_ns                               (l2_ctlr_x2_ns[9:0]),
                            .l2_mbist2_snp0_outdata_b2                   (l2_mbist2_tbnk0_snp0_outdata_b2[79:0]),
                            .l2_mbist2_snp0_outdata_vld_b2               (l2_mbist2_tbnk0_snp0_outdata_vld_b2),
                            .l2_mbist2_snp1_outdata_b2                   (l2_mbist2_tbnk0_snp1_outdata_b2[79:0]),
                            .l2_mbist2_snp1_outdata_vld_b2               (l2_mbist2_tbnk0_snp1_outdata_vld_b2),
                            .l2_mbist2_snp2_outdata_b2                   (l2_mbist2_tbnk0_snp2_outdata_b2[79:0]),
                            .l2_mbist2_snp2_outdata_vld_b2               (l2_mbist2_tbnk0_snp2_outdata_vld_b2),
                            .l2_mbist2_snp3_outdata_b2                   (l2_mbist2_tbnk0_snp3_outdata_b2[79:0]),
                            .l2_mbist2_snp3_outdata_vld_b2               (l2_mbist2_tbnk0_snp3_outdata_vld_b2),
                            .l2_no_ram_acc_nxt_cycle                     (l2_no_ram_acc_nxt_cycle),
                            .l2_rstdisable_x1_q                          (l2_rstdisable_x1_q),
                            .l2_skyros_intf                              (1'b1),
                            .l2_tbnk_addr_l1                             (l2_tbnk0_addr_l1[44:0]),
                            .l2_tbnk_asq_cmp_evict_l3_q                  (l2_tbnk0_asq_cmp_evict_l3_q),
                            .l2_tbnk_asq_full_flsh                       (l2_tbnk0_asq_full_flsh),
                            .l2_tbnk_asq_nc_so_dev_limit                 (l2_tbnk0_asq_nc_so_dev_limit),
                            .l2_tbnk_cache_attr_l1                       (l2_tbnk0_cache_attr_l1[2:0]),
                            .l2_tbnk_cfg_ecc_en                          (l2_tbnk0_cfg_ecc_en),
                            .l2_tbnk_cpu0_peq_full_q                     (l2_tbnk0_cpu0_peq_full_q),
                            .l2_tbnk_cpu0_peq_hit_q                      (l2_tbnk0_cpu0_peq_hit_q),
                            .l2_tbnk_cpu0_peq_self_evict_l3_q            (l2_tbnk0_cpu0_peq_self_evict_l3_q),
                            .l2_tbnk_cpu0_peq_self_evict_wbna_l3_q       (l2_tbnk0_cpu0_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu0_snp_hit_e_l3                   (l2_tbnk0_cpu0_snp_hit_e_l3),
                            .l2_tbnk_cpu0_snp_hit_s_l3                   (l2_tbnk0_cpu0_snp_hit_s_l3),
                            .l2_tbnk_cpu0_snp_setway_addr_l3             (l2_tbnk0_cpu0_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu0_snp_tag_ecc_err_tp_l4_q        (l2_tbnk0_cpu0_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu0_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk0_cpu0_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_cpu1_peq_full_q                     (l2_tbnk0_cpu1_peq_full_q),
                            .l2_tbnk_cpu1_peq_hit_q                      (l2_tbnk0_cpu1_peq_hit_q),
                            .l2_tbnk_cpu1_peq_self_evict_l3_q            (l2_tbnk0_cpu1_peq_self_evict_l3_q),
                            .l2_tbnk_cpu1_peq_self_evict_wbna_l3_q       (l2_tbnk0_cpu1_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu1_snp_hit_e_l3                   (l2_tbnk0_cpu1_snp_hit_e_l3),
                            .l2_tbnk_cpu1_snp_hit_s_l3                   (l2_tbnk0_cpu1_snp_hit_s_l3),
                            .l2_tbnk_cpu1_snp_setway_addr_l3             (l2_tbnk0_cpu1_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu1_snp_tag_ecc_err_tp_l4_q        (l2_tbnk0_cpu1_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu1_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk0_cpu1_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_cpu2_peq_full_q                     (l2_tbnk0_cpu2_peq_full_q),
                            .l2_tbnk_cpu2_peq_hit_q                      (l2_tbnk0_cpu2_peq_hit_q),
                            .l2_tbnk_cpu2_peq_self_evict_l3_q            (l2_tbnk0_cpu2_peq_self_evict_l3_q),
                            .l2_tbnk_cpu2_peq_self_evict_wbna_l3_q       (l2_tbnk0_cpu2_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu2_snp_hit_e_l3                   (l2_tbnk0_cpu2_snp_hit_e_l3),
                            .l2_tbnk_cpu2_snp_hit_s_l3                   (l2_tbnk0_cpu2_snp_hit_s_l3),
                            .l2_tbnk_cpu2_snp_setway_addr_l3             (l2_tbnk0_cpu2_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu2_snp_tag_ecc_err_tp_l4_q        (l2_tbnk0_cpu2_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu2_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk0_cpu2_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_cpu3_peq_full_q                     (l2_tbnk0_cpu3_peq_full_q),
                            .l2_tbnk_cpu3_peq_hit_q                      (l2_tbnk0_cpu3_peq_hit_q),
                            .l2_tbnk_cpu3_peq_self_evict_l3_q            (l2_tbnk0_cpu3_peq_self_evict_l3_q),
                            .l2_tbnk_cpu3_peq_self_evict_wbna_l3_q       (l2_tbnk0_cpu3_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu3_snp_hit_e_l3                   (l2_tbnk0_cpu3_snp_hit_e_l3),
                            .l2_tbnk_cpu3_snp_hit_s_l3                   (l2_tbnk0_cpu3_snp_hit_s_l3),
                            .l2_tbnk_cpu3_snp_setway_addr_l3             (l2_tbnk0_cpu3_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu3_snp_tag_ecc_err_tp_l4_q        (l2_tbnk0_cpu3_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu3_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk0_cpu3_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_dirty_l1                            (l2_tbnk0_dirty_l1),
                            .l2_tbnk_dis_ns_dbg_arr_acc_x2               (l2_tbnk0_dis_ns_dbg_arr_acc_x2),
                            .l2_tbnk_excl_l1                             (l2_tbnk0_excl_l1),
                            .l2_tbnk_feq_alloc_failed_l4                 (l2_tbnk0_feq_alloc_failed_l4),
                            .l2_tbnk_feq_axi_wr_vld_not_popped           (l2_tbnk0_feq_axi_wr_vld_not_popped),
                            .l2_tbnk_feq_frc_incl_l3a                    (l2_tbnk0_feq_frc_incl_l3a[15:0]),
                            .l2_tbnk_feq_kill_l3                         (l2_tbnk0_feq_kill_l3),
                            .l2_tbnk_feq_last_id_q                       (l2_tbnk0_feq_last_id_q[4:0]),
                            .l2_tbnk_feq_tbnk_id_update_hit_prfm_or_l3   (l2_tbnk0_feq_tbnk_id_update_hit_prfm_or_l3),
                            .l2_tbnk_feq_tbnk_id_update_or_l3            (l2_tbnk0_feq_tbnk_id_update_or_l3),
                            .l2_tbnk_hwrst_done_x2                       (l2_tbnk_hwrst_done_x2),
                            .l2_tbnk_hwrst_idx_x1_q                      (l2_tbnk_hwrst_idx_x1_q[13:0]),
                            .l2_tbnk_id_l1                               (l2_tbnk0_id_l1[9:0]),
                            .l2_tbnk_init_req_l1                         (l2_tbnk0_init_req_l1),
                            .l2_tbnk_kill_l2                             (l2_tbnk0_kill_l2),
                            .l2_tbnk_l2bb_fake_wr_l1                     (l2_tbnk0_l2bb_fake_wr_l1),
                            .l2_tbnk_l2bb_wr_l1                          (l2_tbnk0_l2bb_wr_l1),
                            .l2_tbnk_last_qw_l1                          (l2_tbnk0_last_qw_l1),
                            .l2_tbnk_lock_l1                             (l2_tbnk0_lock_l1[2:0]),
                            .l2_tbnk_page_attr_l1                        (l2_tbnk0_page_attr_l1[9:0]),
                            .l2_tbnk_partial_dw_wr_l1                    (l2_tbnk0_partial_dw_wr_l1),
                            .l2_tbnk_pf_hazard_l3                        (l2_tbnk0_pf_hazard_l3),
                            .l2_tbnk_prfm_l1                             (l2_tbnk0_prfm_l1),
                            .l2_tbnk_prot_l1                             (l2_tbnk0_prot_l1[3:0]),
                            .l2_tbnk_qw_cnt_l1                           (l2_tbnk0_qw_cnt_l1[1:0]),
                            .l2_tbnk_rd_fail_hazchk_feq_l3               (l2_tbnk0_rd_fail_hazchk_feq_l3),
                            .l2_tbnk_rwvic_axi_read_err_l1               (l2_tbnk0_rwvic_axi_read_err_l1),
                            .l2_tbnk_rwvic_ccb_ls_xfer_l1                (l2_tbnk0_rwvic_ccb_ls_xfer_l1),
                            .l2_tbnk_rwvic_ccb_way_l1                    (l2_tbnk0_rwvic_ccb_way_l1[3:0]),
                            .l2_tbnk_rwvic_cmo_clean_l1                  (l2_tbnk0_rwvic_cmo_clean_l1),
                            .l2_tbnk_rwvic_cmo_inv_l1                    (l2_tbnk0_rwvic_cmo_inv_l1),
                            .l2_tbnk_rwvic_cmo_pou_l1                    (l2_tbnk0_rwvic_cmo_pou_l1),
                            .l2_tbnk_rwvic_cmo_setway_l1                 (l2_tbnk0_rwvic_cmo_setway_l1),
                            .l2_tbnk_rwvic_cmo_setway_ls_full_miss_l1    (l2_tbnk0_rwvic_cmo_setway_ls_full_miss_l1),
                            .l2_tbnk_rwvic_cpu_fb_id_l1                  (l2_tbnk0_rwvic_cpu_fb_id_l1[2:0]),
                            .l2_tbnk_rwvic_cpu_id_dcd_l1                 (l2_tbnk0_rwvic_cpu_id_dcd_l1[3:0]),
                            .l2_tbnk_rwvic_feq_cmp_l3_q                  (l2_tbnk0_rwvic_feq_cmp_l3_q),
                            .l2_tbnk_rwvic_frc_l2hit_fwd_l1              (l2_tbnk0_rwvic_frc_l2hit_fwd_l1),
                            .l2_tbnk_rwvic_l2hit_e_l1                    (l2_tbnk0_rwvic_l2hit_e_l1),
                            .l2_tbnk_rwvic_mesi_sh_l1                    (l2_tbnk0_rwvic_mesi_sh_l1),
                            .l2_tbnk_rwvic_owner_l1                      (l2_tbnk0_rwvic_owner_l1[2:0]),
                            .l2_tbnk_rwvic_snp_clr_dirty_l1              (l2_tbnk0_rwvic_snp_clr_dirty_l1),
                            .l2_tbnk_rwvic_snp_inv_l1                    (l2_tbnk0_rwvic_snp_inv_l1),
                            .l2_tbnk_rwvic_snp_l1                        (l2_tbnk0_rwvic_snp_l1),
                            .l2_tbnk_rwvic_type_l1                       (l2_tbnk0_rwvic_type_l1[3:0]),
                            .l2_tbnk_rwvic_wa_l1                         (l2_tbnk0_rwvic_wa_l1),
                            .l2_tbnk_sel_l1                              (l2_tbnk0_sel_l1[13:0]),
                            .l2_tbnk_size_l1                             (l2_tbnk0_size_l1[2:0]),
                            .l2_tbnk_snp_byp_peq_haz_pending_q           (l2_tbnk0_snp_byp_peq_haz_pending_q),
                            .l2_tbnk_snp_dvm_cmpl_l1                     (l2_tbnk0_snp_dvm_cmpl_l1),
                            .l2_tbnk_snp_hit_feq_evict_l4_dly            (l2_tbnk0_snp_hit_feq_evict_l4_dly),
                            .l2_tbnk_snp_rd_feq_id_cmp_l4_dly_q          (l2_tbnk0_snp_rd_feq_id_cmp_l4_dly_q[4:0]),
                            .l2_tbnk_snp_tag_double_ecc_err_l4_dly_q     (l2_tbnk0_snp_tag_double_ecc_err_l4_dly_q[7:0]),
                            .l2_tbnk_snp_tag_single_ecc_err_l4_dly_q     (l2_tbnk0_snp_tag_single_ecc_err_l4_dly_q[7:0]),
                            .l2_tbnk_sync_l1                             (l2_tbnk0_sync_l1),
                            .l2_tbnk_type_l1                             (l2_tbnk0_type_l1[6:0]),
                            .l2_tbnk_ulen_l1                             (l2_tbnk0_ulen_l1[1:0]),
                            .l2_tbnk_way_l1                              (l2_tbnk0_way_l1),
                            .l2_tbnk_wr_data_l3a_q                       (l2_tbnk0_wr_data_l3a_q[127:0]),
                            .l2_tbnk_wr_err_l1                           (l2_tbnk0_wr_err_l1),
                            .l2_tbnk_wr_fail_feq_full_l3                 (l2_tbnk0_wr_fail_feq_full_l3),
                            .l2_tbnk_wr_fail_hazchk_feq_l3               (l2_tbnk0_wr_fail_hazchk_feq_l3),
                            .l2_tbnk_wr_non_crit_id_l1                   (l2_tbnk0_wr_non_crit_id_l1[11:0]),
                            .l2_tbnk_wr_strb_mask_l3a_q                  (l2_tbnk0_wr_strb_mask_l3a_q[15:0])
                          );    // ul2_tbnk0

    maia_l2_tbnk ul2_tbnk1( // outputs
                            .l2_mbist2_addr_b1                           (l2_mbist2_tbnk1_addr_b1[16:0]),
                            .l2_mbist2_array_b1                          (l2_mbist2_tbnk1_array_b1[2:0]),
                            .l2_mbist2_be_b1                             (l2_mbist2_tbnk1_be_b1[17:0]),
                            .l2_mbist2_en_b1                             (l2_mbist2_tbnk1_en_b1),
                            .l2_mbist2_indata_b1                         (l2_mbist2_tbnk1_indata_b1[143:0]),
                            .l2_mbist2_tbnk_all_b1                       (l2_mbist2_tbnk1_all_b1),
                            .l2_mbist2_tbnk_outdata_b3                   (l2_mbist2_tbnk1_outdata_b3[143:0]),
                            .l2_mbist2_tbnk_sel_b1                       (l2_mbist2_tbnk1_sel_b1),
                            .l2_mbist2_tbnk_snp0_sel_b1                  (l2_mbist2_tbnk1_snp0_sel_b1),
                            .l2_mbist2_tbnk_snp1_sel_b1                  (l2_mbist2_tbnk1_snp1_sel_b1),
                            .l2_mbist2_tbnk_snp2_sel_b1                  (l2_mbist2_tbnk1_snp2_sel_b1),
                            .l2_mbist2_tbnk_snp3_sel_b1                  (l2_mbist2_tbnk1_snp3_sel_b1),
                            .l2_mbist2_wr_en_b1                          (l2_mbist2_tbnk1_wr_en_b1),
                            .l2_tbnk_addr44_l3_q                         (l2_tbnk1_addr44_l3_q),
                            .l2_tbnk_addr_l6                             (l2_tbnk1_addr_l6[5:2]),
                            .l2_tbnk_all_tag_incl_active_l3              (l2_tbnk1_all_tag_incl_active_l3),
                            .l2_tbnk_cmo_setway_l2_inv_incl_l4           (l2_tbnk1_cmo_setway_l2_inv_incl_l4),
                            .l2_tbnk_cpu0_ccb_xfer_l4_dly2               (l2_tbnk1_cpu0_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu0_hit_l4                         (l2_tbnk1_cpu0_hit_l4),
                            .l2_tbnk_cpu0_l2_inv_l4_dly2                 (l2_tbnk1_cpu0_l2_inv_l4_dly2),
                            .l2_tbnk_cpu0_l2hit_e_l4                     (l2_tbnk1_cpu0_l2hit_e_l4),
                            .l2_tbnk_cpu0_l2hit_s_l4                     (l2_tbnk1_cpu0_l2hit_s_l4),
                            .l2_tbnk_cpu0_rd_access_l4_dly               (l2_tbnk1_cpu0_rd_access_l4_dly),
                            .l2_tbnk_cpu0_self_evict_l4_dly_q            (l2_tbnk1_cpu0_self_evict_l4_dly_q),
                            .l2_tbnk_cpu0_single_ecc_err_l7_q            (l2_tbnk1_cpu0_single_ecc_err_l7_q),
                            .l2_tbnk_cpu0_snp_tag_ecc_rmw_rd_l0          (l2_tbnk1_cpu0_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu0_snp_tag_ecc_rmw_wr_l0          (l2_tbnk1_cpu0_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu0_snp_tag_wr_evict_early_l4_dly  (l2_tbnk1_cpu0_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu0_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk1_cpu0_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu0_vld_nxt_l5                     (l2_tbnk1_cpu0_vld_nxt_l5),
                            .l2_tbnk_cpu0_wr_access_l4_dly               (l2_tbnk1_cpu0_wr_access_l4_dly),
                            .l2_tbnk_cpu1_ccb_xfer_l4_dly2               (l2_tbnk1_cpu1_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu1_hit_l4                         (l2_tbnk1_cpu1_hit_l4),
                            .l2_tbnk_cpu1_l2_inv_l4_dly2                 (l2_tbnk1_cpu1_l2_inv_l4_dly2),
                            .l2_tbnk_cpu1_l2hit_e_l4                     (l2_tbnk1_cpu1_l2hit_e_l4),
                            .l2_tbnk_cpu1_l2hit_s_l4                     (l2_tbnk1_cpu1_l2hit_s_l4),
                            .l2_tbnk_cpu1_rd_access_l4_dly               (l2_tbnk1_cpu1_rd_access_l4_dly),
                            .l2_tbnk_cpu1_self_evict_l4_dly_q            (l2_tbnk1_cpu1_self_evict_l4_dly_q),
                            .l2_tbnk_cpu1_single_ecc_err_l7_q            (l2_tbnk1_cpu1_single_ecc_err_l7_q),
                            .l2_tbnk_cpu1_snp_tag_ecc_rmw_rd_l0          (l2_tbnk1_cpu1_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu1_snp_tag_ecc_rmw_wr_l0          (l2_tbnk1_cpu1_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu1_snp_tag_wr_evict_early_l4_dly  (l2_tbnk1_cpu1_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu1_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk1_cpu1_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu1_vld_nxt_l5                     (l2_tbnk1_cpu1_vld_nxt_l5),
                            .l2_tbnk_cpu1_wr_access_l4_dly               (l2_tbnk1_cpu1_wr_access_l4_dly),
                            .l2_tbnk_cpu2_ccb_xfer_l4_dly2               (l2_tbnk1_cpu2_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu2_hit_l4                         (l2_tbnk1_cpu2_hit_l4),
                            .l2_tbnk_cpu2_l2_inv_l4_dly2                 (l2_tbnk1_cpu2_l2_inv_l4_dly2),
                            .l2_tbnk_cpu2_l2hit_e_l4                     (l2_tbnk1_cpu2_l2hit_e_l4),
                            .l2_tbnk_cpu2_l2hit_s_l4                     (l2_tbnk1_cpu2_l2hit_s_l4),
                            .l2_tbnk_cpu2_rd_access_l4_dly               (l2_tbnk1_cpu2_rd_access_l4_dly),
                            .l2_tbnk_cpu2_self_evict_l4_dly_q            (l2_tbnk1_cpu2_self_evict_l4_dly_q),
                            .l2_tbnk_cpu2_single_ecc_err_l7_q            (l2_tbnk1_cpu2_single_ecc_err_l7_q),
                            .l2_tbnk_cpu2_snp_tag_ecc_rmw_rd_l0          (l2_tbnk1_cpu2_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu2_snp_tag_ecc_rmw_wr_l0          (l2_tbnk1_cpu2_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu2_snp_tag_wr_evict_early_l4_dly  (l2_tbnk1_cpu2_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu2_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk1_cpu2_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu2_vld_nxt_l5                     (l2_tbnk1_cpu2_vld_nxt_l5),
                            .l2_tbnk_cpu2_wr_access_l4_dly               (l2_tbnk1_cpu2_wr_access_l4_dly),
                            .l2_tbnk_cpu3_ccb_xfer_l4_dly2               (l2_tbnk1_cpu3_ccb_xfer_l4_dly2),
                            .l2_tbnk_cpu3_hit_l4                         (l2_tbnk1_cpu3_hit_l4),
                            .l2_tbnk_cpu3_l2_inv_l4_dly2                 (l2_tbnk1_cpu3_l2_inv_l4_dly2),
                            .l2_tbnk_cpu3_l2hit_e_l4                     (l2_tbnk1_cpu3_l2hit_e_l4),
                            .l2_tbnk_cpu3_l2hit_s_l4                     (l2_tbnk1_cpu3_l2hit_s_l4),
                            .l2_tbnk_cpu3_rd_access_l4_dly               (l2_tbnk1_cpu3_rd_access_l4_dly),
                            .l2_tbnk_cpu3_self_evict_l4_dly_q            (l2_tbnk1_cpu3_self_evict_l4_dly_q),
                            .l2_tbnk_cpu3_single_ecc_err_l7_q            (l2_tbnk1_cpu3_single_ecc_err_l7_q),
                            .l2_tbnk_cpu3_snp_tag_ecc_rmw_rd_l0          (l2_tbnk1_cpu3_snp_tag_ecc_rmw_rd_l0[1:0]),
                            .l2_tbnk_cpu3_snp_tag_ecc_rmw_wr_l0          (l2_tbnk1_cpu3_snp_tag_ecc_rmw_wr_l0[1:0]),
                            .l2_tbnk_cpu3_snp_tag_wr_evict_early_l4_dly  (l2_tbnk1_cpu3_snp_tag_wr_evict_early_l4_dly),
                            .l2_tbnk_cpu3_snp_tag_wr_l2_hit_early_l4_dly (l2_tbnk1_cpu3_snp_tag_wr_l2_hit_early_l4_dly),
                            .l2_tbnk_cpu3_vld_nxt_l5                     (l2_tbnk1_cpu3_vld_nxt_l5),
                            .l2_tbnk_cpu3_wr_access_l4_dly               (l2_tbnk1_cpu3_wr_access_l4_dly),
                            .l2_tbnk_cpu_rvalid_init_nxt_l5              (l2_tbnk1_cpu_rvalid_init_nxt_l5[3:0]),
                            .l2_tbnk_cpu_rvalid_nxt_l5                   (l2_tbnk1_cpu_rvalid_nxt_l5[3:0]),
                            .l2_tbnk_cpu_snp_hit_e_l4_q                  (l2_tbnk1_cpu_snp_hit_e_l4_q[3:0]),
                            .l2_tbnk_crit_qw_nxt_l5                      (l2_tbnk1_crit_qw_nxt_l5),
                            .l2_tbnk_data_corrected_l7_q                 (l2_tbnk1_data_corrected_l7_q[143:0]),
                            .l2_tbnk_data_l6                             (l2_tbnk1_data_l6[127:0]),
                            .l2_tbnk_dbg_ram_acc_l5a                     (l2_tbnk1_dbg_ram_acc_l5a),
                            .l2_tbnk_dbg_ram_acc_unit_nxt                (l2_tbnk1_dbg_ram_acc_unit_nxt[2:0]),
                            .l2_tbnk_dbg_ram_id_nxt_l5                   (l2_tbnk1_dbg_ram_id_nxt_l5[7:0]),
                            .l2_tbnk_dirty_l3_q                          (l2_tbnk1_dirty_l3_q),
                            .l2_tbnk_double_ecc_err_l7_q                 (l2_tbnk1_double_ecc_err_l7_q),
                            .l2_tbnk_early_rvalid_l4_q                   (l2_tbnk1_early_rvalid_l4_q),
                            .l2_tbnk_ecc_fixup_blk_arb                   (l2_tbnk1_ecc_fixup_blk_arb),
                            .l2_tbnk_ecc_fixup_inprog_dly_q              (l2_tbnk1_ecc_fixup_inprog_dly_q),
                            .l2_tbnk_ecc_rmw_snp_tag_rd_l3_q             (l2_tbnk1_ecc_rmw_snp_tag_rd_l3_q),
                            .l2_tbnk_ecc_syndrome_reg_q                  (l2_tbnk1_ecc_syndrome_reg_q[31:0]),
                            .l2_tbnk_evict_special_hazard_l3_q           (l2_tbnk1_evict_special_hazard_l3_q),
                            .l2_tbnk_evict_special_hazard_rwvic_l3_q     (l2_tbnk1_evict_special_hazard_rwvic_l3_q),
                            .l2_tbnk_excl_l4_q                           (l2_tbnk1_excl_l4_q),
                            .l2_tbnk_feq_addr_upd                        (l2_tbnk1_feq_addr_upd[44:6]),
                            .l2_tbnk_feq_clr_l4                          (l2_tbnk1_feq_clr_l4),
                            .l2_tbnk_full_miss_l4_q                      (l2_tbnk1_full_miss_l4_q),
                            .l2_tbnk_hit_l4                              (l2_tbnk1_hit_l4),
                            .l2_tbnk_hit_l7_q                            (l2_tbnk1_hit_l7_q),
                            .l2_tbnk_hit_way_l4_q                        (l2_tbnk1_hit_way_l4_q[3:0]),
                            .l2_tbnk_id_l6_q                             (l2_tbnk1_id_l6_q[9:0]),
                            .l2_tbnk_id_nxt_l5                           (l2_tbnk1_id_nxt_l5[9:0]),
                            .l2_tbnk_idle                                (l2_tbnk1_idle),
                            .l2_tbnk_l2hit_e_l4                          (l2_tbnk1_l2hit_e_l4),
                            .l2_tbnk_l2hit_s_l4                          (l2_tbnk1_l2hit_s_l4),
                            .l2_tbnk_l2v_s_q                             (l2_tbnk1_l2v_s_q),
                            .l2_tbnk_l2v_vld_q                           (l2_tbnk1_l2v_vld_q),
                            .l2_tbnk_last_qw_l6_q                        (l2_tbnk1_last_qw_l6_q),
                            .l2_tbnk_last_qw_nxt_l5                      (l2_tbnk1_last_qw_nxt_l5),
                            .l2_tbnk_lock_l4                             (l2_tbnk1_lock_l4[2:0]),
                            .l2_tbnk_merrsr_data                         (l2_tbnk1_merrsr_data[32:0]),
                            .l2_tbnk_pf_cnt_dec_l4_dly                   (l2_tbnk1_pf_cnt_dec_l4_dly),
                            .l2_tbnk_pf_req_sel_for_fwd_l4               (l2_tbnk1_pf_req_sel_for_fwd_l4),
                            .l2_tbnk_prfm_nxt_l5                         (l2_tbnk1_prfm_nxt_l5),
                            .l2_tbnk_prot_l4_q                           (l2_tbnk1_prot_l4_q[3:0]),
                            .l2_tbnk_qw_cnt_l3_q                         (l2_tbnk1_qw_cnt_l3_q[1:0]),
                            .l2_tbnk_raw_hit_l4_q                        (l2_tbnk1_raw_hit_l4_q),
                            .l2_tbnk_rbufid_nxt_l5                       (l2_tbnk1_rbufid_nxt_l5[2:0]),
                            .l2_tbnk_rd_en_nxt_l5                        (l2_tbnk1_rd_en_nxt_l5),
                            .l2_tbnk_rwvic_axi_read_err_l3_q             (l2_tbnk1_rwvic_axi_read_err_l3_q),
                            .l2_tbnk_rwvic_ccb_dirty_l6_q                (l2_tbnk1_rwvic_ccb_dirty_l6_q),
                            .l2_tbnk_rwvic_ccb_ls_xfer_l3_q              (l2_tbnk1_rwvic_ccb_ls_xfer_l3_q),
                            .l2_tbnk_rwvic_ccb_ls_xfer_l6_q              (l2_tbnk1_rwvic_ccb_ls_xfer_l6_q),
                            .l2_tbnk_rwvic_cmo_inv_l7_q                  (l2_tbnk1_rwvic_cmo_inv_l7_q),
                            .l2_tbnk_rwvic_cmo_l7_q                      (l2_tbnk1_rwvic_cmo_l7_q),
                            .l2_tbnk_rwvic_cmo_pou_l6_q                  (l2_tbnk1_rwvic_cmo_pou_l6_q),
                            .l2_tbnk_rwvic_cmo_setway_ls_l6_q            (l2_tbnk1_rwvic_cmo_setway_ls_l6_q),
                            .l2_tbnk_rwvic_ddi_l6_q                      (l2_tbnk1_rwvic_ddi_l6_q),
                            .l2_tbnk_rwvic_l2hit_e_l3_q                  (l2_tbnk1_rwvic_l2hit_e_l3_q),
                            .l2_tbnk_rwvic_l2hit_e_l7_q                  (l2_tbnk1_rwvic_l2hit_e_l7_q),
                            .l2_tbnk_rwvic_l2v_dirty_l7_q                (l2_tbnk1_rwvic_l2v_dirty_l7_q),
                            .l2_tbnk_rwvic_l2v_page_attr_l7_q            (l2_tbnk1_rwvic_l2v_page_attr_l7_q[3:0]),
                            .l2_tbnk_rwvic_l2v_vld_l6_q                  (l2_tbnk1_rwvic_l2v_vld_l6_q),
                            .l2_tbnk_rwvic_non_snp_fail_hazchk_l3        (l2_tbnk1_rwvic_non_snp_fail_hazchk_l3),
                            .l2_tbnk_rwvic_owner_l7_q                    (l2_tbnk1_rwvic_owner_l7_q[2:0]),
                            .l2_tbnk_rwvic_rd_type_l6_q                  (l2_tbnk1_rwvic_rd_type_l6_q),
                            .l2_tbnk_rwvic_snp_l3_q                      (l2_tbnk1_rwvic_snp_l3_q),
                            .l2_tbnk_rwvic_snp_l6_q                      (l2_tbnk1_rwvic_snp_l6_q),
                            .l2_tbnk_rwvic_tag_wr_l0                     (l2_tbnk1_rwvic_tag_wr_l0),
                            .l2_tbnk_rwvic_wa_l6_q                       (l2_tbnk1_rwvic_wa_l6_q),
                            .l2_tbnk_size_l4_q                           (l2_tbnk1_size_l4_q[2:0]),
                            .l2_tbnk_snp_hit_e_l4_q                      (l2_tbnk1_snp_hit_e_l4_q),
                            .l2_tbnk_snp_hit_s_l4_q                      (l2_tbnk1_snp_hit_s_l4_q),
                            .l2_tbnk_snp_tag_wr_l2_hit_addr_l1           (l2_tbnk1_snp_tag_wr_l2_hit_addr_l1[44:7]),
                            .l2_tbnk_snp_tag_wr_l2_hit_state_l1          (l2_tbnk1_snp_tag_wr_l2_hit_state_l1[1:0]),
                            .l2_tbnk_snp_tag_wr_l2_hit_way_l1            (l2_tbnk1_snp_tag_wr_l2_hit_way_l1),
                            .l2_tbnk_special_evict_hazard_l3             (l2_tbnk1_special_evict_hazard_l3),
                            .l2_tbnk_special_hazard_l3_q                 (l2_tbnk1_special_hazard_l3_q),
                            .l2_tbnk_tag_ecc_dbl_rmw_wr_l1               (l2_tbnk1_tag_ecc_dbl_rmw_wr_l1),
                            .l2_tbnk_tag_ecc_err_cpu0_l4                 (l2_tbnk1_tag_ecc_err_cpu0_l4),
                            .l2_tbnk_tag_ecc_err_cpu1_l4                 (l2_tbnk1_tag_ecc_err_cpu1_l4),
                            .l2_tbnk_tag_ecc_err_cpu2_l4                 (l2_tbnk1_tag_ecc_err_cpu2_l4),
                            .l2_tbnk_tag_ecc_err_cpu3_l4                 (l2_tbnk1_tag_ecc_err_cpu3_l4),
                            .l2_tbnk_tag_ecc_err_l4                      (l2_tbnk1_tag_ecc_err_l4),
                            .l2_tbnk_ulen_l4_q                           (l2_tbnk1_ulen_l4_q[1:0]),
                            .l2_tbnk_vld_init_l6_q                       (l2_tbnk1_vld_init_l6_q),
                            .l2_tbnk_vld_l6_q                            (l2_tbnk1_vld_l6_q),
                            .l2_tbnk_way_l4_q                            (l2_tbnk1_way_l4_q),
                            .l2_tbnk_way_nxt_l3a                         (l2_tbnk1_way_nxt_l3a),
                            .l2_tbnk_wr_data_l3                          (l2_tbnk1_wr_data_l3[143:0]),
                            .l2_tbnk_wr_data_l4_en                       (l2_tbnk1_wr_data_l4_en),
                            .l2_tbnk_wr_non_crit_id_l4_q                 (l2_tbnk1_wr_non_crit_id_l4_q[11:0]),

                            // inputs
                            .DFTCLKBYPASS                                (DFTCLKBYPASS),
                            .DFTMCPHOLD                                  (DFTMCPHOLD),
                            .DFTRAMHOLD                                  (DFTRAMHOLD),
                            .DFTRSTDISABLE                               (DFTRSTDISABLE),
                            .DFTSE                                       (DFTSE),
                            .MBISTREQ                                    (MBISTREQ),
                            .ck_areset_l2                                (ck_areset_l2),
                            .ck_gclkl2                                   (ck_gclkb1),
                            .ck_reset1_n_l2                              (ck_reset1_n_l2),
                            .l2_acp_flsh_rd_cnt_active_glb_l2_dly        (l2_acp_flsh_rd_cnt_active_glb_l2_dly),
                            .l2_acp_flsh_wr_cnt_active_glb_l2_dly        (l2_acp_flsh_wr_cnt_active_glb_l2_dly),
                            .l2_acp_rd_haz_vld_l2_dly_q                  (l2_acp_rd_haz_vld_l2_dly_q),
                            .l2_acp_wr_haz_vld_l2_dly_q                  (l2_acp_wr_haz_vld_l2_dly_q),
                            .l2_actlr_disable_b2b_setway_hzd_opt_x2_ns   (l2_actlr_disable_b2b_setway_hzd_opt_x2_ns),
                            .l2_actlr_disable_setway_opt                 (l2_actlr_disable_setway_opt),
                            .l2_actlr_plru_dynamic                       (l2_actlr_plru_dynamic),
                            .l2_actlr_plru_en                            (l2_actlr_plru_en),
                            .l2_actlr_plru_mode                          (l2_actlr_plru_mode[1:0]),
                            .l2_actlr_writeunique_disable                (l2_actlr_writeunique_disable),
                            .l2_cfg_broadcastinner                       (l2_cfg_broadcastinner),
                            .l2_cfg_broadcastouter                       (l2_cfg_broadcastouter),
                            .l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu0_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu0_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu0_ls_rd_haz_vld_l2_dly_q              (l2_cpu0_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu0_ls_wr_haz_vld_l2_dly_q              (l2_cpu0_ls_wr_haz_vld_l2_dly_q),
                            .l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu1_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu1_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu1_ls_rd_haz_vld_l2_dly_q              (l2_cpu1_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu1_ls_wr_haz_vld_l2_dly_q              (l2_cpu1_ls_wr_haz_vld_l2_dly_q),
                            .l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu2_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu2_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu2_ls_rd_haz_vld_l2_dly_q              (l2_cpu2_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu2_ls_wr_haz_vld_l2_dly_q              (l2_cpu2_ls_wr_haz_vld_l2_dly_q),
                            .l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly    (l2_cpu3_tbnk1_flsh_ls_rd_cnt_active_glb_l2_dly),
                            .l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly    (l2_cpu3_tbnk1_flsh_ls_wr_cnt_active_glb_l2_dly),
                            .l2_cpu3_ls_rd_haz_vld_l2_dly_q              (l2_cpu3_ls_rd_haz_vld_l2_dly_q),
                            .l2_cpu3_ls_wr_haz_vld_l2_dly_q              (l2_cpu3_ls_wr_haz_vld_l2_dly_q),
                            .l2_ctlr_x1_wr_q                             (l2_ctlr_x1_wr_q),
                            .l2_ctlr_x2_ns                               (l2_ctlr_x2_ns[9:0]),
                            .l2_mbist2_snp0_outdata_b2                   (l2_mbist2_tbnk1_snp0_outdata_b2[79:0]),
                            .l2_mbist2_snp0_outdata_vld_b2               (l2_mbist2_tbnk1_snp0_outdata_vld_b2),
                            .l2_mbist2_snp1_outdata_b2                   (l2_mbist2_tbnk1_snp1_outdata_b2[79:0]),
                            .l2_mbist2_snp1_outdata_vld_b2               (l2_mbist2_tbnk1_snp1_outdata_vld_b2),
                            .l2_mbist2_snp2_outdata_b2                   (l2_mbist2_tbnk1_snp2_outdata_b2[79:0]),
                            .l2_mbist2_snp2_outdata_vld_b2               (l2_mbist2_tbnk1_snp2_outdata_vld_b2),
                            .l2_mbist2_snp3_outdata_b2                   (l2_mbist2_tbnk1_snp3_outdata_b2[79:0]),
                            .l2_mbist2_snp3_outdata_vld_b2               (l2_mbist2_tbnk1_snp3_outdata_vld_b2),
                            .l2_no_ram_acc_nxt_cycle                     (l2_no_ram_acc_nxt_cycle),
                            .l2_rstdisable_x1_q                          (l2_rstdisable_x1_q),
                            .l2_skyros_intf                              (1'b1),
                            .l2_tbnk_addr_l1                             (l2_tbnk1_addr_l1[44:0]),
                            .l2_tbnk_asq_cmp_evict_l3_q                  (l2_tbnk1_asq_cmp_evict_l3_q),
                            .l2_tbnk_asq_full_flsh                       (l2_tbnk1_asq_full_flsh),
                            .l2_tbnk_asq_nc_so_dev_limit                 (l2_tbnk1_asq_nc_so_dev_limit),
                            .l2_tbnk_cache_attr_l1                       (l2_tbnk1_cache_attr_l1[2:0]),
                            .l2_tbnk_cfg_ecc_en                          (l2_tbnk1_cfg_ecc_en),
                            .l2_tbnk_cpu0_peq_full_q                     (l2_tbnk1_cpu0_peq_full_q),
                            .l2_tbnk_cpu0_peq_hit_q                      (l2_tbnk1_cpu0_peq_hit_q),
                            .l2_tbnk_cpu0_peq_self_evict_l3_q            (l2_tbnk1_cpu0_peq_self_evict_l3_q),
                            .l2_tbnk_cpu0_peq_self_evict_wbna_l3_q       (l2_tbnk1_cpu0_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu0_snp_hit_e_l3                   (l2_tbnk1_cpu0_snp_hit_e_l3),
                            .l2_tbnk_cpu0_snp_hit_s_l3                   (l2_tbnk1_cpu0_snp_hit_s_l3),
                            .l2_tbnk_cpu0_snp_setway_addr_l3             (l2_tbnk1_cpu0_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu0_snp_tag_ecc_err_tp_l4_q        (l2_tbnk1_cpu0_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu0_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk1_cpu0_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_cpu1_peq_full_q                     (l2_tbnk1_cpu1_peq_full_q),
                            .l2_tbnk_cpu1_peq_hit_q                      (l2_tbnk1_cpu1_peq_hit_q),
                            .l2_tbnk_cpu1_peq_self_evict_l3_q            (l2_tbnk1_cpu1_peq_self_evict_l3_q),
                            .l2_tbnk_cpu1_peq_self_evict_wbna_l3_q       (l2_tbnk1_cpu1_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu1_snp_hit_e_l3                   (l2_tbnk1_cpu1_snp_hit_e_l3),
                            .l2_tbnk_cpu1_snp_hit_s_l3                   (l2_tbnk1_cpu1_snp_hit_s_l3),
                            .l2_tbnk_cpu1_snp_setway_addr_l3             (l2_tbnk1_cpu1_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu1_snp_tag_ecc_err_tp_l4_q        (l2_tbnk1_cpu1_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu1_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk1_cpu1_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_cpu2_peq_full_q                     (l2_tbnk1_cpu2_peq_full_q),
                            .l2_tbnk_cpu2_peq_hit_q                      (l2_tbnk1_cpu2_peq_hit_q),
                            .l2_tbnk_cpu2_peq_self_evict_l3_q            (l2_tbnk1_cpu2_peq_self_evict_l3_q),
                            .l2_tbnk_cpu2_peq_self_evict_wbna_l3_q       (l2_tbnk1_cpu2_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu2_snp_hit_e_l3                   (l2_tbnk1_cpu2_snp_hit_e_l3),
                            .l2_tbnk_cpu2_snp_hit_s_l3                   (l2_tbnk1_cpu2_snp_hit_s_l3),
                            .l2_tbnk_cpu2_snp_setway_addr_l3             (l2_tbnk1_cpu2_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu2_snp_tag_ecc_err_tp_l4_q        (l2_tbnk1_cpu2_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu2_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk1_cpu2_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_cpu3_peq_full_q                     (l2_tbnk1_cpu3_peq_full_q),
                            .l2_tbnk_cpu3_peq_hit_q                      (l2_tbnk1_cpu3_peq_hit_q),
                            .l2_tbnk_cpu3_peq_self_evict_l3_q            (l2_tbnk1_cpu3_peq_self_evict_l3_q),
                            .l2_tbnk_cpu3_peq_self_evict_wbna_l3_q       (l2_tbnk1_cpu3_peq_self_evict_wbna_l3_q),
                            .l2_tbnk_cpu3_snp_hit_e_l3                   (l2_tbnk1_cpu3_snp_hit_e_l3),
                            .l2_tbnk_cpu3_snp_hit_s_l3                   (l2_tbnk1_cpu3_snp_hit_s_l3),
                            .l2_tbnk_cpu3_snp_setway_addr_l3             (l2_tbnk1_cpu3_snp_setway_addr_l3[44:14]),
                            .l2_tbnk_cpu3_snp_tag_ecc_err_tp_l4_q        (l2_tbnk1_cpu3_snp_tag_ecc_err_tp_l4_q),
                            .l2_tbnk_cpu3_snp_tag_wr_evict_qual_l4_dly   (l2_tbnk1_cpu3_snp_tag_wr_evict_qual_l4_dly),
                            .l2_tbnk_dirty_l1                            (l2_tbnk1_dirty_l1),
                            .l2_tbnk_dis_ns_dbg_arr_acc_x2               (l2_tbnk1_dis_ns_dbg_arr_acc_x2),
                            .l2_tbnk_excl_l1                             (l2_tbnk1_excl_l1),
                            .l2_tbnk_feq_alloc_failed_l4                 (l2_tbnk1_feq_alloc_failed_l4),
                            .l2_tbnk_feq_axi_wr_vld_not_popped           (l2_tbnk1_feq_axi_wr_vld_not_popped),
                            .l2_tbnk_feq_frc_incl_l3a                    (l2_tbnk1_feq_frc_incl_l3a[15:0]),
                            .l2_tbnk_feq_kill_l3                         (l2_tbnk1_feq_kill_l3),
                            .l2_tbnk_feq_last_id_q                       (l2_tbnk1_feq_last_id_q[4:0]),
                            .l2_tbnk_feq_tbnk_id_update_hit_prfm_or_l3   (l2_tbnk1_feq_tbnk_id_update_hit_prfm_or_l3),
                            .l2_tbnk_feq_tbnk_id_update_or_l3            (l2_tbnk1_feq_tbnk_id_update_or_l3),
                            .l2_tbnk_hwrst_done_x2                       (l2_tbnk_hwrst_done_x2),
                            .l2_tbnk_hwrst_idx_x1_q                      (l2_tbnk_hwrst_idx_x1_q[13:0]),
                            .l2_tbnk_id_l1                               (l2_tbnk1_id_l1[9:0]),
                            .l2_tbnk_init_req_l1                         (l2_tbnk1_init_req_l1),
                            .l2_tbnk_kill_l2                             (l2_tbnk1_kill_l2),
                            .l2_tbnk_l2bb_fake_wr_l1                     (l2_tbnk1_l2bb_fake_wr_l1),
                            .l2_tbnk_l2bb_wr_l1                          (l2_tbnk1_l2bb_wr_l1),
                            .l2_tbnk_last_qw_l1                          (l2_tbnk1_last_qw_l1),
                            .l2_tbnk_lock_l1                             (l2_tbnk1_lock_l1[2:0]),
                            .l2_tbnk_page_attr_l1                        (l2_tbnk1_page_attr_l1[9:0]),
                            .l2_tbnk_partial_dw_wr_l1                    (l2_tbnk1_partial_dw_wr_l1),
                            .l2_tbnk_pf_hazard_l3                        (l2_tbnk1_pf_hazard_l3),
                            .l2_tbnk_prfm_l1                             (l2_tbnk1_prfm_l1),
                            .l2_tbnk_prot_l1                             (l2_tbnk1_prot_l1[3:0]),
                            .l2_tbnk_qw_cnt_l1                           (l2_tbnk1_qw_cnt_l1[1:0]),
                            .l2_tbnk_rd_fail_hazchk_feq_l3               (l2_tbnk1_rd_fail_hazchk_feq_l3),
                            .l2_tbnk_rwvic_axi_read_err_l1               (l2_tbnk1_rwvic_axi_read_err_l1),
                            .l2_tbnk_rwvic_ccb_ls_xfer_l1                (l2_tbnk1_rwvic_ccb_ls_xfer_l1),
                            .l2_tbnk_rwvic_ccb_way_l1                    (l2_tbnk1_rwvic_ccb_way_l1[3:0]),
                            .l2_tbnk_rwvic_cmo_clean_l1                  (l2_tbnk1_rwvic_cmo_clean_l1),
                            .l2_tbnk_rwvic_cmo_inv_l1                    (l2_tbnk1_rwvic_cmo_inv_l1),
                            .l2_tbnk_rwvic_cmo_pou_l1                    (l2_tbnk1_rwvic_cmo_pou_l1),
                            .l2_tbnk_rwvic_cmo_setway_l1                 (l2_tbnk1_rwvic_cmo_setway_l1),
                            .l2_tbnk_rwvic_cmo_setway_ls_full_miss_l1    (l2_tbnk1_rwvic_cmo_setway_ls_full_miss_l1),
                            .l2_tbnk_rwvic_cpu_fb_id_l1                  (l2_tbnk1_rwvic_cpu_fb_id_l1[2:0]),
                            .l2_tbnk_rwvic_cpu_id_dcd_l1                 (l2_tbnk1_rwvic_cpu_id_dcd_l1[3:0]),
                            .l2_tbnk_rwvic_feq_cmp_l3_q                  (l2_tbnk1_rwvic_feq_cmp_l3_q),
                            .l2_tbnk_rwvic_frc_l2hit_fwd_l1              (l2_tbnk1_rwvic_frc_l2hit_fwd_l1),
                            .l2_tbnk_rwvic_l2hit_e_l1                    (l2_tbnk1_rwvic_l2hit_e_l1),
                            .l2_tbnk_rwvic_mesi_sh_l1                    (l2_tbnk1_rwvic_mesi_sh_l1),
                            .l2_tbnk_rwvic_owner_l1                      (l2_tbnk1_rwvic_owner_l1[2:0]),
                            .l2_tbnk_rwvic_snp_clr_dirty_l1              (l2_tbnk1_rwvic_snp_clr_dirty_l1),
                            .l2_tbnk_rwvic_snp_inv_l1                    (l2_tbnk1_rwvic_snp_inv_l1),
                            .l2_tbnk_rwvic_snp_l1                        (l2_tbnk1_rwvic_snp_l1),
                            .l2_tbnk_rwvic_type_l1                       (l2_tbnk1_rwvic_type_l1[3:0]),
                            .l2_tbnk_rwvic_wa_l1                         (l2_tbnk1_rwvic_wa_l1),
                            .l2_tbnk_sel_l1                              (l2_tbnk1_sel_l1[13:0]),
                            .l2_tbnk_size_l1                             (l2_tbnk1_size_l1[2:0]),
                            .l2_tbnk_snp_byp_peq_haz_pending_q           (l2_tbnk1_snp_byp_peq_haz_pending_q),
                            .l2_tbnk_snp_dvm_cmpl_l1                     (l2_tbnk1_snp_dvm_cmpl_l1),
                            .l2_tbnk_snp_hit_feq_evict_l4_dly            (l2_tbnk1_snp_hit_feq_evict_l4_dly),
                            .l2_tbnk_snp_rd_feq_id_cmp_l4_dly_q          (l2_tbnk1_snp_rd_feq_id_cmp_l4_dly_q[4:0]),
                            .l2_tbnk_snp_tag_double_ecc_err_l4_dly_q     (l2_tbnk1_snp_tag_double_ecc_err_l4_dly_q[7:0]),
                            .l2_tbnk_snp_tag_single_ecc_err_l4_dly_q     (l2_tbnk1_snp_tag_single_ecc_err_l4_dly_q[7:0]),
                            .l2_tbnk_sync_l1                             (l2_tbnk1_sync_l1),
                            .l2_tbnk_type_l1                             (l2_tbnk1_type_l1[6:0]),
                            .l2_tbnk_ulen_l1                             (l2_tbnk1_ulen_l1[1:0]),
                            .l2_tbnk_way_l1                              (l2_tbnk1_way_l1),
                            .l2_tbnk_wr_data_l3a_q                       (l2_tbnk1_wr_data_l3a_q[127:0]),
                            .l2_tbnk_wr_err_l1                           (l2_tbnk1_wr_err_l1),
                            .l2_tbnk_wr_fail_feq_full_l3                 (l2_tbnk1_wr_fail_feq_full_l3),
                            .l2_tbnk_wr_fail_hazchk_feq_l3               (l2_tbnk1_wr_fail_hazchk_feq_l3),
                            .l2_tbnk_wr_non_crit_id_l1                   (l2_tbnk1_wr_non_crit_id_l1[11:0]),
                            .l2_tbnk_wr_strb_mask_l3a_q                  (l2_tbnk1_wr_strb_mask_l3a_q[15:0])
                          );    // ul2_tbnk1

    maia_dt_pclk udt_pclk( // outputs
                           .CTICHINACK                             (CTICHINACK[3:0]),
                           .CTICHOUT                               (CTICHOUT[3:0]),
                           .CTIIRQ                                 (CTIIRQ[`MAIA_CN:0]),
                           .DBGPWRUPREQ                            (DBGPWRUPREQ[`MAIA_CN:0]),
                           .PMUSNAPSHOTACK                         (PMUSNAPSHOTACK[`MAIA_CN:0]),
                           .PRDATADBG                              (PRDATADBG[31:0]),
                           .PREADYDBG                              (PREADYDBG),
                           .PSLVERRDBG                             (PSLVERRDBG),
                           .dt_cpu0_apb_active_pclk                (dt_cpu0_apb_active_pclk),
                           .dt_cpu0_cti_triginack_1to0_pclk        (dt_cpu0_cti_triginack_1to0_pclk[1:0]),
                           .dt_cpu0_cti_triginack_7to4_pclk        (dt_cpu0_cti_triginack_7to4_pclk[3:0]),
                           .dt_cpu0_cti_trigout_1to0_pclk          (dt_cpu0_cti_trigout_1to0_pclk[1:0]),
                           .dt_cpu0_cti_trigout_7to4_pclk          (dt_cpu0_cti_trigout_7to4_pclk[3:0]),
                           .dt_cpu0_dbif_addr_pclk                 (dt_cpu0_dbif_addr_pclk[14:2]),
                           .dt_cpu0_dbif_locked_pclk               (dt_cpu0_dbif_locked_pclk),
                           .dt_cpu0_dbif_req_pclk                  (dt_cpu0_dbif_req_pclk),
                           .dt_cpu0_dbif_wrdata_pclk               (dt_cpu0_dbif_wrdata_pclk[31:0]),
                           .dt_cpu0_dbif_write_pclk                (dt_cpu0_dbif_write_pclk),
                           .dt_cpu0_edacr_frc_idleack_pclk         (dt_cpu0_edacr_frc_idleack_pclk),
                           .dt_cpu0_edbgrq_pclk                    (dt_cpu0_edbgrq_pclk),
                           .dt_cpu0_edecr_osuce_pclk               (dt_cpu0_edecr_osuce_pclk),
                           .dt_cpu0_edecr_rce_pclk                 (dt_cpu0_edecr_rce_pclk),
                           .dt_cpu0_edecr_ss_pclk                  (dt_cpu0_edecr_ss_pclk),
                           .dt_cpu0_edprcr_corepurq_pclk           (dt_cpu0_edprcr_corepurq_pclk),
                           .dt_cpu0_noclkstop_pclk                 (dt_cpu0_noclkstop_pclk),
                           .dt_cpu0_pmusnapshot_req_pclk           (dt_cpu0_pmusnapshot_req_pclk),
                           .dt_cpu0_poreset_status_ack_pclk        (dt_cpu0_poreset_status_ack_pclk),
                           .dt_cpu0_trcauxctlr_sb_rcg_disable_pclk (dt_cpu0_trcauxctlr_sb_rcg_disable_pclk),
                           .dt_cpu0_wfx_wakeup_pclk                (dt_cpu0_wfx_wakeup_pclk),
                           .dt_cpu1_apb_active_pclk                (dt_cpu1_apb_active_pclk),
                           .dt_cpu1_cti_triginack_1to0_pclk        (dt_cpu1_cti_triginack_1to0_pclk[1:0]),
                           .dt_cpu1_cti_triginack_7to4_pclk        (dt_cpu1_cti_triginack_7to4_pclk[3:0]),
                           .dt_cpu1_cti_trigout_1to0_pclk          (dt_cpu1_cti_trigout_1to0_pclk[1:0]),
                           .dt_cpu1_cti_trigout_7to4_pclk          (dt_cpu1_cti_trigout_7to4_pclk[3:0]),
                           .dt_cpu1_dbif_addr_pclk                 (dt_cpu1_dbif_addr_pclk[14:2]),
                           .dt_cpu1_dbif_locked_pclk               (dt_cpu1_dbif_locked_pclk),
                           .dt_cpu1_dbif_req_pclk                  (dt_cpu1_dbif_req_pclk),
                           .dt_cpu1_dbif_wrdata_pclk               (dt_cpu1_dbif_wrdata_pclk[31:0]),
                           .dt_cpu1_dbif_write_pclk                (dt_cpu1_dbif_write_pclk),
                           .dt_cpu1_edacr_frc_idleack_pclk         (dt_cpu1_edacr_frc_idleack_pclk),
                           .dt_cpu1_edbgrq_pclk                    (dt_cpu1_edbgrq_pclk),
                           .dt_cpu1_edecr_osuce_pclk               (dt_cpu1_edecr_osuce_pclk),
                           .dt_cpu1_edecr_rce_pclk                 (dt_cpu1_edecr_rce_pclk),
                           .dt_cpu1_edecr_ss_pclk                  (dt_cpu1_edecr_ss_pclk),
                           .dt_cpu1_edprcr_corepurq_pclk           (dt_cpu1_edprcr_corepurq_pclk),
                           .dt_cpu1_noclkstop_pclk                 (dt_cpu1_noclkstop_pclk),
                           .dt_cpu1_pmusnapshot_req_pclk           (dt_cpu1_pmusnapshot_req_pclk),
                           .dt_cpu1_poreset_status_ack_pclk        (dt_cpu1_poreset_status_ack_pclk),
                           .dt_cpu1_trcauxctlr_sb_rcg_disable_pclk (dt_cpu1_trcauxctlr_sb_rcg_disable_pclk),
                           .dt_cpu1_wfx_wakeup_pclk                (dt_cpu1_wfx_wakeup_pclk),
                           .dt_cpu2_apb_active_pclk                (dt_cpu2_apb_active_pclk),
                           .dt_cpu2_cti_triginack_1to0_pclk        (dt_cpu2_cti_triginack_1to0_pclk[1:0]),
                           .dt_cpu2_cti_triginack_7to4_pclk        (dt_cpu2_cti_triginack_7to4_pclk[3:0]),
                           .dt_cpu2_cti_trigout_1to0_pclk          (dt_cpu2_cti_trigout_1to0_pclk[1:0]),
                           .dt_cpu2_cti_trigout_7to4_pclk          (dt_cpu2_cti_trigout_7to4_pclk[3:0]),
                           .dt_cpu2_dbif_addr_pclk                 (dt_cpu2_dbif_addr_pclk[14:2]),
                           .dt_cpu2_dbif_locked_pclk               (dt_cpu2_dbif_locked_pclk),
                           .dt_cpu2_dbif_req_pclk                  (dt_cpu2_dbif_req_pclk),
                           .dt_cpu2_dbif_wrdata_pclk               (dt_cpu2_dbif_wrdata_pclk[31:0]),
                           .dt_cpu2_dbif_write_pclk                (dt_cpu2_dbif_write_pclk),
                           .dt_cpu2_edacr_frc_idleack_pclk         (dt_cpu2_edacr_frc_idleack_pclk),
                           .dt_cpu2_edbgrq_pclk                    (dt_cpu2_edbgrq_pclk),
                           .dt_cpu2_edecr_osuce_pclk               (dt_cpu2_edecr_osuce_pclk),
                           .dt_cpu2_edecr_rce_pclk                 (dt_cpu2_edecr_rce_pclk),
                           .dt_cpu2_edecr_ss_pclk                  (dt_cpu2_edecr_ss_pclk),
                           .dt_cpu2_edprcr_corepurq_pclk           (dt_cpu2_edprcr_corepurq_pclk),
                           .dt_cpu2_noclkstop_pclk                 (dt_cpu2_noclkstop_pclk),
                           .dt_cpu2_pmusnapshot_req_pclk           (dt_cpu2_pmusnapshot_req_pclk),
                           .dt_cpu2_poreset_status_ack_pclk        (dt_cpu2_poreset_status_ack_pclk),
                           .dt_cpu2_trcauxctlr_sb_rcg_disable_pclk (dt_cpu2_trcauxctlr_sb_rcg_disable_pclk),
                           .dt_cpu2_wfx_wakeup_pclk                (dt_cpu2_wfx_wakeup_pclk),
                           .dt_cpu3_apb_active_pclk                (dt_cpu3_apb_active_pclk),
                           .dt_cpu3_cti_triginack_1to0_pclk        (dt_cpu3_cti_triginack_1to0_pclk[1:0]),
                           .dt_cpu3_cti_triginack_7to4_pclk        (dt_cpu3_cti_triginack_7to4_pclk[3:0]),
                           .dt_cpu3_cti_trigout_1to0_pclk          (dt_cpu3_cti_trigout_1to0_pclk[1:0]),
                           .dt_cpu3_cti_trigout_7to4_pclk          (dt_cpu3_cti_trigout_7to4_pclk[3:0]),
                           .dt_cpu3_dbif_addr_pclk                 (dt_cpu3_dbif_addr_pclk[14:2]),
                           .dt_cpu3_dbif_locked_pclk               (dt_cpu3_dbif_locked_pclk),
                           .dt_cpu3_dbif_req_pclk                  (dt_cpu3_dbif_req_pclk),
                           .dt_cpu3_dbif_wrdata_pclk               (dt_cpu3_dbif_wrdata_pclk[31:0]),
                           .dt_cpu3_dbif_write_pclk                (dt_cpu3_dbif_write_pclk),
                           .dt_cpu3_edacr_frc_idleack_pclk         (dt_cpu3_edacr_frc_idleack_pclk),
                           .dt_cpu3_edbgrq_pclk                    (dt_cpu3_edbgrq_pclk),
                           .dt_cpu3_edecr_osuce_pclk               (dt_cpu3_edecr_osuce_pclk),
                           .dt_cpu3_edecr_rce_pclk                 (dt_cpu3_edecr_rce_pclk),
                           .dt_cpu3_edecr_ss_pclk                  (dt_cpu3_edecr_ss_pclk),
                           .dt_cpu3_edprcr_corepurq_pclk           (dt_cpu3_edprcr_corepurq_pclk),
                           .dt_cpu3_noclkstop_pclk                 (dt_cpu3_noclkstop_pclk),
                           .dt_cpu3_pmusnapshot_req_pclk           (dt_cpu3_pmusnapshot_req_pclk),
                           .dt_cpu3_poreset_status_ack_pclk        (dt_cpu3_poreset_status_ack_pclk),
                           .dt_cpu3_trcauxctlr_sb_rcg_disable_pclk (dt_cpu3_trcauxctlr_sb_rcg_disable_pclk),
                           .dt_cpu3_wfx_wakeup_pclk                (dt_cpu3_wfx_wakeup_pclk),

                           // inputs
                           .CIHSBYPASS                             (CIHSBYPASS[3:0]),
                           .CISBYPASS                              (CISBYPASS),
                           .CLUSTERIDAFF1                          (CLUSTERIDAFF1[7:0]),
                           .CLUSTERIDAFF2                          (CLUSTERIDAFF2[7:0]),
                           .CRYPTODISABLE                          (CRYPTODISABLE[`MAIA_CN:0]),
                           .CTICHIN                                (CTICHIN[3:0]),
                           .CTICHOUTACK                            (CTICHOUTACK[3:0]),
                           .CTIIRQACK                              (CTIIRQACK[`MAIA_CN:0]),
                           .DBGEN                                  (DBGEN[`MAIA_CN:0]),
                           .DBGPWRDUP                              (DBGPWRDUP[`MAIA_CN:0]),
                           .DFTRSTDISABLE                          (DFTRSTDISABLE),
                           .EDBGRQ                                 (EDBGRQ[`MAIA_CN:0]),
                           .GICCDISABLE                            (GICCDISABLE),
                           .NIDEN                                  (NIDEN[`MAIA_CN:0]),
                           .PADDRDBG                               (PADDRDBG[21:2]),
                           .PADDRDBG31                             (PADDRDBG31),
                           .PCLKDBG                                (PCLKDBG),
                           .PCLKENDBG                              (PCLKENDBG),
                           .PENABLEDBG                             (PENABLEDBG),
                           .PMUSNAPSHOTREQ                         (PMUSNAPSHOTREQ[`MAIA_CN:0]),
                           .PSELDBG                                (PSELDBG),
                           .PWDATADBG                              (PWDATADBG[31:0]),
                           .PWRITEDBG                              (PWRITEDBG),
                           .SPIDEN                                 (SPIDEN[`MAIA_CN:0]),
                           .SPNIDEN                                (SPNIDEN[`MAIA_CN:0]),
                           .ck_cpu0_dt_standbywfx                  (ck_cpu0_dt_standbywfx),
                           .ck_cpu0_dt_wfx_ack                     (ck_cpu0_dt_wfx_ack),
                           .ck_cpu0_poreset_status                 (ck_cpu0_poreset_status),
                           .ck_cpu1_dt_standbywfx                  (ck_cpu1_dt_standbywfx),
                           .ck_cpu1_dt_wfx_ack                     (ck_cpu1_dt_wfx_ack),
                           .ck_cpu1_poreset_status                 (ck_cpu1_poreset_status),
                           .ck_cpu2_dt_standbywfx                  (ck_cpu2_dt_standbywfx),
                           .ck_cpu2_dt_wfx_ack                     (ck_cpu2_dt_wfx_ack),
                           .ck_cpu2_poreset_status                 (ck_cpu2_poreset_status),
                           .ck_cpu3_dt_standbywfx                  (ck_cpu3_dt_standbywfx),
                           .ck_cpu3_dt_wfx_ack                     (ck_cpu3_dt_wfx_ack),
                           .ck_cpu3_poreset_status                 (ck_cpu3_poreset_status),
                           .ck_dt_cpu0_coredbg_in_reset_gclk       (ck_dt_cpu0_coredbg_in_reset_gclk),
                           .ck_dt_cpu0_cti_trigin_1to0_gclk        (ck_dt_cpu0_cti_trigin_1to0_gclk[1:0]),
                           .ck_dt_cpu0_et_oslock_gclk              (ck_dt_cpu0_et_oslock_gclk),
                           .ck_dt_cpu0_hlt_dbgevt_ok_gclk          (ck_dt_cpu0_hlt_dbgevt_ok_gclk),
                           .ck_dt_cpu0_os_double_lock_gclk         (ck_dt_cpu0_os_double_lock_gclk),
                           .ck_dt_cpu0_pmusnapshot_ack_gclk        (ck_dt_cpu0_pmusnapshot_ack_gclk),
                           .ck_dt_cpu0_wfx_dbg_req_gclk            (ck_dt_cpu0_wfx_dbg_req_gclk),
                           .ck_dt_cpu1_coredbg_in_reset_gclk       (ck_dt_cpu1_coredbg_in_reset_gclk),
                           .ck_dt_cpu1_cti_trigin_1to0_gclk        (ck_dt_cpu1_cti_trigin_1to0_gclk[1:0]),
                           .ck_dt_cpu1_et_oslock_gclk              (ck_dt_cpu1_et_oslock_gclk),
                           .ck_dt_cpu1_hlt_dbgevt_ok_gclk          (ck_dt_cpu1_hlt_dbgevt_ok_gclk),
                           .ck_dt_cpu1_os_double_lock_gclk         (ck_dt_cpu1_os_double_lock_gclk),
                           .ck_dt_cpu1_pmusnapshot_ack_gclk        (ck_dt_cpu1_pmusnapshot_ack_gclk),
                           .ck_dt_cpu1_wfx_dbg_req_gclk            (ck_dt_cpu1_wfx_dbg_req_gclk),
                           .ck_dt_cpu2_coredbg_in_reset_gclk       (ck_dt_cpu2_coredbg_in_reset_gclk),
                           .ck_dt_cpu2_cti_trigin_1to0_gclk        (ck_dt_cpu2_cti_trigin_1to0_gclk[1:0]),
                           .ck_dt_cpu2_et_oslock_gclk              (ck_dt_cpu2_et_oslock_gclk),
                           .ck_dt_cpu2_hlt_dbgevt_ok_gclk          (ck_dt_cpu2_hlt_dbgevt_ok_gclk),
                           .ck_dt_cpu2_os_double_lock_gclk         (ck_dt_cpu2_os_double_lock_gclk),
                           .ck_dt_cpu2_pmusnapshot_ack_gclk        (ck_dt_cpu2_pmusnapshot_ack_gclk),
                           .ck_dt_cpu2_wfx_dbg_req_gclk            (ck_dt_cpu2_wfx_dbg_req_gclk),
                           .ck_dt_cpu3_coredbg_in_reset_gclk       (ck_dt_cpu3_coredbg_in_reset_gclk),
                           .ck_dt_cpu3_cti_trigin_1to0_gclk        (ck_dt_cpu3_cti_trigin_1to0_gclk[1:0]),
                           .ck_dt_cpu3_et_oslock_gclk              (ck_dt_cpu3_et_oslock_gclk),
                           .ck_dt_cpu3_hlt_dbgevt_ok_gclk          (ck_dt_cpu3_hlt_dbgevt_ok_gclk),
                           .ck_dt_cpu3_os_double_lock_gclk         (ck_dt_cpu3_os_double_lock_gclk),
                           .ck_dt_cpu3_pmusnapshot_ack_gclk        (ck_dt_cpu3_pmusnapshot_ack_gclk),
                           .ck_dt_cpu3_wfx_dbg_req_gclk            (ck_dt_cpu3_wfx_dbg_req_gclk),
                           .dt_cpu0_cti_trigin_7to4_gclk           (dt_cpu0_cti_trigin_7to4_gclk[3:0]),
                           .dt_cpu0_cti_trigoutack_7to4_gclk       (dt_cpu0_cti_trigoutack_7to4_gclk[3:0]),
                           .dt_cpu0_cti_trigoutack_bit1_gclk       (dt_cpu0_cti_trigoutack_bit1_gclk),
                           .dt_cpu0_dbif_ack_gclk                  (dt_cpu0_dbif_ack_gclk),
                           .dt_cpu0_dbif_err_gclk                  (dt_cpu0_dbif_err_gclk),
                           .dt_cpu0_dbif_rddata_gclk               (dt_cpu0_dbif_rddata_gclk[31:0]),
                           .dt_cpu0_halt_ack_gclk                  (dt_cpu0_halt_ack_gclk),
                           .dt_cpu1_cti_trigin_7to4_gclk           (dt_cpu1_cti_trigin_7to4_gclk[3:0]),
                           .dt_cpu1_cti_trigoutack_7to4_gclk       (dt_cpu1_cti_trigoutack_7to4_gclk[3:0]),
                           .dt_cpu1_cti_trigoutack_bit1_gclk       (dt_cpu1_cti_trigoutack_bit1_gclk),
                           .dt_cpu1_dbif_ack_gclk                  (dt_cpu1_dbif_ack_gclk),
                           .dt_cpu1_dbif_err_gclk                  (dt_cpu1_dbif_err_gclk),
                           .dt_cpu1_dbif_rddata_gclk               (dt_cpu1_dbif_rddata_gclk[31:0]),
                           .dt_cpu1_halt_ack_gclk                  (dt_cpu1_halt_ack_gclk),
                           .dt_cpu2_cti_trigin_7to4_gclk           (dt_cpu2_cti_trigin_7to4_gclk[3:0]),
                           .dt_cpu2_cti_trigoutack_7to4_gclk       (dt_cpu2_cti_trigoutack_7to4_gclk[3:0]),
                           .dt_cpu2_cti_trigoutack_bit1_gclk       (dt_cpu2_cti_trigoutack_bit1_gclk),
                           .dt_cpu2_dbif_ack_gclk                  (dt_cpu2_dbif_ack_gclk),
                           .dt_cpu2_dbif_err_gclk                  (dt_cpu2_dbif_err_gclk),
                           .dt_cpu2_dbif_rddata_gclk               (dt_cpu2_dbif_rddata_gclk[31:0]),
                           .dt_cpu2_halt_ack_gclk                  (dt_cpu2_halt_ack_gclk),
                           .dt_cpu3_cti_trigin_7to4_gclk           (dt_cpu3_cti_trigin_7to4_gclk[3:0]),
                           .dt_cpu3_cti_trigoutack_7to4_gclk       (dt_cpu3_cti_trigoutack_7to4_gclk[3:0]),
                           .dt_cpu3_cti_trigoutack_bit1_gclk       (dt_cpu3_cti_trigoutack_bit1_gclk),
                           .dt_cpu3_dbif_ack_gclk                  (dt_cpu3_dbif_ack_gclk),
                           .dt_cpu3_dbif_err_gclk                  (dt_cpu3_dbif_err_gclk),
                           .dt_cpu3_dbif_rddata_gclk               (dt_cpu3_dbif_rddata_gclk[31:0]),
                           .dt_cpu3_halt_ack_gclk                  (dt_cpu3_halt_ack_gclk),
                           .nPRESETDBG                             (nPRESETDBG)
                         );    // udt_pclk

    maia_intctrl uic( // outputs
                      .ICCTDATA                      (ICCTDATA[15:0]),
                      .ICCTID                        (ICCTID[1:0]),
                      .ICCTLAST                      (ICCTLAST),
                      .ICCTVALID                     (ICCTVALID),
                      .ICDTREADY                     (ICDTREADY),
                      .ic_block_eoi_sgi_wr           (ic_block_eoi_sgi_wr_o[`MAIA_CN:0]),
                      .ic_cpu0_l2_dsb_block          (ic_cpu0_l2_dsb_block),
                      .ic_cpu0_spr_rd_data           (ic_cpu0_spr_rd_data[63:0]),
                      .ic_cpu1_l2_dsb_block          (ic_cpu1_l2_dsb_block),
                      .ic_cpu1_spr_rd_data           (ic_cpu1_spr_rd_data[63:0]),
                      .ic_cpu2_l2_dsb_block          (ic_cpu2_l2_dsb_block),
                      .ic_cpu2_spr_rd_data           (ic_cpu2_spr_rd_data[63:0]),
                      .ic_cpu3_l2_dsb_block          (ic_cpu3_l2_dsb_block),
                      .ic_cpu3_spr_rd_data           (ic_cpu3_spr_rd_data[63:0]),
                      .ic_el_change_complete_o       (ic_el_change_complete_o[`MAIA_CN:0]),
                      .ic_hcr_change_complete_o      (ic_hcr_change_complete_o[`MAIA_CN:0]),
                      .ic_ich_el2_tall0              (ic_ich_el2_tall0_o[`MAIA_CN:0]),
                      .ic_ich_el2_tall1              (ic_ich_el2_tall1_o[`MAIA_CN:0]),
                      .ic_ich_el2_tc                 (ic_ich_el2_tc_o[`MAIA_CN:0]),
                      .ic_nfiq                       (ic_nfiq_o[`MAIA_CN:0]),
                      .ic_nirq                       (ic_nirq_o[`MAIA_CN:0]),
                      .ic_nsei                       (ic_nsei_o[`MAIA_CN:0]),
                      .ic_nvfiq                      (ic_nvfiq_o[`MAIA_CN:0]),
                      .ic_nvirq                      (ic_nvirq_o[`MAIA_CN:0]),
                      .ic_nvsei                      (ic_nvsei_o[`MAIA_CN:0]),
                      .ic_p_rdata                    (ic_p_rdata[31:0]),
                      .ic_p_rdata_valid              (ic_p_rdata_valid),
                      .ic_p_ready                    (ic_p_ready),
                      .ic_p_valid                    (ic_p_valid[`MAIA_CN:0]),
                      .ic_sample_spr_o               (ic_sample_spr_o[`MAIA_CN:0]),
                      .ic_scr_change_complete_o      (ic_scr_change_complete_o[`MAIA_CN:0]),
                      .ic_sra_el1ns_en               (ic_sra_el1ns_en_o[`MAIA_CN:0]),
                      .ic_sra_el1s_en                (ic_sra_el1s_en_o[`MAIA_CN:0]),
                      .ic_sra_el2_en                 (ic_sra_el2_en_o[`MAIA_CN:0]),
                      .ic_sra_el3_en                 (ic_sra_el3_en_o[`MAIA_CN:0]),
                      .ic_sre_el1ns_hyp_trap         (ic_sre_el1ns_hyp_trap_o[`MAIA_CN:0]),
                      .ic_sre_el1ns_mon_trap         (ic_sre_el1ns_mon_trap_o[`MAIA_CN:0]),
                      .ic_sre_el1s_mon_trap          (ic_sre_el1s_mon_trap_o[`MAIA_CN:0]),
                      .ic_sre_el2_mon_trap           (ic_sre_el2_mon_trap_o[`MAIA_CN:0]),
                      .nVCPUMNTIRQ                   (nVCPUMNTIRQ[`MAIA_CN:0]),

                      // inputs
                      .DFTRSTDISABLE                 (DFTRSTDISABLE),
                      .DFTSE                         (DFTSE),
                      .GICCDISABLE                   (GICCDISABLE),
                      .ICCTREADY                     (ICCTREADY),
                      .ICDTDATA                      (ICDTDATA[15:0]),
                      .ICDTDEST                      (ICDTDEST[1:0]),
                      .ICDTLAST                      (ICDTLAST),
                      .ICDTVALID                     (ICDTVALID),
                      .ck_areset_l2                  (ck_areset_l2),
                      .ck_cpu0_areset_l2cpu          (ck_cpu0_areset_l2cpu),
                      .ck_cpu0_crcx_clk_en_n_ic      (ck_cpu0_crcx_clk_en_n_ic),
                      .ck_cpu0_reset1_n_l2cpu        (ck_cpu0_reset1_n_l2cpu),
                      .ck_cpu1_areset_l2cpu          (ck_cpu1_areset_l2cpu),
                      .ck_cpu1_crcx_clk_en_n_ic      (ck_cpu1_crcx_clk_en_n_ic),
                      .ck_cpu1_reset1_n_l2cpu        (ck_cpu1_reset1_n_l2cpu),
                      .ck_cpu2_areset_l2cpu          (ck_cpu2_areset_l2cpu),
                      .ck_cpu2_crcx_clk_en_n_ic      (ck_cpu2_crcx_clk_en_n_ic),
                      .ck_cpu2_reset1_n_l2cpu        (ck_cpu2_reset1_n_l2cpu),
                      .ck_cpu3_areset_l2cpu          (ck_cpu3_areset_l2cpu),
                      .ck_cpu3_crcx_clk_en_n_ic      (ck_cpu3_crcx_clk_en_n_ic),
                      .ck_cpu3_reset1_n_l2cpu        (ck_cpu3_reset1_n_l2cpu),
                      .ck_gclkfr                     (ck_gclkfr),
                      .ck_reset1_n_l2                (ck_reset1_n_l2),
                      .ds_cpu0_aa64naa32_i           (ds_cpu0_ic_aa64naa32_i),
                      .ds_cpu0_cpsr_mode_i           (ds_cpu0_ic_cpsr_mode_i[4:0]),
                      .ds_cpu0_hcr_change_i          (ds_cpu0_ic_hcr_change_i),
                      .ds_cpu0_hcr_va                (ds_cpu0_hcr_va),
                      .ds_cpu0_l2_spr_addr           (ds_cpu0_l2_spr_addr[8:0]),
                      .ds_cpu0_l2_spr_dw             (ds_cpu0_l2_spr_dw),
                      .ds_cpu0_l2_spr_rd             (ds_cpu0_l2_spr_rd),
                      .ds_cpu0_l2_spr_wr             (ds_cpu0_l2_spr_wr),
                      .ds_cpu0_l2_spr_wr_data        (ds_cpu0_l2_spr_wr_data[63:0]),
                      .ds_cpu0_sample_spr_i          (ds_cpu0_ic_sample_spr_i),
                      .ds_cpu0_scr_change_i          (ds_cpu0_ic_scr_change_i),
                      .ds_cpu1_aa64naa32_i           (ds_cpu1_ic_aa64naa32_i),
                      .ds_cpu1_cpsr_mode_i           (ds_cpu1_ic_cpsr_mode_i[4:0]),
                      .ds_cpu1_hcr_change_i          (ds_cpu1_ic_hcr_change_i),
                      .ds_cpu1_hcr_va                (ds_cpu1_hcr_va),
                      .ds_cpu1_l2_spr_addr           (ds_cpu1_l2_spr_addr[8:0]),
                      .ds_cpu1_l2_spr_dw             (ds_cpu1_l2_spr_dw),
                      .ds_cpu1_l2_spr_rd             (ds_cpu1_l2_spr_rd),
                      .ds_cpu1_l2_spr_wr             (ds_cpu1_l2_spr_wr),
                      .ds_cpu1_l2_spr_wr_data        (ds_cpu1_l2_spr_wr_data[63:0]),
                      .ds_cpu1_sample_spr_i          (ds_cpu1_ic_sample_spr_i),
                      .ds_cpu1_scr_change_i          (ds_cpu1_ic_scr_change_i),
                      .ds_cpu2_aa64naa32_i           (ds_cpu2_ic_aa64naa32_i),
                      .ds_cpu2_cpsr_mode_i           (ds_cpu2_ic_cpsr_mode_i[4:0]),
                      .ds_cpu2_hcr_change_i          (ds_cpu2_ic_hcr_change_i),
                      .ds_cpu2_hcr_va                (ds_cpu2_hcr_va),
                      .ds_cpu2_l2_spr_addr           (ds_cpu2_l2_spr_addr[8:0]),
                      .ds_cpu2_l2_spr_dw             (ds_cpu2_l2_spr_dw),
                      .ds_cpu2_l2_spr_rd             (ds_cpu2_l2_spr_rd),
                      .ds_cpu2_l2_spr_wr             (ds_cpu2_l2_spr_wr),
                      .ds_cpu2_l2_spr_wr_data        (ds_cpu2_l2_spr_wr_data[63:0]),
                      .ds_cpu2_sample_spr_i          (ds_cpu2_ic_sample_spr_i),
                      .ds_cpu2_scr_change_i          (ds_cpu2_ic_scr_change_i),
                      .ds_cpu3_aa64naa32_i           (ds_cpu3_ic_aa64naa32_i),
                      .ds_cpu3_cpsr_mode_i           (ds_cpu3_ic_cpsr_mode_i[4:0]),
                      .ds_cpu3_hcr_change_i          (ds_cpu3_ic_hcr_change_i),
                      .ds_cpu3_hcr_va                (ds_cpu3_hcr_va),
                      .ds_cpu3_l2_spr_addr           (ds_cpu3_l2_spr_addr[8:0]),
                      .ds_cpu3_l2_spr_dw             (ds_cpu3_l2_spr_dw),
                      .ds_cpu3_l2_spr_rd             (ds_cpu3_l2_spr_rd),
                      .ds_cpu3_l2_spr_wr             (ds_cpu3_l2_spr_wr),
                      .ds_cpu3_l2_spr_wr_data        (ds_cpu3_l2_spr_wr_data[63:0]),
                      .ds_cpu3_sample_spr_i          (ds_cpu3_ic_sample_spr_i),
                      .ds_cpu3_scr_change_i          (ds_cpu3_ic_scr_change_i),
                      .l2_actlr_ncpu_rcg_enable      (l2_actlr_ncpu_rcg_enable),
                      .l2_cpu0_wr_decerr_i           (l2_cpu0_wr_decerr_q),
                      .l2_cpu0_wr_slverr_i           (l2_cpu0_wr_slverr_q),
                      .l2_cpu1_wr_decerr_i           (l2_cpu1_wr_decerr_q),
                      .l2_cpu1_wr_slverr_i           (l2_cpu1_wr_slverr_q),
                      .l2_cpu2_wr_decerr_i           (l2_cpu2_wr_decerr_q),
                      .l2_cpu2_wr_slverr_i           (l2_cpu2_wr_slverr_q),
                      .l2_cpu3_wr_decerr_i           (l2_cpu3_wr_decerr_q),
                      .l2_cpu3_wr_slverr_i           (l2_cpu3_wr_slverr_q),
                      .l2_p_addr                     (l2_p_addr[13:0]),
                      .l2_p_cpu                      (l2_p_cpu[1:0]),
                      .l2_p_nsecure                  (l2_p_nsecure),
                      .l2_p_sel                      (l2_p_sel[2:0]),
                      .l2_p_wdata                    (l2_p_wdata[31:0]),
                      .l2_p_write                    (l2_p_write),
                      .ls_cpu0_imp_abort_containable (ls_cpu0_imp_abort_containable),
                      .ls_cpu0_imp_abort_dec         (ls_cpu0_imp_abort_dec),
                      .ls_cpu0_imp_abort_ecc         (ls_cpu0_imp_abort_ecc),
                      .ls_cpu0_imp_abort_slv         (ls_cpu0_imp_abort_slv),
                      .ls_cpu0_raw_eae_nonsec        (ls_cpu0_raw_eae_nonsec),
                      .ls_cpu0_raw_eae_secure        (ls_cpu0_raw_eae_secure),
                      .ls_cpu1_imp_abort_containable (ls_cpu1_imp_abort_containable),
                      .ls_cpu1_imp_abort_dec         (ls_cpu1_imp_abort_dec),
                      .ls_cpu1_imp_abort_ecc         (ls_cpu1_imp_abort_ecc),
                      .ls_cpu1_imp_abort_slv         (ls_cpu1_imp_abort_slv),
                      .ls_cpu1_raw_eae_nonsec        (ls_cpu1_raw_eae_nonsec),
                      .ls_cpu1_raw_eae_secure        (ls_cpu1_raw_eae_secure),
                      .ls_cpu2_imp_abort_containable (ls_cpu2_imp_abort_containable),
                      .ls_cpu2_imp_abort_dec         (ls_cpu2_imp_abort_dec),
                      .ls_cpu2_imp_abort_ecc         (ls_cpu2_imp_abort_ecc),
                      .ls_cpu2_imp_abort_slv         (ls_cpu2_imp_abort_slv),
                      .ls_cpu2_raw_eae_nonsec        (ls_cpu2_raw_eae_nonsec),
                      .ls_cpu2_raw_eae_secure        (ls_cpu2_raw_eae_secure),
                      .ls_cpu3_imp_abort_containable (ls_cpu3_imp_abort_containable),
                      .ls_cpu3_imp_abort_dec         (ls_cpu3_imp_abort_dec),
                      .ls_cpu3_imp_abort_ecc         (ls_cpu3_imp_abort_ecc),
                      .ls_cpu3_imp_abort_slv         (ls_cpu3_imp_abort_slv),
                      .ls_cpu3_raw_eae_nonsec        (ls_cpu3_raw_eae_nonsec),
                      .ls_cpu3_raw_eae_secure        (ls_cpu3_raw_eae_secure),
                      .nFIQ                          (nFIQ[`MAIA_CN:0]),
                      .nIRQ                          (nIRQ[`MAIA_CN:0]),
                      .nREI                          (nREI[`MAIA_CN:0]),
                      .nSEI                          (nSEI[`MAIA_CN:0]),
                      .nVFIQ                         (nVFIQ[`MAIA_CN:0]),
                      .nVIRQ                         (nVIRQ[`MAIA_CN:0]),
                      .nVSEI                         (nVSEI[`MAIA_CN:0])
                    );    // uic

    maia_ck_l2 uck_l2( // outputs
                       .ck_gclkb0          (ck_gclkb0),
                       .ck_gclkb1          (ck_gclkb1),
                       .ck_gclkfr          (ck_gclkfr),
                       .ck_gclkl2          (ck_gclkl2),

                       // inputs
                       .DFTL2CLKDISABLE    (DFTL2CLKDISABLE),
                       .DFTSE              (DFTSE),
                       .ck_gclktl2         (ck_gclktl2),
                       .ck_l2_logic_clk_en (ck_l2_logic_clk_en),
                       .ck_l2_tbnk0_clk_en (ck_l2_tbnk0_clk_en),
                       .ck_l2_tbnk1_clk_en (ck_l2_tbnk1_clk_en),
                       .l2_reset3          (l2_reset3)
                     );    // uck_l2

    maia_ck_top uck_top( // outputs
                         .ck_gclkt   (ck_gclkt[`MAIA_CN:0]),
                         .ck_gclktl2 (ck_gclktl2),

                         // inputs
                         .CLK        (CLK),
                         .CLKEN      (CLKEN),
                         .DFTSE      (DFTSE),
                         .MBISTREQ   (MBISTREQ)
                       );    // uck_top

    maia_ck_logic uck_logic( // outputs
                             .CPUQACCEPTn                      (CPUQACCEPTn[`MAIA_CN:0]),
                             .CPUQACTIVE                       (CPUQACTIVE[`MAIA_CN:0]),
                             .CPUQDENY                         (CPUQDENY[`MAIA_CN:0]),
                             .STANDBYWFE                       (STANDBYWFE[`MAIA_CN:0]),
                             .STANDBYWFI                       (STANDBYWFI[`MAIA_CN:0]),
                             .STANDBYWFIL2                     (STANDBYWFIL2),
                             .WARMRSTREQ                       (WARMRSTREQ[`MAIA_CN:0]),
                             .ck_cpu0_areset_l2cpu             (ck_cpu0_areset_l2cpu),
                             .ck_cpu0_areset_l2dt              (ck_cpu0_areset_l2dt),
                             .ck_cpu0_commrx                   (ck_cpu0_commrx),
                             .ck_cpu0_commtx                   (ck_cpu0_commtx),
                             .ck_cpu0_crcx_clk_en_n            (ck_cpu0_crcx_clk_en_n),
                             .ck_cpu0_crcx_clk_en_n_ic         (ck_cpu0_crcx_clk_en_n_ic),
                             .ck_cpu0_dbgnopwrdwn              (ck_cpu0_dbgnopwrdwn),
                             .ck_cpu0_dbgrstreq                (ck_cpu0_dbgrstreq),
                             .ck_cpu0_dt_standbywfx            (ck_cpu0_dt_standbywfx),
                             .ck_cpu0_dt_wfx_ack               (ck_cpu0_dt_wfx_ack),
                             .ck_cpu0_event_reg                (ck_cpu0_event_reg),
                             .ck_cpu0_l2_standbywfi            (ck_cpu0_l2_standbywfi),
                             .ck_cpu0_l2_standbywfx            (ck_cpu0_l2_standbywfx),
                             .ck_cpu0_ncommirq                 (ck_cpu0_ncommirq),
                             .ck_cpu0_npmuirq                  (ck_cpu0_npmuirq),
                             .ck_cpu0_poreset_status           (ck_cpu0_poreset_status),
                             .ck_cpu0_reset1_n_l2cpu           (ck_cpu0_reset1_n_l2cpu),
                             .ck_cpu0_reset1_n_l2dt            (ck_cpu0_reset1_n_l2dt),
                             .ck_cpu0_wfe_ack                  (ck_cpu0_wfe_ack),
                             .ck_cpu0_wfi_ack                  (ck_cpu0_wfi_ack),
                             .ck_cpu1_areset_l2cpu             (ck_cpu1_areset_l2cpu),
                             .ck_cpu1_areset_l2dt              (ck_cpu1_areset_l2dt),
                             .ck_cpu1_commrx                   (ck_cpu1_commrx),
                             .ck_cpu1_commtx                   (ck_cpu1_commtx),
                             .ck_cpu1_crcx_clk_en_n            (ck_cpu1_crcx_clk_en_n),
                             .ck_cpu1_crcx_clk_en_n_ic         (ck_cpu1_crcx_clk_en_n_ic),
                             .ck_cpu1_dbgnopwrdwn              (ck_cpu1_dbgnopwrdwn),
                             .ck_cpu1_dbgrstreq                (ck_cpu1_dbgrstreq),
                             .ck_cpu1_dt_standbywfx            (ck_cpu1_dt_standbywfx),
                             .ck_cpu1_dt_wfx_ack               (ck_cpu1_dt_wfx_ack),
                             .ck_cpu1_event_reg                (ck_cpu1_event_reg),
                             .ck_cpu1_l2_standbywfi            (ck_cpu1_l2_standbywfi),
                             .ck_cpu1_l2_standbywfx            (ck_cpu1_l2_standbywfx),
                             .ck_cpu1_ncommirq                 (ck_cpu1_ncommirq),
                             .ck_cpu1_npmuirq                  (ck_cpu1_npmuirq),
                             .ck_cpu1_poreset_status           (ck_cpu1_poreset_status),
                             .ck_cpu1_reset1_n_l2cpu           (ck_cpu1_reset1_n_l2cpu),
                             .ck_cpu1_reset1_n_l2dt            (ck_cpu1_reset1_n_l2dt),
                             .ck_cpu1_wfe_ack                  (ck_cpu1_wfe_ack),
                             .ck_cpu1_wfi_ack                  (ck_cpu1_wfi_ack),
                             .ck_cpu2_areset_l2cpu             (ck_cpu2_areset_l2cpu),
                             .ck_cpu2_areset_l2dt              (ck_cpu2_areset_l2dt),
                             .ck_cpu2_commrx                   (ck_cpu2_commrx),
                             .ck_cpu2_commtx                   (ck_cpu2_commtx),
                             .ck_cpu2_crcx_clk_en_n            (ck_cpu2_crcx_clk_en_n),
                             .ck_cpu2_crcx_clk_en_n_ic         (ck_cpu2_crcx_clk_en_n_ic),
                             .ck_cpu2_dbgnopwrdwn              (ck_cpu2_dbgnopwrdwn),
                             .ck_cpu2_dbgrstreq                (ck_cpu2_dbgrstreq),
                             .ck_cpu2_dt_standbywfx            (ck_cpu2_dt_standbywfx),
                             .ck_cpu2_dt_wfx_ack               (ck_cpu2_dt_wfx_ack),
                             .ck_cpu2_event_reg                (ck_cpu2_event_reg),
                             .ck_cpu2_l2_standbywfi            (ck_cpu2_l2_standbywfi),
                             .ck_cpu2_l2_standbywfx            (ck_cpu2_l2_standbywfx),
                             .ck_cpu2_ncommirq                 (ck_cpu2_ncommirq),
                             .ck_cpu2_npmuirq                  (ck_cpu2_npmuirq),
                             .ck_cpu2_poreset_status           (ck_cpu2_poreset_status),
                             .ck_cpu2_reset1_n_l2cpu           (ck_cpu2_reset1_n_l2cpu),
                             .ck_cpu2_reset1_n_l2dt            (ck_cpu2_reset1_n_l2dt),
                             .ck_cpu2_wfe_ack                  (ck_cpu2_wfe_ack),
                             .ck_cpu2_wfi_ack                  (ck_cpu2_wfi_ack),
                             .ck_cpu3_areset_l2cpu             (ck_cpu3_areset_l2cpu),
                             .ck_cpu3_areset_l2dt              (ck_cpu3_areset_l2dt),
                             .ck_cpu3_commrx                   (ck_cpu3_commrx),
                             .ck_cpu3_commtx                   (ck_cpu3_commtx),
                             .ck_cpu3_crcx_clk_en_n            (ck_cpu3_crcx_clk_en_n),
                             .ck_cpu3_crcx_clk_en_n_ic         (ck_cpu3_crcx_clk_en_n_ic),
                             .ck_cpu3_dbgnopwrdwn              (ck_cpu3_dbgnopwrdwn),
                             .ck_cpu3_dbgrstreq                (ck_cpu3_dbgrstreq),
                             .ck_cpu3_dt_standbywfx            (ck_cpu3_dt_standbywfx),
                             .ck_cpu3_dt_wfx_ack               (ck_cpu3_dt_wfx_ack),
                             .ck_cpu3_event_reg                (ck_cpu3_event_reg),
                             .ck_cpu3_l2_standbywfi            (ck_cpu3_l2_standbywfi),
                             .ck_cpu3_l2_standbywfx            (ck_cpu3_l2_standbywfx),
                             .ck_cpu3_ncommirq                 (ck_cpu3_ncommirq),
                             .ck_cpu3_npmuirq                  (ck_cpu3_npmuirq),
                             .ck_cpu3_poreset_status           (ck_cpu3_poreset_status),
                             .ck_cpu3_reset1_n_l2cpu           (ck_cpu3_reset1_n_l2cpu),
                             .ck_cpu3_reset1_n_l2dt            (ck_cpu3_reset1_n_l2dt),
                             .ck_cpu3_wfe_ack                  (ck_cpu3_wfe_ack),
                             .ck_cpu3_wfi_ack                  (ck_cpu3_wfi_ack),
                             .ck_dt_cpu0_coredbg_in_reset_gclk (ck_dt_cpu0_coredbg_in_reset_gclk),
                             .ck_dt_cpu0_cti_trigin_1to0_gclk  (ck_dt_cpu0_cti_trigin_1to0_gclk[1:0]),
                             .ck_dt_cpu0_et_oslock_gclk        (ck_dt_cpu0_et_oslock_gclk),
                             .ck_dt_cpu0_hlt_dbgevt_ok_gclk    (ck_dt_cpu0_hlt_dbgevt_ok_gclk),
                             .ck_dt_cpu0_os_double_lock_gclk   (ck_dt_cpu0_os_double_lock_gclk),
                             .ck_dt_cpu0_pmusnapshot_ack_gclk  (ck_dt_cpu0_pmusnapshot_ack_gclk),
                             .ck_dt_cpu0_wfx_dbg_req_gclk      (ck_dt_cpu0_wfx_dbg_req_gclk),
                             .ck_dt_cpu1_coredbg_in_reset_gclk (ck_dt_cpu1_coredbg_in_reset_gclk),
                             .ck_dt_cpu1_cti_trigin_1to0_gclk  (ck_dt_cpu1_cti_trigin_1to0_gclk[1:0]),
                             .ck_dt_cpu1_et_oslock_gclk        (ck_dt_cpu1_et_oslock_gclk),
                             .ck_dt_cpu1_hlt_dbgevt_ok_gclk    (ck_dt_cpu1_hlt_dbgevt_ok_gclk),
                             .ck_dt_cpu1_os_double_lock_gclk   (ck_dt_cpu1_os_double_lock_gclk),
                             .ck_dt_cpu1_pmusnapshot_ack_gclk  (ck_dt_cpu1_pmusnapshot_ack_gclk),
                             .ck_dt_cpu1_wfx_dbg_req_gclk      (ck_dt_cpu1_wfx_dbg_req_gclk),
                             .ck_dt_cpu2_coredbg_in_reset_gclk (ck_dt_cpu2_coredbg_in_reset_gclk),
                             .ck_dt_cpu2_cti_trigin_1to0_gclk  (ck_dt_cpu2_cti_trigin_1to0_gclk[1:0]),
                             .ck_dt_cpu2_et_oslock_gclk        (ck_dt_cpu2_et_oslock_gclk),
                             .ck_dt_cpu2_hlt_dbgevt_ok_gclk    (ck_dt_cpu2_hlt_dbgevt_ok_gclk),
                             .ck_dt_cpu2_os_double_lock_gclk   (ck_dt_cpu2_os_double_lock_gclk),
                             .ck_dt_cpu2_pmusnapshot_ack_gclk  (ck_dt_cpu2_pmusnapshot_ack_gclk),
                             .ck_dt_cpu2_wfx_dbg_req_gclk      (ck_dt_cpu2_wfx_dbg_req_gclk),
                             .ck_dt_cpu3_coredbg_in_reset_gclk (ck_dt_cpu3_coredbg_in_reset_gclk),
                             .ck_dt_cpu3_cti_trigin_1to0_gclk  (ck_dt_cpu3_cti_trigin_1to0_gclk[1:0]),
                             .ck_dt_cpu3_et_oslock_gclk        (ck_dt_cpu3_et_oslock_gclk),
                             .ck_dt_cpu3_hlt_dbgevt_ok_gclk    (ck_dt_cpu3_hlt_dbgevt_ok_gclk),
                             .ck_dt_cpu3_os_double_lock_gclk   (ck_dt_cpu3_os_double_lock_gclk),
                             .ck_dt_cpu3_pmusnapshot_ack_gclk  (ck_dt_cpu3_pmusnapshot_ack_gclk),
                             .ck_dt_cpu3_wfx_dbg_req_gclk      (ck_dt_cpu3_wfx_dbg_req_gclk),
                             .ck_l2_ace_inactive               (ck_l2_ace_inactive),
                             .ck_l2_acp_inactive               (ck_l2_acp_inactive),
                             .ck_l2_sky_link_deactivate        (ck_l2_sky_link_deactivate),

                             // inputs
                             .ACINACTM                         (SINACT),
                             .AINACTS                          (AINACTS),
                             .CPUQREQn                         (CPUQREQn[`MAIA_CN:0]),
                             .DFTRSTDISABLE                    (DFTRSTDISABLE),
                             .DFTSE                            (DFTSE),
                             .ck_gclkfr                        (ck_gclkfr),
                             .clrexmon_c1                      (clrexmon_c1),
                             .commrx_cpu0_i                    (commrx_cpu0_i),
                             .commrx_cpu1_i                    (commrx_cpu1_i),
                             .commrx_cpu2_i                    (commrx_cpu2_i),
                             .commrx_cpu3_i                    (commrx_cpu3_i),
                             .commtx_cpu0_i                    (commtx_cpu0_i),
                             .commtx_cpu1_i                    (commtx_cpu1_i),
                             .commtx_cpu2_i                    (commtx_cpu2_i),
                             .commtx_cpu3_i                    (commtx_cpu3_i),
                             .dbgnopwrdwn_cpu0_i               (dbgnopwrdwn_cpu0_i),
                             .dbgnopwrdwn_cpu1_i               (dbgnopwrdwn_cpu1_i),
                             .dbgnopwrdwn_cpu2_i               (dbgnopwrdwn_cpu2_i),
                             .dbgnopwrdwn_cpu3_i               (dbgnopwrdwn_cpu3_i),
                             .dbgrstreq_cpu0_i                 (dbgrstreq_cpu0_i),
                             .dbgrstreq_cpu1_i                 (dbgrstreq_cpu1_i),
                             .dbgrstreq_cpu2_i                 (dbgrstreq_cpu2_i),
                             .dbgrstreq_cpu3_i                 (dbgrstreq_cpu3_i),
                             .ds_cpu0_cpuectlr_ret             (ds_cpu0_cpuectlr_ret[2:0]),
                             .ds_cpu0_fiq_wfe_qual             (ds_cpu0_fiq_wfe_qual),
                             .ds_cpu0_fiq_wfi_qual             (ds_cpu0_fiq_wfi_qual),
                             .ds_cpu0_flush                    (ds_cpu0_flush),
                             .ds_cpu0_flush_type               (ds_cpu0_flush_type[5:0]),
                             .ds_cpu0_hcr_va                   (ds_cpu0_hcr_va),
                             .ds_cpu0_hcr_vf                   (ds_cpu0_hcr_vf),
                             .ds_cpu0_hcr_vi                   (ds_cpu0_hcr_vi),
                             .ds_cpu0_imp_abrt_wfe_qual        (ds_cpu0_imp_abrt_wfe_qual),
                             .ds_cpu0_imp_abrt_wfi_qual        (ds_cpu0_imp_abrt_wfi_qual),
                             .ds_cpu0_irq_wfe_qual             (ds_cpu0_irq_wfe_qual),
                             .ds_cpu0_irq_wfi_qual             (ds_cpu0_irq_wfi_qual),
                             .ds_cpu0_reset_req                (ds_cpu0_reset_req),
                             .ds_cpu0_sevl_req                 (ds_cpu0_sevl_req),
                             .ds_cpu0_vfiq_wfe_qual            (ds_cpu0_vfiq_wfe_qual),
                             .ds_cpu0_vfiq_wfi_qual            (ds_cpu0_vfiq_wfi_qual),
                             .ds_cpu0_vimp_abrt_wfe_qual       (ds_cpu0_vimp_abrt_wfe_qual),
                             .ds_cpu0_vimp_abrt_wfi_qual       (ds_cpu0_vimp_abrt_wfi_qual),
                             .ds_cpu0_virq_wfe_qual            (ds_cpu0_virq_wfe_qual),
                             .ds_cpu0_virq_wfi_qual            (ds_cpu0_virq_wfi_qual),
                             .ds_cpu0_wfe_req                  (ds_cpu0_wfe_req),
                             .ds_cpu0_wfi_req                  (ds_cpu0_wfi_req),
                             .ds_cpu1_cpuectlr_ret             (ds_cpu1_cpuectlr_ret[2:0]),
                             .ds_cpu1_fiq_wfe_qual             (ds_cpu1_fiq_wfe_qual),
                             .ds_cpu1_fiq_wfi_qual             (ds_cpu1_fiq_wfi_qual),
                             .ds_cpu1_flush                    (ds_cpu1_flush),
                             .ds_cpu1_flush_type               (ds_cpu1_flush_type[5:0]),
                             .ds_cpu1_hcr_va                   (ds_cpu1_hcr_va),
                             .ds_cpu1_hcr_vf                   (ds_cpu1_hcr_vf),
                             .ds_cpu1_hcr_vi                   (ds_cpu1_hcr_vi),
                             .ds_cpu1_imp_abrt_wfe_qual        (ds_cpu1_imp_abrt_wfe_qual),
                             .ds_cpu1_imp_abrt_wfi_qual        (ds_cpu1_imp_abrt_wfi_qual),
                             .ds_cpu1_irq_wfe_qual             (ds_cpu1_irq_wfe_qual),
                             .ds_cpu1_irq_wfi_qual             (ds_cpu1_irq_wfi_qual),
                             .ds_cpu1_reset_req                (ds_cpu1_reset_req),
                             .ds_cpu1_sevl_req                 (ds_cpu1_sevl_req),
                             .ds_cpu1_vfiq_wfe_qual            (ds_cpu1_vfiq_wfe_qual),
                             .ds_cpu1_vfiq_wfi_qual            (ds_cpu1_vfiq_wfi_qual),
                             .ds_cpu1_vimp_abrt_wfe_qual       (ds_cpu1_vimp_abrt_wfe_qual),
                             .ds_cpu1_vimp_abrt_wfi_qual       (ds_cpu1_vimp_abrt_wfi_qual),
                             .ds_cpu1_virq_wfe_qual            (ds_cpu1_virq_wfe_qual),
                             .ds_cpu1_virq_wfi_qual            (ds_cpu1_virq_wfi_qual),
                             .ds_cpu1_wfe_req                  (ds_cpu1_wfe_req),
                             .ds_cpu1_wfi_req                  (ds_cpu1_wfi_req),
                             .ds_cpu2_cpuectlr_ret             (ds_cpu2_cpuectlr_ret[2:0]),
                             .ds_cpu2_fiq_wfe_qual             (ds_cpu2_fiq_wfe_qual),
                             .ds_cpu2_fiq_wfi_qual             (ds_cpu2_fiq_wfi_qual),
                             .ds_cpu2_flush                    (ds_cpu2_flush),
                             .ds_cpu2_flush_type               (ds_cpu2_flush_type[5:0]),
                             .ds_cpu2_hcr_va                   (ds_cpu2_hcr_va),
                             .ds_cpu2_hcr_vf                   (ds_cpu2_hcr_vf),
                             .ds_cpu2_hcr_vi                   (ds_cpu2_hcr_vi),
                             .ds_cpu2_imp_abrt_wfe_qual        (ds_cpu2_imp_abrt_wfe_qual),
                             .ds_cpu2_imp_abrt_wfi_qual        (ds_cpu2_imp_abrt_wfi_qual),
                             .ds_cpu2_irq_wfe_qual             (ds_cpu2_irq_wfe_qual),
                             .ds_cpu2_irq_wfi_qual             (ds_cpu2_irq_wfi_qual),
                             .ds_cpu2_reset_req                (ds_cpu2_reset_req),
                             .ds_cpu2_sevl_req                 (ds_cpu2_sevl_req),
                             .ds_cpu2_vfiq_wfe_qual            (ds_cpu2_vfiq_wfe_qual),
                             .ds_cpu2_vfiq_wfi_qual            (ds_cpu2_vfiq_wfi_qual),
                             .ds_cpu2_vimp_abrt_wfe_qual       (ds_cpu2_vimp_abrt_wfe_qual),
                             .ds_cpu2_vimp_abrt_wfi_qual       (ds_cpu2_vimp_abrt_wfi_qual),
                             .ds_cpu2_virq_wfe_qual            (ds_cpu2_virq_wfe_qual),
                             .ds_cpu2_virq_wfi_qual            (ds_cpu2_virq_wfi_qual),
                             .ds_cpu2_wfe_req                  (ds_cpu2_wfe_req),
                             .ds_cpu2_wfi_req                  (ds_cpu2_wfi_req),
                             .ds_cpu3_cpuectlr_ret             (ds_cpu3_cpuectlr_ret[2:0]),
                             .ds_cpu3_fiq_wfe_qual             (ds_cpu3_fiq_wfe_qual),
                             .ds_cpu3_fiq_wfi_qual             (ds_cpu3_fiq_wfi_qual),
                             .ds_cpu3_flush                    (ds_cpu3_flush),
                             .ds_cpu3_flush_type               (ds_cpu3_flush_type[5:0]),
                             .ds_cpu3_hcr_va                   (ds_cpu3_hcr_va),
                             .ds_cpu3_hcr_vf                   (ds_cpu3_hcr_vf),
                             .ds_cpu3_hcr_vi                   (ds_cpu3_hcr_vi),
                             .ds_cpu3_imp_abrt_wfe_qual        (ds_cpu3_imp_abrt_wfe_qual),
                             .ds_cpu3_imp_abrt_wfi_qual        (ds_cpu3_imp_abrt_wfi_qual),
                             .ds_cpu3_irq_wfe_qual             (ds_cpu3_irq_wfe_qual),
                             .ds_cpu3_irq_wfi_qual             (ds_cpu3_irq_wfi_qual),
                             .ds_cpu3_reset_req                (ds_cpu3_reset_req),
                             .ds_cpu3_sevl_req                 (ds_cpu3_sevl_req),
                             .ds_cpu3_vfiq_wfe_qual            (ds_cpu3_vfiq_wfe_qual),
                             .ds_cpu3_vfiq_wfi_qual            (ds_cpu3_vfiq_wfi_qual),
                             .ds_cpu3_vimp_abrt_wfe_qual       (ds_cpu3_vimp_abrt_wfe_qual),
                             .ds_cpu3_vimp_abrt_wfi_qual       (ds_cpu3_vimp_abrt_wfi_qual),
                             .ds_cpu3_virq_wfe_qual            (ds_cpu3_virq_wfe_qual),
                             .ds_cpu3_virq_wfi_qual            (ds_cpu3_virq_wfi_qual),
                             .ds_cpu3_wfe_req                  (ds_cpu3_wfe_req),
                             .ds_cpu3_wfi_req                  (ds_cpu3_wfi_req),
                             .dt_cpu0_apb_active_pclk          (dt_cpu0_apb_active_pclk),
                             .dt_cpu0_coredbg_in_reset_gclk    (dt_cpu0_coredbg_in_reset_gclk),
                             .dt_cpu0_cti_trigin_1to0_gclk     (dt_cpu0_cti_trigin_1to0_gclk[1:0]),
                             .dt_cpu0_et_oslock_gclk           (dt_cpu0_et_oslock_gclk),
                             .dt_cpu0_hlt_dbgevt_ok_gclk       (dt_cpu0_hlt_dbgevt_ok_gclk),
                             .dt_cpu0_os_double_lock_gclk      (dt_cpu0_os_double_lock_gclk),
                             .dt_cpu0_pmusnapshot_ack_gclk     (dt_cpu0_pmusnapshot_ack_gclk),
                             .dt_cpu0_poreset_status_ack_pclk  (dt_cpu0_poreset_status_ack_pclk),
                             .dt_cpu0_wfx_dbg_req_gclk         (dt_cpu0_wfx_dbg_req_gclk),
                             .dt_cpu0_wfx_wakeup_pclk          (dt_cpu0_wfx_wakeup_pclk),
                             .dt_cpu1_apb_active_pclk          (dt_cpu1_apb_active_pclk),
                             .dt_cpu1_coredbg_in_reset_gclk    (dt_cpu1_coredbg_in_reset_gclk),
                             .dt_cpu1_cti_trigin_1to0_gclk     (dt_cpu1_cti_trigin_1to0_gclk[1:0]),
                             .dt_cpu1_et_oslock_gclk           (dt_cpu1_et_oslock_gclk),
                             .dt_cpu1_hlt_dbgevt_ok_gclk       (dt_cpu1_hlt_dbgevt_ok_gclk),
                             .dt_cpu1_os_double_lock_gclk      (dt_cpu1_os_double_lock_gclk),
                             .dt_cpu1_pmusnapshot_ack_gclk     (dt_cpu1_pmusnapshot_ack_gclk),
                             .dt_cpu1_poreset_status_ack_pclk  (dt_cpu1_poreset_status_ack_pclk),
                             .dt_cpu1_wfx_dbg_req_gclk         (dt_cpu1_wfx_dbg_req_gclk),
                             .dt_cpu1_wfx_wakeup_pclk          (dt_cpu1_wfx_wakeup_pclk),
                             .dt_cpu2_apb_active_pclk          (dt_cpu2_apb_active_pclk),
                             .dt_cpu2_coredbg_in_reset_gclk    (dt_cpu2_coredbg_in_reset_gclk),
                             .dt_cpu2_cti_trigin_1to0_gclk     (dt_cpu2_cti_trigin_1to0_gclk[1:0]),
                             .dt_cpu2_et_oslock_gclk           (dt_cpu2_et_oslock_gclk),
                             .dt_cpu2_hlt_dbgevt_ok_gclk       (dt_cpu2_hlt_dbgevt_ok_gclk),
                             .dt_cpu2_os_double_lock_gclk      (dt_cpu2_os_double_lock_gclk),
                             .dt_cpu2_pmusnapshot_ack_gclk     (dt_cpu2_pmusnapshot_ack_gclk),
                             .dt_cpu2_poreset_status_ack_pclk  (dt_cpu2_poreset_status_ack_pclk),
                             .dt_cpu2_wfx_dbg_req_gclk         (dt_cpu2_wfx_dbg_req_gclk),
                             .dt_cpu2_wfx_wakeup_pclk          (dt_cpu2_wfx_wakeup_pclk),
                             .dt_cpu3_apb_active_pclk          (dt_cpu3_apb_active_pclk),
                             .dt_cpu3_coredbg_in_reset_gclk    (dt_cpu3_coredbg_in_reset_gclk),
                             .dt_cpu3_cti_trigin_1to0_gclk     (dt_cpu3_cti_trigin_1to0_gclk[1:0]),
                             .dt_cpu3_et_oslock_gclk           (dt_cpu3_et_oslock_gclk),
                             .dt_cpu3_hlt_dbgevt_ok_gclk       (dt_cpu3_hlt_dbgevt_ok_gclk),
                             .dt_cpu3_os_double_lock_gclk      (dt_cpu3_os_double_lock_gclk),
                             .dt_cpu3_pmusnapshot_ack_gclk     (dt_cpu3_pmusnapshot_ack_gclk),
                             .dt_cpu3_poreset_status_ack_pclk  (dt_cpu3_poreset_status_ack_pclk),
                             .dt_cpu3_wfx_dbg_req_gclk         (dt_cpu3_wfx_dbg_req_gclk),
                             .dt_cpu3_wfx_wakeup_pclk          (dt_cpu3_wfx_wakeup_pclk),
                             .ic_nfiq                          (ic_nfiq_o[`MAIA_CN:0]),
                             .ic_nirq                          (ic_nirq_o[`MAIA_CN:0]),
                             .ic_nsei                          (ic_nsei_o[`MAIA_CN:0]),
                             .ic_nvfiq                         (ic_nvfiq_o[`MAIA_CN:0]),
                             .ic_nvirq                         (ic_nvirq_o[`MAIA_CN:0]),
                             .ic_nvsei                         (ic_nvsei_o[`MAIA_CN:0]),
                             .l2_actlr_ncpu_rcg_enable         (l2_actlr_ncpu_rcg_enable),
                             .l2_cpu0_snp_active               (l2_cpu0_snp_active),
                             .l2_cpu1_snp_active               (l2_cpu1_snp_active),
                             .l2_cpu2_snp_active               (l2_cpu2_snp_active),
                             .l2_cpu3_snp_active               (l2_cpu3_snp_active),
                             .l2_idle                          (l2_idle),
                             .l2_mbist1_en_b1                  (l2_mbist1_en_b1[`MAIA_CN:0]),
                             .l2_reset3                        (l2_reset3),
                             .l2_sky_link_stopped              (l2_sky_link_stopped),
                             .ls_cpu0_clrexmon                 (ls_cpu0_clrexmon),
                             .ls_cpu1_clrexmon                 (ls_cpu1_clrexmon),
                             .ls_cpu2_clrexmon                 (ls_cpu2_clrexmon),
                             .ls_cpu3_clrexmon                 (ls_cpu3_clrexmon),
                             .nCORERESET                       (nCORERESET[`MAIA_CN:0]),
                             .nCPUPORESET                      (nCPUPORESET[`MAIA_CN:0]),
                             .nL2RESET                         (nL2RESET),
                             .nMBISTRESET                      (nMBISTRESET),
                             .ncommirq_cpu0_i                  (ncommirq_cpu0_i),
                             .ncommirq_cpu1_i                  (ncommirq_cpu1_i),
                             .ncommirq_cpu2_i                  (ncommirq_cpu2_i),
                             .ncommirq_cpu3_i                  (ncommirq_cpu3_i),
                             .npmuirq_cpu0_i                   (npmuirq_cpu0_i),
                             .npmuirq_cpu1_i                   (npmuirq_cpu1_i),
                             .npmuirq_cpu2_i                   (npmuirq_cpu2_i),
                             .npmuirq_cpu3_i                   (npmuirq_cpu3_i),
                             .tm_cntpct_q                      (tm_cntpct_q[8:0]),
                             .tm_cpu0_event_sev                (tm_cpu0_event_sev),
                             .tm_cpu1_event_sev                (tm_cpu1_event_sev),
                             .tm_cpu2_event_sev                (tm_cpu2_event_sev),
                             .tm_cpu3_event_sev                (tm_cpu3_event_sev)
                           );    // uck_logic

    maia_cpu_io ucpu_io( // outputs
                         .aa64naa32_cpu0_o       (aa64naa32_cpu0_o),
                         .aa64naa32_cpu1_o       (aa64naa32_cpu1_o),
                         .aa64naa32_cpu2_o       (aa64naa32_cpu2_o),
                         .aa64naa32_cpu3_o       (aa64naa32_cpu3_o),
                         .cfgend_cpu0_o          (cfgend_cpu0_o),
                         .cfgend_cpu1_o          (cfgend_cpu1_o),
                         .cfgend_cpu2_o          (cfgend_cpu2_o),
                         .cfgend_cpu3_o          (cfgend_cpu3_o),
                         .cfgte_cpu0_o           (cfgte_cpu0_o),
                         .cfgte_cpu1_o           (cfgte_cpu1_o),
                         .cfgte_cpu2_o           (cfgte_cpu2_o),
                         .cfgte_cpu3_o           (cfgte_cpu3_o),
                         .clrexmon_c1            (clrexmon_c1),
                         .clrexmonack_o          (CLREXMONACK),
                         .clusteridaff1_cpu0_o   (clusteridaff1_cpu0_o[7:0]),
                         .clusteridaff1_cpu1_o   (clusteridaff1_cpu1_o[7:0]),
                         .clusteridaff1_cpu2_o   (clusteridaff1_cpu2_o[7:0]),
                         .clusteridaff1_cpu3_o   (clusteridaff1_cpu3_o[7:0]),
                         .clusteridaff2_cpu0_o   (clusteridaff2_cpu0_o[7:0]),
                         .clusteridaff2_cpu1_o   (clusteridaff2_cpu1_o[7:0]),
                         .clusteridaff2_cpu2_o   (clusteridaff2_cpu2_o[7:0]),
                         .clusteridaff2_cpu3_o   (clusteridaff2_cpu3_o[7:0]),
                         .commrx_o               (COMMRX[`MAIA_CN:0]),
                         .commtx_o               (COMMTX[`MAIA_CN:0]),
                         .cp15sdisable_cpu0_o    (cp15sdisable_cpu0_o),
                         .cp15sdisable_cpu1_o    (cp15sdisable_cpu1_o),
                         .cp15sdisable_cpu2_o    (cp15sdisable_cpu2_o),
                         .cp15sdisable_cpu3_o    (cp15sdisable_cpu3_o),
                         .cpuid_cpu0_o           (cpuid_cpu0_o[1:0]),
                         .cpuid_cpu1_o           (cpuid_cpu1_o[1:0]),
                         .cpuid_cpu2_o           (cpuid_cpu2_o[1:0]),
                         .cpuid_cpu3_o           (cpuid_cpu3_o[1:0]),
                         .cryptodisable_cpu0_o   (cryptodisable_cpu0_o),
                         .cryptodisable_cpu1_o   (cryptodisable_cpu1_o),
                         .cryptodisable_cpu2_o   (cryptodisable_cpu2_o),
                         .cryptodisable_cpu3_o   (cryptodisable_cpu3_o),
                         .dbgack_o               (DBGACK[`MAIA_CN:0]),
                         .dbgen_cpu0_o           (dbgen_cpu0_o),
                         .dbgen_cpu1_o           (dbgen_cpu1_o),
                         .dbgen_cpu2_o           (dbgen_cpu2_o),
                         .dbgen_cpu3_o           (dbgen_cpu3_o),
                         .dbgl1rstdisable_cpu0_o (dbgl1rstdisable_cpu0_o),
                         .dbgl1rstdisable_cpu1_o (dbgl1rstdisable_cpu1_o),
                         .dbgl1rstdisable_cpu2_o (dbgl1rstdisable_cpu2_o),
                         .dbgl1rstdisable_cpu3_o (dbgl1rstdisable_cpu3_o),
                         .dbgnopwrdwn_o          (DBGNOPWRDWN[`MAIA_CN:0]),
                         .dbgromaddr_cpu0_o      (dbgromaddr_cpu0_o[43:12]),
                         .dbgromaddr_cpu1_o      (dbgromaddr_cpu1_o[43:12]),
                         .dbgromaddr_cpu2_o      (dbgromaddr_cpu2_o[43:12]),
                         .dbgromaddr_cpu3_o      (dbgromaddr_cpu3_o[43:12]),
                         .dbgromaddrv_cpu0_o     (dbgromaddrv_cpu0_o),
                         .dbgromaddrv_cpu1_o     (dbgromaddrv_cpu1_o),
                         .dbgromaddrv_cpu2_o     (dbgromaddrv_cpu2_o),
                         .dbgromaddrv_cpu3_o     (dbgromaddrv_cpu3_o),
                         .dbgrstreq_o            (DBGRSTREQ[`MAIA_CN:0]),
                         .dftcrclkdisable_cpu0_o (dftcrclkdisable_cpu0_o),
                         .dftcrclkdisable_cpu1_o (dftcrclkdisable_cpu1_o),
                         .dftcrclkdisable_cpu2_o (dftcrclkdisable_cpu2_o),
                         .dftcrclkdisable_cpu3_o (dftcrclkdisable_cpu3_o),
                         .dftramhold_cpu0_o      (dftramhold_cpu0_o),
                         .dftramhold_cpu1_o      (dftramhold_cpu1_o),
                         .dftramhold_cpu2_o      (dftramhold_cpu2_o),
                         .dftramhold_cpu3_o      (dftramhold_cpu3_o),
                         .dftrstdisable_cpu0_o   (dftrstdisable_cpu0_o),
                         .dftrstdisable_cpu1_o   (dftrstdisable_cpu1_o),
                         .dftrstdisable_cpu2_o   (dftrstdisable_cpu2_o),
                         .dftrstdisable_cpu3_o   (dftrstdisable_cpu3_o),
                         .dftse_cpu0_o           (dftse_cpu0_o),
                         .dftse_cpu1_o           (dftse_cpu1_o),
                         .dftse_cpu2_o           (dftse_cpu2_o),
                         .dftse_cpu3_o           (dftse_cpu3_o),
                         .eventi_sev             (eventi_sev),
                         .evento_o               (EVENTO),
                         .giccdisable_cpu0_o     (giccdisable_cpu0_o),
                         .giccdisable_cpu1_o     (giccdisable_cpu1_o),
                         .giccdisable_cpu2_o     (giccdisable_cpu2_o),
                         .giccdisable_cpu3_o     (giccdisable_cpu3_o),
                         .ncommirq_o             (nCOMMIRQ[`MAIA_CN:0]),
                         .ncorereset_cpu0_o      (ncorereset_cpu0_o),
                         .ncorereset_cpu1_o      (ncorereset_cpu1_o),
                         .ncorereset_cpu2_o      (ncorereset_cpu2_o),
                         .ncorereset_cpu3_o      (ncorereset_cpu3_o),
                         .ncpuporeset_cpu0_o     (ncpuporeset_cpu0_o),
                         .ncpuporeset_cpu1_o     (ncpuporeset_cpu1_o),
                         .ncpuporeset_cpu2_o     (ncpuporeset_cpu2_o),
                         .ncpuporeset_cpu3_o     (ncpuporeset_cpu3_o),
                         .niden_cpu0_o           (niden_cpu0_o),
                         .niden_cpu1_o           (niden_cpu1_o),
                         .niden_cpu2_o           (niden_cpu2_o),
                         .niden_cpu3_o           (niden_cpu3_o),
                         .nmbistreset_cpu0_o     (nmbistreset_cpu0_o),
                         .nmbistreset_cpu1_o     (nmbistreset_cpu1_o),
                         .nmbistreset_cpu2_o     (nmbistreset_cpu2_o),
                         .nmbistreset_cpu3_o     (nmbistreset_cpu3_o),
                         .npmuirq_o              (nPMUIRQ[`MAIA_CN:0]),
                         .pmuevent0_o            (PMUEVENT0[24:0]),
                         .pmuevent1_o            (PMUEVENT1[24:0]),
                         .pmuevent2_o            (PMUEVENT2[24:0]),
                         .pmuevent3_o            (PMUEVENT3[24:0]),
                         .rvbaraddr_cpu0_o       (rvbaraddr_cpu0_o[43:2]),
                         .rvbaraddr_cpu1_o       (rvbaraddr_cpu1_o[43:2]),
                         .rvbaraddr_cpu2_o       (rvbaraddr_cpu2_o[43:2]),
                         .rvbaraddr_cpu3_o       (rvbaraddr_cpu3_o[43:2]),
                         .smpen_o                (SMPEN[`MAIA_CN:0]),
                         .spiden_cpu0_o          (spiden_cpu0_o),
                         .spiden_cpu1_o          (spiden_cpu1_o),
                         .spiden_cpu2_o          (spiden_cpu2_o),
                         .spiden_cpu3_o          (spiden_cpu3_o),
                         .spniden_cpu0_o         (spniden_cpu0_o),
                         .spniden_cpu1_o         (spniden_cpu1_o),
                         .spniden_cpu2_o         (spniden_cpu2_o),
                         .spniden_cpu3_o         (spniden_cpu3_o),
                         .vinithi_cpu0_o         (vinithi_cpu0_o),
                         .vinithi_cpu1_o         (vinithi_cpu1_o),
                         .vinithi_cpu2_o         (vinithi_cpu2_o),
                         .vinithi_cpu3_o         (vinithi_cpu3_o),

                         // inputs
                         .aa64naa32_i            (AA64nAA32[`MAIA_CN:0]),
                         .cfgend_i               (CFGEND[`MAIA_CN:0]),
                         .cfgte_i                (CFGTE[`MAIA_CN:0]),
                         .ck_cpu0_areset_l2cpu   (ck_cpu0_areset_l2cpu),
                         .ck_cpu0_areset_l2dt    (ck_cpu0_areset_l2dt),
                         .ck_cpu0_commrx         (ck_cpu0_commrx),
                         .ck_cpu0_commtx         (ck_cpu0_commtx),
                         .ck_cpu0_dbgnopwrdwn    (ck_cpu0_dbgnopwrdwn),
                         .ck_cpu0_dbgrstreq      (ck_cpu0_dbgrstreq),
                         .ck_cpu0_ncommirq       (ck_cpu0_ncommirq),
                         .ck_cpu0_npmuirq        (ck_cpu0_npmuirq),
                         .ck_cpu0_reset1_n_l2cpu (ck_cpu0_reset1_n_l2cpu),
                         .ck_cpu0_reset1_n_l2dt  (ck_cpu0_reset1_n_l2dt),
                         .ck_cpu1_areset_l2cpu   (ck_cpu1_areset_l2cpu),
                         .ck_cpu1_areset_l2dt    (ck_cpu1_areset_l2dt),
                         .ck_cpu1_commrx         (ck_cpu1_commrx),
                         .ck_cpu1_commtx         (ck_cpu1_commtx),
                         .ck_cpu1_dbgnopwrdwn    (ck_cpu1_dbgnopwrdwn),
                         .ck_cpu1_dbgrstreq      (ck_cpu1_dbgrstreq),
                         .ck_cpu1_ncommirq       (ck_cpu1_ncommirq),
                         .ck_cpu1_npmuirq        (ck_cpu1_npmuirq),
                         .ck_cpu1_reset1_n_l2cpu (ck_cpu1_reset1_n_l2cpu),
                         .ck_cpu1_reset1_n_l2dt  (ck_cpu1_reset1_n_l2dt),
                         .ck_cpu2_areset_l2cpu   (ck_cpu2_areset_l2cpu),
                         .ck_cpu2_areset_l2dt    (ck_cpu2_areset_l2dt),
                         .ck_cpu2_commrx         (ck_cpu2_commrx),
                         .ck_cpu2_commtx         (ck_cpu2_commtx),
                         .ck_cpu2_dbgnopwrdwn    (ck_cpu2_dbgnopwrdwn),
                         .ck_cpu2_dbgrstreq      (ck_cpu2_dbgrstreq),
                         .ck_cpu2_ncommirq       (ck_cpu2_ncommirq),
                         .ck_cpu2_npmuirq        (ck_cpu2_npmuirq),
                         .ck_cpu2_reset1_n_l2cpu (ck_cpu2_reset1_n_l2cpu),
                         .ck_cpu2_reset1_n_l2dt  (ck_cpu2_reset1_n_l2dt),
                         .ck_cpu3_areset_l2cpu   (ck_cpu3_areset_l2cpu),
                         .ck_cpu3_areset_l2dt    (ck_cpu3_areset_l2dt),
                         .ck_cpu3_commrx         (ck_cpu3_commrx),
                         .ck_cpu3_commtx         (ck_cpu3_commtx),
                         .ck_cpu3_dbgnopwrdwn    (ck_cpu3_dbgnopwrdwn),
                         .ck_cpu3_dbgrstreq      (ck_cpu3_dbgrstreq),
                         .ck_cpu3_ncommirq       (ck_cpu3_ncommirq),
                         .ck_cpu3_npmuirq        (ck_cpu3_npmuirq),
                         .ck_cpu3_reset1_n_l2cpu (ck_cpu3_reset1_n_l2cpu),
                         .ck_cpu3_reset1_n_l2dt  (ck_cpu3_reset1_n_l2dt),
                         .ck_gclkfr              (ck_gclkfr),
                         .clrexmonreq_i          (CLREXMONREQ),
                         .clusteridaff1_i        (CLUSTERIDAFF1[7:0]),
                         .clusteridaff2_i        (CLUSTERIDAFF2[7:0]),
                         .cp15sdisable_i         (CP15SDISABLE[`MAIA_CN:0]),
                         .cryptodisable_i        (CRYPTODISABLE[`MAIA_CN:0]),
                         .dbgack_cpu0_i          (dbgack_cpu0_i),
                         .dbgack_cpu1_i          (dbgack_cpu1_i),
                         .dbgack_cpu2_i          (dbgack_cpu2_i),
                         .dbgack_cpu3_i          (dbgack_cpu3_i),
                         .dbgen_i                (DBGEN[`MAIA_CN:0]),
                         .dbgl1rstdisable_i      (DBGL1RSTDISABLE),
                         .dbgromaddr_i           (DBGROMADDR[43:12]),
                         .dbgromaddrv_i          (DBGROMADDRV),
                         .dftcrclkdisable_i      (DFTCRCLKDISABLE[`MAIA_CN:0]),
                         .dftramhold_i           (DFTRAMHOLD),
                         .dftrstdisable_i        (DFTRSTDISABLE),
                         .dftse_i                (DFTSE),
                         .ds_cpu0_cpuectlr_smp   (ds_cpu0_cpuectlr_smp),
                         .ds_cpu0_sev_req        (ds_cpu0_sev_req),
                         .ds_cpu1_cpuectlr_smp   (ds_cpu1_cpuectlr_smp),
                         .ds_cpu1_sev_req        (ds_cpu1_sev_req),
                         .ds_cpu2_cpuectlr_smp   (ds_cpu2_cpuectlr_smp),
                         .ds_cpu2_sev_req        (ds_cpu2_sev_req),
                         .ds_cpu3_cpuectlr_smp   (ds_cpu3_cpuectlr_smp),
                         .ds_cpu3_sev_req        (ds_cpu3_sev_req),
                         .eventi_i               (EVENTI),
                         .giccdisable_i          (GICCDISABLE),
                         .l2_reset3              (l2_reset3),
                         .ncorereset_i           (nCORERESET[`MAIA_CN:0]),
                         .ncpuporeset_i          (nCPUPORESET[`MAIA_CN:0]),
                         .niden_i                (NIDEN[`MAIA_CN:0]),
                         .nmbistreset_i          (nMBISTRESET),
                         .pm_export_cpu0_i       (pm_export_cpu0_i),
                         .pm_export_cpu1_i       (pm_export_cpu1_i),
                         .pm_export_cpu2_i       (pm_export_cpu2_i),
                         .pm_export_cpu3_i       (pm_export_cpu3_i),
                         .pmuevent_cpu0_i        (pmuevent_cpu0_i[24:0]),
                         .pmuevent_cpu1_i        (pmuevent_cpu1_i[24:0]),
                         .pmuevent_cpu2_i        (pmuevent_cpu2_i[24:0]),
                         .pmuevent_cpu3_i        (pmuevent_cpu3_i[24:0]),
                         .rvbaraddr0_i           (RVBARADDR0[43:2]),
                         .rvbaraddr1_i           (RVBARADDR1[43:2]),
                         .rvbaraddr2_i           (RVBARADDR2[43:2]),
                         .rvbaraddr3_i           (RVBARADDR3[43:2]),
                         .spiden_i               (SPIDEN[`MAIA_CN:0]),
                         .spniden_i              (SPNIDEN[`MAIA_CN:0]),
                         .vinithi_i              (VINITHI[`MAIA_CN:0])
                       );    // ucpu_io

    maia_dt_sb udt_sb( // outputs
                       .afreadym0_o                            (AFREADYM0),
                       .afreadym1_o                            (AFREADYM1),
                       .afreadym2_o                            (AFREADYM2),
                       .afreadym3_o                            (AFREADYM3),
                       .afvalidm_cpu0_o                        (afvalidm_cpu0_o),
                       .afvalidm_cpu1_o                        (afvalidm_cpu1_o),
                       .afvalidm_cpu2_o                        (afvalidm_cpu2_o),
                       .afvalidm_cpu3_o                        (afvalidm_cpu3_o),
                       .atbytesm0_o                            (ATBYTESM0[1:0]),
                       .atbytesm1_o                            (ATBYTESM1[1:0]),
                       .atbytesm2_o                            (ATBYTESM2[1:0]),
                       .atbytesm3_o                            (ATBYTESM3[1:0]),
                       .atclken_cpu0_o                         (atclken_cpu0_o),
                       .atclken_cpu1_o                         (atclken_cpu1_o),
                       .atclken_cpu2_o                         (atclken_cpu2_o),
                       .atclken_cpu3_o                         (atclken_cpu3_o),
                       .atdatam0_o                             (ATDATAM0[31:0]),
                       .atdatam1_o                             (ATDATAM1[31:0]),
                       .atdatam2_o                             (ATDATAM2[31:0]),
                       .atdatam3_o                             (ATDATAM3[31:0]),
                       .atidm0_o                               (ATIDM0[6:0]),
                       .atidm1_o                               (ATIDM1[6:0]),
                       .atidm2_o                               (ATIDM2[6:0]),
                       .atidm3_o                               (ATIDM3[6:0]),
                       .atreadym_cpu0_o                        (atreadym_cpu0_o),
                       .atreadym_cpu1_o                        (atreadym_cpu1_o),
                       .atreadym_cpu2_o                        (atreadym_cpu2_o),
                       .atreadym_cpu3_o                        (atreadym_cpu3_o),
                       .atvalidm0_o                            (ATVALIDM0),
                       .atvalidm1_o                            (ATVALIDM1),
                       .atvalidm2_o                            (ATVALIDM2),
                       .atvalidm3_o                            (ATVALIDM3),
                       .syncreqm_cpu0_o                        (syncreqm_cpu0_o),
                       .syncreqm_cpu1_o                        (syncreqm_cpu1_o),
                       .syncreqm_cpu2_o                        (syncreqm_cpu2_o),
                       .syncreqm_cpu3_o                        (syncreqm_cpu3_o),
                       .tsvalueb_cpu0_o                        (tsvalueb_cpu0_o[63:0]),
                       .tsvalueb_cpu1_o                        (tsvalueb_cpu1_o[63:0]),
                       .tsvalueb_cpu2_o                        (tsvalueb_cpu2_o[63:0]),
                       .tsvalueb_cpu3_o                        (tsvalueb_cpu3_o[63:0]),

                       // inputs
                       .DFTMCPHOLD                             (DFTMCPHOLD),
                       .DFTRSTDISABLE                          (DFTRSTDISABLE),
                       .DFTSE                                  (DFTSE),
                       .TSVALUEB                               (TSVALUEB[63:0]),
                       .afreadym_cpu0_i                        (afreadym_cpu0_i),
                       .afreadym_cpu1_i                        (afreadym_cpu1_i),
                       .afreadym_cpu2_i                        (afreadym_cpu2_i),
                       .afreadym_cpu3_i                        (afreadym_cpu3_i),
                       .afvalidm0_i                            (AFVALIDM0),
                       .afvalidm1_i                            (AFVALIDM1),
                       .afvalidm2_i                            (AFVALIDM2),
                       .afvalidm3_i                            (AFVALIDM3),
                       .atbytesm_cpu0_i                        (atbytesm_cpu0_i[1:0]),
                       .atbytesm_cpu1_i                        (atbytesm_cpu1_i[1:0]),
                       .atbytesm_cpu2_i                        (atbytesm_cpu2_i[1:0]),
                       .atbytesm_cpu3_i                        (atbytesm_cpu3_i[1:0]),
                       .atclken_i                              (ATCLKEN),
                       .atdatam_cpu0_i                         (atdatam_cpu0_i[31:0]),
                       .atdatam_cpu1_i                         (atdatam_cpu1_i[31:0]),
                       .atdatam_cpu2_i                         (atdatam_cpu2_i[31:0]),
                       .atdatam_cpu3_i                         (atdatam_cpu3_i[31:0]),
                       .atidm_cpu0_i                           (atidm_cpu0_i[6:0]),
                       .atidm_cpu1_i                           (atidm_cpu1_i[6:0]),
                       .atidm_cpu2_i                           (atidm_cpu2_i[6:0]),
                       .atidm_cpu3_i                           (atidm_cpu3_i[6:0]),
                       .atreadym0_i                            (ATREADYM0),
                       .atreadym1_i                            (ATREADYM1),
                       .atreadym2_i                            (ATREADYM2),
                       .atreadym3_i                            (ATREADYM3),
                       .atvalidm_cpu0_i                        (atvalidm_cpu0_i),
                       .atvalidm_cpu1_i                        (atvalidm_cpu1_i),
                       .atvalidm_cpu2_i                        (atvalidm_cpu2_i),
                       .atvalidm_cpu3_i                        (atvalidm_cpu3_i),
                       .ck_gclkfr                              (ck_gclkfr),
                       .dt_cpu0_trcauxctlr_sb_rcg_disable_pclk (dt_cpu0_trcauxctlr_sb_rcg_disable_pclk),
                       .dt_cpu1_trcauxctlr_sb_rcg_disable_pclk (dt_cpu1_trcauxctlr_sb_rcg_disable_pclk),
                       .dt_cpu2_trcauxctlr_sb_rcg_disable_pclk (dt_cpu2_trcauxctlr_sb_rcg_disable_pclk),
                       .dt_cpu3_trcauxctlr_sb_rcg_disable_pclk (dt_cpu3_trcauxctlr_sb_rcg_disable_pclk),
                       .etclken_cpu0_i                         (etclken_cpu0_i),
                       .etclken_cpu1_i                         (etclken_cpu1_i),
                       .etclken_cpu2_i                         (etclken_cpu2_i),
                       .etclken_cpu3_i                         (etclken_cpu3_i),
                       .nCPUPORESET                            (nCPUPORESET[`MAIA_CN:0]),
                       .nMBISTRESET                            (nMBISTRESET),
                       .syncreqm0_i                            (SYNCREQM0),
                       .syncreqm1_i                            (SYNCREQM1),
                       .syncreqm2_i                            (SYNCREQM2),
                       .syncreqm3_i                            (SYNCREQM3)
                     );    // udt_sb

    maia_ncpu_reg_rep uncpu_reg_rep( // outputs
                                     .ds_cpu0_ic_aa64naa32_reg_o   (ds_cpu0_ic_aa64naa32_i),
                                     .ds_cpu0_ic_cpsr_mode_reg_o   (ds_cpu0_ic_cpsr_mode_i[4:0]),
                                     .ds_cpu0_ic_hcr_change_reg_o  (ds_cpu0_ic_hcr_change_i),
                                     .ds_cpu0_ic_sample_spr_reg_o  (ds_cpu0_ic_sample_spr_i),
                                     .ds_cpu0_ic_scr_change_reg_o  (ds_cpu0_ic_scr_change_i),
                                     .ds_cpu1_ic_aa64naa32_reg_o   (ds_cpu1_ic_aa64naa32_i),
                                     .ds_cpu1_ic_cpsr_mode_reg_o   (ds_cpu1_ic_cpsr_mode_i[4:0]),
                                     .ds_cpu1_ic_hcr_change_reg_o  (ds_cpu1_ic_hcr_change_i),
                                     .ds_cpu1_ic_sample_spr_reg_o  (ds_cpu1_ic_sample_spr_i),
                                     .ds_cpu1_ic_scr_change_reg_o  (ds_cpu1_ic_scr_change_i),
                                     .ds_cpu2_ic_aa64naa32_reg_o   (ds_cpu2_ic_aa64naa32_i),
                                     .ds_cpu2_ic_cpsr_mode_reg_o   (ds_cpu2_ic_cpsr_mode_i[4:0]),
                                     .ds_cpu2_ic_hcr_change_reg_o  (ds_cpu2_ic_hcr_change_i),
                                     .ds_cpu2_ic_sample_spr_reg_o  (ds_cpu2_ic_sample_spr_i),
                                     .ds_cpu2_ic_scr_change_reg_o  (ds_cpu2_ic_scr_change_i),
                                     .ds_cpu3_ic_aa64naa32_reg_o   (ds_cpu3_ic_aa64naa32_i),
                                     .ds_cpu3_ic_cpsr_mode_reg_o   (ds_cpu3_ic_cpsr_mode_i[4:0]),
                                     .ds_cpu3_ic_hcr_change_reg_o  (ds_cpu3_ic_hcr_change_i),
                                     .ds_cpu3_ic_sample_spr_reg_o  (ds_cpu3_ic_sample_spr_i),
                                     .ds_cpu3_ic_scr_change_reg_o  (ds_cpu3_ic_scr_change_i),
                                     .ic_block_eoi_sgi_wr_reg_o    (ic_block_eoi_sgi_wr[`MAIA_CN:0]),
                                     .ic_el_change_complete_reg_o  (ic_el_change_complete[`MAIA_CN:0]),
                                     .ic_hcr_change_complete_reg_o (ic_hcr_change_complete[`MAIA_CN:0]),
                                     .ic_ich_el2_tall0_reg_o       (ic_ich_el2_tall0[`MAIA_CN:0]),
                                     .ic_ich_el2_tall1_reg_o       (ic_ich_el2_tall1[`MAIA_CN:0]),
                                     .ic_ich_el2_tc_reg_o          (ic_ich_el2_tc[`MAIA_CN:0]),
                                     .ic_nfiq_reg_o                (ic_nfiq[`MAIA_CN:0]),
                                     .ic_nirq_reg_o                (ic_nirq[`MAIA_CN:0]),
                                     .ic_nsei_reg_o                (ic_nsei[`MAIA_CN:0]),
                                     .ic_nvfiq_reg_o               (ic_nvfiq[`MAIA_CN:0]),
                                     .ic_nvirq_reg_o               (ic_nvirq[`MAIA_CN:0]),
                                     .ic_nvsei_reg_o               (ic_nvsei[`MAIA_CN:0]),
                                     .ic_sample_spr_reg_o          (ic_sample_spr[`MAIA_CN:0]),
                                     .ic_scr_change_complete_reg_o (ic_scr_change_complete[`MAIA_CN:0]),
                                     .ic_sra_el1ns_en_reg_o        (ic_sra_el1ns_en[`MAIA_CN:0]),
                                     .ic_sra_el1s_en_reg_o         (ic_sra_el1s_en[`MAIA_CN:0]),
                                     .ic_sra_el2_en_reg_o          (ic_sra_el2_en[`MAIA_CN:0]),
                                     .ic_sra_el3_en_reg_o          (ic_sra_el3_en[`MAIA_CN:0]),
                                     .ic_sre_el1ns_hyp_trap_reg_o  (ic_sre_el1ns_hyp_trap[`MAIA_CN:0]),
                                     .ic_sre_el1ns_mon_trap_reg_o  (ic_sre_el1ns_mon_trap[`MAIA_CN:0]),
                                     .ic_sre_el1s_mon_trap_reg_o   (ic_sre_el1s_mon_trap[`MAIA_CN:0]),
                                     .ic_sre_el2_mon_trap_reg_o    (ic_sre_el2_mon_trap[`MAIA_CN:0]),

                                     // inputs
                                     .ck_gclkfr                    (ck_gclkfr),
                                     .ck_reset1_n_l2               (ck_reset1_n_l2),
                                     .ds_cpu0_ic_aa64naa32         (ds_cpu0_ic_aa64naa32),
                                     .ds_cpu0_ic_cpsr_mode         (ds_cpu0_ic_cpsr_mode[4:0]),
                                     .ds_cpu0_ic_hcr_change        (ds_cpu0_ic_hcr_change),
                                     .ds_cpu0_ic_sample_spr        (ds_cpu0_ic_sample_spr),
                                     .ds_cpu0_ic_scr_change        (ds_cpu0_ic_scr_change),
                                     .ds_cpu1_ic_aa64naa32         (ds_cpu1_ic_aa64naa32),
                                     .ds_cpu1_ic_cpsr_mode         (ds_cpu1_ic_cpsr_mode[4:0]),
                                     .ds_cpu1_ic_hcr_change        (ds_cpu1_ic_hcr_change),
                                     .ds_cpu1_ic_sample_spr        (ds_cpu1_ic_sample_spr),
                                     .ds_cpu1_ic_scr_change        (ds_cpu1_ic_scr_change),
                                     .ds_cpu2_ic_aa64naa32         (ds_cpu2_ic_aa64naa32),
                                     .ds_cpu2_ic_cpsr_mode         (ds_cpu2_ic_cpsr_mode[4:0]),
                                     .ds_cpu2_ic_hcr_change        (ds_cpu2_ic_hcr_change),
                                     .ds_cpu2_ic_sample_spr        (ds_cpu2_ic_sample_spr),
                                     .ds_cpu2_ic_scr_change        (ds_cpu2_ic_scr_change),
                                     .ds_cpu3_ic_aa64naa32         (ds_cpu3_ic_aa64naa32),
                                     .ds_cpu3_ic_cpsr_mode         (ds_cpu3_ic_cpsr_mode[4:0]),
                                     .ds_cpu3_ic_hcr_change        (ds_cpu3_ic_hcr_change),
                                     .ds_cpu3_ic_sample_spr        (ds_cpu3_ic_sample_spr),
                                     .ds_cpu3_ic_scr_change        (ds_cpu3_ic_scr_change),
                                     .ic_block_eoi_sgi_wr          (ic_block_eoi_sgi_wr_o[`MAIA_CN:0]),
                                     .ic_el_change_complete        (ic_el_change_complete_o[`MAIA_CN:0]),
                                     .ic_hcr_change_complete       (ic_hcr_change_complete_o[`MAIA_CN:0]),
                                     .ic_ich_el2_tall0             (ic_ich_el2_tall0_o[`MAIA_CN:0]),
                                     .ic_ich_el2_tall1             (ic_ich_el2_tall1_o[`MAIA_CN:0]),
                                     .ic_ich_el2_tc                (ic_ich_el2_tc_o[`MAIA_CN:0]),
                                     .ic_nfiq                      (ic_nfiq_o[`MAIA_CN:0]),
                                     .ic_nirq                      (ic_nirq_o[`MAIA_CN:0]),
                                     .ic_nsei                      (ic_nsei_o[`MAIA_CN:0]),
                                     .ic_nvfiq                     (ic_nvfiq_o[`MAIA_CN:0]),
                                     .ic_nvirq                     (ic_nvirq_o[`MAIA_CN:0]),
                                     .ic_nvsei                     (ic_nvsei_o[`MAIA_CN:0]),
                                     .ic_sample_spr                (ic_sample_spr_o[`MAIA_CN:0]),
                                     .ic_scr_change_complete       (ic_scr_change_complete_o[`MAIA_CN:0]),
                                     .ic_sra_el1ns_en              (ic_sra_el1ns_en_o[`MAIA_CN:0]),
                                     .ic_sra_el1s_en               (ic_sra_el1s_en_o[`MAIA_CN:0]),
                                     .ic_sra_el2_en                (ic_sra_el2_en_o[`MAIA_CN:0]),
                                     .ic_sra_el3_en                (ic_sra_el3_en_o[`MAIA_CN:0]),
                                     .ic_sre_el1ns_hyp_trap        (ic_sre_el1ns_hyp_trap_o[`MAIA_CN:0]),
                                     .ic_sre_el1ns_mon_trap        (ic_sre_el1ns_mon_trap_o[`MAIA_CN:0]),
                                     .ic_sre_el1s_mon_trap         (ic_sre_el1s_mon_trap_o[`MAIA_CN:0]),
                                     .ic_sre_el2_mon_trap          (ic_sre_el2_mon_trap_o[`MAIA_CN:0])
                                   );    // uncpu_reg_rep

//-----------------------------------------------------------------------------
// OVL Assertions
//-----------------------------------------------------------------------------
`ifdef ARM_ASSERT_ON
  `include "maia_noncpu_s_val.v"
`endif

endmodule  // maia_noncpu_s

//ARMAUTO UNDEF START
`define MAIA_UNDEFINE
`include "maia_header.v"
`undef MAIA_UNDEFINE
//ARMAUTO UNDEF END
