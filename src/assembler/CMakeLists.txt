cmake_minimum_required(VERSION 3.18.2)

project(asm)

add_executable(
	asm
	asm.cpp
	main.cpp
	include/asm.h
)

target_link_libraries(
	asm PUBLIC 
	absl::strings 
	absl::flags 
	absl::flags_parse 
)