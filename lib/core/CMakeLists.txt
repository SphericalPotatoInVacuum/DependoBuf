add_subdirectory(parser)

add_library(core INTERFACE)
target_link_libraries(core INTERFACE parser)
