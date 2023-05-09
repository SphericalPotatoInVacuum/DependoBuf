enable_testing()

add_executable(dbufTests test.cc parser_test.cc positivity_test.cc name_resolution_test.cc compile_test.cc)
target_link_libraries(dbufTests PRIVATE dbufAst driver gtest gtest_main pthread glog)

include(GoogleTest)
gtest_discover_tests(dbufTests)
