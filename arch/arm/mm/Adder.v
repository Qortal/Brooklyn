`include "settings.h"  

module Adder (
    input  [`WORD_WIDTH-1:0]  a,
    input  [`WORD_WIDTH-1:0]  b, 
    output [`WORD_WIDTH-1:0] out
);
  
	reg [`WORD_WIDTH-1:0] result;
	
	always@(*) begin
		result = a + b;     
	end
	
	assign out = result;
  
endmodule