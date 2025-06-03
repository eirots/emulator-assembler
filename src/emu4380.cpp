#include "emu4380.h"

bool fetch() {
    // TODO:
    return false;
}

bool decode() {
    return false;
}

bool execute() {
    // TODO:
    return false;
}

bool init_mem(unsigned int size) {
    // TODO:
    return false;
}

int runEmulator(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " INPUT_BINARY_FILE [RESERVED_MEMORY]\n";
        cout << "INPUT_BINARY_FILE: \t\tPath to file containing 4380 byte code\n";
        cout << "RESERVED_MEMORY (optional):\tPositive integer, default 131,072 bytes, max 4,294,967,295\n";
        return 1;
    }
    uint32_t mem_size;

    if (argc < 3) {
        // default minimum size
        mem_size = 131'072;
        cout << "memsize defaulted to: " << mem_size << endl;
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

        cout << "memsize set to: " << mem_size << endl;
    }
    cout << "path given was: " << argv[1] << endl;

    return 0;
}