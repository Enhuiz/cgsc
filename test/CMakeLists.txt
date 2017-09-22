include_directories(${csv_parser_SOURCE_DIR})
include_directories(${gtest_SOURCE_DIR})

add_executable(mytest test.cpp)

target_link_libraries(mytest gtest gtest_main pthread)