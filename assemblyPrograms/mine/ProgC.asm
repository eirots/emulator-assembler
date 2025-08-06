prompt1   .STR "Please enter the Fibonacci term you would like computed: " ;make assemble FILE="assemblyPrograms/mine/ProgC.asm"; make; build/bin/emu4380 build/ProgC.bin
prompt2   .STR "Term " 
prompt3   .STR " in the Fibonacci sequence is: " 

        JMP     MAIN

FIB     CMPI    R9, R0, #1 ;base case
        BLT     R9, FIBEND
        BRZ     R9, FIBEND

        PSHR    R0
        SUBI    R0, R0, #1
        CALL    FIB

        POPR    R1
        PSHR    R0
        SUBI    R0, R1, #2
        CALL    FIB

        POPR    R1
        ADD     R0, R0, R1

FIBEND  RET
        
MAIN    MOVI    R3, prompt1
        TRP     #5            
        TRP     #2            
        MOV     R0, R3        ;argument for fib put in R0
        MOV     R6, R0          ;save original in R6
        ;TRP     #98
        CALL    FIB     
        MOVI    R3, prompt2
        TRP     #5            ; "Term "
        MOV     R3, R6
        TRP     #1            ; print n
        MOVI    R3, prompt3
        TRP     #5            ; " in the Fibonacci sequence is: "
        MOV     R3, R0
        TRP     #1            

        MOVI    R3, '\n'   
        TRP     #3            

        TRP     #0            ; exit


        