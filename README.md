# CS4380 Project 1 - 4380 Emulator Mark 1
# INVOKING THE NON_LATE POLICY


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


___________


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


