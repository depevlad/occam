cmake_minimum_required(VERSION 3.18.2)

project(all)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_subdirectory(./lib)
add_subdirectory(./src)
add_subdirectory(./test)

