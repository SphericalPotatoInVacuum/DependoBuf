add_executable(dbuf "main.cc")
target_compile_options(dbuf PRIVATE -Werror -Wall -Wextra -Wpedantic -Wunused -Wunreachable-code)
target_include_directories(dbuf PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(dbuf PRIVATE -fsanitize=undefined)
  target_link_options(dbuf PRIVATE -fsanitize=undefined)
endif()
target_link_libraries(dbuf PRIVATE driver glog CLI11)
install(TARGETS dbuf DESTINATION bin)
