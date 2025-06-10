#include "emu4380.h"

#include "arith.h"
#include "interrupt.h"
#include "jump.h"
#include "move.h"
#include "opcode.h"
#include "utils.h"

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

// validates general purpose register
bool is_general_rg(uint32_t r) {
    return r >= PC && r <= HP;
}
// validates if its a state register
bool is_state_rg(uint32_t r) {
    return r < 16;
}
// validate any valid register
bool is_valid_rg(uint32_t r) {
    return r < NUM_REGS;
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

            if (!is_valid_rg(rd)) return false;
            if (addr + 4 > mem_size) return false;

            data_regs[REG_VAL_1] = (prog_mem[addr + 0]) | (prog_mem[addr + 1] << 8) | (prog_mem[addr + 2] << 16) | (prog_mem[addr + 3] << 24);
            return true;
        }

        case OP_STB: {
            break;
        }

        case OP_LDB: {
            break;
        }

        case OP_ADD: {
            break;
        }

        case OP_ADDI: {
            break;
        }

        case OP_SUB: {
            break;
        }

        case OP_SUBI: {
            break;
        }

        case OP_MUL: {
            break;
        }

        case OP_MULI: {
            break;
        }

        case OP_DIV: {
            break;
        }

        case OP_SDIV: {
            break;
        }

        case OP_DIVI: {
            break;
        }

        case OP_TRP: {
            break;
        }
    }

    return false;
}

bool execute() {
    // TODO:

    // Executes the effects of decoded and validated instruction or TRP on the state members (regs, memory, etc.) as
    // indicated by cntrl_regs and data_regs, and in accordance with instruction or TRP's specification

    // returns FALSE if illegal operation is encountered (does not execute instruction), otherwise TRUE
    switch (cntrl_regs[OPERATION]) {
        case OP_JMP:
            return JMP();
        case OP_MOV:
            return MOV();
        case OP_MOVI:
            return MOV();
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