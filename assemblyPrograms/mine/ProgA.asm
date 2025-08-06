prompt1         .STR "Please enter the Fibonacci term you would like computed: " 
prompt2         .STR    "Term "
prompt3         .STR    " in the Fibonacci sequence is: "
zero            .INT    #1
one             .INT    #1
two             .INT    #2

n1              LDR R0, one
n2              LDR R1, one
stopvalue       LDR R2, zero ;end of declarations and loading stuff
                movi R3, prompt1 ;output prompt 1
                TRP #5 
                TRP #2      ;get integer from user
                MOV R7, R3
ONE             SUBI R5, R3, #1  
                BNZ R5, TWO
                MOV R4, R0
                JMP PRINT
TWO             SUBI R5, R3, #2
                BNZ R5, ELSE
                MOV R4, R1
                JMP PRINT
ELSE            MOVI R5, #2
LOOP1           SUB R6, R3, R5
                BRZ R6, DONE1
                BGT R6, CONT
                JMP DONE1
CONT            MOV R6, R1
                ADD R1, R0, R1 
                MOV R0, R6
                ADDI R5, R5, #1
                JMP LOOP1
DONE1           MOV R4, R1
PRINT           MOVI R3, prompt2
                TRP #5
                MOV R3, R7
                TRP #1
                MOVI R3, prompt3
                TRP #5
                MOV R3, R4
                TRP #1
                MOVI r3, '\n'
                TRP #3
                TRP #0