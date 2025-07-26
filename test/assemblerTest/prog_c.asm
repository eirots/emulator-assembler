v   .int    #123456789
a   .int    #1103515245 
c   .int    #12345 
m   .int    #2147483647
s   .int    #65536

        jmp MAIN
MAIN    ldr r15, a
        ldr r14, c
        ldr r13, m
        ldr r11, v
        ldr r10, s
        movi r9, #10000
        movi r8, #1

R       mov r0, r15
        mul r0, r0, r11
        add r0, r0, r14
        mov r1, r0
        div r1, r1, r13
        mov r2, r1
        mul r2, r2, r13
        sub r0, r0, r2
        mov r11, r0
        mov r4, r11
        mul r4, r4, r10
        div r4, r4, r10
        ildb r5, r4
        sub r9, r9, r8
        bnz r9, R
END     trp #0