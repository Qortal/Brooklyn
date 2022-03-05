`include "settings.h"

module MEM_Reg
(
    input                       clk,
    input                       rst,
    input [`REG_FILE_DEPTH-1:0] dst,
    input [`WORD_WIDTH-1:0] ALU_res,
    input [`WORD_WIDTH-1:0] mem,
    input mem_read, WB_en,

    output reg [`REG_FILE_DEPTH-1:0] dst_out,
    output reg [`WORD_WIDTH-1:0] ALU_res_out,
    output reg [`WORD_WIDTH-1:0] mem_out,
    output reg mem_read_out, WB_en_out
);

always @(posedge clk, posedge rst) begin
    if(rst) begin
        dst_out <= 0;
        ALU_res_out <= 0;
        mem_out <= 0;
        mem_read_out <= 0;
        WB_en_out <= 0;
    end
    else begin
        dst_out <= dst;
        ALU_res_out <= ALU_res;
        mem_out <= mem;
        mem_read_out <= mem_read;
        WB_en_out <= WB_en;
    end
end

endmodule
