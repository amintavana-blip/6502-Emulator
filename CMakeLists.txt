cmake_minimum_required(VERSION 3.14)
project(6502Emulator)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------
# 1. DOWNLOAD GOOGLE TEST
# -------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows/MSVC: Prevent gtest from overriding compiler settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# -------------------------------------------------------------
# 2. CREATE YOUR 6502 CORE LIBRARY
# -------------------------------------------------------------
# We group your emulator files into a library so the tests can talk to it.
# ADD ANY NEW .cpp FILES HERE (like mem.cpp if you have one)
add_library(6502Core cpu.cpp) 

# -------------------------------------------------------------
# 3. CREATE THE TEST RUNNER EXECUTABLE
# -------------------------------------------------------------
# This creates a new program called "6502Tests" using our test file
add_executable(6502Tests test.cpp)

# Link Google Test and your 6502 Core to the Test Executable
target_link_libraries(6502Tests 
    PRIVATE 
    6502Core 
    GTest::gtest_main # gtest_main provides the main() loop automatically!
)

# Optional: Enable CTest discovery so VS Code test explorers can find them
include(GoogleTest)
gtest_discover_tests(6502Tests)