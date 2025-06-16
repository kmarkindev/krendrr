cmake_minimum_required(VERSION 3.29)

project(krendrr)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
include(FetchContent)
include(ContentHelpers)
include(CMakeHelpers)

add_subdirectory(Source)