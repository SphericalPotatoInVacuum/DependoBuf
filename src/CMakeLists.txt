add_executable(dbuf "main.cpp")
target_compile_options(dbuf PRIVATE -Wall -Wextra -Wpedantic -fsanitize=undefined)
target_link_libraries(dbuf PRIVATE core)
target_link_options(dbuf PRIVATE -fsanitize=undefined)
