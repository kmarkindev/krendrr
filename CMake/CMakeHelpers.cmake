# You can pass directory names to skip when executing this function as arguments
function(add_subdirectory_all_directories)

    file(GLOB MODULES_GLOB LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")
    set(EXCLUDED_LIST ${ARGV})

    foreach(MODULE ${MODULES_GLOB})
        list(FIND EXCLUDED_LIST ${MODULE} IS_EXCLUDED)

        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE} AND ${IS_EXCLUDED} EQUAL -1)
            message(${CMAKE_CURRENT_SOURCE_DIR}/${MODULE})
            add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${MODULE})
        endif()

    endforeach()

endfunction()

function(target_all_sources TARGET)

    file(GLOB_RECURSE GLOB_SOURCES FOLLOW_SYMLINKS
        "*.c"
        "*.cpp"
        "*.h"
        "*.hpp"
    )

    target_sources(${TARGET} PRIVATE ${GLOB_SOURCES})

endfunction()