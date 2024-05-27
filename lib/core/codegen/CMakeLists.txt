add_library(codegen STATIC
  generation.cc
  cpp_gen.cc
  python_target/py_gen.cc
  python_target/py_print.cc
  python_target/py_expression.cc
  python_target/utils.cc
)
target_include_directories(codegen PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
  python_target/include
)
target_link_libraries(codegen PUBLIC
  dbufAst
)
