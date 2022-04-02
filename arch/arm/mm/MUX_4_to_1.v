`include "settings.h"

module MUX_4_to_1
(
  sel,
  in1,
  in2,
  in3,
  in4,
  out
);
  parameter WORD_WIDTH = `WORD_WIDTH;
  input       [1:0] sel;
  input       [WORD_WIDTH-1:0] in1, in2, in3, in4;
  output reg  [WORD_WIDTH-1:0] out;

  always@(sel or in1 or in2 or in3 or in4) begin
    out = 0;
    case(sel)
      2'b00: out = in1;
      2'b01: out = in2;
      2'b10: out = in3;
      2'b11: out = in4;
      default: out = 0;
    endcase
  end

endmodule