cmake_minimum_required(VERSION 3.29)

project(kRendrr)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CMake/Directx/CreateDirectXTarget.cmake)
#include(CMake/Directx/CreateDirectxtk12Target.cmake)

add_subdirectory(Common)
add_subdirectory(Examples)