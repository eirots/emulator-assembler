// opcode.h
// defines byte values for 4380 instructions

#ifndef opcode_h_
#define opcode_h_
#include <cstdint>

// enum definitions
enum JumpOpcode : std::uint8_t {
    OP_JMP = 0x01
};

enum MoveOpcode : std::uint8_t {
    OP_MOV = 0x07,   // 7
    OP_MOVI = 0x08,  // 8
    OP_LDA = 0x09,   // 9
    OP_STR = 0x0A,   // 10
    OP_LDR = 0x0B,   // 11
    OP_STB = 0x0C,   // 12
    OP_LDB = 0x0D    // 13
};

enum ArithOpcode : std::uint8_t {
    OP_ADD = 0x12,   // 18
    OP_ADDI = 0x13,  // 19
    OP_SUB = 0x14,   // 20
    OP_SUBI = 0x15,  // 21
    OP_MUL = 0x16,   // 22
    OP_MULI = 0x17,  // 23
    OP_DIV = 0x18,   // 24
    OP_SDIV = 0x19,  // 25
    OP_DIVI = 0x1A   // 26
};

enum TrapOpcode : std::uint8_t {
    OP_TRP = 0x1F

};

/*
    will need this in the future, temporarily putting this here
    switch (cntrl_regs[OPERATION]) {
        case OP_JMP:
            break;
        case OP_MOV:
            break;
        case OP_MOVI:
            break;
        case OP_LDA:
            break;
        case OP_STR:
            break;
        case OP_LDR:
            break;
        case OP_STB:
            break;
        case OP_LDB:
            break;
        case OP_ADD:
            break;
        case OP_ADDI:
            break;
        case OP_SUB:
            break;
        case OP_SUBI:
            break;
        case OP_MUL:
            break;
        case OP_MULI:
            break;
        case OP_DIV:
            break;
        case OP_SDIV:
            break;
        case OP_DIVI:
            break;
        case OP_TRP:
            break;
    }


*/

#endif