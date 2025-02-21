if(NOT BUILD_EXAMPLES)
    message(STATUS "Examples are not gonna be build")
    return()
else()
    message(STATUS "Examples are gonna be built")
endif()

add_subdirectory(SumExample)