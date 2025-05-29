#include "gtest/gtest.h"

int runEmulator(int argc, char** argv);

TEST(StartupTest, NoArguments) {
    const char* argv[] = {"emu4380"};
    int argc = 1;
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 1);  // Expect usage error
}

TEST(StartupTest, OnlyInputFile) {
    const char* argv[] = {"emu4380", "dummy_input.bin"};
    int argc = 2;
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 0);  // Should be fine
}

TEST(StartupTest, WithMemorySize) {
    const char* argv[] = {"emu4380", "dummy_input.bin", "65536"};
    int argc = 3;
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 0);  // Should be fine
}

TEST(StartupTest, InvalidMemorySize) {
    const char* argv[] = {"emu4380", "dummy_input.bin", "not_a_number"};
    int argc = 3;
    // stoi() would throw an exception, so you might want to catch this in main()
    // Here, we expect it to crash or handle gracefully
    EXPECT_EQ(runEmulator(argc, const_cast<char**>(argv)), 1);
}