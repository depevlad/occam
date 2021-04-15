cmake_minimum_required(VERSION 3.18.2)

project(app)

include_directories(app ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_subdirectory(./assembler)