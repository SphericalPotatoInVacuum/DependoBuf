add_library(serializer_error_handler STATIC
    error_handler.c
)

target_include_directories(serializer_error_handler PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)