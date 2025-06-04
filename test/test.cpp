#include "emu4380.h"
#include "gtest/gtest.h"

int runEmulator(int argc, char** argv);

TEST(StartupTest, NoArguments) {
    const char* argv[] = {"emu4380"};
    int argc = 1;
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 1);  // Expect usage error
}

TEST(StartupTest, OnlyInputFile) {
    const char* argv[] = {"emu4380", "smalladd.bin"};
    int argc = 2;
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 0);  // Should be fine
}

TEST(StartupTest, WithMemorySize) {
    const char* argv[] = {"emu4380", "smalladd.bin", "65536"};
    int argc = 3;
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 0);  // Should be fine
}

TEST(StartupTest, InvalidMemorySize) {
    const char* argv[] = {"emu4380", "dummy_input.bin", "not_a_number"};
    int argc = 3;
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 1);
}

TEST(StartupTest, RegisterInitialize) {
    unsigned int testmemsize = 65536;
    init_mem(testmemsize);
    EXPECT_EQ(reg_file[PC], 0);
    EXPECT_EQ(reg_file[SL], 0);
    EXPECT_EQ(reg_file[SB], testmemsize);
    EXPECT_EQ(reg_file[SP], testmemsize);
    EXPECT_EQ(reg_file[FP], reg_file[SP]);
    EXPECT_EQ(reg_file[HP], reg_file[SL]);
}  // expect all of these to pass