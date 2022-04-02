`include "settings.h"

module Control_Unit
(
    input            S,
    input      [1:0] mode,
    input      [3:0] op_code,
    output reg [3:0] EX_command,
    output reg       mem_read,
    output reg       mem_write,
    output reg       WB_en,
    output reg       B,
    output       SR_update,
    output       has_src1
);

    always @(*) begin
        mem_read = 0;
        mem_write = 0;
        WB_en = 0;
        B = 0;
        case (mode)
            `MODE_MEM: begin
                case (S)
                    0: begin
                        EX_command = `EX_STR;
                        mem_write = 1;
                    end

                    1: begin
                        EX_command = `EX_LDR;
                        mem_read = 1;
                        WB_en = 1;
                    end
                endcase
            end

            `MODE_ARITHMETIC: begin
                case (op_code)
                    `OP_MOV: begin
                        EX_command = `EX_MOV;
                        WB_en = 1;
                    end

                    `OP_MVN: begin
                        EX_command = `EX_MVN;
                        WB_en = 1;
                    end

                    `OP_ADD: begin
                        EX_command = `EX_ADD;
                        WB_en = 1;
                    end

                    `OP_ADC: begin
                        EX_command = `EX_ADC;
                        WB_en = 1;
                    end

                    `OP_SUB: begin
                        EX_command = `EX_SUB;
                        WB_en = 1;
                    end

                    `OP_SBC: begin
                        EX_command = `EX_SBC;
                        WB_en = 1;
                    end
                    `OP_AND: begin
                        EX_command = `EX_AND;
                        WB_en = 1;
                    end

                    `OP_ORR: begin
                        EX_command = `EX_ORR;
                        WB_en = 1;
                    end

                    `OP_EOR: begin
                        EX_command = `EX_EOR;
                        WB_en = 1;
                    end

                    `OP_CMP: begin
                        EX_command = `EX_CMP;
                    end

                    `OP_TST: begin
                        EX_command = `EX_TST;
                    end
                endcase
            end

            `MODE_BRANCH: begin
                B = 1;
            end
        endcase
    end

    assign SR_update = S;
    assign has_src1 = ((EX_command == `EX_MOV) ||
                       (EX_command == `EX_MVN) || B) ? 1'b0 : 1'b1;

endmodule
