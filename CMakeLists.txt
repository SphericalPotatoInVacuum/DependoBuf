cmake_minimum_required(VERSION 3.21)

project(DBUF VERSION 1.0 LANGUAGES CXX)

set(SRC_DIR "src")
set(PARSER_DIR "${SRC_DIR}/parser")

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.4 REQUIRED)

bison_target(PARSER "${PARSER_DIR}/dbuf.yy" ${CMAKE_CURRENT_BINARY_DIR}/dbuf.tab.cpp)
flex_target(LEXER "${PARSER_DIR}/dbuf.l"  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(LEXER PARSER)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dbuf "${SRC_DIR}/main.cpp" "${SRC_DIR}/parser/driver.cpp" "${BISON_PARSER_OUTPUTS}" "${FLEX_LEXER_OUTPUTS}")
target_include_directories(dbuf PRIVATE "${SRC_DIR}")
