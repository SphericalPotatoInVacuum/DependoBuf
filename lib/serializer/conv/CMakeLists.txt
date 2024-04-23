project(serializer_converter VERSION 1.0.0 DESCRIPTION "C Library with converters for memory-efficient data types for serialization")

add_library(serializer_converter STATIC
    conv.c
)

target_include_directories(serializer_converter PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

set_target_properties(serializer_converter PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(serializer_converter PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})