cmake_minimum_required(VERSION 3.2)
project(CGSC)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(Boost COMPONENTS)
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

include_directories(include)

add_subdirectory(lib/fast-cpp-csv-parser)
# add_subdirectory(lib/googletest)
add_subdirectory(lib/json)

add_subdirectory(src)
# add_subdirectory(test)