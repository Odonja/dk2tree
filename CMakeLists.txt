cmake_minimum_required(VERSION 3.10)
project(dk2tree)

set(CMAKE_CXX_STANDARD 14)

# Add include and lib direc
include_directories(~/include)
link_directories(~/lib)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(dk2tree main.cpp TTree.cpp TTree.h BitVector.cpp BitVector.h DKTree.cpp DKTree.h DKTreeTest.cpp)
target_link_libraries(dk2tree LINK_PUBLIC sdsl divsufsort divsufsort64 gtest Threads::Threads)