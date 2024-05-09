cmake_minimum_required(VERSION 3.14)

project(DBUF VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.0.0)
        message(FATAL_ERROR "Cwang 16 ow newew is needed, but ${CMAKE_CXX_COMPILER_VERSION} is used.\
                Pwease use Cwang 16 or newew fow the C++ compiler, ow we can't pway togethew (｡•́︿•̀｡)")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else()
    message(FATAL_ERROR "OwOly Cwang is suppowted, sowwy (｡•́︿•̀｡)")
endif()

add_subdirectory("lib")
add_subdirectory("src")
add_subdirectory("test")

# to start fuzzer remove -stdlib=libc++ from CMAKE_CXX_FLAGS
# add_subdirectory("fuzzer")