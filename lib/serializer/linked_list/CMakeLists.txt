add_library(serializer_list STATIC
    linked_list.c
)

target_include_directories(serializer_list PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

target_link_libraries(serializer_list PUBLIC
    serializer_error_handler
    serializer_layout
)