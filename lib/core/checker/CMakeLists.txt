include(FetchContent)

FetchContent_Declare(
  z3
  GIT_REPOSITORY https://github.com/Z3Prover/z3
  GIT_TAG        z3-4.12.1
)
FetchContent_MakeAvailable(z3)

add_library(checker STATIC
  checker.cc
  name_resolution_checker.cc
  positivity_checker.cc
  type_checker.cc
  type_comparator.cc
  expression_comparator.cc
  generic_resolver.cc
)

target_include_directories(checker PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${z3_SOURCE_DIR}/src/api/c++
  include
)
target_link_libraries(checker PUBLIC
  substitutor
  libz3
  dbufAst
  glog
)
target_compile_options(checker PRIVATE -Werror -Wall -Wextra -Wpedantic -Wunused -Wunreachable-code)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(checker PRIVATE -fsanitize=undefined)
  target_link_options(checker PRIVATE -fsanitize=undefined)
endif()
