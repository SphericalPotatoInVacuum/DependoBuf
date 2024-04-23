add_library(serializer_layout STATIC
    layout.c
)

target_include_directories(serializer_layout PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

target_link_libraries(serializer_layout PUBLIC
    serializer_error_handler
)