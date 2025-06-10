#include "arith.h"

#include <emu4380.h>

bool ADD() {
    // add rs1 to rs2, store result in rd
    //  operand 1 RD
    //  operand 2 RS1
    //  operand 3 RS2
    //  immediate value DC
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] + data_regs[REG_VAL_2];
        return true;
    } catch (const exception&) {
        cerr << "Error in ADD" << endl;
        return false;
    }
}

bool ADDI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool SUB() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool SUBI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool MUL() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool MULI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool DIV() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool SDIV() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool DIVI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}
