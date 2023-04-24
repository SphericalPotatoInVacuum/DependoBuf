add_library(ast
  INTERFACE
)
target_include_directories(ast INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
target_link_libraries(ast INTERFACE
  interning
)
