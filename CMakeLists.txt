cmake_minimum_required(VERSION 3.2)
project(CGSC)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(DCMAKE_CXX_FLAGS "-O3")

find_package(Boost COMPONENTS program_options REQUIRED)

add_subdirectory(src)