add_subdirectory(ast)
add_subdirectory(parser)
add_subdirectory(checker)

add_library(core
  driver.cc
)
target_include_directories(core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  include
)
target_link_libraries(core PUBLIC
  ast
  parser
  checker
)
