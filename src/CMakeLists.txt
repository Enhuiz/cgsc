include_directories(${csv_parser_SOURCE_DIR})
include_directories(${gtest_SOURCE_DIR})
include_directories(${json_SOURCE_DIR})

# add_subdirectory(model)
# add_subdirectory(solver)
# add_subdirectory(utils)

add_executable(expt expt.cpp)
target_link_libraries(expt pthread boost_program_options)

# add_executable(illus illus.cpp)
# target_link_libraries(illus model solver utils pthread boost_program_options)