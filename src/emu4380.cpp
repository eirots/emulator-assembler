#include "emu4380.h"

std::uint32_t* reg_file = nullptr;
unsigned char* prog_mem = nullptr;
std::uint32_t cntrl_regs[5] = {};
std::uint32_t data_regs[2] = {};
std::uint32_t mem_size = 0;
const uint32_t DEFAULT_MEMORY_SIZE = 131'072;  // default minimum size

bool fetch() {
    constexpr size_t INSTR_SIZE = 8;

    if (reg_file[PC] + INSTR_SIZE > mem_size) return false;  // about to run out of memory

    cntrl_regs[OPERATION] = prog_mem[reg_file[PC]++];
    cntrl_regs[OPERAND_1] = prog_mem[reg_file[PC]++];
    cntrl_regs[OPERAND_2] = prog_mem[reg_file[PC]++];
    cntrl_regs[OPERAND_3] = prog_mem[reg_file[PC]++];

    uint32_t imm = 0;                                              // building immediate
    imm |= static_cast<uint32_t>(prog_mem[reg_file[PC]++]);        // least-significant
    imm |= static_cast<uint32_t>(prog_mem[reg_file[PC]++]) << 8;   // next
    imm |= static_cast<uint32_t>(prog_mem[reg_file[PC]++]) << 16;  // next                   bits 23
    imm |= static_cast<uint32_t>(prog_mem[reg_file[PC]++]) << 24;  // most-significant byte, bits 31-34

    cntrl_regs[IMMEDIATE] = imm;

    return true;
}

//-------------DECODE HELPER FUNCTIONS -------------

// validate any valid register
bool is_valid_rg(uint32_t r) {
    return r < NUM_REGS;
}

// validates general purpose register
bool igr(uint32_t r) {
    return is_valid_rg(r) && (r >= R0 && r < PC);
    //  return r >= 16 && r <= 21
}
// validates if its a state register
bool is_state_rg(uint32_t r) {
    return is_valid_rg(r) && (r < PC && r >= HP);
}

// validate if an address is within memory
bool is_valid_addr(uint32_t addr) {
    return addr < mem_size;
}

//----------- END OF DECODE HELPER FUNCTIONS -----------

bool decode() {
    // TODO:

    // verifies that the specified operation (or TRP) and operands as specified cntrl_regs are valid

    // ex, MOV operates on state registers, and there are a limited number of these. A MOV with an RD value of 55 would be a
    //      malformed instruction.

    // Also retrieve register values from the register file and place those values in appropriate data_regs as indicated by the
    //   register operands present in cntrl_regs

    // NOTE: Immediate value instructions shall result in immediate value operands being placed in CNTRL_REGS during fetch stage
    //       Values shall remain in cntrl_regs where they are sourced during execute phase, vs being placed in data_regs

    switch (cntrl_regs[OPERATION]) {
        case OP_JMP: {
            //  operand 1         DC
            //  operand 2         DC
            //  operand 3         DC
            //  immediate value   Address
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;  // malformed instruction

            return true;
        }

        case OP_MOV: {
            // operand 1 RD
            // operand 2 RS
            // operand 3 DC
            // immediate value DC
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs = cntrl_regs[OPERAND_2];

            if (!is_state_rg(rd)) return false;
            if (!is_valid_rg(rs)) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            return true;
        }

        case OP_MOVI: {
            // operand 1 RD
            // operand 2 DC
            // operand 3 DC
            // immediate value IMM

            const uint32_t rd = cntrl_regs[OPERAND_1];

            if (!is_state_rg(rd)) return false;

            return true;
        }

        case OP_LDA: {
            // operand 1 RD
            // operand 2 DC
            // operand 3 DC
            // immediate value IMM
            const uint32_t rd = cntrl_regs[OPERAND_1];

            if (!is_state_rg(rd)) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;
            return true;
        }

        case OP_STR: {
            // operand 1 RS
            // operand 2 DC
            // operand 3 DC
            // immediate value ADDRESS
            // store integer in RS at address
            const uint32_t rs = cntrl_regs[OPERAND_1];

            if (!is_valid_rg(rs)) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            return true;
        }

        case OP_LDR: {
            // load integer at address to RD
            // operand 1 RS
            // operand 2 DC
            // operand 3 DC
            // immediate value ADDRESS
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t addr = cntrl_regs[IMMEDIATE];

            if (!is_state_rg(rd)) return false;
            if (addr + 4 > mem_size) return false;

            data_regs[REG_VAL_1] = (prog_mem[addr + 0]) | (prog_mem[addr + 1] << 8) | (prog_mem[addr + 2] << 16) | (prog_mem[addr + 3] << 24);
            return true;
        }

        case OP_STB: {
            // Store least significant byte in RS at address
            // operand 1 RS
            // operand 2 DC
            // operand 3 DC
            // immediate value ADDRESS
            const uint32_t rs = cntrl_regs[OPERAND_1];

            if (!is_state_rg(cntrl_regs[OPERAND_1])) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;

            data_regs[REG_VAL_1] = reg_file[rs] & 0xFFu;
            return true;
        }

        case OP_LDB: {
            // load byte at address to RD
            // operand 1 RD
            // operand 2 DC
            // operand 3 DC
            // immediate value ADDRESS
            const uint32_t rd = cntrl_regs[OPERAND_1];

            if (!is_state_rg(rd)) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;

            data_regs[REG_VAL_1] = prog_mem[cntrl_regs[IMMEDIATE]];
            return true;
        }

        case OP_ADD: {
            // add rs1 to rs2, store result in RD
            //  operand 1 RD
            //  operand 2 RS1
            //  operand 3 RS2
            //  immediate value DC
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!is_state_rg(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_ADDI: {
            // add imm to rs1, store result in RD
            // operand 1 RD
            // operand 2 RS1
            // operand 3 DC
            // immediate value IMM
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!is_state_rg(rd) | !igr(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];

            return true;
        }

        case OP_SUB: {
            // Subtract RS2 from RS1, store result in RD
            // operand 1 RD
            // operand 2 RS1
            // operand 3 RS2
            // immediate value DC
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!is_state_rg(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_SUBI: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!is_state_rg(rd) | !igr(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];

            return true;
        }

        case OP_MUL: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!is_state_rg(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_MULI: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!is_state_rg(rd) | !igr(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];

            return true;
        }

        case OP_DIV: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!is_state_rg(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_SDIV: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!is_state_rg(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_DIVI: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!is_state_rg(rd) | !igr(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];

            return true;
        }

        case OP_TRP: {
            break;
        }
    }

    return false;
}

bool execute() {
    // Executes the effects of decoded and validated instruction or TRP on the state members (regs, memory, etc.) as
    // indicated by cntrl_regs and data_regs, and in accordance with instruction or TRP's specification

    // returns FALSE if illegal operation is encountered (does not execute instruction), otherwise TRUE
    switch (cntrl_regs[OPERATION]) {
        case OP_JMP:
            return JMP();
        case OP_MOV:
            return MOV();
        case OP_MOVI:
            return MOVI();
        case OP_LDA:
            return LDA();
        case OP_STR:
            return STR();
        case OP_LDR:
            return LDR();
        case OP_STB:
            return STB();
        case OP_LDB:
            return LDB();
        case OP_ADD:
            return ADD();
        case OP_ADDI:
            return ADDI();
        case OP_SUB:
            return SUB();
        case OP_SUBI:
            return SUBI();
        case OP_MUL:
            return MUL();
        case OP_MULI:
            return MULI();
        case OP_DIV:
            return DIV();
        case OP_SDIV:
            return SDIV();
        case OP_DIVI:
            return DIVI();
        case OP_TRP:
            return TRP();
    }

    return false;  // catch in case something went horribly wrong
}

bool init_registers() {
    if (reg_file) return true;  // already initialized

    reg_file = static_cast<uint32_t*>(
        malloc(NUM_REGS * sizeof(uint32_t)));

    if (!reg_file) return false;  // no memory available to allocate regs
    memset(reg_file, 0, NUM_REGS * sizeof(uint32_t));
    return true;
}

bool init_mem(unsigned int size) {
    // first allocation
    if (prog_mem == nullptr) {
        prog_mem = static_cast<unsigned char*>(std::malloc(size));
        if (!prog_mem) return false;

    } else {  // resizing
        void* new_block_of_memory = std::realloc(prog_mem, size);
        if (!new_block_of_memory) return false;  // OUT OF MEMORY

        prog_mem = static_cast<unsigned char*>(new_block_of_memory);
    }
    mem_size = size;

    if (!init_registers()) return false;  // couldn't initialize registers

    reg_file[PC] = 0;
    reg_file[SL] = 0;
    reg_file[SB] = mem_size;
    reg_file[SP] = mem_size;
    reg_file[FP] = reg_file[SP];
    reg_file[HP] = reg_file[SL];

    return true;
}

uint32_t load_binary(const char* filename) {
    ifstream in(filename, std::ios::binary | std::ios::ate);
    if (!in) {
        return 1;
    }

    streamsize file_size = in.tellg();
    if (file_size > mem_size) {
        return 2;
    }

    in.seekg(0, std::ios::beg);

    if (!in.read(reinterpret_cast<char*>(prog_mem), file_size)) {
        cerr << "Error while reading " << filename << endl;
        return false;
    }
    return 0;
}

int runEmulator(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " INPUT_BINARY_FILE [RESERVED_MEMORY]\n";
        cout << "INPUT_BINARY_FILE: \t\tPath to file containing 4380 byte code\n";
        cout << "RESERVED_MEMORY (optional):\tPositive integer, default 131,072 bytes, max 4,294,967,295\n";
        return 1;
    }
    mem_size;

    if (argc < 3) {
        mem_size = DEFAULT_MEMORY_SIZE;

        // cout << "memsize defaulted to: " << mem_size << endl;
    } else {
        try {
            size_t pos = 0;
            unsigned long tmp = stoul(argv[2], &pos, 10);

            if (pos != strlen(argv[2]) ||
                tmp > numeric_limits<uint32_t>::max()) {
                throw std::out_of_range("Bad range, range is (0, 4,294,967,295]");
            }
            mem_size = static_cast<uint32_t>(tmp);

        } catch (const exception&) {
            cerr << "Invalid memory size, range is (0, 4,294,967,295]" << endl;
            return 1;
        }

        // cout << "memsize set to: " << mem_size << endl;
    }

    if (!init_mem(mem_size)) return 1;

    unsigned int binloadresult = load_binary(argv[1]);
    if (binloadresult == 1) {
        cerr << "Cannot open file: " << argv[1] << endl;
        return 1;
    } else if (binloadresult == 2) {
        cerr << "INSUFFICIENT MEMORY SPACE \n";
        return 2;
    }

    return 0;
}

// operation execution functions
bool JMP() {
    try {
        reg_file[PC] = cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in JMP" << endl;
        return false;
    }
}

// move execution functions

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
        prog_mem[cntrl_regs[IMMEDIATE]] = static_cast<unsigned char>(data_regs[REG_VAL_1]);
        return true;
    } catch (const exception&) {
        cerr << "Error in STB" << endl;
        return false;
    }
}

// operand 1 RD
// operand 2 DC
// operand 3 DC
// immediate value ADDRESS
bool LDB() {
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1];
        return true;
    } catch (const exception&) {
        cerr << "Error in LDB" << endl;
        return false;
    }
}

// arithmetic execution functions

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
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] + cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in ADDI" << endl;
        return false;
    }
}

bool SUB() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] - data_regs[REG_VAL_2];
        return true;
    } catch (const exception&) {
        cerr << "Error in SUB" << endl;
        return false;
    }
}

bool SUBI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] - cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in SUBI" << endl;
        return false;
    }
}

bool MUL() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] * data_regs[REG_VAL_2];
        return true;
    } catch (const exception&) {
        cerr << "Error in MUL" << endl;
        return false;
    }
}

bool MULI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1] * cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in MULI" << endl;
        return false;
    }
}

bool DIV() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        if (data_regs[REG_VAL_2] == 0) return false;
        reg_file[cntrl_regs[OPERAND_1]] = static_cast<unsigned int>(data_regs[REG_VAL_1]) / static_cast<unsigned int>(data_regs[REG_VAL_2]);
        return true;
    } catch (const exception&) {
        cerr << "Error in DIV" << endl;
        return false;
    }
}

bool SDIV() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    try {
        if (data_regs[REG_VAL_2] == 0) return false;
        reg_file[cntrl_regs[OPERAND_1]] = static_cast<uint32_t>(static_cast<int32_t>(data_regs[REG_VAL_1]) / static_cast<int32_t>(data_regs[REG_VAL_2]));
        return true;
    } catch (const exception&) {
        cerr << "Error in SDIV" << endl;
        return false;
    }
}

bool DIVI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate value IMM
    try {
        if (cntrl_regs[IMMEDIATE] == 0) return false;
        reg_file[cntrl_regs[OPERAND_1]] = static_cast<uint32_t>(static_cast<int32_t>(data_regs[REG_VAL_1]) / static_cast<int32_t>(cntrl_regs[IMMEDIATE]));
        return true;
    } catch (const exception&) {
        cerr << "Error in DIVI" << endl;
        return false;
    }
}

// trap/interrupt functions

bool TRP() {
    // IMM 0    -> EXECUTE STOP / EXIT ROUTINE
    // IMM 1    -> WRITE INT IN R3 TO STDOUT (CONSOLE)
    //      print the above without any leading or trailing whitespace
    // IMM 2    -> READ AN INTEGER INTO R3 FROM STDIN
    // IMM 3    -> WRITE CHAR IN R3 TO STDOUT
    //      print the above without any leading or trailing whitespace
    // IMM 4    -> READ A CHAR INTO R3 FROM STDIN
    // IMM 98   -> PRINT ALL REGISTER CONTENTS TO STDOUT
    //      format above as follows:
    //          -one register name and value per line
    //          -The register name shall be in all caps, followed by a tab character, followed by the register value printed as unsigned base 10 integer
    //          -example:
    //              -R1 0
    //              -R2 128
    //              -R3 34
    //              -HP 10045
}
