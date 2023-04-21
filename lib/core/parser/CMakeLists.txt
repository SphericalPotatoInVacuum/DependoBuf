find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.4 REQUIRED)

bison_target(DBUF_PARSER dbuf.yy ${CMAKE_CURRENT_BINARY_DIR}/dbuf.tab.cpp)
flex_target(DBUF_LEXER dbuf.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(DBUF_LEXER DBUF_PARSER)

add_library(parser
  ${BISON_DBUF_PARSER_OUTPUTS}
  ${FLEX_DBUF_LEXER_OUTPUTS}
  ast.cc
  driver.cc
)

target_include_directories(parser PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
