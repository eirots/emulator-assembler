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
extern unsigned char* prog_mem;  // default size is 131_072 elements/bytes
extern uint32_t cntrl_regs[5];   // stores instruction operation, register operands, and immediate value
extern uint32_t data_regs[2];    // stores register operand values retrieved from register file
extern uint32_t mem_size;
constexpr size_t NUM_REGS = 22;

// enums
enum RegNames : std::size_t { R0 = 0,  // general purpose registers
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
enum CntrlRegNames { OPERATION = 0,  // opcode
                     OPERAND_1 = 1,
                     OPERAND_2 = 2,
                     OPERAND_3 = 3,
                     IMMEDIATE = 4
};
enum DataRegNames { REG_VAL_1 = 0,
                    REG_VAL_2 = 1
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
 * @return FALSE if invalid instruction is encounterd, otherwise TRUE
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

#endif