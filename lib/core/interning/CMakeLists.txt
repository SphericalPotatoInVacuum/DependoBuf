add_library(interning
  interned_string.cc
)

target_include_directories(interning PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)
