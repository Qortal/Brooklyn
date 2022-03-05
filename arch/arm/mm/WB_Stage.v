`include "settings.h"

module WB_Stage
(
    input                    clk,
    input                    rst,
    input [`REG_FILE_DEPTH-1:0] dst,
    input [`WORD_WIDTH-1:0] ALU_res,
    input [`WORD_WIDTH-1:0] mem,
    input mem_read, WB_en,

    output [`REG_FILE_DEPTH-1:0] WB_Dest,
    output WB_en_out,
    output [`WORD_WIDTH-1:0] WB_Value
);

assign WB_Dest = dst;
assign WB_en_out = WB_en;

MUX_2_to_1 #(.WORD_WIDTH(`WORD_WIDTH)) MUX_2_to_1_Reg_File (
		.in1(ALU_res), .in2(mem),
		.sel(mem_read),
		.out(WB_Value)
	);

endmodule



