cmake_minimum_required(VERSION 3.21)

project(DBUF VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_COMPILER_VERSION "16.0.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Check for Clang
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "Clang is required but not found on your system. Please use Clang as the C++ compiler.")
endif()

# Use libc++ and libc++abi
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -lc++abi")

add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("test")
