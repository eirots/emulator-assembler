        movi    r9, #10000
        movi    r8, #1
loop    ildb    r7, r9
        sub     r9, r9, r8
        bnz     r9, loop
        trp     #0
