`include "settings.h"

module Memory
(
    input                    clk,
    input                    rst,
    input  [`WORD_WIDTH-1:0] alu_res,
    input  [`WORD_WIDTH-1:0] Val_Rm,
    input                    mem_w_en,
    input                    mem_r_en,
    output [`WORD_WIDTH-1:0] res_data
);

    wire [`WORD_WIDTH-1:0] generatedAddr = {alu_res[`WORD_WIDTH-1:2], 2'b00} - `WORD_WIDTH'd1024;

    reg [`MEMORY_DATA_LEN-1:0] mem_data [0:`MEMORY_SIZE-1];

	integer i;

	always @(posedge clk, posedge rst)
	begin
		if (rst)
			begin
				for (i=0; i < `MEMORY_SIZE; i = i+1)
					mem_data [i] <= i;
			end
		else if (mem_w_en) begin	
                mem_data[generatedAddr] <= Val_Rm[7:0];
                mem_data[{generatedAddr[`WORD_WIDTH-1:1], 1'b1}] <= Val_Rm[15:8];
                mem_data[{generatedAddr[`WORD_WIDTH-1:2], 2'b10}] <= Val_Rm[23:16];
                mem_data[{generatedAddr[`WORD_WIDTH-1:2], 2'b11}] <= Val_Rm[31:24];
		end
	end
    assign res_data = mem_r_en ? {mem_data[{generatedAddr[`WORD_WIDTH-1:2], 2'b11}], mem_data[{generatedAddr[`WORD_WIDTH-1:2], 2'b10}], 
                                    mem_data[{generatedAddr[`WORD_WIDTH-1:1], 1'b1}], mem_data[{generatedAddr}]}: `WORD_WIDTH'b0;

endmodule