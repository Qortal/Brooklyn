`include "settings.h"  

module Status_Reg
(
    input            clk,
    input            rst,
    input            load,
    input      [3:0] status_in,
    output reg [3:0] status
);

    always @(negedge clk or posedge rst) begin
  if(rst) begin
    status <= 0;
  end  
  else if(load) begin
    status <= status_in;
  end
  else begin
    status <= status;
  end    
 end
 
endmodule
