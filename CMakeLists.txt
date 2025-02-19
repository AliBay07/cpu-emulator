cmake_minimum_required(VERSION 3.27)
project(cpu_emulator)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(cpu_emulator
        src/cpu.h
        test/tests.cpp)

target_link_libraries(cpu_emulator ${GTEST_LIBRARIES} pthread)

enable_testing()

add_test(NAME cpu_emulator_test COMMAND cpu_emulator)
