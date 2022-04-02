`include "settings.h"  

module IF_Stage
(
    input                    clk,
    input                    rst,
    input                    freeze,
    input                    branch_taken,
    input  [`WORD_WIDTH-1:0] branch_addr,
    output [`WORD_WIDTH-1:0] pc,
    output [`WORD_WIDTH-1:0] instruction
);

  wire [`WORD_WIDTH-1:0] pc_out;
  wire [`WORD_WIDTH-1:0] mux_out;
  wire [`WORD_WIDTH-1:0] adder_out;
  
  Instruction_Mem Instruction_Mem_Inst (
   .addr(pc_out),
   .instruction(instruction)
  );
  
  MUX_2_to_1 MUX_2_to_1_Inst (
   .sel(branch_taken),
   .in1(adder_out),
   .in2(branch_addr),
   .out(mux_out)
  );

  PC PC_Inst (
   .clk(clk),
   .rst(rst),
   .freeze(freeze),
   .pc_in(mux_out),
   .pc(pc_out)
  );

  Adder Adder_Inst (
  //  .a(32'b0100),
   .a(32'b01),
   .b(pc_out),
   .out(adder_out)
  );
  
  assign pc = mux_out;
  
endmodule