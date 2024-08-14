add_library(codegen STATIC
  generation.cc
  cpp_gen.cc
  kotlin_target/kotlin_gen.cc
  kotlin_target/kotlin_printer.cc
  kotlin_target/kotlin_objects.cc
)
target_include_directories(codegen PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
target_link_libraries(codegen PUBLIC
  dbufAst
)
