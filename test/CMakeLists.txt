enable_testing()


add_executable(dbufTests test.cc parser_test.cc positivity_test.cc name_resolution_test.cc compile_test.cc avaliable_formats_test.cc lexer_test.cc cpp_test.cc kotlin_test.cc)
target_link_libraries(dbufTests PRIVATE dbufAst driver gtest gtest_main pthread glog)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(dbufTests PRIVATE -fsanitize=undefined)
  target_link_options(dbufTests PRIVATE -fsanitize=undefined)
endif()


include(GoogleTest)
gtest_discover_tests(dbufTests)
