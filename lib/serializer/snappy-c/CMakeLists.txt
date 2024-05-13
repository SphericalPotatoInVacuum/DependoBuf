#Library functional entirely borrowed from https://github.com/andikleen/snappy-c :D

project(serializer_snappy-c VERSION 1.1.0 DESCRIPTION "Snappy C port from https://github.com/andikleen/snappy-c by Andi Kleen")

add_library(serializer_snappy-c STATIC
    snappy-c.c
)

set_target_properties(serializer_snappy-c PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(serializer_snappy-c PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(serializer_snappy-c PUBLIC
 ${CMAKE_CURRENT_BINARY_DIR}
 include
)
