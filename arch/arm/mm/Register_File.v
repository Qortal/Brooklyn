`include "settings.h"

module Register_File
(
    input                        clk,
    input                        rst,
    input                        WB_en,
    input  [`REG_FILE_DEPTH-1:0] src1,
    input  [`REG_FILE_DEPTH-1:0] src2,
    input  [`REG_FILE_DEPTH-1:0] WB_dest,
    input  [`WORD_WIDTH-1:0]     WB_result,
    output [`WORD_WIDTH-1:0]     reg1,
    output [`WORD_WIDTH-1:0]     reg2
);

  reg [`WORD_WIDTH-1:0] reg_data [0:`REG_FILE_SIZE-1];

	integer i;

	always @(negedge clk, posedge rst)
	begin
		if (rst)
			begin
				for (i=0; i < `REG_FILE_SIZE; i = i+1)
					reg_data [i] <= i;
			end
		else
			begin
				if (WB_en)
						reg_data[WB_dest] <= WB_result;
			end
	end

	assign reg1 = reg_data[src1];
	assign reg2 = reg_data[src2];

endmodule
