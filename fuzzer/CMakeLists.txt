add_executable(fuzz 
    fuzz.cc
)

target_compile_options(fuzz PRIVATE -g -fsanitize=fuzzer)
target_link_libraries(fuzz PRIVATE -fsanitize=fuzzer)

target_link_libraries(fuzz PRIVATE
  dbufAst
  parser
  checker
  codegen
)
