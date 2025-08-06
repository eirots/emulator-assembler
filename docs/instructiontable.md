## Instructions

|Value | Operator | Operand 1 | Operand 2 | Operand 3 | Immediate Value | Description| 
|------|-----------|----------|-----------|-----------|-----------------|------------|
|	1	|	JMP	|	DC	|	DC	|	DC	|	Address	|	Jump to address	|
|	2	|	JMR	|	RS	|	DC	|	DC	|	Address	|	Update PC to value in RS.	|
|	3	|	BNZ	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS != 0.	|
|	4	|	BGT	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS > 0.	|
|	5	|	BLT	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS < 0.	|
|	6	|	BRZ	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS == 0.	|
|	7	|	MOV	|	RD	|	RS	|	DC	|	DC	|	Move contents of RS to RD	|
|	8	|	MOVI	|	RD	|	DC	|	DC	|	Imm***	|	Move imm value into RD	|
|	9	|	LDA	|	RD	|	DC	|	DC	|	Address**	|	Load address into RD	|
|	10	|	STR	|	RS	|	DC	|	DC	|	Address**	|	Store integer in RS at Address	|
|	11	|	LDR	|	RD	|	DC	|	DC	|	Address**	|	Load integer at Address to RD	|
|	12	|	STB	|	RS	|	DC	|	DC	|	Address**	|	Store least significant byte in RS at address	|
|	13	|	LDB	|	RD	|	DC	|	DC	|	Address**	|	Load byte at Address to RD	|
|	14	|	ISTR	|	RS	|	RG	|	DC	|	DC	|	Store integer in RS at address in RG.	|
|	15	|	ILDR	|	RD	|	RG	|	DC	|	DC	|	Load integer at address in RG into RD.	|
|	16	|	ISTB	|	RS	|	RG	|	DC	|	DC	|	Store byte in RS at address in RG.	|
|	17	|	ILDB	|	RD	|	RG	|	DC	|	DC	|	Load byte at address in RG into RD.	|
|	18	|	ADD	|	RD	|	RS1	|	RS2	|	DC	|	Add RS1 to RS2, store result in RD.	|
|	19	|	ADDI	|	RD	|	RS1	|	DC	|	Imm**	|	Add Imm to RS1, store result in RD.	|
|	20	|	SUB	|	RD	|	RS1	|	RS2	|	DC	|	Subtract RS2 from RS1, store result in RD.	|
|	21	|	SUBI	|	RD	|	RS1	|	DC	|	Imm**	|	Subtract Imm from RS1, store result in RD.	|
|	22	|	MUL	|	RD	|	RS1	|	RS2	|	DC	|	Multiply RS1 by RS2, store result in RD. 	|
|	23	|	MULI	|	RD	|	RS1	|	DC	|	Imm**	|	Multiply RS1 by IMM, store the result in RD.	|
|	24	|	DIV	|	RD	|	RS1	|	RS2	|	DC	|	Perform unsigned integer division RS1 / RS2. Store quotient in RD.	|
|	25	|	SDIV	|	RD	|	RS1	|	RS2	|	DC	|	Store result of signed division RS1 / RS2 in RD. @detailsDivision by zero shall result in an emulator error.	|
|	26	|	DIVI	|	RD	|	RS1	|	DC	|	Imm**	|	Divide RS1 by IMM (signed), store the result in RD.	|
|	27	|	AND	|	RD	|	RS1	|	RS2	|	DC	|	Performs a LOGICAL AND (&&) between RS1 and RS2, stores the result in RD. 1 = True, 0 = False 	|
|	28	|	OR	|	RD	|	RS1	|	RS2	|	DC	|	Performs a LOGICAL OR (||) between RS1 and RS2, stores the result in RD. 1 = True, 0 = False	|
|	29	|	CMP	|	RD	|	RS1	|	RS2	|	DC	|	Performs a signed comparison between RS1 and RS2, and stores the result in RD. Set RD = 0 if RS1 == RS2 OR set RD = 1 if RS1 >RS2 OR set RD = -1 if RS1 < RS2 	|
|	30	|	CMPI	|	RD	|	RS1	|	DC	|	IMM	|	Performs a signed comparison between RS1 and IMM and stores the result in RD. Set RD = 0 if RS1 == IMM OR set RD = 1 if RS1 >IMM OR set RD = -1 if RS1 < IMM 	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#0	|	Executes the STOP/Exit routine	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#1	|	Write int in R3 to stdout 	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#2	|	Read an integer into R3 from stdin	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#3	|	Write char in R3 to stdout	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#4	|	Read a char into R3 from stdin	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#5	|	Writes the full null-terminated pascal-style string whose starting address is in R3 to stdout	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#6	|	Read a newline terminated string from stdin and stores it in memory as a null-terminated pascal-style string whose starting address is in R3.	|
|	31	|	TRP	|	DC	|	DC	|	DC	|	#98	|	Print all register contents to stdout	|
|	32	|	ALCI	|	RD	|	DC	|	DC	|	Imm	|	Allocate imm bytes of space on the heap, and increment HP accordingly. Immediate value is a 4-byte unsigned ineger. Initial heap pointer is stored in RD	|
|	33	|	ALLC	|	RD	|	DC	|	DC	|	Address	|	Allocate a number of bytes on the heap according to the value of the 4-byte unsigned integer stored at address. Initial heap pointer is stored in RD	|
|	34	|	IALLC	|	RD	|	RS1	|	DC	|	DC	|	Indirectly allocate a number of bytes on the heap according to the value of the 4-byte unsigned integer at the memory address stored in RS1. Store initial heap pointer in RD	|
|	35	|	PSHR	|	RS	|	DC	|	DC	|	DC	|	Set SP = SP - 4, place the word in RS onto the stack.	|
|	36	|	PSHB	|	RS	|	DC	|	DC	|	DC	|	Set SP = SP - 1, place the least significant byte in RS onto the stack. 	|
|	37	|	POPR	|	RD	|	DC	|	DC	|	DC	|	Place the word on top of the stack into RD, update SP = SP + 4	|
|	38	|	POPB	|	RD	|	DC	|	DC	|	DC	|	Place the byte on top of the stack into RD, update SP = SP + 1	|
|	39	|	CALL	|	DC	|	DC	|	DC	|	Address	|	Push PC onto stack, update PC to Address.	|
|	40	|	RET	|	DC	|	DC	|	DC	|	DC	|	pop stack into PC	|

## Directives
|Directive| Operand | Examples | Behavior | 
|---------|---------|----------|----------|
|	.INT	|	Optional signed decimal value in the range of -2147483648 to 2147483648, inclusive 	|	.INT #45 .INT #-12 .INT #2147483647	|	Allocates memory in place for a 4-byte integer and initializes it with the optional operand value. If no operand is provided the value is initialized to 0. |
|	.BYT	|	Optional unsigned decimal value in the range 0 to 255 inclusive, or apostrophe deliniated ascii character 	|	.BYT #45 .BYT 'a' .BYT '\n'	|	Allocates 1 byte of memory in place and initializes it with the optional decimal value or ascii code. If no operand is provided the value is initialized to 0. 	|
|	.DTS	|	Required unsigned decimal value. This value is the number of bytes to be allocated	|	.BTS #25 <br> .BTS #5 <br>  a_label .BTS #20	|	Allocates the specificed number of bytes in place and initializes them all to 0. If an optional label is present the label shall be associated with the address of the first byte allocated	|
|	.STR	|	Required double quote delimited string <br> OR. <br> A numeric literal. <br> 255 shall be the maximum length of the string and the maximum value of the numeric literal. 	|	.STR "Example!" <br>  name .STR "Fred" <br> .STR #40 <br> .STR #255	|	Allocates a number of bytes equal to the length of the string + 2. The first byte is initialized to the length of the string, and the last byte to 0 (null character). The remaining bytes (in the middle) are initialized to the ascii values for the characters in the string. Note: escape sequences such as \n must be properly handled. <br> OR <br>Allocates a number of bytes equal to the numeric literal + 2. The first byte is initialized to the value of the numeric literal and the remaining bytes are initialized to zeros. In both cases if an optional label is present the label shall be associated with the address of the first byte allocated.	|

