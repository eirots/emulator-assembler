#wrapper makefile, actual makefile for project is generated inside of build 

.PHONY: all test clean 

all: 
	@cmake -S . -B build 
	@cmake --build build 

run_tests: all
	@ctest --test-dir build 

clean:
	rm -rf build