cmake_minimum_required(VERSION 3.18.2)

project(auxlib)

set(CMAKE_CXX_STANDARD 17)

add_library(
	auxlib STATIC
	placeholder.cpp
	BException.h
	Map.h
	Types.h
)