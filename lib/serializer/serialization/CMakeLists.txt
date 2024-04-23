add_library(serializer_serialization STATIC
    serialization.c
    deserialization.c
)

target_include_directories(serializer_serialization PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

target_link_libraries(serializer_serialization PUBLIC
    serializer_list
)

#target_compile_options(serializer_serialization PRIVATE -Werror -Wall -Wextra -Wpedantic -Wunused -Wunreachable-code)