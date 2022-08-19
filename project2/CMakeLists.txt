cmake_minimum_required (VERSION 3.20)

project (DEAD_CODE_ELIMINATION)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)

execute_process (COMMAND llvm-config --cxxflags --ldflags OUTPUT_VARIABLE LLVM_CONFIG_N)

include_directories(${LLVM_INCLUDE_DIRS})

string(REPLACE "\n" " " LLVM_CONFIG "${LLVM_CONFIG_N}")
message(STATUS "LLVM_CONFIG: ${LLVM_CONFIG}")

# set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set (CMAKE_CXX_FLAGS "-O3 -g ${LLVM_CONFIG} -undefined dynamic_lookup")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdiagnostics-color=always")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# include_directories (include)
include_directories (RangeAnalysis)
# add_library (RACli SHARED
# RangeAnalysis/RangeAnalysis.cpp
# RangeAnalysis/vSSA.cpp
# RangeAnalysis/ClientRA.cpp
# )

add_library (RADeadCodeElimination SHARED
RangeAnalysis/RangeAnalysis.cpp
RangeAnalysis/vSSA.cpp
RangeAnalysis/DeadCodeElimination.cpp
)
