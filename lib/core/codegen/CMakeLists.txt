add_library(codegen STATIC
  generation.cc
  cpp_gen.cc
  py_gen.cc
)
target_include_directories(codegen PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
target_link_libraries(codegen PUBLIC
  dbufAst
)
