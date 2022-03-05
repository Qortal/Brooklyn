`include "settings.h"  

module IF_Reg
(
    input                        clk,
    input                        rst,
    input                        freeze,
    input                        flush,
    input      [`WORD_WIDTH-1:0] pc_in,
    input      [`WORD_WIDTH-1:0] instruction_in,
    output reg [`WORD_WIDTH-1:0] pc,
    output reg [`WORD_WIDTH-1:0] instruction
);

always @(posedge clk or posedge rst) begin
  if(rst) begin
    pc <= 0;
    instruction <= 0;
  end  
  else if(flush) begin
    pc <= 0;
    instruction <= 0;
  end
  else if(~freeze) begin
    pc <= pc_in;
    instruction <= instruction_in;
  end 
  else begin
    pc <= pc;
    instruction <= instruction;
  end    
 end
 
endmodule