add_executable(serializer_test_serialization
    test_serialization.c
)

add_executable(serializer_test_barray
    test_barray.c
)

add_executable(serializer_test_varint
    test_varint.c
)

target_link_libraries(serializer_test_serialization PRIVATE
    serializer_serialization
    serializer_converter
)

target_link_libraries(serializer_test_barray PRIVATE
    serializer_converter
)

target_link_libraries(serializer_test_varint PRIVATE
    serializer_converter
)