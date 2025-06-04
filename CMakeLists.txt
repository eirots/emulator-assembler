cmake_minimum_required(VERSION 3.14)
project(CS4320_PROJECT0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)




# ──────────────────────────────────────────────────────────────
# Where final artefacts go
# ──────────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/lib)   # .a / .lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/lib)   # .so / .dylib

# ──────────────────────────────────────────────────────────────
# Object library for code that is reused (utils.cpp)
# ──────────────────────────────────────────────────────────────
add_library(utils_objects OBJECT
    src/utils.cpp
)
target_include_directories(utils_objects
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# ──────────────────────────────────────────────────────────────
# Tool: makebinary
# ──────────────────────────────────────────────────────────────
add_executable(makebinary
    tools/makebinary.cpp
    $<TARGET_OBJECTS:utils_objects>
)
target_include_directories(makebinary
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# ──────────────────────────────────────────────────────────────
# Main emulator executable
# ──────────────────────────────────────────────────────────────
add_executable(emu4380
    src/main.cpp
    src/emu4380.cpp
    $<TARGET_OBJECTS:utils_objects>
)
target_include_directories(emu4380
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# ──────────────────────────────────────────────────────────────
# GoogleTest
# ──────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()



# ──────────────────────────────────────────────────────────────
# Unit-test target
#   NOTE:  we do NOT compile src/main.cpp here – GTest supplies
#   its own main().  We only need the emulator implementation
#   and utils.
# ──────────────────────────────────────────────────────────────
add_executable(runTests
    test/test.cpp
    src/emu4380.cpp
    $<TARGET_OBJECTS:utils_objects>
)




target_include_directories(runTests
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(runTests
    PRIVATE GTest::gtest_main
)

include(GoogleTest)
# copy file beside the executable if you haven't already
add_custom_command(
    TARGET runTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/test/testFiles/smalladd.bin
            $<TARGET_FILE_DIR:runTests>
)
add_custom_target(check
    COMMAND runTests
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}   # = <proj>/bin
    DEPENDS runTests)


gtest_discover_tests(runTests
    WORKING_DIRECTORY $<TARGET_FILE_DIR:runTests>)
