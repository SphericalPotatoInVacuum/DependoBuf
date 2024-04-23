add_library(serializer_converter STATIC
    conv.c
)

target_include_directories(serializer_converter PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)