`include "settings.h"

module EXE_Reg
(
  input                    clk,
  input                    rst,
  input  [`WORD_WIDTH-1:0] pc_in,
  input  [`WORD_WIDTH-1:0] instruction_in,
  input  [`REG_FILE_DEPTH-1:0] dst_in,
  input  mem_read_in, mem_write_in, WB_en_in,
  input  [`WORD_WIDTH-1:0] val_Rm_in,
  input [`WORD_WIDTH-1:0] ALU_res_in,
  output reg [`REG_FILE_DEPTH-1:0] dst_out,
  output reg [`WORD_WIDTH-1:0] ALU_res_out,
  output reg [`WORD_WIDTH-1:0] val_Rm_out,
  output reg mem_read_out, mem_write_out, WB_en_out,
  output reg [`WORD_WIDTH-1:0] pc,
  output reg [`WORD_WIDTH-1:0] instruction
);

always @(posedge clk, posedge rst) begin
    if(rst) begin
      pc <= 0;
      instruction <= 0;
      dst_out <= 0;
      ALU_res_out <= 0;
      val_Rm_out <= 0;
      mem_read_out <= 0;
      mem_write_out <= 0;
      WB_en_out <= 0;
    end
    else begin
      pc <= pc_in;
      instruction <= instruction_in;
      dst_out <= dst_in;
      ALU_res_out <= ALU_res_in;
      val_Rm_out <= val_Rm_in;
      mem_read_out <= mem_read_in;
      mem_write_out <= mem_write_in;
      WB_en_out <= WB_en_in;
    end
end

endmodule

