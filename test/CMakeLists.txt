include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(dbufTests "parser_test.cc")
target_link_libraries(dbufTests PRIVATE core gtest gtest_main pthread)

include(GoogleTest)
gtest_discover_tests(dbufTests)
