cmake_minimum_required(VERSION 3.10)
project(untitled)

set(CMAKE_CXX_STANDARD 14)

# Add include and lib direc
include_directories(~/include)
link_directories(~/lib)

add_executable(untitled main.cpp TTree.cpp TTree.h BitVector.cpp BitVector.h)
target_link_libraries(untitled LINK_PUBLIC sdsl divsufsort divsufsort64)