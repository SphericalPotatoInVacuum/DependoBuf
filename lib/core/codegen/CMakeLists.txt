add_library(codegen STATIC
  generation.cc
  cpp_gen.cc
  sharp_target/sharp_gen.cc
  sharp_target/sharp_print.cc
)
target_include_directories(codegen PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
  sharp_target/include
)
target_link_libraries(codegen PUBLIC
  dbufAst
)
