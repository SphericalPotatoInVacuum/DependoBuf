project(serializer_serialization VERSION 1.0.0 DESCRIPTION "Static C Library with serialization/deserialization algorithms")

add_library(serializer_serialization STATIC
    serialization.c
    deserialization.c
)

set_target_properties(serializer_serialization PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(serializer_serialization PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(serializer_serialization PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

target_link_libraries(serializer_serialization PUBLIC
    serializer_layout
)
target_link_libraries(serializer_serialization PRIVATE
    serializer_snappy-c
    serializer_converter
)