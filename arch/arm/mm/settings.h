`timescale 1ns/1ns

`define WORD_WIDTH 32
`define REG_FILE_DEPTH 4
`define REG_FILE_SIZE 16
`define MEMORY_DATA_LEN 8
`define MEMORY_SIZE 2048
`define SIGNED_IMM_WIDTH 24
`define SHIFTER_OPERAND_WIDTH 12

`define CLOCK_PERIOD 20

`define MODE_ARITHMETIC 2'b00
`define MODE_MEM 2'b01
`define MODE_BRANCH 2'b10

`define EX_MOV 4'b0001
`define EX_MVN 4'b1001
`define EX_ADD 4'b0010
`define EX_ADC 4'b0011
`define EX_SUB 4'b0100
`define EX_SBC 4'b0101
`define EX_AND 4'b0110
`define EX_ORR 4'b0111
`define EX_EOR 4'b1000
`define EX_CMP 4'b1100
`define EX_TST 4'b1110
`define EX_LDR 4'b1010
`define EX_STR 4'b1010

`define OP_MOV 4'b1101
`define OP_MVN 4'b1111
`define OP_ADD 4'b0100
`define OP_ADC 4'b0101
`define OP_SUB 4'b0010
`define OP_SBC 4'b0110
`define OP_AND 4'b0000
`define OP_ORR 4'b1100
`define OP_EOR 4'b0001
`define OP_CMP 4'b1010
`define OP_TST 4'b1000
`define OP_LDR 4'b0100
`define OP_STR 4'b0100

`define FORWARDING_SEL_NOP 2'b00
`define FORWARDING_SEL_MEM 2'b01
`define FORWARDING_SEL_WB 2'b10