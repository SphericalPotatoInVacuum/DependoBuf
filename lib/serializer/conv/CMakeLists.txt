project(serializer_converter VERSION 1.0.0 DESCRIPTION "Static C Library with converters for memory-efficient data types for serialization")

add_library(serializer_converter STATIC
    conv.c
)

set_target_properties(serializer_converter PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(serializer_converter PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(serializer_converter PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

project(serializer_converter_shared VERSION 1.0.0 DESCRIPTION "Shared C Library with converters for memory-efficient data types for serialization")

add_library(serializer_converter_shared SHARED
    conv.c
)

set_target_properties( PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(serializer_converter_shared PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(serializer_converter_shared PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)