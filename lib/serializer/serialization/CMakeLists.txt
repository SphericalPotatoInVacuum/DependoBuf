project(serializer_serialization VERSION 1.0.0 DESCRIPTION "C Library with serialization/deserialization algorithms")

add_library(serializer_serialization STATIC
    serialization.c
    deserialization.c
)

target_include_directories(serializer_serialization PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

set_target_properties(serializer_serialization PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(serializer_serialization PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_link_libraries(serializer_serialization PUBLIC
    serializer_layout
)

#target_compile_options(serializer_serialization PRIVATE -Werror -Wall -Wextra -Wpedantic -Wunused -Wunreachable-code)