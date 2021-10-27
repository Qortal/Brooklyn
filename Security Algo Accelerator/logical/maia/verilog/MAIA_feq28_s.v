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
//     Filename            : $RCSfile: MAIA_feq28.v $
//     Checked In          : $Date: 2014-10-14 15:20:06 -0500 (Tue, 14 Oct 2014) $
//     Revision            : $Revision: 71806 $
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
// This is top-level interconnect layer for the MAIA_feq28 top-level.
//

//#
//# Module Declaration
//# ==================
//#

`include "maia_header.v"

`define MAIA_CN 3

module MAIA_feq28_s (
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
              MBISTREQ
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


    // wires
    wire              aa64naa32_cpu0_o;
    wire              aa64naa32_cpu1_o;
    wire              aa64naa32_cpu2_o;
    wire              aa64naa32_cpu3_o;
    wire              afreadym_cpu0_i;
    wire              afreadym_cpu1_i;
    wire              afreadym_cpu2_i;
    wire              afreadym_cpu3_i;
    wire              afvalidm_cpu0_o;
    wire              afvalidm_cpu1_o;
    wire              afvalidm_cpu2_o;
    wire              afvalidm_cpu3_o;
    wire [1:0]        atbytesm_cpu0_i;
    wire [1:0]        atbytesm_cpu1_i;
    wire [1:0]        atbytesm_cpu2_i;
    wire [1:0]        atbytesm_cpu3_i;
    wire              atclken_cpu0_o;
    wire              atclken_cpu1_o;
    wire              atclken_cpu2_o;
    wire              atclken_cpu3_o;
    wire [31:0]       atdatam_cpu0_i;
    wire [31:0]       atdatam_cpu1_i;
    wire [31:0]       atdatam_cpu2_i;
    wire [31:0]       atdatam_cpu3_i;
    wire [6:0]        atidm_cpu0_i;
    wire [6:0]        atidm_cpu1_i;
    wire [6:0]        atidm_cpu2_i;
    wire [6:0]        atidm_cpu3_i;
    wire              atreadym_cpu0_o;
    wire              atreadym_cpu1_o;
    wire              atreadym_cpu2_o;
    wire              atreadym_cpu3_o;
    wire              atvalidm_cpu0_i;
    wire              atvalidm_cpu1_i;
    wire              atvalidm_cpu2_i;
    wire              atvalidm_cpu3_i;
    wire              cfgend_cpu0_o;
    wire              cfgend_cpu1_o;
    wire              cfgend_cpu2_o;
    wire              cfgend_cpu3_o;
    wire              cfgte_cpu0_o;
    wire              cfgte_cpu1_o;
    wire              cfgte_cpu2_o;
    wire              cfgte_cpu3_o;
    wire              ck_cpu0_crcx_clk_en_n;
    wire              ck_cpu0_event_reg;
    wire              ck_cpu0_wfe_ack;
    wire              ck_cpu0_wfi_ack;
    wire              ck_cpu1_crcx_clk_en_n;
    wire              ck_cpu1_event_reg;
    wire              ck_cpu1_wfe_ack;
    wire              ck_cpu1_wfi_ack;
    wire              ck_cpu2_crcx_clk_en_n;
    wire              ck_cpu2_event_reg;
    wire              ck_cpu2_wfe_ack;
    wire              ck_cpu2_wfi_ack;
    wire              ck_cpu3_crcx_clk_en_n;
    wire              ck_cpu3_event_reg;
    wire              ck_cpu3_wfe_ack;
    wire              ck_cpu3_wfi_ack;
    wire [`MAIA_CN:0] ck_gclkt;
    wire [7:0]        clusteridaff1_cpu0_o;
    wire [7:0]        clusteridaff1_cpu1_o;
    wire [7:0]        clusteridaff1_cpu2_o;
    wire [7:0]        clusteridaff1_cpu3_o;
    wire [7:0]        clusteridaff2_cpu0_o;
    wire [7:0]        clusteridaff2_cpu1_o;
    wire [7:0]        clusteridaff2_cpu2_o;
    wire [7:0]        clusteridaff2_cpu3_o;
    wire              commrx_cpu0_i;
    wire              commrx_cpu1_i;
    wire              commrx_cpu2_i;
    wire              commrx_cpu3_i;
    wire              commtx_cpu0_i;
    wire              commtx_cpu1_i;
    wire              commtx_cpu2_i;
    wire              commtx_cpu3_i;
    wire              cp15sdisable_cpu0_o;
    wire              cp15sdisable_cpu1_o;
    wire              cp15sdisable_cpu2_o;
    wire              cp15sdisable_cpu3_o;
    wire [1:0]        cpuid_cpu0_o;
    wire [1:0]        cpuid_cpu1_o;
    wire [1:0]        cpuid_cpu2_o;
    wire [1:0]        cpuid_cpu3_o;
    wire              cryptodisable_cpu0_o;
    wire              cryptodisable_cpu1_o;
    wire              cryptodisable_cpu2_o;
    wire              cryptodisable_cpu3_o;
    wire              dbgack_cpu0_i;
    wire              dbgack_cpu1_i;
    wire              dbgack_cpu2_i;
    wire              dbgack_cpu3_i;
    wire              dbgen_cpu0_o;
    wire              dbgen_cpu1_o;
    wire              dbgen_cpu2_o;
    wire              dbgen_cpu3_o;
    wire              dbgl1rstdisable_cpu0_o;
    wire              dbgl1rstdisable_cpu1_o;
    wire              dbgl1rstdisable_cpu2_o;
    wire              dbgl1rstdisable_cpu3_o;
    wire              dbgnopwrdwn_cpu0_i;
    wire              dbgnopwrdwn_cpu1_i;
    wire              dbgnopwrdwn_cpu2_i;
    wire              dbgnopwrdwn_cpu3_i;
    wire [43:12]      dbgromaddr_cpu0_o;
    wire [43:12]      dbgromaddr_cpu1_o;
    wire [43:12]      dbgromaddr_cpu2_o;
    wire [43:12]      dbgromaddr_cpu3_o;
    wire              dbgromaddrv_cpu0_o;
    wire              dbgromaddrv_cpu1_o;
    wire              dbgromaddrv_cpu2_o;
    wire              dbgromaddrv_cpu3_o;
    wire              dbgrstreq_cpu0_i;
    wire              dbgrstreq_cpu1_i;
    wire              dbgrstreq_cpu2_i;
    wire              dbgrstreq_cpu3_i;
    wire              dftcrclkdisable_cpu0_o;
    wire              dftcrclkdisable_cpu1_o;
    wire              dftcrclkdisable_cpu2_o;
    wire              dftcrclkdisable_cpu3_o;
    wire              dftramhold_cpu0_o;
    wire              dftramhold_cpu1_o;
    wire              dftramhold_cpu2_o;
    wire              dftramhold_cpu3_o;
    wire              dftrstdisable_cpu0_o;
    wire              dftrstdisable_cpu1_o;
    wire              dftrstdisable_cpu2_o;
    wire              dftrstdisable_cpu3_o;
    wire              dftse_cpu0_o;
    wire              dftse_cpu1_o;
    wire              dftse_cpu2_o;
    wire              dftse_cpu3_o;
    wire [2:0]        ds_cpu0_cpuectlr_ret;
    wire              ds_cpu0_cpuectlr_smp;
    wire              ds_cpu0_fiq_wfe_qual;
    wire              ds_cpu0_fiq_wfi_qual;
    wire              ds_cpu0_flush;
    wire [5:0]        ds_cpu0_flush_type;
    wire              ds_cpu0_hcr_va;
    wire              ds_cpu0_hcr_vf;
    wire              ds_cpu0_hcr_vi;
    wire              ds_cpu0_ic_aa64naa32;
    wire [4:0]        ds_cpu0_ic_cpsr_mode;
    wire              ds_cpu0_ic_hcr_change;
    wire              ds_cpu0_ic_sample_spr;
    wire              ds_cpu0_ic_scr_change;
    wire              ds_cpu0_imp_abrt_wfe_qual;
    wire              ds_cpu0_imp_abrt_wfi_qual;
    wire              ds_cpu0_irq_wfe_qual;
    wire              ds_cpu0_irq_wfi_qual;
    wire [8:0]        ds_cpu0_l2_spr_addr;
    wire              ds_cpu0_l2_spr_dw;
    wire              ds_cpu0_l2_spr_en;
    wire              ds_cpu0_l2_spr_rd;
    wire              ds_cpu0_l2_spr_wr;
    wire [63:0]       ds_cpu0_l2_spr_wr_data;
    wire              ds_cpu0_reset_req;
    wire              ds_cpu0_sev_req;
    wire              ds_cpu0_sevl_req;
    wire              ds_cpu0_vfiq_wfe_qual;
    wire              ds_cpu0_vfiq_wfi_qual;
    wire              ds_cpu0_vimp_abrt_wfe_qual;
    wire              ds_cpu0_vimp_abrt_wfi_qual;
    wire              ds_cpu0_virq_wfe_qual;
    wire              ds_cpu0_virq_wfi_qual;
    wire              ds_cpu0_wfe_req;
    wire              ds_cpu0_wfi_req;
    wire [2:0]        ds_cpu1_cpuectlr_ret;
    wire              ds_cpu1_cpuectlr_smp;
    wire              ds_cpu1_fiq_wfe_qual;
    wire              ds_cpu1_fiq_wfi_qual;
    wire              ds_cpu1_flush;
    wire [5:0]        ds_cpu1_flush_type;
    wire              ds_cpu1_hcr_va;
    wire              ds_cpu1_hcr_vf;
    wire              ds_cpu1_hcr_vi;
    wire              ds_cpu1_ic_aa64naa32;
    wire [4:0]        ds_cpu1_ic_cpsr_mode;
    wire              ds_cpu1_ic_hcr_change;
    wire              ds_cpu1_ic_sample_spr;
    wire              ds_cpu1_ic_scr_change;
    wire              ds_cpu1_imp_abrt_wfe_qual;
    wire              ds_cpu1_imp_abrt_wfi_qual;
    wire              ds_cpu1_irq_wfe_qual;
    wire              ds_cpu1_irq_wfi_qual;
    wire [8:0]        ds_cpu1_l2_spr_addr;
    wire              ds_cpu1_l2_spr_dw;
    wire              ds_cpu1_l2_spr_en;
    wire              ds_cpu1_l2_spr_rd;
    wire              ds_cpu1_l2_spr_wr;
    wire [63:0]       ds_cpu1_l2_spr_wr_data;
    wire              ds_cpu1_reset_req;
    wire              ds_cpu1_sev_req;
    wire              ds_cpu1_sevl_req;
    wire              ds_cpu1_vfiq_wfe_qual;
    wire              ds_cpu1_vfiq_wfi_qual;
    wire              ds_cpu1_vimp_abrt_wfe_qual;
    wire              ds_cpu1_vimp_abrt_wfi_qual;
    wire              ds_cpu1_virq_wfe_qual;
    wire              ds_cpu1_virq_wfi_qual;
    wire              ds_cpu1_wfe_req;
    wire              ds_cpu1_wfi_req;
    wire [2:0]        ds_cpu2_cpuectlr_ret;
    wire              ds_cpu2_cpuectlr_smp;
    wire              ds_cpu2_fiq_wfe_qual;
    wire              ds_cpu2_fiq_wfi_qual;
    wire              ds_cpu2_flush;
    wire [5:0]        ds_cpu2_flush_type;
    wire              ds_cpu2_hcr_va;
    wire              ds_cpu2_hcr_vf;
    wire              ds_cpu2_hcr_vi;
    wire              ds_cpu2_ic_aa64naa32;
    wire [4:0]        ds_cpu2_ic_cpsr_mode;
    wire              ds_cpu2_ic_hcr_change;
    wire              ds_cpu2_ic_sample_spr;
    wire              ds_cpu2_ic_scr_change;
    wire              ds_cpu2_imp_abrt_wfe_qual;
    wire              ds_cpu2_imp_abrt_wfi_qual;
    wire              ds_cpu2_irq_wfe_qual;
    wire              ds_cpu2_irq_wfi_qual;
    wire [8:0]        ds_cpu2_l2_spr_addr;
    wire              ds_cpu2_l2_spr_dw;
    wire              ds_cpu2_l2_spr_en;
    wire              ds_cpu2_l2_spr_rd;
    wire              ds_cpu2_l2_spr_wr;
    wire [63:0]       ds_cpu2_l2_spr_wr_data;
    wire              ds_cpu2_reset_req;
    wire              ds_cpu2_sev_req;
    wire              ds_cpu2_sevl_req;
    wire              ds_cpu2_vfiq_wfe_qual;
    wire              ds_cpu2_vfiq_wfi_qual;
    wire              ds_cpu2_vimp_abrt_wfe_qual;
    wire              ds_cpu2_vimp_abrt_wfi_qual;
    wire              ds_cpu2_virq_wfe_qual;
    wire              ds_cpu2_virq_wfi_qual;
    wire              ds_cpu2_wfe_req;
    wire              ds_cpu2_wfi_req;
    wire [2:0]        ds_cpu3_cpuectlr_ret;
    wire              ds_cpu3_cpuectlr_smp;
    wire              ds_cpu3_fiq_wfe_qual;
    wire              ds_cpu3_fiq_wfi_qual;
    wire              ds_cpu3_flush;
    wire [5:0]        ds_cpu3_flush_type;
    wire              ds_cpu3_hcr_va;
    wire              ds_cpu3_hcr_vf;
    wire              ds_cpu3_hcr_vi;
    wire              ds_cpu3_ic_aa64naa32;
    wire [4:0]        ds_cpu3_ic_cpsr_mode;
    wire              ds_cpu3_ic_hcr_change;
    wire              ds_cpu3_ic_sample_spr;
    wire              ds_cpu3_ic_scr_change;
    wire              ds_cpu3_imp_abrt_wfe_qual;
    wire              ds_cpu3_imp_abrt_wfi_qual;
    wire              ds_cpu3_irq_wfe_qual;
    wire              ds_cpu3_irq_wfi_qual;
    wire [8:0]        ds_cpu3_l2_spr_addr;
    wire              ds_cpu3_l2_spr_dw;
    wire              ds_cpu3_l2_spr_en;
    wire              ds_cpu3_l2_spr_rd;
    wire              ds_cpu3_l2_spr_wr;
    wire [63:0]       ds_cpu3_l2_spr_wr_data;
    wire              ds_cpu3_reset_req;
    wire              ds_cpu3_sev_req;
    wire              ds_cpu3_sevl_req;
    wire              ds_cpu3_vfiq_wfe_qual;
    wire              ds_cpu3_vfiq_wfi_qual;
    wire              ds_cpu3_vimp_abrt_wfe_qual;
    wire              ds_cpu3_vimp_abrt_wfi_qual;
    wire              ds_cpu3_virq_wfe_qual;
    wire              ds_cpu3_virq_wfi_qual;
    wire              ds_cpu3_wfe_req;
    wire              ds_cpu3_wfi_req;
    wire              dt_cpu0_coredbg_in_reset_gclk;
    wire [1:0]        dt_cpu0_cti_trigin_1to0_gclk;
    wire [3:0]        dt_cpu0_cti_trigin_7to4_gclk;
    wire [1:0]        dt_cpu0_cti_triginack_1to0_pclk;
    wire [3:0]        dt_cpu0_cti_triginack_7to4_pclk;
    wire [1:0]        dt_cpu0_cti_trigout_1to0_pclk;
    wire [3:0]        dt_cpu0_cti_trigout_7to4_pclk;
    wire [3:0]        dt_cpu0_cti_trigoutack_7to4_gclk;
    wire              dt_cpu0_cti_trigoutack_bit1_gclk;
    wire              dt_cpu0_dbif_ack_gclk;
    wire [14:2]       dt_cpu0_dbif_addr_pclk;
    wire              dt_cpu0_dbif_err_gclk;
    wire              dt_cpu0_dbif_locked_pclk;
    wire [31:0]       dt_cpu0_dbif_rddata_gclk;
    wire              dt_cpu0_dbif_req_pclk;
    wire [31:0]       dt_cpu0_dbif_wrdata_pclk;
    wire              dt_cpu0_dbif_write_pclk;
    wire              dt_cpu0_edacr_frc_idleack_pclk;
    wire              dt_cpu0_edbgrq_pclk;
    wire              dt_cpu0_edecr_osuce_pclk;
    wire              dt_cpu0_edecr_rce_pclk;
    wire              dt_cpu0_edecr_ss_pclk;
    wire              dt_cpu0_edprcr_corepurq_pclk;
    wire              dt_cpu0_et_oslock_gclk;
    wire              dt_cpu0_halt_ack_gclk;
    wire              dt_cpu0_hlt_dbgevt_ok_gclk;
    wire              dt_cpu0_noclkstop_pclk;
    wire              dt_cpu0_os_double_lock_gclk;
    wire              dt_cpu0_pmusnapshot_ack_gclk;
    wire              dt_cpu0_pmusnapshot_req_pclk;
    wire              dt_cpu0_wfx_dbg_req_gclk;
    wire              dt_cpu0_wfx_wakeup_pclk;
    wire              dt_cpu1_coredbg_in_reset_gclk;
    wire [1:0]        dt_cpu1_cti_trigin_1to0_gclk;
    wire [3:0]        dt_cpu1_cti_trigin_7to4_gclk;
    wire [1:0]        dt_cpu1_cti_triginack_1to0_pclk;
    wire [3:0]        dt_cpu1_cti_triginack_7to4_pclk;
    wire [1:0]        dt_cpu1_cti_trigout_1to0_pclk;
    wire [3:0]        dt_cpu1_cti_trigout_7to4_pclk;
    wire [3:0]        dt_cpu1_cti_trigoutack_7to4_gclk;
    wire              dt_cpu1_cti_trigoutack_bit1_gclk;
    wire              dt_cpu1_dbif_ack_gclk;
    wire [14:2]       dt_cpu1_dbif_addr_pclk;
    wire              dt_cpu1_dbif_err_gclk;
    wire              dt_cpu1_dbif_locked_pclk;
    wire [31:0]       dt_cpu1_dbif_rddata_gclk;
    wire              dt_cpu1_dbif_req_pclk;
    wire [31:0]       dt_cpu1_dbif_wrdata_pclk;
    wire              dt_cpu1_dbif_write_pclk;
    wire              dt_cpu1_edacr_frc_idleack_pclk;
    wire              dt_cpu1_edbgrq_pclk;
    wire              dt_cpu1_edecr_osuce_pclk;
    wire              dt_cpu1_edecr_rce_pclk;
    wire              dt_cpu1_edecr_ss_pclk;
    wire              dt_cpu1_edprcr_corepurq_pclk;
    wire              dt_cpu1_et_oslock_gclk;
    wire              dt_cpu1_halt_ack_gclk;
    wire              dt_cpu1_hlt_dbgevt_ok_gclk;
    wire              dt_cpu1_noclkstop_pclk;
    wire              dt_cpu1_os_double_lock_gclk;
    wire              dt_cpu1_pmusnapshot_ack_gclk;
    wire              dt_cpu1_pmusnapshot_req_pclk;
    wire              dt_cpu1_wfx_dbg_req_gclk;
    wire              dt_cpu1_wfx_wakeup_pclk;
    wire              dt_cpu2_coredbg_in_reset_gclk;
    wire [1:0]        dt_cpu2_cti_trigin_1to0_gclk;
    wire [3:0]        dt_cpu2_cti_trigin_7to4_gclk;
    wire [1:0]        dt_cpu2_cti_triginack_1to0_pclk;
    wire [3:0]        dt_cpu2_cti_triginack_7to4_pclk;
    wire [1:0]        dt_cpu2_cti_trigout_1to0_pclk;
    wire [3:0]        dt_cpu2_cti_trigout_7to4_pclk;
    wire [3:0]        dt_cpu2_cti_trigoutack_7to4_gclk;
    wire              dt_cpu2_cti_trigoutack_bit1_gclk;
    wire              dt_cpu2_dbif_ack_gclk;
    wire [14:2]       dt_cpu2_dbif_addr_pclk;
    wire              dt_cpu2_dbif_err_gclk;
    wire              dt_cpu2_dbif_locked_pclk;
    wire [31:0]       dt_cpu2_dbif_rddata_gclk;
    wire              dt_cpu2_dbif_req_pclk;
    wire [31:0]       dt_cpu2_dbif_wrdata_pclk;
    wire              dt_cpu2_dbif_write_pclk;
    wire              dt_cpu2_edacr_frc_idleack_pclk;
    wire              dt_cpu2_edbgrq_pclk;
    wire              dt_cpu2_edecr_osuce_pclk;
    wire              dt_cpu2_edecr_rce_pclk;
    wire              dt_cpu2_edecr_ss_pclk;
    wire              dt_cpu2_edprcr_corepurq_pclk;
    wire              dt_cpu2_et_oslock_gclk;
    wire              dt_cpu2_halt_ack_gclk;
    wire              dt_cpu2_hlt_dbgevt_ok_gclk;
    wire              dt_cpu2_noclkstop_pclk;
    wire              dt_cpu2_os_double_lock_gclk;
    wire              dt_cpu2_pmusnapshot_ack_gclk;
    wire              dt_cpu2_pmusnapshot_req_pclk;
    wire              dt_cpu2_wfx_dbg_req_gclk;
    wire              dt_cpu2_wfx_wakeup_pclk;
    wire              dt_cpu3_coredbg_in_reset_gclk;
    wire [1:0]        dt_cpu3_cti_trigin_1to0_gclk;
    wire [3:0]        dt_cpu3_cti_trigin_7to4_gclk;
    wire [1:0]        dt_cpu3_cti_triginack_1to0_pclk;
    wire [3:0]        dt_cpu3_cti_triginack_7to4_pclk;
    wire [1:0]        dt_cpu3_cti_trigout_1to0_pclk;
    wire [3:0]        dt_cpu3_cti_trigout_7to4_pclk;
    wire [3:0]        dt_cpu3_cti_trigoutack_7to4_gclk;
    wire              dt_cpu3_cti_trigoutack_bit1_gclk;
    wire              dt_cpu3_dbif_ack_gclk;
    wire [14:2]       dt_cpu3_dbif_addr_pclk;
    wire              dt_cpu3_dbif_err_gclk;
    wire              dt_cpu3_dbif_locked_pclk;
    wire [31:0]       dt_cpu3_dbif_rddata_gclk;
    wire              dt_cpu3_dbif_req_pclk;
    wire [31:0]       dt_cpu3_dbif_wrdata_pclk;
    wire              dt_cpu3_dbif_write_pclk;
    wire              dt_cpu3_edacr_frc_idleack_pclk;
    wire              dt_cpu3_edbgrq_pclk;
    wire              dt_cpu3_edecr_osuce_pclk;
    wire              dt_cpu3_edecr_rce_pclk;
    wire              dt_cpu3_edecr_ss_pclk;
    wire              dt_cpu3_edprcr_corepurq_pclk;
    wire              dt_cpu3_et_oslock_gclk;
    wire              dt_cpu3_halt_ack_gclk;
    wire              dt_cpu3_hlt_dbgevt_ok_gclk;
    wire              dt_cpu3_noclkstop_pclk;
    wire              dt_cpu3_os_double_lock_gclk;
    wire              dt_cpu3_pmusnapshot_ack_gclk;
    wire              dt_cpu3_pmusnapshot_req_pclk;
    wire              dt_cpu3_wfx_dbg_req_gclk;
    wire              dt_cpu3_wfx_wakeup_pclk;
    wire              etclken_cpu0_i;
    wire              etclken_cpu1_i;
    wire              etclken_cpu2_i;
    wire              etclken_cpu3_i;
    wire              giccdisable_cpu0_o;
    wire              giccdisable_cpu1_o;
    wire              giccdisable_cpu2_o;
    wire              giccdisable_cpu3_o;
    wire [`MAIA_CN:0] ic_block_eoi_sgi_wr;
    wire [`MAIA_CN:0] ic_el_change_complete;
    wire [`MAIA_CN:0] ic_hcr_change_complete;
    wire [`MAIA_CN:0] ic_ich_el2_tall0;
    wire [`MAIA_CN:0] ic_ich_el2_tall1;
    wire [`MAIA_CN:0] ic_ich_el2_tc;
    wire [`MAIA_CN:0] ic_nfiq;
    wire [`MAIA_CN:0] ic_nirq;
    wire [`MAIA_CN:0] ic_nsei;
    wire [`MAIA_CN:0] ic_nvfiq;
    wire [`MAIA_CN:0] ic_nvirq;
    wire [`MAIA_CN:0] ic_nvsei;
    wire [`MAIA_CN:0] ic_p_valid;
    wire [`MAIA_CN:0] ic_sample_spr;
    wire [`MAIA_CN:0] ic_scr_change_complete;
    wire [`MAIA_CN:0] ic_sra_el1ns_en;
    wire [`MAIA_CN:0] ic_sra_el1s_en;
    wire [`MAIA_CN:0] ic_sra_el2_en;
    wire [`MAIA_CN:0] ic_sra_el3_en;
    wire [`MAIA_CN:0] ic_sre_el1ns_hyp_trap;
    wire [`MAIA_CN:0] ic_sre_el1ns_mon_trap;
    wire [`MAIA_CN:0] ic_sre_el1s_mon_trap;
    wire [`MAIA_CN:0] ic_sre_el2_mon_trap;
    wire              l2_cpu0_arb_thrshld_timeout_en;
    wire              l2_cpu0_barrier_done;
    wire              l2_cpu0_blk_non_evict_wr;
    wire              l2_cpu0_ccb_dbg_req_c3;
    wire [48:0]       l2_cpu0_ccb_req_addr_c3;
    wire [4:0]        l2_cpu0_ccb_req_id_c3;
    wire [23:0]       l2_cpu0_ccb_req_info_c3;
    wire [8:0]        l2_cpu0_ccb_req_type_c3;
    wire              l2_cpu0_cfg_ecc_en;
    wire [2:0]        l2_cpu0_dbufid_r1;
    wire [129:0]      l2_cpu0_ddata_r2;
    wire              l2_cpu0_ddlb_ecc_err_r3;
    wire              l2_cpu0_dext_err_r2;
    wire              l2_cpu0_dext_err_type_r2;
    wire              l2_cpu0_disable_clean_evict_opt;
    wire              l2_cpu0_dlast_r1;
    wire              l2_cpu0_dsngl_ecc_err_r3;
    wire [3:0]        l2_cpu0_dsq_clr_id_q;
    wire              l2_cpu0_dsq_clr_vld_q;
    wire [3:0]        l2_cpu0_dsq_rd_buf_id;
    wire [15:0]       l2_cpu0_dsq_rd_byte_strb_q;
    wire [129:0]      l2_cpu0_dsq_rd_data_q;
    wire              l2_cpu0_dsq_rd_en;
    wire              l2_cpu0_dsq_rd_en_x2;
    wire              l2_cpu0_dt_pmu_evt_en;
    wire              l2_cpu0_dvalid_r1;
    wire              l2_cpu0_early_rd_reqe4_e5_q;
    wire [1:0]        l2_cpu0_flsh_if_rd_id_l4_dly;
    wire              l2_cpu0_flsh_if_rd_l4_dly;
    wire              l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire [2:0]        l2_cpu0_flsh_ls_rd_id_l2_dly;
    wire [2:0]        l2_cpu0_flsh_ls_rd_id_l4_dly;
    wire              l2_cpu0_flsh_ls_rd_l2_dly;
    wire              l2_cpu0_flsh_ls_rd_l4_dly;
    wire              l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu0_flsh_ls_wr_evict_l4_dly;
    wire [3:0]        l2_cpu0_flsh_ls_wr_id_l2_dly;
    wire [3:0]        l2_cpu0_flsh_ls_wr_id_l4_dly;
    wire              l2_cpu0_flsh_ls_wr_l2_dly;
    wire              l2_cpu0_flsh_ls_wr_l4_dly;
    wire              l2_cpu0_flsh_tw_rd_l4_dly;
    wire [1:0]        l2_cpu0_ibufid_r1;
    wire [15:0]       l2_cpu0_ic_addr_arb_set;
    wire              l2_cpu0_ic_arb_fast;
    wire              l2_cpu0_ic_barrier_stall_q;
    wire [43:18]      l2_cpu0_ic_base;
    wire [31:0]       l2_cpu0_ic_data_arb_set;
    wire [2:0]        l2_cpu0_ic_elem_size_arb_set;
    wire              l2_cpu0_ic_excl_arb_set;
    wire [2:0]        l2_cpu0_ic_id_arb_set;
    wire              l2_cpu0_ic_ns_arb_set;
    wire              l2_cpu0_ic_vld_skid;
    wire              l2_cpu0_ic_write_arb_set;
    wire [127:0]      l2_cpu0_idata_r2;
    wire              l2_cpu0_idlb_ecc_err_r3;
    wire              l2_cpu0_idle_block_reqs_q;
    wire              l2_cpu0_idle_wakeup_q;
    wire              l2_cpu0_iext_err_r2;
    wire              l2_cpu0_iext_err_type_r2;
    wire              l2_cpu0_if_ccb_clken_c3;
    wire              l2_cpu0_if_ccb_req_c3;
    wire              l2_cpu0_if_ccb_resp;
    wire [4:0]        l2_cpu0_if_ccb_resp_id;
    wire              l2_cpu0_if_sync_done_q;
    wire              l2_cpu0_if_sync_req;
    wire              l2_cpu0_ifq_haz_pending;
    wire              l2_cpu0_isngl_ecc_err_r3;
    wire              l2_cpu0_ivalid_r1;
    wire [1:0]        l2_cpu0_l2_cache_size;
    wire [5:0]        l2_cpu0_lrq_haz_clr_id_dcd_q;
    wire              l2_cpu0_lrq_haz_pending;
    wire              l2_cpu0_ls_ccb_clken_c3;
    wire              l2_cpu0_ls_ccb_data_wr;
    wire              l2_cpu0_ls_ccb_req_c3;
    wire              l2_cpu0_ls_ccb_resp;
    wire [4:0]        l2_cpu0_ls_ccb_resp_id;
    wire              l2_cpu0_ls_peq_coll_l4_dly;
    wire [3:0]        l2_cpu0_ls_rd_haz_id_arb_q;
    wire              l2_cpu0_ls_rd_haz_vld_arb_q;
    wire              l2_cpu0_ls_sync_req;
    wire [4:0]        l2_cpu0_ls_wr_ccb_id_w2a;
    wire [127:0]      l2_cpu0_ls_wr_data_w2a;
    wire              l2_cpu0_ls_wr_dirty_w2a;
    wire              l2_cpu0_ls_wr_err_w2a;
    wire [2:0]        l2_cpu0_ls_wr_haz_id_arb_q;
    wire              l2_cpu0_ls_wr_haz_vld_arb_q;
    wire              l2_cpu0_ls_wr_last_w2a;
    wire              l2_cpu0_ls_wr_req_w2a;
    wire [2:0]        l2_cpu0_ls_wr_type_w2a;
    wire [12:0]       l2_cpu0_mbist1_addr_b1;
    wire              l2_cpu0_mbist1_all_b1;
    wire [3:0]        l2_cpu0_mbist1_array_b1;
    wire [7:0]        l2_cpu0_mbist1_be_b1;
    wire              l2_cpu0_mbist1_en_b1;
    wire              l2_cpu0_mbist1_rd_en_b1;
    wire              l2_cpu0_mbist1_wr_en_b1;
    wire              l2_cpu0_no_intctrl;
    wire              l2_cpu0_pf_rd_vld_skid_popped;
    wire              l2_cpu0_pf_throttle_q;
    wire [33:0]       l2_cpu0_pmu_events;
    wire [2:0]        l2_cpu0_rbufid;
    wire              l2_cpu0_rd_aarch64_arb_set;
    wire [44:0]       l2_cpu0_rd_addr_arb_set;
    wire              l2_cpu0_rd_arb;
    wire              l2_cpu0_rd_arb_fast;
    wire [15:8]       l2_cpu0_rd_asid_arb_set;
    wire              l2_cpu0_rd_bypass_arb_set;
    wire [2:0]        l2_cpu0_rd_bypass_bufid_e5;
    wire [2:0]        l2_cpu0_rd_bypass_lrq_id_e5;
    wire              l2_cpu0_rd_bypass_req_can_e5;
    wire              l2_cpu0_rd_bypass_way_e5;
    wire [2:0]        l2_cpu0_rd_cache_attr_arb_set;
    wire [2:0]        l2_cpu0_rd_elem_size_arb_set;
    wire              l2_cpu0_rd_excl_arb_set;
    wire [4:0]        l2_cpu0_rd_id_arb_set;
    wire [2:0]        l2_cpu0_rd_lrq_id_arb_set;
    wire [7:0]        l2_cpu0_rd_page_attr_arb_set;
    wire              l2_cpu0_rd_prfm_arb_set;
    wire              l2_cpu0_rd_priv_arb_set;
    wire              l2_cpu0_rd_replayed_arb_set;
    wire [1:0]        l2_cpu0_rd_shared_arb_set;
    wire [6:0]        l2_cpu0_rd_type_arb_set;
    wire              l2_cpu0_rd_va48_arb_set;
    wire              l2_cpu0_rd_vld_skid;
    wire              l2_cpu0_rd_way_arb_set;
    wire              l2_cpu0_rexfail;
    wire [1:0]        l2_cpu0_rstate;
    wire              l2_cpu0_rvalid;
    wire [2:0]        l2_cpu0_spec_bufid;
    wire              l2_cpu0_spec_valid;
    wire [63:0]       l2_cpu0_spr_rd_data;
    wire              l2_cpu0_tbw_dbl_ecc_err;
    wire [63:0]       l2_cpu0_tbw_desc_data;
    wire              l2_cpu0_tbw_desc_vld;
    wire              l2_cpu0_tbw_ext_err;
    wire              l2_cpu0_tbw_ext_err_type;
    wire              l2_cpu0_tlb_ccb_clken_c3;
    wire              l2_cpu0_tlb_ccb_req_c3;
    wire              l2_cpu0_tlb_sync_complete;
    wire              l2_cpu0_tlb_sync_done_q;
    wire              l2_cpu0_tlb_sync_req;
    wire              l2_cpu0_trq_haz_pending;
    wire              l2_cpu0_tw_ccb_resp;
    wire [4:0]        l2_cpu0_tw_ccb_resp_id;
    wire              l2_cpu0_wr_1st_replayed_arb_set;
    wire [44:0]       l2_cpu0_wr_addr_arb_set;
    wire              l2_cpu0_wr_arb;
    wire              l2_cpu0_wr_arb_fast;
    wire [2:0]        l2_cpu0_wr_cache_attr_arb_set;
    wire [11:0]       l2_cpu0_wr_cl_id_arb_set;
    wire              l2_cpu0_wr_clean_evict_arb_set;
    wire [143:0]      l2_cpu0_wr_data;
    wire              l2_cpu0_wr_data_stall;
    wire              l2_cpu0_wr_data_vld_x1_q;
    wire              l2_cpu0_wr_dirty_arb_set;
    wire [2:0]        l2_cpu0_wr_elem_size_arb_set;
    wire              l2_cpu0_wr_err_arb_set;
    wire              l2_cpu0_wr_evict_x1_q;
    wire              l2_cpu0_wr_ex_fail;
    wire              l2_cpu0_wr_ex_resp;
    wire [3:0]        l2_cpu0_wr_id_arb_set;
    wire              l2_cpu0_wr_last_arb_set;
    wire [7:0]        l2_cpu0_wr_page_attr_arb_set;
    wire [3:0]        l2_cpu0_wr_partial_dw_arb_set;
    wire              l2_cpu0_wr_priv_arb_set;
    wire [1:0]        l2_cpu0_wr_shared_arb_set;
    wire [2:0]        l2_cpu0_wr_type_arb_set;
    wire              l2_cpu0_wr_vld_skid;
    wire              l2_cpu0_wr_way_arb_set;
    wire              l2_cpu0_wrq_almost_full;
    wire [15:0]       l2_cpu0_wrq_haz_clr_id_dcd_q;
    wire              l2_cpu0_wrq_haz_pending;
    wire              l2_cpu1_arb_thrshld_timeout_en;
    wire              l2_cpu1_barrier_done;
    wire              l2_cpu1_blk_non_evict_wr;
    wire              l2_cpu1_ccb_dbg_req_c3;
    wire [48:0]       l2_cpu1_ccb_req_addr_c3;
    wire [4:0]        l2_cpu1_ccb_req_id_c3;
    wire [23:0]       l2_cpu1_ccb_req_info_c3;
    wire [8:0]        l2_cpu1_ccb_req_type_c3;
    wire              l2_cpu1_cfg_ecc_en;
    wire [2:0]        l2_cpu1_dbufid_r1;
    wire [129:0]      l2_cpu1_ddata_r2;
    wire              l2_cpu1_ddlb_ecc_err_r3;
    wire              l2_cpu1_dext_err_r2;
    wire              l2_cpu1_dext_err_type_r2;
    wire              l2_cpu1_disable_clean_evict_opt;
    wire              l2_cpu1_dlast_r1;
    wire              l2_cpu1_dsngl_ecc_err_r3;
    wire [3:0]        l2_cpu1_dsq_clr_id_q;
    wire              l2_cpu1_dsq_clr_vld_q;
    wire [3:0]        l2_cpu1_dsq_rd_buf_id;
    wire [15:0]       l2_cpu1_dsq_rd_byte_strb_q;
    wire [129:0]      l2_cpu1_dsq_rd_data_q;
    wire              l2_cpu1_dsq_rd_en;
    wire              l2_cpu1_dsq_rd_en_x2;
    wire              l2_cpu1_dt_pmu_evt_en;
    wire              l2_cpu1_dvalid_r1;
    wire              l2_cpu1_early_rd_reqe4_e5_q;
    wire [1:0]        l2_cpu1_flsh_if_rd_id_l4_dly;
    wire              l2_cpu1_flsh_if_rd_l4_dly;
    wire              l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire [2:0]        l2_cpu1_flsh_ls_rd_id_l2_dly;
    wire [2:0]        l2_cpu1_flsh_ls_rd_id_l4_dly;
    wire              l2_cpu1_flsh_ls_rd_l2_dly;
    wire              l2_cpu1_flsh_ls_rd_l4_dly;
    wire              l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu1_flsh_ls_wr_evict_l4_dly;
    wire [3:0]        l2_cpu1_flsh_ls_wr_id_l2_dly;
    wire [3:0]        l2_cpu1_flsh_ls_wr_id_l4_dly;
    wire              l2_cpu1_flsh_ls_wr_l2_dly;
    wire              l2_cpu1_flsh_ls_wr_l4_dly;
    wire              l2_cpu1_flsh_tw_rd_l4_dly;
    wire [1:0]        l2_cpu1_ibufid_r1;
    wire [15:0]       l2_cpu1_ic_addr_arb_set;
    wire              l2_cpu1_ic_arb_fast;
    wire              l2_cpu1_ic_barrier_stall_q;
    wire [43:18]      l2_cpu1_ic_base;
    wire [31:0]       l2_cpu1_ic_data_arb_set;
    wire [2:0]        l2_cpu1_ic_elem_size_arb_set;
    wire              l2_cpu1_ic_excl_arb_set;
    wire [2:0]        l2_cpu1_ic_id_arb_set;
    wire              l2_cpu1_ic_ns_arb_set;
    wire              l2_cpu1_ic_vld_skid;
    wire              l2_cpu1_ic_write_arb_set;
    wire [127:0]      l2_cpu1_idata_r2;
    wire              l2_cpu1_idlb_ecc_err_r3;
    wire              l2_cpu1_idle_block_reqs_q;
    wire              l2_cpu1_idle_wakeup_q;
    wire              l2_cpu1_iext_err_r2;
    wire              l2_cpu1_iext_err_type_r2;
    wire              l2_cpu1_if_ccb_clken_c3;
    wire              l2_cpu1_if_ccb_req_c3;
    wire              l2_cpu1_if_ccb_resp;
    wire [4:0]        l2_cpu1_if_ccb_resp_id;
    wire              l2_cpu1_if_sync_done_q;
    wire              l2_cpu1_if_sync_req;
    wire              l2_cpu1_ifq_haz_pending;
    wire              l2_cpu1_isngl_ecc_err_r3;
    wire              l2_cpu1_ivalid_r1;
    wire [1:0]        l2_cpu1_l2_cache_size;
    wire [5:0]        l2_cpu1_lrq_haz_clr_id_dcd_q;
    wire              l2_cpu1_lrq_haz_pending;
    wire              l2_cpu1_ls_ccb_clken_c3;
    wire              l2_cpu1_ls_ccb_data_wr;
    wire              l2_cpu1_ls_ccb_req_c3;
    wire              l2_cpu1_ls_ccb_resp;
    wire [4:0]        l2_cpu1_ls_ccb_resp_id;
    wire              l2_cpu1_ls_peq_coll_l4_dly;
    wire [3:0]        l2_cpu1_ls_rd_haz_id_arb_q;
    wire              l2_cpu1_ls_rd_haz_vld_arb_q;
    wire              l2_cpu1_ls_sync_req;
    wire [4:0]        l2_cpu1_ls_wr_ccb_id_w2a;
    wire [127:0]      l2_cpu1_ls_wr_data_w2a;
    wire              l2_cpu1_ls_wr_dirty_w2a;
    wire              l2_cpu1_ls_wr_err_w2a;
    wire [2:0]        l2_cpu1_ls_wr_haz_id_arb_q;
    wire              l2_cpu1_ls_wr_haz_vld_arb_q;
    wire              l2_cpu1_ls_wr_last_w2a;
    wire              l2_cpu1_ls_wr_req_w2a;
    wire [2:0]        l2_cpu1_ls_wr_type_w2a;
    wire [12:0]       l2_cpu1_mbist1_addr_b1;
    wire              l2_cpu1_mbist1_all_b1;
    wire [3:0]        l2_cpu1_mbist1_array_b1;
    wire [7:0]        l2_cpu1_mbist1_be_b1;
    wire              l2_cpu1_mbist1_en_b1;
    wire              l2_cpu1_mbist1_rd_en_b1;
    wire              l2_cpu1_mbist1_wr_en_b1;
    wire              l2_cpu1_no_intctrl;
    wire              l2_cpu1_pf_rd_vld_skid_popped;
    wire              l2_cpu1_pf_throttle_q;
    wire [33:0]       l2_cpu1_pmu_events;
    wire [2:0]        l2_cpu1_rbufid;
    wire              l2_cpu1_rd_aarch64_arb_set;
    wire [44:0]       l2_cpu1_rd_addr_arb_set;
    wire              l2_cpu1_rd_arb;
    wire              l2_cpu1_rd_arb_fast;
    wire [15:8]       l2_cpu1_rd_asid_arb_set;
    wire              l2_cpu1_rd_bypass_arb_set;
    wire [2:0]        l2_cpu1_rd_bypass_bufid_e5;
    wire [2:0]        l2_cpu1_rd_bypass_lrq_id_e5;
    wire              l2_cpu1_rd_bypass_req_can_e5;
    wire              l2_cpu1_rd_bypass_way_e5;
    wire [2:0]        l2_cpu1_rd_cache_attr_arb_set;
    wire [2:0]        l2_cpu1_rd_elem_size_arb_set;
    wire              l2_cpu1_rd_excl_arb_set;
    wire [4:0]        l2_cpu1_rd_id_arb_set;
    wire [2:0]        l2_cpu1_rd_lrq_id_arb_set;
    wire [7:0]        l2_cpu1_rd_page_attr_arb_set;
    wire              l2_cpu1_rd_prfm_arb_set;
    wire              l2_cpu1_rd_priv_arb_set;
    wire              l2_cpu1_rd_replayed_arb_set;
    wire [1:0]        l2_cpu1_rd_shared_arb_set;
    wire [6:0]        l2_cpu1_rd_type_arb_set;
    wire              l2_cpu1_rd_va48_arb_set;
    wire              l2_cpu1_rd_vld_skid;
    wire              l2_cpu1_rd_way_arb_set;
    wire              l2_cpu1_rexfail;
    wire [1:0]        l2_cpu1_rstate;
    wire              l2_cpu1_rvalid;
    wire [2:0]        l2_cpu1_spec_bufid;
    wire              l2_cpu1_spec_valid;
    wire [63:0]       l2_cpu1_spr_rd_data;
    wire              l2_cpu1_tbw_dbl_ecc_err;
    wire [63:0]       l2_cpu1_tbw_desc_data;
    wire              l2_cpu1_tbw_desc_vld;
    wire              l2_cpu1_tbw_ext_err;
    wire              l2_cpu1_tbw_ext_err_type;
    wire              l2_cpu1_tlb_ccb_clken_c3;
    wire              l2_cpu1_tlb_ccb_req_c3;
    wire              l2_cpu1_tlb_sync_complete;
    wire              l2_cpu1_tlb_sync_done_q;
    wire              l2_cpu1_tlb_sync_req;
    wire              l2_cpu1_trq_haz_pending;
    wire              l2_cpu1_tw_ccb_resp;
    wire [4:0]        l2_cpu1_tw_ccb_resp_id;
    wire              l2_cpu1_wr_1st_replayed_arb_set;
    wire [44:0]       l2_cpu1_wr_addr_arb_set;
    wire              l2_cpu1_wr_arb;
    wire              l2_cpu1_wr_arb_fast;
    wire [2:0]        l2_cpu1_wr_cache_attr_arb_set;
    wire [11:0]       l2_cpu1_wr_cl_id_arb_set;
    wire              l2_cpu1_wr_clean_evict_arb_set;
    wire [143:0]      l2_cpu1_wr_data;
    wire              l2_cpu1_wr_data_stall;
    wire              l2_cpu1_wr_data_vld_x1_q;
    wire              l2_cpu1_wr_dirty_arb_set;
    wire [2:0]        l2_cpu1_wr_elem_size_arb_set;
    wire              l2_cpu1_wr_err_arb_set;
    wire              l2_cpu1_wr_evict_x1_q;
    wire              l2_cpu1_wr_ex_fail;
    wire              l2_cpu1_wr_ex_resp;
    wire [3:0]        l2_cpu1_wr_id_arb_set;
    wire              l2_cpu1_wr_last_arb_set;
    wire [7:0]        l2_cpu1_wr_page_attr_arb_set;
    wire [3:0]        l2_cpu1_wr_partial_dw_arb_set;
    wire              l2_cpu1_wr_priv_arb_set;
    wire [1:0]        l2_cpu1_wr_shared_arb_set;
    wire [2:0]        l2_cpu1_wr_type_arb_set;
    wire              l2_cpu1_wr_vld_skid;
    wire              l2_cpu1_wr_way_arb_set;
    wire              l2_cpu1_wrq_almost_full;
    wire [15:0]       l2_cpu1_wrq_haz_clr_id_dcd_q;
    wire              l2_cpu1_wrq_haz_pending;
    wire              l2_cpu2_arb_thrshld_timeout_en;
    wire              l2_cpu2_barrier_done;
    wire              l2_cpu2_blk_non_evict_wr;
    wire              l2_cpu2_ccb_dbg_req_c3;
    wire [48:0]       l2_cpu2_ccb_req_addr_c3;
    wire [4:0]        l2_cpu2_ccb_req_id_c3;
    wire [23:0]       l2_cpu2_ccb_req_info_c3;
    wire [8:0]        l2_cpu2_ccb_req_type_c3;
    wire              l2_cpu2_cfg_ecc_en;
    wire [2:0]        l2_cpu2_dbufid_r1;
    wire [129:0]      l2_cpu2_ddata_r2;
    wire              l2_cpu2_ddlb_ecc_err_r3;
    wire              l2_cpu2_dext_err_r2;
    wire              l2_cpu2_dext_err_type_r2;
    wire              l2_cpu2_disable_clean_evict_opt;
    wire              l2_cpu2_dlast_r1;
    wire              l2_cpu2_dsngl_ecc_err_r3;
    wire [3:0]        l2_cpu2_dsq_clr_id_q;
    wire              l2_cpu2_dsq_clr_vld_q;
    wire [3:0]        l2_cpu2_dsq_rd_buf_id;
    wire [15:0]       l2_cpu2_dsq_rd_byte_strb_q;
    wire [129:0]      l2_cpu2_dsq_rd_data_q;
    wire              l2_cpu2_dsq_rd_en;
    wire              l2_cpu2_dsq_rd_en_x2;
    wire              l2_cpu2_dt_pmu_evt_en;
    wire              l2_cpu2_dvalid_r1;
    wire              l2_cpu2_early_rd_reqe4_e5_q;
    wire [1:0]        l2_cpu2_flsh_if_rd_id_l4_dly;
    wire              l2_cpu2_flsh_if_rd_l4_dly;
    wire              l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire [2:0]        l2_cpu2_flsh_ls_rd_id_l2_dly;
    wire [2:0]        l2_cpu2_flsh_ls_rd_id_l4_dly;
    wire              l2_cpu2_flsh_ls_rd_l2_dly;
    wire              l2_cpu2_flsh_ls_rd_l4_dly;
    wire              l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu2_flsh_ls_wr_evict_l4_dly;
    wire [3:0]        l2_cpu2_flsh_ls_wr_id_l2_dly;
    wire [3:0]        l2_cpu2_flsh_ls_wr_id_l4_dly;
    wire              l2_cpu2_flsh_ls_wr_l2_dly;
    wire              l2_cpu2_flsh_ls_wr_l4_dly;
    wire              l2_cpu2_flsh_tw_rd_l4_dly;
    wire [1:0]        l2_cpu2_ibufid_r1;
    wire [15:0]       l2_cpu2_ic_addr_arb_set;
    wire              l2_cpu2_ic_arb_fast;
    wire              l2_cpu2_ic_barrier_stall_q;
    wire [43:18]      l2_cpu2_ic_base;
    wire [31:0]       l2_cpu2_ic_data_arb_set;
    wire [2:0]        l2_cpu2_ic_elem_size_arb_set;
    wire              l2_cpu2_ic_excl_arb_set;
    wire [2:0]        l2_cpu2_ic_id_arb_set;
    wire              l2_cpu2_ic_ns_arb_set;
    wire              l2_cpu2_ic_vld_skid;
    wire              l2_cpu2_ic_write_arb_set;
    wire [127:0]      l2_cpu2_idata_r2;
    wire              l2_cpu2_idlb_ecc_err_r3;
    wire              l2_cpu2_idle_block_reqs_q;
    wire              l2_cpu2_idle_wakeup_q;
    wire              l2_cpu2_iext_err_r2;
    wire              l2_cpu2_iext_err_type_r2;
    wire              l2_cpu2_if_ccb_clken_c3;
    wire              l2_cpu2_if_ccb_req_c3;
    wire              l2_cpu2_if_ccb_resp;
    wire [4:0]        l2_cpu2_if_ccb_resp_id;
    wire              l2_cpu2_if_sync_done_q;
    wire              l2_cpu2_if_sync_req;
    wire              l2_cpu2_ifq_haz_pending;
    wire              l2_cpu2_isngl_ecc_err_r3;
    wire              l2_cpu2_ivalid_r1;
    wire [1:0]        l2_cpu2_l2_cache_size;
    wire [5:0]        l2_cpu2_lrq_haz_clr_id_dcd_q;
    wire              l2_cpu2_lrq_haz_pending;
    wire              l2_cpu2_ls_ccb_clken_c3;
    wire              l2_cpu2_ls_ccb_data_wr;
    wire              l2_cpu2_ls_ccb_req_c3;
    wire              l2_cpu2_ls_ccb_resp;
    wire [4:0]        l2_cpu2_ls_ccb_resp_id;
    wire              l2_cpu2_ls_peq_coll_l4_dly;
    wire [3:0]        l2_cpu2_ls_rd_haz_id_arb_q;
    wire              l2_cpu2_ls_rd_haz_vld_arb_q;
    wire              l2_cpu2_ls_sync_req;
    wire [4:0]        l2_cpu2_ls_wr_ccb_id_w2a;
    wire [127:0]      l2_cpu2_ls_wr_data_w2a;
    wire              l2_cpu2_ls_wr_dirty_w2a;
    wire              l2_cpu2_ls_wr_err_w2a;
    wire [2:0]        l2_cpu2_ls_wr_haz_id_arb_q;
    wire              l2_cpu2_ls_wr_haz_vld_arb_q;
    wire              l2_cpu2_ls_wr_last_w2a;
    wire              l2_cpu2_ls_wr_req_w2a;
    wire [2:0]        l2_cpu2_ls_wr_type_w2a;
    wire [12:0]       l2_cpu2_mbist1_addr_b1;
    wire              l2_cpu2_mbist1_all_b1;
    wire [3:0]        l2_cpu2_mbist1_array_b1;
    wire [7:0]        l2_cpu2_mbist1_be_b1;
    wire              l2_cpu2_mbist1_en_b1;
    wire              l2_cpu2_mbist1_rd_en_b1;
    wire              l2_cpu2_mbist1_wr_en_b1;
    wire              l2_cpu2_no_intctrl;
    wire              l2_cpu2_pf_rd_vld_skid_popped;
    wire              l2_cpu2_pf_throttle_q;
    wire [33:0]       l2_cpu2_pmu_events;
    wire [2:0]        l2_cpu2_rbufid;
    wire              l2_cpu2_rd_aarch64_arb_set;
    wire [44:0]       l2_cpu2_rd_addr_arb_set;
    wire              l2_cpu2_rd_arb;
    wire              l2_cpu2_rd_arb_fast;
    wire [15:8]       l2_cpu2_rd_asid_arb_set;
    wire              l2_cpu2_rd_bypass_arb_set;
    wire [2:0]        l2_cpu2_rd_bypass_bufid_e5;
    wire [2:0]        l2_cpu2_rd_bypass_lrq_id_e5;
    wire              l2_cpu2_rd_bypass_req_can_e5;
    wire              l2_cpu2_rd_bypass_way_e5;
    wire [2:0]        l2_cpu2_rd_cache_attr_arb_set;
    wire [2:0]        l2_cpu2_rd_elem_size_arb_set;
    wire              l2_cpu2_rd_excl_arb_set;
    wire [4:0]        l2_cpu2_rd_id_arb_set;
    wire [2:0]        l2_cpu2_rd_lrq_id_arb_set;
    wire [7:0]        l2_cpu2_rd_page_attr_arb_set;
    wire              l2_cpu2_rd_prfm_arb_set;
    wire              l2_cpu2_rd_priv_arb_set;
    wire              l2_cpu2_rd_replayed_arb_set;
    wire [1:0]        l2_cpu2_rd_shared_arb_set;
    wire [6:0]        l2_cpu2_rd_type_arb_set;
    wire              l2_cpu2_rd_va48_arb_set;
    wire              l2_cpu2_rd_vld_skid;
    wire              l2_cpu2_rd_way_arb_set;
    wire              l2_cpu2_rexfail;
    wire [1:0]        l2_cpu2_rstate;
    wire              l2_cpu2_rvalid;
    wire [2:0]        l2_cpu2_spec_bufid;
    wire              l2_cpu2_spec_valid;
    wire [63:0]       l2_cpu2_spr_rd_data;
    wire              l2_cpu2_tbw_dbl_ecc_err;
    wire [63:0]       l2_cpu2_tbw_desc_data;
    wire              l2_cpu2_tbw_desc_vld;
    wire              l2_cpu2_tbw_ext_err;
    wire              l2_cpu2_tbw_ext_err_type;
    wire              l2_cpu2_tlb_ccb_clken_c3;
    wire              l2_cpu2_tlb_ccb_req_c3;
    wire              l2_cpu2_tlb_sync_complete;
    wire              l2_cpu2_tlb_sync_done_q;
    wire              l2_cpu2_tlb_sync_req;
    wire              l2_cpu2_trq_haz_pending;
    wire              l2_cpu2_tw_ccb_resp;
    wire [4:0]        l2_cpu2_tw_ccb_resp_id;
    wire              l2_cpu2_wr_1st_replayed_arb_set;
    wire [44:0]       l2_cpu2_wr_addr_arb_set;
    wire              l2_cpu2_wr_arb;
    wire              l2_cpu2_wr_arb_fast;
    wire [2:0]        l2_cpu2_wr_cache_attr_arb_set;
    wire [11:0]       l2_cpu2_wr_cl_id_arb_set;
    wire              l2_cpu2_wr_clean_evict_arb_set;
    wire [143:0]      l2_cpu2_wr_data;
    wire              l2_cpu2_wr_data_stall;
    wire              l2_cpu2_wr_data_vld_x1_q;
    wire              l2_cpu2_wr_dirty_arb_set;
    wire [2:0]        l2_cpu2_wr_elem_size_arb_set;
    wire              l2_cpu2_wr_err_arb_set;
    wire              l2_cpu2_wr_evict_x1_q;
    wire              l2_cpu2_wr_ex_fail;
    wire              l2_cpu2_wr_ex_resp;
    wire [3:0]        l2_cpu2_wr_id_arb_set;
    wire              l2_cpu2_wr_last_arb_set;
    wire [7:0]        l2_cpu2_wr_page_attr_arb_set;
    wire [3:0]        l2_cpu2_wr_partial_dw_arb_set;
    wire              l2_cpu2_wr_priv_arb_set;
    wire [1:0]        l2_cpu2_wr_shared_arb_set;
    wire [2:0]        l2_cpu2_wr_type_arb_set;
    wire              l2_cpu2_wr_vld_skid;
    wire              l2_cpu2_wr_way_arb_set;
    wire              l2_cpu2_wrq_almost_full;
    wire [15:0]       l2_cpu2_wrq_haz_clr_id_dcd_q;
    wire              l2_cpu2_wrq_haz_pending;
    wire              l2_cpu3_arb_thrshld_timeout_en;
    wire              l2_cpu3_barrier_done;
    wire              l2_cpu3_blk_non_evict_wr;
    wire              l2_cpu3_ccb_dbg_req_c3;
    wire [48:0]       l2_cpu3_ccb_req_addr_c3;
    wire [4:0]        l2_cpu3_ccb_req_id_c3;
    wire [23:0]       l2_cpu3_ccb_req_info_c3;
    wire [8:0]        l2_cpu3_ccb_req_type_c3;
    wire              l2_cpu3_cfg_ecc_en;
    wire [2:0]        l2_cpu3_dbufid_r1;
    wire [129:0]      l2_cpu3_ddata_r2;
    wire              l2_cpu3_ddlb_ecc_err_r3;
    wire              l2_cpu3_dext_err_r2;
    wire              l2_cpu3_dext_err_type_r2;
    wire              l2_cpu3_disable_clean_evict_opt;
    wire              l2_cpu3_dlast_r1;
    wire              l2_cpu3_dsngl_ecc_err_r3;
    wire [3:0]        l2_cpu3_dsq_clr_id_q;
    wire              l2_cpu3_dsq_clr_vld_q;
    wire [3:0]        l2_cpu3_dsq_rd_buf_id;
    wire [15:0]       l2_cpu3_dsq_rd_byte_strb_q;
    wire [129:0]      l2_cpu3_dsq_rd_data_q;
    wire              l2_cpu3_dsq_rd_en;
    wire              l2_cpu3_dsq_rd_en_x2;
    wire              l2_cpu3_dt_pmu_evt_en;
    wire              l2_cpu3_dvalid_r1;
    wire              l2_cpu3_early_rd_reqe4_e5_q;
    wire [1:0]        l2_cpu3_flsh_if_rd_id_l4_dly;
    wire              l2_cpu3_flsh_if_rd_l4_dly;
    wire              l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly;
    wire [2:0]        l2_cpu3_flsh_ls_rd_id_l2_dly;
    wire [2:0]        l2_cpu3_flsh_ls_rd_id_l4_dly;
    wire              l2_cpu3_flsh_ls_rd_l2_dly;
    wire              l2_cpu3_flsh_ls_rd_l4_dly;
    wire              l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly;
    wire              l2_cpu3_flsh_ls_wr_evict_l4_dly;
    wire [3:0]        l2_cpu3_flsh_ls_wr_id_l2_dly;
    wire [3:0]        l2_cpu3_flsh_ls_wr_id_l4_dly;
    wire              l2_cpu3_flsh_ls_wr_l2_dly;
    wire              l2_cpu3_flsh_ls_wr_l4_dly;
    wire              l2_cpu3_flsh_tw_rd_l4_dly;
    wire [1:0]        l2_cpu3_ibufid_r1;
    wire [15:0]       l2_cpu3_ic_addr_arb_set;
    wire              l2_cpu3_ic_arb_fast;
    wire              l2_cpu3_ic_barrier_stall_q;
    wire [43:18]      l2_cpu3_ic_base;
    wire [31:0]       l2_cpu3_ic_data_arb_set;
    wire [2:0]        l2_cpu3_ic_elem_size_arb_set;
    wire              l2_cpu3_ic_excl_arb_set;
    wire [2:0]        l2_cpu3_ic_id_arb_set;
    wire              l2_cpu3_ic_ns_arb_set;
    wire              l2_cpu3_ic_vld_skid;
    wire              l2_cpu3_ic_write_arb_set;
    wire [127:0]      l2_cpu3_idata_r2;
    wire              l2_cpu3_idlb_ecc_err_r3;
    wire              l2_cpu3_idle_block_reqs_q;
    wire              l2_cpu3_idle_wakeup_q;
    wire              l2_cpu3_iext_err_r2;
    wire              l2_cpu3_iext_err_type_r2;
    wire              l2_cpu3_if_ccb_clken_c3;
    wire              l2_cpu3_if_ccb_req_c3;
    wire              l2_cpu3_if_ccb_resp;
    wire [4:0]        l2_cpu3_if_ccb_resp_id;
    wire              l2_cpu3_if_sync_done_q;
    wire              l2_cpu3_if_sync_req;
    wire              l2_cpu3_ifq_haz_pending;
    wire              l2_cpu3_isngl_ecc_err_r3;
    wire              l2_cpu3_ivalid_r1;
    wire [1:0]        l2_cpu3_l2_cache_size;
    wire [5:0]        l2_cpu3_lrq_haz_clr_id_dcd_q;
    wire              l2_cpu3_lrq_haz_pending;
    wire              l2_cpu3_ls_ccb_clken_c3;
    wire              l2_cpu3_ls_ccb_data_wr;
    wire              l2_cpu3_ls_ccb_req_c3;
    wire              l2_cpu3_ls_ccb_resp;
    wire [4:0]        l2_cpu3_ls_ccb_resp_id;
    wire              l2_cpu3_ls_peq_coll_l4_dly;
    wire [3:0]        l2_cpu3_ls_rd_haz_id_arb_q;
    wire              l2_cpu3_ls_rd_haz_vld_arb_q;
    wire              l2_cpu3_ls_sync_req;
    wire [4:0]        l2_cpu3_ls_wr_ccb_id_w2a;
    wire [127:0]      l2_cpu3_ls_wr_data_w2a;
    wire              l2_cpu3_ls_wr_dirty_w2a;
    wire              l2_cpu3_ls_wr_err_w2a;
    wire [2:0]        l2_cpu3_ls_wr_haz_id_arb_q;
    wire              l2_cpu3_ls_wr_haz_vld_arb_q;
    wire              l2_cpu3_ls_wr_last_w2a;
    wire              l2_cpu3_ls_wr_req_w2a;
    wire [2:0]        l2_cpu3_ls_wr_type_w2a;
    wire [12:0]       l2_cpu3_mbist1_addr_b1;
    wire              l2_cpu3_mbist1_all_b1;
    wire [3:0]        l2_cpu3_mbist1_array_b1;
    wire [7:0]        l2_cpu3_mbist1_be_b1;
    wire              l2_cpu3_mbist1_en_b1;
    wire              l2_cpu3_mbist1_rd_en_b1;
    wire              l2_cpu3_mbist1_wr_en_b1;
    wire              l2_cpu3_no_intctrl;
    wire              l2_cpu3_pf_rd_vld_skid_popped;
    wire              l2_cpu3_pf_throttle_q;
    wire [33:0]       l2_cpu3_pmu_events;
    wire [2:0]        l2_cpu3_rbufid;
    wire              l2_cpu3_rd_aarch64_arb_set;
    wire [44:0]       l2_cpu3_rd_addr_arb_set;
    wire              l2_cpu3_rd_arb;
    wire              l2_cpu3_rd_arb_fast;
    wire [15:8]       l2_cpu3_rd_asid_arb_set;
    wire              l2_cpu3_rd_bypass_arb_set;
    wire [2:0]        l2_cpu3_rd_bypass_bufid_e5;
    wire [2:0]        l2_cpu3_rd_bypass_lrq_id_e5;
    wire              l2_cpu3_rd_bypass_req_can_e5;
    wire              l2_cpu3_rd_bypass_way_e5;
    wire [2:0]        l2_cpu3_rd_cache_attr_arb_set;
    wire [2:0]        l2_cpu3_rd_elem_size_arb_set;
    wire              l2_cpu3_rd_excl_arb_set;
    wire [4:0]        l2_cpu3_rd_id_arb_set;
    wire [2:0]        l2_cpu3_rd_lrq_id_arb_set;
    wire [7:0]        l2_cpu3_rd_page_attr_arb_set;
    wire              l2_cpu3_rd_prfm_arb_set;
    wire              l2_cpu3_rd_priv_arb_set;
    wire              l2_cpu3_rd_replayed_arb_set;
    wire [1:0]        l2_cpu3_rd_shared_arb_set;
    wire [6:0]        l2_cpu3_rd_type_arb_set;
    wire              l2_cpu3_rd_va48_arb_set;
    wire              l2_cpu3_rd_vld_skid;
    wire              l2_cpu3_rd_way_arb_set;
    wire              l2_cpu3_rexfail;
    wire [1:0]        l2_cpu3_rstate;
    wire              l2_cpu3_rvalid;
    wire [2:0]        l2_cpu3_spec_bufid;
    wire              l2_cpu3_spec_valid;
    wire [63:0]       l2_cpu3_spr_rd_data;
    wire              l2_cpu3_tbw_dbl_ecc_err;
    wire [63:0]       l2_cpu3_tbw_desc_data;
    wire              l2_cpu3_tbw_desc_vld;
    wire              l2_cpu3_tbw_ext_err;
    wire              l2_cpu3_tbw_ext_err_type;
    wire              l2_cpu3_tlb_ccb_clken_c3;
    wire              l2_cpu3_tlb_ccb_req_c3;
    wire              l2_cpu3_tlb_sync_complete;
    wire              l2_cpu3_tlb_sync_done_q;
    wire              l2_cpu3_tlb_sync_req;
    wire              l2_cpu3_trq_haz_pending;
    wire              l2_cpu3_tw_ccb_resp;
    wire [4:0]        l2_cpu3_tw_ccb_resp_id;
    wire              l2_cpu3_wr_1st_replayed_arb_set;
    wire [44:0]       l2_cpu3_wr_addr_arb_set;
    wire              l2_cpu3_wr_arb;
    wire              l2_cpu3_wr_arb_fast;
    wire [2:0]        l2_cpu3_wr_cache_attr_arb_set;
    wire [11:0]       l2_cpu3_wr_cl_id_arb_set;
    wire              l2_cpu3_wr_clean_evict_arb_set;
    wire [143:0]      l2_cpu3_wr_data;
    wire              l2_cpu3_wr_data_stall;
    wire              l2_cpu3_wr_data_vld_x1_q;
    wire              l2_cpu3_wr_dirty_arb_set;
    wire [2:0]        l2_cpu3_wr_elem_size_arb_set;
    wire              l2_cpu3_wr_err_arb_set;
    wire              l2_cpu3_wr_evict_x1_q;
    wire              l2_cpu3_wr_ex_fail;
    wire              l2_cpu3_wr_ex_resp;
    wire [3:0]        l2_cpu3_wr_id_arb_set;
    wire              l2_cpu3_wr_last_arb_set;
    wire [7:0]        l2_cpu3_wr_page_attr_arb_set;
    wire [3:0]        l2_cpu3_wr_partial_dw_arb_set;
    wire              l2_cpu3_wr_priv_arb_set;
    wire [1:0]        l2_cpu3_wr_shared_arb_set;
    wire [2:0]        l2_cpu3_wr_type_arb_set;
    wire              l2_cpu3_wr_vld_skid;
    wire              l2_cpu3_wr_way_arb_set;
    wire              l2_cpu3_wrq_almost_full;
    wire [15:0]       l2_cpu3_wrq_haz_clr_id_dcd_q;
    wire              l2_cpu3_wrq_haz_pending;
    wire [2:0]        l2_tbnk0_cpu0_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk0_cpu0_lrq_clr_l4_dly2_q;
    wire              l2_tbnk0_cpu0_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk0_cpu0_wrq_clr_l4_dly2_q;
    wire [2:0]        l2_tbnk0_cpu1_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk0_cpu1_lrq_clr_l4_dly2_q;
    wire              l2_tbnk0_cpu1_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk0_cpu1_wrq_clr_l4_dly2_q;
    wire [2:0]        l2_tbnk0_cpu2_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk0_cpu2_lrq_clr_l4_dly2_q;
    wire              l2_tbnk0_cpu2_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk0_cpu2_wrq_clr_l4_dly2_q;
    wire [2:0]        l2_tbnk0_cpu3_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk0_cpu3_lrq_clr_l4_dly2_q;
    wire              l2_tbnk0_cpu3_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk0_cpu3_wrq_clr_l4_dly2_q;
    wire [2:0]        l2_tbnk1_cpu0_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk1_cpu0_lrq_clr_l4_dly2_q;
    wire              l2_tbnk1_cpu0_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk1_cpu0_wrq_clr_l4_dly2_q;
    wire [2:0]        l2_tbnk1_cpu1_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk1_cpu1_lrq_clr_l4_dly2_q;
    wire              l2_tbnk1_cpu1_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk1_cpu1_wrq_clr_l4_dly2_q;
    wire [2:0]        l2_tbnk1_cpu2_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk1_cpu2_lrq_clr_l4_dly2_q;
    wire              l2_tbnk1_cpu2_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk1_cpu2_wrq_clr_l4_dly2_q;
    wire [2:0]        l2_tbnk1_cpu3_ifq_clr_l4_dly2_q;
    wire [3:0]        l2_tbnk1_cpu3_lrq_clr_l4_dly2_q;
    wire              l2_tbnk1_cpu3_trq_clr_l4_dly2_q;
    wire [5:0]        l2_tbnk1_cpu3_wrq_clr_l4_dly2_q;
    wire              ls_cpu0_clrexmon;
    wire              ls_cpu0_imp_abort_containable;
    wire              ls_cpu0_imp_abort_dec;
    wire              ls_cpu0_imp_abort_ecc;
    wire              ls_cpu0_imp_abort_slv;
    wire              ls_cpu0_raw_eae_nonsec;
    wire              ls_cpu0_raw_eae_secure;
    wire              ls_cpu1_clrexmon;
    wire              ls_cpu1_imp_abort_containable;
    wire              ls_cpu1_imp_abort_dec;
    wire              ls_cpu1_imp_abort_ecc;
    wire              ls_cpu1_imp_abort_slv;
    wire              ls_cpu1_raw_eae_nonsec;
    wire              ls_cpu1_raw_eae_secure;
    wire              ls_cpu2_clrexmon;
    wire              ls_cpu2_imp_abort_containable;
    wire              ls_cpu2_imp_abort_dec;
    wire              ls_cpu2_imp_abort_ecc;
    wire              ls_cpu2_imp_abort_slv;
    wire              ls_cpu2_raw_eae_nonsec;
    wire              ls_cpu2_raw_eae_secure;
    wire              ls_cpu3_clrexmon;
    wire              ls_cpu3_imp_abort_containable;
    wire              ls_cpu3_imp_abort_dec;
    wire              ls_cpu3_imp_abort_ecc;
    wire              ls_cpu3_imp_abort_slv;
    wire              ls_cpu3_raw_eae_nonsec;
    wire              ls_cpu3_raw_eae_secure;
    wire              ncommirq_cpu0_i;
    wire              ncommirq_cpu1_i;
    wire              ncommirq_cpu2_i;
    wire              ncommirq_cpu3_i;
    wire              ncorereset_cpu0_o;
    wire              ncorereset_cpu1_o;
    wire              ncorereset_cpu2_o;
    wire              ncorereset_cpu3_o;
    wire              ncpuporeset_cpu0_o;
    wire              ncpuporeset_cpu1_o;
    wire              ncpuporeset_cpu2_o;
    wire              ncpuporeset_cpu3_o;
    wire              niden_cpu0_o;
    wire              niden_cpu1_o;
    wire              niden_cpu2_o;
    wire              niden_cpu3_o;
    wire              nmbistreset_cpu0_o;
    wire              nmbistreset_cpu1_o;
    wire              nmbistreset_cpu2_o;
    wire              nmbistreset_cpu3_o;
    wire              npmuirq_cpu0_i;
    wire              npmuirq_cpu1_i;
    wire              npmuirq_cpu2_i;
    wire              npmuirq_cpu3_i;
    wire              pm_export_cpu0_i;
    wire              pm_export_cpu1_i;
    wire              pm_export_cpu2_i;
    wire              pm_export_cpu3_i;
    wire [24:0]       pmuevent_cpu0_i;
    wire [24:0]       pmuevent_cpu1_i;
    wire [24:0]       pmuevent_cpu2_i;
    wire [24:0]       pmuevent_cpu3_i;
    wire [43:2]       rvbaraddr_cpu0_o;
    wire [43:2]       rvbaraddr_cpu1_o;
    wire [43:2]       rvbaraddr_cpu2_o;
    wire [43:2]       rvbaraddr_cpu3_o;
    wire              spiden_cpu0_o;
    wire              spiden_cpu1_o;
    wire              spiden_cpu2_o;
    wire              spiden_cpu3_o;
    wire              spniden_cpu0_o;
    wire              spniden_cpu1_o;
    wire              spniden_cpu2_o;
    wire              spniden_cpu3_o;
    wire              syncreqm_cpu0_o;
    wire              syncreqm_cpu1_o;
    wire              syncreqm_cpu2_o;
    wire              syncreqm_cpu3_o;
    wire [1:0]        tm_cpu0_cnthctl_kernel;
    wire [3:0]        tm_cpu0_cntkctl_usr;
    wire [1:0]        tm_cpu1_cnthctl_kernel;
    wire [3:0]        tm_cpu1_cntkctl_usr;
    wire [1:0]        tm_cpu2_cnthctl_kernel;
    wire [3:0]        tm_cpu2_cntkctl_usr;
    wire [1:0]        tm_cpu3_cnthctl_kernel;
    wire [3:0]        tm_cpu3_cntkctl_usr;
    wire [63:0]       tsvalueb_cpu0_o;
    wire [63:0]       tsvalueb_cpu1_o;
    wire [63:0]       tsvalueb_cpu2_o;
    wire [63:0]       tsvalueb_cpu3_o;
    wire              vinithi_cpu0_o;
    wire              vinithi_cpu1_o;
    wire              vinithi_cpu2_o;
    wire              vinithi_cpu3_o;

    maia_cpu ucpu0( // outputs
                    .afreadym_cpu                            (afreadym_cpu0_i),
                    .atbytesm_cpu                            (atbytesm_cpu0_i[1:0]),
                    .atdatam_cpu                             (atdatam_cpu0_i[31:0]),
                    .atidm_cpu                               (atidm_cpu0_i[6:0]),
                    .atvalidm_cpu                            (atvalidm_cpu0_i),
                    .commrx_cpu                              (commrx_cpu0_i),
                    .commtx_cpu                              (commtx_cpu0_i),
                    .dbgack_cpu                              (dbgack_cpu0_i),
                    .dbgnopwrdwn_cpu                         (dbgnopwrdwn_cpu0_i),
                    .dbgrstreq_cpu                           (dbgrstreq_cpu0_i),
                    .ds_cpuectlr_ret                         (ds_cpu0_cpuectlr_ret[2:0]),
                    .ds_cpuectlr_smp                         (ds_cpu0_cpuectlr_smp),
                    .ds_fiq_wfe_qual                         (ds_cpu0_fiq_wfe_qual),
                    .ds_fiq_wfi_qual                         (ds_cpu0_fiq_wfi_qual),
                    .ds_flush                                (ds_cpu0_flush),
                    .ds_flush_type                           (ds_cpu0_flush_type[5:0]),
                    .ds_hcr_va                               (ds_cpu0_hcr_va),
                    .ds_hcr_vf                               (ds_cpu0_hcr_vf),
                    .ds_hcr_vi                               (ds_cpu0_hcr_vi),
                    .ds_ic_aa64naa32                         (ds_cpu0_ic_aa64naa32),
                    .ds_ic_cpsr_mode                         (ds_cpu0_ic_cpsr_mode[4:0]),
                    .ds_ic_hcr_change                        (ds_cpu0_ic_hcr_change),
                    .ds_ic_sample_spr                        (ds_cpu0_ic_sample_spr),
                    .ds_ic_scr_change                        (ds_cpu0_ic_scr_change),
                    .ds_imp_abrt_wfe_qual                    (ds_cpu0_imp_abrt_wfe_qual),
                    .ds_imp_abrt_wfi_qual                    (ds_cpu0_imp_abrt_wfi_qual),
                    .ds_irq_wfe_qual                         (ds_cpu0_irq_wfe_qual),
                    .ds_irq_wfi_qual                         (ds_cpu0_irq_wfi_qual),
                    .ds_l2_spr_addr                          (ds_cpu0_l2_spr_addr[8:0]),
                    .ds_l2_spr_dw                            (ds_cpu0_l2_spr_dw),
                    .ds_l2_spr_en                            (ds_cpu0_l2_spr_en),
                    .ds_l2_spr_rd                            (ds_cpu0_l2_spr_rd),
                    .ds_l2_spr_wr                            (ds_cpu0_l2_spr_wr),
                    .ds_l2_spr_wr_data                       (ds_cpu0_l2_spr_wr_data[63:0]),
                    .ds_reset_req                            (ds_cpu0_reset_req),
                    .ds_sev_req                              (ds_cpu0_sev_req),
                    .ds_sevl_req                             (ds_cpu0_sevl_req),
                    .ds_vfiq_wfe_qual                        (ds_cpu0_vfiq_wfe_qual),
                    .ds_vfiq_wfi_qual                        (ds_cpu0_vfiq_wfi_qual),
                    .ds_vimp_abrt_wfe_qual                   (ds_cpu0_vimp_abrt_wfe_qual),
                    .ds_vimp_abrt_wfi_qual                   (ds_cpu0_vimp_abrt_wfi_qual),
                    .ds_virq_wfe_qual                        (ds_cpu0_virq_wfe_qual),
                    .ds_virq_wfi_qual                        (ds_cpu0_virq_wfi_qual),
                    .ds_wfe_req                              (ds_cpu0_wfe_req),
                    .ds_wfi_req                              (ds_cpu0_wfi_req),
                    .dt_coredbg_in_reset_gclk                (dt_cpu0_coredbg_in_reset_gclk),
                    .dt_cti_trigin_1to0_gclk                 (dt_cpu0_cti_trigin_1to0_gclk[1:0]),
                    .dt_cti_trigin_7to4_gclk                 (dt_cpu0_cti_trigin_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_7to4_gclk             (dt_cpu0_cti_trigoutack_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_bit1_gclk             (dt_cpu0_cti_trigoutack_bit1_gclk),
                    .dt_dbif_ack_gclk                        (dt_cpu0_dbif_ack_gclk),
                    .dt_dbif_err_gclk                        (dt_cpu0_dbif_err_gclk),
                    .dt_dbif_rddata_gclk                     (dt_cpu0_dbif_rddata_gclk[31:0]),
                    .dt_et_oslock_gclk                       (dt_cpu0_et_oslock_gclk),
                    .dt_halt_ack_gclk                        (dt_cpu0_halt_ack_gclk),
                    .dt_hlt_dbgevt_ok_gclk                   (dt_cpu0_hlt_dbgevt_ok_gclk),
                    .dt_os_double_lock_gclk                  (dt_cpu0_os_double_lock_gclk),
                    .dt_pmusnapshot_ack_gclk                 (dt_cpu0_pmusnapshot_ack_gclk),
                    .dt_wfx_dbg_req_gclk                     (dt_cpu0_wfx_dbg_req_gclk),
                    .etclken_cpu                             (etclken_cpu0_i),
                    .l2_cpu_dsq_rd_byte_strb_q               (l2_cpu0_dsq_rd_byte_strb_q[15:0]),
                    .l2_cpu_dsq_rd_data_q                    (l2_cpu0_dsq_rd_data_q[129:0]),
                    .l2_cpu_dt_pmu_evt_en                    (l2_cpu0_dt_pmu_evt_en),
                    .l2_cpu_early_rd_reqe4_e5_q              (l2_cpu0_early_rd_reqe4_e5_q),
                    .l2_cpu_ic_addr_arb_set                  (l2_cpu0_ic_addr_arb_set[15:0]),
                    .l2_cpu_ic_arb_fast                      (l2_cpu0_ic_arb_fast),
                    .l2_cpu_ic_data_arb_set                  (l2_cpu0_ic_data_arb_set[31:0]),
                    .l2_cpu_ic_elem_size_arb_set             (l2_cpu0_ic_elem_size_arb_set[2:0]),
                    .l2_cpu_ic_excl_arb_set                  (l2_cpu0_ic_excl_arb_set),
                    .l2_cpu_ic_id_arb_set                    (l2_cpu0_ic_id_arb_set[2:0]),
                    .l2_cpu_ic_ns_arb_set                    (l2_cpu0_ic_ns_arb_set),
                    .l2_cpu_ic_write_arb_set                 (l2_cpu0_ic_write_arb_set),
                    .l2_cpu_idle_wakeup_q                    (l2_cpu0_idle_wakeup_q),
                    .l2_cpu_if_ccb_resp                      (l2_cpu0_if_ccb_resp),
                    .l2_cpu_if_ccb_resp_id                   (l2_cpu0_if_ccb_resp_id[4:0]),
                    .l2_cpu_if_sync_done_q                   (l2_cpu0_if_sync_done_q),
                    .l2_cpu_lrq_haz_clr_id_dcd_q             (l2_cpu0_lrq_haz_clr_id_dcd_q[5:0]),
                    .l2_cpu_ls_ccb_data_wr                   (l2_cpu0_ls_ccb_data_wr),
                    .l2_cpu_ls_ccb_resp                      (l2_cpu0_ls_ccb_resp),
                    .l2_cpu_ls_ccb_resp_id                   (l2_cpu0_ls_ccb_resp_id[4:0]),
                    .l2_cpu_ls_rd_haz_id_arb_q               (l2_cpu0_ls_rd_haz_id_arb_q[3:0]),
                    .l2_cpu_ls_rd_haz_vld_arb_q              (l2_cpu0_ls_rd_haz_vld_arb_q),
                    .l2_cpu_ls_wr_ccb_id_w2a                 (l2_cpu0_ls_wr_ccb_id_w2a[4:0]),
                    .l2_cpu_ls_wr_data_w2a                   (l2_cpu0_ls_wr_data_w2a[127:0]),
                    .l2_cpu_ls_wr_dirty_w2a                  (l2_cpu0_ls_wr_dirty_w2a),
                    .l2_cpu_ls_wr_err_w2a                    (l2_cpu0_ls_wr_err_w2a),
                    .l2_cpu_ls_wr_haz_id_arb_q               (l2_cpu0_ls_wr_haz_id_arb_q[2:0]),
                    .l2_cpu_ls_wr_haz_vld_arb_q              (l2_cpu0_ls_wr_haz_vld_arb_q),
                    .l2_cpu_ls_wr_last_w2a                   (l2_cpu0_ls_wr_last_w2a),
                    .l2_cpu_ls_wr_req_w2a                    (l2_cpu0_ls_wr_req_w2a),
                    .l2_cpu_ls_wr_type_w2a                   (l2_cpu0_ls_wr_type_w2a[2:0]),
                    .l2_cpu_rd_aarch64_arb_set               (l2_cpu0_rd_aarch64_arb_set),
                    .l2_cpu_rd_addr_arb_set                  (l2_cpu0_rd_addr_arb_set[44:0]),
                    .l2_cpu_rd_arb_fast                      (l2_cpu0_rd_arb_fast),
                    .l2_cpu_rd_asid_arb_set                  (l2_cpu0_rd_asid_arb_set[15:8]),
                    .l2_cpu_rd_bypass_arb_set                (l2_cpu0_rd_bypass_arb_set),
                    .l2_cpu_rd_bypass_bufid_e5               (l2_cpu0_rd_bypass_bufid_e5[2:0]),
                    .l2_cpu_rd_bypass_lrq_id_e5              (l2_cpu0_rd_bypass_lrq_id_e5[2:0]),
                    .l2_cpu_rd_bypass_req_can_e5             (l2_cpu0_rd_bypass_req_can_e5),
                    .l2_cpu_rd_bypass_way_e5                 (l2_cpu0_rd_bypass_way_e5),
                    .l2_cpu_rd_cache_attr_arb_set            (l2_cpu0_rd_cache_attr_arb_set[2:0]),
                    .l2_cpu_rd_elem_size_arb_set             (l2_cpu0_rd_elem_size_arb_set[2:0]),
                    .l2_cpu_rd_excl_arb_set                  (l2_cpu0_rd_excl_arb_set),
                    .l2_cpu_rd_id_arb_set                    (l2_cpu0_rd_id_arb_set[4:0]),
                    .l2_cpu_rd_lrq_id_arb_set                (l2_cpu0_rd_lrq_id_arb_set[2:0]),
                    .l2_cpu_rd_page_attr_arb_set             (l2_cpu0_rd_page_attr_arb_set[7:0]),
                    .l2_cpu_rd_prfm_arb_set                  (l2_cpu0_rd_prfm_arb_set),
                    .l2_cpu_rd_priv_arb_set                  (l2_cpu0_rd_priv_arb_set),
                    .l2_cpu_rd_replayed_arb_set              (l2_cpu0_rd_replayed_arb_set),
                    .l2_cpu_rd_shared_arb_set                (l2_cpu0_rd_shared_arb_set[1:0]),
                    .l2_cpu_rd_type_arb_set                  (l2_cpu0_rd_type_arb_set[6:0]),
                    .l2_cpu_rd_va48_arb_set                  (l2_cpu0_rd_va48_arb_set),
                    .l2_cpu_rd_way_arb_set                   (l2_cpu0_rd_way_arb_set),
                    .l2_cpu_tlb_sync_done_q                  (l2_cpu0_tlb_sync_done_q),
                    .l2_cpu_tw_ccb_resp                      (l2_cpu0_tw_ccb_resp),
                    .l2_cpu_tw_ccb_resp_id                   (l2_cpu0_tw_ccb_resp_id[4:0]),
                    .l2_cpu_wr_1st_replayed_arb_set          (l2_cpu0_wr_1st_replayed_arb_set),
                    .l2_cpu_wr_addr_arb_set                  (l2_cpu0_wr_addr_arb_set[44:0]),
                    .l2_cpu_wr_arb_fast                      (l2_cpu0_wr_arb_fast),
                    .l2_cpu_wr_cache_attr_arb_set            (l2_cpu0_wr_cache_attr_arb_set[2:0]),
                    .l2_cpu_wr_cl_id_arb_set                 (l2_cpu0_wr_cl_id_arb_set[11:0]),
                    .l2_cpu_wr_clean_evict_arb_set           (l2_cpu0_wr_clean_evict_arb_set),
                    .l2_cpu_wr_data                          (l2_cpu0_wr_data[143:0]),
                    .l2_cpu_wr_data_vld_x1_q                 (l2_cpu0_wr_data_vld_x1_q),
                    .l2_cpu_wr_dirty_arb_set                 (l2_cpu0_wr_dirty_arb_set),
                    .l2_cpu_wr_elem_size_arb_set             (l2_cpu0_wr_elem_size_arb_set[2:0]),
                    .l2_cpu_wr_err_arb_set                   (l2_cpu0_wr_err_arb_set),
                    .l2_cpu_wr_evict_x1_q                    (l2_cpu0_wr_evict_x1_q),
                    .l2_cpu_wr_id_arb_set                    (l2_cpu0_wr_id_arb_set[3:0]),
                    .l2_cpu_wr_last_arb_set                  (l2_cpu0_wr_last_arb_set),
                    .l2_cpu_wr_page_attr_arb_set             (l2_cpu0_wr_page_attr_arb_set[7:0]),
                    .l2_cpu_wr_partial_dw_arb_set            (l2_cpu0_wr_partial_dw_arb_set[3:0]),
                    .l2_cpu_wr_priv_arb_set                  (l2_cpu0_wr_priv_arb_set),
                    .l2_cpu_wr_shared_arb_set                (l2_cpu0_wr_shared_arb_set[1:0]),
                    .l2_cpu_wr_type_arb_set                  (l2_cpu0_wr_type_arb_set[2:0]),
                    .l2_cpu_wr_way_arb_set                   (l2_cpu0_wr_way_arb_set),
                    .l2_cpu_wrq_almost_full                  (l2_cpu0_wrq_almost_full),
                    .l2_cpu_wrq_haz_clr_id_dcd_q             (l2_cpu0_wrq_haz_clr_id_dcd_q[15:0]),
                    .ls_clrexmon                             (ls_cpu0_clrexmon),
                    .ls_imp_abort_containable                (ls_cpu0_imp_abort_containable),
                    .ls_imp_abort_dec                        (ls_cpu0_imp_abort_dec),
                    .ls_imp_abort_ecc                        (ls_cpu0_imp_abort_ecc),
                    .ls_imp_abort_slv                        (ls_cpu0_imp_abort_slv),
                    .ls_raw_eae_nonsec                       (ls_cpu0_raw_eae_nonsec),
                    .ls_raw_eae_secure                       (ls_cpu0_raw_eae_secure),
                    .ncommirq_cpu                            (ncommirq_cpu0_i),
                    .npmuirq_cpu                             (npmuirq_cpu0_i),
                    .pm_export_cpu                           (pm_export_cpu0_i),
                    .pmuevent_cpu                            (pmuevent_cpu0_i[24:0]),

                    // inputs
                    .aa64naa32_cpu                           (aa64naa32_cpu0_o),
                    .afvalidm_cpu                            (afvalidm_cpu0_o),
                    .atclken_cpu                             (atclken_cpu0_o),
                    .atreadym_cpu                            (atreadym_cpu0_o),
                    .cfgend_cpu                              (cfgend_cpu0_o),
                    .cfgte_cpu                               (cfgte_cpu0_o),
                    .ck_crcx_clk_en_n                        (ck_cpu0_crcx_clk_en_n),
                    .ck_event_reg                            (ck_cpu0_event_reg),
                    .ck_gclkt                                (ck_gclkt[0]),
                    .ck_wfe_ack                              (ck_cpu0_wfe_ack),
                    .ck_wfi_ack                              (ck_cpu0_wfi_ack),
                    .clusteridaff1_cpu                       (clusteridaff1_cpu0_o[7:0]),
                    .clusteridaff2_cpu                       (clusteridaff2_cpu0_o[7:0]),
                    .cp15sdisable_cpu                        (cp15sdisable_cpu0_o),
                    .cpuid                                   (cpuid_cpu0_o[1:0]),
                    .cryptodisable_cpu                       (cryptodisable_cpu0_o),
                    .dbgen_cpu                               (dbgen_cpu0_o),
                    .dbgl1rstdisable_cpu                     (dbgl1rstdisable_cpu0_o),
                    .dbgromaddr_cpu                          (dbgromaddr_cpu0_o[43:12]),
                    .dbgromaddrv_cpu                         (dbgromaddrv_cpu0_o),
                    .dftcrclkdisable_cpu                     (dftcrclkdisable_cpu0_o),
                    .dftramhold_cpu                          (dftramhold_cpu0_o),
                    .dftrstdisable_cpu                       (dftrstdisable_cpu0_o),
                    .dftse_cpu                               (dftse_cpu0_o),
                    .dt_cti_triginack_1to0_pclk              (dt_cpu0_cti_triginack_1to0_pclk[1:0]),
                    .dt_cti_triginack_7to4_pclk              (dt_cpu0_cti_triginack_7to4_pclk[3:0]),
                    .dt_cti_trigout_1to0_pclk                (dt_cpu0_cti_trigout_1to0_pclk[1:0]),
                    .dt_cti_trigout_7to4_pclk                (dt_cpu0_cti_trigout_7to4_pclk[3:0]),
                    .dt_dbif_addr_pclk                       (dt_cpu0_dbif_addr_pclk[14:2]),
                    .dt_dbif_locked_pclk                     (dt_cpu0_dbif_locked_pclk),
                    .dt_dbif_req_pclk                        (dt_cpu0_dbif_req_pclk),
                    .dt_dbif_wrdata_pclk                     (dt_cpu0_dbif_wrdata_pclk[31:0]),
                    .dt_dbif_write_pclk                      (dt_cpu0_dbif_write_pclk),
                    .dt_edacr_frc_idleack_pclk               (dt_cpu0_edacr_frc_idleack_pclk),
                    .dt_edbgrq_pclk                          (dt_cpu0_edbgrq_pclk),
                    .dt_edecr_osuce_pclk                     (dt_cpu0_edecr_osuce_pclk),
                    .dt_edecr_rce_pclk                       (dt_cpu0_edecr_rce_pclk),
                    .dt_edecr_ss_pclk                        (dt_cpu0_edecr_ss_pclk),
                    .dt_edprcr_corepurq_pclk                 (dt_cpu0_edprcr_corepurq_pclk),
                    .dt_noclkstop_pclk                       (dt_cpu0_noclkstop_pclk),
                    .dt_pmusnapshot_req_pclk                 (dt_cpu0_pmusnapshot_req_pclk),
                    .dt_wfx_wakeup_pclk                      (dt_cpu0_wfx_wakeup_pclk),
                    .giccdisable_cpu                         (giccdisable_cpu0_o),
                    .ic_block_eoi_sgi_wr                     (ic_block_eoi_sgi_wr[0]),
                    .ic_el_change_complete                   (ic_el_change_complete[0]),
                    .ic_hcr_change_complete                  (ic_hcr_change_complete[0]),
                    .ic_ich_el2_tall0                        (ic_ich_el2_tall0[0]),
                    .ic_ich_el2_tall1                        (ic_ich_el2_tall1[0]),
                    .ic_ich_el2_tc                           (ic_ich_el2_tc[0]),
                    .ic_nfiq                                 (ic_nfiq[0]),
                    .ic_nirq                                 (ic_nirq[0]),
                    .ic_nsei                                 (ic_nsei[0]),
                    .ic_nvfiq                                (ic_nvfiq[0]),
                    .ic_nvirq                                (ic_nvirq[0]),
                    .ic_nvsei                                (ic_nvsei[0]),
                    .ic_p_valid                              (ic_p_valid[0]),
                    .ic_sample_spr                           (ic_sample_spr[0]),
                    .ic_scr_change_complete                  (ic_scr_change_complete[0]),
                    .ic_sra_el1ns_en                         (ic_sra_el1ns_en[0]),
                    .ic_sra_el1s_en                          (ic_sra_el1s_en[0]),
                    .ic_sra_el2_en                           (ic_sra_el2_en[0]),
                    .ic_sra_el3_en                           (ic_sra_el3_en[0]),
                    .ic_sre_el1ns_hyp_trap                   (ic_sre_el1ns_hyp_trap[0]),
                    .ic_sre_el1ns_mon_trap                   (ic_sre_el1ns_mon_trap[0]),
                    .ic_sre_el1s_mon_trap                    (ic_sre_el1s_mon_trap[0]),
                    .ic_sre_el2_mon_trap                     (ic_sre_el2_mon_trap[0]),
                    .l2_cpu_arb_thrshld_timeout_en           (l2_cpu0_arb_thrshld_timeout_en),
                    .l2_cpu_barrier_done                     (l2_cpu0_barrier_done),
                    .l2_cpu_blk_non_evict_wr                 (l2_cpu0_blk_non_evict_wr),
                    .l2_cpu_ccb_dbg_req_c3                   (l2_cpu0_ccb_dbg_req_c3),
                    .l2_cpu_ccb_req_addr_c3                  (l2_cpu0_ccb_req_addr_c3[48:0]),
                    .l2_cpu_ccb_req_id_c3                    (l2_cpu0_ccb_req_id_c3[4:0]),
                    .l2_cpu_ccb_req_info_c3                  (l2_cpu0_ccb_req_info_c3[23:0]),
                    .l2_cpu_ccb_req_type_c3                  (l2_cpu0_ccb_req_type_c3[8:0]),
                    .l2_cpu_cfg_ecc_en                       (l2_cpu0_cfg_ecc_en),
                    .l2_cpu_dbufid_r1                        (l2_cpu0_dbufid_r1[2:0]),
                    .l2_cpu_ddata_r2                         (l2_cpu0_ddata_r2[129:0]),
                    .l2_cpu_ddbl_ecc_err_r3                  (l2_cpu0_ddlb_ecc_err_r3),
                    .l2_cpu_dext_err_r2                      (l2_cpu0_dext_err_r2),
                    .l2_cpu_dext_err_type_r2                 (l2_cpu0_dext_err_type_r2),
                    .l2_cpu_disable_clean_evict_opt          (l2_cpu0_disable_clean_evict_opt),
                    .l2_cpu_dlast_r1                         (l2_cpu0_dlast_r1),
                    .l2_cpu_dsngl_ecc_err_r3                 (l2_cpu0_dsngl_ecc_err_r3),
                    .l2_cpu_dsq_clr_id_q                     (l2_cpu0_dsq_clr_id_q[3:0]),
                    .l2_cpu_dsq_clr_vld_q                    (l2_cpu0_dsq_clr_vld_q),
                    .l2_cpu_dsq_rd_buf_id                    (l2_cpu0_dsq_rd_buf_id[3:0]),
                    .l2_cpu_dsq_rd_en                        (l2_cpu0_dsq_rd_en),
                    .l2_cpu_dsq_rd_en_x2                     (l2_cpu0_dsq_rd_en_x2),
                    .l2_cpu_dvalid_r1                        (l2_cpu0_dvalid_r1),
                    .l2_cpu_flsh_if_rd_id_l4_dly             (l2_cpu0_flsh_if_rd_id_l4_dly[1:0]),
                    .l2_cpu_flsh_if_rd_l4_dly                (l2_cpu0_flsh_if_rd_l4_dly),
                    .l2_cpu_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_rd_id_l2_dly             (l2_cpu0_flsh_ls_rd_id_l2_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_id_l4_dly             (l2_cpu0_flsh_ls_rd_id_l4_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_l2_dly                (l2_cpu0_flsh_ls_rd_l2_dly),
                    .l2_cpu_flsh_ls_rd_l4_dly                (l2_cpu0_flsh_ls_rd_l4_dly),
                    .l2_cpu_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_wr_evict_l4_dly          (l2_cpu0_flsh_ls_wr_evict_l4_dly),
                    .l2_cpu_flsh_ls_wr_id_l2_dly             (l2_cpu0_flsh_ls_wr_id_l2_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_id_l4_dly             (l2_cpu0_flsh_ls_wr_id_l4_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_l2_dly                (l2_cpu0_flsh_ls_wr_l2_dly),
                    .l2_cpu_flsh_ls_wr_l4_dly                (l2_cpu0_flsh_ls_wr_l4_dly),
                    .l2_cpu_flsh_tw_rd_l4_dly                (l2_cpu0_flsh_tw_rd_l4_dly),
                    .l2_cpu_ibufid_r1                        (l2_cpu0_ibufid_r1[1:0]),
                    .l2_cpu_ic_barrier_stall_q               (l2_cpu0_ic_barrier_stall_q),
                    .l2_cpu_ic_base                          (l2_cpu0_ic_base[43:18]),
                    .l2_cpu_ic_vld_skid                      (l2_cpu0_ic_vld_skid),
                    .l2_cpu_idata_r2                         (l2_cpu0_idata_r2[127:0]),
                    .l2_cpu_idbl_ecc_err_r3                  (l2_cpu0_idlb_ecc_err_r3),
                    .l2_cpu_idle_block_reqs_q                (l2_cpu0_idle_block_reqs_q),
                    .l2_cpu_iext_err_r2                      (l2_cpu0_iext_err_r2),
                    .l2_cpu_iext_err_type_r2                 (l2_cpu0_iext_err_type_r2),
                    .l2_cpu_if_ccb_clken_c3                  (l2_cpu0_if_ccb_clken_c3),
                    .l2_cpu_if_ccb_req_c3                    (l2_cpu0_if_ccb_req_c3),
                    .l2_cpu_if_sync_req                      (l2_cpu0_if_sync_req),
                    .l2_cpu_ifq_haz_pending                  (l2_cpu0_ifq_haz_pending),
                    .l2_cpu_isngl_ecc_err_r3                 (l2_cpu0_isngl_ecc_err_r3),
                    .l2_cpu_ivalid_r1                        (l2_cpu0_ivalid_r1),
                    .l2_cpu_l2_cache_size                    (l2_cpu0_l2_cache_size[1:0]),
                    .l2_cpu_lrq_haz_pending                  (l2_cpu0_lrq_haz_pending),
                    .l2_cpu_ls_ccb_clken_c3                  (l2_cpu0_ls_ccb_clken_c3),
                    .l2_cpu_ls_ccb_req_c3                    (l2_cpu0_ls_ccb_req_c3),
                    .l2_cpu_ls_peq_coll_l4_dly               (l2_cpu0_ls_peq_coll_l4_dly),
                    .l2_cpu_ls_sync_req                      (l2_cpu0_ls_sync_req),
                    .l2_cpu_mbist1_addr_b1                   (l2_cpu0_mbist1_addr_b1[12:0]),
                    .l2_cpu_mbist1_all_b1                    (l2_cpu0_mbist1_all_b1),
                    .l2_cpu_mbist1_array_b1                  (l2_cpu0_mbist1_array_b1[3:0]),
                    .l2_cpu_mbist1_be_b1                     (l2_cpu0_mbist1_be_b1[7:0]),
                    .l2_cpu_mbist1_en_b1                     (l2_cpu0_mbist1_en_b1),
                    .l2_cpu_mbist1_rd_en_b1                  (l2_cpu0_mbist1_rd_en_b1),
                    .l2_cpu_mbist1_wr_en_b1                  (l2_cpu0_mbist1_wr_en_b1),
                    .l2_cpu_no_intctrl                       (l2_cpu0_no_intctrl),
                    .l2_cpu_pf_rd_vld_skid_popped            (l2_cpu0_pf_rd_vld_skid_popped),
                    .l2_cpu_pf_throttle_q                    (l2_cpu0_pf_throttle_q),
                    .l2_cpu_pmu_events                       (l2_cpu0_pmu_events[33:0]),
                    .l2_cpu_rbufid                           (l2_cpu0_rbufid[2:0]),
                    .l2_cpu_rd_arb                           (l2_cpu0_rd_arb),
                    .l2_cpu_rd_vld_skid                      (l2_cpu0_rd_vld_skid),
                    .l2_cpu_rexfail                          (l2_cpu0_rexfail),
                    .l2_cpu_rstate                           (l2_cpu0_rstate[1:0]),
                    .l2_cpu_rvalid                           (l2_cpu0_rvalid),
                    .l2_cpu_spec_bufid                       (l2_cpu0_spec_bufid[2:0]),
                    .l2_cpu_spec_valid                       (l2_cpu0_spec_valid),
                    .l2_cpu_spr_rd_data                      (l2_cpu0_spr_rd_data[63:0]),
                    .l2_cpu_tbw_dbl_ecc_err                  (l2_cpu0_tbw_dbl_ecc_err),
                    .l2_cpu_tbw_desc_data                    (l2_cpu0_tbw_desc_data[63:0]),
                    .l2_cpu_tbw_desc_vld                     (l2_cpu0_tbw_desc_vld),
                    .l2_cpu_tbw_ext_err                      (l2_cpu0_tbw_ext_err),
                    .l2_cpu_tbw_ext_err_type                 (l2_cpu0_tbw_ext_err_type),
                    .l2_cpu_tlb_ccb_clken_c3                 (l2_cpu0_tlb_ccb_clken_c3),
                    .l2_cpu_tlb_ccb_req_c3                   (l2_cpu0_tlb_ccb_req_c3),
                    .l2_cpu_tlb_sync_complete                (l2_cpu0_tlb_sync_complete),
                    .l2_cpu_tlb_sync_req                     (l2_cpu0_tlb_sync_req),
                    .l2_cpu_trq_haz_pending                  (l2_cpu0_trq_haz_pending),
                    .l2_cpu_wr_arb                           (l2_cpu0_wr_arb),
                    .l2_cpu_wr_data_stall                    (l2_cpu0_wr_data_stall),
                    .l2_cpu_wr_ex_fail                       (l2_cpu0_wr_ex_fail),
                    .l2_cpu_wr_ex_resp                       (l2_cpu0_wr_ex_resp),
                    .l2_cpu_wr_vld_skid                      (l2_cpu0_wr_vld_skid),
                    .l2_cpu_wrq_haz_pending                  (l2_cpu0_wrq_haz_pending),
                    .l2_tbnk0_cpu_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu0_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk0_cpu_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu0_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk0_cpu_trq_clr_l4_dly2_q          (l2_tbnk0_cpu0_trq_clr_l4_dly2_q),
                    .l2_tbnk0_cpu_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu0_wrq_clr_l4_dly2_q[5:0]),
                    .l2_tbnk1_cpu_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu0_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk1_cpu_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu0_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk1_cpu_trq_clr_l4_dly2_q          (l2_tbnk1_cpu0_trq_clr_l4_dly2_q),
                    .l2_tbnk1_cpu_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu0_wrq_clr_l4_dly2_q[5:0]),
                    .ncorereset_cpu                          (ncorereset_cpu0_o),
                    .ncpuporeset_cpu                         (ncpuporeset_cpu0_o),
                    .niden_cpu                               (niden_cpu0_o),
                    .nmbistreset_cpu                         (nmbistreset_cpu0_o),
                    .rvbaraddr_cpu                           (rvbaraddr_cpu0_o[43:2]),
                    .spiden_cpu                              (spiden_cpu0_o),
                    .spniden_cpu                             (spniden_cpu0_o),
                    .syncreqm_cpu                            (syncreqm_cpu0_o),
                    .tm_cnthctl_kernel                       (tm_cpu0_cnthctl_kernel[1:0]),
                    .tm_cntkctl_usr                          (tm_cpu0_cntkctl_usr[3:0]),
                    .tsvalueb_cpu                            (tsvalueb_cpu0_o[63:0]),
                    .vinithi_cpu                             (vinithi_cpu0_o)
                  );    // ucpu0

    maia_cpu ucpu1( // outputs
                    .afreadym_cpu                            (afreadym_cpu1_i),
                    .atbytesm_cpu                            (atbytesm_cpu1_i[1:0]),
                    .atdatam_cpu                             (atdatam_cpu1_i[31:0]),
                    .atidm_cpu                               (atidm_cpu1_i[6:0]),
                    .atvalidm_cpu                            (atvalidm_cpu1_i),
                    .commrx_cpu                              (commrx_cpu1_i),
                    .commtx_cpu                              (commtx_cpu1_i),
                    .dbgack_cpu                              (dbgack_cpu1_i),
                    .dbgnopwrdwn_cpu                         (dbgnopwrdwn_cpu1_i),
                    .dbgrstreq_cpu                           (dbgrstreq_cpu1_i),
                    .ds_cpuectlr_ret                         (ds_cpu1_cpuectlr_ret[2:0]),
                    .ds_cpuectlr_smp                         (ds_cpu1_cpuectlr_smp),
                    .ds_fiq_wfe_qual                         (ds_cpu1_fiq_wfe_qual),
                    .ds_fiq_wfi_qual                         (ds_cpu1_fiq_wfi_qual),
                    .ds_flush                                (ds_cpu1_flush),
                    .ds_flush_type                           (ds_cpu1_flush_type[5:0]),
                    .ds_hcr_va                               (ds_cpu1_hcr_va),
                    .ds_hcr_vf                               (ds_cpu1_hcr_vf),
                    .ds_hcr_vi                               (ds_cpu1_hcr_vi),
                    .ds_ic_aa64naa32                         (ds_cpu1_ic_aa64naa32),
                    .ds_ic_cpsr_mode                         (ds_cpu1_ic_cpsr_mode[4:0]),
                    .ds_ic_hcr_change                        (ds_cpu1_ic_hcr_change),
                    .ds_ic_sample_spr                        (ds_cpu1_ic_sample_spr),
                    .ds_ic_scr_change                        (ds_cpu1_ic_scr_change),
                    .ds_imp_abrt_wfe_qual                    (ds_cpu1_imp_abrt_wfe_qual),
                    .ds_imp_abrt_wfi_qual                    (ds_cpu1_imp_abrt_wfi_qual),
                    .ds_irq_wfe_qual                         (ds_cpu1_irq_wfe_qual),
                    .ds_irq_wfi_qual                         (ds_cpu1_irq_wfi_qual),
                    .ds_l2_spr_addr                          (ds_cpu1_l2_spr_addr[8:0]),
                    .ds_l2_spr_dw                            (ds_cpu1_l2_spr_dw),
                    .ds_l2_spr_en                            (ds_cpu1_l2_spr_en),
                    .ds_l2_spr_rd                            (ds_cpu1_l2_spr_rd),
                    .ds_l2_spr_wr                            (ds_cpu1_l2_spr_wr),
                    .ds_l2_spr_wr_data                       (ds_cpu1_l2_spr_wr_data[63:0]),
                    .ds_reset_req                            (ds_cpu1_reset_req),
                    .ds_sev_req                              (ds_cpu1_sev_req),
                    .ds_sevl_req                             (ds_cpu1_sevl_req),
                    .ds_vfiq_wfe_qual                        (ds_cpu1_vfiq_wfe_qual),
                    .ds_vfiq_wfi_qual                        (ds_cpu1_vfiq_wfi_qual),
                    .ds_vimp_abrt_wfe_qual                   (ds_cpu1_vimp_abrt_wfe_qual),
                    .ds_vimp_abrt_wfi_qual                   (ds_cpu1_vimp_abrt_wfi_qual),
                    .ds_virq_wfe_qual                        (ds_cpu1_virq_wfe_qual),
                    .ds_virq_wfi_qual                        (ds_cpu1_virq_wfi_qual),
                    .ds_wfe_req                              (ds_cpu1_wfe_req),
                    .ds_wfi_req                              (ds_cpu1_wfi_req),
                    .dt_coredbg_in_reset_gclk                (dt_cpu1_coredbg_in_reset_gclk),
                    .dt_cti_trigin_1to0_gclk                 (dt_cpu1_cti_trigin_1to0_gclk[1:0]),
                    .dt_cti_trigin_7to4_gclk                 (dt_cpu1_cti_trigin_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_7to4_gclk             (dt_cpu1_cti_trigoutack_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_bit1_gclk             (dt_cpu1_cti_trigoutack_bit1_gclk),
                    .dt_dbif_ack_gclk                        (dt_cpu1_dbif_ack_gclk),
                    .dt_dbif_err_gclk                        (dt_cpu1_dbif_err_gclk),
                    .dt_dbif_rddata_gclk                     (dt_cpu1_dbif_rddata_gclk[31:0]),
                    .dt_et_oslock_gclk                       (dt_cpu1_et_oslock_gclk),
                    .dt_halt_ack_gclk                        (dt_cpu1_halt_ack_gclk),
                    .dt_hlt_dbgevt_ok_gclk                   (dt_cpu1_hlt_dbgevt_ok_gclk),
                    .dt_os_double_lock_gclk                  (dt_cpu1_os_double_lock_gclk),
                    .dt_pmusnapshot_ack_gclk                 (dt_cpu1_pmusnapshot_ack_gclk),
                    .dt_wfx_dbg_req_gclk                     (dt_cpu1_wfx_dbg_req_gclk),
                    .etclken_cpu                             (etclken_cpu1_i),
                    .l2_cpu_dsq_rd_byte_strb_q               (l2_cpu1_dsq_rd_byte_strb_q[15:0]),
                    .l2_cpu_dsq_rd_data_q                    (l2_cpu1_dsq_rd_data_q[129:0]),
                    .l2_cpu_dt_pmu_evt_en                    (l2_cpu1_dt_pmu_evt_en),
                    .l2_cpu_early_rd_reqe4_e5_q              (l2_cpu1_early_rd_reqe4_e5_q),
                    .l2_cpu_ic_addr_arb_set                  (l2_cpu1_ic_addr_arb_set[15:0]),
                    .l2_cpu_ic_arb_fast                      (l2_cpu1_ic_arb_fast),
                    .l2_cpu_ic_data_arb_set                  (l2_cpu1_ic_data_arb_set[31:0]),
                    .l2_cpu_ic_elem_size_arb_set             (l2_cpu1_ic_elem_size_arb_set[2:0]),
                    .l2_cpu_ic_excl_arb_set                  (l2_cpu1_ic_excl_arb_set),
                    .l2_cpu_ic_id_arb_set                    (l2_cpu1_ic_id_arb_set[2:0]),
                    .l2_cpu_ic_ns_arb_set                    (l2_cpu1_ic_ns_arb_set),
                    .l2_cpu_ic_write_arb_set                 (l2_cpu1_ic_write_arb_set),
                    .l2_cpu_idle_wakeup_q                    (l2_cpu1_idle_wakeup_q),
                    .l2_cpu_if_ccb_resp                      (l2_cpu1_if_ccb_resp),
                    .l2_cpu_if_ccb_resp_id                   (l2_cpu1_if_ccb_resp_id[4:0]),
                    .l2_cpu_if_sync_done_q                   (l2_cpu1_if_sync_done_q),
                    .l2_cpu_lrq_haz_clr_id_dcd_q             (l2_cpu1_lrq_haz_clr_id_dcd_q[5:0]),
                    .l2_cpu_ls_ccb_data_wr                   (l2_cpu1_ls_ccb_data_wr),
                    .l2_cpu_ls_ccb_resp                      (l2_cpu1_ls_ccb_resp),
                    .l2_cpu_ls_ccb_resp_id                   (l2_cpu1_ls_ccb_resp_id[4:0]),
                    .l2_cpu_ls_rd_haz_id_arb_q               (l2_cpu1_ls_rd_haz_id_arb_q[3:0]),
                    .l2_cpu_ls_rd_haz_vld_arb_q              (l2_cpu1_ls_rd_haz_vld_arb_q),
                    .l2_cpu_ls_wr_ccb_id_w2a                 (l2_cpu1_ls_wr_ccb_id_w2a[4:0]),
                    .l2_cpu_ls_wr_data_w2a                   (l2_cpu1_ls_wr_data_w2a[127:0]),
                    .l2_cpu_ls_wr_dirty_w2a                  (l2_cpu1_ls_wr_dirty_w2a),
                    .l2_cpu_ls_wr_err_w2a                    (l2_cpu1_ls_wr_err_w2a),
                    .l2_cpu_ls_wr_haz_id_arb_q               (l2_cpu1_ls_wr_haz_id_arb_q[2:0]),
                    .l2_cpu_ls_wr_haz_vld_arb_q              (l2_cpu1_ls_wr_haz_vld_arb_q),
                    .l2_cpu_ls_wr_last_w2a                   (l2_cpu1_ls_wr_last_w2a),
                    .l2_cpu_ls_wr_req_w2a                    (l2_cpu1_ls_wr_req_w2a),
                    .l2_cpu_ls_wr_type_w2a                   (l2_cpu1_ls_wr_type_w2a[2:0]),
                    .l2_cpu_rd_aarch64_arb_set               (l2_cpu1_rd_aarch64_arb_set),
                    .l2_cpu_rd_addr_arb_set                  (l2_cpu1_rd_addr_arb_set[44:0]),
                    .l2_cpu_rd_arb_fast                      (l2_cpu1_rd_arb_fast),
                    .l2_cpu_rd_asid_arb_set                  (l2_cpu1_rd_asid_arb_set[15:8]),
                    .l2_cpu_rd_bypass_arb_set                (l2_cpu1_rd_bypass_arb_set),
                    .l2_cpu_rd_bypass_bufid_e5               (l2_cpu1_rd_bypass_bufid_e5[2:0]),
                    .l2_cpu_rd_bypass_lrq_id_e5              (l2_cpu1_rd_bypass_lrq_id_e5[2:0]),
                    .l2_cpu_rd_bypass_req_can_e5             (l2_cpu1_rd_bypass_req_can_e5),
                    .l2_cpu_rd_bypass_way_e5                 (l2_cpu1_rd_bypass_way_e5),
                    .l2_cpu_rd_cache_attr_arb_set            (l2_cpu1_rd_cache_attr_arb_set[2:0]),
                    .l2_cpu_rd_elem_size_arb_set             (l2_cpu1_rd_elem_size_arb_set[2:0]),
                    .l2_cpu_rd_excl_arb_set                  (l2_cpu1_rd_excl_arb_set),
                    .l2_cpu_rd_id_arb_set                    (l2_cpu1_rd_id_arb_set[4:0]),
                    .l2_cpu_rd_lrq_id_arb_set                (l2_cpu1_rd_lrq_id_arb_set[2:0]),
                    .l2_cpu_rd_page_attr_arb_set             (l2_cpu1_rd_page_attr_arb_set[7:0]),
                    .l2_cpu_rd_prfm_arb_set                  (l2_cpu1_rd_prfm_arb_set),
                    .l2_cpu_rd_priv_arb_set                  (l2_cpu1_rd_priv_arb_set),
                    .l2_cpu_rd_replayed_arb_set              (l2_cpu1_rd_replayed_arb_set),
                    .l2_cpu_rd_shared_arb_set                (l2_cpu1_rd_shared_arb_set[1:0]),
                    .l2_cpu_rd_type_arb_set                  (l2_cpu1_rd_type_arb_set[6:0]),
                    .l2_cpu_rd_va48_arb_set                  (l2_cpu1_rd_va48_arb_set),
                    .l2_cpu_rd_way_arb_set                   (l2_cpu1_rd_way_arb_set),
                    .l2_cpu_tlb_sync_done_q                  (l2_cpu1_tlb_sync_done_q),
                    .l2_cpu_tw_ccb_resp                      (l2_cpu1_tw_ccb_resp),
                    .l2_cpu_tw_ccb_resp_id                   (l2_cpu1_tw_ccb_resp_id[4:0]),
                    .l2_cpu_wr_1st_replayed_arb_set          (l2_cpu1_wr_1st_replayed_arb_set),
                    .l2_cpu_wr_addr_arb_set                  (l2_cpu1_wr_addr_arb_set[44:0]),
                    .l2_cpu_wr_arb_fast                      (l2_cpu1_wr_arb_fast),
                    .l2_cpu_wr_cache_attr_arb_set            (l2_cpu1_wr_cache_attr_arb_set[2:0]),
                    .l2_cpu_wr_cl_id_arb_set                 (l2_cpu1_wr_cl_id_arb_set[11:0]),
                    .l2_cpu_wr_clean_evict_arb_set           (l2_cpu1_wr_clean_evict_arb_set),
                    .l2_cpu_wr_data                          (l2_cpu1_wr_data[143:0]),
                    .l2_cpu_wr_data_vld_x1_q                 (l2_cpu1_wr_data_vld_x1_q),
                    .l2_cpu_wr_dirty_arb_set                 (l2_cpu1_wr_dirty_arb_set),
                    .l2_cpu_wr_elem_size_arb_set             (l2_cpu1_wr_elem_size_arb_set[2:0]),
                    .l2_cpu_wr_err_arb_set                   (l2_cpu1_wr_err_arb_set),
                    .l2_cpu_wr_evict_x1_q                    (l2_cpu1_wr_evict_x1_q),
                    .l2_cpu_wr_id_arb_set                    (l2_cpu1_wr_id_arb_set[3:0]),
                    .l2_cpu_wr_last_arb_set                  (l2_cpu1_wr_last_arb_set),
                    .l2_cpu_wr_page_attr_arb_set             (l2_cpu1_wr_page_attr_arb_set[7:0]),
                    .l2_cpu_wr_partial_dw_arb_set            (l2_cpu1_wr_partial_dw_arb_set[3:0]),
                    .l2_cpu_wr_priv_arb_set                  (l2_cpu1_wr_priv_arb_set),
                    .l2_cpu_wr_shared_arb_set                (l2_cpu1_wr_shared_arb_set[1:0]),
                    .l2_cpu_wr_type_arb_set                  (l2_cpu1_wr_type_arb_set[2:0]),
                    .l2_cpu_wr_way_arb_set                   (l2_cpu1_wr_way_arb_set),
                    .l2_cpu_wrq_almost_full                  (l2_cpu1_wrq_almost_full),
                    .l2_cpu_wrq_haz_clr_id_dcd_q             (l2_cpu1_wrq_haz_clr_id_dcd_q[15:0]),
                    .ls_clrexmon                             (ls_cpu1_clrexmon),
                    .ls_imp_abort_containable                (ls_cpu1_imp_abort_containable),
                    .ls_imp_abort_dec                        (ls_cpu1_imp_abort_dec),
                    .ls_imp_abort_ecc                        (ls_cpu1_imp_abort_ecc),
                    .ls_imp_abort_slv                        (ls_cpu1_imp_abort_slv),
                    .ls_raw_eae_nonsec                       (ls_cpu1_raw_eae_nonsec),
                    .ls_raw_eae_secure                       (ls_cpu1_raw_eae_secure),
                    .ncommirq_cpu                            (ncommirq_cpu1_i),
                    .npmuirq_cpu                             (npmuirq_cpu1_i),
                    .pm_export_cpu                           (pm_export_cpu1_i),
                    .pmuevent_cpu                            (pmuevent_cpu1_i[24:0]),

                    // inputs
                    .aa64naa32_cpu                           (aa64naa32_cpu1_o),
                    .afvalidm_cpu                            (afvalidm_cpu1_o),
                    .atclken_cpu                             (atclken_cpu1_o),
                    .atreadym_cpu                            (atreadym_cpu1_o),
                    .cfgend_cpu                              (cfgend_cpu1_o),
                    .cfgte_cpu                               (cfgte_cpu1_o),
                    .ck_crcx_clk_en_n                        (ck_cpu1_crcx_clk_en_n),
                    .ck_event_reg                            (ck_cpu1_event_reg),
                    .ck_gclkt                                (ck_gclkt[1]),
                    .ck_wfe_ack                              (ck_cpu1_wfe_ack),
                    .ck_wfi_ack                              (ck_cpu1_wfi_ack),
                    .clusteridaff1_cpu                       (clusteridaff1_cpu1_o[7:0]),
                    .clusteridaff2_cpu                       (clusteridaff2_cpu1_o[7:0]),
                    .cp15sdisable_cpu                        (cp15sdisable_cpu1_o),
                    .cpuid                                   (cpuid_cpu1_o[1:0]),
                    .cryptodisable_cpu                       (cryptodisable_cpu1_o),
                    .dbgen_cpu                               (dbgen_cpu1_o),
                    .dbgl1rstdisable_cpu                     (dbgl1rstdisable_cpu1_o),
                    .dbgromaddr_cpu                          (dbgromaddr_cpu1_o[43:12]),
                    .dbgromaddrv_cpu                         (dbgromaddrv_cpu1_o),
                    .dftcrclkdisable_cpu                     (dftcrclkdisable_cpu1_o),
                    .dftramhold_cpu                          (dftramhold_cpu1_o),
                    .dftrstdisable_cpu                       (dftrstdisable_cpu1_o),
                    .dftse_cpu                               (dftse_cpu1_o),
                    .dt_cti_triginack_1to0_pclk              (dt_cpu1_cti_triginack_1to0_pclk[1:0]),
                    .dt_cti_triginack_7to4_pclk              (dt_cpu1_cti_triginack_7to4_pclk[3:0]),
                    .dt_cti_trigout_1to0_pclk                (dt_cpu1_cti_trigout_1to0_pclk[1:0]),
                    .dt_cti_trigout_7to4_pclk                (dt_cpu1_cti_trigout_7to4_pclk[3:0]),
                    .dt_dbif_addr_pclk                       (dt_cpu1_dbif_addr_pclk[14:2]),
                    .dt_dbif_locked_pclk                     (dt_cpu1_dbif_locked_pclk),
                    .dt_dbif_req_pclk                        (dt_cpu1_dbif_req_pclk),
                    .dt_dbif_wrdata_pclk                     (dt_cpu1_dbif_wrdata_pclk[31:0]),
                    .dt_dbif_write_pclk                      (dt_cpu1_dbif_write_pclk),
                    .dt_edacr_frc_idleack_pclk               (dt_cpu1_edacr_frc_idleack_pclk),
                    .dt_edbgrq_pclk                          (dt_cpu1_edbgrq_pclk),
                    .dt_edecr_osuce_pclk                     (dt_cpu1_edecr_osuce_pclk),
                    .dt_edecr_rce_pclk                       (dt_cpu1_edecr_rce_pclk),
                    .dt_edecr_ss_pclk                        (dt_cpu1_edecr_ss_pclk),
                    .dt_edprcr_corepurq_pclk                 (dt_cpu1_edprcr_corepurq_pclk),
                    .dt_noclkstop_pclk                       (dt_cpu1_noclkstop_pclk),
                    .dt_pmusnapshot_req_pclk                 (dt_cpu1_pmusnapshot_req_pclk),
                    .dt_wfx_wakeup_pclk                      (dt_cpu1_wfx_wakeup_pclk),
                    .giccdisable_cpu                         (giccdisable_cpu1_o),
                    .ic_block_eoi_sgi_wr                     (ic_block_eoi_sgi_wr[1]),
                    .ic_el_change_complete                   (ic_el_change_complete[1]),
                    .ic_hcr_change_complete                  (ic_hcr_change_complete[1]),
                    .ic_ich_el2_tall0                        (ic_ich_el2_tall0[1]),
                    .ic_ich_el2_tall1                        (ic_ich_el2_tall1[1]),
                    .ic_ich_el2_tc                           (ic_ich_el2_tc[1]),
                    .ic_nfiq                                 (ic_nfiq[1]),
                    .ic_nirq                                 (ic_nirq[1]),
                    .ic_nsei                                 (ic_nsei[1]),
                    .ic_nvfiq                                (ic_nvfiq[1]),
                    .ic_nvirq                                (ic_nvirq[1]),
                    .ic_nvsei                                (ic_nvsei[1]),
                    .ic_p_valid                              (ic_p_valid[1]),
                    .ic_sample_spr                           (ic_sample_spr[1]),
                    .ic_scr_change_complete                  (ic_scr_change_complete[1]),
                    .ic_sra_el1ns_en                         (ic_sra_el1ns_en[1]),
                    .ic_sra_el1s_en                          (ic_sra_el1s_en[1]),
                    .ic_sra_el2_en                           (ic_sra_el2_en[1]),
                    .ic_sra_el3_en                           (ic_sra_el3_en[1]),
                    .ic_sre_el1ns_hyp_trap                   (ic_sre_el1ns_hyp_trap[1]),
                    .ic_sre_el1ns_mon_trap                   (ic_sre_el1ns_mon_trap[1]),
                    .ic_sre_el1s_mon_trap                    (ic_sre_el1s_mon_trap[1]),
                    .ic_sre_el2_mon_trap                     (ic_sre_el2_mon_trap[1]),
                    .l2_cpu_arb_thrshld_timeout_en           (l2_cpu1_arb_thrshld_timeout_en),
                    .l2_cpu_barrier_done                     (l2_cpu1_barrier_done),
                    .l2_cpu_blk_non_evict_wr                 (l2_cpu1_blk_non_evict_wr),
                    .l2_cpu_ccb_dbg_req_c3                   (l2_cpu1_ccb_dbg_req_c3),
                    .l2_cpu_ccb_req_addr_c3                  (l2_cpu1_ccb_req_addr_c3[48:0]),
                    .l2_cpu_ccb_req_id_c3                    (l2_cpu1_ccb_req_id_c3[4:0]),
                    .l2_cpu_ccb_req_info_c3                  (l2_cpu1_ccb_req_info_c3[23:0]),
                    .l2_cpu_ccb_req_type_c3                  (l2_cpu1_ccb_req_type_c3[8:0]),
                    .l2_cpu_cfg_ecc_en                       (l2_cpu1_cfg_ecc_en),
                    .l2_cpu_dbufid_r1                        (l2_cpu1_dbufid_r1[2:0]),
                    .l2_cpu_ddata_r2                         (l2_cpu1_ddata_r2[129:0]),
                    .l2_cpu_ddbl_ecc_err_r3                  (l2_cpu1_ddlb_ecc_err_r3),
                    .l2_cpu_dext_err_r2                      (l2_cpu1_dext_err_r2),
                    .l2_cpu_dext_err_type_r2                 (l2_cpu1_dext_err_type_r2),
                    .l2_cpu_disable_clean_evict_opt          (l2_cpu1_disable_clean_evict_opt),
                    .l2_cpu_dlast_r1                         (l2_cpu1_dlast_r1),
                    .l2_cpu_dsngl_ecc_err_r3                 (l2_cpu1_dsngl_ecc_err_r3),
                    .l2_cpu_dsq_clr_id_q                     (l2_cpu1_dsq_clr_id_q[3:0]),
                    .l2_cpu_dsq_clr_vld_q                    (l2_cpu1_dsq_clr_vld_q),
                    .l2_cpu_dsq_rd_buf_id                    (l2_cpu1_dsq_rd_buf_id[3:0]),
                    .l2_cpu_dsq_rd_en                        (l2_cpu1_dsq_rd_en),
                    .l2_cpu_dsq_rd_en_x2                     (l2_cpu1_dsq_rd_en_x2),
                    .l2_cpu_dvalid_r1                        (l2_cpu1_dvalid_r1),
                    .l2_cpu_flsh_if_rd_id_l4_dly             (l2_cpu1_flsh_if_rd_id_l4_dly[1:0]),
                    .l2_cpu_flsh_if_rd_l4_dly                (l2_cpu1_flsh_if_rd_l4_dly),
                    .l2_cpu_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_rd_id_l2_dly             (l2_cpu1_flsh_ls_rd_id_l2_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_id_l4_dly             (l2_cpu1_flsh_ls_rd_id_l4_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_l2_dly                (l2_cpu1_flsh_ls_rd_l2_dly),
                    .l2_cpu_flsh_ls_rd_l4_dly                (l2_cpu1_flsh_ls_rd_l4_dly),
                    .l2_cpu_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_wr_evict_l4_dly          (l2_cpu1_flsh_ls_wr_evict_l4_dly),
                    .l2_cpu_flsh_ls_wr_id_l2_dly             (l2_cpu1_flsh_ls_wr_id_l2_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_id_l4_dly             (l2_cpu1_flsh_ls_wr_id_l4_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_l2_dly                (l2_cpu1_flsh_ls_wr_l2_dly),
                    .l2_cpu_flsh_ls_wr_l4_dly                (l2_cpu1_flsh_ls_wr_l4_dly),
                    .l2_cpu_flsh_tw_rd_l4_dly                (l2_cpu1_flsh_tw_rd_l4_dly),
                    .l2_cpu_ibufid_r1                        (l2_cpu1_ibufid_r1[1:0]),
                    .l2_cpu_ic_barrier_stall_q               (l2_cpu1_ic_barrier_stall_q),
                    .l2_cpu_ic_base                          (l2_cpu1_ic_base[43:18]),
                    .l2_cpu_ic_vld_skid                      (l2_cpu1_ic_vld_skid),
                    .l2_cpu_idata_r2                         (l2_cpu1_idata_r2[127:0]),
                    .l2_cpu_idbl_ecc_err_r3                  (l2_cpu1_idlb_ecc_err_r3),
                    .l2_cpu_idle_block_reqs_q                (l2_cpu1_idle_block_reqs_q),
                    .l2_cpu_iext_err_r2                      (l2_cpu1_iext_err_r2),
                    .l2_cpu_iext_err_type_r2                 (l2_cpu1_iext_err_type_r2),
                    .l2_cpu_if_ccb_clken_c3                  (l2_cpu1_if_ccb_clken_c3),
                    .l2_cpu_if_ccb_req_c3                    (l2_cpu1_if_ccb_req_c3),
                    .l2_cpu_if_sync_req                      (l2_cpu1_if_sync_req),
                    .l2_cpu_ifq_haz_pending                  (l2_cpu1_ifq_haz_pending),
                    .l2_cpu_isngl_ecc_err_r3                 (l2_cpu1_isngl_ecc_err_r3),
                    .l2_cpu_ivalid_r1                        (l2_cpu1_ivalid_r1),
                    .l2_cpu_l2_cache_size                    (l2_cpu1_l2_cache_size[1:0]),
                    .l2_cpu_lrq_haz_pending                  (l2_cpu1_lrq_haz_pending),
                    .l2_cpu_ls_ccb_clken_c3                  (l2_cpu1_ls_ccb_clken_c3),
                    .l2_cpu_ls_ccb_req_c3                    (l2_cpu1_ls_ccb_req_c3),
                    .l2_cpu_ls_peq_coll_l4_dly               (l2_cpu1_ls_peq_coll_l4_dly),
                    .l2_cpu_ls_sync_req                      (l2_cpu1_ls_sync_req),
                    .l2_cpu_mbist1_addr_b1                   (l2_cpu1_mbist1_addr_b1[12:0]),
                    .l2_cpu_mbist1_all_b1                    (l2_cpu1_mbist1_all_b1),
                    .l2_cpu_mbist1_array_b1                  (l2_cpu1_mbist1_array_b1[3:0]),
                    .l2_cpu_mbist1_be_b1                     (l2_cpu1_mbist1_be_b1[7:0]),
                    .l2_cpu_mbist1_en_b1                     (l2_cpu1_mbist1_en_b1),
                    .l2_cpu_mbist1_rd_en_b1                  (l2_cpu1_mbist1_rd_en_b1),
                    .l2_cpu_mbist1_wr_en_b1                  (l2_cpu1_mbist1_wr_en_b1),
                    .l2_cpu_no_intctrl                       (l2_cpu1_no_intctrl),
                    .l2_cpu_pf_rd_vld_skid_popped            (l2_cpu1_pf_rd_vld_skid_popped),
                    .l2_cpu_pf_throttle_q                    (l2_cpu1_pf_throttle_q),
                    .l2_cpu_pmu_events                       (l2_cpu1_pmu_events[33:0]),
                    .l2_cpu_rbufid                           (l2_cpu1_rbufid[2:0]),
                    .l2_cpu_rd_arb                           (l2_cpu1_rd_arb),
                    .l2_cpu_rd_vld_skid                      (l2_cpu1_rd_vld_skid),
                    .l2_cpu_rexfail                          (l2_cpu1_rexfail),
                    .l2_cpu_rstate                           (l2_cpu1_rstate[1:0]),
                    .l2_cpu_rvalid                           (l2_cpu1_rvalid),
                    .l2_cpu_spec_bufid                       (l2_cpu1_spec_bufid[2:0]),
                    .l2_cpu_spec_valid                       (l2_cpu1_spec_valid),
                    .l2_cpu_spr_rd_data                      (l2_cpu1_spr_rd_data[63:0]),
                    .l2_cpu_tbw_dbl_ecc_err                  (l2_cpu1_tbw_dbl_ecc_err),
                    .l2_cpu_tbw_desc_data                    (l2_cpu1_tbw_desc_data[63:0]),
                    .l2_cpu_tbw_desc_vld                     (l2_cpu1_tbw_desc_vld),
                    .l2_cpu_tbw_ext_err                      (l2_cpu1_tbw_ext_err),
                    .l2_cpu_tbw_ext_err_type                 (l2_cpu1_tbw_ext_err_type),
                    .l2_cpu_tlb_ccb_clken_c3                 (l2_cpu1_tlb_ccb_clken_c3),
                    .l2_cpu_tlb_ccb_req_c3                   (l2_cpu1_tlb_ccb_req_c3),
                    .l2_cpu_tlb_sync_complete                (l2_cpu1_tlb_sync_complete),
                    .l2_cpu_tlb_sync_req                     (l2_cpu1_tlb_sync_req),
                    .l2_cpu_trq_haz_pending                  (l2_cpu1_trq_haz_pending),
                    .l2_cpu_wr_arb                           (l2_cpu1_wr_arb),
                    .l2_cpu_wr_data_stall                    (l2_cpu1_wr_data_stall),
                    .l2_cpu_wr_ex_fail                       (l2_cpu1_wr_ex_fail),
                    .l2_cpu_wr_ex_resp                       (l2_cpu1_wr_ex_resp),
                    .l2_cpu_wr_vld_skid                      (l2_cpu1_wr_vld_skid),
                    .l2_cpu_wrq_haz_pending                  (l2_cpu1_wrq_haz_pending),
                    .l2_tbnk0_cpu_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu1_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk0_cpu_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu1_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk0_cpu_trq_clr_l4_dly2_q          (l2_tbnk0_cpu1_trq_clr_l4_dly2_q),
                    .l2_tbnk0_cpu_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu1_wrq_clr_l4_dly2_q[5:0]),
                    .l2_tbnk1_cpu_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu1_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk1_cpu_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu1_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk1_cpu_trq_clr_l4_dly2_q          (l2_tbnk1_cpu1_trq_clr_l4_dly2_q),
                    .l2_tbnk1_cpu_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu1_wrq_clr_l4_dly2_q[5:0]),
                    .ncorereset_cpu                          (ncorereset_cpu1_o),
                    .ncpuporeset_cpu                         (ncpuporeset_cpu1_o),
                    .niden_cpu                               (niden_cpu1_o),
                    .nmbistreset_cpu                         (nmbistreset_cpu1_o),
                    .rvbaraddr_cpu                           (rvbaraddr_cpu1_o[43:2]),
                    .spiden_cpu                              (spiden_cpu1_o),
                    .spniden_cpu                             (spniden_cpu1_o),
                    .syncreqm_cpu                            (syncreqm_cpu1_o),
                    .tm_cnthctl_kernel                       (tm_cpu1_cnthctl_kernel[1:0]),
                    .tm_cntkctl_usr                          (tm_cpu1_cntkctl_usr[3:0]),
                    .tsvalueb_cpu                            (tsvalueb_cpu1_o[63:0]),
                    .vinithi_cpu                             (vinithi_cpu1_o)
                  );    // ucpu1

    maia_cpu ucpu2( // outputs
                    .afreadym_cpu                            (afreadym_cpu2_i),
                    .atbytesm_cpu                            (atbytesm_cpu2_i[1:0]),
                    .atdatam_cpu                             (atdatam_cpu2_i[31:0]),
                    .atidm_cpu                               (atidm_cpu2_i[6:0]),
                    .atvalidm_cpu                            (atvalidm_cpu2_i),
                    .commrx_cpu                              (commrx_cpu2_i),
                    .commtx_cpu                              (commtx_cpu2_i),
                    .dbgack_cpu                              (dbgack_cpu2_i),
                    .dbgnopwrdwn_cpu                         (dbgnopwrdwn_cpu2_i),
                    .dbgrstreq_cpu                           (dbgrstreq_cpu2_i),
                    .ds_cpuectlr_ret                         (ds_cpu2_cpuectlr_ret[2:0]),
                    .ds_cpuectlr_smp                         (ds_cpu2_cpuectlr_smp),
                    .ds_fiq_wfe_qual                         (ds_cpu2_fiq_wfe_qual),
                    .ds_fiq_wfi_qual                         (ds_cpu2_fiq_wfi_qual),
                    .ds_flush                                (ds_cpu2_flush),
                    .ds_flush_type                           (ds_cpu2_flush_type[5:0]),
                    .ds_hcr_va                               (ds_cpu2_hcr_va),
                    .ds_hcr_vf                               (ds_cpu2_hcr_vf),
                    .ds_hcr_vi                               (ds_cpu2_hcr_vi),
                    .ds_ic_aa64naa32                         (ds_cpu2_ic_aa64naa32),
                    .ds_ic_cpsr_mode                         (ds_cpu2_ic_cpsr_mode[4:0]),
                    .ds_ic_hcr_change                        (ds_cpu2_ic_hcr_change),
                    .ds_ic_sample_spr                        (ds_cpu2_ic_sample_spr),
                    .ds_ic_scr_change                        (ds_cpu2_ic_scr_change),
                    .ds_imp_abrt_wfe_qual                    (ds_cpu2_imp_abrt_wfe_qual),
                    .ds_imp_abrt_wfi_qual                    (ds_cpu2_imp_abrt_wfi_qual),
                    .ds_irq_wfe_qual                         (ds_cpu2_irq_wfe_qual),
                    .ds_irq_wfi_qual                         (ds_cpu2_irq_wfi_qual),
                    .ds_l2_spr_addr                          (ds_cpu2_l2_spr_addr[8:0]),
                    .ds_l2_spr_dw                            (ds_cpu2_l2_spr_dw),
                    .ds_l2_spr_en                            (ds_cpu2_l2_spr_en),
                    .ds_l2_spr_rd                            (ds_cpu2_l2_spr_rd),
                    .ds_l2_spr_wr                            (ds_cpu2_l2_spr_wr),
                    .ds_l2_spr_wr_data                       (ds_cpu2_l2_spr_wr_data[63:0]),
                    .ds_reset_req                            (ds_cpu2_reset_req),
                    .ds_sev_req                              (ds_cpu2_sev_req),
                    .ds_sevl_req                             (ds_cpu2_sevl_req),
                    .ds_vfiq_wfe_qual                        (ds_cpu2_vfiq_wfe_qual),
                    .ds_vfiq_wfi_qual                        (ds_cpu2_vfiq_wfi_qual),
                    .ds_vimp_abrt_wfe_qual                   (ds_cpu2_vimp_abrt_wfe_qual),
                    .ds_vimp_abrt_wfi_qual                   (ds_cpu2_vimp_abrt_wfi_qual),
                    .ds_virq_wfe_qual                        (ds_cpu2_virq_wfe_qual),
                    .ds_virq_wfi_qual                        (ds_cpu2_virq_wfi_qual),
                    .ds_wfe_req                              (ds_cpu2_wfe_req),
                    .ds_wfi_req                              (ds_cpu2_wfi_req),
                    .dt_coredbg_in_reset_gclk                (dt_cpu2_coredbg_in_reset_gclk),
                    .dt_cti_trigin_1to0_gclk                 (dt_cpu2_cti_trigin_1to0_gclk[1:0]),
                    .dt_cti_trigin_7to4_gclk                 (dt_cpu2_cti_trigin_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_7to4_gclk             (dt_cpu2_cti_trigoutack_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_bit1_gclk             (dt_cpu2_cti_trigoutack_bit1_gclk),
                    .dt_dbif_ack_gclk                        (dt_cpu2_dbif_ack_gclk),
                    .dt_dbif_err_gclk                        (dt_cpu2_dbif_err_gclk),
                    .dt_dbif_rddata_gclk                     (dt_cpu2_dbif_rddata_gclk[31:0]),
                    .dt_et_oslock_gclk                       (dt_cpu2_et_oslock_gclk),
                    .dt_halt_ack_gclk                        (dt_cpu2_halt_ack_gclk),
                    .dt_hlt_dbgevt_ok_gclk                   (dt_cpu2_hlt_dbgevt_ok_gclk),
                    .dt_os_double_lock_gclk                  (dt_cpu2_os_double_lock_gclk),
                    .dt_pmusnapshot_ack_gclk                 (dt_cpu2_pmusnapshot_ack_gclk),
                    .dt_wfx_dbg_req_gclk                     (dt_cpu2_wfx_dbg_req_gclk),
                    .etclken_cpu                             (etclken_cpu2_i),
                    .l2_cpu_dsq_rd_byte_strb_q               (l2_cpu2_dsq_rd_byte_strb_q[15:0]),
                    .l2_cpu_dsq_rd_data_q                    (l2_cpu2_dsq_rd_data_q[129:0]),
                    .l2_cpu_dt_pmu_evt_en                    (l2_cpu2_dt_pmu_evt_en),
                    .l2_cpu_early_rd_reqe4_e5_q              (l2_cpu2_early_rd_reqe4_e5_q),
                    .l2_cpu_ic_addr_arb_set                  (l2_cpu2_ic_addr_arb_set[15:0]),
                    .l2_cpu_ic_arb_fast                      (l2_cpu2_ic_arb_fast),
                    .l2_cpu_ic_data_arb_set                  (l2_cpu2_ic_data_arb_set[31:0]),
                    .l2_cpu_ic_elem_size_arb_set             (l2_cpu2_ic_elem_size_arb_set[2:0]),
                    .l2_cpu_ic_excl_arb_set                  (l2_cpu2_ic_excl_arb_set),
                    .l2_cpu_ic_id_arb_set                    (l2_cpu2_ic_id_arb_set[2:0]),
                    .l2_cpu_ic_ns_arb_set                    (l2_cpu2_ic_ns_arb_set),
                    .l2_cpu_ic_write_arb_set                 (l2_cpu2_ic_write_arb_set),
                    .l2_cpu_idle_wakeup_q                    (l2_cpu2_idle_wakeup_q),
                    .l2_cpu_if_ccb_resp                      (l2_cpu2_if_ccb_resp),
                    .l2_cpu_if_ccb_resp_id                   (l2_cpu2_if_ccb_resp_id[4:0]),
                    .l2_cpu_if_sync_done_q                   (l2_cpu2_if_sync_done_q),
                    .l2_cpu_lrq_haz_clr_id_dcd_q             (l2_cpu2_lrq_haz_clr_id_dcd_q[5:0]),
                    .l2_cpu_ls_ccb_data_wr                   (l2_cpu2_ls_ccb_data_wr),
                    .l2_cpu_ls_ccb_resp                      (l2_cpu2_ls_ccb_resp),
                    .l2_cpu_ls_ccb_resp_id                   (l2_cpu2_ls_ccb_resp_id[4:0]),
                    .l2_cpu_ls_rd_haz_id_arb_q               (l2_cpu2_ls_rd_haz_id_arb_q[3:0]),
                    .l2_cpu_ls_rd_haz_vld_arb_q              (l2_cpu2_ls_rd_haz_vld_arb_q),
                    .l2_cpu_ls_wr_ccb_id_w2a                 (l2_cpu2_ls_wr_ccb_id_w2a[4:0]),
                    .l2_cpu_ls_wr_data_w2a                   (l2_cpu2_ls_wr_data_w2a[127:0]),
                    .l2_cpu_ls_wr_dirty_w2a                  (l2_cpu2_ls_wr_dirty_w2a),
                    .l2_cpu_ls_wr_err_w2a                    (l2_cpu2_ls_wr_err_w2a),
                    .l2_cpu_ls_wr_haz_id_arb_q               (l2_cpu2_ls_wr_haz_id_arb_q[2:0]),
                    .l2_cpu_ls_wr_haz_vld_arb_q              (l2_cpu2_ls_wr_haz_vld_arb_q),
                    .l2_cpu_ls_wr_last_w2a                   (l2_cpu2_ls_wr_last_w2a),
                    .l2_cpu_ls_wr_req_w2a                    (l2_cpu2_ls_wr_req_w2a),
                    .l2_cpu_ls_wr_type_w2a                   (l2_cpu2_ls_wr_type_w2a[2:0]),
                    .l2_cpu_rd_aarch64_arb_set               (l2_cpu2_rd_aarch64_arb_set),
                    .l2_cpu_rd_addr_arb_set                  (l2_cpu2_rd_addr_arb_set[44:0]),
                    .l2_cpu_rd_arb_fast                      (l2_cpu2_rd_arb_fast),
                    .l2_cpu_rd_asid_arb_set                  (l2_cpu2_rd_asid_arb_set[15:8]),
                    .l2_cpu_rd_bypass_arb_set                (l2_cpu2_rd_bypass_arb_set),
                    .l2_cpu_rd_bypass_bufid_e5               (l2_cpu2_rd_bypass_bufid_e5[2:0]),
                    .l2_cpu_rd_bypass_lrq_id_e5              (l2_cpu2_rd_bypass_lrq_id_e5[2:0]),
                    .l2_cpu_rd_bypass_req_can_e5             (l2_cpu2_rd_bypass_req_can_e5),
                    .l2_cpu_rd_bypass_way_e5                 (l2_cpu2_rd_bypass_way_e5),
                    .l2_cpu_rd_cache_attr_arb_set            (l2_cpu2_rd_cache_attr_arb_set[2:0]),
                    .l2_cpu_rd_elem_size_arb_set             (l2_cpu2_rd_elem_size_arb_set[2:0]),
                    .l2_cpu_rd_excl_arb_set                  (l2_cpu2_rd_excl_arb_set),
                    .l2_cpu_rd_id_arb_set                    (l2_cpu2_rd_id_arb_set[4:0]),
                    .l2_cpu_rd_lrq_id_arb_set                (l2_cpu2_rd_lrq_id_arb_set[2:0]),
                    .l2_cpu_rd_page_attr_arb_set             (l2_cpu2_rd_page_attr_arb_set[7:0]),
                    .l2_cpu_rd_prfm_arb_set                  (l2_cpu2_rd_prfm_arb_set),
                    .l2_cpu_rd_priv_arb_set                  (l2_cpu2_rd_priv_arb_set),
                    .l2_cpu_rd_replayed_arb_set              (l2_cpu2_rd_replayed_arb_set),
                    .l2_cpu_rd_shared_arb_set                (l2_cpu2_rd_shared_arb_set[1:0]),
                    .l2_cpu_rd_type_arb_set                  (l2_cpu2_rd_type_arb_set[6:0]),
                    .l2_cpu_rd_va48_arb_set                  (l2_cpu2_rd_va48_arb_set),
                    .l2_cpu_rd_way_arb_set                   (l2_cpu2_rd_way_arb_set),
                    .l2_cpu_tlb_sync_done_q                  (l2_cpu2_tlb_sync_done_q),
                    .l2_cpu_tw_ccb_resp                      (l2_cpu2_tw_ccb_resp),
                    .l2_cpu_tw_ccb_resp_id                   (l2_cpu2_tw_ccb_resp_id[4:0]),
                    .l2_cpu_wr_1st_replayed_arb_set          (l2_cpu2_wr_1st_replayed_arb_set),
                    .l2_cpu_wr_addr_arb_set                  (l2_cpu2_wr_addr_arb_set[44:0]),
                    .l2_cpu_wr_arb_fast                      (l2_cpu2_wr_arb_fast),
                    .l2_cpu_wr_cache_attr_arb_set            (l2_cpu2_wr_cache_attr_arb_set[2:0]),
                    .l2_cpu_wr_cl_id_arb_set                 (l2_cpu2_wr_cl_id_arb_set[11:0]),
                    .l2_cpu_wr_clean_evict_arb_set           (l2_cpu2_wr_clean_evict_arb_set),
                    .l2_cpu_wr_data                          (l2_cpu2_wr_data[143:0]),
                    .l2_cpu_wr_data_vld_x1_q                 (l2_cpu2_wr_data_vld_x1_q),
                    .l2_cpu_wr_dirty_arb_set                 (l2_cpu2_wr_dirty_arb_set),
                    .l2_cpu_wr_elem_size_arb_set             (l2_cpu2_wr_elem_size_arb_set[2:0]),
                    .l2_cpu_wr_err_arb_set                   (l2_cpu2_wr_err_arb_set),
                    .l2_cpu_wr_evict_x1_q                    (l2_cpu2_wr_evict_x1_q),
                    .l2_cpu_wr_id_arb_set                    (l2_cpu2_wr_id_arb_set[3:0]),
                    .l2_cpu_wr_last_arb_set                  (l2_cpu2_wr_last_arb_set),
                    .l2_cpu_wr_page_attr_arb_set             (l2_cpu2_wr_page_attr_arb_set[7:0]),
                    .l2_cpu_wr_partial_dw_arb_set            (l2_cpu2_wr_partial_dw_arb_set[3:0]),
                    .l2_cpu_wr_priv_arb_set                  (l2_cpu2_wr_priv_arb_set),
                    .l2_cpu_wr_shared_arb_set                (l2_cpu2_wr_shared_arb_set[1:0]),
                    .l2_cpu_wr_type_arb_set                  (l2_cpu2_wr_type_arb_set[2:0]),
                    .l2_cpu_wr_way_arb_set                   (l2_cpu2_wr_way_arb_set),
                    .l2_cpu_wrq_almost_full                  (l2_cpu2_wrq_almost_full),
                    .l2_cpu_wrq_haz_clr_id_dcd_q             (l2_cpu2_wrq_haz_clr_id_dcd_q[15:0]),
                    .ls_clrexmon                             (ls_cpu2_clrexmon),
                    .ls_imp_abort_containable                (ls_cpu2_imp_abort_containable),
                    .ls_imp_abort_dec                        (ls_cpu2_imp_abort_dec),
                    .ls_imp_abort_ecc                        (ls_cpu2_imp_abort_ecc),
                    .ls_imp_abort_slv                        (ls_cpu2_imp_abort_slv),
                    .ls_raw_eae_nonsec                       (ls_cpu2_raw_eae_nonsec),
                    .ls_raw_eae_secure                       (ls_cpu2_raw_eae_secure),
                    .ncommirq_cpu                            (ncommirq_cpu2_i),
                    .npmuirq_cpu                             (npmuirq_cpu2_i),
                    .pm_export_cpu                           (pm_export_cpu2_i),
                    .pmuevent_cpu                            (pmuevent_cpu2_i[24:0]),

                    // inputs
                    .aa64naa32_cpu                           (aa64naa32_cpu2_o),
                    .afvalidm_cpu                            (afvalidm_cpu2_o),
                    .atclken_cpu                             (atclken_cpu2_o),
                    .atreadym_cpu                            (atreadym_cpu2_o),
                    .cfgend_cpu                              (cfgend_cpu2_o),
                    .cfgte_cpu                               (cfgte_cpu2_o),
                    .ck_crcx_clk_en_n                        (ck_cpu2_crcx_clk_en_n),
                    .ck_event_reg                            (ck_cpu2_event_reg),
                    .ck_gclkt                                (ck_gclkt[2]),
                    .ck_wfe_ack                              (ck_cpu2_wfe_ack),
                    .ck_wfi_ack                              (ck_cpu2_wfi_ack),
                    .clusteridaff1_cpu                       (clusteridaff1_cpu2_o[7:0]),
                    .clusteridaff2_cpu                       (clusteridaff2_cpu2_o[7:0]),
                    .cp15sdisable_cpu                        (cp15sdisable_cpu2_o),
                    .cpuid                                   (cpuid_cpu2_o[1:0]),
                    .cryptodisable_cpu                       (cryptodisable_cpu2_o),
                    .dbgen_cpu                               (dbgen_cpu2_o),
                    .dbgl1rstdisable_cpu                     (dbgl1rstdisable_cpu2_o),
                    .dbgromaddr_cpu                          (dbgromaddr_cpu2_o[43:12]),
                    .dbgromaddrv_cpu                         (dbgromaddrv_cpu2_o),
                    .dftcrclkdisable_cpu                     (dftcrclkdisable_cpu2_o),
                    .dftramhold_cpu                          (dftramhold_cpu2_o),
                    .dftrstdisable_cpu                       (dftrstdisable_cpu2_o),
                    .dftse_cpu                               (dftse_cpu2_o),
                    .dt_cti_triginack_1to0_pclk              (dt_cpu2_cti_triginack_1to0_pclk[1:0]),
                    .dt_cti_triginack_7to4_pclk              (dt_cpu2_cti_triginack_7to4_pclk[3:0]),
                    .dt_cti_trigout_1to0_pclk                (dt_cpu2_cti_trigout_1to0_pclk[1:0]),
                    .dt_cti_trigout_7to4_pclk                (dt_cpu2_cti_trigout_7to4_pclk[3:0]),
                    .dt_dbif_addr_pclk                       (dt_cpu2_dbif_addr_pclk[14:2]),
                    .dt_dbif_locked_pclk                     (dt_cpu2_dbif_locked_pclk),
                    .dt_dbif_req_pclk                        (dt_cpu2_dbif_req_pclk),
                    .dt_dbif_wrdata_pclk                     (dt_cpu2_dbif_wrdata_pclk[31:0]),
                    .dt_dbif_write_pclk                      (dt_cpu2_dbif_write_pclk),
                    .dt_edacr_frc_idleack_pclk               (dt_cpu2_edacr_frc_idleack_pclk),
                    .dt_edbgrq_pclk                          (dt_cpu2_edbgrq_pclk),
                    .dt_edecr_osuce_pclk                     (dt_cpu2_edecr_osuce_pclk),
                    .dt_edecr_rce_pclk                       (dt_cpu2_edecr_rce_pclk),
                    .dt_edecr_ss_pclk                        (dt_cpu2_edecr_ss_pclk),
                    .dt_edprcr_corepurq_pclk                 (dt_cpu2_edprcr_corepurq_pclk),
                    .dt_noclkstop_pclk                       (dt_cpu2_noclkstop_pclk),
                    .dt_pmusnapshot_req_pclk                 (dt_cpu2_pmusnapshot_req_pclk),
                    .dt_wfx_wakeup_pclk                      (dt_cpu2_wfx_wakeup_pclk),
                    .giccdisable_cpu                         (giccdisable_cpu2_o),
                    .ic_block_eoi_sgi_wr                     (ic_block_eoi_sgi_wr[2]),
                    .ic_el_change_complete                   (ic_el_change_complete[2]),
                    .ic_hcr_change_complete                  (ic_hcr_change_complete[2]),
                    .ic_ich_el2_tall0                        (ic_ich_el2_tall0[2]),
                    .ic_ich_el2_tall1                        (ic_ich_el2_tall1[2]),
                    .ic_ich_el2_tc                           (ic_ich_el2_tc[2]),
                    .ic_nfiq                                 (ic_nfiq[2]),
                    .ic_nirq                                 (ic_nirq[2]),
                    .ic_nsei                                 (ic_nsei[2]),
                    .ic_nvfiq                                (ic_nvfiq[2]),
                    .ic_nvirq                                (ic_nvirq[2]),
                    .ic_nvsei                                (ic_nvsei[2]),
                    .ic_p_valid                              (ic_p_valid[2]),
                    .ic_sample_spr                           (ic_sample_spr[2]),
                    .ic_scr_change_complete                  (ic_scr_change_complete[2]),
                    .ic_sra_el1ns_en                         (ic_sra_el1ns_en[2]),
                    .ic_sra_el1s_en                          (ic_sra_el1s_en[2]),
                    .ic_sra_el2_en                           (ic_sra_el2_en[2]),
                    .ic_sra_el3_en                           (ic_sra_el3_en[2]),
                    .ic_sre_el1ns_hyp_trap                   (ic_sre_el1ns_hyp_trap[2]),
                    .ic_sre_el1ns_mon_trap                   (ic_sre_el1ns_mon_trap[2]),
                    .ic_sre_el1s_mon_trap                    (ic_sre_el1s_mon_trap[2]),
                    .ic_sre_el2_mon_trap                     (ic_sre_el2_mon_trap[2]),
                    .l2_cpu_arb_thrshld_timeout_en           (l2_cpu2_arb_thrshld_timeout_en),
                    .l2_cpu_barrier_done                     (l2_cpu2_barrier_done),
                    .l2_cpu_blk_non_evict_wr                 (l2_cpu2_blk_non_evict_wr),
                    .l2_cpu_ccb_dbg_req_c3                   (l2_cpu2_ccb_dbg_req_c3),
                    .l2_cpu_ccb_req_addr_c3                  (l2_cpu2_ccb_req_addr_c3[48:0]),
                    .l2_cpu_ccb_req_id_c3                    (l2_cpu2_ccb_req_id_c3[4:0]),
                    .l2_cpu_ccb_req_info_c3                  (l2_cpu2_ccb_req_info_c3[23:0]),
                    .l2_cpu_ccb_req_type_c3                  (l2_cpu2_ccb_req_type_c3[8:0]),
                    .l2_cpu_cfg_ecc_en                       (l2_cpu2_cfg_ecc_en),
                    .l2_cpu_dbufid_r1                        (l2_cpu2_dbufid_r1[2:0]),
                    .l2_cpu_ddata_r2                         (l2_cpu2_ddata_r2[129:0]),
                    .l2_cpu_ddbl_ecc_err_r3                  (l2_cpu2_ddlb_ecc_err_r3),
                    .l2_cpu_dext_err_r2                      (l2_cpu2_dext_err_r2),
                    .l2_cpu_dext_err_type_r2                 (l2_cpu2_dext_err_type_r2),
                    .l2_cpu_disable_clean_evict_opt          (l2_cpu2_disable_clean_evict_opt),
                    .l2_cpu_dlast_r1                         (l2_cpu2_dlast_r1),
                    .l2_cpu_dsngl_ecc_err_r3                 (l2_cpu2_dsngl_ecc_err_r3),
                    .l2_cpu_dsq_clr_id_q                     (l2_cpu2_dsq_clr_id_q[3:0]),
                    .l2_cpu_dsq_clr_vld_q                    (l2_cpu2_dsq_clr_vld_q),
                    .l2_cpu_dsq_rd_buf_id                    (l2_cpu2_dsq_rd_buf_id[3:0]),
                    .l2_cpu_dsq_rd_en                        (l2_cpu2_dsq_rd_en),
                    .l2_cpu_dsq_rd_en_x2                     (l2_cpu2_dsq_rd_en_x2),
                    .l2_cpu_dvalid_r1                        (l2_cpu2_dvalid_r1),
                    .l2_cpu_flsh_if_rd_id_l4_dly             (l2_cpu2_flsh_if_rd_id_l4_dly[1:0]),
                    .l2_cpu_flsh_if_rd_l4_dly                (l2_cpu2_flsh_if_rd_l4_dly),
                    .l2_cpu_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_rd_id_l2_dly             (l2_cpu2_flsh_ls_rd_id_l2_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_id_l4_dly             (l2_cpu2_flsh_ls_rd_id_l4_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_l2_dly                (l2_cpu2_flsh_ls_rd_l2_dly),
                    .l2_cpu_flsh_ls_rd_l4_dly                (l2_cpu2_flsh_ls_rd_l4_dly),
                    .l2_cpu_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_wr_evict_l4_dly          (l2_cpu2_flsh_ls_wr_evict_l4_dly),
                    .l2_cpu_flsh_ls_wr_id_l2_dly             (l2_cpu2_flsh_ls_wr_id_l2_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_id_l4_dly             (l2_cpu2_flsh_ls_wr_id_l4_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_l2_dly                (l2_cpu2_flsh_ls_wr_l2_dly),
                    .l2_cpu_flsh_ls_wr_l4_dly                (l2_cpu2_flsh_ls_wr_l4_dly),
                    .l2_cpu_flsh_tw_rd_l4_dly                (l2_cpu2_flsh_tw_rd_l4_dly),
                    .l2_cpu_ibufid_r1                        (l2_cpu2_ibufid_r1[1:0]),
                    .l2_cpu_ic_barrier_stall_q               (l2_cpu2_ic_barrier_stall_q),
                    .l2_cpu_ic_base                          (l2_cpu2_ic_base[43:18]),
                    .l2_cpu_ic_vld_skid                      (l2_cpu2_ic_vld_skid),
                    .l2_cpu_idata_r2                         (l2_cpu2_idata_r2[127:0]),
                    .l2_cpu_idbl_ecc_err_r3                  (l2_cpu2_idlb_ecc_err_r3),
                    .l2_cpu_idle_block_reqs_q                (l2_cpu2_idle_block_reqs_q),
                    .l2_cpu_iext_err_r2                      (l2_cpu2_iext_err_r2),
                    .l2_cpu_iext_err_type_r2                 (l2_cpu2_iext_err_type_r2),
                    .l2_cpu_if_ccb_clken_c3                  (l2_cpu2_if_ccb_clken_c3),
                    .l2_cpu_if_ccb_req_c3                    (l2_cpu2_if_ccb_req_c3),
                    .l2_cpu_if_sync_req                      (l2_cpu2_if_sync_req),
                    .l2_cpu_ifq_haz_pending                  (l2_cpu2_ifq_haz_pending),
                    .l2_cpu_isngl_ecc_err_r3                 (l2_cpu2_isngl_ecc_err_r3),
                    .l2_cpu_ivalid_r1                        (l2_cpu2_ivalid_r1),
                    .l2_cpu_l2_cache_size                    (l2_cpu2_l2_cache_size[1:0]),
                    .l2_cpu_lrq_haz_pending                  (l2_cpu2_lrq_haz_pending),
                    .l2_cpu_ls_ccb_clken_c3                  (l2_cpu2_ls_ccb_clken_c3),
                    .l2_cpu_ls_ccb_req_c3                    (l2_cpu2_ls_ccb_req_c3),
                    .l2_cpu_ls_peq_coll_l4_dly               (l2_cpu2_ls_peq_coll_l4_dly),
                    .l2_cpu_ls_sync_req                      (l2_cpu2_ls_sync_req),
                    .l2_cpu_mbist1_addr_b1                   (l2_cpu2_mbist1_addr_b1[12:0]),
                    .l2_cpu_mbist1_all_b1                    (l2_cpu2_mbist1_all_b1),
                    .l2_cpu_mbist1_array_b1                  (l2_cpu2_mbist1_array_b1[3:0]),
                    .l2_cpu_mbist1_be_b1                     (l2_cpu2_mbist1_be_b1[7:0]),
                    .l2_cpu_mbist1_en_b1                     (l2_cpu2_mbist1_en_b1),
                    .l2_cpu_mbist1_rd_en_b1                  (l2_cpu2_mbist1_rd_en_b1),
                    .l2_cpu_mbist1_wr_en_b1                  (l2_cpu2_mbist1_wr_en_b1),
                    .l2_cpu_no_intctrl                       (l2_cpu2_no_intctrl),
                    .l2_cpu_pf_rd_vld_skid_popped            (l2_cpu2_pf_rd_vld_skid_popped),
                    .l2_cpu_pf_throttle_q                    (l2_cpu2_pf_throttle_q),
                    .l2_cpu_pmu_events                       (l2_cpu2_pmu_events[33:0]),
                    .l2_cpu_rbufid                           (l2_cpu2_rbufid[2:0]),
                    .l2_cpu_rd_arb                           (l2_cpu2_rd_arb),
                    .l2_cpu_rd_vld_skid                      (l2_cpu2_rd_vld_skid),
                    .l2_cpu_rexfail                          (l2_cpu2_rexfail),
                    .l2_cpu_rstate                           (l2_cpu2_rstate[1:0]),
                    .l2_cpu_rvalid                           (l2_cpu2_rvalid),
                    .l2_cpu_spec_bufid                       (l2_cpu2_spec_bufid[2:0]),
                    .l2_cpu_spec_valid                       (l2_cpu2_spec_valid),
                    .l2_cpu_spr_rd_data                      (l2_cpu2_spr_rd_data[63:0]),
                    .l2_cpu_tbw_dbl_ecc_err                  (l2_cpu2_tbw_dbl_ecc_err),
                    .l2_cpu_tbw_desc_data                    (l2_cpu2_tbw_desc_data[63:0]),
                    .l2_cpu_tbw_desc_vld                     (l2_cpu2_tbw_desc_vld),
                    .l2_cpu_tbw_ext_err                      (l2_cpu2_tbw_ext_err),
                    .l2_cpu_tbw_ext_err_type                 (l2_cpu2_tbw_ext_err_type),
                    .l2_cpu_tlb_ccb_clken_c3                 (l2_cpu2_tlb_ccb_clken_c3),
                    .l2_cpu_tlb_ccb_req_c3                   (l2_cpu2_tlb_ccb_req_c3),
                    .l2_cpu_tlb_sync_complete                (l2_cpu2_tlb_sync_complete),
                    .l2_cpu_tlb_sync_req                     (l2_cpu2_tlb_sync_req),
                    .l2_cpu_trq_haz_pending                  (l2_cpu2_trq_haz_pending),
                    .l2_cpu_wr_arb                           (l2_cpu2_wr_arb),
                    .l2_cpu_wr_data_stall                    (l2_cpu2_wr_data_stall),
                    .l2_cpu_wr_ex_fail                       (l2_cpu2_wr_ex_fail),
                    .l2_cpu_wr_ex_resp                       (l2_cpu2_wr_ex_resp),
                    .l2_cpu_wr_vld_skid                      (l2_cpu2_wr_vld_skid),
                    .l2_cpu_wrq_haz_pending                  (l2_cpu2_wrq_haz_pending),
                    .l2_tbnk0_cpu_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu2_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk0_cpu_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu2_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk0_cpu_trq_clr_l4_dly2_q          (l2_tbnk0_cpu2_trq_clr_l4_dly2_q),
                    .l2_tbnk0_cpu_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu2_wrq_clr_l4_dly2_q[5:0]),
                    .l2_tbnk1_cpu_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu2_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk1_cpu_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu2_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk1_cpu_trq_clr_l4_dly2_q          (l2_tbnk1_cpu2_trq_clr_l4_dly2_q),
                    .l2_tbnk1_cpu_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu2_wrq_clr_l4_dly2_q[5:0]),
                    .ncorereset_cpu                          (ncorereset_cpu2_o),
                    .ncpuporeset_cpu                         (ncpuporeset_cpu2_o),
                    .niden_cpu                               (niden_cpu2_o),
                    .nmbistreset_cpu                         (nmbistreset_cpu2_o),
                    .rvbaraddr_cpu                           (rvbaraddr_cpu2_o[43:2]),
                    .spiden_cpu                              (spiden_cpu2_o),
                    .spniden_cpu                             (spniden_cpu2_o),
                    .syncreqm_cpu                            (syncreqm_cpu2_o),
                    .tm_cnthctl_kernel                       (tm_cpu2_cnthctl_kernel[1:0]),
                    .tm_cntkctl_usr                          (tm_cpu2_cntkctl_usr[3:0]),
                    .tsvalueb_cpu                            (tsvalueb_cpu2_o[63:0]),
                    .vinithi_cpu                             (vinithi_cpu2_o)
                  );    // ucpu2

    maia_cpu ucpu3( // outputs
                    .afreadym_cpu                            (afreadym_cpu3_i),
                    .atbytesm_cpu                            (atbytesm_cpu3_i[1:0]),
                    .atdatam_cpu                             (atdatam_cpu3_i[31:0]),
                    .atidm_cpu                               (atidm_cpu3_i[6:0]),
                    .atvalidm_cpu                            (atvalidm_cpu3_i),
                    .commrx_cpu                              (commrx_cpu3_i),
                    .commtx_cpu                              (commtx_cpu3_i),
                    .dbgack_cpu                              (dbgack_cpu3_i),
                    .dbgnopwrdwn_cpu                         (dbgnopwrdwn_cpu3_i),
                    .dbgrstreq_cpu                           (dbgrstreq_cpu3_i),
                    .ds_cpuectlr_ret                         (ds_cpu3_cpuectlr_ret[2:0]),
                    .ds_cpuectlr_smp                         (ds_cpu3_cpuectlr_smp),
                    .ds_fiq_wfe_qual                         (ds_cpu3_fiq_wfe_qual),
                    .ds_fiq_wfi_qual                         (ds_cpu3_fiq_wfi_qual),
                    .ds_flush                                (ds_cpu3_flush),
                    .ds_flush_type                           (ds_cpu3_flush_type[5:0]),
                    .ds_hcr_va                               (ds_cpu3_hcr_va),
                    .ds_hcr_vf                               (ds_cpu3_hcr_vf),
                    .ds_hcr_vi                               (ds_cpu3_hcr_vi),
                    .ds_ic_aa64naa32                         (ds_cpu3_ic_aa64naa32),
                    .ds_ic_cpsr_mode                         (ds_cpu3_ic_cpsr_mode[4:0]),
                    .ds_ic_hcr_change                        (ds_cpu3_ic_hcr_change),
                    .ds_ic_sample_spr                        (ds_cpu3_ic_sample_spr),
                    .ds_ic_scr_change                        (ds_cpu3_ic_scr_change),
                    .ds_imp_abrt_wfe_qual                    (ds_cpu3_imp_abrt_wfe_qual),
                    .ds_imp_abrt_wfi_qual                    (ds_cpu3_imp_abrt_wfi_qual),
                    .ds_irq_wfe_qual                         (ds_cpu3_irq_wfe_qual),
                    .ds_irq_wfi_qual                         (ds_cpu3_irq_wfi_qual),
                    .ds_l2_spr_addr                          (ds_cpu3_l2_spr_addr[8:0]),
                    .ds_l2_spr_dw                            (ds_cpu3_l2_spr_dw),
                    .ds_l2_spr_en                            (ds_cpu3_l2_spr_en),
                    .ds_l2_spr_rd                            (ds_cpu3_l2_spr_rd),
                    .ds_l2_spr_wr                            (ds_cpu3_l2_spr_wr),
                    .ds_l2_spr_wr_data                       (ds_cpu3_l2_spr_wr_data[63:0]),
                    .ds_reset_req                            (ds_cpu3_reset_req),
                    .ds_sev_req                              (ds_cpu3_sev_req),
                    .ds_sevl_req                             (ds_cpu3_sevl_req),
                    .ds_vfiq_wfe_qual                        (ds_cpu3_vfiq_wfe_qual),
                    .ds_vfiq_wfi_qual                        (ds_cpu3_vfiq_wfi_qual),
                    .ds_vimp_abrt_wfe_qual                   (ds_cpu3_vimp_abrt_wfe_qual),
                    .ds_vimp_abrt_wfi_qual                   (ds_cpu3_vimp_abrt_wfi_qual),
                    .ds_virq_wfe_qual                        (ds_cpu3_virq_wfe_qual),
                    .ds_virq_wfi_qual                        (ds_cpu3_virq_wfi_qual),
                    .ds_wfe_req                              (ds_cpu3_wfe_req),
                    .ds_wfi_req                              (ds_cpu3_wfi_req),
                    .dt_coredbg_in_reset_gclk                (dt_cpu3_coredbg_in_reset_gclk),
                    .dt_cti_trigin_1to0_gclk                 (dt_cpu3_cti_trigin_1to0_gclk[1:0]),
                    .dt_cti_trigin_7to4_gclk                 (dt_cpu3_cti_trigin_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_7to4_gclk             (dt_cpu3_cti_trigoutack_7to4_gclk[3:0]),
                    .dt_cti_trigoutack_bit1_gclk             (dt_cpu3_cti_trigoutack_bit1_gclk),
                    .dt_dbif_ack_gclk                        (dt_cpu3_dbif_ack_gclk),
                    .dt_dbif_err_gclk                        (dt_cpu3_dbif_err_gclk),
                    .dt_dbif_rddata_gclk                     (dt_cpu3_dbif_rddata_gclk[31:0]),
                    .dt_et_oslock_gclk                       (dt_cpu3_et_oslock_gclk),
                    .dt_halt_ack_gclk                        (dt_cpu3_halt_ack_gclk),
                    .dt_hlt_dbgevt_ok_gclk                   (dt_cpu3_hlt_dbgevt_ok_gclk),
                    .dt_os_double_lock_gclk                  (dt_cpu3_os_double_lock_gclk),
                    .dt_pmusnapshot_ack_gclk                 (dt_cpu3_pmusnapshot_ack_gclk),
                    .dt_wfx_dbg_req_gclk                     (dt_cpu3_wfx_dbg_req_gclk),
                    .etclken_cpu                             (etclken_cpu3_i),
                    .l2_cpu_dsq_rd_byte_strb_q               (l2_cpu3_dsq_rd_byte_strb_q[15:0]),
                    .l2_cpu_dsq_rd_data_q                    (l2_cpu3_dsq_rd_data_q[129:0]),
                    .l2_cpu_dt_pmu_evt_en                    (l2_cpu3_dt_pmu_evt_en),
                    .l2_cpu_early_rd_reqe4_e5_q              (l2_cpu3_early_rd_reqe4_e5_q),
                    .l2_cpu_ic_addr_arb_set                  (l2_cpu3_ic_addr_arb_set[15:0]),
                    .l2_cpu_ic_arb_fast                      (l2_cpu3_ic_arb_fast),
                    .l2_cpu_ic_data_arb_set                  (l2_cpu3_ic_data_arb_set[31:0]),
                    .l2_cpu_ic_elem_size_arb_set             (l2_cpu3_ic_elem_size_arb_set[2:0]),
                    .l2_cpu_ic_excl_arb_set                  (l2_cpu3_ic_excl_arb_set),
                    .l2_cpu_ic_id_arb_set                    (l2_cpu3_ic_id_arb_set[2:0]),
                    .l2_cpu_ic_ns_arb_set                    (l2_cpu3_ic_ns_arb_set),
                    .l2_cpu_ic_write_arb_set                 (l2_cpu3_ic_write_arb_set),
                    .l2_cpu_idle_wakeup_q                    (l2_cpu3_idle_wakeup_q),
                    .l2_cpu_if_ccb_resp                      (l2_cpu3_if_ccb_resp),
                    .l2_cpu_if_ccb_resp_id                   (l2_cpu3_if_ccb_resp_id[4:0]),
                    .l2_cpu_if_sync_done_q                   (l2_cpu3_if_sync_done_q),
                    .l2_cpu_lrq_haz_clr_id_dcd_q             (l2_cpu3_lrq_haz_clr_id_dcd_q[5:0]),
                    .l2_cpu_ls_ccb_data_wr                   (l2_cpu3_ls_ccb_data_wr),
                    .l2_cpu_ls_ccb_resp                      (l2_cpu3_ls_ccb_resp),
                    .l2_cpu_ls_ccb_resp_id                   (l2_cpu3_ls_ccb_resp_id[4:0]),
                    .l2_cpu_ls_rd_haz_id_arb_q               (l2_cpu3_ls_rd_haz_id_arb_q[3:0]),
                    .l2_cpu_ls_rd_haz_vld_arb_q              (l2_cpu3_ls_rd_haz_vld_arb_q),
                    .l2_cpu_ls_wr_ccb_id_w2a                 (l2_cpu3_ls_wr_ccb_id_w2a[4:0]),
                    .l2_cpu_ls_wr_data_w2a                   (l2_cpu3_ls_wr_data_w2a[127:0]),
                    .l2_cpu_ls_wr_dirty_w2a                  (l2_cpu3_ls_wr_dirty_w2a),
                    .l2_cpu_ls_wr_err_w2a                    (l2_cpu3_ls_wr_err_w2a),
                    .l2_cpu_ls_wr_haz_id_arb_q               (l2_cpu3_ls_wr_haz_id_arb_q[2:0]),
                    .l2_cpu_ls_wr_haz_vld_arb_q              (l2_cpu3_ls_wr_haz_vld_arb_q),
                    .l2_cpu_ls_wr_last_w2a                   (l2_cpu3_ls_wr_last_w2a),
                    .l2_cpu_ls_wr_req_w2a                    (l2_cpu3_ls_wr_req_w2a),
                    .l2_cpu_ls_wr_type_w2a                   (l2_cpu3_ls_wr_type_w2a[2:0]),
                    .l2_cpu_rd_aarch64_arb_set               (l2_cpu3_rd_aarch64_arb_set),
                    .l2_cpu_rd_addr_arb_set                  (l2_cpu3_rd_addr_arb_set[44:0]),
                    .l2_cpu_rd_arb_fast                      (l2_cpu3_rd_arb_fast),
                    .l2_cpu_rd_asid_arb_set                  (l2_cpu3_rd_asid_arb_set[15:8]),
                    .l2_cpu_rd_bypass_arb_set                (l2_cpu3_rd_bypass_arb_set),
                    .l2_cpu_rd_bypass_bufid_e5               (l2_cpu3_rd_bypass_bufid_e5[2:0]),
                    .l2_cpu_rd_bypass_lrq_id_e5              (l2_cpu3_rd_bypass_lrq_id_e5[2:0]),
                    .l2_cpu_rd_bypass_req_can_e5             (l2_cpu3_rd_bypass_req_can_e5),
                    .l2_cpu_rd_bypass_way_e5                 (l2_cpu3_rd_bypass_way_e5),
                    .l2_cpu_rd_cache_attr_arb_set            (l2_cpu3_rd_cache_attr_arb_set[2:0]),
                    .l2_cpu_rd_elem_size_arb_set             (l2_cpu3_rd_elem_size_arb_set[2:0]),
                    .l2_cpu_rd_excl_arb_set                  (l2_cpu3_rd_excl_arb_set),
                    .l2_cpu_rd_id_arb_set                    (l2_cpu3_rd_id_arb_set[4:0]),
                    .l2_cpu_rd_lrq_id_arb_set                (l2_cpu3_rd_lrq_id_arb_set[2:0]),
                    .l2_cpu_rd_page_attr_arb_set             (l2_cpu3_rd_page_attr_arb_set[7:0]),
                    .l2_cpu_rd_prfm_arb_set                  (l2_cpu3_rd_prfm_arb_set),
                    .l2_cpu_rd_priv_arb_set                  (l2_cpu3_rd_priv_arb_set),
                    .l2_cpu_rd_replayed_arb_set              (l2_cpu3_rd_replayed_arb_set),
                    .l2_cpu_rd_shared_arb_set                (l2_cpu3_rd_shared_arb_set[1:0]),
                    .l2_cpu_rd_type_arb_set                  (l2_cpu3_rd_type_arb_set[6:0]),
                    .l2_cpu_rd_va48_arb_set                  (l2_cpu3_rd_va48_arb_set),
                    .l2_cpu_rd_way_arb_set                   (l2_cpu3_rd_way_arb_set),
                    .l2_cpu_tlb_sync_done_q                  (l2_cpu3_tlb_sync_done_q),
                    .l2_cpu_tw_ccb_resp                      (l2_cpu3_tw_ccb_resp),
                    .l2_cpu_tw_ccb_resp_id                   (l2_cpu3_tw_ccb_resp_id[4:0]),
                    .l2_cpu_wr_1st_replayed_arb_set          (l2_cpu3_wr_1st_replayed_arb_set),
                    .l2_cpu_wr_addr_arb_set                  (l2_cpu3_wr_addr_arb_set[44:0]),
                    .l2_cpu_wr_arb_fast                      (l2_cpu3_wr_arb_fast),
                    .l2_cpu_wr_cache_attr_arb_set            (l2_cpu3_wr_cache_attr_arb_set[2:0]),
                    .l2_cpu_wr_cl_id_arb_set                 (l2_cpu3_wr_cl_id_arb_set[11:0]),
                    .l2_cpu_wr_clean_evict_arb_set           (l2_cpu3_wr_clean_evict_arb_set),
                    .l2_cpu_wr_data                          (l2_cpu3_wr_data[143:0]),
                    .l2_cpu_wr_data_vld_x1_q                 (l2_cpu3_wr_data_vld_x1_q),
                    .l2_cpu_wr_dirty_arb_set                 (l2_cpu3_wr_dirty_arb_set),
                    .l2_cpu_wr_elem_size_arb_set             (l2_cpu3_wr_elem_size_arb_set[2:0]),
                    .l2_cpu_wr_err_arb_set                   (l2_cpu3_wr_err_arb_set),
                    .l2_cpu_wr_evict_x1_q                    (l2_cpu3_wr_evict_x1_q),
                    .l2_cpu_wr_id_arb_set                    (l2_cpu3_wr_id_arb_set[3:0]),
                    .l2_cpu_wr_last_arb_set                  (l2_cpu3_wr_last_arb_set),
                    .l2_cpu_wr_page_attr_arb_set             (l2_cpu3_wr_page_attr_arb_set[7:0]),
                    .l2_cpu_wr_partial_dw_arb_set            (l2_cpu3_wr_partial_dw_arb_set[3:0]),
                    .l2_cpu_wr_priv_arb_set                  (l2_cpu3_wr_priv_arb_set),
                    .l2_cpu_wr_shared_arb_set                (l2_cpu3_wr_shared_arb_set[1:0]),
                    .l2_cpu_wr_type_arb_set                  (l2_cpu3_wr_type_arb_set[2:0]),
                    .l2_cpu_wr_way_arb_set                   (l2_cpu3_wr_way_arb_set),
                    .l2_cpu_wrq_almost_full                  (l2_cpu3_wrq_almost_full),
                    .l2_cpu_wrq_haz_clr_id_dcd_q             (l2_cpu3_wrq_haz_clr_id_dcd_q[15:0]),
                    .ls_clrexmon                             (ls_cpu3_clrexmon),
                    .ls_imp_abort_containable                (ls_cpu3_imp_abort_containable),
                    .ls_imp_abort_dec                        (ls_cpu3_imp_abort_dec),
                    .ls_imp_abort_ecc                        (ls_cpu3_imp_abort_ecc),
                    .ls_imp_abort_slv                        (ls_cpu3_imp_abort_slv),
                    .ls_raw_eae_nonsec                       (ls_cpu3_raw_eae_nonsec),
                    .ls_raw_eae_secure                       (ls_cpu3_raw_eae_secure),
                    .ncommirq_cpu                            (ncommirq_cpu3_i),
                    .npmuirq_cpu                             (npmuirq_cpu3_i),
                    .pm_export_cpu                           (pm_export_cpu3_i),
                    .pmuevent_cpu                            (pmuevent_cpu3_i[24:0]),

                    // inputs
                    .aa64naa32_cpu                           (aa64naa32_cpu3_o),
                    .afvalidm_cpu                            (afvalidm_cpu3_o),
                    .atclken_cpu                             (atclken_cpu3_o),
                    .atreadym_cpu                            (atreadym_cpu3_o),
                    .cfgend_cpu                              (cfgend_cpu3_o),
                    .cfgte_cpu                               (cfgte_cpu3_o),
                    .ck_crcx_clk_en_n                        (ck_cpu3_crcx_clk_en_n),
                    .ck_event_reg                            (ck_cpu3_event_reg),
                    .ck_gclkt                                (ck_gclkt[3]),
                    .ck_wfe_ack                              (ck_cpu3_wfe_ack),
                    .ck_wfi_ack                              (ck_cpu3_wfi_ack),
                    .clusteridaff1_cpu                       (clusteridaff1_cpu3_o[7:0]),
                    .clusteridaff2_cpu                       (clusteridaff2_cpu3_o[7:0]),
                    .cp15sdisable_cpu                        (cp15sdisable_cpu3_o),
                    .cpuid                                   (cpuid_cpu3_o[1:0]),
                    .cryptodisable_cpu                       (cryptodisable_cpu3_o),
                    .dbgen_cpu                               (dbgen_cpu3_o),
                    .dbgl1rstdisable_cpu                     (dbgl1rstdisable_cpu3_o),
                    .dbgromaddr_cpu                          (dbgromaddr_cpu3_o[43:12]),
                    .dbgromaddrv_cpu                         (dbgromaddrv_cpu3_o),
                    .dftcrclkdisable_cpu                     (dftcrclkdisable_cpu3_o),
                    .dftramhold_cpu                          (dftramhold_cpu3_o),
                    .dftrstdisable_cpu                       (dftrstdisable_cpu3_o),
                    .dftse_cpu                               (dftse_cpu3_o),
                    .dt_cti_triginack_1to0_pclk              (dt_cpu3_cti_triginack_1to0_pclk[1:0]),
                    .dt_cti_triginack_7to4_pclk              (dt_cpu3_cti_triginack_7to4_pclk[3:0]),
                    .dt_cti_trigout_1to0_pclk                (dt_cpu3_cti_trigout_1to0_pclk[1:0]),
                    .dt_cti_trigout_7to4_pclk                (dt_cpu3_cti_trigout_7to4_pclk[3:0]),
                    .dt_dbif_addr_pclk                       (dt_cpu3_dbif_addr_pclk[14:2]),
                    .dt_dbif_locked_pclk                     (dt_cpu3_dbif_locked_pclk),
                    .dt_dbif_req_pclk                        (dt_cpu3_dbif_req_pclk),
                    .dt_dbif_wrdata_pclk                     (dt_cpu3_dbif_wrdata_pclk[31:0]),
                    .dt_dbif_write_pclk                      (dt_cpu3_dbif_write_pclk),
                    .dt_edacr_frc_idleack_pclk               (dt_cpu3_edacr_frc_idleack_pclk),
                    .dt_edbgrq_pclk                          (dt_cpu3_edbgrq_pclk),
                    .dt_edecr_osuce_pclk                     (dt_cpu3_edecr_osuce_pclk),
                    .dt_edecr_rce_pclk                       (dt_cpu3_edecr_rce_pclk),
                    .dt_edecr_ss_pclk                        (dt_cpu3_edecr_ss_pclk),
                    .dt_edprcr_corepurq_pclk                 (dt_cpu3_edprcr_corepurq_pclk),
                    .dt_noclkstop_pclk                       (dt_cpu3_noclkstop_pclk),
                    .dt_pmusnapshot_req_pclk                 (dt_cpu3_pmusnapshot_req_pclk),
                    .dt_wfx_wakeup_pclk                      (dt_cpu3_wfx_wakeup_pclk),
                    .giccdisable_cpu                         (giccdisable_cpu3_o),
                    .ic_block_eoi_sgi_wr                     (ic_block_eoi_sgi_wr[3]),
                    .ic_el_change_complete                   (ic_el_change_complete[3]),
                    .ic_hcr_change_complete                  (ic_hcr_change_complete[3]),
                    .ic_ich_el2_tall0                        (ic_ich_el2_tall0[3]),
                    .ic_ich_el2_tall1                        (ic_ich_el2_tall1[3]),
                    .ic_ich_el2_tc                           (ic_ich_el2_tc[3]),
                    .ic_nfiq                                 (ic_nfiq[3]),
                    .ic_nirq                                 (ic_nirq[3]),
                    .ic_nsei                                 (ic_nsei[3]),
                    .ic_nvfiq                                (ic_nvfiq[3]),
                    .ic_nvirq                                (ic_nvirq[3]),
                    .ic_nvsei                                (ic_nvsei[3]),
                    .ic_p_valid                              (ic_p_valid[3]),
                    .ic_sample_spr                           (ic_sample_spr[3]),
                    .ic_scr_change_complete                  (ic_scr_change_complete[3]),
                    .ic_sra_el1ns_en                         (ic_sra_el1ns_en[3]),
                    .ic_sra_el1s_en                          (ic_sra_el1s_en[3]),
                    .ic_sra_el2_en                           (ic_sra_el2_en[3]),
                    .ic_sra_el3_en                           (ic_sra_el3_en[3]),
                    .ic_sre_el1ns_hyp_trap                   (ic_sre_el1ns_hyp_trap[3]),
                    .ic_sre_el1ns_mon_trap                   (ic_sre_el1ns_mon_trap[3]),
                    .ic_sre_el1s_mon_trap                    (ic_sre_el1s_mon_trap[3]),
                    .ic_sre_el2_mon_trap                     (ic_sre_el2_mon_trap[3]),
                    .l2_cpu_arb_thrshld_timeout_en           (l2_cpu3_arb_thrshld_timeout_en),
                    .l2_cpu_barrier_done                     (l2_cpu3_barrier_done),
                    .l2_cpu_blk_non_evict_wr                 (l2_cpu3_blk_non_evict_wr),
                    .l2_cpu_ccb_dbg_req_c3                   (l2_cpu3_ccb_dbg_req_c3),
                    .l2_cpu_ccb_req_addr_c3                  (l2_cpu3_ccb_req_addr_c3[48:0]),
                    .l2_cpu_ccb_req_id_c3                    (l2_cpu3_ccb_req_id_c3[4:0]),
                    .l2_cpu_ccb_req_info_c3                  (l2_cpu3_ccb_req_info_c3[23:0]),
                    .l2_cpu_ccb_req_type_c3                  (l2_cpu3_ccb_req_type_c3[8:0]),
                    .l2_cpu_cfg_ecc_en                       (l2_cpu3_cfg_ecc_en),
                    .l2_cpu_dbufid_r1                        (l2_cpu3_dbufid_r1[2:0]),
                    .l2_cpu_ddata_r2                         (l2_cpu3_ddata_r2[129:0]),
                    .l2_cpu_ddbl_ecc_err_r3                  (l2_cpu3_ddlb_ecc_err_r3),
                    .l2_cpu_dext_err_r2                      (l2_cpu3_dext_err_r2),
                    .l2_cpu_dext_err_type_r2                 (l2_cpu3_dext_err_type_r2),
                    .l2_cpu_disable_clean_evict_opt          (l2_cpu3_disable_clean_evict_opt),
                    .l2_cpu_dlast_r1                         (l2_cpu3_dlast_r1),
                    .l2_cpu_dsngl_ecc_err_r3                 (l2_cpu3_dsngl_ecc_err_r3),
                    .l2_cpu_dsq_clr_id_q                     (l2_cpu3_dsq_clr_id_q[3:0]),
                    .l2_cpu_dsq_clr_vld_q                    (l2_cpu3_dsq_clr_vld_q),
                    .l2_cpu_dsq_rd_buf_id                    (l2_cpu3_dsq_rd_buf_id[3:0]),
                    .l2_cpu_dsq_rd_en                        (l2_cpu3_dsq_rd_en),
                    .l2_cpu_dsq_rd_en_x2                     (l2_cpu3_dsq_rd_en_x2),
                    .l2_cpu_dvalid_r1                        (l2_cpu3_dvalid_r1),
                    .l2_cpu_flsh_if_rd_id_l4_dly             (l2_cpu3_flsh_if_rd_id_l4_dly[1:0]),
                    .l2_cpu_flsh_if_rd_l4_dly                (l2_cpu3_flsh_if_rd_l4_dly),
                    .l2_cpu_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_rd_id_l2_dly             (l2_cpu3_flsh_ls_rd_id_l2_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_id_l4_dly             (l2_cpu3_flsh_ls_rd_id_l4_dly[2:0]),
                    .l2_cpu_flsh_ls_rd_l2_dly                (l2_cpu3_flsh_ls_rd_l2_dly),
                    .l2_cpu_flsh_ls_rd_l4_dly                (l2_cpu3_flsh_ls_rd_l4_dly),
                    .l2_cpu_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly),
                    .l2_cpu_flsh_ls_wr_evict_l4_dly          (l2_cpu3_flsh_ls_wr_evict_l4_dly),
                    .l2_cpu_flsh_ls_wr_id_l2_dly             (l2_cpu3_flsh_ls_wr_id_l2_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_id_l4_dly             (l2_cpu3_flsh_ls_wr_id_l4_dly[3:0]),
                    .l2_cpu_flsh_ls_wr_l2_dly                (l2_cpu3_flsh_ls_wr_l2_dly),
                    .l2_cpu_flsh_ls_wr_l4_dly                (l2_cpu3_flsh_ls_wr_l4_dly),
                    .l2_cpu_flsh_tw_rd_l4_dly                (l2_cpu3_flsh_tw_rd_l4_dly),
                    .l2_cpu_ibufid_r1                        (l2_cpu3_ibufid_r1[1:0]),
                    .l2_cpu_ic_barrier_stall_q               (l2_cpu3_ic_barrier_stall_q),
                    .l2_cpu_ic_base                          (l2_cpu3_ic_base[43:18]),
                    .l2_cpu_ic_vld_skid                      (l2_cpu3_ic_vld_skid),
                    .l2_cpu_idata_r2                         (l2_cpu3_idata_r2[127:0]),
                    .l2_cpu_idbl_ecc_err_r3                  (l2_cpu3_idlb_ecc_err_r3),
                    .l2_cpu_idle_block_reqs_q                (l2_cpu3_idle_block_reqs_q),
                    .l2_cpu_iext_err_r2                      (l2_cpu3_iext_err_r2),
                    .l2_cpu_iext_err_type_r2                 (l2_cpu3_iext_err_type_r2),
                    .l2_cpu_if_ccb_clken_c3                  (l2_cpu3_if_ccb_clken_c3),
                    .l2_cpu_if_ccb_req_c3                    (l2_cpu3_if_ccb_req_c3),
                    .l2_cpu_if_sync_req                      (l2_cpu3_if_sync_req),
                    .l2_cpu_ifq_haz_pending                  (l2_cpu3_ifq_haz_pending),
                    .l2_cpu_isngl_ecc_err_r3                 (l2_cpu3_isngl_ecc_err_r3),
                    .l2_cpu_ivalid_r1                        (l2_cpu3_ivalid_r1),
                    .l2_cpu_l2_cache_size                    (l2_cpu3_l2_cache_size[1:0]),
                    .l2_cpu_lrq_haz_pending                  (l2_cpu3_lrq_haz_pending),
                    .l2_cpu_ls_ccb_clken_c3                  (l2_cpu3_ls_ccb_clken_c3),
                    .l2_cpu_ls_ccb_req_c3                    (l2_cpu3_ls_ccb_req_c3),
                    .l2_cpu_ls_peq_coll_l4_dly               (l2_cpu3_ls_peq_coll_l4_dly),
                    .l2_cpu_ls_sync_req                      (l2_cpu3_ls_sync_req),
                    .l2_cpu_mbist1_addr_b1                   (l2_cpu3_mbist1_addr_b1[12:0]),
                    .l2_cpu_mbist1_all_b1                    (l2_cpu3_mbist1_all_b1),
                    .l2_cpu_mbist1_array_b1                  (l2_cpu3_mbist1_array_b1[3:0]),
                    .l2_cpu_mbist1_be_b1                     (l2_cpu3_mbist1_be_b1[7:0]),
                    .l2_cpu_mbist1_en_b1                     (l2_cpu3_mbist1_en_b1),
                    .l2_cpu_mbist1_rd_en_b1                  (l2_cpu3_mbist1_rd_en_b1),
                    .l2_cpu_mbist1_wr_en_b1                  (l2_cpu3_mbist1_wr_en_b1),
                    .l2_cpu_no_intctrl                       (l2_cpu3_no_intctrl),
                    .l2_cpu_pf_rd_vld_skid_popped            (l2_cpu3_pf_rd_vld_skid_popped),
                    .l2_cpu_pf_throttle_q                    (l2_cpu3_pf_throttle_q),
                    .l2_cpu_pmu_events                       (l2_cpu3_pmu_events[33:0]),
                    .l2_cpu_rbufid                           (l2_cpu3_rbufid[2:0]),
                    .l2_cpu_rd_arb                           (l2_cpu3_rd_arb),
                    .l2_cpu_rd_vld_skid                      (l2_cpu3_rd_vld_skid),
                    .l2_cpu_rexfail                          (l2_cpu3_rexfail),
                    .l2_cpu_rstate                           (l2_cpu3_rstate[1:0]),
                    .l2_cpu_rvalid                           (l2_cpu3_rvalid),
                    .l2_cpu_spec_bufid                       (l2_cpu3_spec_bufid[2:0]),
                    .l2_cpu_spec_valid                       (l2_cpu3_spec_valid),
                    .l2_cpu_spr_rd_data                      (l2_cpu3_spr_rd_data[63:0]),
                    .l2_cpu_tbw_dbl_ecc_err                  (l2_cpu3_tbw_dbl_ecc_err),
                    .l2_cpu_tbw_desc_data                    (l2_cpu3_tbw_desc_data[63:0]),
                    .l2_cpu_tbw_desc_vld                     (l2_cpu3_tbw_desc_vld),
                    .l2_cpu_tbw_ext_err                      (l2_cpu3_tbw_ext_err),
                    .l2_cpu_tbw_ext_err_type                 (l2_cpu3_tbw_ext_err_type),
                    .l2_cpu_tlb_ccb_clken_c3                 (l2_cpu3_tlb_ccb_clken_c3),
                    .l2_cpu_tlb_ccb_req_c3                   (l2_cpu3_tlb_ccb_req_c3),
                    .l2_cpu_tlb_sync_complete                (l2_cpu3_tlb_sync_complete),
                    .l2_cpu_tlb_sync_req                     (l2_cpu3_tlb_sync_req),
                    .l2_cpu_trq_haz_pending                  (l2_cpu3_trq_haz_pending),
                    .l2_cpu_wr_arb                           (l2_cpu3_wr_arb),
                    .l2_cpu_wr_data_stall                    (l2_cpu3_wr_data_stall),
                    .l2_cpu_wr_ex_fail                       (l2_cpu3_wr_ex_fail),
                    .l2_cpu_wr_ex_resp                       (l2_cpu3_wr_ex_resp),
                    .l2_cpu_wr_vld_skid                      (l2_cpu3_wr_vld_skid),
                    .l2_cpu_wrq_haz_pending                  (l2_cpu3_wrq_haz_pending),
                    .l2_tbnk0_cpu_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu3_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk0_cpu_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu3_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk0_cpu_trq_clr_l4_dly2_q          (l2_tbnk0_cpu3_trq_clr_l4_dly2_q),
                    .l2_tbnk0_cpu_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu3_wrq_clr_l4_dly2_q[5:0]),
                    .l2_tbnk1_cpu_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu3_ifq_clr_l4_dly2_q[2:0]),
                    .l2_tbnk1_cpu_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu3_lrq_clr_l4_dly2_q[3:0]),
                    .l2_tbnk1_cpu_trq_clr_l4_dly2_q          (l2_tbnk1_cpu3_trq_clr_l4_dly2_q),
                    .l2_tbnk1_cpu_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu3_wrq_clr_l4_dly2_q[5:0]),
                    .ncorereset_cpu                          (ncorereset_cpu3_o),
                    .ncpuporeset_cpu                         (ncpuporeset_cpu3_o),
                    .niden_cpu                               (niden_cpu3_o),
                    .nmbistreset_cpu                         (nmbistreset_cpu3_o),
                    .rvbaraddr_cpu                           (rvbaraddr_cpu3_o[43:2]),
                    .spiden_cpu                              (spiden_cpu3_o),
                    .spniden_cpu                             (spniden_cpu3_o),
                    .syncreqm_cpu                            (syncreqm_cpu3_o),
                    .tm_cnthctl_kernel                       (tm_cpu3_cnthctl_kernel[1:0]),
                    .tm_cntkctl_usr                          (tm_cpu3_cntkctl_usr[3:0]),
                    .tsvalueb_cpu                            (tsvalueb_cpu3_o[63:0]),
                    .vinithi_cpu                             (vinithi_cpu3_o)
                  );    // ucpu3

    maia_noncpu_feq28_s unoncpu( // outputs
                                 .AFREADYM0                                (AFREADYM0),
                                 .AFREADYM1                                (AFREADYM1),
                                 .AFREADYM2                                (AFREADYM2),
                                 .AFREADYM3                                (AFREADYM3),
                                 .ARREADYS                                 (ARREADYS),
                                 .ATBYTESM0                                (ATBYTESM0[1:0]),
                                 .ATBYTESM1                                (ATBYTESM1[1:0]),
                                 .ATBYTESM2                                (ATBYTESM2[1:0]),
                                 .ATBYTESM3                                (ATBYTESM3[1:0]),
                                 .ATDATAM0                                 (ATDATAM0[31:0]),
                                 .ATDATAM1                                 (ATDATAM1[31:0]),
                                 .ATDATAM2                                 (ATDATAM2[31:0]),
                                 .ATDATAM3                                 (ATDATAM3[31:0]),
                                 .ATIDM0                                   (ATIDM0[6:0]),
                                 .ATIDM1                                   (ATIDM1[6:0]),
                                 .ATIDM2                                   (ATIDM2[6:0]),
                                 .ATIDM3                                   (ATIDM3[6:0]),
                                 .ATVALIDM0                                (ATVALIDM0),
                                 .ATVALIDM1                                (ATVALIDM1),
                                 .ATVALIDM2                                (ATVALIDM2),
                                 .ATVALIDM3                                (ATVALIDM3),
                                 .AWREADYS                                 (AWREADYS),
                                 .BIDS                                     (BIDS[4:0]),
                                 .BRESPS                                   (BRESPS[1:0]),
                                 .BVALIDS                                  (BVALIDS),
                                 .CLREXMONACK                              (CLREXMONACK),
                                 .COMMRX                                   (COMMRX[`MAIA_CN:0]),
                                 .COMMTX                                   (COMMTX[`MAIA_CN:0]),
                                 .CPUQACCEPTn                              (CPUQACCEPTn[`MAIA_CN:0]),
                                 .CPUQACTIVE                               (CPUQACTIVE[`MAIA_CN:0]),
                                 .CPUQDENY                                 (CPUQDENY[`MAIA_CN:0]),
                                 .CTICHINACK                               (CTICHINACK[3:0]),
                                 .CTICHOUT                                 (CTICHOUT[3:0]),
                                 .CTIIRQ                                   (CTIIRQ[`MAIA_CN:0]),
                                 .DBGACK                                   (DBGACK[`MAIA_CN:0]),
                                 .DBGNOPWRDWN                              (DBGNOPWRDWN[`MAIA_CN:0]),
                                 .DBGPWRUPREQ                              (DBGPWRUPREQ[`MAIA_CN:0]),
                                 .DBGRSTREQ                                (DBGRSTREQ[`MAIA_CN:0]),
                                 .EVENTO                                   (EVENTO),
                                 .ICCTDATA                                 (ICCTDATA[15:0]),
                                 .ICCTID                                   (ICCTID[1:0]),
                                 .ICCTLAST                                 (ICCTLAST),
                                 .ICCTVALID                                (ICCTVALID),
                                 .ICDTREADY                                (ICDTREADY),
                                 .L2FLUSHDONE                              (L2FLUSHDONE),
                                 .L2QACCEPTn                               (L2QACCEPTn),
                                 .L2QACTIVE                                (L2QACTIVE),
                                 .L2QDENY                                  (L2QDENY),
                                 .PMUEVENT0                                (PMUEVENT0[24:0]),
                                 .PMUEVENT1                                (PMUEVENT1[24:0]),
                                 .PMUEVENT2                                (PMUEVENT2[24:0]),
                                 .PMUEVENT3                                (PMUEVENT3[24:0]),
                                 .PMUSNAPSHOTACK                           (PMUSNAPSHOTACK[`MAIA_CN:0]),
                                 .PRDATADBG                                (PRDATADBG[31:0]),
                                 .PREADYDBG                                (PREADYDBG),
                                 .PSLVERRDBG                               (PSLVERRDBG),
                                 .RDATAS                                   (RDATAS[127:0]),
                                 .REQMEMATTR                               (REQMEMATTR[7:0]),
                                 .RIDS                                     (RIDS[4:0]),
                                 .RLASTS                                   (RLASTS),
                                 .RRESPS                                   (RRESPS[1:0]),
                                 .RVALIDS                                  (RVALIDS),
                                 .RXDATLCRDV                               (RXDATLCRDV),
                                 .RXLINKACTIVEACK                          (RXLINKACTIVEACK),
                                 .RXRSPLCRDV                               (RXRSPLCRDV),
                                 .RXSNPLCRDV                               (RXSNPLCRDV),
                                 .SMPEN                                    (SMPEN[`MAIA_CN:0]),
                                 .STANDBYWFE                               (STANDBYWFE[`MAIA_CN:0]),
                                 .STANDBYWFI                               (STANDBYWFI[`MAIA_CN:0]),
                                 .STANDBYWFIL2                             (STANDBYWFIL2),
                                 .TXDATFLIT                                (TXDATFLIT[193:0]),
                                 .TXDATFLITPEND                            (TXDATFLITPEND),
                                 .TXDATFLITV                               (TXDATFLITV),
                                 .TXLINKACTIVEREQ                          (TXLINKACTIVEREQ),
                                 .TXREQFLIT                                (TXREQFLIT[99:0]),
                                 .TXREQFLITPEND                            (TXREQFLITPEND),
                                 .TXREQFLITV                               (TXREQFLITV),
                                 .TXRSPFLIT                                (TXRSPFLIT[44:0]),
                                 .TXRSPFLITPEND                            (TXRSPFLITPEND),
                                 .TXRSPFLITV                               (TXRSPFLITV),
                                 .TXSACTIVE                                (TXSACTIVE),
                                 .WARMRSTREQ                               (WARMRSTREQ[`MAIA_CN:0]),
                                 .WREADYS                                  (WREADYS),
                                 .aa64naa32_cpu0_o                         (aa64naa32_cpu0_o),
                                 .aa64naa32_cpu1_o                         (aa64naa32_cpu1_o),
                                 .aa64naa32_cpu2_o                         (aa64naa32_cpu2_o),
                                 .aa64naa32_cpu3_o                         (aa64naa32_cpu3_o),
                                 .afvalidm_cpu0_o                          (afvalidm_cpu0_o),
                                 .afvalidm_cpu1_o                          (afvalidm_cpu1_o),
                                 .afvalidm_cpu2_o                          (afvalidm_cpu2_o),
                                 .afvalidm_cpu3_o                          (afvalidm_cpu3_o),
                                 .atclken_cpu0_o                           (atclken_cpu0_o),
                                 .atclken_cpu1_o                           (atclken_cpu1_o),
                                 .atclken_cpu2_o                           (atclken_cpu2_o),
                                 .atclken_cpu3_o                           (atclken_cpu3_o),
                                 .atreadym_cpu0_o                          (atreadym_cpu0_o),
                                 .atreadym_cpu1_o                          (atreadym_cpu1_o),
                                 .atreadym_cpu2_o                          (atreadym_cpu2_o),
                                 .atreadym_cpu3_o                          (atreadym_cpu3_o),
                                 .cfgend_cpu0_o                            (cfgend_cpu0_o),
                                 .cfgend_cpu1_o                            (cfgend_cpu1_o),
                                 .cfgend_cpu2_o                            (cfgend_cpu2_o),
                                 .cfgend_cpu3_o                            (cfgend_cpu3_o),
                                 .cfgte_cpu0_o                             (cfgte_cpu0_o),
                                 .cfgte_cpu1_o                             (cfgte_cpu1_o),
                                 .cfgte_cpu2_o                             (cfgte_cpu2_o),
                                 .cfgte_cpu3_o                             (cfgte_cpu3_o),
                                 .ck_cpu0_crcx_clk_en_n                    (ck_cpu0_crcx_clk_en_n),
                                 .ck_cpu0_event_reg                        (ck_cpu0_event_reg),
                                 .ck_cpu0_wfe_ack                          (ck_cpu0_wfe_ack),
                                 .ck_cpu0_wfi_ack                          (ck_cpu0_wfi_ack),
                                 .ck_cpu1_crcx_clk_en_n                    (ck_cpu1_crcx_clk_en_n),
                                 .ck_cpu1_event_reg                        (ck_cpu1_event_reg),
                                 .ck_cpu1_wfe_ack                          (ck_cpu1_wfe_ack),
                                 .ck_cpu1_wfi_ack                          (ck_cpu1_wfi_ack),
                                 .ck_cpu2_crcx_clk_en_n                    (ck_cpu2_crcx_clk_en_n),
                                 .ck_cpu2_event_reg                        (ck_cpu2_event_reg),
                                 .ck_cpu2_wfe_ack                          (ck_cpu2_wfe_ack),
                                 .ck_cpu2_wfi_ack                          (ck_cpu2_wfi_ack),
                                 .ck_cpu3_crcx_clk_en_n                    (ck_cpu3_crcx_clk_en_n),
                                 .ck_cpu3_event_reg                        (ck_cpu3_event_reg),
                                 .ck_cpu3_wfe_ack                          (ck_cpu3_wfe_ack),
                                 .ck_cpu3_wfi_ack                          (ck_cpu3_wfi_ack),
                                 .ck_gclkt                                 (ck_gclkt[`MAIA_CN:0]),
                                 .clusteridaff1_cpu0_o                     (clusteridaff1_cpu0_o[7:0]),
                                 .clusteridaff1_cpu1_o                     (clusteridaff1_cpu1_o[7:0]),
                                 .clusteridaff1_cpu2_o                     (clusteridaff1_cpu2_o[7:0]),
                                 .clusteridaff1_cpu3_o                     (clusteridaff1_cpu3_o[7:0]),
                                 .clusteridaff2_cpu0_o                     (clusteridaff2_cpu0_o[7:0]),
                                 .clusteridaff2_cpu1_o                     (clusteridaff2_cpu1_o[7:0]),
                                 .clusteridaff2_cpu2_o                     (clusteridaff2_cpu2_o[7:0]),
                                 .clusteridaff2_cpu3_o                     (clusteridaff2_cpu3_o[7:0]),
                                 .cp15sdisable_cpu0_o                      (cp15sdisable_cpu0_o),
                                 .cp15sdisable_cpu1_o                      (cp15sdisable_cpu1_o),
                                 .cp15sdisable_cpu2_o                      (cp15sdisable_cpu2_o),
                                 .cp15sdisable_cpu3_o                      (cp15sdisable_cpu3_o),
                                 .cpuid_cpu0_o                             (cpuid_cpu0_o[1:0]),
                                 .cpuid_cpu1_o                             (cpuid_cpu1_o[1:0]),
                                 .cpuid_cpu2_o                             (cpuid_cpu2_o[1:0]),
                                 .cpuid_cpu3_o                             (cpuid_cpu3_o[1:0]),
                                 .cryptodisable_cpu0_o                     (cryptodisable_cpu0_o),
                                 .cryptodisable_cpu1_o                     (cryptodisable_cpu1_o),
                                 .cryptodisable_cpu2_o                     (cryptodisable_cpu2_o),
                                 .cryptodisable_cpu3_o                     (cryptodisable_cpu3_o),
                                 .dbgen_cpu0_o                             (dbgen_cpu0_o),
                                 .dbgen_cpu1_o                             (dbgen_cpu1_o),
                                 .dbgen_cpu2_o                             (dbgen_cpu2_o),
                                 .dbgen_cpu3_o                             (dbgen_cpu3_o),
                                 .dbgl1rstdisable_cpu0_o                   (dbgl1rstdisable_cpu0_o),
                                 .dbgl1rstdisable_cpu1_o                   (dbgl1rstdisable_cpu1_o),
                                 .dbgl1rstdisable_cpu2_o                   (dbgl1rstdisable_cpu2_o),
                                 .dbgl1rstdisable_cpu3_o                   (dbgl1rstdisable_cpu3_o),
                                 .dbgromaddr_cpu0_o                        (dbgromaddr_cpu0_o[43:12]),
                                 .dbgromaddr_cpu1_o                        (dbgromaddr_cpu1_o[43:12]),
                                 .dbgromaddr_cpu2_o                        (dbgromaddr_cpu2_o[43:12]),
                                 .dbgromaddr_cpu3_o                        (dbgromaddr_cpu3_o[43:12]),
                                 .dbgromaddrv_cpu0_o                       (dbgromaddrv_cpu0_o),
                                 .dbgromaddrv_cpu1_o                       (dbgromaddrv_cpu1_o),
                                 .dbgromaddrv_cpu2_o                       (dbgromaddrv_cpu2_o),
                                 .dbgromaddrv_cpu3_o                       (dbgromaddrv_cpu3_o),
                                 .dftcrclkdisable_cpu0_o                   (dftcrclkdisable_cpu0_o),
                                 .dftcrclkdisable_cpu1_o                   (dftcrclkdisable_cpu1_o),
                                 .dftcrclkdisable_cpu2_o                   (dftcrclkdisable_cpu2_o),
                                 .dftcrclkdisable_cpu3_o                   (dftcrclkdisable_cpu3_o),
                                 .dftramhold_cpu0_o                        (dftramhold_cpu0_o),
                                 .dftramhold_cpu1_o                        (dftramhold_cpu1_o),
                                 .dftramhold_cpu2_o                        (dftramhold_cpu2_o),
                                 .dftramhold_cpu3_o                        (dftramhold_cpu3_o),
                                 .dftrstdisable_cpu0_o                     (dftrstdisable_cpu0_o),
                                 .dftrstdisable_cpu1_o                     (dftrstdisable_cpu1_o),
                                 .dftrstdisable_cpu2_o                     (dftrstdisable_cpu2_o),
                                 .dftrstdisable_cpu3_o                     (dftrstdisable_cpu3_o),
                                 .dftse_cpu0_o                             (dftse_cpu0_o),
                                 .dftse_cpu1_o                             (dftse_cpu1_o),
                                 .dftse_cpu2_o                             (dftse_cpu2_o),
                                 .dftse_cpu3_o                             (dftse_cpu3_o),
                                 .dt_cpu0_cti_triginack_1to0_pclk          (dt_cpu0_cti_triginack_1to0_pclk[1:0]),
                                 .dt_cpu0_cti_triginack_7to4_pclk          (dt_cpu0_cti_triginack_7to4_pclk[3:0]),
                                 .dt_cpu0_cti_trigout_1to0_pclk            (dt_cpu0_cti_trigout_1to0_pclk[1:0]),
                                 .dt_cpu0_cti_trigout_7to4_pclk            (dt_cpu0_cti_trigout_7to4_pclk[3:0]),
                                 .dt_cpu0_dbif_addr_pclk                   (dt_cpu0_dbif_addr_pclk[14:2]),
                                 .dt_cpu0_dbif_locked_pclk                 (dt_cpu0_dbif_locked_pclk),
                                 .dt_cpu0_dbif_req_pclk                    (dt_cpu0_dbif_req_pclk),
                                 .dt_cpu0_dbif_wrdata_pclk                 (dt_cpu0_dbif_wrdata_pclk[31:0]),
                                 .dt_cpu0_dbif_write_pclk                  (dt_cpu0_dbif_write_pclk),
                                 .dt_cpu0_edacr_frc_idleack_pclk           (dt_cpu0_edacr_frc_idleack_pclk),
                                 .dt_cpu0_edbgrq_pclk                      (dt_cpu0_edbgrq_pclk),
                                 .dt_cpu0_edecr_osuce_pclk                 (dt_cpu0_edecr_osuce_pclk),
                                 .dt_cpu0_edecr_rce_pclk                   (dt_cpu0_edecr_rce_pclk),
                                 .dt_cpu0_edecr_ss_pclk                    (dt_cpu0_edecr_ss_pclk),
                                 .dt_cpu0_edprcr_corepurq_pclk             (dt_cpu0_edprcr_corepurq_pclk),
                                 .dt_cpu0_noclkstop_pclk                   (dt_cpu0_noclkstop_pclk),
                                 .dt_cpu0_pmusnapshot_req_pclk             (dt_cpu0_pmusnapshot_req_pclk),
                                 .dt_cpu0_wfx_wakeup_pclk                  (dt_cpu0_wfx_wakeup_pclk),
                                 .dt_cpu1_cti_triginack_1to0_pclk          (dt_cpu1_cti_triginack_1to0_pclk[1:0]),
                                 .dt_cpu1_cti_triginack_7to4_pclk          (dt_cpu1_cti_triginack_7to4_pclk[3:0]),
                                 .dt_cpu1_cti_trigout_1to0_pclk            (dt_cpu1_cti_trigout_1to0_pclk[1:0]),
                                 .dt_cpu1_cti_trigout_7to4_pclk            (dt_cpu1_cti_trigout_7to4_pclk[3:0]),
                                 .dt_cpu1_dbif_addr_pclk                   (dt_cpu1_dbif_addr_pclk[14:2]),
                                 .dt_cpu1_dbif_locked_pclk                 (dt_cpu1_dbif_locked_pclk),
                                 .dt_cpu1_dbif_req_pclk                    (dt_cpu1_dbif_req_pclk),
                                 .dt_cpu1_dbif_wrdata_pclk                 (dt_cpu1_dbif_wrdata_pclk[31:0]),
                                 .dt_cpu1_dbif_write_pclk                  (dt_cpu1_dbif_write_pclk),
                                 .dt_cpu1_edacr_frc_idleack_pclk           (dt_cpu1_edacr_frc_idleack_pclk),
                                 .dt_cpu1_edbgrq_pclk                      (dt_cpu1_edbgrq_pclk),
                                 .dt_cpu1_edecr_osuce_pclk                 (dt_cpu1_edecr_osuce_pclk),
                                 .dt_cpu1_edecr_rce_pclk                   (dt_cpu1_edecr_rce_pclk),
                                 .dt_cpu1_edecr_ss_pclk                    (dt_cpu1_edecr_ss_pclk),
                                 .dt_cpu1_edprcr_corepurq_pclk             (dt_cpu1_edprcr_corepurq_pclk),
                                 .dt_cpu1_noclkstop_pclk                   (dt_cpu1_noclkstop_pclk),
                                 .dt_cpu1_pmusnapshot_req_pclk             (dt_cpu1_pmusnapshot_req_pclk),
                                 .dt_cpu1_wfx_wakeup_pclk                  (dt_cpu1_wfx_wakeup_pclk),
                                 .dt_cpu2_cti_triginack_1to0_pclk          (dt_cpu2_cti_triginack_1to0_pclk[1:0]),
                                 .dt_cpu2_cti_triginack_7to4_pclk          (dt_cpu2_cti_triginack_7to4_pclk[3:0]),
                                 .dt_cpu2_cti_trigout_1to0_pclk            (dt_cpu2_cti_trigout_1to0_pclk[1:0]),
                                 .dt_cpu2_cti_trigout_7to4_pclk            (dt_cpu2_cti_trigout_7to4_pclk[3:0]),
                                 .dt_cpu2_dbif_addr_pclk                   (dt_cpu2_dbif_addr_pclk[14:2]),
                                 .dt_cpu2_dbif_locked_pclk                 (dt_cpu2_dbif_locked_pclk),
                                 .dt_cpu2_dbif_req_pclk                    (dt_cpu2_dbif_req_pclk),
                                 .dt_cpu2_dbif_wrdata_pclk                 (dt_cpu2_dbif_wrdata_pclk[31:0]),
                                 .dt_cpu2_dbif_write_pclk                  (dt_cpu2_dbif_write_pclk),
                                 .dt_cpu2_edacr_frc_idleack_pclk           (dt_cpu2_edacr_frc_idleack_pclk),
                                 .dt_cpu2_edbgrq_pclk                      (dt_cpu2_edbgrq_pclk),
                                 .dt_cpu2_edecr_osuce_pclk                 (dt_cpu2_edecr_osuce_pclk),
                                 .dt_cpu2_edecr_rce_pclk                   (dt_cpu2_edecr_rce_pclk),
                                 .dt_cpu2_edecr_ss_pclk                    (dt_cpu2_edecr_ss_pclk),
                                 .dt_cpu2_edprcr_corepurq_pclk             (dt_cpu2_edprcr_corepurq_pclk),
                                 .dt_cpu2_noclkstop_pclk                   (dt_cpu2_noclkstop_pclk),
                                 .dt_cpu2_pmusnapshot_req_pclk             (dt_cpu2_pmusnapshot_req_pclk),
                                 .dt_cpu2_wfx_wakeup_pclk                  (dt_cpu2_wfx_wakeup_pclk),
                                 .dt_cpu3_cti_triginack_1to0_pclk          (dt_cpu3_cti_triginack_1to0_pclk[1:0]),
                                 .dt_cpu3_cti_triginack_7to4_pclk          (dt_cpu3_cti_triginack_7to4_pclk[3:0]),
                                 .dt_cpu3_cti_trigout_1to0_pclk            (dt_cpu3_cti_trigout_1to0_pclk[1:0]),
                                 .dt_cpu3_cti_trigout_7to4_pclk            (dt_cpu3_cti_trigout_7to4_pclk[3:0]),
                                 .dt_cpu3_dbif_addr_pclk                   (dt_cpu3_dbif_addr_pclk[14:2]),
                                 .dt_cpu3_dbif_locked_pclk                 (dt_cpu3_dbif_locked_pclk),
                                 .dt_cpu3_dbif_req_pclk                    (dt_cpu3_dbif_req_pclk),
                                 .dt_cpu3_dbif_wrdata_pclk                 (dt_cpu3_dbif_wrdata_pclk[31:0]),
                                 .dt_cpu3_dbif_write_pclk                  (dt_cpu3_dbif_write_pclk),
                                 .dt_cpu3_edacr_frc_idleack_pclk           (dt_cpu3_edacr_frc_idleack_pclk),
                                 .dt_cpu3_edbgrq_pclk                      (dt_cpu3_edbgrq_pclk),
                                 .dt_cpu3_edecr_osuce_pclk                 (dt_cpu3_edecr_osuce_pclk),
                                 .dt_cpu3_edecr_rce_pclk                   (dt_cpu3_edecr_rce_pclk),
                                 .dt_cpu3_edecr_ss_pclk                    (dt_cpu3_edecr_ss_pclk),
                                 .dt_cpu3_edprcr_corepurq_pclk             (dt_cpu3_edprcr_corepurq_pclk),
                                 .dt_cpu3_noclkstop_pclk                   (dt_cpu3_noclkstop_pclk),
                                 .dt_cpu3_pmusnapshot_req_pclk             (dt_cpu3_pmusnapshot_req_pclk),
                                 .dt_cpu3_wfx_wakeup_pclk                  (dt_cpu3_wfx_wakeup_pclk),
                                 .giccdisable_cpu0_o                       (giccdisable_cpu0_o),
                                 .giccdisable_cpu1_o                       (giccdisable_cpu1_o),
                                 .giccdisable_cpu2_o                       (giccdisable_cpu2_o),
                                 .giccdisable_cpu3_o                       (giccdisable_cpu3_o),
                                 .ic_block_eoi_sgi_wr                      (ic_block_eoi_sgi_wr[`MAIA_CN:0]),
                                 .ic_el_change_complete                    (ic_el_change_complete[`MAIA_CN:0]),
                                 .ic_hcr_change_complete                   (ic_hcr_change_complete[`MAIA_CN:0]),
                                 .ic_ich_el2_tall0                         (ic_ich_el2_tall0[`MAIA_CN:0]),
                                 .ic_ich_el2_tall1                         (ic_ich_el2_tall1[`MAIA_CN:0]),
                                 .ic_ich_el2_tc                            (ic_ich_el2_tc[`MAIA_CN:0]),
                                 .ic_nfiq                                  (ic_nfiq[`MAIA_CN:0]),
                                 .ic_nirq                                  (ic_nirq[`MAIA_CN:0]),
                                 .ic_nsei                                  (ic_nsei[`MAIA_CN:0]),
                                 .ic_nvfiq                                 (ic_nvfiq[`MAIA_CN:0]),
                                 .ic_nvirq                                 (ic_nvirq[`MAIA_CN:0]),
                                 .ic_nvsei                                 (ic_nvsei[`MAIA_CN:0]),
                                 .ic_p_valid                               (ic_p_valid[`MAIA_CN:0]),
                                 .ic_sample_spr                            (ic_sample_spr[`MAIA_CN:0]),
                                 .ic_scr_change_complete                   (ic_scr_change_complete[`MAIA_CN:0]),
                                 .ic_sra_el1ns_en                          (ic_sra_el1ns_en[`MAIA_CN:0]),
                                 .ic_sra_el1s_en                           (ic_sra_el1s_en[`MAIA_CN:0]),
                                 .ic_sra_el2_en                            (ic_sra_el2_en[`MAIA_CN:0]),
                                 .ic_sra_el3_en                            (ic_sra_el3_en[`MAIA_CN:0]),
                                 .ic_sre_el1ns_hyp_trap                    (ic_sre_el1ns_hyp_trap[`MAIA_CN:0]),
                                 .ic_sre_el1ns_mon_trap                    (ic_sre_el1ns_mon_trap[`MAIA_CN:0]),
                                 .ic_sre_el1s_mon_trap                     (ic_sre_el1s_mon_trap[`MAIA_CN:0]),
                                 .ic_sre_el2_mon_trap                      (ic_sre_el2_mon_trap[`MAIA_CN:0]),
                                 .l2_cpu0_arb_thrshld_timeout_en           (l2_cpu0_arb_thrshld_timeout_en),
                                 .l2_cpu0_barrier_done                     (l2_cpu0_barrier_done),
                                 .l2_cpu0_blk_non_evict_wr                 (l2_cpu0_blk_non_evict_wr),
                                 .l2_cpu0_ccb_dbg_req_c3                   (l2_cpu0_ccb_dbg_req_c3),
                                 .l2_cpu0_ccb_req_addr_c3                  (l2_cpu0_ccb_req_addr_c3[48:0]),
                                 .l2_cpu0_ccb_req_id_c3                    (l2_cpu0_ccb_req_id_c3[4:0]),
                                 .l2_cpu0_ccb_req_info_c3                  (l2_cpu0_ccb_req_info_c3[23:0]),
                                 .l2_cpu0_ccb_req_type_c3                  (l2_cpu0_ccb_req_type_c3[8:0]),
                                 .l2_cpu0_cfg_ecc_en                       (l2_cpu0_cfg_ecc_en),
                                 .l2_cpu0_dbufid_r1                        (l2_cpu0_dbufid_r1[2:0]),
                                 .l2_cpu0_ddata_r2                         (l2_cpu0_ddata_r2[129:0]),
                                 .l2_cpu0_ddbl_ecc_err_r3                  (l2_cpu0_ddlb_ecc_err_r3),
                                 .l2_cpu0_dext_err_r2                      (l2_cpu0_dext_err_r2),
                                 .l2_cpu0_dext_err_type_r2                 (l2_cpu0_dext_err_type_r2),
                                 .l2_cpu0_disable_clean_evict_opt          (l2_cpu0_disable_clean_evict_opt),
                                 .l2_cpu0_dlast_r1                         (l2_cpu0_dlast_r1),
                                 .l2_cpu0_dsngl_ecc_err_r3                 (l2_cpu0_dsngl_ecc_err_r3),
                                 .l2_cpu0_dsq_clr_id_q                     (l2_cpu0_dsq_clr_id_q[3:0]),
                                 .l2_cpu0_dsq_clr_vld_q                    (l2_cpu0_dsq_clr_vld_q),
                                 .l2_cpu0_dsq_rd_buf_id                    (l2_cpu0_dsq_rd_buf_id[3:0]),
                                 .l2_cpu0_dsq_rd_en                        (l2_cpu0_dsq_rd_en),
                                 .l2_cpu0_dsq_rd_en_x2                     (l2_cpu0_dsq_rd_en_x2),
                                 .l2_cpu0_dvalid_r1                        (l2_cpu0_dvalid_r1),
                                 .l2_cpu0_flsh_if_rd_id_l4_dly             (l2_cpu0_flsh_if_rd_id_l4_dly[1:0]),
                                 .l2_cpu0_flsh_if_rd_l4_dly                (l2_cpu0_flsh_if_rd_l4_dly),
                                 .l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu0_flsh_ls_rd_cnt_active_glb_l2_dly),
                                 .l2_cpu0_flsh_ls_rd_id_l2_dly             (l2_cpu0_flsh_ls_rd_id_l2_dly[2:0]),
                                 .l2_cpu0_flsh_ls_rd_id_l4_dly             (l2_cpu0_flsh_ls_rd_id_l4_dly[2:0]),
                                 .l2_cpu0_flsh_ls_rd_l2_dly                (l2_cpu0_flsh_ls_rd_l2_dly),
                                 .l2_cpu0_flsh_ls_rd_l4_dly                (l2_cpu0_flsh_ls_rd_l4_dly),
                                 .l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu0_flsh_ls_wr_cnt_active_glb_l2_dly),
                                 .l2_cpu0_flsh_ls_wr_evict_l4_dly          (l2_cpu0_flsh_ls_wr_evict_l4_dly),
                                 .l2_cpu0_flsh_ls_wr_id_l2_dly             (l2_cpu0_flsh_ls_wr_id_l2_dly[3:0]),
                                 .l2_cpu0_flsh_ls_wr_id_l4_dly             (l2_cpu0_flsh_ls_wr_id_l4_dly[3:0]),
                                 .l2_cpu0_flsh_ls_wr_l2_dly                (l2_cpu0_flsh_ls_wr_l2_dly),
                                 .l2_cpu0_flsh_ls_wr_l4_dly                (l2_cpu0_flsh_ls_wr_l4_dly),
                                 .l2_cpu0_flsh_tw_rd_l4_dly                (l2_cpu0_flsh_tw_rd_l4_dly),
                                 .l2_cpu0_ibufid_r1                        (l2_cpu0_ibufid_r1[1:0]),
                                 .l2_cpu0_ic_barrier_stall_q               (l2_cpu0_ic_barrier_stall_q),
                                 .l2_cpu0_ic_base                          (l2_cpu0_ic_base[43:18]),
                                 .l2_cpu0_ic_vld_skid                      (l2_cpu0_ic_vld_skid),
                                 .l2_cpu0_idata_r2                         (l2_cpu0_idata_r2[127:0]),
                                 .l2_cpu0_idbl_ecc_err_r3                  (l2_cpu0_idlb_ecc_err_r3),
                                 .l2_cpu0_idle_block_reqs_q                (l2_cpu0_idle_block_reqs_q),
                                 .l2_cpu0_iext_err_r2                      (l2_cpu0_iext_err_r2),
                                 .l2_cpu0_iext_err_type_r2                 (l2_cpu0_iext_err_type_r2),
                                 .l2_cpu0_if_ccb_clken_c3                  (l2_cpu0_if_ccb_clken_c3),
                                 .l2_cpu0_if_ccb_req_c3                    (l2_cpu0_if_ccb_req_c3),
                                 .l2_cpu0_if_sync_req                      (l2_cpu0_if_sync_req),
                                 .l2_cpu0_ifq_haz_pending                  (l2_cpu0_ifq_haz_pending),
                                 .l2_cpu0_isngl_ecc_err_r3                 (l2_cpu0_isngl_ecc_err_r3),
                                 .l2_cpu0_ivalid_r1                        (l2_cpu0_ivalid_r1),
                                 .l2_cpu0_l2_cache_size                    (l2_cpu0_l2_cache_size[1:0]),
                                 .l2_cpu0_lrq_haz_pending                  (l2_cpu0_lrq_haz_pending),
                                 .l2_cpu0_ls_ccb_clken_c3                  (l2_cpu0_ls_ccb_clken_c3),
                                 .l2_cpu0_ls_ccb_req_c3                    (l2_cpu0_ls_ccb_req_c3),
                                 .l2_cpu0_ls_peq_coll_l4_dly               (l2_cpu0_ls_peq_coll_l4_dly),
                                 .l2_cpu0_ls_sync_req                      (l2_cpu0_ls_sync_req),
                                 .l2_cpu0_mbist1_addr_b1                   (l2_cpu0_mbist1_addr_b1[12:0]),
                                 .l2_cpu0_mbist1_all_b1                    (l2_cpu0_mbist1_all_b1),
                                 .l2_cpu0_mbist1_array_b1                  (l2_cpu0_mbist1_array_b1[3:0]),
                                 .l2_cpu0_mbist1_be_b1                     (l2_cpu0_mbist1_be_b1[7:0]),
                                 .l2_cpu0_mbist1_en_b1                     (l2_cpu0_mbist1_en_b1),
                                 .l2_cpu0_mbist1_rd_en_b1                  (l2_cpu0_mbist1_rd_en_b1),
                                 .l2_cpu0_mbist1_wr_en_b1                  (l2_cpu0_mbist1_wr_en_b1),
                                 .l2_cpu0_no_intctrl                       (l2_cpu0_no_intctrl),
                                 .l2_cpu0_pf_rd_vld_skid_popped            (l2_cpu0_pf_rd_vld_skid_popped),
                                 .l2_cpu0_pf_throttle_q                    (l2_cpu0_pf_throttle_q),
                                 .l2_cpu0_pmu_events                       (l2_cpu0_pmu_events[33:0]),
                                 .l2_cpu0_rbufid                           (l2_cpu0_rbufid[2:0]),
                                 .l2_cpu0_rd_arb                           (l2_cpu0_rd_arb),
                                 .l2_cpu0_rd_vld_skid                      (l2_cpu0_rd_vld_skid),
                                 .l2_cpu0_rexfail                          (l2_cpu0_rexfail),
                                 .l2_cpu0_rstate                           (l2_cpu0_rstate[1:0]),
                                 .l2_cpu0_rvalid                           (l2_cpu0_rvalid),
                                 .l2_cpu0_spec_bufid                       (l2_cpu0_spec_bufid[2:0]),
                                 .l2_cpu0_spec_valid                       (l2_cpu0_spec_valid),
                                 .l2_cpu0_spr_rd_data                      (l2_cpu0_spr_rd_data[63:0]),
                                 .l2_cpu0_tbw_dbl_ecc_err                  (l2_cpu0_tbw_dbl_ecc_err),
                                 .l2_cpu0_tbw_desc_data                    (l2_cpu0_tbw_desc_data[63:0]),
                                 .l2_cpu0_tbw_desc_vld                     (l2_cpu0_tbw_desc_vld),
                                 .l2_cpu0_tbw_ext_err                      (l2_cpu0_tbw_ext_err),
                                 .l2_cpu0_tbw_ext_err_type                 (l2_cpu0_tbw_ext_err_type),
                                 .l2_cpu0_tlb_ccb_clken_c3                 (l2_cpu0_tlb_ccb_clken_c3),
                                 .l2_cpu0_tlb_ccb_req_c3                   (l2_cpu0_tlb_ccb_req_c3),
                                 .l2_cpu0_tlb_sync_complete                (l2_cpu0_tlb_sync_complete),
                                 .l2_cpu0_tlb_sync_req                     (l2_cpu0_tlb_sync_req),
                                 .l2_cpu0_trq_haz_pending                  (l2_cpu0_trq_haz_pending),
                                 .l2_cpu0_wr_arb                           (l2_cpu0_wr_arb),
                                 .l2_cpu0_wr_data_stall                    (l2_cpu0_wr_data_stall),
                                 .l2_cpu0_wr_ex_fail                       (l2_cpu0_wr_ex_fail),
                                 .l2_cpu0_wr_ex_resp                       (l2_cpu0_wr_ex_resp),
                                 .l2_cpu0_wr_vld_skid                      (l2_cpu0_wr_vld_skid),
                                 .l2_cpu0_wrq_haz_pending                  (l2_cpu0_wrq_haz_pending),
                                 .l2_cpu1_arb_thrshld_timeout_en           (l2_cpu1_arb_thrshld_timeout_en),
                                 .l2_cpu1_barrier_done                     (l2_cpu1_barrier_done),
                                 .l2_cpu1_blk_non_evict_wr                 (l2_cpu1_blk_non_evict_wr),
                                 .l2_cpu1_ccb_dbg_req_c3                   (l2_cpu1_ccb_dbg_req_c3),
                                 .l2_cpu1_ccb_req_addr_c3                  (l2_cpu1_ccb_req_addr_c3[48:0]),
                                 .l2_cpu1_ccb_req_id_c3                    (l2_cpu1_ccb_req_id_c3[4:0]),
                                 .l2_cpu1_ccb_req_info_c3                  (l2_cpu1_ccb_req_info_c3[23:0]),
                                 .l2_cpu1_ccb_req_type_c3                  (l2_cpu1_ccb_req_type_c3[8:0]),
                                 .l2_cpu1_cfg_ecc_en                       (l2_cpu1_cfg_ecc_en),
                                 .l2_cpu1_dbufid_r1                        (l2_cpu1_dbufid_r1[2:0]),
                                 .l2_cpu1_ddata_r2                         (l2_cpu1_ddata_r2[129:0]),
                                 .l2_cpu1_ddbl_ecc_err_r3                  (l2_cpu1_ddlb_ecc_err_r3),
                                 .l2_cpu1_dext_err_r2                      (l2_cpu1_dext_err_r2),
                                 .l2_cpu1_dext_err_type_r2                 (l2_cpu1_dext_err_type_r2),
                                 .l2_cpu1_disable_clean_evict_opt          (l2_cpu1_disable_clean_evict_opt),
                                 .l2_cpu1_dlast_r1                         (l2_cpu1_dlast_r1),
                                 .l2_cpu1_dsngl_ecc_err_r3                 (l2_cpu1_dsngl_ecc_err_r3),
                                 .l2_cpu1_dsq_clr_id_q                     (l2_cpu1_dsq_clr_id_q[3:0]),
                                 .l2_cpu1_dsq_clr_vld_q                    (l2_cpu1_dsq_clr_vld_q),
                                 .l2_cpu1_dsq_rd_buf_id                    (l2_cpu1_dsq_rd_buf_id[3:0]),
                                 .l2_cpu1_dsq_rd_en                        (l2_cpu1_dsq_rd_en),
                                 .l2_cpu1_dsq_rd_en_x2                     (l2_cpu1_dsq_rd_en_x2),
                                 .l2_cpu1_dvalid_r1                        (l2_cpu1_dvalid_r1),
                                 .l2_cpu1_flsh_if_rd_id_l4_dly             (l2_cpu1_flsh_if_rd_id_l4_dly[1:0]),
                                 .l2_cpu1_flsh_if_rd_l4_dly                (l2_cpu1_flsh_if_rd_l4_dly),
                                 .l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu1_flsh_ls_rd_cnt_active_glb_l2_dly),
                                 .l2_cpu1_flsh_ls_rd_id_l2_dly             (l2_cpu1_flsh_ls_rd_id_l2_dly[2:0]),
                                 .l2_cpu1_flsh_ls_rd_id_l4_dly             (l2_cpu1_flsh_ls_rd_id_l4_dly[2:0]),
                                 .l2_cpu1_flsh_ls_rd_l2_dly                (l2_cpu1_flsh_ls_rd_l2_dly),
                                 .l2_cpu1_flsh_ls_rd_l4_dly                (l2_cpu1_flsh_ls_rd_l4_dly),
                                 .l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu1_flsh_ls_wr_cnt_active_glb_l2_dly),
                                 .l2_cpu1_flsh_ls_wr_evict_l4_dly          (l2_cpu1_flsh_ls_wr_evict_l4_dly),
                                 .l2_cpu1_flsh_ls_wr_id_l2_dly             (l2_cpu1_flsh_ls_wr_id_l2_dly[3:0]),
                                 .l2_cpu1_flsh_ls_wr_id_l4_dly             (l2_cpu1_flsh_ls_wr_id_l4_dly[3:0]),
                                 .l2_cpu1_flsh_ls_wr_l2_dly                (l2_cpu1_flsh_ls_wr_l2_dly),
                                 .l2_cpu1_flsh_ls_wr_l4_dly                (l2_cpu1_flsh_ls_wr_l4_dly),
                                 .l2_cpu1_flsh_tw_rd_l4_dly                (l2_cpu1_flsh_tw_rd_l4_dly),
                                 .l2_cpu1_ibufid_r1                        (l2_cpu1_ibufid_r1[1:0]),
                                 .l2_cpu1_ic_barrier_stall_q               (l2_cpu1_ic_barrier_stall_q),
                                 .l2_cpu1_ic_base                          (l2_cpu1_ic_base[43:18]),
                                 .l2_cpu1_ic_vld_skid                      (l2_cpu1_ic_vld_skid),
                                 .l2_cpu1_idata_r2                         (l2_cpu1_idata_r2[127:0]),
                                 .l2_cpu1_idbl_ecc_err_r3                  (l2_cpu1_idlb_ecc_err_r3),
                                 .l2_cpu1_idle_block_reqs_q                (l2_cpu1_idle_block_reqs_q),
                                 .l2_cpu1_iext_err_r2                      (l2_cpu1_iext_err_r2),
                                 .l2_cpu1_iext_err_type_r2                 (l2_cpu1_iext_err_type_r2),
                                 .l2_cpu1_if_ccb_clken_c3                  (l2_cpu1_if_ccb_clken_c3),
                                 .l2_cpu1_if_ccb_req_c3                    (l2_cpu1_if_ccb_req_c3),
                                 .l2_cpu1_if_sync_req                      (l2_cpu1_if_sync_req),
                                 .l2_cpu1_ifq_haz_pending                  (l2_cpu1_ifq_haz_pending),
                                 .l2_cpu1_isngl_ecc_err_r3                 (l2_cpu1_isngl_ecc_err_r3),
                                 .l2_cpu1_ivalid_r1                        (l2_cpu1_ivalid_r1),
                                 .l2_cpu1_l2_cache_size                    (l2_cpu1_l2_cache_size[1:0]),
                                 .l2_cpu1_lrq_haz_pending                  (l2_cpu1_lrq_haz_pending),
                                 .l2_cpu1_ls_ccb_clken_c3                  (l2_cpu1_ls_ccb_clken_c3),
                                 .l2_cpu1_ls_ccb_req_c3                    (l2_cpu1_ls_ccb_req_c3),
                                 .l2_cpu1_ls_peq_coll_l4_dly               (l2_cpu1_ls_peq_coll_l4_dly),
                                 .l2_cpu1_ls_sync_req                      (l2_cpu1_ls_sync_req),
                                 .l2_cpu1_mbist1_addr_b1                   (l2_cpu1_mbist1_addr_b1[12:0]),
                                 .l2_cpu1_mbist1_all_b1                    (l2_cpu1_mbist1_all_b1),
                                 .l2_cpu1_mbist1_array_b1                  (l2_cpu1_mbist1_array_b1[3:0]),
                                 .l2_cpu1_mbist1_be_b1                     (l2_cpu1_mbist1_be_b1[7:0]),
                                 .l2_cpu1_mbist1_en_b1                     (l2_cpu1_mbist1_en_b1),
                                 .l2_cpu1_mbist1_rd_en_b1                  (l2_cpu1_mbist1_rd_en_b1),
                                 .l2_cpu1_mbist1_wr_en_b1                  (l2_cpu1_mbist1_wr_en_b1),
                                 .l2_cpu1_no_intctrl                       (l2_cpu1_no_intctrl),
                                 .l2_cpu1_pf_rd_vld_skid_popped            (l2_cpu1_pf_rd_vld_skid_popped),
                                 .l2_cpu1_pf_throttle_q                    (l2_cpu1_pf_throttle_q),
                                 .l2_cpu1_pmu_events                       (l2_cpu1_pmu_events[33:0]),
                                 .l2_cpu1_rbufid                           (l2_cpu1_rbufid[2:0]),
                                 .l2_cpu1_rd_arb                           (l2_cpu1_rd_arb),
                                 .l2_cpu1_rd_vld_skid                      (l2_cpu1_rd_vld_skid),
                                 .l2_cpu1_rexfail                          (l2_cpu1_rexfail),
                                 .l2_cpu1_rstate                           (l2_cpu1_rstate[1:0]),
                                 .l2_cpu1_rvalid                           (l2_cpu1_rvalid),
                                 .l2_cpu1_spec_bufid                       (l2_cpu1_spec_bufid[2:0]),
                                 .l2_cpu1_spec_valid                       (l2_cpu1_spec_valid),
                                 .l2_cpu1_spr_rd_data                      (l2_cpu1_spr_rd_data[63:0]),
                                 .l2_cpu1_tbw_dbl_ecc_err                  (l2_cpu1_tbw_dbl_ecc_err),
                                 .l2_cpu1_tbw_desc_data                    (l2_cpu1_tbw_desc_data[63:0]),
                                 .l2_cpu1_tbw_desc_vld                     (l2_cpu1_tbw_desc_vld),
                                 .l2_cpu1_tbw_ext_err                      (l2_cpu1_tbw_ext_err),
                                 .l2_cpu1_tbw_ext_err_type                 (l2_cpu1_tbw_ext_err_type),
                                 .l2_cpu1_tlb_ccb_clken_c3                 (l2_cpu1_tlb_ccb_clken_c3),
                                 .l2_cpu1_tlb_ccb_req_c3                   (l2_cpu1_tlb_ccb_req_c3),
                                 .l2_cpu1_tlb_sync_complete                (l2_cpu1_tlb_sync_complete),
                                 .l2_cpu1_tlb_sync_req                     (l2_cpu1_tlb_sync_req),
                                 .l2_cpu1_trq_haz_pending                  (l2_cpu1_trq_haz_pending),
                                 .l2_cpu1_wr_arb                           (l2_cpu1_wr_arb),
                                 .l2_cpu1_wr_data_stall                    (l2_cpu1_wr_data_stall),
                                 .l2_cpu1_wr_ex_fail                       (l2_cpu1_wr_ex_fail),
                                 .l2_cpu1_wr_ex_resp                       (l2_cpu1_wr_ex_resp),
                                 .l2_cpu1_wr_vld_skid                      (l2_cpu1_wr_vld_skid),
                                 .l2_cpu1_wrq_haz_pending                  (l2_cpu1_wrq_haz_pending),
                                 .l2_cpu2_arb_thrshld_timeout_en           (l2_cpu2_arb_thrshld_timeout_en),
                                 .l2_cpu2_barrier_done                     (l2_cpu2_barrier_done),
                                 .l2_cpu2_blk_non_evict_wr                 (l2_cpu2_blk_non_evict_wr),
                                 .l2_cpu2_ccb_dbg_req_c3                   (l2_cpu2_ccb_dbg_req_c3),
                                 .l2_cpu2_ccb_req_addr_c3                  (l2_cpu2_ccb_req_addr_c3[48:0]),
                                 .l2_cpu2_ccb_req_id_c3                    (l2_cpu2_ccb_req_id_c3[4:0]),
                                 .l2_cpu2_ccb_req_info_c3                  (l2_cpu2_ccb_req_info_c3[23:0]),
                                 .l2_cpu2_ccb_req_type_c3                  (l2_cpu2_ccb_req_type_c3[8:0]),
                                 .l2_cpu2_cfg_ecc_en                       (l2_cpu2_cfg_ecc_en),
                                 .l2_cpu2_dbufid_r1                        (l2_cpu2_dbufid_r1[2:0]),
                                 .l2_cpu2_ddata_r2                         (l2_cpu2_ddata_r2[129:0]),
                                 .l2_cpu2_ddbl_ecc_err_r3                  (l2_cpu2_ddlb_ecc_err_r3),
                                 .l2_cpu2_dext_err_r2                      (l2_cpu2_dext_err_r2),
                                 .l2_cpu2_dext_err_type_r2                 (l2_cpu2_dext_err_type_r2),
                                 .l2_cpu2_disable_clean_evict_opt          (l2_cpu2_disable_clean_evict_opt),
                                 .l2_cpu2_dlast_r1                         (l2_cpu2_dlast_r1),
                                 .l2_cpu2_dsngl_ecc_err_r3                 (l2_cpu2_dsngl_ecc_err_r3),
                                 .l2_cpu2_dsq_clr_id_q                     (l2_cpu2_dsq_clr_id_q[3:0]),
                                 .l2_cpu2_dsq_clr_vld_q                    (l2_cpu2_dsq_clr_vld_q),
                                 .l2_cpu2_dsq_rd_buf_id                    (l2_cpu2_dsq_rd_buf_id[3:0]),
                                 .l2_cpu2_dsq_rd_en                        (l2_cpu2_dsq_rd_en),
                                 .l2_cpu2_dsq_rd_en_x2                     (l2_cpu2_dsq_rd_en_x2),
                                 .l2_cpu2_dvalid_r1                        (l2_cpu2_dvalid_r1),
                                 .l2_cpu2_flsh_if_rd_id_l4_dly             (l2_cpu2_flsh_if_rd_id_l4_dly[1:0]),
                                 .l2_cpu2_flsh_if_rd_l4_dly                (l2_cpu2_flsh_if_rd_l4_dly),
                                 .l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu2_flsh_ls_rd_cnt_active_glb_l2_dly),
                                 .l2_cpu2_flsh_ls_rd_id_l2_dly             (l2_cpu2_flsh_ls_rd_id_l2_dly[2:0]),
                                 .l2_cpu2_flsh_ls_rd_id_l4_dly             (l2_cpu2_flsh_ls_rd_id_l4_dly[2:0]),
                                 .l2_cpu2_flsh_ls_rd_l2_dly                (l2_cpu2_flsh_ls_rd_l2_dly),
                                 .l2_cpu2_flsh_ls_rd_l4_dly                (l2_cpu2_flsh_ls_rd_l4_dly),
                                 .l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu2_flsh_ls_wr_cnt_active_glb_l2_dly),
                                 .l2_cpu2_flsh_ls_wr_evict_l4_dly          (l2_cpu2_flsh_ls_wr_evict_l4_dly),
                                 .l2_cpu2_flsh_ls_wr_id_l2_dly             (l2_cpu2_flsh_ls_wr_id_l2_dly[3:0]),
                                 .l2_cpu2_flsh_ls_wr_id_l4_dly             (l2_cpu2_flsh_ls_wr_id_l4_dly[3:0]),
                                 .l2_cpu2_flsh_ls_wr_l2_dly                (l2_cpu2_flsh_ls_wr_l2_dly),
                                 .l2_cpu2_flsh_ls_wr_l4_dly                (l2_cpu2_flsh_ls_wr_l4_dly),
                                 .l2_cpu2_flsh_tw_rd_l4_dly                (l2_cpu2_flsh_tw_rd_l4_dly),
                                 .l2_cpu2_ibufid_r1                        (l2_cpu2_ibufid_r1[1:0]),
                                 .l2_cpu2_ic_barrier_stall_q               (l2_cpu2_ic_barrier_stall_q),
                                 .l2_cpu2_ic_base                          (l2_cpu2_ic_base[43:18]),
                                 .l2_cpu2_ic_vld_skid                      (l2_cpu2_ic_vld_skid),
                                 .l2_cpu2_idata_r2                         (l2_cpu2_idata_r2[127:0]),
                                 .l2_cpu2_idbl_ecc_err_r3                  (l2_cpu2_idlb_ecc_err_r3),
                                 .l2_cpu2_idle_block_reqs_q                (l2_cpu2_idle_block_reqs_q),
                                 .l2_cpu2_iext_err_r2                      (l2_cpu2_iext_err_r2),
                                 .l2_cpu2_iext_err_type_r2                 (l2_cpu2_iext_err_type_r2),
                                 .l2_cpu2_if_ccb_clken_c3                  (l2_cpu2_if_ccb_clken_c3),
                                 .l2_cpu2_if_ccb_req_c3                    (l2_cpu2_if_ccb_req_c3),
                                 .l2_cpu2_if_sync_req                      (l2_cpu2_if_sync_req),
                                 .l2_cpu2_ifq_haz_pending                  (l2_cpu2_ifq_haz_pending),
                                 .l2_cpu2_isngl_ecc_err_r3                 (l2_cpu2_isngl_ecc_err_r3),
                                 .l2_cpu2_ivalid_r1                        (l2_cpu2_ivalid_r1),
                                 .l2_cpu2_l2_cache_size                    (l2_cpu2_l2_cache_size[1:0]),
                                 .l2_cpu2_lrq_haz_pending                  (l2_cpu2_lrq_haz_pending),
                                 .l2_cpu2_ls_ccb_clken_c3                  (l2_cpu2_ls_ccb_clken_c3),
                                 .l2_cpu2_ls_ccb_req_c3                    (l2_cpu2_ls_ccb_req_c3),
                                 .l2_cpu2_ls_peq_coll_l4_dly               (l2_cpu2_ls_peq_coll_l4_dly),
                                 .l2_cpu2_ls_sync_req                      (l2_cpu2_ls_sync_req),
                                 .l2_cpu2_mbist1_addr_b1                   (l2_cpu2_mbist1_addr_b1[12:0]),
                                 .l2_cpu2_mbist1_all_b1                    (l2_cpu2_mbist1_all_b1),
                                 .l2_cpu2_mbist1_array_b1                  (l2_cpu2_mbist1_array_b1[3:0]),
                                 .l2_cpu2_mbist1_be_b1                     (l2_cpu2_mbist1_be_b1[7:0]),
                                 .l2_cpu2_mbist1_en_b1                     (l2_cpu2_mbist1_en_b1),
                                 .l2_cpu2_mbist1_rd_en_b1                  (l2_cpu2_mbist1_rd_en_b1),
                                 .l2_cpu2_mbist1_wr_en_b1                  (l2_cpu2_mbist1_wr_en_b1),
                                 .l2_cpu2_no_intctrl                       (l2_cpu2_no_intctrl),
                                 .l2_cpu2_pf_rd_vld_skid_popped            (l2_cpu2_pf_rd_vld_skid_popped),
                                 .l2_cpu2_pf_throttle_q                    (l2_cpu2_pf_throttle_q),
                                 .l2_cpu2_pmu_events                       (l2_cpu2_pmu_events[33:0]),
                                 .l2_cpu2_rbufid                           (l2_cpu2_rbufid[2:0]),
                                 .l2_cpu2_rd_arb                           (l2_cpu2_rd_arb),
                                 .l2_cpu2_rd_vld_skid                      (l2_cpu2_rd_vld_skid),
                                 .l2_cpu2_rexfail                          (l2_cpu2_rexfail),
                                 .l2_cpu2_rstate                           (l2_cpu2_rstate[1:0]),
                                 .l2_cpu2_rvalid                           (l2_cpu2_rvalid),
                                 .l2_cpu2_spec_bufid                       (l2_cpu2_spec_bufid[2:0]),
                                 .l2_cpu2_spec_valid                       (l2_cpu2_spec_valid),
                                 .l2_cpu2_spr_rd_data                      (l2_cpu2_spr_rd_data[63:0]),
                                 .l2_cpu2_tbw_dbl_ecc_err                  (l2_cpu2_tbw_dbl_ecc_err),
                                 .l2_cpu2_tbw_desc_data                    (l2_cpu2_tbw_desc_data[63:0]),
                                 .l2_cpu2_tbw_desc_vld                     (l2_cpu2_tbw_desc_vld),
                                 .l2_cpu2_tbw_ext_err                      (l2_cpu2_tbw_ext_err),
                                 .l2_cpu2_tbw_ext_err_type                 (l2_cpu2_tbw_ext_err_type),
                                 .l2_cpu2_tlb_ccb_clken_c3                 (l2_cpu2_tlb_ccb_clken_c3),
                                 .l2_cpu2_tlb_ccb_req_c3                   (l2_cpu2_tlb_ccb_req_c3),
                                 .l2_cpu2_tlb_sync_complete                (l2_cpu2_tlb_sync_complete),
                                 .l2_cpu2_tlb_sync_req                     (l2_cpu2_tlb_sync_req),
                                 .l2_cpu2_trq_haz_pending                  (l2_cpu2_trq_haz_pending),
                                 .l2_cpu2_wr_arb                           (l2_cpu2_wr_arb),
                                 .l2_cpu2_wr_data_stall                    (l2_cpu2_wr_data_stall),
                                 .l2_cpu2_wr_ex_fail                       (l2_cpu2_wr_ex_fail),
                                 .l2_cpu2_wr_ex_resp                       (l2_cpu2_wr_ex_resp),
                                 .l2_cpu2_wr_vld_skid                      (l2_cpu2_wr_vld_skid),
                                 .l2_cpu2_wrq_haz_pending                  (l2_cpu2_wrq_haz_pending),
                                 .l2_cpu3_arb_thrshld_timeout_en           (l2_cpu3_arb_thrshld_timeout_en),
                                 .l2_cpu3_barrier_done                     (l2_cpu3_barrier_done),
                                 .l2_cpu3_blk_non_evict_wr                 (l2_cpu3_blk_non_evict_wr),
                                 .l2_cpu3_ccb_dbg_req_c3                   (l2_cpu3_ccb_dbg_req_c3),
                                 .l2_cpu3_ccb_req_addr_c3                  (l2_cpu3_ccb_req_addr_c3[48:0]),
                                 .l2_cpu3_ccb_req_id_c3                    (l2_cpu3_ccb_req_id_c3[4:0]),
                                 .l2_cpu3_ccb_req_info_c3                  (l2_cpu3_ccb_req_info_c3[23:0]),
                                 .l2_cpu3_ccb_req_type_c3                  (l2_cpu3_ccb_req_type_c3[8:0]),
                                 .l2_cpu3_cfg_ecc_en                       (l2_cpu3_cfg_ecc_en),
                                 .l2_cpu3_dbufid_r1                        (l2_cpu3_dbufid_r1[2:0]),
                                 .l2_cpu3_ddata_r2                         (l2_cpu3_ddata_r2[129:0]),
                                 .l2_cpu3_ddbl_ecc_err_r3                  (l2_cpu3_ddlb_ecc_err_r3),
                                 .l2_cpu3_dext_err_r2                      (l2_cpu3_dext_err_r2),
                                 .l2_cpu3_dext_err_type_r2                 (l2_cpu3_dext_err_type_r2),
                                 .l2_cpu3_disable_clean_evict_opt          (l2_cpu3_disable_clean_evict_opt),
                                 .l2_cpu3_dlast_r1                         (l2_cpu3_dlast_r1),
                                 .l2_cpu3_dsngl_ecc_err_r3                 (l2_cpu3_dsngl_ecc_err_r3),
                                 .l2_cpu3_dsq_clr_id_q                     (l2_cpu3_dsq_clr_id_q[3:0]),
                                 .l2_cpu3_dsq_clr_vld_q                    (l2_cpu3_dsq_clr_vld_q),
                                 .l2_cpu3_dsq_rd_buf_id                    (l2_cpu3_dsq_rd_buf_id[3:0]),
                                 .l2_cpu3_dsq_rd_en                        (l2_cpu3_dsq_rd_en),
                                 .l2_cpu3_dsq_rd_en_x2                     (l2_cpu3_dsq_rd_en_x2),
                                 .l2_cpu3_dvalid_r1                        (l2_cpu3_dvalid_r1),
                                 .l2_cpu3_flsh_if_rd_id_l4_dly             (l2_cpu3_flsh_if_rd_id_l4_dly[1:0]),
                                 .l2_cpu3_flsh_if_rd_l4_dly                (l2_cpu3_flsh_if_rd_l4_dly),
                                 .l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly (l2_cpu3_flsh_ls_rd_cnt_active_glb_l2_dly),
                                 .l2_cpu3_flsh_ls_rd_id_l2_dly             (l2_cpu3_flsh_ls_rd_id_l2_dly[2:0]),
                                 .l2_cpu3_flsh_ls_rd_id_l4_dly             (l2_cpu3_flsh_ls_rd_id_l4_dly[2:0]),
                                 .l2_cpu3_flsh_ls_rd_l2_dly                (l2_cpu3_flsh_ls_rd_l2_dly),
                                 .l2_cpu3_flsh_ls_rd_l4_dly                (l2_cpu3_flsh_ls_rd_l4_dly),
                                 .l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly (l2_cpu3_flsh_ls_wr_cnt_active_glb_l2_dly),
                                 .l2_cpu3_flsh_ls_wr_evict_l4_dly          (l2_cpu3_flsh_ls_wr_evict_l4_dly),
                                 .l2_cpu3_flsh_ls_wr_id_l2_dly             (l2_cpu3_flsh_ls_wr_id_l2_dly[3:0]),
                                 .l2_cpu3_flsh_ls_wr_id_l4_dly             (l2_cpu3_flsh_ls_wr_id_l4_dly[3:0]),
                                 .l2_cpu3_flsh_ls_wr_l2_dly                (l2_cpu3_flsh_ls_wr_l2_dly),
                                 .l2_cpu3_flsh_ls_wr_l4_dly                (l2_cpu3_flsh_ls_wr_l4_dly),
                                 .l2_cpu3_flsh_tw_rd_l4_dly                (l2_cpu3_flsh_tw_rd_l4_dly),
                                 .l2_cpu3_ibufid_r1                        (l2_cpu3_ibufid_r1[1:0]),
                                 .l2_cpu3_ic_barrier_stall_q               (l2_cpu3_ic_barrier_stall_q),
                                 .l2_cpu3_ic_base                          (l2_cpu3_ic_base[43:18]),
                                 .l2_cpu3_ic_vld_skid                      (l2_cpu3_ic_vld_skid),
                                 .l2_cpu3_idata_r2                         (l2_cpu3_idata_r2[127:0]),
                                 .l2_cpu3_idbl_ecc_err_r3                  (l2_cpu3_idlb_ecc_err_r3),
                                 .l2_cpu3_idle_block_reqs_q                (l2_cpu3_idle_block_reqs_q),
                                 .l2_cpu3_iext_err_r2                      (l2_cpu3_iext_err_r2),
                                 .l2_cpu3_iext_err_type_r2                 (l2_cpu3_iext_err_type_r2),
                                 .l2_cpu3_if_ccb_clken_c3                  (l2_cpu3_if_ccb_clken_c3),
                                 .l2_cpu3_if_ccb_req_c3                    (l2_cpu3_if_ccb_req_c3),
                                 .l2_cpu3_if_sync_req                      (l2_cpu3_if_sync_req),
                                 .l2_cpu3_ifq_haz_pending                  (l2_cpu3_ifq_haz_pending),
                                 .l2_cpu3_isngl_ecc_err_r3                 (l2_cpu3_isngl_ecc_err_r3),
                                 .l2_cpu3_ivalid_r1                        (l2_cpu3_ivalid_r1),
                                 .l2_cpu3_l2_cache_size                    (l2_cpu3_l2_cache_size[1:0]),
                                 .l2_cpu3_lrq_haz_pending                  (l2_cpu3_lrq_haz_pending),
                                 .l2_cpu3_ls_ccb_clken_c3                  (l2_cpu3_ls_ccb_clken_c3),
                                 .l2_cpu3_ls_ccb_req_c3                    (l2_cpu3_ls_ccb_req_c3),
                                 .l2_cpu3_ls_peq_coll_l4_dly               (l2_cpu3_ls_peq_coll_l4_dly),
                                 .l2_cpu3_ls_sync_req                      (l2_cpu3_ls_sync_req),
                                 .l2_cpu3_mbist1_addr_b1                   (l2_cpu3_mbist1_addr_b1[12:0]),
                                 .l2_cpu3_mbist1_all_b1                    (l2_cpu3_mbist1_all_b1),
                                 .l2_cpu3_mbist1_array_b1                  (l2_cpu3_mbist1_array_b1[3:0]),
                                 .l2_cpu3_mbist1_be_b1                     (l2_cpu3_mbist1_be_b1[7:0]),
                                 .l2_cpu3_mbist1_en_b1                     (l2_cpu3_mbist1_en_b1),
                                 .l2_cpu3_mbist1_rd_en_b1                  (l2_cpu3_mbist1_rd_en_b1),
                                 .l2_cpu3_mbist1_wr_en_b1                  (l2_cpu3_mbist1_wr_en_b1),
                                 .l2_cpu3_no_intctrl                       (l2_cpu3_no_intctrl),
                                 .l2_cpu3_pf_rd_vld_skid_popped            (l2_cpu3_pf_rd_vld_skid_popped),
                                 .l2_cpu3_pf_throttle_q                    (l2_cpu3_pf_throttle_q),
                                 .l2_cpu3_pmu_events                       (l2_cpu3_pmu_events[33:0]),
                                 .l2_cpu3_rbufid                           (l2_cpu3_rbufid[2:0]),
                                 .l2_cpu3_rd_arb                           (l2_cpu3_rd_arb),
                                 .l2_cpu3_rd_vld_skid                      (l2_cpu3_rd_vld_skid),
                                 .l2_cpu3_rexfail                          (l2_cpu3_rexfail),
                                 .l2_cpu3_rstate                           (l2_cpu3_rstate[1:0]),
                                 .l2_cpu3_rvalid                           (l2_cpu3_rvalid),
                                 .l2_cpu3_spec_bufid                       (l2_cpu3_spec_bufid[2:0]),
                                 .l2_cpu3_spec_valid                       (l2_cpu3_spec_valid),
                                 .l2_cpu3_spr_rd_data                      (l2_cpu3_spr_rd_data[63:0]),
                                 .l2_cpu3_tbw_dbl_ecc_err                  (l2_cpu3_tbw_dbl_ecc_err),
                                 .l2_cpu3_tbw_desc_data                    (l2_cpu3_tbw_desc_data[63:0]),
                                 .l2_cpu3_tbw_desc_vld                     (l2_cpu3_tbw_desc_vld),
                                 .l2_cpu3_tbw_ext_err                      (l2_cpu3_tbw_ext_err),
                                 .l2_cpu3_tbw_ext_err_type                 (l2_cpu3_tbw_ext_err_type),
                                 .l2_cpu3_tlb_ccb_clken_c3                 (l2_cpu3_tlb_ccb_clken_c3),
                                 .l2_cpu3_tlb_ccb_req_c3                   (l2_cpu3_tlb_ccb_req_c3),
                                 .l2_cpu3_tlb_sync_complete                (l2_cpu3_tlb_sync_complete),
                                 .l2_cpu3_tlb_sync_req                     (l2_cpu3_tlb_sync_req),
                                 .l2_cpu3_trq_haz_pending                  (l2_cpu3_trq_haz_pending),
                                 .l2_cpu3_wr_arb                           (l2_cpu3_wr_arb),
                                 .l2_cpu3_wr_data_stall                    (l2_cpu3_wr_data_stall),
                                 .l2_cpu3_wr_ex_fail                       (l2_cpu3_wr_ex_fail),
                                 .l2_cpu3_wr_ex_resp                       (l2_cpu3_wr_ex_resp),
                                 .l2_cpu3_wr_vld_skid                      (l2_cpu3_wr_vld_skid),
                                 .l2_cpu3_wrq_haz_pending                  (l2_cpu3_wrq_haz_pending),
                                 .l2_tbnk0_cpu0_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu0_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk0_cpu0_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu0_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk0_cpu0_trq_clr_l4_dly2_q          (l2_tbnk0_cpu0_trq_clr_l4_dly2_q),
                                 .l2_tbnk0_cpu0_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu0_wrq_clr_l4_dly2_q[5:0]),
                                 .l2_tbnk0_cpu1_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu1_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk0_cpu1_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu1_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk0_cpu1_trq_clr_l4_dly2_q          (l2_tbnk0_cpu1_trq_clr_l4_dly2_q),
                                 .l2_tbnk0_cpu1_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu1_wrq_clr_l4_dly2_q[5:0]),
                                 .l2_tbnk0_cpu2_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu2_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk0_cpu2_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu2_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk0_cpu2_trq_clr_l4_dly2_q          (l2_tbnk0_cpu2_trq_clr_l4_dly2_q),
                                 .l2_tbnk0_cpu2_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu2_wrq_clr_l4_dly2_q[5:0]),
                                 .l2_tbnk0_cpu3_ifq_clr_l4_dly2_q          (l2_tbnk0_cpu3_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk0_cpu3_lrq_clr_l4_dly2_q          (l2_tbnk0_cpu3_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk0_cpu3_trq_clr_l4_dly2_q          (l2_tbnk0_cpu3_trq_clr_l4_dly2_q),
                                 .l2_tbnk0_cpu3_wrq_clr_l4_dly2_q          (l2_tbnk0_cpu3_wrq_clr_l4_dly2_q[5:0]),
                                 .l2_tbnk1_cpu0_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu0_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk1_cpu0_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu0_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk1_cpu0_trq_clr_l4_dly2_q          (l2_tbnk1_cpu0_trq_clr_l4_dly2_q),
                                 .l2_tbnk1_cpu0_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu0_wrq_clr_l4_dly2_q[5:0]),
                                 .l2_tbnk1_cpu1_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu1_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk1_cpu1_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu1_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk1_cpu1_trq_clr_l4_dly2_q          (l2_tbnk1_cpu1_trq_clr_l4_dly2_q),
                                 .l2_tbnk1_cpu1_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu1_wrq_clr_l4_dly2_q[5:0]),
                                 .l2_tbnk1_cpu2_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu2_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk1_cpu2_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu2_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk1_cpu2_trq_clr_l4_dly2_q          (l2_tbnk1_cpu2_trq_clr_l4_dly2_q),
                                 .l2_tbnk1_cpu2_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu2_wrq_clr_l4_dly2_q[5:0]),
                                 .l2_tbnk1_cpu3_ifq_clr_l4_dly2_q          (l2_tbnk1_cpu3_ifq_clr_l4_dly2_q[2:0]),
                                 .l2_tbnk1_cpu3_lrq_clr_l4_dly2_q          (l2_tbnk1_cpu3_lrq_clr_l4_dly2_q[3:0]),
                                 .l2_tbnk1_cpu3_trq_clr_l4_dly2_q          (l2_tbnk1_cpu3_trq_clr_l4_dly2_q),
                                 .l2_tbnk1_cpu3_wrq_clr_l4_dly2_q          (l2_tbnk1_cpu3_wrq_clr_l4_dly2_q[5:0]),
                                 .nCNTHPIRQ                                (nCNTHPIRQ[`MAIA_CN:0]),
                                 .nCNTPNSIRQ                               (nCNTPNSIRQ[`MAIA_CN:0]),
                                 .nCNTPSIRQ                                (nCNTPSIRQ[`MAIA_CN:0]),
                                 .nCNTVIRQ                                 (nCNTVIRQ[`MAIA_CN:0]),
                                 .nCOMMIRQ                                 (nCOMMIRQ[`MAIA_CN:0]),
                                 .nEXTERRIRQ                               (nEXTERRIRQ),
                                 .nINTERRIRQ                               (nINTERRIRQ),
                                 .nPMUIRQ                                  (nPMUIRQ[`MAIA_CN:0]),
                                 .nVCPUMNTIRQ                              (nVCPUMNTIRQ[`MAIA_CN:0]),
                                 .ncorereset_cpu0_o                        (ncorereset_cpu0_o),
                                 .ncorereset_cpu1_o                        (ncorereset_cpu1_o),
                                 .ncorereset_cpu2_o                        (ncorereset_cpu2_o),
                                 .ncorereset_cpu3_o                        (ncorereset_cpu3_o),
                                 .ncpuporeset_cpu0_o                       (ncpuporeset_cpu0_o),
                                 .ncpuporeset_cpu1_o                       (ncpuporeset_cpu1_o),
                                 .ncpuporeset_cpu2_o                       (ncpuporeset_cpu2_o),
                                 .ncpuporeset_cpu3_o                       (ncpuporeset_cpu3_o),
                                 .niden_cpu0_o                             (niden_cpu0_o),
                                 .niden_cpu1_o                             (niden_cpu1_o),
                                 .niden_cpu2_o                             (niden_cpu2_o),
                                 .niden_cpu3_o                             (niden_cpu3_o),
                                 .nmbistreset_cpu0_o                       (nmbistreset_cpu0_o),
                                 .nmbistreset_cpu1_o                       (nmbistreset_cpu1_o),
                                 .nmbistreset_cpu2_o                       (nmbistreset_cpu2_o),
                                 .nmbistreset_cpu3_o                       (nmbistreset_cpu3_o),
                                 .rvbaraddr_cpu0_o                         (rvbaraddr_cpu0_o[43:2]),
                                 .rvbaraddr_cpu1_o                         (rvbaraddr_cpu1_o[43:2]),
                                 .rvbaraddr_cpu2_o                         (rvbaraddr_cpu2_o[43:2]),
                                 .rvbaraddr_cpu3_o                         (rvbaraddr_cpu3_o[43:2]),
                                 .spiden_cpu0_o                            (spiden_cpu0_o),
                                 .spiden_cpu1_o                            (spiden_cpu1_o),
                                 .spiden_cpu2_o                            (spiden_cpu2_o),
                                 .spiden_cpu3_o                            (spiden_cpu3_o),
                                 .spniden_cpu0_o                           (spniden_cpu0_o),
                                 .spniden_cpu1_o                           (spniden_cpu1_o),
                                 .spniden_cpu2_o                           (spniden_cpu2_o),
                                 .spniden_cpu3_o                           (spniden_cpu3_o),
                                 .syncreqm_cpu0_o                          (syncreqm_cpu0_o),
                                 .syncreqm_cpu1_o                          (syncreqm_cpu1_o),
                                 .syncreqm_cpu2_o                          (syncreqm_cpu2_o),
                                 .syncreqm_cpu3_o                          (syncreqm_cpu3_o),
                                 .tm_cpu0_cnthctl_kernel                   (tm_cpu0_cnthctl_kernel[1:0]),
                                 .tm_cpu0_cntkctl_usr                      (tm_cpu0_cntkctl_usr[3:0]),
                                 .tm_cpu1_cnthctl_kernel                   (tm_cpu1_cnthctl_kernel[1:0]),
                                 .tm_cpu1_cntkctl_usr                      (tm_cpu1_cntkctl_usr[3:0]),
                                 .tm_cpu2_cnthctl_kernel                   (tm_cpu2_cnthctl_kernel[1:0]),
                                 .tm_cpu2_cntkctl_usr                      (tm_cpu2_cntkctl_usr[3:0]),
                                 .tm_cpu3_cnthctl_kernel                   (tm_cpu3_cnthctl_kernel[1:0]),
                                 .tm_cpu3_cntkctl_usr                      (tm_cpu3_cntkctl_usr[3:0]),
                                 .tsvalueb_cpu0_o                          (tsvalueb_cpu0_o[63:0]),
                                 .tsvalueb_cpu1_o                          (tsvalueb_cpu1_o[63:0]),
                                 .tsvalueb_cpu2_o                          (tsvalueb_cpu2_o[63:0]),
                                 .tsvalueb_cpu3_o                          (tsvalueb_cpu3_o[63:0]),
                                 .vinithi_cpu0_o                           (vinithi_cpu0_o),
                                 .vinithi_cpu1_o                           (vinithi_cpu1_o),
                                 .vinithi_cpu2_o                           (vinithi_cpu2_o),
                                 .vinithi_cpu3_o                           (vinithi_cpu3_o),

                                 // inputs
                                 .AA64nAA32                                (AA64nAA32[`MAIA_CN:0]),
                                 .ACLKENS                                  (ACLKENS),
                                 .AFVALIDM0                                (AFVALIDM0),
                                 .AFVALIDM1                                (AFVALIDM1),
                                 .AFVALIDM2                                (AFVALIDM2),
                                 .AFVALIDM3                                (AFVALIDM3),
                                 .AINACTS                                  (AINACTS),
                                 .ARADDRS                                  (ARADDRS[43:0]),
                                 .ARCACHES                                 (ARCACHES[3:0]),
                                 .ARIDS                                    (ARIDS[4:0]),
                                 .ARLENS                                   (ARLENS[7:0]),
                                 .ARPROTS                                  (ARPROTS[2:0]),
                                 .ARUSERS                                  (ARUSERS[1:0]),
                                 .ARVALIDS                                 (ARVALIDS),
                                 .ATCLKEN                                  (ATCLKEN),
                                 .ATREADYM0                                (ATREADYM0),
                                 .ATREADYM1                                (ATREADYM1),
                                 .ATREADYM2                                (ATREADYM2),
                                 .ATREADYM3                                (ATREADYM3),
                                 .AWADDRS                                  (AWADDRS[43:0]),
                                 .AWCACHES                                 (AWCACHES[3:0]),
                                 .AWIDS                                    (AWIDS[4:0]),
                                 .AWLENS                                   (AWLENS[7:0]),
                                 .AWPROTS                                  (AWPROTS[2:0]),
                                 .AWUSERS                                  (AWUSERS[1:0]),
                                 .AWVALIDS                                 (AWVALIDS),
                                 .BREADYS                                  (BREADYS),
                                 .BROADCASTCACHEMAINT                      (BROADCASTCACHEMAINT),
                                 .BROADCASTINNER                           (BROADCASTINNER),
                                 .BROADCASTOUTER                           (BROADCASTOUTER),
                                 .CFGEND                                   (CFGEND[`MAIA_CN:0]),
                                 .CFGTE                                    (CFGTE[`MAIA_CN:0]),
                                 .CIHSBYPASS                               (CIHSBYPASS[3:0]),
                                 .CISBYPASS                                (CISBYPASS),
                                 .CLK                                      (CLK),
                                 .CLKEN                                    (CLKEN),
                                 .CLREXMONREQ                              (CLREXMONREQ),
                                 .CLUSTERIDAFF1                            (CLUSTERIDAFF1[7:0]),
                                 .CLUSTERIDAFF2                            (CLUSTERIDAFF2[7:0]),
                                 .CNTCLKEN                                 (CNTCLKEN),
                                 .CNTVALUEB                                (CNTVALUEB[63:0]),
                                 .CP15SDISABLE                             (CP15SDISABLE[`MAIA_CN:0]),
                                 .CPUQREQn                                 (CPUQREQn[`MAIA_CN:0]),
                                 .CRYPTODISABLE                            (CRYPTODISABLE[`MAIA_CN:0]),
                                 .CTICHIN                                  (CTICHIN[3:0]),
                                 .CTICHOUTACK                              (CTICHOUTACK[3:0]),
                                 .CTIIRQACK                                (CTIIRQACK[`MAIA_CN:0]),
                                 .DBGEN                                    (DBGEN[`MAIA_CN:0]),
                                 .DBGL1RSTDISABLE                          (DBGL1RSTDISABLE),
                                 .DBGPWRDUP                                (DBGPWRDUP[`MAIA_CN:0]),
                                 .DBGROMADDR                               (DBGROMADDR[43:12]),
                                 .DBGROMADDRV                              (DBGROMADDRV),
                                 .DFTCLKBYPASS                             (DFTCLKBYPASS),
                                 .DFTCRCLKDISABLE                          (DFTCRCLKDISABLE[`MAIA_CN:0]),
                                 .DFTL2CLKDISABLE                          (DFTL2CLKDISABLE),
                                 .DFTMCPHOLD                               (DFTMCPHOLD),
                                 .DFTRAMHOLD                               (DFTRAMHOLD),
                                 .DFTRSTDISABLE                            (DFTRSTDISABLE),
                                 .DFTSE                                    (DFTSE),
                                 .EDBGRQ                                   (EDBGRQ[`MAIA_CN:0]),
                                 .EVENTI                                   (EVENTI),
                                 .GICCDISABLE                              (GICCDISABLE),
                                 .ICCTREADY                                (ICCTREADY),
                                 .ICDTDATA                                 (ICDTDATA[15:0]),
                                 .ICDTDEST                                 (ICDTDEST[1:0]),
                                 .ICDTLAST                                 (ICDTLAST),
                                 .ICDTVALID                                (ICDTVALID),
                                 .L2FLUSHREQ                               (L2FLUSHREQ),
                                 .L2QREQn                                  (L2QREQn),
                                 .L2RSTDISABLE                             (L2RSTDISABLE),
                                 .MBISTREQ                                 (MBISTREQ),
                                 .NIDEN                                    (NIDEN[`MAIA_CN:0]),
                                 .NODEID                                   (NODEID[6:0]),
                                 .PADDRDBG                                 (PADDRDBG[21:2]),
                                 .PADDRDBG31                               (PADDRDBG31),
                                 .PCLKDBG                                  (PCLKDBG),
                                 .PCLKENDBG                                (PCLKENDBG),
                                 .PENABLEDBG                               (PENABLEDBG),
                                 .PERIPHBASE                               (PERIPHBASE[43:18]),
                                 .PMUSNAPSHOTREQ                           (PMUSNAPSHOTREQ[`MAIA_CN:0]),
                                 .PSELDBG                                  (PSELDBG),
                                 .PWDATADBG                                (PWDATADBG[31:0]),
                                 .PWRITEDBG                                (PWRITEDBG),
                                 .RREADYS                                  (RREADYS),
                                 .RVBARADDR0                               (RVBARADDR0[43:2]),
                                 .RVBARADDR1                               (RVBARADDR1[43:2]),
                                 .RVBARADDR2                               (RVBARADDR2[43:2]),
                                 .RVBARADDR3                               (RVBARADDR3[43:2]),
                                 .RXDATFLIT                                (RXDATFLIT[193:0]),
                                 .RXDATFLITPEND                            (RXDATFLITPEND),
                                 .RXDATFLITV                               (RXDATFLITV),
                                 .RXLINKACTIVEREQ                          (RXLINKACTIVEREQ),
                                 .RXRSPFLIT                                (RXRSPFLIT[44:0]),
                                 .RXRSPFLITPEND                            (RXRSPFLITPEND),
                                 .RXRSPFLITV                               (RXRSPFLITV),
                                 .RXSACTIVE                                (RXSACTIVE),
                                 .RXSNPFLIT                                (RXSNPFLIT[64:0]),
                                 .RXSNPFLITPEND                            (RXSNPFLITPEND),
                                 .RXSNPFLITV                               (RXSNPFLITV),
                                 .SAMADDRMAP0                              (SAMADDRMAP0[1:0]),
                                 .SAMADDRMAP1                              (SAMADDRMAP1[1:0]),
                                 .SAMADDRMAP10                             (SAMADDRMAP10[1:0]),
                                 .SAMADDRMAP11                             (SAMADDRMAP11[1:0]),
                                 .SAMADDRMAP12                             (SAMADDRMAP12[1:0]),
                                 .SAMADDRMAP13                             (SAMADDRMAP13[1:0]),
                                 .SAMADDRMAP14                             (SAMADDRMAP14[1:0]),
                                 .SAMADDRMAP15                             (SAMADDRMAP15[1:0]),
                                 .SAMADDRMAP16                             (SAMADDRMAP16[1:0]),
                                 .SAMADDRMAP17                             (SAMADDRMAP17[1:0]),
                                 .SAMADDRMAP18                             (SAMADDRMAP18[1:0]),
                                 .SAMADDRMAP19                             (SAMADDRMAP19[1:0]),
                                 .SAMADDRMAP2                              (SAMADDRMAP2[1:0]),
                                 .SAMADDRMAP3                              (SAMADDRMAP3[1:0]),
                                 .SAMADDRMAP4                              (SAMADDRMAP4[1:0]),
                                 .SAMADDRMAP5                              (SAMADDRMAP5[1:0]),
                                 .SAMADDRMAP6                              (SAMADDRMAP6[1:0]),
                                 .SAMADDRMAP7                              (SAMADDRMAP7[1:0]),
                                 .SAMADDRMAP8                              (SAMADDRMAP8[1:0]),
                                 .SAMADDRMAP9                              (SAMADDRMAP9[1:0]),
                                 .SAMHNF0NODEID                            (SAMHNF0NODEID[6:0]),
                                 .SAMHNF1NODEID                            (SAMHNF1NODEID[6:0]),
                                 .SAMHNF2NODEID                            (SAMHNF2NODEID[6:0]),
                                 .SAMHNF3NODEID                            (SAMHNF3NODEID[6:0]),
                                 .SAMHNF4NODEID                            (SAMHNF4NODEID[6:0]),
                                 .SAMHNF5NODEID                            (SAMHNF5NODEID[6:0]),
                                 .SAMHNF6NODEID                            (SAMHNF6NODEID[6:0]),
                                 .SAMHNF7NODEID                            (SAMHNF7NODEID[6:0]),
                                 .SAMHNFMODE                               (SAMHNFMODE[2:0]),
                                 .SAMHNI0NODEID                            (SAMHNI0NODEID[6:0]),
                                 .SAMHNI1NODEID                            (SAMHNI1NODEID[6:0]),
                                 .SAMMNBASE                                (SAMMNBASE[43:24]),
                                 .SAMMNNODEID                              (SAMMNNODEID[6:0]),
                                 .SCLKEN                                   (SCLKEN),
                                 .SINACT                                   (SINACT),
                                 .SPIDEN                                   (SPIDEN[`MAIA_CN:0]),
                                 .SPNIDEN                                  (SPNIDEN[`MAIA_CN:0]),
                                 .SYNCREQM0                                (SYNCREQM0),
                                 .SYNCREQM1                                (SYNCREQM1),
                                 .SYNCREQM2                                (SYNCREQM2),
                                 .SYNCREQM3                                (SYNCREQM3),
                                 .SYSBARDISABLE                            (SYSBARDISABLE),
                                 .TSVALUEB                                 (TSVALUEB[63:0]),
                                 .TXDATLCRDV                               (TXDATLCRDV),
                                 .TXLINKACTIVEACK                          (TXLINKACTIVEACK),
                                 .TXREQLCRDV                               (TXREQLCRDV),
                                 .TXRSPLCRDV                               (TXRSPLCRDV),
                                 .VINITHI                                  (VINITHI[`MAIA_CN:0]),
                                 .WDATAS                                   (WDATAS[127:0]),
                                 .WLASTS                                   (WLASTS),
                                 .WSTRBS                                   (WSTRBS[15:0]),
                                 .WVALIDS                                  (WVALIDS),
                                 .afreadym_cpu0_i                          (afreadym_cpu0_i),
                                 .afreadym_cpu1_i                          (afreadym_cpu1_i),
                                 .afreadym_cpu2_i                          (afreadym_cpu2_i),
                                 .afreadym_cpu3_i                          (afreadym_cpu3_i),
                                 .atbytesm_cpu0_i                          (atbytesm_cpu0_i[1:0]),
                                 .atbytesm_cpu1_i                          (atbytesm_cpu1_i[1:0]),
                                 .atbytesm_cpu2_i                          (atbytesm_cpu2_i[1:0]),
                                 .atbytesm_cpu3_i                          (atbytesm_cpu3_i[1:0]),
                                 .atdatam_cpu0_i                           (atdatam_cpu0_i[31:0]),
                                 .atdatam_cpu1_i                           (atdatam_cpu1_i[31:0]),
                                 .atdatam_cpu2_i                           (atdatam_cpu2_i[31:0]),
                                 .atdatam_cpu3_i                           (atdatam_cpu3_i[31:0]),
                                 .atidm_cpu0_i                             (atidm_cpu0_i[6:0]),
                                 .atidm_cpu1_i                             (atidm_cpu1_i[6:0]),
                                 .atidm_cpu2_i                             (atidm_cpu2_i[6:0]),
                                 .atidm_cpu3_i                             (atidm_cpu3_i[6:0]),
                                 .atvalidm_cpu0_i                          (atvalidm_cpu0_i),
                                 .atvalidm_cpu1_i                          (atvalidm_cpu1_i),
                                 .atvalidm_cpu2_i                          (atvalidm_cpu2_i),
                                 .atvalidm_cpu3_i                          (atvalidm_cpu3_i),
                                 .commrx_cpu0_i                            (commrx_cpu0_i),
                                 .commrx_cpu1_i                            (commrx_cpu1_i),
                                 .commrx_cpu2_i                            (commrx_cpu2_i),
                                 .commrx_cpu3_i                            (commrx_cpu3_i),
                                 .commtx_cpu0_i                            (commtx_cpu0_i),
                                 .commtx_cpu1_i                            (commtx_cpu1_i),
                                 .commtx_cpu2_i                            (commtx_cpu2_i),
                                 .commtx_cpu3_i                            (commtx_cpu3_i),
                                 .dbgack_cpu0_i                            (dbgack_cpu0_i),
                                 .dbgack_cpu1_i                            (dbgack_cpu1_i),
                                 .dbgack_cpu2_i                            (dbgack_cpu2_i),
                                 .dbgack_cpu3_i                            (dbgack_cpu3_i),
                                 .dbgnopwrdwn_cpu0_i                       (dbgnopwrdwn_cpu0_i),
                                 .dbgnopwrdwn_cpu1_i                       (dbgnopwrdwn_cpu1_i),
                                 .dbgnopwrdwn_cpu2_i                       (dbgnopwrdwn_cpu2_i),
                                 .dbgnopwrdwn_cpu3_i                       (dbgnopwrdwn_cpu3_i),
                                 .dbgrstreq_cpu0_i                         (dbgrstreq_cpu0_i),
                                 .dbgrstreq_cpu1_i                         (dbgrstreq_cpu1_i),
                                 .dbgrstreq_cpu2_i                         (dbgrstreq_cpu2_i),
                                 .dbgrstreq_cpu3_i                         (dbgrstreq_cpu3_i),
                                 .ds_cpu0_cpuectlr_ret                     (ds_cpu0_cpuectlr_ret[2:0]),
                                 .ds_cpu0_cpuectlr_smp                     (ds_cpu0_cpuectlr_smp),
                                 .ds_cpu0_fiq_wfe_qual                     (ds_cpu0_fiq_wfe_qual),
                                 .ds_cpu0_fiq_wfi_qual                     (ds_cpu0_fiq_wfi_qual),
                                 .ds_cpu0_flush                            (ds_cpu0_flush),
                                 .ds_cpu0_flush_type                       (ds_cpu0_flush_type[5:0]),
                                 .ds_cpu0_hcr_va                           (ds_cpu0_hcr_va),
                                 .ds_cpu0_hcr_vf                           (ds_cpu0_hcr_vf),
                                 .ds_cpu0_hcr_vi                           (ds_cpu0_hcr_vi),
                                 .ds_cpu0_ic_aa64naa32                     (ds_cpu0_ic_aa64naa32),
                                 .ds_cpu0_ic_cpsr_mode                     (ds_cpu0_ic_cpsr_mode[4:0]),
                                 .ds_cpu0_ic_hcr_change                    (ds_cpu0_ic_hcr_change),
                                 .ds_cpu0_ic_sample_spr                    (ds_cpu0_ic_sample_spr),
                                 .ds_cpu0_ic_scr_change                    (ds_cpu0_ic_scr_change),
                                 .ds_cpu0_imp_abrt_wfe_qual                (ds_cpu0_imp_abrt_wfe_qual),
                                 .ds_cpu0_imp_abrt_wfi_qual                (ds_cpu0_imp_abrt_wfi_qual),
                                 .ds_cpu0_irq_wfe_qual                     (ds_cpu0_irq_wfe_qual),
                                 .ds_cpu0_irq_wfi_qual                     (ds_cpu0_irq_wfi_qual),
                                 .ds_cpu0_l2_spr_addr                      (ds_cpu0_l2_spr_addr[8:0]),
                                 .ds_cpu0_l2_spr_dw                        (ds_cpu0_l2_spr_dw),
                                 .ds_cpu0_l2_spr_en                        (ds_cpu0_l2_spr_en),
                                 .ds_cpu0_l2_spr_rd                        (ds_cpu0_l2_spr_rd),
                                 .ds_cpu0_l2_spr_wr                        (ds_cpu0_l2_spr_wr),
                                 .ds_cpu0_l2_spr_wr_data                   (ds_cpu0_l2_spr_wr_data[63:0]),
                                 .ds_cpu0_reset_req                        (ds_cpu0_reset_req),
                                 .ds_cpu0_sev_req                          (ds_cpu0_sev_req),
                                 .ds_cpu0_sevl_req                         (ds_cpu0_sevl_req),
                                 .ds_cpu0_vfiq_wfe_qual                    (ds_cpu0_vfiq_wfe_qual),
                                 .ds_cpu0_vfiq_wfi_qual                    (ds_cpu0_vfiq_wfi_qual),
                                 .ds_cpu0_vimp_abrt_wfe_qual               (ds_cpu0_vimp_abrt_wfe_qual),
                                 .ds_cpu0_vimp_abrt_wfi_qual               (ds_cpu0_vimp_abrt_wfi_qual),
                                 .ds_cpu0_virq_wfe_qual                    (ds_cpu0_virq_wfe_qual),
                                 .ds_cpu0_virq_wfi_qual                    (ds_cpu0_virq_wfi_qual),
                                 .ds_cpu0_wfe_req                          (ds_cpu0_wfe_req),
                                 .ds_cpu0_wfi_req                          (ds_cpu0_wfi_req),
                                 .ds_cpu1_cpuectlr_ret                     (ds_cpu1_cpuectlr_ret[2:0]),
                                 .ds_cpu1_cpuectlr_smp                     (ds_cpu1_cpuectlr_smp),
                                 .ds_cpu1_fiq_wfe_qual                     (ds_cpu1_fiq_wfe_qual),
                                 .ds_cpu1_fiq_wfi_qual                     (ds_cpu1_fiq_wfi_qual),
                                 .ds_cpu1_flush                            (ds_cpu1_flush),
                                 .ds_cpu1_flush_type                       (ds_cpu1_flush_type[5:0]),
                                 .ds_cpu1_hcr_va                           (ds_cpu1_hcr_va),
                                 .ds_cpu1_hcr_vf                           (ds_cpu1_hcr_vf),
                                 .ds_cpu1_hcr_vi                           (ds_cpu1_hcr_vi),
                                 .ds_cpu1_ic_aa64naa32                     (ds_cpu1_ic_aa64naa32),
                                 .ds_cpu1_ic_cpsr_mode                     (ds_cpu1_ic_cpsr_mode[4:0]),
                                 .ds_cpu1_ic_hcr_change                    (ds_cpu1_ic_hcr_change),
                                 .ds_cpu1_ic_sample_spr                    (ds_cpu1_ic_sample_spr),
                                 .ds_cpu1_ic_scr_change                    (ds_cpu1_ic_scr_change),
                                 .ds_cpu1_imp_abrt_wfe_qual                (ds_cpu1_imp_abrt_wfe_qual),
                                 .ds_cpu1_imp_abrt_wfi_qual                (ds_cpu1_imp_abrt_wfi_qual),
                                 .ds_cpu1_irq_wfe_qual                     (ds_cpu1_irq_wfe_qual),
                                 .ds_cpu1_irq_wfi_qual                     (ds_cpu1_irq_wfi_qual),
                                 .ds_cpu1_l2_spr_addr                      (ds_cpu1_l2_spr_addr[8:0]),
                                 .ds_cpu1_l2_spr_dw                        (ds_cpu1_l2_spr_dw),
                                 .ds_cpu1_l2_spr_en                        (ds_cpu1_l2_spr_en),
                                 .ds_cpu1_l2_spr_rd                        (ds_cpu1_l2_spr_rd),
                                 .ds_cpu1_l2_spr_wr                        (ds_cpu1_l2_spr_wr),
                                 .ds_cpu1_l2_spr_wr_data                   (ds_cpu1_l2_spr_wr_data[63:0]),
                                 .ds_cpu1_reset_req                        (ds_cpu1_reset_req),
                                 .ds_cpu1_sev_req                          (ds_cpu1_sev_req),
                                 .ds_cpu1_sevl_req                         (ds_cpu1_sevl_req),
                                 .ds_cpu1_vfiq_wfe_qual                    (ds_cpu1_vfiq_wfe_qual),
                                 .ds_cpu1_vfiq_wfi_qual                    (ds_cpu1_vfiq_wfi_qual),
                                 .ds_cpu1_vimp_abrt_wfe_qual               (ds_cpu1_vimp_abrt_wfe_qual),
                                 .ds_cpu1_vimp_abrt_wfi_qual               (ds_cpu1_vimp_abrt_wfi_qual),
                                 .ds_cpu1_virq_wfe_qual                    (ds_cpu1_virq_wfe_qual),
                                 .ds_cpu1_virq_wfi_qual                    (ds_cpu1_virq_wfi_qual),
                                 .ds_cpu1_wfe_req                          (ds_cpu1_wfe_req),
                                 .ds_cpu1_wfi_req                          (ds_cpu1_wfi_req),
                                 .ds_cpu2_cpuectlr_ret                     (ds_cpu2_cpuectlr_ret[2:0]),
                                 .ds_cpu2_cpuectlr_smp                     (ds_cpu2_cpuectlr_smp),
                                 .ds_cpu2_fiq_wfe_qual                     (ds_cpu2_fiq_wfe_qual),
                                 .ds_cpu2_fiq_wfi_qual                     (ds_cpu2_fiq_wfi_qual),
                                 .ds_cpu2_flush                            (ds_cpu2_flush),
                                 .ds_cpu2_flush_type                       (ds_cpu2_flush_type[5:0]),
                                 .ds_cpu2_hcr_va                           (ds_cpu2_hcr_va),
                                 .ds_cpu2_hcr_vf                           (ds_cpu2_hcr_vf),
                                 .ds_cpu2_hcr_vi                           (ds_cpu2_hcr_vi),
                                 .ds_cpu2_ic_aa64naa32                     (ds_cpu2_ic_aa64naa32),
                                 .ds_cpu2_ic_cpsr_mode                     (ds_cpu2_ic_cpsr_mode[4:0]),
                                 .ds_cpu2_ic_hcr_change                    (ds_cpu2_ic_hcr_change),
                                 .ds_cpu2_ic_sample_spr                    (ds_cpu2_ic_sample_spr),
                                 .ds_cpu2_ic_scr_change                    (ds_cpu2_ic_scr_change),
                                 .ds_cpu2_imp_abrt_wfe_qual                (ds_cpu2_imp_abrt_wfe_qual),
                                 .ds_cpu2_imp_abrt_wfi_qual                (ds_cpu2_imp_abrt_wfi_qual),
                                 .ds_cpu2_irq_wfe_qual                     (ds_cpu2_irq_wfe_qual),
                                 .ds_cpu2_irq_wfi_qual                     (ds_cpu2_irq_wfi_qual),
                                 .ds_cpu2_l2_spr_addr                      (ds_cpu2_l2_spr_addr[8:0]),
                                 .ds_cpu2_l2_spr_dw                        (ds_cpu2_l2_spr_dw),
                                 .ds_cpu2_l2_spr_en                        (ds_cpu2_l2_spr_en),
                                 .ds_cpu2_l2_spr_rd                        (ds_cpu2_l2_spr_rd),
                                 .ds_cpu2_l2_spr_wr                        (ds_cpu2_l2_spr_wr),
                                 .ds_cpu2_l2_spr_wr_data                   (ds_cpu2_l2_spr_wr_data[63:0]),
                                 .ds_cpu2_reset_req                        (ds_cpu2_reset_req),
                                 .ds_cpu2_sev_req                          (ds_cpu2_sev_req),
                                 .ds_cpu2_sevl_req                         (ds_cpu2_sevl_req),
                                 .ds_cpu2_vfiq_wfe_qual                    (ds_cpu2_vfiq_wfe_qual),
                                 .ds_cpu2_vfiq_wfi_qual                    (ds_cpu2_vfiq_wfi_qual),
                                 .ds_cpu2_vimp_abrt_wfe_qual               (ds_cpu2_vimp_abrt_wfe_qual),
                                 .ds_cpu2_vimp_abrt_wfi_qual               (ds_cpu2_vimp_abrt_wfi_qual),
                                 .ds_cpu2_virq_wfe_qual                    (ds_cpu2_virq_wfe_qual),
                                 .ds_cpu2_virq_wfi_qual                    (ds_cpu2_virq_wfi_qual),
                                 .ds_cpu2_wfe_req                          (ds_cpu2_wfe_req),
                                 .ds_cpu2_wfi_req                          (ds_cpu2_wfi_req),
                                 .ds_cpu3_cpuectlr_ret                     (ds_cpu3_cpuectlr_ret[2:0]),
                                 .ds_cpu3_cpuectlr_smp                     (ds_cpu3_cpuectlr_smp),
                                 .ds_cpu3_fiq_wfe_qual                     (ds_cpu3_fiq_wfe_qual),
                                 .ds_cpu3_fiq_wfi_qual                     (ds_cpu3_fiq_wfi_qual),
                                 .ds_cpu3_flush                            (ds_cpu3_flush),
                                 .ds_cpu3_flush_type                       (ds_cpu3_flush_type[5:0]),
                                 .ds_cpu3_hcr_va                           (ds_cpu3_hcr_va),
                                 .ds_cpu3_hcr_vf                           (ds_cpu3_hcr_vf),
                                 .ds_cpu3_hcr_vi                           (ds_cpu3_hcr_vi),
                                 .ds_cpu3_ic_aa64naa32                     (ds_cpu3_ic_aa64naa32),
                                 .ds_cpu3_ic_cpsr_mode                     (ds_cpu3_ic_cpsr_mode[4:0]),
                                 .ds_cpu3_ic_hcr_change                    (ds_cpu3_ic_hcr_change),
                                 .ds_cpu3_ic_sample_spr                    (ds_cpu3_ic_sample_spr),
                                 .ds_cpu3_ic_scr_change                    (ds_cpu3_ic_scr_change),
                                 .ds_cpu3_imp_abrt_wfe_qual                (ds_cpu3_imp_abrt_wfe_qual),
                                 .ds_cpu3_imp_abrt_wfi_qual                (ds_cpu3_imp_abrt_wfi_qual),
                                 .ds_cpu3_irq_wfe_qual                     (ds_cpu3_irq_wfe_qual),
                                 .ds_cpu3_irq_wfi_qual                     (ds_cpu3_irq_wfi_qual),
                                 .ds_cpu3_l2_spr_addr                      (ds_cpu3_l2_spr_addr[8:0]),
                                 .ds_cpu3_l2_spr_dw                        (ds_cpu3_l2_spr_dw),
                                 .ds_cpu3_l2_spr_en                        (ds_cpu3_l2_spr_en),
                                 .ds_cpu3_l2_spr_rd                        (ds_cpu3_l2_spr_rd),
                                 .ds_cpu3_l2_spr_wr                        (ds_cpu3_l2_spr_wr),
                                 .ds_cpu3_l2_spr_wr_data                   (ds_cpu3_l2_spr_wr_data[63:0]),
                                 .ds_cpu3_reset_req                        (ds_cpu3_reset_req),
                                 .ds_cpu3_sev_req                          (ds_cpu3_sev_req),
                                 .ds_cpu3_sevl_req                         (ds_cpu3_sevl_req),
                                 .ds_cpu3_vfiq_wfe_qual                    (ds_cpu3_vfiq_wfe_qual),
                                 .ds_cpu3_vfiq_wfi_qual                    (ds_cpu3_vfiq_wfi_qual),
                                 .ds_cpu3_vimp_abrt_wfe_qual               (ds_cpu3_vimp_abrt_wfe_qual),
                                 .ds_cpu3_vimp_abrt_wfi_qual               (ds_cpu3_vimp_abrt_wfi_qual),
                                 .ds_cpu3_virq_wfe_qual                    (ds_cpu3_virq_wfe_qual),
                                 .ds_cpu3_virq_wfi_qual                    (ds_cpu3_virq_wfi_qual),
                                 .ds_cpu3_wfe_req                          (ds_cpu3_wfe_req),
                                 .ds_cpu3_wfi_req                          (ds_cpu3_wfi_req),
                                 .dt_cpu0_coredbg_in_reset_gclk            (dt_cpu0_coredbg_in_reset_gclk),
                                 .dt_cpu0_cti_trigin_1to0_gclk             (dt_cpu0_cti_trigin_1to0_gclk[1:0]),
                                 .dt_cpu0_cti_trigin_7to4_gclk             (dt_cpu0_cti_trigin_7to4_gclk[3:0]),
                                 .dt_cpu0_cti_trigoutack_7to4_gclk         (dt_cpu0_cti_trigoutack_7to4_gclk[3:0]),
                                 .dt_cpu0_cti_trigoutack_bit1_gclk         (dt_cpu0_cti_trigoutack_bit1_gclk),
                                 .dt_cpu0_dbif_ack_gclk                    (dt_cpu0_dbif_ack_gclk),
                                 .dt_cpu0_dbif_err_gclk                    (dt_cpu0_dbif_err_gclk),
                                 .dt_cpu0_dbif_rddata_gclk                 (dt_cpu0_dbif_rddata_gclk[31:0]),
                                 .dt_cpu0_et_oslock_gclk                   (dt_cpu0_et_oslock_gclk),
                                 .dt_cpu0_halt_ack_gclk                    (dt_cpu0_halt_ack_gclk),
                                 .dt_cpu0_hlt_dbgevt_ok_gclk               (dt_cpu0_hlt_dbgevt_ok_gclk),
                                 .dt_cpu0_os_double_lock_gclk              (dt_cpu0_os_double_lock_gclk),
                                 .dt_cpu0_pmusnapshot_ack_gclk             (dt_cpu0_pmusnapshot_ack_gclk),
                                 .dt_cpu0_wfx_dbg_req_gclk                 (dt_cpu0_wfx_dbg_req_gclk),
                                 .dt_cpu1_coredbg_in_reset_gclk            (dt_cpu1_coredbg_in_reset_gclk),
                                 .dt_cpu1_cti_trigin_1to0_gclk             (dt_cpu1_cti_trigin_1to0_gclk[1:0]),
                                 .dt_cpu1_cti_trigin_7to4_gclk             (dt_cpu1_cti_trigin_7to4_gclk[3:0]),
                                 .dt_cpu1_cti_trigoutack_7to4_gclk         (dt_cpu1_cti_trigoutack_7to4_gclk[3:0]),
                                 .dt_cpu1_cti_trigoutack_bit1_gclk         (dt_cpu1_cti_trigoutack_bit1_gclk),
                                 .dt_cpu1_dbif_ack_gclk                    (dt_cpu1_dbif_ack_gclk),
                                 .dt_cpu1_dbif_err_gclk                    (dt_cpu1_dbif_err_gclk),
                                 .dt_cpu1_dbif_rddata_gclk                 (dt_cpu1_dbif_rddata_gclk[31:0]),
                                 .dt_cpu1_et_oslock_gclk                   (dt_cpu1_et_oslock_gclk),
                                 .dt_cpu1_halt_ack_gclk                    (dt_cpu1_halt_ack_gclk),
                                 .dt_cpu1_hlt_dbgevt_ok_gclk               (dt_cpu1_hlt_dbgevt_ok_gclk),
                                 .dt_cpu1_os_double_lock_gclk              (dt_cpu1_os_double_lock_gclk),
                                 .dt_cpu1_pmusnapshot_ack_gclk             (dt_cpu1_pmusnapshot_ack_gclk),
                                 .dt_cpu1_wfx_dbg_req_gclk                 (dt_cpu1_wfx_dbg_req_gclk),
                                 .dt_cpu2_coredbg_in_reset_gclk            (dt_cpu2_coredbg_in_reset_gclk),
                                 .dt_cpu2_cti_trigin_1to0_gclk             (dt_cpu2_cti_trigin_1to0_gclk[1:0]),
                                 .dt_cpu2_cti_trigin_7to4_gclk             (dt_cpu2_cti_trigin_7to4_gclk[3:0]),
                                 .dt_cpu2_cti_trigoutack_7to4_gclk         (dt_cpu2_cti_trigoutack_7to4_gclk[3:0]),
                                 .dt_cpu2_cti_trigoutack_bit1_gclk         (dt_cpu2_cti_trigoutack_bit1_gclk),
                                 .dt_cpu2_dbif_ack_gclk                    (dt_cpu2_dbif_ack_gclk),
                                 .dt_cpu2_dbif_err_gclk                    (dt_cpu2_dbif_err_gclk),
                                 .dt_cpu2_dbif_rddata_gclk                 (dt_cpu2_dbif_rddata_gclk[31:0]),
                                 .dt_cpu2_et_oslock_gclk                   (dt_cpu2_et_oslock_gclk),
                                 .dt_cpu2_halt_ack_gclk                    (dt_cpu2_halt_ack_gclk),
                                 .dt_cpu2_hlt_dbgevt_ok_gclk               (dt_cpu2_hlt_dbgevt_ok_gclk),
                                 .dt_cpu2_os_double_lock_gclk              (dt_cpu2_os_double_lock_gclk),
                                 .dt_cpu2_pmusnapshot_ack_gclk             (dt_cpu2_pmusnapshot_ack_gclk),
                                 .dt_cpu2_wfx_dbg_req_gclk                 (dt_cpu2_wfx_dbg_req_gclk),
                                 .dt_cpu3_coredbg_in_reset_gclk            (dt_cpu3_coredbg_in_reset_gclk),
                                 .dt_cpu3_cti_trigin_1to0_gclk             (dt_cpu3_cti_trigin_1to0_gclk[1:0]),
                                 .dt_cpu3_cti_trigin_7to4_gclk             (dt_cpu3_cti_trigin_7to4_gclk[3:0]),
                                 .dt_cpu3_cti_trigoutack_7to4_gclk         (dt_cpu3_cti_trigoutack_7to4_gclk[3:0]),
                                 .dt_cpu3_cti_trigoutack_bit1_gclk         (dt_cpu3_cti_trigoutack_bit1_gclk),
                                 .dt_cpu3_dbif_ack_gclk                    (dt_cpu3_dbif_ack_gclk),
                                 .dt_cpu3_dbif_err_gclk                    (dt_cpu3_dbif_err_gclk),
                                 .dt_cpu3_dbif_rddata_gclk                 (dt_cpu3_dbif_rddata_gclk[31:0]),
                                 .dt_cpu3_et_oslock_gclk                   (dt_cpu3_et_oslock_gclk),
                                 .dt_cpu3_halt_ack_gclk                    (dt_cpu3_halt_ack_gclk),
                                 .dt_cpu3_hlt_dbgevt_ok_gclk               (dt_cpu3_hlt_dbgevt_ok_gclk),
                                 .dt_cpu3_os_double_lock_gclk              (dt_cpu3_os_double_lock_gclk),
                                 .dt_cpu3_pmusnapshot_ack_gclk             (dt_cpu3_pmusnapshot_ack_gclk),
                                 .dt_cpu3_wfx_dbg_req_gclk                 (dt_cpu3_wfx_dbg_req_gclk),
                                 .etclken_cpu0_i                           (etclken_cpu0_i),
                                 .etclken_cpu1_i                           (etclken_cpu1_i),
                                 .etclken_cpu2_i                           (etclken_cpu2_i),
                                 .etclken_cpu3_i                           (etclken_cpu3_i),
                                 .l2_cpu0_dsq_rd_byte_strb_q               (l2_cpu0_dsq_rd_byte_strb_q[15:0]),
                                 .l2_cpu0_dsq_rd_data_q                    (l2_cpu0_dsq_rd_data_q[129:0]),
                                 .l2_cpu0_dt_pmu_evt_en                    (l2_cpu0_dt_pmu_evt_en),
                                 .l2_cpu0_early_rd_reqe4_e5_q              (l2_cpu0_early_rd_reqe4_e5_q),
                                 .l2_cpu0_ic_addr_arb_set                  (l2_cpu0_ic_addr_arb_set[15:0]),
                                 .l2_cpu0_ic_arb_fast                      (l2_cpu0_ic_arb_fast),
                                 .l2_cpu0_ic_data_arb_set                  (l2_cpu0_ic_data_arb_set[31:0]),
                                 .l2_cpu0_ic_elem_size_arb_set             (l2_cpu0_ic_elem_size_arb_set[2:0]),
                                 .l2_cpu0_ic_excl_arb_set                  (l2_cpu0_ic_excl_arb_set),
                                 .l2_cpu0_ic_id_arb_set                    (l2_cpu0_ic_id_arb_set[2:0]),
                                 .l2_cpu0_ic_ns_arb_set                    (l2_cpu0_ic_ns_arb_set),
                                 .l2_cpu0_ic_write_arb_set                 (l2_cpu0_ic_write_arb_set),
                                 .l2_cpu0_idle_wakeup_q                    (l2_cpu0_idle_wakeup_q),
                                 .l2_cpu0_if_ccb_resp                      (l2_cpu0_if_ccb_resp),
                                 .l2_cpu0_if_ccb_resp_id                   (l2_cpu0_if_ccb_resp_id[4:0]),
                                 .l2_cpu0_if_sync_done_q                   (l2_cpu0_if_sync_done_q),
                                 .l2_cpu0_lrq_haz_clr_id_dcd_q             (l2_cpu0_lrq_haz_clr_id_dcd_q[5:0]),
                                 .l2_cpu0_ls_ccb_data_wr                   (l2_cpu0_ls_ccb_data_wr),
                                 .l2_cpu0_ls_ccb_resp                      (l2_cpu0_ls_ccb_resp),
                                 .l2_cpu0_ls_ccb_resp_id                   (l2_cpu0_ls_ccb_resp_id[4:0]),
                                 .l2_cpu0_ls_rd_haz_id_arb_q               (l2_cpu0_ls_rd_haz_id_arb_q[3:0]),
                                 .l2_cpu0_ls_rd_haz_vld_arb_q              (l2_cpu0_ls_rd_haz_vld_arb_q),
                                 .l2_cpu0_ls_wr_ccb_id_w2a                 (l2_cpu0_ls_wr_ccb_id_w2a[4:0]),
                                 .l2_cpu0_ls_wr_data_w2a                   (l2_cpu0_ls_wr_data_w2a[127:0]),
                                 .l2_cpu0_ls_wr_dirty_w2a                  (l2_cpu0_ls_wr_dirty_w2a),
                                 .l2_cpu0_ls_wr_err_w2a                    (l2_cpu0_ls_wr_err_w2a),
                                 .l2_cpu0_ls_wr_haz_id_arb_q               (l2_cpu0_ls_wr_haz_id_arb_q[2:0]),
                                 .l2_cpu0_ls_wr_haz_vld_arb_q              (l2_cpu0_ls_wr_haz_vld_arb_q),
                                 .l2_cpu0_ls_wr_last_w2a                   (l2_cpu0_ls_wr_last_w2a),
                                 .l2_cpu0_ls_wr_req_w2a                    (l2_cpu0_ls_wr_req_w2a),
                                 .l2_cpu0_ls_wr_type_w2a                   (l2_cpu0_ls_wr_type_w2a[2:0]),
                                 .l2_cpu0_rd_aarch64_arb_set               (l2_cpu0_rd_aarch64_arb_set),
                                 .l2_cpu0_rd_addr_arb_set                  (l2_cpu0_rd_addr_arb_set[44:0]),
                                 .l2_cpu0_rd_arb_fast                      (l2_cpu0_rd_arb_fast),
                                 .l2_cpu0_rd_asid_arb_set                  (l2_cpu0_rd_asid_arb_set[15:8]),
                                 .l2_cpu0_rd_bypass_arb_set                (l2_cpu0_rd_bypass_arb_set),
                                 .l2_cpu0_rd_bypass_bufid_e5               (l2_cpu0_rd_bypass_bufid_e5[2:0]),
                                 .l2_cpu0_rd_bypass_lrq_id_e5              (l2_cpu0_rd_bypass_lrq_id_e5[2:0]),
                                 .l2_cpu0_rd_bypass_req_can_e5             (l2_cpu0_rd_bypass_req_can_e5),
                                 .l2_cpu0_rd_bypass_way_e5                 (l2_cpu0_rd_bypass_way_e5),
                                 .l2_cpu0_rd_cache_attr_arb_set            (l2_cpu0_rd_cache_attr_arb_set[2:0]),
                                 .l2_cpu0_rd_elem_size_arb_set             (l2_cpu0_rd_elem_size_arb_set[2:0]),
                                 .l2_cpu0_rd_excl_arb_set                  (l2_cpu0_rd_excl_arb_set),
                                 .l2_cpu0_rd_id_arb_set                    (l2_cpu0_rd_id_arb_set[4:0]),
                                 .l2_cpu0_rd_lrq_id_arb_set                (l2_cpu0_rd_lrq_id_arb_set[2:0]),
                                 .l2_cpu0_rd_page_attr_arb_set             (l2_cpu0_rd_page_attr_arb_set[7:0]),
                                 .l2_cpu0_rd_prfm_arb_set                  (l2_cpu0_rd_prfm_arb_set),
                                 .l2_cpu0_rd_priv_arb_set                  (l2_cpu0_rd_priv_arb_set),
                                 .l2_cpu0_rd_replayed_arb_set              (l2_cpu0_rd_replayed_arb_set),
                                 .l2_cpu0_rd_shared_arb_set                (l2_cpu0_rd_shared_arb_set[1:0]),
                                 .l2_cpu0_rd_type_arb_set                  (l2_cpu0_rd_type_arb_set[6:0]),
                                 .l2_cpu0_rd_va48_arb_set                  (l2_cpu0_rd_va48_arb_set),
                                 .l2_cpu0_rd_way_arb_set                   (l2_cpu0_rd_way_arb_set),
                                 .l2_cpu0_tlb_sync_done_q                  (l2_cpu0_tlb_sync_done_q),
                                 .l2_cpu0_tw_ccb_resp                      (l2_cpu0_tw_ccb_resp),
                                 .l2_cpu0_tw_ccb_resp_id                   (l2_cpu0_tw_ccb_resp_id[4:0]),
                                 .l2_cpu0_wr_1st_replayed_arb_set          (l2_cpu0_wr_1st_replayed_arb_set),
                                 .l2_cpu0_wr_addr_arb_set                  (l2_cpu0_wr_addr_arb_set[44:0]),
                                 .l2_cpu0_wr_arb_fast                      (l2_cpu0_wr_arb_fast),
                                 .l2_cpu0_wr_cache_attr_arb_set            (l2_cpu0_wr_cache_attr_arb_set[2:0]),
                                 .l2_cpu0_wr_cl_id_arb_set                 (l2_cpu0_wr_cl_id_arb_set[11:0]),
                                 .l2_cpu0_wr_clean_evict_arb_set           (l2_cpu0_wr_clean_evict_arb_set),
                                 .l2_cpu0_wr_data                          (l2_cpu0_wr_data[143:0]),
                                 .l2_cpu0_wr_data_vld_x1_q                 (l2_cpu0_wr_data_vld_x1_q),
                                 .l2_cpu0_wr_dirty_arb_set                 (l2_cpu0_wr_dirty_arb_set),
                                 .l2_cpu0_wr_elem_size_arb_set             (l2_cpu0_wr_elem_size_arb_set[2:0]),
                                 .l2_cpu0_wr_err_arb_set                   (l2_cpu0_wr_err_arb_set),
                                 .l2_cpu0_wr_evict_x1_q                    (l2_cpu0_wr_evict_x1_q),
                                 .l2_cpu0_wr_id_arb_set                    (l2_cpu0_wr_id_arb_set[3:0]),
                                 .l2_cpu0_wr_last_arb_set                  (l2_cpu0_wr_last_arb_set),
                                 .l2_cpu0_wr_page_attr_arb_set             (l2_cpu0_wr_page_attr_arb_set[7:0]),
                                 .l2_cpu0_wr_partial_dw_arb_set            (l2_cpu0_wr_partial_dw_arb_set[3:0]),
                                 .l2_cpu0_wr_priv_arb_set                  (l2_cpu0_wr_priv_arb_set),
                                 .l2_cpu0_wr_shared_arb_set                (l2_cpu0_wr_shared_arb_set[1:0]),
                                 .l2_cpu0_wr_type_arb_set                  (l2_cpu0_wr_type_arb_set[2:0]),
                                 .l2_cpu0_wr_way_arb_set                   (l2_cpu0_wr_way_arb_set),
                                 .l2_cpu0_wrq_almost_full                  (l2_cpu0_wrq_almost_full),
                                 .l2_cpu0_wrq_haz_clr_id_dcd_q             (l2_cpu0_wrq_haz_clr_id_dcd_q[15:0]),
                                 .l2_cpu1_dsq_rd_byte_strb_q               (l2_cpu1_dsq_rd_byte_strb_q[15:0]),
                                 .l2_cpu1_dsq_rd_data_q                    (l2_cpu1_dsq_rd_data_q[129:0]),
                                 .l2_cpu1_dt_pmu_evt_en                    (l2_cpu1_dt_pmu_evt_en),
                                 .l2_cpu1_early_rd_reqe4_e5_q              (l2_cpu1_early_rd_reqe4_e5_q),
                                 .l2_cpu1_ic_addr_arb_set                  (l2_cpu1_ic_addr_arb_set[15:0]),
                                 .l2_cpu1_ic_arb_fast                      (l2_cpu1_ic_arb_fast),
                                 .l2_cpu1_ic_data_arb_set                  (l2_cpu1_ic_data_arb_set[31:0]),
                                 .l2_cpu1_ic_elem_size_arb_set             (l2_cpu1_ic_elem_size_arb_set[2:0]),
                                 .l2_cpu1_ic_excl_arb_set                  (l2_cpu1_ic_excl_arb_set),
                                 .l2_cpu1_ic_id_arb_set                    (l2_cpu1_ic_id_arb_set[2:0]),
                                 .l2_cpu1_ic_ns_arb_set                    (l2_cpu1_ic_ns_arb_set),
                                 .l2_cpu1_ic_write_arb_set                 (l2_cpu1_ic_write_arb_set),
                                 .l2_cpu1_idle_wakeup_q                    (l2_cpu1_idle_wakeup_q),
                                 .l2_cpu1_if_ccb_resp                      (l2_cpu1_if_ccb_resp),
                                 .l2_cpu1_if_ccb_resp_id                   (l2_cpu1_if_ccb_resp_id[4:0]),
                                 .l2_cpu1_if_sync_done_q                   (l2_cpu1_if_sync_done_q),
                                 .l2_cpu1_lrq_haz_clr_id_dcd_q             (l2_cpu1_lrq_haz_clr_id_dcd_q[5:0]),
                                 .l2_cpu1_ls_ccb_data_wr                   (l2_cpu1_ls_ccb_data_wr),
                                 .l2_cpu1_ls_ccb_resp                      (l2_cpu1_ls_ccb_resp),
                                 .l2_cpu1_ls_ccb_resp_id                   (l2_cpu1_ls_ccb_resp_id[4:0]),
                                 .l2_cpu1_ls_rd_haz_id_arb_q               (l2_cpu1_ls_rd_haz_id_arb_q[3:0]),
                                 .l2_cpu1_ls_rd_haz_vld_arb_q              (l2_cpu1_ls_rd_haz_vld_arb_q),
                                 .l2_cpu1_ls_wr_ccb_id_w2a                 (l2_cpu1_ls_wr_ccb_id_w2a[4:0]),
                                 .l2_cpu1_ls_wr_data_w2a                   (l2_cpu1_ls_wr_data_w2a[127:0]),
                                 .l2_cpu1_ls_wr_dirty_w2a                  (l2_cpu1_ls_wr_dirty_w2a),
                                 .l2_cpu1_ls_wr_err_w2a                    (l2_cpu1_ls_wr_err_w2a),
                                 .l2_cpu1_ls_wr_haz_id_arb_q               (l2_cpu1_ls_wr_haz_id_arb_q[2:0]),
                                 .l2_cpu1_ls_wr_haz_vld_arb_q              (l2_cpu1_ls_wr_haz_vld_arb_q),
                                 .l2_cpu1_ls_wr_last_w2a                   (l2_cpu1_ls_wr_last_w2a),
                                 .l2_cpu1_ls_wr_req_w2a                    (l2_cpu1_ls_wr_req_w2a),
                                 .l2_cpu1_ls_wr_type_w2a                   (l2_cpu1_ls_wr_type_w2a[2:0]),
                                 .l2_cpu1_rd_aarch64_arb_set               (l2_cpu1_rd_aarch64_arb_set),
                                 .l2_cpu1_rd_addr_arb_set                  (l2_cpu1_rd_addr_arb_set[44:0]),
                                 .l2_cpu1_rd_arb_fast                      (l2_cpu1_rd_arb_fast),
                                 .l2_cpu1_rd_asid_arb_set                  (l2_cpu1_rd_asid_arb_set[15:8]),
                                 .l2_cpu1_rd_bypass_arb_set                (l2_cpu1_rd_bypass_arb_set),
                                 .l2_cpu1_rd_bypass_bufid_e5               (l2_cpu1_rd_bypass_bufid_e5[2:0]),
                                 .l2_cpu1_rd_bypass_lrq_id_e5              (l2_cpu1_rd_bypass_lrq_id_e5[2:0]),
                                 .l2_cpu1_rd_bypass_req_can_e5             (l2_cpu1_rd_bypass_req_can_e5),
                                 .l2_cpu1_rd_bypass_way_e5                 (l2_cpu1_rd_bypass_way_e5),
                                 .l2_cpu1_rd_cache_attr_arb_set            (l2_cpu1_rd_cache_attr_arb_set[2:0]),
                                 .l2_cpu1_rd_elem_size_arb_set             (l2_cpu1_rd_elem_size_arb_set[2:0]),
                                 .l2_cpu1_rd_excl_arb_set                  (l2_cpu1_rd_excl_arb_set),
                                 .l2_cpu1_rd_id_arb_set                    (l2_cpu1_rd_id_arb_set[4:0]),
                                 .l2_cpu1_rd_lrq_id_arb_set                (l2_cpu1_rd_lrq_id_arb_set[2:0]),
                                 .l2_cpu1_rd_page_attr_arb_set             (l2_cpu1_rd_page_attr_arb_set[7:0]),
                                 .l2_cpu1_rd_prfm_arb_set                  (l2_cpu1_rd_prfm_arb_set),
                                 .l2_cpu1_rd_priv_arb_set                  (l2_cpu1_rd_priv_arb_set),
                                 .l2_cpu1_rd_replayed_arb_set              (l2_cpu1_rd_replayed_arb_set),
                                 .l2_cpu1_rd_shared_arb_set                (l2_cpu1_rd_shared_arb_set[1:0]),
                                 .l2_cpu1_rd_type_arb_set                  (l2_cpu1_rd_type_arb_set[6:0]),
                                 .l2_cpu1_rd_va48_arb_set                  (l2_cpu1_rd_va48_arb_set),
                                 .l2_cpu1_rd_way_arb_set                   (l2_cpu1_rd_way_arb_set),
                                 .l2_cpu1_tlb_sync_done_q                  (l2_cpu1_tlb_sync_done_q),
                                 .l2_cpu1_tw_ccb_resp                      (l2_cpu1_tw_ccb_resp),
                                 .l2_cpu1_tw_ccb_resp_id                   (l2_cpu1_tw_ccb_resp_id[4:0]),
                                 .l2_cpu1_wr_1st_replayed_arb_set          (l2_cpu1_wr_1st_replayed_arb_set),
                                 .l2_cpu1_wr_addr_arb_set                  (l2_cpu1_wr_addr_arb_set[44:0]),
                                 .l2_cpu1_wr_arb_fast                      (l2_cpu1_wr_arb_fast),
                                 .l2_cpu1_wr_cache_attr_arb_set            (l2_cpu1_wr_cache_attr_arb_set[2:0]),
                                 .l2_cpu1_wr_cl_id_arb_set                 (l2_cpu1_wr_cl_id_arb_set[11:0]),
                                 .l2_cpu1_wr_clean_evict_arb_set           (l2_cpu1_wr_clean_evict_arb_set),
                                 .l2_cpu1_wr_data                          (l2_cpu1_wr_data[143:0]),
                                 .l2_cpu1_wr_data_vld_x1_q                 (l2_cpu1_wr_data_vld_x1_q),
                                 .l2_cpu1_wr_dirty_arb_set                 (l2_cpu1_wr_dirty_arb_set),
                                 .l2_cpu1_wr_elem_size_arb_set             (l2_cpu1_wr_elem_size_arb_set[2:0]),
                                 .l2_cpu1_wr_err_arb_set                   (l2_cpu1_wr_err_arb_set),
                                 .l2_cpu1_wr_evict_x1_q                    (l2_cpu1_wr_evict_x1_q),
                                 .l2_cpu1_wr_id_arb_set                    (l2_cpu1_wr_id_arb_set[3:0]),
                                 .l2_cpu1_wr_last_arb_set                  (l2_cpu1_wr_last_arb_set),
                                 .l2_cpu1_wr_page_attr_arb_set             (l2_cpu1_wr_page_attr_arb_set[7:0]),
                                 .l2_cpu1_wr_partial_dw_arb_set            (l2_cpu1_wr_partial_dw_arb_set[3:0]),
                                 .l2_cpu1_wr_priv_arb_set                  (l2_cpu1_wr_priv_arb_set),
                                 .l2_cpu1_wr_shared_arb_set                (l2_cpu1_wr_shared_arb_set[1:0]),
                                 .l2_cpu1_wr_type_arb_set                  (l2_cpu1_wr_type_arb_set[2:0]),
                                 .l2_cpu1_wr_way_arb_set                   (l2_cpu1_wr_way_arb_set),
                                 .l2_cpu1_wrq_almost_full                  (l2_cpu1_wrq_almost_full),
                                 .l2_cpu1_wrq_haz_clr_id_dcd_q             (l2_cpu1_wrq_haz_clr_id_dcd_q[15:0]),
                                 .l2_cpu2_dsq_rd_byte_strb_q               (l2_cpu2_dsq_rd_byte_strb_q[15:0]),
                                 .l2_cpu2_dsq_rd_data_q                    (l2_cpu2_dsq_rd_data_q[129:0]),
                                 .l2_cpu2_dt_pmu_evt_en                    (l2_cpu2_dt_pmu_evt_en),
                                 .l2_cpu2_early_rd_reqe4_e5_q              (l2_cpu2_early_rd_reqe4_e5_q),
                                 .l2_cpu2_ic_addr_arb_set                  (l2_cpu2_ic_addr_arb_set[15:0]),
                                 .l2_cpu2_ic_arb_fast                      (l2_cpu2_ic_arb_fast),
                                 .l2_cpu2_ic_data_arb_set                  (l2_cpu2_ic_data_arb_set[31:0]),
                                 .l2_cpu2_ic_elem_size_arb_set             (l2_cpu2_ic_elem_size_arb_set[2:0]),
                                 .l2_cpu2_ic_excl_arb_set                  (l2_cpu2_ic_excl_arb_set),
                                 .l2_cpu2_ic_id_arb_set                    (l2_cpu2_ic_id_arb_set[2:0]),
                                 .l2_cpu2_ic_ns_arb_set                    (l2_cpu2_ic_ns_arb_set),
                                 .l2_cpu2_ic_write_arb_set                 (l2_cpu2_ic_write_arb_set),
                                 .l2_cpu2_idle_wakeup_q                    (l2_cpu2_idle_wakeup_q),
                                 .l2_cpu2_if_ccb_resp                      (l2_cpu2_if_ccb_resp),
                                 .l2_cpu2_if_ccb_resp_id                   (l2_cpu2_if_ccb_resp_id[4:0]),
                                 .l2_cpu2_if_sync_done_q                   (l2_cpu2_if_sync_done_q),
                                 .l2_cpu2_lrq_haz_clr_id_dcd_q             (l2_cpu2_lrq_haz_clr_id_dcd_q[5:0]),
                                 .l2_cpu2_ls_ccb_data_wr                   (l2_cpu2_ls_ccb_data_wr),
                                 .l2_cpu2_ls_ccb_resp                      (l2_cpu2_ls_ccb_resp),
                                 .l2_cpu2_ls_ccb_resp_id                   (l2_cpu2_ls_ccb_resp_id[4:0]),
                                 .l2_cpu2_ls_rd_haz_id_arb_q               (l2_cpu2_ls_rd_haz_id_arb_q[3:0]),
                                 .l2_cpu2_ls_rd_haz_vld_arb_q              (l2_cpu2_ls_rd_haz_vld_arb_q),
                                 .l2_cpu2_ls_wr_ccb_id_w2a                 (l2_cpu2_ls_wr_ccb_id_w2a[4:0]),
                                 .l2_cpu2_ls_wr_data_w2a                   (l2_cpu2_ls_wr_data_w2a[127:0]),
                                 .l2_cpu2_ls_wr_dirty_w2a                  (l2_cpu2_ls_wr_dirty_w2a),
                                 .l2_cpu2_ls_wr_err_w2a                    (l2_cpu2_ls_wr_err_w2a),
                                 .l2_cpu2_ls_wr_haz_id_arb_q               (l2_cpu2_ls_wr_haz_id_arb_q[2:0]),
                                 .l2_cpu2_ls_wr_haz_vld_arb_q              (l2_cpu2_ls_wr_haz_vld_arb_q),
                                 .l2_cpu2_ls_wr_last_w2a                   (l2_cpu2_ls_wr_last_w2a),
                                 .l2_cpu2_ls_wr_req_w2a                    (l2_cpu2_ls_wr_req_w2a),
                                 .l2_cpu2_ls_wr_type_w2a                   (l2_cpu2_ls_wr_type_w2a[2:0]),
                                 .l2_cpu2_rd_aarch64_arb_set               (l2_cpu2_rd_aarch64_arb_set),
                                 .l2_cpu2_rd_addr_arb_set                  (l2_cpu2_rd_addr_arb_set[44:0]),
                                 .l2_cpu2_rd_arb_fast                      (l2_cpu2_rd_arb_fast),
                                 .l2_cpu2_rd_asid_arb_set                  (l2_cpu2_rd_asid_arb_set[15:8]),
                                 .l2_cpu2_rd_bypass_arb_set                (l2_cpu2_rd_bypass_arb_set),
                                 .l2_cpu2_rd_bypass_bufid_e5               (l2_cpu2_rd_bypass_bufid_e5[2:0]),
                                 .l2_cpu2_rd_bypass_lrq_id_e5              (l2_cpu2_rd_bypass_lrq_id_e5[2:0]),
                                 .l2_cpu2_rd_bypass_req_can_e5             (l2_cpu2_rd_bypass_req_can_e5),
                                 .l2_cpu2_rd_bypass_way_e5                 (l2_cpu2_rd_bypass_way_e5),
                                 .l2_cpu2_rd_cache_attr_arb_set            (l2_cpu2_rd_cache_attr_arb_set[2:0]),
                                 .l2_cpu2_rd_elem_size_arb_set             (l2_cpu2_rd_elem_size_arb_set[2:0]),
                                 .l2_cpu2_rd_excl_arb_set                  (l2_cpu2_rd_excl_arb_set),
                                 .l2_cpu2_rd_id_arb_set                    (l2_cpu2_rd_id_arb_set[4:0]),
                                 .l2_cpu2_rd_lrq_id_arb_set                (l2_cpu2_rd_lrq_id_arb_set[2:0]),
                                 .l2_cpu2_rd_page_attr_arb_set             (l2_cpu2_rd_page_attr_arb_set[7:0]),
                                 .l2_cpu2_rd_prfm_arb_set                  (l2_cpu2_rd_prfm_arb_set),
                                 .l2_cpu2_rd_priv_arb_set                  (l2_cpu2_rd_priv_arb_set),
                                 .l2_cpu2_rd_replayed_arb_set              (l2_cpu2_rd_replayed_arb_set),
                                 .l2_cpu2_rd_shared_arb_set                (l2_cpu2_rd_shared_arb_set[1:0]),
                                 .l2_cpu2_rd_type_arb_set                  (l2_cpu2_rd_type_arb_set[6:0]),
                                 .l2_cpu2_rd_va48_arb_set                  (l2_cpu2_rd_va48_arb_set),
                                 .l2_cpu2_rd_way_arb_set                   (l2_cpu2_rd_way_arb_set),
                                 .l2_cpu2_tlb_sync_done_q                  (l2_cpu2_tlb_sync_done_q),
                                 .l2_cpu2_tw_ccb_resp                      (l2_cpu2_tw_ccb_resp),
                                 .l2_cpu2_tw_ccb_resp_id                   (l2_cpu2_tw_ccb_resp_id[4:0]),
                                 .l2_cpu2_wr_1st_replayed_arb_set          (l2_cpu2_wr_1st_replayed_arb_set),
                                 .l2_cpu2_wr_addr_arb_set                  (l2_cpu2_wr_addr_arb_set[44:0]),
                                 .l2_cpu2_wr_arb_fast                      (l2_cpu2_wr_arb_fast),
                                 .l2_cpu2_wr_cache_attr_arb_set            (l2_cpu2_wr_cache_attr_arb_set[2:0]),
                                 .l2_cpu2_wr_cl_id_arb_set                 (l2_cpu2_wr_cl_id_arb_set[11:0]),
                                 .l2_cpu2_wr_clean_evict_arb_set           (l2_cpu2_wr_clean_evict_arb_set),
                                 .l2_cpu2_wr_data                          (l2_cpu2_wr_data[143:0]),
                                 .l2_cpu2_wr_data_vld_x1_q                 (l2_cpu2_wr_data_vld_x1_q),
                                 .l2_cpu2_wr_dirty_arb_set                 (l2_cpu2_wr_dirty_arb_set),
                                 .l2_cpu2_wr_elem_size_arb_set             (l2_cpu2_wr_elem_size_arb_set[2:0]),
                                 .l2_cpu2_wr_err_arb_set                   (l2_cpu2_wr_err_arb_set),
                                 .l2_cpu2_wr_evict_x1_q                    (l2_cpu2_wr_evict_x1_q),
                                 .l2_cpu2_wr_id_arb_set                    (l2_cpu2_wr_id_arb_set[3:0]),
                                 .l2_cpu2_wr_last_arb_set                  (l2_cpu2_wr_last_arb_set),
                                 .l2_cpu2_wr_page_attr_arb_set             (l2_cpu2_wr_page_attr_arb_set[7:0]),
                                 .l2_cpu2_wr_partial_dw_arb_set            (l2_cpu2_wr_partial_dw_arb_set[3:0]),
                                 .l2_cpu2_wr_priv_arb_set                  (l2_cpu2_wr_priv_arb_set),
                                 .l2_cpu2_wr_shared_arb_set                (l2_cpu2_wr_shared_arb_set[1:0]),
                                 .l2_cpu2_wr_type_arb_set                  (l2_cpu2_wr_type_arb_set[2:0]),
                                 .l2_cpu2_wr_way_arb_set                   (l2_cpu2_wr_way_arb_set),
                                 .l2_cpu2_wrq_almost_full                  (l2_cpu2_wrq_almost_full),
                                 .l2_cpu2_wrq_haz_clr_id_dcd_q             (l2_cpu2_wrq_haz_clr_id_dcd_q[15:0]),
                                 .l2_cpu3_dsq_rd_byte_strb_q               (l2_cpu3_dsq_rd_byte_strb_q[15:0]),
                                 .l2_cpu3_dsq_rd_data_q                    (l2_cpu3_dsq_rd_data_q[129:0]),
                                 .l2_cpu3_dt_pmu_evt_en                    (l2_cpu3_dt_pmu_evt_en),
                                 .l2_cpu3_early_rd_reqe4_e5_q              (l2_cpu3_early_rd_reqe4_e5_q),
                                 .l2_cpu3_ic_addr_arb_set                  (l2_cpu3_ic_addr_arb_set[15:0]),
                                 .l2_cpu3_ic_arb_fast                      (l2_cpu3_ic_arb_fast),
                                 .l2_cpu3_ic_data_arb_set                  (l2_cpu3_ic_data_arb_set[31:0]),
                                 .l2_cpu3_ic_elem_size_arb_set             (l2_cpu3_ic_elem_size_arb_set[2:0]),
                                 .l2_cpu3_ic_excl_arb_set                  (l2_cpu3_ic_excl_arb_set),
                                 .l2_cpu3_ic_id_arb_set                    (l2_cpu3_ic_id_arb_set[2:0]),
                                 .l2_cpu3_ic_ns_arb_set                    (l2_cpu3_ic_ns_arb_set),
                                 .l2_cpu3_ic_write_arb_set                 (l2_cpu3_ic_write_arb_set),
                                 .l2_cpu3_idle_wakeup_q                    (l2_cpu3_idle_wakeup_q),
                                 .l2_cpu3_if_ccb_resp                      (l2_cpu3_if_ccb_resp),
                                 .l2_cpu3_if_ccb_resp_id                   (l2_cpu3_if_ccb_resp_id[4:0]),
                                 .l2_cpu3_if_sync_done_q                   (l2_cpu3_if_sync_done_q),
                                 .l2_cpu3_lrq_haz_clr_id_dcd_q             (l2_cpu3_lrq_haz_clr_id_dcd_q[5:0]),
                                 .l2_cpu3_ls_ccb_data_wr                   (l2_cpu3_ls_ccb_data_wr),
                                 .l2_cpu3_ls_ccb_resp                      (l2_cpu3_ls_ccb_resp),
                                 .l2_cpu3_ls_ccb_resp_id                   (l2_cpu3_ls_ccb_resp_id[4:0]),
                                 .l2_cpu3_ls_rd_haz_id_arb_q               (l2_cpu3_ls_rd_haz_id_arb_q[3:0]),
                                 .l2_cpu3_ls_rd_haz_vld_arb_q              (l2_cpu3_ls_rd_haz_vld_arb_q),
                                 .l2_cpu3_ls_wr_ccb_id_w2a                 (l2_cpu3_ls_wr_ccb_id_w2a[4:0]),
                                 .l2_cpu3_ls_wr_data_w2a                   (l2_cpu3_ls_wr_data_w2a[127:0]),
                                 .l2_cpu3_ls_wr_dirty_w2a                  (l2_cpu3_ls_wr_dirty_w2a),
                                 .l2_cpu3_ls_wr_err_w2a                    (l2_cpu3_ls_wr_err_w2a),
                                 .l2_cpu3_ls_wr_haz_id_arb_q               (l2_cpu3_ls_wr_haz_id_arb_q[2:0]),
                                 .l2_cpu3_ls_wr_haz_vld_arb_q              (l2_cpu3_ls_wr_haz_vld_arb_q),
                                 .l2_cpu3_ls_wr_last_w2a                   (l2_cpu3_ls_wr_last_w2a),
                                 .l2_cpu3_ls_wr_req_w2a                    (l2_cpu3_ls_wr_req_w2a),
                                 .l2_cpu3_ls_wr_type_w2a                   (l2_cpu3_ls_wr_type_w2a[2:0]),
                                 .l2_cpu3_rd_aarch64_arb_set               (l2_cpu3_rd_aarch64_arb_set),
                                 .l2_cpu3_rd_addr_arb_set                  (l2_cpu3_rd_addr_arb_set[44:0]),
                                 .l2_cpu3_rd_arb_fast                      (l2_cpu3_rd_arb_fast),
                                 .l2_cpu3_rd_asid_arb_set                  (l2_cpu3_rd_asid_arb_set[15:8]),
                                 .l2_cpu3_rd_bypass_arb_set                (l2_cpu3_rd_bypass_arb_set),
                                 .l2_cpu3_rd_bypass_bufid_e5               (l2_cpu3_rd_bypass_bufid_e5[2:0]),
                                 .l2_cpu3_rd_bypass_lrq_id_e5              (l2_cpu3_rd_bypass_lrq_id_e5[2:0]),
                                 .l2_cpu3_rd_bypass_req_can_e5             (l2_cpu3_rd_bypass_req_can_e5),
                                 .l2_cpu3_rd_bypass_way_e5                 (l2_cpu3_rd_bypass_way_e5),
                                 .l2_cpu3_rd_cache_attr_arb_set            (l2_cpu3_rd_cache_attr_arb_set[2:0]),
                                 .l2_cpu3_rd_elem_size_arb_set             (l2_cpu3_rd_elem_size_arb_set[2:0]),
                                 .l2_cpu3_rd_excl_arb_set                  (l2_cpu3_rd_excl_arb_set),
                                 .l2_cpu3_rd_id_arb_set                    (l2_cpu3_rd_id_arb_set[4:0]),
                                 .l2_cpu3_rd_lrq_id_arb_set                (l2_cpu3_rd_lrq_id_arb_set[2:0]),
                                 .l2_cpu3_rd_page_attr_arb_set             (l2_cpu3_rd_page_attr_arb_set[7:0]),
                                 .l2_cpu3_rd_prfm_arb_set                  (l2_cpu3_rd_prfm_arb_set),
                                 .l2_cpu3_rd_priv_arb_set                  (l2_cpu3_rd_priv_arb_set),
                                 .l2_cpu3_rd_replayed_arb_set              (l2_cpu3_rd_replayed_arb_set),
                                 .l2_cpu3_rd_shared_arb_set                (l2_cpu3_rd_shared_arb_set[1:0]),
                                 .l2_cpu3_rd_type_arb_set                  (l2_cpu3_rd_type_arb_set[6:0]),
                                 .l2_cpu3_rd_va48_arb_set                  (l2_cpu3_rd_va48_arb_set),
                                 .l2_cpu3_rd_way_arb_set                   (l2_cpu3_rd_way_arb_set),
                                 .l2_cpu3_tlb_sync_done_q                  (l2_cpu3_tlb_sync_done_q),
                                 .l2_cpu3_tw_ccb_resp                      (l2_cpu3_tw_ccb_resp),
                                 .l2_cpu3_tw_ccb_resp_id                   (l2_cpu3_tw_ccb_resp_id[4:0]),
                                 .l2_cpu3_wr_1st_replayed_arb_set          (l2_cpu3_wr_1st_replayed_arb_set),
                                 .l2_cpu3_wr_addr_arb_set                  (l2_cpu3_wr_addr_arb_set[44:0]),
                                 .l2_cpu3_wr_arb_fast                      (l2_cpu3_wr_arb_fast),
                                 .l2_cpu3_wr_cache_attr_arb_set            (l2_cpu3_wr_cache_attr_arb_set[2:0]),
                                 .l2_cpu3_wr_cl_id_arb_set                 (l2_cpu3_wr_cl_id_arb_set[11:0]),
                                 .l2_cpu3_wr_clean_evict_arb_set           (l2_cpu3_wr_clean_evict_arb_set),
                                 .l2_cpu3_wr_data                          (l2_cpu3_wr_data[143:0]),
                                 .l2_cpu3_wr_data_vld_x1_q                 (l2_cpu3_wr_data_vld_x1_q),
                                 .l2_cpu3_wr_dirty_arb_set                 (l2_cpu3_wr_dirty_arb_set),
                                 .l2_cpu3_wr_elem_size_arb_set             (l2_cpu3_wr_elem_size_arb_set[2:0]),
                                 .l2_cpu3_wr_err_arb_set                   (l2_cpu3_wr_err_arb_set),
                                 .l2_cpu3_wr_evict_x1_q                    (l2_cpu3_wr_evict_x1_q),
                                 .l2_cpu3_wr_id_arb_set                    (l2_cpu3_wr_id_arb_set[3:0]),
                                 .l2_cpu3_wr_last_arb_set                  (l2_cpu3_wr_last_arb_set),
                                 .l2_cpu3_wr_page_attr_arb_set             (l2_cpu3_wr_page_attr_arb_set[7:0]),
                                 .l2_cpu3_wr_partial_dw_arb_set            (l2_cpu3_wr_partial_dw_arb_set[3:0]),
                                 .l2_cpu3_wr_priv_arb_set                  (l2_cpu3_wr_priv_arb_set),
                                 .l2_cpu3_wr_shared_arb_set                (l2_cpu3_wr_shared_arb_set[1:0]),
                                 .l2_cpu3_wr_type_arb_set                  (l2_cpu3_wr_type_arb_set[2:0]),
                                 .l2_cpu3_wr_way_arb_set                   (l2_cpu3_wr_way_arb_set),
                                 .l2_cpu3_wrq_almost_full                  (l2_cpu3_wrq_almost_full),
                                 .l2_cpu3_wrq_haz_clr_id_dcd_q             (l2_cpu3_wrq_haz_clr_id_dcd_q[15:0]),
                                 .ls_cpu0_clrexmon                         (ls_cpu0_clrexmon),
                                 .ls_cpu0_imp_abort_containable            (ls_cpu0_imp_abort_containable),
                                 .ls_cpu0_imp_abort_dec                    (ls_cpu0_imp_abort_dec),
                                 .ls_cpu0_imp_abort_ecc                    (ls_cpu0_imp_abort_ecc),
                                 .ls_cpu0_imp_abort_slv                    (ls_cpu0_imp_abort_slv),
                                 .ls_cpu0_raw_eae_nonsec                   (ls_cpu0_raw_eae_nonsec),
                                 .ls_cpu0_raw_eae_secure                   (ls_cpu0_raw_eae_secure),
                                 .ls_cpu1_clrexmon                         (ls_cpu1_clrexmon),
                                 .ls_cpu1_imp_abort_containable            (ls_cpu1_imp_abort_containable),
                                 .ls_cpu1_imp_abort_dec                    (ls_cpu1_imp_abort_dec),
                                 .ls_cpu1_imp_abort_ecc                    (ls_cpu1_imp_abort_ecc),
                                 .ls_cpu1_imp_abort_slv                    (ls_cpu1_imp_abort_slv),
                                 .ls_cpu1_raw_eae_nonsec                   (ls_cpu1_raw_eae_nonsec),
                                 .ls_cpu1_raw_eae_secure                   (ls_cpu1_raw_eae_secure),
                                 .ls_cpu2_clrexmon                         (ls_cpu2_clrexmon),
                                 .ls_cpu2_imp_abort_containable            (ls_cpu2_imp_abort_containable),
                                 .ls_cpu2_imp_abort_dec                    (ls_cpu2_imp_abort_dec),
                                 .ls_cpu2_imp_abort_ecc                    (ls_cpu2_imp_abort_ecc),
                                 .ls_cpu2_imp_abort_slv                    (ls_cpu2_imp_abort_slv),
                                 .ls_cpu2_raw_eae_nonsec                   (ls_cpu2_raw_eae_nonsec),
                                 .ls_cpu2_raw_eae_secure                   (ls_cpu2_raw_eae_secure),
                                 .ls_cpu3_clrexmon                         (ls_cpu3_clrexmon),
                                 .ls_cpu3_imp_abort_containable            (ls_cpu3_imp_abort_containable),
                                 .ls_cpu3_imp_abort_dec                    (ls_cpu3_imp_abort_dec),
                                 .ls_cpu3_imp_abort_ecc                    (ls_cpu3_imp_abort_ecc),
                                 .ls_cpu3_imp_abort_slv                    (ls_cpu3_imp_abort_slv),
                                 .ls_cpu3_raw_eae_nonsec                   (ls_cpu3_raw_eae_nonsec),
                                 .ls_cpu3_raw_eae_secure                   (ls_cpu3_raw_eae_secure),
                                 .nCORERESET                               (nCORERESET[`MAIA_CN:0]),
                                 .nCPUPORESET                              (nCPUPORESET[`MAIA_CN:0]),
                                 .nFIQ                                     (nFIQ[`MAIA_CN:0]),
                                 .nIRQ                                     (nIRQ[`MAIA_CN:0]),
                                 .nL2RESET                                 (nL2RESET),
                                 .nMBISTRESET                              (nMBISTRESET),
                                 .nPRESETDBG                               (nPRESETDBG),
                                 .nREI                                     (nREI[`MAIA_CN:0]),
                                 .nSEI                                     (nSEI[`MAIA_CN:0]),
                                 .nVFIQ                                    (nVFIQ[`MAIA_CN:0]),
                                 .nVIRQ                                    (nVIRQ[`MAIA_CN:0]),
                                 .nVSEI                                    (nVSEI[`MAIA_CN:0]),
                                 .ncommirq_cpu0_i                          (ncommirq_cpu0_i),
                                 .ncommirq_cpu1_i                          (ncommirq_cpu1_i),
                                 .ncommirq_cpu2_i                          (ncommirq_cpu2_i),
                                 .ncommirq_cpu3_i                          (ncommirq_cpu3_i),
                                 .npmuirq_cpu0_i                           (npmuirq_cpu0_i),
                                 .npmuirq_cpu1_i                           (npmuirq_cpu1_i),
                                 .npmuirq_cpu2_i                           (npmuirq_cpu2_i),
                                 .npmuirq_cpu3_i                           (npmuirq_cpu3_i),
                                 .pm_export_cpu0_i                         (pm_export_cpu0_i),
                                 .pm_export_cpu1_i                         (pm_export_cpu1_i),
                                 .pm_export_cpu2_i                         (pm_export_cpu2_i),
                                 .pm_export_cpu3_i                         (pm_export_cpu3_i),
                                 .pmuevent_cpu0_i                          (pmuevent_cpu0_i[24:0]),
                                 .pmuevent_cpu1_i                          (pmuevent_cpu1_i[24:0]),
                                 .pmuevent_cpu2_i                          (pmuevent_cpu2_i[24:0]),
                                 .pmuevent_cpu3_i                          (pmuevent_cpu3_i[24:0])
                               );    // unoncpu
endmodule    // MAIA_feq28_s


//ARMAUTO UNDEF START
`define MAIA_UNDEFINE
`include "maia_header.v"
`undef MAIA_UNDEFINE
//ARMAUTO UNDEF END
