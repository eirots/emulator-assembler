#ifndef emu4380_h_
#define emu4380_h_

// contains function prototypes and declarations for structural elements of the processor
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>

using namespace std;

// Global variables
extern uint32_t* reg_file;

extern unsigned char* prog_mem;  // default size is 131,072 elements/bytes

extern uint32_t cntrl_regs[5];  // stores instruction operation, register operands, and immediate value
extern uint32_t data_regs[2];   // stores register operand values retrieved from register file
extern uint32_t mem_size;

extern bool runBool;  // boolean for executing fetch, decode, execute

// enums
enum RegNames : std::size_t {
    R0 = 0,  // general purpose registers
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8,
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
 * @brief First call to start the emulator, used heavily in testing
 * @return ints that you would expect from a main.
 */
int runEmulator(int argc, char** argv);

//------------START OF OPERATION FUNCTIONS------------

// jump instructions

/**
 * @brief Jump to address
 */
bool JMP();

// move instructions

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

// arithmetic functions
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

// trap/interrupt functions
/**
 * @brief function for traps
 * @note based on immediate value
 */
bool TRP();

/**
 * @brief executes the stop routine
 * @details toggles runBool, a value used to execute the main fetch, decode, execute loop.
 */
void STOP();

/**
 * @brief Prints all register contents to stdout
 * @details Prints one register name and value per line. Register name is in all caps, followed by a tab character, followed by the integer value printed as an unsigned base 10 integer.
 */
void dumpRegisterContents();

void invalidInstruction();

#endif