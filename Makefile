# <project-root>/Makefile  (thin wrapper – all real work stays in build/)
.PHONY: build test clean run

BUILD_DIR := build
CXXFLAGS += -I$(PROJECT_ROOT)/include

# ------------------------------------------------------------------
# Build everything (configure if build/ does not exist).
# ------------------------------------------------------------------
build: 
	@cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	@cmake --build $(BUILD_DIR) --parallel
	@echo "Copying Python driver and example asm into $(BUILD_DIR)/"
	@cp assembler/asm4380.py $(BUILD_DIR)/
	@cp test/assemblerTest/example.asm $(BUILD_DIR)/

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
# Assemble + run example in one go
# ------------------------------------------------------------------
asm_run: 
	@cp assembler/asm4380.py $(BUILD_DIR)/
	@cp test/assemblerTest/example.asm $(BUILD_DIR)/
	@echo "assembling example.asm → example.bin"
	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/example.asm
#	@echo "→ running emulator on example.bin"
#	@bin/emu4380 $(BUILD_DIR)/example.bin
# ------------------------------------------------------------------
# Wipe the entire build tree.
# ------------------------------------------------------------------
clean:
	@rm -rf build