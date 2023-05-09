add_library(driver
  driver.cc
)
target_include_directories(driver PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  include
)
target_link_libraries(driver PUBLIC
  dbufAst
  parser
  checker
  substitutor
)
