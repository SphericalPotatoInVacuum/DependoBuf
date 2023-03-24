cmake_minimum_required(VERSION 3.21)

project(DBUF VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR "src")
set(PARSER_DIR "${SRC_DIR}/parser")

set(TEST_DIR "test")

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.4 REQUIRED)

bison_target(PARSER "${PARSER_DIR}/dbuf.yy" ${CMAKE_CURRENT_BINARY_DIR}/dbuf.tab.cpp)
flex_target(LEXER "${PARSER_DIR}/dbuf.l"  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(LEXER PARSER)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dbuf "${SRC_DIR}/main.cpp" "${SRC_DIR}/parser/driver.cpp" "${BISON_PARSER_OUTPUTS}" "${FLEX_LEXER_OUTPUTS}")
target_include_directories(dbuf PRIVATE "${SRC_DIR}")

enable_testing()

add_executable(parserTest "${TEST_DIR}/parser_test.cpp" "${SRC_DIR}/parser/driver.cpp" "${BISON_PARSER_OUTPUTS}" "${FLEX_LEXER_OUTPUTS}")
target_include_directories(parserTest PRIVATE "${SRC_DIR}")
target_link_libraries(parserTest PRIVATE GTest::GTest pthread)
