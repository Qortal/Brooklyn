`include "settings.h"

module EXE_Stage
(
  input                    clk,
  input                    rst,
  input  [`WORD_WIDTH-1:0] pc_in,
  input  [`WORD_WIDTH-1:0] instruction_in,
  input  [`WORD_WIDTH-1:0] MEM_stage_val,
  input  [`WORD_WIDTH-1:0] WB_stage_val,
  input  [`SIGNED_IMM_WIDTH-1:0] signed_immediate,
  input  [3:0] EX_command,
  input  [3:0] SR_in,
  input  [`SHIFTER_OPERAND_WIDTH-1:0] shifter_operand,
  input  [`REG_FILE_DEPTH-1:0] dst_in,
  input  mem_read_in, mem_write_in, imm, WB_en_in, B_in,
  input  [`WORD_WIDTH-1:0] val_Rn_in, val_Rm_in,

  input  [1:0] sel_src1,
  input  [1:0] sel_src2,

  output [`REG_FILE_DEPTH-1:0] dst_out,
  output [3:0] SR_out,
  output [`WORD_WIDTH-1:0] ALU_res,
  output [`WORD_WIDTH-1:0] val_Rm_out,
  output [`WORD_WIDTH-1:0] branch_address,
  output mem_read_out, mem_write_out, WB_en_out, B_out,
  output [`WORD_WIDTH-1:0] pc,
  output [`WORD_WIDTH-1:0] instruction
);

  wire [`WORD_WIDTH-1:0] val2;
  wire is_for_memory;

  Adder Adder_Inst(
    .a(pc_in),
    .b({{(8){signed_immediate[`SIGNED_IMM_WIDTH-1]}}, signed_immediate}),
    .out(branch_address)
  );

  wire [`WORD_WIDTH-1:0] alu_src1;
  wire [`WORD_WIDTH-1:0] alu_src2;

  MUX_4_to_1 #(.WORD_WIDTH(`WORD_WIDTH)) MUX_ALU_sel1 (
		.in1(val_Rn_in), .in2(MEM_stage_val), .in3(WB_stage_val), .in4(val_Rn_in),
		.sel(sel_src1),
		.out(alu_src1)
	);

  MUX_4_to_1 #(.WORD_WIDTH(`WORD_WIDTH)) MUX_ALU_sel2 (
		.in1(val_Rm_in), .in2(MEM_stage_val), .in3(WB_stage_val), .in4(val_Rm_in),
		.sel(sel_src2),
		.out(alu_src2)
	);

  Val2_Generator Val2_Generator_Inst(
    .val_Rm(alu_src2),
    .shifter_operand(shifter_operand),
    .imm(imm),
    .is_for_memory(is_for_memory),
    .val2_out(val2)
	);

  ALU ALU_Inst(
    .val1(alu_src1),
    .val2(val2),
    .EX_command(EX_command),
    .carry(SR_in[2]),
    .res(ALU_res),
    .SR(SR_out)
  );

  assign is_for_memory = mem_read_in | mem_write_in;
  assign pc = pc_in;
  assign instruction = instruction_in;
  assign dst_out = dst_in;
	assign mem_read_out = mem_read_in;
	assign mem_write_out = mem_write_in;
  assign WB_en_out = WB_en_in;
  assign B_out = B_in;
  assign val_Rm_out = alu_src2;

endmodule
