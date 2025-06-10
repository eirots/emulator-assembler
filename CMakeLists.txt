cmake_minimum_required(VERSION 3.14)
project(CS4320_PROJECT0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ──────────────────────────────────────────────────────────────
# Where ALL artefacts go (inside build tree for cleanliness)
# ──────────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)

# ──────────────────────────────────────────────────────────────
# Shared object library for utils.cpp (re-used by several exe’s)
# ──────────────────────────────────────────────────────────────
add_library(utils_objects OBJECT src/utils.cpp)
target_include_directories(utils_objects PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(instruction_objects OBJECT
    src/jump.cpp
    src/move.cpp
    src/arith.cpp
    src/interrupt.cpp)
target_include_directories(instruction_objects PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ──────────────────────────────────────────────────────────────
# Tools / executables
# ──────────────────────────────────────────────────────────────
add_executable(makebinary tools/makebinary.cpp  $<TARGET_OBJECTS:utils_objects>)
target_include_directories(makebinary PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(emu4380     
  src/main.cpp 
  src/emu4380.cpp 
  $<TARGET_OBJECTS:utils_objects>
  $<TARGET_OBJECTS:instruction_objects>)

  target_include_directories(emu4380 PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ──────────────────────────────────────────────────────────────
#  GoogleTest + GoogleMock (fetched once)
# ──────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)   # provides GTest::gtest, GTest::gmock, etc.

enable_testing()

# ──────────────────────────────────────────────────────────────
# Unit-test target
# ──────────────────────────────────────────────────────────────
add_executable(runTests
    test/test.cpp
    src/emu4380.cpp
    $<TARGET_OBJECTS:utils_objects>
    $<TARGET_OBJECTS:instruction_objects>
)
target_include_directories(runTests PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ---  NEW: link gMock as well  --------------------------------
target_link_libraries(runTests
    PRIVATE
      GTest::gtest_main   # supplies main()
      GTest::gmock        # EXPECT_THAT, matchers, etc.
)

# Copy sample binary beside test executable
add_custom_command(
  TARGET runTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_SOURCE_DIR}/test/testFiles/smalladd.bin
          $<TARGET_FILE_DIR:runTests>
)

# Convenience ‘make check’ target
add_custom_target(check
  COMMAND runTests
  WORKING_DIRECTORY $<TARGET_FILE_DIR:runTests>
  DEPENDS runTests)

# Let CTest discover individual Google-test cases
include(GoogleTest)
gtest_discover_tests(runTests
  WORKING_DIRECTORY $<TARGET_FILE_DIR:runTests>)
