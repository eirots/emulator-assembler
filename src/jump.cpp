#include "jump.h"

#include <emu4380.h>

bool JMP() {
    try {
        reg_file[PC] = cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in JMP" << endl;
        return false;
    }
    //  operand 1         DC
    //  operand 2         DC
    //  operand 3         DC
    //  immediate value   Address
    //                    When represented in an assembly file the Address shall be represented as a label. This label must
    //                    be resolved to a 32 bit unsigned integer value in bytecode.
}