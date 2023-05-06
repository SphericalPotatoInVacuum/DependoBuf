add_library(substitutor
  substitutor.cc
)

target_include_directories(substitutor PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include  
)

target_link_libraries(substitutor PUBLIC
  ast
  core
  interning
)