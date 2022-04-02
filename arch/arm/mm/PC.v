`include "settings.h"  

module PC
(
    input                        clk,
    input                        rst,
    input                        freeze,
    input      [`WORD_WIDTH-1:0] pc_in,
    output reg [`WORD_WIDTH-1:0] pc
);

always @(posedge clk or posedge rst) begin
  if(rst) begin
    pc <= 0;
  end  
  else if(~freeze) begin
    pc <= pc_in;
  end
  else begin
    pc <= pc;
  end    
 end
 
endmodule