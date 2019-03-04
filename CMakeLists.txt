cmake_minimum_required(VERSION 3.10)
project(dk2tree)

set(CMAKE_CXX_STANDARD 14)

# enable maximum optimisation for release mode
# comment this line out for faster compilation and to allow debugging, but slower resulting code
set(CMAKE_BUILD_TYPE Release)

# Add include and lib direc
include_directories(~/include)
link_directories(~/lib)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(dk2tree_test main_test.cpp)
#target_link_libraries(dk2tree LINK_PUBLIC sdsl divsufsort divsufsort64 gtest Threads::Threads)
target_link_libraries(dk2tree_test LINK_PUBLIC gtest Threads::Threads stdc++)

add_executable(dk2tree main.cpp)
#target_link_libraries(dk2tree LINK_PUBLIC sdsl divsufsort divsufsort64 gtest Threads::Threads)
target_link_libraries(dk2tree LINK_PUBLIC gtest Threads::Threads stdc++)