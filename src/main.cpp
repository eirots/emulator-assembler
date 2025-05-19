#include "emu4380.h"
#include "utils.h"
using namespace std;

int main(int argc, char** argv) {
    // TODO:
    // first argument is assumed to contain a binary input file containing 4380 byte-code
    // function will read contents into program memory
    // then initialize the PC register
    // then enter an execution loop that calls fetch(), decode(), and execute functions UNTIL:
    // trp 0 or illegal instruction are encountered
    return 0;
}
