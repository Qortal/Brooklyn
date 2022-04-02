`include "settings.h"

module Forwarding_Unit (
    input enable,
    input  [`REG_FILE_DEPTH-1:0] src1,
    input  [`REG_FILE_DEPTH-1:0] src2,
    input  [`REG_FILE_DEPTH-1:0] MEM_dest,
    input  [`REG_FILE_DEPTH-1:0] WB_dest,
    input        MEM_WB_en,
    input        WB_WB_en,
    output reg [1:0] sel_src1,
    output reg [1:0] sel_src2
);

  always @(*) begin
    sel_src1 = `FORWARDING_SEL_NOP;
    sel_src2 = `FORWARDING_SEL_NOP;
    if(enable) begin
        if ((src1 == MEM_dest) && (MEM_WB_en == 1'b1))
            sel_src1 = `FORWARDING_SEL_MEM;
        else if ((src1 == WB_dest) && (WB_WB_en == 1'b1))
            sel_src1 = `FORWARDING_SEL_WB;
        if ((src2 == MEM_dest) && (MEM_WB_en == 1'b1))
            sel_src2 = `FORWARDING_SEL_MEM;
        else if ((src2 == WB_dest) && (WB_WB_en == 1'b1))
            sel_src2 = `FORWARDING_SEL_WB;
    end
  end

endmodule
