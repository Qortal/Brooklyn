`include "settings.h"

module Hazard_Detection_Unit (
    input enableForwarding,
    input  [`REG_FILE_DEPTH-1:0] src1,
    input  [`REG_FILE_DEPTH-1:0] src2,
    input  [`REG_FILE_DEPTH-1:0] EXE_dest,
    input  [`REG_FILE_DEPTH-1:0] MEM_dest,
    input        EXE_WB_en,
    input        MEM_WB_en,
		input 		 EXE_memread_en,
    input        has_src1,
    input        has_src2,
    output reg   hazard_detected
);

always @(*) begin
	hazard_detected = 1'b0;
    if(~enableForwarding) begin
			if ((src1 == EXE_dest) && (EXE_WB_en == 1'b1) && (has_src1 == 1'b1)) begin
				hazard_detected = 1'b1;
			end
			else if ((src1 == MEM_dest) && (MEM_WB_en == 1'b1) && (has_src1 == 1'b1)) begin
				hazard_detected = 1'b1;
			end
			else if ((src2 == EXE_dest) && (EXE_WB_en == 1'b1) && (has_src2 == 1'b1)) begin
				hazard_detected = 1'b1;
			end
			else if ((src2 == MEM_dest) && (MEM_WB_en == 1'b1) && (has_src2 == 1'b1)) begin
				hazard_detected = 1'b1;
			end
    end
	else begin
		if(EXE_memread_en) begin
			if ((src1 == EXE_dest) && (EXE_WB_en == 1'b1) && (has_src1 == 1'b1)) begin
				hazard_detected = 1'b1;
			end
			else if ((src2 == EXE_dest) && (EXE_WB_en == 1'b1) && (has_src2 == 1'b1)) begin
				hazard_detected = 1'b1;
			end
		end
	end
end

endmodule
