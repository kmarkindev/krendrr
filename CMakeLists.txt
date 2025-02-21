cmake_minimum_required(VERSION 3.29)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_EXAMPLES "Example executables are gonna be built if this is ON" ON)
option(BUILD_TESTS "Test executables are gonna be build if this is ON" ON)

project(kRendrr)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(Sources)
add_subdirectory(Examples)
add_subdirectory(Tests)