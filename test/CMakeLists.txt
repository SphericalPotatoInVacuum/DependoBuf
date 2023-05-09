include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(dbufTests test.cc parser_test.cc positivity_test.cc name_resolution_test.cc compile_test.cc)
target_link_libraries(dbufTests PRIVATE dbufAst driver gtest gtest_main pthread glog)

include(GoogleTest)
gtest_discover_tests(dbufTests)
