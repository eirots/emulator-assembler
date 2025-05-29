#include <cstdint>
#include <fstream>
#include <iostream>

using namespace std;

/** @brief Reads numbers in a text file, and outputs a matching byte to a binary file.
 *  @details Can be used to create bytecode binaries to test emulator/assembler functionality.
 *  @return True if able to output, false and console out if error is present.
 */
int main(int argC, char** argV) {
    if (argC < 3) {
        cout << "Usage: " << argV[0] << " <input_text_file> <output_filename> \n";
        return false;
    }

    ifstream input(argV[1]);
    if (!input.is_open()) {
        return false;
    }

    ofstream output(argV[2], ofstream::binary);
    if (!output.is_open()) {
        return false;
    }

    unsigned int value = 0;
    while (input >> value) {
        output.write((char*)&value, 1);
    }

    input.close();
    output.close();
    return true;
}
