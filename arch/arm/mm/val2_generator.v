`include "settings.h"

module Val2_Generator
(
    input       [`SHIFTER_OPERAND_WIDTH-1:0]    shifter_operand,
    input                           imm, is_for_memory,
    input   [`WORD_WIDTH-1:0]       val_Rm,
    output reg [`WORD_WIDTH-1:0]   val2_out
);

    reg [`WORD_WIDTH-1:0]  _32bit_immd_temp;

    integer i;

    always @(*) begin
        if(is_for_memory == 1'b1)
            val2_out = {20'b0, shifter_operand};
        else if(imm == 1'b1) begin
            _32bit_immd_temp = {24'b0, shifter_operand[7:0]};

            for (i=0; i<{shifter_operand[11:8], 1'b0}; i=i+1) begin
                _32bit_immd_temp = {_32bit_immd_temp[0], _32bit_immd_temp[`WORD_WIDTH-1:1]};
            end
            val2_out = _32bit_immd_temp;
        end

        else begin
            case (shifter_operand[6:5])
                00: begin
                    val2_out = val_Rm << shifter_operand[11:7];
                end
                01: begin
                    val2_out = val_Rm >> shifter_operand[11:7];
                end
                10: begin
                    val2_out = val_Rm >>> shifter_operand[11:7];
                end
                11: begin
                    val2_out = val_Rm;
                    for (i=0; i<{shifter_operand[11:7]}; i=i+1) begin
                        val2_out = {val2_out[0], val2_out[`WORD_WIDTH-1:1]};
                    end
                end
                default:begin
                    val2_out = val_Rm << shifter_operand[11:7];
                end
            endcase
        end

    end

endmodule
