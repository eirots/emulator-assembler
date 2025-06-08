// ──────────────────────────────────────────────────────────────
//  Google-Test file:  test/test.cpp
// ──────────────────────────────────────────────────────────────
#include <gmock/gmock.h>  // EXPECT_THAT, ElementsAreArray
#include <gtest/gtest.h>  // General GTest stuff

#include <array>
#include <cstring>

#include "emu4380.h"

// Helper constants
static constexpr uint32_t kDefaultMemSize = 65'536;
static constexpr char kGoodBin[] = "smalladd.bin";
static constexpr std::array<unsigned char, 5> kExpected{0x00, 0x05, 0x0F, 0x00, 0x12};

// ──────────────────────────────────────────────────────────────
//  Fixture for tests that drive the full CLI (runEmulator)
// ──────────────────────────────────────────────────────────────
class StartupTest : public ::testing::Test {
   protected:
    // Nothing to set up here – each test builds its own argv/argc
};

// ---------------- CLI / argument handling ---------------------
TEST_F(StartupTest, NoArguments) {
    const char* argv[] = {"emu4380"};
    EXPECT_EQ(runEmulator(1, const_cast<char**>(argv)), 1);
}

TEST_F(StartupTest, OnlyInputFile) {
    const char* argv[] = {"emu4380", kGoodBin};
    EXPECT_EQ(runEmulator(2, const_cast<char**>(argv)), 0);
}

TEST_F(StartupTest, WithMemorySize) {
    const char* argv[] = {"emu4380", kGoodBin, "65536"};
    EXPECT_EQ(runEmulator(3, const_cast<char**>(argv)), 0);
}

TEST_F(StartupTest, InvalidMemorySize) {
    const char* argv[] = {"emu4380", "dummy.bin", "not_a_number"};
    EXPECT_EQ(runEmulator(3, const_cast<char**>(argv)), 1);
}

TEST_F(StartupTest, MemorySizeTooSmall) {
    const char* argv[] = {"emu4380", kGoodBin, "1"};
    EXPECT_EQ(runEmulator(3, const_cast<char**>(argv)), 2);
}

// ──────────────────────────────────────────────────────────────
//  Fixture for lower-level emulator​/runtime tests
// ──────────────────────────────────────────────────────────────
class EmulatorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_TRUE(init_mem(kDefaultMemSize));
        ASSERT_EQ(load_binary(kGoodBin), 0u)  // 0 == success
            << "failed to load test binary";
        reg_file[PC] = 0;  // start PC at first byte
    }
};

// ---------------- register initialisation ---------------------
TEST_F(EmulatorTest, RegistersInitialisedCorrectly) {
    EXPECT_EQ(reg_file[PC], 0u);
    EXPECT_EQ(reg_file[SL], 0u);
    EXPECT_EQ(reg_file[SB], kDefaultMemSize);
    EXPECT_EQ(reg_file[SP], kDefaultMemSize);
    EXPECT_EQ(reg_file[FP], reg_file[SP]);
    EXPECT_EQ(reg_file[HP], reg_file[SL]);
}

// ---------------- binary contents -----------------------------
TEST_F(EmulatorTest, ProgramMemoryMatchesExpected) {
    constexpr std::array<unsigned char, 5> kExpected{0x00, 0x05, 0x0F, 0x00, 0x12};

    using ::testing::ElementsAreArray;
    EXPECT_THAT(
        std::vector<unsigned char>(prog_mem, prog_mem + kExpected.size()),
        ElementsAreArray(kExpected));
}

// ---------------- fetch behaviour -----------------------------
TEST_F(EmulatorTest, FetchSucceedsAndCopiesBytes) {
    const std::array<uint8_t, 8> raw = {
        0x13, 0x01, 0x02, 0x00,
        0x78, 0x56, 0x34, 0x12  // immediate 0x12345678
    };
    std::copy(raw.begin(), raw.end(), prog_mem);
    ASSERT_TRUE(fetch());

    EXPECT_EQ(cntrl_regs[OPERATION], 0x13u);
    EXPECT_EQ(cntrl_regs[OPERAND_1], 0x01u);  // rd
    EXPECT_EQ(cntrl_regs[OPERAND_2], 0x02u);  // rs
    EXPECT_EQ(cntrl_regs[OPERAND_3], 0x00u);  // rt / unused
    EXPECT_EQ(cntrl_regs[IMMEDIATE], 0x12345678u);

    EXPECT_EQ(reg_file[PC], 8u);  // PC advanced by 8 bytes
}

TEST_F(EmulatorTest, FetchFailsWhenOutOfBounds) {
    reg_file[PC] = mem_size - 3;  // only 3 bytes remain
    EXPECT_FALSE(fetch());
}
