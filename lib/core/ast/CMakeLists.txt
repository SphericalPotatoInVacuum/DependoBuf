add_library(dbufAst
  INTERFACE
)
target_include_directories(dbufAst INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
target_link_libraries(dbufAst INTERFACE
  interning
  parser
)
