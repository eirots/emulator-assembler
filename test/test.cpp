// ──────────────────────────────────────────────────────────────
//  Google-Test file:  test/test.cpp
// ──────────────────────────────────────────────────────────────
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstring>
#include <numeric>  // std::iota

#include "emu4380.h"

// -----------------------------------------------------------------------------
// 1.  Loader
// -----------------------------------------------------------------------------
static constexpr uint32_t kMem = 131'072;
static constexpr char kGoodBin[] = "example.bin";

class LoaderTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_TRUE(init_mem(kMem));
        ASSERT_EQ(load_binary(kGoodBin), 0u) << "load failed";
    }
};

TEST_F(LoaderTest, LoadsEntryPointInPC) {
    uint32_t entry = prog_mem[0] |
                     prog_mem[1] << 8 |
                     prog_mem[2] << 16 |
                     prog_mem[3] << 24;

    EXPECT_EQ(reg_file[PC], entry) << "PC should be at file's entry point";
}

TEST_F(LoaderTest, FirstInstrAtEntry) {
    uint32_t entry = reg_file[PC];

    const unsigned char expected[8] = {
        0x01, 0x00, 0x00, 0x00,
        0x12, 0x00, 0x00, 0x00};

    EXPECT_TRUE(std::equal(
        prog_mem + entry,
        prog_mem + entry + 8,
        expected))
        << "First instruction bytes did not match at entry=" << entry;
}

TEST_F(LoaderTest, PcUnaffectedByLoad) { EXPECT_EQ(reg_file[PC], prog_mem[0] |
                                                                     prog_mem[1] << 8 |
                                                                     prog_mem[2] << 16 |
                                                                     prog_mem[3] << 24); }

// -----------------------------------------------------------------------------
// 2.  Generic instruction fixture
// -----------------------------------------------------------------------------
class InstrTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_TRUE(init_mem(kMem));
        std::memset(prog_mem, 0, kMem);
        std::memset(reg_file, 0, 22 * sizeof(uint32_t));
        runBool = true;
        reg_file[PC] = 0;
    }

    void loadInstr(uint8_t op,
                   uint8_t a = 0, uint8_t b = 0, uint8_t c = 0,

                   uint32_t imm = 0) {
        prog_mem[0] = op;
        prog_mem[1] = a;
        prog_mem[2] = b;
        prog_mem[3] = c;
        prog_mem[4] = imm & 0xFF;
        prog_mem[5] = (imm >> 8) & 0xFF;
        prog_mem[6] = (imm >> 16) & 0xFF;
        prog_mem[7] = (imm >> 24) & 0xFF;
    }
};

// -----------------------------------------------------------------------------
// 3. good path fetch tests
// -----------------------------------------------------------------------------
TEST_F(InstrTest, FetchSingleInstruction) {
    loadInstr(0x12, 0x05, 0x00, 0x0F);  // ADD R5,R0,R15
    ASSERT_TRUE(fetch());

    EXPECT_EQ(cntrl_regs[OPERATION], 0x12u);
    EXPECT_EQ(cntrl_regs[OPERAND_1], 0x05u);
    EXPECT_EQ(cntrl_regs[OPERAND_2], 0x00u);
    EXPECT_EQ(cntrl_regs[OPERAND_3], 0x0Fu);
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 0u);
    EXPECT_EQ(reg_file[PC], 8u);
}

TEST_F(InstrTest, FetchFailsNearEnd) {
    reg_file[PC] = mem_size - 3;
    EXPECT_FALSE(fetch());
}

// -----------------------------------------------------------------------------
// 4.  Parameterised operation tests
// -----------------------------------------------------------------------------

struct ArithCase {
    uint8_t opcode;
    uint32_t rs1, rs2, imm;
    uint32_t expect;
    bool ok;
};

struct JumpCase {
    uint8_t opcode, rs1;
    uint32_t regval, imm, expectPC;
    bool ok;
};

struct MoveCase {
    uint8_t opcode, rd, rs;
    uint32_t imm;
};

struct IndirectStoreCase {
    uint8_t opcode, rs, rg;
    bool ok;
};

struct IndirectLoadCase {
    uint8_t opcode, rd, rg;
    bool ok;
};

struct TrpCase {
    uint32_t imm;
    uint32_t r3In;
    const char* cinBuffer;
    const char* coutExpected;
    bool ok;
};

struct CmpCase {
    uint8_t opcode, rd, rs1, rs2;
    int8_t rs1val, rs2val;
    int32_t imm;
    int32_t expect;
};

static const JumpCase kJumpCases[] = {
    // opcode rs1 regval imm expectPC ok
    {OP_JMP, 0, 0, 0x40, 0x40, true},  // plain jump
    {OP_JMP, 0, 0, kMem, 0u, false},   // jump out of bounds
    {OP_JMR, 9, 1, 0, 1, true},        // should pass
    {OP_BNZ, 9, 1, 0x08, 8u, true},    // should not branch
    {OP_BNZ, 9, 0, 0x10, 8u, true},    // should branch

    // Update PC to Address if RS > 0
    {OP_BGT, 0, 0, 0, 8u, true},  // no change
    {OP_BGT, 1, 1, 0, 0, true},   // branch back to 0
    {OP_BGT, 1, 1, kMem, false},  // branch oob
    // update pc to address if rs < 0
    {OP_BLT, 0, 0, 0, 8u, true},                             // no branch
    {OP_BLT, 1, static_cast<uint32_t>(-1), 0, 0, true},      // branch
    {OP_BLT, 1, static_cast<uint32_t>(-1), kMem, 0, false},  // branch oob
    // update PC to address if RS = 0
    {OP_BRZ, 5, 0, 0x14, 0x14, true},  // should not branch
    {OP_BRZ, 5, 3, 0x20, 8u, true}     // oob, should fail
    //{kMem, 0, false}  // out of bounds, decode should fail
};

static const ArithCase kArithCases[] = {
    // opcode  rs1 rs2 imm  expect  ok
    {OP_ADD, 2, 3, 0, 5, true},
    {OP_ADDI, 7, 0, 4, 11, true},
    {OP_SUB, 5, 8, 0, 0xFFFFFFFD, true},
    {OP_SUBI, 5, 0, 0x08, 0xFFFFFFFD, true},
    {OP_MUL, 5, 2, 0, 10, true},
    {OP_MULI, 5, 0, 2, 10, true},
    {OP_MUL, 5, 0, 0, 0, true},
    {OP_DIV, 9, 0, 0, 0, false},  // div/0
    {OP_DIV, 9, 3, 0, 3, true},
    {OP_SDIV, 9, 0, 0, 0, false},  // div/0
    {OP_SDIV, 9, 3, 0, 3, true},
    {OP_DIVI, 9, 0, 0, 0, false},  // div/0
    {OP_DIVI, 9, 0, 3, 3, true},   // div/0
};

static const MoveCase kMoveCases[] = {
    // opcode, rd, rs (can just be 0 if not needed), imm
    {OP_MOV, HP, R2, 0},
    {OP_MOVI, HP, 0, 0xCAFEBABE},
    {OP_LDA, HP, 0, 0x0040},
    {OP_STR, R2, 0, 0x0020},
    {OP_LDR, HP, 0, 0x0020},
    {OP_STB, HP, 0, 0x0031},
    {OP_LDB, HP, 0, 0x0031},
};

static const IndirectStoreCase kStoreCases[]{
    // opcode, rd, rg, ok
    {OP_ISTR, R2, R3, true},
    {OP_ISTB, R2, R3, true},
    {OP_ISTR, R1, R0, false},  // OOB on ISTR
    {OP_ISTB, R1, R0, false}   // OOB on ISTB}
};

static const IndirectLoadCase kLoadCases[]{
    // opcode, rd, rg, ok
    {OP_ILDR, R3, R2, true},
    {OP_ILDB, R3, R2, true},
    {OP_ILDR, R0, R1, false},  // oob
    {OP_ILDB, R0, R1, false}   // oob
};

static const CmpCase kCmpCases[]{
    // opcode, rd, rs1, rs2, rs1val, rs2val, imm,  expect
    {OP_CMP, 5, R2, R3, 4, 3, 0, 1},   // rs1  > rs2, rd = 1
    {OP_CMP, 5, R4, R4, 3, 3, 0, 0},   // rs1 == rs2, rd = 0
    {OP_CMP, 5, R7, R1, 3, 4, 0, -1},  // rs1 < rs2, rd = -1

    // negative cases
    {OP_CMP, 5, R2, R3, -4, -3, 0, -1},  // rs1  > rs2, rd = -1
    {OP_CMP, 5, R4, R4, -3, -3, 0, 0},   // rs1 == rs2, rd = 0
    {OP_CMP, 5, R7, R1, -3, -4, 0, 1},   // rs1 < rs2, rd = 1

    // one negative, one not
    {OP_CMP, 5, R2, R3, 4, -3, 0, 1},   // rs1  > rs2, rd = 1
    {OP_CMP, 5, R7, R1, -3, 4, 0, -1},  // rs1 < rs2, rd = -1

    // cmpi tests
    {OP_CMPI, 6, R2, 0, 100, 0, 42, 1},  // rs1  > imm, rd = 1
    {OP_CMPI, 6, R3, 0, 13, 0, 13, 0},   // rs1 == imm, rd = 0
    {OP_CMPI, 6, R9, 0, 0, 0, 7, -1},    // rs1 < imm, rd = -1

    // negative cases
    {OP_CMPI, 6, R2, 0, -100, 0, -42, -1},  // rs1  > imm, rd = -1
    {OP_CMPI, 6, R3, 0, -13, 0, -13, 0},    // rs1 == imm, rd = 0
    {OP_CMPI, 6, R9, 0, 0, 0, -7, 1},       // rs1 < imm, rd = 1

    // one negative, one not
    {OP_CMPI, 6, R2, 0, 100, 0, -42, 1},  // rs1  > imm, rd = 1
    {OP_CMPI, 6, R9, 0, -1, 0, 7, -1},    // rs1 < imm, rd = -1
};

static const TrpCase kTrpCases[] = {
    // TODO: fix these parameters
    // imm, r3In, , cinBuffer, coutExpected ok
    {0, 0, "", "", false},          // STOP
    {1, 12345, "", "12345", true},  // print int
    {2, 0, "42\n", "", true},       // read int
    {3, 'X', "", "X", true},        // print char
    {4, 0, "Q\n", "", true},        // read char
};

class JumpParam : public InstrTest,
                  public ::testing::WithParamInterface<JumpCase> {};
class ArithParam : public InstrTest,
                   public ::testing::WithParamInterface<ArithCase> {};
class MoveParam : public InstrTest,
                  public ::testing::WithParamInterface<MoveCase> {};
class TrpParam : public InstrTest,
                 public ::testing::WithParamInterface<TrpCase> {};
class StoreParam : public InstrTest,
                   public ::testing::WithParamInterface<IndirectStoreCase> {};
class LoadParam : public InstrTest,
                  public ::testing::WithParamInterface<IndirectLoadCase> {};
class CmpParam : public InstrTest,
                 public ::testing::WithParamInterface<CmpCase> {};

TEST_P(JumpParam, UpdatesPCorFails) {
    const auto& tc = GetParam();
    loadInstr(tc.opcode, tc.rs1, 0, 0, tc.imm);
    reg_file[tc.rs1] = tc.regval;

    ASSERT_TRUE(fetch());
    bool decodeOK = decode();

    if (tc.ok) {
        ASSERT_TRUE(decodeOK);
        ASSERT_TRUE(execute());
        EXPECT_EQ(reg_file[PC], tc.expectPC);
    } else {
        EXPECT_FALSE(decodeOK);
    }
}

TEST_P(ArithParam, ExecuteProducesExpectedResult) {
    const ArithCase& tc = GetParam();

    // Use R5 as destination, R0 as RS1, R15 as RS2
    reg_file[R0] = tc.rs1;
    reg_file[R15] = tc.rs2;

    loadInstr(tc.opcode, /*RD*/ 5, /*RS1*/ 0, /*RS2*/ 15, tc.imm);

    ASSERT_TRUE(fetch());
    ASSERT_TRUE(decode());

    bool ok = execute();
    if (tc.ok) {
        EXPECT_TRUE(ok);
        EXPECT_EQ(reg_file[R5], tc.expect);
    } else {
        EXPECT_FALSE(ok);
    }
}

TEST_P(MoveParam, MoveParamTests) {
    const MoveCase& tc = GetParam();

    reg_file[R2] = 0x12345678;
    reg_file[R3] = 0xAABBCCDD;

    // Pre-load memory for loads
    *reinterpret_cast<uint32_t*>(prog_mem + 0x0020) = 0xDEADBEEF;
    prog_mem[0x0031] = 0xEF;

    loadInstr(tc.opcode,
              (tc.opcode == OP_STR || tc.opcode == OP_STB) ? tc.rs : tc.rd, tc.rs, 0, tc.imm);

    ASSERT_TRUE(fetch());
    ASSERT_TRUE(decode());
    ASSERT_TRUE(execute());

    switch (tc.opcode) {
        case OP_MOV:
            EXPECT_EQ(reg_file[tc.rd], reg_file[tc.rs]);
            break;

        case OP_MOVI:
        case OP_LDA:  // both just copy immediate into RD
            EXPECT_EQ(reg_file[tc.rd], tc.imm);
            break;

        case OP_STR:
            EXPECT_EQ(*reinterpret_cast<uint32_t*>(prog_mem + tc.imm),
                      reg_file[tc.rs]);
            break;

        case OP_LDR:
            EXPECT_EQ(reg_file[tc.rd],
                      *reinterpret_cast<uint32_t*>(prog_mem + tc.imm));
            break;

        case OP_STB:
            EXPECT_EQ(prog_mem[tc.imm] & 0xFF,
                      static_cast<uint8_t>(reg_file[tc.rs]));
            break;

        case OP_LDB:
            EXPECT_EQ(reg_file[tc.rd] & 0xFF, prog_mem[tc.imm]);
            break;
    }
}

TEST_P(StoreParam, ExecuteorFails) {
    auto tc = GetParam();
    reg_file[tc.rg] = (tc.ok ? 0x0020u : mem_size);

    reg_file[tc.rs] = 0xDEADBEEF;

    loadInstr(tc.opcode, tc.rs, tc.rg, 0, 0);

    ASSERT_TRUE(fetch());
    ASSERT_TRUE(decode());
    bool ok = execute();

    if (tc.ok) {
        ASSERT_TRUE(ok);
        if (tc.opcode == OP_ISTR) {
            uint32_t stored = *reinterpret_cast<uint32_t*>(prog_mem + reg_file[tc.rg]);
            EXPECT_EQ(stored, reg_file[tc.rs]);
        } else {
            EXPECT_EQ(prog_mem[reg_file[tc.rg]] & 0xFF, static_cast<uint8_t>(reg_file[tc.rs]));
        }
    } else {
        EXPECT_FALSE(ok);
    }
}

TEST_P(LoadParam, ExecutesorFails) {
    auto tc = GetParam();
    uint32_t addr = tc.ok ? 0x0020u : mem_size;
    reg_file[tc.rg] = addr;

    if (tc.opcode == OP_ILDR) {
        *reinterpret_cast<uint32_t*>(prog_mem + addr) = 0xDEADBEEF;
    } else {
        prog_mem[addr] = 0xAB;
    }

    loadInstr(tc.opcode, tc.rd, tc.rg, 0, 0);
    ASSERT_TRUE(fetch());
    ASSERT_TRUE(decode());
    bool ok = execute();

    if (tc.ok) {
        ASSERT_TRUE(ok);
        if (tc.opcode == OP_ILDR) {
            EXPECT_EQ(reg_file[tc.rd], 0xDEADBEEF);
        } else {
            // ildb case
            EXPECT_EQ(reg_file[tc.rd] & 0xFF, prog_mem[addr]);
        }
    } else {
        EXPECT_FALSE(ok);
    }
}

TEST_P(CmpParam, EmitsCorrectCompareResult) {
    auto tc = GetParam();

    reg_file[tc.rs1] = tc.rs1val;
    reg_file[tc.rs2] = tc.rs2val;

    loadInstr(tc.opcode, tc.rd, tc.rs1, tc.rs2, tc.imm);

    ASSERT_TRUE(fetch());
    ASSERT_TRUE(decode());
    ASSERT_TRUE(execute());

    EXPECT_EQ(static_cast<int32_t>(reg_file[tc.rd]), tc.expect);
}

TEST_P(TrpParam, TrpParamTestsMinus_98) {
    const auto& tc = GetParam();

    reg_file[R3] = tc.r3In;
    std::istringstream fakeIn(tc.cinBuffer);
    std::streambuf* oldCin = std::cin.rdbuf(fakeIn.rdbuf());

    testing::internal::CaptureStdout();

    loadInstr(OP_TRP, 0, 0, 0, tc.imm);
    ASSERT_TRUE(fetch());
    ASSERT_TRUE(decode());
    ASSERT_TRUE(execute());

    std::string cameOut = testing::internal::GetCapturedStdout();
    std::cin.rdbuf(oldCin);

    if (tc.imm == 0) {
        string expected = "Execution completed. Total memory cycles: 10\n";
        EXPECT_EQ(cameOut, expected)
            << "TRP 0 should dump total cycles, not '" << cameOut << "'";

        EXPECT_FALSE(runBool);
    } else {
        EXPECT_EQ(cameOut, tc.coutExpected);
        EXPECT_EQ(runBool, tc.ok);

        if (tc.imm == 2 || tc.imm == 4)
            EXPECT_EQ(reg_file[R3], tc.cinBuffer[0] == 'Q' ? 'Q' : 42u);
    }
}

TEST_F(InstrTest, Trap98_PrintsFirstSixRegistersCorrectly) {
    reg_file[R0] = 0x111;
    reg_file[R1] = 0x222;
    reg_file[R2] = 0x333;
    reg_file[R3] = 0x444;
    reg_file[R4] = 0x555;
    reg_file[R5] = 0x666;

    loadInstr(OP_TRP, 0, 0, 0, 98);

    testing::internal::CaptureStdout();
    ASSERT_TRUE(fetch());
    ASSERT_TRUE(decode());
    ASSERT_TRUE(execute());
    std::string dump = testing::internal::GetCapturedStdout();

    std::istringstream iss(dump);
    std::vector<std::string> lines;
    for (std::string line; std::getline(iss, line);)
        if (!line.empty()) lines.push_back(line);

    ASSERT_GE(lines.size(), 6u) << "Dump too short";

    EXPECT_EQ(lines[0], "R0\t273");
    EXPECT_EQ(lines[1], "R1\t546");
    EXPECT_EQ(lines[2], "R2\t819");
    EXPECT_EQ(lines[3], "R3\t1092");
    EXPECT_EQ(lines[4], "R4\t1365");
    EXPECT_EQ(lines[5], "R5\t1638");
}

// -----------------------------------------------------------------------------
// 5.  Cache tests
// -----------------------------------------------------------------------------

class CacheTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_TRUE(init_mem(kMem));

        memset(prog_mem, 0xAA, kMem);
        memset(reg_file, 0, 22 * sizeof(uint32_t));
        mem_cycle_cntr = 0;

        runBool = true;
        reg_file[PC] = 0;
    }
    void loadInstr(uint8_t op,
                   uint8_t a = 0, uint8_t b = 0, uint8_t c = 0,

                   uint32_t imm = 0) {
        prog_mem[0] = op;
        prog_mem[1] = a;
        prog_mem[2] = b;
        prog_mem[3] = c;
        prog_mem[4] = imm & 0xFF;
        prog_mem[5] = (imm >> 8) & 0xFF;
        prog_mem[6] = (imm >> 16) & 0xFF;
        prog_mem[7] = (imm >> 24) & 0xFF;
    }
};

TEST_F(CacheTest, NoCacheInitializes) {
    init_cache(0);
    EXPECT_FALSE(cacheUsed);
    EXPECT_EQ(cache, nullptr);
    EXPECT_EQ(associativity, -1);
    EXPECT_EQ(num_sets, -1);
}

// testing with 64 lines and block size 16
TEST_F(CacheTest, DirectMappedInitializes) {
    init_cache(1);
    EXPECT_TRUE(cacheUsed);
    EXPECT_NE(cache, nullptr);

    EXPECT_EQ(associativity, 1);
    EXPECT_EQ(num_sets, NUM_CACHE_LINES / associativity);

    for (size_t s = 0; s < num_sets; s++) {
        for (size_t w = 0; w < associativity; w++) {
            EXPECT_FALSE(cache[s][w].dirty);
            EXPECT_FALSE(cache[s][w].valid);
            EXPECT_EQ(0, cache[s][w].tag);
        }
    }
    EXPECT_EQ(current_cache_type, DIRECT_MAPPED);
}

// testing with 64 lines and block size 16
TEST_F(CacheTest, FullAssociativeInitializes) {
    init_cache(2);
    EXPECT_TRUE(cacheUsed);
    EXPECT_NE(cache, nullptr);

    EXPECT_EQ(associativity, 64);
    EXPECT_EQ(num_sets, NUM_CACHE_LINES / associativity);

    for (size_t s = 0; s < num_sets; s++) {
        for (size_t w = 0; w < associativity; w++) {
            EXPECT_FALSE(cache[s][w].dirty);
            EXPECT_FALSE(cache[s][w].valid);
            EXPECT_EQ(0, cache[s][w].tag);
        }
    }
    EXPECT_EQ(current_cache_type, FULLY_ASSOCIATIVE);
}

// testing with 64 lines and block size 16
TEST_F(CacheTest, TwoWayInitialzies) {
    init_cache(3);
    EXPECT_TRUE(cacheUsed);
    EXPECT_NE(cache, nullptr);

    EXPECT_EQ(associativity, 2);
    EXPECT_EQ(num_sets, NUM_CACHE_LINES / associativity);

    for (size_t s = 0; s < num_sets; s++) {
        for (size_t w = 0; w < associativity; w++) {
            EXPECT_FALSE(cache[s][w].dirty);
            EXPECT_FALSE(cache[s][w].valid);
            EXPECT_EQ(0, cache[s][w].tag);
        }
    }
    EXPECT_EQ(current_cache_type, TWO_WAY_SET_ASSOCIATIVE);
}

TEST_F(CacheTest, DirectMappedMissToHit) {
    init_cache(1);
    mem_cycle_cntr = 0;

    readByte(0x1000);
    // For a miss causing a single cache block to be read before returning the requested value, 1 + 8  + (2*3) = 15 clock cycles
    EXPECT_EQ(mem_cycle_cntr, 15) << "Mem cycle counter is " << mem_cycle_cntr;

    readByte(0x1000);
    EXPECT_EQ(mem_cycle_cntr, 16);
}

TEST_F(CacheTest, FullyAssociativeMissToHit) {
    init_cache(2);
    mem_cycle_cntr = 0;

    readByte(0x1000);
    // For a miss causing a single cache block to be read before returning the requested value, 1 + 8  + (2*3) = 15 clock cycles
    EXPECT_EQ(mem_cycle_cntr, 15) << "Mem cycle counter is " << mem_cycle_cntr;

    readByte(0x1000);
    EXPECT_EQ(mem_cycle_cntr, 16);
}

TEST_F(CacheTest, TwoWayMissToHit) {
    init_cache(3);
    mem_cycle_cntr = 0;

    readByte(0x1000);
    // For a miss causing a single cache block to be read before returning the requested value, 1 + 8  + (2*3) = 15 clock cycles
    EXPECT_EQ(mem_cycle_cntr, 15) << "Mem cycle counter is " << mem_cycle_cntr;

    readByte(0x1000);
    EXPECT_EQ(mem_cycle_cntr, 16);
}

TEST_F(CacheTest, ReadSingleWordUncachedCosts8CyclesEachTime) {
    init_cache(0);
    readWord(0x1000);
    EXPECT_EQ(mem_cycle_cntr, 8) << "First read ";

    readWord(0x1000);
    EXPECT_EQ(mem_cycle_cntr, 16) << "Second read";
}

TEST_F(CacheTest, FirstInstructionUncachedFetchCosts8Plus2Cycles) {
    init_cache(0);  // no cache used

    loadInstr(OP_ADD, 2, 3, 0, 0);
    ASSERT_TRUE(fetch());
    EXPECT_EQ(mem_cycle_cntr, 10);
}

TEST_F(CacheTest, DirectMappedEviction) {
    init_cache(DIRECT_MAPPED);

    uint32_t A = 0;
    uint32_t B = 1024;

    readWord(A);
    EXPECT_GT(mem_cycle_cntr, 1);

    mem_cycle_cntr = 0;
    readWord(B);
    EXPECT_GT(mem_cycle_cntr, 1);

    mem_cycle_cntr = 0;
    readWord(A);
    EXPECT_GT(mem_cycle_cntr, 1);
}

TEST_F(CacheTest, FullyAssociativeEviction) {
    vector<uint32_t> blocks;
    for (uint32_t i = 0; i < NUM_CACHE_LINES; i++) {
        blocks.push_back(i * BLOCK_SIZE);
    }
    const uint32_t x = NUM_CACHE_LINES * BLOCK_SIZE;

    for (auto addr : blocks) readWord(addr);  // miss everything
    mem_cycle_cntr = 0;
    readWord(x);
    EXPECT_GT(mem_cycle_cntr, 2);
}

INSTANTIATE_TEST_SUITE_P(AllArithmetic,
                         ArithParam,
                         ::testing::ValuesIn(kArithCases));

INSTANTIATE_TEST_SUITE_P(AllJumps,
                         JumpParam,
                         ::testing::ValuesIn(kJumpCases));

INSTANTIATE_TEST_SUITE_P(AllMoves,
                         MoveParam,
                         ::testing::ValuesIn(kMoveCases));

INSTANTIATE_TEST_SUITE_P(AllIndirectStores, StoreParam,
                         ::testing::ValuesIn(kStoreCases));

INSTANTIATE_TEST_SUITE_P(AllIndirectLoads, LoadParam,
                         ::testing::ValuesIn(kLoadCases));

INSTANTIATE_TEST_SUITE_P(AllCompares, CmpParam,
                         ::testing::ValuesIn(kCmpCases));

INSTANTIATE_TEST_SUITE_P(Trps_zero_thru_four,
                         TrpParam,
                         ::testing::ValuesIn(kTrpCases));
