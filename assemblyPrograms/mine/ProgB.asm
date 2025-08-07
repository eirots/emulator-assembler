prompt1 .STR "Please enter an integer dividend: "
prompt2 .STR "Please enter an integer divisor: "
prompt3 .STR " divided by "
prompt4 .STR " results in a remainder of: "

dividend .INT #0 
divisor  .INT #0


        JMP  MAIN
MOD     PSHR R8
        SDIV R2, R0, R1
        MUL  R3, R2, R1 
        SUB  R0, R0, R3
        POPR R8
        RET  

MAIN    MOVI R3, prompt1    ;read dividend
        TRP  #5
        TRP  #2  ;get int from user 
        STR  R3, dividend
        MOVI R3, prompt2    ;read divisor
        TRP  #5
        TRP  #2 ;get second int from user

        STR  R3, divisor
        LDR  R0, dividend       ;unneeded, but I was using this program to practice INT directives. 
        LDR  R1, divisor        ;unneeded, but I was using this program to practice INT directives. 

        CALL MOD
        MOV  R4, R0

        LDR  R3, dividend
        TRP  #1
        MOVI R3, prompt3
        TRP  #5
        LDR  R3, divisor
        TRP  #1
        MOVI R3, prompt4
        TRP  #5
        MOV  R3, R0
        TRP  #1
        MOVI R3, '\n'
        TRP  #3
        ;TRP  #98;DEBUG LINE
        TRP  #0