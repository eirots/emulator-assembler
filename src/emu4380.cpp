#include "emu4380.h"
std::uint32_t* reg_file = nullptr;
unsigned char* prog_mem = nullptr;
std::uint32_t cntrl_regs[5] = {};
std::uint32_t data_regs[2] = {};
std::uint32_t mem_size = 0;

bool fetch() {
    // TODO:
    // read 8 bytes starting at the address at PC
    // convert those bytes into a 64-bit word
    // split the word into the 5 fields so that decode can validate
    // increment PC by 8 (8 bytes, size of 1 instruction )
    // return TRUE on success, false if PC address is OOB.
    return false;
}

bool decode() {
    return false;
}

bool execute() {
    // TODO:
    return false;
}

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
        prog_mem = static_cast<unsigned char*>(std::malloc(size));
        if (!prog_mem) return false;

    } else {  // resizing
        void* new_block_of_memory = std::realloc(prog_mem, size);
        if (!new_block_of_memory) return false;  // OUT OF MEMORY

        prog_mem = static_cast<unsigned char*>(new_block_of_memory);
    }
    mem_size = size;

    if (!init_registers()) return false;  // couldn't initialize registers

    reg_file[PC] = 0;
    reg_file[SL] = 0;
    reg_file[SB] = mem_size;
    reg_file[SP] = mem_size;
    reg_file[FP] = reg_file[SP];
    reg_file[HP] = reg_file[SL];

    return true;
}

bool load_binary(const char* filename) {
    ifstream in(filename, std::ios::binary | std::ios::ate);
    if (!in) {
        cerr << "Cannot open file: " << filename << endl;
        return false;
    }

    streamsize file_size = in.tellg();
    if (file_size > mem_size) {
        cerr << "Program too large, file is " << file_size << " bytes, reserved RAM is " << mem_size << " bytes" << endl;
        return false;
    }

    in.seekg(0, std::ios::beg);

    if (!in.read(reinterpret_cast<char*>(prog_mem), file_size)) {
        cerr << "Error while reading " << filename << endl;
        return false;
    }
    return true;
}

int runEmulator(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " INPUT_BINARY_FILE [RESERVED_MEMORY]\n";
        cout << "INPUT_BINARY_FILE: \t\tPath to file containing 4380 byte code\n";
        cout << "RESERVED_MEMORY (optional):\tPositive integer, default 131,072 bytes, max 4,294,967,295\n";
        return 1;
    }
    mem_size;

    if (argc < 3) {
        // default minimum size
        mem_size = 131'072;

        // cout << "memsize defaulted to: " << mem_size << endl;
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

    if (!init_mem(mem_size)) return 1;

    if (!load_binary(argv[1])) return 1;
    // TODO: add tests for this section. make sure that
    //       memory looks how we expect it to
    //       instructions are in the correct order

    cout << "path given was: " << argv[1] << endl;

    return 0;
}