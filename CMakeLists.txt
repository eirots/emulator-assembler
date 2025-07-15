cmake_minimum_required(VERSION 3.14)
project(CS4320_PROJECT0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ──────────────────────────────────────────────────────────────
# Where *all* artefacts go   <build>/bin   <build>/lib
# ──────────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/lib")

# ──────────────────────────────────────────────────────────────
# 1.  Re-usable object library for utils.cpp
# ──────────────────────────────────────────────────────────────
add_library(utils_objects OBJECT src/utils.cpp)
target_include_directories(utils_objects PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ──────────────────────────────────────────────────────────────
# 2.  Main emulator executable
# ──────────────────────────────────────────────────────────────
add_executable(emu4380
    src/emu4380.cpp
    src/main.cpp
    $<TARGET_OBJECTS:utils_objects>)
target_include_directories(emu4380 PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ──────────────────────────────────────────────────────────────
# 3.  Tool: makebinary   (optional helper)
# ──────────────────────────────────────────────────────────────
add_executable(makebinary
    tools/makebinary.cpp
    $<TARGET_OBJECTS:utils_objects>)
target_include_directories(makebinary PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ──────────────────────────────────────────────────────────────
# 4.  GoogleTest & GoogleMock
# ──────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
FetchContent_MakeAvailable(googletest)   # exports GTest::gtest, gmock, …

enable_testing()

# ──────────────────────────────────────────────────────────────
# 5.  Unit-test target
# ──────────────────────────────────────────────────────────────
add_executable(runTests
    test/test.cpp
    src/emu4380.cpp               # compile emulator again for test binary
    $<TARGET_OBJECTS:utils_objects>)
target_include_directories(runTests PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(runTests PRIVATE
    GTest::gtest_main
    GTest::gmock)

# Copy test asset beside the test executable
add_custom_command(TARGET runTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_SOURCE_DIR}/test/testFiles/smalladd.bin
          $<TARGET_FILE_DIR:runTests>)

# ‘make check’ helper
add_custom_target(check
  COMMAND runTests
  WORKING_DIRECTORY $<TARGET_FILE_DIR:runTests>
  DEPENDS runTests)

include(GoogleTest)
gtest_discover_tests(runTests
  WORKING_DIRECTORY $<TARGET_FILE_DIR:runTests>)
