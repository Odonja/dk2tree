cmake_minimum_required(VERSION 3.10)
project(untitled)

set(CMAKE_CXX_STANDARD 14)

include_directories(~/include)
link_directories(~/lib)

add_executable(untitled main.cpp)
target_link_libraries(untitled LINK_PUBLIC sdsl divsufsort divsufsort64)