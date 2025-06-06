# <project-root>/Makefile  (thin wrapper – all real work stays in build/)
.PHONY: build test clean run

# ------------------------------------------------------------------
# Build everything (configure if build/ does not exist).
# ------------------------------------------------------------------
build:
	@cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	@cmake --build build --parallel

# ------------------------------------------------------------------
# Run the Google-Test suite **through CTest** (always the same way).
# ------------------------------------------------------------------
test: build
	@cd build && ctest --output-on-failure

# ------------------------------------------------------------------
# Quick run of GTest, builds and runs.
# ------------------------------------------------------------------
check: build
	@cmake --build build --target check 

# ------------------------------------------------------------------
# Example helper to run the emulator manually.
# ------------------------------------------------------------------
run: build
	@cd bin && ./emu4380

# ------------------------------------------------------------------
# Wipe the entire build tree.
# ------------------------------------------------------------------
clean:
	@rm -rf build
	@rm -rf bin 