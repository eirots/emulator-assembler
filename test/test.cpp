// ──────────────────────────────────────────────────────────────
//  Google-Test file:  test/test.cpp
// ──────────────────────────────────────────────────────────────
#include <gmock/gmock.h>  // EXPECT_THAT, ElementsAreArray
#include <gtest/gtest.h>

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
    constexpr std::array<unsigned char, 5> kExpected{0x00, 0x05, 0x0F, 0x00, 0x12};

    ASSERT_TRUE(fetch());  // should succeed
    std::vector<unsigned char> buffer(prog_mem,
                                      prog_mem + kExpected.size());
    EXPECT_THAT(buffer,
                ::testing::ElementsAreArray((unsigned char[]){0x00, 0x05, 0x0F, 0x00, 0x12}));

    EXPECT_EQ(reg_file[PC], 5u);  // PC advanced by 5
}

TEST_F(EmulatorTest, FetchFailsWhenOutOfBounds) {
    reg_file[PC] = mem_size - 3;  // only 3 bytes remain
    EXPECT_FALSE(fetch());
}
