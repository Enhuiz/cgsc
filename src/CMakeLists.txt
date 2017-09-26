include_directories(${csv_parser_SOURCE_DIR})
include_directories(${gtest_SOURCE_DIR})
include_directories(${json_SOURCE_DIR})

add_subdirectory(model)
add_subdirectory(solver)
add_subdirectory(utils)

add_executable(main main.cpp)
target_link_libraries(main model solver utils pthread)