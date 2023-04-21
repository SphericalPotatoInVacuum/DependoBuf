add_library(ast
  ast.cc
)

target_include_directories(ast PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
