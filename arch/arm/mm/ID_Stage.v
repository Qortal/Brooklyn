`include "settings.h"

module ID_Stage
(
	input                    					  clk,
	input                    						rst,
	input 											freeze,
	input  [`WORD_WIDTH-1:0] 						pc_in,
	input  [`WORD_WIDTH-1:0] 						instruction_in,
	input  [`REG_FILE_DEPTH-1:0]			  reg_file_wb_address,
	input  [`WORD_WIDTH-1:0] 						reg_file_wb_data,
	input 										reg_file_enable,
	input  [3:0]                        status_register,
	output [`WORD_WIDTH-1:0] 						pc,
	output [`WORD_WIDTH-1:0] 						instruction,
	output [`REG_FILE_DEPTH-1:0] 		    reg_file_src1,
	output [`REG_FILE_DEPTH-1:0] 		    reg_file_src2,
	output [`REG_FILE_DEPTH-1:0] 		    reg_file_dst,
	output [`WORD_WIDTH-1:0] 						val_Rn, val_Rm,
	output [`SIGNED_IMM_WIDTH-1:0]      signed_immediate,
	output [`SHIFTER_OPERAND_WIDTH-1:0] shifter_operand,
	output [3:0] 												EX_command_out,
	output                              mem_read_out, mem_write_out,
	output                              WB_en_out,
	output                              Imm_out,
	output                              B_out,
	output                              SR_update_out,
	output                              has_src2,
	output                              has_src1
);

	wire [3:0] EX_command;
	wire mem_read, mem_write,
		WB_en,
		Imm,
		B,
		SR_update;

	wire [8:0] control_unit_mux_in, control_unit_mux_out;
	wire condition_state;

	MUX_2_to_1 #(.WORD_WIDTH(4)) MUX_2_to_1_Reg_File (
		.in1(instruction_in[3:0]), .in2(instruction_in[15:12]),
		.sel(mem_write),
		.out(reg_file_src2)
	);

	Register_File register_file(
		.clk(clk), .rst(rst),
		.WB_en(reg_file_enable),
		.src1(reg_file_src1), .src2(reg_file_src2),
		.WB_dest(reg_file_wb_address),
		.WB_result(reg_file_wb_data),
		.reg1(val_Rn), .reg2(val_Rm)
	);

	MUX_2_to_1 #(.WORD_WIDTH(9)) MUX_2_to_1_Control_Unit (
		.in1(control_unit_mux_in), .in2(9'b0),
		.sel(control_unit_mux_enable),
		.out(control_unit_mux_out)
	);

	Control_Unit Control_Unit_Inst (
		.S(instruction_in[20]),
		.mode(instruction_in[27:26]), .op_code(instruction_in[24:21]),
		.EX_command(EX_command),
		.mem_read(mem_read), .mem_write(mem_write),
		.WB_en(WB_en), .B(B),
		.SR_update(SR_update),
		.has_src1(has_src1)
	);

	Condition_Check Condition_Check_Inst (
		.condition(instruction_in[31:28]),
		.status_register(status_register),
		.condition_state(condition_state)
	);

	assign pc = pc_in;
	assign instruction = instruction_in;
	assign control_unit_mux_enable = (~condition_state) | freeze;
	assign control_unit_mux_in = {SR_update, B, EX_command, mem_write, mem_read, WB_en};
	assign {SR_update_out,
	         B_out,
	         EX_command_out,
	         mem_write_out, mem_read_out,
	         WB_en_out} = control_unit_mux_out;
	assign shifter_operand = instruction_in[11:0];
	assign reg_file_dst = instruction_in[15:12];
	assign reg_file_src1 = instruction_in[19:16];
	assign signed_immediate = instruction_in[23:0];
	assign Imm_out = instruction_in[25];
	assign has_src2 = (~instruction_in[25]) | mem_write;

endmodule
