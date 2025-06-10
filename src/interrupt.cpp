#include "interrupt.h"

bool TRP() {
    // IMM 0    -> EXECUTE STOP / EXIT ROUTINE
    // IMM 1    -> WRITE INT IN R3 TO STDOUT (CONSOLE)
    //      print the above without any leading or trailing whitespace
    // IMM 2    -> READ AN INTEGER INTO R3 FROM STDIN
    // IMM 3    -> WRITE CHAR IN R3 TO STDOUT
    //      print the above without any leading or trailing whitespace
    // IMM 4    -> READ A CHAR INTO R3 FROM STDIN
    // IMM 98   -> PRINT ALL REGISTER CONTENTS TO STDOUT
    //      format above as follows:
    //          -one register name and value per line
    //          -The register name shall be in all caps, followed by a tab character, followed by the register value printed as unsigned base 10 integer
    //          -example:
    //              -R1 0
    //              -R2 128
    //              -R3 34
    //              -HP 10045
}
