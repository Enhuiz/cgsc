project(CGSC_benchmarks)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/include
)

#
# Build PartialCsvParser benchmark
ADD_EXECUTABLE(CGSC_benchmarks benchmarks.cpp)
