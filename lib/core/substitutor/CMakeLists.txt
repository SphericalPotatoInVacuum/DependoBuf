add_library(substitutor STATIC
  substitutor.cc
)

target_include_directories(substitutor PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  include
)

target_link_libraries(substitutor PUBLIC
  dbufAst
  glog
  interning
)

add_dependencies(substitutor parser)
