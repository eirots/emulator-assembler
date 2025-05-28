cmake_minimum_required(VERSION 3.14)
project(CS4320_PROJECT0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)   # .a / .lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)   # .so / .dylib


add_library(utils_objects OBJECT
    src/utils.cpp
)

target_include_directories(utils_objects PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(makebinary
    tools/makebinary.cpp
    $<TARGET_OBJECTS:utils_objects>
)
target_include_directories(makebinary
    PUBLIC $(PROJECT_SOURCE_DIR)/include   
)


# Build the main executable
add_executable(emu4380
    src/main.cpp
    $<TARGET_OBJECTS:utils_objects>
)
target_include_directories(emu4380
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

#setup / install gtest and build test executable 
include(FetchContent)
FetchContent_Declare(
    googletest 
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

#test executable
add_executable(runTests
    test/test.cpp
    $<TARGET_OBJECTS:utils_objects>

)

target_link_libraries(runTests
    PRIVATE
        GTest::gtest_main     # from googletest
                     
)

include(GoogleTest)
gtest_discover_tests(runTests)