#ifndef emu4380_h_
#define emu4380_h_

// contains function prototypes and declarations for structural elements of the processor
#include <cstdint>

// Global variables
extern uint32_t* reg_file;
extern unsigned char* prog_mem;
extern uint32_t cntrl_regs[5];
extern uint32_t data_regs[2];

// enums
enum RegNames {};
enum CntrlRegNames {};
enum DataRegNames {};

// function prototypes
bool fetch();
bool decode();
bool execute();
bool init_mem(unsigned int size);

#endif