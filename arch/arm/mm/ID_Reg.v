`include "settings.h"

module ID_Reg
(
  input                               clk,
  input                               rst,
  input                               flush,
  input [`WORD_WIDTH-1:0]             pc_in,
  input [`WORD_WIDTH-1:0]             instruction_in,
  input [`REG_FILE_DEPTH-1:0] 				reg_file_dst_in,
  input [`WORD_WIDTH-1:0] 						val_Rn_in, val_Rm_in,
  input [`SIGNED_IMM_WIDTH-1:0] 			signed_immediate_in,
  input [`SHIFTER_OPERAND_WIDTH-1:0]  shifter_operand_in,
  input [3:0] 												EX_command_in,
  input [3:0]             status_register_in,
  input mem_read_in, mem_write_in,
    WB_en_in,
    Imm_in,
    B_in,
    SR_update_in,
  input [`REG_FILE_DEPTH-1:0] 				reg_file_src1_in,
  input [`REG_FILE_DEPTH-1:0] 				reg_file_src2_in,
  output reg [`WORD_WIDTH-1:0]            pc,
  output reg [`WORD_WIDTH-1:0]            instruction,
  output reg [`REG_FILE_DEPTH-1:0] 				reg_file_dst_out,
  output reg [`WORD_WIDTH-1:0] 						val_Rn_out, val_Rm_out,
  output reg [`SIGNED_IMM_WIDTH-1:0] 			signed_immediate_out,
  output reg [`SHIFTER_OPERAND_WIDTH-1:0] shifter_operand_out,
  output reg [3:0] 												EX_command_out,
  output reg [3:0]             status_register_out,
  output reg mem_read_out, mem_write_out,
    WB_en_out,
    Imm_out,
    B_out,
    SR_update_out,
  output reg [`REG_FILE_DEPTH-1:0] 				reg_file_src1_out,
  output reg [`REG_FILE_DEPTH-1:0] 				reg_file_src2_out
);

  always @(posedge clk, posedge rst) begin
    if (rst || flush) begin
      pc <= 0;
      instruction <= 0;
      reg_file_dst_out <= 0;
      val_Rn_out <= 0;
      val_Rm_out <=0;
      signed_immediate_out <= 0;
      shifter_operand_out <= 0;
      EX_command_out <= 0;
      status_register_out <= 0;
      mem_read_out <= 0;
      mem_write_out <= 0;
      WB_en_out <= 0;
      Imm_out <= 0;
      B_out <= 0;
      SR_update_out <= 0;
      reg_file_src1_out <= 0;
      reg_file_src2_out <= 0;
    end
    else begin
      pc <= pc_in;
      instruction <= instruction_in;
      reg_file_dst_out <= reg_file_dst_in;
      val_Rn_out <= val_Rn_in;
      val_Rm_out <= val_Rm_in;
      signed_immediate_out <= signed_immediate_in;
      shifter_operand_out <= shifter_operand_in;
      EX_command_out <= EX_command_in;
      status_register_out <= status_register_in;
      mem_read_out <= mem_read_in;
      mem_write_out <= mem_write_in;
      WB_en_out <= WB_en_in;
      Imm_out <= Imm_in;
      B_out <= B_in;
      SR_update_out <= SR_update_in;
      reg_file_src1_out <= reg_file_src1_in;
      reg_file_src2_out <= reg_file_src2_in;
    end
  end

endmodule
