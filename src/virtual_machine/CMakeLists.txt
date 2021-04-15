cmake_minimum_required(VERSION 3.18.2)

project(emu)

add_executable(
	emu
	Transputer.cpp
	main.cpp
	include/Memory.h
	include/Transputer.h
)

target_link_libraries(
	emu PUBLIC 
	absl::strings 
	absl::flags 
	absl::flags_parse 
)