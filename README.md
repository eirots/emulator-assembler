# CS4380 Project 4 - Procedures and Beyond


## Summary

##### Instruction Size
The 4380 is a RISC-based processor. Instructions are fixed at 8 bytes (64 bits), matching the following:
| Operation | Operand 1 | Operand 2 | Operand 3 | Immediate Value |
|-----------|-----------|-----------|-----------|-----------------|
| 1 byte    | 1 byte register | 1 byte register | 1 byte register | 4 byte immediate value |

Not all instructions require all registers, but in 4380 bytecode, all fields are required to be present for an instruction to be valid. 

##### Registers (22)
The 4380 has 22 registers matching the following: 
| Name     | Encoding | Description (All registers are 32 bits in size) | 
|------|----------|-------------------------------------------------|
|R0-R15 | 0-15 | General purpose registers, initialized to 0 at startup.  | 
| PC | 16 | Program counter, initalized to the address of first instruction in memory. | 
| SL | 17 | Stack lower limit, initialized to the lowest legal address avaialable to the stack. | 
| SB | 18 | Stack bottom, initialized to the highest available address in memory. | 
| SP | 19 | Stack pointer, initialized to the latest allocated byte on the stack. Grows downward. | 
| FP | 20 | Frame pointer, points to the first word beneath the return address. |
| HP | 21 | Heap pointer, initialized to SL, grows upward. |

It is recommended to use registers R0-R7 as caller registers, and R8-R16 as callee registers, but this is not required.

##### Programmable Memory 
32 bit address space, byte addressable, little-endian, user specifiable. Defaults to 2^17 (131,072) bytes of addressable memory. 

## How to run: 

`make` 
`make build` 
-  Builds the Emulator inside of `build/bin`. Creates `build` folder if it doesn't exist. 
-  Moves the Assembler and a test file into the `build` directory.

`make test`
- Runs `make build`.
- Compiles and runs emulator tests. 

`make assemble FILE="[EXAMPLE_TEST_FILE_NAME].asm"`
- Copies assembler file into `build` folder and executes assembler on `EXAMPLE_TEST_FILE_NAME.asm`.

`make run FILE="[EXAMPLE_TEST_FILE_NAME].asm"` 
- Assembles `EXAMPLE_TEST_FILE_NAME.asm` into a bin file and places it in build folder. 
- Runs emulator on `EXAMPLE_TEST_FILE_NAME.bin` 

`make docs`
- Generates autodocumentation.
- NOTE: This requires `doxygen` to be installed 



`make asm_test`
- Runs assembler tests. 
- NOTE: This command requires `pytest` to be installed. 

`make clean`
- Removes build folder and autodocs.


## Supported Instructions 
|	Operator	|	Operand_1	|	Operand_2	|	Operand_3	|	Immediate Value	|	Description |
|---------------|---------------|---------------|---------------|-------------------|---------------|
|	JMP	|	DC	|	DC	|	DC	|	Address	|	Jump to address	|
|	JMR	|	RS	|	DC	|	DC	|	Address	|	Update PC to value in RS.	|
|	BNZ	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS != 0.	|
|	BGT	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS > 0.	|
|	BLT	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS < 0.	|
|	BRZ	|	RS	|	DC	|	DC	|	Address	|	Update PC to Address if RS == 0.	|
|	MOV	|	RD	|	RS	|	DC	|	DC	|	Move contents of RS to RD	|
|	MOVI	|	RD	|	DC	|	DC	|	Imm	|	Move imm value into RD	|
|	LDA	|	RD	|	DC	|	DC	|	Address	|	Load address into RD	|
|	STR	|	RS	|	DC	|	DC	|	Address	|	Store integer in RS at Address	|
|	LDR	|	RD	|	DC	|	DC	|	Address	|	Load integer at Address to RD	|
|	STB	|	RS	|	DC	|	DC	|	Address	|	Store least significant byte in RS at address	|
|	LDB	|	RD	|	DC	|	DC	|	Address	|	Load byte at Address to RD	|
|	ISTR	|	RS	|	RG	|	DC	|	DC	|	Store integer in RS at address in RG.	|
|	ILDR	|	RD	|	RG	|	DC	|	DC	|	Load integer at address in RG into RD.	|
|	ISTB	|	RS	|	RG	|	DC	|	DC	|	Store byte in RS at address in RG.	|
|	ILDB	|	RD	|	RG	|	DC	|	DC	|	Load byte at address in RG into RD.	|
|	ADD	|	RD	|	RS1	|	RS2	|	DC	|	Add RS1 to RS2, store result in RD.	|
|	ADDI	|	RD	|	RS1	|	DC	|	Imm	|	Add Imm to RS1, store result in RD.	|
|	SUB	|	RD	|	RS1	|	RS2	|	DC	|	Subtract RS2 from RS1, store result in RD.	|
|	SUBI	|	RD	|	RS1	|	DC	|	Imm	|	Subtract Imm from RS1, store result in RD.	|
|	MUL	|	RD	|	RS1	|	RS2	|	DC	|	Multiply RS1 by RS2, store result in RD. 	|
|	MULI	|	RD	|	RS1	|	DC	|	Imm	|	Multiply RS1 by IMM, store the result in RD.	|
|	DIV	|	RD	|	RS1	|	RS2	|	DC	|	Perform unsigned integer division RS1 / RS2. Store quotient in RD.	|
|	SDIV	|	RD	|	RS1	|	RS2	|	DC	|	Store result of signed division RS1 / RS2 in RD. @detailsDivision by zero shall result in an emulator error.	|
|	DIVI	|	RD	|	RS1	|	DC	|	Imm	|	Divide RS1 by IMM (signed), store the result in RD.	|
|	AND	|	RD	|	RS1	|	RS2	|	DC	|	Performs a LOGICAL AND (&&) between RS1 and RS2, stores the result in RD. 1 = True, 0 = False 	|
|	OR	|	RD	|	RS1	|	RS2	|	DC	|	Performs a LOGICAL OR (||) between RS1 and RS2, stores the result in RD. 1 = True, 0 = False	|
|	CMP	|	RD	|	RS1	|	RS2	|	DC	|	Performs a signed comparison between RS1 and RS2, and stores the result in RD. Set RD = 0 if RS1 == RS2 OR set RD = 1 if RS1 >RS2 OR set RD = -1 if RS1 < RS2 	|
|	CMPI	|	RD	|	RS1	|	DC	|	IMM	|	Performs a signed comparison between RS1 and IMM and stores the result in RD. Set RD = 0 if RS1 == IMM OR set RD = 1 if RS1 >IMM OR set RD = -1 if RS1 < IMM 	|
|	TRP	|	DC	|	DC	|	DC	|	#0	|	Executes the STOP/Exit routine	|
|	TRP	|	DC	|	DC	|	DC	|	#1	|	Write int in R3 to stdout 	|
|	TRP	|	DC	|	DC	|	DC	|	#2	|	Read an integer into R3 from stdin	|
|	TRP	|	DC	|	DC	|	DC	|	#3	|	Write char in R3 to stdout	|
|	TRP	|	DC	|	DC	|	DC	|	#4	|	Read a char into R3 from stdin	|
|	TRP	|	DC	|	DC	|	DC	|	#5	|	Writes the full null-terminated pascal-style string whose starting address is in R3 to stdout	|
|	TRP	|	DC	|	DC	|	DC	|	#6	|	Read a newline terminated string from stdin and stores it in memory as a null-terminated pascal-style string whose starting address is in R3.	|
|	TRP	|	DC	|	DC	|	DC	|	#98	|	Print all register contents to stdout	|
|	ALCI	|	RD	|	DC	|	DC	|	Imm	|	Allocate imm bytes of space on the heap, and increment HP accordingly. Immediate value is a 4-byte unsigned ineger. Initial heap pointer is stored in RD	|
|	ALLC	|	RD	|	DC	|	DC	|	Address	|	Allocate a number of bytes on the heap according to the value of the 4-byte unsigned integer stored at address. Initial heap pointer is stored in RD	|
|	IALLC	|	RD	|	RS1	|	DC	|	DC	|	Indirectly allocate a number of bytes on the heap according to the value of the 4-byte unsigned integer at the memory address stored in RS1. Store initial heap pointer in RD	|
|	PSHR	|	RS	|	DC	|	DC	|	DC	|	Set SP = SP - 4, place the word in RS onto the stack.	|
|	PSHB	|	RS	|	DC	|	DC	|	DC	|	Set SP = SP - 1, place the least significant byte in RS onto the stack. 	|
|	POPR	|	RD	|	DC	|	DC	|	DC	|	Place the word on top of the stack into RD, update SP = SP + 4	|
|	POPB	|	RD	|	DC	|	DC	|	DC	|	Place the byte on top of the stack into RD, update SP = SP + 1	|
|	CALL	|	DC	|	DC	|	DC	|	Address	|	Push PC onto stack, update PC to Address.	|
|	RET	|	DC	|	DC	|	DC	|	DC	|	pop stack into PC	|

## Supported Directives 
|Directive| Operand | Examples | Behavior | 
|---------|---------|----------|----------|
|	.INT	|	Optional signed decimal value in the range of -2147483648 to 2147483648, inclusive 	|	.INT #45 .INT #-12 .INT #2147483647	|	Allocates memory in place for a 4-byte integer and initializes it with the optional operand value. If no operand is provided the value is initialized to 0. |
|	.BYT	|	Optional unsigned decimal value in the range 0 to 255 inclusive, or apostrophe deliniated ascii character 	|	.BYT #45 .BYT 'a' .BYT '\n'	|	Allocates 1 byte of memory in place and initializes it with the optional decimal value or ascii code. If no operand is provided the value is initialized to 0. 	|
|	.DTS	|	Required unsigned decimal value. This value is the number of bytes to be allocated	|	.BTS #25 <br> .BTS #5 <br>  a_label .BTS #20	|	Allocates the specificed number of bytes in place and initializes them all to 0. If an optional label is present the label shall be associated with the address of the first byte allocated	|
|	.STR	|	Required double quote delimited string <br> OR. <br> A numeric literal. <br> 255 shall be the maximum length of the string and the maximum value of the numeric literal. 	|	.STR "Example!" <br>  name .STR "Fred" <br> .STR #40 <br> .STR #255	|	Allocates a number of bytes equal to the length of the string + 2. The first byte is initialized to the length of the string, and the last byte to 0 (null character). The remaining bytes (in the middle) are initialized to the ascii values for the characters in the string. Note: escape sequences such as \n must be properly handled. <br> OR <br>Allocates a number of bytes equal to the numeric literal + 2. The first byte is initialized to the value of the numeric literal and the remaining bytes are initialized to zeros. In both cases if an optional label is present the label shall be associated with the address of the first byte allocated.	|

_______________

## Worklog (Project 3)
|date | hours worked | what did I work on? | 
|-----|--------------|---------------------|
|7/11| 2h | Paper design, worked on designing `Line` structure and fully understanding how caches work.  |
|7/12| 1h | Further paper design. Understood intuitively how caches are suppoesd to work. |
|7/13| 0h | Weekend day. |
|7/14| 2h | Started on implementing new instructions in assembler and emulator. Actually had a lot of fun doing this part.  |
|7/15| 5h | Finished new instruction implementations, tested assembler.  |
|7/16| 5h | Exploratory look refactoring emu4380 into a class to try and make cache I had designed easier to implement. Spoiler: was not worth it, basically a waste of devtime. Spent another 2 hours of this time undoing everything I had done and redesigning how I wanted the cache to interact inside the emulator. |
|7/17| 4h | Created tests for new instruction behavior, found a bug that I was struggling to sort out. Memory was being mangled by the cache, issue was with new instructions. Started implementing my design of Lines and Cache |
|7/18| 8h | Problems with lines and cache, created tons of new globals that were handled by the cache. Struggled with accurate memory cycles, misses weren't being counted. Known test results were not matching, tabled for today.  |
|7/19| 5h | Cache is extremely broken, my efforts to fix it have turned it into a spaghetti code mess. Decided it is time to trash what I have, and try to salvage what I can from this horrible, horrible mess.  That will be done on Monday.|
|7/20| 0h | Weekend day. |
|7/21| 5h | Refactor of cache started, have a solid idea of where I want to go from here. |
|7/22| 3h | Continuing with cache refactor, starting to come together. Tests were broken because I was calling global variables, fixed some of those today. |
|7/23| 4h | Finished cache, added autodocs, changed assembler call in makefile to make testing easier. |
|7/24| 7h | Big bug in cache that took way too long to figure out. Was right shifting somewhere instead of left shifiting, tests were showing mangled data but couldn't figure out why until I went through all code line-by-line. Fixed mangled data, ensured it was correct with new cache dump functions. |
|7/25| 10h | Finished testing, found additional bugs through testing. Repaired those, created writeup for progs a - f. |


_______________
## Worklog (Project 2)
| date | hours worked | what did I work on? | 
|-----|--------------|---------------------|
| 6/19 | 2 h | "state machine" for  instructions |
| 6/20 | 1 h | continuing "state machine" for instructions |
| 6/25 | 2 h | More pen and paper design, created branch and added file structure  |
| 6/26 | 3 h | EVEN MORE pen and paper design, finished state machine for instructions. Started working on assembler, laid out states I will need. Finished (what I thought at the time was half) of the data secton. | 
| 6/27 | 8 h | Created classes, caused a refactor. Got lost in the sauce while refactoring, ended up making a whole file skeleton. Added argument parser, had major issues with that. |
| 6/28 | 2 h | Worked on major bug with data directives lacking labels. Moved to code section | 
| 6/29 | weekend day | weekend day | 
| 6/30 | 3 h | ran into yet another refactor. starting to realize that I should plan better ahead of time. |
| 7/01 |  4 h | tightened argument-parser edge cases |
| 7/02 | 3 h | Added Makefile targets; integrated `asm_copy` and `asm_run` rule for easier testing; improved build output |
| 7/03 | 2 h | Debugged symbol-table collision; improved error-report formatting. More progress on code section|
| 7/04 | 1 h | Light coding (holiday): comments & TODO cleanup |
| 7/05 | 3 h | fixed off-by-one bug that was killing me, added some more helpers to stream object |
| 7/06 | weekend | weekend |
| 7/08 | 6 h | project stopped working because of Apple update. Spent the entire time fixing my enviromnet, because Apple decided to update things on their own. Really huge bummer, large amount of time wasted. |
| 7/09 | 6 h | More solid progress on code section |
| 7/10 | 6 h | Still working on code section |
| 7/11 | 3 h| serious issues with my previous design for handling instructions in code section. Devised a new way using handlers |
| 7/12 | 4 h| More issues with how I was handling literals and "fixups." Did some more reading on how to handle them, then made adjustments as needed |
| 7/13 | weekend | weekend |
| 7/14 | 5 h | Finished testing, cleaned up some bugs I missed, verified that functions work as they should.  |

_______________

## Resubmission 1 Notes: 
#### Root Causes:
 1. Misunderstanding of the spec, and I did not account for the 4 byte address of first instruction when running through a binary. This is a big oversight, and threw many things off. 
 2. `decode()` was performing bounds checking on immeditate values, when that step actually belongs in `execute()`. This would either reject valid instructions in the best case, or in the worst, continue on with garbage values. I believe this then led to the issues seen with the arithmatic tests. 
#### Fixes implemented:
1. **`load_binary()`** 
    * Copy the entire file at address 0. 
    * Read first 4 bytes as the entry pointer, and set `PC`
2. **`decode()`**
    * Removed improper memory-range checks for move functions that use immediates, `STR`, `LDR`, `STB`, and `LDB` now only validate register numbers. 
3. **`execute()`**
    * Added a new helper function `addr_in_range(addr, bytes)`, and put checks inside `execute()`, defaulting to 1 for byte operations and needing to pass 4 for word operations. 


_______________
## Worklog: Part 1 
|date | hours worked | what did I work on? |
|-----|--------------|---------------------|
|5/19/25| 2 hours | Set up base of project, added declarations for enums and globals and function prototypes  |
|5/27/25 | 3 hours | Added function comments, added to enums etc, began architecture. Less code today, more desiging. | 
|5/28/25 | 1.4 hours | Added executable for makebinary, updated cmakelists to include. Added another folder to test where this translation will happen. |
|5/29/25 | 3 hours | Made some changes to build, added validation on start, updated build path for test executable | 
|6/3/25 | 3 hours  | Finished init_mem, added some files for other sections later. Most work was done not on computer, drawing diagrams/flowcharts etc|
|6/4/25 | 3.75ish hours | designed on paper, designed structures for handling memory. Updated tests and build path for use with binaries later down the line. Changed make/cmake to play nice with test folder|
|6/5/25 | 1.5 hours | added some more options for make, added tests for the memory loader|
|6/6/25 | 3 hours | Added functionality to fetch(), completely redid the test file AND cmakelists.txt. Laid out (bad) prototypes for instruction set.|
|6/7/25 | 3 hours | Realized I had made a mistake in fetch and corrected it, added comments for what to do with instructions. Started working on decode.| 
|6/9/25 | 2 hours | Worked on mov.cpp and decode. Started implementing execute functions, realized I made a mistake in function signatures. Things are broken right now (linker issues, need to add things to cmakelists), but I will fix it all tomorrow. | 
| 6/10/25 | 1.5 hours | Finished up move operations, started on arithmatic ones.  | 
| 6/11/25 | 3 hours | Moved away from operations being in files, moved everything into emu4380 header and cpp file. Added arith operations| 
|6/12/25 | 2 hours | added trp functionality |
|6/13/25 | 1.4 hours | found some serious bugs in emu4380, created plan to fix |   
|6/14/25 | 1.7 hours | research on gtest, started working on making test fixtures|
|6/16/25 | 1 hour | realized I need a way to stop the emulator, added STOP functionality | 
|6/17/25 | 1.5 hours | worked on gtest fixtures, worked on arith tests. regretting that I didn't do this before | 
|6/18/25 | 3.5 hours | Finished writing parametrized tests, tests showed that I had some incorrect logic on move operations. Fixed that bug as well |


