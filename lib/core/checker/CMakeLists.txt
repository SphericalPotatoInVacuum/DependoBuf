add_library(checker
  checker.cc
  name_resolution_checker.cc
  positivity_checker.cc
  type_expression_checker.cc
)

target_include_directories(checker PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
target_link_libraries(checker PUBLIC
  ast
)
