cmake_minimum_required(VERSION 3.18.2)

project(tester)

set(CMAKE_CXX_STANDARD 20)

add_executable(
	tester 
	assembler_test.cpp
)

target_include_directories(
	tester 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib/
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../app/asm/
)

target_link_libraries(
	tester
	gtest_main
)

include(GoogleTest)

gtest_discover_tests(tester)