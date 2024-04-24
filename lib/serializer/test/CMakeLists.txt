add_executable(serializer_test_serialization
    test_serialization.c
)

target_include_directories(serializer_test_serialization PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)

target_link_libraries(serializer_test_serialization PUBLIC
    serializer_serialization
    serializer_converter
)

target_compile_options(serializer_test_serialization PRIVATE -g)