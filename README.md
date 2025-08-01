# CS4380 Project 4 - 4380 Emulator Mark 1


## Worklog (Project 4)
|date | hours worked | what did I work on? | 
|-----|--------------|---------------------|
|7/30 | 8h         | Started adding mew instructions and directives to assembler, added prototypes for new instructions to emulator. Emulator will be really easy to implement, but I'll do that once I'm finished with the assembler. Also fixed a bug with int directives|


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


## How to use: 

`make` 
`make build` 
-  Builds the Emulator inside of `build/bin`. Creates `build` folder if it doesn't exist. 
-  Moves the Assembler and a test file into the `build` directory.

`make test`
- Runs `make build`.
- Compiles and runs emulator tests. 

`make emu_run` 
- Runs `make build`
- Runs emulator on example test file `prog_a.bin` with cache setting of 1. 

`make docs`
- Generates autodocumentation.
- NOTE: This requires `doxygen` to be installed 

`make assemble FILE=[EXAMPLE_TEST_FILE_NAME.asm]`
- Copies assembler file into `build` folder and executes assembler on `EXAMPLE_TEST_FILE_NAME.asm`.

`make asm_test`
- Runs assembler tests. 
- NOTE: This command requires `pytest` to be installed. 

`make clean`
- Removes build folder and autodocs.
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


