find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.8.2 REQUIRED)

bison_target(DBUF_PARSER dbuf.y ${CMAKE_CURRENT_BINARY_DIR}/dbuf.tab.cpp
  COMPILE_FLAGS "--warnings=all --warnings=other --warnings=cex --warnings=error")
flex_target(DBUF_LEXER dbuf.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
add_flex_bison_dependency(DBUF_LEXER DBUF_PARSER)

add_library(parser STATIC
  ${BISON_DBUF_PARSER_OUTPUTS}
  ${FLEX_DBUF_LEXER_OUTPUTS}
  parse_helper.cc
)
target_include_directories(parser PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
target_link_libraries(parser PUBLIC
  dbufAst
  interning
)
