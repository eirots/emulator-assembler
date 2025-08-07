# <project-root>/Makefile  (thin wrapper – all real work stays in build/)
.PHONY: build test clean run assemble docs 

BUILD_DIR := build
CXXFLAGS += -I$(PROJECT_ROOT)/include
ASM_SRC := $(if $(FILE),$(FILE),test/assemblerTest/example.asm)
EMU_SRC := $(if $(FILE),$(FILE),build/example.bin)

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
# this is so broken it isn't even funny. These tests aren't really worth running outside of gtest though, commenting out for now 
# ------------------------------------------------------------------
#check: build
#	@cmake --build build --target check 

# ------------------------------------------------------------------
# Run with cache option 1, used for quick checks 
# ------------------------------------------------------------------
#emu_run: build
#	@cd build && ./bin/emu4380 prog_a.bin -c 1


# ------------------------------------------------------------------
# Generate autodocs
# ------------------------------------------------------------------
docs:
	@doxygen Doxyfile
# ------------------------------------------------------------------
# Assemble specific file 
# ------------------------------------------------------------------
assemble:
# make asm FILE="FILEPATH"
	@mkdir -p $(BUILD_DIR)
	@cp assembler/asm4380.py $(BUILD_DIR)/
	@cp $(ASM_SRC) $(BUILD_DIR)/
	@echo "Assembling $(notdir $(ASM_SRC)) → $(notdir $(ASM_SRC:.asm=.bin))"
	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/$(notdir $(ASM_SRC))

# usage: make run FILE=path/to/MyProg.asm
run: assemble build
	@echo "Running emulator on $(notdir $(FILE:.asm=.bin))"
	@cd $(BUILD_DIR) && ./bin/emu4380 $(notdir $(FILE:.asm=.bin)) 

asm_test:
	@pytest -v
# ------------------------------------------------------------------
# Assemble + run example in one go
# ------------------------------------------------------------------
#asm_example: 
#	@cp assembler/asm4380.py $(BUILD_DIR)/
#	@cp test/assemblerTest/example.asm $(BUILD_DIR)/
#	@echo "assembling example.asm → example.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/example.asm
#	@echo "→ running emulator on example.bin"
#	@bin/emu4380 $(BUILD_DIR)/example.bin




# ------------------------------------------------------------------
# Assemble sample binaries, useful after running make clean 
# ------------------------------------------------------------------
#asm_binaries:
#	@cp assembler/asm4380.py $(BUILD_DIR)/
#	@cp test/assemblerTest/example.asm $(BUILD_DIR)/
#	@cp test/assemblerTest/prog_a.asm $(BUILD_DIR)/
#	@cp test/assemblerTest/prog_b.asm $(BUILD_DIR)/
#	@cp test/assemblerTest/prog_c.asm $(BUILD_DIR)/
#	@cp test/assemblerTest/prog_d.asm $(BUILD_DIR)/
#	@cp test/assemblerTest/prog_e.asm $(BUILD_DIR)/
#	@cp test/assemblerTest/prog_f.asm $(BUILD_DIR)/
#	@echo "assembling example.asm → example.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/example.asm
#	@echo "assembling prog_a.asm → prog_a.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/prog_a.asm
#	@echo "assembling prog_b.asm → prog_b.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/prog_b.asm
#	@echo "assembling prog_c.asm → prog_c.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/prog_c.asm
#	@echo "assembling prog_d.asm → prog_d.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/prog_d.asm
#	@echo "assembling prog_e.asm → prog_e.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/prog_e.asm
#	@echo "assembling prog_f.asm → prog_f.bin"
#	@python3 $(BUILD_DIR)/asm4380.py $(BUILD_DIR)/prog_f.asm
	
	
# ------------------------------------------------------------------
# Wipe the entire build tree.
# ------------------------------------------------------------------
clean:
	@rm -rf build
	@rm -rf docs/automatedDocumentation