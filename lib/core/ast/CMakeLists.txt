add_library(dbufAst
  STATIC
  ast.cpp
)
target_include_directories(dbufAst PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
target_link_libraries(dbufAst PUBLIC
  interning
  parser
)
