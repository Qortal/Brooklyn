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


module herculesae_vx_pmull
(


input wire          clk,
      input wire   reset,
input wire          ival_v1_q,

input wire  [63:0]  opa_v1,
input wire  [63:0]  opb_v1,

output reg  [127:0]  pmullout_v2_q 
);







  wire [63:0]                            a_in;
  wire [63:0]                            b_in;
  wire [127:0]                           p_out;
  wire [63:0]                            pp0;
  wire [63:0]                            pp1;
  wire [63:0]                            pp2;
  wire [63:0]                            pp3;
  wire [63:0]                            pp4;
  wire [63:0]                            pp5;
  wire [63:0]                            pp6;
  wire [63:0]                            pp7;
  wire [63:0]                            pp8;
  wire [63:0]                            pp9;
  wire [63:0]                            pp10;
  wire [63:0]                            pp11;
  wire [63:0]                            pp12;
  wire [63:0]                            pp13;
  wire [63:0]                            pp14;
  wire [63:0]                            pp15;
  wire [63:0]                            pp16;
  wire [63:0]                            pp17;
  wire [63:0]                            pp18;
  wire [63:0]                            pp19;
  wire [63:0]                            pp20;
  wire [63:0]                            pp21;
  wire [63:0]                            pp22;
  wire [63:0]                            pp23;
  wire [63:0]                            pp24;
  wire [63:0]                            pp25;
  wire [63:0]                            pp26;
  wire [63:0]                            pp27;
  wire [63:0]                            pp28;
  wire [63:0]                            pp29;
  wire [63:0]                            pp30;
  wire [63:0]                            pp31;
  wire [63:0]                            pp32;
  wire [63:0]                            pp33;
  wire [63:0]                            pp34;
  wire [63:0]                            pp35;
  wire [63:0]                            pp36;
  wire [63:0]                            pp37;
  wire [63:0]                            pp38;
  wire [63:0]                            pp39;
  wire [63:0]                            pp40;
  wire [63:0]                            pp41;
  wire [63:0]                            pp42;
  wire [63:0]                            pp43;
  wire [63:0]                            pp44;
  wire [63:0]                            pp45;
  wire [63:0]                            pp46;
  wire [63:0]                            pp47;
  wire [63:0]                            pp48;
  wire [63:0]                            pp49;
  wire [63:0]                            pp50;
  wire [63:0]                            pp51;
  wire [63:0]                            pp52;
  wire [63:0]                            pp53;
  wire [63:0]                            pp54;
  wire [63:0]                            pp55;
  wire [63:0]                            pp56;
  wire [63:0]                            pp57;
  wire [63:0]                            pp58;
  wire [63:0]                            pp59;
  wire [63:0]                            pp60;
  wire [63:0]                            pp61;
  wire [63:0]                            pp62;
  wire [63:0]                            pp63;
  wire [66:0]                            rednl0_0;
  wire [66:0]                            rednl0_1;
  wire [66:0]                            rednl0_2;
  wire [66:0]                            rednl0_3;
  wire [66:0]                            rednl0_4;
  wire [66:0]                            rednl0_5;
  wire [66:0]                            rednl0_6;
  wire [66:0]                            rednl0_7;
  wire [66:0]                            rednl0_8;
  wire [66:0]                            rednl0_9;
  wire [66:0]                            rednl0_10;
  wire [66:0]                            rednl0_11;
  wire [66:0]                            rednl0_12;
  wire [66:0]                            rednl0_13;
  wire [66:0]                            rednl0_14;
  wire [66:0]                            rednl0_15;
  wire [78:0]                            rednl1_0;
  wire [78:0]                            rednl1_1;
  wire [78:0]                            rednl1_2;
  wire [78:0]                            rednl1_3;


assign a_in[63:0] = opa_v1[63:0];
assign b_in[63:0] = opb_v1[63:0];
assign  pp0[63:0] = {64{a_in[ 0]}} & b_in[63:0];
assign  pp1[63:0] = {64{a_in[ 1]}} & b_in[63:0];
assign  pp2[63:0] = {64{a_in[ 2]}} & b_in[63:0];
assign  pp3[63:0] = {64{a_in[ 3]}} & b_in[63:0];
assign  pp4[63:0] = {64{a_in[ 4]}} & b_in[63:0];
assign  pp5[63:0] = {64{a_in[ 5]}} & b_in[63:0];
assign  pp6[63:0] = {64{a_in[ 6]}} & b_in[63:0];
assign  pp7[63:0] = {64{a_in[ 7]}} & b_in[63:0];
assign  pp8[63:0] = {64{a_in[ 8]}} & b_in[63:0];
assign  pp9[63:0] = {64{a_in[ 9]}} & b_in[63:0];
assign pp10[63:0] = {64{a_in[10]}} & b_in[63:0];
assign pp11[63:0] = {64{a_in[11]}} & b_in[63:0];
assign pp12[63:0] = {64{a_in[12]}} & b_in[63:0];
assign pp13[63:0] = {64{a_in[13]}} & b_in[63:0];
assign pp14[63:0] = {64{a_in[14]}} & b_in[63:0];
assign pp15[63:0] = {64{a_in[15]}} & b_in[63:0];
assign pp16[63:0] = {64{a_in[16]}} & b_in[63:0];
assign pp17[63:0] = {64{a_in[17]}} & b_in[63:0];
assign pp18[63:0] = {64{a_in[18]}} & b_in[63:0];
assign pp19[63:0] = {64{a_in[19]}} & b_in[63:0];
assign pp20[63:0] = {64{a_in[20]}} & b_in[63:0];
assign pp21[63:0] = {64{a_in[21]}} & b_in[63:0];
assign pp22[63:0] = {64{a_in[22]}} & b_in[63:0];
assign pp23[63:0] = {64{a_in[23]}} & b_in[63:0];
assign pp24[63:0] = {64{a_in[24]}} & b_in[63:0];
assign pp25[63:0] = {64{a_in[25]}} & b_in[63:0];
assign pp26[63:0] = {64{a_in[26]}} & b_in[63:0];
assign pp27[63:0] = {64{a_in[27]}} & b_in[63:0];
assign pp28[63:0] = {64{a_in[28]}} & b_in[63:0];
assign pp29[63:0] = {64{a_in[29]}} & b_in[63:0];
assign pp30[63:0] = {64{a_in[30]}} & b_in[63:0];
assign pp31[63:0] = {64{a_in[31]}} & b_in[63:0];
assign pp32[63:0] = {64{a_in[32]}} & b_in[63:0];
assign pp33[63:0] = {64{a_in[33]}} & b_in[63:0];
assign pp34[63:0] = {64{a_in[34]}} & b_in[63:0];
assign pp35[63:0] = {64{a_in[35]}} & b_in[63:0];
assign pp36[63:0] = {64{a_in[36]}} & b_in[63:0];
assign pp37[63:0] = {64{a_in[37]}} & b_in[63:0];
assign pp38[63:0] = {64{a_in[38]}} & b_in[63:0];
assign pp39[63:0] = {64{a_in[39]}} & b_in[63:0];
assign pp40[63:0] = {64{a_in[40]}} & b_in[63:0];
assign pp41[63:0] = {64{a_in[41]}} & b_in[63:0];
assign pp42[63:0] = {64{a_in[42]}} & b_in[63:0];
assign pp43[63:0] = {64{a_in[43]}} & b_in[63:0];
assign pp44[63:0] = {64{a_in[44]}} & b_in[63:0];
assign pp45[63:0] = {64{a_in[45]}} & b_in[63:0];
assign pp46[63:0] = {64{a_in[46]}} & b_in[63:0];
assign pp47[63:0] = {64{a_in[47]}} & b_in[63:0];
assign pp48[63:0] = {64{a_in[48]}} & b_in[63:0];
assign pp49[63:0] = {64{a_in[49]}} & b_in[63:0];
assign pp50[63:0] = {64{a_in[50]}} & b_in[63:0];
assign pp51[63:0] = {64{a_in[51]}} & b_in[63:0];
assign pp52[63:0] = {64{a_in[52]}} & b_in[63:0];
assign pp53[63:0] = {64{a_in[53]}} & b_in[63:0];
assign pp54[63:0] = {64{a_in[54]}} & b_in[63:0];
assign pp55[63:0] = {64{a_in[55]}} & b_in[63:0];
assign pp56[63:0] = {64{a_in[56]}} & b_in[63:0];
assign pp57[63:0] = {64{a_in[57]}} & b_in[63:0];
assign pp58[63:0] = {64{a_in[58]}} & b_in[63:0];
assign pp59[63:0] = {64{a_in[59]}} & b_in[63:0];
assign pp60[63:0] = {64{a_in[60]}} & b_in[63:0];
assign pp61[63:0] = {64{a_in[61]}} & b_in[63:0];
assign pp62[63:0] = {64{a_in[62]}} & b_in[63:0];
assign pp63[63:0] = {64{a_in[63]}} & b_in[63:0];

assign  rednl0_0[66:0] = { pp3[63:0], 3'b000} ^ {1'b0,  pp2[63:0], 2'b00} ^ {2'b00,  pp1[63:0], 1'b0} ^ {3'b000,  pp0[63:0]};
assign  rednl0_1[66:0] = { pp7[63:0], 3'b000} ^ {1'b0,  pp6[63:0], 2'b00} ^ {2'b00,  pp5[63:0], 1'b0} ^ {3'b000,  pp4[63:0]};
assign  rednl0_2[66:0] = {pp11[63:0], 3'b000} ^ {1'b0, pp10[63:0], 2'b00} ^ {2'b00,  pp9[63:0], 1'b0} ^ {3'b000,  pp8[63:0]};
assign  rednl0_3[66:0] = {pp15[63:0], 3'b000} ^ {1'b0, pp14[63:0], 2'b00} ^ {2'b00, pp13[63:0], 1'b0} ^ {3'b000, pp12[63:0]};
assign  rednl0_4[66:0] = {pp19[63:0], 3'b000} ^ {1'b0, pp18[63:0], 2'b00} ^ {2'b00, pp17[63:0], 1'b0} ^ {3'b000, pp16[63:0]};
assign  rednl0_5[66:0] = {pp23[63:0], 3'b000} ^ {1'b0, pp22[63:0], 2'b00} ^ {2'b00, pp21[63:0], 1'b0} ^ {3'b000, pp20[63:0]};
assign  rednl0_6[66:0] = {pp27[63:0], 3'b000} ^ {1'b0, pp26[63:0], 2'b00} ^ {2'b00, pp25[63:0], 1'b0} ^ {3'b000, pp24[63:0]};
assign  rednl0_7[66:0] = {pp31[63:0], 3'b000} ^ {1'b0, pp30[63:0], 2'b00} ^ {2'b00, pp29[63:0], 1'b0} ^ {3'b000, pp28[63:0]};
assign  rednl0_8[66:0] = {pp35[63:0], 3'b000} ^ {1'b0, pp34[63:0], 2'b00} ^ {2'b00, pp33[63:0], 1'b0} ^ {3'b000, pp32[63:0]};
assign  rednl0_9[66:0] = {pp39[63:0], 3'b000} ^ {1'b0, pp38[63:0], 2'b00} ^ {2'b00, pp37[63:0], 1'b0} ^ {3'b000, pp36[63:0]};
assign rednl0_10[66:0] = {pp43[63:0], 3'b000} ^ {1'b0, pp42[63:0], 2'b00} ^ {2'b00, pp41[63:0], 1'b0} ^ {3'b000, pp40[63:0]};
assign rednl0_11[66:0] = {pp47[63:0], 3'b000} ^ {1'b0, pp46[63:0], 2'b00} ^ {2'b00, pp45[63:0], 1'b0} ^ {3'b000, pp44[63:0]};
assign rednl0_12[66:0] = {pp51[63:0], 3'b000} ^ {1'b0, pp50[63:0], 2'b00} ^ {2'b00, pp49[63:0], 1'b0} ^ {3'b000, pp48[63:0]};
assign rednl0_13[66:0] = {pp55[63:0], 3'b000} ^ {1'b0, pp54[63:0], 2'b00} ^ {2'b00, pp53[63:0], 1'b0} ^ {3'b000, pp52[63:0]};
assign rednl0_14[66:0] = {pp59[63:0], 3'b000} ^ {1'b0, pp58[63:0], 2'b00} ^ {2'b00, pp57[63:0], 1'b0} ^ {3'b000, pp56[63:0]};
assign rednl0_15[66:0] = {pp63[63:0], 3'b000} ^ {1'b0, pp62[63:0], 2'b00} ^ {2'b00, pp61[63:0], 1'b0} ^ {3'b000, pp60[63:0]};

assign  rednl1_0[78:0] = { rednl0_3[66:0], 12'h000} ^ {4'h0,  rednl0_2[66:0], 8'h00} ^ {8'h00,  rednl0_1[66:0], 4'h0} ^ {12'h000,  rednl0_0[66:0]};
assign  rednl1_1[78:0] = { rednl0_7[66:0], 12'h000} ^ {4'h0,  rednl0_6[66:0], 8'h00} ^ {8'h00,  rednl0_5[66:0], 4'h0} ^ {12'h000,  rednl0_4[66:0]};
assign  rednl1_2[78:0] = {rednl0_11[66:0], 12'h000} ^ {4'h0, rednl0_10[66:0], 8'h00} ^ {8'h00,  rednl0_9[66:0], 4'h0} ^ {12'h000,  rednl0_8[66:0]};
assign  rednl1_3[78:0] = {rednl0_15[66:0], 12'h000} ^ {4'h0, rednl0_14[66:0], 8'h00} ^ {8'h00, rednl0_13[66:0], 4'h0} ^ {12'h000, rednl0_12[66:0]};

assign  p_out[15:  0] =                                                              rednl1_0[15: 0];
assign  p_out[31: 16] =                                            rednl1_1[15: 0] ^ rednl1_0[31:16];
assign  p_out[47: 32] =                          rednl1_2[15: 0] ^ rednl1_1[31:16] ^ rednl1_0[47:32];
assign  p_out[78: 48] =        rednl1_3[30: 0] ^ rednl1_2[46:16] ^ rednl1_1[62:32] ^ rednl1_0[78:48];
assign  p_out[94: 79] =        rednl1_3[46:31] ^ rednl1_2[62:47] ^ rednl1_1[78:63];
assign p_out[110: 95] =        rednl1_3[62:47] ^ rednl1_2[78:63];
assign p_out[127:111] = {1'b0, rednl1_3[78:63]};


  always_ff @(posedge clk or posedge reset)
  begin: u_pmullout_v2_q_127_0
    if (reset == 1'b1)
      pmullout_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'b0}};
`ifdef HERCULESAE_XPROP_FLOP
    else if (reset == 1'b0 && ival_v1_q == 1'b1)
      pmullout_v2_q[127:0] <= `HERCULESAE_DFF_DELAY p_out[127:0];
    else if (reset == 1'b0 && ival_v1_q == 1'b0)
    begin
    end
    else
      pmullout_v2_q[127:0] <= `HERCULESAE_DFF_DELAY {128{1'bx}};
`else
    else if (ival_v1_q == 1'b1)
      pmullout_v2_q[127:0] <= `HERCULESAE_DFF_DELAY p_out[127:0];
`endif
  end


endmodule


`define HERCULESAE_UNDEFINE
`include "herculesae_header.sv"
`undef HERCULESAE_UNDEFINE
