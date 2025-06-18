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
// 1.  Loader suite  (unchanged)
// -----------------------------------------------------------------------------
static constexpr uint32_t kMem = 65'536;
static constexpr char kGoodBin[] = "smalladd.bin";

class LoaderTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_TRUE(init_mem(kMem));
        ASSERT_EQ(load_binary(kGoodBin), 0u) << "load failed";
    }
};

TEST_F(LoaderTest, FirstEightBytesMatchFile) {
    const unsigned char expect[8] =
        {0x12, 0x05, 0x00, 0x0F, 0, 0, 0, 0};
    EXPECT_TRUE(std::equal(prog_mem, prog_mem + 8, expect));
}

TEST_F(LoaderTest, PcUnaffectedByLoad) { EXPECT_EQ(reg_file[PC], 0u); }

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
// 4.  Parameterised operation tests (trp excluded)
// -----------------------------------------------------------------------------

struct ArithCase {
    uint8_t opcode;
    uint32_t rs1, rs2, imm;
    uint32_t expect;
    bool ok;
};

struct JumpCase {
    uint32_t imm;
    uint32_t expectPC;
    bool ok;
};

struct MoveCase {
    uint8_t opcode, rd, rs;
    uint32_t imm;
};

struct TrpCase {
    uint32_t imm;
    uint32_t r3In;
    const char* cinBuffer;
    const char* coutExpected;
    bool ok;
};

static const JumpCase kJumpCases[] = {
    // imm expectPC ok
    {0x40, 0x40, true},
    {kMem, 0, false}  // out of bounds, decode should fail
};

static const ArithCase kArithCases[] = {
    // opcode  rs1 rs2 imm  expect  ok
    {OP_ADD, 2, 3, 0, 5, true},
    {OP_ADDI, 7, 0, 4, 11, true},
    {OP_SUB, 5, 8, 0, 0xFFFFFFFD, true},
    {OP_SUBI, 5, 0, 0x08, 0xFFFFFFFD, true},
    {OP_MUL, 5, 2, 0, 0x0A, true},
    {OP_MULI, 5, 0, 2, 0x0A, true},
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
    {OP_STR, 0, R2, 0x0020},
    {OP_LDR, HP, 0, 0x0020},
    {OP_STB, 0, HP, 0x0031},  // use HP as RS so is_state_rg passes
    {OP_LDB, HP, 0, 0x0031},
};

static const TrpCase kTrpCases[] = {
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

TEST_P(JumpParam, UpdatesPCorFails) {
    const auto& tc = GetParam();
    loadInstr(OP_JMP, 0, 0, 0, tc.imm);

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

    EXPECT_EQ(cameOut, tc.coutExpected);
    EXPECT_EQ(runBool, tc.ok);

    if (tc.imm == 2 || tc.imm == 4)
        EXPECT_EQ(reg_file[R3], tc.cinBuffer[0] == 'Q' ? 'Q' : 42u);
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

INSTANTIATE_TEST_SUITE_P(AllArithmetic,
                         ArithParam,
                         ::testing::ValuesIn(kArithCases));

INSTANTIATE_TEST_SUITE_P(AllJumps,
                         JumpParam,
                         ::testing::ValuesIn(kJumpCases));

INSTANTIATE_TEST_SUITE_P(AllMoves,
                         MoveParam,
                         ::testing::ValuesIn(kMoveCases));

INSTANTIATE_TEST_SUITE_P(Trps_zero_thru_four,
                         TrpParam,
                         ::testing::ValuesIn(kTrpCases));