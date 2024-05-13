cmake_minimum_required(VERSION 3.14)

project(DBUF VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.0.0)
        message(FATAL_ERROR "Clang 16 ow newer is needed, but ${CMAKE_CXX_COMPILER_VERSION} is used.\
                Please use Clang 16 or newer fow the C++ compiler, ow we can't play together (｡•́︿•̀｡)")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else()
    message(FATAL_ERROR "Only Clang is supported, sorry (｡•́︿•̀｡)")
endif()

add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("test")
