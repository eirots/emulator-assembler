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
void printInvalidArgs(string str) {
    cout
        << "\nUsage: " << str << " [-m MEMORY] [-c CACHE] INPUT_BINARY_FILE\n\n"
        << "Options:\n"
        << "  -m <size>      Set reserved memory size (bytes).  \n"
        << "                   Default: 131,072 bytes (128 KiB)\n"
        << "  -c <config>    Cache configuration.  One of:\n"
        << "\t\t    0 No Cache\n\t\t    1 Direct Mapped Cache\n\t\t    2 Fully Associative Cache\n\t\t    3 2-Way Set Associative Cache\n"
        << "\n"
        << "INPUT_BINARY_FILE:\n"
        << "                   Path to 4380 bytecode binary.\n";
}
void printBadCacheConfig() {
    cout << "Invalid cache configuration. Aborting.\n";
}
void printBadMem() {
    cout << "Invalid memory configuaration. Aborting.\n";
}
int main(int argc, char** argv) {
    if (argc < 2) {
        printInvalidArgs(argv[0]);
        return 1;
    }

    uint32_t desired_memory = 131'072;
    int cache_config = 0;
    string input_file;

    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "-m") {
            if (i + 1 == argc) {
                printBadMem();
                return 2;
            }
            string val = argv[++i];
            size_t pos;
            uint64_t tmp = stoull(val, &pos);
            if (pos != val.size() || tmp == 0 || tmp > UINT32_MAX) {
                printBadMem();
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
                    printBadCacheConfig();
                    return 2;
                }
            }

        } else if (input_file.empty()) {
            input_file = a;
        } else {
            printInvalidArgs(argv[0]);
            return 1;
        }
    }
    if (input_file.empty()) {
        printInvalidArgs(argv[0]);
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
    free_cache();

    return 0;
}
