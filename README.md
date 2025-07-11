# CS4380 Project 1 - 4380 Emulator Mark 1


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
| 7/11 | TODO: | TODO: |




## Worklog (Project 1)
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
## Questions: 


## Validation
