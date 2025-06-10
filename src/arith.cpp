#include "arith.h"

#include <emu4380.h>

bool ADD() {
    // add rs1 to rs2, store result in rd
    //  operand 1 RD
    //  operand 2 RS1
    //  operand 3 RS2
    //  immediate value DC

    try {
        return true;
    } catch (const exception&) {
        cerr << "Error in " << endl;
        return false;
    }
}

bool ADDI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
}

bool SUB() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
}

bool SUBI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
}

bool MUL() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
}

bool MULI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
}

bool DIV() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
}

bool SDIV() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
}

bool DIVI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
}
