prompt1 .STR "\n\nWelcome to the Prime Number Generator\n\n"
prompt2 .STR "This program searches for and displays the first 20 prime numbers greater than\nor equal to a user provided lower bound.\n\n";
prompt3 .STR "Please enter a lower bound:"
prompt4 .STR "The first 20 prime numbers greater than or equal to "
prompt5 .STR " are: \n"



ENTRY           JMP MAIN      ;registers R0-R7 are volatile, R8-R15 are non-volatile and must be pushed before using in a function. 

;function void beginprint() returns integer of user input. 
BEGINPRT        MOVI R3, prompt1    ;user-provided int is stored in R0 
                TRP #5
                MOVI R3, prompt2
                TRP #5
                MOVI R3, prompt3
                TRP #5 
                TRP #2  ;get int from user 
                MOV R0, R3
                RET                 ;returns R0

;function mod(r0, r1), returns mod value
MOD             PSHR R8     ;inputs go in R0 and R1, result is returned in R0. 
                SDIV R2, R0, R1
                MUL  R3, R2, R1 
                SUB  R0, R0, R3
                POPR R8
                RET         ;returns R0


;function bool is_prime(n), returns true if prime, false if not 
IS_PRIME        PSHR R8    ;R0 is input 
                PSHR R9
                PSHR R10
                PSHR R11

                MOV  R8, R0 ;keep original value in r8 
                MOVI R9, #2

PRIMELOOP       MUL  R10, R9, R9 ;r10 = i * i
                CMP  R11, R8, R10  ;if i * i <= n
                BLT  R11, PRIMEDONE

                MOV  R0, R8  ;argument 0 = n 
                MOV  R1, R9  ; argument 1 = i 
                CALL MOD    ;r0 should be n % i 
                MOV  R11, R0 
                CMPI R11, R11, #0
                BRZ  R11, NOTPRIME
            
                ADDI R9, R9, #1 
                JMP  PRIMELOOP

PRIMEDONE       CMPI R11, R8, #2
                BLT  R11, NOTPRIME

                MOVI R0, #1 ;return true 
                JMP  PRIMEEXIT

NOTPRIME        MOVI R0, #0 ;return false
PRIMEEXIT       POPR R11
                POPR R10
                POPR R9
                POPR R8
                RET


;function COUNTPRIMESUP(int n)
COUNTPRIMESUP   PSHR R8   ;need input of n, number to start counting upwards for. 
                PSHR R9
                PSHR R10 
                PSHR R11
                PSHR R12
                PSHR R13
                PSHR R14

                MOV  R8, R0    ;lower bound (n)
                MOVI R9, #0       ;counter of our primes
                MOV  R10, R8       ;current i value (like in for loop) 

FINDNEXTPRIME   MOV  R0, R10      ;setting up for is_prime
                CALL IS_PRIME
                CMPI R11, R0, #0   ; R11 = compare(R0, 0)   
                BRZ  R11, NOT_PRIME  ;anything that moves past here IS prime
              
                MULI R14, R13, #4     ;byte offset, R13 * 4
                ADD  R14, R12, R14     ;address to put the prime 
                ISTR R10, R14         ;store found prime 
                ADDI R13, R13, #1     ;add to the byte ofset 
                ADDI R10, R10, #1

                CMPI R9, R13, #20
                BLT  R9, FINDNEXTPRIME
                JMP  COUNTPRIMESDONE

NOT_PRIME       ADDI R10, R10, #1   ;basically for loop i++
                JMP  FINDNEXTPRIME

COUNTPRIMESDONE POPR R14    ;restore everything. We don't really *have* to, but I think it's good practice 
                POPR R13
                POPR R12
                POPR R11
                POPR R10
                POPR R9
                POPR R8
                RET

MAIN            CALL BEGINPRT   ;user int is inside R0
                MOV R15, R0
                ALCI R12, #80   ;r12 is the base pointer into the heap, basically have remade pointer arrays 
                MOVI R13, #0    ;insertion count  
                CALL COUNTPRIMESUP

                MOVI R13, #0

                MOVI R3, prompt4 
                TRP #5
                MOV R3, R15
                TRP #1
                MOVI R3, prompt5
                TRP #5

PRNT_LOOP       MULI R14, R13, #4
                ADD R14, R12, R14 
                ILDR R0, R14
                MOV R3, R0 
                TRP #1
        
                MOVI R3, '\n'
                TRP #3 

                ADDI R13, R13, #1
                CMPI R9, R13, #20 
                BLT R9, PRNT_LOOP
                ;TRP #98
                TRP #0