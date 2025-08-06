prompt .STR "Enter a number to test BLT:"
trueMsg  .STR "<2"
falseMsg .STR ">=2"
newline  .BYT '\n'

        JMP MAIN

MAIN    MOVI  R3, prompt
        TRP   #5              ; print prompt
        TRP   #2              ; read int → R3

        MOV   R0, R3          ; move into R0 for compare
        CMPI  R8, R0, #2      ; R8 = (signed) R0−2
        BLT   R8, LESS        ; if R8<0 jump

    ; ≥2 path
        MOVI  R3, falseMsg
        TRP   #5
        TRP   #0              ; exit

LESS MOVI  R3, trueMsg
        TRP   #5
        TRP   #0              ; exit
