#include "emu4380.h"
/**
 * @file emu4380.cpp
 * @brief Core Emulator logic
 */

constexpr size_t CNTRL_REG_SIZE = 5;
constexpr size_t DATA_REG_SIZE = 2;
constexpr uint32_t DEFAULT_MEMORY_SIZE = 131'072;

constexpr size_t NUM_REGS = 22;
constexpr size_t INSTR_SIZE = 8;

// using constant syntax because it needs to be calculated at runtime, but still will act like a constant afterwards
uint32_t SET_BITS;
uint32_t TAG_BITS;

uint32_t OFFSET_MASK;
uint32_t SET_MASK;

// constexpr uint32_t CACHE_SIZE = NUM_CACHE_LINES * BLOCK_SIZE;  //sanity check

uint32_t* reg_file = nullptr;
unsigned char* prog_mem = nullptr;

uint32_t cntrl_regs[CNTRL_REG_SIZE] = {};
uint32_t data_regs[DATA_REG_SIZE] = {};
uint32_t mem_size = 0;
bool runBool = true;

unsigned int mem_cycle_cntr = 0;

CacheType current_cache_type = NO_CACHE;
bool cacheUsed;

bool fetching_second = false;
size_t associativity = -1;  // user provided, completely unused if not using a cache
size_t num_sets = -1;       // set index is log2(#sets)
// size_t num_tag_bits = -1;   // whatever's left

Line** cache = nullptr;

constexpr const char* REG_NAMES[NUM_REGS] = {
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15",
    "PC",
    "SL",
    "SB",
    "SP",
    "FP",
    "HP"};

//-------------MEMORY VALIDATION FUNCTIONS -------------
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
    return is_valid_rg(r) && (r >= PC && r <= HP);
}
// validate if an address is within memory
bool is_valid_addr(uint32_t addr) {
    return addr < mem_size;
}

bool addr_in_range(uint32_t addr, size_t bytes = 1) {
    return addr + bytes <= mem_size;
}
//---------------------------------------------------------

//----------- MEMORY ACCESS FUNCTIONS -----------

bool handleCacheHit(Line& line,
                    uint32_t offset,
                    AccessType accessType,
                    uint32_t& outWord,
                    unsigned char writeByte = 0,
                    uint32_t writeWord = 0) {
    // cout << "did we just die at the start?" << endl;

    if (!line.valid) return false;
    if (associativity > 1) line.lastused = mem_cycle_cntr;

    mem_cycle_cntr++;
    switch (accessType) {
        case READBYTE:
            outWord = line.data[offset];
            break;
        case READWORD:
            outWord = static_cast<uint32_t>(line.data[offset]) |
                      (static_cast<uint32_t>(line.data[offset + 1]) << 8) |
                      (static_cast<uint32_t>(line.data[offset + 2]) << 16) |
                      (static_cast<uint32_t>(line.data[offset + 3]) << 24);
            break;
        case WRITEBYTE:
            line.data[offset] = writeByte;
            line.dirty = true;
            break;
        case WRITEWORD:
            for (size_t i = 0; i < 4; i++) {
                line.data[offset + i] = static_cast<uint8_t>((writeWord >> (8 * i)) & 0xFF);
            }
            line.dirty = true;
            break;
    }
    return true;
}

void handleCacheMiss(uint32_t address,
                     uint32_t setidx,
                     Line& line,
                     uint32_t offset,
                     AccessType accessType,
                     uint32_t& outWord,
                     unsigned char writeByte = 0,
                     uint32_t writeWord = 0) {
    // handy debug lines below
    //  cout << "PC is at " << reg_file[PC] << endl;
    // cout << "\t" << "address:" << address << "\n"
    //      << "\t" << "setidx:" << setidx << "\n"
    //      //"\t" <<<< "line:" << line << "\n"
    //      << "\t" << "offset:" << offset << "\n"
    //      << "\t" << "accessType:" << accessType << "\n"
    //      << "\t" << "outWord:" << outWord << "\n"
    //      << "\t" << "writeByte:" << writeByte << "\n"
    //      << "\t" << "writeWord:" << writeWord << "\n";

    constexpr int WORDS_PER_BLOCK = BLOCK_SIZE / 4;

    // first 4-byte word is 8 cycles, then every cycle after that is 2 cycles.
    const auto cyclesneeded = [](int words) { return 6 + 2 * words; };

    if (line.valid && line.dirty) {
        uint32_t writebackaddr = (line.tag << (SET_BITS + OFFSET_BITS)) | (setidx << OFFSET_BITS);
        mem_cycle_cntr += cyclesneeded(WORDS_PER_BLOCK);

        memcpy(&prog_mem[writebackaddr], line.data, BLOCK_SIZE);
        line.dirty = false;
    }

    uint32_t base = address & ~(BLOCK_SIZE - 1);
    mem_cycle_cntr += cyclesneeded(WORDS_PER_BLOCK);
    memcpy(line.data, &prog_mem[base], BLOCK_SIZE);

    line.tag = address >> (OFFSET_BITS + SET_BITS);
    line.valid = true;
    line.dirty = (accessType == WRITEBYTE || accessType == WRITEWORD);
    line.lastused = mem_cycle_cntr;

    handleCacheHit(line, offset, accessType, outWord, writeByte, writeWord);
}

void checkCache(uint32_t addr,
                AccessType accessType,
                uint32_t& outWord,
                unsigned char writeByte = 0,
                uint32_t writeWord = 0) {
    uint32_t tagbits = addr >> (OFFSET_BITS + SET_BITS);
    uint32_t offset = addr & OFFSET_MASK;
    uint32_t setidx = (addr >> OFFSET_BITS) & SET_MASK;

    size_t victim = 0;
    bool foundempty = false;
    uint64_t oldest = UINT64_MAX;

    // dumpCacheVerbose(false, 0, true);

    // ******CACHE HIT******
    for (size_t way = 0; way < associativity; ++way) {
        Line& current = cache[setidx][way];

        if (current.valid && current.tag == tagbits) {
            // cout << "Did we ever actually hit on anything? " << endl;

            handleCacheHit(current, offset, accessType, outWord, writeByte, writeWord);
            return;
        }

        if (!current.valid && !foundempty) {
            // cout << "was it inside the current.valid and found empty section? " << endl;
            victim = way;
            foundempty = true;

        } else if (current.valid && current.lastused < oldest) {
            // cout << "was it inside the current.valid and current.lastused < oldest section? " << endl;
            oldest = current.lastused;
            victim = way;
        }

    }  // ******CACHE MISS******

    handleCacheMiss(addr, setidx, cache[setidx][victim], offset, accessType, outWord, writeByte, writeWord);
}

unsigned char readByte(uint32_t address) {
    if (cacheUsed) {
        uint32_t outbyte;
        checkCache(address, READBYTE, outbyte);
        return static_cast<unsigned char>(outbyte);

    } else {
        mem_cycle_cntr += 8;

        if (!addr_in_range(address)) {
            cerr << "Address was not in range for readByte. Returning a null character." << endl;
            return 0;
        }

        return prog_mem[address];
    }
}

unsigned int readWord(uint32_t address) {
    if (cacheUsed) {
        uint32_t outword;
        checkCache(address, READWORD, outword);
        return outword;

    } else {
        if (!fetching_second)
            mem_cycle_cntr += 8;
        else
            mem_cycle_cntr += 2;

        if (!addr_in_range(address, 4)) {
            cerr << "Address was not in range for readWord. Returning a garbage int of -1." << endl;
            return UINT32_MAX;
        }

        uint32_t val = (prog_mem[address + 0]) |
                       (prog_mem[address + 1] << 8) |
                       (prog_mem[address + 2] << 16) |
                       (prog_mem[address + 3] << 24);

        return static_cast<uint32_t>(val);
    }
}

void writeByte(uint32_t address, unsigned char byte) {
    if (cacheUsed) {
        uint32_t dummyvar = 0;
        checkCache(address, WRITEBYTE, dummyvar, byte);
    } else {
        mem_cycle_cntr += 8;
        if (!addr_in_range(address)) {
            // noop
            return;
        }

        prog_mem[address] = byte;
    }
}

void writeWord(uint32_t address, unsigned int word) {
    if (cacheUsed) {
        uint32_t dummyvar = 0;
        checkCache(address, WRITEWORD, dummyvar, 0, word);

    } else {
        mem_cycle_cntr += 8;
        if (!addr_in_range(address, 4)) {
            // noop
            return;
        }

        for (size_t i = 0; i < 4; i++) {
            prog_mem[address + i] = static_cast<unsigned char>(((word) >> (i * 8)) & 0xFFu);
        }
    }
}

uint32_t load_binary(const char* filename) {
    ifstream in(filename, ios::binary | ios::ate);
    if (!in) return 1;

    streamsize file_size = in.tellg();
    if (file_size > mem_size || file_size < 4) {
        return 2;  // file too small
    }

    in.seekg(0);

    if (!in.read(reinterpret_cast<char*>(prog_mem), file_size)) return 3;

    // reading entry point, this is why memory was off at the beginning
    uint32_t entry = prog_mem[0] |
                     (prog_mem[1] << 8) |
                     (prog_mem[2] << 16) |
                     (prog_mem[3] << 24);

    if (entry >= mem_size) return 4;

    reg_file[PC] = entry;
    return 0;
}
//----------- START OF MEMORY INIT FUNCTIONS -----------
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
        prog_mem = static_cast<unsigned char*>(malloc(size));
        if (!prog_mem) return false;

    } else {  // resizing
        void* new_block_of_memory = realloc(prog_mem, size);
        if (!new_block_of_memory) return false;  // OUT OF MEMORY

        prog_mem = static_cast<unsigned char*>(new_block_of_memory);
    }
    mem_size = size;

    if (!init_registers()) return false;  // couldn't initialize registers

    reg_file[SL] = 0;
    reg_file[SB] = mem_size;
    reg_file[SP] = mem_size;
    reg_file[FP] = reg_file[SP];
    reg_file[HP] = reg_file[SL];
    mem_cycle_cntr = 0;

    return true;
}

void init_cache(uint32_t cacheType) {
    switch (cacheType) {
        case NO_CACHE:
            current_cache_type = NO_CACHE;
            cacheUsed = false;
            return;
        case DIRECT_MAPPED:
            current_cache_type = DIRECT_MAPPED;
            associativity = 1;
            break;
        case FULLY_ASSOCIATIVE:
            current_cache_type = FULLY_ASSOCIATIVE;
            associativity = NUM_CACHE_LINES;
            break;
        case TWO_WAY_SET_ASSOCIATIVE:
            current_cache_type = TWO_WAY_SET_ASSOCIATIVE;
            associativity = 2;
            break;
    }
    cacheUsed = true;
    num_sets = NUM_CACHE_LINES / associativity;

    SET_BITS = log2(num_sets);
    TAG_BITS = 32u - OFFSET_BITS - SET_BITS;
    OFFSET_MASK = (1u << OFFSET_BITS) - 1;
    SET_MASK = (1u << SET_BITS) - 1;

    cache = new Line*[num_sets];

    for (size_t s = 0; s < num_sets; ++s) {
        cache[s] = new Line[associativity];
    }

    for (size_t s = 0; s < num_sets; ++s) {
        for (size_t w = 0; w < associativity; ++w)
            cache[s][w].badline();
    }
}

// used for debugging problems with cache initialization
void free_cache() {
    if (!cache) return;
    for (size_t s = 0; s < num_sets; s++)
        delete[] cache[s];
    delete[] cache;
    cache = nullptr;
}
//-------------------------------

//----------- MAIN FETCH DECODE EXECUTE LOOP -----------

bool fetch() {
    if (reg_file[PC] + INSTR_SIZE > mem_size) return false;  // about to run out of memory

    uint32_t inter = readWord(reg_file[PC]);
    cntrl_regs[OPERATION] = inter & 0xFF;  // unrolled
    cntrl_regs[OPERAND_1] = (inter >> 8) & 0xFF;
    cntrl_regs[OPERAND_2] = (inter >> 16) & 0xFF;
    cntrl_regs[OPERAND_3] = (inter >> 24) & 0xFF;

    if (!cacheUsed) fetching_second = true;

    reg_file[PC] += 4;

    cntrl_regs[IMMEDIATE] = (readWord(reg_file[PC]));
    reg_file[PC] += 4;

    if (!cacheUsed) fetching_second = false;

    return true;
}

bool decode() {
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

        case OP_JMR: {
            const uint32_t rs = cntrl_regs[OPERAND_1];

            if (!is_valid_rg(rs)) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            return true;
        }

        case OP_BNZ: {
            const uint32_t rs = cntrl_regs[OPERAND_1];

            if (!is_valid_rg(rs)) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            return true;
        }

        case OP_BGT: {
            const uint32_t rs = cntrl_regs[OPERAND_1];

            if (!is_valid_rg(rs)) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            return true;
        }

        case OP_BLT: {
            const uint32_t rs = cntrl_regs[OPERAND_1];

            if (!is_valid_rg(rs)) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            return true;
        }

        case OP_BRZ: {
            const uint32_t rs = cntrl_regs[OPERAND_1];

            if (!is_valid_rg(rs)) return false;
            if (!is_valid_addr(cntrl_regs[IMMEDIATE])) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            return true;
        }

        case OP_MOV: {
            // operand 1 RD
            // operand 2 RS
            // operand 3 DC
            // immediate value DC
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs = cntrl_regs[OPERAND_2];

            if (!is_valid_rg(rd)) return false;
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

            if (!is_valid_rg(rd)) return false;

            return true;
        }

        case OP_LDA: {
            // operand 1 RD
            // operand 2 DC
            // operand 3 DC
            // immediate value IMM
            const uint32_t rd = cntrl_regs[OPERAND_1];

            if (!is_valid_rg(rd)) return false;
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
            if (!is_valid_rg(rd)) return false;
            return true;
        }

        case OP_STB: {
            // Store least significant byte in RS at address
            // operand 1 RS
            // operand 2 DC
            // operand 3 DC
            // immediate value ADDRESS
            const uint32_t rs = cntrl_regs[OPERAND_1];
            if (!is_valid_rg(rs)) return false;

            data_regs[REG_VAL_1] = reg_file[rs] & 0xFFu;  // least significant byte
            return true;
        }

        case OP_LDB: {
            // load byte at address to RD
            // operand 1 RD
            // operand 2 DC
            // operand 3 DC
            // immediate value ADDRESS
            const uint32_t rd = cntrl_regs[OPERAND_1];
            if (!is_valid_rg(rd)) return false;

            return true;
        }

        case OP_ISTR: {
            const uint32_t rs = cntrl_regs[OPERAND_1];
            const uint32_t rg = cntrl_regs[OPERAND_2];

            if (!is_valid_rg(rs)) return false;
            if (!is_valid_rg(rg)) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            data_regs[REG_VAL_2] = reg_file[rg];
            return true;
        }

        case OP_ILDR: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rg = cntrl_regs[OPERAND_2];

            if (!is_valid_rg(rd)) return false;
            if (!is_valid_rg(rg)) return false;

            data_regs[REG_VAL_1] = reg_file[rg];

            return true;
        }

        case OP_ISTB: {
            const uint32_t rs = cntrl_regs[OPERAND_1];
            const uint32_t rg = cntrl_regs[OPERAND_2];

            if (!is_valid_rg(rs)) return false;
            if (!is_valid_rg(rg)) return false;

            data_regs[REG_VAL_1] = reg_file[rs];
            data_regs[REG_VAL_2] = reg_file[rg];
            return true;
        }

        case OP_ILDB: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rg = cntrl_regs[OPERAND_2];

            if (!is_valid_rg(rd)) return false;
            if (!is_valid_rg(rg)) return false;

            data_regs[REG_VAL_1] = reg_file[rg];

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

            if (!igr(rd) || !igr(rs1) || !igr(rs2)) return false;

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

            if (!igr(rd) || !igr(rs1)) return false;

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

            if (!igr(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_SUBI: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!igr(rd) || !igr(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];

            return true;
        }

        case OP_MUL: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!igr(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_MULI: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!igr(rd) || !igr(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];

            return true;
        }

        case OP_DIV: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!igr(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_SDIV: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!igr(rd) || !igr(rs1) || !igr(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_DIVI: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!igr(rd) || !igr(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];

            return true;
        }

        case OP_CMP: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];
            const uint32_t rs2 = cntrl_regs[OPERAND_3];

            if (!is_valid_rg(rd)) return false;
            if (!is_valid_rg(rs1)) return false;
            if (!is_valid_rg(rs2)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            data_regs[REG_VAL_2] = reg_file[rs2];

            return true;
        }

        case OP_CMPI: {
            const uint32_t rd = cntrl_regs[OPERAND_1];
            const uint32_t rs1 = cntrl_regs[OPERAND_2];

            if (!is_valid_rg(rd)) return false;
            if (!is_valid_rg(rs1)) return false;

            data_regs[REG_VAL_1] = reg_file[rs1];
            return true;
        }

        case OP_TRP: {
            // valid values are 0-4 unsigned ints, and 98
            uint32_t imm = cntrl_regs[IMMEDIATE];

            if (imm <= 4 || imm == 98)
                return true;
            else
                return false;
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
        case OP_JMR:
            return JMR();
        case OP_BNZ:
            return BNZ();
        case OP_BGT:
            return BGT();
        case OP_BLT:
            return BLT();
        case OP_BRZ:
            return BRZ();
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
        case OP_ISTR:
            return ISTR();
        case OP_ILDR:
            return ILDR();
        case OP_ISTB:
            return ISTB();
        case OP_ILDB:
            return ILDB();
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
        case OP_CMP:
            return CMP();
        case OP_CMPI:
            return CMPI();
        case OP_TRP:
            return TRP();
        defaut:
            return false;
    }

    return false;  // catch in case something went horribly wrong
}

int runEmulator(int argc, char** argv) {
    if (argc < 2) {
        cout
            << "\nUsage: " << argv[0] << " [-m MEMORY] [-c CACHE] INPUT_BINARY_FILE\n\n"
            << "Options:\n"
            << "  -m <size>      Set reserved memory size (bytes).  \n"
            << "                   Default: 131,072 bytes (128 KiB)\n"
            << "  -c <config>    Cache configuration.  One of:\n"
            << "\t\t    0 No Cache\n\t\t    1 Direct Mapped Cache\n\t\t    2 Fully Associative Cache\n\t\t    3 2-Way Set Associative Cache\n"
            << "\n"
            << "INPUT_BINARY_FILE:\n"
            << "                   Path to 4380 bytecode binary.\n";
        return 1;
    }

    uint32_t desired_memory = 131'072;
    int cache_config = 0;
    string input_file;

    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "-m") {
            if (i + 1 == argc) {
                cout << "Invalid memory configuaration. Aborting.\n";
                return 2;
            }
            string val = argv[++i];
            size_t pos;
            uint64_t tmp = stoull(val, &pos);
            if (pos != val.size() || tmp == 0 || tmp > UINT32_MAX) {
                cout << "Invalid memory configuaration. Aborting.\n";
                return 2;
            }
            desired_memory = static_cast<uint32_t>(tmp);

        } else if (a == "-c") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                string val = argv[++i];
                if (val == "0")  // no cache
                    cache_config = 0;
                else if (val == "1")  // direct mapped
                    cache_config = 1;
                else if (val == "2")  // fully associative
                    cache_config = 2;
                else if (val == "3")  // 2-way set associative
                    cache_config = 3;
                else {
                    cout << "Invalid cache configuration. Aborting.\n";
                    return 2;
                }
            }

        } else if (input_file.empty()) {
            input_file = a;
        } else {
            cout
                << "\nUsage: " << argv[0] << " [-m MEMORY] [-c CACHE] INPUT_BINARY_FILE\n\n"
                << "Options:\n"
                << "  -m <size>      Set reserved memory size (bytes).  \n"
                << "                   Default: 131,072 bytes (128 KiB)\n"
                << "  -c <config>    Cache configuration.  One of:\n"
                << "\t\t    0 No Cache\n\t\t    1 Direct Mapped Cache\n\t\t    2 Fully Associative Cache\n\t\t    3 2-Way Set Associative Cache\n"
                << "\n"
                << "INPUT_BINARY_FILE:\n"
                << "                   Path to 4380 bytecode binary.\n";
            ;
            return 1;
        }
    }
    if (input_file.empty()) {
        cout
            << "\nUsage: " << argv[0] << " [-m MEMORY] [-c CACHE] INPUT_BINARY_FILE\n\n"
            << "Options:\n"
            << "  -m <size>      Set reserved memory size (bytes).  \n"
            << "                   Default: 131,072 bytes (128 KiB)\n"
            << "  -c <config>    Cache configuration.  One of:\n"
            << "\t\t    0 No Cache\n\t\t    1 Direct Mapped Cache\n\t\t    2 Fully Associative Cache\n\t\t    3 2-Way Set Associative Cache\n"
            << "\n"
            << "INPUT_BINARY_FILE:\n"
            << "                   Path to 4380 bytecode binary.\n";
        ;
        return 1;
    }

    mem_size = desired_memory;
    if (!init_mem(mem_size)) return 1;

    init_cache(cache_config);

    unsigned int rc = load_binary(input_file.c_str());
    if (rc == 1) {
        cerr << "Cannot open file: " << input_file << "\n";
        return 1;
    }
    if (rc == 2) {
        cerr << "INSUFFICIENT MEMORY SPACE\n";
        return 2;
    }

    while (runBool) {
        if (!fetch() || !decode() || !execute()) {
            invalidInstruction();
            return 1;
        }
    }

    // dumpCacheSummary();
    return 0;
}

//------------------ START OF EXECUTE HELPER FUNCTIONS ------------------

// jump execution functions
bool JMP() {
    try {
        reg_file[PC] = cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in JMP" << endl;
        return false;
    }
}

bool JMR() {
    // update PC to value in RS
    //  operand 1 RS
    //  operand 2 DC
    //  operand 3 DC
    //  immediate DC
    try {
        reg_file[PC] = data_regs[REG_VAL_1];
        return true;
    } catch (const exception&) {
        cerr << "Error in JMR" << endl;
        return false;
    }
}

bool BNZ() {
    // update PC to address if rs != 0
    // operand 1 RS
    // operand 2 DC
    // operand 3 DC
    // immediate ADDRESS
    try {
        const uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr, 4)) return false;

        if (data_regs[REG_VAL_1] != 0)
            reg_file[PC] = addr;

        return true;

    } catch (const exception&) {
        cerr << "Error in BNZ" << endl;
        return false;
    }
}

bool BGT() {
    // operand 1 RS
    // operand 2 DC
    // operand 3 DC
    // immediate ADDRESS

    try {
        const uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr, 4)) return false;

        if (static_cast<int32_t>(data_regs[REG_VAL_1]) > 0)
            reg_file[PC] = addr;

        return true;

    } catch (const exception&) {
        cerr << "Error in BGT" << endl;
        return false;
    }
}

bool BLT() {
    // operand 1 RS
    // operand 2 DC
    // operand 3 DC
    // immediate  ADDRESS

    try {
        const uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr, 4)) return false;

        if (static_cast<int32_t>(data_regs[REG_VAL_1]) < 0)
            reg_file[PC] = addr;

        return true;

    } catch (const exception&) {
        cerr << "Error in BLT" << endl;
        return false;
    }
}

bool BRZ() {
    // operand 1 RS
    // operand 2 DC
    // operand 3 DC
    // immediate ADDRESS

    try {
        const uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr, 4)) return false;

        if (static_cast<int32_t>(data_regs[REG_VAL_1]) == 0)
            reg_file[PC] = addr;

        return true;

    } catch (const exception&) {
        cerr << "Error in BRZ" << endl;
        return false;
    }
}

// move execution functions
bool MOV() {
    // operand 1 RD
    // operand 2 RS
    // operand 3 DC
    // immediate value DC
    try {
        reg_file[cntrl_regs[OPERAND_1]] = data_regs[REG_VAL_1];
        return true;
    } catch (const exception&) {
        cerr << "Error in MOV" << endl;
        return false;
    }
}

bool MOVI() {
    // operand 1 RD
    // operand 2 DC
    // operand 3 DC
    // immediate value IMM
    try {
        reg_file[cntrl_regs[OPERAND_1]] = cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in MOVI" << endl;
        return false;
    }
}

bool LDA() {
    // operand 1 RD
    // operand 2 DC
    // operand 3 DC
    // immediate value IMM
    try {
        reg_file[cntrl_regs[OPERAND_1]] = cntrl_regs[IMMEDIATE];
        return true;
    } catch (const exception&) {
        cerr << "Error in LDA" << endl;
        return false;
    }
}

bool STR() {
    // operand 1 RS
    // operand 2 DC
    // operand 3 DC
    // immediate value ADDRESS
    try {
        uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr, 4)) return false;

        uint32_t val = data_regs[REG_VAL_1];
        writeWord(addr, val);

        return true;

    } catch (const exception&) {
        cerr << "Error in STR" << endl;
        return false;
    }
}

bool LDR() {
    // operand 1 RD
    // operand 2 DC
    // operand 3 DC
    // immediate value ADDRESS
    try {
        const uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr, 4)) return false;

        reg_file[cntrl_regs[OPERAND_1]] = readWord(addr);
        return true;

    } catch (const exception&) {
        cerr << "Error in LDR" << endl;
        return false;
    }
}

bool STB() {
    // operand 1 RS
    // operand 2 DC
    // operand 3 DC
    // immediate value ADDRESS
    // store leasssssst ___|^~ (that's a snake) significant byte in RS at address
    try {
        const uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr)) return false;

        writeByte(addr, static_cast<unsigned char>(data_regs[REG_VAL_1]));
        return true;
    } catch (const exception&) {
        cerr << "Error in STB" << endl;
        return false;
    }
}

bool LDB() {
    // operand 1 RD
    // operand 2 DC
    // operand 3 DC
    // immediate value ADDRESS
    // load byte at address to RD
    try {
        const uint32_t addr = cntrl_regs[IMMEDIATE];
        if (!addr_in_range(addr)) return false;

        reg_file[cntrl_regs[OPERAND_1]] = static_cast<uint32_t>(readByte(addr));
        return true;
    } catch (const exception&) {
        cerr << "Error in LDB" << endl;
        return false;
    }
}

bool ISTR() {
    // Store integer in RS at address in RG
    //  operand 1 RS
    //  operand 2 RG
    //  operand 3 DC
    //  immediate DC
    try {
        uint32_t value = data_regs[REG_VAL_1];  // rs
        uint32_t addr = data_regs[REG_VAL_2];   // rg

        if (!addr_in_range(addr, 4)) return false;

        writeWord(addr, value);

        return true;

    } catch (const exception&) {
        cerr << "Error in ISTR" << endl;
        return false;
    }
    return false;
}

bool ILDR() {
    // Load integer at address in RG into RD
    //   operand 1 RD
    //   operand 2 RG
    //   operand 3 DC
    //   immediate DC

    try {
        uint32_t addr = data_regs[REG_VAL_1];

        if (!addr_in_range(addr, sizeof(uint32_t))) return false;

        uint32_t rd = cntrl_regs[OPERAND_1];
        uint32_t val = readWord(addr);

        reg_file[rd] = val;

        return true;

    } catch (const exception&) {
        cerr << "Error in ILDR" << endl;
        return false;
    }
    return false;
}

bool ISTB() {
    // Store byte in RS at address in RG
    //  operand 1 RS
    //  operand 2 RG
    //  operand 3 DC
    //  immediate DC

    try {
        uint32_t val = data_regs[REG_VAL_1] & 0xFFu;
        uint32_t addr = data_regs[REG_VAL_2];

        if (!addr_in_range(addr, 4)) return false;

        writeByte(addr, static_cast<unsigned char>(val));
        return true;

    } catch (const exception&) {
        cerr << "Error in ISTB" << endl;
        return false;
    }
    return false;
}

bool ILDB() {
    // Load byte at address in RG into RD
    // operand 1 RD
    // operand 2 RG
    // operand 3 DC
    // immediate DC
    try {
        const uint32_t addr = data_regs[REG_VAL_1];
        if (!addr_in_range(addr)) return false;
        uint32_t rd = cntrl_regs[OPERAND_1];

        reg_file[rd] = static_cast<uint8_t>(readByte(addr));
        return true;

    } catch (const exception&) {
        cerr << "Error in ILDB" << endl;
        return false;
    }
    return false;
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
    uint32_t dividend = data_regs[REG_VAL_1];
    uint32_t divisor = data_regs[REG_VAL_2];

    if (divisor == 0) return false;  // illegal, cannot divide by zero.

    reg_file[cntrl_regs[OPERAND_1]] = dividend / divisor;  // unsigned division
    return true;
}

bool SDIV() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 RS2
    // immediate value DC
    int32_t dividend = static_cast<int32_t>(data_regs[REG_VAL_1]);
    int32_t divisor = static_cast<int32_t>(data_regs[REG_VAL_2]);

    if (divisor == 0) return false;  // illegal, cannot divide by zero

    reg_file[cntrl_regs[OPERAND_1]] = static_cast<uint32_t>(dividend / divisor);  // signed divison
    return true;
}

bool DIVI() {
    // operand 1 RD
    // operand 2 RS1
    // operand 3 DC
    // immediate IMM
    // signed immediate divison
    int32_t dividend = static_cast<int32_t>(data_regs[REG_VAL_1]);
    int32_t divisor = static_cast<int32_t>(cntrl_regs[IMMEDIATE]);

    if (divisor == 0) return false;  // illegal, cannot divide by zero

    reg_file[cntrl_regs[OPERAND_1]] = static_cast<uint32_t>(dividend / divisor);

    return true;
}

// compare execution functions
bool CMP() {
    // Performs a signed comparison between RS1 and RS2, and stores the result in RD
    // Set RD = 0 if RS1 == RS2 OR set RD = 1 if RS1 >RS2 OR set RD = -1 if RS1 < RS2

    //  operand 1 RD
    //  operand 2 RS1
    //  operand 3 RS2
    //  immediate DC
    //  SIGNED COMPARISON
    try {
        uint32_t rd = cntrl_regs[OPERAND_1];
        int32_t rs1 = static_cast<int32_t>(data_regs[REG_VAL_1]);
        int32_t rs2 = static_cast<int32_t>(data_regs[REG_VAL_2]);

        if (rs1 > rs2)
            reg_file[rd] = 1;
        else if (rs1 < rs2)
            reg_file[rd] = static_cast<uint32_t>(-1);
        else
            reg_file[rd] = 0;

        return true;

    } catch (const exception&) {
        cerr << "Error in CMP" << endl;
        return false;
    }
    return false;
}

bool CMPI() {
    // Performs a signed comparison between RS1 and IMM and stores the result in RD
    // Set RD = 0 if RS1 == IMM OR set RD = 1 if RS1 >IMM OR set RD = -1 if RS1 < IMM

    //  operand 1 RD
    //  operand 2 RS1
    //  operand 3 DC
    //  immediate IMM
    //  SIGNED COMPARISON
    try {
        uint32_t rd = cntrl_regs[OPERAND_1];
        int32_t rs1 = static_cast<int32_t>(data_regs[REG_VAL_1]);
        int32_t imm = cntrl_regs[IMMEDIATE];

        if (rs1 > imm)
            reg_file[rd] = 1;
        else if (rs1 < imm)
            reg_file[rd] = static_cast<uint32_t>(-1);
        else
            reg_file[rd] = 0;

        return true;

    } catch (const exception&) {
        cerr << "Error in CMPI" << endl;
        return false;
    }
    return false;
}

// trap/interrupt execution functions
bool TRP() {
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
    uint32_t imm = cntrl_regs[IMMEDIATE];

    switch (imm) {
        case 0: {
            cout << "Execution completed. Total memory cycles: " << mem_cycle_cntr << endl;
            // dumpCacheSummary();
            // dumpRegisterContents();
            // dumpMemory(prog_mem, mem_size);
            runBool = false;
            return true;
        }
        // write int in r3 to stdout (console)
        case 1: {
            cout << static_cast<uint32_t>(reg_file[R3]) << flush;  // flush the buffer to make sure that it writes. cast to make sure that its an int going out
            return true;
        }
        // read an integer into R3 from stdin
        case 2: {
            uint32_t inInt;
            if (!(cin >> inInt)) return false;            // fails if it wasnt able to get anything from cin
            reg_file[R3] = static_cast<uint32_t>(inInt);  // casts to unsigned int
            return true;
        }
        // write char in R3 to stdout
        case 3: {
            cout << static_cast<char>(reg_file[R3]) << flush;  // cast to make sure a char gets out there, flush to make sure that buffer goes out
            return true;
        }
        // read a char into R3 from stdin
        case 4: {
            char inChar;
            if (!(cin >> inChar)) return false;
            reg_file[R3] = static_cast<uint32_t>(inChar);
            return true;
        }
        case 98: {
            try {
                dumpRegisterContents();
                return true;
            } catch (const exception&) {
                cout << "ERROR IN TRP 98:" << endl;
                return false;
            }
            return false;
        }
        default: {
            return false;  // in case something went horribly wrong, say it went wrong
        }
    }

    // IMM 0    -> EXECUTE STOP / EXIT ROUTINE
}

//------------------ END OF EXECUTE HELPER FUNCTIONS ------------------

void STOP() {
    runBool = false;
    return;
}

// Prints one register name and value per line. Register name is in all caps, followed by a tab character, followed by the integer value printed as an unsigned base 10 integer.
void dumpRegisterContents() {
    // cout << "Dumping contents, pc currently at: " << reg_file[PC] << endl;
    for (size_t i = 0; i < NUM_REGS; i++) {
        cout << REG_NAMES[i] << "\t" << static_cast<uint32_t>(reg_file[i]) << endl;
    }
    return;
}

void dumpMemory(const uint8_t* mem, size_t size) {
    for (size_t addr = 0; addr < size; addr += 16) {
        // 1) print the offset
        std::cout
            << std::setw(8) << std::setfill('0') << std::hex << addr
            << "  ";

        // 2) print up to 16 bytes
        for (size_t j = 0; j < 16; ++j) {
            if (addr + j < size) {
                std::cout
                    << std::setw(2)
                    << static_cast<int>(mem[addr + j])
                    << ' ';
            } else {
                std::cout << "   ";  // pad past end
            }
        }

        std::cout << '\n';
    }

    // restore defaults
    std::cout << std::dec << std::setfill(' ');
}
static void dumpBlock(const uint8_t* data,
                      size_t bytes,
                      bool showOffset) {
    constexpr size_t BYTES_PER_ROW = 16;
    for (size_t i = 0; i < bytes; i += BYTES_PER_ROW) {
        if (showOffset)
            cout << "      +" << setw(3) << setfill('0') << hex
                 << i << " : ";

        size_t rowBytes = min<size_t>(BYTES_PER_ROW, bytes - i);
        for (size_t j = 0; j < rowBytes; ++j)
            cout << uppercase << setw(2) << setfill('0')
                 << hex << static_cast<int>(data[i + j]) << ' ';
        cout << dec << '\n';
    }
}

void dumpCacheVerbose(bool showEmpty = false,
                      size_t maxSets = 0,
                      bool showOffsets = true) {
    cout << "\n================ Complete cache dump ================\n";
    cout << "  Sets   : " << num_sets << '\n';
    cout << "  Ways   : " << associativity << '\n';
    cout << "  BlkSz  : " << BLOCK_SIZE << " bytes\n";
    cout << "-----------------------------------------------------\n\n";

    size_t setsShown = 0;

    for (size_t set = 0; set < num_sets; ++set) {
        // Check if this set is entirely invalid (skip unless showEmpty==true)
        bool anyValid = false;
        for (size_t way = 0; way < associativity; ++way)
            if (cache[set][way].valid) {
                anyValid = true;
                break;
            }
        if (!anyValid && !showEmpty) continue;

        cout << "Set " << setw(3) << set << ":\n";

        for (size_t way = 0; way < associativity; ++way) {
            const Line& line = cache[set][way];
            if (!line.valid && !showEmpty) continue;

            cout << "  Way " << way
                 << " | V:" << line.valid
                 << " D:" << line.dirty
                 << " Tag:0x" << uppercase << hex << line.tag
                 << dec
                 << "  LRU:" << line.lastused
                 << '\n';

            dumpBlock(line.data, BLOCK_SIZE, showOffsets);
        }
        cout << '\n';

        if (maxSets && ++setsShown >= maxSets) break;
    }

    cout << "================ End of cache dump =================\n";
}
void dumpCacheSummary() {
    if (!cacheUsed || cache == nullptr) {
        cout << "Cache not used, nothing to dump" << endl;
        return;
    }
    dumpCacheVerbose();

    const char* typeStr = "UNKNOWN";
    switch (current_cache_type) {
        case NO_CACHE:
            typeStr = "No cache";
            break;
        case DIRECT_MAPPED:
            typeStr = "Direct mapped";
            break;
        case FULLY_ASSOCIATIVE:
            typeStr = "Fully associative";
            break;
        case TWO_WAY_SET_ASSOCIATIVE:
            typeStr = "2-way set associative";
            break;
    }
    // dumpCacheVerbose(true, 99);
    size_t lineSize = sizeof(Line);                  // bytes / line object
    size_t cacheBytes = NUM_CACHE_LINES * lineSize;  // total capacity

    cout << "\n=========== Cache summary ===========\n";
    cout << left << setw(22) << "Cache type:" << typeStr << '\n';
    cout << setw(22) << "Block size:" << BLOCK_SIZE << "  bytes\n";
    cout << setw(22) << "# cache lines:" << NUM_CACHE_LINES << '\n';
    cout << setw(22) << "Associativity:" << associativity << ((associativity == 1) ? " (direct-mapped)" : "") << '\n';
    cout << setw(22) << "# sets:" << num_sets << '\n';
    cout << setw(22) << "Line object size:" << lineSize << "  bytes\n";
    cout << setw(22) << "Total cache size:" << cacheBytes << "  bytes\n";
    cout << "======================================\n";
}

void invalidInstruction() {
    cout << "INVALID INSTRUCTION AT: " << (reg_file[PC] - 8) << endl;
}
