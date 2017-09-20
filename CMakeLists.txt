cmake_minimum_required(VERSION 3.2)
project(CGSC)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost COMPONENTS)

include_directories(include)

add_subdirectory(lib/fast-cpp-csv-parser)
add_subdirectory(lib/googletest)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(benchmark)
