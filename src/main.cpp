#include <iostream>

#include "emu4380.h"
#include "utils.h"

using namespace std;

// first argument is assumed to contain a binary input file containing 4380 byte-code
// function will read contents into program memory
// then initialize the PC register
// then enter an execution loop that calls fetch(), decode(), and execute functions UNTIL:
// trp 0 or illegal instruction are encountered

// USAGE: first arg is mandatory, name of binary file containing 4380 byte code
//           second arg is desired memory size (in bytes), positive integer with upper bound 4_294_967_295
int main(int argc, char** argv) {
    return runEmulator(argc, argv);
}
