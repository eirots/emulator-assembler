#ifndef emu4380_h_
#define emu4380_h_

// contains function prototypes and declarations for structural elements of the processor

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

using namespace std;

// Global variables
extern uint32_t* reg_file;
extern unsigned char* prog_mem;  // default size is 131,072 elements/bytes

extern uint32_t cntrl_regs[5];  // stores instruction operation, register operands, and immediate value
extern uint32_t data_regs[2];   // stores register operand values retrieved from register file
extern uint32_t mem_size;
extern uint32_t mem_cycle_cntr;

extern bool runBool;  // boolean for executing fetch, decode, execute
extern bool cacheUsed;

constexpr uint32_t BLOCK_SIZE = 16;
constexpr uint32_t NUM_CACHE_LINES = 64;

extern size_t associativity;
extern size_t num_sets;
extern uint32_t SET_BITS;
extern uint32_t TAG_BITS;

const uint32_t OFFSET_BITS = log2(BLOCK_SIZE);

struct Line {
    uint32_t tag = 0;
    bool valid = false;
    bool dirty = false;
    uint8_t data[BLOCK_SIZE]{};
    size_t lastused = 0;

    void badline() noexcept {
        valid = false;
        dirty = false;
        tag = 0;
    }
};

extern Line** cache;
// extern uint8_t* callstack;
//  extern PointerStack stack;

// enums

enum RegNames : std::size_t {
    /**
     * WHEN USING THIS EMULATOR
     *          Best practice, Use registers R0-R7 as caller registers, and R8-R15 for the callee.
     */
    R0 = 0,  // general purpose registers, R0-R7 are for function callers.
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8,  // General purpose regsiters, R8-R15 are for function callees.
    R9 = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    R14 = 14,
    R15 = 15,
    PC = 16,  // program counter, initialized to the address of first instruction in memory
    SL = 17,  // stack lower limit (lowest legal address available to stack)
    SB = 18,  // stack bottom (highest address)
    SP = 19,  // stack pointer (latest allocated byte on the stack, grows downward)
    FP = 20,  // frame pointer (points to first word beneath return address)
    HP = 21   // heap pointer (initially set to SL, grows upwards)
};

enum CntrlRegNames {
    OPERATION = 0,  // opcode
    OPERAND_1 = 1,
    OPERAND_2 = 2,
    OPERAND_3 = 3,
    IMMEDIATE = 4
};

enum DataRegNames {
    REG_VAL_1 = 0,
    REG_VAL_2 = 1
};

enum Opcode : std::uint8_t {
    OP_JMP  = 0x01,    // 1
    OP_JMR  = 0x02,    // 2
    OP_BNZ  = 0x03,    // 3
    OP_BGT  = 0x04,    // 4
    OP_BLT  = 0x05,    // 5
    OP_BRZ  = 0x06,    // 6
    OP_MOV  = 0x07,    // 7
    OP_MOVI  = 0x08,   // 8
    OP_LDA  = 0x09,    // 9
    OP_STR  = 0x0A,    // 10
    OP_LDR  = 0x0B,    // 11
    OP_STB  = 0x0C,    // 12
    OP_LDB  = 0x0D,    // 13
    OP_ISTR = 0x0E,   // 14
    OP_ILDR = 0x0F,   // 15
    OP_ISTB = 0x10,   // 16
    OP_ILDB = 0x11,   // 17
    OP_ADD  = 0x12,    // 18
    OP_ADDI = 0x13,   // 19
    OP_SUB  = 0x14,    // 20
    OP_SUBI = 0x15,   // 21
    OP_MUL  = 0x16,    // 22
    OP_MULI = 0x17,   // 23
    OP_DIV  = 0x18,    // 24
    OP_SDIV = 0x19,   // 25
    OP_DIVI = 0x1A,   // 26
    OP_AND  = 0x1B,    // 27
    OP_OR   = 0x1C,     // 28
    OP_CMP  = 0x1D,    // 29
    OP_CMPI = 0x1E,   // 30
    OP_TRP  = 0x1F,    // 31
    OP_ALCI = 0x20,   // 32
    OP_ALLC = 0x21,   // 33
    OP_IALLC = 0x22,  // 34
    OP_PSHR = 0x23,   // 35
    OP_PSHB = 0x24,   // 36
    OP_POPR = 0x25,   // 37
    OP_POPB = 0x26,   // 38
    OP_CALL = 0x27,   // 39
    OP_RET  = 0x28     // 40
};

// used in `init_cache()` for determining which kind of cache to use.
enum CacheType : std::uint32_t {
    NO_CACHE = 0,
    DIRECT_MAPPED = 1,
    FULLY_ASSOCIATIVE = 2,
    TWO_WAY_SET_ASSOCIATIVE = 3
};

extern CacheType current_cache_type;

// used in cache functions `readWord()`, `readByte()`. `writeWord()`, and `writeByte()`.
enum AccessType {
    READBYTE,
    READWORD,
    WRITEBYTE,
    WRITEWORD

};

// function prototypes
/**
 * @brief Retrieves the bytes for the next instruction, and places them in the appropriate cntrl_regs
 * @details Also increments the PC so it points at the next instruction.
 * @return FALSE if invalid fetch location (OOB) is encountered, otherwise TRUE
 */
bool fetch();

/**
 * @brief Verifies that the specified operation (or TRP) and operands specified in cntrl_regs are valid. Also retrieves register values from register file and places these in appropriate data_regs as indicated by operands present in cntrl_regs
 * @details A MOV instruction operates on state registers, and there are a limited number of these; a MOV with an RD value of 55 would be a malformed instruction.
 * @return FALSE if invalid instruction is encountered, otherwise TRUE
 */
bool decode();

/**
 * @brief Executes the effects of decoded and validated instruction or TRP on the state members (regs, memory, etc.) as indicated by cntrl_regs and data_regs, and in accordance with instruction or TRP's specification
 * @return FALSE if illegal operation is encountered (does not execute instruction), otherwise TRUE
 */
bool execute();

/**
 * @brief Helper that loads a binary file into program memory
 * @return 1 if file can't be opened, 2 if insufficient memory space, otherwise 0.
 */
uint32_t load_binary(const char* filename);

/**
 * @brief Dynamically allocates size bytes of memory for the program memory, initializes all values in array to zero, and stores address of this memory in prog_mem
 * @return FALSE if unable to initalize memory, otherwise TRUE
 */
bool init_mem(unsigned int size);

/**
 *@brief Initializes the emulator's cache
 *@details Must be called before memory access functions can be called.
 */
void init_cache(uint32_t cacheType);

/**
 * @brief frees the cache from memory
 */
void free_cache();
/**
 * @brief First call to start the emulator, used heavily in testing
 * @return ints that you would expect from a main.
 */
int runEmulator(int argc, char** argv);

//------------START OF OPERATION FUNCTIONS------------

// -----------------jump functions-----------------

/**
 * @brief Jump to address
 */
bool JMP();

/**
 * @brief Update PC to value in RS
 */
bool JMR();

/**
 * @brief Update PC to Address if RS != 0
 */
bool BNZ();

/**
 * @brief Update PC to Address if RS > 0
 */
bool BGT();

/**
 * @brief Update PC to Address if RS < 0
 */
bool BLT();

/**
 * @brief Update PC to Address if RS = 0
 */
bool BRZ();

// -----------------move functions-----------------

/**
 * @brief Move contents of RS to RD
 * @details
 * @return
 */
bool MOV();

/**
 * @brief Move IMM value into RD
 * @details
 * @return
 */
bool MOVI();

/**
 * @brief Load address into RD
 * @details
 * @return
 */
bool LDA();

/**
 * @brief Store integer in RS at address
 * @details
 * @return
 */
bool STR();

/**
 * @brief Load integer at Address to RD
 * @details
 * @return
 */
bool LDR();

/**
 * @brief Store least significant byte in RS at address
 * @details
 * @return
 */
bool STB();

/**
 * @brief Load byte at Address to RD
 * @details
 * @return
 */
bool LDB();

/**
 * @brief Store integer in RS at address in RG
 * @details
 * @return
 */
bool ISTR();

/**
 * @brief Load integer at address in RG into RD
 * @details
 * @return
 */
bool ILDR();

/**
 * @brief Store byte in RS at address in RG
 * @details
 * @return
 */
bool ISTB();

/**
 * @brief Load byte at address in RG into RD
 * @details
 * @return
 */
bool ILDB();

// -----------------arithmetic functions-----------------
/**
 * @brief Add RS1 to RS2, store result in RD
 * @details
 */
bool ADD();

/**
 * @brief Add Imm to RS1, store result in RD
 * @details
 */
bool ADDI();

/**
 * @brief Subtract RS2 from RS1, store result in RD
 * @details
 */
bool SUB();

/**
 * @brief Subtract Imm from RS1, store result in RD
 * @details
 */
bool SUBI();

/**
 * @brief Multiply RS1 by RS2, store result in RD
 * @details
 */
bool MUL();

/**
 * @brief Multiply RS1 by IMM, store the result in RD
 * @details
 */
bool MULI();

/**
 * @brief Perform unsigned integer division RS1 / RS2. Store quotient in RD
 * @details Division by zero shall result in an emulator error
 */
bool DIV();

/**
 * @brief Store result of signed division RS1 / RS2 in RD.
 * @detailsDivision by zero shall result in an emulator error
 */
bool SDIV();

/**
 * @brief Divide RS1 by IMM (signed), store the result in RD.
 * @details Division by zero shall result in an emulator error
 */
bool DIVI();

/**
 * @brief Performs a LOGICAL AND (&&) between RS1 and RS2, stores the result in RD.
 * @details 1 = True, 0 = False
 */
bool AND();

/**
 * @brief Performs a LOGICAL OR (||) between RS1 and RS2, stores the result in RD.
 * @details 1 = True, 0 = False
 */
bool OR();

// -----------------comparison functions-----------------
/**
 * @brief  Performs a signed comparison between RS1 and RS2, and stores the result in RD
 * @details Set RD = 0 if RS1 == RS2 OR set RD = 1 if RS1 >RS2 OR set RD = -1 if RS1 < RS2
 */
bool CMP();

/**
 * @brief  Performs a signed comparison between RS1 and IMM and stores the result in RD
 * @details Set RD = 0 if RS1 == IMM OR set RD = 1 if RS1 >IMM OR set RD = -1 if RS1 < IMM
 */
bool CMPI();

// -----------------trap/interrupt functions-----------------
/**
 * @brief function for traps
 * @note based on immediate value
 * @details Trp 0 ends program and outputs number of memory cycles. \n TRP 1 Writes an int in r3 to stdout. \n TRP2 Read an integer into r3 from stdout \n TRP3 Write character in R3 to stdout \n TRP4 Read a char into R3 from stdin \n TRP5 Writes the full null-terminated pascal-style string whose starting address is in R3 to stdout \n TRP6 Read a newline terminated string from stdin and stores it in memory as a null-terminated pascal-style string whose starting address is in R3.
 */
bool TRP();

/**
 * @brief Allocate imm bytes of space on the heap, and increment HP accordingly.
 * @details The imm value is a 4-byte unsigned integer. Store the initial heap pointer in RD.
 */
bool ALCI();

/**
 * @brief Allocate a number of bytes on the heap according to the value of the 4-byte unsigned integer stored at address
 * @details Also increment HP accordingly. Store the initial heap pointer in RD
 */
bool ALLC();

/**
 * @brief Indirectly allocate a number of bytes on the heap according to the value of the 4-byte uint at memory address stored in RS1.
 * @details Increments HP accordingly. Store the initial heap pointer in RD.
 */
bool IALLC();

/**
 * @brief Set SP = SP - 4, place the word in RS onto the stack
 */
bool PSHR();

/**
 * @brief Set SP = SP - 1, place the least significant byte in RS onto the stack
 */
bool PSHB();

/**
 * @brief place the word on top of the stack into RD, update SP = SP + 4
 */
bool POPR();

/**
 * @brief place the byte on top of the stack into RD, update SP = SP + 1
 */
bool POPB();

/**
 * @brief Push PC onto stack, update PC to Address
 */
bool CALL();

/**
 * @brief pop stack into PC
 */
bool RET();

/**
 * @brief executes the stop routine
 * @details toggles runBool, a value used to execute the main fetch, decode, execute loop.
 */
void STOP();

/**
 * @brief Prints all register contents to stdout
 * @details Prints one register name and value per line. Register name is in all caps, followed by a tab character, followed by the integer value printed as an unsigned base 10 integer.
 */

// -----------------Other helpers-----------------
void dumpRegisterContents();
void dumpCacheSummary();

void invalidInstruction();

//-----------------Timing functions-----------------
/**
 * @brief Returns the unsigned char located at index address in `prog_mem`.
 * @details Also increments the global `mem_cycle_cntr` by 8 when called.
 */
unsigned char readByte(uint32_t address);

/**
 * @brief Returns the unsigned int located at index address in `prog_mem`.
 * @details also increments the global `mem_cycle_cntr` by 8 when called.
 */
unsigned int readWord(uint32_t address);

/**
 * @brief Places the value in byte at index address in the `prog_mem` array.
 * @details also increments the global `mem_cycle_counter` by 8 when called
 */
void writeByte(uint32_t address, unsigned char byte);

/**
 * @brief Places the value in `word`, beginning at index `address` in `prog_mem` array
 * @details increments the global mem_cycle_cntr by 8 when called
 */
void writeWord(uint32_t address, unsigned int word);

/**
 * @brief Dumps the contents of the cache to console. Useful in debugging.
 */
void dumpCacheSummary();

/**
 * @brief dumps contents of memory to the console. Useful in debugging.
 */
void dumpMemory(const uint8_t* mem, size_t size);
#endif