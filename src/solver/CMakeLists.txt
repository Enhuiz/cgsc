add_library(solver greedy.cpp solver.cpp)

target_link_libraries(solver model utils)