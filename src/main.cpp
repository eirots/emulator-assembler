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
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " INPUT_BINARY_FILE [RESERVED_MEMORY]\n";
        cout << "INPUT_BINARY_FILE: \t\tPath to file containing 4380 byte code\n";
        cout << "RESERVED_MEMORY (optional):\tPositive integer, default 131,072 bytes, max 4,294,967,295\n";
        return 1;
    }

    if (argc < 3) {
        mem_size = 131'072;
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

    while (runBool) {
        if (!fetch() || !decode() || !execute()) {
            invalidInstruction();
            return 1;
        }
    }
    return 0;
}
