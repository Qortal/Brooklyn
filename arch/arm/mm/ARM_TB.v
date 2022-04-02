`include "settings.h"

module ARM_TB;
  parameter clock_period = `CLOCK_PERIOD;

  reg clk;
  reg rst;
  reg enableForwarding;


  ARM CPU(
    .clk(clk),
    .rst(rst),
    .enableForwarding(enableForwarding)
  );

  initial begin
    clk = 0;
    forever clk = #clock_period ~clk;
  end

  initial begin
    enableForwarding = 1;
    rst = 1;
    # (clock_period / 2);
    rst = 0;
    # (1000*clock_period);
    $stop;
  end
endmodule