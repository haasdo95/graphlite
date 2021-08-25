cmake_minimum_required(VERSION 3.19)
project(graphlite)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
add_subdirectory(src)

add_subdirectory(tests)
add_subdirectory(lib/googletest)
