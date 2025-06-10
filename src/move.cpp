#include "move.h"

#include <emu4380.h>

// operand 1 RD
// operand 2 RS
// operand 3 DC
// immediate value DC
bool MOV() {
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1];
        return true;
    } catch (const exception&) {
        cerr << "Error in MOV" << endl;
        return false;
    }
}

// operand 1 RD
// operand 2 DC
// operand 3 DC
// immediate value IMM
bool MOVI() {
    try {
        reg_file[cntrl_regs[OPERAND_1]] = cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in MOVI" << endl;
        return false;
    }
}

// operand 1 RD
// operand 2 DC
// operand 3 DC
// immediate value IMM
bool LDA() {
    try {
        reg_file[cntrl_regs[OPERAND_1]] = cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in LDA" << endl;
        return false;
    }
}

// operand 1 RS
// operand 2 DC
// operand 3 DC
// immediate value ADDRESS
bool STR() {
    try {
        uint32_t addr = cntrl_regs[IMMEDIATE];
        uint32_t val = data_regs[REG_VAL_1];

        for (int i = 0; i < 4; i++) {
            prog_mem[addr + i] = (val >> (i * 8)) & 0xFF;
        }
        return true;
    } catch (const exception&) {
        cerr << "Error in STR" << endl;
        return false;
    }
}

// operand 1 RD
// operand 2 DC
// operand 3 DC
// immediate value ADDRESS
bool LDR() {
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1];
        return true;
    } catch (const exception&) {
        cerr << "Error in LDR" << endl;
        return false;
    }
}

// operand 1 RS
// operand 2 DC
// operand 3 DC
// immediate value ADDRESS
bool STB() {
    // store list significant byte in RS at address
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

// operand 1 RD
// operand 2 DC
// operand 3 DC
// immediate value ADDRESS
bool LDB() {
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}
