include_directories(${csv_parser_SOURCE_DIR})
include_directories(${gtest_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)

link_directories(aoi)

add_executable(mytest test.cpp)

target_link_libraries(mytest model solver gtest gtest_main pthread)