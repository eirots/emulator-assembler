Everything	.INT	#42 		;constant int
H		.BYT	'H' 		;character
I		.BYT	'i' 		;character
	jmp 	MAIN
MAIN ldr 	r0, Everything 		;42 into r0
	movi 	r1, #0 			;0 into r1
	sub 	r2, r0, r1 		;42 - 0
	ldr 	r3, Everything 		;42 into r3
	trp 	#1 			;42 to stdout
	movi 	r3, #32 		;space into r3
	trp 	#3 			;space to stdout
	movi 	r3, #45 		;'-' into r3
	trp 	#3 			;'-' to stdout
	movi 	r3, #32 		;space into r3
	trp 	#3 			;space to stdout
	mov 	r3, r1 			;0 into r3
	trp 	#1 			;0 to stdout
	movi 	r3, ' ' 		;space into r3
	trp 	#3 			;space to stdout
	movi 	r3, '=' 		;'=' into r3
	trp 	#3 			;'=' to stdout
	movi 	r3, #32 		;space into r3
	trp 	#3 			;space to stdout
	mov 	r3, r2 			;result into r3
	trp 	#1 			;result to stdout
	movi 	r3, #10 		;newline into r3
	trp 	#3 			;newline to stdout
	ldb 	r3, H 			;load 'H' into r3
	trp 	#3 			;print 'H' to stdout
	ldb 	r3, I 			;load 'i' into r3
	trp 	#3 			;print 'i' to stdout
	movi 	r3, #33 		;move '!' into r3
	trp 	#3 			;print '!' to stdout
	movi 	r3, #10 		;newline into r3
	trp 	#3 			;newline to stdout
	trp 	#0 			;end program
